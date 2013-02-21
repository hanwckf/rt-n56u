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

#include <config.h>
#include "command.h"
#include "ui.h"

#include <parted/debug.h>

#include <stdlib.h>
#include <string.h>
#include "xalloc.h"

Command*
command_create (const StrList* names,
		int (*method) (PedDevice** dev),
		const StrList* summary,
		const StrList* help,
                const int non_interactive)
{
	Command*	cmd;

	cmd = xmalloc (sizeof (Command));

        if (non_interactive)
                cmd->non_interactive = 1;
        else
                cmd->non_interactive = 0;

	cmd->names = (StrList*) names;
	cmd->method = method;
	cmd->summary = (StrList*) summary;
	cmd->help = (StrList*) help;

	return cmd;
}

void
command_destroy (Command* cmd)
{
	str_list_destroy (cmd->names);
	str_list_destroy (cmd->summary);
	str_list_destroy (cmd->help);
	free (cmd);
}

void
command_register (Command** list, Command* cmd)
{
	int	i;

	for (i = 0; list [i]; i++);

	list [i] = cmd;
	list [i + 1] = (Command*) NULL;
}

Command*
command_get (Command** list, char* name)
{
	int		i;
	int		partial_match = -1;
	int		ambiguous = 0;

	if (!name)
		return NULL;

	for (i=0; list [i]; i++) {
		switch (str_list_match_any (list [i]->names, name)) {
		case 2:
			return list [i];

		case 1:
			if (!ambiguous) {
				if (partial_match == -1) {
					partial_match = i;
				} else {
					partial_match = -1;
					ambiguous = 1;
				}
			}
		}
	}

	if (partial_match == -1)
		return NULL;
	else
		return list [partial_match];
}

StrList*
command_get_names (Command** list)
{
	Command**	walk;
	StrList*	result = NULL;

	for (walk = list; *walk; walk++)
		result = str_list_join (result,
					str_list_duplicate ((*walk)->names));
	return result;
}

void
command_print_summary (Command* cmd)
{
        fputs ("  ", stdout);
	str_list_print_wrap (cmd->summary, screen_width(), 2, 8, stdout);
	putchar ('\n');
}

void
command_print_help (Command* cmd)
{
	command_print_summary (cmd);
	if (cmd->help) {
                fputs ("\n\t", stdout);
		str_list_print_wrap (cmd->help, screen_width(), 8, 8, stdout);
	}
}

int
command_run (Command* cmd, PedDevice** dev)
{
	return cmd->method (dev);
}
