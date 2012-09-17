
/* Copyright (C) 2002, David McCullough <davidm@snapgear.com> */
/* This file is released under the LGPL, any version you like */

#define _ASM
#define _SETJMP_H
#include <features.h>
#include <bits/setjmp.h>

.globl __longjmp
.type  __longjmp,@function
.align 4
__longjmp:
	moveal	%sp@(4), %a0
	movel	%sp@(8), %d0
	bne		1f
	movel	#1, %d0
1:
	moveml	%a0@(JB_REGS), %d2-%d7/%a2-%a7
#if defined(__HAVE_68881__) || defined(__HAVE_FPU__)
	fmovemx %a0@(JB_FPREGS), %fp2-%fp7
#endif
	movel	%a0@(JB_PC), %sp@
	rts

libc_hidden_def(__longjmp)
