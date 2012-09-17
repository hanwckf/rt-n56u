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
/* Hop Limit modification module for ip6tables
 * Maciej Soltysiak <solt@dns.toxicfilms.tv>
 * Based on HW's TTL module */

#ifndef _IP6T_HL_H
#define _IP6T_HL_H

enum {
	IP6T_HL_SET = 0,
	IP6T_HL_INC,
	IP6T_HL_DEC
};

#define IP6T_HL_MAXMODE	IP6T_HL_DEC

struct ip6t_HL_info {
	u_int8_t	mode;
	u_int8_t	hop_limit;
};


#endif
