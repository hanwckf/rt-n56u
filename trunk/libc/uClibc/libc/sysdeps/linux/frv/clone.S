/* Copyright (C) 2003, 2004 Free Software Foudnation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>, 2003.

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

#include <asm/unistd.h>
#define _ERRNO_H	1
#include <bits/errno.h>

	.text
	.globl	__clone
	.type __clone,@function
/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg) */
__clone:
	/* Sanity check arguments.  */
	cmp.p	gr8, gr0, icc0
	cmp	gr9, gr0, icc1
	mov.p	gr8, gr4
	beq	icc0, #0, .Lerror
	mov.p	gr11, gr5
	beq	icc1, #0, .Lerror

	mov.p	gr10, gr8
	setlos	#__NR_clone, gr7
	tra	gr0,gr0

	cmp.p	gr8, gr0, icc0
	setlos #-4096, gr6
	cmp.p	gr8, gr6, icc1
	beq	icc0, #0, .Lthread_start
	blslr	icc1, #2

.Lsys_error:
	sethi.p	#gotofffuncdeschi(__syscall_error), gr14
	setlo	#gotofffuncdesclo(__syscall_error), gr14
	ldd	@(gr14, gr15), gr14
	jmpl	@(gr14, gr0)

.Lerror:
	setlos.p #-EINVAL, gr8
	bra	.Lsys_error

###############################################################################
#
# come here as the new thread [GR4 is fn, GR5 is arg]
#
###############################################################################
.Lthread_start:
	/* Save the PIC register.  */
	mov	gr15, gr17

	/* Call the user's function.  */
	ldd.p	@(gr4, gr0), gr14
	mov	gr5, gr8
	calll	@(gr14, gr0)

	/* Call _exit, rather simply inlining the syscall, such that
	   breakpoints work.*/

	mov.p	gr17, gr15
	call	_exit

	/* Should never get here.  */
	jmpl	@(gr0, gr0)
	.size	__clone,.-__clone

.weak	clone
	clone = __clone
