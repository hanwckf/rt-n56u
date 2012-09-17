/* Copyright (C) 2000, 2002 Free Software Foundation, Inc.
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


#include "sysdep.h"
#define _SIGNAL_H
#include <bits/signum.h>

/* pid_t fork(void); */
/* Implemented as a clone system call with parameters SIGCHLD and 0	*/

ENTRY(__libc_fork)
	alloc r2=ar.pfs,0,0,2,0
	mov out0=SIGCHLD	/* Return SIGCHLD when child finishes	*/
				/* no other clone flags; nothing shared	*/
	mov out1=0		/* Standard sp value.			*/
	;;
	DO_CALL (SYS_ify (clone))
	cmp.eq p6,p0=-1,r10
(p6)	br.cond.spnt.few __syscall_error
	ret
PSEUDO_END(__libc_fork)

weak_alias (__libc_fork, __fork)
libc_hidden_def (__fork)
weak_alias (__libc_fork, fork)
libc_hidden_weak (fork)
