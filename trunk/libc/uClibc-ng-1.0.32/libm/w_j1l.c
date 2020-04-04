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
/* wrapper j1l */
long double
j1l (long double x)
{
#  if defined(__UCLIBC_HAS_FENV__)
  if (__builtin_expect (isgreater (fabsl (x), X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_ && _LIB_VERSION != _POSIX_)
    /* j1(|x|>X_TLOSS) */
    return __kernel_standard_l (x, x, 236);
#  endif /* __UCLIBC_HAS_FENV__ */
  return (long double) __ieee754_j1 ((double) x);
}

/* wrapper y1l */
long double
y1l (long double x)
{
#  if defined(__UCLIBC_HAS_FENV__)
  if (__builtin_expect (islessequal (x, 0.0L) || isgreater (x, X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_)
    {
      if (x < 0.0L)
	{
	  /* d = zero/(x-x) */
	  feraiseexcept (FE_INVALID);
	  return __kernel_standard_l (x, x, 211);
	}
      else if (x == 0.0L)
	{
	  /* d = -one/(x-x) */
	  feraiseexcept (FE_DIVBYZERO);
	  return __kernel_standard_l (x, x, 210);
	}
      else if (_LIB_VERSION != _POSIX_)
	/* y1(x>X_TLOSS) */
	return __kernel_standard_l (x, x, 237);
    }
#  endif /* __UCLIBC_HAS_FENV__ */
  return (long double) __ieee754_y1 ((double) x);
}
# endif /* __DO_XSI_MATH__ */
#endif /* __NO_LONG_DOUBLE_MATH */
