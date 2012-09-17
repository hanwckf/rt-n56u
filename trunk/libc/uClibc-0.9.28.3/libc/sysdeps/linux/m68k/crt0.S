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

/*
 *	NOTE: this file works for PIC and non-PIC code.  Be very careful how
 *	you modify it !
 */

#include <features.h>

	.global	_start
	.global __exit
	.global atexit
	.global main

	.text

	nop
	nop
_start: /* put here so that references to _start work with elf-PIC */

	movea.l %d5, %a5	/* uClinux passes in data segment here */
/*
 *	argc, argv and envp are on the stack,  just call to main
 */
	lea __uClibc_main-.-8, %a0	/* call uClibc main */
	jsr %pc@(%a0)

	/* If that didn't kill us, ... */
__exit:
	move.l %sp@+,%d1
	moveq #1,%d0 /* __NR_exit */
	trap #0

/*
 *	this was needed for gcc/g++-builds,  atexit was not getting included
 *	for some stupid reason,  this gets us a compiler
 */
empty_func:
	rts
#if defined(__HAVE_ELF__)
	.weak atexit
	atexit = empty_func
#else
	.set atexit,empty_func
#endif


/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

