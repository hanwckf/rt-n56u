/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ralf Baechle <ralf@gnu.ai.mit.edu>, 1996.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <asm/unistd.h>
#include <sys/regdef.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <asm/asm.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg) */

	.text
.globl   __clone ;
	.align       2;
	.type         __clone,@function;
	.ent        __clone, 0;
	
__clone:
	.frame	    sp, 4*SZREG, sp
#ifdef __PIC__
	.set		noreorder
	.cpload		$25
	.set		reorder
	subu		sp,32
	.cprestore	16
#else
	subu		sp,32
#endif


	/* Sanity check arguments.  */
	li		v0,EINVAL
	beqz		a0,error	/* No NULL function pointers.  */
	beqz		a1,error	/* No NULL stack pointers.  */

	subu		a1,32		/* Reserve argument save space.  */
	sw		a0,0(a1)	/* Save function pointer.  */
	sw		a3,4(a1)	/* Save argument pointer.  */


	/* Do the system call */
	move		a0,a2
	li		v0,__NR_clone
	syscall

	bnez		a3,error
	beqz		v0,__thread_start

	/* Successful return from the parent */
	addiu		sp,32
	j $31  ; nop

	/* Something bad happened -- no child created */
error:
	addiu		sp,32

	/* uClibc change -- start */
	move		a0,v0		/* Pass return val to C function. */
	/* uClibc change -- stop */

#ifdef __PIC__
	la		t9,__syscall_error
	jr		t9
#else
	j		__syscall_error
#endif
	.end  __clone

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.  */

.globl  __thread_start;
	.align 2;
	.ent  __thread_start, 0;

__thread_start:
	/* cp is already loaded.  */
	.cprestore	16
	/* The stackframe has been created on entry of clone().  */
	/* Restore the arg for user's function.  */
	lw		t9,0(sp)	/* Function pointer.  */
	lw		a0,4(sp)	/* Argument pointer.  */

	/* Call the user's function.  */
	jal		t9

	/* Call _exit rather than doing it inline for breakpoint purposes.  */
	move		a0,v0
#ifdef __PIC__
	la		t9,_exit
	jalr		t9
#else
	jal		_exit
#endif
	.end  __thread_start

.weak clone;
	clone = __clone
