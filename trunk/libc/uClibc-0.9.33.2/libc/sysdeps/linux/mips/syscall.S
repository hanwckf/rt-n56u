/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
#include <sys/asm.h>
#include <sys/regdef.h>

#ifdef __PIC__
	.option pic2
#endif
.text
.align  2
.globl  syscall
.type   syscall,@function
.ent    syscall
syscall:
#ifdef __PIC__
	SETUP_GP
#endif
	move	v0, a0		/* Load system call number from first arg.  */
	move	a0, a1		/* Move the next three args up a register.  */
	move	a1, a2
	move	a2, a3
     	/* Load the remaining possible args (up to 7) from the stack.  */
#ifdef __mips64
	move	a3, a4
	move	a4, a5
	move	a5, a6
	move	a6, a7
	daddiu	sp,sp,-16
	sd	v0,0*8(sp)
	ld	v0,0*8(sp)	/* for system call restarts */
#else
	lw	a3,4*4(sp)
	lw	t0,5*4(sp)
	lw	t1,6*4(sp)
	lw	t2,7*4(sp)
	addiu	sp,sp,-32
	sw	t0,4*4(sp)
	sw	t1,5*4(sp)
	sw	t2,6*4(sp)
	sw	v0,7*4(sp)
	lw	v0,7*4(sp)	/* for system call restarts */
#endif
	syscall			/* Do the system call.  */
#ifdef __mips64
	daddiu	sp,sp,16
#else
	addiu	sp,sp,32
#endif
	bnez	a3, 1f
     	j ra			/* Return to caller.  */
1:
	move	a0,v0		/* Pass return val to C function. */

#ifdef __PIC__
	SETUP_GP64(v0, syscall)
	PTR_LA	t9, __syscall_error
	RESTORE_GP64
	jr	t9
#else
	j	__syscall_error
#endif
.end    syscall
.size   syscall,.-syscall
