/* Startup code for Alpha/ELF.
   Copyright (C) 1993,1995,1996,1997,1998,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <features.h>
#include <asm/regdef.h>

.text
.global _start
.ent    _start,0
.type   _start,%function
#if defined(__UCLIBC_CTOR_DTOR__)
.type	_init,%function
.type	_fini,%function
#else
.weak   _init
.weak   _fini
#endif
.type   __uClibc_main,%function
/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
.type   main,%function

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
	 * But let the libc call main.  */
	jsr	ra, __uClibc_main

	/* Die very horribly if exit returns.  Call_pal hlt is callable from
	 * kernel mode only; this will result in an illegal instruction trap.  */
	call_pal 0
	.end _start

/* Define a symbol for the first piece of initialized data.  */
.data
.global __data_start
__data_start:
.long 0
.weak data_start
	data_start = __data_start
