/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  i386 version.
   Copyright (C) 1994,1995,1996,1997,2000,2001 Free Software Foundation, Inc.
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

/* This just does a tail-call to `__sigsetjmp (ARG, 0)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

.global _setjmp
.type   _setjmp,%function
_setjmp:
	xorl %eax, %eax
	movl 4   (%esp), %edx

	/* Save registers.  */
	movl %ebx, (JB_BX*4)(%edx)
	movl %esi, (JB_SI*4)(%edx)
	movl %edi, (JB_DI*4)(%edx)
	leal 4 (%esp), %ecx	/* Save SP as it will be after we return.  */
	movl %ecx, (JB_SP*4)(%edx)
	movl 0 (%esp), %ecx	/* Save PC we are returning to now.  */
	movl %ecx, (JB_PC*4)(%edx)
	movl %ebp, (JB_BP*4)(%edx) /* Save caller's frame pointer.  */

	movl %eax, JB_SIZE(%edx) /* No signal mask set.  */
	ret
.size _setjmp,.-_setjmp
