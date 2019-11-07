/* Copyright (C) 1997-2017 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu).

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

#include <sysdep.h>
#include <jmpbuf-offsets.h>

ENTRY(__longjmp)

	/* Modify the context with the value we want to return.  */
	movre	%o1, 1, %o1
	stx	%o1, [%o0 + O_g1]

	/* Let setcontext know if we want to modify the current sigmask. */
	ld	[%o0 + O_mask_was_saved], %o1

	/* And bamf back to where we belong!  */
	ta	0x6f

END(__longjmp)

libc_hidden_def (__longjmp)
