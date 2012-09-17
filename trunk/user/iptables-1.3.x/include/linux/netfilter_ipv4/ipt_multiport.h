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
#ifndef _IPT_MULTIPORT_H
#define _IPT_MULTIPORT_H

enum ipt_multiport_flags
{
	IPT_MULTIPORT_SOURCE,
	IPT_MULTIPORT_DESTINATION,
	IPT_MULTIPORT_EITHER
};

#define IPT_MULTI_PORTS	15

/* Must fit inside union ipt_matchinfo: 16 bytes */
struct ipt_multiport
{
	u_int8_t flags;				/* Type of comparison */
	u_int8_t count;				/* Number of ports */
	u_int16_t ports[IPT_MULTI_PORTS];	/* Ports */
};

struct ipt_multiport_v1
{
	u_int8_t flags;				/* Type of comparison */
	u_int8_t count;				/* Number of ports */
	u_int16_t ports[IPT_MULTI_PORTS];	/* Ports */
	u_int8_t pflags[IPT_MULTI_PORTS];	/* Port flags */
	u_int8_t invert;			/* Invert flag */
};
#endif /*_IPT_MULTIPORT_H*/
