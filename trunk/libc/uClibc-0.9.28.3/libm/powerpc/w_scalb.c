/***********************************************************************
**      File:    scalb.c
**
**      Contains: C source code for implementations of floating-point
**                scalb functions defined in header <fp.h>.  In
**                particular, this file contains implementations of
**                functions scalb and scalbl for double and long double
**                formats on PowerPC platforms.
**
**      Written by: Jon Okada, SANEitation Engineer, ext. 4-4838
**
**      Copyright: © 1992 by Apple Computer, Inc., all rights reserved
**
**      Change History ( most recent first ):
**
**      28 May 97  ali   made an speed improvement for large n,
**                       removed scalbl.
**      12 Dec 92  JPO   First created.
**
***********************************************************************/

#include <endian.h>

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

static const double twoTo1023  = 8.988465674311579539e307;   // 0x1p1023
static const double twoToM1022 = 2.225073858507201383e-308;  // 0x1p-1022


/***********************************************************************
      double  scalb( double  x, long int n ) returns its argument x scaled
      by the factor 2^m.  NaNs, signed zeros, and infinities are propagated
      by this function regardless of the value of n.

      Exceptions:  OVERFLOW/INEXACT or UNDERFLOW inexact may occur;
                         INVALID for signaling NaN inputs ( quiet NaN returned ).

      Calls:  none.
***********************************************************************/

double scalb ( double x, int n  )
      {
      DblInHex xInHex;

      xInHex.words.lo = 0UL;                     // init. low half of xInHex

      if ( n > 1023 )
            {                                   // large positive scaling
            if ( n > 2097 )                     // huge scaling
            	return ( ( x * twoTo1023 ) * twoTo1023 ) * twoTo1023;
            while ( n > 1023 )
                  {                             // scale reduction loop
                  x *= twoTo1023;               // scale x by 2^1023
                  n -= 1023;                    // reduce n by 1023
                  }
            }

      else if ( n < -1022 )
            {                                   // large negative scaling
            if ( n < -2098 )                    // huge negative scaling
            	return ( ( x * twoToM1022 ) * twoToM1022 ) * twoToM1022;
            while ( n < -1022 )
                  {                             // scale reduction loop
                  x *= twoToM1022;              // scale x by 2^( -1022 )
                  n += 1022;                    // incr n by 1022
                  }
            }

/*******************************************************************************
*      -1022 <= n <= 1023; convert n to double scale factor.                   *
*******************************************************************************/

      xInHex.words.hi = ( ( unsigned long ) ( n + 1023 ) ) << 20;
      return ( x * xInHex.dbl );
      }
