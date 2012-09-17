/* Copyright (C) 2000, 2002, 2003 Free Software Foundation, Inc.
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


#include <sysdep-cancel.h>
#define _SIGNAL_H
#include <bits/signum.h>

/* The following are defined in linux/sched.h, which unfortunately	*/
/* is not safe for inclusion in an assembly file.			*/
#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_VFORK     0x00004000      /* set if the parent wants the child to wake it up on mm_release */

/* pid_t vfork(void); */
/* Implemented as __clone_syscall(CLONE_VFORK | CLONE_VM | SIGCHLD, 0)	*/

ENTRY(__vfork)
#ifdef SHARED
	addl r14 = @gprel(__libc_pthread_functions#), gp;;
#else
	.weak	pthread_create
	addl r14 = @ltoff(@fptr(pthread_create#)), gp;;
#endif
	ld8 r14 = [r14];;
	cmp.ne p6, p7 = 0, r14
(p6)	br.cond.spnt.few HIDDEN_JUMPTARGET (__fork);;
	alloc r2=ar.pfs,0,0,2,0
	mov out0=CLONE_VM+CLONE_VFORK+SIGCHLD
	mov out1=0		/* Standard sp value.			*/
	;;
	DO_CALL (SYS_ify (clone))
	cmp.eq p6,p0=-1,r10
	;;
(p6)	br.cond.spnt.few __syscall_error
	ret
PSEUDO_END(__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
