/**
 * collectd-agent
 * Copyright (C) 2011 Jason Schmidlapp
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the License is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 **/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/mib_api.h>

#include <collectd/client.h>

#include <assert.h>

#include "data.h"
#include "common.h"
#include "settings.h"
#include "client.h"
#include "agent_helper.h"

static int agent_running;


typedef struct row_entry {

	/* The statistic index for this row (given by the wildcard in the config file */
	int index;

	/* A pointer to the original table configuration, as set by the user in the config file */
	snmp_table_t *table_data;

	struct row_entry *next;

} row_entry_t;


void free_table_indexes(void *v_entry)
{
	row_entry_t *entry = (row_entry_t *) v_entry;

	while (entry != NULL) {
		row_entry_t *next_entry = entry->next;
		free(entry);
		entry = next_entry;
	}
}


/* Returns a list of rows, including index values - must be freed by caller */
static void *populate_table_indexes(snmp_table_t * table)
{
	uint32_t *indexes;
	size_t num_indexes;
	int result;
	row_entry_t *first_entry = NULL;
	int i;

	assert(table != NULL);

	result = get_list_of_indexes(table, &indexes, &num_indexes);

	if ((result != 0) || (indexes == NULL))
		return NULL;

	for (i = 0; i < num_indexes; i++) {

		row_entry_t *entry;

		entry = (row_entry_t *) malloc(sizeof(row_entry_t));
		entry->index = indexes[i];
		entry->next = first_entry;
		entry->table_data = table;

		first_entry = entry;
	}

	free(indexes);

	return first_entry;
}

static netsnmp_variable_list *
table_get_next_data_point(void **my_loop_context,
						  void **my_data_context,
						  netsnmp_variable_list * put_index_data,
						  netsnmp_iterator_info * mydata)
{
	row_entry_t *entry = (row_entry_t *) * my_loop_context;

	if (entry) {
		set_snmp_value(ASN_INTEGER, entry->index, put_index_data);

		*my_data_context = entry;
		*my_loop_context = entry->next;
	} else {
		*my_loop_context = NULL;
		return NULL;
	}

	return put_index_data;

}

static netsnmp_variable_list *
table_get_first_data_point(void **my_loop_context,
						   void **my_data_context,
						   netsnmp_variable_list * put_index_data,
						   netsnmp_iterator_info * mydata)
{
	snmp_table_t *table;
	row_entry_t *entry;

	assert(mydata != NULL);

	table = (snmp_table_t *) mydata->myvoid;

	if (table->agent_info != NULL) {
		free_table_indexes(table->agent_info);
	}

	entry = populate_table_indexes((snmp_table_t *) table);

	table->agent_info = entry;	// I think we're just saving this so we can free it later.  Right?

	*my_loop_context = entry;

	return table_get_next_data_point(my_loop_context, my_data_context, put_index_data, mydata);
}

static int
table_handler(netsnmp_mib_handler * handler,
			  netsnmp_handler_registration * reginfo,
			  netsnmp_agent_request_info * reqinfo, 
			  netsnmp_request_info * requests)
{
	netsnmp_request_info *request;

	DEBUG("Calling table handler\n");

	for (request = requests; request; request = request->next) {

		char *generic_stat_name;
		netsnmp_table_request_info *table_info;
		row_entry_t *entry;
		snmp_table_t *table_data;
		char *specific_stat_name;
		char index_str[3];
		int result;
		double stat_val;

		if (request->processed != 0) {
			continue;
		}

		table_info = netsnmp_extract_table_info(request);

		entry = (row_entry_t *) netsnmp_extract_iterator_context(request);

		if (entry == NULL) {
			continue;  
		}

		table_data = entry->table_data;

		if (table_info == NULL) {
			INFO("Couldn't get table info\n");
			continue; 
		}

		if (reqinfo->mode != MODE_GET) {
			continue;
		}

		DEBUG_OID(request->requestvb->name, request->requestvb->name_length);

		generic_stat_name = table_data->columns[table_info->colnum - 1].name;

		snprintf(index_str, sizeof(index_str), "%d", entry->index);
		specific_stat_name = replace_wildcard(generic_stat_name, index_str);

		if (specific_stat_name == NULL)
			return SNMP_ERR_GENERR;

		result = get_single_value_from_collectd(specific_stat_name, &stat_val);

		free(specific_stat_name);

		if (result != 0)
			return SNMP_ERR_GENERR;

		set_snmp_value(ASN_INTEGER, stat_val, request->requestvb);

	}

	return SNMP_ERR_NOERROR;
}




static int
value_handler(netsnmp_mib_handler * handler,
			  netsnmp_handler_registration * reginfo,
			  netsnmp_agent_request_info * reqinfo, 
			  netsnmp_request_info * request)
{
	int result;

	assert(reqinfo != NULL);

	/* We only support GETs, ignore anything else */
	if (reqinfo->mode != MODE_GET)
		return SNMP_ERR_NOERROR;

	DEBUG("Running value handler:\n");

	while (request != NULL) {

		netsnmp_variable_list *vb = request->requestvb;

		while (vb != NULL) {

			double dbl_value;
			snmp_value_t *value;
			
			DEBUG_OID(vb->name, vb->name_length);

			value = find_value_config_entry(vb->name, vb->name_length);

			if (value == NULL) {
				ERROR("Couldn't find statistic\n");
				return SNMP_ERR_GENERR;
			}

			DEBUG("   Stat name = %s\n", value->stat_name);

			result = get_single_value_from_collectd(value->stat_name, &dbl_value);

			DEBUG("   Stat value = %f\n", dbl_value);

			if (result != 0) {
				ERROR("Failed to get statistic from collectd for stat %s (error = %d)\n",
					  value->stat_name, result);
				return SNMP_ERR_GENERR;
			}
			
			set_snmp_value(value->type, dbl_value, request->requestvb);

			if (result != 0) {
				ERROR("Failed to set SNMP value for stat %s (error = %d)\n",
				      value->stat_name, result);
				return SNMP_ERR_GENERR;
			}

			vb = vb->next_variable;
		}

		request = request->next;
	}

	return SNMP_ERR_NOERROR;
}


static int register_value_handlers(void)
{
	int i;

	for (i = 0; i < value_list.num_values; i++) {

		int result;
		netsnmp_handler_registration *value_reg;
		snmp_value_t *value = &(value_list.values[i]);

		value->val_oid_len = MAX_OID_LEN;
		if ((result =
		     convert_string_to_oid(value->oid_str, value->val_oid,
					   &value->val_oid_len)) != 0) {
			return -1;
		}

		DEBUG("Registering handle for stat name %s, ", value->stat_name);
		DEBUG_OID(value->val_oid, value->val_oid_len);

		value_reg = netsnmp_create_handler_registration(value->stat_name,
								value_handler,
								value->val_oid,
								value->val_oid_len, 0);
		netsnmp_register_instance(value_reg);
	}

	return 0;
}

static int register_table_handlers(void)
{
	int i;
	int result;
	char table_name[9];

	for (i = 0; i < table_list.num_tables; i++) {

		netsnmp_handler_registration *reg;
		snmp_table_t *table;
		netsnmp_table_registration_info *table_info;
		netsnmp_iterator_info *iinfo;

		table = &(table_list.tables[i]);
		table->agent_info = NULL;

		table->table_oid_len = MAX_OID_LEN;
		if ((result =
		     convert_string_to_oid(table->table_oid_str, table->table_oid,
					   &table->table_oid_len)) != 0) {
			return -1;
		}

		DEBUG("Registering table handler for OID ");
		DEBUG_OID(table->table_oid, table->table_oid_len);

		snprintf(table_name, sizeof(table_name), "Table-%d", i);

		reg = netsnmp_create_handler_registration(table_name,
							  table_handler,
							  table->table_oid,
							  table->table_oid_len, 0);


		table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);

		if (table_info == NULL) {
			ERROR("Failed to allocate memory for SNMP table info\n");
			return -1;
		}

		netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER, 0);

		table_info->min_column = 1;
		table_info->max_column = table->num_columns;

		iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);

		if (iinfo == NULL) {
			ERROR("Failed to allocate memory for SNMP iinfo\n");
			return -1;
		}

		iinfo->get_first_data_point = table_get_first_data_point;
		iinfo->get_next_data_point = table_get_next_data_point;

		iinfo->myvoid = (void *) table;
		iinfo->table_reginfo = table_info;

		netsnmp_register_table_iterator(reg, iinfo);

	}

	return 0;
}

int init_snmp_agent(void)
{
	int result;

	if (user_settings.is_master_agent) {
		netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
	} else {
		netsnmp_enable_subagent();
	}

	if (user_settings.agentx_address != NULL) {
		netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET, user_settings.agentx_address);
	}

	if (user_settings.daemonize)
		snmp_enable_syslog();
	else
		snmp_enable_stderrlog();	

	SOCK_STARTUP;

	if ((result = init_agent("collectd-agent")) != 0) {
		return -1;
	}

	if ((result = register_value_handlers()) != 0) {
		return -1;
	}

	if ((result = register_table_handlers()) != 0) {
		return -1;
	}

	if (user_settings.is_master_agent) {
		init_vacm_vars();
		init_usmUser();
	}

	init_snmp("collectd-snmpd");

	if (user_settings.is_master_agent) {
		init_master_agent();
	}

	return 0;
}

void shutdown_snmp_agent(void)
{
	snmp_shutdown("collectd-snmpd");
	SOCK_CLEANUP;
}

void stop_snmp_agent(int dummy)
{
	agent_running = 0;
}

void run_snmp_agent(void)
{
	agent_running = 1;

	while (agent_running) {
		agent_check_and_process(1);
	}
}
