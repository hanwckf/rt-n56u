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

static const double two54 =  1.80143985094819840000e+16; /* 0x43500000, 0x00000000 */

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

double frexp ( double value, int *eptr )
      {
      DblInHex argument;
      unsigned long int valueHead;

      argument.dbl = value;
      valueHead = argument.words.hi & 0x7fffffffUL; // valueHead <- |x|

      *eptr = 0;
	if ( valueHead >= 0x7ff00000 || ( valueHead | argument.words.lo ) == 0 )
		return value;		// 0, inf, or NaN

	if ( valueHead < 0x00100000 )
		{	// denorm
		argument.dbl = two54 * value;
		valueHead = argument.words.hi &0x7fffffff;
		*eptr = -54;
		}
	*eptr += ( valueHead >> 20 ) - 1022;
	argument.words.hi = ( argument.words.hi & 0x800fffff ) | 0x3fe00000;
	return argument.dbl;
	}

