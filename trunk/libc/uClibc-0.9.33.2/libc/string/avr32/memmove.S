/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

#define dst r12
#define src r11
#define len r10

	.text
	.global memmove
	.type	memmove, @function
memmove:
	cp.w	src, dst
	brge	HIDDEN_JUMPTARGET(memcpy)

	add	dst, len
	add	src, len
	pref	src[-1]

	/*
	 * The rest is basically the same as in memcpy.S except that
	 * the direction is reversed.
	 */
	cp.w	len, 32
	brge	.Lmore_than_31

	sub	len, 1
	retlt	r12
1:	ld.ub	r8, --src
	st.b	--dst, r8
	sub	len, 1
	brge	1b
	retal	r12

.Lmore_than_31:
	pushm	r0-r7, lr

	/* Check alignment */
	mov	r8, src
	andl	r8, 31, COH
	brne	.Lunaligned_src
	mov	r8, r12
	andl	r8, 3, COH
	brne	.Lunaligned_dst

.Laligned_copy:
	sub	len, 32
	brlt	.Lless_than_32

1:	/* Copy 32 bytes at a time */
	sub	src, 32
	ldm	src, r0-r7
	sub	dst, 32
	sub	len, 32
	stm	dst, r0-r7
	brge	1b

.Lless_than_32:
	/* Copy 16 more bytes if possible */
	sub	len, -16
	brlt	.Lless_than_16
	sub	src, 16
	ldm	src, r0-r3
	sub	dst, 16
	sub	len, 16
	stm	dst, r0-r3

.Lless_than_16:
	/* Do the remaining as byte copies */
	sub	len, -16
	breq	2f
1:	ld.ub	r0, --src
	st.b	--dst, r0
	sub	len, 1
	brne	1b

2:	popm	r0-r7, pc

.Lunaligned_src:
	/* Make src cacheline-aligned. r8 = (src & 31) */
	sub	len, r8
1:	ld.ub	r0, --src
	st.b	--dst, r0
	sub	r8, 1
	brne	1b

	/* If dst is word-aligned, we're ready to go */
	pref	src[-4]
	mov	r8, 3
	tst	dst, r8
	breq	.Laligned_copy

.Lunaligned_dst:
	/* src is aligned, but dst is not. Expect bad performance */
	sub	len, 4
	brlt	2f
1:	ld.w	r0, --src
	st.w	--dst, r0
	sub	len, 4
	brge	1b

2:	neg	len
	add	pc, pc, len << 2
	.rept	3
	ld.ub	r0, --src
	st.b	--dst, r0
	.endr

	popm	r0-r7, pc
	.size	memmove, . - memmove

libc_hidden_def(memmove)
