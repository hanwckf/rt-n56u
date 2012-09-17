/* Copyright (C) 1999, 2002, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>.

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
#include <kernel-features.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

#ifdef __NR_vfork

#ifdef SHARED
	ldr	ip, 1f
	ldr	r0, 2f
3:	add	ip, pc, ip
	ldr	r0, [ip, r0]
#else
	ldr	r0, 1f
#endif
	movs	r0, r0
	bne	HIDDEN_JUMPTARGET (__fork)

	DO_CALL (vfork, 0)
	cmn	a1, #4096
	RETINSTR(cc, lr)

#ifndef __ASSUME_VFORK_SYSCALL
	/* Check if vfork syscall is known at all.  */
	cmn	a1, #ENOSYS
	bne	PLTJMP(C_SYMBOL_NAME(__syscall_error))
#endif

#endif

#ifndef __ASSUME_VFORK_SYSCALL
	/* If we don't have vfork, fork is close enough.  */
	DO_CALL (fork, 0)
	cmn	a1, #4096
	RETINSTR(cc, lr)
#elif !defined __NR_vfork
# error "__NR_vfork not available and __ASSUME_VFORK_SYSCALL defined"
#endif
    	b	PLTJMP(C_SYMBOL_NAME(__syscall_error))

#ifdef SHARED
1:	.word	_GLOBAL_OFFSET_TABLE_ - 3b - 8
2:	.word	__libc_pthread_functions(GOTOFF)
#else
	.weak	pthread_create
1:	.word	pthread_create
#endif

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
