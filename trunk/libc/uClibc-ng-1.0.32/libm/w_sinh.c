/* @(#)w_sinh.c 5.1 93/09/24 */
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
 * wrapper sinh(x)
 */

#include <math.h>
#include "math_private.h"

double
sinh (double x)
{
#if defined(__UCLIBC_HAS_FENV__)
	double z = __ieee754_sinh (x);
	if (__builtin_expect (!isfinite (z), 0) && isfinite (x)
	    && _LIB_VERSION != _IEEE_)
		return __kernel_standard (x, x, 25); /* sinh overflow */

	return z;
#else
	return __ieee754_sinh (x);
#endif
}
libm_hidden_def(sinh)
