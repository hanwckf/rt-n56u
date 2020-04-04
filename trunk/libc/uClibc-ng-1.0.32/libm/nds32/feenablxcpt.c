/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Enable floating-point exceptions.
   Copyright (C) 2001-2013 Free Software Foundation, Inc.
   Contributed by Philip Blundell <philb@gnu.org>, 2001.

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
#include "fenv_libc.h"
#include <fpu_control.h>

int
feenableexcept (int excepts)
{
#ifdef __NDS32_ABI_2FP_PLUS__
	unsigned long int new_exc, old_exc;

	_FPU_GETCW(new_exc);

	old_exc = (new_exc & ENABLE_MASK) >> ENABLE_SHIFT;

	excepts &= FE_ALL_EXCEPT;

	new_exc |= (excepts << ENABLE_SHIFT);
	new_exc &= ~_FPU_RESERVED;

	_FPU_SETCW(new_exc);

	return old_exc;
#else
  /* Unsupported, so return -1 for failure.  */
  return -1;
#endif
}
