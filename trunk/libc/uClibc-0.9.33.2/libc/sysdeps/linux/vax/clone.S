/* Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sys/syscall.h>

#warning "This file contains a hardcoded constant for SYS_clone"

.section	.rodata
		.align 2
.LC0:		.long 120	/* SYS_clone */
.align 4
.text
.type	clone,@function
.globl	clone;
clone:
	.word 0x0040
	/* subl2	$8, %sp	*/
	movl	4(%ap), %r1
	movl	8(%ap), %r0
	mcoml	$21, %r6

	/* Sanity check args.  */
	tstl	%r1
	jeql	CLONE_ERROR_LABEL
	tstl	%r0
	jeql	CLONE_ERROR_LABEL

	/* Need to setup the child stack the same as the parent.  */
	subl2	$24, %r0
	movl	16(%ap), 20(%r0)
	movl	%r1, 16(%r0)
	movl	%r0, %r1
	addl2	$16, %r1
	movl	%r1, 12(%r0)

	/* Do the system call.  */
	pushl	%ap
	pushl	%r0
        pushl	12(%ap)
	pushl	$0x2
	movl	%sp, %ap
        chmk	.LC0	/* %r0 .LC0 -4(%fp) -8(%fp) */
	addl2	$12, %sp
	movl	(%sp)+, %ap
	movl	%r0, %r6
	jneq	CLONE_ERROR_LABEL

	movl	$0, %fp
	pushl	4(%ap)
	movl	(%r1), %r0
	calls	$1, (%r0)
	pushl	%r0
	calls	$1, HIDDEN_JUMPTARGET(_exit)

CLONE_ERROR_LABEL:
	cmpl	%r6, $-126	/* -ENOKEY?!?! Fuck, this must be wrong! FIXME */
	jlequ	CLONE_RETURN_LABEL
	calls	$0, __errno_location
	mnegl	%r6, (%r0)
	mcoml	$0, %r6
	movl	%r6, %r0
	ret

CLONE_RETURN_LABEL:
	ret

.size clone,.-clone

