/* w_lgammal_r.c -- long double version of w_lgamma_r.c.
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
 * wrapper long double lgammal_r(long double x, int *signgamp)
 */

#include <math.h>
#include "math_private.h"

#if !defined __NO_LONG_DOUBLE_MATH
libm_hidden_proto(lgammal_r)
long double
lgammal_r(long double x, int *signgamp)
{
# if defined(__UCLIBC_HAS_FENV__)
	long double y = (long double) __ieee754_lgamma_r((double)x,signgamp);
	if(__builtin_expect(!isfinite(y), 0)
	   && isfinite(x) && _LIB_VERSION != _IEEE_)
		return __kernel_standard(x, x,
					 floorl(x)==x&&x<=0.0
					 ? 215 /* lgamma pole */
					 : 214); /* lgamma overflow */

	return y;
# else
	return (long double) __ieee754_lgamma_r((double)x,signgamp);
# endif /* __UCLIBC_HAS_FENV__ */
}
libm_hidden_def(lgammal_r)
#endif /* __NO_LONG_DOUBLE_MATH */
