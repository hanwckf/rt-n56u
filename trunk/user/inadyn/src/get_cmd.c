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
  Inplementation functions for CMD options parsing system
  Author: Narcis Ilisei
  Date: May 2003
  History:
  - may 15 2003 : first version.
  Basic, table driven option line parsing.
*/

#define MODULE_TAG ""
#include <string.h>
#include <stdlib.h>
#include "get_cmd.h"
#include "debug_if.h"


static CMD_DESCRIPTION_TYPE * opt_search(CMD_DESCRIPTION_TYPE *p_table, char *p_opt)
{
	CMD_DESCRIPTION_TYPE *it = p_table;

	while( it->p_option != NULL)
	{
		if (strcmp(p_opt, it->p_option) == 0)
		{
			return it;
		}
		++it;
	}

	return NULL;
}

/**
   Init the CMD_DATA
*/
RC_TYPE cmd_init(CMD_DATA *p_cmd)
{
	if (!p_cmd)
	{
		return RC_INVALID_POINTER;
	}

	memset(p_cmd, 0, sizeof(*p_cmd));

	return RC_OK;
}

RC_TYPE cmd_destruct(CMD_DATA *p_cmd)
{
	if (!p_cmd)
	{
		return RC_INVALID_POINTER;
	}

	if (p_cmd->argv)
	{
		int i;

		for (i = 0; i < p_cmd->argc; ++i)
		{
			if (p_cmd->argv[i])
			{
				free(p_cmd->argv[i]);
			}
		}
		free(p_cmd->argv);
	}

	return RC_OK;
}

/** Adds a new option (string) to the command line
 */
RC_TYPE cmd_add_val(CMD_DATA *p_cmd, char *p_val)
{
	RC_TYPE rc = RC_OK;

	if (!p_cmd || !p_val)
	{
		return RC_INVALID_POINTER;
	}

	do
	{
		char *p;
		char **pp = (char **)realloc(p_cmd->argv, (p_cmd->argc + 1) * sizeof(char *));
		if (!pp)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}
		p_cmd->argv = pp;

		p = (char *)malloc(strlen(p_val) + 1);
		if (!p)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		strcpy(p, p_val);
		p_cmd->argv[p_cmd->argc] = p;
		p_cmd->argc ++;
	}
	while (0);

	return rc;
}
/** Creates a struct of argvals from the given command line.
    Action:
    copy the argv from the command line to the given CMD_DATA struct
    set the data val of the list element to the current argv
*/
RC_TYPE cmd_add_vals_from_argv(CMD_DATA *p_cmd, char **argv, int argc)
{
	int i;
	RC_TYPE rc = RC_OK;

	if (!p_cmd || !argv || !argc)
	{
		return RC_INVALID_POINTER;
	}

	for (i = 0; i < argc; ++i)
	{
		rc = cmd_add_val(p_cmd, argv[i]);
		if (rc != RC_OK)
			break;
	}

	return rc;
}


/*
  Parses the incoming argv list.
  Arguments:
  argv, argc,
  cmd description

  Action:
  performs a match for every p_option string in the CMD description.
  checks the number of arguments left
  calls the user handler with the pointer to the correct arguments

  Implementation:
  - for each option in the table
  - find it in the argv list
  - check the required number of arguments
  - call the handler
*/
RC_TYPE get_cmd_parse_data(char **argv, int argc, CMD_DESCRIPTION_TYPE *p_cmd_descr)
{
	RC_TYPE rc = RC_OK;
	CMD_DATA cmd;
	int curr_arg_nr = 1; /* without the prg name*/

	if (argv == NULL || p_cmd_descr == NULL)
	{
		return RC_INVALID_POINTER;
	}

	do
	{
		rc = cmd_init(&cmd);
		if (rc != RC_OK)
		{
			break;
		}

		rc = cmd_add_vals_from_argv(&cmd, argv, argc);
		if (rc != RC_OK)
		{
			break;
		}

	 	while (curr_arg_nr < cmd.argc)
		{
			CMD_DESCRIPTION_TYPE *p_curr_opt = opt_search(p_cmd_descr, cmd.argv[curr_arg_nr]);

			if (p_curr_opt == NULL)
			{
				rc = RC_CMD_PARSER_INVALID_OPTION;
				logit(LOG_WARNING, MODULE_TAG "Invalid option name at position %d: %s",
				      curr_arg_nr + 1, cmd.argv[curr_arg_nr]);
				break;
			}

//			logit(LOG_NOTICE, MODULE_TAG "Found opt %d: %s", curr_arg_nr, cmd.argv[curr_arg_nr]);

			++curr_arg_nr;

			/*check arg nr required by the current option*/
			if (curr_arg_nr + p_curr_opt->arg_nr > cmd.argc)
			{
				rc = RC_CMD_PARSER_INVALID_OPTION_ARGUMENT;
				logit(LOG_WARNING, MODULE_TAG "Missing option value at position %d: %s",
				      curr_arg_nr + 1, p_curr_opt->p_option);
				break;
			}

			rc = p_curr_opt->p_handler.p_func(&cmd, curr_arg_nr, p_curr_opt->p_handler.p_context);
			if (rc != RC_OK)
			{
				logit(LOG_WARNING, MODULE_TAG "Error parsing option %d: %s",
				      curr_arg_nr, cmd.argv[curr_arg_nr-1]);
				break;
			}

			curr_arg_nr += p_curr_opt->arg_nr;
		}
	}
	while (0);

	cmd_destruct(&cmd);

	return rc;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 8
 * End:
 */
