/* Copyright (C) 2005-2016 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY(__vfork)

	DO_CALL (vfork, 0)
	addik	r12,r0,-4095
	cmpu	r12,r12,r3
	bgei	r12,1f
	rtsd	r15,8
	nop

1:      rsubk   r3,r3,r0
        rtsd    r15,8
        addik   r3,r0,-1        /* delay slot.  */

END(__vfork)

weak_alias(__vfork, vfork)
libc_hidden_def(vfork)
