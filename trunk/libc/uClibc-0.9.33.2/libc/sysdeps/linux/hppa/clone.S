/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Huggins-Daines <dhd@debian.org>, 2000.
   Based on the Alpha version by Richard Henderson <rth@tamu.edu>, 1996.

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
#include <sys/syscall.h>

/* Non-thread code calls __clone with the following parameters:
   int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg)
   
   NPTL Code will call __clone with the following parameters:
   int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     int *parent_tidptr, struct user_desc *newtls, int *child_pidptr)
	
   The code should not mangle the extra input registers.
   Syscall expects:				Input to __clone:
	4(r25) - function pointer 		(r26, arg0) 
	0(r25) - argument			(r23, arg3)
	r26 - clone flags.			(r24, arg2)
	r25+64 - user stack pointer.		(r25, arg1)
	r24 - parent tid pointer.		(stack - 52)
	r23 - struct user_desc newtls pointer.	(stack - 56)
	r22 - child tid pointer.		(stack - 60)
	r20 - clone syscall number		(constant)
 */

.text
.global __clone
.type   __clone,%function
__clone:

	/* Sanity check arguments.  */
	ldi     -EINVAL,%ret0
	comib,=,n  0,%arg0,.Lerror        /* no NULL function pointers */
	comib,=,n  0,%arg1,.Lerror        /* no NULL stack pointers */

	/* Save the fn ptr and arg on the new stack.  */
	stwm    %r26,64(%r25)
	stw	%r23,-60(%r25)
	/* Clone arguments are (int flags, void * child_stack) */
	copy	%r24,%r26	/* flags are first */
	/* User stack pointer is in the correct register already */

	/* Load args from stack... */
	ldw	-52(%sp), %r24	/* Load parent_tidptr */
	ldw	-56(%sp), %r23 	/* Load newtls */
	ldw	-60(%sp), %r22	/* Load child_tidptr */

	/* Create frame to get r3 free */
	copy	%sp, %r21
	stwm	%r3, 64(%sp)
	stw	%r21, -4(%sp)

	/* Save the PIC register. */
#ifdef __PIC__
	copy	%r19, %r3		/* parent */
#endif

	/* Do the system call */
	ble     0x100(%sr2,%r0)
	ldi	__NR_clone,%r20

	ldi	-4096,%r1
	comclr,>>= %r1,%ret0,%r0	/* Note: unsigned compare. */
	b,n	.LerrorRest

	comib,=,n 0,%ret0,thread_start

	/* Successful return from the parent
	   No need to restore the PIC register, 
	   since we return immediately. */

	bv	%r0(%rp)
	ldwm	-64(%sp), %r3

.LerrorRest:
	/* Restore the PIC register on error */
#ifdef __PIC__
	copy	%r3, %r19		/* parent */ 
#endif

	/* Something bad happened -- no child created */
.Lerror:

	/* Set errno, save ret0 so we return with that value. */
	copy	%ret0, %r3
	b	__syscall_error
	sub     %r0,%ret0,%arg0
	copy	%r3, %ret0
	/* Return after setting errno, and restoring ret0 */
	bv	%r0(%rp)
	ldwm	-64(%sp), %r3

thread_start:

	/* Load up the arguments.  */
	ldw	-60(%sr0, %sp),%arg0
	ldw     -64(%sr0, %sp),%r22

	/* $$dyncall fixes childs PIC register */

	/* Call the user's function */
	bl	$$dyncall,%r31
	copy	%r31,%rp

	bl	HIDDEN_JUMPTARGET(_exit),%rp
	copy	%ret0,%arg0

	/* Die horribly.  */
	iitlbp	%r0,(%sr0,%r0)

.size clone,.-clone

weak_alias (__clone, clone)
