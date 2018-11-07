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

#include "command.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
    Initialize the command buffer and put the command stream in
    non-blocking mode.
*/
void init_command_buffer(
    struct command_buffer_t *command_buffer,
    int command_stream)
{
    int flags;

    memset(command_buffer, 0, sizeof(struct command_buffer_t));
    command_buffer->command_stream = command_stream;

    /*  Get the current command stream flags  */
    flags = fcntl(command_stream, F_GETFL, 0);
    if (flags == -1) {
        perror("Unexpected command stream error");
        exit(EXIT_FAILURE);
    }

    /*  Set the O_NONBLOCK bit  */
    if (fcntl(command_stream, F_SETFL, flags | O_NONBLOCK)) {
        perror("Unexpected command stream error");
        exit(EXIT_FAILURE);
    }
}

/*  Read currently available data from the command stream  */
int read_commands(
    struct command_buffer_t *buffer)
{
    int space_remaining =
        COMMAND_BUFFER_SIZE - buffer->incoming_read_position - 1;
    char *read_position =
        &buffer->incoming_buffer[buffer->incoming_read_position];
    int read_count;
    int command_stream = buffer->command_stream;

    read_count = read(command_stream, read_position, space_remaining);

    /*  If the command stream has been closed, read will return zero.  */
    if (read_count == 0) {
        errno = EPIPE;
        return -1;
    }

    if (read_count > 0) {
        /*  Account for the newly read data  */
        buffer->incoming_read_position += read_count;
    }

    if (read_count < 0) {
        /*  EAGAIN simply means there is no available data to read  */
        /*  EINTR indicates we received a signal during read  */
        if (errno != EINTR && errno != EAGAIN) {
            perror("Unexpected command buffer read error");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
