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
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>


/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000

	.text

	.globl	pthread_rwlock_timedrdlock
	.type	pthread_rwlock_timedrdlock,@function
	.align	16
pthread_rwlock_timedrdlock:
	cfi_startproc
	pushq	%r12
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r12, 0)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r13, 0)
#ifdef __ASSUME_FUTEX_CLOCK_REALTIME
# define VALREG	%edx
#else
	pushq	%r14
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r14, 0)

	subq	$16, %rsp
	cfi_adjust_cfa_offset(16)
# define VALREG %r14d
#endif

	movq	%rdi, %r12
	movq	%rsi, %r13

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

2:	movl	WRITER(%r12), %eax
	testl	%eax, %eax
	jne	14f
	cmpl	$0, WRITERS_QUEUED(%r12)
	je	5f
	cmpl	$0, FLAGS(%r12)
	je	5f

	/* Check the value of the timeout parameter.  */
3:	cmpq	$1000000000, 8(%r13)
	jae	19f

	incl	READERS_QUEUED(%r12)
	je	4f

	movl	READERS_WAKEUP(%r12), VALREG

	/* Unlock.  */
	LOCK
#if MUTEX == 0
	decl	(%r12)
#else
	decl	MUTEX(%r12)
#endif
	jne	10f

11:
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
#  ifdef __PIC__
	cmpl	$0, __have_futex_clock_realtime@GOTOFF(%rip)
#  else
	cmpl	$0, __have_futex_clock_realtime
#  endif
	je	.Lreltmo
#endif

	movl	$FUTEX_PRIVATE_FLAG|FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, %esi
	xorl	PSHARED(%r12), %esi
	movq	%r13, %r10
	movl	$0xffffffff, %r9d
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	movl	%r14d, %edx
#endif
21:	leaq	READERS_WAKEUP(%r12), %rdi
	movl	$SYS_futex, %eax
	syscall
	movq	%rax, %rdx

#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	.subsection 2
.Lreltmo:
	/* Get current time.  */
	movq	%rsp, %rdi
	xorl	%esi, %esi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movl	$1000, %edi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdi
	subq	(%rsp), %rcx
	subq	%rax, %rdi
	jns	15f
	addq	$1000000000, %rdi
	decq	%rcx
15:	testq	%rcx, %rcx
	js	16f		/* Time is already up.  */

	/* Futex call.  */
	movq	%rcx, (%rsp)	/* Store relative timeout.  */
	movq	%rdi, 8(%rsp)

# ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_PRIVATE_FLAG|FUTEX_WAIT, %esi
	xorl	PSHARED(%r12), %esi
# else
#  if FUTEX_WAIT == 0
	movl	PSHARED(%r12), %esi
#  else
	movl	$FUTEX_WAIT, %esi
	orl	PSHARED(%r12), %esi
#  endif
	xorl	%fs:PRIVATE_FUTEX, %esi
# endif
	movq	%rsp, %r10
	movl	%r14d, %edx

	jmp	21b
	.previous
#endif

17:	/* Reget the lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %esi, (%r12)
#else
	cmpxchgl %esi, MUTEX(%r12)
#endif
	jnz	12f

13:	decl	READERS_QUEUED(%r12)
	cmpq	$-ETIMEDOUT, %rdx
	jne	2b

18:	movl	$ETIMEDOUT, %edx
	jmp	9f


5:	xorl	%edx, %edx
	incl	NR_READERS(%r12)
	je	8f
9:	LOCK
#if MUTEX == 0
	decl	(%r12)
#else
	decl	MUTEX(%r12)
#endif
	jne	6f

7:	movq	%rdx, %rax

#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	addq	$16, %rsp
	cfi_adjust_cfa_offset(-16)
	popq	%r14
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r14)
#endif
	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r12)
	retq

#ifdef __ASSUME_PRIVATE_FUTEX
	cfi_adjust_cfa_offset(16)
	cfi_rel_offset(%r12, 8)
	cfi_rel_offset(%r13, 0)
#else
	cfi_adjust_cfa_offset(40)
	cfi_offset(%r12, -16)
	cfi_offset(%r13, -24)
	cfi_offset(%r14, -32)
#endif
1:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_lock_wait
	jmp	2b

14:	cmpl	%fs:TID, %eax
	jne	3b
	movl	$EDEADLK, %edx
	jmp	9b

6:	movl	PSHARED(%r12), %esi
#if MUTEX == 0
	movq	%r12, %rdi
#else
	leal	MUTEX(%r12), %rdi
#endif
	callq	__lll_unlock_wake
	jmp	7b

	/* Overflow.  */
8:	decl	NR_READERS(%r12)
	movl	$EAGAIN, %edx
	jmp	9b

	/* Overflow.  */
4:	decl	READERS_QUEUED(%r12)
	movl	$EAGAIN, %edx
	jmp	9b

10:	movl	PSHARED(%r12), %esi
#if MUTEX == 0
	movq	%r12, %rdi
#else
	leaq	MUTEX(%r12), %rdi
#endif
	callq	__lll_unlock_wake
	jmp	11b

12:	movl	PSHARED(%r12), %esi
#if MUTEX == 0
	movq	%r12, %rdi
#else
	leaq	MUTEX(%r12), %rdi
#endif
	callq	__lll_lock_wait
	jmp	13b

16:	movq	$-ETIMEDOUT, %rdx
	jmp	17b

19:	movl	$EINVAL, %edx
	jmp	9b
	cfi_endproc
	.size	pthread_rwlock_timedrdlock,.-pthread_rwlock_timedrdlock
