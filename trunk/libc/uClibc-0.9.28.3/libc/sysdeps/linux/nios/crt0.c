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

#include <asm/ptrace.h>

#define nop() __asm__ __volatile__ ("nop")

extern inline int _stack_frame_address(void)
{
	int retval;
	__asm__ __volatile__(
		"mov	%0, %%fp\n\t"
		: "=r" (retval) );
	return retval;
}

void __uClibc_main(int argc,void *argv,void *envp);

void _start(void)
{
	void **p;

	nop();				/* placeholder for breakpoint */
	nop();

	p =  (int *) (_stack_frame_address() + REGWIN_SZ);
	__uClibc_main( (int) *p, *(p+1), *(p+2) );

/* If that didn't kill us, ... */

	asm("trap 0");
}

/*
 *	this was needed for gcc/g++-builds,  atexit was not getting included
 *	for some stupid reason,  this gets us a compiler
 */
// empty_func:
// //	rts
// 	.weak atexit
// atexit = empty_func
// 
// /*
//  *	a little bit of stuff to support C++
//  */
// 	.section .ctors,"aw"
// 	.align 4
// 	.global __CTOR_LIST__
// __CTOR_LIST__:
// 	.long -1
// 
// 	.section .dtors,"aw"
// 	.align 4
// 	.global __DTOR_LIST__
// __DTOR_LIST__:
// 	.long -1

