/* `HUGE_VAL' constants for m68k (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.
   Copyright (C) 1992, 1995, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _MATH_H
# error "Never use <bits/huge_val.h> directly; include <math.h> instead."
#endif


#include <features.h>
#include <sys/cdefs.h>

/* IEEE positive infinity (-HUGE_VAL is negative infinity).  */

#ifdef	__GNUC__

# if __GNUC_PREREQ(2,96)

#  define HUGE_VAL (__extension__ 0x1.0p2047)

# else

#  define HUGE_VAL					\
  (__extension__					\
   ((union { unsigned long long __l; double __d; })	\
    { __l: 0x7ff0000000000000ULL }).__d)

# endif

#else /* not GCC */

static union { unsigned char __c[8]; double __d; } __huge_val =
  { { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 } };
# define HUGE_VAL	(__huge_val.__d)

#endif	/* GCC.  */


/* ISO C 99 extensions: (float) HUGE_VALF and (long double) HUGE_VALL.  */

#ifdef __USE_ISOC99

# if __GNUC_PREREQ(2,96)

#  define HUGE_VALF (__extension__ 0x1.0p255f)
#  define HUGE_VALL (__extension__ 0x1.0p32767L)

# else

#  ifdef __GNUC__

#   define HUGE_VALF					\
  (__extension__					\
   ((union { unsigned long __l; float __f; })		\
    { __l: 0x7f800000UL }).__f)

#   define HUGE_VALL					\
  (__extension__					\
   ((union { unsigned long __l[3]; long double __ld; })	\
    { __l: { 0x7fff0000UL, 0x80000000UL, 0UL } }).__ld)

#  else /* not GCC */

static union { unsigned char __c[4]; float __f; } __huge_valf =
  { { 0x7f, 0x80, 0, 0 } };
#   define HUGE_VALF	(__huge_valf.__f)

static union { unsigned char __c[12]; long double __ld; } __huge_vall =
  { { 0x7f, 0xff, 0, 0, 0x80, 0, 0, 0, 0, 0, 0, 0 } };
#   define HUGE_VALL	(__huge_vall.__ld)

#  endif /* GCC.  */

# endif /* GCC 2.95.  */

#endif	/* __USE_ISOC99.  */
