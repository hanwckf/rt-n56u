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

#ifndef CMDPIPE_H
#define CMDPIPE_H

#include "mtr.h"

#define COMMAND_BUFFER_SIZE 4096
#define PACKET_REPLY_BUFFER_SIZE 4096

/*  We use a pipe to the mtr-packet subprocess to generate probes  */
struct packet_command_pipe_t {
    /*  the process id of mtr-packet  */
    pid_t pid;

    /*  the end of the pipe we read for replies  */
    int read_fd;

    /*  the end of the pipe we write for commands  */
    int write_fd;

    /* storage for incoming replies */
    char reply_buffer[PACKET_REPLY_BUFFER_SIZE];

    /*  the number of bytes currently used in reply_buffer  */
    size_t reply_buffer_used;
};

typedef
void (
    *probe_reply_func_t) (
    struct mtr_ctl * ctl,
    int sequence,
    struct mplslen * mpls,
    ip_t * addr,
    int round_trip_time);

int open_command_pipe(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe);

void close_command_pipe(
    struct packet_command_pipe_t *cmdpipe);

void send_probe_command(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    ip_t * address,
    ip_t * localaddress,
    int packet_size,
    int sequence,
    int time_to_live);

void handle_command_replies(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    probe_reply_func_t reply_func);

#endif
