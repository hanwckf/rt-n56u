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
#include <nvram/bcmnvram.h>

#include "nvram_f.h"

int
nvram_set_x(const char *sid, const char *name, const char *value)
{
	return (nvram_set(name, value));
}

char*
nvram_get_x(const char *sid, const char *name)
{
	return (nvram_safe_get(name));
}

char*
nvram_get_f(const char *file, const char *field)
{
	return (nvram_safe_get(field));
}

int 
nvram_set_f(const char *file, const char *field, const char *value)
{
	return (nvram_set(field, value));
}

char *nvram_get_list_x(const char *sid, const char *name, int index)
{
	char new_name[MAX_LINE_SIZE];
	sprintf(new_name, "%s%d", name, index);
	return (nvram_get_f(sid, new_name));
}

int nvram_add_lists_x(const char *sid, const char *name, const char *value, int count)
{
    char name1[64], name2[64];

    strcpy(name1, name);

    if (name[0]!='\0')
    {
	sprintf(name2, "%s%d", name1, count);
	nvram_set(name2, value);
    }
    return 0;
}

int nvram_del_lists_x(const char *sid, const char *name, int *delMap)
{
    char names[64], oname[64], nname[64], *oval, *nval;
    int oi, ni, di;

    strcpy(names, name);

    if (names[0]!='\0')
    {
	oi=0;
	ni=0;
	di=0;
	while (1)
	{
		sprintf(oname, "%s%d", names, oi);
		sprintf(nname, "%s%d", names, ni);

		oval = nvram_get(oname);
		nval = nvram_get(nname);

		if (oval==NULL) break;

		printf("d: %d %d %d %d\n", oi, ni, di, delMap[di]);
		if (delMap[di]!=-1&&delMap[di]==oi)
		{
			oi++;
			di++;
		}
		else
		{
			nvram_set(nname, oval);
			ni++;
			oi++;
		}
	}
    }
    return (0);
}

