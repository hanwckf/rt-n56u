/*
 * Copyright (C) 2001 Palmchip Corporation.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 */
#ifndef __ASM_MACH_MIPS_RT2880_SURFBOARD_H
#define __ASM_MACH_MIPS_RT2880_SURFBOARD_H

/*
 * Surfboard UART base baud rate = System Clock / 16.
 * Ex. (14.7456 MHZ / 16) = 921600
 *     (32.0000 MHZ / 16) = 2000000
 */
#define SURFBOARD_BAUD_DIV	(16)

extern unsigned int get_surfboard_sysclk(void);

#endif
