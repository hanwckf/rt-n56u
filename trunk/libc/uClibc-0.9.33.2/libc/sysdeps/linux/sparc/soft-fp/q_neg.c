/* Software floating-point emulation.
   Return !a
   Copyright (C) 1997,1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@cygnus.com) and
		  Jakub Jelinek (jj@ultra.linux.cz).

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
#include "quad.h"

long double _Q_neg(const long double a)
{
  FP_DECL_EX;
  long double c = a;
  
#if (__BYTE_ORDER == __BIG_ENDIAN)
  ((UWtype *)&c)[0] ^= (((UWtype)1) << (W_TYPE_SIZE - 1));
#elif (__BYTE_ORDER == __LITTLE_ENDIAN) && (W_TYPE_SIZE == 64)
  ((UWtype *)&c)[1] ^= (((UWtype)1) << (W_TYPE_SIZE - 1));
#elif (__BYTE_ORDER == __LITTLE_ENDIAN) && (W_TYPE_SIZE == 32)
  ((UWtype *)&c)[3] ^= (((UWtype)1) << (W_TYPE_SIZE - 1));
#else
  FP_DECL_Q(A); FP_DECL_Q(C);

  FP_UNPACK_Q(A, a);
  FP_NEG_Q(C, A);
  FP_PACK_Q(c, C);
#endif
  FP_CLEAR_EXCEPTIONS;
  FP_HANDLE_EXCEPTIONS;
  return c;
}
