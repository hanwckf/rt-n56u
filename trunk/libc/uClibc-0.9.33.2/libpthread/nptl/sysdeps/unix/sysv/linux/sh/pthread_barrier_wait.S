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
#include <lowlevellock.h>
#include <lowlevelbarrier.h>
#include "lowlevel-atomic.h"

	.text

	.globl	pthread_barrier_wait
	.type	pthread_barrier_wait,@function
	.align	5
pthread_barrier_wait:
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r4, r8

	/* Get the mutex.  */
	mov	#0, r3
	mov	#1, r4
	CMPXCHG (r3, @(MUTEX,r8), r4, r2)
	bf	1f

	/* One less waiter.  If this was the last one needed wake
	   everybody.  */
2:
	mov.l	@(LEFT,r8), r0
	add	#-1, r0
	mov.l	r0, @(LEFT,r8)
	tst	r0, r0
	bt	3f

	/* There are more threads to come.  */
	mov.l	@(CURR_EVENT,r8), r6

	/* Release the mutex.  */
	DEC (@(MUTEX,r8), r2)
	tst	r2, r2
	bf	6f
7:
	/* Wait for the remaining threads.  The call will return immediately
	   if the CURR_EVENT memory has meanwhile been changed.  */
	mov	r8, r4
#if CURR_EVENT != 0
	add	#CURR_EVENT, r4
#endif
#if FUTEX_WAIT == 0
	mov.l	@(PRIVATE,r8), r5
#else
	mov	#FUTEX_WAIT, r5
	mov.l	@(PRIVATE,r8), r0
	or	r0, r5
#endif
	mov	#0, r7
8:
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	/* Don't return on spurious wakeups.  The syscall does not change
	   any register except r0 so there is no need to reload any of
	   them.  */
	mov.l	@(CURR_EVENT,r8), r0
	cmp/eq	r0, r6
	bt	8b

	/* Increment LEFT.  If this brings the count back to the
	   initial count unlock the object.  */
	mov	#1, r3
	mov.l	@(INIT_COUNT,r8), r4
	XADD	(r3, @(LEFT,r8), r2, r5)
	add	#-1, r4
	cmp/eq	r2, r4
	bf	10f

	/* Release the mutex.  We cannot release the lock before
	   waking the waiting threads since otherwise a new thread might
	   arrive and gets waken up, too.  */
	DEC (@(MUTEX,r8), r2)
	tst	r2, r2
	bf	9f

10:
	mov	#0, r0		/* != PTHREAD_BARRIER_SERIAL_THREAD */
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	rts
	 mov.l	@r15+, r9

3:
	/* The necessary number of threads arrived.  */
	mov.l	@(CURR_EVENT,r8), r1
	add	#1, r1
	mov.l	r1, @(CURR_EVENT,r8)

	/* Wake up all waiters.  The count is a signed number in the kernel
	   so 0x7fffffff is the highest value.  */
	mov.l	.Lall, r6
	mov	r8, r4
#if CURR_EVENT != 0
	add	#CURR_EVENT, r4
#endif
	mov	#0, r7
	mov	#FUTEX_WAKE, r5
	mov.l	@(PRIVATE,r8), r0
	or	r0, r5
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	/* Increment LEFT.  If this brings the count back to the
	   initial count unlock the object.  */
	mov	#1, r3
	mov.l	@(INIT_COUNT,r8), r4
	XADD	(r3, @(LEFT,r8), r2, r5)
	add	#-1, r4
	cmp/eq	r2, r4
	bf	5f

	/* Release the mutex.  */
	DEC (@(MUTEX,r8), r2)
	tst	r2, r2
	bf	4f
5:
	mov	#-1, r0		/* == PTHREAD_BARRIER_SERIAL_THREAD */
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	rts
	 mov.l	@r15+, r9

1:
	mov.l	@(PRIVATE,r8), r6
	mov	#LLL_SHARED, r0
	extu.b	r0, r0
	xor	r0, r6
	mov	r2, r4
	mov	r8, r5
	mov.l	.Lwait0, r1
	bsrf	r1
	 add	#MUTEX, r5
.Lwait0b:
	bra	2b
	 nop

4:
	mov.l	@(PRIVATE,r8), r5
	mov	#LLL_SHARED, r0
	extu.b	r0, r0
	xor	r0, r5
	mov	r8, r4
	mov.l	.Lwake0, r1
	bsrf	r1
	 add	#MUTEX, r4
.Lwake0b:
	bra	5b
	 nop

6:
	mov	r6, r9
	mov.l	@(PRIVATE,r8), r5
	mov	#LLL_SHARED, r0
	extu.b	r0, r0
	xor	r0, r5
	mov	r8, r4
	mov.l	.Lwake1, r1
	bsrf	r1
	 add	#MUTEX, r4
.Lwake1b:
	bra	7b
	 mov	r9, r6

9:	
	mov	r6, r9
	mov.l	@(PRIVATE,r8), r5
	mov	#LLL_SHARED, r0
	extu.b	r0, r0
	xor	r0, r5
	mov	r8, r4
	mov.l	.Lwake2, r1
	bsrf	r1
	 add	#MUTEX, r4
.Lwake2b:
	bra	10b
	 mov	r9, r6

	.align	2
.Lall:
	.long	0x7fffffff
.Lwait0:
	.long	__lll_lock_wait-.Lwait0b
.Lwake0:
	.long	__lll_unlock_wake-.Lwake0b
.Lwake1:
	.long	__lll_unlock_wake-.Lwake1b
.Lwake2:
	.long	__lll_unlock_wake-.Lwake2b
	.size	pthread_barrier_wait,.-pthread_barrier_wait
