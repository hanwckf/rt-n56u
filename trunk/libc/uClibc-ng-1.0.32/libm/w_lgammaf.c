/* w_lgammaf.c -- float version of w_lgamma.c.
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

float lgammaf(float x)
{
	return lgammaf_r(x, &signgam);
}

/* NB: gamma function is an old name for lgamma.
 * It is deprecated.
 * Some C math libraries redefine it as a "true gamma", i.e.,
 * not a ln(|Gamma(x)|) but just Gamma(x), but standards
 * introduced tgamma name for that.
 */
strong_alias(lgammaf, gammaf)