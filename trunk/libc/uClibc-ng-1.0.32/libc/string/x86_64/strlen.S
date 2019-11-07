/* strlen(str) -- determine the length of the string STR.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   Based on i486 version contributed by Ulrich Drepper <drepper@redhat.com>.
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
ENTRY (strlen)
	movq %rdi, %rcx		/* Duplicate source pointer. */
	andl $7, %ecx		/* mask alignment bits */
	movq %rdi, %rax		/* duplicate destination.  */
	jz 1f			/* aligned => start loop */

	neg %ecx		/* We need to align to 8 bytes.  */
	addl $8,%ecx
	/* Search the first bytes directly.  */
0:	cmpb $0x0,(%rax)	/* is byte NUL? */
	je 2f			/* yes => return */
	incq %rax		/* increment pointer */
	decl %ecx
	jnz 0b

1:	movq $0xfefefefefefefeff,%r8 /* Save magic.  */

	/* Align loop.  */
	/* Next 3 insns are 10 bytes total, make sure we decode them in one go */
	.p2align 4,,10
4:
	/* Main Loop is unrolled 4 times.  */
	/* First unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found NUL => return pointer */

	/* Second unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found NUL => return pointer */

	/* Third unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz 3f			/* found NUL => return pointer */

	/* Fourth unroll.  */
	movq (%rax), %rcx	/* get double word (= 8 bytes) in question */
	addq $8,%rax		/* adjust pointer for next word */
	movq %r8, %rdx		/* magic value */
	addq %rcx, %rdx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc 3f			/* highest byte is NUL => return pointer */
	xorq %rcx, %rdx		/* (word+magic)^word */
	orq %r8, %rdx		/* set all non-carry bits */
	incq %rdx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jz 4b			/* no NUL found => continue loop */

	/* Align, it is a jump target.  */
	/* Next 3 insns are 8 bytes total, make sure we decode them in one go */
	.p2align 3,,8
3:
	subq $8,%rax		/* correct pointer increment.  */

	testb %cl, %cl		/* is first byte NUL? */
	jz 2f			/* yes => return */
	incq %rax		/* increment pointer */

	testb %ch, %ch		/* is second byte NUL? */
	jz 2f			/* yes => return */
	incq %rax		/* increment pointer */

	testl $0x00ff0000, %ecx /* is third byte NUL? */
	jz 2f			/* yes => return pointer */
	incq %rax		/* increment pointer */

	testl $0xff000000, %ecx /* is fourth byte NUL? */
	jz 2f			/* yes => return pointer */
	incq %rax		/* increment pointer */

	shrq $32, %rcx		/* look at other half.  */

	testb %cl, %cl		/* is first byte NUL? */
	jz 2f			/* yes => return */
	incq %rax		/* increment pointer */

	testb %ch, %ch		/* is second byte NUL? */
	jz 2f			/* yes => return */
	incq %rax		/* increment pointer */

	testl $0xff0000, %ecx	/* is third byte NUL? */
	jz 2f			/* yes => return pointer */
	incq %rax		/* increment pointer */
2:
	subq %rdi, %rax		/* compute difference to string start */
	ret
END (strlen)

libc_hidden_def(strlen)
