/* Copyright (C) 2002, 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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
#include <pthread-errnos.h>
#include <lowlevellock.h>
#include <lowlevelrobustlock.h>
#include <bits/kernel-features.h>
#include <tls.h>

	.text

#define FUTEX_WAITERS		0x80000000
#define FUTEX_OWNER_DIED	0x40000000

#ifdef __ASSUME_PRIVATE_FUTEX
# define LOAD_FUTEX_WAIT(reg) \
	xorl	$(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg
#else
# if FUTEX_WAIT == 0
#  define LOAD_FUTEX_WAIT(reg) \
	xorl	$FUTEX_PRIVATE_FLAG, reg ; \
	andl	%gs:PRIVATE_FUTEX, reg
# else
#  define LOAD_FUTEX_WAIT(reg) \
	xorl	$FUTEX_PRIVATE_FLAG, reg ; \
	andl	%gs:PRIVATE_FUTEX, reg ; \
	orl	$FUTEX_WAIT, reg
# endif
#endif

	.globl	__lll_robust_lock_wait
	.type	__lll_robust_lock_wait,@function
	.hidden	__lll_robust_lock_wait
	.align	16
__lll_robust_lock_wait:
	cfi_startproc
	pushl	%edx
	cfi_adjust_cfa_offset(4)
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	pushl	%esi
	cfi_adjust_cfa_offset(4)
	cfi_offset(%edx, -8)
	cfi_offset(%ebx, -12)
	cfi_offset(%esi, -16)

	movl	%edx, %ebx
	xorl	%esi, %esi	/* No timeout.  */
	LOAD_FUTEX_WAIT (%ecx)

4:	movl	%eax, %edx
	orl	$FUTEX_WAITERS, %edx

	testl	$FUTEX_OWNER_DIED, %eax
	jnz	3f

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	je	1f

	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	2f

1:	movl	$SYS_futex, %eax
	ENTER_KERNEL

	movl	(%ebx), %eax

2:	test	%eax, %eax
	jne	4b

	movl	%gs:TID, %edx
	orl	$FUTEX_WAITERS, %edx
	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	4b
	/* NB:	 %eax == 0 */

3:	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%esi)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	popl	%edx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edx)
	ret
	cfi_endproc
	.size	__lll_robust_lock_wait,.-__lll_robust_lock_wait


	.globl	__lll_robust_timedlock_wait
	.type	__lll_robust_timedlock_wait,@function
	.hidden	__lll_robust_timedlock_wait
	.align	16
__lll_robust_timedlock_wait:
	cfi_startproc
	/* Check for a valid timeout value.  */
	cmpl	$1000000000, 4(%edx)
	jae	3f

	pushl	%edi
	cfi_adjust_cfa_offset(4)
	pushl	%esi
	cfi_adjust_cfa_offset(4)
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	pushl	%ebp
	cfi_adjust_cfa_offset(4)
	cfi_offset(%edi, -8)
	cfi_offset(%esi, -12)
	cfi_offset(%ebx, -16)
	cfi_offset(%ebp, -20)

	/* Stack frame for the timespec and timeval structs.  */
	subl	$12, %esp
	cfi_adjust_cfa_offset(12)

	movl	%ecx, %ebp
	movl	%edx, %edi

1:	movl	%eax, 8(%esp)

	/* Get current time.  */
	movl	%esp, %ebx
	xorl	%ecx, %ecx
	movl	$__NR_gettimeofday, %eax
	ENTER_KERNEL

	/* Compute relative timeout.  */
	movl	4(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%edi), %ecx
	movl	4(%edi), %edx
	subl	(%esp), %ecx
	subl	%eax, %edx
	jns	4f
	addl	$1000000000, %edx
	subl	$1, %ecx
4:	testl	%ecx, %ecx
	js	8f		/* Time is already up.  */

	/* Store relative timeout.  */
	movl	%ecx, (%esp)
	movl	%edx, 4(%esp)

	movl	%ebp, %ebx

	movl	8(%esp), %edx
	movl	%edx, %eax
	orl	$FUTEX_WAITERS, %edx

	testl	$FUTEX_OWNER_DIED, %eax
	jnz	6f

	cmpl	%eax, %edx
	je	2f

	LOCK
	cmpxchgl %edx, (%ebx)
	movl	$0, %ecx	/* Must use mov to avoid changing cc.  */
	jnz	5f

2:
	/* Futex call.  */
	movl	%esp, %esi
	movl	20(%esp), %ecx
	LOAD_FUTEX_WAIT (%ecx)
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	movl	%eax, %ecx

	movl	(%ebx), %eax

5:	testl	%eax, %eax
	jne	7f

	movl	%gs:TID, %edx
	orl	$FUTEX_WAITERS, %edx
	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	7f

6:	addl	$12, %esp
	cfi_adjust_cfa_offset(-12)
	popl	%ebp
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebp)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%esi)
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edi)
	ret

3:	movl	$EINVAL, %eax
	ret

	cfi_adjust_cfa_offset(28)
	cfi_offset(%edi, -8)
	cfi_offset(%esi, -12)
	cfi_offset(%ebx, -16)
	cfi_offset(%ebp, -20)
	/* Check whether the time expired.  */
7:	cmpl	$-ETIMEDOUT, %ecx
	jne	1b

8:	movl	$ETIMEDOUT, %eax
	jmp	6b
	cfi_endproc
	.size	__lll_robust_timedlock_wait,.-__lll_robust_timedlock_wait
