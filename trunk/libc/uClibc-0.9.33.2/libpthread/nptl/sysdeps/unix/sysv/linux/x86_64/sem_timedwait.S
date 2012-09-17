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
#include <bits/kernel-features.h>
#include <lowlevellock.h>
#include <pthread-errnos.h>
#include <structsem.h>


/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000

	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	16
sem_timedwait:
.LSTARTCODE:
	cfi_startproc
#if VALUE == 0
	movl	(%rdi), %eax
#else
	movl	VALUE(%rdi), %eax
#endif
2:	testl	%eax, %eax
	je	1f

	leaq	-1(%rax), %rdx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%rdi)
#else
	cmpxchgl %edx, VALUE(%rdi)
#endif
	jne	2b

	xorl	%eax, %eax
	retq

	/* Check whether the timeout value is valid.  */
1:	cmpq	$1000000000, 8(%rsi)
	jae	6f

#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
#  ifdef __PIC__
	cmpl	$0, __have_futex_clock_realtime@GOTOFF(%rip)
#  else
	cmpl	$0, __have_futex_clock_realtime
#  endif
	je	.Lreltmo
#endif

	/* This push is only needed to store the sem_t pointer for the
	   exception handler.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)

	movq	%rsi, %r10

	LOCK
	addq	$1, NWAITERS(%rdi)

.LcleanupSTART:
13:	call	__pthread_enable_asynccancel
	movl	%eax, %r8d

#if VALUE != 0
	leaq	VALUE(%rdi), %rdi
#endif
	movl	$0xffffffff, %r9d
	movl	$FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, %esi
	orl	PRIVATE(%rdi), %esi
	movl	$SYS_futex, %eax
	xorl	%edx, %edx
	syscall
	movq	%rax, %r9
#if VALUE != 0
	leaq	-VALUE(%rdi), %rdi
#endif

	xchgq	%r8, %rdi
	call	__pthread_disable_asynccancel
.LcleanupEND:
	movq	%r8, %rdi

	testq	%r9, %r9
	je	11f
	cmpq	$-EWOULDBLOCK, %r9
	jne	3f

11:
#if VALUE == 0
	movl	(%rdi), %eax
#else
	movl	VALUE(%rdi), %eax
#endif
14:	testl	%eax, %eax
	je	13b

	leaq	-1(%rax), %rcx
	LOCK
#if VALUE == 0
	cmpxchgl %ecx, (%rdi)
#else
	cmpxchgl %ecx, VALUE(%rdi)
#endif
	jne	14b

	xorl	%eax, %eax

15:	LOCK
	subq	$1, NWAITERS(%rdi)

	leaq	8(%rsp), %rsp
	cfi_adjust_cfa_offset(-8)
	retq

	cfi_adjust_cfa_offset(8)
3:	negq	%r9
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%r9d, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	%r9d, (%rax)
#endif

	orl	$-1, %eax
	jmp	15b

	cfi_adjust_cfa_offset(-8)
6:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	$EINVAL, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	$EINVAL, (%rax)
#endif

	orl	$-1, %eax

	retq

#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
.Lreltmo:
	pushq	%r12
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r12, 0)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r13, 0)
	pushq	%r14
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r14, 0)

#ifdef __ASSUME_FUTEX_CLOCK_REALTIME
# define STACKFRAME 8
#else
# define STACKFRAME 24
#endif
	subq	$STACKFRAME, %rsp
	cfi_adjust_cfa_offset(STACKFRAME)

	movq	%rdi, %r12
	movq	%rsi, %r13

	LOCK
	addq	$1, NWAITERS(%r12)

7:	xorl	%esi, %esi
	movq	%rsp, %rdi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movl	$1000, %edi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rdi
	movq	8(%r13), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	5f
	addq	$1000000000, %rsi
	decq	%rdi
5:	testq	%rdi, %rdi
	movl	$ETIMEDOUT, %r14d
	js	36f		/* Time is already up.  */

	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

.LcleanupSTART2:
	call	__pthread_enable_asynccancel
	movl	%eax, 16(%rsp)

	movq	%rsp, %r10
# if VALUE == 0
	movq	%r12, %rdi
# else
	leaq	VALUE(%r12), %rdi
# endif
# if FUTEX_WAIT == 0
	movl	PRIVATE(%rdi), %esi
# else
	movl	$FUTEX_WAIT, %esi
	orl	PRIVATE(%rdi), %esi
# endif
	movl	$SYS_futex, %eax
	xorl	%edx, %edx
	syscall
	movq	%rax, %r14

	movl	16(%rsp), %edi
	call	__pthread_disable_asynccancel
.LcleanupEND2:

	testq	%r14, %r14
	je	9f
	cmpq	$-EWOULDBLOCK, %r14
	jne	33f

9:
# if VALUE == 0
	movl	(%r12), %eax
# else
	movl	VALUE(%r12), %eax
# endif
8:	testl	%eax, %eax
	je	7b

	leaq	-1(%rax), %rcx
	LOCK
# if VALUE == 0
	cmpxchgl %ecx, (%r12)
# else
	cmpxchgl %ecx, VALUE(%r12)
# endif
	jne	8b

	xorl	%eax, %eax

45:	LOCK
	subq	$1, NWAITERS(%r12)

	addq	$STACKFRAME, %rsp
	cfi_adjust_cfa_offset(-STACKFRAME)
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

	cfi_adjust_cfa_offset(STACKFRAME + 3 * 8)
	cfi_rel_offset(%r12, STACKFRAME + 2 * 8)
	cfi_rel_offset(%r13, STACKFRAME + 1 * 8)
	cfi_rel_offset(%r14, STACKFRAME)
33:	negq	%r14
36:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%r14d, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	%r14d, (%rax)
#endif

	orl	$-1, %eax
	jmp	45b
#endif
	cfi_endproc
	.size	sem_timedwait,.-sem_timedwait


	.type	sem_timedwait_cleanup,@function
sem_timedwait_cleanup:
	cfi_startproc
	cfi_adjust_cfa_offset(8)

	movq	(%rsp), %rdi
	LOCK
	subq	$1, NWAITERS(%rdi)
	movq	%rax, %rdi
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	cfi_endproc
	.size	sem_timedwait_cleanup,.-sem_timedwait_cleanup


#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	.type	sem_timedwait_cleanup2,@function
sem_timedwait_cleanup2:
	cfi_startproc
	cfi_adjust_cfa_offset(STACKFRAME + 3 * 8)
	cfi_rel_offset(%r12, STACKFRAME + 2 * 8)
	cfi_rel_offset(%r13, STACKFRAME + 1 * 8)
	cfi_rel_offset(%r14, STACKFRAME)

	LOCK
	subq	$1, NWAITERS(%r12)
	movq	%rax, %rdi
	movq	STACKFRAME(%rsp), %r14
	movq	STACKFRAME+8(%rsp), %r13
	movq	STACKFRAME+16(%rsp), %r12
.LcallUR2:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE2:
	cfi_endproc
	.size	sem_timedwait_cleanup2,.-sem_timedwait_cleanup2
#endif


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 sem_timedwait_cleanup-.LSTARTCODE
	.uleb128  0
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	.uleb128 .LcleanupSTART2-.LSTARTCODE
	.uleb128 .LcleanupEND2-.LcleanupSTART2
	.uleb128 sem_timedwait_cleanup2-.LSTARTCODE
	.uleb128  0
#endif
	.uleb128 .LcallUR-.LSTARTCODE
	.uleb128 .LENDCODE-.LcallUR
	.uleb128 0
	.uleb128  0
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	.uleb128 .LcallUR2-.LSTARTCODE
	.uleb128 .LENDCODE2-.LcallUR2
	.uleb128 0
	.uleb128  0
#endif
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
