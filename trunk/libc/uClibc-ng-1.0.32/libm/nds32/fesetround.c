/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Set current rounding direction.
   Copyright (C) 2004-2013 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <fpu_control.h>

int
fesetround (int round)
{
#ifdef __NDS32_ABI_2FP_PLUS__
	fpu_control_t temp;
	if ((round & ~0x3) != 0)
	  /* ROUND is no valid rounding mode.  */
		return 1;

	_FPU_GETCW (temp);
	temp = (temp & ~0x3) | round;
	_FPU_SETCW (temp);
	return 0;
#else
	return (round != FE_TONEAREST);
#endif
}
