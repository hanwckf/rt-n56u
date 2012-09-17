/* Copyright (C) 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <features.h>
#define _ERRNO_H 1
#include <bits/errno.h>
#include <sys/syscall.h>

#define CLONE_VM	0x00000100
#define CLONE_THREAD	0x00010000

/* The userland implementation is:
   int clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg),
   the kernel entry is:
   int clone (long flags, void *child_stack).

   The parameters are passed in register and on the stack from userland:
   rdi: fn
   rsi: child_stack
   rdx:	flags
   rcx: arg
   r8d:	TID field in parent
   r9d: thread pointer
%esp+8:	TID field in child

   The kernel expects:
   rax: system call number
   rdi: flags
   rsi: child_stack
   rdx: TID field in parent
   r10: TID field in child
   r8:	thread pointer  */


.text
.global clone
.type   clone,%function
clone:
	/* Sanity check arguments.  */
	movq	$-EINVAL,%rax
	testq	%rdi,%rdi		/* no NULL function pointers */
	jz	__syscall_error
	testq	%rsi,%rsi		/* no NULL stack pointers */
	jz	__syscall_error

	/* Insert the argument onto the new stack.  */
	subq	$16,%rsi
	movq	%rcx,8(%rsi)

	/* Save the function pointer.  It will be popped off in the
	   child in the ebx frobbing below.  */
	movq	%rdi,0(%rsi)

	/* Do the system call.  */
	movq	%rdx, %rdi
	movq	%r8, %rdx
	movq	%r9, %r8
	movq	8(%rsp), %r10
	movl	$__NR_clone,%eax

	syscall

	testq	%rax,%rax
	jl	__syscall_error
	jz	.Lthread_start

.Lpseudo_end:
	ret

.Lthread_start:
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	xorl	%ebp, %ebp

#ifdef RESET_PID
	testq	$CLONE_THREAD, %rdi
	jne	1f
	testq	$CLONE_VM, %rdi
	movl	$-1, %eax
	jne	2f
	movl	$__NR_getpid, %eax
	syscall
2:	movl	%eax, %fs:PID
	movl	%eax, %fs:TID
1:
#endif

	/* Set up arguments for the function call.  */
	popq	%rax		/* Function to call.  */
	popq	%rdi		/* Argument.  */
	call	*%rax
	/* Call exit with return value from function call. */
	movq	%rax, %rdi
	movl	$__NR_exit, %eax
	syscall

.size clone,.-clone
weak_alias(clone, __clone)
