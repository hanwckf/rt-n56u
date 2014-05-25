/* Interface functions for CMD options parsing system
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2014  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
*/

#ifndef INADYN_CMD_H_
#define INADYN_CMD_H_

#include "error.h"
typedef struct {
	int argc;
	char **argv;
} cmd_data_t;

typedef int (*cmd_fn_t) (cmd_data_t *cmd, int no, void *context);

typedef struct {
	cmd_fn_t func;
	void *context;
} cmd_handler_t;

typedef struct {
	char *option;
	int argno;
	cmd_handler_t handler;
	char *description;
} cmd_desc_t;

int get_cmd_parse_data(char **argv, int argc, cmd_desc_t *desc);
int cmd_add_val(cmd_data_t *cmd, char *val);

#endif /* INADYN_CMD_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
