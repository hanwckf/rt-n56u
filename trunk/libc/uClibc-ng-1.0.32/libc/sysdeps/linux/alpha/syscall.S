/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@azstarnet.com>, 1996.

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

#include <features.h>
#include <sys/regdef.h>
#include <asm/pal.h>

/*
 * This is for COMPATIBILITY with Linux/x86 only.  Linux/Alpha system
 * calls return an error indication in a3.  This allows arbitrary 64bit 
 * values to be returned in v0 (because negative values are not
 * mistaken as error numbers).  However, C allows only one value to
 * be returned, so the interface below folds the error indication passed in
 * a3 back into v0: it sets v0 to -errno if an error occurs.  Thus,
 * no negative 64bit numbers can be returned.  To avoid this problem,
 * use assembly stubs wherever possible/convenient.
 *
 * Usage:
 *
 * long	syscall(syscall_number, arg1, arg2, arg3, arg4, arg5)
 *
 * syscall_number = the index of the system call we're invoking
 * arg1-arg5 = up to 5 integer arguments to the system call
 *
 * We need to do some arg shifting: the kernel expects the
 * syscall number in v0 and the first five args in a0-a4.
 *
 */

.globl __syscall;
.align 4;
.ent __syscall, 0;
__syscall:
.frame sp, 0, ra
	.prologue 0

	mov	a0, v0		/* Syscall number -> v0 */
	mov	a1, a0		/* arg1-arg5 -> a0-a4 */
	mov	a2, a1
	mov	a3, a2
	mov	a4, a3
	mov	a5, a4

	call_pal PAL_callsys	/* Invoke system call */
	bne	a3, $error
	ret

$error:
	br	gp, 2f
2:	ldgp	gp, 0(gp)
	jmp	zero,__syscall_error

.end __syscall

weak_alias (__syscall, syscall)
