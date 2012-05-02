/*******************************************************************************
*                                                                              *
*      File logb.c,                                                            *
*      Functions logb.                                                         *
*      Implementation of logb for the PowerPC.                                 *
*                                                                              *
*      Copyright © 1991 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on June 1991,                          *
*                                                                              *
*      August   26 1991: removed CFront Version 1.1d17 warnings.               *
*      August   27 1991: no errors reported by the test suite.                 *
*      November 11 1991: changed CLASSEXTENDED to the macro CLASSIFY and       *
*                        + or - infinity to constants.                         *
*      November 18 1991: changed the macro CLASSIFY to CLASSEXTENDEDint to     *
*                        improve performance.                                  *
*      February 07 1992: changed bit operations to macros (  object size is    *
*                        unchanged  ).                                         *
*      September24 1992: took the "#include support.h" out.                    *
*      December 03 1992: first rs/6000 port.                                   *
*      August   30 1992: set the divide by zero for the zero argument case.    *
*      October  05 1993: corrected the environment.                            *
*      October  17 1994: replaced all environmental functions with __setflm.   *
*      May      28 1997: made speed improvements.                              *
*      April    30 2001: forst mac os x port using gcc.                        *
*                                                                              *
********************************************************************************
*     The C math library offers a similar function called "frexp".  It is      *
*     different in details from logb, but similar in spirit.  This current     *
*     implementation of logb follows the recommendation in IEEE Standard 854   *
*     which is different in its handling of denormalized numbers from the IEEE *
*     Standard 754.                                                            *
*******************************************************************************/

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

static const double twoTo52 = 4.50359962737049600e15;              // 0x1p52
static const double klTod = 4503601774854144.0;                    // 0x1.000008p52
static const unsigned long int signMask = 0x80000000ul;
static const DblInHex minusInf  = {{ 0xFFF00000, 0x00000000 }};


/*******************************************************************************
********************************************************************************
*                                    L  O  G  B                                *
********************************************************************************
*******************************************************************************/

double logb (  double x  )
      {
      DblInHex xInHex;
      long int shiftedExp;

      xInHex.dbl = x;
      shiftedExp = ( xInHex.words.hi & 0x7ff00000UL ) >> 20;

      if ( shiftedExp == 2047 )
            {                                            // NaN or INF
            if ( ( ( xInHex.words.hi & signMask ) == 0 ) || ( x != x ) )
                  return x;                              // NaN or +INF return x
            else
                  return -x;                             // -INF returns +INF
            }

      if ( shiftedExp != 0 )                             // normal number
            shiftedExp -= 1023;                          // unbias exponent

      else if ( x == 0.0 )
            {                                            // zero
            xInHex.words.hi = 0x0UL;                      // return -infinity
            return (  minusInf.dbl  );
            }

      else
            {                                            // subnormal number
            xInHex.dbl *= twoTo52;                       // scale up
            shiftedExp = ( xInHex.words.hi & 0x7ff00000UL ) >> 20;
            shiftedExp -= 1075;                          // unbias exponent
            }

      if ( shiftedExp == 0 )                             // zero result
            return ( 0.0 );

      else
            {                                            // nonzero result
            xInHex.dbl = klTod;
            xInHex.words.lo += shiftedExp;
            return ( xInHex.dbl - klTod );
            }
      }

