/* Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
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
#include <pthread-errnos.h>
#include <lowlevellock.h>
#include "lowlevel-atomic.h"


	.text

	.globl	__new_sem_trywait
	.type	__new_sem_trywait,@function
	.align	5
__new_sem_trywait:
	mov.l	r12, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r4, r8
	mov.l	@r8, r0
2:
	tst	r0, r0
	bt	1f

	mov	r0, r3
	mov	r0, r4
	add	#-1, r3
	CMPXCHG (r4, @r8, r3, r2)
	bf/s	2b
	 mov	r2, r0

	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r12
	rts
	 mov	#0, r0

1:
	mov	#EAGAIN, r8
	mova	.Lgot1, r0
	mov.l	.Lgot1, r12
	add	r0, r12

#if USE___THREAD
	mov.l	.Lerrno1, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	bra	.Lexit
	 add	r1, r0
	.align	2
.Lerrno1:
	.long	errno@GOTTPOFF
.Lexit:
#else
	mov.l	.Lerrloc1, r1
	bsrf	r1
	 nop
.Lerrloc1b:
#endif
	mov.l	r8, @r0
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r12
	rts
	 mov	#-1, r0

	.align	2
.Lgot1:
	.long	_GLOBAL_OFFSET_TABLE_
#if !USE___THREAD
.Lerrloc1:
	.long	__errno_location@PLT-(.Lerrloc1b-.)
#endif
	.size	__new_sem_trywait,.-__new_sem_trywait
	weak_alias(__new_sem_trywait, sem_trywait)
