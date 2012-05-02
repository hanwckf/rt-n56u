/* Copyright (C) 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe <brendan@zen.org>, 1993.

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

/* __brk is a special syscall under Linux since it never returns an
   error.  Instead, the error condition is indicated by returning the old
   break value (instead of the new, requested one).  */

#include <features.h>
#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

#ifdef __PIC__
.section .bss
	.align 3
	.globl __curbrk
__curbrk: .skip 8
	.type __curbrk,@object
	.size __curbrk,8
#else
.comm __curbrk, 8
#endif

	.text
.globl  __brk;
.align 3;
.ent  __brk , 0;

__brk:
    .frame $30 ,   8 , $26
	ldgp	$29, 0($27)
	subq	$30, 8, $30
	.prologue 1

	/* Save the requested brk across the system call.  */
	stq	$16, 0($30)

	ldiq	$0, __NR_brk
	call_pal 131

	ldq	$16, 0($30)

	/* Be prepared for an OSF-style brk.  */
	bne	$19, $err1
	beq	$0, $ok

	/* Correctly handle the brk(0) query case.  */
	cmoveq	$16, $0, $16
	xor	$16, $0, $1
	bne	$1, $err0

	/* Update __curbrk and return cleanly.  */
	mov	$31, $0
$ok:	stq	$16, __curbrk
	addq	$30, 8, $30
	ret

	/* What a horrible way to die.  */
$err0:	ldi	$0, ENOMEM
$err1:	addq	$30, 8, $30
	jmp	$31, __syscall_error

	.end __brk

.weak    brk;
    brk    =   __brk
