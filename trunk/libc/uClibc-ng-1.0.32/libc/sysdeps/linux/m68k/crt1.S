/* Startup code compliant to the ELF m68k ABI.
   Copyright (C) 1996, 1997, 1998, 2001, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>
#include "m68k_pic.S"

#ifndef L_Scrt1
/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/m68k ABI says that when the entry point runs,
   most registers' values are unspecified, except for:

   %a1		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %sp		The stack contains the arguments and environment:
		0(%sp)			argc
		4(%sp)			argv[0]
		...
		(4*argc)(%sp)		NULL
		(4*(argc+1))(%sp)	envp[0]
		...
					NULL

   The uclinux conventions are different.  %a1 is not defined on entry
   and the stack is laid out as follows:

		0(%sp)			argc
		4(%sp)			argv
		8(%sp)			envp
*/

	.text
	.type	_init,%function
	.type	_fini,%function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif
	.globl _start
	.type _start,@function
_start:
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	sub.l %fp, %fp

#if !defined __ARCH_USE_MMU__ && defined __PIC__
	/* Set up the global pointer.  The GOT is at the beginning of the
	   data segment, whose address is in %d5.  */
	move.l %d5,%a5
	.equ have_current_got, 1
#endif
	
#ifdef __UCLIBC_FORMAT_SHARED_FLAT__
	CALL __shared_flat_add_library,%a1
#endif

	/* Extract the arguments as encoded on the stack and set up the
	   arguments for `main': argc, argv.  envp will be determined
	   later in __libc_start_main.  */
	move.l (%sp)+, %d0	/* Pop the argument count.  */
#ifndef __ARCH_USE_MMU__
	move.l (%sp)+, %a0
#else
	move.l %sp, %a0		/* The argument vector starts just at the
				   current stack top.  */
#endif

	/* Provide the highest stack address to the user code (for stacks
	   which grow downward).  */
	pea (%sp)

#ifndef __ARCH_USE_MMU__
	clr.l -(%sp)
#else
	pea (%a1)		/* Push address of the shared library
				   termination function.  */
#endif

	/* Push the address of our own entry points to `.fini' and
	   `.init'.  */
#if defined __UCLIBC_FORMAT_SHARED_FLAT__
	PEA_TEXT __shared_flat_fini,%a1
	PEA_TEXT __shared_flat_init,%a1
#else
	PEA_TEXT _fini,%a1
	PEA_TEXT _init,%a1
#endif

	pea (%a0)		/* Push second argument: argv.  */
	move.l %d0, -(%sp)	/* Push first argument: argc.  */

	PEA_TEXT main,%a1

	/* Call the user's main function, and exit with its value.  But
	   let the libc call main.  */
	CALL __uClibc_main,%a1

	illegal			/* Crash if somehow `exit' does return.  */
#else
	.text
	.globl lib_main
	.hidden lib_main
	.type lib_main,@function
lib_main:
	move.l %d5,%a5
	JUMP __shared_flat_add_library,%a0

	.hidden _current_shared_library_a5_offset_
#endif

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
