/* Copyright (C) 2001, 2002, 2004 Free Software Foundation, Inc.
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

#include <sys/syscall.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

#ifndef __NR_vfork
/* No vfork so use fork instead */
.weak vfork
	vfork = __libc_fork

#else

.text
.global __vfork
.type	__vfork,%function
.align 16
__vfork:

	/* Pop the return PC value into RDI.  We need a register that
	   is preserved by the syscall and that we're allowed to destroy. */
	popq	%rdi

	/* Stuff the syscall number in RAX and enter into the kernel.  */
	movl	$__NR_vfork, %eax
	syscall

	/* Push back the return PC.  */
	pushq	%rdi

	cmpl	$-4095, %eax
	jae __error		/* Branch forward if it failed.  */

	/* Normal return.  */
	ret

__error:
	jmp __syscall_error

.size __vfork,.-__vfork

.weak vfork
	vfork = __vfork

#endif /* __NR_vfork */
