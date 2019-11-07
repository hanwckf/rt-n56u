/* w_acoshl.c -- long double version of w_acosh.c.
 * Conversion to long double by Ulrich Drepper,
 * Cygnus Support, drepper@cygnus.com.
 * Optimizations bu Ulrich Drepper <drepper@gmail.com>, 2011.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * wrapper coshl(x)
 */

#include <math.h>
#include "math_private.h"

#if !defined __NO_LONG_DOUBLE_MATH
long double
coshl (long double x)
{
#if defined(__UCLIBC_HAS_FENV__)
	long double z = (long double) __ieee754_cosh ((double) x);
	if (__builtin_expect (!isfinite (z), 0) && isfinite (x)
	    && _LIB_VERSION != _IEEE_)
		return __kernel_standard_l (x, x, 205); /* cosh overflow */

	return z;
#else
	return (long double) __ieee754_cosh ((double) x);
#endif /* __UCLIBC_HAS_FENV__ */
}
#endif /* __NO_LONG_DOUBLE_MATH */
