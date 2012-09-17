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
 *
 * PIC code based on glibc 2.3.6 */

/*
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

#include <libc-symbols.h>

	.text
	.globl C_SYMBOL_NAME(setjmp)
	.align 4
C_SYMBOL_NAME(setjmp):
#ifdef __PIC__
	brid	1f
#else
	braid	C_SYMBOL_NAME(__sigsetjmp)
#endif
	addi	r6, r0, 1			/* Save the signal mask.  */

	.globl C_SYMBOL_NAME(_setjmp)
C_SYMBOL_NAME(_setjmp):
	and	r6, r0, r0			/* Don't save the signal mask.  */

	.globl C_SYMBOL_NAME(__sigsetjmp)
C_SYMBOL_NAME(__sigsetjmp):
1:
	/* Save registers relative to r5 (arg0)*/
	swi	r1, r5, 0			/* stack pointer */
	swi	r15, r5, 4			/* link register */
	swi	r2,  r5, 8			/* SDA and SDA2 ptrs */
	swi	r13, r5, 12
	swi	r18, r5, 16			/* assembler temp */
	swi	r19, r5, 20			/* now call-preserved regs */
	swi	r20, r5, 24
	swi	r21, r5, 28
	swi	r22, r5, 32
	swi	r23, r5, 36
	swi	r24, r5, 40
	swi	r25, r5, 44
	swi	r26, r5, 48
	swi	r27, r5, 52
	swi	r28, r5, 56
	swi	r29, r5, 60
	swi	r30, r5, 64
	swi	r31, r5, 68

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef __PIC__
	mfs   r12,rpc
	addik r12,r12,_GLOBAL_OFFSET_TABLE_+8
	lwi   r12,r12,__sigjmp_save@GOT
	brad  r12
	nop
#else
	braid	C_SYMBOL_NAME(__sigjmp_save)
	nop
#endif
