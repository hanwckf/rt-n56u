/* Copyright (C) 2002, 2003, 2005, 2007, 2009 Free Software Foundation, Inc.
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
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>


	.text

	.globl	__pthread_rwlock_rdlock
	.type	__pthread_rwlock_rdlock,@function
	.protected	__pthread_rwlock_rdlock
	.align	16
__pthread_rwlock_rdlock:
	cfi_startproc
	xorq	%r10, %r10

	/* Get the lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, MUTEX(%rdi)
#endif
	jnz	1f

2:	movl	WRITER(%rdi), %eax
	testl	%eax, %eax
	jne	14f
	cmpl	$0, WRITERS_QUEUED(%rdi)
	je	5f
	cmpl	$0, FLAGS(%rdi)
	je	5f

3:	incl	READERS_QUEUED(%rdi)
	je	4f

	movl	READERS_WAKEUP(%rdi), %edx

	LOCK
#if MUTEX == 0
	decl	(%rdi)
#else
	decl	MUTEX(%rdi)
#endif
	jne	10f

11:
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_PRIVATE_FLAG|FUTEX_WAIT, %esi
	xorl	PSHARED(%rdi), %esi
#else
# if FUTEX_WAIT == 0
	movl	PSHARED(%rdi), %esi
# else
	movl	$FUTEX_WAIT, %esi
	orl	PSHARED(%rdi), %esi
# endif
	xorl	%fs:PRIVATE_FUTEX, %esi
#endif
	addq	$READERS_WAKEUP, %rdi
	movl	$SYS_futex, %eax
	syscall

	subq	$READERS_WAKEUP, %rdi

	/* Reget the lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, MUTEX(%rdi)
#endif
	jnz	12f

13:	decl	READERS_QUEUED(%rdi)
	jmp	2b

5:	xorl	%edx, %edx
	incl	NR_READERS(%rdi)
	je	8f
9:	LOCK
#if MUTEX == 0
	decl	(%rdi)
#else
	decl	MUTEX(%rdi)
#endif
	jne	6f
7:

	movq	%rdx, %rax
	retq

1:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_lock_wait
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	2b

14:	cmpl	%fs:TID, %eax
	jne	3b
	/* Deadlock detected.  */
	movl	$EDEADLK, %edx
	jmp	9b

6:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	7b

	/* Overflow.  */
8:	decl	NR_READERS(%rdi)
	movl	$EAGAIN, %edx
	jmp	9b

	/* Overflow.  */
4:	decl	READERS_QUEUED(%rdi)
	movl	$EAGAIN, %edx
	jmp	9b

10:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	11b

12:	movl	PSHARED(%rdi), %esi
#if MUTEX == 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_lock_wait
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	13b
	cfi_endproc
	.size	__pthread_rwlock_rdlock,.-__pthread_rwlock_rdlock

	.globl	pthread_rwlock_rdlock
pthread_rwlock_rdlock = __pthread_rwlock_rdlock

	.globl	__pthread_rwlock_rdlock_internal
__pthread_rwlock_rdlock_internal = __pthread_rwlock_rdlock
