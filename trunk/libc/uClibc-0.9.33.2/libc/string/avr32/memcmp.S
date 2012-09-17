/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

#include <features.h>

#define s1 r12
#define s2 r11
#define len r10

	.text
	.global memcmp
	.type	memcmp, @function
	.align	1
memcmp:
	sub	len, 4
	brlt	.Lless_than_4

1:	ld.w	r8, s1++
	ld.w	r9, s2++
	cp.w	r8, r9
	brne	.Lfound_word
	sub	len, 4
	brge	1b

.Lless_than_4:
	sub	len, -4
	reteq	0

1:	ld.ub	r8, s1++
	ld.ub	r9, s2++
	sub	r8, r9
	retne	r8
	sub	len, 1
	brgt	1b

	retal	0

.Lfound_word:
	mov	len, 4

2:	bfextu	r11, r9, 24, 8
	bfextu	r12, r8, 24, 8
	sub	r12, r11
	retne	r12
	lsl	r8, 8
	lsl	r9, 8
	sub	len, 1
	brne	2b
	retal	r12

	.size	memcmp, . - memcmp

libc_hidden_def(memcmp)
#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(memcmp,bcmp)
#endif
