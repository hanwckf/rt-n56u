/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

	/* This just does a tail-call to __sigsetjmp(env, 1) */
	.global	setjmp
	.type	setjmp,"function"
	.align	1
setjmp:
	mov	r11, 1
	bral	HIDDEN_JUMPTARGET(__sigsetjmp)
	.size	setjmp, . - setjmp
