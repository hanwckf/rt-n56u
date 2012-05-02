/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005 Embedded Alley Solutions, Inc
 * Copyright (C) 2005 Ralf Baechle (ralf@linux-mips.org)
 */
#ifndef __ASM_MACH_GENERIC_KERNEL_ENTRY_H
#define __ASM_MACH_GENERIC_KERNEL_ENTRY_H

/* Intentionally empty macro, used in head.S. Override in
 * arch/mips/mach-xxx/kernel-entry-init.h when necessary.
 */
.macro	kernel_entry_setup
#if 0
/* FIXME */

	# Initialize the register file
	# should not be required with good software practices
	or	$1,$0, $0
	or	$2,$0, $0
	or	$3,$0, $0
	or	$4,$0, $0
	or	$5,$0, $0
	or	$6,$0, $0
	or	$7,$0, $0
	or	$8,$0, $0
	or	$9,$0, $0
	or	$10,$0, $0
	or	$11,$0, $0
	or	$12,$0, $0
	or	$13,$0, $0
	or	$14,$0, $0
	or	$15,$0, $0
	or	$16,$0, $0
	or	$17,$0, $0
	or	$18,$0, $0
	or	$19,$0, $0
	or	$20,$0, $0
	or	$21,$0, $0
	or	$22,$0, $0
	or	$23,$0, $0
	or	$24,$0, $0
	or	$25,$0, $0
	or	$26,$0, $0
	or	$27,$0, $0
	or	$28,$0, $0
	or	$29,$0, $0
	or	$30,$0, $0
	or	$31,$0, $0

# Initialize Misc. Cop0 state	

	# Read status register
	mfc0	$10, $12
	# Set up Status register:
	# Disable Coprocessor Usable bits
	# Turn off Reduce Power bit
	# Turn off reverse endian
	# Turn off BEV (use normal exception vectors)
	# Clear TS, SR, NMI bits
	# Clear Interrupt masks
	# Clear User Mode
	# Clear ERL
	# Set EXL
	# Clear Interrupt Enable
	li	$11, 0x0000ff02
	mtc0	$11, $12


	# Disable watch exceptions
	mtc0	$0, $18

	# Clear Watch Status bits
	li	$11, 0x3
	mtc0	$11, $19

	# Clear WP bit to avoid watch exception upon user code entry
	# Clear IV bit - Interrupts go to general exception vector
	# Clear software interrupts
	mtc0	$0, $13

	# Set KSeg0 to cacheable
	# Config.K0
	mfc0	$10, $16
	li	$11, 0x7
	not	$11
	and	$10, $11
	or	$10, 0x3
	mtc0	$10, $16

	# Clear Count register
	mtc0	$0, $9

	# Set compare to -1 to delay 1st count=compare
	# Also, clears timer interrupt
	li	$10, -1
	mtc0	$10, $11


	# Cache initialization routine
	# Long and needed on HW 
	# Can be skipped if using magic simulation cache flush
	
#endif
.endm

/*
 * Do SMP slave processor setup necessary before we can savely execute C code.
 */
	.macro	smp_slave_setup
	.endm


#endif /* __ASM_MACH_GENERIC_KERNEL_ENTRY_H */
