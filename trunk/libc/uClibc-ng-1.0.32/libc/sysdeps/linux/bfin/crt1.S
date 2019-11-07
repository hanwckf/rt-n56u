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
not, see <http://www.gnu.org/licenses/>.  */


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

#include <features.h>

#undef USE_GOT
#if defined (__UCLIBC_FORMAT_SHARED_FLAT__) || defined (__UCLIBC_FORMAT_FLAT_SEP_DATA__)
#define USE_GOT
#endif

#if !(defined L_Scrt1 && defined __UCLIBC_FORMAT_SHARED_FLAT__)

.text
.align 2
.global __start;
.type	__start,STT_FUNC;
.weak	__init;
.weak	__fini;
.global	___uClibc_main;
.type	___uClibc_main,STT_FUNC;

/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */

.type	_main,STT_FUNC;

__start:

#if defined(__BFIN_FDPIC__) && !defined(L_Scrt1)
	/* P0 contains a pointer to the program's load map.  */
	call	.Lcall;
.Lcall:
	R4 = RETS;
	SP += -12;
	R0.L = .Lcall;
	R0.H = .Lcall;
	R1.L = __ROFIXUP_LIST__;
	R1.H = __ROFIXUP_LIST__;
	R2.L = __ROFIXUP_END__;
	R2.H = __ROFIXUP_END__;
	R1 = R1 - R0;
	R1 = R1 + R4;
	R2 = R2 - R0;
	R2 = R2 + R4;
	R0 = P0;
	CALL	___self_reloc;
	SP += 12;
	P3 = R0;
#endif

/*	clear the frame pointer and the L registers.  */
	FP = 0;
	L0 = 0;
	L1 = 0;
	L2 = 0;
	L3 = 0;

#ifdef __ID_SHARED_LIB__
	/* We know we have a local copy, so we can avoid the GOT.  */
	CALL ___shared_flat_add_library;
#endif
/*	Load register R1 (argc) from the stack to its final resting place */
	P0 = SP;
	R1 = [P0++];

/*	Copy argv pointer into R2 -- which its final resting place */
	R2 = P0;

	SP += -28;

#ifndef __BFIN_FDPIC__
	R7 = 0;
#endif
	/* Pass highest stack pointer to the app.  */
	[SP + 24] = P2;
	/* Store the pointer to ld.so's fini that we got in P1.  */
	[SP + 20] = R7;

/*	Ok, now run uClibc's main() -- shouldn't return */
#if (defined L_crt1 || defined L_Scrt1) && defined __UCLIBC_CTOR_DTOR__

#ifdef __BFIN_FDPIC__
	R3 = [P3 + __init@FUNCDESC_GOT17M4];
#elif defined USE_GOT
	R3 = [P5 + ___shared_flat_init@GOT];
#else
	R3.H = __init;
	R3.L = __init;
#endif
	[SP+12] = R3;


#ifdef __BFIN_FDPIC__
	R3 = [P3 + __fini@FUNCDESC_GOT17M4];
#elif defined USE_GOT
	R3 = [P5 + ___shared_flat_fini@GOT];
#else	
	R3.H = __fini;
	R3.L = __fini;
#endif
	[SP+16] = R3;
#else /* no ctor/dtor handling */
	R3 = 0;
	[SP + 12] = R3;
	[SP + 16] = R3;
#endif

#ifdef __BFIN_FDPIC__
	R0 = [P3 + _main@FUNCDESC_GOT17M4];
#elif defined USE_GOT
	R0 = [P5 + _main@GOT];
#else
	R0.H = _main;
	R0.L = _main;
#endif
#ifdef USE_GOT
	P0 = [P5 + ___uClibc_main@GOT];
	jump (P0)
#else
	jump.l	___uClibc_main;
#endif

#else
	.text
	.global lib_main
	.hidden lib_main
	.type lib_main,@function
lib_main:
	RETS = [SP++];
	/* We know we have a local copy, so we can avoid the GOT.  */
	JUMP.L ___shared_flat_add_library;

	.hidden _current_shared_library_p5_offset_
#endif

.section .note.GNU-stack,"",%progbits
