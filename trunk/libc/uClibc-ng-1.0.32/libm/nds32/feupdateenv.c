/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Install given floating-point environment and raise exceptions.
   Copyright (C) 1997-2013 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
feupdateenv (const fenv_t *envp)
{
#ifdef __NDS32_ABI_2FP_PLUS__
	unsigned int temp;

	/* Get the current exception state.  */
	_FPU_GETCW (temp);

	/* Install new environment.  */
	fesetenv (envp);

	/* Raise the saved exceptions.  */
	feraiseexcept (temp & FE_ALL_EXCEPT);

	/* Success.  */
	return 0;
#else
	/* Unsupported, so fail.  */
	return 1;
#endif
}
