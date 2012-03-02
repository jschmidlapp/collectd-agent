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

#include <assert.h>
#include <collectd/client.h>

#include "common.h"
#include "data.h"

static lcc_connection_t *collectd_handle;

/* Maximum length of a statistic name (e.g. hostname/plugin-1/type-1). */
#define MAX_STAT_STR_LEN 64	

#define INITIAL_NUM_INDEXES 10


int connect_to_collectd(const char *address)
{
	assert(address != NULL);

	return lcc_connect(address, &collectd_handle);
}

int get_single_value_from_collectd(const char *name, double *val)
{
	size_t ret_values_num;
	gauge_t *ret_values;
	char **ret_values_names;
	lcc_identifier_t ident;
	int result;
	int i;

	assert(name != NULL);
	assert(val != NULL);

	result = lcc_string_to_identifier(collectd_handle, &ident, name);

	if (result != 0) {
		ERROR("Couldn't convert stat name %s to identifier\n", name);
		return -1;
	}

	result = lcc_getval(collectd_handle,
			    &ident, &ret_values_num, &ret_values, &ret_values_names);

	if (result != 0) {
		ERROR("Couldn't get statistic %s from collectd\n", name);
		return -1;
	}

	if (ret_values_num == 0) {
		ERROR("No values for statistic %s returned from collectd\n", name);
		return -1;
	}

	*val = ret_values[0];

	checked_free(ret_values);

	if (ret_values_names != NULL) {

		for (i = 0; i < ret_values_num; i++) {
			checked_free(ret_values_names[i]);
		}

		checked_free(ret_values_names);
	}

	return 0;
}

/**
 * Make sure that all of the statistic names specified by the user are OK by
 * trying to convert them to identifiers.
 */
int check_stat_names(void)
{
	int i;

	for (i = 0; i < value_list.num_values; i++) {

		lcc_identifier_t ident;
		snmp_value_t *value = &(value_list.values[i]);

		if (lcc_string_to_identifier(collectd_handle, &ident, value->stat_name) != 0) {
			ERROR("Invalid statistic %s (%s)\n", value->stat_name,
			      lcc_strerror(collectd_handle));
			return -1;
		}
	}

	return 0;
}

#define REGEX_ALL_CHARS "([0-9]*)"


static int compile_index_regex(snmp_table_t *table)
{
	char *regex_str;
	int   result;
	
	assert(table != NULL);

	if (!table->is_regex_initialized) {

		regex_str = replace_wildcard(table->columns[0].name, REGEX_ALL_CHARS);
		
		if (regex_str == NULL) {
			ERROR("Couldn't convert match string %s to regex\n", table->columns[0].name);
			return -1;
		}
		
		result = regcomp(&(table->index_regex), regex_str, REG_EXTENDED);
		
		if (result != 0) {
			ERROR("Failed to compile regex\n");
			return -1;
		}
		
		table->is_regex_initialized = 1;
		
		checked_free(regex_str);
	}

	return 0;
}

/* This function takes ownership of match_str */
int get_list_of_indexes(snmp_table_t *table, uint32_t ** ret_indexes, size_t * ret_num_indexes)
{

	int result;
	lcc_identifier_t *stats;
	size_t num_stats;
	int i;
	regmatch_t regex_match[2];
	uint32_t *indexes;
	uint32_t num_indexes = 0;

	assert(table != NULL);
	assert(ret_indexes != NULL);
	assert(ret_num_indexes != NULL);

	*ret_indexes = NULL;
	*ret_num_indexes = 0;

	indexes = (uint32_t *) malloc(INITIAL_NUM_INDEXES * sizeof(uint32_t));

	if (compile_index_regex(table) != 0) {
		return -1;
	}

	result = lcc_listval(collectd_handle, &stats, &num_stats);

	if (result != 0) {
		ERROR("Couldn't get list of stats from collectd (%s)\n",
		      lcc_strerror(collectd_handle));
		return -1;
	}

	for (i = 0; i < num_stats; i++) {

		char stat_str[MAX_STAT_STR_LEN];

		if (stat_str == NULL) {
			ERROR("Couldn't allocate memory for statistic\n");
			return -1;
		}

		result =
		    lcc_identifier_to_string(collectd_handle, stat_str, MAX_STAT_STR_LEN,
					     &stats[i]);

		if (result != 0) {
			ERROR("Couldn't convert stat to string (%s)\n",
			      lcc_strerror(collectd_handle));
			return -1;
		}

		/* Maximum of two matches - entire string and wildcard substring */
		result = regexec(&table->index_regex, stat_str, 2, regex_match, 0);

		if (result == 0) {

			char *index_start_ptr;
			char *index_end_ptr;
			char *test_end_ptr;
			int index_val;

			/* This shouldn't happen, because regexec says that we found
			 * a match just in case */
			if ((regex_match[1].rm_so == -1) || (regex_match[1].rm_eo == -1)) {
				continue;
			}

			index_start_ptr = stat_str + regex_match[1].rm_so;
			index_end_ptr = stat_str + regex_match[1].rm_eo;

			index_val = strtol(index_start_ptr, &test_end_ptr, 10);

			if (test_end_ptr == index_start_ptr) {

				/* This should never happen, because the regex shouldn't have matched if
				 * it wasn't a valid number. */
				ERROR("Couldn't convert value in stat string\n");
				return -1;
			}

			num_indexes++;

			indexes = realloc(indexes, num_indexes * sizeof(uint32_t));

			indexes[num_indexes - 1] = index_val;

		}

	}

	free(stats);

	*ret_indexes = indexes;
	*ret_num_indexes = num_indexes;

	return 0;
}
