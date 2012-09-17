/* longjmp for SH-5.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */


#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>


#define	INTEGER(reg,offset)	ld.q	r2, offset*8, reg
#define	DOUBLE(reg,offset)	fld.d	r2, offset*8, reg


	.file	"longjmp.S"

	.section .text64,"xa"
	.align 2

	.global	longjmp	
	.type	longjmp,@function

longjmp:
	/*
	 *	extern void longjmp(jmp_buf env, int val);
	 *
	 *	r2 == env
	 *	r3 == val
	 *	r4 == temporary
	 */

			/* callee-save registers R10-R16 */
	INTEGER(r10, __SETJMP_INT(0))
	INTEGER(r11, __SETJMP_INT(1))
	INTEGER(r12, __SETJMP_INT(2))
	INTEGER(r13, __SETJMP_INT(3))
	INTEGER(r14, __SETJMP_INT(4))
	INTEGER(r15, __SETJMP_INT(5))
	INTEGER(r16, __SETJMP_INT(6))

			/* callee-save registers R28-R35 */
	INTEGER(r28, __SETJMP_INT(7))
	INTEGER(r29, __SETJMP_INT(8))
	INTEGER(r30, __SETJMP_INT(9))
	INTEGER(r31, __SETJMP_INT(10))
	INTEGER(r32, __SETJMP_INT(11))
	INTEGER(r33, __SETJMP_INT(12))
	INTEGER(r34, __SETJMP_INT(13))
	INTEGER(r35, __SETJMP_INT(14))

			/* callee-save registers R44-R59 */
	INTEGER(r44, __SETJMP_INT(15))
	INTEGER(r45, __SETJMP_INT(16))
	INTEGER(r46, __SETJMP_INT(17))
	INTEGER(r47, __SETJMP_INT(18))
	INTEGER(r48, __SETJMP_INT(19))
	INTEGER(r49, __SETJMP_INT(20))
	INTEGER(r50, __SETJMP_INT(21))
	INTEGER(r51, __SETJMP_INT(22))
	INTEGER(r52, __SETJMP_INT(23))
	INTEGER(r53, __SETJMP_INT(24))
	INTEGER(r54, __SETJMP_INT(25))
	INTEGER(r55, __SETJMP_INT(26))
	INTEGER(r56, __SETJMP_INT(27))
	INTEGER(r57, __SETJMP_INT(28))
	INTEGER(r58, __SETJMP_INT(29))
	INTEGER(r59, __SETJMP_INT(30))

	#if __SETJMP_NUM_INT != 31
	#error	__SETJMP_NUM_INT does agree with expected value
	#endif

#if __SETJMP_NUM_DBL > 0
			/* callee-save registers FR12-FR15 */
	DOUBLE(d12, __SETJMP_DBL(0))
	DOUBLE(d14, __SETJMP_DBL(1))

			/* callee-save registers FR36-FR63 */
	DOUBLE(d36, __SETJMP_DBL(2))
	DOUBLE(d38, __SETJMP_DBL(3))
	DOUBLE(d40, __SETJMP_DBL(4))
	DOUBLE(d42, __SETJMP_DBL(5))
	DOUBLE(d44, __SETJMP_DBL(6))
	DOUBLE(d46, __SETJMP_DBL(7))
	DOUBLE(d48, __SETJMP_DBL(8))
	DOUBLE(d50, __SETJMP_DBL(9))
	DOUBLE(d52, __SETJMP_DBL(10))
	DOUBLE(d54, __SETJMP_DBL(11))
	DOUBLE(d56, __SETJMP_DBL(12))
	DOUBLE(d58, __SETJMP_DBL(13))
	DOUBLE(d60, __SETJMP_DBL(14))
	DOUBLE(d62, __SETJMP_DBL(15))

	#if __SETJMP_NUM_DBL != 16
	#error	__SETJMP_NUM_DBL does agree with expected value
	#endif

#endif	/* __SETJMP_NUM_DBL > 0 */

			/* callee-save registers TR5-TR7 */
	INTEGER(r4, __SETJMP_TRG(0))
	ptabs	r4, tr5
	INTEGER(r4, __SETJMP_TRG(1))
	ptabs	r4, tr6
	INTEGER(r4, __SETJMP_TRG(2))
	ptabs	r4, tr7

	#if __SETJMP_NUM_TRG != 3
	#error	__SETJMP_NUM_TRG does agree with expected value
	#endif

			/* restore Linkage Register (LR) for longjmp return */
	INTEGER(r18, __SETJMP_LR)
	ptabs/l	r18, tr0

			/*
			 *	must ensure longjmp() never returns 0.
			 *	if 'val' == 0, then return 1.
			 */
	cmpeq	r3, r63, r2	/* r2 = (r3==0) ? 1 : 0; */
	add.l	r3, r2, r2	/* return value */

			/* return to caller */
	blink	tr0, r63

