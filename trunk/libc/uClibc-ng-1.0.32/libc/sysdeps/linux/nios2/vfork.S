/* vfork for Nios II Linux.
   Copyright (C) 2005-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

ENTRY(__vfork)
	movi	r4, 0x4111 /* (CLONE_VM | CLONE_VFORK | SIGCHLD) */
	mov	r5, zero

	/* Do the system call.  */
    	movi 	r2, SYS_ify(clone)

    	trap

	beq	r2, zero, 1f
1:
	ret

END(__vfork)

weak_alias(__vfork, vfork)
libc_hidden_def(vfork)
