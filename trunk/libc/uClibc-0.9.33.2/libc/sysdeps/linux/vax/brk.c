/* brk system call for Linux/VAX.
   Copyright (C) 2000, 2005 Free Software Foundation, Inc.
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
void *__curbrk attribute_hidden = NULL;

int
brk (void *addr)
{
	register unsigned long int result __asm__ ("%%r0");

	__asm__ (
	"	pushl	%%ap		\n"	/* Start frame				*/
	"	pushl	%2		\n"	/* New top address we wish to get	*/
	"	pushl	$1		\n"	/* One argument				*/
	"	movl	%%sp, %%ap	\n"	/* Finish frame				*/
	"	chmk	%1		\n"	/* Perform the system call		*/
	"	addl2	$8, %%sp	\n"	/* Remove pushed arg			*/
	"	movl	(%%sp)+, %%ap	\n"	/* Get back %AP				*/
	: "=r" (result)
	: "0" (__NR_brk),
	  "g" (addr));

	if ((void *) result < addr) {
		__set_errno (ENOMEM);
		return -1;
	} else
		__curbrk = (void *) result;

	return 0;
}
libc_hidden_def(brk)

