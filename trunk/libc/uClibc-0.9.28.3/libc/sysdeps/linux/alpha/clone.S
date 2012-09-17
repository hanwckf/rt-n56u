/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>, 1996.

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

#include <features.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <sys/syscall.h>
#include <asm/regdef.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg) */

.text
.globl  __clone;
.align 3;
.ent  __clone , 0;

__clone:
	.frame $30 , 0, $26
	.prologue 0

	/* Sanity check arguments.  */
	ldiq	v0,EINVAL
	beq	a0,$error		/* no NULL function pointers */
	beq	a1,$error		/* no NULL stack pointers */

	/* Save the fn ptr and arg on the new stack.  */
	subq	a1,16,a1
	stq	a0,0(a1)
	stq	a3,8(a1)

	/* Do the system call */
	mov	a2,a0
	ldiq	v0,__NR_clone
	call_pal 131

	bne	a3,$error
	beq	v0,thread_start

	/* Successful return from the parent */
	ret

	/* Something bad happened -- no child created */
$error:
	br	gp,1f
1:	ldgp	gp,0(gp)
	jmp	zero,__syscall_error

.end __clone

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.  */

	.ent thread_start
thread_start:
	.frame fp,0,zero,0
	mov	zero,fp
	.prologue 0

	/* Load up the arguments.  */
	ldq	pv,0($30)
	ldq	a0,8($30)
	addq	$30,16,$30

	/* Call the user's function */
	jsr	ra,(pv)
	ldgp	gp,0(ra)

	/* Call _exit rather than doing it inline for breakpoint purposes */
	mov	v0,a0
	jsr	ra,_exit

	/* Die horribly.  */
	halt

	.end thread_start

.weak    clone;
    clone    =   __clone

