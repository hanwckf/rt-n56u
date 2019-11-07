/* Copyright (C) 2002, 2003, 2005, 2007 Free Software Foundation, Inc.
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

	.text

	.globl	sem_trywait
	.type	sem_trywait,@function
	.align	16
sem_trywait:
	movl	4(%esp), %ecx

	movl	(%ecx), %eax
2:	testl	%eax, %eax
	jz	1f

	leal	-1(%eax), %edx
	LOCK
	cmpxchgl %edx, (%ecx)
	jne	2b
	xorl	%eax, %eax
	ret

1:
#ifdef __PIC__
	call	__x86.get_pc_thunk.cx
#else
	movl	$3f, %ecx
3:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
#if USE___THREAD
# ifdef NO_TLS_DIRECT_SEG_REFS
	movl	errno@gotntpoff(%ecx), %edx
	addl	%gs:0, %edx
	movl	$EAGAIN, (%edx)
# else
	movl	errno@gotntpoff(%ecx), %edx
	movl	$EAGAIN, %gs:(%edx)
# endif
#else
	call	__errno_location@plt
	movl	$EAGAIN, (%eax)
#endif
	orl	$-1, %eax
	ret
	.size	sem_trywait,.-sem_trywait

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
