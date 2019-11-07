/* w_gammaf.c -- float version of w_gamma.c.
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

#include <math.h>
#include "math_private.h"
#if defined(__UCLIBC_HAS_FENV__)
#include <errno.h>
#endif

float
tgammaf(float x)
{
#if defined(__UCLIBC_HAS_FENV__)
	float y = (float) __ieee754_tgamma((double)x);

	if(__builtin_expect (!isfinite (y) || y == 0, 0)
	   && (isfinite (x) || (isinf (x) && x < 0.0))
	   && _LIB_VERSION != _IEEE_) {
	  if (x == (float)0.0)
	    /* tgammaf pole */
	    return __kernel_standard_f(x, x, 150);
	  else if(floorf(x)==x&&x<0.0f)
	    /* tgammaf domain */
	    return __kernel_standard_f(x, x, 141);
	  else if (y == 0)
	    /* tgammaf underflow */
	    __set_errno (ERANGE);
	  else
	    /* tgammaf overflow */
	    return __kernel_standard_f(x, x, 140);
	}
	return y;
#else
	return (float) __ieee754_tgamma((double)x);
#endif /* __UCLIBC_HAS_FENV__ */
}
