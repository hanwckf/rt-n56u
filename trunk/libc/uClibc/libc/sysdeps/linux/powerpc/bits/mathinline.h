/* Inline math functions for powerpc.
   Copyright (C) 1995,1996,1997,1998,1999,2000 Free Software Foundation, Inc.
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

#if defined __GNUC__ && !defined _SOFT_FLOAT

#ifdef __USE_ISOC99
# if __GNUC_PREREQ (2,96)

#  define isgreater(x, y) __builtin_isgreater (x, y)
#  define isgreaterequal(x, y) __builtin_isgreaterequal (x, y)
#  define isless(x, y) __builtin_isless (x, y)
#  define islessequal(x, y) __builtin_islessequal (x, y)
#  define islessgreater(x, y) __builtin_islessgreater (x, y)
#  define isunordered(x, y) __builtin_isunordered (x, y)

# else

#  define __unordered_cmp(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      unsigned __r;							      \
      __asm__("fcmpu 7,%1,%2 ; mfcr %0" : "=r" (__r) : "f" (__x), "f"(__y)    \
              : "cr7");  \
      __r; }))

#  define isgreater(x, y) (__unordered_cmp (x, y) >> 2 & 1)
#  define isgreaterequal(x, y) ((__unordered_cmp (x, y) & 6) != 0)
#  define isless(x, y) (__unordered_cmp (x, y) >> 3 & 1)
#  define islessequal(x, y) ((__unordered_cmp (x, y) & 0xA) != 0)
#  define islessgreater(x, y) ((__unordered_cmp (x, y) & 0xC) != 0)
#  define isunordered(x, y) (__unordered_cmp (x, y) & 1)

# endif /* __GNUC_PREREQ (2,97) */
#endif /* __USE_ISOC99 */

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

#ifdef __cplusplus
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE extern __inline
#endif  /* __cplusplus */

#ifdef __USE_ISOC99
__MATH_INLINE long int lrint (double __x) __THROW;
__MATH_INLINE long int
lrint (double __x) __THROW
{
  union {
    double __d;
    long int __ll[2];
  } __u;
  __asm__ ("fctiw %0,%1" : "=f"(__u.__d) : "f"(__x));
  return __u.__ll[1];
}

__MATH_INLINE long int lrintf (float __x) __THROW;
__MATH_INLINE long int
lrintf (float __x) __THROW
{
  union {
    double __d;
    long int __ll[2];
  } __u;
  __asm__ ("fctiw %0,%1" : "=f"(__u.__d) : "f"(__x));
  return __u.__ll[1];
}

__MATH_INLINE double fdim (double __x, double __y) __THROW;
__MATH_INLINE double
fdim (double __x, double __y) __THROW
{
  return __x < __y ? 0 : __x - __y;
}

__MATH_INLINE float fdimf (float __x, float __y) __THROW;
__MATH_INLINE float
fdimf (float __x, float __y) __THROW
{
  return __x < __y ? 0 : __x - __y;
}

#endif /* __USE_ISOC99 */
#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */
#endif /* __GNUC__ && !_SOFT_FLOAT */
