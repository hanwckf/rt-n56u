/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.

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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */


/* When we enter this piece of code, the user stack looks like this:
*      argc            argument counter (integer)
*      argv[0]         program name (pointer)
*      argv[1...N]     program args (pointers)
*      NULL
*      env[0...N]      environment variables (pointers)
*      NULL

*   When we are done here, we want
*	R0=argc
*	R1=*argv[0]
*	R2=*envp[0]
*/

.text
.align 2
.global _start;
.type	_start,STT_FUNC;
.global ___uClibc_main;
.type	___uClibc_main,STT_FUNC;

/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */

.type	_main,STT_FUNC;

_start:

/*	clear the frame pointer */
	FP = 0;
/*	Load register R0 (argc) from the stack to its final resting place */
	P0 = SP;
	R0 = [P0++];

/*	Copy argv pointer into R1 -- which its final resting place */
	R1 = P0;

/*	Skip to the end of argv and put a pointer to the environment in R2 */
	R2 = R0;
	R2 <<= 2;
	R2 += 4;
	R2 = R1+R2;

/*	Ok, now run uClibc's main() -- shouldn't return */
	sp += -12;
	jump.l	___uClibc_main;

