/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  ARM version.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <bits/arm_asm.h>

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

.global setjmp
.type setjmp,%function
.align 2
#if defined(THUMB1_ONLY)
.thumb_func
setjmp:
	mov	r1, #1
#ifdef __PIC__
	ldr	r3, .L_GOT
	adr	r2, .L_GOT
	add	r3, r2, r3

	ldr	r2, .L_GOT+4	/* __sigsetjmp */
	ldr	r2, [r2, r3]
	bx	r2

	.align 2
.L_GOT:
	.word	_GLOBAL_OFFSET_TABLE_-.L_GOT
	.word	__sigsetjmp(GOT)
#else
	ldr	r2, =__sigsetjmp
	bx	r2
.pool
#endif
#else
setjmp:
	mov	r1, #1
#ifdef __PIC__
	b	__sigsetjmp(PLT)
#else
	b	__sigsetjmp
#endif
#endif

.size setjmp,.-setjmp
