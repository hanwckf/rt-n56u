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
#include <pthread-errnos.h>
#include <structsem.h>


	.text

	.globl	sem_wait
	.type	sem_wait,@function
	.align	16
sem_wait:
.LSTARTCODE:
	cfi_startproc

#if VALUE == 0
	movl	(%rdi), %eax
#else
	movl	VALUE(%rdi), %eax
#endif
2:	testl	%eax, %eax
	je	1f

	leal	-1(%rax), %edx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%rdi)
#else
	cmpxchgl %edx, VALUE(%rdi)
#endif
	jne	2b

	xorl	%eax, %eax
	retq

	/* This push is only needed to store the sem_t pointer for the
	   exception handler.  */
1:	pushq	%rdi
	cfi_adjust_cfa_offset(8)

	LOCK
	addq	$1, NWAITERS(%rdi)

.LcleanupSTART:
6:	call	__pthread_enable_asynccancel
	movl	%eax, %r8d

	xorq	%r10, %r10
	movl	$SYS_futex, %eax
#if FUTEX_WAIT == 0
	movl	PRIVATE(%rdi), %esi
#else
	movl	$FUTEX_WAIT, %esi
	orl	PRIVATE(%rdi), %esi
#endif
	xorl	%edx, %edx
	syscall
	movq	%rax, %rcx

	xchgq	%r8, %rdi
	call	__pthread_disable_asynccancel
.LcleanupEND:
	movq	%r8, %rdi

	testq	%rcx, %rcx
	je	3f
	cmpq	$-EWOULDBLOCK, %rcx
	jne	4f

3:
#if VALUE == 0
	movl	(%rdi), %eax
#else
	movl	VALUE(%rdi), %eax
#endif
5:	testl	%eax, %eax
	je	6b

	leal	-1(%rax), %edx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%rdi)
#else
	cmpxchgl %edx, VALUE(%rdi)
#endif
	jne	5b

	xorl	%eax, %eax

9:	LOCK
	subq	$1, NWAITERS(%rdi)

	leaq	8(%rsp), %rsp
	cfi_adjust_cfa_offset(-8)

	retq

	cfi_adjust_cfa_offset(8)
4:	negq	%rcx
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%ecx, %fs:(%rdx)
#else
# error "not supported.  %rcx and %rdi must be preserved"
	callq	__errno_location@plt
	movl	%ecx, (%rax)
#endif
	orl	$-1, %eax

	jmp 9b
	.size	sem_wait,.-sem_wait


	.type	sem_wait_cleanup,@function
sem_wait_cleanup:
	movq	(%rsp), %rdi
	LOCK
	subq	$1, NWAITERS(%rdi)
	movq	%rax, %rdi
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	cfi_endproc
	.size	sem_wait_cleanup,.-sem_wait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 sem_wait_cleanup-.LSTARTCODE
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
