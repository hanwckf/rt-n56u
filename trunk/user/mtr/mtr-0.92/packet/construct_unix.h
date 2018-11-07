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

#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include "probe.h"

int construct_packet(
    const struct net_state_t *net_state,
    int *packet_socket,
    int sequence,
    char *packet_buffer,
    int packet_buffer_size,
    const struct sockaddr_storage *dest_sockaddr,
    const struct sockaddr_storage *src_sockaddr,
    const struct probe_param_t *param);

#endif
