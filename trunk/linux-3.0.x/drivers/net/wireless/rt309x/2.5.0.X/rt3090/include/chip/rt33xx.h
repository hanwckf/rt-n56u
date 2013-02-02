/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt30xx.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT33XX_H__
#define __RT33XX_H__

#ifdef RT33xx

#include "rtmp_type.h"

#ifdef RT33xx
extern REG_PAIR RF3320_RFRegTable[];
extern UCHAR NUM_RF_3320_REG_PARMS;
#endif // RT3390 //




#ifdef RT3390
#define BW20RFR24	0x48
#define BW40RFR24	0X68
#define BW20RFR31	0x4F
#define BW40RFR31	0X6F
#endif // RT3390 //

#endif // RT33xx //

#endif //__RT33XX_H__ //

