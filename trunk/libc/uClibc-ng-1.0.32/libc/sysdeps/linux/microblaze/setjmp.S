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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

ENTRY(_setjmp)
	addik   r6, r0, 0
END(_setjmp)
libc_hidden_def(_setjmp)

ENTRY(setjmp)
	brid    1f
	addik   r6, r0, 1
END(setjmp)
libc_hidden_def(setjmp)

ENTRY(__sigsetjmp)
1:
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

	bri __sigjmp_save

END(__sigsetjmp)
libc_hidden_def(__sigsetjmp)
