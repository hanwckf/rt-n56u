/* w_lgammaf_r.c -- float version of w_lgamma_r.c.
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
 * wrapper float lgammaf_r(float x, int *signgamp)
 */

#include <math.h>
#include "math_private.h"

libm_hidden_proto(lgammaf_r)
float
lgammaf_r (float x, int *signgamp)
{
#if defined(__UCLIBC_HAS_FENV__)
	float y = (float) __ieee754_lgamma_r ((double)x,signgamp);
	if(__builtin_expect(!isfinite(y), 0)
	   && isfinite(x) && _LIB_VERSION != _IEEE_)
		return __kernel_standard_f(x, x,
					   floorf(x)==x&&x<=0.0f
					   ? 115 /* lgamma pole */
					   : 114); /* lgamma overflow */

	return y;
#else
	return (float) __ieee754_lgamma_r ((double)x,signgamp);
#endif
}
libm_hidden_def(lgammaf_r)
