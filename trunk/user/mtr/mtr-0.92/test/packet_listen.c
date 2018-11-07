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

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "packet/protocols.h"

#define MAX_PACKET_SIZE 9000

/*
    The first probe sent by mtr-packet will have this sequence number,
    so wait for an ICMP packet with this sequence ID.
*/
#define SEQUENCE_NUM 33000

/*
    Check to see if the packet we've recieved is intended for this test
    process.  We expected the ICMP sequence number to be equal to our
    process ID.
*/
bool is_packet_for_us4(
    char *packet,
    int packet_size)
{
    int ip_icmp_size = sizeof(struct IPHeader) + sizeof(struct ICMPHeader);
    int expected_sequence;
    struct IPHeader *ip;
    struct ICMPHeader *icmp;

    if (packet_size < ip_icmp_size) {
        return false;
    }

    ip = (struct IPHeader *)packet;
    icmp = (struct ICMPHeader *)(ip + 1);

    expected_sequence = htons(SEQUENCE_NUM);
    if (icmp->sequence == expected_sequence) {
        return true;
    }

    return false;
}

/*
    Check to see if the ICMPv6 packet is for us.
    Unlike ICMPv4 packets, ICMPv6 packets don't include the IP header.
*/
bool is_packet_for_us6(
    char *packet,
    int packet_size)
{
    int expected_sequence;
    struct ICMPHeader *icmp;

    if (packet_size < sizeof(struct ICMPHeader)) {
        return false;
    }

    icmp = (struct ICMPHeader *)packet;

    expected_sequence = htons(SEQUENCE_NUM);
    if (icmp->sequence == expected_sequence) {
        return true;
    }

    return false;
}

/*
    Check that all the bytes in the body of the packet have the same value.
    If so, return that value.  If not, return -1.
*/
int get_packet_pattern(
    unsigned char *packet,
    int packet_size)
{
    int fill_value;
    int i;

    if (packet_size <= 0) {
        return -1;
    }

    fill_value = packet[0];
    for (i = 1; i < packet_size; i++) {
        if (packet[i] != fill_value) {
            return -1;
        }
    }

    return fill_value;
}

/*  Print information about the ICMPv4 packet we received  */
void dump_packet_info4(
    char *packet,
    int packet_size)
{
    int ip_icmp_size = sizeof(struct IPHeader) + sizeof(struct ICMPHeader);
    int pattern;
    struct IPHeader *ip;
    struct ICMPHeader *icmp;
    unsigned char *body;
    int body_size;

    ip = (struct IPHeader *)packet;
    icmp = (struct ICMPHeader *)(ip + 1);
    body = (unsigned char *)(icmp + 1);
    body_size = packet_size - ip_icmp_size;

    printf("size %d\n", packet_size);
    printf("tos %d\n", ip->tos);

    pattern = get_packet_pattern(body, body_size);
    if (pattern < 0) {
        printf("bitpattern none\n");
    } else {
        printf("bitpattern %d\n", pattern);
    }
}

/*  Print information about an ICMPv6 packet  */
void dump_packet_info6(
    char *packet,
    int packet_size)
{
    int pattern;
    struct ICMPHeader *icmp;
    unsigned char *body;
    int body_size;
    int total_size;

    icmp = (struct ICMPHeader *)packet;
    body = (unsigned char *)(icmp + 1);
    body_size = packet_size - sizeof(struct ICMPHeader);

    total_size = packet_size + sizeof(struct IP6Header);
    printf("size %d\n", total_size);

    pattern = get_packet_pattern(body, body_size);
    if (pattern < 0) {
        printf("bitpattern none\n");
    } else {
        printf("bitpattern %d\n", pattern);
    }
}

/*  Receive ICMP packets until we get one intended for this test process  */
void loop_on_receive(
    int icmp_socket,
    int ip_version)
{
    int packet_size;
    char packet[MAX_PACKET_SIZE];

    while (true) {
        packet_size = recv(icmp_socket, packet, MAX_PACKET_SIZE, 0);
        if (packet_size < -1) {
            perror("Failure during receive");
            exit(EXIT_FAILURE);
        }

        if (ip_version == 6) {
            if (is_packet_for_us6(packet, packet_size)) {
                dump_packet_info6(packet, packet_size);
                return;
            }
        } else {
            if (is_packet_for_us4(packet, packet_size)) {
                dump_packet_info4(packet, packet_size);
                return;
            }
        }
    }
}

/*  Parse the commandline arguments  */
void parse_cmdline(
    int argc,
    char **argv,
    int *ip_version)
{
    int opt;

    *ip_version = 4;

    while ((opt = getopt(argc, argv, "46")) != -1) {
        if (opt == '4') {
            *ip_version = 4;
        }

        if (opt == '6') {
            *ip_version = 6;
        }
    }
}

/*
    A helper for mtr-packet testing which waits for an ICMP packet
    intended for this test process, and then prints information about
    it.
*/
int main(
    int argc,
    char **argv)
{
    int icmp_socket;
    int ip_version;

    parse_cmdline(argc, argv, &ip_version);

    if (ip_version == 6) {
        icmp_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    } else {
        icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    }
    if (icmp_socket < 0) {
        perror("Failure opening listening socket");
        exit(EXIT_FAILURE);
    }

    printf("status listening\n");
    fflush(stdout);

    loop_on_receive(icmp_socket, ip_version);

    return EXIT_SUCCESS;
}
