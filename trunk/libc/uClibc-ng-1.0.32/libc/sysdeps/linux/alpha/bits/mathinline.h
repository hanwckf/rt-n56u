/* Inline math functions for Alpha.
   Copyright (C) 1996, 1997, 1999-2001, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger-Tang.

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
# error "Never use <bits/mathinline.h> directly; include <math.h> instead."
#endif

#ifdef __cplusplus
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE __extern_inline
#endif

#if defined __USE_ISOC99 && defined __GNUC__ && !__GNUC_PREREQ(3,0)
# undef isgreater
# undef isgreaterequal
# undef isless
# undef islessequal
# undef islessgreater
# undef isunordered
# define isunordered(u, v)				\
  (__extension__					\
   ({ double __r, __u = (u), __v = (v);			\
      __asm__ ("cmptun/su %1,%2,%0\n\ttrapb"		\
	     : "=&f" (__r) : "f" (__u), "f"(__v));	\
      __r != 0; }))
#endif /* ISO C99 */

#if (!defined __NO_MATH_INLINES || defined __LIBC_INTERNAL_MATH_INLINES) \
    && defined __OPTIMIZE__

#if !__GNUC_PREREQ (4, 0)
# define __inline_copysign(NAME, TYPE)					\
__MATH_INLINE TYPE							\
__NTH (NAME (TYPE __x, TYPE __y))					\
{									\
  TYPE __z;								\
  __asm__ ("cpys %1, %2, %0" : "=f" (__z) : "f" (__y), "f" (__x));	\
  return __z;								\
}

__inline_copysign (__copysignf, float)
__inline_copysign (copysignf, float)
__inline_copysign (__copysign, double)
__inline_copysign (copysign, double)

# undef __inline_copysign
#endif


#if !__GNUC_PREREQ (2, 8)
# define __inline_fabs(NAME, TYPE)			\
__MATH_INLINE TYPE					\
__NTH (NAME (TYPE __x))					\
{							\
  TYPE __z;						\
  __asm__ ("cpys $f31, %1, %0" : "=f" (__z) : "f" (__x));	\
  return __z;						\
}

__inline_fabs (__fabsf, float)
__inline_fabs (fabsf, float)
__inline_fabs (__fabs, double)
__inline_fabs (fabs, double)

# undef __inline_fabs
#endif


/* Use the -inf rounding mode conversion instructions to implement
   floor.  We note when the exponent is large enough that the value
   must be integral, as this avoids unpleasant integer overflows.  */

__MATH_INLINE float
__NTH (__floorf (float __x))
{
  /* Check not zero since floor(-0) == -0.  */
  if (__x != 0 && fabsf (__x) < 16777216.0f)  /* 1 << FLT_MANT_DIG */
    {
      /* Note that Alpha S_Floating is stored in registers in a
	 restricted T_Floating format, so we don't even need to
	 convert back to S_Floating in the end.  The initial
	 conversion to T_Floating is needed to handle denormals.  */

      float __tmp1, __tmp2;

      __asm__ ("cvtst/s %3,%2\n\t"
#ifdef _IEEE_FP_INEXACT
	     "cvttq/svim %2,%1\n\t"
#else
	     "cvttq/svm %2,%1\n\t"
#endif
	     "cvtqt/m %1,%0\n\t"
	     : "=f"(__x), "=&f"(__tmp1), "=&f"(__tmp2)
	     : "f"(__x));
    }
  return __x;
}

__MATH_INLINE double
__NTH (__floor (double __x))
{
  if (__x != 0 && fabs (__x) < 9007199254740992.0)  /* 1 << DBL_MANT_DIG */
    {
      double __tmp1;
      __asm__ (
#ifdef _IEEE_FP_INEXACT
	     "cvttq/svim %2,%1\n\t"
#else
	     "cvttq/svm %2,%1\n\t"
#endif
	     "cvtqt/m %1,%0\n\t"
	     : "=f"(__x), "=&f"(__tmp1)
	     : "f"(__x));
    }
  return __x;
}

__MATH_INLINE float __NTH (floorf (float __x)) { return __floorf(__x); }
__MATH_INLINE double __NTH (floor (double __x)) { return __floor(__x); }


#ifdef __USE_ISOC99

__MATH_INLINE float
__NTH (__fdimf (float __x, float __y))
{
  return __x <= __y ? 0.0f : __x - __y;
}

__MATH_INLINE float
__NTH (fdimf (float __x, float __y))
{
  return __x <= __y ? 0.0f : __x - __y;
}

__MATH_INLINE double
__NTH (__fdim (double __x, double __y))
{
  return __x <= __y ? 0.0 : __x - __y;
}

__MATH_INLINE double
__NTH (fdim (double __x, double __y))
{
  return __x <= __y ? 0.0 : __x - __y;
}

/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
  __extension__ union { float __f; int __i; } __u = { __f: __x };
  return __u.__i < 0;
}

__MATH_INLINE int
__NTH (__signbit (double __x))
{
  __extension__ union { double __d; long __i; } __u = { __d: __x };
  return __u.__i < 0;
}

#endif /* C99 */

#endif /* __NO_MATH_INLINES */
