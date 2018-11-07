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

#ifndef PROBE_H
#define PROBE_H

#include "platform.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "portability/queue.h"

#ifdef PLATFORM_CYGWIN
#include "probe_cygwin.h"
#else
#include "probe_unix.h"
#endif

#define MAX_PROBES 1024

/*  Use the "jumbo" frame size as the max packet size  */
#define PACKET_BUFFER_SIZE 9000

/*  Parameters for sending a new probe  */
struct probe_param_t {
    /*  The version of the Internet Protocol to use.  (4 or 6)  */
    int ip_version;

    /*  The command token used to identify a probe when it is completed  */
    int command_token;

    /*  The IP address to probe  */
    const char *remote_address;

    /*  The local address from which to send probes  */
    const char *local_address;

    /*  Protocol for the probe, using the IPPROTO_* defines  */
    int protocol;

    /*  The destination port for non-ICMP probes  */
    int dest_port;

    /*  The local port number to use when sending probes  */
    int local_port;

    /*  The "type of service" field in the IP header  */
    int type_of_service;

    /*  The packet "mark" used for mark-based routing on Linux  */
    int routing_mark;

    /*  Time to live for the transmited probe  */
    int ttl;

    /*  The packet size (in bytes) including protocol headers  */
    int packet_size;

    /*  The value with which to fill the bytes of the packet.  */
    int bit_pattern;

    /*  The number of seconds to wait before assuming the probe was lost  */
    int timeout;
};

/*  Tracking information for an outstanding probe  */
struct probe_t {
    /*  Our entry in the probe list  */
    LIST_ENTRY(
    probe_t) probe_list_entry;

    /*
       Also the ICMP sequence ID used to identify the probe.

       Also used as the port number to use when binding stream protocol
       sockets for this probe.  (i.e. TCP or SCTP)
     */
    int sequence;

    /*  Command token of the probe request  */
    int token;

    /*  The address being probed  */
    struct sockaddr_storage remote_addr;

    /*  Platform specific probe tracking  */
    struct probe_platform_t platform;
};

/*  Global state for interacting with the network  */
struct net_state_t {
    /*  The number of entries in the outstanding_probes list  */
    int outstanding_probe_count;

    /*  Tracking information for in-flight probes  */
     LIST_HEAD(
    probe_list_head_t,
    probe_t) outstanding_probes;

    /*  Platform specific tracking information  */
    struct net_state_platform_t platform;
};

/*  Multiprotocol Label Switching information  */
struct mpls_label_t {
    uint32_t label;
    uint8_t experimental_use;
    uint8_t bottom_of_stack;
    uint8_t ttl;
};

void init_net_state_privileged(
    struct net_state_t *net_state);

void init_net_state(
    struct net_state_t *net_state);

bool is_ip_version_supported(
    struct net_state_t *net_state,
    int ip_version);

bool is_protocol_supported(
    struct net_state_t *net_state,
    int protocol);

bool get_next_probe_timeout(
    const struct net_state_t *net_state,
    struct timeval *timeout);

void send_probe(
    struct net_state_t *net_state,
    const struct probe_param_t *param);

void receive_replies(
    struct net_state_t *net_state);

void check_probe_timeouts(
    struct net_state_t *net_state);

void respond_to_probe(
    struct net_state_t *net_state,
    struct probe_t *probe,
    int icmp_type,
    const struct sockaddr_storage *remote_addr,
    unsigned int round_trip_us,
    int mpls_count,
    const struct mpls_label_t *mpls);

int decode_address_string(
    int ip_version,
    const char *address_string,
    struct sockaddr_storage *address);

int resolve_probe_addresses(
    const struct probe_param_t *param,
    struct sockaddr_storage *dest_sockaddr,
    struct sockaddr_storage *src_sockaddr);

struct probe_t *alloc_probe(
    struct net_state_t *net_state,
    int token);

void free_probe(
    struct net_state_t *net_state,
    struct probe_t *probe);

void platform_alloc_probe(
    struct net_state_t *net_state,
    struct probe_t *probe);

void platform_free_probe(
    struct probe_t *probe);

struct probe_t *find_probe(
    struct net_state_t *net_state,
    int protocol,
    int icmp_id,
    int icmp_sequence);

int find_source_addr(
    struct sockaddr_storage *srcaddr,
    const struct sockaddr_storage *destaddr);

#endif
