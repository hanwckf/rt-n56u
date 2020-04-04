/* Copyright (C) 1996, 1997, 2000, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ralf Baechle <ralf@linux-mips.org>, 1996.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <features.h>
#include <sys/asm.h>
#include <sysdep.h>
#define _ERRNO_H        1
#include <bits/errno.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     void *parent_tidptr, void *tls, void *child_tidptr) */

	.text
#if _MIPS_SIM == _ABIO32
# define EXTRA_LOCALS 1
#else
# define EXTRA_LOCALS 0
#endif
LOCALSZ= 4
FRAMESZ= (((NARGSAVE+LOCALSZ)*SZREG)+ALSZ)&ALMASK
GPOFF= FRAMESZ-(1*SZREG)
NESTED(clone,4*SZREG,sp)
#ifdef __PIC__
	SETUP_GP
#endif
	PTR_SUBU sp, FRAMESZ
	SETUP_GP64 (GPOFF, clone)
#ifdef __PIC__
	SAVE_GP (GPOFF)
#endif

	/* Sanity check arguments.  */
	li		v0,EINVAL
	beqz		a0,L(error)	/* No NULL function pointers.  */
	beqz		a1,L(error)	/* No NULL stack pointers.  */

	PTR_SUBU	a1,32		/* Reserve argument save space.  */
	PTR_S		a0,0(a1)	/* Save function pointer.  */
	PTR_S		a3,PTRSIZE(a1)	/* Save argument pointer.  */

	move		a0,a2

	/* Shuffle in the last three arguments - arguments 5, 6, and 7 to
	   this function, but arguments 3, 4, and 5 to the syscall.  */
#if _MIPS_SIM == _ABIO32
	PTR_L		a2,(FRAMESZ+PTRSIZE+PTRSIZE+16)(sp)
	PTR_S		a2,16(sp)
	PTR_L		a2,(FRAMESZ+16)(sp)
	PTR_L		a3,(FRAMESZ+PTRSIZE+16)(sp)
#else
	move		a2,a4
	move		a3,a5
	move		a4,a6
#endif

	/* Do the system call */
	li		v0,__NR_clone
	syscall

	bnez		a3,L(error)
	beqz		v0,L(thread_start)

	/* Successful return from the parent */
	RESTORE_GP64
	PTR_ADDU	sp, FRAMESZ
	j $31  ; nop

	/* Something bad happened -- no child created */
L(error):
#ifdef __PIC__
	PTR_LA		t9,__syscall_error
	RESTORE_GP64
	PTR_ADDU	sp, FRAMESZ
	/* uClibc change -- start */
	move            a0,v0           /* Pass return val to C function. */
	/* uClibc change -- stop */
	jr		t9
#else
	RESTORE_GP64
	PTR_ADDU	sp, FRAMESZ
	/* uClibc change -- start */
	move            a0,v0           /* Pass return val to C function. */
	/* uClibc change -- stop */
	j		__syscall_error
#endif
	END(clone)

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.  */

ENTRY(__thread_start)
L(thread_start):
	/* cp is already loaded.  */
	SAVE_GP (GPOFF)
	/* The stackframe has been created on entry of clone().  */

	/* Restore the arg for user's function.  */
	PTR_L		t9,0(sp)	/* Function pointer.  */
	PTR_L		a0,PTRSIZE(sp)	/* Argument pointer.  */

	/* Call the user's function.  */
	jal		t9

	/* Call _exit rather than doing it inline for breakpoint purposes.  */
	move		a0,v0
#ifdef __PIC__
	PTR_LA		t9,_exit
	jalr		t9
#else
	jal		_exit
#endif

	END(__thread_start)

weak_alias(clone, __clone)
