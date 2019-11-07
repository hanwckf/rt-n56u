/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

#include <features.h>

#define str r12

	.text
	.global strlen
	.type	strlen, @function
strlen:
	mov	r11, r12

	mov	r9, str
	andl	r9, 3, COH
	brne	.Lunaligned_str

1:	ld.w	r8, str++
	tnbz	r8
	brne	1b

	sub	r12, r11
	bfextu	r9, r8, 24, 8
	cp.w	r9, 0
	subeq	r12, 4
	reteq	r12
	bfextu	r9, r8, 16, 8
	cp.w	r9, 0
	subeq	r12, 3
	reteq	r12
	bfextu	r9, r8, 8, 8
	cp.w	r9, 0
	subeq	r12, 2
	reteq	r12
	sub	r12, 1
	retal	r12

.Lunaligned_str:
	add	pc, pc, r9 << 3
	sub	r0, r0, 0	/* 4-byte nop */
	ld.ub	r8, str++
	sub	r8, r8, 0
	breq	1f
	ld.ub	r8, str++
	sub	r8, r8, 0
	breq	1f
	ld.ub	r8, str++
	sub	r8, r8, 0
	brne	1b

1:	sub	r12, 1
	sub	r12, r11
	retal	r12

	.size	strlen, . - strlen

libc_hidden_def(strlen)
