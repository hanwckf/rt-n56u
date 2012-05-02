/* Copyright (C) 1992, 1994, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <features.h>
#define _ASM
#define _SETJMP_H
#define __ASSEMBLY__
#include <bits/setjmp.h>

#define a0  $16

       .ent __sigsetjmp
       .global __sigsetjmp
__sigsetjmp:
       ldgp    $29, 0($27)

$sigsetjmp_local:
       subq    $30, 16, $30
       .frame  $26, 16, $26, 0
       stq     $26, 0($30)
       .mask   0x04000000, -16
	.prologue 1

	stq	$9, JB_S0*8(a0)
	stq	$10, JB_S1*8(a0)
	stq	$11, JB_S2*8(a0)
	stq	$12, JB_S3*8(a0)
	stq	$13, JB_S4*8(a0)
	stq	$14, JB_S5*8(a0)
	stq	$26, JB_PC*8(a0)
	addq	$30, 16, $1
	stq	$15, JB_FP*8(a0)
	stq	$1, JB_SP*8(a0)
	stt	$f2, JB_F2*8(a0)
	stt	$f3, JB_F3*8(a0)
	stt	$f4, JB_F4*8(a0)
	stt	$f5, JB_F5*8(a0)
	stt	$f6, JB_F6*8(a0)
	stt	$f7, JB_F7*8(a0)
	stt	$f8, JB_F8*8(a0)
	stt	$f9, JB_F9*8(a0)

	/* Call to C to (potentially) save our signal mask.  */
	jsr	$26, __sigjmp_save

	ldq	$26, 0($30)
	addq	$30, 16, $30
	ret

.end __sigsetjmp

/* Put these traditional entry points in the same file so that we can
   elide much of the nonsense in trying to jmp to the real function.  */

.globl  _setjmp;
.align 3;
.ent  _setjmp , 0;
_setjmp:
	.frame $30 , 0, $26
	ldgp	$29, 0($27)
	mov	0, $17
	br	$sigsetjmp_local
.end _setjmp

.globl  setjmp;
.align 3;
.ent  setjmp , 0;
setjmp:
	.frame $30 , 0, $26
	ldgp	$29, 0($27)
	mov	1, $17
	br	$sigsetjmp_local
.end setjmp

.weak   _setjmp
.weak   setjmp
