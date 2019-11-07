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


/*
 * wrapper exp10f(x)
 */

#include <math.h>
#include "math_private.h"

float
exp10f (float x)
{
#if defined(__UCLIBC_HAS_FENV__)
  float z = (float) __ieee754_exp10 ((double) x);
  if (__builtin_expect (!isfinite (z) || z == 0, 0)
      && isfinite (x) && _LIB_VERSION != _IEEE_)
    /* exp10f overflow (146) if x > 0, underflow (147) if x < 0.  */
    return __kernel_standard_f (x, x, 146 + !!signbit (x));

  return z;
#else
  return (float) __ieee754_exp10 ((double) x);
#endif
}
strong_alias (exp10f, pow10f)
