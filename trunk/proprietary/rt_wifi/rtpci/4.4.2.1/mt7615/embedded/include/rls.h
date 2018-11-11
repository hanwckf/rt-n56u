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
	rls.h
*/

#ifndef __RLS_H__
#define __RLS_H__

#ifdef RADIO_LINK_SELECTION

/* ioctl */
INT Show_Rls_Info(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_Rls_Period(PRTMP_ADAPTER pAd, RTMP_STRING *arg);


INT Rls_Init(PRTMP_ADAPTER pAd);
INT Rls_TableInit(PRTMP_ADAPTER pAd, PRLS_CLI_TABLE table);
INT Rls_Release(PRTMP_ADAPTER pAd);
INT Rls_TableRelease(PRTMP_ADAPTER pAd, PRLS_CLI_TABLE table);
INT Rls_InfCliLinkDown(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
INT Rls_InfCliLinkUp(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
INT Set_Rls_Enable(PRTMP_ADAPTER pAd,RTMP_STRING * arg);

INT Rls_SetInfInfo(PRTMP_ADAPTER pAd, BOOLEAN bInfReady, struct wifi_dev *wdev);
void Rls_UpdateTableChannel(PRTMP_ADAPTER pAd, INT old_channel, INT new_channel);
void Rls_UpdateDevOpMode(PRTMP_ADAPTER pAd, BOOLEAN enable, struct wifi_dev *wdev);

INT Rls_MsgHandlePro(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);

#define RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt) MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt)

#endif /* RADIO_LINK_SELECTION */
#endif /* __RLS_H__ */

