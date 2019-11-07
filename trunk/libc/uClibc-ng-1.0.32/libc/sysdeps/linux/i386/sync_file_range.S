/* Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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

#define _ERRNO_H	1
#include <features.h>
#include <bits/errno.h>
#include <sys/syscall.h>

.text
.global sync_file_range
.type   sync_file_range,%function
sync_file_range:
#ifdef __NR_sync_file_range

	/* Save regs */
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ebp

	movl $__NR_sync_file_range, %eax	/* Syscall number in %eax.  */

	movl	20(%esp), %ebx
	movl	24(%esp), %ecx
	movl	28(%esp), %edx
	movl	32(%esp), %esi
	movl	36(%esp), %edi
	movl	40(%esp), %ebp

	/* Do the system call trap.  */
	int $0x80

	/* Restore regs  */
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%ebx

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja __syscall_error
#else
	movl	$-ENOSYS, %eax
	jmp __syscall_error
#endif
	/* Successful; return the syscall's value.  */
	ret

.size sync_file_range,.-sync_file_range

libc_hidden_def(sync_file_range)

