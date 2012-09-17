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
#ifndef _SURFBOARD_H
#define _SURFBOARD_H

#include <asm/addrspace.h>



/*
 * Surfboard system clock.
 * This is the default value and maybe overidden by System Clock passed on the
 * command line (sysclk=).
 */
#define SURFBOARD_SYSTEM_CLOCK		(125000000)

/*
 * Surfboard UART base baud rate = System Clock / 16.
 * Ex. (14.7456 MHZ / 16) = 921600
 *     (32.0000 MHZ / 16) = 2000000
 */
#define SURFBOARD_BAUD_DIV	(16)
#define SURFBOARD_BASE_BAUD	(SURFBOARD_SYSTEM_CLOCK / SURFBOARD_BAUD_DIV)

/*
 * Maximum number of IDE Controllers
 * Surfboard only has one ide (ide0), so only 2 drives are
 * possible.  (no need to check for more hwifs.)
 */
//#define MAX_IDE_HWIFS		(1)	/* Surfboard/Wakeboard */
#define MAX_IDE_HWIFS		(2)	/* Graphite board */


#endif /* !(_SURFBOARD_H) */
