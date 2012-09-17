/* Startup code for SH5 & ELF.
   Copyright (C) 1999 Free Software Foundation, Inc.
   Copyright (C) 2001 Hewlett-Packard Australia

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.

	At this entry point, most registers' values are unspecified, except:

   sp		The stack contains the arguments and environment:
   		0(sp)			argc
		4(sp)			argv[0]
		...
		(4*argc)(sp)		NULL
		(4*(argc+1))(sp)	envp[0]
		...
					NULL
*/

	.file	"crt0.S"

	.globl _start
	.globl __main

	.type	__uClibc_start_main,@function

	.section .text64,"xa"
	.align 2	/* 2^2 = 4 */

_start:
	/* Clear the frame pointer since this is the outermost frame.  */
###	mov #0, r14	# qqq

	/* Pop argc off the stack and save a pointer to argv */
	ld.l	r15, 0, r2	/* argc */
	addi	r15, 4, r3	/* argv */

	/* set up the value for the environment pointer r4 = (argc+1)*4+argv */
	addi	r2, 1, r4	/* envp = argc + 1 */
	shlli	r4, 2, r4	/* envp = envp * 4 */
	add	r3, r4, r4	/* envp = envp + argv */

	/* call main() */
	movi	__uClibc_start_main,r17
	ptabs/l	r17,tr0
	blink	tr0,r18

	/* should never get here....*/
###	movi	abort@lh,r17
###	shori	abort@ll,r17
	ptabs/l	r17,tr0
	blink	tr0,r63	/* call abort() => (r63) do not come back ... */

	/*
	 *	The following is a stub to stop the GNU toolchain
	 *	from calling its C-RTL initialization routines.
	 */
__main:
	ptabs/l	r18,tr0
	blink	tr0,r63

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

