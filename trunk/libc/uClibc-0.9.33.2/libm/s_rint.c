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
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 * Exception:
 *	Inexact flag raised if x not equal to rint(x).
 */

#include "math.h"
#include "math_private.h"

static const double
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

double rint(double x)
{
	int32_t i0, j0, sx;
	u_int32_t i,i1;
	double t;
	/* We use w = x + 2^52; t = w - 2^52; trick to round x to integer.
	 * This trick requires that compiler does not optimize it
	 * by keeping intermediate result w in a register wider than double.
	 * Declaring w volatile assures that value gets truncated to double
	 * (unfortunately, it also forces store+load):
	 */
	volatile double w;

	EXTRACT_WORDS(i0,i1,x);
	/* Unbiased exponent */
	j0 = ((((u_int32_t)i0) >> 20)&0x7ff)-0x3ff;

	if (j0 > 51) {
		//Why bother? Just returning x works too
		//if (j0 == 0x400)  /* inf or NaN */
		//	return x+x;
		return x;  /* x is integral */
	}

	/* Sign */
	sx = ((u_int32_t)i0) >> 31;

	if (j0<20) {
	    if (j0<0) { /* |x| < 1 */
		if (((i0&0x7fffffff)|i1)==0) return x;
		i1 |= (i0&0x0fffff);
		i0 &= 0xfffe0000;
		i0 |= ((i1|-i1)>>12)&0x80000;
		SET_HIGH_WORD(x,i0);
		w = TWO52[sx]+x;
		t = w-TWO52[sx];
		GET_HIGH_WORD(i0,t);
		SET_HIGH_WORD(t,(i0&0x7fffffff)|(sx<<31));
		return t;
	    } else {
		i = (0x000fffff)>>j0;
		if (((i0&i)|i1)==0) return x; /* x is integral */
		i>>=1;
		if (((i0&i)|i1)!=0) {
		    if (j0==19) i1 = 0x40000000;
		    else i0 = (i0&(~i))|((0x20000)>>j0);
		}
	    }
	} else {
	    i = ((u_int32_t)(0xffffffff))>>(j0-20);
	    if ((i1&i)==0) return x;	/* x is integral */
	    i>>=1;
	    if ((i1&i)!=0) i1 = (i1&(~i))|((0x40000000)>>(j0-20));
	}
	INSERT_WORDS(x,i0,i1);
	w = TWO52[sx]+x;
	return w-TWO52[sx];
}
libm_hidden_def(rint)

strong_alias(rint, nearbyint)
libm_hidden_def(nearbyint)
