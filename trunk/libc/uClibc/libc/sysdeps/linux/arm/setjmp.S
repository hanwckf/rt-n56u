/* setjmp for ARM.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

.global __sigsetjmp
.type __sigsetjmp,%function
.align 4
__sigsetjmp:
	/* Save registers */
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
	sfmea   f4, 4, [r0]!
#else
	add     r0, r0, #48		/* skip the FP registers */
#endif
	stmia   r0, {v1-v6, sl, fp, sp, lr}

	/* Restore pointer to jmp_buf */
	sub     r0, r0, #48

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef __PIC__
	B	__sigjmp_save(PLT)
#else
	B	__sigjmp_save
#endif

.size __sigsetjmp,.-__sigsetjmp
