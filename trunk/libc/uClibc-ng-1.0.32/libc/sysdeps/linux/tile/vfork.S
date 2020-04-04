/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define __ASSEMBLY__  /* for kernel headers */
#include <linux/sched.h>
#include <asm/signal.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

	.text
ENTRY (__vfork)
	{
	 moveli r0, CLONE_VFORK | CLONE_VM | SIGCHLD
	 move r1, zero
	}
	{
	 move r2, zero
	 move r3, zero
	}
	moveli TREG_SYSCALL_NR_NAME, __NR_clone
	swint1

	bnez r1, 0f
	jrp lr
PSEUDO_END (__vfork)
libc_hidden_def (vfork)

weak_alias (__vfork, vfork)
