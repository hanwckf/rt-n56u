/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <features.h>
#include <asm/unistd.h>
#define _ERRNO_H	1
#include <bits/errno.h>


#if defined __HAVE_ELF__ && defined __HAVE_SHARED__ 
#define PLTJMP(_x)	_x##@PLT
#else
#define PLTJMP(_x)	_x
#endif


/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

        .text

.text
.align 4
.type	__clone,@function
.globl	__clone;
__clone:
	/* sanity check arguments.  */
	tst	r4, r4
	bt	0f
	tst	r5, r5
	bf/s	1f
	 mov	#+__NR_clone, r3
0:		
	bra __syscall_error
	 mov	#-EINVAL, r4

1:
	/* insert the args onto the new stack */
	mov.l	r7, @-r5
	/* save the function pointer as the 0th element */
	mov.l	r4, @-r5
	
	/* do the system call */
	mov	r6, r4
	trapa	#0x12
	mov     r0, r1
#ifdef __sh2__
// 12 arithmetic shifts for the crappy sh2, because shad doesn't exist!	
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
	shar	r1
#else		
	mov	#-12, r2
	shad	r2, r1
#endif
	not	r1, r1			// r1=0 means r0 = -1 to -4095
	tst	r1, r1			// i.e. error in linux
	bf/s	2f
	 tst	r0, r0
        bra __syscall_error
	 mov	r0, r4

2:
	bt	3f
	rts
	 nop
3:
	/* thread starts */
	mov.l	@r15, r1
	jsr	@r1
	 mov.l	@(4,r15), r4

	/* we are done, passing the return value through r0  */
	mov.l	.L1, r1
#if defined __HAVE_ELF__ && defined __HAVE_SHARED__ 
	mov.l	r12, @-r15
	sts.l	pr, @-r15
	mov	r0, r4
	mova	.LG, r0  /* .LG from syscall_error.S */
	mov.l	.LG, r12
	add	r0, r12
	mova	.L1, r0
	add	r0, r1
	jsr	@r1
	 nop
	lds.l	@r15+, pr
	rts
	 mov.l	@r15+, r12
#else
	jmp	@r1
	 mov	r0, r4
#endif
	.align	2
.L1:
	.long	PLTJMP(_exit)
.size __clone,.-__clone;

.globl  clone;
    clone = __clone

#include "syscall_error.S"
