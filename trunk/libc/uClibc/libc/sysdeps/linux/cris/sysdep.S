/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <features.h>
#include <sysdep.h>

/* Make space for the errno variable.  */

	.globl	C_SYMBOL_NAME(errno)
	.type	C_SYMBOL_NAME(errno),@object
	.lcomm	C_SYMBOL_NAME(errno),4

	errno = _errno

	/* weak_alias (errno, _errno) */


/* The syscall stubs jump here when they detect an error, bot for PIC and
   non-PIC.  */

	.syntax	no_register_prefix

ENTRY (__syscall_error)
	neg.d	r10,r10

#ifdef	__UCLIBC_HAS_THREADS__
	push	r10
	push	srp

	/* Note that __syscall_error is only visible within this library,
	   and no-one passes it on as a pointer, so can assume that R0 (GOT
	   pointer) is correctly set up.  */
	PLTCALL	(__errno_location)

	pop	srp
	pop	r11
	move.d	r11,[r10]

#else /* not __UCLIBC_HAS_THREADS__ */
# ifdef __PIC__
	move.d	[r0+C_SYMBOL_NAME(errno:GOT)],r9
	move.d  r10,[r9]
# else
	move.d	r10,[C_SYMBOL_NAME(errno)]
# endif
#endif /* __UCLIBC_HAS_THREADS__ */

#ifdef __PIC__
/* PIC callers are supposed to have R0 on stack, ready for us to restore.
   Callers are only allowed from within this DSO, so the GOT in r0 is the
   one we want to use.

   (Don't use "ret" - it's a macro).   */

	moveq	-1,r10
	Ret
	pop	r0
#else
	Ret
	moveq	-1,r10
#endif

END (__syscall_error)
