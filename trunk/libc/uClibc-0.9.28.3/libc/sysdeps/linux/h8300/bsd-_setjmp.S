/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  H8/300 version. */

#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text
	
.global __setjmp

__setjmp:
	add.l	#JB_SIZE,er0
	mov.l	@sp,er1		; return PC
	mov.l	er1,@-er0
	mov.l	sp,@-er0
	mov.l	er6,@-er0
	mov.l	er5,@-er0
	mov.l	er4,@-er0
	mov.l	er0,er1
	sub.l	er0,er0
	mov.l	er0,@(JB_SIZE:16,er1)
	rts
