/* Copyright (C) 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

#ifdef SHARED
	cmpq	$0, __libc_pthread_functions(%rip)
#else
	.weak	pthread_create
	movq	$pthread_create, %rax
	testq	%rax, %rax
#endif
	jne	HIDDEN_JUMPTARGET (__fork)

	/* Pop the return PC value into RDI.  We need a register that
	   is preserved by the syscall and that we're allowed to destroy. */
	popq	%rdi
	cfi_adjust_cfa_offset(-8)

	/* Stuff the syscall number in RAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	syscall

	/* Push back the return PC.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)

	cmpl	$-4095, %eax
	jae SYSCALL_ERROR_LABEL		/* Branch forward if it failed.  */

	/* Normal return.  */
.Lpseudo_end:
	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
