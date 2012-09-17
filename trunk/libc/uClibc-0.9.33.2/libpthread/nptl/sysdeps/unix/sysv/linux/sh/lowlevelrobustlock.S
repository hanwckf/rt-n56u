/* Copyright (C) 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <pthread-errnos.h>
#include <lowlevellock.h>
#include <lowlevelrobustlock.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"

	.text

#define FUTEX_WAITERS		0x80000000
#define FUTEX_OWNER_DIED	0x40000000

#ifdef __ASSUME_PRIVATE_FUTEX
# define LOAD_FUTEX_WAIT(reg,tmp,tmp2) \
	mov	#(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), tmp; \
	extu.b	tmp, tmp; \
	xor	tmp, reg
#else
# if FUTEX_WAIT == 0
#  define LOAD_FUTEX_WAIT(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, tmp2	; \
	add	tmp2, tmp 	; \
	mov.l	@tmp, tmp2	; \
	bra	98f		; \
	 mov	#FUTEX_PRIVATE_FLAG, tmp ; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:	extu.b	tmp, tmp	; \
	xor	tmp, reg	; \
	and	tmp2, reg
# else
#  define LOAD_FUTEX_WAIT(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, tmp2	; \
	add	tmp2, tmp 	; \
	mov.l	@tmp, tmp2	; \
	bra	98f		; \
	 mov	#FUTEX_PRIVATE_FLAG, tmp ; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:	extu.b	tmp, tmp	; \
	xor	tmp, reg	; \
	and	tmp2, reg	; \
	mov	#FUTEX_WAIT, tmp ; \
	or	tmp, reg
# endif
#endif

	.globl	__lll_robust_lock_wait
	.type	__lll_robust_lock_wait,@function
	.hidden	__lll_robust_lock_wait
	.align	5
	cfi_startproc
__lll_robust_lock_wait:
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r8, 0)
	mov	r5, r8
	mov	#0, r7		/* No timeout.  */
	mov	r6, r5
	LOAD_FUTEX_WAIT (r5, r0, r1)

4:
	mov	r4, r6
	mov.l	.L_FUTEX_WAITERS, r0
	or	r0, r6
	shlr	r0		/* r0 = FUTEX_OWNER_DIED */
	tst	r0, r4
	bf/s	3f
	 cmp/eq	r4, r6
	bt	1f

	CMPXCHG (r4, @r8, r6, r2)
	bf	2f

1:
	mov	r8, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	@r8, r2

2:
	tst	r2, r2
	bf/s	4b
	 mov	r2, r4

	stc	gbr, r1
	mov.w	.Ltidoff, r2
	add	r2, r1
	mov.l	@r1, r6
	mov	#0, r3
	CMPXCHG (r3, @r8, r6, r4)
	bf	4b
	mov	#0, r4

3:
	mov.l	@r15+, r8
	ret
	 mov	r4, r0
	cfi_endproc
	.align	2
.L_FUTEX_WAITERS:
	.long	FUTEX_WAITERS
.Ltidoff:
	.word	TID - TLS_PRE_TCB_SIZE
	.size	__lll_robust_lock_wait,.-__lll_robust_lock_wait


	.globl	__lll_robust_timedlock_wait
	.type	__lll_robust_timedlock_wait,@function
	.hidden	__lll_robust_timedlock_wait
	.align	5
	cfi_startproc
__lll_robust_timedlock_wait:
	/* Check for a valid timeout value.  */
	mov.l	@(4,r6), r1
	mov.l	.L1g, r0
	cmp/hs	r0, r1
	bt	3f

	mov.l	r11, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r11, 0)
	mov.l	r10, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r10, 0)
	mov.l	r9, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r9, 0)
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r8, 0)
	mov	r7, r11
	mov	r4, r10
	mov	r6, r9
	mov	r5, r8

	/* Stack frame for the timespec and timeval structs.  */
	add	#-8, r15
	cfi_adjust_cfa_offset(8)

1:
	/* Get current time.  */
	mov	r15, r4
	mov	#0, r5
	mov	#__NR_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(4,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Micro seconds to nano seconds.  */
	mov.l	@r9, r2
	mov.l	@(4,r9), r3
	mov.l	@r15, r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	4f
	mov.l	.L1g, r1
	add	r1, r3
	add	#-1, r2
4:
	cmp/pz	r2
	bf	8f		/* Time is already up.  */

	mov.l	r2, @r15	/* Store relative timeout.  */
	mov.l	r3, @(4,r15)

	mov	r10, r6
	mov.l	.L_FUTEX_WAITERS2, r0
	or	r0, r6
	shlr	r0		/* r0 = FUTEX_OWNER_DIED */
	tst	r0, r4
	bf/s	6f
	 cmp/eq	r4, r6
	bt	2f

	CMPXCHG (r4, @r8, r6, r2)
	bf/s	5f
	 mov	#0, r5

2:
	mov	r8, r4
	mov	r11, r5
	LOAD_FUTEX_WAIT (r5, r0, r1)
	mov	r10, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	mov	r0, r5

	mov.l	@r8, r2

5:
	tst	r2, r2
	bf/s	7f
	 mov	r2, r10

	stc	gbr, r1
	mov.w	.Ltidoff2, r2
	add	r2, r1
	mov.l	@r1, r4
	mov	#0, r3
	CMPXCHG (r3, @r8, r4, r10)
	bf	7f
	mov	#0, r0

6:
	add	#8, r15
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r10
	rts
	 mov.l	@r15+, r11

7:
	/* Check whether the time expired.  */
	mov	#-ETIMEDOUT, r1
	cmp/eq	r5, r1
	bf	1b

8:
	bra	6b
	 mov	#ETIMEDOUT, r0
3:
	rts
	 mov	#EINVAL, r0
	cfi_endproc
	.align	2
.L_FUTEX_WAITERS2:
	.long	FUTEX_WAITERS
.L1g:
	.long	1000000000
.Ltidoff2:
	.word	TID - TLS_PRE_TCB_SIZE
.L1k:
	.word	1000
	.size	__lll_robust_timedlock_wait,.-__lll_robust_timedlock_wait
