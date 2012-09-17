/* setjmp for SH3.
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

#include <features.h>
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

	.text

/* This just does a tail-call to `__sigsetjmp (ARG, 0)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

	.align 4
	.type	_setjmp,@function
	.globl	_setjmp;
_setjmp:
	bra	__sigsetjmp_intern
	 mov	#0, r1
	.size _setjmp,.-_setjmp;

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

	.align 4
	.type	setjmp,@function
	.globl	setjmp;
setjmp:	
	bra	__sigsetjmp_intern
	 mov	#1, r1
	.size setjmp,.-setjmp;
	
	.align 4
	.type	__sigsetjmp,@function
	.globl	__sigsetjmp;
__sigsetjmp:
	mov     r0, r1
	nop		/* align this guy */
__sigsetjmp_intern:
	/* Save registers */
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
	add	#(JB_SIZE), r4
	fmov.s	fr15, @-r4
	fmov.s	fr14, @-r4
	fmov.s	fr13, @-r4
	fmov.s	fr12, @-r4
	sts.l   fpscr, @-r4
#else
	add	#(JB_SIZE-(5*4)), r4 /* this code doesn't do FP yet */
#endif		
	stc.l	gbr, @-r4
	sts.l	pr, @-r4
	mov.l	r15, @-r4
	mov.l	r14, @-r4
	mov.l	r13, @-r4
	mov.l	r12, @-r4
	mov.l	r11, @-r4
	mov.l	r10, @-r4
	mov.l	r9, @-r4
	mov.l	r8, @-r4

#ifdef __HAVE_SHARED__
	mov.l	.LG, r2
	mova	.LG, r0
	add	r0, r2
	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	mov.l	.L1, r0
	mov.l   @(r0,r2),r0
	jmp	@r0
	 mov     r1, r0
	.align	2
.LG:	.long	_GLOBAL_OFFSET_TABLE_
.L1:	.long	__sigjmp_save@GOT
#else
	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	mov.l	.L1, r0
	braf	r0
	 mov     r1, r0
.jmp_loc:		
	.align	2
.L1:	.long	__sigjmp_save - .jmp_loc
#endif	

	.size __sigsetjmp,.-__sigsetjmp;
