/*
 * libc/sysdeps/linux/microblaze/longjmp.S -- `longjmp' for microblaze
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
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
	/* load registers from memory to r5 (arg0)*/
	lwi	r1, r5, 0
	lwi	r15, r5, 4
	lwi	r18, r5, 8
	lwi	r19, r5, 12
	lwi	r20, r5, 16
	lwi	r21, r5, 20 
	lwi	r22, r5, 24 
	lwi	r23, r5, 28
	lwi	r24, r5, 32 
	lwi	r25, r5, 36
	lwi	r26, r5, 40
	lwi	r27, r5, 44
	lwi	r28, r5, 48
	lwi	r29, r5, 52
	lwi	r30, r5, 56
	
	addi	r3, r0, 1		// return val
	rtsd	r15, 8			// normal return
	nop

C_END(__longjmp)

.weak C_SYMBOL_NAME(longjmp)
C_SYMBOL_NAME(longjmp) = C_SYMBOL_NAME(__longjmp)

.weak C_SYMBOL_NAME(siglongjmp)
C_SYMBOL_NAME(siglongjmp) = C_SYMBOL_NAME(__longjmp)

.weak C_SYMBOL_NAME(__sigprocmask)
C_SYMBOL_NAME(__sigprocmask) = C_SYMBOL_NAME(sigprocmask)
