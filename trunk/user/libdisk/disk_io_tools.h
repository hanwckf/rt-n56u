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
#ifndef _DISK_IO_TOOLS_
#define _DISK_IO_TOOLS_

#define csprintf(fmt, args...) fprintf(stderr, fmt, ## args)

extern void sanity_name(char *name);
extern char *read_whole_file(const char *);
extern int mkdir_if_none(char *);
extern int delete_file_or_dir(char *);
extern int test_if_file(const char *);
extern int test_if_dir(const char *);
extern int test_if_System_folder(const char *);
extern int test_mounted_disk_size_status(char *);

extern char *get_upper_str(const char *const);
extern int upper_strcmp(const char *const, const char *const);
extern int upper_strncmp(const char *const, const char *const, int);
extern char *upper_strstr(const char *const, const char *const);
extern void strntrim(char *);
extern void write_escaped_value(FILE *, const char *);

#endif // _DISK_IO_TOOLS_
