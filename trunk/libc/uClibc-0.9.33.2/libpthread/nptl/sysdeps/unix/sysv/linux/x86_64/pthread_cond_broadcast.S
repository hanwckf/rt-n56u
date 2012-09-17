/* Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2009
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <lowlevellock.h>
#include <lowlevelcond.h>
#include <bits/kernel-features.h>
#include <pthread-pi-defines.h>
#include <pthread-errnos.h>


	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.protected	__pthread_cond_broadcast
	.align	16
__pthread_cond_broadcast:

	/* Get internal lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jnz	1f

2:	addq	$cond_futex, %rdi
	movq	total_seq-cond_futex(%rdi), %r9
	cmpq	wakeup_seq-cond_futex(%rdi), %r9
	jna	4f

	/* Cause all currently waiting threads to recognize they are
	   woken up.  */
	movq	%r9, wakeup_seq-cond_futex(%rdi)
	movq	%r9, woken_seq-cond_futex(%rdi)
	addq	%r9, %r9
	movl	%r9d, (%rdi)
	incl	broadcast_seq-cond_futex(%rdi)

	/* Get the address of the mutex used.  */
	movq	dep_mutex-cond_futex(%rdi), %r8

	/* Unlock.  */
	LOCK
	decl	cond_lock-cond_futex(%rdi)
	jne	7f

8:	cmpq	$-1, %r8
	je	9f

	/* Do not use requeue for pshared condvars.  */
	testl	$PS_BIT, MUTEX_KIND(%r8)
	jne	9f

	/* Requeue to a PI mutex if the PI bit is set.  */
	movl	MUTEX_KIND(%r8), %eax
	andl	$(ROBUST_BIT|PI_BIT), %eax
	cmpl	$PI_BIT, %eax
	je	81f

	/* Wake up all threads.  */
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$(FUTEX_CMP_REQUEUE|FUTEX_PRIVATE_FLAG), %esi
#else
	movl	%fs:PRIVATE_FUTEX, %esi
	orl	$FUTEX_CMP_REQUEUE, %esi
#endif
	movl	$SYS_futex, %eax
	movl	$1, %edx
	movl	$0x7fffffff, %r10d
	syscall

	/* For any kind of error, which mainly is EAGAIN, we try again
	   with WAKE.  The general test also covers running on old
	   kernels.  */
	cmpq	$-4095, %rax
	jae	9f

10:	xorl	%eax, %eax
	retq

	/* Wake up all threads.  */
81:	movl	$(FUTEX_CMP_REQUEUE_PI|FUTEX_PRIVATE_FLAG), %esi
	movl	$SYS_futex, %eax
	movl	$1, %edx
	movl	$0x7fffffff, %r10d
	syscall

	/* For any kind of error, which mainly is EAGAIN, we try again
	   with WAKE.  The general test also covers running on old
	   kernels.  */
	cmpq	$-4095, %rax
	jb	10b
	jmp	9f

	.align	16
	/* Unlock.  */
4:	LOCK
	decl	cond_lock-cond_futex(%rdi)
	jne	5f

6:	xorl	%eax, %eax
	retq

	/* Initial locking failed.  */
1:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	2b

	/* Unlock in loop requires wakeup.  */
5:	addq	$cond_lock-cond_futex, %rdi
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	jmp	6b

	/* Unlock in loop requires wakeup.  */
7:	addq	$cond_lock-cond_futex, %rdi
	cmpq	$-1, %r8
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	subq	$cond_lock-cond_futex, %rdi
	jmp	8b

9:	/* The futex requeue functionality is not available.  */
	cmpq	$-1, %r8
	movl	$0x7fffffff, %edx
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAKE, %eax
	movl	$(FUTEX_WAKE|FUTEX_PRIVATE_FLAG), %esi
	cmove	%eax, %esi
#else
	movl	$0, %eax
	movl	%fs:PRIVATE_FUTEX, %esi
	cmove	%eax, %esi
	orl	$FUTEX_WAKE, %esi
#endif
	movl	$SYS_futex, %eax
	syscall
	jmp	10b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
weak_alias(__pthread_cond_broadcast, pthread_cond_broadcast)
