/* longjmp for ARM.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <features.h>
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>


.global __longjmp
.type __longjmp,%function
.align 4
__longjmp:
	mov	ip, r0		/* save jmp_buf pointer */
	
	movs	r0, r1		/* get the return value in place */
	moveq	r0, #1		/* can't let setjmp() return zero! */

#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
	lfmfd	f4, 4, [ip] !	/* load the floating point regs */
#else
	add		ip, ip, #48		/* skip the FP registers */
#endif	

	ldmia     ip ,  {v1-v6, sl, fp, sp, pc}

.size __longjmp,.-__longjmp
