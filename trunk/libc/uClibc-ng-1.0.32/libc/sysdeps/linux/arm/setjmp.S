/* setjmp for ARM.
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

.global __sigsetjmp
.type __sigsetjmp,%function
.align 2
#if defined(THUMB1_ONLY)
.thumb_func
__sigsetjmp:
	push	{r3, r4, r5, r6, r7, lr}
	mov	ip, r0
	stmia	r0!, {r4, r5, r6, r7}
	mov	r2, r8
	mov	r3, r9
	mov	r4, sl
	mov	r5, fp
	add	r6, sp, #(6 * 4)
	mov	r7, lr
	stmia	r0!, {r2, r3, r4, r5, r6, r7}

	mov	r0, ip
	bl	__sigjmp_save
	pop	{r3, r4, r5, r6, r7, pc}

#else
__sigsetjmp:
	/* Save registers */
	mov	ip, r0
#if defined(__thumb2__)
	stmia	ip!, {v1-v6, sl, fp}
	mov	r2, sp
	stmia	ip!, {r2, lr}
#else
	/* Save registers */
	stmia	ip!, {v1-v6, sl, fp, sp, lr}
#endif
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
# ifdef __VFP_FP__
	/* Store the VFP registers.  */
	/* Following instruction is fstmiax ip!, {d8-d15}.  */
	stc	p11, cr8, [r12], #68
	/* Store the floating-point status register.  */
	/* Following instruction is fmrx r2, fpscr.  */
	mrc	p10, 7, r2, cr1, cr0, 0
	str	r2, [ip], #4
# elif defined __MAVERICK__
	cfstrd	mvd4,  [ip], #8 ; nop
	cfstrd	mvd5,  [ip], #8 ; nop
	cfstrd	mvd6,  [ip], #8 ; nop
	cfstrd	mvd7,  [ip], #8 ; nop
	cfstrd	mvd8,  [ip], #8 ; nop
	cfstrd	mvd9,  [ip], #8 ; nop
	cfstrd	mvd10, [ip], #8 ; nop
	cfstrd	mvd11, [ip], #8 ; nop
	cfstrd	mvd12, [ip], #8 ; nop
	cfstrd	mvd13, [ip], #8 ; nop
	cfstrd	mvd14, [ip], #8 ; nop
	cfstrd	mvd15, [ip], #8
# else
	sfmea   f4, 4, [ip]!
# endif
#endif
#ifdef __IWMMXT__
	/* Save the call-preserved iWMMXt registers.  */
	/* Following instructions are wstrd wr10, [ip], #8 (etc.)  */
	stcl	p1, cr10, [r12], #8
	stcl	p1, cr11, [r12], #8
	stcl	p1, cr12, [r12], #8
	stcl	p1, cr13, [r12], #8
	stcl	p1, cr14, [r12], #8
	stcl	p1, cr15, [r12], #8
#endif

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef __PIC__
	B	__sigjmp_save(PLT)
#else
	B	__sigjmp_save
#endif
#endif

.size __sigsetjmp,.-__sigsetjmp
