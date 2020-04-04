/* Copyright (C) 2007, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <tls.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <bits/kernel-features.h>

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgetcpu	0xffffffffff600800


ENTRY (sched_getcpu)
	/* Align stack and create local variable for result.  */
	sub	$0x8, %rsp
	cfi_adjust_cfa_offset(8)

	movq	%rsp, %rdi
	xorl	%esi, %esi
	movl	$VGETCPU_CACHE_OFFSET, %edx
	addq	%fs:0, %rdx

#ifdef __NR_getcpu
	movl	$__NR_getcpu, %eax
	syscall
# ifndef __ASSUME_GETCPU_SYSCALL
	cmpq	$-ENOSYS, %rax
	jne	1f
# endif
#endif
#ifndef __ASSUME_GETCPU_SYSCALL
	movq	$VSYSCALL_ADDR_vgetcpu, %rax
	callq	*%rax
1:
#else
# ifndef __NR_getcpu
#  error "cannot happen"
# endif
#endif

	cmpq	$-4095, %rax
	jae	SYSCALL_ERROR_LABEL

	movl	(%rsp), %eax

L(pseudo_end):
	add	$0x8, %rsp
	cfi_adjust_cfa_offset(-8)
	ret
PSEUDO_END(sched_getcpu)
