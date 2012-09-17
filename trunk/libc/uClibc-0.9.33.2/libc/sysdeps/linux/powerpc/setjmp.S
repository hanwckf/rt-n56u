/* setjmp for PowerPC and PowerPC e500.
   Copyright (C) 1995, 1996, 1997, 1999, 2000, 2004
   Free Software Foundation, Inc.
   e500 contributed by Aldy Hernandez <aldyh@redhat.com>.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <features.h>
#include "ppc_asm.h"
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
#define FP(x...) x
#else
#define FP(x...)
#endif

#if defined __CONFIG_E500__
#define LFD(reg) evldd r##reg
#define STFD(reg) evstdd r##reg
#else
#define LFD(reg) lfd fp##reg
#define STFD(reg) stfd fp##reg
#endif /* __CONFIG_E500__ */

.globl      __sigsetjmp;
.type      __sigsetjmp, @function;
.align  2;        

__sigsetjmp:   
	stw  r1,(JB_GPR1*4)(3)
	mflr r0
	stw  r2,(JB_GPR2*4)(3)
	stw  r14,((JB_GPRS+0)*4)(3)
FP(     STFD (14),((JB_FPRS+0*2)*4)(3))
        stw  r0,(JB_LR*4)(3)
        stw  r15,((JB_GPRS+1)*4)(3)
FP(     STFD (15),((JB_FPRS+1*2)*4)(3))
        mfcr r0
        stw  r16,((JB_GPRS+2)*4)(3)
FP(     STFD (16),((JB_FPRS+2*2)*4)(3))
        stw  r0,(JB_CR*4)(3)
        stw  r17,((JB_GPRS+3)*4)(3)
FP(     STFD (17),((JB_FPRS+3*2)*4)(3))
        stw  r18,((JB_GPRS+4)*4)(3)
FP(     STFD (18),((JB_FPRS+4*2)*4)(3))
        stw  r19,((JB_GPRS+5)*4)(3)
FP(     STFD (19),((JB_FPRS+5*2)*4)(3))
        stw  r20,((JB_GPRS+6)*4)(3)
FP(     STFD (20),((JB_FPRS+6*2)*4)(3))
        stw  r21,((JB_GPRS+7)*4)(3)
FP(     STFD (21),((JB_FPRS+7*2)*4)(3))
        stw  r22,((JB_GPRS+8)*4)(3)
FP(     STFD (22),((JB_FPRS+8*2)*4)(3))
        stw  r23,((JB_GPRS+9)*4)(3)
FP(     STFD (23),((JB_FPRS+9*2)*4)(3))
        stw  r24,((JB_GPRS+10)*4)(3)
FP(     STFD (24),((JB_FPRS+10*2)*4)(3))
        stw  r25,((JB_GPRS+11)*4)(3)
FP(     STFD (25),((JB_FPRS+11*2)*4)(3))
        stw  r26,((JB_GPRS+12)*4)(3)
FP(     STFD (26),((JB_FPRS+12*2)*4)(3))
        stw  r27,((JB_GPRS+13)*4)(3)
FP(     STFD (27),((JB_FPRS+13*2)*4)(3))
        stw  r28,((JB_GPRS+14)*4)(3)
FP(     STFD (28),((JB_FPRS+14*2)*4)(3))
        stw  r29,((JB_GPRS+15)*4)(3)
FP(     STFD (29),((JB_FPRS+15*2)*4)(3))
        stw  r30,((JB_GPRS+16)*4)(3)
FP(     STFD (30),((JB_FPRS+16*2)*4)(3))
        stw  r31,((JB_GPRS+17)*4)(3)
FP(     STFD (31),((JB_FPRS+17*2)*4)(3))

	b __sigjmp_save@local

.size     __sigsetjmp,.-__sigsetjmp
