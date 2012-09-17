/*
 * libc/sysdeps/linux/v850/setjmp.S -- `setjmp' for v850
 *
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
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
C_ENTRY(setjmp):
	mov	1, r7			/* Save the signal mask.  */
	br	C_SYMBOL_NAME(__sigsetjmp)

	.globl C_SYMBOL_NAME(_setjmp)
C_SYMBOL_NAME(_setjmp):
	mov	0, r7			/* Don't save the signal mask.  */

	.globl C_SYMBOL_NAME(__sigsetjmp)
C_SYMBOL_NAME(__sigsetjmp):
	/* Save registers */
	mov	r6, ep
	sst.w	sp, 0[ep]
	sst.w	lp, 4[ep]
	sst.w	r2, 8[ep]
	sst.w	r20, 12[ep]
	sst.w	r21, 16[ep]
	sst.w	r22, 20[ep]
	sst.w	r23, 24[ep]
	sst.w	r24, 28[ep]
	sst.w	r25, 32[ep]
	sst.w	r26, 36[ep]
	sst.w	r27, 40[ep]
	sst.w	r28, 44[ep]
	sst.w	r29, 48[ep]
	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	jr	C_SYMBOL_NAME(__sigjmp_save)
C_END(setjmp)
