/* setjmp for x86-64.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
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

.global __sigsetjmp
.type   __sigsetjmp,%function
__sigsetjmp:
	/* Save registers.  */
	movq %rbx, (JB_RBX*8)(%rdi)
	movq %rbp, (JB_RBP*8)(%rdi)
	movq %r12, (JB_R12*8)(%rdi)
	movq %r13, (JB_R13*8)(%rdi)
	movq %r14, (JB_R14*8)(%rdi)
	movq %r15, (JB_R15*8)(%rdi)
	leaq 8(%rsp), %rdx	/* Save SP as it will be after we return.  */
	movq %rdx, (JB_RSP*8)(%rdi)
	movq (%rsp), %rax	/* Save PC we are returning to now.  */
	movq %rax, (JB_PC*8)(%rdi)

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef	__PIC__
	jmp __sigjmp_save@PLT
#else
	jmp __sigjmp_save
#endif
.size __sigsetjmp,.-__sigsetjmp
libc_hidden_def(__sigsetjmp)
