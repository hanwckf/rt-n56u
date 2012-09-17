/*
 * libc/sysdeps/linux/nios2/__longjmp.S
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

#include <features.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

.globl __longjmp
.type  __longjmp,@function
.balign 4

__longjmp:
    /* return value is in r5*/
    mov     r2,  r5
  
    /* jmp_buf in r4, restore regs.  */
    ldw     r16, (JB_REGS+ 0)(r4)
    ldw     r17, (JB_REGS+ 4)(r4)
    ldw     r18, (JB_REGS+ 8)(r4)
    ldw     r19, (JB_REGS+12)(r4)
    ldw     r20, (JB_REGS+16)(r4)
    ldw     r21, (JB_REGS+20)(r4)
    ldw     r22, (JB_REGS+24)(r4)
    ldw     r23, (JB_REGS+28)(r4)
    
    ldw     ra, JB_PC(r4)
    ldw     fp, JB_FP(r4)
    ldw     gp, JB_GP(r4)
    ldw     sp, JB_SP(r4)
    
#if defined(__HAVE_FPU__)
    RESTORE_FPU r4 JB_FPREGS
#endif

  /* return to saved RA */    
    ret

.size __longjmp,.-__longjmp
libc_hidden_def(__longjmp)
