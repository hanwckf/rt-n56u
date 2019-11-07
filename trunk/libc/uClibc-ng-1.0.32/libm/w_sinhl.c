/* w_sinhl.c -- long double version of w_sinh.c.
 * Conversion to long double by Ulrich Drepper,
 * Cygnus Support, drepper@cygnus.com.
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
 * wrapper sinhl(x)
 */

#include <math.h>
#include "math_private.h"

#if !defined __NO_LONG_DOUBLE_MATH
long double
sinhl (long double x)
{
#if defined(__UCLIBC_HAS_FENV__)
	long double z = (long double) __ieee754_sinh ((double) x);
	if (__builtin_expect (!isfinite (z), 0) && isfinite (x)
	    && _LIB_VERSION != _IEEE_)
	    return __kernel_standard_l (x, x, 225); /* sinh overflow */

	return z;
# else
	return (long double) __ieee754_sinh ((double) x);
# endif /* __UCLIBC_HAS_FENV__ */
}
#endif
