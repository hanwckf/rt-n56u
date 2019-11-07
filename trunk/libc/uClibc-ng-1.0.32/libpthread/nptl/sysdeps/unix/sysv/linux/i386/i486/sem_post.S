/* Copyright (C) 2002, 2003, 2005, 2007, 2008 Free Software Foundation, Inc.
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
#include <structsem.h>
#include <lowlevellock.h>


	.text

	.globl	sem_post
	.type	sem_post,@function
	.align	16
sem_post:
	cfi_startproc
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)

	movl	8(%esp), %ebx

#if VALUE == 0
	movl	(%ebx), %eax
#else
	movl	VALUE(%ebx), %eax
#endif
0:	cmpl	$SEM_VALUE_MAX, %eax
	je	3f
	leal	1(%eax), %edx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, VALUE(%ebx)
#endif
	jnz	0b

	cmpl	$0, NWAITERS(%ebx)
	je	2f

	movl	$FUTEX_WAKE, %ecx
	orl	PRIVATE(%ebx), %ecx
	movl	$1, %edx
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	testl	%eax, %eax
	js	1f

2:	xorl	%eax, %eax
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret

	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)
1:
#ifdef __PIC__
	call	__x86.get_pc_thunk.bx
#else
	movl	$4f, %ebx
4:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#if USE___THREAD
# ifdef NO_TLS_DIRECT_SEG_REFS
	movl	errno@gotntpoff(%ebx), %edx
	addl	%gs:0, %edx
	movl	$EINVAL, (%edx)
# else
	movl	errno@gotntpoff(%ebx), %edx
	movl	$EINVAL, %gs:(%edx)
# endif
#else
	call	__errno_location@plt
	movl	$EINVAL, (%eax)
#endif

	orl	$-1, %eax
	popl	%ebx
	ret

3:
#ifdef __PIC__
	call	__x86.get_pc_thunk.bx
#else
	movl	$5f, %ebx
5:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#if USE___THREAD
# ifdef NO_TLS_DIRECT_SEG_REFS
	movl	errno@gotntpoff(%ebx), %edx
	addl	%gs:0, %edx
	movl	$EOVERFLOW, (%edx)
# else
	movl	errno@gotntpoff(%ebx), %edx
	movl	$EOVERFLOW, %gs:(%edx)
# endif
#else
	call	__errno_location@plt
	movl	$EOVERFLOW, (%eax)
#endif

	orl	$-1, %eax
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret
	cfi_endproc
	.size	sem_post,.-sem_post

#ifdef __PIC__
	.section .gnu.linkonce.t.__x86.get_pc_thunk.bx,"ax",@progbits
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx,@function
__x86.get_pc_thunk.bx:
	movl (%esp), %ebx;
	ret
	.size	__x86.get_pc_thunk.bx,.-__x86.get_pc_thunk.bx
#endif
