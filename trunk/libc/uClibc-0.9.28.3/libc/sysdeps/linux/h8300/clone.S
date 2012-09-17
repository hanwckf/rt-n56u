/* Adapted from glibc */
/* Copyright (C) 1996, 1997 Free Software Foundation, Inc. */

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

/* int _clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

#ifdef __H8300H__
	.h8300h
#endif
#ifdef __H8300S__
	.h8300s
#endif

.text
.globl	_clone
.globl	___clone
_clone:
___clone:
	/* Sanity check arguments.  */
	mov.l	#-EINVAL,er3
	mov.l	er0,er0			/* no NULL function pointers */
	beq	__syscall_error
	mov.l	er1,er1			/* no NULL stack pointers */
	beq	__syscall_error

	/* Allocate space and copy the argument onto the new stack.  */
	mov.l	@(4:16,sp),er3
	mov.l	er3,@-er1

	/* Do the system call */
	mov.l	er0,er3			/* er3 = child entry */
	mov.l	er1,er0
	mov.l	er2,er1			/* er1 = flags */
	mov.l	er0,er2			/* er2 = child sp */
	mov.l	#__NR_clone,r0
	trapa	#0
	mov.l	er0,er0
	bmi	__syscall_error
	beq	thread_start

	rts

__syscall_error:
	neg.l	er0
	mov.l	er0,@-sp
#if !defined(__PIC__)
	jsr	@__errno_location
#else
	mov.l	@(__errno_location@GOTOFF,er5),er1
	jsr	@er1
#endif
	mov.l	@sp,er1
	mov.l	er1,@er0
	sub.l	er0,er0
	dec.l	#1,er0

	rts

thread_start:
	mov.l	@sp+,er0		/* restore args */
	jsr     @er3
	mov.l	er0,er1
	mov.l	#__NR_exit,er0
	trapa	#0

#if defined(__HAVE_ELF__)
	.weak clone
	clone = __clone
#else
	.set clone,__clone
#endif

