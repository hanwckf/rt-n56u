/* @(#)w_gamma.c 5.1 93/09/24 */
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

/* double gamma(double x)
 * Return  the logarithm of the Gamma function of x or the Gamma function of x,
 * depending on the library mode.
 */

#include <math.h>
#include "math_private.h"
#if defined(__UCLIBC_HAS_FENV__)
#include <errno.h>
#endif

double
tgamma(double x)
{
#if defined(__UCLIBC_HAS_FENV__)
	double y = __ieee754_tgamma(x);

	if(__builtin_expect (!isfinite (y) || y == 0, 0)
	   && (isfinite (x) || (isinf (x) && x < 0.0))
	   && _LIB_VERSION != _IEEE_) {
	  if (x == 0.0)
	    return __kernel_standard(x,x,50); /* tgamma pole */
	  else if(floor(x)==x&&x<0.0)
	    return __kernel_standard(x,x,41); /* tgamma domain */
	  else if (y == 0)
	    __set_errno (ERANGE); /* tgamma underflow */
	  else
	    return __kernel_standard(x,x,40); /* tgamma overflow */
	}
	return y;
#else
	return __ieee754_tgamma(x);
#endif
}
libm_hidden_def(tgamma);
