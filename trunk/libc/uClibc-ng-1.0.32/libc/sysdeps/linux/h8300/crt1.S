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

#include <features.h>

	.global	_start
	.global __exit
	.global atexit
	.global main

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
/*
void __uClibc_main(int (*main)(int, char **, char **), int argc,
                    char **argv, void (*app_init)(void), void (*app_fini)(void),
                    void (*rtld_fini)(void),void *stack_end attribute_unused)
*/
	.text

_start: /* put here so that references to _start work with elf-PIC */

	mov.l	@sp+,er1	/* argc */
	mov.l	@sp+,er2	/* argv */
	add.l	#4,sp

#if !defined(__PIC__)
	mov.l	sp, @-sp
	sub.l	er0,er0
	mov.l	er0,@-sp
	mov.l	#_fini,er0
	mov.l	er0,@-sp
	mov.l	#_init,er0
	mov.l	er0,@-sp
	mov.l	#main,er0
	jsr	@__uClibc_main
#else
	mov.l	sp, @-sp
	sub.l	er0,er0
	mov.l	er0,@-sp
	mov.l	@(_fini@GOTOFF,er5),er0
	mov.l	er0,@-sp
	mov.l	@(_init@GOTOFF,er5),er0
	mov.l	er0,@-sp
	mov.l	@(main@GOTOFF,er5),er0
	mov.l	@(__uClibc_main@GOTOFF,er5),er3
	jsr	@er3
#endif

	/* If that didn't kill us, ... */
__exit:
	mov.l	er0,er1
	sub.l	er0,er0
	inc.l	#1,er0		 /* __NR_exit */
	trapa	#0

/*
 *	this was needed for gcc/g++-builds,  atexit was not getting included
 *	for some stupid reason,  this gets us a compiler
 */
empty_func:
	rts
	.weak atexit
	atexit = empty_func


/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl _data_start
_data_start:
	.long 0
	.weak data_start
	data_start = _data_start

