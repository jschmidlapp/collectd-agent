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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

char *replace_wildcard(const char *match_str, const char *new_substr);

#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

extern int logLevel;

#define LOG(level, ...)  \
  if (level > logLevel) ; \
  else printf(__VA_ARGS__);

#define DEBUG(...)  LOG(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define ERROR(...)  LOG(LOG_LEVEL_ERROR, __VA_ARGS__)
#define INFO(...)   LOG(LOG_LEVEL_INFO, __VA_ARGS__)

#define DEBUG_OID(name, length)					\
	do { \
	char debug_str[128]; \
    snprint_objid(debug_str, sizeof(debug_str), (name),	(length)); \
	DEBUG("  OID = %s\n", debug_str); \
	} while(0); 

#define ERROR_AND_RETURN(...) do { LOG(LOG_LEVEL_DEBUG, __VA_ARGS__); return -1; } while(0)

#define checked_free(ptr) \
  do { \
  if ((ptr) != NULL) \
    free((ptr)); \
  } while(0)

#endif /* _COMMON_H_ */
