/* vi: set sw=4 ts=4: */
/*
 * Wrapper functions implementing all the long double math functions
 * defined by SuSv3 by actually calling the double version of
 * each function and then casting the result back to a long double
 * to return to the user.
 *
 * Copyright (C) 2005 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
/* Prevent math.h from defining colliding inlines */
#undef __USE_EXTERN_INLINES
#include "math.h"
#include <complex.h>

#define WRAPPER1(func) \
long double func##l(long double x) \
{ \
	return (long double) func((double) x); \
}
#define WRAPPER2(func) \
long double func##l(long double x, long double y) \
{ \
	return (long double) func((double) x, (double) y); \
}
#define int_WRAPPER1(func) \
int func##l(long double x) \
{ \
	return func((double) x); \
}
#define long_WRAPPER1(func) \
long func##l(long double x) \
{ \
	return func((double) x); \
}
#define long_long_WRAPPER1(func) \
long long func##l(long double x) \
{ \
	return func((double) x); \
}

/* Implement the following, as defined by SuSv3 */
#if 0
long double acoshl(long double);
long double acosl(long double);
long double asinhl(long double);
long double asinl(long double);
long double atan2l(long double, long double);
long double atanhl(long double);
long double atanl(long double);
long double cargl(long double complex);
long double cbrtl(long double);
long double ceill(long double);
long double copysignl(long double, long double);
long double coshl(long double);
long double cosl(long double);
long double erfcl(long double);
long double erfl(long double);
long double exp2l(long double);
long double expl(long double);
long double expm1l(long double);
long double fabsl(long double);
long double fdiml(long double, long double);
long double floorl(long double);
long double fmal(long double, long double, long double);
long double fmaxl(long double, long double);
long double fminl(long double, long double);
long double fmodl(long double, long double);
long double frexpl(long double value, int *);
long double hypotl(long double, long double);
int         ilogbl(long double);
long double ldexpl(long double, int);
long double lgammal(long double);
long long   llrintl(long double);
long long   llroundl(long double);
long double log10l(long double);
long double log1pl(long double);
long double log2l(long double);
long double logbl(long double);
long double logl(long double);
long        lrintl(long double);
long        lroundl(long double);
long double modfl(long double, long double *);
long double nearbyintl(long double);
long double nextafterl(long double, long double);
long double nexttowardl(long double, long double);
long double powl(long double, long double);
long double remainderl(long double, long double);
long double remquol(long double, long double, int *);
long double rintl(long double);
long double roundl(long double);
long double scalblnl(long double, long);
long double scalbnl(long double, int);
long double sinhl(long double);
long double sinl(long double);
long double sqrtl(long double);
long double tanhl(long double);
long double tanl(long double);
long double tgammal(long double);
long double truncl(long double);
#endif

#ifdef L_acoshl
WRAPPER1(acosh)
#endif

#ifdef L_acosl
WRAPPER1(acos)
#endif

#ifdef L_asinhl
WRAPPER1(asinh)
#endif

#ifdef L_asinl
WRAPPER1(asin)
#endif

#ifdef L_atan2l
WRAPPER2(atan2)
#endif

#ifdef L_atanhl
WRAPPER1(atanh)
#endif

#ifdef L_atanl
WRAPPER1(atan)
#endif

#ifdef L_cargl
long double cargl (long double complex x)
{
	return (long double) carg( (double complex)x );
}
#endif

#ifdef L_cbrtl
WRAPPER1(cbrt)
#endif

#ifdef L_ceill
WRAPPER1(ceil)
#endif

#ifdef L_copysignl
WRAPPER2(copysign)
#endif

#ifdef L_coshl
WRAPPER1(cosh)
#endif

#ifdef L_cosl
WRAPPER1(cos)
#endif

#ifdef L_erfcl
WRAPPER1(erfc)
#endif

#ifdef L_erfl
WRAPPER1(erf)
#endif

#ifdef L_exp2l
WRAPPER1(exp2)
#endif

#ifdef L_expl
WRAPPER1(exp)
#endif

#ifdef L_expm1l
WRAPPER1(expm1)
#endif

#ifdef L_fabsl
WRAPPER1(fabs)
#endif

#ifdef L_fdiml
WRAPPER2(fdim)
#endif

#ifdef L_floorl
WRAPPER1(floor)
#endif

#ifdef L_fmal
long double fmal (long double x, long double y, long double z)
{
	return (long double) fma( (double)x, (double)y, (double)z );
}
#endif

#ifdef L_fmaxl
WRAPPER2(fmax)
#endif

#ifdef L_fminl
WRAPPER2(fmin)
#endif

#ifdef L_fmodl
WRAPPER2(fmod)
#endif

#ifdef L_frexpl
long double frexpl (long double x, int *ex)
{
	return (long double) frexp( (double)x, ex );
}
#endif

#ifdef L_gammal
WRAPPER1(gamma)
#endif

#ifdef L_hypotl
WRAPPER2(hypot)
#endif

#ifdef L_ilogbl
int_WRAPPER1(ilogb)
#endif

#ifdef L_ldexpl
long double ldexpl (long double x, int ex)
{
	return (long double) ldexp( (double)x, ex );
}
#endif

#ifdef L_lgammal
WRAPPER1(lgamma)
#endif

#ifdef L_llrintl
long_long_WRAPPER1(llrint)
#endif

#ifdef L_llroundl
long_long_WRAPPER1(llround)
#endif

#ifdef L_log10l
WRAPPER1(log10)
#endif

#ifdef L_log1pl
WRAPPER1(log1p)
#endif

#ifdef L_log2l
WRAPPER1(log2)
#endif

#ifdef L_logbl
WRAPPER1(logb)
#endif

#ifdef L_logl
WRAPPER1(log)
#endif

#ifdef L_lrintl
long_WRAPPER1(lrint)
#endif

#ifdef L_lroundl
long_WRAPPER1(lround)
#endif

#ifdef L_modfl
long double modfl (long double x, long double *iptr)
{
	double y, result;
	result = modf ( x, &y );
	*iptr = (long double)y;
	return (long double) result;
}
#endif

#ifdef L_nearbyintl
WRAPPER1(nearbyint)
#endif

#ifdef L_nextafterl
WRAPPER2(nextafter)
#endif

/* Disabled in Makefile.in */
#if 0 /* def L_nexttowardl */
WRAPPER2(nexttoward)
libm_hidden_def(nexttowardl)
#endif

#ifdef L_powl
WRAPPER2(pow)
#endif

#ifdef L_remainderl
WRAPPER2(remainder)
#endif

#ifdef L_remquol
long double remquol (long double x, long double y, int *quo)
{
	return (long double) remquo( (double)x, (double)y, quo );
}
#endif

#ifdef L_rintl
WRAPPER1(rint)
#endif

#ifdef L_roundl
WRAPPER1(round)
#endif

#ifdef L_scalblnl
long double scalblnl (long double x, long ex)
{
	return (long double) scalbln( (double)x, ex );
}
#endif

#ifdef L_scalbnl
long double scalbnl (long double x, int ex)
{
	return (long double) scalbn( (double)x, ex );
}
#endif

/* scalb is an obsolete function */

#ifdef L_sinhl
WRAPPER1(sinh)
#endif

#ifdef L_sinl
WRAPPER1(sin)
#endif

#ifdef L_sqrtl
WRAPPER1(sqrt)
#endif

#ifdef L_tanhl
WRAPPER1(tanh)
#endif

#ifdef L_tanl
WRAPPER1(tan)
#endif

#ifdef L_tgammal
WRAPPER1(tgamma)
#endif

#ifdef L_truncl
WRAPPER1(trunc)
#endif

#ifdef L_significandl
WRAPPER1(significand)
#endif

#if defined __DO_C99_MATH__ && !defined __NO_LONG_DOUBLE_MATH

# ifdef L___fpclassifyl
int_WRAPPER1(__fpclassify)
libm_hidden_def(__fpclassifyl)
# endif

# ifdef L___finitel
int_WRAPPER1(__finite)
libm_hidden_def(__finitel)
# endif

# ifdef L___signbitl
int_WRAPPER1(__signbit)
libm_hidden_def(__signbitl)
# endif

# ifdef L___isnanl
int_WRAPPER1(__isnan)
libm_hidden_def(__isnanl)
# endif

# ifdef L___isinfl
int_WRAPPER1(__isinf)
libm_hidden_def(__isinfl)
# endif

#endif
