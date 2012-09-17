/* BSD `_setjmp' entry point to `sigsetjmp (..., 1)'.  H8/300 version. */

#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text
	
.global _setjmp

_setjmp:
	add.l	#JB_SIZE,er0
	mov.l	@sp,er1		; return PC
	mov.l	er1,@-er0
	mov.l	sp,@-er0
	mov.l	er6,@-er0
	mov.l	er5,@-er0
	mov.l	er4,@-er0
	sub.l	er0,er0
#if !defined(__PIC__)
	jmp	@___sigjmp_save
#else
	mov.l	@(___sigjmp_save@GOTOFF,er5),er1
	jmp	@er3
#endif
