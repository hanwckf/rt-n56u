/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 2002-2013 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#if defined(__NDS32_ABI_2FP_PLUS__) && defined(__NDS32_EXT_FPU_DP__)

double __ieee754_sqrt (double x)
{
  double z;
  __asm__ ("fsqrtd %0,%1" : "=f" (z) : "f" (x));
  return z;
}
strong_alias(__ieee754_sqrt, sqrt)
libm_hidden_def(sqrt)
#else
#include <libm/e_sqrt.c>
#endif
