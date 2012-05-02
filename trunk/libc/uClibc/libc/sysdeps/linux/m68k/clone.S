/* Adapted from glibc */
/* Copyright (C) 1996, 1997 Free Software Foundation, Inc. */

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

/* int _clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

.text
.align 4
.type	clone,@function
.globl	clone;
clone:
.type	__clone,@function
.globl	__clone;
__clone:
	/* Sanity check arguments.  */
	movel   #-EINVAL, %d0
	movel   4(%sp), %d1             /* no NULL function pointers */
	movel	%d1, %a0
	tstl    %d1
	beq.w   __syscall_error
	movel   8(%sp), %d1             /* no NULL stack pointers */
	movel	%d1, %a1
	tstl    %d1
	beq.w   __syscall_error

	/* Allocate space and copy the argument onto the new stack.  */
	movel   16(%sp), -(%a1)

	/* Do the system call */
#if 1 /* defined (CONFIG_COLDFIRE) */
	movel   %d2, %d1                /* save %d2 and get stack pointer */
	movel   %a1, %d2
	movel   %d1, %a1
#else
	exg     %d2, %a1                /* save %d2 and get stack pointer */
#endif
	movel   12(%sp), %d1            /* get flags */
	movel   #__NR_clone, %d0
	trap    #0
#if 1 /* defined (CONFIG_COLDFIRE) */
	movel   %d2, %d1                /* restore %d2 */
	movel   %a1, %d2
	movel   %d1, %a1
#else
	exg     %d2, %a1                /* restore %d2 */
#endif

	tstl    %d0
	bmi.w   __syscall_error
	beq.w   thread_start

	rts

__syscall_error:
	negl	%d0
	movel	%d0, %sp@-
	lea		__errno_location-.-8, %a0
	jsr		%pc@(%a0)
	movel	%d0, %a0
	movel	%sp@+, %a0@
	moveq	#-1, %d0

	rts

thread_start:
	/*subl    %fp, %fp*/        /* terminate the stack frame */
	jsr     (%a0)
	movel   %d0, -(%sp)
	movel	#__NR_exit, %d0
	trap	#0
	/*jsr    exit*/

#if defined(__HAVE_ELF__)
	.weak clone
	clone = __clone
#else
	.set clone,__clone
#endif

