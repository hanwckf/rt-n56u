/* w_gammal.c -- long double version of w_gamma.c.
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

/* long double gammal(double x)
 * Return the Gamma function of x.
 */

#include <math.h>
#include "math_private.h"
#if defined(__UCLIBC_HAS_FENV__)
#include <errno.h>
#endif

#if !defined __NO_LONG_DOUBLE_MATH
long double
tgammal(long double x)
{
# if defined(__UCLIBC_HAS_FENV__)
	long double y = (long double) __ieee754_tgamma((long double)x);

	if(__builtin_expect (!isfinite (y) || y == 0, 0)
	   && (isfinite (x) || (isinf (x) && x < 0.0))
	   && _LIB_VERSION != _IEEE_) {
	  if(x==0.0)
	    return __kernel_standard_l(x,x,250); /* tgamma pole */
	  else if(floorl(x)==x&&x<0.0L)
	    return __kernel_standard_l(x,x,241); /* tgamma domain */
	  else if (y == 0)
	    __set_errno (ERANGE); /* tgamma underflow */
	  else
	    return __kernel_standard_l(x,x,240); /* tgamma overflow */
	}
	return y;
# else
	return (long double) __ieee754_tgamma((long double)x);
# endif /* __UCLIBC_HAS_FENV__ */
}
#endif /* __NO_LONG_DOUBLE_MATH */
