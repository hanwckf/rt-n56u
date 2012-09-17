/* longjmp for SH.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>
#include <features.h>

/* __longjmp(jmpbuf, val) */

.text
.align 4
.type	__longjmp,@function
.globl	__longjmp;
__longjmp:
	mov.l	@r4+, r8
	mov.l	@r4+, r9
	mov.l	@r4+, r10
	mov.l	@r4+, r11
	mov.l	@r4+, r12
	mov.l	@r4+, r13
	mov.l	@r4+, r14
	mov.l	@r4+, r15
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
	lds.l	@r4+, pr
	ldc.l	@r4+, gbr
	lds.l	@r4+, fpscr
	fmov.s	@r4+, fr12
	fmov.s	@r4+, fr13
	mov	r5, r0		/* get the return value in place */
	tst	r0, r0
	bf.s	1f
	 fmov.s	@r4+, fr14
	mov	#1,r0		/* can't let setjmp() return zero! */
1:
	rts
	 fmov.s	@r4+, fr15
#else
	mov	r5, r0		/* get the return value in place */
	tst	r0, r0
	bf.s	1f
	 lds.l	@r4+, pr
	mov	#1,r0		/* can't let setjmp() return zero! */
1:
	rts
	 ldc.l	@r4+, gbr
#endif		
.size __longjmp,.-__longjmp
libc_hidden_def(__longjmp)
