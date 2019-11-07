/*
 * Copyright (C) 2016 by Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 * ported from GNU libc
 */

/* Copyright (C) 2005-2016 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>

#define PTR_REG(n)		x##n
#define PTR_LOG_SIZE		3
#define PTR_SIZE	(1<<PTR_LOG_SIZE)

#define ip0 x16
#define ip0l PTR_REG (16)
#define ip1 x17
#define lr  x30

/* RELA relocatons are 3 pointers */
#define RELA_SIZE (PTR_SIZE * 3)

 .text
 .globl _dl_linux_resolve
 .type _dl_linux_resolve, %function
 .align 2

_dl_linux_resolve:
	/* AArch64 we get called with:
	   ip0		&PLTGOT[2]
	   ip1		temp(dl resolver entry point)
	   [sp, #8]	lr
	   [sp, #0]	&PLTGOT[n]
	 */

	/* Save arguments.  */
	stp	x8, x9, [sp, #-(80+8*16)]!
	stp	x6, x7, [sp,  #16]
	stp	x4, x5, [sp,  #32]
	stp	x2, x3, [sp,  #48]
	stp	x0, x1, [sp,  #64]
	stp	q0, q1, [sp, #(80+0*16)]
	stp	q2, q3, [sp, #(80+2*16)]
	stp	q4, q5, [sp, #(80+4*16)]
	stp	q6, q7, [sp, #(80+6*16)]

	/* Get pointer to linker struct.  */
	ldr	PTR_REG (0), [ip0, #-PTR_SIZE]

	/* Prepare to call _dl_linux_resolver().  */
	ldr	x1, [sp, 80+8*16]	/* Recover &PLTGOT[n] */

	sub     x1, x1, ip0
	add     x1, x1, x1, lsl #1
	lsl     x1, x1, #3
	sub     x1, x1, #(RELA_SIZE<<3)
	lsr     x1, x1, #3

	/* Call resolver routine.  */
	bl	_dl_linux_resolver

	/* Save the return.  */
	mov	ip0, x0

	/* Get arguments and return address back.  */
	ldp	q0, q1, [sp, #(80+0*16)]
	ldp	q2, q3, [sp, #(80+2*16)]
	ldp	q4, q5, [sp, #(80+4*16)]
	ldp	q6, q7, [sp, #(80+6*16)]
	ldp	x0, x1, [sp, #64]
	ldp	x2, x3, [sp, #48]
	ldp	x4, x5, [sp, #32]
	ldp	x6, x7, [sp, #16]
	ldp	x8, x9, [sp], #(80+8*16)

	ldp	ip1, lr, [sp], #16

	/* Jump to the newly found address.  */
	br	ip0

.size _dl_linux_resolve, .-_dl_linux_resolve
