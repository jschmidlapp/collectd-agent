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

#ifndef _AGENT_HELPER_H_
#define _AGENT_HELPER_H_

int set_snmp_value(snmp_type_t type, 
				   double      value, 
				   netsnmp_variable_list * vb);

int oids_equal(oid    *a, 
			   size_t  a_len, 
			   oid    *b, 
			   size_t  b_len);

snmp_value_t *find_value_config_entry(oid   *val_oid, 
									  size_t val_oid_len);

int convert_string_to_oid(const char *oid_str, 
						  oid        *targ_oid, 
						  size_t     *targ_oid_len);

#endif	/* _AGENT_HELPER_H_ */
