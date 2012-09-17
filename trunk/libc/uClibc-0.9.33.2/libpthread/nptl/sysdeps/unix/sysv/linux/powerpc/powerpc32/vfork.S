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
#define _ERRNO_H	1
#include <bits/errno.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
	lwz	0,PID(2)
	cmpwi	0,0,0
	neg	0,0
	bne-	0,1f
	lis	0,0x8000
1:	stw	0,PID(2)

	DO_CALL (SYS_ify (vfork))

	cmpwi	1,3,0
	beqlr-	1

	lwz	0,PID(2)
	/* Cannot use clrlwi. here, because cr0 needs to be preserved
	   until PSEUDO_RET.  */
	clrlwi	4,0,1
	cmpwi	1,4,0
	beq-	1,1f
	neg	4,0
1:	stw	4,PID(2)

	PSEUDO_RET

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)
weak_alias (__vfork, vfork)
libc_hidden_weak(vfork)
