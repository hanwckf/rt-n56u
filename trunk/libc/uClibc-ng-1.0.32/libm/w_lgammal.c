/* w_lgammal.c -- long double version of w_lgamma.c.
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

/* long double lgammal(long double x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_lgammal_r
 */

#include <math.h>
#include "math_private.h"

#if !defined __NO_LONG_DOUBLE_MATH
long double lgammal(long double x)
{
	return lgammal_r(x, &signgam);
}

/* NB: gamma function is an old name for lgamma.
 * It is deprecated.
 * Some C math libraries redefine it as a "true gamma", i.e.,
 * not a ln(|Gamma(x)|) but just Gamma(x), but standards
 * introduced tgamma name for that.
 */
strong_alias(lgammal, gammal)
#endif
