/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

#include <features.h>

#ifdef __UCLIBC_SUSV3_LEGACY__

	.text
	.global bcopy
	.type	bcopy, @function
	.align	1
bcopy:
	/* Swap the first two arguments */
	eor	r11, r12
	eor	r12, r11
	eor	r11, r12
	rjmp	HIDDEN_JUMPTARGET(memmove)

	.size	bcopy, . - bcopy

#endif /* __UCLIBC_SUSV3_LEGACY__ */
