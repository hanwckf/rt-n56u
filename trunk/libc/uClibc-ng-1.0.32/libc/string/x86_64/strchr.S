/* strchr (str, ch) -- Return pointer to first occurrence of CH in STR.
   For AMD x86-64.
   Copyright (C) 2002, 2005 Free Software Foundation, Inc.
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

#include "_glibc_inc.h"

/* Seems to be unrolled too much */

	.text
ENTRY (BP_SYM (strchr))

	/* Before we start with the main loop we process single bytes
	   until the source pointer is aligned.  This has two reasons:
	   1. aligned 64-bit memory access is faster
	   and (more important)
	   2. we process in the main loop 64 bit in one step although
	      we don't know the end of the string.  But accessing at
	      8-byte alignment guarantees that we never access illegal
	      memory if this would not also be done by the trivial
	      implementation (this is because all processor inherent
	      boundaries are multiples of 8).  */

	movq	%rdi, %rdx
	andl	$7, %edx	/* Mask alignment bits  */
	movq	%rdi, %rax	/* duplicate destination.  */
	jz	1f		/* aligned => start loop */
	neg	%edx
	addl	$8, %edx	/* Align to 8 bytes.  */

	/* Search the first bytes directly.  */
0:	movb	(%rax), %cl	/* load byte  */
	cmpb	%cl,%sil	/* compare byte.  */
	je	6f		/* target found */
	testb	%cl,%cl		/* is byte NUL? */
	je	7f		/* yes => return NULL */
	incq	%rax		/* increment pointer */
	decl	%edx
	jnz	0b


1:
	/* At the moment %rsi contains C.  What we need for the
	   algorithm is C in all bytes of the register.  Avoid
	   operations on 16 bit words because these require an
	   prefix byte (and one more cycle).  */
	/* Populate 8 bit data to full 64-bit.  */
	movabs	$0x0101010101010101,%r9
	movzbl	%sil,%edx
	imul	%rdx,%r9

	movq $0xfefefefefefefeff, %r8 /* Save magic.  */

      /* We exit the loop if adding MAGIC_BITS to LONGWORD fails to
	 change any of the hole bits of LONGWORD.

	 1) Is this safe?  Will it catch all the zero bytes?
	 Suppose there is a byte with all zeros.  Any carry bits
	 propagating from its left will fall into the hole at its
	 least significant bit and stop.  Since there will be no
	 carry from its most significant bit, the LSB of the
	 byte to the left will be unchanged, and the zero will be
	 detected.

	 2) Is this worthwhile?  Will it ignore everything except
	 zero bytes?  Suppose every byte of QUARDWORD has a bit set
	 somewhere.  There will be a carry into bit 8.	If bit 8
	 is set, this will carry into bit 16.  If bit 8 is clear,
	 one of bits 9-15 must be set, so there will be a carry
	 into bit 16.  Similarly, there will be a carry into bit
	 24 tec..  If one of bits 54-63 is set, there will be a carry
	 into bit 64 (=carry flag), so all of the hole bits will
	 be changed.

	 3) But wait!  Aren't we looking for C, not zero?
	 Good point.  So what we do is XOR LONGWORD with a longword,
	 each of whose bytes is C.  This turns each byte that is C
	 into a zero.  */

	/* Next 3 insns are 10 bytes total, make sure we decode them in one go */
	.p2align 4,,10
4:
	/* Main Loop is unrolled 4 times.  */
	/* First unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	xorq %r9, %rcx		/* XOR with qword c|...|c => bytes of str == c
				   are now 0 */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found c => return pointer */

	/* The quadword we looked at does not contain the value we're looking
	   for.  Let's search now whether we have reached the end of the
	   string.  */
	xorq %r9, %rcx		/* restore original dword without reload */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 7f			/* highest byte is NUL => return NULL */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 7f			/* found NUL => return NULL */

	/* Second unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	xorq %r9, %rcx		/* XOR with qword c|...|c => bytes of str == c
				   are now 0 */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found c => return pointer */

	/* The quadword we looked at does not contain the value we're looking
	   for.  Let's search now whether we have reached the end of the
	   string.  */
	xorq %r9, %rcx		/* restore original dword without reload */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 7f			/* highest byte is NUL => return NULL */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 7f			/* found NUL => return NULL */
	/* Third unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	xorq %r9, %rcx		/* XOR with qword c|...|c => bytes of str == c
				   are now 0 */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found c => return pointer */

	/* The quadword we looked at does not contain the value we're looking
	   for.  Let's search now whether we have reached the end of the
	   string.  */
	xorq %r9, %rcx		/* restore original dword without reload */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 7f			/* highest byte is NUL => return NULL */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 7f			/* found NUL => return NULL */
	/* Fourth unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	xorq %r9, %rcx		/* XOR with qword c|...|c => bytes of str == c
				   are now 0 */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found c => return pointer */

	/* The quadword we looked at does not contain the value we're looking
	   for.  Let's search now whether we have reached the end of the
	   string.  */
	xorq %r9, %rcx		/* restore original dword without reload */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 7f			/* highest byte is NUL => return NULL */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jz 4b			/* no NUL found => restart loop */


7:	/* Return NULL.  */
	xorl %eax, %eax
	retq


	/* We now scan for the byte in which the character was matched.
	   But we have to take care of the case that a NUL char is
	   found before this in the dword.  Note that we XORed %rcx
	   with the byte we're looking for, therefore the tests below look
	   reversed.  */


	/* Align, it's a jump target.  */
	/* Next 3 insns are 9 bytes total, make sure we decode them in one go */
	.p2align 4,,9
3:
	movq	%r9,%rdx	/* move to %rdx so that we can access bytes */
	subq	$8,%rax		/* correct pointer increment.  */
	testb %cl, %cl		/* is first byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %cl		/* is first byte NUL? */
	je 7b			/* yes => return NULL */
	incq %rax		/* increment pointer */

	testb %ch, %ch		/* is second byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %ch		/* is second byte NUL? */
	je 7b			/* yes => return NULL? */
	incq %rax		/* increment pointer */

	shrq $16, %rcx		/* make upper bytes accessible */
	testb %cl, %cl		/* is third byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %cl		/* is third byte NUL? */
	je 7b			/* yes => return NULL */
	incq %rax		/* increment pointer */

	testb %ch, %ch		/* is fourth byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %ch		/* is fourth byte NUL? */
	je 7b			/* yes => return NULL? */
	incq %rax		/* increment pointer */

	shrq $16, %rcx		/* make upper bytes accessible */
	testb %cl, %cl		/* is fifth byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %cl		/* is fifth byte NUL? */
	je 7b			/* yes => return NULL */
	incq %rax		/* increment pointer */

	testb %ch, %ch		/* is sixth byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %ch		/* is sixth byte NUL? */
	je 7b			/* yes => return NULL? */
	incq %rax		/* increment pointer */

	shrq $16, %rcx		/* make upper bytes accessible */
	testb %cl, %cl		/* is seventh byte C? */
	jz 6f			/* yes => return pointer */
	cmpb %dl, %cl		/* is seventh byte NUL? */
	je 7b			/* yes => return NULL */

	/* It must be in the eigth byte and it cannot be NUL.  */
	incq %rax

6:
	/* nop - huh?? */
	retq
END (BP_SYM (strchr))

libc_hidden_def(strchr)
#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias (BP_SYM (strchr), BP_SYM (index))
#endif
