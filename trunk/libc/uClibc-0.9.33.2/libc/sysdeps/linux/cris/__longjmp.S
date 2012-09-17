/* longjmp for CRIS.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
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
#include "sysdep.h"
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

	.syntax no_register_prefix

/* Saving and restoring CCR is meaningless, so we don't do it.  */
ENTRY (__longjmp)
	/* Note that r10 = jmp_buf, r11 = retval.  */
#ifdef __arch_v32

	/* We don't restore the call-clobbered registers for v32;
	   their space (corresponding to v10) is now defined as
	   reserved.  */
	movem [r10],r9
	addq 14*4,r10
	cmpq 0,r11
	beq 0f
	move.d [r10+],sp

	move.d r11,r9
0:
	move.d [r10+],acr
	jump acr
	move [r10],srp

#else

	move [r10+16*4],srp
	test.d r11
	beq 0f                  /* Already a 1 in place. */
	nop
	/* Offset for r9, the return value (see setjmp).  */
	move.d r11,[r10+6*4]
0:
	movem [r10],pc

#endif
END (__longjmp)

libc_hidden_def(__longjmp)
