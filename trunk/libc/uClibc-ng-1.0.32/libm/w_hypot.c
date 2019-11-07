/* @(#)w_hypot.c 5.1 93/09/24 */
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
 * wrapper hypot(x,y)
 */

#include <math.h>
#include "math_private.h"

double
hypot (double x, double y)
{
#if defined(__UCLIBC_HAS_FENV__)
	double z = __ieee754_hypot(x,y);
	if(__builtin_expect(!isfinite(z), 0)
	   && isfinite(x) && isfinite(y) && _LIB_VERSION != _IEEE_)
	    return __kernel_standard(x, y, 4); /* hypot overflow */

	return z;
#else
	return __ieee754_hypot(x,y);
#endif
}
libm_hidden_def(hypot)
