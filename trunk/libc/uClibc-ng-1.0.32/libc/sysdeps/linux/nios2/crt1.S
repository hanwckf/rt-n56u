/* Startup code for Nios II
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

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
    .text

_start:
	/* Set up the global pointer.  */
	movhi	gp, %hiadj(_gp)
	addi	gp, gp, %lo(_gp)

	/* Save the stack pointer.  */
	mov	r2, sp

	/* Create room on the stack for the fini, rtld_fini and stack_end args
	   to __uClibc_main. */
	subi	sp, sp, 12

	/* Push stack_end */
	stw	r2, 8(sp)

	/* Push rtld_fini */
	stw	r4, 4(sp)

	/* Set up the GOT pointer.  */
	nextpc	r22
1:	movhi	r2, %hiadj(_gp_got - 1b)
	addi	r2, r2, %lo(_gp_got - 1b)
	add	r22, r22, r2

	/* r6 == argv */
	addi	r6, sp, 16

	/* r5 == argc */
	ldw	r5, 12(sp)

	/* r4 == main */
	movhi	r4, %call_hiadj(main)
	addi	r4, r4, %call_lo(main)
	add	r4, r4, r22
	ldw	r4, 0(r4)

	/* fp == 0 */
	mov	fp, zero

	/* Let the libc call main and exit with its return code.  */
	movhi	r2, %call_hiadj(__uClibc_main)
	addi	r2, r2, %call_lo(__uClibc_main)
	add	r2, r2, r22
	ldw	r2, 0(r2)
	callr	r2

	/* should never get here....*/
	movhi	r2, %call_hiadj(abort)
	addi	r2, r2, %call_lo(abort)
	add	r2, r2, r22
	ldw	r2, 0(r2)
	callr	r2

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
