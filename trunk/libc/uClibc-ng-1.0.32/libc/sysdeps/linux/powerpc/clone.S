/* Wrapper around clone system call.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <features.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <sysdep.h>

/* This is the only really unusual system call in PPC linux, but not
   because of any weirdness in the system call itself; because of
   all the freaky stuff we have to do to make the call useful.  */

/* int [r3] clone(int (*fn)(void *arg) [r3], void *child_stack [r4],
   		  int flags [r5], void *arg [r6], void *parent_tid [r7],
   		  void *tls [r8], void *child_tid [r9]); */

#ifdef __NR_clone
	.globl __clone
	.type __clone,@function
	.align 2

__clone:
	/* Check for child_stack == NULL || fn == NULL.  */
	cmpwi	cr0,r4,0
	cmpwi	cr1,r3,0
	cror	cr0*4+eq,cr1*4+eq,cr0*4+eq
	beq-	cr0,.Lbadargs

	/* Set up stack frame for parent.  */
	stwu	r1,-32(r1)
	cfi_adjust_cfa_offset (32)
#ifndef __ASSUME_FIXED_CLONE_SYSCALL
	stmw	r29,16(r1)
#else
	stmw	r30,16(r1)
#endif

	/* Set up stack frame for child.  */
	clrrwi	r4,r4,4
	li	r0,0
	stwu	r0,-16(r4)

	/* Save fn, args, stack across syscall.  */
	mr	r30,r3			/* Function in r30.  */
#ifndef __ASSUME_FIXED_CLONE_SYSCALL
	mr	r29,r4			/* Stack pointer in r29.  */
#endif
	mr	r31,r6			/* Argument in r31.  */

	/* 'flags' argument is first parameter to clone syscall. (The other
	   argument is the stack pointer, already in r4.)  */
	mr	r3,r5

	/* Move the parent_tid, child_tid and tls arguments. */
	mr	r5,r7
	mr	r6,r8
	mr	r7,r9

	/* End FDE now, because in the child the unwind info will be wrong.  */
	cfi_endproc

	/* Do the call.  */
	li 0, __NR_clone
	sc

	/* Check for child process.  */
	cmpwi	cr1,r3,0
	crandc	cr1*4+eq,cr1*4+eq,cr0*4+so
	bne-	cr1,.Lparent		/* The '-' is to minimise the race.  */

#ifndef __ASSUME_FIXED_CLONE_SYSCALL
	/* On at least mklinux DR3a5, clone() doesn't actually change
	   the stack pointer.  I'm pretty sure this is a bug, because
	   it adds a race condition if a signal is sent to a thread
	   just after it is created (in the previous three instructions).  */
	mr	r1,r29
#endif

	/* Call procedure.  */
	mtctr	r30
	mr	r3,r31
	bctrl
	/* Call _exit with result from procedure.  */
	b	HIDDEN_JUMPTARGET(_exit)

.Lparent:
	/* Parent.  Restore registers & return.  */
#ifndef __ASSUME_FIXED_CLONE_SYSCALL
	lmw	r29,16(r1)
#else
	lmw	r30,16(r1)
#endif
	addi	r1,r1,32
	bnslr+
	b	__syscall_error

.Lbadargs:
	li	r3,EINVAL
	b	__syscall_error

	cfi_startproc
	.size __clone,.-__clone
weak_alias(__clone, clone)
#endif
