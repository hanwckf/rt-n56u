/* Copyright (C) 1997-2017 Free Software Foundation, Inc.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.

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

/* __brk is a special syscall under Linux since it never returns an
   error.  Instead, the error condition is indicated by returning the old
   break value (instead of the new, requested one).  */

#include <sysdep.h>
#define _ERRNO_H
#include <bits/errno.h>

#ifdef __PIC__
.section .bss
	.align 8
	.globl __curbrk
__curbrk: .skip 8
	.type __curbrk,@object
	.size __curbrk,8
#else
.common __curbrk, 8, 8
#endif

	.text
ENTRY (__brk)
	save	%sp, -192, %sp
	cfi_def_cfa_register(%fp)
	cfi_window_save
	cfi_register(%o7, %i7)
#ifdef __PIC__
	SETUP_PIC_REG(l7)
#endif

	LOADSYSCALL(brk)
	mov	%i0, %o0

	ta	0x6d

	/* All the ways we can fail... */
	bcs,pn	%xcc, .Lerr1
	 nop
	brz,pt	%i0, .Lok
	 subcc	%i0, %o0, %g0
	bne,pn	%xcc, .Lerr0
	 nop

	/* Update __curbrk and return cleanly.  */
.Lok:
#ifndef __PIC__
	sethi	%hi(__curbrk), %g1
	or	%g1, %lo(__curbrk), %g1
#else
	sethi	%gdop_hix22(__curbrk), %g1
	xor	%g1, %gdop_lox10(__curbrk), %g1
	ldx	[%l7 + %g1], %g1, %gdop(__curbrk)
#endif
	stx	%o0, [%g1]
	mov	%g0, %i0

	/* Don't use "ret" cause the preprocessor will eat it.  */
	jmpl	%i7+8, %g0
	 restore

	/* What a horrible way to die.  */
.Lerr0:	set	ENOMEM, %o0
.Lerr1:
#ifndef _LIBC_REENTRANT
#ifndef __PIC__
	sethi	%hi(errno), %g1
	or	%g1, %lo(errno), %g1
#else
	sethi	%gdop_hix22(errno), %g1
	xor	%g1, %gdop_lox10(errno), %g1
	ldx	[%l7 + %g1], %g1, %gdop(errno)
#endif
	st	%o0, [%g1]
#else
	call	HIDDEN_JUMPTARGET(__errno_location)
	 mov	%o0,%l1
	st	%l1, [%o0]
#endif
	sub	%g0, 1, %i0
	jmpl	%i7+8, %g0
	 restore
END (__brk)

weak_alias (__brk, brk)
libc_hidden_def(brk)
