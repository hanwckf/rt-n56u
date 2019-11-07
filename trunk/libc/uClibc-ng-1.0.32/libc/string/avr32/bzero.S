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
	.global bzero
	.type	bzero, @function
	.align	1
bzero:
	mov	r10, r11
	mov	r11, 0
	rjmp	HIDDEN_JUMPTARGET(memset)

	.size	bzero, . - bzero

#endif /* __UCLIBC_SUSV3_LEGACY__ */
