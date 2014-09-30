/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 by Ralf Baechle
 */
#ifndef __ASM_MACH_MIPS_RT2880_IRQ_H
#define __ASM_MACH_MIPS_RT2880_IRQ_H

#include <asm/rt2880/surfboardint.h>

#define GIC_NUM_INTRS	(SURFBOARDGIC_END + 1 + NR_CPUS * 2)
#define NR_IRQS		(SURFBOARDINT_END + 1)

#include_next <irq.h>

#endif
