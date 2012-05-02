/* Definitions of inline math functions implemented by the m68881/2.
   Copyright (C) 1991,92,93,94,96,97,98,99,2000 Free Software Foundation, Inc.
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

#ifdef	__GNUC__

#ifdef __USE_ISOC99

/* ISO C99 defines some macros to perform unordered comparisons.  The
   m68k FPU supports this with special opcodes and we should use them.
   These must not be inline functions since we have to be able to handle
   all floating-point types.  */
# define isgreater(x, y)					\
   __extension__					\
   ({ char __result;					\
      __asm__ ("fcmp%.x %2,%1; fsogt %0"		\
	       : "=dm" (__result) : "f" (x), "f" (y));	\
      __result != 0; })

# define isgreaterequal(x, y)				\
   __extension__					\
   ({ char __result;					\
      __asm__ ("fcmp%.x %2,%1; fsoge %0"		\
	       : "=dm" (__result) : "f" (x), "f" (y));	\
      __result != 0; })

# define isless(x, y)					\
   __extension__					\
   ({ char __result;					\
      __asm__ ("fcmp%.x %2,%1; fsolt %0"		\
	       : "=dm" (__result) : "f" (x), "f" (y));	\
      __result != 0; })

# define islessequal(x, y)				\
   __extension__					\
   ({ char __result;					\
      __asm__ ("fcmp%.x %2,%1; fsole %0"		\
	       : "=dm" (__result) : "f" (x), "f" (y));	\
      __result != 0; })

# define islessgreater(x, y)				\
   __extension__					\
   ({ char __result;					\
      __asm__ ("fcmp%.x %2,%1; fsogl %0"		\
	       : "=dm" (__result) : "f" (x), "f" (y));	\
      __result != 0; })

# define isunordered(x, y)				\
   __extension__					\
   ({ char __result;					\
      __asm__ ("fcmp%.x %2,%1; fsun %0"			\
	       : "=dm" (__result) : "f" (x), "f" (y));	\
      __result != 0; })
#endif


#if (!defined __NO_MATH_INLINES && defined __OPTIMIZE__) \
    || defined __LIBC_INTERNAL_MATH_INLINES

#ifdef	__LIBC_INTERNAL_MATH_INLINES
/* This is used when defining the functions themselves.  Define them with
   __ names, and with `static inline' instead of `extern inline' so the
   bodies will always be used, never an external function call.  */
# define __m81_u(x)		__CONCAT(__,x)
# define __m81_inline		static __inline
#else
# define __m81_u(x)		x
# ifdef __cplusplus
#  define __m81_inline		__inline
# else
#  define __m81_inline		extern __inline
# endif
# define __M81_MATH_INLINES	1
#endif

/* Define a const math function.  */
#define __m81_defun(rettype, func, args)				      \
  __m81_inline rettype __attribute__((__const__))			      \
  __m81_u(func) args

/* Define the three variants of a math function that has a direct
   implementation in the m68k fpu.  FUNC is the name for C (which will be
   suffixed with f and l for the float and long double version, resp).  OP
   is the name of the fpu operation (without leading f).  */

#if defined __USE_MISC || defined __USE_ISOC99
# define __inline_mathop(func, op)			\
  __inline_mathop1(double, func, op)			\
  __inline_mathop1(float, __CONCAT(func,f), op)		\
  __inline_mathop1(long double, __CONCAT(func,l), op)
#else
# define __inline_mathop(func, op)			\
  __inline_mathop1(double, func, op)
#endif

#define __inline_mathop1(float_type,func, op)				      \
  __m81_defun (float_type, func, (float_type __mathop_x)) __THROW	      \
  {									      \
    float_type __result;						      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }

__inline_mathop(__atan, atan)
__inline_mathop(__cos, cos)
__inline_mathop(__sin, sin)
__inline_mathop(__tan, tan)
__inline_mathop(__tanh, tanh)
__inline_mathop(__fabs, abs)

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__inline_mathop(__rint, int)
__inline_mathop(__expm1, etoxm1)
__inline_mathop(__log1p, lognp1)
#endif

#ifdef __USE_MISC
__inline_mathop(__significand, getman)
#endif

#ifdef __USE_ISOC99
__inline_mathop(__trunc, intrz)
#endif

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

__inline_mathop(atan, atan)
__inline_mathop(cos, cos)
__inline_mathop(sin, sin)
__inline_mathop(tan, tan)
__inline_mathop(tanh, tanh)

# if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__inline_mathop(rint, int)
__inline_mathop(expm1, etoxm1)
__inline_mathop(log1p, lognp1)
# endif

# ifdef __USE_MISC
__inline_mathop(significand, getman)
# endif

# ifdef __USE_ISOC99
__inline_mathop(trunc, intrz)
# endif

#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */

/* This macro contains the definition for the rest of the inline
   functions, using FLOAT_TYPE as the domain type and S as the suffix
   for the function names.  */

#define __inline_functions(float_type, s)				  \
__m81_inline float_type							  \
__m81_u(__CONCAT(__frexp,s))(float_type __value, int *__expptr)	__THROW	  \
{									  \
  float_type __mantissa, __exponent;					  \
  int __iexponent;							  \
  unsigned long __fpsr;							  \
  __asm("ftst%.x %1\n"							  \
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	  \
  if (__fpsr & (7 << 24))						  \
    {									  \
      /* Not finite or zero.  */					  \
      *__expptr = 0;							  \
      return __value;							  \
    }									  \
  __asm("fgetexp%.x %1, %0" : "=f" (__exponent) : "f" (__value));	  \
  __iexponent = (int) __exponent + 1;					  \
  *__expptr = __iexponent;						  \
  __asm("fscale%.l %2, %0" : "=f" (__mantissa)				  \
	: "0" (__value), "dmi" (-__iexponent));				  \
  return __mantissa;							  \
}									  \
									  \
__m81_defun (float_type, __CONCAT(__floor,s), (float_type __x))	__THROW	  \
{									  \
  float_type __result;							  \
  unsigned long int __ctrl_reg;						  \
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));		  \
  /* Set rounding towards negative infinity.  */			  \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		  \
		      : "dmi" ((__ctrl_reg & ~0x10) | 0x20));		  \
  /* Convert X to an integer, using -Inf rounding.  */			  \
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));	  \
  /* Restore the previous rounding mode.  */				  \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		  \
		      : "dmi" (__ctrl_reg));				  \
  return __result;							  \
}									  \
									  \
__m81_defun (float_type, __CONCAT(__ceil,s), (float_type __x)) __THROW	  \
{									  \
  float_type __result;							  \
  unsigned long int __ctrl_reg;						  \
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));		  \
  /* Set rounding towards positive infinity.  */			  \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		  \
		      : "dmi" (__ctrl_reg | 0x30));			  \
  /* Convert X to an integer, using +Inf rounding.  */			  \
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));	  \
  /* Restore the previous rounding mode.  */				  \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		  \
		      : "dmi" (__ctrl_reg));				  \
  return __result;							  \
}

__inline_functions(double,)
#if defined __USE_MISC || defined __USE_ISOC99
__inline_functions(float,f)
__inline_functions(long double,l)
#endif
#undef __inline_functions

#ifdef __USE_MISC

# define __inline_functions(float_type, s)				  \
__m81_defun (int, __CONCAT(__isinf,s), (float_type __value)) __THROW	  \
{									  \
  /* There is no branch-condition for infinity,				  \
     so we must extract and examine the condition codes manually.  */	  \
  unsigned long int __fpsr;						  \
  __asm("ftst%.x %1\n"							  \
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	  \
  return (__fpsr & (2 << 24)) ? (__fpsr & (8 << 24) ? -1 : 1) : 0;	  \
}									  \
									  \
__m81_defun (int, __CONCAT(__finite,s), (float_type __value)) __THROW	  \
{									  \
  /* There is no branch-condition for infinity, so we must extract and	  \
     examine the condition codes manually.  */				  \
  unsigned long int __fpsr;						  \
  __asm ("ftst%.x %1\n"							  \
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	  \
  return (__fpsr & (3 << 24)) == 0;					  \
}									  \
									  \
__m81_defun (float_type, __CONCAT(__scalbn,s),				  \
	     (float_type __x, int __n))	__THROW				  \
{									  \
  float_type __result;							  \
  __asm ("fscale%.l %1, %0" : "=f" (__result) : "dmi" (__n), "0" (__x));  \
  return __result;							  \
}

__inline_functions(double,)
__inline_functions(float,f)
__inline_functions(long double,l)
# undef __inline_functions

#endif /* Use misc.  */

#if defined __USE_MISC || defined __USE_XOPEN

# define __inline_functions(float_type, s)				  \
__m81_defun (int, __CONCAT(__isnan,s), (float_type __value)) __THROW	  \
{									  \
  char __result;							  \
  __asm("ftst%.x %1\n"							  \
	"fsun %0" : "=dm" (__result) : "f" (__value));			  \
  return __result;							  \
}

__inline_functions(double,)
# ifdef __USE_MISC
__inline_functions(float,f)
__inline_functions(long double,l)
# endif
# undef __inline_functions

#endif

#ifdef __USE_ISOC99

# define __inline_functions(float_type, s)				  \
__m81_defun (int, __CONCAT(__signbit,s), (float_type __value)) __THROW	  \
{									  \
  /* There is no branch-condition for the sign bit, so we must extract	  \
     and examine the condition codes manually.  */			  \
  unsigned long int __fpsr;						  \
  __asm ("ftst%.x %1\n"							  \
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	  \
  return (__fpsr >> 27) & 1;						  \
}									  \
									  \
__m81_defun (float_type, __CONCAT(__scalbln,s),				  \
	     (float_type __x, long int __n)) __THROW			  \
{									  \
  return __CONCAT(__scalbn,s) (__x, __n);				  \
}									  \
									  \
__m81_defun (float_type, __CONCAT(__nearbyint,s), (float_type __x)) __THROW \
{									  \
  float_type __result;							  \
  unsigned long int __ctrl_reg;						  \
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));		  \
  /* Temporarily disable the inexact exception.  */			  \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		  \
		      : "dmi" (__ctrl_reg & ~0x200));			  \
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));	  \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		  \
		      : "dmi" (__ctrl_reg));				  \
  return __result;							  \
}									  \
									  \
__m81_defun (long int, __CONCAT(__lrint,s), (float_type __x)) __THROW	  \
{									  \
  long int __result;							  \
  __asm ("fmove%.l %1, %0" : "=dm" (__result) : "f" (__x));		  \
  return __result;							  \
}									  \
									  \
__m81_inline float_type							  \
__m81_u(__CONCAT(__fma,s))(float_type __x, float_type __y,		  \
			   float_type __z) __THROW			  \
{									  \
  return (__x * __y) + __z;						  \
}

__inline_functions (double,)
__inline_functions (float,f)
__inline_functions (long double,l)
# undef __inline_functions

#endif /* Use ISO C9x */

#ifdef __USE_GNU

# define __inline_functions(float_type, s)				\
__m81_inline void							\
__m81_u(__CONCAT(__sincos,s))(float_type __x, float_type *__sinx,	\
			      float_type *__cosx) __THROW		\
{									\
  __asm ("fsincos%.x %2,%1:%0"						\
	 : "=f" (*__sinx), "=f" (*__cosx) : "f" (__x));			\
}

__inline_functions (double,)
__inline_functions (float,f)
__inline_functions (long double,l)
# undef __inline_functions

#endif

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

/* Define inline versions of the user visible functions.  */

/* Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work correctly with -traditional.  */
# define __inline_forward_c(rettype, name, args1, args2)	\
extern __inline rettype __attribute__((__const__))	\
name args1 __THROW					\
{							\
  return __CONCAT(__,name) args2;			\
}

# define __inline_forward(rettype, name, args1, args2)	\
extern __inline rettype name args1 __THROW		\
{							\
  return __CONCAT(__,name) args2;			\
}

__inline_forward(double,frexp, (double __value, int *__expptr),
		 (__value, __expptr))
__inline_forward_c(double,floor, (double __x), (__x))
__inline_forward_c(double,ceil, (double __x), (__x))
# ifdef __USE_MISC
#  ifndef __USE_ISOC99 /* Conflict with macro of same name.  */
__inline_forward_c(int,isinf, (double __value), (__value))
#  endif
__inline_forward_c(int,finite, (double __value), (__value))
__inline_forward_c(double,scalbn, (double __x, int __n), (__x, __n))
# endif
# if defined __USE_MISC || defined __USE_XOPEN
#  ifndef __USE_ISOC99 /* Conflict with macro of same name.  */
__inline_forward_c(int,isnan, (double __value), (__value))
#  endif
# endif
# ifdef __USE_ISOC99
__inline_forward_c(double,scalbln, (double __x, long int __n), (__x, __n))
__inline_forward_c(double,nearbyint, (double __value), (__value))
__inline_forward_c(long int,lrint, (double __value), (__value))
__inline_forward_c(double,fma, (double __x, double __y, double __z),
		   (__x, __y, __z))
# endif
# ifdef __USE_GNU
__inline_forward(void,sincos, (double __x, double *__sinx, double *__cosx),
		 (__x, __sinx, __cosx))
# endif

# if defined __USE_MISC || defined __USE_ISOC99

__inline_forward(float,frexpf, (float __value, int *__expptr),
		 (__value, __expptr))
__inline_forward_c(float,floorf, (float __x), (__x))
__inline_forward_c(float,ceilf, (float __x), (__x))
#  ifdef __USE_MISC
__inline_forward_c(int,isinff, (float __value), (__value))
__inline_forward_c(int,finitef, (float __value), (__value))
__inline_forward_c(float,scalbnf, (float __x, int __n), (__x, __n))
__inline_forward_c(int,isnanf, (float __value), (__value))
#  endif
# ifdef __USE_ISOC99
__inline_forward_c(float,scalblnf, (float __x, long int __n), (__x, __n))
__inline_forward_c(float,nearbyintf, (float __value), (__value))
__inline_forward_c(long int,lrintf, (float __value), (__value))
__inline_forward_c(float,fmaf, (float __x, float __y, float __z),
		   (__x, __y, __z))
# endif
# ifdef __USE_GNU
__inline_forward(void,sincosf, (float __x, float *__sinx, float *__cosx),
		 (__x, __sinx, __cosx))
# endif

__inline_forward(long double,frexpl, (long double __value, int *__expptr),
		 (__value, __expptr))
__inline_forward_c(long double,floorl, (long double __x), (__x))
__inline_forward_c(long double,ceill, (long double __x), (__x))
# ifdef __USE_MISC
__inline_forward_c(int,isinfl, (long double __value), (__value))
__inline_forward_c(int,finitel, (long double __value), (__value))
__inline_forward_c(long double,scalbnl, (long double __x, int __n), (__x, __n))
__inline_forward_c(int,isnanl, (long double __value), (__value))
# endif
# ifdef __USE_ISOC99
__inline_forward_c(long double,scalblnl, (long double __x, long int __n),
		   (__x, __n))
__inline_forward_c(long double,nearbyintl, (long double __value), (__value))
__inline_forward_c(long int,lrintl, (long double __value), (__value))
__inline_forward_c(long double,fmal,
		   (long double __x, long double __y, long double __z),
		   (__x, __y, __z))
# endif
# ifdef __USE_GNU
__inline_forward(void,sincosl,
		 (long double __x, long double *__sinx, long double *__cosx),
		 (__x, __sinx, __cosx))
# endif

#endif /* Use misc or ISO C99 */

#undef __inline_forward
#undef __inline_forward_c

#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */

#endif
#endif	/* GCC.  */
