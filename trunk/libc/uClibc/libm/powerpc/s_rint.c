/*******************************************************************************
**      File:   rndint.c
**
**      Contains: C source code for implementations of floating-point
**                functions which round to integral value or format, as
**                defined in header <fp.h>.  In particular, this file
**                contains implementations of functions rint, nearbyint,
**                rinttol, round, roundtol, trunc, modf and modfl.  This file
**                targets PowerPC or Power platforms.
**
**      Written by: A. Sazegari, Apple AltiVec Group
**	    Created originally by Jon Okada, Apple Numerics Group
**
**      Copyright: © 1992-2001 by Apple Computer, Inc., all rights reserved
**
**      Change History (most recent first):
**
**      13 Jul 01  ram  replaced --setflm calls with inline assembly
**      03 Mar 01  ali	first port to os x using gcc, added the crucial __setflm
**                      definition.
**				1. removed double_t, put in double for now.
**				2. removed iclass from nearbyint.
**				3. removed wrong comments intrunc.
**				4.
**      13 May 97  ali  made performance improvements in rint, rinttol, roundtol
**                      and trunc by folding some of the taligent ideas into this
**                      implementation.  nearbyint is faster than the one in taligent,
**                      rint is more elegant, but slower by %30 than the taligent one.
**      09 Apr 97  ali  deleted modfl and deferred to AuxiliaryDD.c
**      15 Sep 94  ali  Major overhaul and performance improvements of all functions.
**      20 Jul 94  PAF  New faster version
**      16 Jul 93  ali  Added the modfl function.
**      18 Feb 93  ali  Changed the return value of fenv functions
**                      feclearexcept and feraiseexcept to their new
**                      NCEG X3J11.1/93-001 definitions.
**      16 Dec 92  JPO  Removed __itrunc implementation to a
**                      separate file.
**      15 Dec 92  JPO  Added __itrunc implementation and modified
**                      rinttol to include conversion from double
**                      to long int format.  Modified roundtol to
**                      call __itrunc.
**      10 Dec 92  JPO  Added modf (double) implementation.
**      04 Dec 92  JPO  First created.
**
*******************************************************************************/

#include <limits.h>
#include <math.h>
#include <endian.h>

#define      SET_INVALID      0x01000000UL

typedef union
      {
      struct {
#if (__BYTE_ORDER == __BIG_ENDIAN)
        unsigned long int hi;
        unsigned long int lo;
#else
        unsigned long int lo;
        unsigned long int hi;
#endif
      } words;
      double dbl;
      } DblInHex;

static const unsigned long int signMask = 0x80000000ul;
static const double twoTo52      = 4503599627370496.0;
static const double doubleToLong = 4503603922337792.0;	            // 2^52
static const DblInHex Huge       = {{ 0x7FF00000, 0x00000000 }};
static const DblInHex TOWARDZERO = {{ 0x00000000, 0x00000001 }};

/*******************************************************************************
*                                                                              *
*     The function rint rounds its double argument to integral value           *
*     according to the current rounding direction and returns the result in    *
*     double format.  This function signals inexact if an ordered return       *
*     value is not equal to the operand.                                       *
*                                                                              *
********************************************************************************
*                                                                              *
*     This function calls:  fabs. 	                                           *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*     First, an elegant implementation.                                        *
********************************************************************************
*
*double rint ( double x )
*      {
*      double y;
*
*      y = twoTo52.fval;
*
*      if ( fabs ( x ) >= y )                          // huge case is exact
*            return x;
*      if ( x < 0 ) y = -y;                            // negative case
*      y = ( x + y ) - y;                              // force rounding
*      if ( y == 0.0 )                                 // zero results mirror sign of x
*            y = copysign ( y, x );
*      return ( y );
*      }
********************************************************************************
*     Now a bit twidling version that is about %30 faster.                     *
*******************************************************************************/

double rint ( double x )
      {
      DblInHex argument;
      register double y;
      unsigned long int xHead;
      register long int target;

      argument.dbl = x;
      xHead = argument.words.hi & 0x7fffffffUL;          // xHead <- high half of |x|
      target = ( argument.words.hi < signMask );         // flags positive sign

      if ( xHead < 0x43300000ul )
/*******************************************************************************
*     Is |x| < 2.0^52?                                                         *
*******************************************************************************/
            {
            if ( xHead < 0x3ff00000ul )
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
                  {
                  if ( target )
                        y = ( x + twoTo52 ) - twoTo52;  // round at binary point
                  else
                        y = ( x - twoTo52 ) + twoTo52;  // round at binary point
                  if ( y == 0.0 )
                        {                               // fix sign of zero result
                        if ( target )
                              return ( 0.0 );
                        else
                              return ( -0.0 );
                        }
                  return y;
                  }

/*******************************************************************************
*     Is 1.0 < |x| < 2.0^52?                                                   *
*******************************************************************************/

            if ( target )
                  return ( ( x + twoTo52 ) - twoTo52 ); //   round at binary pt.
            else
                  return ( ( x - twoTo52 ) + twoTo52 );
            }

/*******************************************************************************
*     |x| >= 2.0^52 or x is a NaN.                                             *
*******************************************************************************/
      return ( x );
      }

