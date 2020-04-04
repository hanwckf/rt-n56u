/* Startup code for elf{32,64}-sparc
   Copyright (C) 1997, 1998, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Originally based on glibc's sysdeps/sparc/sparc{32,64}/elf/start.S */

#include <features.h>
#include <bits/wordsize.h>

# define STACK_BIAS 0
# define ELE_SIZE 4
# define LD ld

.text
.align 4
.global _start
.type   _start,%function
#if defined(__UCLIBC_CTOR_DTOR__)
.type   _init,%function
.type   _fini,%function
#else
.weak   _init
.weak   _fini
#endif
.type   __uClibc_main,%function
/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
.global main
.type   main,%function

#ifdef __PIC__
.LLGETPC0:
	retl
	add	%o7, %l7, %l7
#endif

_start:
#ifdef __PIC__
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %l7
	call	.LLGETPC0
	add	%l7, %lo(_GLOBAL_OFFSET_TABLE_+4), %l7
#endif

	/* Terminate the stack frame, and reserve space for functions to
	 * drop their arguments.  */
	mov	%g0, %fp
	sub	%sp, 6*ELE_SIZE, %sp

	/* Extract the arguments and environment as encoded on the stack.  The
	 * argument info starts after one register window (16 words) past the SP.  */
	LD	[%sp+STACK_BIAS+22*ELE_SIZE], %o1   /* %o1 = argc */
	add	%sp, STACK_BIAS+23*ELE_SIZE, %o2    /* %o2 = argv */

	/* Load the addresses of the user entry points.  */
	sethi	%hi(main), %o0
	sethi	%hi(_init), %o3
	sethi	%hi(_fini), %o4
	or	%o0, %lo(main), %o0
	or	%o3, %lo(_init), %o3
	or	%o4, %lo(_fini), %o4
#ifdef __PIC__
	/* Need a little more magic when building PIC to get addr of main */
	LD	[%l7 + %o0], %o0
	LD	[%l7 + %o3], %o3
	LD	[%l7 + %o4], %o4
#endif

	/* When starting a binary via the dynamic linker, %g1 contains the
	 * address of the shared library termination function, which will be
	 * registered with atexit().  If we are statically linked, this will
	 * be NULL.  */
	mov	%g1, %o5

	/* Let libc do the rest of the initialization, and call main.  */
	call __uClibc_main
	nop

	/* Die very horribly if exit returns.  */
	unimp

.size _start,.-_start

/* Define a symbol for the first piece of initialized data. */
.data
.global __data_start
__data_start:
.long 0
.weak data_start
	data_start = __data_start
