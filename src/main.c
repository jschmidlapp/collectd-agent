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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "data.h"
#include "agent.h"
#include "client.h"
#include "common.h"
#include "settings.h"

#define DEFAULT_CONFIG_FILENAME "/etc/collectd-agent.conf"

/* From bison parser */
int parse_config_file(const char *filename);

/* Global data of all values and tables to expose via SNMP */
snmp_table_list_t table_list;
snmp_value_list_t value_list;

settings_t user_settings;

int logLevel;

void print_usage_and_exit()
{
	printf("Usage: " PACKAGE_NAME " [OPTION]\n\n"
	       "       Available options:\n"
	       "         -C <file>       Configuration file\n"
	       "                         Default:" DEFAULT_CONFIG_FILENAME "\n"
	       "         -d              Turn on debugging\n"
	       "         -h              Display help\n"
	       "         -D<token>       Enables net-snmp debug token\n"
		   "         -f              Run in foreground\n\n"
	       " " PACKAGE_NAME " " VERSION "\n\n");
	exit(-1);
}


static char *parse_cmdline(int argc, char **argv)
{
	assert(argv != NULL);

	while (1) {

		int c;

		c = getopt(argc, argv, "D:C:hdf");

		if (c == -1) {
			break;
		}

		switch (c) {

		case 'C':
			if (optarg != NULL)
				return optarg;
			else
				print_usage_and_exit();
			break;

		case 'd':
			logLevel = LOG_LEVEL_DEBUG;
			break;

		case 'D':
			printf("DEBUG!\n");
			if (optarg != NULL) {
				printf("Enabling net-snmp debug token %s\n", optarg);
				debug_register_tokens(optarg);
				snmp_set_do_debugging(1);
			}

			break;

		case 'f':
			user_settings.daemonize = 0;
			break;

		case 'h':
		default:
			print_usage_and_exit();
			break;
		}

	}

	return NULL;
}

static void free_data_structures(void)
{
	int i;

	for (i = 0; i < value_list.num_values; i++) {
		snmp_value_t *value;

		value = &value_list.values[i];

		checked_free(value->stat_name);
		checked_free(value->oid_str);
	}

	checked_free(value_list.values);

	checked_free(user_settings.collectd_address);
	checked_free(user_settings.agentx_address);

	for (i = 0; i < table_list.num_tables; i++) {

		snmp_table_t *table = &table_list.tables[i];

		free_table_indexes(table->agent_info);
		regfree(&table->index_regex);
		checked_free(table->table_oid_str);
	}
}

int main(int argc, char **argv)
{
	char *cfg_filename = DEFAULT_CONFIG_FILENAME;
	int ret_value = -1;
	pid_t pid;

	logLevel = LOG_LEVEL_ERROR;
	user_settings.daemonize = 1;


	if (argc > 1) {
		cfg_filename = parse_cmdline(argc, argv);
	}

	if ( parse_config_file(cfg_filename) ) {
		/* Parser will output actual error */
		ERROR("Parse error\n");
		goto error;
	}

	if ( connect_to_collectd(user_settings.collectd_address) ) {
		ERROR("Couldn't connect to collectd daemon at address %s\n",
		      user_settings.collectd_address);
		goto error;
	}

	if ( check_stat_names() ) {
		ERROR("Statistic name check failed\n");
		goto error;
	}

	if ( init_snmp_agent() ) {
		ERROR("Failed to initialize SNMP agent\n");
		goto error;
	}

	if (user_settings.daemonize) {

		pid = fork();
		if (pid < 0) {
			perror("Failed to fork collectd-agent daemon");
			return(1);
		} else if (pid != 0) {
			return(0);
		}
		
		setsid();

		close(0); close(1); close(2);

		open("/dev/null", O_RDWR); dup(0); dup(0);
	}

	signal(SIGTERM, stop_snmp_agent);
	signal(SIGINT, stop_snmp_agent);

	run_snmp_agent();	/* Will only return when signal is received */

	shutdown_snmp_agent();

	ret_value = 0;

error:
	free_data_structures();
	return ret_value;
}
