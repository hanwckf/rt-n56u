/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>

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

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
	basr	%r1,0
0:
#ifdef SHARED
	al	%r1,4f-0b(%r1)
	l	%r1,0(%r1)
	ltr	%r1,%r1
#else
	icm	%r1,15,4f-0b(%r1)
#endif
	jne	1f

	/* Do vfork system call.  */
	svc	SYS_ify (vfork)

	/* Check for error.  */
	lhi	%r4,-4095
	clr	%r2,%r4
	jnl	SYSCALL_ERROR_LABEL

	/* Normal return.  */
	br	%r14
1:
	basr	%r1,0
2:
	al	%r1,3f-2b(%r1)
	br	%r1
3:
	.long	HIDDEN_JUMPTARGET(__fork)-2b
4:
#ifdef SHARED
	.long	__libc_pthread_functions-0b
#else
	.weak	pthread_create
	.long	pthread_create
#endif
PSEUDO_END(__vfork)

libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
