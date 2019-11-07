/* Startup code compliant to the ELF Mips ABI.
   Copyright (C) 1995, 1997, 2000, 2001, 2002, 2003, 2004
	Free Software Foundation, Inc.
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


#include <sys/regdef.h>
#include <sys/asm.h>
#include <features.h>


/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/Mips ABI (pages 3-31, 3-32) says that when the entry
   point runs, most registers' values are unspecified, except for:

   v0 ($2)	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp ($29)	The stack contains the arguments and environment:
		0(%esp)			argc
		4(%esp)			argv[0]
		...
		(4*argc)(%esp)		NULL
		(4*(argc+1))(%esp)	envp[0]
		...
					NULL
   ra ($31)	The return address register is set to zero so that programs
		that search backword through stack frames recognize the last
		stack frame.
*/


/* We need to call:
   __uClibc_main (int (*main) (int, char **, char **), int argc,
		      char **argv, void (*init) (void), void (*fini) (void),
		      void (*rtld_fini) (void), void *stack_end)
*/

.text
	.globl	__start
	.type	__start,@function
	.type	_init,@function
	.type	_fini,@function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif
	.type	    main,@function
	.type	    __uClibc_main,@function
	.ent	__start

__start:
#ifdef __PIC__
	SETUP_GPX($0)
	SETUP_GPX64($25,$0)
#else
	PTR_LA $28, _gp             /* Setup GP correctly if we're non-PIC.  */
	move $31, $0
#endif

	PTR_LA $4, main		/* main */
	PTR_L $5, 0($29)		/* argc */
	PTR_ADDIU $6, $29, PTRSIZE	/* argv  */

	/* Allocate space on the stack for seven arguments and
	 * make sure the stack is aligned to double words (8 bytes) */

	and $29, -2 * SZREG

#if _MIPS_SIM == _MIPS_SIM_ABI32
	PTR_SUBIU $29, 32
#endif
	PTR_LA $7, _init		/* init */
	PTR_LA $8, _fini
#if _MIPS_SIM == _MIPS_SIM_ABI32
	PTR_S $8, 16($29)		/* fini */
	PTR_S $2, 20($29)		/* rtld_fini */
	PTR_S $29, 24($29)		/* stack_end */
#else
	move $9, $2		/* rtld_fini */
	move $10, $29		/* stack_end */
#endif
	jal __uClibc_main
hlt:
	/* Crash if somehow `__uClibc_main' returns anyway.  */
	b   hlt
.size __start,.-__start
.end __start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

