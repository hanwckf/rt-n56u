/* s_finitef.c -- float version of s_finite.c.
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

/*
 * finitef(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

int __finitef(float x)
{
	u_int32_t ix;

	GET_FLOAT_WORD(ix, x);
	/* Finite numbers have at least one zero bit in exponent. */
	/* All other numbers will result in 0xffffffff after OR: */
	return (ix | 0x807fffff) != 0xffffffff;
}
libm_hidden_def(__finitef)
