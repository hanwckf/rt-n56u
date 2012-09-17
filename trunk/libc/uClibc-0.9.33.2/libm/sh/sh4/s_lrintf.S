/* Round argument to nearest integer value. SH4 version.
 * According to ISO/IEC 9899:1999. This version doesn't handle range error.
 * If arg is not finite or if the result cannot be represented into a long,
 * return an unspecified value. No exception raised.
 *
 * Copyright (C) 2010 STMicroelectronics Ltd.
 *
 * Author: Christian Bruel <christian.bruel@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

ENTRY(lrintf)
	mov	#0,r0
	sts	fpscr,r3
	lds	r0,fpscr
	flds	fr5,fpul
	mov.l	LOCAL(mask),r1
	sts	fpul,r2
	and	r2,r1
	mov.l	LOCAL(midway),r2
	or	r1,r2
	lds	r2,fpul
	fsts	fpul,fr2
	fadd	fr2,fr5
	ftrc	fr5,fpul
	sts	fpul,r0
	float	fpul,fr2
	fcmp/eq	fr5,fr2
	bf/s	0f
	mov	#1,r2
	tst	r1,r1
	and	r0,r2
	movt	r1
	shal	r1
	tst	r2,r2
	add	#-1,r1
	bt	0f
	sub	r1,r0
0:
	rts
	lds	r3,fpscr

	.align 2
LOCAL(mask):
	.long	0x80000000
LOCAL(midway):
	.long	1056964608

END(lrintf)
