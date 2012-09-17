/* Copyright (C) 2003, 2004, 2007, 2008 Free Software Foundation, Inc.
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
#include <structsem.h>
#include <lowlevellock.h>
#include "lowlevel-atomic.h"


	.text

	.globl	__new_sem_post
	.type	__new_sem_post,@function
	.align	5
__new_sem_post:
	mov.l	@(VALUE,r4), r2
0:
	mov.l	.Lmax, r1
	cmp/eq	r1, r2
	bt/s	3f
	 mov	r2, r3
	mov	r3, r5
	add	#1, r5
	CMPXCHG (r3, @(VALUE,r4), r5, r2)
	bf	0b
	mov.l	@(NWAITERS,r4), r2
	tst	r2, r2
	bt	2f
	mov	#FUTEX_WAKE, r5
	mov.l	@(PRIVATE,r4), r1
	or	r1, r5
	mov	#1, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	cmp/pz	r0
	bf	1f
2:
	rts
	 mov	#0, r0

1:
	bra	4f
	 mov	#EINVAL, r2

3:
	mov	#EOVERFLOW, r2
4:
	mov.l	r12, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mova	.Lgot3, r0
	mov.l	.Lgot3, r12
	add	r0, r12

#if USE___THREAD
	mov.l	.Lerrno3, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	bra	.Lexit
	 add	r1, r0
	.align	2
.Lerrno3:
	.long	errno@GOTTPOFF
.Lexit:
	mov.l	r2, @r0
#else
	mov	r2, r8
	mov.l	.Lerrloc3, r1
	bsrf	r1
	 nop
.Lerrloc3b:
	mov	r8, @r0
#endif
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r12
	rts
	 mov	#-1, r0

	.align	2
.Lmax:
	.long	SEM_VALUE_MAX
.Lgot3:
	.long	_GLOBAL_OFFSET_TABLE_
#if !USE___THREAD
.Lerrloc3:
	.long	__errno_location@PLT-(.Lerrloc3b-.)
#endif
	.size	__new_sem_post,.-__new_sem_post
	weak_alias(__new_sem_post, sem_post)
