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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* vfork() is just a special case of clone().  */

#include <sys/asm.h>
#include <sysdep.h>

#ifndef SAVE_PID
#define SAVE_PID
#endif

#ifndef RESTORE_PID
#define RESTORE_PID
#endif

#ifdef __NR_fork

/* int vfork() */

	.text
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
#ifdef PROF
# if (_MIPS_SIM != _ABIO32)
	PTR_S		a5, GPOFF(sp)
# endif
	.set		noat
	move		$1, ra
# if (_MIPS_SIM == _ABIO32)
	subu		sp,sp,8
# endif
	jal		_mcount
	.set		at
# if (_MIPS_SIM != _ABIO32)
	PTR_L		a5, GPOFF(sp)
# endif
#endif

	PTR_ADDU	sp, FRAMESZ

	SAVE_PID

	li		a0, 0x4112	/* CLONE_VM | CLONE_VFORK | SIGCHLD */
	move		a1, sp

	/* Do the system call */
	li		v0,__NR_clone
	syscall

	RESTORE_PID

	bnez		a3,L(error)

	/* Successful return from the parent or child.  */
	RESTORE_GP64
	j		ra
	nop

	/* Something bad happened -- no child created.  */
L(error):
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
libc_hidden_weak(vfork)

#endif
