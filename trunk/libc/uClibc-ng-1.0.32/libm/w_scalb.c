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

#if defined __UCLIBC_SUSV3_LEGACY__

#if defined(__UCLIBC_HAS_FENV__)
#include <errno.h>

static double
__attribute__ ((noinline))
sysv_scalb (double x, double fn)
{
  double z = __ieee754_scalb (x, fn);

  if (__builtin_expect (isinf (z), 0))
    {
      if (isfinite (x))
	return __kernel_standard (x, fn, 32); /* scalb overflow */
      else
	__set_errno (ERANGE);
    }
  else if (__builtin_expect (z == 0.0, 0) && z != x)
    return __kernel_standard (x, fn, 33); /* scalb underflow */

  return z;
}
#endif /* __UCLIBC_HAS_FENV__ */

/* Wrapper scalb */
double
scalb (double x, double fn)
{
#if defined(__UCLIBC_HAS_FENV__)
  if (__builtin_expect (_LIB_VERSION == _SVID_,0))
    return sysv_scalb (x, fn);
  else
    {
      double z = __ieee754_scalb (x, fn);

      if (__builtin_expect (!isfinite (z) || z == 0.0, 0))
	{
	  if (isnan (z))
	    {
	      if (!isnan (x) && !isnan (fn))
		__set_errno (EDOM);
	    }
	  else if (isinf (z))
	    {
	      if (!isinf (x) && !isinf (fn))
		__set_errno (ERANGE);
	    }
	  else
	    {
	      /* z == 0.  */
	      if (x != 0.0 && !isinf (fn))
		__set_errno (ERANGE);
	    }
	}
      return z;
    }
#else
  return __ieee754_scalb (x, fn);
#endif /* __UCLIBC_HAS_FENV__ */
}

libm_hidden_def(scalb)
#endif
