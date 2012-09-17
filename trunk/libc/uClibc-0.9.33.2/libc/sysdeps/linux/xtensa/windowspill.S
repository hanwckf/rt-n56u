/* Function to force register windows to the stack.
   Copyright (C) 2005, 2007 Free Software Foundation, Inc.
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

#include <bits/xtensa-config.h>

	.text
	.align  4
	.literal_position
	.global __window_spill
	.type   __window_spill, @function
__window_spill:
	entry	a1, 48
        bbci.l  a0, 31, .L4		/* branch if called with call4 */
        bbsi.l  a0, 30, .L12		/* branch if called with call12 */

	/* Called with call8: touch register NUM_REGS-12 (4/20/52) */
.L8:
#if XCHAL_NUM_AREGS > 16
	call12	1f
	retw

	.align	4
1:	_entry	a1, 48			/* touch NUM_REGS-24 (x/8/40) */

#if XCHAL_NUM_AREGS == 32
	mov	a8, a0
	retw
#else
	mov	a12, a0
	_entry	a1, 48			/* touch NUM_REGS-36 (x/x/28) */
	mov	a12, a0
	_entry	a1, 48			/* touch NUM_REGS-48 (x/x/16) */
	mov	a12, a0
	_entry	a1, 16			/* touch NUM_REGS-60 (x/x/4) */
#endif
#endif
	mov	a4, a0
	retw

	/* Called with call4: touch register NUM_REGS-8 (8/24/56) */
.L4:
#if XCHAL_NUM_AREGS == 16
	mov	a8, a0
#else
	call12	1f
	retw

	.align	4
1:	_entry	a1, 48			/* touch NUM_REGS-20 (x/12/44) */
	mov	a12, a0
#if XCHAL_NUM_AREGS > 32
	_entry	a1, 48			/* touch NUM_REGS-32 (x/x/32) */
	mov	a12, a0
	_entry	a1, 48			/* touch NUM_REGS-44 (x/x/20) */
	mov	a12, a0
	_entry	a1, 48			/* touch NUM_REGS-56 (x/x/8) */
	mov	a8, a0
#endif
#endif
	retw

	/* Called with call12: touch register NUM_REGS-16 (x/16/48) */
.L12:
#if XCHAL_NUM_AREGS > 16
	call12	1f
	retw

	.align	4
1:	_entry	a1, 48			/* touch NUM_REGS-28 (x/4/36) */
#if XCHAL_NUM_AREGS == 32
	mov	a4, a0
#else
	mov	a12, a0
	_entry	a1, 48			/* touch NUM_REGS-40 (x/x/24) */
	mov	a12, a0
	_entry	a1, 48			/* touch NUM_REGS-52 (x/x/12) */
	mov	a12, a0
#endif
#endif
	retw
