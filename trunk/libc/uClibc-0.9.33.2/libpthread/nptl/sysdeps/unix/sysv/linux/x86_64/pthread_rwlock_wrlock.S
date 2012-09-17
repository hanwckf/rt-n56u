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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <lowlevellock.h>
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>


	.text

	.globl	__pthread_rwlock_wrlock
	.type	__pthread_rwlock_wrlock,@function
	.protected	__pthread_rwlock_wrlock
	.align	16
__pthread_rwlock_wrlock:
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
	cmpl	$0, NR_READERS(%rdi)
	je	5f

3:	incl	WRITERS_QUEUED(%rdi)
	je	4f

	movl	WRITERS_WAKEUP(%rdi), %edx

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
	addq	$WRITERS_WAKEUP, %rdi
	movl	$SYS_futex, %eax
	syscall

	subq	$WRITERS_WAKEUP, %rdi

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

13:	decl	WRITERS_QUEUED(%rdi)
	jmp	2b

5:	xorl	%edx, %edx
	movl	%fs:TID, %eax
	movl	%eax, WRITER(%rdi)
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
	movl	$EDEADLK, %edx
	jmp	9b

6:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
	jmp	7b

4:	decl	WRITERS_QUEUED(%rdi)
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
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_lock_wait
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	13b
	cfi_endproc
	.size	__pthread_rwlock_wrlock,.-__pthread_rwlock_wrlock

	.globl	pthread_rwlock_wrlock
pthread_rwlock_wrlock = __pthread_rwlock_wrlock

	.globl	__pthread_rwlock_wrlock_internal
__pthread_rwlock_wrlock_internal = __pthread_rwlock_wrlock
