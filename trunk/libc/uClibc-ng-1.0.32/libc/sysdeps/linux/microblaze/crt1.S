/*
 * libc/sysdeps/linux/microblaze/crt1.S -- Initial program entry point for linux/microblaze
 *
 *  Copyright (C) 2009       Meyer Sound Laboratories
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#include <libc-symbols.h>

/* Upon entry, the stack contains the following data:
	argc, argv[0], ..., argv[argc-1], 0, envp[0], ..., 0
*/

	.text
	.globl C_SYMBOL_NAME(_start)
	.align 4
C_SYMBOL_NAME(_start):

	/*
	Preparing arguments for uClibc's startup routine.
	The routine has 6 arguments, so 5 of them are placed
	into registers, one on the stack
	*/

	la      r5, r0, C_SYMBOL_NAME(main) /* Arg 1: main() */
	lw	r6, r0, r1		    /* Arg 2: argc   */
	addi	r7, r1, 4		    /* Arg 3: argv   */
	la	r8, r0, _init               /* Arg 4: init   */
	la	r9, r0, _fini               /* Arg 5: fini   */
	addk	r10,r0,r0                   /* Arg 6: rtld_fini = NULL */


	/* Reserve space for __uClibc_main to save parameters
	   (Microblaze ABI stack calling convention)
	   and for stack_end argument to __uClibc_main  */
	add 	r3, r1, r0
	addi    r1, r1, -32

	/* tail-call uClibc's startup routine */
	brid	C_SYMBOL_NAME(__uClibc_main)
	swi 	r3, r1, 28	/* Arg 7: stack end [DELAY SLOT] */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
