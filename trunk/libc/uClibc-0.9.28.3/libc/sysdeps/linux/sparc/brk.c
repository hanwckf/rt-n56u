/* brk system call for Linux/i386.
   Copyright (C) 1995, 1996, 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>


/* This must be initialized data because commons can't have aliases.  */
void *__curbrk = 0;


int brk (void *addr)
{
    void *newbrk;

    {
	register void *o0 __asm__("%o0") = addr;
	register int g1 __asm__("%g1") = 17 ;
	__asm ("t 0x10" : "=r"(o0) : "r"(g1), "0"(o0) : "cc");
	newbrk = o0;
    }

    __curbrk = newbrk;

    if (newbrk < addr)
    {
	__set_errno (ENOMEM);
	return -1;
    }

    return 0;
}
