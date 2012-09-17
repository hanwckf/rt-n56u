
/* Copyright (C) 2002, David McCullough <davidm@snapgear.com> */
/* This file is released under the LGPL, any version you like */

#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

.globl setjmp;
.type  setjmp,@function
.align 4;

setjmp:
	moveal	%sp@(4), %a0
	movel	%sp@(0), %a0@(JB_PC)
	moveml	%d2-%d7/%a2-%a7, %a0@(JB_REGS)
#if defined(__HAVE_68881__) || defined(__HAVE_FPU__)
	fmovemx %fp2-%fp7, %a0@(JB_FPREGS)
#endif
	clrl	%d0
	lea		__sigjmp_save-.-8, %a0
	jmp		0(%pc, %a0)

