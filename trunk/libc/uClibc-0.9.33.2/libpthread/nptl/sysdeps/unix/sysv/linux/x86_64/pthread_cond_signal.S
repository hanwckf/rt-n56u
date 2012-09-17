/* Copyright (C) 2002-2005, 2007, 2009 Free Software Foundation, Inc.
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
#include <pthread-pi-defines.h>
#include <bits/kernel-features.h>
#include <pthread-errnos.h>


	.text

	/* int pthread_cond_signal (pthread_cond_t *cond) */
	.globl	__pthread_cond_signal
	.type	__pthread_cond_signal, @function
	.protected	__pthread_cond_signal
	.align	16
__pthread_cond_signal:

	/* Get internal lock.  */
	movq	%rdi, %r8
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
	movq	total_seq(%r8), %rcx
	cmpq	wakeup_seq(%r8), %rcx
	jbe	4f

	/* Bump the wakeup number.  */
	addq	$1, wakeup_seq(%r8)
	addl	$1, (%rdi)

	/* Wake up one thread.  */
	cmpq	$-1, dep_mutex(%r8)
	movl	$FUTEX_WAKE_OP, %esi
	movl	$1, %edx
	movl	$SYS_futex, %eax
	je	8f

	/* Get the address of the mutex used.  */
	movq    dep_mutex(%r8), %rcx
	movl	MUTEX_KIND(%rcx), %r11d
	andl	$(ROBUST_BIT|PI_BIT), %r11d
	cmpl	$PI_BIT, %r11d
	je	9f

#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$(FUTEX_WAKE_OP|FUTEX_PRIVATE_FLAG), %esi
#else
	orl	%fs:PRIVATE_FUTEX, %esi
#endif

8:	movl	$1, %r10d
#if cond_lock != 0
	addq	$cond_lock, %r8
#endif
	movl	$FUTEX_OP_CLEAR_WAKE_IF_GT_ONE, %r9d
	syscall
#if cond_lock != 0
	subq	$cond_lock, %r8
#endif
	/* For any kind of error, we try again with WAKE.
	   The general test also covers running on old kernels.  */
	cmpq	$-4095, %rax
	jae	7f

	xorl	%eax, %eax
	retq

	/* Wake up one thread and requeue none in the PI Mutex case.  */
9:	movl	$(FUTEX_CMP_REQUEUE_PI|FUTEX_PRIVATE_FLAG), %esi
	movq	%rcx, %r8
	xorq	%r10, %r10
	movl	(%rdi), %r9d	// XXX Can this be right?
	syscall

	leaq	-cond_futex(%rdi), %r8

	/* For any kind of error, we try again with WAKE.
	   The general test also covers running on old kernels.  */
	cmpq	$-4095, %rax
	jb	4f

7:
#ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %esi
#else
	andl	%fs:PRIVATE_FUTEX, %esi
#endif
	orl	$FUTEX_WAKE, %esi
	movl	$SYS_futex, %eax
	/* %rdx should be 1 already from $FUTEX_WAKE_OP syscall.
	movl	$1, %edx  */
	syscall

	/* Unlock.  */
4:	LOCK
#if cond_lock == 0
	decl	(%r8)
#else
	decl	cond_lock(%r8)
#endif
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
5:
	movq	%r8, %rdi
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	jmp	6b
	.size	__pthread_cond_signal, .-__pthread_cond_signal
weak_alias(__pthread_cond_signal, pthread_cond_signal)
