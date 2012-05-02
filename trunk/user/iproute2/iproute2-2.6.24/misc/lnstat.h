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
#ifndef _LNSTAT_H
#define _LNSTAT_H

#include <limits.h>

#define LNSTAT_VERSION "0.02 041002"

#define PROC_NET_STAT	"/proc/net/stat"

#define LNSTAT_MAX_FILES			32
#define LNSTAT_MAX_FIELDS_PER_LINE		32
#define LNSTAT_MAX_FIELD_NAME_LEN		32

struct lnstat_file;

struct lnstat_field {
	struct lnstat_file *file;
	unsigned int num;			/* field number in line */
	char name[LNSTAT_MAX_FIELD_NAME_LEN+1];
	unsigned long values[2];		/* two buffers for values */
	unsigned long result;
};

struct lnstat_file {
	struct lnstat_file *next;
	char path[PATH_MAX+1];
	char basename[NAME_MAX+1];
	struct timeval last_read;		/* last time of read */
	struct timeval interval;		/* interval */
	int compat;				/* 1 == backwards compat mode */
	FILE *fp;
	unsigned int num_fields;		/* number of fields */
	struct lnstat_field fields[LNSTAT_MAX_FIELDS_PER_LINE];
};


struct lnstat_file *lnstat_scan_dir(const char *path, const int num_req_files,
				    const char **req_files);
int lnstat_update(struct lnstat_file *lnstat_files);
int lnstat_dump(FILE *outfd, struct lnstat_file *lnstat_files);
struct lnstat_field *lnstat_find_field(struct lnstat_file *lnstat_files,
				       const char *name);
#endif /* _LNSTAT_H */
