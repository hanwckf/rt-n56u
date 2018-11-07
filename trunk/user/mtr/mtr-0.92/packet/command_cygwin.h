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

#ifndef COMMAND_CYGWIN_H
#define COMMAND_CYGWIN_H

/*
    Though Cygwin supports the usual Unix non-blocking reads on
    the command stream, we've got to use Overlapped I/O instead because
    ICMP.DLL's support for sending probes requires Overlapped I/O
    and alertable waits for notification of replies.  Since we need
    alertable waits, we can't use Cygwin's select to determine when
    command stream data is available, but Overlapped I/O completion
    will work.
*/

/*  Overlapped I/O manament for Windows command buffer reads  */
struct command_buffer_platform_t {
    /*  true if an overlapped I/O read is active  */
    bool read_active;

    /*  true if the command pipe is still open  */
    bool pipe_open;

    /*  Windows OVERLAPPED I/O data  */
    OVERLAPPED overlapped;

    /*  The buffer which active OVERLAPPED reads read into  */
    char overlapped_buffer[COMMAND_BUFFER_SIZE];
};

struct command_buffer_t;

void start_read_command(
    struct command_buffer_t *buffer);

#endif
