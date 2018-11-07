/*
    mtr  --  a network diagnostic tool
    Copyright (C) 2016  Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef CMDPARSE_H
#define CMDPARSE_H

enum {
    MAX_COMMAND_ARGUMENTS = 16,
    MAX_COMMAND_TOKENS = MAX_COMMAND_ARGUMENTS * 2 + 2
};

/*  Parsed commands, or command replies, ready for semantic interpretation  */
struct command_t {
    /*  A unique value for matching command requests with replies  */
    int token;

    /*  Text indiciating the command type, or reply type  */
    char *command_name;

    /*  The number of key, value argument pairs used  */
    int argument_count;

    /*  Names for each argument  */
    char *argument_name[MAX_COMMAND_ARGUMENTS];

    /*  Values for each argument, parallel to the argument_name array  */
    char *argument_value[MAX_COMMAND_ARGUMENTS];
};

int parse_command(
    struct command_t *command,
    char *command_string);

#endif
