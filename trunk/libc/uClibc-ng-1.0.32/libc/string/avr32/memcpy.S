/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

/* Don't use r12 as dst since we must return it unmodified */
#define dst r9
#define src r11
#define len r10

	.text
	.global	memcpy
	.type	memcpy, @function
memcpy:
	pref	src[0]
	mov	dst, r12

	/* If we have less than 32 bytes, don't do anything fancy */
	cp.w	len, 32
	brge	.Lmore_than_31

	sub	len, 1
	retlt	r12
1:	ld.ub	r8, src++
	st.b	dst++, r8
	sub	len, 1
	brge	1b
	retal	r12

.Lmore_than_31:
	pushm	r0-r7, lr

	/* Check alignment */
	mov	r8, src
	andl	r8, 31, COH
	brne	.Lunaligned_src
	mov	r8, dst
	andl	r8, 3, COH
	brne	.Lunaligned_dst

.Laligned_copy:
	sub	len, 32
	brlt	.Lless_than_32

1:	/* Copy 32 bytes at a time */
	ldm	src, r0-r7
	sub	src, -32
	stm	dst, r0-r7
	sub	dst, -32
	sub	len, 32
	brge	1b

.Lless_than_32:
	/* Copy 16 more bytes if possible */
	sub	len, -16
	brlt	.Lless_than_16
	ldm	src, r0-r3
	sub	src, -16
	sub	len, 16
	stm	dst, r0-r3
	sub	dst, -16

.Lless_than_16:
	/* Do the remaining as byte copies */
	neg	len
	add	pc, pc, len << 2
	.rept	15
	ld.ub	r0, src++
	st.b	dst++, r0
	.endr

	popm	r0-r7, pc

.Lunaligned_src:
	/* Make src cacheline-aligned. r8 = (src & 31) */
	rsub	r8, r8, 32
	sub	len, r8
1:	ld.ub	r0, src++
	st.b	dst++, r0
	sub	r8, 1
	brne	1b

	/* If dst is word-aligned, we're ready to go */
	pref	src[0]
	mov	r8, 3
	tst	dst, r8
	breq	.Laligned_copy

.Lunaligned_dst:
	/* src is aligned, but dst is not. Expect bad performance */
	sub	len, 4
	brlt	2f
1:	ld.w	r0, src++
	st.w	dst++, r0
	sub	len, 4
	brge	1b

2:	neg	len
	add	pc, pc, len << 2
	.rept	3
	ld.ub	r0, src++
	st.b	dst++, r0
	.endr

	popm	r0-r7, pc
	.size	memcpy, . - memcpy

libc_hidden_def(memcpy)
