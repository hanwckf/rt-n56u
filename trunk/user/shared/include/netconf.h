/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Network configuration layer
 *
 * Copyright 2003, ASUSTeK Inc.
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of ASUSTeK Inc..                            
 *
 * $Id: netconf.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 */

#ifndef _netconf_h_
#define _netconf_h_

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <net/ethernet.h>

//typedef unsigned char   bool;   // 1204 ham

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1  /* TRUE */
#endif

#ifndef TYPEDEF_UINT16
typedef unsigned short	uint16;
#endif

#ifndef WEBSTRFILTER
//2008.10 magic{
#define WEBSTRFILTER/* Cherry Cho added in 2008/1/7. */
//2008.10 magic}
#endif

/* Supported match states */
#define NETCONF_INVALID		0x01	/* Packet could not be classified */
#define NETCONF_ESTABLISHED	0x02	/* Packet is related to an existing connection */
#define NETCONF_RELATED		0x04	/* Packet is part of an established connection */
#define NETCONF_NEW		0x08	/* Packet is trying to establish a new connection */

/* Supported match flags */
#define	NETCONF_INV_SRCIP	0x01	/* Invert the sense of source IP address */
#define	NETCONF_INV_DSTIP	0x02	/* Invert the sense of destination IP address */
#define	NETCONF_INV_SRCPT	0x04	/* Invert the sense of source port range */
#define	NETCONF_INV_DSTPT	0x08	/* Invert the sense of destination port range */
#define NETCONF_INV_MAC		0x10	/* Invert the sense of source MAC address */
#define NETCONF_INV_IN		0x20	/* Invert the sense of inbound interface */
#define NETCONF_INV_OUT		0x40	/* Invert the sense of outbound interface */
#define NETCONF_INV_STATE	0x80	/* Invert the sense of state */
#define NETCONF_INV_DAYS	0x100	/* Invert the sense of day of the week */
#define NETCONF_INV_SECS	0x200	/* Invert the sense of time of day */

/* Entry is disabled */
#define NETCONF_DISABLED	0x80000000

/* Match description */
typedef struct _netconf_match_t {
	int ipproto;			/* IP protocol (TCP/UDP) */
	struct {
		struct in_addr ipaddr;	/* Match by IP address */
		struct in_addr netmask;
		uint16 ports[2];	/* Match by TCP/UDP port range */
	} src, dst;
	struct ether_addr mac;		/* Match by source MAC address */
	struct {
		char name[IFNAMSIZ];	/* Match by interface name */
	} in, out;
	int state;			/* Match by packet state */
	int flags;			/* Match flags */
	uint days[2];			/* Match by day of the week (local time) (0 == Sunday) */
	uint secs[2];			/* Match by time of day (local time) (0 == 12:00 AM) */

//2008.10 magic{
        /* +++ Match by module name. Cherry Cho added in 2007/12/27. +++ */
#ifdef WEBSTRFILTER
        char module_name[30];
        struct netconf_webstr_info {
    char string[256];
    u_int16_t invert;
    u_int16_t len;
    u_int8_t type;
        } webstr_info;
        /* --- Match by module name. Cherry Cho added in 2007/12/27. --- */
#endif
//2008.10 magic}

	struct _netconf_match_t *next, *prev;
} netconf_match_t;

//2008.10 magic{
#ifdef WEBSTRFILTER
enum netconf_webstr_type/* Cherry Cho added in 2007/12/28. */
{
    NETCONF_WEBSTR_HOST,
    NETCONF_WEBSTR_URL,
    NETCONF_WEBSTR_CONTENT
};
#endif
//2008.10 magic}

#define netconf_valid_ipproto(ipproto) \
	 ((ipproto == 0) || (ipproto) == IPPROTO_IGMP || (ipproto) == IPPROTO_TCP || (ipproto) == IPPROTO_UDP)	// oleg patch
	//((ipproto == 0) || (ipproto) == IPPROTO_TCP || (ipproto) == IPPROTO_UDP)

/* Supported firewall target types */
enum netconf_target {
	NETCONF_DROP,			/* Drop packet (filter) */
	NETCONF_ACCEPT,			/* Accept packet (filter) */
	NETCONF_LOG_DROP,		/* Log and drop packet (filter) */
	NETCONF_LOG_ACCEPT,		/* Log and accept packet (filter) */
	NETCONF_SNAT,			/* Source NAT (nat) */
	NETCONF_DNAT,			/* Destination NAT (nat) */
	NETCONF_MASQ,			/* IP masquerade (nat) */
	NETCONF_APP,			/* Application specific port forward (app) */
	NETCONF_TARGET_MAX
};

#define netconf_valid_filter(target) \
	((target) == NETCONF_DROP || (target) == NETCONF_ACCEPT || \
	 (target) == NETCONF_LOG_DROP || (target) == NETCONF_LOG_ACCEPT)

#define netconf_valid_nat(target) \
	((target) == NETCONF_SNAT || (target) == NETCONF_DNAT || (target) == NETCONF_MASQ)

#define netconf_valid_target(target) \
	((target) >= 0 && (target) < NETCONF_TARGET_MAX)

#define NETCONF_FW_COMMON \
	netconf_match_t match;		/* Match type */ \
	enum netconf_target target;	/* Target type */ \
	char desc[40];			/* String description */ \
	struct _netconf_fw_t *next, *prev \

/* Generic firewall entry description */
typedef struct _netconf_fw_t {
	NETCONF_FW_COMMON;
	char data[0];			/* Target specific */
} netconf_fw_t;

/* Supported filter directions */
enum netconf_dir {
	NETCONF_IN,			/* Packets destined for the firewall */
	NETCONF_FORWARD,		/* Packets routed through the firewall */
	NETCONF_OUT,			/* Packets generated by the firewall */
	NETCONF_DIR_MAX
};

#define netconf_valid_dir(dir) \
	((dir) >= 0 && (dir) < NETCONF_DIR_MAX)

/* Filter target firewall entry description */
typedef struct _netconf_filter_t {
	NETCONF_FW_COMMON;
	enum netconf_dir dir;		/* Direction to filter */
} netconf_filter_t;

/* NAT target firewall entry description */
typedef struct _netconf_nat_t {
	NETCONF_FW_COMMON;
	struct in_addr ipaddr;		/* Address to map packet to */
	uint16 ports[2];		/* Port(s) to map packet to (network order) */
} netconf_nat_t;

/* Application specific port forward description */
typedef struct _netconf_app_t {
	NETCONF_FW_COMMON;
	uint16 proto;			/* Related protocol */
	uint16 dport[2];		/* Related destination port(s) (network order) */
	uint16 to[2];			/* Port(s) to map related destination port to (network order) */
} netconf_app_t;

/* Generic doubly linked list processing macros */
#define netconf_list_init(head) ((head)->next = (head)->prev = (head))

#define netconf_list_empty(head) ((head)->next == (head))

#define netconf_list_add(new, head) do { \
	(head)->next->prev = (new); \
	(new)->next = (head)->next; \
	(new)->prev = (head); \
	(head)->next = (new); \
} while (0)

#define netconf_list_del(old) do { \
	(old)->next->prev = (old)->prev; \
	(old)->prev->next = (old)->next; \
} while (0)

#define netconf_list_for_each(pos, head) \
	for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

#define netconf_list_free(head) do { \
	typeof (head) pos, next; \
	for ((pos) = (head)->next; (pos) != (head); (pos) = next) { \
		next = pos->next; \
		netconf_list_del(pos); \
		free(pos); \
	} \
} while (0)


#endif /* _netconf_h_ */
