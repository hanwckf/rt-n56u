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
#if defined(__UCLIBC_HAS_FENV__)
#include <errno.h>
#endif

#if !defined __NO_LONG_DOUBLE_MATH
# if defined(__UCLIBC_HAS_FENV__)
static long double
__attribute__ ((noinline))
sysv_scalbl (long double x, long double fn)
{
  long double z = (long double) __ieee754_scalb ((double) x, (double) fn);

  if (__builtin_expect (isinf (z), 0))
    {
      if (isfinite (x))
	return __kernel_standard_l (x, fn, 232); /* scalb overflow */
      else
	__set_errno (ERANGE);
    }
  else if (__builtin_expect (z == 0.0L, 0) && z != x)
    return __kernel_standard_l (x, fn, 233); /* scalb underflow */

  return z;
}
# endif /* __UCLIBC_HAS_FENV__ */

/* Wrapper scalbl */
long double
scalbl (long double x, long double fn)
{
# if defined(__UCLIBC_HAS_FENV__)
  if (__builtin_expect (_LIB_VERSION == _SVID_, 0))
    return sysv_scalbl (x, fn);
  else
    {
      long double z = (long double) __ieee754_scalb ((double) x, (double) fn);

      if (__builtin_expect (!isfinite (z) || z == 0.0L, 0))
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
	      if (x != 0.0L && !isinf (fn))
		__set_errno (ERANGE);
	    }
	}
      return z;
    }
# else
    return (long double) __ieee754_scalb ((double) x, (double) fn);
# endif /* __UCLIBC_HAS_FENV__ */
}
#endif /* __NO_LONG_DOUBLE_MATH */
