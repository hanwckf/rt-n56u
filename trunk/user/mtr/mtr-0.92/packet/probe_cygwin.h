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

#ifndef PROBE_CYGWIN_H
#define PROBE_CYGWIN_H

#include <arpa/inet.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>

/*
    This should be in the Windows headers, but is missing from
    Cygwin's Windows headers.
*/
typedef struct icmpv6_echo_reply_lh {
    /*
       Although Windows uses an IPV6_ADDRESS_EX here, we are using uint8_t
       fields to avoid structure padding differences between gcc and
       Visual C++.  (gcc wants to align the flow info to a 4 byte boundary,
       and Windows uses it unaligned.)
     */
    uint8_t PortBits[2];
    uint8_t FlowInfoBits[4];
    uint8_t AddressBits[16];
    uint8_t ScopeIdBits[4];

    ULONG Status;
    unsigned int RoundTripTime;
} ICMPV6_ECHO_REPLY,
*PICMPV6_ECHO_REPLY;

/*
	Windows requires an echo reply structure for each in-flight
	ICMP probe.
*/
struct probe_platform_t {
    /*
       We need a backpointer to the net_state because of the way
       IcmpSendEcho2 passes our context.
     */
    struct net_state_t *net_state;

    /*  IP version (4 or 6) used for the probe  */
    int ip_version;

    union {
        ICMP_ECHO_REPLY32 *reply4;
        ICMPV6_ECHO_REPLY *reply6;
    };
};

/*  A Windows HANDLE for the ICMP session  */
struct net_state_platform_t {
    HANDLE icmp4;
    HANDLE icmp6;
};

#endif
