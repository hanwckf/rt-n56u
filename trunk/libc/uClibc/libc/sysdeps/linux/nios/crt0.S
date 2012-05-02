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

#include <features.h>
#include <asm/unistd.h>
#include "NM_Macros.S"
#define __ASSEMBLY__
#include <asm/ptrace.h>		/* for REGWIN_SZ */

    .global _start
    .type   __start,@function
    .weak   _init
    .weak   _fini
    .type   __uClibc_start_main,@function
    .type   __h_errno_location, @function
    .type   _stdio_init, @function
    .type   _stdio_term, @function

	.text

_start: 
	nop
	nop

	lds	%o0,[%sp, (REGWIN_SZ / 4) + 0]	// main's argc
	lds	%o1,[%sp, (REGWIN_SZ / 4) + 1]	// main's argv
	lds	%o2,[%sp, (REGWIN_SZ / 4) + 2]	// main's envp

	MOVIA	%o3, _init@h
	MOVIA	%o4, _fini@h
	MOVIA	%o5, __uClibc_start_main@h
	
	call	%o5
	nop


	/* If that didn't kill us, ... */
__exit:
	MOVIP	%g1, __NR_exit
	trap	63
	