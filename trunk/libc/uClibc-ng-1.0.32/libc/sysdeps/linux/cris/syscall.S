/* Copyright (C) 2001, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"

	.syntax no_register_prefix

/* Make syscall (callno, ...) into a system call.  */

ENTRY (syscall)
#ifdef __arch_v32
	subq	4,sp
	move.d	r10,r9
	move	srp,[sp]
	addoq	8,sp,acr
	move.d	r11,r10
	move	[acr],mof
	addoq	12,sp,acr
	move.d	r12,r11
	move	[acr],srp
	addoq	4,sp,acr
	move.d	r13,r12
	move.d	[acr],r13
	break	13
	cmps.w	-4096,r10
	bhs	0f
	move	[sp+],srp
	Ret
	nop
#else
	push	srp
	move.d	r10,r9
	move.d	r11,r10
	move.d	r12,r11
	move.d	r13,r12
	move.d	[sp+4],r13
	move	[sp+8],mof
	move	[sp+12],srp
	break	13
	cmps.w	-4096,r10
	bhs	0f
	pop	srp
	Ret
	nop
#endif
PSEUDO_END (syscall)
