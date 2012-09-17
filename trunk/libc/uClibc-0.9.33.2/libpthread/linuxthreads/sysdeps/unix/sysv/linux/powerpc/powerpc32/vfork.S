/* Copyright (C) 2003 Free Software Foundation, Inc.
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
#include <bits/kernel-features.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

#ifdef __NR_vfork
# ifdef SHARED
	mflr	9
#  ifdef HAVE_ASM_PPC_REL16
	bcl	20,31,1f
1:	mflr	10
	addis	10,10,__libc_pthread_functions-1b@ha
	lwz	10,__libc_pthread_functions-1b@l(10)
	mtlr	9
#  else
	bl	_GLOBAL_OFFSET_TABLE_@local-4
	mflr	10
	mtlr	9
	lwz	10,__libc_pthread_functions@got(10)
	lwz	10,0(10)
#  endif
# else
	.weak	pthread_create
	lis	10,pthread_create@ha
	la	10,pthread_create@l(10)
# endif

	cmpwi	10,0
	bne-	.Lhidden_fork

	DO_CALL (SYS_ify (vfork));

# ifdef __ASSUME_VFORK_SYSCALL
	PSEUDO_RET
# else
	bnslr+
	/* Check if vfork syscall is known at all.  */
	cmpwi	r3,ENOSYS
	bne-	.Lsyscall_error

# endif

.Lhidden_fork:
	b	HIDDEN_JUMPTARGET(__fork)

#endif

#ifndef __ASSUME_VFORK_SYSCALL
	/* If we don't have vfork, fork is close enough.  */

	DO_CALL (SYS_ify (fork));
	bnslr+

.Lsyscall_error:
	b	__syscall_error@local
#endif

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
