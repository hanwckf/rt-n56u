/* Copyright (C) 2001, 2005, 2007 Free Software Foundation, Inc.

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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after it's all over.  */

#include "sysdep.h"
#include <sys/syscall.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int clone (a2 = int (*fn)(void *arg),
	      a3 = void *child_stack,
	      a4 = int flags,
	      a5 = void *arg,
              a6 = pid_t *ptid,
	      a7 = struct user_desc *tls,
	      16(sp) = pid_t *ctid) */

        .text
ENTRY (__clone)

	/* Sanity check arguments.  */
	beqz	a2, .Leinval	/* no NULL function pointers */
	beqz	a3, .Leinval	/* no NULL stack pointers */

	/* a2 and a3 are candidates for destruction by system-call return
	   parameters.  We don't need the stack pointer after the system
	   call.  We trust that the kernel will preserve a7, a9, and a6.  */

	mov	a9, a5			/* save function argument */
	mov	a5, a7
	mov	a7, a2			/* save function pointer */
	mov	a8, a6			/* use a8 as a temp */
	mov	a6, a4
	mov	a4, a8
	l32i	a8, a1, 16		/* child_tid */
	movi	a2, SYS_ify (clone)

	/* syscall (a2 = NR_clone,
		    a6 = clone_flags,
		    a3 = usp,
		    a4 = parent_tid,
		    a5 = child_tls,
		    a8 = child_tid) */
	syscall
	bltz	a2, SYSCALL_ERROR_LABEL
	beqz	a2, .Lthread_start

	/* Fall through for parent.  */
.Lpseudo_end:
	retw

.Leinval:
	movi	a2, -EINVAL
	j	SYSCALL_ERROR_LABEL

.Lthread_start:
	/* Start child thread.  */
	movi	a0, 0			/* terminate the stack frame */

#ifdef RESET_PID
	/* Check and see if we need to reset the PID.  */
	bbsi.l	a6, 16, 1f		/* CLONE_THREAD = 0x00010000 */
	movi	a2, -1
	bbsi.l	a6, 8, 2f		/* CLONE_VM = 0x00000100 */
	movi	a2, SYS_ify (getpid)
	syscall
2:	rur	a3, THREADPTR
	movi	a4, PID_OFFSET
	add	a4, a4, a3
	s32i	a2, a4, 0
	movi	a4, TID_OFFSET
	add	a4, a4, a3
	s32i	a2, a3, 0
1:
#endif /* RESET_PID */

	mov	a6, a9			/* load up the 'arg' parameter */
	callx4	a7			/* call the user's function */

	/* Call _exit.  Note that any return parameter from the user's
	   function in a6 is seen as inputs to _exit.  */
	movi	a2, JUMPTARGET(_exit)
	callx4	a2

PSEUDO_END (__clone)

weak_alias (__clone, clone)
