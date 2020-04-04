/* @(#)w_lgamma.c 5.1 93/09/24 */
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

/* double lgamma(double x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_lgamma_r
 */

#include <math.h>
#include "math_private.h"

double
lgamma(double x)
{
	return lgamma_r(x, &signgam);
}

libm_hidden_def(lgamma)

/* NB: gamma function is an old name for lgamma.
 * It is deprecated.
 * Some C math libraries redefine it as a "true gamma", i.e.,
 * not a ln(|Gamma(x)|) but just Gamma(x), but standards
 * introduced tgamma name for that.
 */
strong_alias(lgamma, gamma)
libm_hidden_def(gamma)
