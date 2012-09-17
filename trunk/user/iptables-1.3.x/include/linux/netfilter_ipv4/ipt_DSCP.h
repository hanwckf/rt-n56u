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
/* iptables module for setting the IPv4 DSCP field
 *
 * (C) 2002 Harald Welte <laforge@gnumonks.org>
 * based on ipt_FTOS.c (C) 2000 by Matthew G. Marsh <mgm@paktronix.com>
 * This software is distributed under GNU GPL v2, 1991
 * 
 * See RFC2474 for a description of the DSCP field within the IP Header.
 *
 * Id: ipt_DSCP.h,v 1.7 2002/03/14 12:03:13 laforge Exp
*/
#ifndef _IPT_DSCP_TARGET_H
#define _IPT_DSCP_TARGET_H
#include <linux/netfilter_ipv4/ipt_dscp.h>

/* target info */
struct ipt_DSCP_info {
	u_int8_t dscp;
};

#endif /* _IPT_DSCP_TARGET_H */
