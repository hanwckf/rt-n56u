/* Software floating-point emulation.
   ilogbl(x, exp)
   Copyright (C) 1999-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek (jj@ultra.linux.cz).

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

/* ilogbl(long double x)
 * return the binary exponent of non-zero x
 * ilogbl(0) = 0x80000001
 * ilogbl(inf/NaN) = 0x7fffffff (no signal is raised)
 */

#include "soft-fp.h"
#include "quad.h"
#include <math.h>

int __ieee754_ilogbl (long double x)
{
  FP_DECL_EX;
  FP_DECL_Q(X);

/*
  FP_UNPACK_Q(X, x);
  switch (X_c)
    {
    case FP_CLS_ZERO:
      return FP_ILOGB0;
    case FP_CLS_NAN:
    case FP_CLS_INF:
      return FP_ILOGBNAN;
    default:
      return X_e;
    }
 */
  FP_UNPACK_RAW_Q(X, x);
  switch (X_e)
    {
    default:
      return X_e - _FP_EXPBIAS_Q;
    case 0:
#if (2 * _FP_W_TYPE_SIZE) < _FP_FRACBITS_Q
      if (_FP_FRAC_ZEROP_4(X))
	return FP_ILOGB0;
      else
	{
	  _FP_I_TYPE shift;
	  _FP_FRAC_CLZ_4(shift, X);
	  shift -= _FP_FRACXBITS_Q;
	  return X_e - _FP_EXPBIAS_Q - 1 + shift;
	}
#else
      if (_FP_FRAC_ZEROP_2(X))
	return FP_ILOGB0;
      else
	{
	  _FP_I_TYPE shift;
	  _FP_FRAC_CLZ_2(shift, X);
	  shift -= _FP_FRACXBITS_Q;
	  return X_e - _FP_EXPBIAS_Q - 1 + shift;
	}
#endif
    case _FP_EXPBIAS_Q:
      return FP_ILOGBNAN;
    }
}
