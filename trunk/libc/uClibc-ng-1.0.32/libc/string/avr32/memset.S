/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

#include <features.h>

#define s r12
#define c r11
#define n r10

	.text
	.global memset
	.type	memset, @function

	.align	1
memset:
	cp.w	n, 32
	mov	r9, s
	brge	.Llarge_memset

	sub	n, 1
	retlt	s
1:	st.b	s++, c
	sub	n, 1
	brge	1b

	retal	r9

.Llarge_memset:
	mov	r8, r11
	mov	r11, 3
	bfins	r8, r8, 8, 8
	bfins	r8, r8, 16, 16
	tst	s, r11
	breq	2f

1:	st.b	s++, r8
	sub	n, 1
	tst	s, r11
	brne	1b

2:	mov	r11, r9
	mov	r9, r8
	sub	n, 8

3:	st.d	s++, r8
	sub	n, 8
	brge	3b

	/* If we are done, n == -8 and we'll skip all st.b insns below */
	neg	n
	lsl	n, 1
	add	pc, n
	.rept	7
	st.b	s++, r8
	.endr
	retal	r11

	.size	memset, . - memset

libc_hidden_def(memset)
