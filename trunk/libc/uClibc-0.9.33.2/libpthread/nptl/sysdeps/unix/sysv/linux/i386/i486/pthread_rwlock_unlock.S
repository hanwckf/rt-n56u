/* Copyright (C) 2002, 2003, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <lowlevellock.h>
#include <lowlevelrwlock.h>
#include <bits/kernel-features.h>
#include <tls.h>


	.text

	.globl	__pthread_rwlock_unlock
	.type	__pthread_rwlock_unlock,@function
	.protected	__pthread_rwlock_unlock
	.align	16
__pthread_rwlock_unlock:
	cfi_startproc
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	pushl	%edi
	cfi_adjust_cfa_offset(4)
	cfi_offset(%ebx, -8)
	cfi_offset(%edi, -12)

	movl	12(%esp), %edi

	/* Get the lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %edx, (%edi)
#else
	cmpxchgl %edx, MUTEX(%edi)
#endif
	jnz	1f

2:	cmpl	$0, WRITER(%edi)
	jne	5f
	subl	$1, NR_READERS(%edi)
	jnz	6f

5:	movl	$0, WRITER(%edi)

	movl	$1, %edx
	leal	WRITERS_WAKEUP(%edi), %ebx
	cmpl	$0, WRITERS_QUEUED(%edi)
	jne	0f

	/* If also no readers waiting nothing to do.  */
	cmpl	$0, READERS_QUEUED(%edi)
	je	6f

	movl	$0x7fffffff, %edx
	leal	READERS_WAKEUP(%edi), %ebx

0:	addl	$1, (%ebx)
	LOCK
#if MUTEX == 0
	subl	$1, (%edi)
#else
	subl	$1, MUTEX(%edi)
#endif
	jne	7f

8:
#ifdef __ASSUME_PRIVATE_FUTEX
	movzbl	PSHARED(%edi), %ecx
	xorl	$FUTEX_PRIVATE_FLAG|FUTEX_WAKE, %ecx
#else
	movzbl	PSHARED(%edi), %ecx
	orl	$FUTEX_WAKE, %ecx
	xorl	%gs:PRIVATE_FUTEX, %ecx
#endif
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	xorl	%eax, %eax
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edi)
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	ret

	cfi_adjust_cfa_offset(8)
	cfi_offset(%ebx, -8)
	cfi_offset(%edi, -12)
	.align	16
6:	LOCK
#if MUTEX == 0
	subl	$1, (%edi)
#else
	subl	$1, MUTEX(%edi)
#endif
	jne	3f

4:	xorl	%eax, %eax
	popl	%edi
	popl	%ebx
	ret

1:
#if MUTEX == 0
	movl	%edi, %edx
#else
	leal	MUTEX(%edi), %edx
#endif
	movzbl	PSHARED(%edi), %ecx
	call	__lll_lock_wait
	jmp	2b

3:
#if MUTEX == 0
	movl	%edi, %eax
#else
	leal	MUTEX(%edi), %eax
#endif
	movzbl	PSHARED(%edi), %ecx
	call	__lll_unlock_wake
	jmp	4b

7:
#if MUTEX == 0
	movl	%edi, %eax
#else
	leal	MUTEX(%edi), %eax
#endif
	movzbl	PSHARED(%edi), %ecx
	call	__lll_unlock_wake
	jmp	8b
	cfi_endproc
	.size	__pthread_rwlock_unlock,.-__pthread_rwlock_unlock

	.globl	pthread_rwlock_unlock
pthread_rwlock_unlock = __pthread_rwlock_unlock

	.globl	__pthread_rwlock_unlock_internal
__pthread_rwlock_unlock_internal = __pthread_rwlock_unlock
