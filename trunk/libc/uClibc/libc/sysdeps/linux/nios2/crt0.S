/*
 * libc/sysdeps/linux/nios2/crt0.S -- entry point for linux/nios2
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
#include <asm/unistd.h>

    .global _start
    .type   __start,@function
    .weak   _init
    .weak   _fini
    .type   __uClibc_start_main,@function
    .type   __h_errno_location, @function
    .type   _stdio_init, @function
    .type   _stdio_term, @function
    .text
	.balign 4
_start:
    nop
    br 0f
	.long _stext	/* start address of .text + 4 */
	.long _etext	/* start address of .data */
	.long _edata	/* start address of .bss */
0:
    /* load gp */
    movhi gp, %hiadj(_gp)
    addi gp, gp, %lo(_gp)

    /* load argc, argv, envp from stack */
    ldw r4, 0(sp)
    ldw r5, 4(sp)
    ldw r6, 8(sp)
    
    /* load the 4th arg */
    movhi r7, %hi(_init)
    ori  r7, r7, %lo(_init)
  
    /* reuse the argc stack slot for the 5th arg */
    movhi r8, %hi(_fini)
    ori  r8, r8, %lo(_fini)
    stw  r8, 0(sp)
   
    /* call uClibc_main, shouldn't return */
#ifdef __PIC__
    /* just pray 16 bit offset is enough */
    br __uClibc_start_main
#else
    call __uClibc_start_main
#endif

    /* crash in the event of return */
__exit:
    movui r2, TRAP_ID_SYSCALL
    movui r3, __NR_exit
    trap