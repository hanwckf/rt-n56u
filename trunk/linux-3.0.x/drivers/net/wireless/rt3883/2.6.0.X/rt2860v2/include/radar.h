/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    radar.h

    Abstract:
     CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#ifndef __RADAR_H__
#define __RADAR_H__

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
INT	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

VOID CckMrcStatusCtrl(
	IN PRTMP_ADAPTER pAd);

VOID RadarGLRTCompensate(
	IN PRTMP_ADAPTER pAd);

#endif /*defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)*/

#endif /* __RADAR_H__ */
