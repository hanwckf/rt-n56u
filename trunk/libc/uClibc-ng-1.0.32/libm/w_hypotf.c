/* w_hypotf.c -- float version of w_hypot.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
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
 * wrapper hypotf(x,y)
 */

#include <math.h>
#include "math_private.h"

float
hypotf(float x, float y)
{
#if defined(__UCLIBC_HAS_FENV__)
	float z = (float) __ieee754_hypot((double) x, (double) y);
	if(__builtin_expect(!isfinite(z), 0)
	   && isfinite(x) && isfinite(y) && _LIB_VERSION != _IEEE_)
	    /* hypot overflow */
	    return __kernel_standard_f(x, y, 104);

	return z;
#else
	return (float) __ieee754_hypot((double) x, (double) y);
#endif
}
