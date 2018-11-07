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

#ifndef COMMAND_H
#define COMMAND_H

#include "platform.h"
#include "probe.h"

#define COMMAND_BUFFER_SIZE 4096

#ifdef PLATFORM_CYGWIN
#include "command_cygwin.h"
#else
#include "command_unix.h"
#endif

/*  Storage for incoming commands, prior to command parsing  */
struct command_buffer_t {
    /*  The file descriptor of the incoming command stream  */
    int command_stream;

    /*  Storage to read commands into  */
    char incoming_buffer[COMMAND_BUFFER_SIZE];

    /*  The number of bytes read so far in incoming_buffer  */
    int incoming_read_position;

    /*  Platform specific  */
    struct command_buffer_platform_t platform;
};

void init_command_buffer(
    struct command_buffer_t *command_buffer,
    int command_stream);

int read_commands(
    struct command_buffer_t *buffer);

void dispatch_buffer_commands(
    struct command_buffer_t *buffer,
    struct net_state_t *net_state);

#endif
