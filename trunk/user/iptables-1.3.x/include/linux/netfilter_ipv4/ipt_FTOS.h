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
/* Set TOS field in header to any value
 *
 * (C) 2000 by Matthew G. Marsh <mgm@paktronix.com>
 *
 * This software is distributed under GNU GPL v2, 1991
 * 
 * ipt_FTOS.h borrowed heavily from ipt_TOS.h  11/09/2000
*/
#ifndef _IPT_FTOS_H
#define _IPT_FTOS_H

struct ipt_FTOS_info {
	u_int8_t ftos;
};

#endif /*_IPT_FTOS_H*/
