/*
 * libc/sysdeps/linux/microblaze/setjmp.S -- `setjmp' for microblaze
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Miles Bader <miles@gnu.org>
 */

#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

#include <clinkage.h>

	.text
C_ENTRY(setjmp):
	addi	r6, r0, 1			/* Save the signal mask.  */
	braid	C_SYMBOL_NAME(__sigsetjmp)
	nop

	.globl C_SYMBOL_NAME(_setjmp)
C_SYMBOL_NAME(_setjmp):
	add	r6, r0, r0			/* Don't save the signal mask.  */

	.globl C_SYMBOL_NAME(__sigsetjmp)
C_SYMBOL_NAME(__sigsetjmp):
	/* Save registers relative to r5 (arg0)*/
	swi	r1, r5, 0			/* stack pointer */
	swi	r15, r5, 4			/* link register */
	swi	r18, r5, 8			/* assembler temp */
	swi	r19, r5, 12			/* now call-preserved regs */
	swi	r20, r5, 16
	swi	r21, r5, 20
	swi	r22, r5, 24
	swi	r23, r5, 28
	swi	r24, r5, 32
	swi	r25, r5, 36
	swi	r26, r5, 40
	swi	r27, r5, 44
	swi	r28, r5, 48
	swi	r29, r5, 52
	swi	r30, r5, 56

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	braid	C_SYMBOL_NAME(__sigjmp_save)
	nop
C_END(setjmp)
