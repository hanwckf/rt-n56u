/* Copyright (C) 1995-2000,2002,2003,2004,2005,2006
      Free Software Foundation, Inc.
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

#include <_lfs_64.h>
#include <sys/syscall.h>

#ifdef __NR_fadvise64_64

.text
.global posix_fadvise64
.type   posix_fadvise64,%function
posix_fadvise64:
	/* Save regs  */
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	movl $__NR_fadvise64_64, %eax	/* Syscall number in %eax.  */

	movl	20(%esp), %ebx
	movl	24(%esp), %ecx
	movl	28(%esp), %edx
	movl	32(%esp), %esi
	movl	36(%esp), %edi
	movl	40(%esp), %ebp

	/* Do the system call trap.  */
	int $0x80

	/* Restore regs  */
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp

	/* Returns 0 on success, else an error code.  */
	negl	%eax

	/* Successful; return the syscall's value.  */
	ret
.size posix_fadvise64,.-posix_fadvise64
#endif
