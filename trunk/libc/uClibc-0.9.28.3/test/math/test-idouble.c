/* Copyright (C) 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1997.

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

#define FUNC(function) function
#define FLOAT double
#define TEST_MSG "testing double (inline functions)\n"
#define MATHCONST(x) x
#define CHOOSE(Clongdouble,Cdouble,Cfloat,Cinlinelongdouble,Cinlinedouble,Cinlinefloat) Cinlinedouble
#define PRINTF_EXPR "e"
#define PRINTF_XEXPR "a"
#define PRINTF_NEXPR "f"
#define TEST_DOUBLE 1
#define TEST_INLINE

#ifdef __NO_MATH_INLINES
# undef __NO_MATH_INLINES
#endif

#include "libm-test.c"
