/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

	.global	__longjmp
	.type	__longjmp,"function"
	.align	1
__longjmp:
	ldm	r12++, r0-r8,sp,lr
	mustr	r8		/* restore status register (lower half) */
	cp	r11, 0		/* can't return zero */
	frs
	moveq	r11, 1
	retal	r11
	.size	__longjmp, . - __longjmp

libc_hidden_def(__longjmp)
