/* Copyright (C) 2002,2003,2004,2006,2007,2009 Free Software Foundation, Inc.
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
#include <lowlevelcond.h>
#include <bits/kernel-features.h>
#include <pthread-pi-defines.h>
#include <pthread-errnos.h>
#include <tls.h>

	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.protected	__pthread_cond_broadcast
	.align	16
__pthread_cond_broadcast:
	cfi_startproc
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%ebx, 0)
	pushl	%esi
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%esi, 0)
	pushl	%edi
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%edi, 0)
	pushl	%ebp
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%ebp, 0)
	cfi_remember_state

	movl	20(%esp), %ebx

	/* Get internal lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, cond_lock(%ebx)
#endif
	jnz	1f

2:	addl	$cond_futex, %ebx
	movl	total_seq+4-cond_futex(%ebx), %eax
	movl	total_seq-cond_futex(%ebx), %ebp
	cmpl	wakeup_seq+4-cond_futex(%ebx), %eax
	ja	3f
	jb	4f
	cmpl	wakeup_seq-cond_futex(%ebx), %ebp
	jna	4f

	/* Cause all currently waiting threads to recognize they are
	   woken up.  */
3:	movl	%ebp, wakeup_seq-cond_futex(%ebx)
	movl	%eax, wakeup_seq-cond_futex+4(%ebx)
	movl	%ebp, woken_seq-cond_futex(%ebx)
	movl	%eax, woken_seq-cond_futex+4(%ebx)
	addl	%ebp, %ebp
	addl	$1, broadcast_seq-cond_futex(%ebx)
	movl	%ebp, (%ebx)

	/* Get the address of the mutex used.  */
	movl	dep_mutex-cond_futex(%ebx), %edi

	/* Unlock.  */
	LOCK
	subl	$1, cond_lock-cond_futex(%ebx)
	jne	7f

	/* Don't use requeue for pshared condvars.  */
8:	cmpl	$-1, %edi
	je	9f

	/* Do not use requeue for pshared condvars.  */
	testl	$PS_BIT, MUTEX_KIND(%edi)
	jne	9f

	/* Requeue to a non-robust PI mutex if the PI bit is set and
	   the robust bit is not set.  */
	movl	MUTEX_KIND(%edi), %eax
	andl	$(ROBUST_BIT|PI_BIT), %eax
	cmpl	$PI_BIT, %eax
	je	81f

	/* Wake up all threads.  */
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$(FUTEX_CMP_REQUEUE|FUTEX_PRIVATE_FLAG), %ecx
#else
	movl	%gs:PRIVATE_FUTEX, %ecx
	orl	$FUTEX_CMP_REQUEUE, %ecx
#endif
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %esi
	movl	$1, %edx
	/* Get the address of the futex involved.  */
# if MUTEX_FUTEX != 0
	addl	$MUTEX_FUTEX, %edi
# endif
/* FIXME: Until Ingo fixes 4G/4G vDSO, 6 arg syscalls are broken for sysenter.
	ENTER_KERNEL  */
	int	$0x80

	/* For any kind of error, which mainly is EAGAIN, we try again
	   with WAKE.  The general test also covers running on old
	   kernels.  */
	cmpl	$0xfffff001, %eax
	jae	9f

6:	xorl	%eax, %eax
	popl	%ebp
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebp)
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edi)
	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%esi)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret

	cfi_restore_state

81:	movl	$(FUTEX_CMP_REQUEUE_PI|FUTEX_PRIVATE_FLAG), %ecx
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %esi
	movl	$1, %edx
	/* Get the address of the futex involved.  */
# if MUTEX_FUTEX != 0
	addl	$MUTEX_FUTEX, %edi
# endif
	int	$0x80

	/* For any kind of error, which mainly is EAGAIN, we try again
	with WAKE.  The general test also covers running on old
	kernels.  */
	cmpl	$0xfffff001, %eax
	jb	6b
	jmp	9f

	/* Initial locking failed.  */
1:
#if cond_lock == 0
	movl	%ebx, %edx
#else
	leal	cond_lock(%ebx), %edx
#endif
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_lock_wait
	jmp	2b

	.align	16
	/* Unlock.  */
4:	LOCK
	subl	$1, cond_lock-cond_futex(%ebx)
	je	6b

	/* Unlock in loop requires wakeup.  */
5:	leal	cond_lock-cond_futex(%ebx), %eax
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex-cond_futex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_unlock_wake
	jmp	6b

	/* Unlock in loop requires wakeup.  */
7:	leal	cond_lock-cond_futex(%ebx), %eax
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex-cond_futex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_unlock_wake
	jmp	8b

9:	/* The futex requeue functionality is not available.  */
	movl	$0x7fffffff, %edx
#if FUTEX_PRIVATE_FLAG > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex-cond_futex(%ebx)
	sete	%cl
	subl	$1, %ecx
#ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %ecx
#else
	andl	%gs:PRIVATE_FUTEX, %ecx
#endif
	addl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	jmp	6b
	cfi_endproc
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
weak_alias(__pthread_cond_broadcast, pthread_cond_broadcast)
