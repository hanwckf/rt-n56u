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

#ifdef SHARED
	.section	".toc","aw"
.LC0:
	.tc __libc_pthread_functions[TC],__libc_pthread_functions
	.section	".text"
	.align 2
#endif

ENTRY (__vfork)

#ifdef __NR_vfork

# ifdef SHARED
  ld  10,.LC0@toc(2)
  ld  10,0(10)
  cmpwi  10,0
  bne-  HIDDEN_JUMPTARGET(__fork)
# else
  .weak  pthread_create
	lis  10,pthread_create@highest
	ori  10,10,pthread_create@higher
  sldi 10,10,32
  oris 10,10,pthread_create@h
  ori  10,10,pthread_create@l
  cmpwi  10,0
  bne-  .Lhidden_fork
# endif

	DO_CALL (SYS_ify (vfork));

# ifdef __ASSUME_VFORK_SYSCALL
  PSEUDO_RET
# else
  bnslr+
  /* Check if vfork syscall is known at all.  */
  cmpdi	r3,ENOSYS
# ifdef SHARED
  bne	JUMPTARGET(__syscall_error)
# else
  bne-  .Lsyscall_error
# endif

# endif
#endif

#ifndef __ASSUME_VFORK_SYSCALL
	/* If we don't have vfork, fork is close enough.  */

	DO_CALL (SYS_ify (fork));
	PSEUDO_RET
#endif

# ifndef SHARED
.Lhidden_fork:
	b	HIDDEN_JUMPTARGET(__fork)
.Lsyscall_error:
	b	JUMPTARGET(__syscall_error)
# endif

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
