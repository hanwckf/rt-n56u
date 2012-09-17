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
#include <features.h>
#include <bits/errno.h>
#include <sys/syscall.h>
#include <bits/arm_asm.h>

#if defined(__NR_clone)
/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

.text
.global __clone
.type __clone,%function
.align 2
#if defined(THUMB1_ONLY)
.thumb_func
__clone:
	@ sanity check args
	cmp	r0, #0
	beq	__einval
	cmp	r1, #0
	beq	__einval

	@ insert the args onto the new stack
	sub	r1, r1, #8
	str	r3, [r1, #4]
	@ save the function pointer as the 0th element
	str	r0, [r1]

	@ do the system call
	@ get flags
	mov	r0, r2
	@ new sp is already in r1
	@ load remaining arguments off the stack
	stmfd	sp!, {r4}
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #8]
	ldr	r4, [sp, #12]
	DO_CALL (clone)
	movs	a1, a1
	blt	__error
	ldmnefd sp!, {r4}
	beq	1f
	bx	lr
1:

	@ pick the function arg and call address off the stack and execute
	ldr	r0, [sp, #4]
	ldr	r1, [sp]
	bl	2f	@ blx r1

	@ and we are done, passing the return value through r0
	bl	HIDDEN_JUMPTARGET(_exit)
	@ Should never return
	b	.

2:
	bx	r1

__einval:
	ldr	r0, =-EINVAL
__error:
	push	{r3, lr}
	bl	__syscall_error
	POP_RET
.pool
#else
__clone:
	@ sanity check args
	cmp	r0, #0
	IT(te, ne)
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
	@ load remaining arguments off the stack
	stmfd	sp!, {r4}
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #8]
	ldr	r4, [sp, #12]
	DO_CALL (clone)
	movs	a1, a1
	blt	__error
	ldmnefd	sp!, {r4}
	IT(t, ne)
#if defined(__USE_BX__)
	bxne	lr
#else
	movne	pc, lr
#endif

	@ pick the function arg and call address off the stack and execute
	ldr	r0, [sp, #4]
	mov	lr, pc
	ldr 	pc, [sp]

	@ and we are done, passing the return value through r0
	b	HIDDEN_JUMPTARGET(_exit)

__error:
	b	__syscall_error
#endif

.size __clone,.-__clone
weak_alias(__clone, clone)

#endif
