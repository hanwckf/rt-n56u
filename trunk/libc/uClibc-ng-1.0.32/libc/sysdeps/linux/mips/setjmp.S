/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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

#include <sys/regdef.h>
#include <sys/asm.h>

/* The function __sigsetjmp_aux saves all the registers, but it can't
   reliably access the stack or frame pointers, so we pass them in as
   extra arguments.  */
#ifdef __PIC__
	.option pic2
#endif


.text
.global __sigsetjmp
.align 2;
.ent __sigsetjmp,0;
.type __sigsetjmp,@function

__sigsetjmp:
#ifdef __PIC__
	.set	noreorder
#if _MIPS_SIM == _MIPS_SIM_ABI32
	.cpload	t9
#else
	.cpsetup t9, v0, __sigsetjmp
#endif
	.set	reorder
#endif
	move	a2, sp
#ifdef fp
	move	a3, fp
#else
	move	a3, $fp
#endif
#ifdef __PIC__
	PTR_LA	t9, __sigsetjmp_aux
#if _MIPS_SIM != _MIPS_SIM_ABI32
	.cpreturn
	move 	a4, gp
#endif
	jr	t9
#else
	j	__sigsetjmp_aux
#endif
	.end __sigsetjmp
