/* Copyright (C) 2005, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include "sysdep.h"
#include <sys/syscall.h>
#define _SIGNAL_H
#include <bits/signum.h>


/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.

   Note that it is important that we don't create a new stack frame for the
   caller.  */


/* The following are defined in linux/sched.h, which unfortunately
   is not safe for inclusion in an assembly file.  */
#define CLONE_VM        0x00000100     /* set if VM shared between processes */
#define CLONE_VFORK     0x00004000     /* set if the parent wants the child to
					  wake it up on mm_release */

#ifndef SAVE_PID
#define SAVE_PID
#endif

#ifndef RESTORE_PID
#define RESTORE_PID
#endif


/* pid_t vfork(void);
   Implemented as __clone_syscall(CLONE_VFORK | CLONE_VM | SIGCHLD, 0) */

ENTRY (__vfork)

	movi	a6, .Ljumptable
	extui	a2, a0, 30, 2		/* call-size: call4/8/12 = 1/2/3 */
	addx4	a4, a2, a6		/* find return address in jumptable */
	l32i	a4, a4, 0
	add	a4, a4, a6

	slli	a2, a2, 30
	xor	a3, a0, a2		/* remove call-size from return addr */
	extui	a5, a4, 30, 2		/* get high bits of jump target */
	slli	a5, a5, 30
	or	a3, a3, a5		/* stuff them into the return address */
	xor	a4, a4, a5		/* clear high bits of jump target */
	or	a0, a4, a2		/* create temporary return address */
	retw				/* "return" to .L4, .L8, or .L12 */

	.align	4
.Ljumptable:
	.word	0
	.word	.L4 - .Ljumptable
	.word	.L8 - .Ljumptable
	.word	.L12 - .Ljumptable

	/* a7: return address */
.L4:	mov	a12, a2
	mov	a13, a3

	SAVE_PID

	/* Use syscall 'clone'.  Set new stack pointer to the same address.  */
	movi	a2, SYS_ify (clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD
        syscall

	RESTORE_PID

	movi	a5, -4096

	mov	a6, a2
	mov	a2, a12
	mov	a3, a13

	bgeu	a6, a5, 1f
	jx	a7
1:	call4	.Lerr			/* returns to original caller */


	/* a11: return address */
.L8:	mov	a12, a2
	mov	a13, a3
	mov	a14, a6

	SAVE_PID

	movi	a2, SYS_ify (clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD
	syscall

	RESTORE_PID

	movi	a9, -4096

	mov	a10, a2
	mov	a2, a12
	mov	a3, a13
	mov	a6, a14

	bgeu	a10, a9, 1f
	jx	a11
1:	call8	.Lerr			/* returns to original caller */


	/* a15: return address */
.L12:	mov	a12, a2
	mov	a13, a3
	mov	a14, a6

	SAVE_PID

	movi	a2, SYS_ify (clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD
	syscall

	RESTORE_PID

	mov	a3, a13
	movi	a13, -4096

	mov	a6, a14
	mov	a14, a2

	mov	a2, a12

	bgeu	a14, a13, 1f
	jx	a15
1:	call12	.Lerr			/* returns to original caller */


	.align 4
.Lerr:	entry	a1, 16

	/* Restore the return address.  */
	extui	a4, a0, 30, 2		/* get the call-size bits */
	slli	a4, a4, 30
	slli	a3, a3, 2		/* clear high bits of target address */
	srli	a3, a3, 2
	or	a0, a3, a4		/* combine them */

	PSEUDO_END (__vfork)
.Lpseudo_end:
	retw

libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
