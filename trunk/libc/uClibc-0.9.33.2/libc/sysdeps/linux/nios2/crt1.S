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
 * Updated by Thomas Chou <thomas@wytron.com.tw> for crt1.S
 * 
 */

#include <features.h>
#include <asm/unistd.h>

    .global _start
    .type   _start,@function
    .type	_init,%function
    .type	_fini,%function
#ifndef __UCLIBC_CTOR_DTOR__
    .weak   _init
    .weak   _fini
#endif
    .type   main,@function
    .type   __uClibc_main,@function
    .type   __h_errno_location, @function
    .type   _stdio_init, @function
    .type   _stdio_term, @function
    .text
	.balign 4
_start:
    nop
    br 0f
0:
    /* load gp */
    movhi gp, %hiadj(_gp)
    addi gp, gp, %lo(_gp)

    /* load main, argc, argv from stack */
    movhi r4, %hi(main)
    ori  r4, r4, %lo(main)	/* main */
    ldw r5, 0(sp)		/* argc */
    ldw r6, 4(sp)		/* argv */

    /* load the 4th arg */
    movhi r7, %hi(_init)
    ori  r7, r7, %lo(_init)

    /* Allocate space on the stack for 6-7th arg, reuse 5th space */
    addi sp,sp,-8
    /* push 5-7th args on stack */
    movhi r8, %hi(_fini)
    ori  r8, r8, %lo(_fini)
    stw  r8, 0(sp)

    stw  r2, 4(sp)		/* rtld_fini */
    stw  sp, 8(sp)		/* stack_end */

    /* call uClibc_main, shouldn't return */
#ifdef __PIC__
    /* just pray 16 bit offset is enough */
    br __uClibc_main
#else
    call __uClibc_main
#endif

    /* crash in the event of return */
__exit:
    movui r2, TRAP_ID_SYSCALL
    movui r3, __NR_exit
    trap
