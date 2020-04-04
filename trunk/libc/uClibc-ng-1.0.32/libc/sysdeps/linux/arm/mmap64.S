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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <_lfs_64.h>
#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

#ifdef __NR_mmap2

/* The mmap2 system call takes six arguments, all in registers.  */
.text
.global mmap64
.type mmap64,%function
.align 2

#ifdef __ARM_EABI__
#if defined(THUMB1_ONLY)
.thumb_func
mmap64:
#ifdef __ARMEB__
/* Offsets are after pushing 3 words.  */
# define LOW_OFFSET  12 + 8 + 4
# define HIGH_OFFSET 12 + 8 + 0
#else
# define LOW_OFFSET  12 + 8 + 0
# define HIGH_OFFSET 12 + 8 + 4
#endif
	push	{r4, r5, r6}
	ldr	r6, [sp, $LOW_OFFSET]
	ldr	r5, [sp, $HIGH_OFFSET]
	lsl	r4, r6, #20		@ check that offset is page-aligned
	bne	.Linval
	lsr	r4, r5, #12		@ check for overflow
	bne	.Linval
	@ compose page offset
	lsr	r6, r6, #12
	lsl	r5, r5, #20
	orr	r5, r5, r6
	ldr	r4, [sp, #8]		@ load fd
	DO_CALL (mmap2)
	ldr	r1, =0xfffff000
	cmp	r0, r1
	bcs	.Lerror
	bx	lr
.Linval:
	ldr	r0, =-EINVAL
	pop	{r4, r5, r6}
.Lerror:
	push	{r3, lr}
	bl	__syscall_error
	POP_RET
.pool
#else /* !THUMB1_ONLY */
mmap64:
#ifdef __ARMEB__
# define LOW_OFFSET      8 + 4
/* The initial + 4 is for the stack postdecrement.  */
# define HIGH_OFFSET 4 + 8 + 0
#else
# define LOW_OFFSET      8 + 0
# define HIGH_OFFSET 4 + 8 + 4
#endif
	ldr	ip, [sp, $LOW_OFFSET]
	str	r5, [sp, #-4]!
	ldr	r5, [sp, $HIGH_OFFSET]
	str	r4, [sp, #-4]!
	movs	r4, ip, lsl $20		@ check that offset is page-aligned
	mov	ip, ip, lsr $12
	IT(t, eq)
	moveqs	r4, r5, lsr $12		@ check for overflow
	bne	.Linval
	ldr	r4, [sp, $8]		@ load fd
	orr	r5, ip, r5, lsl $20	@ compose page offset
	DO_CALL (mmap2)
	cmn	r0, $4096
	ldmfd	sp!, {r4, r5}
	IT(t, cc)
	BXC(cc, lr)
	b	__syscall_error
.Linval:
	mov	r0, $-EINVAL
	ldmfd	sp!, {r4, r5}
	b	__syscall_error
#endif
#else /* !__ARM_EABI__ */
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
	DO_CALL (mmap2)
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
	beq	HIDDEN_JUMPTARGET(mmap)
.Linval:
	mov	r0, $-EINVAL
	ldmfd	sp!, {r4, r5, lr}
	b	__error

__error:
	b	__syscall_error
#endif
.size mmap64,.-mmap64

#endif
