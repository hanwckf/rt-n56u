/* Copyright (C) 1995-2016 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.

   Note that the code in the .init section has already been run.
   This includes _init and _libc_init


   At this entry point, most registers' values are unspecified, except:

   x0		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
		0(sp)			argc
		8(sp)			argv[0]
		...
		(8*argc)(sp)		NULL
		(8*(argc+1))(sp)	envp[0]
		...
					NULL
 */

	.text
	.globl _start
	.type _start,#function
_start:
	/* Create an initial frame with 0 LR and FP */
	mov	x29, #0
	mov	x30, #0

	/* Setup _fini in argument register */
	mov	x5, x0

	/* Load argc and a pointer to argv */
	ldr	x1, [sp, #0]
	add	x2, sp, #8

	/* Setup stack limit in argument register */
	mov	x6, sp

#ifdef __PIC__
        adrp    x0, :got:main
	ldr     x0, [x0, #:got_lo12:main]

        adrp    x3, :got:_init
	ldr     x3, [x3, #:got_lo12:_init]

        adrp    x4, :got:_fini
	ldr     x4, [x4, #:got_lo12:_fini]
#else
	/* Set up the other arguments in registers */
	ldr	x0, =main
	ldr	x3, =_init
	ldr	x4, =_fini
#endif

	/* Let the libc call main and exit with its return code.  */
	bl	__uClibc_main

	/* should never get here....*/
	bl	abort

	/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
