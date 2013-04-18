/*
Copyright (C) 2003-2004 Narcis Ilisei

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* 
	Interface functions for CMD options parsing system
	Author: Narcis Ilisei
	Date: May 2003
	History:
		- may 15 2003 : first version.
			Basic, table driven option line parsing.
*/

#ifndef _GET_CMD_IF_INCLUDED
#define _GET_CMD_IF_INCLUDED


#include "errorcode.h"
typedef struct
{
    int argc;
    char **argv;
} CMD_DATA;

typedef RC_TYPE (*CMD_OPTION_HANDLER_FUNC)(CMD_DATA *p_cmd, int current_nr, void *p_context);
typedef struct 
{
	CMD_OPTION_HANDLER_FUNC p_func;
	void *p_context;
} CMD_OPTION_HANDLER_TYPE;


typedef struct
{
	char *p_option;
	int arg_nr;
	CMD_OPTION_HANDLER_TYPE p_handler;	
	char *p_description;
} CMD_DESCRIPTION_TYPE;




/*
	Parses the incoming argv list data.
	Arguments:
		argv, argc,
		cmd description 

	Action:
		performs a match for every p_option string in the CMD description.
		checks the number of arguments left
		calls the user handler with the pointer to the correct arguments
*/
RC_TYPE get_cmd_parse_data(char **argv, int argc, CMD_DESCRIPTION_TYPE *p_cmd_descr);

/** Adds a new option (string) to the command line 
*/
RC_TYPE cmd_add_val(CMD_DATA *p_cmd, char *p_val);

#endif /*_GET_CMD_IF_INCLUDED*/
