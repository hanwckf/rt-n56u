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

/* ilogb(double x)
 * return the binary exponent of x
 * ilogb(+-0) = FP_ILOGB0
 * ilogb(+-inf) = INT_MAX
 * ilogb(NaN) = FP_ILOGBNAN (no signal is raised)
 */

#include "math.h"
#include "math_private.h"

int ilogb(double x)
{
	int32_t hx,lx,ix;

	GET_HIGH_WORD(hx, x);
	hx &= 0x7fffffff;

	if (hx < 0x00100000) {
		GET_LOW_WORD(lx, x);
		if ((hx|lx)==0)  /* +-0, ilogb(0) = FP_ILOGB0 */
			return FP_ILOGB0;
		/* subnormal x */
		ix = -1043;
		if (hx != 0) {
			ix = -1022;
			lx = (hx << 11);
		}
		/* each leading zero mantissa bit makes exponent smaller */
		for (; lx > 0; lx <<= 1)
			ix--;
		return ix;
	}

	if (hx < 0x7ff00000) /* normal x */
		return (hx>>20) - 1023;

	if (FP_ILOGBNAN != (~0U >> 1)) {
		GET_LOW_WORD(lx, x);
		if (hx == 0x7ff00000 && lx == 0)  /* +-inf */
			return ~0U >> 1; /* = INT_MAX */
	}

	/* NAN. ilogb(NAN) = FP_ILOGBNAN */
	return FP_ILOGBNAN;
}
libm_hidden_def(ilogb)
