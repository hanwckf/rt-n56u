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
#ifndef _XT_TCPUDP_H
#define _XT_TCPUDP_H

/* TCP matching stuff */
struct xt_tcp
{
	u_int16_t spts[2];			/* Source port range. */
	u_int16_t dpts[2];			/* Destination port range. */
	u_int8_t option;			/* TCP Option iff non-zero*/
	u_int8_t flg_mask;			/* TCP flags mask byte */
	u_int8_t flg_cmp;			/* TCP flags compare byte */
	u_int8_t invflags;			/* Inverse flags */
};

/* Values for "inv" field in struct ipt_tcp. */
#define XT_TCP_INV_SRCPT	0x01	/* Invert the sense of source ports. */
#define XT_TCP_INV_DSTPT	0x02	/* Invert the sense of dest ports. */
#define XT_TCP_INV_FLAGS	0x04	/* Invert the sense of TCP flags. */
#define XT_TCP_INV_OPTION	0x08	/* Invert the sense of option test. */
#define XT_TCP_INV_MASK		0x0F	/* All possible flags. */

/* UDP matching stuff */
struct xt_udp
{
	u_int16_t spts[2];			/* Source port range. */
	u_int16_t dpts[2];			/* Destination port range. */
	u_int8_t invflags;			/* Inverse flags */
};

/* Values for "invflags" field in struct ipt_udp. */
#define XT_UDP_INV_SRCPT	0x01	/* Invert the sense of source ports. */
#define XT_UDP_INV_DSTPT	0x02	/* Invert the sense of dest ports. */
#define XT_UDP_INV_MASK	0x03	/* All possible flags. */


#endif
