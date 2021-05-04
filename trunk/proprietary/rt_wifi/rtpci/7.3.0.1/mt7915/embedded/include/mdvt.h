/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mdvt.h
*/
#ifndef __MDVT_H__
#define __MDVT_H__

#ifdef WIFI_MODULE_DVT
INT mdvt_init(struct _RTMP_ADAPTER *ad);
VOID mdvt_exit(struct _RTMP_ADAPTER *ad);
BOOLEAN mdvt_block_command(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);

INT SetMdvtModuleParameterProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif
#endif /* __MDVT_H__ */
