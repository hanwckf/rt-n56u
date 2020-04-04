/* Copyright (C) 2004-2017 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2004.

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

#include <sysdep.h>

	.text
	.globl	__syscall_error
ENTRY(__vfork)
	LOADSYSCALL(vfork)
	ta	0x6d
	bcc,pt	%xcc, 2f
	 mov	%o7, %g1
	call	__syscall_error
	 mov	%g1, %o7
2:	sub	%o1, 1, %o1
	andcc	%o0, %o1, %o0
1:	retl
	 nop
END(__vfork)

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
