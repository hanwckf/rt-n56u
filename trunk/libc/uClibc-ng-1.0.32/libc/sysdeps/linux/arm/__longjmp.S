/* longjmp for ARM.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

.global __longjmp
.type __longjmp,%function
.align 2
#if defined(THUMB1_ONLY)
.thumb_func
__longjmp:
	mov	r2, r0
	movs	r0, r1
	/* can't let setjmp() return zero! */
	bne	1f
	mov	r0, #1
1:
	mov	r1, r2
	/* Restore registers, shuffling them through low regs.  */
	add	r2, #(4 * 4)
	ldmia	r2!, {r4, r5, r6, r7}
	mov	r8, r4
	mov	r9, r5
	mov	sl, r6
	mov	fp, r7
	ldmia	r2!, {r4, r5}
	mov	sp, r4
	mov	lr, r5
	ldmia	r1!, {r4, r5, r6, r7}
	bx	lr
#else
__longjmp:
	mov	ip, r0		/* save jmp_buf pointer */
	
	movs	r0, r1		/* get the return value in place */
	IT(t, eq)
	moveq	r0, #1		/* can't let setjmp() return zero! */

#if defined(__thumb2__)
	/* Thumb-2 does not allow loading sp with ldm.  */
	ldmia     ip!,  {v1-v6, sl, fp}
	ldr	  sp, [ip], #4
	ldr	  lr, [ip], #4
#else
	ldmia     ip!,  {v1-v6, sl, fp, sp, lr}
#endif

#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
#ifdef __VFP_FP__
	/* Restore the VFP registers.  */
	/* Following instruction is fldmiax ip!, {d8-d15}.  */
	ldc	p11, cr8, [r12], #68
	/* Restore the floating-point status register.  */
	ldr     r1, [ip], #4
	/* Following instruction is fmxr fpscr, r1.  */
	mcr	p10, 7, r1, cr1, cr0, 0
# elif defined __MAVERICK__
	cfldrd	mvd4,  [ip], #8 ; nop
	cfldrd	mvd5,  [ip], #8 ; nop
	cfldrd	mvd6,  [ip], #8 ; nop
	cfldrd	mvd7,  [ip], #8 ; nop
	cfldrd	mvd8,  [ip], #8 ; nop
	cfldrd	mvd9,  [ip], #8 ; nop
	cfldrd	mvd10, [ip], #8 ; nop
	cfldrd	mvd11, [ip], #8 ; nop
	cfldrd	mvd12, [ip], #8 ; nop
	cfldrd	mvd13, [ip], #8 ; nop
	cfldrd	mvd14, [ip], #8 ; nop
	cfldrd	mvd15, [ip], #8
# else
	lfmfd	f4, 4, [ip] !	/* load the floating point regs */
# endif
#endif	
#ifdef __IWMMXT__
	/* Restore the call-preserved iWMMXt registers.  */
	/* Following instructions are wldrd wr10, [ip], #8 (etc.)  */
	ldcl	p1, cr10, [r12], #8
	ldcl	p1, cr11, [r12], #8
	ldcl	p1, cr12, [r12], #8
	ldcl	p1, cr13, [r12], #8
	ldcl	p1, cr14, [r12], #8
	ldcl	p1, cr15, [r12], #8
#endif
	BX(lr)
#endif

.size __longjmp,.-__longjmp
libc_hidden_def(__longjmp)
