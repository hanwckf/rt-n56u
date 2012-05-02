/***********************************************************************
**  File:  fpmacros.c
**
**  Contains:  C source code for implementations of floating-point
**             functions which involve float format numbers, as
**             defined in header <fp.h>.  In particular, this file
**             contains implementations of functions
**              __fpclassify(d,f), __isnormal(d,f), __isfinite(d,f),
**             __isnan(d,f), and __signbit(d,f).  This file targets
**             PowerPC platforms.
**
**  Written by:   Robert A. Murley, Ali Sazegari
**
**  Copyright:   c 2001 by Apple Computer, Inc., all rights reserved
**
**  Change History (most recent first):
**
**     07 Jul 01   ram      First created from fpfloatfunc.c, fp.c,
**				classify.c and sign.c in MathLib v3 Mac OS9.
**
***********************************************************************/

#include     <features.h>
#include     <sys/types.h>
#include     <math.h>
#include     "fp_private.h"

#define SIGN_MASK 0x80000000
#define NSIGN_MASK 0x7fffffff
#define FEXP_MASK 0x7f800000
#define FFRAC_MASK 0x007fffff

/***********************************************************************
   int __fpclassifyf(float x) returns the classification code of the
   argument x, as defined in <fp.h>.

   Exceptions:  INVALID signaled if x is a signaling NaN; in this case,
                the FP_QNAN code is returned.

   Calls:  none
***********************************************************************/

int __fpclassifyf ( float x )
{
   unsigned int iexp;

   union {
      u_int32_t lval;
      float fval;
   } z;

   z.fval = x;
   iexp = z.lval & FEXP_MASK;                 /* isolate float exponent */

   if (iexp == FEXP_MASK) {                   /* NaN or INF case */
      if ((z.lval & 0x007fffff) == 0)
         return FP_INFINITE;
	return FP_NAN;
   }

   if (iexp != 0)                             /* normal float */
      return FP_NORMAL;

   if (x == 0.0)
      return FP_ZERO;             /* zero */
   else
      return FP_SUBNORMAL;        /* must be subnormal */
}


/***********************************************************************
      Function __fpclassify,
      Implementation of classify of a double number for the PowerPC.

   Exceptions:  INVALID signaled if x is a signaling NaN; in this case,
                the FP_QNAN code is returned.

   Calls:  none
***********************************************************************/

int __fpclassify ( double arg )
{
	register unsigned int exponent;
      union
            {
            dHexParts hex;
            double dbl;
            } x;

	x.dbl = arg;

	exponent = x.hex.high & dExpMask;
	if ( exponent == dExpMask )
		{
		if ( ( ( x.hex.high & dHighMan ) | x.hex.low ) == 0 )
			return FP_INFINITE;
		else
            	return FP_NAN;
		}
	else if ( exponent != 0)
		return FP_NORMAL;
	else {
		if ( arg == 0.0 )
			return FP_ZERO;
		else
			return FP_SUBNORMAL;
		}
}


/***********************************************************************
   int __isnormalf(float x) returns nonzero if and only if x is a
   normalized float number and zero otherwise.

   Exceptions:  INVALID is raised if x is a signaling NaN; in this case,
                zero is returned.

   Calls:  none
***********************************************************************/

int __isnormalf ( float x )
{
   unsigned int iexp;
   union {
      u_int32_t lval;
      float fval;
   } z;

   z.fval = x;
   iexp = z.lval & FEXP_MASK;                 /* isolate float exponent */
   return ((iexp != FEXP_MASK) && (iexp != 0));
}


int __isnormal ( double x )
{
	return ( __fpclassify ( x ) == FP_NORMAL );
}


/***********************************************************************
   int __isfinitef(float x) returns nonzero if and only if x is a
   finite (normal, subnormal, or zero) float number and zero otherwise.

   Exceptions:  INVALID is raised if x is a signaling NaN; in this case,
                zero is returned.

   Calls:  none
***********************************************************************/

int __finitef ( float x )
{
   union {
      u_int32_t lval;
      float fval;
   } z;

   z.fval = x;
   return ((z.lval & FEXP_MASK) != FEXP_MASK);
}
weak_alias (__finitef, finitef)

int __finite ( double x )
{
	return ( __fpclassify ( x ) >= FP_ZERO );
}
weak_alias (__finite, finite)


/***********************************************************************
   int __signbitf(float x) returns nonzero if and only if the sign
   bit of x is set and zero otherwise.

   Exceptions:  INVALID is raised if x is a signaling NaN.

   Calls:  none
***********************************************************************/

int __signbitf ( float x )
{
   union {
      u_int32_t lval;
      float fval;
   } z;

   z.fval = x;
   return ((z.lval & SIGN_MASK) != 0);
}


/***********************************************************************
      Function sign of a double.
      Implementation of sign bit for the PowerPC.

   Calls:  none
***********************************************************************/

int __signbit ( double arg )
{
      union
            {
            dHexParts hex;
            double dbl;
            } x;
      int sign;

      x.dbl = arg;
      sign = ( ( x.hex.high & dSgnMask ) == dSgnMask ) ? 1 : 0;
      return sign;
}


/***********************************************************************
* int __isinff(float x) returns -1 if value represents  negative
*	infinity,  1  if value represents positive infinity,
*	and 0 otherwise.
*
* Calls:  __signbit
* +***********************************************************************/
int __isinff ( float x )
{
    int class = __fpclassifyf(x);
    if ( class == FP_INFINITE ) {
	return ( (__signbitf(x)) ? -1 : 1);
    }
    return 0;
}
weak_alias (__isinff, isinff)

int __isinf ( double x )
{
    int class = __fpclassify(x);
    if ( class == FP_INFINITE ) {
	return ( (__signbit(x)) ? -1 : 1);
    }
    return 0;
}
weak_alias (__isinf, isinf)

#if 0
int __isinfl ( long double x )
{
    int class = __fpclassify(x);
    if ( class == FP_INFINITE ) {
	return ( (__signbit(x)) ? -1 : 1);
    }
    return 0;
}
weak_alias (__isinfl, isinfl);
#endif

/***********************************************************************
   int __isnanf(float x) returns nonzero if and only if x is a
   NaN and zero otherwise.

   Exceptions:  INVALID is raised if x is a signaling NaN; in this case,
                nonzero is returned.

   Calls:  none
***********************************************************************/

int __isnanf ( float x )
{
   union {
      u_int32_t lval;
      float fval;
   } z;

   z.fval = x;
   return (((z.lval&FEXP_MASK) == FEXP_MASK) && ((z.lval&FFRAC_MASK) != 0));
}
weak_alias (__isnanf, isnanf);

int __isnan ( double x )
{
	int class = __fpclassify(x);
	return ( class == FP_NAN );
}
weak_alias (__isnan, isnan);

#if 0
int __isnanl ( long double x )
{
	int class = __fpclassify(x);
	return ( class == FP_NAN );
}
weak_alias (__isnanl, isnanl);
#endif

