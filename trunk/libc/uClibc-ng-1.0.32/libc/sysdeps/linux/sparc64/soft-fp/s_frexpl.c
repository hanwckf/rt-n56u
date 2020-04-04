/* Software floating-point emulation.
   frexpl(x, exp)
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

/*
 * for non-zero x
 *      x = frexpl(arg,&exp);
 * return a long double fp quantity x such that 0.5 <= |x| <1.0
 * and the corresponding binary exponent "exp". That is
 *      arg = x*2^exp.
 * If arg is inf, 0.0, or NaN, then frexpl(arg,&exp) returns arg
 * with *exp=0.
 */

#include "soft-fp.h"
#include "quad.h"

long double __frexpl(long double arg, int *exp)
{
  FP_DECL_EX;
  FP_DECL_Q(A);
  long double r;

  *exp = 0;
  FP_UNPACK_Q(A, arg);
  if (A_c != FP_CLS_NORMAL)
    return arg;
  *exp = A_e + 1;
  A_e = -1;
  FP_PACK_Q(r, A);

  return r;
}

weak_alias (__frexpl, frexpl)
