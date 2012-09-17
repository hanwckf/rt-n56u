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
/* iptables module for matching the IPv4 DSCP field
 *
 * (C) 2002 Harald Welte <laforge@gnumonks.org>
 * This software is distributed under GNU GPL v2, 1991
 * 
 * See RFC2474 for a description of the DSCP field within the IP Header.
 *
 * Id: ipt_dscp.h,v 1.3 2002/08/05 19:00:21 laforge Exp
*/
#ifndef _IPT_DSCP_H
#define _IPT_DSCP_H

#define IPT_DSCP_MASK	0xfc	/* 11111100 */
#define IPT_DSCP_SHIFT	2
#define IPT_DSCP_MAX	0x3f	/* 00111111 */

/* match info */
struct ipt_dscp_info {
	u_int8_t dscp;
	u_int8_t invert;
};

#endif /* _IPT_DSCP_H */
