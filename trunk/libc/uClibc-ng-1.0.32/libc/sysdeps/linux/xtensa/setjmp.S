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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This implementation relies heavily on the Xtensa register window
   mechanism.  Setjmp flushes all the windows except its own to the
   stack and then copies registers from the save areas on the stack
   into the jmp_buf structure, along with the return address of the call
   to setjmp.  Longjmp invalidates all the windows except its own, and
   then sets things up so that it will return to the right place,
   using a window underflow to automatically restore the registers.

   Note that we also save the area located just below the stack pointer
   of the caller. This save area could get overwritten by alloca
   following the call to setjmp. The alloca moves the stack pointer
   to allocate memory on the stack. This newly allocated memory
   includes(!) the original save area (alloca copies the save area
   before it moves that stack pointer).


   previous caller SP     -> |------------------------------| <-----+
                             | caller-2 registers a0-a3     |       | p
                             |------------------------------|       | o
                             | caller registers a4-a8/a12   |       | i
                             |------------------------------|       | n
                             | caller local stack           |       | t
   caller SP              -> |------------------------------| <-+   | s
                             | caller-1 registers a0-a3     |  -:---+
   callee (setjmp) SP     -> |==============================|   |
                             | caller registers a0-a3       | --+
                             |------------------------------|

   In case of an alloca, registers a0-a3 of the previous caller (caller-1)
   are copied (*), and the original location get likely overwritten.

   previous caller SP     -> |------------------------------| <-----+
                             | caller-2 registers a0-a3     |       | p
                             |------------------------------|       | o
                             | caller registers a4-a8/a12   |       | i
                             |------------------------------|       | n
                             | caller local stack           |       | t
   caller SP before alloca-> |------------------------------|       | s
                             | alloca area (overwrites old  |       |
                             | copy of caller-1 registers)  |       |
   caller SP after alloca -> |------------------------------| <-+   |
                             | caller-1 registers a0-a3 (*) |  -:---+
   callee (setjmp) SP     -> |==============================|   |
                             | caller registers a0-a3       | --+
                             |------------------------------|

   So, when longcall returns to the original caller SP, it also needs
   to restore the save area below the SP.

  */

#include "sysdep.h"
 
/* NOTE: The ENTRY macro must allocate exactly 16 bytes (entry a1, 16) */

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

/* int __sigsetjmp (a2 = jmp_buf env, a3 = int savemask)  */

ENTRY (__sigsetjmp)
1:
#if defined(__XTENSA_WINDOWED_ABI__)
	/* Flush registers.  */
	movi	a4, __window_spill
	callx4	a4

	/* Copy the caller register a0-a3 at (sp - 16) to jmpbuf.  */
	addi	a7, a1, -16
	l32i	a4, a7, 0
	l32i	a5, a7, 4
	s32i	a4, a2, 0
	s32i	a5, a2, 4
	l32i	a4, a7, 8
	l32i	a5, a7, 12
	s32i	a4, a2, 8
	s32i	a5, a2, 12

	/* Copy the caller registers a4-a8/a12 from the overflow area.  */
	/* Note that entry moved the SP by 16B, so SP of caller-1 is at 4(sp) */
	extui	a7, a0, 30, 2
	blti	a7, 2, .Lendsj
	l32i	a8, a1, 4	/* a8: SP of 'caller-1' */
	slli	a4, a7, 4
	sub	a6, a8, a4
	addi	a5, a2, 16
	addi	a8, a8, -16	/* a8: end of register overflow area */
.Lsjloop:
	l32i	a7, a6, 0
	l32i	a4, a6, 4
	s32i	a7, a5, 0
	s32i	a4, a5, 4
	l32i	a7, a6, 8
	l32i	a4, a6, 12
	s32i	a7, a5, 8
	s32i	a4, a5, 12
	addi	a5, a5, 16
	addi	a6, a6, 16
	blt	a6, a8, .Lsjloop
.Lendsj:

	/* Copy previous caller registers (this is assuming 'entry a1,16') */
	l32i	a4, a1, 0
	l32i	a5, a1, 4
	s32i	a4, a2, 48
	s32i	a5, a2, 52
	l32i	a4, a1, 8
	l32i	a5, a1, 12
	s32i	a4, a2, 56
	s32i	a5, a2, 60

	/* Save the return address, including the window size bits.  */
	s32i	a0, a2, 64

	/* a2 still points to jmp_buf.  a3 contains savemask.  */
	mov	a6, a2
	mov	a7, a3
	movi	a3, __sigjmp_save
	callx4	a3
	mov	a2, a6
	retw
#elif defined(__XTENSA_CALL0_ABI__)
	s32i	a0, a2, 0
	s32i	a1, a2, 4
	s32i	a12, a2, 8
	s32i	a13, a2, 12
	s32i	a14, a2, 16
	s32i	a15, a2, 20
	mov	a12, a2
	movi	a0, __sigjmp_save
	callx0	a0
	l32i	a0, a12, 0
	l32i	a12, a12, 8
	ret
#else
#error Unsupported Xtensa ABI
#endif
END(__sigsetjmp)

weak_extern(_setjmp)
weak_extern(setjmp)
