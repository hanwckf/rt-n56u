/* Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
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

#include <unwindbuf.h>
#include <sysdep.h>
#include <bits/kernel-features.h>
#include <lowlevellock.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"


	.comm	__fork_generation, 4, 4

	.text
	.globl	__pthread_once
	.type	__pthread_once,@function
	.protected	__pthread_once
	.align	5
	cfi_startproc
__pthread_once:
	mov.l	@r4, r0
	tst	#2, r0
	bt	1f
	rts
	 mov	#0, r0

1:
	mov.l	r12, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r12, 0)
	mov.l	r9, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r9, 0)
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r8, 0)
	sts.l	pr, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (pr, 0)
	mov	r5, r8
	mov	r4, r9

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:
	mov.l	@r4, r1
	mova	.Lgot, r0
	mov.l	.Lgot, r12
	add	r0, r12

5:
	mov	r1, r0

	tst	#2, r0
	bf	4f

	and	#3, r0
	mov.l	.Lfgen, r2
#ifdef __PIC__
	add	r12, r2
#endif
	mov.l	@r2, r3
	or	r3, r0
	or	#1, r0
	mov	r0, r3
	mov	r1, r5

	CMPXCHG (r5, @r4, r3, r2)
	bf	5b

	/* Check whether another thread already runs the initializer.  */
	mov	r2, r0
	tst	#1, r0
	bt	3f	/* No -> do it.  */

	/* Check whether the initializer execution was interrupted
	   by a fork.  */
	xor	r3, r0
	mov	#-4, r1	/* -4 = 0xfffffffc */
	tst	r1, r0
	bf	3f	/* Different for generation -> run initializer.  */

	/* Somebody else got here first.  Wait.  */
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_PRIVATE_FLAG|FUTEX_WAIT), r5
	extu.b	r5, r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
# if FUTEX_WAIT != 0
	mov	#FUTEX_WAIT, r0
	or	r0, r5
# endif
#endif
	mov	r3, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	bra	6b
	 nop

	.align	2
.Lgot:
	.long	_GLOBAL_OFFSET_TABLE_
#ifdef __PIC__
.Lfgen:
	.long	__fork_generation@GOTOFF
#else
.Lfgen:
	.long	__fork_generation
#endif

3:
	/* Call the initializer function after setting up the
	   cancellation handler.  Note that it is not possible here
	   to use the unwind-based cleanup handling.  This would require
	   that the user-provided function and all the code it calls
	   is compiled with exceptions.  Unfortunately this cannot be
	   guaranteed.  */
	add	#-UNWINDBUFSIZE, r15
	cfi_adjust_cfa_offset (UNWINDBUFSIZE)

	mov.l	.Lsigsetjmp, r1
	mov	#UWJMPBUF, r4
	add	r15, r4
	bsrf	r1
	 mov	#0, r5
.Lsigsetjmp0:
	tst	r0, r0
	bf	7f

	mov.l	.Lcpush, r1
	bsrf	r1
	 mov	r15, r4
.Lcpush0:

	/* Call the user-provided initialization function.  */
	jsr	@r8
	 nop

	/* Pop the cleanup handler.  */
	mov.l	.Lcpop, r1
	bsrf	r1
	 mov	r15, r4
.Lcpop0:

	add	#UNWINDBUFSIZE, r15
	cfi_adjust_cfa_offset (-UNWINDBUFSIZE)

	/* Sucessful run of the initializer.  Signal that we are done.  */
	INC (@r9, r2)
	/* Wake up all other threads.  */
	mov	r9, r4
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_PRIVATE_FLAG|FUTEX_WAKE), r5
	extu.b	r5, r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
	mov	#FUTEX_WAKE, r0
	or	r0, r5
#endif
	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

4:
	lds.l	@r15+, pr
	cfi_adjust_cfa_offset (-4)
	cfi_restore (pr)
	mov.l	@r15+, r8
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r8)
	mov.l	@r15+, r9
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r9)
	mov.l	@r15+, r12
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r12)
	rts
	 mov	#0, r0

7:
	/* __sigsetjmp returned for the second time.  */
	cfi_adjust_cfa_offset (UNWINDBUFSIZE+16)
	cfi_offset (r12, -4)
	cfi_offset (r9, -8)
	cfi_offset (r8, -12)
	cfi_offset (pr, -16)
	mov	#0, r7
	mov.l	r7, @r9
	mov	r9, r4
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_PRIVATE_FLAG|FUTEX_WAKE), r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
	mov	#FUTEX_WAKE, r0
	or	r0, r5
#endif
	extu.b	r5, r5
	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	.Lunext, r1
	bsrf	r1
	 mov	r15, r4
.Lunext0:
	/* NOTREACHED */
	sleep
	cfi_endproc

#ifndef __ASSUME_PRIVATE_FUTEX
.Lpfoff:
	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE
#endif
	.align	2
.Lsigsetjmp:
	.long	__sigsetjmp@PLT-(.Lsigsetjmp0-.)
.Lcpush:
	.long	HIDDEN_JUMPTARGET(__pthread_register_cancel)-.Lcpush0
.Lcpop:
	.long	HIDDEN_JUMPTARGET(__pthread_unregister_cancel)-.Lcpop0
.Lunext:
	.long	HIDDEN_JUMPTARGET(__pthread_unwind_next)-.Lunext0
	.size	__pthread_once,.-__pthread_once

	.globl	__pthread_once_internal
__pthread_once_internal = __pthread_once

	.globl	pthread_once
pthread_once = __pthread_once
