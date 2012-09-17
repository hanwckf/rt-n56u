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
#ifndef _IP6T_ESP_H
#define _IP6T_ESP_H

struct ip6t_esp
{
	u_int32_t spis[2];			/* Security Parameter Index */
	u_int8_t  invflags;			/* Inverse flags */
};

#define MASK_HOPOPTS    128
#define MASK_DSTOPTS    64
#define MASK_ROUTING    32
#define MASK_FRAGMENT   16
#define MASK_AH         8
#define MASK_ESP        4
#define MASK_NONE       2
#define MASK_PROTO      1

/* Values for "invflags" field in struct ip6t_esp. */
#define IP6T_ESP_INV_SPI		0x01	/* Invert the sense of spi. */
#define IP6T_ESP_INV_MASK	0x01	/* All possible flags. */

#endif /*_IP6T_ESP_H*/
