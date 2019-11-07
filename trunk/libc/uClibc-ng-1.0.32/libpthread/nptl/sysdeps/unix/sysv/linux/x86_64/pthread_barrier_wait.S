/* Copyright (C) 2002, 2003, 2004, 2005, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include <lowlevellock.h>
#include <lowlevelbarrier.h>


	.text

	.globl	pthread_barrier_wait
	.type	pthread_barrier_wait,@function
	.align	16
pthread_barrier_wait:
	/* Get the mutex.  */
	xorl	%eax, %eax
	movl	$1, %esi
	LOCK
	cmpxchgl %esi, MUTEX(%rdi)
	jnz	1f

	/* One less waiter.  If this was the last one needed wake
	   everybody.  */
2:	decl	LEFT(%rdi)
	je	3f

	/* There are more threads to come.  */
#if CURR_EVENT == 0
	movl	(%rdi), %edx
#else
	movl	CURR_EVENT(%rdi), %edx
#endif

	/* Release the mutex.  */
	LOCK
	decl	MUTEX(%rdi)
	jne	6f

	/* Wait for the remaining threads.  The call will return immediately
	   if the CURR_EVENT memory has meanwhile been changed.  */
7:
#if FUTEX_WAIT == 0
	movl	PRIVATE(%rdi), %esi
#else
	movl	$FUTEX_WAIT, %esi
	orl	PRIVATE(%rdi), %esi
#endif
	xorq	%r10, %r10
8:	movl	$SYS_futex, %eax
	syscall

	/* Don't return on spurious wakeups.  The syscall does not change
	   any register except %eax so there is no need to reload any of
	   them.  */
#if CURR_EVENT == 0
	cmpl	%edx, (%rdi)
#else
	cmpl	%edx, CURR_EVENT(%rdi)
#endif
	je	8b

	/* Increment LEFT.  If this brings the count back to the
	   initial count unlock the object.  */
	movl	$1, %edx
	movl	INIT_COUNT(%rdi), %eax
	LOCK
	xaddl	%edx, LEFT(%rdi)
	subl	$1, %eax
	cmpl	%eax, %edx
	jne,pt	10f

	/* Release the mutex.  We cannot release the lock before
	   waking the waiting threads since otherwise a new thread might
	   arrive and gets waken up, too.  */
	LOCK
	decl	MUTEX(%rdi)
	jne	9f

10:	xorl	%eax, %eax		/* != PTHREAD_BARRIER_SERIAL_THREAD */

	retq

	/* The necessary number of threads arrived.  */
3:
#if CURR_EVENT == 0
	incl	(%rdi)
#else
	incl	CURR_EVENT(%rdi)
#endif

	/* Wake up all waiters.  The count is a signed number in the kernel
	   so 0x7fffffff is the highest value.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %esi
	orl	PRIVATE(%rdi), %esi
	movl	$SYS_futex, %eax
	syscall

	/* Increment LEFT.  If this brings the count back to the
	   initial count unlock the object.  */
	movl	$1, %edx
	movl	INIT_COUNT(%rdi), %eax
	LOCK
	xaddl	%edx, LEFT(%rdi)
	subl	$1, %eax
	cmpl	%eax, %edx
	jne,pt	5f

	/* Release the mutex.  We cannot release the lock before
	   waking the waiting threads since otherwise a new thread might
	   arrive and gets waken up, too.  */
	LOCK
	decl	MUTEX(%rdi)
	jne	4f

5:	orl	$-1, %eax		/* == PTHREAD_BARRIER_SERIAL_THREAD */

	retq

1:	movl	PRIVATE(%rdi), %esi
	addq	$MUTEX, %rdi
	xorl	$LLL_SHARED, %esi
	callq	__lll_lock_wait
	subq	$MUTEX, %rdi
	jmp	2b

4:	movl	PRIVATE(%rdi), %esi
	addq	$MUTEX, %rdi
	xorl	$LLL_SHARED, %esi
	callq	__lll_unlock_wake
	jmp	5b

6:	movl	PRIVATE(%rdi), %esi
	addq	$MUTEX, %rdi
	xorl	$LLL_SHARED, %esi
	callq	__lll_unlock_wake
	subq	$MUTEX, %rdi
	jmp	7b

9:	movl	PRIVATE(%rdi), %esi
	addq	$MUTEX, %rdi
	xorl	$LLL_SHARED, %esi
	callq	__lll_unlock_wake
	jmp	10b
	.size	pthread_barrier_wait,.-pthread_barrier_wait
