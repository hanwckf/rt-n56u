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
/* iptables module for matching the ECN header in IPv4 and TCP header
 *
 * (C) 2002 Harald Welte <laforge@gnumonks.org>
 *
 * This software is distributed under GNU GPL v2, 1991
 * 
 * Id: ipt_ecn.h,v 1.4 2002/08/05 19:39:00 laforge Exp
*/
#ifndef _IPT_ECN_H
#define _IPT_ECN_H
#include <linux/netfilter_ipv4/ipt_dscp.h>

#define IPT_ECN_IP_MASK	(~IPT_DSCP_MASK)

#define IPT_ECN_OP_MATCH_IP	0x01
#define IPT_ECN_OP_MATCH_ECE	0x10
#define IPT_ECN_OP_MATCH_CWR	0x20

#define IPT_ECN_OP_MATCH_MASK	0xce

/* match info */
struct ipt_ecn_info {
	u_int8_t operation;
	u_int8_t invert;
	u_int8_t ip_ect;
	union {
		struct {
			u_int8_t ect;
		} tcp;
	} proto;
};

#endif /* _IPT_ECN_H */
