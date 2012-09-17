/* Copyright (C) 1991, 1992, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Torbjorn Granlund (tege@sics.se).

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

#include <limits.h>
#include <string.h>

/* Find the first bit set in I.  */
int ffsll (long long int i)
{
  unsigned long long int x = i & -i;

  if (x <= 0xffffffff)
    return ffs (i);
  else
    return 32 + ffs (i >> 32);
}

#if ULONG_MAX != UINT_MAX
strong_alias_untyped(ffsll, ffsl)
#endif
