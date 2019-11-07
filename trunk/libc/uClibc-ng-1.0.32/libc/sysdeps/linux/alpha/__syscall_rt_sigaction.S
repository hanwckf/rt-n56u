/* Copyright (C) 1998, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@cygnus.com>, 1998

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

#include <features.h>
#include <sys/syscall.h>
#include <sys/regdef.h>

/* On Alpha we desparately want to avoid having to issue an imb.  Ordinarily
   the kernel would have to issue one after setting up the signal return
   stack, but the Linux rt_sigaction syscall is prepared to accept a pointer
   to the sigreturn syscall, instead of inlining it on the stack.

   This just about halves signal delivery time.  */

	.text

.globl __syscall_rt_sigaction
#ifndef __UCLIBC_HAS_THREADS_NATIVE__
.hidden __syscall_rt_sigaction
#endif
.align 4
.ent __syscall_rt_sigaction, 0
__syscall_rt_sigaction:
	.frame	sp,0,ra,0
	ldgp	gp,0(pv)
	.prologue 1

	beq	a1, 0f
	ldl	t0, 8(a1)			# sa_flags

	/* The unwinder will subtract one from the return address when
	   attempting to find the call instruction that led us here.
	   Since we didn't get here via a normal call, if we do nothing
	   we would pick up the wrong symbol and the wrong FDE.  Account
	   for this by adding a nop to the start of the function and 
	   then skipping it here by adding 4.  */
	ldah	a4, __syscall_sigreturn+4(gp)		!gprelhigh
	ldah	t1, __syscall_rt_sigreturn+4(gp)	!gprelhigh
	lda	a4, __syscall_sigreturn+4(a4)		!gprellow
	lda	t1, __syscall_rt_sigreturn+4(t1)	!gprellow
	and	t0, 0x40, t0				# SA_SIGINFO
	cmovne	t0, t1, a4

0:	ldi	v0, __NR_rt_sigaction
	callsys
	bne	a3, $error
	ret

$error:
	jmp	zero,__syscall_error

.end __syscall_rt_sigaction

__syscall_sigreturn:
	nop
	mov	sp, a0
	ldi	v0, __NR_sigreturn
	callsys
	.size	__syscall_sigreturn, .-__syscall_sigreturn
	.type	__syscall_sigreturn, @function

__syscall_rt_sigreturn:
	nop
	mov	sp,a0
	ldi	v0,__NR_rt_sigreturn
	callsys
	.size	__syscall_rt_sigreturn, .-__syscall_rt_sigreturn
	.type	__syscall_rt_sigreturn, @function
