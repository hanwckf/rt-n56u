/* Round double value to long long int.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

long long int
llround (double x)
{
  int32_t _j0;
  u_int32_t i1, i0;
  long long int result;
  int sign;

  EXTRACT_WORDS (i0, i1, x);
  _j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  sign = (i0 & 0x80000000) != 0 ? -1 : 1;
  i0 &= 0xfffff;
  i0 |= 0x100000;

  if (_j0 < 20)
    {
      if (_j0 < 0)
	return _j0 < -1 ? 0 : sign;
      else
	{
	  i0 += 0x80000 >> _j0;

	  result = i0 >> (20 - _j0);
	}
    }
  else if (_j0 < (int32_t) (8 * sizeof (long long int)) - 1)
    {
      if (_j0 >= 52)
	result = (((long long int) i0 << 32) | i1) << (_j0 - 52);
      else
	{
	  u_int32_t j = i1 + (0x80000000 >> (_j0 - 20));
	  if (j < i1)
	    ++i0;

	  if (_j0 == 20)
	    result = (long long int) i0;
	  else
	    result = ((long long int) i0 << (_j0 - 20)) | (j >> (52 - _j0));
	}
    }
  else
    {
      /* The number is too large.  It is left implementation defined
	 what happens.  */
      return (long long int) x;
    }

  return sign * result;
}
libm_hidden_def(llround)
