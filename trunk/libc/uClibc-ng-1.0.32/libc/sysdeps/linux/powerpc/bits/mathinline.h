/* Inline math functions for powerpc.
   Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2004, 2006
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

#include <features.h>

#ifndef _MATH_H
# error "Never use <bits/mathinline.h> directly; include <math.h> instead."
#endif

#ifdef __cplusplus
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE __extern_inline
#endif  /* __cplusplus */

#if defined __GNUC__ && !defined _SOFT_FLOAT

#ifdef __USE_ISOC99
# if !__GNUC_PREREQ (2,97)
#  define __unordered_cmp(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      unsigned __r;							      \
      __asm__("fcmpu 7,%1,%2 ; mfcr %0" : "=r" (__r) : "f" (__x), "f"(__y)    \
              : "cr7");  \
      __r; }))

#  undef isgreater
#  undef isgreaterequal
#  undef isless
#  undef islessequal
#  undef islessgreater
#  undef isunordered

#  define isgreater(x, y) (__unordered_cmp (x, y) >> 2 & 1)
#  define isgreaterequal(x, y) ((__unordered_cmp (x, y) & 6) != 0)
#  define isless(x, y) (__unordered_cmp (x, y) >> 3 & 1)
#  define islessequal(x, y) ((__unordered_cmp (x, y) & 0xA) != 0)
#  define islessgreater(x, y) ((__unordered_cmp (x, y) & 0xC) != 0)
#  define isunordered(x, y) (__unordered_cmp (x, y) & 1)

# endif /* __GNUC_PREREQ (2,97) */

#endif /* __USE_ISOC99 */

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

#ifdef __USE_ISOC99

__MATH_INLINE long int lrint (double __x) __THROW;
__MATH_INLINE long int
__NTH (lrint (double __x))
{
  union {
    double __d;
    int __ll[2];
  } __u;
  __asm__ ("fctiw %0,%1" : "=f"(__u.__d) : "f"(__x));
  return __u.__ll[1];
}

__MATH_INLINE long int lrintf (float __x) __THROW;
__MATH_INLINE long int
__NTH (lrintf (float __x))
{
  union {
    double __d;
    int __ll[2];
  } __u;
  __asm__ ("fctiw %0,%1" : "=f"(__u.__d) : "f"(__x));
  return __u.__ll[1];
}

__MATH_INLINE double fdim (double __x, double __y) __THROW;
__MATH_INLINE double
__NTH (fdim (double __x, double __y))
{
  return __x <= __y ? 0 : __x - __y;
}

__MATH_INLINE float fdimf (float __x, float __y) __THROW;
__MATH_INLINE float
__NTH (fdimf (float __x, float __y))
{
  return __x <= __y ? 0 : __x - __y;
}

#endif /* __USE_ISOC99 */
#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */

#endif /* __GNUC__ && !_SOFT_FLOAT */

