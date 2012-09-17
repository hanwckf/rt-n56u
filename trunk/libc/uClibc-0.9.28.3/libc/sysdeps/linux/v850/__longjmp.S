/*
 * libc/sysdeps/linux/v850/longjmp.S -- `longjmp' for v850
 *
 *  Copyright (C) 2001,02  NEC Corporation
 *  Copyright (C) 2001,02  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Miles Bader <miles@gnu.org>
 */

#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

#include <clinkage.h>

	.text
C_ENTRY(__longjmp):
	/* Save registers */
	mov	r6, ep
	sld.w	0[ep], sp
	sld.w	4[ep], lp
	sld.w	8[ep], r2
	sld.w	12[ep], r20
	sld.w	16[ep], r21
	sld.w	20[ep], r22
	sld.w	24[ep], r23
	sld.w	28[ep], r24
	sld.w	32[ep], r25
	sld.w	36[ep], r26
	sld.w	40[ep], r27
	sld.w	44[ep], r28
	sld.w	48[ep], r29
	mov	1, r10			// return val
	jmp	[lp]
C_END(__longjmp)

.weak C_SYMBOL_NAME(__sigprocmask)
C_SYMBOL_NAME(__sigprocmask) = C_SYMBOL_NAME(sigprocmask)
