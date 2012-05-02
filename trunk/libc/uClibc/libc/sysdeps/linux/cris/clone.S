/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <asm/errno.h>
#include <sys/syscall.h>
#include <sysdep.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

	.syntax no_register_prefix

        .text
ENTRY (__clone)
	/* Sanity check arguments: No NULL function pointers.  Allow a NULL
	   stack pointer though; it makes the kernel allocate stack.  */
	test.d	r10
	beq	1f
	nop

	/* We need to muck with a few registers.  */
	movem	r1,[sp=sp-8]

	/* Save the function pointer and argument.  We can't save them
	   onto the new stack since it can be NULL.  */
	move.d	r10,r0
	move.d	r13,r1

        /* Move the other arguments into place for the system call.  */
	move.d	r11,r10
	move.d	r12,r11

	/* Do the system call.  */
	movu.w	SYS_ify (clone),r9
	break	13
	test.d	r10
	beq	.Lthread_start
	nop

	/* Jump to error handler if we get (unsigned) -4096 .. 0xffffffff.  */
	cmps.w	-4096,r10
	bhs	0f
	movem	[sp+],r1

	/* In parent, successful return.  (Avoid using "ret" - it's a macro.)  */
	Ret
	nop

.Lthread_start:
	/* Terminate frame pointers here.  */
	moveq	0,r8

	/* I've told you once.  */
	move.d	r1,r10
	jsr	r0

	SETUP_PIC
	PLTCALL	(_exit)

	/* Die horribly.  */
	test.d	[6809]

	/* Stop the unstoppable.  */
9:
	ba	9b
	nop

/* Local error handler.  */
1:
	movs.w	-EINVAL,r10
	/* Drop through into the ordinary error handler.  */
PSEUDO_END (__clone)

.globl clone
	clone = __clone
