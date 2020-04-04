/* longjmp for or1k

   Based on:
   longjmp for PowerPC.
   Copyright (C) 2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <jmpbuf-offsets.h>

#ifdef __UCLIBC_HAS_FLOATS__
#define FP(x...) x
#else
#define FP(x...)
#endif

.globl     __longjmp;
.type      __longjmp,@function;
.align     4;

__longjmp:
	l.lwz   r1,0(r3)
	l.lwz   r2,4(r3)

	/* if r4 is 0, something wrong, so set it to 1 */
	l.sfeqi r4, 0x0
	l.bnf   1f /* r4 != 0, longjmp value sensible */
	 l.nop
	l.ori   r4, r0, 0x1 /* make nonzero */
1:
	l.addi	r11, r4, 0
	l.lwz   r9,8(r3)
	l.lwz   r10,12(r3)
	l.lwz   r14,16(r3)
	l.lwz   r16,20(r3)
	l.lwz   r18,24(r3)
	l.lwz   r20,28(r3)
	l.lwz   r22,32(r3)
	l.lwz   r24,36(r3)
	l.lwz   r26,40(r3)
	l.lwz   r28,44(r3)
	l.lwz   r30,48(r3)
	l.jr	r9
	 l.nop
.size     __longjmp,.-__longjmp

libc_hidden_def(__longjmp)
