/* Copyright (C) 2002, 2003, 2004, 2005, 2007, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <sysdep.h>
#include <pthread-errnos.h>
#include <structsem.h>
#include <lowlevellock.h>


#if VALUE != 0
# error "code needs to be rewritten for VALUE != 0"
#endif


	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	16
sem_timedwait:
.LSTARTCODE:
	movl	4(%esp), %ecx

	movl	(%ecx), %eax
2:	testl	%eax, %eax
	je	1f

	leal	-1(%eax), %edx
	LOCK
	cmpxchgl %edx, (%ecx)
	jne	2b

	xorl	%eax, %eax
	ret

	/* Check whether the timeout value is valid.  */
1:	pushl	%esi
.Lpush_esi:
	pushl	%edi
.Lpush_edi:
	pushl	%ebx
.Lpush_ebx:
	subl	$12, %esp
.Lsub_esp:

	movl	32(%esp), %edi

	/* Check for invalid nanosecond field.  */
	cmpl	$1000000000, 4(%edi)
	movl	$EINVAL, %esi
	jae	6f

	LOCK
	incl	NWAITERS(%ecx)

7:	xorl	%ecx, %ecx
	movl	%esp, %ebx
	movl	%ecx, %edx
	movl	$__NR_gettimeofday, %eax
	ENTER_KERNEL

	/* Compute relative timeout.  */
	movl	4(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%edi), %ecx
	movl	4(%edi), %edx
	subl	(%esp), %ecx
	subl	%eax, %edx
	jns	5f
	addl	$1000000000, %edx
	subl	$1, %ecx
5:	testl	%ecx, %ecx
	movl	$ETIMEDOUT, %esi
	js	6f		/* Time is already up.  */

	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)

.LcleanupSTART:
	call	__pthread_enable_asynccancel
	movl	%eax, 8(%esp)

	movl	28(%esp), %ebx	/* Load semaphore address.  */
#if FUTEX_WAIT == 0
	movl	PRIVATE(%ebx), %ecx
#else
	movl	$FUTEX_WAIT, %ecx
	orl	PRIVATE(%ebx), %ecx
#endif
	movl	%esp, %esi
	xorl	%edx, %edx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	movl	%eax, %esi

	movl	8(%esp), %eax
	call	__pthread_disable_asynccancel
.LcleanupEND:

	testl	%esi, %esi
	je	9f
	cmpl	$-EWOULDBLOCK, %esi
	jne	3f

9:	movl	(%ebx), %eax
8:	testl	%eax, %eax
	je	7b

	leal	-1(%eax), %ecx
	LOCK
	cmpxchgl %ecx, (%ebx)
	jne	8b

	xorl	%eax, %eax

	LOCK
	decl	NWAITERS(%ebx)

10:	addl	$12, %esp
.Ladd_esp:
	popl	%ebx
.Lpop_ebx:
	popl	%edi
.Lpop_edi:
	popl	%esi
.Lpop_esi:
	ret

.Lafter_ret:
3:	negl	%esi
6:
#ifdef __PIC__
	call	__x86.get_pc_thunk.bx
#else
	movl	$4f, %ebx
4:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#if USE___THREAD
# ifdef NO_TLS_DIRECT_SEG_REFS
	movl	errno@gotntpoff(%ebx), %edx
	addl	%gs:0, %edx
	movl	%esi, (%edx)
# else
	movl	errno@gotntpoff(%ebx), %edx
	movl	%esi, %gs:(%edx)
# endif
#else
	call	__errno_location@plt
	movl	%esi, (%eax)
#endif

	movl	28(%esp), %ebx	/* Load semaphore address.  */
	orl	$-1, %eax
	jmp	10b
	.size	sem_timedwait,.-sem_timedwait


	.type	sem_wait_cleanup,@function
sem_wait_cleanup:
	LOCK
	decl	NWAITERS(%ebx)
	movl	%eax, (%esp)
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	.size	sem_wait_cleanup,.-sem_wait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	0xff				# @LPStart format (omit)
	.byte	0xff				# @TType format (omit)
	.byte	0x01				# call-site format
						# DW_EH_PE_uleb128
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
	.long	.LENDCIE-.LSTARTCIE		# Length of the CIE.
.LSTARTCIE:
	.long	0				# CIE ID.
	.byte	1				# Version number.
#ifdef SHARED
	.string	"zPLR"				# NUL-terminated augmentation
						# string.
#else
	.string	"zPL"				# NUL-terminated augmentation
						# string.
#endif
	.uleb128 1				# Code alignment factor.
	.sleb128 -4				# Data alignment factor.
	.byte	8				# Return address register
						# column.
#ifdef SHARED
	.uleb128 7				# Augmentation value length.
	.byte	0x9b				# Personality: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4
						# + DW_EH_PE_indirect
	.long	DW.ref.__gcc_personality_v0-.
	.byte	0x1b				# LSDA Encoding: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4.
	.byte	0x1b				# FDE Encoding: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4.
#else
	.uleb128 6				# Augmentation value length.
	.byte	0x0				# Personality: absolute
	.long	__gcc_personality_v0
	.byte	0x0				# LSDA Encoding: absolute
#endif
	.byte 0x0c				# DW_CFA_def_cfa
	.uleb128 4
	.uleb128 4
	.byte	0x88				# DW_CFA_offset, column 0x10
	.uleb128 1
	.align 4
.LENDCIE:

	.long	.LENDFDE-.LSTARTFDE		# Length of the FDE.
.LSTARTFDE:
	.long	.LSTARTFDE-.LSTARTFRAME		# CIE pointer.
#ifdef SHARED
	.long	.LSTARTCODE-.			# PC-relative start address
						# of the code.
#else
	.long	.LSTARTCODE			# Start address of the code.
#endif
	.long	.LENDCODE-.LSTARTCODE		# Length of the code.
	.uleb128 4				# Augmentation size
#ifdef SHARED
	.long	.LexceptSTART-.
#else
	.long	.LexceptSTART
#endif

	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_esi-.LSTARTCODE
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte   0x86				# DW_CFA_offset %esi
	.uleb128 2
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_edi-.Lpush_esi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 12
	.byte   0x87				# DW_CFA_offset %edi
	.uleb128 3
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_ebx-.Lpush_edi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte   0x83				# DW_CFA_offset %ebx
	.uleb128 4
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lsub_esp-.Lpush_ebx
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 28
	.byte	4				# DW_CFA_advance_loc4
	.long	.Ladd_esp-.Lsub_esp
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_ebx-.Ladd_esp
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 12
	.byte	0xc3				# DW_CFA_restore %ebx
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_edi-.Lpop_ebx
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	0xc7				# DW_CFA_restore %edi
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_esi-.Lpop_edi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 4
	.byte	0xc6				# DW_CFA_restore %esi
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lafter_ret-.Lpop_esi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 28
	.byte   0x86				# DW_CFA_offset %esi
	.uleb128 2
	.byte   0x87				# DW_CFA_offset %edi
	.uleb128 3
	.byte   0x83				# DW_CFA_offset %ebx
	.uleb128 4
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
