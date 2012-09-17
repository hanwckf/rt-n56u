/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep-cancel.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <sys/syscall.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
#ifdef SHARED
	mov.l	.Lgot, r1
	mova	.Lgot, r0
	add	r0, r1
	mov.l	.Lpthread_func, r0
	mov.l	@(r0,r1), r0
#else
	mov.l	.Lpthread_create, r0
#endif
	tst	r0, r0
	bf	.Lhidden_fork

	mov.w	.L1, r3
	trapa	#__SH_SYSCALL_TRAP_BASE
	mov     r0, r1
	mov	#-12, r2
	shad	r2, r1
	not	r1, r1			/* r1=0 means r0 = -1 to -4095 */
	tst	r1, r1			/* i.e. error in linux */
	bf	.Lpseudo_end
	SYSCALL_ERROR_HANDLER
.Lpseudo_end:
	rts
	 nop
.L1:	.word	__NR_vfork
	.align	2
#ifdef SHARED
.Lgot:
	.long	_GLOBAL_OFFSET_TABLE_
.Lpthread_func:
	.long	__libc_pthread_functions@GOTOFF
#else
.Lpthread_create:
	.weak	pthread_create
	.long	pthread_create
#endif

.Lhidden_fork:
	mov.l	.L2, r1
	braf	r1
	 nop
1:
	.align 2
.L2:	.long	HIDDEN_JUMPTARGET(__fork)-1b

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
