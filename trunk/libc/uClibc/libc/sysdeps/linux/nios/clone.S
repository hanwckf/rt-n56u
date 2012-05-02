/* Copyright (C) 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu).

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <asm/errno.h>
#include <asm/unistd.h>
#include "NM_Macros.S"

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

	.text
	.align	2
	.globl	__clone
	.type	__clone,@function

__clone:
	save	%sp,-16

	MOVIP	%l0, -EINVAL
	/* sanity check arguments */
	skprnz	%i0				/* no NULL function pointers */
	 br	CLONE_ERROR_LABEL
	mov	%o0, %i2

	skprnz  %i1				/* no NULL stack pointers */
	 br	CLONE_ERROR_LABEL
	mov	%o1, %i1

	/* Do the system call */
	MOVIP	%g1, __NR_clone
	trap	63

	/* if ret >=0? */
	cmpi %o0, 0
	skps cc_pl
	 br CLONE_ERROR_LABEL
	 mov %l0, %o0

	/* Start thread */
	skprz  %o1
	 br __thread_start
	 nop
	mov %i0, %o0
	ret
	 restore

CLONE_ERROR_LABEL:
	neg %l0
	MOVIA	%g1, __errno_location@h
	call	%g1
	 nop
	st	[%o0], %l0		/* store errno */

	xor %i0, %i0
	dec %i0				/* retval=-1 */
	ret
	 restore

	.size	__clone, .-__clone

	.type	__thread_start,@function

__thread_start:
	call	%i0
	 mov	%o0, %i3
	MOVIA	%g1, _exit@h
	call	%g1
	 nop

	.size	__thread_start, .-__thread_start

.globl	clone;
    clone = __clone
