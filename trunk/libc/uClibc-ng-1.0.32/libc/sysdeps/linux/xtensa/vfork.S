/* Copyright (C) 2005-2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include <sys/syscall.h>
#define _SIGNAL_H
#include <bits/signum.h>
#define __ASSEMBLY__
#include <linux/sched.h>


/*
   Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.

   Note that it is important that we don't create a new stack frame for the
   caller.

*/

/*
   pid_t vfork(void);
   Implemented as __clone_syscall(CLONE_VFORK | CLONE_VM | SIGCHLD, 0)
 */


HIDDEN_ENTRY (__vfork)

#if defined(__XTENSA_WINDOWED_ABI__)
        .literal .Ljumptable, 0, .L4, .L8, .L12

	mov	a3, a0			# move return address out of the way
	movi	a0, .Ljumptable
	extui	a2, a3, 30, 2		# call-size: call4/8/12 = 1/2/3
	addx4	a0, a2, a0		# find return address in jumptable
	l32i	a0, a0, 0
					# exchange top 2 bits of a0 and a3:
	xor	a2, a0, a3
	extui	a2, a2, 30, 2
	slli	a2, a2, 30
	xor	a0, a0, a2
	xor	a3, a3, a2
	retw

	/* a7: return address */

.L4:	mov	a12, a2
	mov	a13, a3

	/* use syscall 'clone' and set new stack pointer to the same address */

	movi	a2, SYS_ify(clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD

        syscall

	movi	a5, -4096

	mov	a6, a2
	mov	a2, a12
	mov	a3, a13

	bgeu	a6, a5, 1f
	jx	a7

1:	call4	.Lerr

	/* a11: return address */

.L8:	mov	a12, a2
	mov	a13, a3
	mov	a14, a6

	movi	a2, SYS_ify(clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD

	syscall

	movi	a9, -4096

	mov	a10, a2
	mov	a2, a12
	mov	a3, a13
	mov	a6, a14

	bgeu	a10, a9, 1f
	jx	a11

1:	call8	.Lerr


	/* a15: return address */

.L12:	mov	a12, a2
	mov	a13, a3
	mov	a14, a6

	movi	a2, SYS_ify(clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD

	syscall

	mov	a3, a13
	movi	a13, -4096

	mov	a6, a14
	mov	a14, a2

	mov	a2, a12

	bgeu	a14, a13, 1f
	jx	a15

1:	call12	.Lerr

	.align 4

.Lerr:	entry	a1, 16

	/* Restore return address */

	extui	a4, a0, 30, 2
	slli	a4, a4, 30
	or	a0, a3, a4

	PSEUDO_END (__vfork)
.Lpseudo_end:
	retw
#elif defined(__XTENSA_CALL0_ABI__)

	/* Use syscall 'clone'.  Set new stack pointer to the same address.  */
	movi	a2, SYS_ify (clone)
	movi	a3, 0
	movi	a6, CLONE_VM | CLONE_VFORK | SIGCHLD
	syscall

	movi	a3, -4096
	bgeu	a2, a3, 1f
	ret
1:
	PSEUDO_END (__vfork)
.Lpseudo_end:
	ret
#else
#error Unsupported Xtensa ABI
#endif

weak_alias (__vfork, vfork)
libc_hidden_def(vfork)
