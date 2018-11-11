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
	band_steering.h
*/

#ifndef _BAND_STEERING_H_
#define __BAND_STEERING_H__

#ifdef BAND_STEERING

/* ioctl */
INT Show_BndStrg_List(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Show_BndStrg_Info(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Show_BndStrg_Help(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_Enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_RssiDiff(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_RssiLow(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_Age(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_DwellTime(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_SteerTimeWindow(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_MaxSteerCount(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_HoldTime(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_CheckTime(PRTMP_ADAPTER	pAd,RTMP_STRING	*arg);
INT Set_BndStrg_FrmChkFlag(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
//INT Set_BndStrg_CndChkFlag(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#ifdef BND_STRG_DBG
INT Set_BndStrg_MonitorAddr(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);
#endif /* BND_STRG_DBG */

INT BndStrg_Init(PRTMP_ADAPTER pAd);
INT BndStrg_Release(PRTMP_ADAPTER pAd);
INT BndStrg_TableInit(PRTMP_ADAPTER pAd, INT apidx);
INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table);
PBND_STRG_CLI_TABLE Get_BndStrgTable(PRTMP_ADAPTER pAd, INT apidx);

BOOLEAN BndStrg_CheckConnectionReq(
		PRTMP_ADAPTER	pAd,
		struct wifi_dev *wdev,
		PUCHAR pSrcAddr,
		UINT8 FrameType,
		PCHAR Rssi,
		BOOLEAN bAllowStaConnectInHt,
		BOOLEAN bVHTCap,
		UINT8 nss);

INT BndStrg_Tb_Enable(PBND_STRG_CLI_TABLE table, BOOLEAN enable, CHAR *IfName);
INT BndStrg_SetInfFlags(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PBND_STRG_CLI_TABLE table, BOOLEAN bInfReady);
BOOLEAN BndStrg_IsClientStay(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry);
INT BndStrg_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx);
INT Set_BndStrg_CndPriority(PRTMP_ADAPTER pAd, RTMP_STRING *arg); 
INT Set_BndStrg_Steering_Num(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
INT Set_BndStrg_LB_Assoc_Thres(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
INT Set_BndStrg_LB_Qload_Thres(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
INT Set_BndStrg_LB_Min_Rssi_Thres(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
//INT Set_BndStrg_LB_Cnd(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
//INT Set_BndStrg_LB_CndPriority(PRTMP_ADAPTER pAd, RTMP_STRING *arg); 
INT Set_BndStrg_NSS_Thres(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
INT Set_BndStrg_Sta_Poll_Period(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
INT Set_BndStrg_Daemon_State(PRTMP_ADAPTER	pAd,RTMP_STRING		*arg);
INT Set_BndStrg_BssIdx(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
void BndStrg_UpdateEntry(PRTMP_ADAPTER pAd,MAC_TABLE_ENTRY *pEntry, BOOLEAN bHTCap,  BOOLEAN bVHTCap, UINT8 nss, BOOLEAN bConnStatus);
UINT8 GetNssFromHTCapRxMCSBitmask(UINT32 RxMCSBitmask);
void BndStrgSetProfileParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer);
void BndStrgHeartBeatMonitor(PRTMP_ADAPTER	pAd);

#define IS_VALID_MAC(addr) \
	(addr[0])|(addr[1])|(addr[2])|(addr[3])|(addr[4])|(addr[5])




/* Macro */
#define IS_BND_STRG_DUAL_BAND_CLIENT(_Control_Flags) \
	((_Control_Flags & fBND_STRG_CLIENT_SUPPORT_2G) && (_Control_Flags & fBND_STRG_CLIENT_SUPPORT_5G))

#define BND_STRG_CHECK_CONNECTION_REQ(_pAd, _wdev, _SrcAddr, _FrameType, _RssiInfo, _bAllowStaConnectInHt, _bVHTCap, _nss, _pRet) \
   {	\
	   CHAR Rssi[4] = {0};	\
	   Rssi[0] = _RssiInfo.raw_rssi[0] ? ConvertToRssi(_pAd, &_RssiInfo, RSSI_IDX_0) : 0;	\
	   Rssi[1] = _RssiInfo.raw_rssi[1] ? ConvertToRssi(_pAd, &_RssiInfo, RSSI_IDX_1) : 0;	\
	   Rssi[2] = _RssiInfo.raw_rssi[2] ? ConvertToRssi(_pAd, &_RssiInfo, RSSI_IDX_2) : 0;	\
   	   Rssi[3] = _RssiInfo.raw_rssi[3] ? ConvertToRssi(_pAd, &_RssiInfo, RSSI_IDX_3) : 0;	\
	   \
	   *_pRet = BndStrg_CheckConnectionReq( _pAd, 	\
											_wdev,	\
											_SrcAddr,		\
											_FrameType,		\
											Rssi,			\
											_bAllowStaConnectInHt, \
											_bVHTCap,	\
											_nss);	\
   }

#ifdef BND_STRG_DBG
#define RED(_text)  "\033[1;31m"_text"\033[0m"
#define GRN(_text)  "\033[1;32m"_text"\033[0m"
#define YLW(_text)  "\033[1;33m"_text"\033[0m"
#define BLUE(_text) "\033[1;36m"_text"\033[0m"

#define BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt) MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt)
#else /* BND_STRG_DBG */
#define RED(_text)	 _text
#define GRN(_text) _text
#define YLW(_text) _text
#define BLUE(_text) _text

#define BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt)
#endif /* !BND_STRG_DBG */

#ifdef BND_STRG_QA
#define BND_STRG_PRINTQAMSG(_table, _Addr, _Fmt) \
{	\
	if (MAC_ADDR_EQUAL(_table->MonitorAddr, _Addr))	\
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, _Fmt); \
}
#else
#define BND_STRG_PRINTQAMSG(_Level, _Fmt)
#endif /* BND_STRG_QA */

#endif /* BAND_STEERING */
#endif /* _BAND_STEERING_H_ */

