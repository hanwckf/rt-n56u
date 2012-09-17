/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  PowerPC version.
   Copyright (C) 1994, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include "ppc_asm.h"

.globl      __setjmp;
.type      __setjmp, @function;
.align  2;

__setjmp:
	li r4,1			/* Set second argument to 1.  */

	b __sigsetjmp@local

.size     __setjmp,.-__setjmp

.globl     setjmp;
.set    setjmp,__setjmp
