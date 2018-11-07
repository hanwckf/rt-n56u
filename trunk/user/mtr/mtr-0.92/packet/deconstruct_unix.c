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

#include "deconstruct_unix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocols.h"

#define MAX_MPLS_LABELS 8

/*
    Given an ICMP id + ICMP sequence, find the match probe we've
    transmitted and if found, respond to the command which sent it
*/
static
void find_and_receive_probe(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    struct timeval *timestamp,
    int icmp_type,
    int protocol,
    int icmp_id,
    int icmp_sequence,
    int mpls_count,
    struct mpls_label_t *mpls)
{
    struct probe_t *probe;

    probe = find_probe(net_state, protocol, icmp_id, icmp_sequence);
    if (probe == NULL) {
        return;
    }

    receive_probe(net_state, probe, icmp_type,
                  remote_addr, timestamp, mpls_count, mpls);
}

/*
    Handle a UDP packet received embedded in an ICMP reply.
    The sequence number identifying the probe might be in
    the source port number, the destination port number, or
    the checksum.  We'll check all three.
*/
static
void handle_inner_udp_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    int icmp_result,
    const struct UDPHeader *udp,
    int udp_length,
    struct timeval *timestamp,
    int mpls_count,
    struct mpls_label_t *mpls)
{
    struct probe_t *probe;

    probe = find_probe(net_state, IPPROTO_UDP, 0, udp->dstport);
    if (probe == NULL) {
        probe = find_probe(net_state, IPPROTO_UDP, 0, udp->srcport);
    }
    if (probe == NULL) {
        probe = find_probe(net_state, IPPROTO_UDP, 0, udp->checksum);
    }

    if (probe != NULL) {
        receive_probe(net_state, probe, icmp_result,
                      remote_addr, timestamp, mpls_count, mpls);
    }
}

/*
    We've received an ICMP message with an embedded IP packet.
    We will try to determine which of our outgoing probes
    corresponds to the embedded IP packet and record the response.
*/
static
void handle_inner_ip4_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    int icmp_result,
    const struct IPHeader *ip,
    int packet_length,
    struct timeval *timestamp,
    int mpls_count,
    struct mpls_label_t *mpls)
{
    const int ip_icmp_size =
        sizeof(struct IPHeader) + sizeof(struct ICMPHeader);
    const int ip_udp_size =
        sizeof(struct IPHeader) + sizeof(struct UDPHeader);
    const int ip_tcp_size =
        sizeof(struct IPHeader) + sizeof(struct TCPHeader);
    const struct ICMPHeader *icmp;
    const struct UDPHeader *udp;
    const struct TCPHeader *tcp;
    int udp_length;
#ifdef IPPROTO_SCTP
    const int ip_sctp_size =
        sizeof(struct IPHeader) + sizeof(struct SCTPHeader);
    const struct SCTPHeader *sctp;
#endif

    if (ip->protocol == IPPROTO_ICMP) {
        if (packet_length < ip_icmp_size) {
            return;
        }

        icmp = (struct ICMPHeader *) (ip + 1);

        find_and_receive_probe(net_state, remote_addr, timestamp,
                               icmp_result, IPPROTO_ICMP, icmp->id,
                               icmp->sequence, mpls_count, mpls);
    } else if (ip->protocol == IPPROTO_UDP) {
        if (packet_length < ip_udp_size) {
            return;
        }

        udp = (struct UDPHeader *) (ip + 1);
        udp_length = packet_length - sizeof(struct IPHeader);

        handle_inner_udp_packet(net_state, remote_addr, icmp_result, udp,
                                udp_length, timestamp, mpls_count, mpls);
    } else if (ip->protocol == IPPROTO_TCP) {
        if (packet_length < ip_tcp_size) {
            return;
        }

        tcp = (struct TCPHeader *) (ip + 1);

        find_and_receive_probe(net_state, remote_addr, timestamp,
                               icmp_result, IPPROTO_TCP, 0, tcp->srcport,
                               mpls_count, mpls);
#ifdef IPPROTO_SCTP
    } else if (ip->protocol == IPPROTO_SCTP) {
        if (packet_length < ip_sctp_size) {
            return;
        }

        sctp = (struct SCTPHeader *) (ip + 1);

        find_and_receive_probe(net_state, remote_addr, timestamp,
                               icmp_result, IPPROTO_SCTP, 0, sctp->srcport,
                               mpls_count, mpls);
#endif
    }
}

/*
    Examine the IPv6 header embedded in a returned ICMPv6 packet
    in order to match it with a probe which we previously sent.
*/
static
void handle_inner_ip6_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    int icmp_result,
    const struct IP6Header *ip,
    int packet_length,
    struct timeval *timestamp,
    int mpls_count,
    struct mpls_label_t *mpls)
{
    const int ip_icmp_size =
        sizeof(struct IP6Header) + sizeof(struct ICMPHeader);
    const int ip_udp_size =
        sizeof(struct IP6Header) + sizeof(struct UDPHeader);
    const int ip_tcp_size =
        sizeof(struct IP6Header) + sizeof(struct TCPHeader);
    const struct ICMPHeader *icmp;
    const struct UDPHeader *udp;
    const struct TCPHeader *tcp;
    int udp_length;
#ifdef IPPROTO_SCTP
    const int ip_sctp_size =
        sizeof(struct IPHeader) + sizeof(struct SCTPHeader);
    const struct SCTPHeader *sctp;
#endif

    if (ip->protocol == IPPROTO_ICMPV6) {
        if (packet_length < ip_icmp_size) {
            return;
        }

        icmp = (struct ICMPHeader *) (ip + 1);

        find_and_receive_probe(net_state, remote_addr, timestamp,
                               icmp_result, IPPROTO_ICMP, icmp->id,
                               icmp->sequence, mpls_count, mpls);
    } else if (ip->protocol == IPPROTO_UDP) {
        if (packet_length < ip_udp_size) {
            return;
        }

        udp = (struct UDPHeader *) (ip + 1);
        udp_length = packet_length - sizeof(struct IP6Header);

        handle_inner_udp_packet(net_state, remote_addr, icmp_result, udp,
                                udp_length, timestamp, mpls_count, mpls);
    } else if (ip->protocol == IPPROTO_TCP) {
        if (packet_length < ip_tcp_size) {
            return;
        }

        tcp = (struct TCPHeader *) (ip + 1);
        find_and_receive_probe(net_state, remote_addr, timestamp,
                               icmp_result, IPPROTO_TCP, 0, tcp->srcport,
                               mpls_count, mpls);
#ifdef IPPROTO_SCTP
    } else if (ip->protocol == IPPROTO_SCTP) {
        if (packet_length < ip_sctp_size) {
            return;
        }

        sctp = (struct SCTPHeader *) (ip + 1);

        find_and_receive_probe(net_state, remote_addr, timestamp,
                               icmp_result, IPPROTO_SCTP, 0, sctp->srcport,
                               mpls_count, mpls);
#endif
    }
}

/*  Convert an ICMP MPLS extension object into an mpls_label_t structure  */
static
int decode_mpls_object(
    struct ICMPExtensionObject *icmp_obj,
    int obj_len,
    struct mpls_label_t *mpls,
    int mpls_count)
{
    int label_bytes;
    int labels_present;
    int i;
    struct ICMPExtMPLSLabel *ext_mpls;
    struct ICMPExtMPLSLabel *ext_label;
    struct mpls_label_t *label;

    label_bytes = obj_len - sizeof(struct ICMPExtensionObject);
    labels_present = label_bytes / sizeof(struct ICMPExtMPLSLabel);

    ext_mpls = (struct ICMPExtMPLSLabel *) (icmp_obj + 1);
    for (i = 0; i < mpls_count && i < labels_present; i++) {
        ext_label = &ext_mpls[i];
        label = &mpls[i];

        memset(label, 0, sizeof(struct mpls_label_t));

        label->label =
            ext_label->label[0] << 12 |
            ext_label->label[1] << 4 | ext_label->label[2] >> 4;
        label->experimental_use = (ext_label->label[2] & 0x0E) >> 1;
        label->bottom_of_stack = ext_label->label[2] & 0x01;
        label->ttl = ext_label->ttl;
    }

    return i;
}

/*  Extract MPLS labels from the ICMP extension header, if present  */
static
int decode_mpls_labels(
    const struct ICMPHeader *icmp,
    int packet_length,
    struct mpls_label_t *mpls,
    int mpls_count)
{
    const int icmp_orig_icmp_ext_size =
        sizeof(struct ICMPHeader) + ICMP_ORIGINAL_DATAGRAM_MIN_SIZE +
        sizeof(struct ICMPExtensionHeader);
    char *inner_packet;
    char *icmp_object_bytes;
    struct ICMPExtensionHeader *icmp_ext;
    struct ICMPExtensionObject *icmp_obj;
    int remaining_size;
    int obj_len;

    if (packet_length < icmp_orig_icmp_ext_size) {
        return 0;
    }

    inner_packet = (char *) (icmp + 1);
    icmp_ext = (struct ICMPExtensionHeader *)
        (inner_packet + ICMP_ORIGINAL_DATAGRAM_MIN_SIZE);

    if ((icmp_ext->version & 0xF0) != 0x20) {
        return 0;
    }

    remaining_size = packet_length - icmp_orig_icmp_ext_size;
    icmp_object_bytes = (char *) (icmp_ext + 1);

    /*
       Iterate through the chain of extension objects, looking for
       an MPLS label extension.
     */
    while (remaining_size >= sizeof(struct ICMPExtensionObject)) {
        icmp_obj = (struct ICMPExtensionObject *) icmp_object_bytes;
        obj_len = ntohs(icmp_obj->len);

        if (obj_len > remaining_size) {
            return 0;
        }
        if (obj_len < sizeof(struct ICMPExtensionObject)) {
            return 0;
        }

        if (icmp_obj->classnum == ICMP_EXT_MPLS_CLASSNUM &&
            icmp_obj->ctype == ICMP_EXT_MPLS_CTYPE) {

            return decode_mpls_object(icmp_obj, obj_len, mpls, mpls_count);
        }

        remaining_size -= obj_len;
        icmp_object_bytes += obj_len;
    }

    return 0;
}

/*
    Decode the ICMP header received and try to find a probe which it
    is in response to.
*/
static
void handle_received_icmp4_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    const struct ICMPHeader *icmp,
    int packet_length,
    struct timeval *timestamp)
{
    const int icmp_ip_size =
        sizeof(struct ICMPHeader) + sizeof(struct IPHeader);
    const struct IPHeader *inner_ip;
    int inner_size = packet_length - sizeof(struct ICMPHeader);
    int mpls_count;
    struct mpls_label_t mpls[MAX_MPLS_LABELS];

    mpls_count =
        decode_mpls_labels(icmp, packet_length, mpls, MAX_MPLS_LABELS);

    /*  If we get an echo reply, our probe reached the destination host  */
    if (icmp->type == ICMP_ECHOREPLY) {
        find_and_receive_probe(net_state, remote_addr, timestamp,
                               ICMP_ECHOREPLY, IPPROTO_ICMP, icmp->id,
                               icmp->sequence, mpls_count, mpls);
    }

    if (packet_length < icmp_ip_size) {
        return;
    }
    inner_ip = (struct IPHeader *) (icmp + 1);

    /*
       If we get a time exceeded, we got a response from an intermediate
       host along the path to our destination.
     */
    if (icmp->type == ICMP_TIME_EXCEEDED) {
        /*
           The IP packet inside the ICMP response contains our original
           IP header.  That's where we can get our original ID and
           sequence number.
         */
        handle_inner_ip4_packet(net_state, remote_addr,
                                ICMP_TIME_EXCEEDED, inner_ip, inner_size,
                                timestamp, mpls_count, mpls);
    }

    if (icmp->type == ICMP_DEST_UNREACH) {
        /*
           We'll get a ICMP_PORT_UNREACH when a non-ICMP probe
           reaches its final destination.  (Assuming that port isn't
           open on the destination host.)
         */
        if (icmp->code == ICMP_PORT_UNREACH) {
            handle_inner_ip4_packet(net_state, remote_addr,
                                    ICMP_ECHOREPLY, inner_ip, inner_size,
                                    timestamp, mpls_count, mpls);
        }
    }
}

/*
    Decode the ICMPv6 header.  The code duplication with ICMPv4 is
    unfortunate, but small details in structure size and ICMP
    constants differ.
*/
static
void handle_received_icmp6_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    const struct ICMPHeader *icmp,
    int packet_length,
    struct timeval *timestamp)
{
    const int icmp_ip_size =
        sizeof(struct ICMPHeader) + sizeof(struct IP6Header);
    const struct IP6Header *inner_ip;
    int inner_size = packet_length - sizeof(struct ICMPHeader);
    int mpls_count;
    struct mpls_label_t mpls[MAX_MPLS_LABELS];

    mpls_count =
        decode_mpls_labels(icmp, packet_length, mpls, MAX_MPLS_LABELS);

    if (icmp->type == ICMP6_ECHOREPLY) {
        find_and_receive_probe(net_state, remote_addr, timestamp,
                               ICMP_ECHOREPLY, IPPROTO_ICMP, icmp->id,
                               icmp->sequence, mpls_count, mpls);
    }

    if (packet_length < icmp_ip_size) {
        return;
    }
    inner_ip = (struct IP6Header *) (icmp + 1);

    if (icmp->type == ICMP6_TIME_EXCEEDED) {
        handle_inner_ip6_packet(net_state, remote_addr,
                                ICMP_TIME_EXCEEDED, inner_ip, inner_size,
                                timestamp, mpls_count, mpls);
    }

    if (icmp->type == ICMP6_DEST_UNREACH) {
        if (icmp->code == ICMP6_PORT_UNREACH) {
            handle_inner_ip6_packet(net_state, remote_addr,
                                    ICMP_ECHOREPLY, inner_ip, inner_size,
                                    timestamp, mpls_count, mpls);
        }
    }
}

/*
    We've received a new IPv4 ICMP packet.
    We'll check to see that it is a response to one of our probes, and
    if so, report the result of the probe to our command stream.
*/
void handle_received_ip4_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    const void *packet,
    int packet_length,
    struct timeval *timestamp)
{
    const int ip_icmp_size =
        sizeof(struct IPHeader) + sizeof(struct ICMPHeader);
    const struct IPHeader *ip;
    const struct ICMPHeader *icmp;
    int icmp_length;

    /*  Ensure that we don't access memory beyond the bounds of the packet  */
    if (packet_length < ip_icmp_size) {
        return;
    }

    ip = (struct IPHeader *) packet;
    if (ip->protocol != IPPROTO_ICMP) {
        return;
    }

    icmp = (struct ICMPHeader *) (ip + 1);
    icmp_length = packet_length - sizeof(struct IPHeader);

    handle_received_icmp4_packet(net_state, remote_addr, icmp, icmp_length,
                                 timestamp);
}

/*
    Unlike ICMPv6 raw sockets, unlike ICMPv4, don't include the IP header
    in received packets, so we can assume the packet we got starts
    with the ICMP packet.
*/
void handle_received_ip6_packet(
    struct net_state_t *net_state,
    const struct sockaddr_storage *remote_addr,
    const void *packet,
    int packet_length,
    struct timeval *timestamp)
{
    const struct ICMPHeader *icmp;

    icmp = (struct ICMPHeader *) packet;

    handle_received_icmp6_packet(net_state, remote_addr, icmp,
                                 packet_length, timestamp);
}
