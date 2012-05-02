/* `HUGE_VAL' constants for IEEE 754 machines (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.
   ARM version.
   Copyright (C) 1992, 95, 96, 97, 98, 99, 2000 Free Software Foundation, Inc.
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

/* IEEE positive infinity (-HUGE_VAL is negative infinity).  */

#ifdef	__GNUC__

# if __GNUC_PREREQ(2,96)

#  define HUGE_VAL (__extension__ 0x1.0p2047)

# else

#  define HUGE_VAL \
  (__extension__							      \
   ((union { unsigned __l __attribute__((__mode__(__DI__))); double __d; })   \
    { __l: 0x000000007ff00000ULL }).__d)

# endif

#else /* not GCC */

# include <endian.h>

typedef union { unsigned char __c[8]; double __d; } __huge_val_t;

# if __BYTE_ORDER == __BIG_ENDIAN
#  define __HUGE_VAL_bytes	{ 0, 0, 0, 0, 0x7f, 0xf0, 0, 0 }
# endif
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define __HUGE_VAL_bytes	{ 0, 0, 0xf0, 0x7f, 0, 0, 0, 0 }
# endif

static __huge_val_t __huge_val = { __HUGE_VAL_bytes };
# define HUGE_VAL	(__huge_val.__d)

#endif	/* GCC.  */


/* ISO C99 extensions: (float) HUGE_VALF and (long double) HUGE_VALL.  */

#ifdef __USE_ISOC99

# ifdef __GNUC__

#  if __GNUC_PREREQ(2,96)

#   define HUGE_VALF (__extension__ 0x1.0p255f)

#  else

#   define HUGE_VALF \
  (__extension__							      \
   ((union { unsigned __l __attribute__((__mode__(__SI__))); float __d; })    \
    { __l: 0x7f800000UL }).__d)

#  endif

# else /* not GCC */

typedef union { unsigned char __c[4]; float __f; } __huge_valf_t;

#  if __BYTE_ORDER == __BIG_ENDIAN
#   define __HUGE_VALF_bytes	{ 0x7f, 0x80, 0, 0 }
#  endif
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define __HUGE_VALF_bytes	{ 0, 0, 0x80, 0x7f }
#  endif

static __huge_valf_t __huge_valf = { __HUGE_VALF_bytes };
#  define HUGE_VALF	(__huge_valf.__f)

# endif	/* GCC.  */


/* Generally there is no separate `long double' format and it is the
   same as `double'.  */
# define HUGE_VALL HUGE_VAL

#endif /* __USE_ISOC99.  */
