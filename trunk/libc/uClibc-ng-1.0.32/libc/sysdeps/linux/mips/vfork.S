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

/* vfork() is just a special case of clone().  */

#include <sys/syscall.h>
#include <sys/asm.h>
#include <sysdep.h>

#ifdef __NR_fork

/* int vfork() */

	.text
	.hidden __vfork
LOCALSZ= 1
FRAMESZ= (((NARGSAVE+LOCALSZ)*SZREG)+ALSZ)&ALMASK
GPOFF= FRAMESZ-(1*SZREG)
NESTED(__vfork,FRAMESZ,sp)
#ifdef __PIC__
	SETUP_GP
#endif
	PTR_SUBU sp, FRAMESZ
	SETUP_GP64 (a5, __vfork)
#ifdef __PIC__
	SAVE_GP (GPOFF)
#endif

	PTR_ADDU	sp, FRAMESZ

	li		a0, 0x4112	/* CLONE_VM | CLONE_VFORK | SIGCHLD */
	move		a1, sp

	/* Do the system call */
	li		v0,__NR_clone
	syscall

	bnez		a3,L(error)

	/* Successful return from the parent or child.  */
	RESTORE_GP64
	j		ra
	nop

	/* Something bad happened -- no child created.  */
L(error):
	move	a0, v0
#ifdef __PIC__
	PTR_LA		t9, __syscall_error
	RESTORE_GP64
	jr		t9
#else
	RESTORE_GP64
	j		__syscall_error
#endif
	END(__vfork)

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)

#endif
