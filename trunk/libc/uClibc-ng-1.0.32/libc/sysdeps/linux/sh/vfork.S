/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   Copyright (C) 2001 Hewlett-Packard Australia

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, see <http://www.gnu.org/licenses/>.

 Derived in part from the Linux-8086 C library, the GNU C Library, and several
 other sundry sources.  Files within this library are copyright by their
 respective copyright holders.
*/

#include <sys/syscall.h>
#define _ERRNO_H
#include <bits/errno.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

.text
.globl	__vfork
.hidden	__vfork
.type	__vfork,@function
.align 4

__vfork:
	mov.w	.L2, r3
	trapa	#__SH_SYSCALL_TRAP_BASE
	mov     r0, r1
#ifdef __sh2__
/* 12 arithmetic shifts for the crappy sh2, because shad doesn't exist!	 */
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

	not	r1, r1			/* r1=0 means r0 = -1 to -4095 */
	tst	r1, r1			/* i.e. error in linux */
	bf	2f
	mov.w	.L1, r1
	cmp/eq	r1, r0
	bf/s	__syscall_error
	 mov	r0, r4

	/* If we don't have vfork, use fork.  */
	mov.w	.L3, r3
	trapa	#__SH_SYSCALL_TRAP_BASE
	mov     r0, r1
#ifdef __sh2__
/* 12 arithmetic shifts for the crappy sh2, because shad doesn't exist!	 */
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

	not	r1, r1			/* r1=0 means r0 = -1 to -4095 */
	tst	r1, r1			/* i.e. error in linux */
	bt/s	__syscall_error
	 mov	r0, r4
2:
	rts
	 nop

	.align	2
.L1:
	.word	-ENOSYS
.L2:
	.word	__NR_vfork
.L3:
	.word	__NR_fork

.size   __vfork, .-__vfork
weak_alias(__vfork,vfork)
libc_hidden_def(vfork)

#include "syscall_error.S"
