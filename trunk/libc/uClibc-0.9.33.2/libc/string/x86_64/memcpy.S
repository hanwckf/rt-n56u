/* Highly optimized version for x86-64.
   Copyright (C) 1997, 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on i586 version contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include "_glibc_inc.h"

/* BEWARE: `#ifdef memcpy' means that memcpy is redefined as `mempcpy',
   and the return value is the byte after the last one copied in
   the destination. */
#define MEMPCPY_P (defined memcpy)

        .text
#if defined __PIC__ && !defined NOT_IN_libc && defined __UCLIBC_HAS_FORTIFY__
ENTRY (__memcpy_chk)
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (__memcpy_chk)
#endif
ENTRY (BP_SYM (memcpy))
	/* Cutoff for the big loop is a size of 32 bytes since otherwise
	   the loop will never be entered.  */
	cmpq	$32, %rdx
	movq	%rdx, %rcx
#if !MEMPCPY_P
	movq	%rdi, %r10	/* Save value. */
#endif

	/* We need this in any case.  */
	cld

	jbe	1f

	/* Align destination.  */
	movq	%rdi, %rax
	negq	%rax
	andq	$7, %rax
	subq	%rax, %rcx
	xchgq	%rax, %rcx

	rep; movsb

	movq	%rax, %rcx
	subq	$32, %rcx
	js	2f

	/* Next 3 insns are 11 bytes total, make sure we decode them in one go */
	.p2align 4,,11
3:
	/* Now correct the loop counter.  Please note that in the following
	   code the flags are not changed anymore.  */
	subq	$32, %rcx

	movq	(%rsi), %rax
	movq	8(%rsi), %rdx
	movq	16(%rsi), %r8
	movq	24(%rsi), %r9
	movq	%rax, (%rdi)
	movq	%rdx, 8(%rdi)
	movq	%r8, 16(%rdi)
	movq	%r9, 24(%rdi)

	leaq	32(%rsi), %rsi
	leaq	32(%rdi), %rdi

	jns	3b

	/* Correct extra loop counter modification.  */
2:	addq	$32, %rcx
1:	rep; movsb

#if MEMPCPY_P
	movq	%rdi, %rax		/* Set return value.  */
#else
	movq	%r10, %rax		/* Set return value.  */
	
#endif
	ret

END (BP_SYM (memcpy))
#if !MEMPCPY_P
libc_hidden_def(memcpy)
#endif
