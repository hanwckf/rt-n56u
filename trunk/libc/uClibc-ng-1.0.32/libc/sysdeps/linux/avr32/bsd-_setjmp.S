/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

	/* This just does a tail-call to __sigsetjmp(env, 0) */
	.global	_setjmp
	.type	_setjmp,"function"
	.align	1
_setjmp:
	mov	r11, 0
	bral	HIDDEN_JUMPTARGET(__sigsetjmp)
	.size	_setjmp, . - _setjmp
