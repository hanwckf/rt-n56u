/*******************************************************************************
*                                                                              *
*      File sign.c,                                                            *
*      Functions copysign and __signbitd.					             *
*      For PowerPC based machines.                                             *
*                                                                              *
*      Copyright © 1991, 2001 Apple Computer, Inc.  All rights reserved.       *
*                                                                              *
*      Written by Ali Sazegari, started on June 1991.                          *
*                                                                              *
*      August    26 1991: no CFront Version 1.1d17 warnings.                   *
*      September 06 1991: passes the test suite with invalid raised on         *
*                         signaling nans.  sane rom code behaves the same.     *
*      September 24 1992: took the “#include support.h” out.                   *
*      Dcember   02 1992: PowerPC port.                                        *
*      July      20 1994: __fabs added                                         *
*      July      21 1994: deleted unnecessary functions: neg, COPYSIGNnew,     *
*                         and SIGNNUMnew.                                      *
*	 April     11 2001: first port to os x using gcc.				 *
*				  removed fabs and deffered to gcc for direct          *
*				  instruction generation.					 *
*                                                                              *
*******************************************************************************/

#include "../fp_private.h"

/*******************************************************************************
*                                                                              *
*     Function copysign.                                                       *
*     Implementation of copysign for the PowerPC.                              *
*                                                                              *
********************************************************************************
*     Note: The order of the operands in this function is reversed from that   *
*     suggested in the IEEE standard 754.                                      *
*******************************************************************************/

double copysign ( double arg2, double arg1 )
      {
      union
            {
            dHexParts hex;
            double dbl;
            } x, y;

/*******************************************************************************
*     No need to flush NaNs out.                                               *
*******************************************************************************/

      x.dbl = arg1;
      y.dbl = arg2;

      y.hex.high = y.hex.high & 0x7FFFFFFF;
      y.hex.high = ( y.hex.high | ( x.hex.high & dSgnMask ) );

      return y.dbl;
      }
