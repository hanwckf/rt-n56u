/* Copyright (C) 2002, 2003, 2004, 2007 Free Software Foundation, Inc.
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
	cfi_startproc
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)

	movl	8(%esp), %ebx

	/* Get the mutex.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
	cmpxchgl %edx, MUTEX(%ebx)
	jnz	1f

	/* One less waiter.  If this was the last one needed wake
	   everybody.  */
2:	subl	$1, LEFT(%ebx)
	je	3f

	/* There are more threads to come.  */
	pushl	%esi
	cfi_adjust_cfa_offset(4)
	cfi_offset(%esi, -12)

#if CURR_EVENT == 0
	movl	(%ebx), %edx
#else
	movl	CURR_EVENT(%ebx), %edx
#endif

	/* Release the mutex.  */
	LOCK
	subl	$1, MUTEX(%ebx)
	jne	6f

	/* Wait for the remaining threads.  The call will return immediately
	   if the CURR_EVENT memory has meanwhile been changed.  */
7:
#if FUTEX_WAIT == 0
	movl	PRIVATE(%ebx), %ecx
#else
	movl	$FUTEX_WAIT, %ecx
	orl	PRIVATE(%ebx), %ecx
#endif
	xorl	%esi, %esi
8:	movl	$SYS_futex, %eax
	ENTER_KERNEL

	/* Don't return on spurious wakeups.  The syscall does not change
	   any register except %eax so there is no need to reload any of
	   them.  */
#if CURR_EVENT == 0
	cmpl	%edx, (%ebx)
#else
	cmpl	%edx, CURR_EVENT(%ebx)
#endif
	je	8b

	/* Increment LEFT.  If this brings the count back to the
	   initial count unlock the object.  */
	movl	$1, %edx
	movl	INIT_COUNT(%ebx), %ecx
	LOCK
	xaddl	%edx, LEFT(%ebx)
	subl	$1, %ecx
	cmpl	%ecx, %edx
	jne	10f

	/* Release the mutex.  We cannot release the lock before
	   waking the waiting threads since otherwise a new thread might
	   arrive and gets waken up, too.  */
	LOCK
	subl	$1, MUTEX(%ebx)
	jne	9f

	/* Note: %esi is still zero.  */
10:	movl	%esi, %eax		/* != PTHREAD_BARRIER_SERIAL_THREAD */

	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%esi)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret

	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)
	
	/* The necessary number of threads arrived.  */
3:
#if CURR_EVENT == 0
	addl	$1, (%ebx)
#else
	addl	$1, CURR_EVENT(%ebx)
#endif

	/* Wake up all waiters.  The count is a signed number in the kernel
	   so 0x7fffffff is the highest value.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %ecx
	orl	PRIVATE(%ebx), %ecx
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	/* Increment LEFT.  If this brings the count back to the
	   initial count unlock the object.  */
	movl	$1, %edx
	movl	INIT_COUNT(%ebx), %ecx
	LOCK
	xaddl	%edx, LEFT(%ebx)
	subl	$1, %ecx
	cmpl	%ecx, %edx
	jne	5f

	/* Release the mutex.  We cannot release the lock before
	   waking the waiting threads since otherwise a new thread might
	   arrive and gets waken up, too.  */
	LOCK
	subl	$1, MUTEX(%ebx)
	jne	4f

5:	orl	$-1, %eax		/* == PTHREAD_BARRIER_SERIAL_THREAD */

	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret

	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)
1:	movl	PRIVATE(%ebx), %ecx
	leal	MUTEX(%ebx), %edx
	xorl	$LLL_SHARED, %ecx
	call	__lll_lock_wait
	jmp	2b

4:	movl	PRIVATE(%ebx), %ecx
	leal	MUTEX(%ebx), %eax
	xorl	$LLL_SHARED, %ecx
	call	__lll_unlock_wake
	jmp	5b

	cfi_adjust_cfa_offset(4)
	cfi_offset(%esi, -12)
6:	movl	PRIVATE(%ebx), %ecx
	leal	MUTEX(%ebx), %eax
	xorl	$LLL_SHARED, %ecx
	call	__lll_unlock_wake
	jmp	7b

9:	movl	PRIVATE(%ebx), %ecx
	leal	MUTEX(%ebx), %eax
	xorl	$LLL_SHARED, %ecx
	call	__lll_unlock_wake
	jmp	10b
	cfi_endproc
	.size	pthread_barrier_wait,.-pthread_barrier_wait
