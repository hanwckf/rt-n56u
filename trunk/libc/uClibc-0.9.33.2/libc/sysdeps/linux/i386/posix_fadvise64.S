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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define _ERRNO_H	1
#include <features.h>
#include <bits/errno.h>
#include <sys/syscall.h>

#if defined __NR_fadvise64_64 || defined __NR_fadvise64

/* Was named __libc_posix_fadvise64 for some inexplicable reason.
** google says only uclibc has *__libc*_posix_fadviseXXX,
** so it cannot be compat with anything.
**
** Remove this comment and one at the end after 0.9.31
*/

.text
.global posix_fadvise64
.type   posix_fadvise64,%function
posix_fadvise64:
#if defined __NR_fadvise64_64
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

#elif defined __NR_fadvise64
	/* Save regs  */
	pushl	%ebx
	pushl	%esi
	pushl	%edi
#if 0
	/* does len overflow long?  */
	cmpl	$0, 28(%esp)
	movl	$-EOVERFLOW, %eax
	jne	overflow
#endif
	movl $__NR_fadvise64, %eax	/* Syscall number in %eax.  */

	movl	16(%esp), %ebx
	movl	20(%esp), %ecx
	movl	24(%esp), %edx
	movl	28(%esp), %esi
	movl	32(%esp), %edi

	/* Do the system call trap.  */
	int $0x80
overflow:
	/* Restore regs  */
	popl	%edi
	popl	%esi
	popl	%ebx

	/* Returns 0 on success, else an error code.  */
	negl	%eax
#endif

	/* Successful; return the syscall's value.  */
	ret

.size posix_fadvise64,.-posix_fadvise64

/*
** libc_hidden_def(__libc_posix_fadvise64)
** #if defined __UCLIBC_HAS_LFS__ && defined __UCLIBC_HAS_ADVANCED_REALTIME__
** weak_alias(__libc_posix_fadvise64,posix_fadvise64)
** #endif
*/

#endif
