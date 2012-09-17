/* Copyright (C) 2005, 2007 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include <sys/syscall.h>

#define PAGE_SHIFT 12

/* __ptr_t __mmap (a2 = __ptr_t addr,
		   a3 = size_t len,
		   a4 = int prot,
		   a5 = int flags,
		   a6 = int fd,
		   a7 = off_t offset) */

ENTRY (__mmap)

	/* We only support mmap2 in the kernel, so shift offset by
	   page - size.  */
	mov	a8, a6
	mov	a6, a2
	movi	a2, SYS_ify (mmap2)
	srli	a9, a7, PAGE_SHIFT

	/* syscall (a2 = NR_mmap2,
		    a6 = arg0,
		    a3 = arg1,
		    a4 = arg2,
		    a5 = arg3,
		    a8 = arg4,
		    a9 = arg5) */

	syscall
	bltz	a2, SYSCALL_ERROR_LABEL

.Lpseudo_end:
	retw

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
libc_hidden_weak (mmap)
