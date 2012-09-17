/* Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
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
#include <tcb-offsets.h>
#include <structsem.h>
#include <lowlevellock.h>
#include "lowlevel-atomic.h"


#if VALUE != 0
# error "code needs to be rewritten for VALUE != 0"
#endif

	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	5
sem_timedwait:
.LSTARTCODE:
	mov.l	@r4, r0
2:
	tst	r0, r0
	bt	1f
	mov	r0, r3
	mov	r0, r6
	add	#-1, r3
	CMPXCHG (r6, @r4, r3, r2)
	bf/s	2b
	 mov	r2, r0
	rts
	 mov	#0, r0

1:
	/* Check whether the timeout value is valid.  */
	mov.l	r8, @-r15
.Lpush_r8:
	mov.l	r9, @-r15
.Lpush_r9:
	mov.l	r10, @-r15
.Lpush_r10:
	mov.l	r12, @-r15
.Lpush_r12:
	sts.l	pr, @-r15
.Lpush_pr:
	add	#-8, r15
.Lalloc:
	mov	r4, r8
	mov	r5, r9

	/* Check for invalid nanosecond field.  */
	mov.l	@(4,r9), r0
	mov.l	.L1g, r1
	cmp/hs	r1, r0
	bt/s	6f
	 mov	#EINVAL, r0
	INC (@(NWAITERS,r8),r2)

7:
	/* Compute relative timeout.  */
	mov	r15, r4
	mov	#0, r5
	mov	#__NR_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	mov.l	@(4,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Milli seconds to nano seconds.  */
	mov.l	@r9, r2
	mov.l	@(4,r9), r3
	mov.l	@r15, r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	5f
	mov.l	.L1g, r1
	add	r1, r3
	add	#-1, r2
5:
	cmp/pz	r2
	bf/s	6f		/* Time is already up.  */
	 mov	#ETIMEDOUT, r0

	/* Store relative timeout.  */
	mov.l	r2, @r15
	mov.l	r3, @(4,r15)

.LcleanupSTART:
	mov.l	.Lenable0, r1
	bsrf	r1
	 nop
.Lenable0b:
	mov	r0, r10

	mov	r8, r4
#if FUTEX_WAIT == 0
	mov.l	@(PRIVATE,r8), r5
#else
	mov.l	@(PRIVATE,r8), r5
	mov	#FUTEX_WAIT, r0
	or	r0, r5
#endif
	mov	#0, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	.Ldisable0, r1
	mov	r10, r4
	bsrf	r1
	 mov	r0, r10
.Ldisable0b:
	mov	r10, r0
.LcleanupEND:

	tst	r0, r0
	bt	9f
	cmp/eq	#-EWOULDBLOCK, r0
	bf	3f
9:
	mov.l	@r8, r0
8:
	tst	r0, r0
	bt	7b

	mov	r0, r3
	mov	r0, r4
	add	#-1, r3
	CMPXCHG (r4, @r8, r3, r2)
	bf/s	8b
	 mov	r2, r0

	DEC (@(NWAITERS,r8), r2)
	mov	#0, r0

10:
	add	#8, r15
	lds.l	@r15+, pr
	mov.l	@r15+, r12
	mov.l	@r15+, r10
	mov.l	@r15+, r9
	mov.l	@r15+, r8
	rts
	 nop

3:
	neg	r0, r0
6:
	mov	r0, r10
	mova	.Lgot2, r0
	mov.l	.Lgot2, r12
	add	r0, r12

#if USE___THREAD
	mov.l	.Lerrno2, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	bra	.Lexit
	 add	r1, r0
	.align	2
.Lerrno2:
 	.long	errno@GOTTPOFF
.Lexit:
#else
	mov.l	.Lerrloc2, r1
	bsrf	r1
	 nop
.Lerrloc2b:
#endif
	mov.l	r10, @r0
	DEC (@(NWAITERS,r8), r2)
	bra	10b
	 mov	#-1, r0

.L1k:
	.word	1000
	.align	2
.L1g:
	.long	1000000000
.Lgot2:
	.long	_GLOBAL_OFFSET_TABLE_
#if !USE___THREAD
.Lerrloc2:
	.long	__errno_location@PLT-(.Lerrloc2b-.)
#endif
.Lenable0:
	.long	__pthread_enable_asynccancel-.Lenable0b
.Ldisable0:
	.long	__pthread_disable_asynccancel-.Ldisable0b
	.size	sem_timedwait,.-sem_timedwait

	.type	sem_wait_cleanup,@function
sem_wait_cleanup:
 	DEC (@(NWAITERS,r8), r2)
.LcallUR:
	mov.l	.Lresume, r1
#ifdef __PIC__
	add	r12, r1
#endif
	jsr	@r1
	 nop
	sleep

	.align	2
.Lresume:
#ifdef __PIC__
	.long	_Unwind_Resume@GOTOFF
#else
	.long	_Unwind_Resume
#endif
.LENDCODE:
	.size	sem_wait_cleanup,.-sem_wait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	0xff				! @LPStart format (omit)
	.byte	0xff				! @TType format (omit)
	.byte	0x01				! call-site format
						! DW_EH_PE_uleb128
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 sem_wait_cleanup-.LSTARTCODE
	.uleb128  0
	.uleb128 .LcallUR-.LSTARTCODE
	.uleb128 .LENDCODE-.LcallUR
	.uleb128 0
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
	.align 4
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

	.byte	4				! DW_CFA_advance_loc4
	.ualong	.Lpush_r8-.LSTARTCODE
	.byte	14				! DW_CFA_def_cfa_offset
	.uleb128 4
	.byte   0x88				! DW_CFA_offset r8
        .uleb128 1
	.byte	4				! DW_CFA_advance_loc4
	.ualong	.Lpush_r9-.Lpush_r8
	.byte	14				! DW_CFA_def_cfa_offset
	.uleb128 8
	.byte   0x89				! DW_CFA_offset r9
        .uleb128 2
	.byte	4				! DW_CFA_advance_loc4
	.ualong	.Lpush_r10-.Lpush_r9
	.byte	14				! DW_CFA_def_cfa_offset
	.uleb128 12
	.byte   0x8a				! DW_CFA_offset r10
        .uleb128 3
	.byte	4				! DW_CFA_advance_loc4
	.ualong	.Lpush_r12-.Lpush_r10
	.byte	14				! DW_CFA_def_cfa_offset
	.uleb128 16
	.byte   0x8c				! DW_CFA_offset r12
        .uleb128 4
	.byte	4				! DW_CFA_advance_loc4
	.ualong	.Lpush_pr-.Lpush_r12
	.byte	14				! DW_CFA_def_cfa_offset
	.uleb128 20
	.byte	0x91				! DW_CFA_offset pr
	.uleb128 5
	.byte	4				! DW_CFA_advance_loc4
	.ualong	.Lalloc-.Lpush_pr
	.byte	14				! DW_CFA_def_cfa_offset
	.uleb128 28
	.align	4
.LENDFDE:


#ifdef SHARED
	.hidden	DW.ref.__gcc_personality_v0
	.weak	DW.ref.__gcc_personality_v0
	.section .gnu.linkonce.d.DW.ref.__gcc_personality_v0,"aw",@progbits
	.align	4
	.type	DW.ref.__gcc_personality_v0, @object
	.size	DW.ref.__gcc_personality_v0, 4
DW.ref.__gcc_personality_v0:
	.long	__gcc_personality_v0
#endif
