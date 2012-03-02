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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

typedef struct settings {
	char *collectd_address;
	char is_master_agent;
	char *agentx_address;
	char daemonize;
} settings_t;


extern settings_t user_settings;

#endif /* _SETTINGS_H_ */
