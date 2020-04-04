/* Copyright (C) 2005-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ERRNO_H        1
#include <bits/errno.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

.Lthread_start:						ASM_LINE_SEP

	/* r26, r25, r24, r23 are free since vfork has no arguments */
ENTRY(__vfork)
	/* We must not create a frame. When the child unwinds to call
	   exec it will clobber the same frame that the parent
	   needs to unwind.  */

	/* Save the PIC register. */
#ifdef __PIC__
	copy	%r19, %r25	/* parent */
#endif

	/* Syscall saves and restores all register states */
	ble	0x100(%sr2,%r0)
	ldi	__NR_vfork,%r20

	/* Check for error */
	ldi	-4096,%r1
	comclr,>>= %r1,%ret0,%r0        /* Note: unsigned compare. */
	b,n	.Lerror

	/* Return, and DO NOT restore rp. The child may have called
	   functions that updated the frame's rp. This works because
	   the kernel ensures rp is preserved across the vfork
	   syscall.  */
	bv,n	%r0(%rp)

.Lerror:
	/* Now we need a stack to call a function. We are assured
	   that there is no child now, so it's safe to create
	   a frame.  */
	stw	%rp, -20(%sp)
	.cfi_offset 2, -20
	stwm	%r3, 64(%sp)
	.cfi_def_cfa_offset -64
	.cfi_offset 3, 0
	stw	%sp, -4(%sp)

	sub	%r0,%ret0,%r3
	SYSCALL_ERROR_HANDLER
	/* Restore the PIC register (in delay slot) on error */
#ifdef __PIC__
	copy	%r25, %r19    /* parent */
#else
	nop
#endif
	/* Write syscall return into errno location */
	stw	%r3, 0(%ret0)
	ldw	-84(%sp), %rp
	bv	%r0(%rp)
	ldwm	-64(%sp), %r3

PSEUDO_END (__vfork)
libc_hidden_def(vfork)

weak_alias (__vfork, vfork)
