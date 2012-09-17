/* Copyright (C) 1999, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@gnu.org>.

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

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>

/* Save the PID value.  */
#define SAVE_PID \
	movl	%gs:PID, %edx; 						      \
	movl	%edx, %eax;						      \
	negl	%eax;							      \
	movl	%eax, %gs:PID

/* Restore the old PID value in the parent.  */
#define RESTORE_PID \
	testl	%eax, %eax;						      \
	je	1f;							      \
	movl	%edx, %gs:PID;						      \
1:

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
	/* Pop the return PC value into ECX.  */
	popl	%ecx

	SAVE_PID

	/* Stuff the syscall number in EAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	int	$0x80

	RESTORE_PID

	/* Jump to the return PC.  Don't jump directly since this
	   disturbs the branch target cache.  Instead push the return
	   address back on the stack.  */
	pushl	%ecx

	cmpl	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL	/* Branch forward if it failed.  */
L(pseudo_end):
	ret
PSEUDO_END (__vfork)

weak_alias (__vfork, vfork)
