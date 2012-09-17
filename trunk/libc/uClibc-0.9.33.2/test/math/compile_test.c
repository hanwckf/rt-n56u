#include <math.h>

static int testf(float float_x, long double long_double_x, /*float complex float_complex_x,*/ int int_x, long long_x)
{
int r = 0;
r += acosf(float_x);
r += acoshf(float_x);
r += asinf(float_x);
r += asinhf(float_x);
r += atan2f(float_x, float_x);
r += atanf(float_x);
r += atanhf(float_x);
/*r += cargf(float_complex_x); - will fight with complex numbers later */
r += cbrtf(float_x);
r += ceilf(float_x);
r += copysignf(float_x, float_x);
r += cosf(float_x);
r += coshf(float_x);
r += erfcf(float_x);
r += erff(float_x);
r += exp2f(float_x);
r += expf(float_x);
r += expm1f(float_x);
r += fabsf(float_x);
/*r += fdimf(float_x, float_x); - uclibc does not have it (yet?) */
r += floorf(float_x);
/*r += fmaf(float_x, float_x, float_x); - uclibc does not have it (yet?) */
/*r += fmaxf(float_x, float_x); - uclibc does not have it (yet?) */
/*r += fminf(float_x, float_x); - uclibc does not have it (yet?) */
r += fmodf(float_x, float_x);
r += frexpf(float_x, &int_x);
r += gammaf(float_x);
r += hypotf(float_x, float_x);
r += ilogbf(float_x);
r += ldexpf(float_x, int_x);
r += lgammaf(float_x);
r += llrintf(float_x);
r += llroundf(float_x);
r += log10f(float_x);
r += log1pf(float_x);
r += log2f(float_x);
r += logbf(float_x);
r += logf(float_x);
r += lrintf(float_x);
r += lroundf(float_x);
r += modff(float_x, &float_x);
/*r += nearbyintf(float_x); - uclibc does not have it (yet?) */
/*r += nexttowardf(float_x, long_double_x); - uclibc does not have it (yet?) */
r += powf(float_x, float_x);
r += remainderf(float_x, float_x);
/*r += remquof(float_x, float_x, &int_x); - uclibc does not have it (yet?) */
r += rintf(float_x);
r += roundf(float_x);
#ifdef __UCLIBC_SUSV3_LEGACY__
r += scalbf(float_x, float_x);
#endif
/*r += scalblnf(float_x, long_x); - uclibc does not have it (yet?) */
r += scalbnf(float_x, int_x);
r += significandf(float_x);
r += sinf(float_x);
r += sinhf(float_x);
r += sqrtf(float_x);
r += tanf(float_x);
r += tanhf(float_x);
/*r += tgammaf(float_x); - uclibc does not have it (yet?) */
r += truncf(float_x);
return r;
}

static int testl(long double long_double_x, int int_x, long long_x)
{
int r = 0;
r += __finitel(long_double_x);
r += __fpclassifyl(long_double_x);
r += __isinfl(long_double_x);
r += __isnanl(long_double_x);
r += __signbitl(long_double_x);
r += acoshl(long_double_x);
r += acosl(long_double_x);
r += asinhl(long_double_x);
r += asinl(long_double_x);
r += atan2l(long_double_x, long_double_x);
r += atanhl(long_double_x);
r += atanl(long_double_x);
r += cbrtl(long_double_x);
r += ceill(long_double_x);
r += copysignl(long_double_x, long_double_x);
r += coshl(long_double_x);
r += cosl(long_double_x);
r += erfcl(long_double_x);
r += erfl(long_double_x);
r += exp2l(long_double_x);
r += expl(long_double_x);
r += expm1l(long_double_x);
r += fabsl(long_double_x);
r += fdiml(long_double_x, long_double_x);
r += floorl(long_double_x);
r += fmal(long_double_x, long_double_x, long_double_x);
r += fmaxl(long_double_x, long_double_x);
r += fminl(long_double_x, long_double_x);
r += fmodl(long_double_x, long_double_x);
r += frexpl(long_double_x, &int_x);
r += hypotl(long_double_x, long_double_x);
r += ilogbl(long_double_x);
r += ldexpl(long_double_x, int_x);
r += lgammal(long_double_x);
r += llrintl(long_double_x);
r += llroundl(long_double_x);
r += log10l(long_double_x);
r += log1pl(long_double_x);
r += log2l(long_double_x);
r += logbl(long_double_x);
r += logl(long_double_x);
r += lrintl(long_double_x);
r += lroundl(long_double_x);
r += modfl(long_double_x, &long_double_x);
r += nearbyintl(long_double_x);
r += nextafterl(long_double_x, long_double_x);
/* r += nexttowardl(long_double_x, long_double_x); - uclibc doesn't provide this [yet?] */
r += powl(long_double_x, long_double_x);
r += remainderl(long_double_x, long_double_x);
r += remquol(long_double_x, long_double_x, &int_x);
r += rintl(long_double_x);
r += roundl(long_double_x);
r += scalblnl(long_double_x, long_x);
r += scalbnl(long_double_x, int_x);
r += sinhl(long_double_x);
r += sinl(long_double_x);
r += sqrtl(long_double_x);
r += tanhl(long_double_x);
r += tanl(long_double_x);
r += tgammal(long_double_x);
r += truncl(long_double_x);
return r;
}

int main(int argc, char **argv)
{
        /* Always 0 but gcc hopefully won't be able to notice */
        return 5 & ((long)&testf) & ((long)&testl) & 2;
}
