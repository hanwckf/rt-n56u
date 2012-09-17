/* memset/bzero -- set memory area to CH/0
   Optimized version for x86-64.
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>.

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

/* BEWARE: `#ifdef memset' means that memset is redefined as `bzero' */
#define BZERO_P (defined memset)

/* This is somehow experimental and could made dependend on the cache
   size.  */
#define LARGE $120000

        .text
#if defined __PIC__ && !defined NOT_IN_libc && defined __UCLIBC_HAS_FORTIFY__
ENTRY (__memset_chk)
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (__memset_chk)
#endif
ENTRY (memset)
#if BZERO_P
	mov	%rsi,%rdx	/* Adjust parameter.  */
	xorl	%esi,%esi	/* Fill with 0s.  */
#endif
	cmp	$0x7,%rdx	/* Check for small length.  */
	mov	%rdi,%rcx	/* Save ptr as return value.  */
	jbe	7f

#if BZERO_P
	mov	%rsi,%r8	/* Just copy 0.  */
#else
	/* Populate 8 bit data to full 64-bit.  */
	movabs	$0x0101010101010101,%r8
	movzbl	%sil,%eax
	imul	%rax,%r8
#endif
	test	$0x7,%edi	/* Check for alignment.  */
	jz	2f

	/* Next 3 insns are 9 bytes total, make sure we decode them in one go */
	.p2align 4,,9
1:
	/* Align ptr to 8 byte.  */
	mov	%sil,(%rcx)
	dec	%rdx
	inc	%rcx
	test	$0x7,%cl
	jnz	1b

2:	/* Check for really large regions.  */
	mov	%rdx,%rax
	shr	$0x6,%rax
	je	4f
	cmp	LARGE, %rdx
	jae	11f

	/* Next 3 insns are 11 bytes total, make sure we decode them in one go */
	.p2align 4,,11
3:
	/* Fill 64 bytes.  */
	mov	%r8,(%rcx)
	mov	%r8,0x8(%rcx)
	mov	%r8,0x10(%rcx)
	mov	%r8,0x18(%rcx)
	mov	%r8,0x20(%rcx)
	mov	%r8,0x28(%rcx)
	mov	%r8,0x30(%rcx)
	mov	%r8,0x38(%rcx)
	add	$0x40,%rcx
	dec	%rax
	jne	3b

4:	/* Fill final bytes.  */
	and	$0x3f,%edx
	mov	%rdx,%rax
	shr	$0x3,%rax
	je	6f

5:	/* First in chunks of 8 bytes.  */
	mov	%r8,(%rcx)
	add	$0x8,%rcx
	dec	%rax
	jne	5b
6:
	and	$0x7,%edx
7:
	test	%rdx,%rdx
	je	9f
8:	/* And finally as bytes (up to 7).  */
	mov	%sil,(%rcx)
	inc	%rcx
	dec	%rdx
	jne	8b
9:
#if BZERO_P
	/* nothing */
#else
	/* Load result (only if used as memset).  */
	mov	%rdi,%rax	/* start address of destination is result */
#endif
	retq

	/* Next 3 insns are 14 bytes total, make sure we decode them in one go */
	.p2align 4,,14
11:
	/* Fill 64 bytes without polluting the cache.  */
	/* We could use	movntdq %xmm0,(%rcx) here to further
	   speed up for large cases but let's not use XMM registers.  */
	movnti	%r8,(%rcx)
	movnti  %r8,0x8(%rcx)
	movnti  %r8,0x10(%rcx)
	movnti  %r8,0x18(%rcx)
	movnti  %r8,0x20(%rcx)
	movnti  %r8,0x28(%rcx)
	movnti  %r8,0x30(%rcx)
	movnti  %r8,0x38(%rcx)
	add	$0x40,%rcx
	dec	%rax
	jne	11b
	jmp	4b

END (memset)
#if !BZERO_P
libc_hidden_def(memset)
#endif

#if !BZERO_P && defined __PIC__ && !defined NOT_IN_libc && defined __UCLIBC_HAS_FORTIFY__
strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
#endif
