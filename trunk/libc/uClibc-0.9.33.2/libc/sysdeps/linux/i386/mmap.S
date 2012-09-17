/* Copyright (C) 1995,96,97,98,99,2000,2002,2005 Free Software Foundation, Inc.
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

.text
.global mmap
.type   mmap,%function
mmap:

	/* Save registers.  */
	movl %ebx, %edx

	movl $__NR_mmap, %eax	/* System call number in %eax.  */

	lea 4(%esp), %ebx		/* Address of args is 1st arg.  */

	/* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	movl %edx, %ebx

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja __syscall_error

	/* Successful; return the syscall's value.  */
	ret

.size mmap,.-mmap

libc_hidden_def(mmap)
