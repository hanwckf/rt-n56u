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
#ifndef _IPT_IPRANGE_H
#define _IPT_IPRANGE_H

#define IPRANGE_SRC		0x01	/* Match source IP address */
#define IPRANGE_DST		0x02	/* Match destination IP address */
#define IPRANGE_SRC_INV		0x10	/* Negate the condition */
#define IPRANGE_DST_INV		0x20	/* Negate the condition */

struct ipt_iprange {
	/* Inclusive: network order. */
	u_int32_t min_ip, max_ip;
};

struct ipt_iprange_info
{
	struct ipt_iprange src;
	struct ipt_iprange dst;

	/* Flags from above */
	u_int8_t flags;
};

#endif /* _IPT_IPRANGE_H */
