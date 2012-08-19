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

#ifndef _nvram_f_h_
#define _nvram_f_h_

#define MAX_LINE_SIZE 512

char * nvram_get_x(const char *sid, const char *name);
char * nvram_get_f(const char *file, const char *field);

#define nvram_safe_get_x(sid, name) (nvram_get_x(sid, name) ? : "")
#define nvram_safe_get_f(file, field) (nvram_get_f(file, field) ? : "")

#define nvram_match_x(sid, name, match) ({ const char *value = nvram_get_x(sid, name); (value && !strcmp(value, match)); })
#define nvram_match_list_x(sid, name, match, index) ({ const char *value = nvram_get_list_x(sid, name, index); (value && !strcmp(value, match)); })
#define nvram_invmatch_x(sid, name, invmatch) ({ const char *value = nvram_get_x(sid, name); (value && strcmp(value, invmatch)); })

int nvram_set_x(const char *sid, const char *name, const char *value);
int nvram_set_f(const char *file, const char *field, const char *value);

char *nvram_get_list_x(const char *sid, const char *name, int index);
int nvram_add_lists_x(const char *sid, const char *name, const char *value, int count);
int nvram_del_lists_x(const char *sid, const char *name, int *delMap);


#endif /* _nvram_f_h_ */
