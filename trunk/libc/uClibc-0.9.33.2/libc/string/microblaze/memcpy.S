/*
 * Copyright (C) 2008-2009 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008-2009 PetaLogix
 * Copyright (C) 2008 Jim Law - Iris LP  All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Jim Law <jlaw@irispower.com>
 *
 * intended to replace:
 *	memcpy in memcpy.c and
 *	memmove in memmove.c
 * ... in arch/microblaze/lib
 *
 *
 * assly_fastcopy.S
 *
 * Attempt at quicker memcpy and memmove for MicroBlaze
 *	Input :	Operand1 in Reg r5 - destination address
 *		Operand2 in Reg r6 - source address
 *		Operand3 in Reg r7 - number of bytes to transfer
 *	Output: Result in Reg r3 - starting destinaition address
 *
 *
 * Explanation:
 *	Perform (possibly unaligned) copy of a block of memory
 *	between mem locations with size of xfer spec'd in bytes
 */

	.text
	.globl	memcpy
	.type  memcpy, @function
	.ent	memcpy

memcpy:
fast_memcpy_ascending:
	/* move d to return register as value of function */
	addi	r3, r5, 0

	addi	r4, r0, 4	/* n = 4 */
	cmpu	r4, r4, r7	/* n = c - n  (unsigned) */
	blti	r4, a_xfer_end	/* if n < 0, less than one word to transfer */

	/* transfer first 0~3 bytes to get aligned dest address */
	andi	r4, r5, 3		/* n = d & 3 */
	/* if zero, destination already aligned */
	beqi	r4, a_dalign_done
	/* n = 4 - n (yields 3, 2, 1 transfers for 1, 2, 3 addr offset) */
	rsubi	r4, r4, 4
	rsub	r7, r4, r7		/* c = c - n adjust c */

a_xfer_first_loop:
	/* if no bytes left to transfer, transfer the bulk */
	beqi	r4, a_dalign_done
	lbui	r11, r6, 0		/* h = *s */
	sbi	r11, r5, 0		/* *d = h */
	addi	r6, r6, 1		/* s++ */
	addi	r5, r5, 1		/* d++ */
	brid	a_xfer_first_loop	/* loop */
	addi	r4, r4, -1		/* n-- (IN DELAY SLOT) */

a_dalign_done:
	addi	r4, r0, 32		/* n = 32 */
	cmpu	r4, r4, r7		/* n = c - n  (unsigned) */
	/* if n < 0, less than one block to transfer */
	blti	r4, a_block_done

a_block_xfer:
	andi	r9, r6, 3		/* t1 = s & 3 */
	/* if temp == 0, everything is word-aligned */
	beqi	r9, a_word_xfer

a_block_unaligned:
	andi	r4, r7, 0xffffffe0	/* n = c & ~31 */
	rsub	r7, r4, r7		/* c = c - n */
	andi	r8, r6, 0xfffffffc	/* as = s & ~3 */
	add	r6, r6, r4		/* s = s + n */
	lwi	r11, r8, 0		/* h = *(as + 0) */

	addi	r9, r9, -1
	beqi	r9, a_block_u1		/* t1 was 1 => 1 byte offset */
	addi	r9, r9, -1
	beqi	r9, a_block_u2		/* t1 was 2 => 2 byte offset */

a_block_u3:
	bslli	r11, r11, 24	/* h = h << 24 */
a_bu3_loop:
	lwi	r12, r8, 4	/* v = *(as + 4) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 0	/* *(d + 0) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 8	/* v = *(as + 8) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 4	/* *(d + 4) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 12	/* v = *(as + 12) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 8	/* *(d + 8) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 16	/* v = *(as + 16) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 12	/* *(d + 12) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 20	/* v = *(as + 20) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 16	/* *(d + 16) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 24	/* v = *(as + 24) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 20	/* *(d + 20) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 28	/* v = *(as + 28) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 24	/* *(d + 24) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	lwi	r12, r8, 32	/* v = *(as + 32) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 28	/* *(d + 28) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	addi	r8, r8, 32	/* as = as + 32 */
	addi	r4, r4, -32	/* n = n - 32 */
	bneid	r4, a_bu3_loop	/* while (n) loop */
	addi	r5, r5, 32	/* d = d + 32 (IN DELAY SLOT) */
	bri	a_block_done

a_block_u1:
	bslli	r11, r11, 8	/* h = h << 8 */
a_bu1_loop:
	lwi	r12, r8, 4	/* v = *(as + 4) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 0	/* *(d + 0) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 8	/* v = *(as + 8) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 4	/* *(d + 4) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 12	/* v = *(as + 12) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 8	/* *(d + 8) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 16	/* v = *(as + 16) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 12	/* *(d + 12) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 20	/* v = *(as + 20) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 16	/* *(d + 16) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 24	/* v = *(as + 24) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 20	/* *(d + 20) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 28	/* v = *(as + 28) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 24	/* *(d + 24) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	lwi	r12, r8, 32	/* v = *(as + 32) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 28	/* *(d + 28) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	addi	r8, r8, 32	/* as = as + 32 */
	addi	r4, r4, -32	/* n = n - 32 */
	bneid	r4, a_bu1_loop	/* while (n) loop */
	addi	r5, r5, 32	/* d = d + 32 (IN DELAY SLOT) */
	bri	a_block_done

a_block_u2:
	bslli	r11, r11, 16	/* h = h << 16 */
a_bu2_loop:
	lwi	r12, r8, 4	/* v = *(as + 4) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 0	/* *(d + 0) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 8	/* v = *(as + 8) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 4	/* *(d + 4) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 12	/* v = *(as + 12) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 8	/* *(d + 8) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 16	/* v = *(as + 16) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 12	/* *(d + 12) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 20	/* v = *(as + 20) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 16	/* *(d + 16) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 24	/* v = *(as + 24) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 20	/* *(d + 20) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 28	/* v = *(as + 28) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 24	/* *(d + 24) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	lwi	r12, r8, 32	/* v = *(as + 32) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	swi	r9, r5, 28	/* *(d + 28) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	addi	r8, r8, 32	/* as = as + 32 */
	addi	r4, r4, -32	/* n = n - 32 */
	bneid	r4, a_bu2_loop	/* while (n) loop */
	addi	r5, r5, 32	/* d = d + 32 (IN DELAY SLOT) */

a_block_done:
	addi	r4, r0, 4	/* n = 4 */
	cmpu	r4, r4, r7	/* n = c - n  (unsigned) */
	blti	r4, a_xfer_end	/* if n < 0, less than one word to transfer */

a_word_xfer:
	andi	r4, r7, 0xfffffffc	/* n = c & ~3 */
	addi	r10, r0, 0		/* offset = 0 */

	andi	r9, r6, 3		/* t1 = s & 3 */
	/* if temp != 0, unaligned transfers needed */
	bnei	r9, a_word_unaligned

a_word_aligned:
	lw	r9, r6, r10		/* t1 = *(s+offset) */
	sw	r9, r5, r10		/* *(d+offset) = t1 */
	addi	r4, r4,-4		/* n-- */
	bneid	r4, a_word_aligned	/* loop */
	addi	r10, r10, 4		/* offset++ (IN DELAY SLOT) */

	bri	a_word_done

a_word_unaligned:
	andi	r8, r6, 0xfffffffc	/* as = s & ~3 */
	lwi	r11, r8, 0		/* h = *(as + 0) */
	addi	r8, r8, 4		/* as = as + 4 */

	addi	r9, r9, -1
	beqi	r9, a_word_u1		/* t1 was 1 => 1 byte offset */
	addi	r9, r9, -1
	beqi	r9, a_word_u2		/* t1 was 2 => 2 byte offset */

a_word_u3:
	bslli	r11, r11, 24	/* h = h << 24 */
a_wu3_loop:
	lw	r12, r8, r10	/* v = *(as + offset) */
	bsrli	r9, r12, 8	/* t1 = v >> 8 */
	or	r9, r11, r9	/* t1 = h | t1 */
	sw	r9, r5, r10	/* *(d + offset) = t1 */
	bslli	r11, r12, 24	/* h = v << 24 */
	addi	r4, r4,-4	/* n = n - 4 */
	bneid	r4, a_wu3_loop	/* while (n) loop */
	addi	r10, r10, 4	/* offset = ofset + 4 (IN DELAY SLOT) */

	bri	a_word_done

a_word_u1:
	bslli	r11, r11, 8	/* h = h << 8 */
a_wu1_loop:
	lw	r12, r8, r10	/* v = *(as + offset) */
	bsrli	r9, r12, 24	/* t1 = v >> 24 */
	or	r9, r11, r9	/* t1 = h | t1 */
	sw	r9, r5, r10	/* *(d + offset) = t1 */
	bslli	r11, r12, 8	/* h = v << 8 */
	addi	r4, r4,-4	/* n = n - 4 */
	bneid	r4, a_wu1_loop	/* while (n) loop */
	addi	r10, r10, 4	/* offset = ofset + 4 (IN DELAY SLOT) */

	bri	a_word_done

a_word_u2:
	bslli	r11, r11, 16	/* h = h << 16 */
a_wu2_loop:
	lw	r12, r8, r10	/* v = *(as + offset) */
	bsrli	r9, r12, 16	/* t1 = v >> 16 */
	or	r9, r11, r9	/* t1 = h | t1 */
	sw	r9, r5, r10	/* *(d + offset) = t1 */
	bslli	r11, r12, 16	/* h = v << 16 */
	addi	r4, r4,-4	/* n = n - 4 */
	bneid	r4, a_wu2_loop	/* while (n) loop */
	addi	r10, r10, 4	/* offset = ofset + 4 (IN DELAY SLOT) */

a_word_done:
	add	r5, r5, r10	/* d = d + offset */
	add	r6, r6, r10	/* s = s + offset */
	rsub	r7, r10, r7	/* c = c - offset */

a_xfer_end:
a_xfer_end_loop:
	beqi	r7, a_done		/* while (c) */
	lbui	r9, r6, 0		/* t1 = *s */
	addi	r6, r6, 1		/* s++ */
	sbi	r9, r5, 0		/* *d = t1 */
	addi	r7, r7, -1		/* c-- */
	brid	a_xfer_end_loop		/* loop */
	addi	r5, r5, 1		/* d++ (IN DELAY SLOT) */

a_done:
	rtsd	r15, 8
	nop

.size  memcpy, . - memcpy
.end memcpy
libc_hidden_def(memcpy)
