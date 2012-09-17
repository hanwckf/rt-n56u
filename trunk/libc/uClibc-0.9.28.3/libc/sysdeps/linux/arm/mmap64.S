/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <features.h>
#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_mmap2

/* The mmap2 system call takes six arguments, all in registers.  */
.text
.global mmap64
.type mmap64,%function
.align 4
mmap64:
	stmfd	sp!, {r4, r5, lr}
	ldr	r5, [sp, $16]
	ldr	r4, [sp, $12]
	movs	ip, r5, lsl $20		@ check that offset is page-aligned
	bne	.Linval
	ldr	ip, [sp, $20]
	mov	r5, r5, lsr $12
	orr	r5, r5, ip, lsl $20	@ compose page offset
	movs	ip, ip, lsr $12
	bne	.Linval			@ check for overflow
	mov	ip, r0
	swi	__NR_mmap2
	cmn	r0, $4096
	ldmccfd	sp!, {r4, r5, pc}
	cmn	r0, $ENOSYS
	ldmnefd	sp!, {r4, r5, lr}
	bne	__error
	/* The current kernel does not support mmap2.  Fall back to plain
	   mmap if the offset is small enough.  */
	ldr	r5, [sp, $20]
	mov	r0, ip			@ first arg was clobbered
	teq	r5, $0
	ldmeqfd	sp!, {r4, r5, lr}
#ifdef __PIC__
	beq	mmap(PLT)
#else
	beq	mmap
#endif
.Linval:
	mov	r0, $-EINVAL
	ldmfd	sp!, {r4, r5, lr}
	b	__error

__error:
	b	__syscall_error

.size mmap64,.-mmap64

#endif
