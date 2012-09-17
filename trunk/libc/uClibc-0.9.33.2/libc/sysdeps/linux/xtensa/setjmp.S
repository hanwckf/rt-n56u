/* setjmp for Xtensa Processors.
   Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* This implementation relies heavily on the Xtensa register window
   mechanism.  Setjmp flushes all the windows except its own to the
   stack and then copies registers from the save areas on the stack
   into the jmp_buf structure, along with the return address of the call
   to setjmp.  Longjmp invalidates all the windows except its own, and
   then sets things up so that it will return to the right place,
   using a window underflow to automatically restore the registers.

   Note that it would probably be sufficient to only copy the
   registers from setjmp's caller into jmp_buf.  However, we also copy
   the save area located at the stack pointer of setjmp's caller.
   This save area will typically remain intact until the longjmp call.
   The one exception is when there is an intervening alloca in
   setjmp's caller.  This is certainly an unusual situation and is
   likely to cause problems in any case (the storage allocated on the
   stack cannot be safely accessed following the longjmp).  As bad as
   it is, on most systems this situation would not necessarily lead to
   a catastrophic failure.  If we did not preserve the extra save area
   on Xtensa, however, it would.  When setjmp's caller returns after a
   longjmp, there will be a window underflow; an invalid return
   address or stack pointer in the save area will almost certainly
   lead to a crash.  Keeping a copy of the extra save area in the
   jmp_buf avoids this with only a small additional cost.  If setjmp
   and longjmp are ever time-critical, this could be removed.  */

#include "sysdep.h"

/* int setjmp (a2 = jmp_buf env) */

ENTRY (_setjmp)
	movi	a3, 0
	j	1f
END (_setjmp)
libc_hidden_def (_setjmp)

ENTRY (setjmp)
	movi	a3, 1
	j	1f
END (setjmp)

/* int __sigsetjmp (a2 = jmp_buf env,
		    a3 = int savemask)  */

ENTRY (__sigsetjmp)
1:
	/* Flush registers.  */
	movi	a4, __window_spill
	callx4	a4

	/* Preserve the second argument (savemask) in a15.  The selection
	   of a15 is arbitrary, except it's otherwise unused.  There is no
	   risk of triggering a window overflow since we just returned
	   from __window_spill().  */
	mov	a15, a3

	/* Copy the register save area at (sp - 16).  */
	addi	a5, a1, -16
	l32i	a3, a5, 0
	l32i	a4, a5, 4
	s32i	a3, a2, 0
	s32i	a4, a2, 4
	l32i	a3, a5, 8
	l32i	a4, a5, 12
	s32i	a3, a2, 8
	s32i	a4, a2, 12

	/* Copy 0-8 words from the register overflow area.  */
	extui	a3, a0, 30, 2
	blti	a3, 2, .Lendsj
	l32i	a7, a1, 4
	slli	a4, a3, 4
	sub	a5, a7, a4
	addi	a6, a2, 16
	addi	a7, a7, -16		/* a7 = end of register overflow area */
.Lsjloop:
	l32i	a3, a5, 0
	l32i	a4, a5, 4
	s32i	a3, a6, 0
	s32i	a4, a6, 4
	l32i	a3, a5, 8
	l32i	a4, a5, 12
	s32i	a3, a6, 8
	s32i	a4, a6, 12
	addi	a5, a5, 16
	addi	a6, a6, 16
	blt	a5, a7, .Lsjloop
.Lendsj:

	/* Copy the register save area at sp.  */
	l32i	a3, a1, 0
	l32i	a4, a1, 4
	s32i	a3, a2, 48
	s32i	a4, a2, 52
	l32i	a3, a1, 8
	l32i	a4, a1, 12
	s32i	a3, a2, 56
	s32i	a4, a2, 60

	/* Save the return address, including the window size bits.  */
	s32i	a0, a2, 64

	/* a2 still addresses jmp_buf.  a15 contains savemask.  */
	mov	a6, a2
	mov	a7, a15
	movi	a3, __sigjmp_save
	callx4	a3
	mov	a2, a6
	retw
END(__sigsetjmp)

weak_extern(_setjmp)
weak_extern(setjmp)
