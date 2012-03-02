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

#ifndef _DATA_H_
#define _DATA_H_

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/mib_api.h>

#include <sys/types.h>
#include <regex.h>

typedef unsigned char snmp_type_t;

typedef struct snmp_value {

	char       *stat_name;
	char       *oid_str;
	oid         val_oid[MAX_OID_LEN];
	size_t      val_oid_len;
	snmp_type_t type;

} snmp_value_t;

typedef struct snmp_value_list {
	unsigned int   num_values;
	snmp_value_t  *values;
} snmp_value_list_t;

typedef struct snmp_column {
	int         col_num;
	snmp_type_t type;
	char        *name;
} snmp_column_t;

typedef struct snmp_table {

	char          *table_oid_str;
	oid            table_oid[MAX_OID_LEN];
	size_t         table_oid_len;

	unsigned int   num_columns;
	snmp_column_t *columns;

	void          *agent_info;

	/* Pre-compiled regular expression used to get the
	 * indexes that exist for this table */
	regex_t        index_regex;
	char           is_regex_initialized;

} snmp_table_t;

typedef struct snmp_table_list {
	unsigned int   num_tables;
	snmp_table_t  *tables;
} snmp_table_list_t;

extern snmp_value_list_t value_list;
extern snmp_table_list_t table_list;

#endif /* _DATA_H_ */
