/* clone() implementation for Nios II.
   Copyright (C) 2008-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andrew Jenner <andrew@codesourcery.com>, 2008.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

#define CLONE_VM      0x00000100

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     void *parent_tidptr, void *tls, void *child_tidptr) */

	.text
ENTRY(__clone)
	/* Sanity check arguments.  */
	movi	r2, EINVAL

	subi	r5, r5, 8	/* Reserve argument save space.  */
	stw	r4, 4(r5)	/* Save function pointer.  */
	stw	r7, 0(r5)	/* Save argument pointer.  */

	/* Load arguments.  */
	mov	r4, r6
	ldw	r6, 0(sp)
	ldw	r7, 8(sp)
	ldw	r8, 4(sp)

	/* Do the system call.  */
	movi	r2, SYS_ify (clone)

	trap

	/* See if we're on the newly created thread.  */
	beq	r2, zero, thread_start
	/* Successful return from the parent */
	ret

thread_start:
	/* We expect the argument registers to be preserved across system
	   calls and across task cloning, so flags should be in r4 here.  */
	andi	r2, r4, CLONE_VM
	bne	r2, zero, 2f
        DO_CALL (getpid, 0)
2:
	ldw	r5, 4(sp)	/* Function pointer.  */
	ldw	r4, 0(sp)	/* Argument pointer.  */
	addi	sp, sp, 8

        /* Call the user's function.  */
	callr	r5

	/* _exit with the result.  */
	mov	r4, r2
#ifdef __PIC__
	nextpc	r22
1:	movhi	r8, %hiadj(_gp_got - 1b)
	addi	r8, r8, %lo(_gp_got - 1b)
	add	r22, r22, r8
	ldw	r8, %call(HIDDEN_JUMPTARGET(_exit))(r22)
	jmp	r8
#else
	jmpi	_exit
#endif

END(__clone)
libc_hidden_def (__clone)
weak_alias (__clone, clone)
