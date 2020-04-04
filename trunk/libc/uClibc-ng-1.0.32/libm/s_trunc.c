/* Truncate argument to nearest integral value not larger than the argument.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <math.h>
#include "math_private.h"

double
trunc (double x)
{
  int32_t i0, _j0;
  u_int32_t i1;
  int sx;

  EXTRACT_WORDS (i0, i1, x);
  sx = i0 & 0x80000000;
  _j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (_j0 < 20)
    {
      if (_j0 < 0)
	/* The magnitude of the number is < 1 so the result is +-0.  */
	INSERT_WORDS (x, sx, 0);
      else
	INSERT_WORDS (x, sx | (i0 & ~(0x000fffff >> _j0)), 0);
    }
  else if (_j0 > 51)
    {
      if (_j0 == 0x400)
	/* x is inf or NaN.  */
	return x + x;
    }
  else
    {
      INSERT_WORDS (x, i0, i1 & ~(0xffffffffu >> (_j0 - 20)));
    }

  return x;
}
libm_hidden_def(trunc)
