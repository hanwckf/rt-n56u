/* Copyright (C) 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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
#include <lowlevellock.h>
#include <lowlevelcond.h>
#include <pthread-errnos.h>
#include <bits/kernel-features.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"

	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.protected	__pthread_cond_timedwait
	.align	5
__pthread_cond_timedwait:
.LSTARTCODE:
	mov.l	r8, @-r15
.Lpush_r8:
	mov.l	r9, @-r15
.Lpush_r9:
	mov.l	r10, @-r15
.Lpush_r10:
	mov.l	r11, @-r15
.Lpush_r11:
	mov.l	r12, @-r15
.Lpush_r12:
	mov.l	r13, @-r15
.Lpush_r13:
	sts.l	pr, @-r15
.Lpush_pr:
	add	#-64, r15
.Lalloc:
	mov	r4, r8
	mov	r5, r9
	mov	r6, r13
#ifdef __PIC__
	mova	.Lgot0, r0
	mov.l	.Lgot0, r12
	add	r0, r12
#endif

	mov.l	@(4,r13), r0
	mov.l	.L1g, r1
	cmp/hs	r1, r0
	bf	0f
	bra	18f
	 mov	#EINVAL, r0
0:
	/* Get internal lock.  */
	mov	#0, r3
	mov	#1, r4
#if cond_lock != 0
	CMPXCHG (r3, @(cond_lock,r8), r4, r2)
#else
	CMPXCHG (r3, @r8, r4, r2)
#endif
	bt	2f
	bra	1f
	 nop
#ifdef __PIC__
	.align	2
.Lgot0:
	.long	_GLOBAL_OFFSET_TABLE_
#endif

2:
	/* Store the reference to the mutex.  If there is already a
	   different value in there this is a bad user bug.  */
	mov.l	@(dep_mutex,r8),r0
	cmp/eq	#-1, r0
	bt	17f
	mov.l	r9, @(dep_mutex,r8)

17:
	/* Unlock the mutex.  */
	mov.l	.Lmunlock1, r1
	mov	#0, r5
	bsrf	r1
	 mov	r9, r4
.Lmunlock1b:

	tst	r0, r0
	bt	0f
	bra	16f
	 nop
0:
	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(total_seq,r8),r0
	mov.l	@(total_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(total_seq,r8)
	mov.l	r1,@(total_seq+4,r8)
	mov.l	@(cond_futex,r8), r0
	add	r2, r0
	mov.l	r0, @(cond_futex,r8)
	mov	#(1 << nwaiters_shift), r2
	mov.l	@(cond_nwaiters,r8), r0
	add	r2, r0
	mov.l	r0, @(cond_nwaiters,r8)

	/* Get and store current wakeup_seq value.  */
	mov.l	@(wakeup_seq,r8), r10
	mov.l	@(wakeup_seq+4,r8), r11
	mov.l	@(broadcast_seq,r8), r0
	mov.l	r0, @(4,r15)

8:
	/* Get current time.  */
#ifdef __NR_clock_gettime
	/* Get the clock number.	 */
	mov.l	@(cond_nwaiters,r8), r4
	mov	#((1 << nwaiters_shift) - 1), r0
	and	r0, r4
	/* Only clocks 0 and 1 are allowed.  Both are handled in the
	   kernel.  */
	mov	r15, r5
	add	#16, r5
	mov.w	.L__NR_clock_gettime, r3
	trapa	#0x12
	SYSCALL_INST_PAD
# ifndef __ASSUME_POSIX_TIMERS
	cmp/eq	#-ENOSYS, r0
	bt	19f
# endif

	/* Compute relative timeout.  */
	mov.l	@r13, r2
	mov.l	@(4,r13), r3
	mov.l	@(16,r15), r0
	bra	0f
	 mov.l	@(20,r15), r1
.L__NR_clock_gettime:
	.word	__NR_clock_gettime

# ifndef __ASSUME_POSIX_TIMERS
19:
	mov	r15, r4
	add	#16, r4
	mov	#0, r5
	mov	#__NR_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(20,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Micro seconds to nano seconds.  */
	mov.l	@r13, r2
	mov.l	@(4,r13), r3
	mov.l	@(16,r15), r0
	sts	macl, r1
#endif
0:
#else
	mov	r15, r4
	add	#16, r4
	mov	#0, r5
	mov	#__NR_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(20,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Micro seconds to nano seconds.  */
	mov.l	@r13, r2
	mov.l	@(4,r13), r3
	mov.l	@(16,r15), r0
	sts	macl, r1
#endif
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	12f
	mov.l	.L1g, r1
	add	r1, r3
	add	#-1, r2
12:
	mov	#-ETIMEDOUT, r1
	mov.l	r1, @(12,r15)
	cmp/pz	r2
	bf	6f		/* Time is already up.  */

	/* Store relative timeout.  */
	mov.l	r2, @(16,r15)
	mov.l	r3, @(20,r15)
	mov.l	@(cond_futex,r8), r1
	mov.l	r1, @(8,r15)

	/* Unlock.  */
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bt	4f
	bra	3f
	 nop
4:
.LcleanupSTART:
	mov.l	.Lenable1, r1
	bsrf	r1
	 nop
.Lenable1b:
	mov.l	r0, @r15

	mov	r15, r7
	add	#16, r7
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bt/s	99f
	 mov	#FUTEX_WAIT, r5
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_WAIT|FUTEX_PRIVATE_FLAG), r5
	extu.b	r5, r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
	mov	#FUTEX_WAIT, r0
	or	r0, r5
#endif
99:
	mov.l	@(8,r15), r6
	mov	r8, r4
	add	#cond_futex, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	mov.l	r0, @(12,r15)

	mov.l	.Ldisable1, r1
	bsrf	r1
	 mov.l	@r15, r4
.Ldisable1b:
.LcleanupEND:

	/* Lock.  */
	mov	#0, r3
	mov	#1, r4
#if cond_lock != 0
	CMPXCHG (r3, @(cond_lock,r8), r4, r2)
#else
	CMPXCHG (r3, @r8, r4, r2)
#endif
	bf	5f
6:
	mov.l	@(broadcast_seq,r8), r0
	mov.l	@(4,r15), r1
	cmp/eq	r0, r1
	bf	23f

	mov.l	@(woken_seq,r8), r0
	mov.l	@(woken_seq+4,r8), r1

	mov.l	@(wakeup_seq,r8), r2
	mov.l	@(wakeup_seq+4,r8), r3

	cmp/eq	r3, r11
	bf	7f
	cmp/eq	r2, r10
	bt	15f
7:
	cmp/eq	r1, r3
	bf	9f
	cmp/eq	r0, r2
	bf	9f
15:
	mov.l	@(12,r15),r0
	cmp/eq	#-ETIMEDOUT, r0
	bf	8b

	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(wakeup_seq,r8),r0
	mov.l	@(wakeup_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(wakeup_seq,r8)
	mov.l	r1,@(wakeup_seq+4,r8)
	mov.l	@(cond_futex,r8),r0
	add	r2, r0
	mov.l	r0,@(cond_futex,r8)
	mov	#ETIMEDOUT, r0
	bra	14f
	 mov.l	r0, @(24,r15)

23:
	mov	#0, r0
	bra	24f
	 mov.l	r0, @(24,r15)

9:
	mov	#0, r0
	mov.l	r0, @(24,r15)
14:
	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(woken_seq,r8),r0
	mov.l	@(woken_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(woken_seq,r8)
	mov.l	r1,@(woken_seq+4,r8)

24:
	mov	#(1 << nwaiters_shift), r2
	mov.l	@(cond_nwaiters,r8),r0
	sub	r2, r0
	mov.l	r0,@(cond_nwaiters,r8)

	/* Wake up a thread which wants to destroy the condvar object.  */
	mov.l	@(total_seq,r8),r0
	mov.l	@(total_seq+4,r8),r1
	and	r1, r0
	not	r0, r0
	cmp/eq	#0, r0
	bf/s	25f
	 mov	#((1 << nwaiters_shift) - 1), r1
	not	r1, r1
	mov.l	@(cond_nwaiters,r8),r0
	tst	r1, r0
	bf	25f

	mov	r8, r4
	add	#cond_nwaiters, r4
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bt/s	99f
	 mov	#FUTEX_WAKE, r5
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#(FUTEX_WAKE|FUTEX_PRIVATE_FLAG), r5
	extu.b	r5, r5
#else
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r5
	mov	#FUTEX_WAKE, r0
	or	r0, r5
#endif
99:
	mov	#1, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

25:
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	10f

11:
	mov	r9, r4
	mov.l	.Lmlocki1, r1
	bsrf	r1
	 nop
.Lmlocki1b:

	/* We return the result of the mutex_lock operation if it failed.  */
	tst	r0, r0
	bf	18f
	mov.l	@(24,r15), r0

18:
	add	#64, r15
	lds.l	@r15+, pr
	mov.l	@r15+, r13
	mov.l	@r15+, r12
	mov.l	@r15+, r11
	mov.l	@r15+, r10
	mov.l	@r15+, r9
	rts
	 mov.l	@r15+, r8

#ifndef __ASSUME_PRIVATE_FUTEX
.Lpfoff:
	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE
#endif
.L1k:
	.word	1000
	.align	2
.Lmunlock1:
	.long	__pthread_mutex_unlock_usercnt-.Lmunlock1b
.Lenable1:
	.long	__pthread_enable_asynccancel-.Lenable1b
.Ldisable1:
	.long	__pthread_disable_asynccancel-.Ldisable1b
.Lmlocki1:
	.long	__pthread_mutex_cond_lock-.Lmlocki1b
.L1g:
	.long	1000000000

1:
	/* Initial locking failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r6
	mov	#LLL_SHARED, r6
99:
	extu.b	r6, r6
	mov.l	.Lwait2, r1
	bsrf	r1
	 mov	r2, r4
.Lwait2b:
	bra	2b
	 nop

3:
	/* Unlock in loop requires wakeup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r5
	mov	#LLL_SHARED, r5
99:
	mov.l	.Lmwait2, r1
	bsrf	r1
	 extu.b	r5, r5
.Lmwait2b:
	bra	4b
	 nop

5:
	/* Locking in loop failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r6
	mov	#LLL_SHARED, r6
99:
	extu.b	r6, r6
	mov.l	.Lwait3, r1
	bsrf	r1
	 mov	r2, r4
.Lwait3b:
	bra	6b
	 nop

10:
	/* Unlock after loop requires wakeup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r5
	mov	#LLL_SHARED, r5
99:
	mov.l	.Lmwait3, r1
	bsrf	r1
	 extu.b	r5, r5
.Lmwait3b:
	bra	11b
	 nop

16:
	/* The initial unlocking of the mutex failed.  */
	mov.l	r0, @(24,r15)
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	17f

	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r5
	mov	#LLL_SHARED, r5
99:
	mov.l	.Lmwait4, r1
	bsrf	r1
	 extu.b	r5, r5
.Lmwait4b:
17:
	bra	18b
	 mov.l	@(24,r15), r0

	.align	2
.Lwait2:
	.long	__lll_lock_wait-.Lwait2b
.Lmwait2:
	.long	__lll_unlock_wake-.Lmwait2b
.Lwait3:
	.long	__lll_lock_wait-.Lwait3b
.Lmwait3:
	.long	__lll_unlock_wake-.Lmwait3b
.Lmwait4:
	.long	__lll_unlock_wake-.Lmwait4b
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
weak_alias (__pthread_cond_timedwait, pthread_cond_timedwait)


	.type	__condvar_tw_cleanup, @function
__condvar_tw_cleanup:
	mov	r4, r11

	/* Get internal lock.  */
	mov	#0, r3
	mov	#1, r4
#if cond_lock != 0
	CMPXCHG (r3, @(cond_lock,r8), r4, r2)
#else
	CMPXCHG (r3, @r8, r4, r2)
#endif
	bt	1f
	 nop

	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r6
	mov	#LLL_SHARED, r6
99:
	extu.b	r6, r6
	mov.l	.Lwait5, r1
	bsrf	r1
	 mov	r2, r4
.Lwait5b:

1:
	mov.l	@(broadcast_seq,r8), r0
	mov.l	@(4,r15), r1
	cmp/eq	r0, r1
	bf	3f

	mov	#1, r2
	mov	#0, r3

	/* We increment the wakeup_seq counter only if it is lower than
	   total_seq.  If this is not the case the thread was woken and
	   then canceled.  In this case we ignore the signal.  */
	mov.l	@(total_seq+4,r8), r0
	mov.l	@(wakeup_seq+4,r8), r1
	cmp/hi	r1, r0
	bt/s	6f
	 cmp/hi	r0, r1
	bt	7f
	mov.l	@(total_seq,r8), r0
	mov.l	@(wakeup_seq,r8), r1
	cmp/hs	r0, r1
	bt	7f

6:
	clrt
	mov.l	@(wakeup_seq,r8),r0
	mov.l	@(wakeup_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(wakeup_seq,r8)
	mov.l	r1,@(wakeup_seq+4,r8)
	mov.l	@(cond_futex,r8),r0
	add	r2, r0
	mov.l	r0,@(cond_futex,r8)

7:
	clrt
	mov.l	@(woken_seq,r8),r0
	mov.l	@(woken_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(woken_seq,r8)
	mov.l	r1,@(woken_seq+4,r8)

3:
	mov	#(1 << nwaiters_shift), r2
	mov.l	@(cond_nwaiters,r8),r0
	sub	r2, r0
	mov.l	r0,@(cond_nwaiters,r8)

	/* Wake up a thread which wants to destroy the condvar object.  */
	mov	#0, r10
	mov.l	@(total_seq,r8),r0
	mov.l	@(total_seq+4,r8),r1
	and	r1, r0
	not	r0, r0
	cmp/eq	#0, r0
	bf/s	4f
	 mov	#((1 << nwaiters_shift) - 1), r1
	not	r1, r1
	mov.l	@(cond_nwaiters,r8),r0
	tst	r1, r0
	bf	4f

	mov	r8, r4
	add	#cond_nwaiters, r4
	mov	#FUTEX_WAKE, r5
	mov	#1, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	mov	#1, r10

4:
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bt	2f

	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	@(dep_mutex,r8), r0
	cmp/eq	#-1, r0
	bf/s	99f
	 mov	#LLL_PRIVATE, r5
	mov	#LLL_SHARED, r5
99:
	mov.l	.Lmwait5, r1
	bsrf	r1
	 extu.b	r5, r5
.Lmwait5b:

2:
	/* Wake up all waiters to make sure no signal gets lost.  */
	tst	r10, r10
	bf/s	5f
	 mov	r8, r4
	add	#cond_futex, r4
	mov	#FUTEX_WAKE, r5
	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

5:
        mov.l   .Lmlocki5, r1
        bsrf    r1
         mov     r9, r4
.Lmlocki5b:

.LcallUR:
	mov.l	.Lresume, r1
#ifdef __PIC__
	add	r12, r1
#endif
	jsr	@r1
	 mov	r11, r4
	sleep

	.align	2
.Lwait5:
	.long   __lll_lock_wait-.Lwait5b
.Lmwait5:
        .long   __lll_unlock_wake-.Lmwait5b
.Lmlocki5:
	.long   __pthread_mutex_cond_lock-.Lmlocki5b
.Lresume:
#ifdef __PIC__
	.long	_Unwind_Resume@GOTOFF
#else
	.long	_Unwind_Resume
#endif
.LENDCODE:
	.size	__condvar_tw_cleanup, .-__condvar_tw_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	0xff				! @LPStart format (omit)
	.byte	0xff				! @TType format (omit)
	.byte	0x0b				! call-site format
						! DW_EH_PE_sdata4
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.ualong	.LcleanupSTART-.LSTARTCODE
	.ualong	.LcleanupEND-.LcleanupSTART
	.ualong	__condvar_tw_cleanup-.LSTARTCODE
	.uleb128  0
	.ualong	.LcallUR-.LSTARTCODE
	.ualong	.LENDCODE-.LcallUR
	.ualong	0
	.uleb128  0
.Lcstend:

	.section .eh_frame,"a",@progbits
.LSTARTFRAME:
	.ualong	.LENDCIE-.LSTARTCIE		! Length of the CIE.
.LSTARTCIE:
	.ualong	0				! CIE ID.
	.byte	1				! Version number.
#ifdef SHARED
	.string	"zPLR"				! NUL-terminated augmentation
						! string.
#else
	.string	"zPL"				! NUL-terminated augmentation
						! string.
#endif
	.uleb128 1				! Code alignment factor.
	.sleb128 -4				! Data alignment factor.
	.byte	0x11				! Return address register
						! column.
#ifdef SHARED
	.uleb128 7				! Augmentation value length.
	.byte	0x9b				! Personality: DW_EH_PE_pcrel
						! + DW_EH_PE_sdata4
						! + DW_EH_PE_indirect
	.ualong	DW.ref.__gcc_personality_v0-.
	.byte	0x1b				! LSDA Encoding: DW_EH_PE_pcrel
						! + DW_EH_PE_sdata4.
	.byte	0x1b				! FDE Encoding: DW_EH_PE_pcrel
						! + DW_EH_PE_sdata4.
#else
	.uleb128 6				! Augmentation value length.
	.byte	0x0				! Personality: absolute
	.ualong	__gcc_personality_v0
	.byte	0x0				! LSDA Encoding: absolute
#endif
	.byte 0x0c				! DW_CFA_def_cfa
	.uleb128 0xf
	.uleb128 0
	.align 2
.LENDCIE:

	.ualong	.LENDFDE-.LSTARTFDE		! Length of the FDE.
.LSTARTFDE:
	.ualong	.LSTARTFDE-.LSTARTFRAME		! CIE pointer.
#ifdef SHARED
	.ualong	.LSTARTCODE-.			! PC-relative start address
						! of the code.
#else
	.ualong	.LSTARTCODE			! Start address of the code.
#endif
	.ualong	.LENDCODE-.LSTARTCODE		! Length of the code.
	.uleb128 4				! Augmentation size
#ifdef SHARED
	.ualong	.LexceptSTART-.
#else
	.ualong	.LexceptSTART
#endif
	.byte	0x4
	.ualong	.Lpush_r8-.LSTARTCODE
	.byte	0xe
	.uleb128 4
	.byte	0x88
	.uleb128 1
	.byte	0x4
	.ualong	.Lpush_r9-.Lpush_r8
	.byte	0xe
	.uleb128 8
	.byte	0x89
	.uleb128 2
	.byte	0x4
	.ualong	.Lpush_r10-.Lpush_r9
	.byte	0xe
	.uleb128 12
	.byte	0x8a
	.uleb128 3
	.byte	0x4
	.ualong	.Lpush_r11-.Lpush_r10
	.byte	0xe
	.uleb128 16
	.byte	0x8b
	.uleb128 4
	.byte	0x4
	.ualong	.Lpush_r12-.Lpush_r11
	.byte	0xe
	.uleb128 20
	.byte	0x8c
	.uleb128 5
	.byte	0x4
	.ualong	.Lpush_r13-.Lpush_r12
	.byte	0xe
	.uleb128 24
	.byte	0x8d
	.uleb128 6
	.byte	0x4
	.ualong	.Lpush_pr-.Lpush_r13
	.byte	0xe
	.uleb128 28
	.byte	0x91
	.uleb128 7
	.byte	0x4
	.ualong	.Lalloc-.Lpush_pr
	.byte	0xe
	.uleb128 92
	.align	2
.LENDFDE:

#ifdef SHARED
	.hidden DW.ref.__gcc_personality_v0
	.weak   DW.ref.__gcc_personality_v0
	.section .gnu.linkonce.d.DW.ref.__gcc_personality_v0,"aw",@progbits
	.align 4
	.type   DW.ref.__gcc_personality_v0, @object
	.size   DW.ref.__gcc_personality_v0, 4
DW.ref.__gcc_personality_v0:
	.long   __gcc_personality_v0
#endif
