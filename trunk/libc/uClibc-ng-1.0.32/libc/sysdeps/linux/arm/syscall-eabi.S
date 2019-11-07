/* Copyright (C) 2005 Free Software Foundation, Inc.
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
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

/* In the EABI syscall interface, we don't need a special syscall to
   implement syscall().  It won't work reliably with 64-bit arguments
   (but that is true on many modern platforms).  */

.text
.global syscall
.type syscall,%function
.align 4
#if defined(THUMB1_ONLY)
.thumb_func
syscall:
	push	{r4, r5, r6, r7}
	mov	ip, r0
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	add	r7, sp, #(4 * 4)
	ldmia	r7!, {r3, r4, r5, r6}
	mov	r7, ip
	swi	0x0
	pop	{r4, r5, r6, r7}
	ldr	r1, =0xfffff000
	cmp	r0, r1
	bcs	1f
	bx lr
1:
	push	{r3, lr}
	bl	__syscall_error
	POP_RET
.pool
#else
syscall:
	mov	ip, sp
	stmfd	sp!, {r4, r5, r6, r7}
	mov	r7, r0
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	ldmfd	ip, {r3, r4, r5, r6}
	swi	0x0
	ldmfd	sp!, {r4, r5, r6, r7}
	cmn	r0, #4096
	IT(t, cc)
	BXC(cc, lr)
	b	__syscall_error
#endif

.size syscall,.-syscall
