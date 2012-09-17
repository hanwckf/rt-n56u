/*
 * libc/sysdeps/linux/microblaze/crt0.S -- Initial program entry point for linux/microblaze
 *
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

#include <clinkage.h>

/* Upon entry, the stack contains the following data:
	argc, argv[0], ..., argv[argc-1], 0, envp[0], ..., 0
*/

	.text
C_ENTRY(_start):
	lw	r5, r0, r1		// Arg 0: argc

	addi	r6, r1, 4		// Arg 1: argv

					// Arg 2: envp
	addi	r3, r5, 1		// skip argc elements to get envp start
					// ...plus the NULL at the end of argv
	add	r3, r3, r3		// Make word offset
	add	r3, r3, r3
	add	r7, r6, r3		// add to argv to get offset

	// Load SDAs
	la	r2, r0, C_SYMBOL_NAME(_SDA_BASE_)
	la	r13, r0, C_SYMBOL_NAME(_SDA2_BASE_)

	// tail-call uclibc's startup routine
	brid	C_SYMBOL_NAME(__uClibc_main)
	nop


/* Stick in a dummy reference to `main', so that if an application
   is linking when the `main' function is in a static library (.a)
   we can be sure that `main' actually gets linked in.  */
L_dummy_main_reference:
	.long	C_SYMBOL_NAME(main)

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

