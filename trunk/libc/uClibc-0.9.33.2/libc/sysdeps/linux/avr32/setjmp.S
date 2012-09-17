/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

	.text

	.global	__sigsetjmp
	.type	__sigsetjmp,"function"

	.align	1
__sigsetjmp:
	mustr	r8
	stm	r12, r0,r1,r2,r3,r4,r5,r6,r7,r8,sp,lr

	/*
	 * Make a tail call to __sigjmp_save; it takes the same args
	 * and is hidden so we don't need to mess around with the GOT.
	 */
	rjmp	__sigjmp_save
	.size	__sigsetjmp, . - __sigsetjmp

libc_hidden_def(__sigsetjmp)
