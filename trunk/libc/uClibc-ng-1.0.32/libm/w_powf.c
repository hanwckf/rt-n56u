/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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

/* wrapper powf */
float
powf (float x, float y)
{
#if defined(__UCLIBC_HAS_FENV__)
  float z = (float)  __ieee754_pow ((double) x, (double) y);
  if (__builtin_expect (!isfinite (z), 0))
    {
      if (_LIB_VERSION != _IEEE_)
	{
	  if (isnan (x))
	    {
	      if (y == 0.0f)
		/* pow(NaN,0.0) */
		return __kernel_standard_f (x, y, 142);
	    }
	  else if (isfinite (x) && isfinite (y))
	    {
	      if (isnan (z))
		/* pow neg**non-int */
		return __kernel_standard_f (x, y, 124);
	      else if (x == 0.0f && y < 0.0f)
		{
		  if (signbit (x) && signbit (z))
		    /* pow(-0.0,negative) */
		    return __kernel_standard_f (x, y, 123);
		  else
		    /* pow(+0.0,negative) */
		    return __kernel_standard_f (x, y, 143);
		}
	      else
		/* pow overflow */
		return __kernel_standard_f (x, y, 121);
	    }
	}
    }
  else if (__builtin_expect (z == 0.0f, 0) && isfinite (x) && isfinite (y)
	   && _LIB_VERSION != _IEEE_)
    {
      if (x == 0.0f)
	{
	  if (y == 0.0f)
	    /* pow(0.0,0.0) */
	    return __kernel_standard_f (x, y, 120);
	}
      else
	/* pow underflow */
	return __kernel_standard_f (x, y, 122);
    }

	return z;
#else
	return (float) __ieee754_pow ((double) x, (double) y);
#endif /* __UCLIBC_HAS_FENV__ */
}
