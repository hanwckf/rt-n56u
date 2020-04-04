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
#include <bits/kernel-features.h>
#include <tcb-offsets.h>
#include <lowlevellock.h>


	.comm	__fork_generation, 4, 4

	.text


	.globl	__pthread_once
	.type	__pthread_once,@function
	.protected	__pthread_once
	.align	16
__pthread_once:
.LSTARTCODE:
	cfi_startproc
	testl	$2, (%rdi)
	jz	1f
	xorl	%eax, %eax
	retq

	/* Preserve the function pointer.  */
1:	pushq	%rsi
	cfi_adjust_cfa_offset(8)
	xorq	%r10, %r10

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:	movl	(%rdi), %eax

5:	movl	%eax, %edx

	testl	$2, %eax
	jnz	4f

	andl	$3, %edx
	orl	__fork_generation(%rip), %edx
	orl	$1, %edx

	LOCK
	cmpxchgl %edx, (%rdi)
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
	movl	$FUTEX_WAIT|FUTEX_PRIVATE_FLAG, %esi
#else
# if FUTEX_WAIT == 0
	movl	%fs:PRIVATE_FUTEX, %esi
# else
	movl	$FUTEX_WAIT, %esi
	orl	%fs:PRIVATE_FUTEX, %esi
# endif
#endif
	movl	$SYS_futex, %eax
	syscall
	jmp	6b

	/* Preserve the pointer to the control variable.  */
3:	pushq	%rdi
	cfi_adjust_cfa_offset(8)
	pushq	%rdi
	cfi_adjust_cfa_offset(8)

.LcleanupSTART:
	callq	*16(%rsp)
.LcleanupEND:

	/* Get the control variable address back.  */
	popq	%rdi
	cfi_adjust_cfa_offset(-8)

	/* Sucessful run of the initializer.  Signal that we are done.  */
	LOCK
	incl	(%rdi)

	addq	$8, %rsp
	cfi_adjust_cfa_offset(-8)

	/* Wake up all other threads.  */
	movl	$0x7fffffff, %edx
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAKE|FUTEX_PRIVATE_FLAG, %esi
#else
	movl	$FUTEX_WAKE, %esi
	orl	%fs:PRIVATE_FUTEX, %esi
#endif
	movl	$SYS_futex, %eax
	syscall

4:	addq	$8, %rsp
	cfi_adjust_cfa_offset(-8)
	xorl	%eax, %eax
	retq
	.size	__pthread_once,.-__pthread_once


	.globl	__pthread_once_internal
__pthread_once_internal = __pthread_once

	.globl	pthread_once
pthread_once = __pthread_once


	.type	clear_once_control,@function
	.align	16
clear_once_control:
	cfi_adjust_cfa_offset(3 * 8)
	movq	(%rsp), %rdi
	movq	%rax, %r8
	movl	$0, (%rdi)

	movl	$0x7fffffff, %edx
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_WAKE|FUTEX_PRIVATE_FLAG, %esi
#else
	movl	$FUTEX_WAKE, %esi
	orl	%fs:PRIVATE_FUTEX, %esi
#endif
	movl	$SYS_futex, %eax
	syscall

	movq	%r8, %rdi
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	cfi_endproc
	.size	clear_once_control,.-clear_once_control


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 clear_once_control-.LSTARTCODE
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
