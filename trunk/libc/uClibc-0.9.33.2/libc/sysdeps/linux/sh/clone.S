/* Copyright (C) 1999, 2000, 2003, 2004, 2007 Free Software Foundation, Inc.
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
#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#ifdef RESET_PID
#include <tcb-offsets.h>
#endif
/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     pid_t *ptid, void *tls, pid_t *ctid); */

	.text
ENTRY(__clone)
	/* sanity check arguments.  */
	tst	r4, r4
	bt/s	0f
	 tst	r5, r5
	bf	1f
0:
	bra	.Lsyscall_error
	 mov	#-EINVAL,r0
1:
	/* insert the args onto the new stack */
	mov.l	r7, @-r5
	/* save the function pointer as the 0th element */
	mov.l	r4, @-r5

	/* do the system call */
	mov	r6, r4
	mov.l	@r15, r6
	mov.l	@(8,r15), r7
	mov.l	@(4,r15), r0
	mov	#+SYS_ify(clone), r3
	trapa	#0x15
	mov     r0, r1
	mov	#-12, r2
	shad	r2, r1
	not	r1, r1			// r1=0 means r0 = -1 to -4095
	tst	r1, r1			// i.e. error in linux
	bf	.Lclone_end
.Lsyscall_error:
	SYSCALL_ERROR_HANDLER
.Lclone_end:
	tst	r0, r0
	bt	2f
.Lpseudo_end:
	rts
	 nop
2:
	/* terminate the stack frame */
	mov	#0, r14
#ifdef RESET_PID
	mov	r4, r0
	shlr16	r0
	tst	#1, r0			// CLONE_THREAD = (1 << 16)
	bf/s	4f
	 mov	r4, r0
	/* new pid */
	shlr8	r0
	tst	#1, r0			// CLONE_VM = (1 << 8)
	bf/s	3f
	 mov	#-1, r0
	mov	#+SYS_ify(getpid), r3
	trapa	#0x15
3:
	stc	gbr, r1
	mov.w	.Lpidoff, r2
	add	r1, r2
	mov.l	r0, @r2
	mov.w	.Ltidoff, r2
	add	r1, r2
	mov.l	r0, @r2
4:
#endif
	/* thread starts */
	mov.l	@r15, r1
	jsr	@r1
	 mov.l	@(4,r15), r4

	/* we are done, passing the return value through r0  */
	mov.l	.L3, r1
#ifdef SHARED
	mov.l	r12, @-r15
	sts.l	pr, @-r15
	mov	r0, r4
	mova	.LG, r0
	mov.l	.LG, r12
	add	r0, r12
	mova	.L3, r0
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
.LG:
	.long	_GLOBAL_OFFSET_TABLE_
.L3:
	.long	PLTJMP(C_SYMBOL_NAME(_exit))
#ifdef RESET_PID
.Lpidoff:
	.word	PID - TLS_PRE_TCB_SIZE
.Ltidoff:
	.word	TID - TLS_PRE_TCB_SIZE
#endif
PSEUDO_END (__clone)

weak_alias (__clone, clone)
