/* brk system call for Linux/ppc.
   Copyright (C) 1995, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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
#include "ppc_asm.h"
#define _ERRNO_H	1
#include <bits/errno.h>
#include <sys/syscall.h>

#ifdef __NR_brk

#ifdef __PIC__
.section .bss
	.align 4
	.globl __curbrk
	.hidden __curbrk
__curbrk: .skip 4
	.type __curbrk,@object
	.size __curbrk,4
#else
.comm __curbrk, 4,4
#endif

	.text
	.globl	brk
	.type	brk,@function
	.align	2

brk:
	stwu    r1,-16(r1)
	stw	r3,8(r1)
	li 0, __NR_brk;   
	sc
	lwz     r6,8(r1)
#ifdef __PIC__
	mflr    r4
# ifdef HAVE_ASM_PPC_REL16
	bcl	20,31,1f
1:	mflr	r5
	addis	r5,r5,_GLOBAL_OFFSET_TABLE_-1b@ha
	addi	r5,r5,_GLOBAL_OFFSET_TABLE_-1b@l
# else	
	bl      _GLOBAL_OFFSET_TABLE_@local-4
	mflr    r5
# endif
	lwz     r5,__curbrk@got(r5)
	mtlr    r4
	stw     r3,0(r5)
#else
	lis     r4,__curbrk@ha
	stw     r3,__curbrk@l(r4)
#endif
	cmplw   r6,r3
	addi    r1,r1,16
	li	r3,0
	blelr+
	li      r3,ENOMEM

	b	__syscall_error

	.size brk,.-brk

libc_hidden_def(brk)
#endif
