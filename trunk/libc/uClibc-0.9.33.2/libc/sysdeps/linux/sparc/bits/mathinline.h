/* Inline math functions for SPARC.
   Copyright (C) 1999, 2000, 2001, 2002, 2004, 2006
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

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
# error "Never use <bits/mathinline.h> directly; include <math.h> instead."
#endif

#include <bits/wordsize.h>

#ifdef __GNUC__

#if defined __USE_ISOC99 && !__GNUC_PREREQ (3, 0)
# undef isgreater
# undef isgreaterequal
# undef isless
# undef islessequal
# undef islessgreater
# undef isunordered

# if __WORDSIZE == 32

#  ifndef __NO_LONG_DOUBLE_MATH

#   define __unordered_cmp(x, y) \
  (__extension__							      \
   ({ unsigned __r;							      \
      if (sizeof (x) == 4 && sizeof (y) == 4)				      \
	{								      \
	  float __x = (x); float __y = (y);				      \
	  __asm__ ("fcmps %1,%2; st %%fsr, %0" : "=m" (__r) : "f" (__x),      \
		   "f" (__y) : "cc");					      \
	}								      \
      else if (sizeof (x) <= 8 && sizeof (y) <= 8)			      \
	{								      \
	  double __x = (x); double __y = (y);				      \
	  __asm__ ("fcmpd\t%1,%2\n\tst\t%%fsr,%0" : "=m" (__r) : "f" (__x),   \
		   "f" (__y) : "cc");					      \
	}								      \
      else								      \
	{								      \
	  long double __x = (x); long double __y = (y);			      \
	  extern int _Q_cmp (const long double a, const long double b);	      \
	  __r = _Q_cmp (__x, __y) << 10;				      \
	}								      \
      __r; }))

#  else

#   define __unordered_cmp(x, y) \
  (__extension__							      \
   ({ unsigned __r;							      \
      if (sizeof (x) == 4 && sizeof (y) == 4)				      \
	{								      \
	  float __x = (x); float __y = (y);				      \
	  __asm__ ("fcmps %1,%2; st %%fsr, %0" : "=m" (__r) : "f" (__x),      \
		   "f" (__y) : "cc");					      \
	}								      \
      else								      \
	{								      \
	  double __x = (x); double __y = (y);				      \
	  __asm__ ("fcmpd\t%1,%2\n\tst\t%%fsr,%0" : "=m" (__r) : "f" (__x),   \
		   "f" (__y) : "cc");					      \
	}								      \
      __r; }))

#  endif

#  define isgreater(x, y) ((__unordered_cmp (x, y) & (3 << 10)) == (2 << 10))
#  define isgreaterequal(x, y) ((__unordered_cmp (x, y) & (1 << 10)) == 0)
#  define isless(x, y) ((__unordered_cmp (x, y) & (3 << 10)) == (1 << 10))
#  define islessequal(x, y) ((__unordered_cmp (x, y) & (2 << 10)) == 0)
#  define islessgreater(x, y) (((__unordered_cmp (x, y) + (1 << 10)) & (2 << 10)) != 0)
#  define isunordered(x, y) ((__unordered_cmp (x, y) & (3 << 10)) == (3 << 10))

# else /* sparc64 */

#  define __unordered_v9cmp(x, y, op, qop) \
  (__extension__							      \
   ({ unsigned __r;							      \
      if (sizeof (x) == 4 && sizeof (y) == 4)				      \
	{								      \
	  float __x = (x); float __y = (y);				      \
	  __asm__ ("fcmps\t%%fcc3,%1,%2\n\tmov" op "\t%%fcc3,1,%0"	      \
		   : "=r" (__r) : "f" (__x), "f" (__y), "0" (0) : "cc");      \
	}								      \
      else if (sizeof (x) <= 8 && sizeof (y) <= 8)			      \
	{								      \
	  double __x = (x); double __y = (y);				      \
	  __asm__ ("fcmpd\t%%fcc3,%1,%2\n\tmov" op "\t%%fcc3,1,%0"	      \
		   : "=r" (__r) : "f" (__x), "f" (__y), "0" (0) : "cc");      \
	}								      \
      else								      \
	{								      \
	  long double __x = (x); long double __y = (y);			      \
	  extern int _Qp_cmp (const long double *a, const long double *b);    \
	  __r = qop;							      \
	}								      \
      __r; }))

#  define isgreater(x, y) __unordered_v9cmp(x, y, "g", _Qp_cmp (&__x, &__y) == 2)
#  define isgreaterequal(x, y) __unordered_v9cmp(x, y, "ge", (_Qp_cmp (&__x, &__y) & 1) == 0)
#  define isless(x, y) __unordered_v9cmp(x, y, "l", _Qp_cmp (&__x, &__y) == 1)
#  define islessequal(x, y) __unordered_v9cmp(x, y, "le", (_Qp_cmp (&__x, &__y) & 2) == 0)
#  define islessgreater(x, y) __unordered_v9cmp(x, y, "lg", ((_Qp_cmp (&__x, &__y) + 1) & 2) != 0)
#  define isunordered(x, y) __unordered_v9cmp(x, y, "u", _Qp_cmp (&__x, &__y) == 3)

# endif /* sparc64 */

#endif /* __USE_ISOC99 */

#if (!defined __NO_MATH_INLINES || defined __LIBC_INTERNAL_MATH_INLINES) && defined __OPTIMIZE__

# ifdef __cplusplus
#  define __MATH_INLINE __inline
# else
#  define __MATH_INLINE extern __inline
# endif  /* __cplusplus */

/* The gcc, version 2.7 or below, has problems with all this inlining
   code.  So disable it for this version of the compiler.  */
# if __GNUC_PREREQ (2, 8)

#  ifdef __USE_ISOC99

/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
  __extension__ union { float __f; int __i; } __u = { __f: __x };
  return __u.__i < 0;
}

#   if __WORDSIZE == 32

__MATH_INLINE int
__NTH (__signbit (double __x))
{
  __extension__ union { double __d; int __i[2]; } __u = { __d: __x };
  return __u.__i[0] < 0;
}

#    ifndef __NO_LONG_DOUBLE_MATH
__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  __extension__ union { long double __l; int __i[4]; } __u = { __l: __x };
  return __u.__i[0] < 0;
}
#    else
__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  return __signbit ((double)__x);
}
#    endif

#   else /* sparc64 */

__MATH_INLINE int
__NTH (__signbit (double __x))
{
  __extension__ union { double __d; long int __i; } __u = { __d: __x };
  return __u.__i < 0;
}

__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  __extension__ union { long double __l; long int __i[2]; } __u = { __l: __x };
  return __u.__i[0] < 0;
}

#   endif /* sparc64 */

#  endif /* __USE_ISOC99 */

#  if !defined __NO_MATH_INLINES && !__GNUC_PREREQ (3, 2)

__MATH_INLINE double
__NTH (sqrt (double __x))
{
  register double __r;
  __asm__ ("fsqrtd %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

__MATH_INLINE float
__NTH (sqrtf (float __x))
{
  register float __r;
  __asm__ ("fsqrts %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

#   if __WORDSIZE == 64
__MATH_INLINE long double
__NTH (sqrtl (long double __x))
{
  long double __r;
  extern void _Qp_sqrt (long double *, __const__ long double *);
  _Qp_sqrt (&__r, &__x);
  return __r;
}
#   elif !defined __NO_LONG_DOUBLE_MATH
__MATH_INLINE long double
sqrtl (long double __x) __THROW
{
  extern long double _Q_sqrt (__const__ long double);
  return _Q_sqrt (__x);
}
#   endif /* sparc64 */

#  endif /* !__NO_MATH_INLINES && !GCC 3.2+ */

/* This code is used internally in the GNU libc.  */
#  ifdef __LIBC_INTERNAL_MATH_INLINES
__MATH_INLINE double
__ieee754_sqrt (double __x)
{
  register double __r;
  __asm__ ("fsqrtd %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

__MATH_INLINE float
__ieee754_sqrtf (float __x)
{
  register float __r;
  __asm__ ("fsqrts %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

#   if __WORDSIZE == 64
__MATH_INLINE long double
__ieee754_sqrtl (long double __x)
{
  long double __r;
  extern void _Qp_sqrt (long double *, __const__ long double *);
  _Qp_sqrt(&__r, &__x);
  return __r;
}
#   elif !defined __NO_LONG_DOUBLE_MATH
__MATH_INLINE long double
__ieee754_sqrtl (long double __x)
{
  extern long double _Q_sqrt (__const__ long double);
  return _Q_sqrt (__x);
}
#   endif /* sparc64 */
#  endif /* __LIBC_INTERNAL_MATH_INLINES */
# endif /* gcc 2.8+ */

# ifdef __USE_ISOC99

#  ifndef __NO_MATH_INLINES

__MATH_INLINE double __NTH (fdim (double __x, double __y));
__MATH_INLINE double
__NTH (fdim (double __x, double __y))
{
  return __x <= __y ? 0 : __x - __y;
}

__MATH_INLINE float __NTH (fdimf (float __x, float __y));
__MATH_INLINE float
__NTH (fdimf (float __x, float __y))
{
  return __x <= __y ? 0 : __x - __y;
}

#  endif /* !__NO_MATH_INLINES */
# endif /* __USE_ISOC99 */
#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */
#endif /* __GNUC__ */
