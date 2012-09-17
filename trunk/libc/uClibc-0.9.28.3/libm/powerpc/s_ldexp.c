/*******************************************************************************
*                                                                              *
*      File frexpldexp.c,                                                      *
*      Functions frexp(x) and ldexp(x),                                        *
*      Implementation of frexp and ldexp functions for the PowerPC.            *
*                                                                              *
*      Copyright © 1991 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on January 1991,                       *
*                                                                              *
*      W A R N I N G:  This routine expects a 64 bit double model.             *
*                                                                              *
*      December03 1992: first rs6000 implementation.                           *
*      October 05 1993: added special cases for NaN and ° in frexp.            *
*      May     27 1997: improved the performance of frexp by eliminating the   *
*                       switch statement.                                      *
*	 June	   13 2001: (ram) rewrote frexp to eliminate calls to scalb and    *
*				logb.									 *
*                                                                              *
*******************************************************************************/

#include <limits.h>
#include <math.h>
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

double ldexp ( double value, int exp )
      {
      if ( exp > SHRT_MAX )
            exp = SHRT_MAX;
      else if ( exp < -SHRT_MAX )
            exp = -SHRT_MAX;
      return scalb ( value, exp  );
      }

