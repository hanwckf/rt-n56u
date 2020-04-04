/* Copyright (C) 2001, 2005 Free Software Foundation, Inc.

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
   see <http://www.gnu.org/licenses/>.  */

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <features.h>
#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#define __ASSEMBLY__

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
                    a2                    a3               a4        a5
             pid_t *ptid, struct user_desc *tls, pid_t *ctid)
                   a6               a7              16(sp)
*/

        .text
ENTRY (__clone)

	/* Sanity check arguments.  */
	beqz	a2, .Leinval	/* no NULL function pointers */
	beqz	a3, .Leinval	/* no NULL stack pointers */

	/* a2 and a3 are candidates for destruction by system-call return
	   parameters.  We don't need the stack pointer after the system
	   call.  We trust that the kernel will preserve a6, a7 and a9. */

	mov	a9, a5			/* save function argument */
	mov	a5, a7
	mov	a7, a2			/* save function pointer */
	mov	a8, a6			/* use a8 as a temp */
	mov	a6, a4
	mov	a4, a8
	l32i	a8, a1, FRAMESIZE	/* child_tid */
	movi	a2, SYS_ify(clone)

	/* syscall(NR_clone,clone_flags, usp, parent_tid, child_tls, child_tid)
                     a2         a6        a3        a4        a5         a8
         */

	syscall
	bltz	a2, SYSCALL_ERROR_LABEL
	beqz	a2, .Lthread_start

	/* fall through for parent */

.Lpseudo_end:
	abi_ret

.Leinval:
	movi	a2, -EINVAL
	j	SYSCALL_ERROR_LABEL

.Lthread_start:

	/* start child thread */
	movi	a0, 0			/* terminate the stack frame */

#if defined(__XTENSA_WINDOWED_ABI__)
	mov	a6, a9			/* load up the 'arg' parameter */
	callx4	a7			/* call the user's function */

	/* Call _exit.  Note that any return parameter from the user's
	   function in a6 is seen as inputs to _exit. */
	movi	a2, JUMPTARGET(_exit)
	callx4	a2
#elif defined(__XTENSA_CALL0_ABI__)
	mov	a2, a9			/* load up the 'arg' parameter */
	callx0	a7			/* call the user's function */

	/* Call _exit.  Note that any return parameter from the user's
	   function in a2 is seen as inputs to _exit.  */
	movi	a0, JUMPTARGET(_exit)
	callx0	a0
#else
#error Unsupported Xtensa ABI
#endif

PSEUDO_END (__clone)
weak_alias (__clone, clone)
