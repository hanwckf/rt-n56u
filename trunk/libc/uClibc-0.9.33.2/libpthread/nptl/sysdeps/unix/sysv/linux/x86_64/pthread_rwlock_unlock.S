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
#include <bits/kernel-features.h>


	.text

	.globl	__pthread_rwlock_unlock
	.type	__pthread_rwlock_unlock,@function
	.protected	__pthread_rwlock_unlock
	.align	16
__pthread_rwlock_unlock:
	cfi_startproc
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

2:	cmpl	$0, WRITER(%rdi)
	jne	5f
	decl	NR_READERS(%rdi)
	jnz	6f

5:	movl	$0, WRITER(%rdi)

	movl	$1, %edx
	leaq	WRITERS_WAKEUP(%rdi), %r10
	cmpl	$0, WRITERS_QUEUED(%rdi)
	jne	0f

	/* If also no readers waiting nothing to do.  */
	cmpl	$0, READERS_QUEUED(%rdi)
	je	6f

	movl	$0x7fffffff, %edx
	leaq	READERS_WAKEUP(%rdi), %r10

0:	incl	(%r10)
	LOCK
#if MUTEX == 0
	decl	(%rdi)
#else
	decl	MUTEX(%rdi)
#endif
	jne	7f

8:
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_PRIVATE_FLAG|FUTEX_WAKE, %esi
	xorl	PSHARED(%rdi), %esi
#else
	movl	$FUTEX_WAKE, %esi
	orl	PSHARED(%rdi), %esi
	xorl	%fs:PRIVATE_FUTEX, %esi
#endif
	movl	$SYS_futex, %eax
	movq	%r10, %rdi
	syscall

	xorl	%eax, %eax
	retq

	.align	16
6:	LOCK
#if MUTEX == 0
	decl	(%rdi)
#else
	decl	MUTEX(%rdi)
#endif
	jne	3f

4:	xorl	%eax, %eax
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

3:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
	jmp	4b

7:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
	jmp	8b
	cfi_endproc
	.size	__pthread_rwlock_unlock,.-__pthread_rwlock_unlock

	.globl	pthread_rwlock_unlock
pthread_rwlock_unlock = __pthread_rwlock_unlock

	.globl	__pthread_rwlock_unlock_internal
__pthread_rwlock_unlock_internal = __pthread_rwlock_unlock
