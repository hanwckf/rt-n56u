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
#include <io.h>
#include <stdio.h>

/*
    A completion routine to be called by Windows when a read from
    the command stream has completed.
*/
static
void CALLBACK finish_read_command(
    DWORD status,
    DWORD size_read,
    OVERLAPPED * overlapped)
{
    struct command_buffer_t *buffer;
    char *read_position;

    /*
       hEvent is unusuaed by ReadFileEx, so we use it to pass
       our command_buffer structure.
     */
    buffer = (struct command_buffer_t *) overlapped->hEvent;

    if (status) {
        /*  When the stream is closed ERROR_BROKEN_PIPE will be the result  */
        if (status == ERROR_BROKEN_PIPE) {
            buffer->platform.pipe_open = false;
            return;
        }

        fprintf(stderr, "ReadFileEx completion failure %d\n", status);
        exit(EXIT_FAILURE);
    }

    /*  Copy from the overlapped I/O buffer to the incoming command buffer  */
    read_position =
        &buffer->incoming_buffer[buffer->incoming_read_position];
    memcpy(read_position, buffer->platform.overlapped_buffer, size_read);

    /*  Account for the newly read data  */
    buffer->incoming_read_position += size_read;
    buffer->platform.read_active = false;
}

/*
    An APC which does nothing, to be used only to wake from the current
    alertable wait.
*/
static
void CALLBACK empty_apc(
    ULONG * param)
{
}

/*  Wake from the next alertable wait without waiting for newly read data  */
static
void queue_empty_apc(
    void)
{
    if (QueueUserAPC((PAPCFUNC) empty_apc, GetCurrentThread(), 0) == 0) {
        fprintf(stderr, "Unexpected QueueUserAPC failure %d\n",
                GetLastError());
        exit(EXIT_FAILURE);
    }
}

/*  Start a new overlapped I/O read from the command stream  */
void start_read_command(
    struct command_buffer_t *buffer)
{
    HANDLE command_stream = (HANDLE) get_osfhandle(buffer->command_stream);
    int space_remaining =
        COMMAND_BUFFER_SIZE - buffer->incoming_read_position - 1;
    int err;

    /*  If a read is already active, or the pipe is closed, do nothing  */
    if (!buffer->platform.pipe_open || buffer->platform.read_active) {
        return;
    }

    memset(&buffer->platform.overlapped, 0, sizeof(OVERLAPPED));
    buffer->platform.overlapped.hEvent = (HANDLE) buffer;

    if (!ReadFileEx
        (command_stream, buffer->platform.overlapped_buffer,
         space_remaining, &buffer->platform.overlapped,
         finish_read_command)) {

        err = GetLastError();

        if (err == ERROR_BROKEN_PIPE) {
            /*  If the command stream has been closed, we need to wake from
               the next altertable wait to exit the main loop  */
            buffer->platform.pipe_open = false;
            queue_empty_apc();

            return;
        } else if (err != WAIT_IO_COMPLETION) {
            fprintf(stderr, "Unexpected ReadFileEx failure %d\n",
                    GetLastError());
            exit(EXIT_FAILURE);
        }
    }

    /*  Remember that we have started an overlapped read already  */
    buffer->platform.read_active = true;
}

/*  Initialize the command buffer, and start the first overlapped read  */
void init_command_buffer(
    struct command_buffer_t *command_buffer,
    int command_stream)
{
    memset(command_buffer, 0, sizeof(struct command_buffer_t));
    command_buffer->command_stream = command_stream;
    command_buffer->platform.pipe_open = true;
}

/*
    Return with errno EPIPE if the command stream has been closed.
    Otherwise, not much to do for Cygwin, since we are using Overlapped I/O
    to read commands.
*/
int read_commands(
    struct command_buffer_t *buffer)
{
    if (!buffer->platform.pipe_open) {
        errno = EPIPE;
        return -1;
    }

    return 0;
}
