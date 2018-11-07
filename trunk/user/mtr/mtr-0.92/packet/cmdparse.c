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

#include "cmdparse.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
    NUL terminate the whitespace separated tokens in the command string.
    This modifies command_string in-place with NUL characters.
    Fill the tokens array with pointers to the tokens, and return the
    number of tokens found.
*/
static
int tokenize_command(
    char **tokens,
    int max_tokens,
    char *command_string)
{
    int token_count = 0;
    int on_space = 1;
    int i;

    for (i = 0; command_string[i]; i++) {
        if (on_space) {
            if (!isspace((unsigned char) command_string[i])) {
                /*  Take care not to exceed the token array length  */
                if (token_count >= max_tokens) {
                    return -1;
                }

                tokens[token_count++] = &command_string[i];
                on_space = 0;
            }
        } else {
            if (isspace((unsigned char) command_string[i])) {
                command_string[i] = 0;
                on_space = 1;
            }
        }
    }

    return token_count;
}

/*
    Parse a command string (or command reply string) into a command_t
    structure for later semantic interpretation.  Returns EINVAL if the
    command string is unparseable or zero for success.

    comamnd_string will be modified in-place with NUL characters terminating
    tokens, and the command_t will use pointers to the conents of
    command_string without copying, so any interpretation of the
    command_t structure requires that the command_string memory has not yet
    been freed or otherwise reused.
*/
int parse_command(
    struct command_t *command,
    char *command_string)
{
    char *tokens[MAX_COMMAND_TOKENS];
    int token_count;
    int i;

    memset(command, 0, sizeof(struct command_t));

    /*  Tokenize the string using whitespace  */
    token_count =
        tokenize_command(tokens, MAX_COMMAND_TOKENS, command_string);
    if (token_count < 2) {
        errno = EINVAL;
        return -1;
    }

    /*  Expect the command token to be a numerical value  */
    errno = 0;
    command->token = strtol(tokens[0], NULL, 10);
    if (errno) {
        errno = EINVAL;
        return -1;
    }
    command->command_name = tokens[1];

    /*
       The tokens beyond the command name are expected to be in
       name, value pairs.
     */
    i = 2;
    command->argument_count = 0;
    while (i < token_count) {
        /*  It's an error if we get a name without a key  */
        if (i + 1 >= token_count) {
            errno = EINVAL;
            return -1;
        }

        /*  It's an error if we get more arguments than we have space for  */
        if (command->argument_count >= MAX_COMMAND_ARGUMENTS) {
            errno = EINVAL;
            return -1;
        }

        command->argument_name[command->argument_count] = tokens[i];
        command->argument_value[command->argument_count] = tokens[i + 1];
        command->argument_count++;

        i += 2;
    }

    return 0;
}
