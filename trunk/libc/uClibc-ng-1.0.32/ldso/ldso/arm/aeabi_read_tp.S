/* Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__

#include <sysdep.h>

/* GCC will emit calls to this routine under -mtp=soft.  Linux has an
   equivalent helper function (which clobbers fewer registers than
   a normal function call) in a high page of memory; tail call to the
   helper.

   This function is exported from libc for use by user code.  libpthread, librt,
   and the dynamic linker get their own private copies, for
   performance (and in the case of ld.so, out of necessity); those are
   all hidden.  */

#ifndef NOT_IN_libc
	.global __aeabi_read_tp
#else
	.hidden __aeabi_read_tp
#endif
#if !defined(__ARM_ARCH_ISA_ARM)
	.thumb_func
	.thumb
	.syntax unified
	ENTRY (__aeabi_read_tp)
	push	{r7}
	mov	r7, #0x0f0000
	orr	r7, r7, #6
	svc	#0
	pop	{r7}
	bx	lr
#else
	.arm
ENTRY (__aeabi_read_tp)
	mov	r0, #0xffff0fff
	sub	pc, r0, #31
#endif
END (__aeabi_read_tp)

#endif /* __UCLIBC_HAS_THREADS_NATIVE__ */

