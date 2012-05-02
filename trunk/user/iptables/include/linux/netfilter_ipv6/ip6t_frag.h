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
#ifndef _IP6T_FRAG_H
#define _IP6T_FRAG_H

struct ip6t_frag
{
	u_int32_t ids[2];			/* Security Parameter Index */
	u_int32_t hdrlen;			/* Header Length */
	u_int8_t  flags;			/*  */
	u_int8_t  invflags;			/* Inverse flags */
};

#define IP6T_FRAG_IDS 		0x01
#define IP6T_FRAG_LEN 		0x02
#define IP6T_FRAG_RES 		0x04
#define IP6T_FRAG_FST 		0x08
#define IP6T_FRAG_MF  		0x10
#define IP6T_FRAG_NMF  		0x20

/* Values for "invflags" field in struct ip6t_frag. */
#define IP6T_FRAG_INV_IDS	0x01	/* Invert the sense of ids. */
#define IP6T_FRAG_INV_LEN	0x02	/* Invert the sense of length. */
#define IP6T_FRAG_INV_MASK	0x03	/* All possible flags. */

#define MASK_HOPOPTS    128
#define MASK_DSTOPTS    64
#define MASK_ROUTING    32
#define MASK_FRAGMENT   16
#define MASK_AH         8
#define MASK_ESP        4
#define MASK_NONE       2
#define MASK_PROTO      1

#endif /*_IP6T_FRAG_H*/
