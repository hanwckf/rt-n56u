/*
 * libc/sysdeps/linux/nios2/setjmp.S
 *
 *  Copyright (C) 2004,05,06  Microtronix Datacom Ltd
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Wentao Xu <wentao@microtronix.com>
 * 
 */

#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

.globl __sigsetjmp
.type  __sigsetjmp,@function
.balign 4

__sigsetjmp:
    stw     r16, (JB_REGS+ 0)(r4)
    stw     r17, (JB_REGS+ 4)(r4)
    stw     r18, (JB_REGS+ 8)(r4)
    stw     r19, (JB_REGS+12)(r4)
    stw     r20, (JB_REGS+16)(r4)
    stw     r21, (JB_REGS+20)(r4)
    stw     r22, (JB_REGS+24)(r4)
    stw     r23, (JB_REGS+28)(r4)
    
    stw     ra, JB_PC(r4)
    stw     sp, JB_SP(r4)
    stw     fp, JB_FP(r4)
    stw     gp, JB_GP(r4)
    
#if defined(__HAVE_FPU__)
    SAVE_FPU r4 JB_FPREGS
#endif

#ifdef   __PIC__
    /* just pray 16 bit offset is enough */
    br __sigjmp_save
#else
    movhi r8, %hi(__sigjmp_save)
    ori r8, r8, %lo(__sigjmp_save)    
    jmp r8
#endif



