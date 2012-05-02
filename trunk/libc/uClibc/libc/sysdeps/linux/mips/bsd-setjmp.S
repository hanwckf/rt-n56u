/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  MIPS version.
   Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sys/regdef.h>

#ifdef __PIC__
	.option pic2
#endif

.text
.global setjmp
.align 2;
.ent setjmp,0;
.type setjmp,@function

setjmp:
	.set	noreorder
#ifdef __PIC__
	.cpload t9
	.set	reorder
	la	t9, __sigsetjmp
#endif
	li	a1, 1		/* Pass a second argument of one.  */
#ifdef __PIC__
	jr	t9
#else
	j	__sigsetjmp
#endif
	.end	setjmp
