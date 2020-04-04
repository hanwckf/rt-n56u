/* Software floating-point emulation.
   scalblnl(x, exp)
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
 * scalblnl (long double x, long int n)
 * scalblnl(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

#include "soft-fp.h"
#include "quad.h"

long double __scalblnl(long double arg, int exp)
{
  FP_DECL_EX;
  FP_DECL_Q(A);
  long double r;

  FP_UNPACK_Q(A, arg);
  switch (A_c)
    {
    case FP_CLS_ZERO:
      return arg;
    case FP_CLS_NAN:
    case FP_CLS_INF:
      FP_HANDLE_EXCEPTIONS;
      return arg;
    }
  A_e += exp;
  FP_PACK_Q(r, A);
  FP_HANDLE_EXCEPTIONS;

  return r;
}
