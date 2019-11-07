/* setjmp for or1k

   Based on:
   setjmp for PowerPC.
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

#include "spr_defs.h"
#include <jmpbuf-offsets.h>

#ifdef __UCLIBC_HAS_FLOATS__
#define FP(x...) x
#else
#define FP(x...)
#endif

.globl _setjmp
.type _setjmp, @function
.align 2

_setjmp:
	l.addi	r4,r0,0			/* Set second argument to 0.  */
	l.j	.Local_sigsetjmp
	l.nop
.size     _setjmp,.-_setjmp

.globl __setjmp
.type __setjmp, @function
.align 2

__setjmp:
	l.addi	r4,r0,1			/* Set second argument to 1.  */
	l.j	.Local_sigsetjmp
	l.nop
.size __setjmp,.-__setjmp

.globl setjmp
.set setjmp,__setjmp

.globl __sigsetjmp
.type __sigsetjmp, @function
.align 2

__sigsetjmp:
.Local_sigsetjmp:
	l.sw    0(r3),r1
	l.sw    4(r3),r2
	l.sw    8(r3),r9
	l.sw    12(r3),r10
	l.sw    16(r3),r14
	l.sw    20(r3),r16
	l.sw    24(r3),r18
	l.sw    28(r3),r20
	l.sw    32(r3),r22
	l.sw    36(r3),r24
	l.sw    40(r3),r26
	l.sw    44(r3),r28
	l.sw    48(r3),r30
	l.j	__sigjmp_save
	 l.nop
.size __sigsetjmp,.-__sigsetjmp
