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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/mib_api.h>

#include "data.h"
#include "common.h"

// TODO: I don't really like this whole mess with types - maybe just go to ASN?
int set_snmp_value(snmp_type_t type, double value, netsnmp_variable_list * vb)
{
	int32_t int32;
	uint32_t uint32;
	uint64_t uint64;
	int result;
	unsigned char *value_ptr;
	size_t value_size;

	switch (type) {
	case ASN_INTEGER:
		int32 = (int32_t) value;
		value_ptr = (unsigned char *) &int32;
		value_size = sizeof(int32);
		break;

	case ASN_COUNTER:
		uint32 = (uint32_t) value;
		value_ptr = (unsigned char *) &uint32;
		value_size = sizeof(uint32);
		break;

	case ASN_GAUGE:
		uint32 = (uint32_t) value;
		value_ptr = (unsigned char *) &uint32;
		value_size = sizeof(uint32);
		break;

	case ASN_COUNTER64:
		uint64 = (uint64_t) value;
		value_ptr = (unsigned char *) &uint64;
		value_size = sizeof(uint64);
		break;

	default:
		ERROR("Bad value type\n");
		return -1;
		break;
	}

	result = snmp_set_var_typed_value(vb, type, value_ptr, value_size);

	if (result != 0) {
		return -1;
	}

	return 0;
}

int oids_equal(oid * a, size_t a_len, oid * b, size_t b_len)
{
	int i;

	assert(a != NULL);
	assert(b != NULL);

	if (a_len != b_len)
		return 0;

	for (i = 0; i < a_len; i++) {
		if (a[i] != b[i])
			return 0;
	}

	return 1;
}

snmp_value_t *find_value_config_entry(oid * val_oid, size_t val_oid_len)
{
	int i;

	assert(val_oid != NULL);

	for (i = 0; i < value_list.num_values; i++) {
		snmp_value_t *value = &(value_list.values[i]);

		if (oids_equal(value->val_oid, value->val_oid_len, val_oid, val_oid_len)) {
			return value;
		}
	}

	return NULL;

}

int convert_string_to_oid(const char *oid_str, oid * targ_oid, size_t * targ_oid_len)
{
	assert(oid_str != NULL);
	assert(targ_oid != NULL);
	assert(targ_oid_len != NULL);

	if (read_objid(oid_str, targ_oid, targ_oid_len) != 1) {
		ERROR("Invalid OID %s\n", oid_str);
		return -1;
	}

	return 0;
}
