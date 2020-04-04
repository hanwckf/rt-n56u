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
#include <fenv.h>
#endif

#if !defined __NO_LONG_DOUBLE_MATH
# ifndef __DO_XSI_MATH__
/* wrapper j0l */
long double
j0l (long double x)
{
#  if defined(__UCLIBC_HAS_FENV__)
  if (__builtin_expect (isgreater (fabsl (x), X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_ && _LIB_VERSION != _POSIX_)
    /* j0(|x|>X_TLOSS) */
    return __kernel_standard_l (x, x, 234);
#  endif /* __UCLIBC_HAS_FENV__ */
  return (long double) __ieee754_j0 ((double)x);
}

/* wrapper y0l */
long double
y0l (long double x)
{
#  if defined(__UCLIBC_HAS_FENV__)
  if (__builtin_expect (islessequal (x, 0.0L) || isgreater (x, X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_)
    {
      if (x < 0.0L)
	{
	  /* d = zero/(x-x) */
	  feraiseexcept (FE_INVALID);
	  return __kernel_standard_l (x, x, 209);
	}
      else if (x == 0.0L)
	{
	  /* d = -one/(x-x) */
	  feraiseexcept (FE_DIVBYZERO);
	  return __kernel_standard_l (x, x, 208);
	}
      else if (_LIB_VERSION != _POSIX_)
	/* y0(x>X_TLOSS) */
	return __kernel_standard_l (x, x, 235);
    }
#  endif /* __UCLIBC_HAS_FENV__ */
  return (long double)__ieee754_y0 ((double) x);
}
# endif /* __DO_XSI_MATH__ */
#endif /* !defined __NO_LONG_DOUBLE_MATH */
