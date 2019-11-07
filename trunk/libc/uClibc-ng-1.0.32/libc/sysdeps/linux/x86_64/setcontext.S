/* Install given context.
   Copyright (C) 2002-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2002.

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

#include "ucontext_i.h"


/*  int __setcontext (const ucontext_t *ucp)

  Restores the machine context in UCP and thereby resumes execution
  in that context.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to restore anything
  other than the PRESERVED state.  */

ENTRY(__setcontext)
	/* Save argument since syscall will destroy it.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)

	/* Set the signal mask with
	   rt_sigprocmask (SIG_SETMASK, mask, NULL, _NSIG/8).  */
	leaq	oSIGMASK(%rdi), %rsi
	xorl	%edx, %edx
	movl	$SIG_SETMASK, %edi
	movl	$_NSIG8,%r10d
	movl	$__NR_rt_sigprocmask, %eax
	syscall
	popq	%rdi			/* Reload %rdi, adjust stack.  */
	cfi_adjust_cfa_offset(-8)
	cmpq	$-4095, %rax		/* Check %rax for error.  */
	jae	SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */

	/* Restore the floating-point context.  Not the registers, only the
	   rest.  */
	movq	oFPREGS(%rdi), %rcx
	fldenv	(%rcx)
	ldmxcsr oMXCSR(%rdi)


	/* Load the new stack pointer, the preserved registers and
	   registers used for passing args.  */
	cfi_def_cfa(%rdi, 0)
	cfi_offset(%rbx,oRBX)
	cfi_offset(%rbp,oRBP)
	cfi_offset(%r12,oR12)
	cfi_offset(%r13,oR13)
	cfi_offset(%r14,oR14)
	cfi_offset(%r15,oR15)
	cfi_offset(%rsp,oRSP)
	cfi_offset(%rip,oRIP)

	movq	oRSP(%rdi), %rsp
	movq	oRBX(%rdi), %rbx
	movq	oRBP(%rdi), %rbp
	movq	oR12(%rdi), %r12
	movq	oR13(%rdi), %r13
	movq	oR14(%rdi), %r14
	movq	oR15(%rdi), %r15

	/* The following ret should return to the address set with
	getcontext.  Therefore push the address on the stack.  */
	movq	oRIP(%rdi), %rcx
	pushq	%rcx

	movq	oRSI(%rdi), %rsi
	movq	oRDX(%rdi), %rdx
	movq	oRCX(%rdi), %rcx
	movq	oR8(%rdi), %r8
	movq	oR9(%rdi), %r9

	/* Setup finally  %rdi.  */
	movq	oRDI(%rdi), %rdi

	/* End FDE here, we fall into another context.  */
	cfi_endproc
	cfi_startproc

	/* Clear rax to indicate success.  */
	xorl	%eax, %eax
L(pseudo_end):
	ret
PSEUDO_END(__setcontext)

weak_alias (__setcontext, setcontext)
