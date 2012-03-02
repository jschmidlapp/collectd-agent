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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"

/* Caller owns new string on return */
char *replace_wildcard(const char *match_str, const char *new_substr)
{
	char *new_str;
	size_t star_idx;
	size_t match_str_len;
	size_t new_substr_len;

	assert(match_str != NULL);
	assert(new_substr != NULL);

	match_str_len = strlen(match_str);
	new_substr_len = strlen(new_substr);
	if ((new_str = malloc(match_str_len + new_substr_len + 1)) == NULL) {
		return NULL;
	}

	star_idx = strcspn(match_str, "*");

	if ((star_idx == 0) || (star_idx >= match_str_len))
		return NULL;

	memcpy(new_str, match_str, star_idx);

	memcpy(new_str + star_idx, new_substr, new_substr_len);

	memcpy(new_str + star_idx + new_substr_len,
	       match_str + star_idx + 1, match_str_len - star_idx);

	new_str[match_str_len + new_substr_len] = '\0';

	return new_str;
}
