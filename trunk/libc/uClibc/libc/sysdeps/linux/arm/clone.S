/* Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Pat Beirne <patb@corelcomputer.com>

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

#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

#ifdef __NR_clone
/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

.text
.global __clone
.type __clone,%function
.align 4
__clone:
	@ sanity check args
	cmp	r0, #0
	cmpne	r1, #0
	moveq	r0, #-EINVAL
	beq	__error

	@ insert the args onto the new stack
	sub	r1, r1, #8
	str	r3, [r1, #4]
	@ save the function pointer as the 0th element
	str	r0, [r1]

	@ do the system call
	@ get flags
	mov	r0, r2
	@ new sp is already in r1
	swi	__NR_clone
	movs	a1, a1
	blt	__error
	movne    pc, lr

	@ pick the function arg and call address off the stack and execute
	ldr	r0, [sp, #4]
	mov	lr, pc
	ldr 	pc, [sp]

	@ and we are done, passing the return value through r0
#ifdef __PIC__
	b	_exit(PLT)
#else
	b	_exit
#endif

__error:
	b	__syscall_error

.size __clone,.-__clone

.weak clone
	clone = __clone

#endif
