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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Code taken from glibc2.2.2/sysdeps/unix/sysv/linux/sparc/vfork.S */

#include <features.h>
#include <sys/syscall.h>

.text
.global	fork
#ifdef __UCLIBC_HAS_THREADS__
.weak   fork
#endif
.type	fork,%function
.align 4

fork:
	mov __NR_fork, %g1
	ta	0x10
	bcc,a	9000f
	nop
	save    %sp,-96,%sp
	call __errno_location
	nop
	st	%i0,[%o0]
	jmpl	%i7+8,%g0
	restore	%g0,-1,%o0

9000:
	sub	%o1, 1, %o1
	retl
	and	%o0, %o1, %o0

.size fork,.-fork
#ifdef __UCLIBC_HAS_THREADS__
strong_alias(fork,__libc_fork)
#endif
libc_hidden_def(fork)
