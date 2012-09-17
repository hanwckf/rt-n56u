/* vi: set sw=4 ts=4: */
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


/* For the time being, do _NOT_ implement these functions
 * that are defined by SuSv3 [because we don't need them
 * and nobody asked to include them] */
#undef L_fdimf         /*float       fdimf(float, float);*/
#undef L_fmaf          /*float       fmaf(float, float, float);*/
#undef L_fmaxf         /*float       fmaxf(float, float);*/
#undef L_fminf         /*float       fminf(float, float);*/
#undef L_nearbyintf    /*float       nearbyintf(float);*/
#undef L_nexttowardf   /*float       nexttowardf(float, long double);*/
#undef L_remquof       /*float       remquof(float, float, int *);*/
#undef L_scalblnf      /*float       scalblnf(float, long);*/
#undef L_tgammaf       /*float       tgammaf(float);*/

/* Implement the following, as defined by SuSv3 */
#if 0
float       acosf(float);
float       acoshf(float);
float       asinf(float);
float       asinhf(float);
float       atan2f(float, float);
float       atanf(float);
float       atanhf(float);
float       cargf(float complex);
float       cbrtf(float);
float       ceilf(float);
float       copysignf(float, float);
float       cosf(float);
float       coshf(float);
float       erfcf(float);
float       erff(float);
float       exp2f(float);
float       expf(float);
float       expm1f(float);
float       fabsf(float);
float       floorf(float);
float       fmodf(float, float);
float       frexpf(float value, int *);
float       hypotf(float, float);
int         ilogbf(float);
float       ldexpf(float, int);
float       lgammaf(float);
long long   llroundf(float);
float       log10f(float);
float       log1pf(float);
float       log2f(float);
float       logbf(float);
float       logf(float);
long        lroundf(float);
float       modff(float, float *);
float       powf(float, float);
float       remainderf(float, float);
float       rintf(float);
float       roundf(float);
float       scalbnf(float, int);
float       sinf(float);
float       sinhf(float);
float       sqrtf(float);
float       tanf(float);
float       tanhf(float);
#endif

#ifdef L_acosf
WRAPPER1(acos)
#endif

#ifdef L_acoshf
WRAPPER1(acosh)
#endif

#ifdef L_asinf
WRAPPER1(asin)
#endif

#ifdef L_asinhf
WRAPPER1(asinh)
#endif

#ifdef L_atan2f
float atan2f (float x, float y)
{
	return (float) atan2( (double)x, (double)y );
}
#endif

#ifdef L_atanf
WRAPPER1(atan)
#endif

#ifdef L_atanhf
WRAPPER1(atanh)
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
#endif

#ifdef L_coshf
WRAPPER1(cosh)
#endif

#ifdef L_erfcf
WRAPPER1(erfc)
#endif

#ifdef L_erff
WRAPPER1(erf)
#endif

#ifdef L_exp2f
WRAPPER1(exp2)
#endif

#ifdef L_expf
WRAPPER1(exp)
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

#ifdef L_fmodf
float fmodf (float x, float y)
{
	return (float) fmod( (double)x, (double)y );
}
#endif

#ifdef L_frexpf
float frexpf (float x, int *_exp)
{
	return (float) frexp( (double)x, _exp );
}
#endif

#ifdef L_hypotf
float hypotf (float x, float y)
{
	return (float) hypot( (double)x, (double)y );
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

#ifdef L_lgammaf
WRAPPER1(lgamma)
#endif

#ifdef L_llrintf
long_long_WRAPPER1(llrint)
#endif

#ifdef L_llroundf
long_long_WRAPPER1(llround)
#endif

#ifdef L_log10f
WRAPPER1(log10)
#endif

#ifdef L_log1pf
WRAPPER1(log1p)
#endif

#ifdef L_log2f
WRAPPER1(log2)
#endif

#ifdef L_logbf
WRAPPER1(logb)
#endif

#ifdef L_logf
WRAPPER1(log)
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
	return (float) nexttoward( (double)x, (double)y );
}
#endif

#ifdef L_powf
float powf (float x, float y)
{
	return (float) pow( (double)x, (double)y );
}
#endif

#ifdef L_remainderf
float remainderf (float x, float y)
{
	return (float) remainder( (double)x, (double)y );
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
#endif

#ifdef L_sinhf
WRAPPER1(sinh)
#endif

#ifdef L_sqrtf
WRAPPER1(sqrt)
#endif

#ifdef L_tanf
WRAPPER1(tan)
#endif

#ifdef L_tanhf
WRAPPER1(tanh)
#endif

#ifdef L_tgammaf
WRAPPER1(tgamma)
#endif

#ifdef L_truncf
WRAPPER1(trunc)
#endif

#ifdef L_fmaf
float fmaf (float x, float y, float z)
{
	return (float) fma( (double)x, (double)y, (double)z );
}
#endif

#if defined L_scalbf && defined __UCLIBC_SUSV3_LEGACY__
float scalbf (float x, float y)
{
	return (float) scalb( (double)x, (double)y );
}
#endif

#ifdef L_gammaf
WRAPPER1(gamma)
#endif

#ifdef L_significandf
WRAPPER1(significand)
#endif
