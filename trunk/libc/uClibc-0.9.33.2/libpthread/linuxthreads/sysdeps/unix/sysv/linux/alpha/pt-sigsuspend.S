/* Internal sigsuspend system call for LinuxThreads.  Alpha version.
   Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <sysdep.h>

#undef PSEUDO_PREPARE_ARGS
#define PSEUDO_PREPARE_ARGS	ldq	a0, 0(a0);

	.hidden __pthread_sigsuspend
PSEUDO_NOERRNO(__pthread_sigsuspend, sigsuspend, 1)
	ret
PSEUDO_END_NOERRNO(__pthread_sigsuspend)
