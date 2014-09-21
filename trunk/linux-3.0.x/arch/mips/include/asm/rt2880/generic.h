/*
 * Copyright (C) 2001 Palmchip Corporation.  All rights reserved.
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Defines of the Palmchip boards specific address-MAP, registers, etc.
 */
#ifndef __ASM_MACH_MIPS_RT2880_GENERIC_H
#define __ASM_MACH_MIPS_RT2880_GENERIC_H

#include <asm/addrspace.h>
#include <asm/byteorder.h>
#include <asm/rt2880/rt_mmap.h>

/*
 * Reset register.
 */
#define SOFTRES_REG       (KSEG1ADDR(RALINK_SYSCTL_BASE+0x34))
#define GORESET           (0x1)

/*
 * Power-off register
 */
#define POWER_DIR_REG     (KSEG1ADDR(RALINK_PIO_BASE+0x24))
#define POWER_DIR_OUTPUT  (0x80)	/* GPIO 7 */
#define POWER_POL_REG     (KSEG1ADDR(RALINK_PIO_BASE+0x28))
#define POWEROFF_REG      (KSEG1ADDR(RALINK_PIO_BASE+0x20))
#define POWEROFF          (0x0)		/* drive low */


#endif
