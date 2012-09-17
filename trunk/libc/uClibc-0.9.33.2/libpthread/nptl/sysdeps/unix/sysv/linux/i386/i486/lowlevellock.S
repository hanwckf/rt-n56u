/* Copyright (C) 2002-2004, 2006, 2007, 2009 Free Software Foundation, Inc.
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
#include <pthread-errnos.h>
#include <bits/kernel-features.h>
#include <lowlevellock.h>
#include <tcb-offsets.h>

	.text

#ifdef __ASSUME_PRIVATE_FUTEX
# define LOAD_PRIVATE_FUTEX_WAIT(reg) \
	movl	$(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg
# define LOAD_PRIVATE_FUTEX_WAKE(reg) \
	movl	$(FUTEX_WAKE | FUTEX_PRIVATE_FLAG), reg
# define LOAD_FUTEX_WAIT(reg) \
	xorl	$(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg
# define LOAD_FUTEX_WAIT_ABS(reg) \
	xorl	$(FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME), reg
# define LOAD_FUTEX_WAKE(reg) \
	xorl	$(FUTEX_WAKE | FUTEX_PRIVATE_FLAG), reg
#else
# if FUTEX_WAIT == 0
#  define LOAD_PRIVATE_FUTEX_WAIT(reg) \
	movl	%gs:PRIVATE_FUTEX, reg
# else
#  define LOAD_PRIVATE_FUTEX_WAIT(reg) \
	movl	%gs:PRIVATE_FUTEX, reg ; \
	orl	$FUTEX_WAIT, reg
# endif
# define LOAD_PRIVATE_FUTEX_WAKE(reg) \
	movl	%gs:PRIVATE_FUTEX, reg ; \
	orl	$FUTEX_WAKE, reg
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
# define LOAD_FUTEX_WAIT_ABS(reg) \
	xorl	$FUTEX_PRIVATE_FLAG, reg ; \
	andl	%gs:PRIVATE_FUTEX, reg ; \
	orl	$FUTEX_WAIT_BITSET | FUTEX_CLOCK_REALTIME, reg
# define LOAD_FUTEX_WAKE(reg) \
	xorl	$FUTEX_PRIVATE_FLAG, reg ; \
	andl	%gs:PRIVATE_FUTEX, reg ; \
	orl	$FUTEX_WAKE, reg
#endif

	.globl	__lll_lock_wait_private
	.type	__lll_lock_wait_private,@function
	.hidden	__lll_lock_wait_private
	.align	16
__lll_lock_wait_private:
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

	movl	$2, %edx
	movl	%ecx, %ebx
	xorl	%esi, %esi	/* No timeout.  */
	LOAD_PRIVATE_FUTEX_WAIT (%ecx)

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	jne 2f

1:	movl	$SYS_futex, %eax
	ENTER_KERNEL

2:	movl	%edx, %eax
	xchgl	%eax, (%ebx)	/* NB:	 lock is implied */

	testl	%eax, %eax
	jnz	1b

	popl	%esi
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
	.size	__lll_lock_wait_private,.-__lll_lock_wait_private

#ifdef NOT_IN_libc
	.globl	__lll_lock_wait
	.type	__lll_lock_wait,@function
	.hidden	__lll_lock_wait
	.align	16
__lll_lock_wait:
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
	movl	$2, %edx
	xorl	%esi, %esi	/* No timeout.  */
	LOAD_FUTEX_WAIT (%ecx)

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	jne 2f

1:	movl	$SYS_futex, %eax
	ENTER_KERNEL

2:	movl	%edx, %eax
	xchgl	%eax, (%ebx)	/* NB:	 lock is implied */

	testl	%eax, %eax
	jnz	1b

	popl	%esi
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
	.size	__lll_lock_wait,.-__lll_lock_wait

	/*      %ecx: futex
		%esi: flags
		%edx: timeout
		%eax: futex value
	*/
	.globl	__lll_timedlock_wait
	.type	__lll_timedlock_wait,@function
	.hidden	__lll_timedlock_wait
	.align	16
__lll_timedlock_wait:
	cfi_startproc
	pushl	%ebp
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%ebp, 0)
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%ebx, 0)

# ifndef __ASSUME_FUTEX_CLOCK_REALTIME
#  ifdef __PIC__
	LOAD_PIC_REG (bx)
	cmpl	$0, __have_futex_clock_realtime@GOTOFF(%ebx)
#  else
	cmpl	$0, __have_futex_clock_realtime
#  endif
	je	.Lreltmo
# endif

	movl	%ecx, %ebx
	movl	%esi, %ecx
	movl	%edx, %esi
	movl	$0xffffffff, %ebp
	LOAD_FUTEX_WAIT_ABS (%ecx)

	movl	$2, %edx
	cmpl	%edx, %eax
	jne	2f

1:	movl	$SYS_futex, %eax
	movl	$2, %edx
	ENTER_KERNEL

2:	xchgl	%edx, (%ebx)	/* NB:   lock is implied */

	testl	%edx, %edx
	jz	3f

	cmpl	$-ETIMEDOUT, %eax
	je	4f
	cmpl	$-EINVAL, %eax
	jne	1b
4:	movl	%eax, %edx
	negl	%edx

3:	movl	%edx, %eax
7:	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	popl	%ebp
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebp)
	ret

# ifndef __ASSUME_FUTEX_CLOCK_REALTIME
.Lreltmo:
	/* Check for a valid timeout value.  */
	cmpl	$1000000000, 4(%edx)
	jae	3f

	pushl	%esi
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%esi, 0)
	pushl	%edi
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%edi, 0)

	/* Stack frame for the timespec and timeval structs.  */
	subl	$8, %esp
	cfi_adjust_cfa_offset(8)

	movl	%ecx, %ebp
	movl	%edx, %edi

	movl	$2, %edx
	xchgl	%edx, (%ebp)

	test	%edx, %edx
	je	6f

1:
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
	js	2f		/* Time is already up.  */

	/* Store relative timeout.  */
	movl	%ecx, (%esp)
	movl	%edx, 4(%esp)

	/* Futex call.  */
	movl	%ebp, %ebx
	movl	$2, %edx
	movl	%esp, %esi
	movl	16(%esp), %ecx
	LOAD_FUTEX_WAIT (%ecx)
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	/* NB: %edx == 2 */
	xchgl	%edx, (%ebp)

	testl	%edx, %edx
	je	6f

	cmpl	$-ETIMEDOUT, %eax
	jne	1b
2:	movl	$ETIMEDOUT, %edx

6:	addl	$8, %esp
	cfi_adjust_cfa_offset(-8)
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edi)
	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%esi)
7:	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	popl	%ebp
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebp)
	movl	%edx, %eax
	ret

3:	movl	$EINVAL, %edx
	jmp	7b
# endif
	cfi_endproc
	.size	__lll_timedlock_wait,.-__lll_timedlock_wait
#endif

	.globl	__lll_unlock_wake_private
	.type	__lll_unlock_wake_private,@function
	.hidden	__lll_unlock_wake_private
	.align	16
__lll_unlock_wake_private:
	cfi_startproc
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	pushl	%ecx
	cfi_adjust_cfa_offset(4)
	pushl	%edx
	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)
	cfi_offset(%ecx, -12)
	cfi_offset(%edx, -16)

	movl	%eax, %ebx
	movl	$0, (%eax)
	LOAD_PRIVATE_FUTEX_WAKE (%ecx)
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	popl	%edx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edx)
	popl	%ecx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ecx)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret
	cfi_endproc
	.size	__lll_unlock_wake_private,.-__lll_unlock_wake_private

#ifdef NOT_IN_libc
	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
	.align	16
__lll_unlock_wake:
	cfi_startproc
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	pushl	%ecx
	cfi_adjust_cfa_offset(4)
	pushl	%edx
	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)
	cfi_offset(%ecx, -12)
	cfi_offset(%edx, -16)

	movl	%eax, %ebx
	movl	$0, (%eax)
	LOAD_FUTEX_WAKE (%ecx)
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	popl	%edx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edx)
	popl	%ecx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ecx)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret
	cfi_endproc
	.size	__lll_unlock_wake,.-__lll_unlock_wake

	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
	.align	16
__lll_timedwait_tid:
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	movl	%eax, %ebp
	movl	%edx, %edi
	subl	$8, %esp

	/* Get current time.  */
2:	movl	%esp, %ebx
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
	jns	5f
	addl	$1000000000, %edx
	subl	$1, %ecx
5:	testl	%ecx, %ecx
	js	6f		/* Time is already up.  */

	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)

	movl	(%ebp), %edx
	testl	%edx, %edx
	jz	4f

	movl	%esp, %esi
	/* XXX The kernel so far uses global futex for the wakeup at
	   all times.  */
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%ebp, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	cmpl	$0, (%ebx)
	jne	1f
4:	xorl	%eax, %eax

3:	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

1:	cmpl	$-ETIMEDOUT, %eax
	jne	2b
6:	movl	$ETIMEDOUT, %eax
	jmp	3b
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
#endif
