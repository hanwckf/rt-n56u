/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <nvram_linux.h>

#include "common.h"
#include "nvram_x.h"


char *nvram_get_list_x(const char *name, int index)
{
	char nv_name[64];

	snprintf(nv_name, sizeof(nv_name), "%s%d", name, index);
	return nvram_safe_get(nv_name);
}

int nvram_match_list_x(const char *name, char *match, int index)
{
	const char *value = nvram_get_list_x(name, index);
	return (!strcmp(value, match));
}

void nvram_del_list_map_x(const char *name, int group_count, int *delMap)
{
	char oname[64], nname[64], tmp[256], *oval;
	int i, i_len, ni, di, group_count_new;

	if (!(*name) || group_count < 1)
		return;

	ni = 0;
	di = 0;
	tmp[0] = 0;

	for (i=0; i < group_count; i++) {
		snprintf(oname, sizeof(oname), "%s%d", name, i);
		snprintf(nname, sizeof(nname), "%s%d", name, ni);
		
		if (delMap[di] == i) {
			/* store first deleted value */
			if (di == 0) {
				oval = nvram_safe_get(oname);
				i_len = MIN(strlen(oval), sizeof(tmp)-1);
				strncpy(tmp, oval, i_len);
				tmp[i_len] = 0;
			}
			
			di++;
			if (di > MAX_GROUP_COUNT)
				break;
		} else {
			oval = nvram_safe_get(oname);
			nvram_set(nname, oval);
			ni++;
		}
	}

	/* store single deleted value */
	snprintf(oname, sizeof(oname), "%s_0", name);
	if (di == 1)
		nvram_set_temp(oname, tmp);
	else
		nvram_unset(oname);

	group_count_new = group_count - di;
	if (group_count_new < 0)
		group_count_new = 0;

	/* clear old array data (try +8 items for clear old trash) */
	for (i = (group_count+7); i >= group_count_new; i--) {
		snprintf(oname, sizeof(oname), "%s%d", name, i);
		nvram_unset(oname);
	}
}

