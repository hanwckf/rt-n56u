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
	.global strcmp
	.type	strcmp, @function
	.align	1
strcmp:
	mov	r8, 3
	tst	s1, r8
	brne	.Lunaligned_s1
	tst	s2, r8
	brne	.Lunaligned_s2

1:	ld.w	r8, s1++
	ld.w	r9, s2++
	cp.w	r8, r9
	brne	2f
	tnbz	r8
	brne	1b
	retal	0

2:	bfextu	r12, r8, 24, 8
	bfextu	r11, r9, 24, 8
	sub	r12, r11
	retne	r12
	cp.w	r11, 0
	reteq	0
	bfextu	r12, r8, 16, 8
	bfextu	r11, r9, 16, 8
	sub	r12, r11
	retne	r12
	cp.w	r11, 0
	reteq	0
	bfextu	r12, r8, 8, 8
	bfextu	r11, r9, 8, 8
	sub	r12, r11
	retne	r12
	cp.w	r11, 0
	reteq	0
	bfextu	r12, r8, 0, 8
	bfextu	r11, r9, 0, 8
	sub	r12, r11
	retal	r12

.Lunaligned_s1:
3:	tst	s1, r8
	breq	4f
	ld.ub	r10, s1++
	ld.ub	r9, s2++
	sub	r10, r9
	retne	r10
	cp.w	r9, 0
	brne	3b
	retal	r10

4:	tst	s2, r8
	breq	1b

.Lunaligned_s2:
	/*
	 * s1 and s2 can't both be aligned, and unaligned word loads
	 * can trigger spurious exceptions if we cross a page boundary.
	 * Do it the slow way...
	 */
1:	ld.ub	r8, s1++
	ld.ub	r9, s2++
	sub	r8, r9
	retne	r8
	cp.w	r9, 0
	brne	1b
	retal	0

	.size	strcmp, . - strcmp

libc_hidden_def(strcmp)
#ifndef __UCLIBC_HAS_LOCALE__
strong_alias(strcmp, strcoll)
libc_hidden_def(strcoll)
#endif
