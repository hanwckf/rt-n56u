/* longjmp for Xtensa Processors.

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


ENTRY (__longjmp)

	/* Invalidate all but the current window.  Reading and writing
	   special registers WINDOWBASE and WINDOWSTART are
	   privileged operations, so user processes must call the
	   slower __window_spill() to do the job.  */

	movi	a4, __window_spill
	callx4	a4

	/* Return to the return address of the setjmp, using the
	   window size bits from the setjmp call so that the caller
	   will be able to find the return value that we put in a2.  */

	l32i	a0, a2, 64

	/* Copy the first 4 saved registers from jmp_buf into the save area
	   at the current sp so that the values will be restored to registers
	   when longjmp returns. */

	addi	a7, a1, -16
	l32i	a4, a2, 0
	l32i	a5, a2, 4
	s32i	a4, a7, 0
	s32i	a5, a7, 4
	l32i	a4, a2, 8
	l32i	a5, a2, 12
	s32i	a4, a7, 8
	s32i	a5, a7, 12

	/* Copy the remaining 0-8 saved registers.  */
	extui	a7, a0, 30, 2
	blti	a7, 2, .Lendlj
	l32i	a8, a2, 52
	slli	a4, a7, 4
	sub	a6, a8, a4
	addi	a5, a2, 16
	addi	a8, a8, -16		/* a8 = end of register overflow area */
.Lljloop:
	l32i	a7, a5, 0
	l32i	a4, a5, 4
	s32i	a7, a6, 0
	s32i	a4, a6, 4
	l32i	a7, a5, 8
	l32i	a4, a5, 12
	s32i	a7, a6, 8
	s32i	a4, a6, 12
	addi	a5, a5, 16
	addi	a6, a6, 16
	blt	a6, a8, .Lljloop
.Lendlj:

	/* The 4 words saved from the register save area at the target's
	   sp are copied back to the target procedure's save area.  The
	   only point of this is to prevent a catastrophic failure in
	   case the contents were moved by an alloca after calling
	   setjmp.  This is a bit paranoid but it doesn't cost much.  */

	l32i	a7, a2, 4		/* load the target stack pointer */
	addi	a7, a7, -16		/* find the destination save area */
	l32i	a4, a2, 48
	l32i	a5, a2, 52
	s32i	a4, a7, 0
	s32i	a5, a7, 4
	l32i	a4, a2, 56
	l32i	a5, a2, 60
	s32i	a4, a7, 8
	s32i	a5, a7, 12

	/* Return v ? v : 1.  */
	movi	a2, 1
	movnez	a2, a3, a3

	retw
END (__longjmp)

libc_hidden_def (__longjmp)
