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
#include <lowlevellock.h>
#include <pthread-errnos.h>

	.text

	.globl	sem_trywait
	.type	sem_trywait,@function
	.align	16
sem_trywait:
	movl	(%rdi), %eax
2:	testl	%eax, %eax
	jz	1f

	leal	-1(%rax), %edx
	LOCK
	cmpxchgl %edx, (%rdi)
	jne	2b

	xorl	%eax, %eax
	retq

1:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	$EAGAIN, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	$EAGAIN, (%rax)
#endif
	orl	$-1, %eax
	retq
	.size	sem_trywait,.-sem_trywait
