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
#ifndef _IP6T_PHYSDEV_H
#define _IP6T_PHYSDEV_H

#ifdef __KERNEL__
#include <linux/if.h>
#endif

#define IP6T_PHYSDEV_OP_IN		0x01
#define IP6T_PHYSDEV_OP_OUT		0x02
#define IP6T_PHYSDEV_OP_BRIDGED		0x04
#define IP6T_PHYSDEV_OP_ISIN		0x08
#define IP6T_PHYSDEV_OP_ISOUT		0x10
#define IP6T_PHYSDEV_OP_MASK		(0x20 - 1)

struct ip6t_physdev_info {
	char physindev[IFNAMSIZ];
	char in_mask[IFNAMSIZ];
	char physoutdev[IFNAMSIZ];
	char out_mask[IFNAMSIZ];
	u_int8_t invert;
	u_int8_t bitmask;
};

#endif /*_IP6T_PHYSDEV_H*/
