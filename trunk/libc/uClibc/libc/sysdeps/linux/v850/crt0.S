/*
 * libc/sysdeps/linux/v850/crt0.S -- Initial program entry point for linux/v850
 *
 *  Copyright (C) 2001,02,03  NEC Electronics Corporation
 *  Copyright (C) 2001,02,03  Miles Bader <miles@gnu.org>
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
C_ENTRY(start):
	ld.w	0[sp], r6		// Arg 0: argc

	addi	4, sp, r7		// Arg 1: argv

	mov	r7, r8			// Arg 2: envp
	mov	r6, r10			// skip argc elements to get envp start
	add	1, r10			// ...plus the NULL at the end of argv
	shl	2, r10			// Convert to byte-count to skip
	add	r10, r8

	// Load CTBP register
	mov	hilo(C_SYMBOL_NAME(_ctbp)), r19
	ldsr	r19, ctbp

	// Load GP
	mov	hilo(C_SYMBOL_NAME(_gp)), gp

	// tail-call uclibc's startup routine
	addi	-24, sp, sp		// Stack space reserved for args
	jr	C_SYMBOL_NAME(__uClibc_main)


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

