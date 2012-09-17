/* Copyright (C) 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza <miguel@gnu.ai.mit.edu>, 1997.

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

/* Code taken from glibc2.2.2/sysdeps/unix/sysv/linux/sparc/vfork.S */

#include <features.h>
#include <sys/syscall.h>

.text
.global	__libc_fork
.type	__libc_fork,%function
.align 4

__libc_fork:
	mov __NR_fork, %g1
	ta	0x10
	bcc,a	9000f
	nop
	save    %sp,-96,%sp
	call HIDDEN_JUMPTARGET(__errno_location)
	nop
	st	%i0,[%o0]
	jmpl	%i7+8,%g0
	restore	%g0,-1,%o0

9000:
	sub	%o1, 1, %o1
	retl
	and	%o0, %o1, %o0

.size __libc_fork,.-__libc_fork
weak_alias(__libc_fork,fork)
libc_hidden_weak(fork)
