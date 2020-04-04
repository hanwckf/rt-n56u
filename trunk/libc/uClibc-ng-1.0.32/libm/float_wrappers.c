/*
 * Wrapper functions implementing all the float math functions
 * defined by SuSv3 by actually calling the double version of
 * each function and then casting the result back to a float
 * to return to the user.
 *
 * Copyright (C) 2005 by Erik Andersen <andersen@uclibc.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#include <features.h>
/* Prevent math.h from defining colliding inlines */
#undef __USE_EXTERN_INLINES
#include <math.h>
#include <complex.h>


#define WRAPPER1(func) \
float func##f (float x) \
{ \
	return (float) func((double)x); \
}
#define int_WRAPPER1(func) \
int func##f (float x) \
{ \
	return func((double)x); \
}
#define long_WRAPPER1(func) \
long func##f (float x) \
{ \
	return func((double)x); \
}
#define long_long_WRAPPER1(func) \
long long func##f (float x) \
{ \
	return func((double)x); \
}

/* Implement the following, as defined by SuSv3 */
#if 0
float       asinhf(float);
float       atanf(float);
float       cargf(float complex);
float       cbrtf(float);
float       ceilf(float);
float       copysignf(float, float);
float       cosf(float);
float       erfcf(float);
float       erff(float);
float       expm1f(float);
float       fabsf(float);
float       floorf(float);
float       frexpf(float value, int *);
int         ilogbf(float);
float       ldexpf(float, int);
long long   llroundf(float);
float       log1pf(float);
float       logbf(float);
long        lroundf(float);
float       modff(float, float *);
float       rintf(float);
float       roundf(float);
float       scalbnf(float, int);
float       sinf(float);
float       tanf(float);
float       tanhf(float);
#endif

/* The following functions implemented as wrappers
 * in separate files (w_funcf.c)
 */
#if 0
float       acosf(float);
float       acoshf(float);
float       asinf(float);
float       atan2f(float, float);
float       atanhf(float);
float       coshf(float);
float       exp2f(float);
float       expf(float);
float       fmodf(float, float);
float       hypotf(float, float);
float       lgammaf(float);
float       log10f(float);
float       log2f(float);
float       logf(float);
float       powf(float, float);
float       remainderf(float, float);
float       sinhf(float);
float       sqrtf(float);
float		j0f(float x);
float		j1f(float x);
float		jnf(int n, float x);
float		y0f(float x);
float		y1f(float x);
float		ynf(int n, float x);
float 		tgammaf(float x);
float 		scalbf(float x, float fn);
float 		gammaf(float x);
float	 	scalbl(float x, float fn);
#endif

#ifdef L_asinhf
WRAPPER1(asinh)
#endif

#ifdef L_atanf
WRAPPER1(atan)
#endif

#ifdef L_cargf
float cargf (float complex x)
{
	return (float) carg( (double complex)x );
}
#endif

#ifdef L_cbrtf
WRAPPER1(cbrt)
#endif

#ifdef L_ceilf
WRAPPER1(ceil)
#endif

#ifdef L_copysignf
float copysignf (float x, float y)
{
	return (float) copysign( (double)x, (double)y );
}
#endif

#ifdef L_cosf
WRAPPER1(cos)
libm_hidden_def(cosf)
#endif

#ifdef L_erfcf
WRAPPER1(erfc)
#endif

#ifdef L_erff
WRAPPER1(erf)
#endif

#ifdef L_expm1f
WRAPPER1(expm1)
#endif

#ifdef L_fabsf
WRAPPER1(fabs)
#endif

#ifdef L_fdimf
float fdimf (float x, float y)
{
	return (float) fdim( (double)x, (double)y );
}
#endif

#ifdef L_floorf
WRAPPER1(floor)
#endif

#ifdef L_fmaf
float fmaf (float x, float y, float z)
{
	return (float) fma( (double)x, (double)y, (double)z );
}
#endif

#ifdef L_fmaxf
float fmaxf (float x, float y)
{
	return (float) fmax( (double)x, (double)y );
}
#endif

#ifdef L_fminf
float fminf (float x, float y)
{
	return (float) fmin( (double)x, (double)y );
}
#endif

#ifdef L_frexpf
float frexpf (float x, int *_exp)
{
	return (float) frexp( (double)x, _exp );
}
#endif

#ifdef L_ilogbf
int_WRAPPER1(ilogb)
#endif

#ifdef L_ldexpf
float ldexpf (float x, int _exp)
{
	return (float) ldexp( (double)x, _exp );
}
#endif

#ifdef L_llrintf
long_long_WRAPPER1(llrint)
#endif

#ifdef L_llroundf
long_long_WRAPPER1(llround)
#endif

#ifdef L_log1pf
WRAPPER1(log1p)
#endif

#ifdef L_logbf
WRAPPER1(logb)
#endif

#ifdef L_lrintf
long_WRAPPER1(lrint)
#endif

#ifdef L_lroundf
long_WRAPPER1(lround)
#endif

#ifdef L_modff
float modff (float x, float *iptr)
{
	double y, result;
	result = modf( x, &y );
	*iptr = (float)y;
	return (float) result;
}
#endif

#ifdef L_nearbyintf
WRAPPER1(nearbyint)
#endif

#ifdef L_nexttowardf
float nexttowardf (float x, long double y)
{
	return (float) nexttoward( (double)x, (long double)y );
}
#endif

#ifdef L_remquof
float remquof (float x, float y, int *quo)
{
	return (float) remquo( (double)x, (double)y, quo );
}
#endif

#ifdef L_rintf
WRAPPER1(rint)
#endif

#ifdef L_roundf
WRAPPER1(round)
#endif

#ifdef L_scalblnf
float scalblnf (float x, long _exp)
{
	return (float) scalbln( (double)x, _exp );
}
#endif

#ifdef L_scalbnf
float scalbnf (float x, int _exp)
{
	return (float) scalbn( (double)x, _exp );
}
#endif

#ifdef L_sinf
WRAPPER1(sin)
libm_hidden_def(sinf)
#endif

#ifdef L_tanf
WRAPPER1(tan)
#endif

#ifdef L_tanhf
WRAPPER1(tanh)
#endif

#ifdef L_truncf
WRAPPER1(trunc)
#endif

#ifdef L_significandf
WRAPPER1(significand)
#endif
