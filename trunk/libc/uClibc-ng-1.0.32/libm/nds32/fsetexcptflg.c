/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Set floating-point environment exception handling.
   Copyright (C) 1997-2013 Free Software Foundation, Inc.

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
fesetexceptflag (const fexcept_t *flagp, int excepts)
{
#ifdef __NDS32_ABI_2FP_PLUS__
	fexcept_t temp;

	/* Get the current environment.  */
	_FPU_GETCW (temp);

	/* Set the desired exception mask.  */
	temp &= ~(excepts & FE_ALL_EXCEPT);
	temp |= (*flagp & excepts & FE_ALL_EXCEPT);

	/* Save state back to the FPU.  */
	_FPU_SETCW (temp);

	/* Success.  */
	return 0;
#else
	/* Unsupported, so fail unless nothing needs to be done.  */
	return (excepts != 0);
#endif
}
