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

#include "construct_unix.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocols.h"

/*  A source of data for computing a checksum  */
struct checksum_source_t {
    const void *data;
    size_t size;
};

/*  Compute the IP checksum (or ICMP checksum) of a packet.  */
static
uint16_t compute_checksum(
    const void *packet,
    int size)
{
    const uint8_t *packet_bytes = (uint8_t *) packet;
    uint32_t sum = 0;
    int i;

    for (i = 0; i < size; i++) {
        if ((i & 1) == 0) {
            sum += packet_bytes[i] << 8;
        } else {
            sum += packet_bytes[i];
        }
    }

    /*
       Sums which overflow a 16-bit value have the high bits
       added back into the low 16 bits.
     */
    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xffff);
    }

    /*
       The value stored is the one's complement of the
       mathematical sum.
     */
    return (~sum & 0xffff);
}

/*  Encode the IP header length field in the order required by the OS.  */
static
uint16_t length_byte_swap(
    const struct net_state_t *net_state,
    uint16_t length)
{
    if (net_state->platform.ip_length_host_order) {
        return length;
    } else {
        return htons(length);
    }
}

/*  Construct a combined sockaddr from a source address and source port  */
static
void construct_addr_port(
    struct sockaddr_storage *addr_with_port,
    const struct sockaddr_storage *addr,
    int port)
{
    struct sockaddr_in *addr4;
    struct sockaddr_in6 *addr6;

    memcpy(addr_with_port, addr, sizeof(struct sockaddr_storage));

    if (addr->ss_family == AF_INET6) {
        addr6 = (struct sockaddr_in6 *) addr_with_port;
        addr6->sin6_port = htons(port);
    } else {
        addr4 = (struct sockaddr_in *) addr_with_port;
        addr4->sin_port = htons(port);
    }
}

/*  Construct a header for IP version 4  */
static
void construct_ip4_header(
    const struct net_state_t *net_state,
    char *packet_buffer,
    int packet_size,
    const struct sockaddr_storage *srcaddr,
    const struct sockaddr_storage *destaddr,
    const struct probe_param_t *param)
{
    struct IPHeader *ip;
    struct sockaddr_in *srcaddr4 = (struct sockaddr_in *) srcaddr;
    struct sockaddr_in *destaddr4 = (struct sockaddr_in *) destaddr;

    ip = (struct IPHeader *) &packet_buffer[0];

    memset(ip, 0, sizeof(struct IPHeader));

    ip->version = 0x45;
    ip->tos = param->type_of_service;
    ip->len = length_byte_swap(net_state, packet_size);
    ip->ttl = param->ttl;
    ip->protocol = param->protocol;
    memcpy(&ip->saddr, &srcaddr4->sin_addr, sizeof(uint32_t));
    memcpy(&ip->daddr, &destaddr4->sin_addr, sizeof(uint32_t));
}

/*  Construct an ICMP header for IPv4  */
static
void construct_icmp4_header(
    const struct net_state_t *net_state,
    int sequence,
    char *packet_buffer,
    int packet_size,
    const struct probe_param_t *param)
{
    struct ICMPHeader *icmp;
    int icmp_size;

    icmp = (struct ICMPHeader *) &packet_buffer[sizeof(struct IPHeader)];
    icmp_size = packet_size - sizeof(struct IPHeader);

    memset(icmp, 0, sizeof(struct ICMPHeader));

    icmp->type = ICMP_ECHO;
    icmp->id = htons(getpid());
    icmp->sequence = htons(sequence);
    icmp->checksum = htons(compute_checksum(icmp, icmp_size));
}

/*  Construct an ICMP header for IPv6  */
static
int construct_icmp6_packet(
    const struct net_state_t *net_state,
    int sequence,
    char *packet_buffer,
    int packet_size,
    const struct probe_param_t *param)
{
    struct ICMPHeader *icmp;

    icmp = (struct ICMPHeader *) packet_buffer;

    memset(icmp, 0, sizeof(struct ICMPHeader));

    icmp->type = ICMP6_ECHO;
    icmp->id = htons(getpid());
    icmp->sequence = htons(sequence);

    return 0;
}

/*
    Set the port numbers for an outgoing UDP probe.
    There is limited space in the header for a sequence number
    to identify the probe upon return.

    We store the sequence number in the destination port, the local
    port, or the checksum.  The location chosen depends upon which
    probe parameters have been requested.
*/
static
void set_udp_ports(
    struct UDPHeader *udp,
    int sequence,
    const struct probe_param_t *param)
{
    if (param->dest_port) {
        udp->dstport = htons(param->dest_port);

        if (param->local_port) {
            udp->srcport = htons(param->local_port);
            udp->checksum = htons(sequence);
        } else {
            udp->srcport = htons(sequence);
            udp->checksum = 0;
        }
    } else {
        udp->dstport = htons(sequence);

        if (param->local_port) {
            udp->srcport = htons(param->local_port);
        } else {
            udp->srcport = htons(getpid());
        }

        udp->checksum = 0;
    }
}

/*
    Construct a header for UDP probes, using the port number associated
    with the probe.
*/
static
void construct_udp4_header(
    const struct net_state_t *net_state,
    int sequence,
    char *packet_buffer,
    int packet_size,
    const struct probe_param_t *param)
{
    struct UDPHeader *udp;
    int udp_size;

    udp = (struct UDPHeader *) &packet_buffer[sizeof(struct IPHeader)];
    udp_size = packet_size - sizeof(struct IPHeader);

    memset(udp, 0, sizeof(struct UDPHeader));

    set_udp_ports(udp, sequence, param);
    udp->length = htons(udp_size);
}

/*  Construct a header for UDPv6 probes  */
static
int construct_udp6_packet(
    const struct net_state_t *net_state,
    int sequence,
    char *packet_buffer,
    int packet_size,
    const struct probe_param_t *param)
{
    int udp_socket = net_state->platform.udp6_send_socket;
    struct UDPHeader *udp;
    int udp_size;

    udp = (struct UDPHeader *) packet_buffer;
    udp_size = packet_size;

    memset(udp, 0, sizeof(struct UDPHeader));

    set_udp_ports(udp, sequence, param);
    udp->length = htons(udp_size);

    /*
       Instruct the kernel to put the pseudoheader checksum into the
       UDP header.
     */
    int chksum_offset = (char *) &udp->checksum - (char *) udp;
    if (setsockopt(udp_socket, IPPROTO_IPV6,
                   IPV6_CHECKSUM, &chksum_offset, sizeof(int))) {
        return -1;
    }

    return 0;
}

/*
    Set the socket options for an outgoing stream protocol socket based on
    the packet parameters.
*/
static
int set_stream_socket_options(
    int stream_socket,
    const struct probe_param_t *param)
{
    int level;
    int opt;
    int reuse = 1;

    /*  Allow binding to a local port previously in use  */
#ifdef SO_REUSEPORT
    /*
       FreeBSD wants SO_REUSEPORT in addition to SO_REUSEADDR to
       bind to the same port
     */
    if (setsockopt(stream_socket, SOL_SOCKET, SO_REUSEPORT,
                   &reuse, sizeof(int)) == -1) {

        return -1;
    }
#endif

    if (setsockopt(stream_socket, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(int)) == -1) {

        return -1;
    }

    /*  Set the number of hops the probe will transit across  */
    if (param->ip_version == 6) {
        level = IPPROTO_IPV6;
        opt = IPV6_UNICAST_HOPS;
    } else {
        level = IPPROTO_IP;
        opt = IP_TTL;
    }

    if (setsockopt(stream_socket, level, opt, &param->ttl, sizeof(int)) ==
        -1) {

        return -1;
    }

    /*  Set the "type of service" field of the IP header  */
    if (param->ip_version == 6) {
        level = IPPROTO_IPV6;
        opt = IPV6_TCLASS;
    } else {
        level = IPPROTO_IP;
        opt = IP_TOS;
    }

    if (setsockopt(stream_socket, level, opt,
                   &param->type_of_service, sizeof(int)) == -1) {

        return -1;
    }
#ifdef SO_MARK
    if (param->routing_mark) {
        if (setsockopt(stream_socket, SOL_SOCKET,
                       SO_MARK, &param->routing_mark, sizeof(int))) {
            return -1;
        }
    }
#endif

    return 0;
}

/*
    Open a TCP or SCTP socket, respecting the probe paramters as much as
    we can, and use it as an outgoing probe.
*/
static
int open_stream_socket(
    const struct net_state_t *net_state,
    int protocol,
    int port,
    const struct sockaddr_storage *src_sockaddr,
    const struct sockaddr_storage *dest_sockaddr,
    const struct probe_param_t *param)
{
    int stream_socket;
    int addr_len;
    int dest_port;
    struct sockaddr_storage dest_port_addr;
    struct sockaddr_storage src_port_addr;

    if (param->ip_version == 6) {
        stream_socket = socket(AF_INET6, SOCK_STREAM, protocol);
        addr_len = sizeof(struct sockaddr_in6);
    } else if (param->ip_version == 4) {
        stream_socket = socket(AF_INET, SOCK_STREAM, protocol);
        addr_len = sizeof(struct sockaddr_in);
    } else {
        errno = EINVAL;
        return -1;
    }

    if (stream_socket == -1) {
        return -1;
    }

    set_socket_nonblocking(stream_socket);

    if (set_stream_socket_options(stream_socket, param)) {
        close(stream_socket);
        return -1;
    }

    /*
       Bind to a known local port so we can identify which probe
       causes a TTL expiration.
     */
    construct_addr_port(&src_port_addr, src_sockaddr, port);
    if (bind(stream_socket, (struct sockaddr *) &src_port_addr, addr_len)) {
        close(stream_socket);
        return -1;
    }

    if (param->dest_port) {
        dest_port = param->dest_port;
    } else {
        /*  Use http if no port is specified  */
        dest_port = HTTP_PORT;
    }

    /*  Attempt a connection  */
    construct_addr_port(&dest_port_addr, dest_sockaddr, dest_port);
    if (connect
        (stream_socket, (struct sockaddr *) &dest_port_addr, addr_len)) {

        /*  EINPROGRESS simply means the connection is in progress  */
        if (errno != EINPROGRESS) {
            close(stream_socket);
            return -1;
        }
    }

    return stream_socket;
}

/*
    Determine the size of the constructed packet based on the packet
    parameters.  This is the amount of space the packet *we* construct
    uses, and doesn't include any headers the operating system tacks
    onto the packet.  (Such as the IPv6 header on non-Linux operating
    systems.)
*/
static
int compute_packet_size(
    const struct net_state_t *net_state,
    const struct probe_param_t *param)
{
    int packet_size;

    if (param->protocol == IPPROTO_TCP) {
        return 0;
    }
#ifdef IPPROTO_SCTP
    if (param->protocol == IPPROTO_SCTP) {
        return 0;
    }
#endif

    /*  Start by determining the full size, including omitted headers  */
    if (param->ip_version == 6) {
        packet_size = sizeof(struct IP6Header);
    } else if (param->ip_version == 4) {
        packet_size = sizeof(struct IPHeader);
    } else {
        errno = EINVAL;
        return -1;
    }

    if (param->protocol == IPPROTO_ICMP) {
        packet_size += sizeof(struct ICMPHeader);
    } else if (param->protocol == IPPROTO_UDP) {
        packet_size += sizeof(struct UDPHeader);

        /*  We may need to put the sequence number in the payload  */
        packet_size += sizeof(int);
    } else {
        errno = EINVAL;
        return -1;
    }

    /*
       If the requested size from send-probe is greater, extend the
       packet size.
     */
    if (param->packet_size > packet_size) {
        packet_size = param->packet_size;
    }

    /*
       Since we don't explicitly construct the IPv6 header, we
       need to account for it in our transmitted size.
     */
    if (param->ip_version == 6) {
        packet_size -= sizeof(struct IP6Header);
    }

    return packet_size;
}

/*  Construct a packet for an IPv4 probe  */
static
int construct_ip4_packet(
    const struct net_state_t *net_state,
    int *packet_socket,
    int sequence,
    char *packet_buffer,
    int packet_size,
    const struct sockaddr_storage *src_sockaddr,
    const struct sockaddr_storage *dest_sockaddr,
    const struct probe_param_t *param)
{
    int send_socket = net_state->platform.ip4_send_socket;
    bool is_stream_protocol = false;

    if (param->protocol == IPPROTO_TCP) {
        is_stream_protocol = true;
#ifdef IPPROTO_SCTP
    } else if (param->protocol == IPPROTO_SCTP) {
        is_stream_protocol = true;
#endif
    } else {
        construct_ip4_header(net_state, packet_buffer, packet_size,
                             src_sockaddr, dest_sockaddr, param);

        if (param->protocol == IPPROTO_ICMP) {
            construct_icmp4_header(net_state, sequence, packet_buffer,
                                   packet_size, param);
        } else if (param->protocol == IPPROTO_UDP) {
            construct_udp4_header(net_state, sequence, packet_buffer,
                                  packet_size, param);
        } else {
            errno = EINVAL;
            return -1;
        }
    }

    if (is_stream_protocol) {
        send_socket =
            open_stream_socket(net_state, param->protocol, sequence,
                               src_sockaddr, dest_sockaddr, param);

        if (send_socket == -1) {
            return -1;
        }

        *packet_socket = send_socket;
        return 0;
    }

    /*
       The routing mark requires CAP_NET_ADMIN, as opposed to the
       CAP_NET_RAW which we are sometimes explicitly given.
       If we don't have CAP_NET_ADMIN, this will fail, so we'll 
       only set the mark if the user has explicitly requested it.

       Unfortunately, this means that once the mark is set, it won't
       be set on the socket again until a new mark is explicitly
       specified.
     */
#ifdef SO_MARK
    if (param->routing_mark) {
        if (setsockopt(send_socket, SOL_SOCKET,
                       SO_MARK, &param->routing_mark, sizeof(int))) {
            return -1;
        }
    }
#endif

    return 0;
}

/*  Construct a packet for an IPv6 probe  */
static
int construct_ip6_packet(
    const struct net_state_t *net_state,
    int *packet_socket,
    int sequence,
    char *packet_buffer,
    int packet_size,
    const struct sockaddr_storage *src_sockaddr,
    const struct sockaddr_storage *dest_sockaddr,
    const struct probe_param_t *param)
{
    int send_socket;
    bool is_stream_protocol = false;
    bool bind_send_socket = true;
    struct sockaddr_storage current_sockaddr;
    int current_sockaddr_len;

    if (param->protocol == IPPROTO_TCP) {
        is_stream_protocol = true;
#ifdef IPPROTO_SCTP
    } else if (param->protocol == IPPROTO_SCTP) {
        is_stream_protocol = true;
#endif
    } else if (param->protocol == IPPROTO_ICMP) {
        send_socket = net_state->platform.icmp6_send_socket;

        if (construct_icmp6_packet
            (net_state, sequence, packet_buffer, packet_size, param)) {
            return -1;
        }
    } else if (param->protocol == IPPROTO_UDP) {
        send_socket = net_state->platform.udp6_send_socket;

        if (construct_udp6_packet
            (net_state, sequence, packet_buffer, packet_size, param)) {
            return -1;
        }
    } else {
        errno = EINVAL;
        return -1;
    }

    if (is_stream_protocol) {
        send_socket =
            open_stream_socket(net_state, param->protocol, sequence,
                               src_sockaddr, dest_sockaddr, param);

        if (send_socket == -1) {
            return -1;
        }

        *packet_socket = send_socket;
        return 0;
    }

    /*
       Check the current socket address, and if it is the same
       as the source address we intend, we will skip the bind.
       This is to accomodate Solaris, which, as of Solaris 11.3,
       will return an EINVAL error on bind if the socket is already
       bound, even if the same address is used.
     */
    current_sockaddr_len = sizeof(struct sockaddr_in6);
    if (getsockname(send_socket, (struct sockaddr *) &current_sockaddr,
                    &current_sockaddr_len) == 0) {

        if (memcmp(&current_sockaddr,
                   src_sockaddr, sizeof(struct sockaddr_in6)) == 0) {
            bind_send_socket = false;
        }
    }

    /*  Bind to our local address  */
    if (bind_send_socket) {
        if (bind(send_socket, (struct sockaddr *) src_sockaddr,
                 sizeof(struct sockaddr_in6))) {
            return -1;
        }
    }

    /*  The traffic class in IPv6 is analagous to ToS in IPv4  */
    if (setsockopt(send_socket, IPPROTO_IPV6,
                   IPV6_TCLASS, &param->type_of_service, sizeof(int))) {
        return -1;
    }

    /*  Set the time-to-live  */
    if (setsockopt(send_socket, IPPROTO_IPV6,
                   IPV6_UNICAST_HOPS, &param->ttl, sizeof(int))) {
        return -1;
    }
#ifdef SO_MARK
    if (param->routing_mark) {
        if (setsockopt(send_socket,
                       SOL_SOCKET, SO_MARK, &param->routing_mark,
                       sizeof(int))) {
            return -1;
        }
    }
#endif

    return 0;
}

/*  Construct a probe packet based on the probe parameters  */
int construct_packet(
    const struct net_state_t *net_state,
    int *packet_socket,
    int sequence,
    char *packet_buffer,
    int packet_buffer_size,
    const struct sockaddr_storage *dest_sockaddr,
    const struct sockaddr_storage *src_sockaddr,
    const struct probe_param_t *param)
{
    int packet_size;

    packet_size = compute_packet_size(net_state, param);
    if (packet_size < 0) {
        return -1;
    }

    if (packet_buffer_size < packet_size) {
        errno = EINVAL;
        return -1;
    }

    memset(packet_buffer, param->bit_pattern, packet_size);

    if (param->ip_version == 6) {
        if (construct_ip6_packet(net_state, packet_socket, sequence,
                                 packet_buffer, packet_size,
                                 src_sockaddr, dest_sockaddr, param)) {
            return -1;
        }
    } else if (param->ip_version == 4) {
        if (construct_ip4_packet(net_state, packet_socket, sequence,
                                 packet_buffer, packet_size,
                                 src_sockaddr, dest_sockaddr, param)) {
            return -1;
        }
    } else {
        errno = EINVAL;
        return -1;
    }

    return packet_size;
}
