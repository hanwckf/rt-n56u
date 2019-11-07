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
 * finite(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include <features.h>
/* Prevent math.h from defining a colliding inline */
#undef __USE_EXTERN_INLINES
#include "math.h"
#include "math_private.h"

int __finite(double x)
{
	u_int32_t hx;

	GET_HIGH_WORD(hx, x);
	/* Finite numbers have at least one zero bit in exponent. */
	/* All other numbers will result in 0xffffffff after OR: */
	return (hx | 0x800fffff) != 0xffffffff;
}
libm_hidden_def(__finite)
