/*******************************************************************************
*                                                                              *
*      File ceilfloor.c,                                                       *
*      Function ceil(x) and floor(x),                                          *
*      Implementation of ceil and floor for the PowerPC.                       *
*                                                                              *
*      Copyright © 1991 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on November 1991,                      *
*                                                                              *
*      based on math.h, library code for Macintoshes with a 68881/68882        *
*      by Jim Thomas.                                                          *
*                                                                              *
*      W A R N I N G:  This routine expects a 64 bit double model.             *
*                                                                              *
*      December 03 1992: first rs6000 port.                                    *
*      July     14 1993: comment changes and addition of #pragma fenv_access.  *
*	 May      06 1997: port of the ibm/taligent ceil and floor routines.     *
*	 April    11 2001: first port to os x using gcc.				 *
*	 June	    13 2001: replaced __setflm with in-line assembly			 *
*                                                                              *
*******************************************************************************/

#include <endian.h>

static const double        twoTo52  = 4503599627370496.0;
static const unsigned long signMask = 0x80000000ul;

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

/*******************************************************************************
*            Functions needed for the computation.                             *
*******************************************************************************/

/*******************************************************************************
*      Floor(x) returns the largest integer not greater than x.                *
*******************************************************************************/

double floor ( double x )
	{
	DblInHex xInHex,OldEnvironment;
	register double y;
	register unsigned long int xhi;
	register long int target;

	xInHex.dbl = x;
	xhi = xInHex.words.hi & 0x7fffffffUL;	  // xhi is the high half of |x|
	target = ( xInHex.words.hi < signMask );

	if ( xhi < 0x43300000ul )
/*******************************************************************************
*      Is |x| < 2.0^52?                                                        *
*******************************************************************************/
		{
		if ( xhi < 0x3ff00000ul )
/*******************************************************************************
*      Is |x| < 1.0?                                                           *
*******************************************************************************/
			{
			if ( ( xhi | xInHex.words.lo ) == 0ul )  // zero x is exact case
				return ( x );
			else
				{			                // inexact case
				asm ("mffs %0" : "=f" (OldEnvironment.dbl));
				OldEnvironment.words.lo |= 0x02000000ul;
				asm ("mtfsf 255,%0" : /*NULLOUT*/ : /*IN*/ "f" ( OldEnvironment.dbl ));
				if ( target )
					return ( 0.0 );
				else
					return ( -1.0 );
				}
			}
/*******************************************************************************
*      Is 1.0 < |x| < 2.0^52?                                                  *
*******************************************************************************/
		if ( target )
			{
			y = ( x + twoTo52 ) - twoTo52;          // round at binary pt.
			if ( y > x )
				return ( y - 1.0 );
			else
				return ( y );
			}

		else
			{
			y = ( x - twoTo52 ) + twoTo52;          // round at binary pt.
			if ( y > x )
				return ( y - 1.0 );
			else
				return ( y );
			}
		}
/*******************************************************************************
*      |x| >= 2.0^52 or x is a NaN.                                            *
*******************************************************************************/
	return ( x );
	}

