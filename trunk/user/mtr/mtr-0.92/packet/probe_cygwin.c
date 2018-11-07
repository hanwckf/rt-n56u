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

#include "probe.h"

#include <errno.h>
#include <stdio.h>
#include <winternl.h>

#include "protocols.h"

/*  Windows doesn't require any initialization at a privileged level  */
void init_net_state_privileged(
    struct net_state_t *net_state)
{
}

/*  Open the ICMP.DLL interface  */
void init_net_state(
    struct net_state_t *net_state)
{
    memset(net_state, 0, sizeof(struct net_state_t));

    net_state->platform.icmp4 = IcmpCreateFile();
    net_state->platform.icmp6 = Icmp6CreateFile();

    if (net_state->platform.icmp4 == INVALID_HANDLE_VALUE
        && net_state->platform.icmp6 == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failure opening ICMP %d\n", GetLastError());
        exit(EXIT_FAILURE);
    }
}

/*
    If we succeeded at opening the ICMP file handle, we can
    assume that IP protocol version is supported.
*/
bool is_ip_version_supported(
    struct net_state_t *net_state,
    int ip_version)
{
    if (ip_version == 4) {
        return (net_state->platform.icmp4 != INVALID_HANDLE_VALUE);
    } else if (ip_version == 6) {
        return (net_state->platform.icmp6 != INVALID_HANDLE_VALUE);
    }

    return false;
}

/*  On Windows, we only support ICMP probes  */
bool is_protocol_supported(
    struct net_state_t * net_state,
    int protocol)
{
    if (protocol == IPPROTO_ICMP) {
        return true;
    }

    return false;
}

/*  Set the back pointer to the net_state when a probe is allocated  */
void platform_alloc_probe(
    struct net_state_t *net_state,
    struct probe_t *probe)
{
    probe->platform.net_state = net_state;
}

/*  Free the reply buffer when the probe is freed  */
void platform_free_probe(
    struct probe_t *probe)
{
    if (probe->platform.reply4) {
        free(probe->platform.reply4);
        probe->platform.reply4 = NULL;
    }
}

/*  Report a windows error code using a platform-independent error string  */
static
void report_win_error(
    int command_token,
    int err)
{
    /*  It could be that we got no reply because of timeout  */
    if (err == IP_REQ_TIMED_OUT || err == IP_SOURCE_QUENCH) {
        printf("%d no-reply\n", command_token);
    } else if (err == IP_DEST_HOST_UNREACHABLE
               || err == IP_DEST_PORT_UNREACHABLE
               || err == IP_DEST_PROT_UNREACHABLE
               || err == IP_DEST_NET_UNREACHABLE
               || err == IP_DEST_UNREACHABLE
               || err == IP_DEST_NO_ROUTE
               || err == IP_BAD_ROUTE || err == IP_BAD_DESTINATION) {
        printf("%d no-route\n", command_token);
    } else if (err == ERROR_INVALID_NETNAME) {
        printf("%d address-not-available\n", command_token);
    } else if (err == ERROR_INVALID_PARAMETER) {
        printf("%d invalid-argument\n", command_token);
    } else {
        printf("%d unexpected-error winerror %d\n", command_token, err);
    }
}

/*
    The overlapped I/O style completion routine to be called by
    Windows during an altertable wait when an ICMP probe has
    completed, either by reply, or by ICMP.DLL timeout.
*/
static
void WINAPI on_icmp_reply(
    PVOID context,
    PIO_STATUS_BLOCK status,
    ULONG reserved)
{
    struct probe_t *probe = (struct probe_t *) context;
    struct net_state_t *net_state = probe->platform.net_state;
    int icmp_type;
    int round_trip_us = 0;
    int reply_count;
    int reply_status = 0;
    struct sockaddr_storage remote_addr;
    struct sockaddr_in *remote_addr4;
    struct sockaddr_in6 *remote_addr6;
    ICMP_ECHO_REPLY32 *reply4;
    ICMPV6_ECHO_REPLY *reply6;

    if (probe->platform.ip_version == 6) {
        reply6 = probe->platform.reply6;
        reply_count = Icmp6ParseReplies(reply6, sizeof(ICMPV6_ECHO_REPLY));

        if (reply_count > 0) {
            reply_status = reply6->Status;

            /*  Unfortunately, ICMP.DLL only has millisecond precision  */
            round_trip_us = reply6->RoundTripTime * 1000;

            remote_addr6 = (struct sockaddr_in6 *) &remote_addr;
            remote_addr6->sin6_family = AF_INET6;
            remote_addr6->sin6_port = 0;
            remote_addr6->sin6_flowinfo = 0;
            memcpy(&remote_addr6->sin6_addr, reply6->AddressBits,
                   sizeof(struct in6_addr));
            remote_addr6->sin6_scope_id = 0;
        }
    } else {
        reply4 = probe->platform.reply4;
        reply_count = IcmpParseReplies(reply4, sizeof(ICMP_ECHO_REPLY));

        if (reply_count > 0) {
            reply_status = reply4->Status;

            /*  Unfortunately, ICMP.DLL only has millisecond precision  */
            round_trip_us = reply4->RoundTripTime * 1000;

            remote_addr4 = (struct sockaddr_in *) &remote_addr;
            remote_addr4->sin_family = AF_INET;
            remote_addr4->sin_port = 0;
            remote_addr4->sin_addr.s_addr = reply4->Address;
        }
    }

    if (reply_count == 0) {
        reply_status = GetLastError();
    }

    icmp_type = -1;
    if (reply_status == IP_SUCCESS) {
        icmp_type = ICMP_ECHOREPLY;
    } else if (reply_status == IP_TTL_EXPIRED_TRANSIT
               || reply_status == IP_TTL_EXPIRED_REASSEM) {
        icmp_type = ICMP_TIME_EXCEEDED;
    }

    if (icmp_type != -1) {
        /*  Record probe result  */
        respond_to_probe(net_state, probe, icmp_type,
                         &remote_addr, round_trip_us, 0, NULL);
    } else {
        report_win_error(probe->token, reply_status);
        free_probe(net_state, probe);
    }
}

/*  Use ICMP.DLL's send echo support to send a probe  */
static
void icmp_send_probe(
    struct net_state_t *net_state,
    struct probe_t *probe,
    const struct probe_param_t *param,
    struct sockaddr_storage *src_sockaddr,
    struct sockaddr_storage *dest_sockaddr,
    char *payload,
    int payload_size)
{
    IP_OPTION_INFORMATION option;
    DWORD timeout;
    DWORD send_result;
    int reply_size;
    int err;
    struct sockaddr_in *dest_sockaddr4;
    struct sockaddr_in6 *src_sockaddr6;
    struct sockaddr_in6 *dest_sockaddr6;

    if (param->timeout > 0) {
        timeout = 1000 * param->timeout;
    } else {
        /*
           IcmpSendEcho2 will return invalid argument on a timeout of 
           zero.  Our Unix implementation allows it.  Bump up the timeout
           to 1 millisecond.
         */
        timeout = 1;
    }

    memset(&option, 0, sizeof(IP_OPTION_INFORMATION32));
    option.Ttl = param->ttl;

    if (param->ip_version == 6) {
        reply_size = sizeof(ICMPV6_ECHO_REPLY) + payload_size;
    } else {
        reply_size = sizeof(ICMP_ECHO_REPLY32) + payload_size;
    }

    probe->platform.reply4 = malloc(reply_size);
    if (probe->platform.reply4 == NULL) {
        perror("failure to allocate reply buffer");
        exit(EXIT_FAILURE);
    }

    if (param->ip_version == 6) {
        src_sockaddr6 = (struct sockaddr_in6 *) src_sockaddr;
        dest_sockaddr6 = (struct sockaddr_in6 *) dest_sockaddr;

        send_result = Icmp6SendEcho2(net_state->platform.icmp6, NULL,
                                     (FARPROC) on_icmp_reply, probe,
                                     src_sockaddr6, dest_sockaddr6,
                                     payload, payload_size, &option,
                                     probe->platform.reply6, reply_size,
                                     timeout);
    } else {
        dest_sockaddr4 = (struct sockaddr_in *) dest_sockaddr;

        send_result = IcmpSendEcho2(net_state->platform.icmp4, NULL,
                                    (FARPROC) on_icmp_reply, probe,
                                    dest_sockaddr4->sin_addr.s_addr,
                                    payload, payload_size, &option,
                                    probe->platform.reply4, reply_size,
                                    timeout);
    }

    if (send_result == 0) {
        err = GetLastError();

        /*
           ERROR_IO_PENDING is expected for asynchronous probes,
           but any other error is unexpected.
         */
        if (err != ERROR_IO_PENDING) {
            report_win_error(probe->token, err);
            free_probe(net_state, probe);
        }
    }
}

/*  Fill the payload of the packet as specified by the probe parameters  */
static
int fill_payload(
    const struct probe_param_t *param,
    char *payload,
    int payload_buffer_size)
{
    int ip_icmp_size;
    int payload_size;

    if (param->ip_version == 6) {
        ip_icmp_size =
            sizeof(struct IP6Header) + sizeof(struct ICMPHeader);
    } else if (param->ip_version == 4) {
        ip_icmp_size = sizeof(struct IPHeader) + sizeof(struct ICMPHeader);
    } else {
        errno = EINVAL;
        return -1;
    }

    payload_size = param->packet_size - ip_icmp_size;
    if (payload_size < 0) {
        payload_size = 0;
    }

    if (payload_size > payload_buffer_size) {
        errno = EINVAL;
        return -1;
    }

    memset(payload, param->bit_pattern, payload_size);

    return payload_size;
}

/*  Decode the probe parameters and send a probe  */
void send_probe(
    struct net_state_t *net_state,
    const struct probe_param_t *param)
{
    struct probe_t *probe;
    struct sockaddr_storage dest_sockaddr;
    struct sockaddr_storage src_sockaddr;
    char payload[PACKET_BUFFER_SIZE];
    int payload_size;

    if (resolve_probe_addresses(param, &dest_sockaddr, &src_sockaddr)) {
        printf("%d invalid-argument\n", param->command_token);
        return;
    }

    probe = alloc_probe(net_state, param->command_token);
    if (probe == NULL) {
        printf("%d probes-exhausted\n", param->command_token);
        return;
    }

    probe->platform.ip_version = param->ip_version;

    payload_size = fill_payload(param, payload, PACKET_BUFFER_SIZE);
    if (payload_size < 0) {
        perror("Error construction packet");
        exit(EXIT_FAILURE);
    }

    icmp_send_probe(net_state, probe, param,
                    &src_sockaddr, &dest_sockaddr, payload, payload_size);
}

/*
    On Windows, an implementation of receive_replies is unnecessary, because,
    unlike Unix, replies are completed using Overlapped I/O during an
    alertable wait, and don't require explicit reads.
*/
void receive_replies(
    struct net_state_t *net_state)
{
}

/*
    On Windows, an implementation of check_probe_timeout is unnecesary because
    timeouts are managed by ICMP.DLL, including a call to the I/O completion
    routine when the time fully expires.
*/
void check_probe_timeouts(
    struct net_state_t *net_state)
{
}

/*
    As in the case of check_probe_timeout, getting the next probe timeout is
    unnecessary under Windows, as ICMP.DLL manages timeouts for us.
*/
bool get_next_probe_timeout(
    const struct net_state_t *net_state,
    struct timeval *timeout)
{
    return false;
}
