/* Copyright (C) 2003, 2004, 2005, 2007, 2008, 2009
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
#include <bits/kernel-features.h>
#include <lowlevellock.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"

	.text

#ifdef __ASSUME_PRIVATE_FUTEX
# define LOAD_PRIVATE_FUTEX_WAIT(reg,tmp,tmp2) \
	mov	#(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg; \
	extu.b	reg, reg
# define LOAD_PRIVATE_FUTEX_WAKE(reg,tmp,tmp2) \
	mov	#(FUTEX_WAKE | FUTEX_PRIVATE_FLAG), reg; \
	extu.b	reg, reg
# define LOAD_FUTEX_WAIT(reg,tmp,tmp2) \
	mov	#(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), tmp; \
	extu.b	tmp, tmp; \
	xor	tmp, reg
# define LOAD_FUTEX_WAIT_ABS(reg,tmp,tmp2) \
	mov	#(FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG), tmp; \
	extu.b	tmp, tmp; \
	mov	#(FUTEX_CLOCK_REALTIME >> 8), tmp2; \
	swap.b	tmp2, tmp2; \
	or	tmp2, tmp; \
	xor	tmp, reg
# define LOAD_FUTEX_WAKE(reg,tmp,tmp2) \
	mov	#(FUTEX_WAKE | FUTEX_PRIVATE_FLAG), tmp; \
	extu.b	tmp, tmp; \
	xor	tmp, reg
#else
# if FUTEX_WAIT == 0
#  define LOAD_PRIVATE_FUTEX_WAIT(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, reg	; \
	add	reg, tmp	; \
	bra	98f		; \
	 mov.l	@tmp, reg	; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:
# else
#  define LOAD_PRIVATE_FUTEX_WAIT(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, reg	; \
	add	reg, tmp	; \
	mov.l	@tmp, reg	; \
	bra	98f		; \
	 mov	#FUTEX_WAIT, tmp ; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:	or	tmp, reg
# endif
# define LOAD_PRIVATE_FUTEX_WAKE(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, reg	; \
	add	reg, tmp	; \
	mov.l	@tmp, reg	; \
	bra	98f		; \
	 mov	#FUTEX_WAKE, tmp ; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:	or	tmp, reg
# if FUTEX_WAIT == 0
#  define LOAD_FUTEX_WAIT(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, tmp2	; \
	add	tmp2, tmp	; \
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
	add	tmp2, tmp	; \
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
# define LOAD_FUTEX_WAIT_ABS(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, tmp2	; \
	add	tmp2, tmp	; \
	mov.l	@tmp, tmp2	; \
	bra	98f		; \
	 mov	#FUTEX_PRIVATE_FLAG, tmp ; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:	extu.b	tmp, tmp	; \
	xor	tmp, reg	; \
	and	tmp2, reg	; \
	mov	#FUTEX_WAIT_BITSET, tmp ; \
	mov	#(FUTEX_CLOCK_REALTIME >> 8), tmp2; \
	swap.b	tmp2, tmp2; \
	or	tmp2, tmp; \
	or	tmp, reg
# define LOAD_FUTEX_WAKE(reg,tmp,tmp2) \
	stc	gbr, tmp	; \
	mov.w	99f, tmp2	; \
	add	tmp2, tmp	; \
	mov.l	@tmp, tmp2	; \
	bra	98f		; \
	 mov	#FUTEX_PRIVATE_FLAG, tmp ; \
99:	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE ; \
98:	extu.b	tmp, tmp	; \
	xor	tmp, reg	; \
	and	tmp2, reg	; \
	mov	#FUTEX_WAKE, tmp ; \
	or	tmp, reg
#endif

	.globl	__lll_lock_wait_private
	.type	__lll_lock_wait_private,@function
	.hidden	__lll_lock_wait_private
	.align	5
	cfi_startproc
__lll_lock_wait_private:
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r8, 0)
	mov	r4, r6
	mov	r5, r8
	mov	#0, r7		/* No timeout.  */
	LOAD_PRIVATE_FUTEX_WAIT (r5, r0, r1)

	mov	#2, r4
	cmp/eq	r4, r6
	bf	2f

1:
	mov	r8, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

2:
	mov	#2, r6
	XCHG (r6, @r8, r2)
	tst	r2, r2
	bf	1b

	mov.l	@r15+, r8
	rts
	 mov	r2, r0
	cfi_endproc
	.size	__lll_lock_wait_private,.-__lll_lock_wait_private

#ifdef NOT_IN_libc
	.globl	__lll_lock_wait
	.type	__lll_lock_wait,@function
	.hidden	__lll_lock_wait
	.align	5
	cfi_startproc
__lll_lock_wait:
	mov.l	r9, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r9, 0)
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r8, 0)
	mov	r6, r9
	mov	r4, r6
	mov	r5, r8
	mov	#0, r7		/* No timeout.  */
	mov	r9, r5
	LOAD_FUTEX_WAIT (r5, r0, r1)

	mov	#2, r4
	cmp/eq	r4, r6
	bf	2f

1:
	mov	r8, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

2:
	mov	#2, r6
	XCHG (r6, @r8, r2)
	tst	r2, r2
	bf	1b

	mov.l	@r15+, r8
	mov.l	@r15+, r9
	ret
	 mov	r2, r0
	cfi_endproc
	.size	__lll_lock_wait,.-__lll_lock_wait

	/*      r5  (r8): futex
		r7 (r11): flags
		r6  (r9): timeout
		r4 (r10): futex value
	*/
	.globl	__lll_timedlock_wait
	.type	__lll_timedlock_wait,@function
	.hidden	__lll_timedlock_wait
	.align	5
	cfi_startproc
__lll_timedlock_wait:
	mov.l	r12, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r12, 0)

# ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	mov.l	.Lhave, r1
#  ifdef __PIC__
	mova	.Lgot, r0
	mov.l	.Lgot, r12
	add	r0, r12
	add	r12, r1
#  endif
	mov.l	@r1, r0
	tst	r0, r0
	bt	.Lreltmo
# endif

	mov	r4, r2
	mov	r5, r4
	mov	r7, r5
	mov	r6, r7
	LOAD_FUTEX_WAIT_ABS (r5, r0, r1)

	mov	#2, r6
	cmp/eq	r6, r2
	bf/s	2f
	 mov	r6, r2

1:
	mov	#2, r6
	mov	#-1, r1
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x16
	SYSCALL_INST_PAD
	mov	r0, r6

2:
	XCHG	(r2, @r4, r3)	/* NB:   lock is implied */

	tst	r3, r3
	bt/s	3f
	 mov	r6, r0

	cmp/eq	#-ETIMEDOUT, r0
	bt	4f
	cmp/eq	#-EINVAL, r0
	bf	1b
4:
	neg	r0, r3
3:
	mov	r3, r0
	rts
	 mov.l	@r15+, r12

	.align	2
# ifndef __ASSUME_FUTEX_CLOCK_REALTIME
# ifdef __PIC__
.Lgot:
	.long	_GLOBAL_OFFSET_TABLE_
.Lhave:
	.long	__have_futex_clock_realtime@GOTOFF
# else
.Lhave:
	.long	__have_futex_clock_realtime
# endif

.Lreltmo:
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

	mov	#2, r2
	XCHG (r2, @r8, r3)

	tst	r3, r3
	bt	6f

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
	bf	2f		/* Time is already up.  */

	mov.l	r2, @r15	/* Store relative timeout.  */
	mov.l	r3, @(4,r15)

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

	mov	#2, r2
	XCHG (r2, @r8, r3)

	tst	r3, r3
	bt/s	6f
	 mov	#-ETIMEDOUT, r1
	cmp/eq	r5, r1
	bf	1b

2:	mov	#ETIMEDOUT, r3

6:
	mov	r3, r0
	add	#8, r15
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r10
	mov.l	@r15+, r11
	rts
	 mov.l	@r15+, r12

3:
	mov.l	@r15+, r12
	rts
	 mov	#EINVAL, r0
# endif
	cfi_endproc

.L1k:
	.word	1000
	.align	2
.L1g:
	.long	1000000000

	.size	__lll_timedlock_wait,.-__lll_timedlock_wait
#endif

	.globl	__lll_unlock_wake_private
	.type	__lll_unlock_wake_private,@function
	.hidden	__lll_unlock_wake_private
	.align	5
	cfi_startproc
__lll_unlock_wake_private:
	LOAD_PRIVATE_FUTEX_WAKE (r5, r0, r1)
	mov	#1, r6		/* Wake one thread.  */
	mov	#0, r7
	mov.l	r7, @r4		/* Stores 0.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	rts
	 nop
	cfi_endproc
	.size	__lll_unlock_wake_private,.-__lll_unlock_wake_private

#ifdef NOT_IN_libc
	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
	.align	5
	cfi_startproc
__lll_unlock_wake:
	LOAD_FUTEX_WAKE (r5, r0, r1)
	mov	#1, r6		/* Wake one thread.  */
	mov	#0, r7
	mov.l	r7, @r4		/* Stores 0.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	rts
	 nop
	cfi_endproc
	.size	__lll_unlock_wake,.-__lll_unlock_wake

	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
	.align	5
	cfi_startproc
__lll_timedwait_tid:
	mov.l	r9, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r9, 0)
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r8, 0)
	mov	r4, r8
	mov	r5, r9

	/* Stack frame for the timespec and timeval structs.  */
	add	#-8, r15
	cfi_adjust_cfa_offset(8)

2:
	/* Get current time.  */
	mov	r15, r4
	mov	#0, r5
	mov	#__NR_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(4,r15), r0
	mov.w	.L1k2, r1
	dmulu.l	r0, r1		/* Micro seconds to nano seconds.  */
	mov.l	@r9, r2
	mov.l	@(4,r9), r3
	mov.l	@r15, r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	5f
	mov.l	.L1g2, r1
	add	r1, r3
	add	#-1, r2
5:
	cmp/pz	r2
	bf	6f		/* Time is already up.  */

	mov.l	r2, @r15	/* Store relative timeout.  */
	mov.l	r3, @(4,r15)

	mov.l	@r8, r2
	tst	r2, r2
	bt	4f

	mov	r8, r4
	/* XXX The kernel so far uses global futex for the wakeup at
	   all times.  */
	mov	#0, r5
	extu.b	r5, r5
	mov	r2, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	@r8, r2
	tst	r2, r2
	bf	1f
4:
	mov	#0, r0
3:
	add	#8, r15
	mov.l	@r15+, r8
	rts
	 mov.l	@r15+, r9
1:
	/* Check whether the time expired.  */
	mov	#-ETIMEDOUT, r1
	cmp/eq	r0, r1
	bf	2b
6:
	bra	3b
	 mov	#ETIMEDOUT, r0
	cfi_endproc

.L1k2:
	.word	1000
	.align	2
.L1g2:
	.long	1000000000
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
#endif
