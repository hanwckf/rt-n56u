/* Copyright (C) 2002, 2003, 2007 Free Software Foundation, Inc.
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

#include <unwindbuf.h>
#include <sysdep.h>
#include <bits/kernel-features.h>
#include <lowlevellock.h>
#include <tls.h>


	.comm	__fork_generation, 4, 4

	.text


	.globl	__pthread_once
	.type	__pthread_once,@function
	.protected	__pthread_once
	.align	16
	cfi_startproc
__pthread_once:
	movl	4(%esp), %ecx
	testl	$2, (%ecx)
	jz	1f
	xorl	%eax, %eax
	ret

1:	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (3, 0)
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (6, 0)
	movl	%ecx, %ebx
	xorl	%esi, %esi

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:	movl	(%ebx), %eax
#ifdef __PIC__
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
#endif

5:	movl	%eax, %edx

	testl	$2, %eax
	jnz	4f

	andl	$3, %edx
#ifdef __PIC__
	orl	__fork_generation@GOTOFF(%ecx), %edx
#else
	orl	__fork_generation, %edx
#endif
	orl	$1, %edx

	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	5b

	/* Check whether another thread already runs the initializer.  */
	testl	$1, %eax
	jz	3f	/* No -> do it.  */

	/* Check whether the initializer execution was interrupted
	   by a fork.  */
	xorl	%edx, %eax
	testl	$0xfffffffc, %eax
	jnz	3f	/* Different for generation -> run initializer.  */

	/* Somebody else got here first.  Wait.  */
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAIT|FUTEX_PRIVATE_FLAG, %ecx
#else
# if FUTEX_WAIT == 0
	movl	%gs:PRIVATE_FUTEX, %ecx
# else
	movl	$FUTEX_WAIT, %ecx
	orl	%gs:PRIVATE_FUTEX, %ecx
# endif
#endif
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	jmp	6b

3:	/* Call the initializer function after setting up the
	   cancellation handler.  Note that it is not possible here
	   to use the unwind-based cleanup handling.  This would require
	   that the user-provided function and all the code it calls
	   is compiled with exceptions.  Unfortunately this cannot be
	   guaranteed.  */
	subl	$UNWINDBUFSIZE+8, %esp
	cfi_adjust_cfa_offset (UNWINDBUFSIZE+8)
	movl	%ecx, %ebx		/* PIC register value.  */

	leal	8+UWJMPBUF(%esp), %eax
	movl	$0, 4(%esp)
	movl	%eax, (%esp)
	call	__sigsetjmp@PLT
	testl	%eax, %eax
	jne	7f

	leal	8(%esp), %eax
	call	HIDDEN_JUMPTARGET(__pthread_register_cancel)

	/* Call the user-provided initialization function.  */
	call	*24+UNWINDBUFSIZE(%esp)

	/* Pop the cleanup handler.  */
	leal	8(%esp), %eax
	call	HIDDEN_JUMPTARGET(__pthread_unregister_cancel)
	addl	$UNWINDBUFSIZE+8, %esp
	cfi_adjust_cfa_offset (-UNWINDBUFSIZE-8)

	/* Sucessful run of the initializer.  Signal that we are done.  */
	movl	12(%esp), %ebx
	LOCK
	addl	$1, (%ebx)

	/* Wake up all other threads.  */
	movl	$0x7fffffff, %edx
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAKE|FUTEX_PRIVATE_FLAG, %ecx
#else
	movl	$FUTEX_WAKE, %ecx
	orl	%gs:PRIVATE_FUTEX, %ecx
#endif
	movl	$SYS_futex, %eax
	ENTER_KERNEL

4:	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (6)
	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (3)
	xorl	%eax, %eax
	ret

7:	/* __sigsetjmp returned for the second time.  */
	movl	20+UNWINDBUFSIZE(%esp), %ebx
	cfi_adjust_cfa_offset (UNWINDBUFSIZE+16)
	cfi_offset (3, -8)
	cfi_offset (6, -12)
	movl	$0, (%ebx)

	movl	$0x7fffffff, %edx
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAKE|FUTEX_PRIVATE_FLAG, %ecx
#else
	movl	$FUTEX_WAKE, %ecx
	orl	%gs:PRIVATE_FUTEX, %ecx
#endif
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	leal	8(%esp), %eax
	call	HIDDEN_JUMPTARGET (__pthread_unwind_next)
	/* NOTREACHED */
	hlt
	cfi_endproc
	.size	__pthread_once,.-__pthread_once

	.globl	__pthread_once_internal
__pthread_once_internal = __pthread_once

	.globl	pthread_once
pthread_once = __pthread_once


#ifdef __PIC__
	.section .gnu.linkonce.t.__x86.get_pc_thunk.cx,"ax",@progbits
	.globl	__x86.get_pc_thunk.cx
	.hidden	__x86.get_pc_thunk.cx
	.type	__x86.get_pc_thunk.cx,@function
__x86.get_pc_thunk.cx:
	movl (%esp), %ecx;
	ret
	.size	__x86.get_pc_thunk.cx,.-__x86.get_pc_thunk.cx
#endif
