/* brk system call for Linux/i386.
   Copyright (C) 1995, 1996, 2000 Free Software Foundation, Inc.
   Copyright (c) 2015 mirabilos <tg@mirbsd.org>
   This file is part of uclibc-ng, derived from the GNU C Library.

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

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

/* This must be initialized data because commons can't have aliases.  */
void *__curbrk attribute_hidden = 0;

int brk(void *addr)
{
	void *newbrk;

	/*
	 * EBC is used in PIC code, we need to save/restore it manually.
	 * Unfortunately, GCC won't do that for us even if we use con-
	 * straints, and we cannot push it either as ESP clobbers are
	 * silently ignored, but EDX is preserved, so it's scratch space.
	 */
	__asm__("xchgl	%%edx,%%ebx"
	    "\n	int	$0x80"
	    "\n	xchgl	%%edx,%%ebx"
		: "=a" (newbrk)
		: "0" (__NR_brk), "d" (addr)
		: "cc"
	);

	__curbrk = newbrk;

	if (newbrk < addr) {
		__set_errno(ENOMEM);
		return -1;
	}

	return 0;
}
libc_hidden_def(brk)
