/* Copyright (C) 2002-2007, 2009 Free Software Foundation, Inc.
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
#include <tcb-offsets.h>
#include <pthread-pi-defines.h>

#include <bits/kernel-features.h>


	.text

/* int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)  */
	.globl	__pthread_cond_wait
	.type	__pthread_cond_wait, @function
	.protected	__pthread_cond_wait
	.align	16
__pthread_cond_wait:
.LSTARTCODE:
	cfi_startproc

#define FRAME_SIZE 32
	leaq	-FRAME_SIZE(%rsp), %rsp
	cfi_adjust_cfa_offset(FRAME_SIZE)

	/* Stack frame:

	   rsp + 32
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

	je	15f
	movq	%rsi, dep_mutex(%rdi)

	/* Get internal lock.  */
15:	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jne	1f

	/* Unlock the mutex.  */
2:	movq	16(%rsp), %rdi
	xorl	%esi, %esi
	callq	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	12f

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

	/* Unlock.  */
8:	movl	cond_futex(%rdi), %edx
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	3f

.LcleanupSTART:
4:	callq	__pthread_enable_asynccancel
	movl	%eax, (%rsp)

	xorq	%r10, %r10
	cmpq	$-1, dep_mutex(%rdi)
	leaq	cond_futex(%rdi), %rdi
	movl	$FUTEX_WAIT, %esi
	je	60f

	movq	dep_mutex-cond_futex(%rdi), %r8
	/* Requeue to a non-robust PI mutex if the PI bit is set and
	the robust bit is not set.  */
	movl	MUTEX_KIND(%r8), %eax
	andl	$(ROBUST_BIT|PI_BIT), %eax
	cmpl	$PI_BIT, %eax
	jne	61f

	movl	$(FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG), %esi
	movl	$SYS_futex, %eax
	syscall

	movl	$1, %r8d
#ifdef __ASSUME_REQUEUE_PI
	jmp	62f
#else
	cmpq	$-4095, %rax
	jnae	62f

# ifndef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAIT, %esi
# endif
#endif

61:
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$(FUTEX_WAIT|FUTEX_PRIVATE_FLAG), %esi
#else
	orl	%fs:PRIVATE_FUTEX, %esi
#endif
60:	xorl	%r8d, %r8d
	movl	$SYS_futex, %eax
	syscall

62:	movl	(%rsp), %edi
	callq	__pthread_disable_asynccancel
.LcleanupEND:

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
	jnz	5f

6:	movl	broadcast_seq(%rdi), %edx

	movq	woken_seq(%rdi), %rax

	movq	wakeup_seq(%rdi), %r9

	cmpl	4(%rsp), %edx
	jne	16f

	cmpq	24(%rsp), %r9
	jbe	8b

	cmpq	%rax, %r9
	jna	8b

	incq	woken_seq(%rdi)

	/* Unlock */
16:	subl	$(1 << nwaiters_shift), cond_nwaiters(%rdi)

	/* Wake up a thread which wants to destroy the condvar object.  */
	cmpq	$0xffffffffffffffff, total_seq(%rdi)
	jne	17f
	movl	cond_nwaiters(%rdi), %eax
	andl	$~((1 << nwaiters_shift) - 1), %eax
	jne	17f

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

17:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	10f

	/* If requeue_pi is used the kernel performs the locking of the
	   mutex. */
11:	movq	16(%rsp), %rdi
	testl	%r8d, %r8d
	jnz	18f

	callq	__pthread_mutex_cond_lock

14:	leaq	FRAME_SIZE(%rsp), %rsp
	cfi_adjust_cfa_offset(-FRAME_SIZE)

	/* We return the result of the mutex_lock operation.  */
	retq

	cfi_adjust_cfa_offset(FRAME_SIZE)

18:	callq	__pthread_mutex_cond_lock_adjust
	xorl	%eax, %eax
	jmp	14b

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
	jmp	2b

	/* Unlock in loop requires wakeup.  */
3:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	/* The call preserves %rdx.  */
	callq	__lll_unlock_wake
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	4b

	/* Locking in loop failed.  */
5:
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
	jmp	6b

	/* Unlock after loop requires wakeup.  */
10:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
12:	movq	%rax, %r10
	movq	8(%rsp), %rdi
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	je	13f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	cmpq	$-1, dep_mutex-cond_lock(%rdi)
	movl	$LLL_PRIVATE, %eax
	movl	$LLL_SHARED, %esi
	cmovne	%eax, %esi
	callq	__lll_unlock_wake

13:	movq	%r10, %rax
	jmp	14b
	.size	__pthread_cond_wait, .-__pthread_cond_wait
weak_alias(__pthread_cond_wait, pthread_cond_wait)


	.align	16
	.type	__condvar_cleanup1, @function
	.globl	__condvar_cleanup1
	.hidden	__condvar_cleanup1
__condvar_cleanup1:
	/* Stack frame:

	   rsp + 32
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
	xorl	%ecx, %ecx
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
	movl	$1, %ecx

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
	/* The call preserves %rcx.  */
	callq	__lll_unlock_wake

	/* Wake up all waiters to make sure no signal gets lost.  */
2:	testl	%ecx, %ecx
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
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	cfi_endproc
	.size	__condvar_cleanup1, .-__condvar_cleanup1


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 __condvar_cleanup1-.LSTARTCODE
	.uleb128  0
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
