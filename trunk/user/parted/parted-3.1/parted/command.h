/*
    parted - a frontend to libparted
    Copyright (C) 1999-2000, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#include <parted/parted.h>
#include "strlist.h"

typedef struct {
	StrList*	names;
	int		(*method) (PedDevice** dev);
	StrList*	summary;
	StrList*	help;
        int             non_interactive:1;
} Command;

extern Command* command_create (const StrList* names,
				int (*method) (PedDevice** dev),
				const StrList* summary,
				const StrList* help,
                                int non_interactive);
extern void command_destroy (Command* cmd);
void command_register (Command** list, Command* cmd);

extern Command* command_get (Command** list, char* name);
extern StrList* command_get_names (Command** list);
extern void command_print_summary (Command* cmd);
extern void command_print_help (Command* cmd);
extern int command_run (Command* cmd, PedDevice** dev);

#endif /* COMMAND_H_INCLUDED */
