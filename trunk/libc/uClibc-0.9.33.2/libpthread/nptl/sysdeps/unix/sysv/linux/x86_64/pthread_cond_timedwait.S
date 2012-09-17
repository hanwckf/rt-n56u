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
#include <pthread-errnos.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000


	.text


/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.protected	__pthread_cond_timedwait
	.align	16
__pthread_cond_timedwait:
.LSTARTCODE:
	cfi_startproc

	pushq	%r12
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r12, 0)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r13, 0)
	pushq	%r14
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r14, 0)
	pushq	%r15
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r15, 0)
#ifdef __ASSUME_FUTEX_CLOCK_REALTIME
# define FRAME_SIZE 32
#else
# define FRAME_SIZE 48
#endif
	subq	$FRAME_SIZE, %rsp
	cfi_adjust_cfa_offset(FRAME_SIZE)
	cfi_remember_state

	cmpq	$1000000000, 8(%rdx)
	movl	$EINVAL, %eax
	jae	48f

	/* Stack frame:

	   rsp + 48
		    +--------------------------+
	   rsp + 32 | timeout value            |
		    +--------------------------+
	   rsp + 24 | old wake_seq value       |
		    +--------------------------+
	   rsp + 16 | mutex pointer            |
		    +--------------------------+
	   rsp +  8 | condvar pointer          |
		    +--------------------------+
	   rsp +  4 | old broadcast_seq value  |
		    +--------------------------+
	   rsp +  0 | old cancellation mode    |
		    +--------------------------+
	*/

	cmpq	$-1, dep_mutex(%rdi)

	/* Prepare structure passed to cancellation handler.  */
	movq	%rdi, 8(%rsp)
	movq	%rsi, 16(%rsp)
	movq	%rdx, %r13

	je	22f
	movq	%rsi, dep_mutex(%rdi)

22:
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
#  ifdef __PIC__
	cmpl	$0, __have_futex_clock_realtime@GOTOFF(%rip)
#  else
	cmpl	$0, __have_futex_clock_realtime
#  endif
	je	.Lreltmo
#endif

	/* Get internal lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jnz	31f

	/* Unlock the mutex.  */
32:	movq	16(%rsp), %rdi
	xorl	%esi, %esi
	callq	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	46f

	movq	8(%rsp), %rdi
	incq	total_seq(%rdi)
	incl	cond_futex(%rdi)
	addl	$(1 << nwaiters_shift), cond_nwaiters(%rdi)

	/* Get and store current wakeup_seq value.  */
	movq	8(%rsp), %rdi
	movq	wakeup_seq(%rdi), %r9
	movl	broadcast_seq(%rdi), %edx
	movq	%r9, 24(%rsp)
	movl	%edx, 4(%rsp)

38:	movl	cond_futex(%rdi), %r12d

	/* Unlock.  */
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	33f

.LcleanupSTART1:
34:	callq	__pthread_enable_asynccancel
	movl	%eax, (%rsp)

	movq	%r13, %r10
	movl	$FUTEX_WAIT_BITSET, %esi
	cmpq	$-1, dep_mutex(%rdi)
	je	60f

	movq	dep_mutex(%rdi), %r8
	/* Requeue to a non-robust PI mutex if the PI bit is set and
	the robust bit is not set.  */
	movl	MUTEX_KIND(%r8), %eax
	andl	$(ROBUST_BIT|PI_BIT), %eax
	cmpl	$PI_BIT, %eax
	jne	61f

	movl	$(FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG), %esi
	xorl	%eax, %eax
	/* The following only works like this because we only support
	   two clocks, represented using a single bit.  */
	testl	$1, cond_nwaiters(%rdi)
	movl	$FUTEX_CLOCK_REALTIME, %edx
	cmove	%edx, %eax
	orl	%eax, %esi
	movq	%r12, %rdx
	addq	$cond_futex, %rdi
	movl	$SYS_futex, %eax
	syscall

	movl	$1, %r15d
#ifdef __ASSUME_REQUEUE_PI
	jmp	62f
#else
	cmpq	$-4095, %rax
	jnae	62f

	subq	$cond_futex, %rdi
#endif

61:	movl	$(FUTEX_WAIT_BITSET|FUTEX_PRIVATE_FLAG), %esi
60:	xorl	%r15d, %r15d
	xorl	%eax, %eax
	/* The following only works like this because we only support
	   two clocks, represented using a single bit.  */
	testl	$1, cond_nwaiters(%rdi)
	movl	$FUTEX_CLOCK_REALTIME, %edx
	movl	$0xffffffff, %r9d
	cmove	%edx, %eax
	orl	%eax, %esi
	movq	%r12, %rdx
	addq	$cond_futex, %rdi
	movl	$SYS_futex, %eax
	syscall
62:	movq	%rax, %r14

	movl	(%rsp), %edi
	callq	__pthread_disable_asynccancel
.LcleanupEND1:

	/* Lock.  */
	movq	8(%rsp), %rdi
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jne	35f

36:	movl	broadcast_seq(%rdi), %edx

	movq	woken_seq(%rdi), %rax

	movq	wakeup_seq(%rdi), %r9

	cmpl	4(%rsp), %edx
	jne	53f

	cmpq	24(%rsp), %r9
	jbe	45f

	cmpq	%rax, %r9
	ja	39f

45:	cmpq	$-ETIMEDOUT, %r14
	jne	38b

99:	incq	wakeup_seq(%rdi)
	incl	cond_futex(%rdi)
	movl	$ETIMEDOUT, %r14d
	jmp	44f

53:	xorq	%r14, %r14
	jmp	54f

39:	xorq	%r14, %r14
44:	incq	woken_seq(%rdi)

54:	subl	$(1 << nwaiters_shift), cond_nwaiters(%rdi)

	/* Wake up a thread which wants to destroy the condvar object.  */
	cmpq	$0xffffffffffffffff, total_seq(%rdi)
	jne	55f
	movl	cond_nwaiters(%rdi), %eax
	andl	$~((1 << nwaiters_shift) - 1), %eax
	jne	55f

	addq	$cond_nwaiters, %rdi
	cmpq	$-1, dep_mutex-cond_nwaiters(%rdi)
	movl	$1, %edx
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
	subq	$cond_nwaiters, %rdi

55:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	40f

	/* If requeue_pi is used the kernel performs the locking of the
	   mutex. */
41:	movq	16(%rsp), %rdi
	testl	%r15d, %r15d
	jnz	64f

	callq	__pthread_mutex_cond_lock

63:	testq	%rax, %rax
	cmoveq	%r14, %rax

48:	addq	$FRAME_SIZE, %rsp
	cfi_adjust_cfa_offset(-FRAME_SIZE)
	popq	%r15
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r15)
	popq	%r14
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r14)
	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r12)

	retq

	cfi_restore_state

64:	callq	__pthread_mutex_cond_lock_adjust
	movq	%r14, %rax
	jmp	48b

	/* Initial locking failed.  */
31:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_lock_wait
	jmp	32b

	/* Unlock in loop requires wakeup.  */
33:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	jmp	34b

	/* Locking in loop failed.  */
35:
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
	jmp	36b

	/* Unlock after loop requires wakeup.  */
40:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	jmp	41b

	/* The initial unlocking of the mutex failed.  */
46:	movq	8(%rsp), %rdi
	movq	%rax, (%rsp)
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	47f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake

47:	movq	(%rsp), %rax
	jmp	48b


#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
.Lreltmo:
	xorl	%r15d, %r15d

	/* Get internal lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
# if cond_lock == 0
	cmpxchgl %esi, (%rdi)
# else
	cmpxchgl %esi, cond_lock(%rdi)
# endif
	jnz	1f

	/* Unlock the mutex.  */
2:	movq	16(%rsp), %rdi
	xorl	%esi, %esi
	callq	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	46b

	movq	8(%rsp), %rdi
	incq	total_seq(%rdi)
	incl	cond_futex(%rdi)
	addl	$(1 << nwaiters_shift), cond_nwaiters(%rdi)

	/* Get and store current wakeup_seq value.  */
	movq	8(%rsp), %rdi
	movq	wakeup_seq(%rdi), %r9
	movl	broadcast_seq(%rdi), %edx
	movq	%r9, 24(%rsp)
	movl	%edx, 4(%rsp)

	/* Get the current time.  */
8:
# ifdef __NR_clock_gettime
	/* Get the clock number.  Note that the field in the condvar
	   structure stores the number minus 1.  */
	movq	8(%rsp), %rdi
	movl	cond_nwaiters(%rdi), %edi
	andl	$((1 << nwaiters_shift) - 1), %edi
	/* Only clocks 0 and 1 are allowed so far.  Both are handled in the
	   kernel.  */
	leaq	32(%rsp), %rsi
26:	movl	$__NR_clock_gettime, %eax
	syscall
27:
#  ifndef __ASSUME_POSIX_TIMERS
	cmpq	$-ENOSYS, %rax
	je	19f
#  endif

	/* Compute relative timeout.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdx
	subq	32(%rsp), %rcx
	subq	40(%rsp), %rdx
# else
	leaq	24(%rsp), %rdi
	xorl	%esi, %esi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	40(%rsp), %rax
	movl	$1000, %edx
	mul	%rdx		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdx
	subq	32(%rsp), %rcx
	subq	%rax, %rdx
# endif
	jns	12f
	addq	$1000000000, %rdx
	decq	%rcx
12:	testq	%rcx, %rcx
	movq	8(%rsp), %rdi
	movq	$-ETIMEDOUT, %r14
	js	6f

	/* Store relative timeout.  */
21:	movq	%rcx, 32(%rsp)
	movq	%rdx, 40(%rsp)

	movl	cond_futex(%rdi), %r12d

	/* Unlock.  */
	LOCK
# if cond_lock == 0
	decl	(%rdi)
# else
	decl	cond_lock(%rdi)
# endif
	jne	3f

.LcleanupSTART2:
4:	callq	__pthread_enable_asynccancel
	movl	%eax, (%rsp)

	leaq	32(%rsp), %r10
	cmpq	$-1, dep_mutex(%rdi)
	movq	%r12, %rdx
# ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAIT, %eax
	movl	$(FUTEX_WAIT|FUTEX_PRIVATE_FLAG), %esi
	cmove	%eax, %esi
# else
	movl	$0, %eax
	movl	%fs:PRIVATE_FUTEX, %esi
	cmove	%eax, %esi
#  if FUTEX_WAIT != 0
	orl	$FUTEX_WAIT, %esi
#  endif
# endif
	addq	$cond_futex, %rdi
	movl	$SYS_futex, %eax
	syscall
	movq	%rax, %r14

	movl	(%rsp), %edi
	callq	__pthread_disable_asynccancel
.LcleanupEND2:

	/* Lock.  */
	movq	8(%rsp), %rdi
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
# if cond_lock == 0
	cmpxchgl %esi, (%rdi)
# else
	cmpxchgl %esi, cond_lock(%rdi)
# endif
	jne	5f

6:	movl	broadcast_seq(%rdi), %edx

	movq	woken_seq(%rdi), %rax

	movq	wakeup_seq(%rdi), %r9

	cmpl	4(%rsp), %edx
	jne	53b

	cmpq	24(%rsp), %r9
	jbe	15f

	cmpq	%rax, %r9
	ja	39b

15:	cmpq	$-ETIMEDOUT, %r14
	jne	8b

	jmp	99b

	/* Initial locking failed.  */
1:
# if cond_lock != 0
	addq	$cond_lock, %rdi
# endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_lock_wait
	jmp	2b

	/* Unlock in loop requires wakeup.  */
3:
# if cond_lock != 0
	addq	$cond_lock, %rdi
# endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	jmp	4b

	/* Locking in loop failed.  */
5:
# if cond_lock != 0
	addq	$cond_lock, %rdi
# endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_lock_wait
# if cond_lock != 0
	subq	$cond_lock, %rdi
# endif
	jmp	6b

# if defined __NR_clock_gettime && !defined __ASSUME_POSIX_TIMERS
	/* clock_gettime not available.  */
19:	leaq	32(%rsp), %rdi
	xorl	%esi, %esi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	40(%rsp), %rax
	movl	$1000, %edx
	mul	%rdx		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdx
	subq	32(%rsp), %rcx
	subq	%rax, %rdx
	jns	20f
	addq	$1000000000, %rdx
	decq	%rcx
20:	testq	%rcx, %rcx
	movq	8(%rsp), %rdi
	movq	$-ETIMEDOUT, %r14
	js	6b
	jmp	21b
# endif
#endif
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
weak_alias(__pthread_cond_timedwait, pthread_cond_timedwait)


	.align	16
	.type	__condvar_cleanup2, @function
__condvar_cleanup2:
	/* Stack frame:

	   rsp + 72
		    +--------------------------+
	   rsp + 64 | %r12                     |
		    +--------------------------+
	   rsp + 56 | %r13                     |
		    +--------------------------+
	   rsp + 48 | %r14                     |
		    +--------------------------+
	   rsp + 24 | unused                   |
		    +--------------------------+
	   rsp + 16 | mutex pointer            |
		    +--------------------------+
	   rsp +  8 | condvar pointer          |
		    +--------------------------+
	   rsp +  4 | old broadcast_seq value  |
		    +--------------------------+
	   rsp +  0 | old cancellation mode    |
		    +--------------------------+
	*/

	movq	%rax, 24(%rsp)

	/* Get internal lock.  */
	movq	8(%rsp), %rdi
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jz	1f

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

1:	movl	broadcast_seq(%rdi), %edx
	cmpl	4(%rsp), %edx
	jne	3f

	/* We increment the wakeup_seq counter only if it is lower than
	   total_seq.  If this is not the case the thread was woken and
	   then canceled.  In this case we ignore the signal.  */
	movq	total_seq(%rdi), %rax
	cmpq	wakeup_seq(%rdi), %rax
	jbe	6f
	incq	wakeup_seq(%rdi)
	incl	cond_futex(%rdi)
6:	incq	woken_seq(%rdi)

3:	subl	$(1 << nwaiters_shift), cond_nwaiters(%rdi)

	/* Wake up a thread which wants to destroy the condvar object.  */
	xorq	%r12, %r12
	cmpq	$0xffffffffffffffff, total_seq(%rdi)
	jne	4f
	movl	cond_nwaiters(%rdi), %eax
	andl	$~((1 << nwaiters_shift) - 1), %eax
	jne	4f

	cmpq	$-1, dep_mutex(%rdi)
	leaq	cond_nwaiters(%rdi), %rdi
	movl	$1, %edx
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
	subq	$cond_nwaiters, %rdi
	movl	$1, %r12d

4:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	je	2f
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake

	/* Wake up all waiters to make sure no signal gets lost.  */
2:	testq	%r12, %r12
	jnz	5f
	addq	$cond_futex, %rdi
	cmpq	$-1, dep_mutex-cond_futex(%rdi)
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

5:	movq	16(%rsp), %rdi
	callq	__pthread_mutex_cond_lock

	movq	24(%rsp), %rdi
	movq	FRAME_SIZE(%rsp), %r15
	movq	FRAME_SIZE+8(%rsp), %r14
	movq	FRAME_SIZE+16(%rsp), %r13
	movq	FRAME_SIZE+24(%rsp), %r12
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	cfi_endproc
	.size	__condvar_cleanup2, .-__condvar_cleanup2


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART1-.LSTARTCODE
	.uleb128 .LcleanupEND1-.LcleanupSTART1
	.uleb128 __condvar_cleanup2-.LSTARTCODE
	.uleb128  0
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	.uleb128 .LcleanupSTART2-.LSTARTCODE
	.uleb128 .LcleanupEND2-.LcleanupSTART2
	.uleb128 __condvar_cleanup2-.LSTARTCODE
	.uleb128  0
#endif
	.uleb128 .LcallUR-.LSTARTCODE
	.uleb128 .LENDCODE-.LcallUR
	.uleb128 0
	.uleb128  0
.Lcstend:


#ifdef SHARED
	.hidden	DW.ref.__gcc_personality_v0
	.weak	DW.ref.__gcc_personality_v0
	.section .gnu.linkonce.d.DW.ref.__gcc_personality_v0,"aw",@progbits
	.align	8
	.type	DW.ref.__gcc_personality_v0, @object
	.size	DW.ref.__gcc_personality_v0, 8
DW.ref.__gcc_personality_v0:
	.quad	__gcc_personality_v0
#endif
