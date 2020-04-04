/* Copyright (C) 1992, 1994, 1997 Free Software Foundation, Inc.
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

#include <jmpbuf-offsets.h>

#define a0  $16

.globl  __longjmp;
.align 3;
.ent  __longjmp , 0;

__longjmp:
	.frame $30 , 0, $26
	.prologue 0

	mov     $17, $0
	ldq     $9, JB_S0*8(a0)
	ldq     $10, JB_S1*8(a0)
	ldq     $11, JB_S2*8(a0)
	ldq     $12, JB_S3*8(a0)
	ldq     $13, JB_S4*8(a0)
	ldq     $14, JB_S5*8(a0)
	ldq     $26, JB_PC*8(a0)
	ldq     $15, JB_FP*8(a0)
	ldq     $1, JB_SP*8(a0)
	ldt     $f2, JB_F2*8(a0)
	ldt     $f3, JB_F3*8(a0)
	ldt     $f4, JB_F4*8(a0)
	ldt     $f5, JB_F5*8(a0)
	ldt     $f6, JB_F6*8(a0)
	ldt     $f7, JB_F7*8(a0)
	ldt     $f8, JB_F8*8(a0)
	ldt     $f9, JB_F9*8(a0)
	cmoveq  $0, 1, $0
	mov     $1, $30
	ret

.end  __longjmp
libc_hidden_def(__longjmp)
