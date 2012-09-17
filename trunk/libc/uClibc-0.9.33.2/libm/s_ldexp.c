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

#include "math.h"
#include "math_private.h"
#include <errno.h>

/* TODO: POSIX says:
 *
 * "If the integer expression (math_errhandling & MATH_ERRNO) is non-zero,
 * then errno shall be set to [ERANGE]. If the integer expression
 * (math_errhandling & MATH_ERREXCEPT) is non-zero, then the underflow
 * floating-point exception shall be raised."
 *
 * *And it says the same about scalbn*! Thus these two functions
 * are the same and can be just aliased.
 *
 * Currently, ldexp tries to be vaguely POSIX compliant while scalbn
 * does not (it does not set ERRNO).
 */

double ldexp(double value, int _exp)
{
	if (!isfinite(value) || value == 0.0)
		return value;
	value = scalbn(value, _exp);
	if (!isfinite(value) || value == 0.0)
		errno = ERANGE;
	return value;
}
libm_hidden_def(ldexp)
