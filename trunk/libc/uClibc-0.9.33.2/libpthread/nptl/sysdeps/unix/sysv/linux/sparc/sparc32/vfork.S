/* Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <tcb-offsets.h>

	.text
	.globl		__syscall_error
ENTRY(__vfork)
	ld	[%g7 + PID], %o5
	cmp	%o5, 0
	bne	1f
	 sub	%g0, %o5, %o4
	sethi	%hi(0x80000000), %o4
1:	st	%o4, [%g7 + PID]

	LOADSYSCALL(vfork)
	ta	0x10
	bcc	2f
	 mov	%o7, %g1
	st	%o5, [%g7 + PID]
	call	__syscall_error
	 mov	%g1, %o7
2:	sub	%o1, 1, %o1
	andcc	%o0, %o1, %o0
	bne,a	1f
	 st	%o5, [%g7 + PID]
1:	retl
	 nop
END(__vfork)

libc_hidden_def (vfork)
weak_alias (__vfork, vfork)
