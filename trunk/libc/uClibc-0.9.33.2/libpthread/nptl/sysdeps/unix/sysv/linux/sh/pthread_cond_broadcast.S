/* Copyright (C) 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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
#include <lowlevellock.h>
#include <lowlevelcond.h>
#include <bits/kernel-features.h>
#include <pthread-pi-defines.h>
#include <pthread-errnos.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"

	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.protected	__pthread_cond_broadcast
	.align	5
__pthread_cond_broadcast:
	mov.l   r10, @-r15
	mov.l   r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r4, r8

	/* Get internal lock.  */
	mov	#0, r3
	mov	#1, r4
#if cond_lock != 0
	CMPXCHG (r3, @(cond_lock,r8), r4, r2)
#else
	CMPXCHG (r3, @r8, r4, r2)
#endif
	bf	1f
2:
	mov.l	@(total_seq+4,r8),r0
	mov.l	@(total_seq,r8),r1
	mov.l	@(wakeup_seq+4,r8), r2
	cmp/hi	r2, r0
	bt	3f
	cmp/hi	r0, r2
	bt	4f
	mov.l	@(wakeup_seq,r8), r2
	cmp/hi	r2, r1
	bf	4f

3:
	/* Cause all currently waiting threads to recognize they are
	   woken up.  */
	mov.l	r1, @(wakeup_seq,r8)
	mov.l	r0, @(wakeup_seq+4,r8)
	mov.l	r1, @(woken_seq,r8)
	mov.l	r0, @(woken_seq+4,r8)
	mov.l	@(broadcast_seq,r8), r2
	add	#1, r2
	mov.l	r2, @(broadcast_seq,r8)
	add	r1, r1
	mov	r1, r10
	mov.l	r10, @(cond_futex,r8)

	/* Get the address of the mutex used.  */
	mov.l	@(dep_mutex,r8), r9

	/* Unlock.  */
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	7f

8:
	/* Don't use requeue for pshared condvars.  */
	mov	#-1, r0
	cmp/eq	r0, r9
	mov	r8, r4
	bt/s	9f
	 add	#cond_futex, r4

	/* XXX: The kernel only supports FUTEX_CMP_REQUEUE to the same
	   type of futex (private resp. shared).  */
	mov.l	@(MUTEX_KIND,r9), r0
	tst	#(PI_BIT|PS_BIT), r0
	bf	9f

	/* Wake up all threads.  */
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_CMP_REQUEUE|FUTEX_PRIVATE_FLAG), r5
	extu.b	r5, r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
	mov	#FUTEX_CMP_REQUEUE, r0
	or	r0, r5
#endif
	mov	#1, r6
	mov	#-1, r7
	shlr	r7		/* r7 = 0x7fffffff */
	mov	r9, r0
# if MUTEX_FUTEX != 0
	add	#MUTEX_FUTEX, r0
# endif
	mov	r10, r1
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x16
	SYSCALL_INST_PAD

	/* For any kind of error, which mainly is EAGAIN, we try again
	   with WAKE.  The general test also covers running on old
	   kernels.  */
	mov	r0, r1
	mov	#-12, r2
	shad	r2, r1
	not	r1, r1
	tst	r1, r1
	mov	r8, r4
	bt/s	9f
	 add	#cond_futex, r4

10:
	mov	#0, r0
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	rts
	 mov.l	@r15+, r10

4:
	/* Unlock.  */
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	5f
6:
	mov	#0, r0
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	rts
	 mov.l	@r15+, r10

1:
	/* Initial locking failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r6
	mov	#LLL_SHARED, r6
99:
	extu.b	r6, r6
	mov.l	.Lwait5, r1
	bsrf	r1
	 mov	r2, r4
.Lwait5b:
	bra	2b
	 nop

5:
	/* Unlock in loop requires wakeup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r5
	mov	#LLL_SHARED, r5
99:
	mov.l	.Lwake5, r1
	bsrf	r1
	 extu.b	r5, r5
.Lwake5b:
	bra	6b
	 nop

7:
	/* Unlock in loop requires wakeup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov	#-1, r0
	cmp/eq	r0, r9
	bf/s	99f
	 mov	#LLL_PRIVATE, r5
	mov	#LLL_SHARED, r5
99:
	mov.l	.Lwake6, r1
	bsrf	r1
	 extu.b	r5, r5
.Lwake6b:
	bra	8b
	 nop

9:
	mov	#-1, r0
	cmp/eq	r0, r9
	bt/s	99f
	 mov	#FUTEX_WAKE, r5
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_WAKE|FUTEX_PRIVATE_FLAG), r5
	extu.b	r5, r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
	mov	#FUTEX_WAKE, r0
	or	r0, r5
#endif
99:
	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	bra	10b
	 nop

#ifndef __ASSUME_PRIVATE_FUTEX
.Lpfoff:
	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE
#endif

	.align	2
.Lwait5:
	.long	__lll_lock_wait-.Lwait5b
.Lwake5:
	.long	__lll_unlock_wake-.Lwake5b
.Lwake6:
	.long	__lll_unlock_wake-.Lwake6b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
weak_alias (__pthread_cond_broadcast, pthread_cond_broadcast)
