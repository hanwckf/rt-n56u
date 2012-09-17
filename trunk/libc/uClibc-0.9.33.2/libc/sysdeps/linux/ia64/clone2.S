/* Copyright (C) 2000, 2001, 2003, 2004, 2006 Free Software Foundation, Inc.
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

#include "sysdep.h"

#include "sysdep.h"
#include <asm/errno.h>


/* int  __clone2(int (*fn) (void *arg), void *child_stack_base, 	*/
/* 	         size_t child_stack_size, int flags, void *arg,		*/
/*	         pid_t *parent_tid, void *tls, pid_t *child_tid)	*/

#define CHILD	p8
#define PARENT	p9

ENTRY(__clone2)
	.prologue
	alloc r2=ar.pfs,8,1,6,0
	cmp.eq p6,p0=0,in0
	cmp.eq p7,p0=0,in1
	mov r8=EINVAL
	mov out0=in3		/* Flags are first syscall argument.	*/
	mov out1=in1		/* Stack address.			*/
(p6)	br.cond.spnt.many __syscall_error	/* no NULL function pointers */
(p7)	br.cond.spnt.many __syscall_error	/* no NULL stack pointers */
	;;
	mov out2=in2		/* Stack size.				*/
	mov out3=in5		/* Parent TID Pointer			*/
	mov out4=in7		/* Child TID Pointer			*/
 	mov out5=in6		/* TLS pointer				*/
	/*
	 * clone2() is special: the child cannot execute br.ret right
	 * after the system call returns, because it starts out
	 * executing on an empty stack.  Because of this, we can't use
	 * the new (lightweight) syscall convention here.  Instead, we
	 * just fall back on always using "break".
	 *
	 * Furthermore, since the child starts with an empty stack, we
	 * need to avoid unwinding past invalid memory.  To that end,
	 * we'll pretend now that __clone2() is the end of the
	 * call-chain.  This is wrong for the parent, but only until
	 * it returns from clone2() but it's better than the
	 * alternative.
	 */
	mov r15=SYS_ify (clone2)
	.save rp, r0
	break __BREAK_SYSCALL
	.body
        cmp.eq p6,p0=-1,r10
	cmp.eq CHILD,PARENT=0,r8 /* Are we the child?   */
(p6)	br.cond.spnt.many __syscall_error
	;;
(CHILD)	mov loc0=gp
(PARENT) ret
	;;
#ifdef RESET_PID
	tbit.nz p6,p0=in3,16	/* CLONE_THREAD */
	tbit.z p7,p10=in3,8	/* CLONE_VM */
(p6)	br.cond.dptk 1f
	;;
	mov r15=SYS_ify (getpid)
(p10)	addl r8=-1,r0
(p7)	break __BREAK_SYSCALL
	;;
	add r9=PID,r13
	add r10=TID,r13
	;;
	st4 [r9]=r8
	st4 [r10]=r8
	;;
#endif
1:	ld8 out1=[in0],8	/* Retrieve code pointer.	*/
	mov out0=in4		/* Pass proper argument	to fn */
	;;
	ld8 gp=[in0]		/* Load function gp.		*/
	mov b6=out1
	br.call.dptk.many rp=b6	/* Call fn(arg) in the child 	*/
	;;
	mov out0=r8		/* Argument to _exit		*/
	mov gp=loc0
	.globl HIDDEN_JUMPTARGET(_exit)
	br.call.dpnt.many rp=HIDDEN_JUMPTARGET(_exit)
				/* call _exit with result from fn.	*/
	ret			/* Not reached.		*/
PSEUDO_END(__clone2)

/* For now we leave __clone undefined.  This is unlikely to be a	*/
/* problem, since at least the i386 __clone in glibc always failed	*/
/* with a 0 sp (eventhough the kernel explicitly handled it).		*/
/* Thus all such calls needed to pass an explicit sp, and as a result,	*/
/* would be unlikely to work on ia64.					*/
