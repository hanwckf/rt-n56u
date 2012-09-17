/*******************************************************************************
 * 
 * Copyright (c) 1993 Intel Corporation
 * 
 * Intel hereby grants you permission to copy, modify, and distribute this
 * software and its documentation.  Intel grants this permission provided
 * that the above copyright notice appears in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  In addition, Intel grants this permission provided that
 * you prominently mark as "not part of the original" any modifications
 * made to this software or documentation, and that the name of Intel
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software or the documentation without specific,
 * written prior permission.
 * 
 * Intel Corporation provides this AS IS, WITHOUT ANY WARRANTY, EXPRESS OR
 * IMPLIED, INCLUDING, WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY
 * OR FITNESS FOR A PARTICULAR PURPOSE.  Intel makes no guarantee or
 * representations regarding the use of, or the results of the use of,
 * the software and documentation in terms of correctness, accuracy,
 * reliability, currentness, or otherwise; and you rely on the software,
 * documentation and results solely at your own risk.
 *
 * IN NO EVENT SHALL INTEL BE LIABLE FOR ANY LOSS OF USE, LOSS OF BUSINESS,
 * LOSS OF PROFITS, INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES
 * OF ANY KIND.  IN NO EVENT SHALL INTEL'S TOTAL LIABILITY EXCEED THE SUM
 * PAID TO INTEL FOR THE PRODUCT LICENSED HEREUNDER.
 * 
 ******************************************************************************/

/***************************************************************************
 *
 * Modified from the original in order to fit with 
 * uClibc's setjmp, _setjmp, __sigsetjmp and ___sigjmp_save.
 *
 *
 * int setjmp (jmp_buf __env)  is the BSD style setjmp function.
 * It simply calls __sigsetjmp(env, 1)
 *
 * int _setjmp (jmp_buf __env) is the posix style setjmp function.
 * It simply calls __sigsetjmp(env, 0)
 * This is the one normally used.
 *
 ***************************************************************************/

        .text 
        .align  4
        .globl _setjmp
        .globl __setjmp
        
_setjmp:
        mov 1, g1   /* __sigsetjmp(env, 1) */
        bx __sigsetjmp

__setjmp:
        mov 0, g1   /* __sigsetjmp(env, 0) */
        bx __sigsetjmp     

        
/******************************************************************************/
/*                                                                            */
/*      setjmp(), longjmp()                                                   */
/*                                                                            */
/******************************************************************************/
	.file "setjmp.S"
	.text
	/* .link_pix */

	.align	4
	.globl	__sigsetjmp
__sigsetjmp:
	flushreg
	andnot	0xf,pfp,g2	/* get pfp, mask out return status bits */
	st	g2, 0x58(g0)	/* save fp of caller*/
   /* save globals not killed by the calling convention */
	stq	g8, 0x40(g0)	/* save g8-g11*/
	st	g12, 0x50(g0)	/* save g12*/
	st	g14, 0x54(g0)	/* save g14*/
   /* save previous frame local registers */
	ldq	(g2), g4	/* get previous frame pfp, sp, rip, r3 */
	stq	g4, (g0)	/* save pfp, sp, rip, r3 */
	ldq	0x10(g2), g4	/* get previous frame r4-r7 */
	stq	g4, 0x10(g0)	/* save r4-r7 */
	ldq	0x20(g2), g4	/* get previous frame r8-r11 */
	stq	g4, 0x20(g0)	/* save r8-r11 */
	ldq	0x30(g2), g4	/* get previous frame r12-r15 */
	stq	g4, 0x30(g0)	/* save r12-r15 */

        bx ___sigjmp_save   

   /*
    * fake a return to the place that called the corresponding __sigsetjmp
   */
	.align	4
	.globl	___longjmp
___longjmp:
	call	0f		/* ensure there is at least one stack frame */

0:
	flushreg		/* do this before swapping stack */
	ld	0x58(g0), pfp	/* get fp of caller of setjmp */
   /* restore local registers 
    * the following code modifies the frame of the function which originally 
    *  called setjmp.
    */
	ldq	(g0), g4	/* get pfp, sp, rip, r3 */
	stq	g4, (pfp)	/* restore pfp, sp, rip, r3 */
	ldq	0x10(g0), g4	/* get r4-r7 */
	stq	g4, 0x10(pfp)	/* restore r4-r7 */
	ldq	0x20(g0), g4	/* get r8-r11 */
	stq	g4, 0x20(pfp)	/* restore r8-r11 */
	ldq	0x30(g0), g4	/* get r12-r15 */
	stq	g4, 0x30(pfp)	/* restore r12-r15 */
   /* restore global registers */
	ldq	0x40(g0), g8	/* get old g8-g11 values */
	ld	0x50(g0), g12	/* get old g12 value */
	ld	0x54(g0), g14	/* get old g14 value */

	mov	g1, g0		/* get return value */
	cmpo	g0, 0		/* make sure it is not zero */
	bne	0f
	mov	1, g0		/* return 1 by default */
0:
	ret			/* return to caller of __sigsetjmp */
