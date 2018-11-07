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

#ifndef DECONSTRUCT_H
#define DECONSTRUCT_H

#include "probe.h"

typedef void (
    *received_packet_func_t) (
    struct net_state_t * net_state,
    const struct sockaddr_storage * remote_addr,
    const void *packet,
    int packet_length,
    struct timeval * timestamp);

void handle_received_ip4_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    const void *packet,
    int packet_length,
    struct timeval *timestamp);

void handle_received_ip6_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    const void *packet,
    int packet_length,
    struct timeval *timestamp);

#endif
