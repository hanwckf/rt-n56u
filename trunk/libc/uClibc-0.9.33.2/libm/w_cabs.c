/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include <complex.h>
#include <math.h>

double cabs(double _Complex z)
{
	return hypot(__real__ z, __imag__ z);
}
libm_hidden_def(cabs)

libm_hidden_proto(cabsf)
float cabsf(float _Complex z)
{
	return (float) hypot(__real__ z, __imag__ z);
}
libm_hidden_def(cabsf)

#if defined __UCLIBC_HAS_LONG_DOUBLE_MATH__ && !defined __NO_LONG_DOUBLE_MATH
libm_hidden_proto(cabsl)
long double cabsl(long double _Complex z)
{
	return hypotl(__real__ z, __imag__ z);
}
libm_hidden_def(cabsl)
#endif
