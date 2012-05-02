/*
 * libc/sysdeps/linux/nios2/vfork.S -- `vfork' syscall for linux/nios2
 *
 *  Copyright (C) 2004  Microtronix Datacom Ltd
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Wentao Xu <wentao@microtronix.com>
 */
#include <asm/unistd.h>

#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif

   .text
   .balign 4
   .globl errno
   .globl vfork
   .globl __libc_vfork
#if defined __HAVE_ELF__
   .type    vfork,@function
   .type    __libc_vfork,@function
#endif
vfork:
__libc_vfork:
    movui   r2, TRAP_ID_SYSCALL
    movui   r3, __NR_vfork
    trap
    movi    r8, -4096
    bltu    r8, r2, fix_errno
    ret
fix_errno:
    sub     r8, r0, r2
    
    addi    sp, sp, -8
    stw     ra, 4(sp)
    stw     r8, 0(sp)
#ifndef __PIC__
    call    __errno_location
#else
    
#endif
    ldw     ra, 4(sp)
    ldw     r8, 0(sp)
    stw     r8, 0(r2)
    
    addi    r2, r0, -1
    addi  sp, sp, 8
    ret

