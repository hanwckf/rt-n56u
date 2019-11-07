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


/* wrapper pow */
double
pow (double x, double y)
{
#if defined(__UCLIBC_HAS_FENV__)
	double z = __ieee754_pow (x, y);
  if (__builtin_expect (!isfinite (z), 0))
    {
      if (_LIB_VERSION != _IEEE_)
	{
	  if (isnan (x))
	    {
	      if (y == 0.0)
		/* pow(NaN,0.0) */
		return __kernel_standard (x, y, 42);
	    }
	  else if (isfinite (x) && isfinite (y))
	    {
	      if (isnan (z))
		/* pow neg**non-int */
		return __kernel_standard (x, y, 24);
	      else if (x == 0.0 && y < 0.0)
		{
		  if (signbit (x) && signbit (z))
		    /* pow(-0.0,negative) */
		    return __kernel_standard (x, y, 23);
		  else
		    /* pow(+0.0,negative) */
		    return __kernel_standard (x, y, 43);
		}
	      else
		/* pow overflow */
		return __kernel_standard (x, y, 21);
	    }
	}
    }
  else if (__builtin_expect (z == 0.0, 0) && isfinite (x) && isfinite (y)
	   && _LIB_VERSION != _IEEE_)
    {
      if (x == 0.0)
	{
	  if (y == 0.0)
	    /* pow(0.0,0.0) */
	    return __kernel_standard (x, y, 20);
	}
      else
	/* pow underflow */
	return __kernel_standard (x, y, 22);
    }

	return z;
#else
	return __ieee754_pow (x, y);
#endif
}
libm_hidden_def(pow)
