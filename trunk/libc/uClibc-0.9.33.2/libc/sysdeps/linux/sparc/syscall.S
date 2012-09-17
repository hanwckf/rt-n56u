/* Copyright (C) 1991, 1992, 1997, 2002 Free Software Foundation, Inc.
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

#include <features.h>
#include <sys/syscall.h>

.text
.global syscall
.type   syscall,%function
#if __WORDSIZE == 32
.align  4
#else
.align  2
#endif

syscall:
	mov	%o0, %g1
	mov	%o1, %o0
	mov	%o2, %o1
	mov	%o3, %o2
	mov	%o4, %o3
	mov	%o5, %o4

#if __WORDSIZE == 32
	ta	0x10
	bcs	__syscall_error
#else
	ta	0x6d
	bcs,pn %xcc,__syscall_error
#endif

	 nop
	retl
	 nop
.size syscall,.-syscall
