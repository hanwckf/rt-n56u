/* Copyright (C) 1991, 1992, 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>

	.text
	.globl	syscall
	.type	syscall,@function
	.align  2

syscall:
	mr 0,3
	mr 3,4
	mr 4,5
	mr 5,6
	mr 6,7
	mr 7,8
	mr 8,9
	sc
	bnslr;

	b __syscall_error

	.size syscall,.-syscall
