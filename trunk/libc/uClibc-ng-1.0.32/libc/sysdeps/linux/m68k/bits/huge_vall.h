/* `HUGE_VALL' constant for m68k (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.
   Copyright (C) 1992, 1995, 1996, 1997, 1999, 2000, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _MATH_H
# error "Never use <bits/huge_val.h> directly; include <math.h> instead."
#endif

#if __GNUC_PREREQ(3,3)
# define HUGE_VALL	(__builtin_huge_vall ())
#elif __GNUC_PREREQ(2,96)
# define HUGE_VALL	(__extension__ 0x1.0p32767L)
#elif defined __GNUC__

# define HUGE_VALL					\
  (__extension__					\
   ((union { unsigned long __l[3]; long double __ld; })	\
    { __l: { 0x7fff0000UL, 0x80000000UL, 0UL } }).__ld)

#else /* not GCC */

static union { unsigned char __c[12]; long double __ld; } __huge_vall =
  { { 0x7f, 0xff, 0, 0, 0x80, 0, 0, 0, 0, 0, 0, 0 } };
# define HUGE_VALL	(__huge_vall.__ld)

#endif /* GCC 2.95.  */
