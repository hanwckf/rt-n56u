/* Copyright (C) 2001, 2004 Free Software Foundation, Inc.
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

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.
   void __longjmp (__jmp_buf env, int val).  */
.global __longjmp
.type   __longjmp,%function
__longjmp:
	/* Restore registers.  */
	movq (JB_RBX*8)(%rdi),%rbx
	movq (JB_RBP*8)(%rdi),%rbp
	movq (JB_R12*8)(%rdi),%r12
	movq (JB_R13*8)(%rdi),%r13
	movq (JB_R14*8)(%rdi),%r14
	movq (JB_R15*8)(%rdi),%r15
	/* Set return value for setjmp.  */
	test %esi,%esi
	mov $01,%eax
	cmove %eax,%esi
	mov %esi, %eax
	movq (JB_PC*8)(%rdi),%rdx
	movq (JB_RSP*8)(%rdi),%rsp
	jmpq *%rdx
.size __longjmp,.-__longjmp
libc_hidden_def(__longjmp)
