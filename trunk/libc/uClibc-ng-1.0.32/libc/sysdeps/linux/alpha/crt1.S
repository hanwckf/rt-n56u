/* Startup code for Alpha/ELF.
   Copyright (C) 1993, 1995, 1996, 1997, 1998, 2000, 2001, 2002, 2003
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>

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
#include <sys/regdef.h>

	.text
	.align 3
	.globl _start
	.ent _start, 0
	.type _start,@function
#if defined(__UCLIBC_CTOR_DTOR__)
	.type _init,%function
	.type _fini,%function
#else
	.weak _init
	.weak _fini
#endif
	.type __uClibc_main,%function

_start:
	.frame	$15, 0, $15
	br	gp, 1f
1:	ldgp	gp, 0(gp)
	subq	sp, 16, sp
	mov	0, $15
	.prologue 0

  /* Load address of the user's main function.  */
	lda	a0, main

	ldl	a1, 16(sp)	/* get argc */
	lda	a2, 24(sp)	/* get argv */

  /* Load address of our own entry points to .fini and .init.  */
	lda	a3, _init
	lda	a4, _fini

  /* Store address of the shared library termination function.  */
	mov	v0, a5

  /* Provide the highest stack address to the user code.  */
	stq	sp, 0(sp)

  /* Call the user's main function, and exit with its value.
     But let the libc call main.  */
	jsr	ra, __uClibc_main

  /* Die very horribly if exit returns.  Call_pal hlt is callable from
     kernel mode only; this will result in an illegal instruction trap.  */
	call_pal 0
	.end _start

/* For ECOFF backwards compatibility. */
/*weak_alias (_start, __start)*/

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.weak data_start
	data_start = __data_start
