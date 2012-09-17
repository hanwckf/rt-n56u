/* Software floating-point emulation.
   Helper routine for _Q_* routines.
   Simulate exceptions using double arithmetics.
   Copyright (C) 1999 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "soft-fp.h"

unsigned long long ___Q_numbers [] = {
0x0000000000000000ULL, /* Zero */
0x0010100000000000ULL, /* Very tiny number */
0x0010000000000000ULL, /* Minimum normalized number */
0x7fef000000000000ULL, /* A huge double number */
};

double ___Q_simulate_exceptions(int exceptions)
{
  double d, *p = (double *)___Q_numbers;
  if (exceptions & FP_EX_INVALID)
    d = p[0]/p[0];
  if (exceptions & FP_EX_OVERFLOW)
    {
      d = p[3] + p[3];
      exceptions &= ~FP_EX_INEXACT;
    }
  if (exceptions & FP_EX_UNDERFLOW)
    {
      if (exceptions & FP_EX_INEXACT)
        {
	  d = p[2] * p[2];
	  exceptions &= ~FP_EX_INEXACT;
	}
      else
	d = p[1] - p[2];
    }
  if (exceptions & FP_EX_DIVZERO)
    d = 1.0/p[0];
  if (exceptions & FP_EX_INEXACT)
    d = p[3] - p[2];
  return d;
}
