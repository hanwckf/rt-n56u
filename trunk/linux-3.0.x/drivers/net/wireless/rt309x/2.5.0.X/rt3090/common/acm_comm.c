/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All related WMM ACM common (AP/STA) function body.

	History:
		1. 2009/08/26	Sample Lin
			(1) Move ACM_APSD_Ctrl() to ACM_TC_Destroy()
			(2) Do not delete TSPEC if DELTS is not sent to STA when STA
				is in Power Save mode, i.e. UAPSD mode.
			(3) Duplicate action frames to legacy PS queue
				in AP_QueuePsActionPacket if UAPSD of VO is enabled,
				not all ACs are UAPSD mode and STA is in PS mode.
		2. 2009/09/29	Sample Lin
			(1) Improve WMM ACM TCLAS add command in acm_iocl.c.
			(2) No need check timeout if timeout function is disabled in
				ACMP_DataNullHandle().
			(3) No need TCLAS UP check in ACMP_DataNullHandle().
		3. 2009/11/09	Sample Lin
			(1) Add some TODO notes in the fucture.
		4. 2009/12/15	Sample Lin
			(1) Fix bugs in CPU 64bit. Can not use (UINT32)pAd.
		5. 2009/12/22	Sample Lin
			(1) Fix bugs for big-endian mode in acm_extr.h.
		6. 2010/01/07	Sample Lin
			(1) Add function: ACL list. (admit control list)
				Only accept ACM request in the list.
			(2) Add function: Aggregation (AMSDU)
			(3) Fix bug: LenDataId --
				No need to do "LenDataId --" when getting LenDataId.
				EX: Data Size = 64, LenDataId will be 64 >> 5 = 2
				So the tx time of 64B will be same as the tx time of 95B.
				(64+32-1 = 95B)
		7. 2010/01/25	Sample Lin
			(1) Fix bug: Minimum physical rate infinite loop when rate is
				not one of supported rates.
		8. 2010/02/20	Sample Lin
			(1) Fix bug: Minimum physical rate of TSPEC must meet support
				rate list, but the rule is error for 5.5Mbps.
			(2) Fix bug: Bandwidth check for bi-directional TSPEC replacement.
				The problem will occur when the available ACM capacity is less.
			(3) Fix bug: Only station mode, update QBSS Load from AP.
				The problem will only influence the OBSS load element in beacon.

***************************************************************************/




#include "rt_config.h"

//#define PERFORMANCE_IMPACT_TEST /* only for test */
//#define WMM_ACM_FUNC_DEBUG /* only for function hang debug */
//#define WMM_ACM_PKT_NUM_DEBUG /* only for ACM packet number debug */

#ifdef WMM_ACM_SUPPORT

/* IEEE802.11E related include files */
#include "acm_extr.h" /* used for other modules */
#include "acm_comm.h" /* used for edca/wmm */
#include "acm_edca.h" /* used for edca/wmm */


#ifdef WMM_ACM_FUNC_DEBUG
#define WMM_ACM_FUNC_NAME_PRINT(__pMsg)	\
	ACMR_DEBUG(ACMR_DEBUG_ERR, ("acm_func> %s: %s\n", __FUNCTION__, __pMsg));

#else
#define WMM_ACM_FUNC_NAME_PRINT(__pMsg)
#endif // WMM_ACM_FUNC_DEBUG //

/* ----- Extern Variable ----- */
/* other WLAN modules */
extern VOID BA_MaxWinSizeReasign(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY  *pEntryPeer,
	OUT UCHAR			*pWinSize);


/* from EDCA module (acm_edca.c) */
extern UCHAR gEDCA_UP_AC[];		/* EDCA Priority vs. AC */
extern UCHAR gEDCA_AC_UP[];		/* EDCA AC vs. Priority */
extern UCHAR gEDCA_UP_DSCP[];	/* DSCP vs. Priority */


/* ----- Private Variable ----- */

/* TCLAS related */
UCHAR gTCLAS_Elm_Len[3] = {	ACM_TCLAS_TYPE_WME_ETHERNET_LEN,
							ACM_TCLAS_TYPE_WME_IP_V4_LEN,
							ACM_TCLAS_TYPE_WME_8021DQ_LEN };

UCHAR gAcmTestFlag = 0; /* used for self-test */

static const UCHAR gAcmRateLegacy[4] =
	{	ACM_RATE_11M, ACM_RATE_5_5M, ACM_RATE_2M, ACM_RATE_1M};
static const UCHAR gAcmRateG[8] =
	{	ACM_RATE_54M, ACM_RATE_48M, ACM_RATE_36M, ACM_RATE_24M,
		ACM_RATE_18M, ACM_RATE_12M, ACM_RATE_9M, ACM_RATE_6M};

/* cck & ofdm MCS */
#define ACM_RATE_UNIT		((UINT32)100000)	/* 100000bps */
#define ACM_CCK_LPM_MIN_MCS		0
#define ACM_CCK_LPM_MAX_MCS		3
#define ACM_CCK_SPM_MIN_MCS		8
#define ACM_CCK_SPM_MAX_MCS		11

UINT8 gAcmMCS_CCK[2][4][2] =
	{
		/* b mode, [2]: long preamble and short preamble */
		/* unit: 100000bps */
		{ { 0, 10 }, { 1, 20 }, { 2, 55 }, { 3, 110 } },
		{ { 8, 10 }, { 9, 20 }, { 10, 55 }, { 11, 110 } }
	};

UINT16 gAcmMCS_OFDM[8][2] =
	{
		/* g mode */
		/* unit: 100000bps */
		{ 0, 60 }, { 1, 90 }, { 2, 120 }, { 3, 180 },
		{ 4, 240 }, { 5, 360 }, { 6, 480 }, { 7, 540 }
	};

#ifndef ACM_CC_FUNC_AUX_TX_TIME
/* use approximation method to calculate packet transmission time */

static UINT16 gAcmTxTimeBody[ACM_RATE_MAX_NUM][ACM_PRE_TIME_DATA_SIZE_NUM];

static UINT16 gAcmTxTimeOthers[ACM_RATE_MAX_NUM][2][5];
#endif // ACM_CC_FUNC_AUX_TX_TIME //

#ifdef ACM_CC_FUNC_11N
static const UINT16 gAcmMCS_HT[2][2][32] =
	{
		/* 20MHz */
		{
			/* Regular GI */
			/* MCS0 ~ MCS31: (unit 100000bps) */
			{
				 65,  130,   195,  260,  390,  520, 585,  650,  130,  260,
				390,  520,   780, 1040, 1170, 1300, 195,  390,  585,  780,
				1170, 1560, 1755, 1950,  260,  520, 780, 1040, 1560, 2080,
				2340, 2600
			},

			/* Short GI */
			/* MCS0 ~ MCS31: (unit 100000bps) */
			{
				 72,   144,  217,  289,  433,  578, 650,  722,  144,  289,
				433,   578,  867, 1156, 1300, 1444, 217,  433,  650,  867,
				1300, 1733, 1950, 2167, 2890,  578, 867, 1156, 1733, 2311,
				2600, 2889
			},
		},

		/* 40MHz */
		{
			/* Regular GI */
			/* MCS0 ~ MCS31: (unit 100000bps) */
			{
				 135,  270,  405,  540,  810, 1080, 1215, 1350,  270,  540,
				 810, 1080, 1620, 2160, 2430, 2700,  405,  810, 1215, 1620,
				2430, 3240, 3645, 4050,  540, 1080, 1620, 2160, 3240, 4320,
				4860, 5400
			},

			/* Short GI */
			/* MCS0 ~ MCS31: (unit 100000bps) */
			{
				 150,  300,  450,  600,  900, 1200, 1350, 1500,  300,  600,
				 900, 1200, 1800, 2400, 2700, 3000,  450,  900, 1350, 1800,
				2700, 3600, 4050, 4500,  600, 1200, 1800, 2400, 3600, 4800,
				5400, 6000
			},
		},
	};

static const UINT16 gAcmRateNdbps[2][32] =
	{
		/* MCS0 ~ MCS31 */
		/* 20MHz */
		{
			 26,  52,  78, 104, 156, 208, 234,  260,
			 52, 104, 156, 208, 312, 416, 468,  520,
			 78, 156, 234, 312, 468, 624, 702,  780,
			104, 208, 312, 416, 624, 832, 936, 1040
		},

		/* MCS0 ~ MCS31 */
		/* 40MHz */
		{
			 54, 108, 162, 216,  324,  432,  486,  540,
			108, 216, 324, 432,  648,  864,  972, 1080,
			162, 324, 486, 648,  972, 1296, 1458, 1620,
			216, 432, 648, 864, 1296, 1728, 1944, 2160
		},
	};

static const UINT32 gAcmRateNes[2] = { 0x00000000, 0xF0E00000 };

#ifndef ACM_CC_FUNC_AUX_TX_TIME


static UINT16 gAcmTxTimeBodyHT[2][2][ACM_RATE_MAX_NUM_HT][ACM_PRE_TIME_DATA_SIZE_NUM][3];

/* tx time for block ack whatever 20/40 or GI */
static UINT16 gAcmTxTimeOthersHT;

#endif // ACM_CC_FUNC_AUX_TX_TIME //

#endif // ACM_CC_FUNC_11N //

/* for memory allocation/free test purpose */
#ifdef ACM_MEMORY_TEST
UINT32 gAcmMemAllocNum = 0;
UINT32 gAcmMemFreeNum = 0;
#endif // ACM_MEMORY_TEST //




/* =========================== Global Function (AP) ========================= */

/*
========================================================================
Routine Description:
	Initialize the ACM Module.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsAcm0Enable	- the ACM flag for AC0
	FlgIsAcm1Enable	- the ACM flag for AC1
	FlgIsAcm2Enable	- the ACM flag for AC2
	FlgIsAcm3Enable	- the ACM flag for AC3
	FlgDatl			- the Dynamic ATL flag

Return Value:
	ACM_RTN_OK		- init OK
	ACM_RTN_FAIL	- init fail

Note:
	FlgIsAcm0Enable ~ FlgIsAcm3Enable and FlgDatl are valid only for QAP mode.
========================================================================
*/
#ifdef CONFIG_AP_SUPPORT
ACM_FUNC_STATUS ACMP_Init(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsAcm0Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm1Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm2Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm3Enable,
	ACM_PARAM_IN	UCHAR					FlgDatl)
#endif // CONFIG_AP_SUPPORT //

{
	ACM_CTRL_PARAM *pEdcaParam;


	/* allocate ACM Control Block memory */
	ACMR_MEM_ALLOC(ACMR_ADAPTER_DB, sizeof(ACM_CTRL_BLOCK), (VOID *));
	if (ACMR_ADAPTER_DB == NULL)
		return ACM_RTN_FAIL;
	/* End of if */
	ACMR_MEM_ZERO(ACMR_ADAPTER_DB, sizeof(ACM_CTRL_BLOCK));

	/* init tasklets */
	/* Note: pAd can not be casted to non-ULONG, ex: (UINT32)pAd */
	ACMR_TASK_INIT(pAd, ACMR_CB->TaskletTspecReqCheck,
					ACM_TASK_TC_ReqCheck, pAd, "ACM_TCREQ");

	ACMR_TASK_INIT(pAd, ACMR_CB->TaskletStreamAliveCheck,
					ACM_TASK_STM_Check, pAd, "ACM_AVCK");

	/* init timers */
	ACMR_TIMER_INIT(pAd, ACMR_CB->TimerTspecReqCheck,
					ACMP_TR_TC_ReqCheck, pAd);

	ACMR_TIMER_INIT(pAd, ACMR_CB->TimerStreamAliveCheck,
					ACMP_TR_STM_Check, pAd);

	/* init other parameters */
	pEdcaParam = &ACMR_CB->EdcaCtrlParam;

	/* enable channel busy time calculation */
	pEdcaParam->FlgIsChanUtilEnable = 1;
	ACMR_CHAN_BUSY_DETECT_ENABLE(pAd);

#ifdef CONFIG_AP_SUPPORT
    /* init available ACM time */
    ACMR_AVAIL_ACM_TIME_UPDATE(pAd, 0);
#endif // CONFIG_AP_SUPPORT //

	/* init chan utilization */
	pEdcaParam->ChanUtil = 0;

	/* init mininum time for total EDCA streams and AC0/1 streams */
	/* use default value */
	pEdcaParam->CP_MinNu = ACM_MIN_CP_NU_DEFAULT;
	pEdcaParam->CP_MinDe = ACM_MIN_CP_DE_DEFAULT;

	/* use default value */
	pEdcaParam->BEK_MinNu = ACM_MIN_BEK_NU_DEFAULT;
	pEdcaParam->BEK_MinDe = ACM_MIN_BEK_DE_DEFAULT;

	/* default TSPEC can change UAPSD settings */
	pEdcaParam->FlgIsTspecUpasdEnable = 1;

	/* init Downgrade function (default: DISABLE) */
	ACMR_MEM_SET(
				pEdcaParam->DowngradeAcNum,
				ACM_DOWNGRADE_DISABLE,
				sizeof(pEdcaParam->DowngradeAcNum));

#ifdef CONFIG_AP_SUPPORT
	/* init ACM flag in QAP; QSTA will follow the ACM indication in beacon */
	pEdcaParam->FlgAcmStatus[0] = FlgIsAcm0Enable;
	pEdcaParam->FlgAcmStatus[1] = FlgIsAcm1Enable;
	pEdcaParam->FlgAcmStatus[2] = FlgIsAcm2Enable;
	pEdcaParam->FlgAcmStatus[3] = FlgIsAcm3Enable;

	ACMR_AC_ACM_CTRL(pAd,
					FlgIsAcm0Enable,
					FlgIsAcm1Enable,
					FlgIsAcm2Enable,
					FlgIsAcm3Enable);

	/* init DATL */
	pEdcaParam->FlgDatl = FlgDatl;

#ifdef ACM_CC_FUNC_ACL
	/* init link list */
	ACMR_CB->ACL_IsEnabled = FALSE;
	ACMR_LIST_INIT(&ACMR_CB->ACL_List);
#endif // ACM_CC_FUNC_ACL //
#endif // CONFIG_AP_SUPPORT //

	pEdcaParam->DatlBwMin[ACM_EDCA_VO_AC_QUE_ID] = ACM_DATL_BW_MIN_VO;
	pEdcaParam->DatlBwMax[ACM_EDCA_VO_AC_QUE_ID] = ACM_DATL_BW_MAX_VO;
	pEdcaParam->DatlBwMin[ACM_EDCA_VI_AC_QUE_ID] = ACM_DATL_BW_MIN_VI;
	pEdcaParam->DatlBwMax[ACM_EDCA_VI_AC_QUE_ID] = ACM_DATL_BW_MAX_VI;
	pEdcaParam->DatlBwMin[ACM_EDCA_BE_AC_QUE_ID] = ACM_DATL_BW_MIN_BE;
	pEdcaParam->DatlBwMax[ACM_EDCA_BE_AC_QUE_ID] = ACM_DATL_BW_MAX_BE;
	pEdcaParam->DatlBwMin[ACM_EDCA_BK_AC_QUE_ID] = ACM_DATL_BW_MIN_BK;
	pEdcaParam->DatlBwMax[ACM_EDCA_BK_AC_QUE_ID] = ACM_DATL_BW_MAX_BK;

	/* init Transmission Time value for different packet size */
	ACM_TX_TimeCalPre(pAd);

	/* init cmd */
	ACM_CMD_Init(pAd);

	/* backup original UAPSD state */
	ACMR_UAPSD_BACKUP(pAd);

	/* allow to handle TSPEC request */
	ACM_MR_TSPEC_ALLOW();

	/* activate a general timer */
	ACMR_TASK_INIT(pAd, ACMR_CB->TaskletGeneral,
					ACM_TASK_General, (ULONG)pAd, "ACM_OTHER");

	ACMR_TIMER_INIT(pAd, ACMR_CB->TimerGeneral,
					ACMP_TR_TC_General, (ULONG)pAd);

#if defined(ACM_CC_FUNC_MBSS) || defined(ACM_CC_FUNC_CHAN_UTIL_MONITOR)
	ACMR_TIMER_ENABLE(ACMR_CB->FlgTimerGeneralEnable,
						ACMR_CB->TimerGeneral,
						ACM_TIMER_GENERAL_PERIOD_TIMEOUT);
#endif // ACM_CC_FUNC_MBSS || ACM_CC_FUNC_CHAN_UTIL_MONITOR //

#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_CHAN_UTIL_MONITOR
	/* backup original AIFSN of AC */
	ACMR_AIFSN_DEFAULT_GET(pAd, ACMR_CB->CU_MON_AifsnAp,
							ACMR_CB->CU_MON_AifsnBss);
#endif // ACM_CC_FUNC_CHAN_UTIL_MONITOR //
#endif // CONFIG_AP_SUPPORT //

	return ACM_RTN_OK;
} /* End of ACMP_Init */


/*
========================================================================
Routine Description:
	Release the ACM Resource.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- release OK
	ACM_RTN_FAIL	- release fail

Note:
	Only used in module remove.
========================================================================
*/
ACM_FUNC_STATUS ACMP_Release(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	/* sanity check */
	if (ACMR_CB == NULL)
		return ACM_RTN_OK;
	/* End of if */

	/* release all resources */
	ACMR_TIMER_DISABLE(ACMR_CB->FlgStreamAliveCheckEnable,
						ACMR_CB->TimerStreamAliveCheck);
	ACMR_TIMER_DISABLE(ACMR_CB->FlgTspecReqCheckEnable,
						ACMR_CB->TimerTspecReqCheck);
	ACMR_TIMER_DISABLE(ACMR_CB->FlgTimerGeneralEnable,
						ACMR_CB->TimerGeneral);

#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_ACL
	ACM_LIST_EMPTY(pAd);
#endif // ACM_CC_FUNC_ACL //
#endif // CONFIG_AP_SUPPORT //

	ACM_TC_ReleaseAll(pAd);
	ACM_CMD_Release(pAd);

	ACMR_MEM_FREE(ACMR_ADAPTER_DB);

	ACMR_ADAPTER_DB = NULL;

#ifdef ACM_MEMORY_TEST
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_msg> ACM_MEM_Alloc_Num = %d\n", gAcmMemAllocNum));
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_msg> ACM_MEM_Free_Num  = %d\n", gAcmMemFreeNum));
#endif // ACM_MEMORY_TEST //

	return ACM_RTN_OK;
} /* End of ACMP_Release */


/*
========================================================================
Routine Description:
	Get bandwidth information.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- the bandwidth information

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_BandwidthInfoGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC			pAd,
	ACM_PARAM_OUT	ACM_BANDWIDTH_INFO			*pInfo)
{
	ACM_CTRL_PARAM *pEdcaParam;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if ((pAd == NULL) || (pInfo == NULL))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Input is NULL! ControlInfomationGet()\n"));
		return ACM_RTN_FAIL;
	} /* End of if */

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	/* total ACM time = EDCA ACM time + HCCA ACM time (HCCA not support) */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	pInfo->AcmUsedTime = pEdcaParam->AcmTotalTime;

#ifdef ACM_CC_FUNC_MBSS
	/* MBSS time */
	pInfo->MbssTotalUsedTime = ACMR_CB->MbssTotalUsedTime;
#endif // ACM_CC_FUNC_MBSS //

	/* EDCA AC ACM time */
	pInfo->AcUsedTime = pEdcaParam->AcmTotalTime;

	/* undetermined TSPEC number */
	pInfo->NumReqLink = ACMR_CB->TspecListReq.TspecNum;

	/* determined EDCA link number */
	pInfo->NumAcLinkUp = pEdcaParam->LinkNumUp;
	pInfo->NumAcLinkDn = pEdcaParam->LinkNumDn;
	pInfo->NumAcLinkDi = pEdcaParam->LinkNumDi;
	pInfo->NumAcLinkBi = pEdcaParam->LinkNumBi;

	/* channel utilization & busy time */
#ifdef CONFIG_AP_SUPPORT
	pEdcaParam->StationCount = ACMR_STATION_COUNT_GET(pAd);
	pEdcaParam->ChanUtil = ACMR_CHAN_UTIL_GET(pAd);
#endif // CONFIG_AP_SUPPORT //

	pInfo->StationCount = pEdcaParam->StationCount;
	pInfo->ChanUtil = pEdcaParam->ChanUtil;
	pInfo->AvalAdmCap = pEdcaParam->AvalAdmCap;
	ACMR_CHAN_BUSY_GET(pAd, pInfo->ChanBusyTime);

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
	return ACM_RTN_OK;

LabelSemErr:
	return ACM_RTN_FAIL;
} /* End of ACMP_BandwidthInfoGet */




/*
========================================================================
Routine Description:
	Check if the BE packet is needed to release.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the destination QSTA
	QueIdx			- 0 ~ 3 (AC0 ~ AC3)
	*pQueueHeader	- the software queue header
	pMbuf			- the packet expected to send out

Return Value:
	ACM_RTN_OK		- release it
	ACM_RTN_FAIL	- do not release it

Note:
	If we can find a TSPEC for the BE packet, we will search a non-TSPEC
	packet in the BE software queue and release it.
========================================================================
*/
ACM_FUNC_STATUS ACMP_BE_IsReallyToReleaseWhenQueFull(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					QueIdx,
	ACM_PARAM_IN	ACMR_QUEUE_HEADER		*pQueueHeader,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf)
{
#ifdef ACM_CC_FUNC_BE_BW_CTRL
	ACM_FUNC_STATUS Status;
	UINT32 QueueType;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* check if the packet is BE and ACM of BE is enabled */
	if ((QueIdx == ACM_EDCA_BE_AC_QUE_ID) &&
		(ACMR_CB->EdcaCtrlParam.FlgAcmStatus[ACM_EDCA_BE_AC_QUE_ID]))
	{
		ACMR_PKT_QOS_TYPE_SET(pMbuf, 0);

		/* check if any TSPEC is built and calculate the tx time */
		Status = ACMP_DataPacketQueue(pAd, pCdb, pMbuf, 0, &QueueType);

		if ((Status == ACM_RTN_OK) &&
			(ACMP_BE_QueueFullHandle(pAd, pCdb, pMbuf) == ACM_RTN_OK))
		{
			/*
				Medium time is enough and another non-TSPEC packet is released
				so we can enqueue the TSPEC packet.
			*/
			return ACM_RTN_FAIL;
		} /* End of if */
	} /* End of if */
#endif // ACM_CC_FUNC_BE_BW_CTRL //

	return ACM_RTN_OK;
} /* End of ACMP_BE_IsReallyToReleaseWhenQueFull */


/*
========================================================================
Routine Description:
	Handle the event when BE software queue is full.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the destination QSTA
	pMbuf			- the packet expected to send out

Return Value:
	ACM_RTN_OK		- handle ok, a non-TSPEC packet is released
	ACM_RTN_FAIL	- handle fail, no non-TSPEC packet is released

Note:
	If the ACM of BE is enabled and the bandwidth is saturated, we will
	protect the bandwidth for BE traffic with TSPEC.

	Two cases:
	1. (only for AP) Origin priority is BE but no any BE TSPEC is built
		for the station, we do NOT need to protect it.
	2. (for AP & STA) Origin priority is not BE but no any non-BE TSPEC
		is built for the station, we will translate it to BE traffic,
		but we do NOT need to protect it.
========================================================================
*/
ACM_FUNC_STATUS ACMP_BE_QueueFullHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf)
{
#ifdef ACM_CC_FUNC_BE_BW_CTRL
	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (pCdb->ACM_NumOfOutTspecInAc[ACM_EDCA_BE_AC_QUE_ID] > 0)
	{
		/* we only need to do protection when BE TSPEC exists */
		if (RTMP_GET_PACKET_TX_TIME(pMbuf) > 0)
		{
			/* the packet is not downgraded to BE so we handle it */
			ACMR_QUEUE_HEADER *pQueueHeader;
			ACMR_QUEUE_ENTRY *pQueueEntry, *pQueueEntryPrev;
			ULONG IrqFlags;


			/* try to look for any queued packet without TSPEC in BE queue */
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			pQueueHeader = &pAd->TxSwQueue[ACM_EDCA_BE_AC_QUE_ID];

			if (pQueueHeader != NULL)
			{
				/* should be here */
				pQueueEntry = pQueueHeader->Head;
				pQueueEntryPrev = pQueueEntry;

				while(pQueueEntry != NULL)
				{
					if (RTMP_GET_PACKET_TX_TIME(\
									QUEUE_ENTRY_TO_PACKET(pQueueEntry)) == 0)
					{
						/* if the packet is not allowed by TSPEC, tx time = 0 */

						/* remove the entry from the queue list */
						pQueueEntryPrev->Next = pQueueEntry->Next;

						if (pQueueEntry == pQueueHeader->Head)
							pQueueHeader->Head = pQueueEntry->Next;
						/* End of if */
						if (pQueueEntry == pQueueHeader->Tail)
							pQueueHeader->Tail = pQueueEntryPrev;
						/* End of if */
						pQueueEntry->Next = NULL;
						pQueueHeader->Number --;

						RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

						/* discard the packet without matched TSPEC */
						RELEASE_NDIS_PACKET(pAd,
											QUEUE_ENTRY_TO_PACKET(pQueueEntry),
											NDIS_STATUS_FAILURE);
						return ACM_RTN_OK;
					} /* End of if */

					pQueueEntryPrev = pQueueEntry;
					pQueueEntry = pQueueEntry->Next;
				} /* End of while */
			} /* End of if */
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
		} /* End of if */
	} /* End of if */
#endif // ACM_CC_FUNC_BE_BW_CTRL //

	return ACM_RTN_FAIL;
} /* End of ACMP_BE_QueueFullHandle */


/*
========================================================================
Routine Description:
	Get current EDCA ACM Information.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- the EDCA ACM information

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_ControlInfomationGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACM_CTRL_INFO			*pInfo)
{
	ACM_CTRL_PARAM *pEdcaParam;
	UINT32 IdAcNum;
#ifdef CONFIG_AP_SUPPORT
	INT32 AvailAdmTime;
	UINT32 IdAcOther;
#endif // CONFIG_AP_SUPPORT //
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if ((pAd == NULL) || (pInfo == NULL))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Input is NULL! ControlInfomationGet()\n"));
		return ACM_RTN_FAIL;
	} /* End of if */

	/* init */
	ACMR_MEM_ZERO(pInfo, sizeof(ACM_CTRL_INFO));

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	/* copy information */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);

	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
	{
		pInfo->FlgIsAcmEnable[IdAcNum] = pEdcaParam->FlgAcmStatus[IdAcNum];
		pInfo->DowngradeAcNum[IdAcNum] = pEdcaParam->DowngradeAcNum[IdAcNum];
		pInfo->AcmOutTime[IdAcNum] = pEdcaParam->AcmOutTime[IdAcNum];
		pInfo->AcmAcTime[IdAcNum] = pEdcaParam->AcmAcTime[IdAcNum];
	} /* End of for */

	pInfo->CP_MinNu = pEdcaParam->CP_MinNu;
	pInfo->CP_MinDe = pEdcaParam->CP_MinDe;
	pInfo->BEK_MinNu = pEdcaParam->BEK_MinNu;
	pInfo->BEK_MinDe = pEdcaParam->BEK_MinDe;

	pInfo->AcmTotalTime = pEdcaParam->AcmTotalTime;
	pInfo->LinkNumUp = pEdcaParam->LinkNumUp;
	pInfo->LinkNumDn = pEdcaParam->LinkNumDn;
	pInfo->LinkNumBi = pEdcaParam->LinkNumBi;
	pInfo->LinkNumDi = pEdcaParam->LinkNumDi;

	pInfo->FlgDatl = pEdcaParam->FlgDatl;

	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
	{
		pInfo->DatlBwMin[IdAcNum] = pEdcaParam->DatlBwMin[IdAcNum];
		pInfo->DatlBwMax[IdAcNum] = pEdcaParam->DatlBwMax[IdAcNum];
	} /* End of for */

	ACMR_MEM_COPY(
				pInfo->DatlBorAcBw,
				pEdcaParam->DatlBorAcBw,
				sizeof(pInfo->DatlBorAcBw));

#ifdef CONFIG_AP_SUPPORT
	/* calculate available admission capability for each AC */
	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
	{
		if (pEdcaParam->FlgDatl == TRUE)
		{
			AvailAdmTime = (INT32)pEdcaParam->DatlBwMax[IdAcNum];
			AvailAdmTime *= ACM_TIME_BASE;
			AvailAdmTime /= 100;

			for(IdAcOther=0; IdAcOther<ACM_DEV_NUM_OF_AC; IdAcOther++)
				AvailAdmTime -= (INT32)pEdcaParam->DatlBorAcBw[IdAcNum][IdAcOther];
			/* End of for */

			if (AvailAdmTime < 0)
				AvailAdmTime = 0; /* should not be here */
			/* End of if */

			pInfo->AvalAdmCapAc[IdAcNum] = AvailAdmTime >> 5; /* unit: 32us */
		}
		else
		{
			/* not support for the AC */
			pInfo->AvalAdmCapAc[IdAcNum] = pEdcaParam->AvalAdmCap; /* unit: 32us */
		} /* End of if */
	} /* End of for */
#endif // CONFIG_AP_SUPPORT //

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
	return ACM_RTN_OK;

LabelSemErr:
	return ACM_RTN_FAIL;
} /* End of ACMP_ControlInfomationGet */


/*
========================================================================
Routine Description:
	Handle something when a QoS data or null frame is received.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	pHeader			- the WLAN MAC header

Return Value:
	None

Note:
	1. Only for QAP.
	2. The frame shall be uplink.
	3. For EDCA, we shall reset activity timeout for QoS data frames.
	4. In LINUX, the function must be called in a tasklet.
	5. If PktTsid is 0xFF, we will get TSID from pHeader.
========================================================================
*/
VOID ACMP_DataNullHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_WLAN_HEADER		*pHeader)
{
	ACM_CTRL_PARAM *pEdcaParam;
	ACM_ENTRY_INFO *pStaAcmInfo;
	ACM_STREAM **ppStmListIn;
	ACM_STREAM *pStream;
	UINT16 FrmSubType;
	UINT16 QosCtrl;
	UCHAR UP, PktAcId;
	UINT32 IdTidNum;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	/*
		We will check ACM_NumOfTspecIn of pCdb before calling the function;
		if > 0, at least one TSPEC exists.
	*/
//	if (!ACMR_IS_ENABLED(pAd))
//		return;
	/* End of if */
	if (ACMR_CB == NULL)
		return;
	/* End of if */

	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	if (pEdcaParam->FlgIsTspecTimeoutEnable == 0)
		return; /* no need to do TSPEC timeout check */
	/* End of if */

	/* init */
	QosCtrl = ACMR_FME_QOSCTRL_GET(pHeader);
	FrmSubType = ACMR_FME_SUBTYPE_GET(pHeader);
	pStream = NULL;

	/* reset activity & suspension timeout for QoS Data or QoS Null frames */
	if ((FrmSubType == ACMR_FME_SUB_TYPE_QOS_DATA) ||
		(FrmSubType == ACMR_FME_SUB_TYPE_QOS_NULL))
	{
#ifdef ACM_CC_FUNC_TCLAS
		/*
			TODO: We need to compare packet with all TCLAS to get UP.
			Solution: Currently we use same UP for all TCLAS in a TSPEC.
		*/
		UP = ACM_TID_GET(QosCtrl);
#else
		UP = ACM_TID_GET(QosCtrl);
#endif // ACM_CC_FUNC_TCLAS //

		if (ACM_IS_EDCA_STREAM(UP))
		{
			/* EDCA stream */

			/* translate UP to AC ID */
			PktAcId = ACM_MR_EDCA_AC(UP);

			/* get management semaphore */
			ACM_TSPEC_SEM_LOCK_CHK(pAd, SplFlags, LabelSemErr);

			/* try to find the IN TSPEC by AC ID */
			pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);
			ppStmListIn = (ACM_STREAM **)pStaAcmInfo->pAcStmIn;

			for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
			{
				if (ppStmListIn[IdTidNum] != NULL)
				{
					if (PktAcId == ACM_MR_EDCA_AC(ppStmListIn[IdTidNum]->UP))
					{
						/* only one IN TSPEC for a AC */
						pStream = ppStmListIn[IdTidNum];
						break;
					} /* End of if */
				} /* End of if */
			} /* End of for */

			/* update its timeout timers if exists */
			if (pStream != NULL)
			{
				ACM_TSPEC *pTspec = pStream->pTspec;


				if (pTspec->InactivityInt != ACM_TSPEC_INACTIVITY_DISABLE)
					pStream->InactivityCur = pStream->pTspec->InactivityInt;
				/* End of if */

#ifdef ACM_CC_FUNC_HCCA
				if (pTspec->SuspensionInt != ACM_TSPEC_SUSPENSION_DISABLE)
					pStream->SuspensionCur = pStream->pTspec->SuspensionInt;
				/* End of if */
#endif // ACM_CC_FUNC_HCCA //
			} /* End of if */

			/* release semaphore */
			ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
		} /* End of if */
	} /* End of if */

LabelSemErr:
	return;
} /* End of ACMP_DataNullHandle */


/*
========================================================================
Routine Description:
	Check if the packet can be queued to the packet queue of the AC.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the peer device
	*pMbuf				- the packet
	FlgIsForceToHighAc	- 1: force the packet to AC3
	*pQueueType			- the new packet queue type
							(ACMR_QID_AC_BE ~ ACMR_QID_AC_VO)

Return Value:
	ACM_RTN_OK			- classify successfully
	ACM_RTN_FAIL		- do not allow to send the packet
	ACM_RTN_NO_ACM		- the ACM of the AC is disabled
========================================================================
*/


#ifdef WMM_ACM_PKT_NUM_DEBUG
UINT32 WMM_ACM_NumOfPkt[4] = { 0, 0, 0, 0 };
#endif // WMM_ACM_PKT_NUM_DEBUG //

ACM_FUNC_STATUS ACMP_DataPacketQueue(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf,
	ACM_PARAM_IN	UCHAR					FlgIsForceToHighAc,
	ACM_PARAM_OUT	UINT32					*pQueueType)
{
	ACM_STREAM **ppAcmStmList;
	ACM_STREAM *pStream;
	ACM_TS_INFO *pTsInfo;
	ACM_ENTRY_INFO *pStaAcmInfo;
	ACM_CTRL_PARAM *pEdcaParam;
	ACM_STATISTICS *pStats;

	UINT32 PktLen, LenFrag, LenLastFrag, NumFrag;
	UCHAR  FlgIsFindTspec;
	UINT32 PktAcId, TxTime;
	UCHAR  UP, TSID;
	UINT32 TxQueueType;
	UCHAR  AccessPolicy, AckPolicy;
	UINT32 IdTidNum;
	ULONG  SplFlags;

#ifdef ACM_CC_FUNC_TCLAS
	ACM_TCLAS *pTclas, TclasIpClass;
	UINT32 IdTclasNum;
	UCHAR  TclasBitmap;

	UCHAR *pPkt; /* same as pMbuf */
	UCHAR *pAddrSrc, *pAddrDst;
	UINT16 Type; /* ethernet frame type/len */
	UINT16 VlanTag;
	UCHAR  FlgIsMatchTspec, FlgIsIpPkt;
#endif // ACM_CC_FUNC_TCLAS //


	WMM_ACM_FUNC_NAME_PRINT("IN");

    /* sanity check */
	RTMP_SET_PACKET_TX_TIME(pMbuf, 0);

	*pQueueType = ACMR_QID_AC_BE;

    if (pCdb == NULL)
	{
		/* *pQueueType is useless */
        return ACM_RTN_NO_ACM;
    } /* End of if */

	/* we will check before calling the function to speed up */
//	if (ACMP_IsAnyACEnabled(pAd) != ACM_RTN_OK)
//		return ACM_RTN_NO_ACM;
	/* End of if */

	/* init */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	pStats = &pEdcaParam->Stats;
	PktLen = ACMR_WLAN_LEN_GET(pMbuf);

#ifdef ACM_CC_FUNC_TCLAS
	pPkt = (UCHAR *)ACMR_WLAN_PKT_GET(pMbuf);

	pAddrDst = (UCHAR *)pPkt;
	pAddrSrc = (UCHAR *)(pPkt + ACM_ETH_DA_ADDR_LEN);
	Type = *(UINT16 *)(pPkt + ACM_ETH_DA_ADDR_LEN + ACM_ETH_SA_ADDR_LEN);

	TclasIpClass.ClassifierMask = 0;

	VlanTag			= 0xFFFF;
	FlgIsIpPkt		= 0;
	FlgIsMatchTspec	= 0;
#endif // ACM_CC_FUNC_TCLAS //

	pStream			= NULL;
	FlgIsFindTspec	= 0;
	TSID			= 0xff; /* default: no TSID information */
	UP				= 0xff;
	LenFrag			= 0;
	LenLastFrag		= 0;
	NumFrag			= 0;
	TxTime			= 33; /* minimum tx time, > 1 unit = 32us */

	if (FlgIsForceToHighAc == 1)
		TSID = 0x07; /* force to use AC3 */
	else
	{
		if (ACMR_PKT_QOS_TYPE_GET(pMbuf) == ACM_QOS_TYPE_NULL)
			TSID = ACMR_PKT_UP_GET(pMbuf);
		/* End of if */
	} /* End of if */

	ACMR_PKT_MARK_MIN_PHY_MODE(pMbuf, ACMR_PHY_NONE);
	ACMR_PKT_MARK_MIN_PHY_MCS(pMbuf, 0);

	/* get management semaphore */
	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);

	ACM_TSPEC_IRQ_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_CLSFY_NOT_ALLOW);

#ifdef ACM_CC_FUNC_TCLAS
	/* check TCLAS in WMM streams */
	if (FlgIsForceToHighAc == 0)
	{
		ppAcmStmList = (ACM_STREAM **)pStaAcmInfo->pAcStmOut;

		for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
		{
			pStream = ppAcmStmList[IdTidNum];

			if ((pStream == NULL) || (pStream->pTclas == NULL))
				continue; /* no TSPEC exists, check next one */
			/* End of if */

			for(IdTclasNum=0; IdTclasNum<ACM_TSPEC_TCLAS_MAX_NUM; IdTclasNum++)
			{
				if (pStream->pTclas[IdTclasNum] == NULL)
				{
					IdTclasNum = ACM_TSPEC_TCLAS_MAX_NUM;
					break; /* no other TCLAS */
				} /* End of if */

				pTclas = pStream->pTclas[IdTclasNum];
				TclasBitmap = pTclas->ClassifierMask;
				FlgIsMatchTspec = 0;

				switch(pTclas->ClassifierType)
				{
					case ACM_TCLAS_TYPE_ETHERNET:
						if (TclasBitmap & 0x01)
						{
							/* check source address */
							if (!(AMR_IS_SAME_MAC(pAddrSrc,
										pTclas->Clasifier.Ethernet.AddrSrc)))
							{
								break; /* compare fail */
							} /* End of if */
						} /* End of if */

						if (TclasBitmap & 0x02)
						{
							/* check destination address */
							if (!(AMR_IS_SAME_MAC(pAddrDst,
										pTclas->Clasifier.Ethernet.AddrDst)))
							{
								break; /* compare fail */
							} /* End of if */
						} /* End of if */

						if (TclasBitmap & 0x04)
						{
							/* check type */
							if (Type != pTclas->Clasifier.Ethernet.Type)
								break; /* compare fail */
							/* End of if */
						} /* End of if */

						FlgIsMatchTspec = 1;
						break;

					case ACM_TCLAS_TYPE_IP_V4:
						if (TclasIpClass.ClassifierMask == 0)
						{
							/* get IP info. from the frame at first loop */
							TclasIpClass.ClassifierMask = 1;

							if (ACM_TCLAS_IP_INFO_Get(pPkt,
												&TclasIpClass) == ACM_RTN_OK)
							{
								FlgIsIpPkt = 1;
							} /* End of if */
						} /* End of if */

						if (FlgIsIpPkt == 0)
							break; /* the frame is not a IP packet */
						/* End of if */

						if ((TclasBitmap & 0x01) &&
							(pTclas->Clasifier.IPv4.Version != \
										TclasIpClass.Clasifier.IPv4.Version))
						{
							break; /* compare Version fail */
						} /* End of if */

						if ((TclasBitmap & 0x02) &&
							(pTclas->Clasifier.IPv4.IpSource != \
										TclasIpClass.Clasifier.IPv4.IpSource))
						{
							break; /* compare source IP fail */
						} /* End of if */

						if ((TclasBitmap & 0x04) &&
							(pTclas->Clasifier.IPv4.IpDest != \
										TclasIpClass.Clasifier.IPv4.IpDest))
						{
							break; /* compare destination IP fail */
						} /* End of if */

						if ((TclasBitmap & 0x08) &&
							(pTclas->Clasifier.IPv4.PortSource != \
										TclasIpClass.Clasifier.IPv4.PortSource))
						{
							break; /* compare source port fail */
						} /* End of if */

						if ((TclasBitmap & 0x10) &&
							(pTclas->Clasifier.IPv4.PortDest != \
										TclasIpClass.Clasifier.IPv4.PortDest))
						{
							break; /* compare destination port fail */
						} /* End of if */

						if ((TclasBitmap & 0x20) &&
							(pTclas->Clasifier.IPv4.DSCP != \
										TclasIpClass.Clasifier.IPv4.DSCP))
						{
							break; /* compare DSCP fail */
						} /* End of if */

						if ((TclasBitmap & 0x40) &&
							(pTclas->Clasifier.IPv4.Protocol != \
										TclasIpClass.Clasifier.IPv4.Protocol))
						{
							break; /* compare protocol fail */
						} /* End of if */

						FlgIsMatchTspec = 1;
						break;

					case ACM_TCLAS_TYPE_8021DQ:
						if (VlanTag == 0xFFFF)
						{
							/* only get VLAN tag once at first loop */
							ACM_TCLAS_VLAN_INFO_Get(pPkt, &VlanTag);
						} /* End of if */

						if ((TclasBitmap & 0x01) &&
							(VlanTag != 0xFFFF) &&
							(pTclas->Clasifier.IEEE8021Q.TagType != \
																	VlanTag))
						{
							break; /* compare TAG Type fail */
						} /* End of if */

						FlgIsMatchTspec = 1;
						break;
				} /* End of switch */

				if (FlgIsMatchTspec == 1)
				{
					if (pStream->TclasProcessing == \
												ACM_WME_TCLAS_PROCESSING_ONE)
					{
						/* find a matched TCLAS so is the TS */
						FlgIsFindTspec = 1;
						break;
					} /* End of if */
				}
				else
				{
					if (pStream->TclasProcessing == \
												ACM_WME_TCLAS_PROCESSING_ALL)
						break; /* all TCLAS must be matched so not the TS */
					/* End of if */
				} /* End of if */
			} /* End of for */

			if ((FlgIsMatchTspec == 1) &&
				(IdTclasNum == ACM_TSPEC_TCLAS_MAX_NUM))
			{
				FlgIsFindTspec = 1; /* all TCLASS are matched */
			} /* End of if */

			if (FlgIsFindTspec == 1)
				break; /* the frame belongs to the stream */
			/* End of if */
		} /* End of for */
	} /* End of if */
#endif // ACM_CC_FUNC_TCLAS //

	/* check if any TSPEC with TCLAS number = 0 exists */
	if ((FlgIsFindTspec == 0) &&
		(FlgIsForceToHighAc == 0) &&
		((pStream == NULL) ||
		 pStream->pTclas[0] == NULL))
	{
		/*
			Though, TCLAS number = 0, but maybe the medium time of a
			TSPEC != 0, search the TSPEC.
		*/

		/* check if the packet is QoS Null frame */
		if (!(ACMR_PKT_QOS_TYPE_GET(pMbuf) == ACM_QOS_TYPE_NULL))
		{
			/* the packet is NOT QoS Null frame */

			/* the condition to find the TSPEC is 'same AC ID' */
			TSID = ACM_TSID_Get(pAd, pMbuf);
			PktAcId = ACM_MR_EDCA_AC(TSID);

			/* check if the ACM of the AC is enabled */
			if (pEdcaParam->FlgAcmStatus[PktAcId] != ACM_FLG_FUNC_ENABLED)
			{
				/* *pQueueType is useless */
				goto LabelErrNoACM;
			} /* End of if */

			/* check if the UP matches any out TSPEC with TCLAS number = 0 */
			ppAcmStmList = (ACM_STREAM **)pStaAcmInfo->pAcStmOut;

			for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
			{
				pStream = ppAcmStmList[IdTidNum];

				if ((pStream == NULL) || (pStream->pTspec == NULL))
					continue; /* no TSPEC exists, check next one */
				/* End of if */

				if (pStream->pTclas[0] == NULL)
				{
					/* find the TSPEC without TCLAS */
					if (PktAcId == ACM_MR_EDCA_AC(pStream->pTspec->TsInfo.UP))
					{
						/* find the TSPEC */
						FlgIsFindTspec = 1;
						break;
					} /* End of if */
				} /* End of if */
			} /* End of for */
		} /* End of if */
	} /* End of if */

	if ((FlgIsFindTspec == 1) &&
		(pStream != NULL) &&
		(FlgIsForceToHighAc == 0))
	{
		/* at least one TSPEC matches the packet */
		pTsInfo = &pStream->pTspec->TsInfo;
		AccessPolicy = pTsInfo->AccessPolicy;
		AckPolicy = pTsInfo->AckPolicy;
		PktAcId = pStream->AcmAcId;

		/* refresh inactivity & suspension timeout for the TS */
		if (pStream->pTspec->InactivityInt != ACM_TSPEC_INACTIVITY_DISABLE)
			pStream->InactivityCur = pStream->pTspec->InactivityInt;
		/* End of if */

#ifdef ACM_CC_FUNC_HCCA
		if (pStream->pTspec->SuspensionInt != ACM_TSPEC_SUSPENSION_DISABLE)
			pStream->SuspensionCur = pStream->pTspec->SuspensionInt;
		/* End of if */
#endif // ACM_CC_FUNC_HCCA //
	}
	else
	{
		/* no TSPEC is matched */
		AccessPolicy = ACM_ACCESS_POLICY_EDCA;
		AckPolicy = ACM_ACK_POLICY_NORMAL;

		if (ACMR_PKT_QOS_TYPE_GET(pMbuf) == ACM_QOS_TYPE_NULL)
		{
			/* use the TID of the QoS Null packet */
			PktAcId = ACM_MR_EDCA_AC(ACMR_PKT_UP_GET(pMbuf));
		}
		else
			PktAcId = ACM_MR_EDCA_AC(TSID);
		/* End of if */
	} /* End of if */


	/* here, we have got the PktAcId */

#ifdef ACM_CC_FUNC_SOFT_ACM

	/*
		Test Note:
		When you want to test a TSPEC with 208B/83Kbps, you must not
		send packets with 1514B to do the test.
		Because we have about 51 packets with 208B and we will have
		51 * (11g preamble time + sifs + ack time), if you send packets
		with 1514B, these extra times will be used to send packets with
		1514B, you will not see the 83kbps throughput and you will see
		higher throughput.
	*/

	ACM_TG_CMT_USED_TIME_PASS_CRITERIA;

	/* check whether tx time > TXOP limit for AC2 & AC3 */

	if (pStream != NULL)
	{
		/* no TSPEC is found so no need to calculate the tx time */
		UINT64 Timestamp, TimeOffset;
		UINT32 TimeAllowed;


		/* get 64-bit Timestamp */
		ACMR_TIMESTAMP_GET(pAd, Timestamp);

#ifdef ACM_CC_FUNC_11N
		if (ACMR_IS_HT_RATE_USED(pCdb))
		{
			/* the station uses HT rate */
			TxTime = ACM_TX_TimeCalOnFlyHT(
											pAd,
											pCdb,
											pStream,
											Timestamp,
											PktLen,
											ACMR_CLIENT_MCS_GET(pCdb),
											0);
		}
		else
#endif // ACM_CC_FUNC_11N //
		{
			UCHAR FlgIsRtsEnable, FlgIsCtsEnable;

			FlgIsRtsEnable = ACMR_RTS_FLAG_GET(pAd, pMbuf);

			/* priority of RTS/CTS is over CTS-self */
			if (FlgIsRtsEnable == 0)
				FlgIsCtsEnable = ACMR_CTS_FLAG_GET(pAd, pMbuf);
			else
				FlgIsCtsEnable = 0;
			/* End of if */

			TxTime = ACM_TX_TimeCalOnFly(
											pAd,
											pCdb,
											PktLen,
											ACM_Rate_Mapping(pAd, pCdb),
											FlgIsCtsEnable,
											FlgIsRtsEnable,
											ACMR_STA_IS_SPREAMBLE(pAd, pCdb),
											0);
		}

		if (TxTime > 0x0000FFFF)
		{
			ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_err> Tx Time > 0xFFFF us!\n"));
		} /* End of if */

		RTMP_SET_PACKET_TX_TIME(pMbuf, (UINT16)TxTime);
		RTMP_SET_PACKET_STM_TSID(pMbuf, pStream->pTspec->TsInfo.TSID);
		/* no need to mark direction, must be output TSPEC */

		/* translate MediumTime (unit: 32us) to us */
		ACMR_ALLOWED_TIME_GET(pStream, TimeAllowed);

		/* use signed INT64 to avoid round-up problem */
		TimeOffset = (UINT64)((INT64)Timestamp - \
								(INT64)pStream->TxTimestampMarkEnqueue);

		if (TimeOffset >= ACM_TIME_BASE)
		{
			/* the first packet in next second */
			pStream->TxTimestampMarkEnqueue = Timestamp;

			/* reset the used time */
			if (pStream->AcmUsedTimeEnqueue > TimeAllowed)
				pStream->AcmUsedTimeEnqueue -= TimeAllowed;
			else
				pStream->AcmUsedTimeEnqueue = 0;
			/* End of if */

#ifdef WMM_ACM_PKT_NUM_DEBUG
			ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm> Num Of Packet for AC %d in a second = %d\n", PktAcId,
				WMM_ACM_NumOfPkt[PktAcId]));
			WMM_ACM_NumOfPkt[PktAcId] = 0;
#endif // WMM_ACM_PKT_NUM_DEBUG //
		}
		else
		{
#ifndef PERFORMANCE_IMPACT_TEST
			/* during a second */
			if (pStream->AcmUsedTimeEnqueue > TimeAllowed)
			{
				/* can not tx the packet to the AC so check downgrade flag */
				RTMP_SET_PACKET_TX_TIME(pMbuf, 0);

				if (pEdcaParam->DowngradeAcNum[PktAcId] != \
														ACM_DOWNGRADE_DISABLE)
				{
					TxQueueType = ACM_TxQueueTypeGet( \
										pEdcaParam->DowngradeAcNum[PktAcId]);

					ACM_STATS_COUNT_INC(pStats->Downgrade[PktAcId]);

					/*
						In WMM spec., A.2 Use of Admission Control and
						Downgrading

						(3)	The MSDU is sent using a different UP.  The UP is
						changed to map to a lower AC that does not require
						admission control.  The UP has to be changed prior to
						calculating the MIC, assignment of the TSC and mixing
						the keys. Changing of the UP is performed outside the
						MAC and is out of the scope of this spec.

						In an AC for which the admission control mandatory
						flag is set to 1, a WMM AP should use option (3).
					*/

					/* need to change the UP of QoS Control field */
					TSID = gEDCA_AC_UP[pEdcaParam->DowngradeAcNum[PktAcId]];
					ACMR_PKT_MARK_UP(pMbuf, TSID);

					goto LabelOK;
				} /* End of if */

				ACM_STATS_COUNT_INC(pStats->DropByAdmittedTime);

				/* downgrade function is disabled so discarding the packet */
				goto LabelErr;
			} /* End of if */
#endif // PERFORMANCE_IMPACT_TEST //
		} /* End of if */

		/* accumulate used time */
#ifdef WMM_ACM_PKT_NUM_DEBUG
		WMM_ACM_NumOfPkt[PktAcId] ++;
#endif // WMM_ACM_PKT_NUM_DEBUG //

		pStream->AcmUsedTimeEnqueue += TxTime;
	}
	else
	{
		/* No TSPEC is found so Tx Time = 0 (default) */
	} /* End of if */
#endif // ACM_CC_FUNC_SOFT_ACM //

	/* assign AC ID or TS ID to the frame */
	if ((FlgIsFindTspec == 1) &&
		(pStream != NULL) &&
		(FlgIsForceToHighAc == 0))
	{
		/* re-set user priority to packet information, used by WLAN module */
		TxQueueType = pStream->TxQueueType;

		if (TSID == 0xff)
			TSID = ACM_TSID_Get(pAd, pMbuf);/* get TSID by UP or DSCP */
		/* End of if */

		ACMR_PKT_MARK_UP(pMbuf, TSID);
		ACMR_PKT_MARK_MIN_PHY_MODE(pMbuf, pStream->PhyModeMin);
		ACMR_PKT_MARK_MIN_PHY_MCS(pMbuf, pStream->McsMin);
	}
	else
	{
		/*
			Can not find any matched TCLAS/TSPEC so classify the packet to
			the queue by UP or DSCP (EDCA stream).
		*/

		if (!(ACMR_PKT_QOS_TYPE_GET(pMbuf) == ACM_QOS_TYPE_NULL))
		{
			/* for non-QoS Null frame */
			if (TSID == 0xff)
				TSID = ACM_TSID_Get(pAd, pMbuf);/* get TSID by UP or DSCP */
			/* End of if */

			PktAcId = ACM_MR_EDCA_AC(TSID);
		} /* End of if */

		if ((PktAcId != ACM_EDCA_BK_AC_QUE_ID) &&
			(pEdcaParam->FlgAcmStatus[PktAcId] == ACM_FLG_FUNC_ENABLED) &&
			(FlgIsForceToHighAc == 0))
		{
			/*
				The ACM for the AC is enabled but we can not get any TSPEC
				for the packet so we shall send the frame to AC0 (BE) queue
				except BK traffic.
			*/

			TSID = 0; /* use BE */
			PktAcId = ACM_EDCA_BE_AC_QUE_ID;


			ACM_STATS_COUNT_INC(pStats->PriorityChange[PktAcId]);
		} /* End of if */

		/* send the frame the the AC */
		pStream = NULL; /* no TSPEC is found */
		TxQueueType = ACM_TxQueueTypeGet(PktAcId);

		/* re-set user priority to packet information, used by WLAN module */
		ACMR_PKT_MARK_UP(pMbuf, TSID);
	} /* End of if */

LabelOK:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	*pQueueType = TxQueueType;
	return ACM_RTN_OK;

LabelErrNoACM:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return ACM_RTN_NO_ACM;

LabelErr:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return ACM_RTN_FAIL;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! ACMP_DataPacketQueue()\n"));
	return ACM_RTN_NO_ACM;
} /* End of ACMP_DataPacketQueue */


/*
========================================================================
Routine Description:
	Enable or disable Dynamic ATL function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable
	*pDatlBwMin		- new minimum bandwidth threshold
	*pDatlBwMax		- new maximum bandwidth threshold

Return Value:
	None

Note:
	if you dont want to change bandwidth threshold, you can input NULL.
	pDatlBwMin = NULL or pDatlBwMax = NULL
========================================================================
*/
VOID ACMP_DatlCtrl(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable,
	ACM_PARAM_IN	UCHAR					*pDatlBwMin,
	ACM_PARAM_IN	UCHAR					*pDatlBwMax)
{
	ACM_CTRL_PARAM *pEdcaParam;
	UINT32 IdAcNum, IdAcNumOther;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (!ACMR_IS_ENABLED(pAd))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> WMM ACM is disabled!\n"));
		return;
	} /* End of if */

	/* first delete all TSPEC */
	ACMP_TC_DeleteAll(pAd);

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* change DATL flag */
	ACMR_CB->EdcaCtrlParam.FlgDatl = FlgIsEnable;

	/*
		Assign new minimum and maximum bandwidth threshold
		and clear all borrowing bandwidth.
	*/
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);

	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
	{
		if (pDatlBwMin != NULL)
			pEdcaParam->DatlBwMin[IdAcNum] = pDatlBwMin[IdAcNum];
		/* End of if */

		if (pDatlBwMax != NULL)
			pEdcaParam->DatlBwMax[IdAcNum] = pDatlBwMax[IdAcNum];
		/* End of if */

		for(IdAcNumOther=0; IdAcNumOther<ACM_DEV_NUM_OF_AC; IdAcNumOther++)
			pEdcaParam->DatlBorAcBw[IdAcNum][IdAcNumOther] = 0;
		/* End of for */
	} /* End of for */

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> DATL reset!\n"));
	return;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! DatlCtrl()\n"));
	return;
} /* End of ACMP_DatlCtrl */


/*
========================================================================
Routine Description:
	Inform us that tx compleletion interrupt occurred.

Arguments:
	pAd				- WLAN control block pointer
	*pDevMac		- the destination MAC
	*pTsInfo		- the TS Info of the packet
	FlgIsErr		- if the frame tx is error

Return Value:
	None

Note:
	1. Responsible for DELTS ACK frame check.
	2. Now we call the function after DELTS frame is sent immediately.
	3. If we need to guarantee the DELTS frame is received in peer device,
		we need to call the function in TX done service routine.
========================================================================
*/
VOID ACMP_DeltsFrameACK(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	UCHAR				*pTsInfo,
	ACM_PARAM_IN	UCHAR				FlgIsErr)
{
	ACM_STREAM *pStream;
	ACMR_STA_DB *pCdb;
	UINT32 NumMaxSearch;
	UCHAR StmAcId;
	UCHAR Direction;
//	UCHAR FlgIsActive;
//	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");


	/* init */
	pCdb = NULL;
	NumMaxSearch = 0;
	StmAcId = 0;
	Direction = 0;
//	FlgIsActive = 0;

	/* find it in request or active list */
//	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	while(1)
	{
		/* should be once in the while loop */
		pStream = ACM_TC_Find(pAd, pDevMac, (ACM_TS_INFO *)pTsInfo);
		if (pStream == NULL)
			break; /* not find */
		/* End of if */

		pCdb = pStream->pCdb;
		StmAcId = pStream->AcmAcId;
		Direction = pStream->pTspec->TsInfo.Direction;


		/* destroy the request or active stream */
		ACM_TC_Destroy(pAd, pStream, 0);

		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> DEL the request ok! DeltsFrameACK()\n"));

		if (++NumMaxSearch > 5)
			break; /* avoid forever loop, should not be here */
		/* End of if */
	} /* End of while */

//	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);

	/* recover the UAPSD state if the deleted TSPEC is ever actived */
	/* we have already recover UAPSD state in ACM_TC_Destroy() */
//	if ((pCdb != NULL) && (FlgIsActive))
//		ACM_APSD_Ctrl(pAd, pCdb, StmAcId, Direction, 0, 0);
	/* End of if */

	return;

} /* End of ACMP_DeltsFrameACK */


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Append the QBSS Load element to the beacon frame.

Arguments:
	pAd				- WLAN control block pointer
	*pPkt			- the beacon frame

Return Value:
	the element total length

Note:
========================================================================
*/
UINT32 ACMP_Element_QBSS_LoadAppend(
 	ACM_PARAM_IN		ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN_OUT	UCHAR					*pPkt)
{
	ACM_ELM_QBSS_LOAD QBSS_Load, *pLoad = &QBSS_Load;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (!ACMR_IS_ENABLED(pAd))
		return 0;
	/* End of if */

	/* copy information */
	pLoad->ElementId = ACM_ELM_QBSS_LOAD_ID;
	pLoad->Length = ACM_ELM_QBSS_LOAD_LEN;

	pLoad->StationCount = ACMR_STA_CUR_COUNT(pAd);
	pLoad->ChanUtil = ACMR_CHAN_UTILIZATION_GET(pAd);
	pLoad->AvalAdmCap = 0;

	/* copy the QBSS element to the frame, pPkt */
	ACMR_MEM_COPY(pPkt, (UCHAR *)pLoad, sizeof(ACM_ELM_QBSS_LOAD));

	return (ACM_ELM_ID_LEN_SIZE + ACM_ELM_QBSS_LOAD_LEN);
} /* End of ACMP_Element_QBSS_LoadAppend */
#endif // CONFIG_AP_SUPPORT //


/*
========================================================================
Routine Description:
	Reset current ACM Flag for each AC.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsAcm0Enable	- the ACM flag for AC0
	FlgIsAcm1Enable	- the ACM flag for AC1
	FlgIsAcm2Enable	- the ACM flag for AC2
	FlgIsAcm3Enable	- the ACM flag for AC3

Return Value:
	None

Note:
========================================================================
*/
VOID ACMP_EnableFlagReset(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsAcm0Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm1Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm2Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm3Enable)
{
	UCHAR *pFlgArrayIsAcmEnabled;
	UCHAR FlgIsAllTspecNeedToDelete;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	FlgIsAllTspecNeedToDelete = 1;

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* check if we need to delete all TS */
	pFlgArrayIsAcmEnabled = ACMR_CB->EdcaCtrlParam.FlgAcmStatus;

	if ((pFlgArrayIsAcmEnabled[0] == FlgIsAcm0Enable) &&
		(pFlgArrayIsAcmEnabled[1] == FlgIsAcm1Enable) &&
		(pFlgArrayIsAcmEnabled[2] == FlgIsAcm2Enable) &&
		(pFlgArrayIsAcmEnabled[3] == FlgIsAcm3Enable))
	{
		/* same ACM flag for all AC so no need to delete TSPECs */
		FlgIsAllTspecNeedToDelete = 0;
	}

	/* update new ACM flag */
	pFlgArrayIsAcmEnabled[0] = FlgIsAcm0Enable;
	pFlgArrayIsAcmEnabled[1] = FlgIsAcm1Enable;
	pFlgArrayIsAcmEnabled[2] = FlgIsAcm2Enable;
	pFlgArrayIsAcmEnabled[3] = FlgIsAcm3Enable;


	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Reset ACM flag to %d %d %d %d\n",
				FlgIsAcm0Enable, FlgIsAcm1Enable, FlgIsAcm2Enable, FlgIsAcm3Enable));

#ifdef CONFIG_AP_SUPPORT
	ACMR_AC_ACM_CTRL(pAd,
					FlgIsAcm0Enable,
					FlgIsAcm1Enable,
					FlgIsAcm2Enable,
					FlgIsAcm3Enable);
#endif // CONFIG_AP_SUPPORT //

	/* delete all streams if needed */
	if (FlgIsAllTspecNeedToDelete)
		ACMP_TC_DeleteAll(pAd);
	/* End of if */


	return;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! EnableFlagReset()\n"));
	return;
} /* End of ACMP_EnableFlagReset */


/*
========================================================================
Routine Description:
	Resume the ACM.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID ACMP_FSM_Resume(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	WMM_ACM_FUNC_NAME_PRINT("IN");

	ACM_MR_TSPEC_ALLOW(pAd);
} /* End of ACMP_FSM_Resume */


/*
========================================================================
Routine Description:
	Suspend the ACM.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
	QSTA: No any TSPEC request can be issued.
	QAP: No any TSPEC request can be handled.
========================================================================
*/
VOID ACMP_FSM_Suspend(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	WMM_ACM_FUNC_NAME_PRINT("IN");

	ACM_MR_TSPEC_DISALLOW(pAd);
} /* End of ACMP_FSM_Suspend */


/*
========================================================================
Routine Description:
	Return TRUE if the ACM of all AC are enabled.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- the ACM of all AC is enabled
	ACM_RTN_FAIL	- the ACM of one AC is disabled

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_IsAllACEnabled(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	UCHAR FlgAcmStatus[ACM_DEV_NUM_OF_AC];
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* copy ACM enabled flag first */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	ACMR_MEM_COPY(FlgAcmStatus,
				ACMR_CB->EdcaCtrlParam.FlgAcmStatus,
				sizeof(ACMR_CB->EdcaCtrlParam.FlgAcmStatus));

	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	/* check ACM enabled flag */
	if ((FlgAcmStatus[ACM_EDCA_BE_AC_QUE_ID] == 0) ||
		(FlgAcmStatus[ACM_EDCA_BK_AC_QUE_ID] == 0) ||
		(FlgAcmStatus[ACM_EDCA_VI_AC_QUE_ID] == 0) ||
		(FlgAcmStatus[ACM_EDCA_VO_AC_QUE_ID] == 0))
	{
		/* at least one AC ACM is disabled */
		return ACM_RTN_FAIL;
	} /* End of if */

	return ACM_RTN_OK;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! IsAllACEnabled()\n"));
	return ACM_RTN_FAIL;
} /* End of ACMP_IsAllACEnabled */


/*
========================================================================
Routine Description:
	Return TRUE if the ACM of any AC is enabled.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- the ACM of any AC is enabled
	ACM_RTN_FAIL	- the ACM of all AC is disabled

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_IsAnyACEnabled(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{

	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* check ACM enabled flag */
	if (*(UINT32 *)(ACMR_CB->EdcaCtrlParam.FlgAcmStatus) == 0)
	{
		/* all AC ACM are disabled */
		return ACM_RTN_FAIL;
	} /* End of if */

	return ACM_RTN_OK;

} /* End of ACMP_IsAnyACEnabled */


/*
========================================================================
Routine Description:
	Return TRUE if the frame is Bandwidth Announce Action Frame.

Arguments:
	pAd				- WLAN control block pointer
	*pMbuf			- the frame

Return Value:
	ACM_RTN_OK		- Yes
	ACM_RTN_FAIL	- No

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_IsBwAnnounceActionFrame(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	VOID					*pMbuf)
{
#ifdef ACM_CC_FUNC_MBSS
	ACM_BW_ANN_FRAME *pFrameAnn;
	UCHAR *pActFrame;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pActFrame = (UCHAR *)pMbuf + ACMR_FME_LEG_HEADER_SIZE;
	pFrameAnn = (ACM_BW_ANN_FRAME *)pActFrame;

	if ((pFrameAnn->Category == ACM_CATEGORY_WME) &&
		(pFrameAnn->Action == ACM_ACTION_WME_BW_ANN))
	{
		return ACM_RTN_OK;
	} /* End of if */

	return ACM_RTN_FAIL;
#else

	return ACM_RTN_FAIL;
#endif // ACM_CC_FUNC_MBSS //
} /* End of ACMP_IsBwAnnounceActionFrame */


/*
========================================================================
Routine Description:
	Check if the action frame is the DELTS frame.

Arguments:
	*pMblk			- the action frame

Return Value:
	TRUE			- Yes
	FALSE			- No

Note:
========================================================================
*/
BOOLEAN ACMP_IsDeltsFrame(
	ACM_PARAM_IN	UCHAR					*pMblk)
{
	UCHAR *pActFrame;
	UCHAR Category, Action;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* points to the action frame WLAN header */
	pActFrame = (UCHAR *)pMblk;

	/* get Category & Action field */
	Category = *pActFrame;
	Action = *(pActFrame+1);

	/* checking */
	if (Category == ACM_CATEGORY_WME)
	{
		if (Action == ACM_ACTION_WME_TEAR_DOWN)
			return TRUE;
		/* End of if */
	} /* End of if */

	return FALSE;
} /* End of ACMP_IsDeltsFrame */


/*
========================================================================
Routine Description:
	Check if the packet needs to do ACM.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the destination QSTA
	UP				- user priority

Return Value:
	TRUE			- Yes
	FALSE			- No

Note:
	When the ACM of AC is set, the packet is needed to do ACM.

	Only used in transmission path.
========================================================================
*/
BOOLEAN ACMP_IsNeedToDoAcm(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					UP)
{
	UINT8 AcId;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	AcId = ACM_MR_EDCA_AC(UP);

	if (ACMR_CB->EdcaCtrlParam.FlgAcmStatus[AcId])
		return TRUE;
	/* End of if */

	return FALSE;
} /* End of ACMP_IsNeedToDoAcm */


/*
========================================================================
Routine Description:
	Check if the action frame is the ADDTS Response frame.

Arguments:
	*pMblk			- the action frame

Return Value:
	TRUE			- Yes
	FALSE			- No

Note:
========================================================================
*/
BOOLEAN ACMP_IsResponseFrame(
	ACM_PARAM_IN	UCHAR					*pMblk)
{
	UCHAR *pActFrame;
	UCHAR Category, Action;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* points to the action frame WLAN header */
	pActFrame = (UCHAR *)pMblk;

	/* get Category & Action field */
	Category = *pActFrame;
	Action = *(pActFrame+1);

	/* checking */
	if (Category == ACM_CATEGORY_WME)
	{
		if (Action == ACM_ACTION_WME_SETUP_RSP)
			return TRUE;
		/* End of if */
	} /* End of if */

	return FALSE;
} /* End of ACMP_IsResponseFrame */


/*
========================================================================
Routine Description:
	Handle the management action frame.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	SubType			- the subtype of the frame
	pMblk			- the received frame
	PhyRate			- the physical tx rate for the frame

Return Value:
	ACM_RTN_OK		- pMblk is released or forwarded
	ACM_RTN_FAIL	- handle ok and pMblk is not released

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_ManagementHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UINT32					SubType,
	ACM_PARAM_IN	UCHAR					*pMblk,
	ACM_PARAM_IN	UINT32					PktLen,
	ACM_PARAM_IN	UINT32					PhyRate)
{
	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */

	/*
		We can accept request due to ps mode change only
		so we do NOT use ACMR_IS_ENABLED(pAd) here.
	*/
	if (!ACMR_SANITY_CHECK(pAd))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> discard ACM action frame!\n"));
		return ACM_RTN_FAIL;
	} /* End of if */

	if (SubType != ACMR_SUBTYPE_ACTION)
		return ACM_RTN_FAIL; /* not ACTION frame */
	/* End of if */

	/* QAP mode */
#ifdef CONFIG_AP_SUPPORT
	ACM_ActionHandleByQAP(pAd, pCdb, pMblk, PktLen, PhyRate);
#endif // CONFIG_AP_SUPPORT //

	/* QSTA mode */

	return ACM_RTN_OK;
} /* End of ACMP_ManagementHandle */


/*
========================================================================
Routine Description:
	Classify the QoS frame to a AC queue.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the source QSTA
	pMbuf				- the received frame
	QueueTypeCur		- the current used queue type
	FlgIsForceToHighAc	- 1: force the packet to AC3

Return Value:
	Queue Type: AC0 ~ AC3
	not AC0 ~ AC3: can not transmit

Note:
	1. Suppose the Tx Rate is not changed between ACMP_DataPacketQueue()
		and ACMP_MsduClassify().
	2. We re-do ACM control here because maybe OS delay between
		ACMP_DataPacketQueue() and ACMP_MsduClassify().
========================================================================
*/
UINT32 ACMP_MsduClassify(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf,
	ACM_PARAM_IN	UINT32					QueueTypeCur,
	ACM_PARAM_IN	UCHAR					FlgIsForceToHighAc,
	ACM_PARAM_IN	UCHAR					AggType,
	ACM_PARAM_IN	UCHAR					AggId)
{
#ifdef ACM_CC_FUNC_SOFT_ACM
#ifdef ACM_CC_FUNC_QUE_TX_CTRL
	ACM_STREAM *pStream;
	ACM_ENTRY_INFO *pStaAcmInfo;
	ACM_CTRL_PARAM *pEdcaParam;
	ACM_STATISTICS *pStats;

	UCHAR *pPkt; /* same as pMbuf */
	UINT32 PktLen;
	UINT32 TxTime;
	UCHAR  TSID;
#ifndef PERFORMANCE_IMPACT_TEST
	UINT32 PktAcId;
	UINT32 TxQueueType;
#endif // PERFORMANCE_IMPACT_TEST //
	ULONG  SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

    /* sanity check */
    if (pCdb == NULL)
        return QueueTypeCur;
    /* End of if */

	/* init */
	TxTime = RTMP_GET_PACKET_TX_TIME(pMbuf);
	if (TxTime == 0)
		return QueueTypeCur;
	/* End of if */

	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	pStats = &pEdcaParam->Stats;

	pPkt = (UCHAR *)ACMR_WLAN_PKT_GET(pMbuf);
	PktLen = ACMR_WLAN_LEN_GET(pMbuf);

	/* aggregation check */
#ifdef ACM_CC_FUNC_11N
	/*
		WLAN Header + A-MSDU Subframe 1 + A-MSDU Subframe 2 + ...

		A-MSDU Subframe is as below:
			DA(6) + SA(6) + Length(2) + MSDU + Padding (0~3)

		Nothing to do for 1st A-MSDU.
	*/
	if ((AggType == ACMR_AGG_AMSDU) && (AggId > 1))
	{
		/* re-calculate TX time */
#ifdef ACM_CC_FUNC_AUX_TX_TIME
		ACM_TX_TimeCalHT(pAd, NULL,
							PktLen+FRM_LENGTH_AGG_AMSDU_HDR, /* body len */
							McsId,					/* MCS Index */
							ACMR_IS_2040_STA(pCdb),	/* 20 or 20/40 MHz */
							0,				/* regular or short GI */
							0,				/* rts/cts */
							0,				/* no AMPDU or fist pkt in AMPDU */
							1,				/* no ack */
							0xFFFFFFFF,		/* txop limit */
							NULL,			/* no data tx time */
							NULL,			/* wlan header tx time */
							NULL,			/* ack tx time */
							NULL, 			/* data+hdr only tx time */
							&TxTime);		/* data only tx time */
#else
		UINT32 LenDataId;
		UINT32 McsId;

		McsId = ACMR_CLIENT_MCS_GET(pCdb);

		LenDataId = (PktLen+FRM_LENGTH_AGG_AMSDU_HDR);
		LenDataId >>= ACM_PRE_TIME_DATA_SIZE_OFFSET;

//		if (LenDataId > 0)
//			LenDataId --; /* use small length */
		/* End of if */

		if (LenDataId > ACM_PRE_TIME_DATA_SIZE_NUM)
			LenDataId = ACM_PRE_TIME_DATA_SIZE_NUM;
		/* End of if */

		/* data time */
		TxTime = gAcmTxTimeBodyHT\
							[ACMR_IS_2040_STA(pCdb)][0][McsId][LenDataId][2];
#endif // ACM_CC_FUNC_AUX_TX_TIME //
	} /* End of if */
#endif // ACM_CC_FUNC_11N //

	if (AggType == ACMR_AGG_RALINK)
	{
		/* TODO */
	} /* End of if */

	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_CLSFY_NOT_ALLOW);

	/* get TSPEC */
	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);
	TSID = RTMP_GET_PACKET_STM_TSID(pMbuf);
	pStream = (ACM_STREAM *)pStaAcmInfo->pAcStmOut[TSID];

	/* check if tx time > TXOP limit for AC2 & AC3 */
	if (pStream != NULL)
	{
		UINT64 Timestamp, TimeOffset;
		UINT32 TimeAllowed;


		/* get 64-bit Timestamp */
		ACMR_TIMESTAMP_GET(pAd, Timestamp);

		/* translate MediumTime (unit: 32us) to us */
		ACMR_ALLOWED_TIME_GET(pStream, TimeAllowed);

		TimeOffset = (UINT64)((INT64)Timestamp - \
								(INT64)pStream->TxTimestampMarkTransmit);

		if (TimeOffset >= ACM_TIME_BASE)
		{
			/* the first packet in next second */
			pStream->TxTimestampMarkTransmit = Timestamp;

			/* reset the used time */
			if (pStream->AcmUsedTimeTransmit > TimeAllowed)
				pStream->AcmUsedTimeTransmit -= TimeAllowed;
			else
				pStream->AcmUsedTimeTransmit = 0;
			/* End of if */
		}
		else
		{
#ifndef PERFORMANCE_IMPACT_TEST
			/* during a second */
			if (pStream->AcmUsedTimeTransmit > TimeAllowed)
			{
				/* can not tx the packet to the AC so check downgrade flag */
				PktAcId = pStream->AcmAcId;

				if (ACMR_CB->EdcaCtrlParam.DowngradeAcNum[PktAcId] != \
														ACM_DOWNGRADE_DISABLE)
				{
					TxQueueType = ACM_TxQueueTypeGet( \
							ACMR_CB->EdcaCtrlParam.DowngradeAcNum[PktAcId]);

					ACM_STATS_COUNT_INC(pStats->Downgrade[PktAcId]);

					/* need to change the UP of QoS Control field */
					TSID = gEDCA_AC_UP[pEdcaParam->DowngradeAcNum[PktAcId]];
					ACMR_PKT_MARK_UP(pMbuf, TSID);

					goto LabelDowngrade;
				} /* End of if */

				ACM_STATS_COUNT_INC(pStats->DropByAdmittedTime);

				/* downgrade function is disabled so discarding the packet */
				goto LabelDiscard;
			} /* End of if */
#endif // PERFORMANCE_IMPACT_TEST //
		} /* End of if */

		/* accumulate used time */
		pStream->AcmUsedTimeTransmit += TxTime;
	} /* End of if */

	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return QueueTypeCur;

LabelDowngrade:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return TxQueueType;

LabelDiscard:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return ACM_CLSFY_NOT_ALLOW;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! MsduClassify()\n"));
	return ACM_CLSFY_NOT_ALLOW;
#else

	/* no ACM in packet dequeue */
	return QueueTypeCur;
#endif // ACM_CC_FUNC_QUE_TX_CTRL //

#else

	/* no hardware ACM */
	return QueueTypeCur;
#endif // ACM_CC_FUNC_SOFT_ACM //
} /* End of ACMP_MsduClassify */


/*
========================================================================
Routine Description:
	Get new adjust parameters for non-ACM AC.

Arguments:
	pAd				- WLAN control block pointer
	*pEdcaParam		- the parameters

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
	Only for QAP.
========================================================================
*/
VOID ACMP_NonAcmAdjustParamUpdate(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pEdcaParam)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_CHAN_UTIL_MONITOR

	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (!ACMR_SANITY_CHECK(pAd))
		return;
	/* End of if */

	if (pEdcaParam == NULL)
		return;
	/* End of if */

	if (ACMR_CB->CU_MON_FlgChangeNeed != 0)
	{
		/* update WMM Parameters to AP CSR */
		EDCA_AC_CFG_STRUC CsrCfgAc0, CsrCfgAc1, CsrCfgAc2, CsrCfgAc3;
		AIFSN_CSR_STRUC CsrAifsn;
		UCHAR AIFSN[ACM_DEV_NUM_OF_AC];


		ACMR_MEM_COPY(AIFSN, ACMR_CB->CU_MON_AifsnAp, ACM_DEV_NUM_OF_AC);

		CsrAifsn.field.Aifsn0 = AIFSN[0];
		CsrAifsn.field.Aifsn1 = AIFSN[1];
		CsrAifsn.field.Aifsn2 = AIFSN[2];
		CsrAifsn.field.Aifsn3 = AIFSN[3];

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, CsrAifsn.word);

		RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &CsrCfgAc0.word);
		CsrCfgAc0.field.Aifsn = AIFSN[0];
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, CsrCfgAc0.word);

		RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &CsrCfgAc1.word);
		CsrCfgAc1.field.Aifsn = AIFSN[1];
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, CsrCfgAc1.word);

		RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &CsrCfgAc2.word);
		CsrCfgAc2.field.Aifsn = AIFSN[2];
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, CsrCfgAc2.word);

		RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &CsrCfgAc3.word);
		CsrCfgAc3.field.Aifsn = AIFSN[3];
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, CsrCfgAc3.word);
	} /* End of if */

	/* update to BSS beacon */
	ACMR_MEM_COPY(pEdcaParam, ACMR_CB->CU_MON_AifsnBss, ACM_DEV_NUM_OF_AC);
	ACMR_CB->CU_MON_FlgChangeNeed = 0;

#endif // ACM_CC_FUNC_CHAN_UTIL_MONITOR //
#endif // CONFIG_AP_SUPPORT //
} /* End of ACMP_NonAcmAdjustParamUpdate */


/*
========================================================================
Routine Description:
	Get current number of input TSPEC for the UP.

Arguments:
	*pCdb			- the QSTA
	UP				- the UP

Return Value:
	Number of TSPEC

Note:
========================================================================
*/
UINT32 ACMP_NumOfAcTspecInGet(
	ACM_PARAM_IN		ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN		UCHAR				UP)
{
	UINT32 AcId = ACM_MR_EDCA_AC(UP);


	return pCdb->ACM_NumOfInTspecInAc[AcId];
} /* End of ACMP_NumOfAcTspecInGet */

/*
========================================================================
Routine Description:
	Get current number of output TSPEC for the UP.

Arguments:
	*pCdb			- the QSTA
	UP				- the UP

Return Value:
	Number of TSPEC

Note:
========================================================================
*/
UINT32 ACMP_NumOfAcTspecOutGet(
	ACM_PARAM_IN		ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN		UCHAR				UP)
{
	UINT32 AcId = ACM_MR_EDCA_AC(UP);


	return pCdb->ACM_NumOfOutTspecInAc[AcId];
} /* End of ACMP_NumOfAcTspecOutGet */


/*
========================================================================
Routine Description:
	Check if the current tx PHY Mode and MCS > minimum PHY Mode and MCS.

Arguments:
	pAd				- WLAN control block pointer
	*pMbuf			- the frame expected to transmit
	FlgIs2040		- 1: the packet uses 40MHz
	FlgIsShortGI	- 1: the packet uses Short GI
	PhyMode			- the PHY Mode expected to use
	Mcs				- the MCS expected to use

Return Value:
	ACM_RTN_OK		- current Mode & MCS is allowed
	ACM_RTN_FAIL	- current Mode & MCS is not allowed

Note:
========================================================================
*/
ACM_FUNC_STATUS ACMP_PacketPhyModeMCSCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf,
	ACM_PARAM_IN	UCHAR					FlgIs2040,
	ACM_PARAM_IN	UCHAR					FlgIsShortGI,
	ACM_PARAM_IN	UCHAR					PhyMode,
	ACM_PARAM_IN	UCHAR					Mcs)
{

	UINT32 RateMin, Rate;
	UCHAR PhyModeMin, McsMin;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* get minimum rate information from the packet */
	ACMR_PKT_MIN_PHY_MODE_GET(pMbuf, PhyModeMin);

	if (PhyModeMin == ACMR_PHY_NONE)
	{
		/* no minimum rate requirement for the packet, do NOT need to check */
		return ACM_RTN_OK;
	} /* End of if */

	/* init */
	ACMR_PKT_MIN_PHY_MCS_GET(pMbuf, McsMin);

	/* get minimum physical rate */
	switch(PhyModeMin)
	{
		case ACMR_PHY_CCK:
			if (McsMin <= ACM_CCK_LPM_MAX_MCS)
			{
				/* long preamble */
				RateMin = gAcmMCS_CCK[0][McsMin][1];
			}
			else if (McsMin <= ACM_CCK_SPM_MAX_MCS)
			{
				/* short preamble */
				RateMin = gAcmMCS_CCK[1][McsMin - ACM_CCK_SPM_MIN_MCS][1];
			}
			else
				RateMin = gAcmMCS_CCK[0][0][1];
			/* End of if */
			break;

		case ACMR_PHY_OFDM:
			RateMin = gAcmMCS_OFDM[McsMin][1];
			break;

#ifdef ACM_CC_FUNC_11N
		case ACMR_PHY_HT:
			/* always use regular GI */
			RateMin = gAcmMCS_HT[FlgIs2040][0][McsMin];
			break;
#endif // ACM_CC_FUNC_11N //

		default:
			/* no minimum rate requirement for the packet, do NOT need to check */
			return ACM_RTN_OK;
	} /* End of switch */

	/* get current physical rate */
	switch(PhyMode)
	{
		case ACMR_PHY_CCK:
			if (Mcs <= ACM_CCK_LPM_MAX_MCS)
			{
				/* long preamble */
				Rate = gAcmMCS_CCK[0][Mcs][1];
			}
			else if (Mcs <= ACM_CCK_SPM_MAX_MCS)
			{
				/* short preamble */
				Rate = gAcmMCS_CCK[1][Mcs - ACM_CCK_SPM_MIN_MCS][1];
			}
			else
				Rate = gAcmMCS_CCK[0][0][1];
			/* End of if */
			break;

		case ACMR_PHY_OFDM:
			Rate = gAcmMCS_OFDM[Mcs][1];
			break;

#ifdef ACM_CC_FUNC_11N
		case ACMR_PHY_HT:
			Rate = gAcmMCS_HT[FlgIs2040][FlgIsShortGI][Mcs];
			break;
#endif // ACM_CC_FUNC_11N //

		default:
			/* no minimum rate requirement for the packet, do NOT need to check */
			return ACM_RTN_OK;
	} /* End of switch */

	/* check if current used rate is not smaller than the minimum rate */
	if (Rate < RateMin)
		return ACM_RTN_FAIL;
	/* End of if */

	return ACM_RTN_OK;
} /* End of ACMP_PacketPhyModeMCSCheck */


/*
========================================================================
Routine Description:
	Return power save right to system.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	1. Only for STATION mode.
	2. We will return PS right when no any pending ADDTS request frame.
========================================================================
*/
VOID ACMP_StaPsCtrlRightReturn(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{

	return;
} /* End of ACMP_StaPsCtrlRightReturn */


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Used to signal WMM ACM Non-ACM TSPEC response support in the AP.

Arguments:
	pAd				- WLAN control block pointer
	*pElementWme	- the WMM Parameter IE

Return Value:
	None

Note:
	Used in QAP.

	Now that we've agreed that WMM-AC APs shall respond to/accept TSPEC
	requests on non-ACM ACs, how about setting b6 of the "Reserved" octet
	in the WMM Parameter IE .
========================================================================
*/
VOID ACMP_NullTspecSupportSignal(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN_OUT	UCHAR					*pElementWme)
{
	if (!ACMR_SANITY_CHECK(pAd))
		return;
	/* End of if */

	pElementWme[9] |= 0x40; /* bit 6 in reserved field */
} /* End of ACMP_NullTspecSupportSignal */


/*
========================================================================
Routine Description:
	Update UAPSD states after ADDTS Response or DELTS is sent out.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA
	*pActFrameBody	- the action frame

Return Value:
	None

Note:
	Used in QAP.
========================================================================
*/
ACM_EXTERN VOID ACMP_PsRspDeltsSentOutHandle(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN		ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN_OUT	UCHAR				*pActFrameBody)
{
	ACM_WME_NOT_FRAME *pNotFrame;
	ACM_STREAM **ppAcmStmList, *pStream;
	UINT32 TID, Direction;
	UCHAR AcId, UP;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if ((pCdb == NULL) || (!ACMR_SANITY_CHECK(pAd)))
		return;
	/* End of if */

	/* get TSID & Direction */
	pNotFrame = (ACM_WME_NOT_FRAME *)pActFrameBody;

	if (pNotFrame->Category != ACM_CATEGORY_WME)
		return; /* not WME action frame */
	/* End of if */

	/* DELTS */
	if (pNotFrame->Action == ACM_ACTION_WME_TEAR_DOWN)
	{
		ACM_TS_INFO TsInfo;

		/* get TID from the DELTS frame */
		ACMR_MEM_ZERO(&TsInfo, sizeof(TsInfo));
		TsInfo.TSID = pNotFrame->ElmTspec.Tspec.TsInfo.TID;

		/* handle the DELTS frame */
		ACMP_DeltsFrameACK(pAd, ACMR_CLIENT_MAC(pCdb), (UCHAR *)&TsInfo, 0);
		ACM_FrameBwAnnSend(pAd, FALSE);
		return; /* handle DELTS ok for power-save station */
	} /* End of if */

	/* ADDTS Response */
	if (pNotFrame->StatusCode != WLAN_STATUS_CODE_WME_ACM_ACCEPTED)
		return; /* only care about SUCCESS */
	/* End of if */

	Direction = pNotFrame->ElmTspec.Tspec.TsInfo.Direction;

	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* check if we need to update UAPSD state after the RESPONSE is sent out */
	if ((Direction == ACM_DIRECTION_BIDIREC_LINK) ||
		(Direction == ACM_DIRECTION_DOWN_LINK))
	{
		ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
						pAd, ACMR_CLIENT_MAC(pCdb), ACM_PEER_TSPEC_OUTPUT_GET);
	}
	else
	{
		ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
						pAd, ACMR_CLIENT_MAC(pCdb), ACM_PEER_TSPEC_INPUT_GET);
	} /* End of if */

	if (ppAcmStmList == NULL)
		goto LabelOK;
	/* End of if */

	TID = pNotFrame->ElmTspec.Tspec.TsInfo.TID;
	pStream = ppAcmStmList[TID];

	if ((pStream != NULL) && (pStream->FlgUapsdHandleNeed))
	{
		/* we need to reset UAPSD state for the PS TSPEC */
		UP = pNotFrame->ElmTspec.Tspec.TsInfo.UP;

		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> ADDTS Rsp is sent! (TID=%d, DIR=%d, UP=%d)\n",
					TID, Direction, UP));

		AcId = ACM_MR_EDCA_AC(UP);

		ACM_APSD_Ctrl(pAd, pCdb, AcId,
						pStream->pTspec->TsInfo.Direction,
						1, pStream->pTspec->TsInfo.APSD);

		pStream->FlgUapsdHandleNeed = 0; /* handle ok */
	} /* End of if */

LabelOK:
	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
    return;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! ACMP_PsRspDeltsSentOutHandle()\n"));
	return;
} /* End of ACMP_PsRspDeltsSentOutHandle */


/*
========================================================================
Routine Description:
	Handle the resource allocation.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	*pBufRscReq		- the buffer which includes the TSPEC request
	*pBufRscRsp		- the buffer where we can put TSPEC response
	*pBufRspLen		- the respone frame length

Return Value:
	ACM_RTN_OK		- handle ok
	ACM_RTN_FAIL	- handle fail

Note:
	1. Used in QAP.
	2. Currently only TSPEC element for request/response, no TCLAS.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_ResourceAllocate(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pBufRscReq,
	ACM_PARAM_OUT	UCHAR				*pBufRscRsp,
	ACM_PARAM_OUT	UINT32				*pBufRspLen)
{
	ACM_ELM_WME_TSPEC *pElmWmeTspec;
	ACM_TCLAS *pTclas[ACM_TSPEC_TCLAS_MAX_NUM];
	ACM_TSPEC Tspec;
	UINT32 TclasNum;
	UCHAR TclasProcessing;
	UCHAR StatusCode;
	UINT16 MediumTime;
	ACM_FUNC_STATUS RtnCode;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	pElmWmeTspec = (ACM_ELM_WME_TSPEC *)pBufRscReq;

	if ((pElmWmeTspec->ElementId != ACM_ELM_WME_ID) ||
		(pElmWmeTspec->OUI[0] != ACM_WME_OUI_0) ||
		(pElmWmeTspec->OUI[1] != ACM_WME_OUI_1) ||
		(pElmWmeTspec->OUI[2] != ACM_WME_OUI_2) ||
		(pElmWmeTspec->OUI_Type != ACM_WME_OUI_TYPE) ||
		(pElmWmeTspec->OUI_SubType != ACM_WME_OUI_SUBTYPE_TSPEC) ||
		(pElmWmeTspec->Version != ACM_WME_OUI_VERSION))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Not WME TSPEC! ACMP_ResourceAllocate()\n"));
		goto label_fail; /* sanity check fail */
	} /* End of if */

	/* translate WME TSPEC to 11e TSPEC */
	/* skip 4B Category, action, DialogToken, & StatusCode */
	if (ACM_WME_11E_TSPEC_TCLAS_Translate(
										pBufRscReq,
										4+sizeof(ACM_ELM_WME_TSPEC),
										&Tspec,
										pTclas,
										&TclasNum,
										&TclasProcessing) != ACM_RTN_OK)
	{
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> Translate TSPEC fail! "
					"ACMP_ResourceAllocate()\n"));
		goto label_fail; /* translate fail */
	} /* End of if */

	/* handle the request */
	RtnCode = ACM_TC_ReqHandle(
							pAd, pCdb, ACM_STREAM_TYPE_WIFI,
							0, &Tspec,
							TclasNum, pTclas,
							TclasProcessing,
							0, &StatusCode, &MediumTime);

	if (RtnCode != ACM_RTN_OK)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> A WME Setup request is not allowed %d! "
					"ACMP_ResourceAllocate()\n", RtnCode));
		goto label_fail; /* allocate fail */
	}
	else
		pElmWmeTspec->Tspec.MediumTime = MediumTime;
	/* End of if */

	/* copy response content */
	ACMR_MEM_COPY(pBufRscRsp, pElmWmeTspec, sizeof(ACM_ELM_WME_TSPEC));
	*pBufRspLen = sizeof(ACM_ELM_WME_TSPEC);

	/* send a broadcast private ACTION frame to advise used ACM time in AP */
	ACM_FrameBwAnnSend(pAd, FALSE);

	return ACM_RTN_OK;

label_fail:
	/* copy response content */
	ACMR_MEM_COPY(pBufRscRsp, pElmWmeTspec, sizeof(ACM_ELM_WME_TSPEC));
	*pBufRspLen = sizeof(ACM_ELM_WME_TSPEC);

	return ACM_RTN_FAIL;
} /* End of ACMP_ResourceAllocate */


/*
========================================================================
Routine Description:
	Update the UAPSD state based on current all TSPECs.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA

Return Value:
	None

Note:
	Used in QAP.

	Use the function after reassociation request.

	Because TSPEC is not deleted after reassociation request, we need
	to update new UAPSD state based on these TSPEC and recover to some
	static settings in reassociation frame after any TSPEC is deleted.
========================================================================
*/
VOID ACMP_UAPSD_StateUpdate(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb)
{
	ACM_STREAM **ppAcmStmList;
	ACM_TS_INFO *pTsInfo;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UINT32 IdTidNum, IdLinkNum;
	UCHAR AcId, Direction, UP, APSD;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if ((pCdb == NULL) || (!ACMR_SANITY_CHECK(pAd)))
		return;
	/* End of if */

	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* check all output and input streams for the peer device */
	for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
	{
		ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
						pAd, ACMR_CLIENT_MAC(pCdb), DirectionId[IdLinkNum]);
		if (ppAcmStmList == NULL)
			break;
		/* End of if */

		for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
		{
			if (ppAcmStmList[IdTidNum] != NULL)
			{
				pTsInfo = &(ppAcmStmList[IdTidNum]->pTspec->TsInfo);

				Direction = pTsInfo->Direction;
				UP = pTsInfo->UP;
				APSD = pTsInfo->APSD;

				AcId = ACM_MR_EDCA_AC(UP);

				if (Direction == ACM_DIRECTION_BIDIREC_LINK)
				{
					pCdb->bAPSDCapablePerAC[AcId] = APSD;
					pCdb->bAPSDDeliverEnabledPerAC[AcId] = APSD;
				}
				else if (Direction == ACM_DIRECTION_DOWN_LINK)
				{
					pCdb->bAPSDDeliverEnabledPerAC[AcId] = APSD;
				}
				else if (Direction == ACM_DIRECTION_UP_LINK)
				{
					pCdb->bAPSDCapablePerAC[AcId] = APSD;
				} /* End of if */
			} /* End of if */
		} /* End of for */
	} /* End of while */

	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);

    if ((pCdb->bAPSDDeliverEnabledPerAC[QID_AC_BE] == 1) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_BK] == 1) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_VI] == 1) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_VO] == 1))
    {
        /* all AC are U-APSD delivery-enabled */
        pCdb->bAPSDAllAC = 1;

		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> all AC are UAPSD!\n"));
    }
    else
    {
        /* at least one AC is not U-APSD delivery-enabled */
        pCdb->bAPSDAllAC = 0;

		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> not all AC are UAPSD!\n"));
    } /* End of if */

    return;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! ACMP_UAPSD_StateUpdate()\n"));
	return;
} /* End of ACMP_UAPSD_StateUpdate */
#endif // CONFIG_AP_SUPPORT //


/*
========================================================================
Routine Description:
	Get ACM related statistics counts.

Arguments:
	pAd				- WLAN control block pointer
	*pStats			- the statistics counts

Return Value:
	None

Note:
========================================================================
*/
VOID ACMP_StatisticsGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_OUT	ACM_STATISTICS			*pStats)
{
	ACM_CTRL_PARAM *pEdcaParam;


	/* sanity check for WMM */
	if (!ACMR_SANITY_CHECK(pAd))
		return;
	/* End of if */

	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	*pStats = pEdcaParam->Stats;
} /* End of ACMP_StatisticsGet */


/*
========================================================================
Routine Description:
	Delete a QSTA due to deauthentication or deassociation, etc.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA

Return Value:
	None

Note:
	1. Used in QAP and QSTA.
	2. Before any entry deletion, you must call the function to release TS first.
========================================================================
*/
VOID ACMP_StationDelete(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb)
{
	ACM_ENTRY_INFO *pStaAcmInfo;
	ACM_STREAM *pStream;
	UINT32 IdTidNum;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");


	/* sanity check for ACM */
	if ((pCdb == NULL) || (!ACMR_SANITY_CHECK(pAd)))
		return;
	/* End of if */

    ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Station associates or is deleted! "
                "ACMP_StationDelete()\n"));

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* free all requested TSPECs for the device entry if exists */
	ACM_TC_ReqDeviceFree(pAd, pCdb);

	/* free all TSPECs silently without sending DELTS frames */
	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);

	for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
	{
		/* for OUT stream */
		pStream = (ACM_STREAM *)(pStaAcmInfo->pAcStmOut[IdTidNum]);

		if (pStream != NULL)
			ACM_TC_Discard(pAd, pStream);
		/* End of if */

		pStaAcmInfo->pAcStmOut[IdTidNum] = NULL;

		/* for IN stream */
		pStream = (ACM_STREAM *)pStaAcmInfo->pAcStmIn[IdTidNum];

		if (pStream != NULL)
			ACM_TC_Discard(pAd, pStream);
		/* End of if */

		pStaAcmInfo->pAcStmIn[IdTidNum] = NULL;
	} /* End of for */

	/* free the peer device record ever reserving bandwidth */
	ACM_PeerDeviceDel(pAd, ACMR_CLIENT_MAC(pCdb));

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	/* announce new bandwidth */
	ACM_FrameBwAnnSend(pAd, FALSE);
	return;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! StationDelete()\n"));
	return;
} /* End of ACMP_StationDelete */


/*
========================================================================
Routine Description:
	Clear failed stream information.

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID ACMP_StreamFailClear(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	ACM_TSPEC_REQ_LIST *pTspecFreedList;
	ULONG SplFlags;


	/* sanity check */
	if (!ACMR_IS_ENABLED(pAd))
	{
	    ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> WMM ACM is disabled! StreamFailClear()\n"));
		return;
	} /* End of if */

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* free "all" failed stream records */
	pTspecFreedList = &ACMR_CB->TspecListFail;
	ACM_LIST_ALL_FREE(pAd, pTspecFreedList);

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
	return;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! StreamFailClear()\n"));
	return;
} /* End of ACMP_StreamFailClear */


/*
========================================================================
Routine Description:
	Get some streams information.

Arguments:
	pAd				- WLAN control block pointer
	Category		-	ACM_STM_CATEGORY_REQ,
						ACM_STM_CATEGORY_ACT,
						ACM_SM_CATEGORY_PEER,
						ACM_STM_CATEGORY_ERR
	Type			- ACM_ACCESS_POLICY_EDCA
	*pNumStm		- the number of streams you want, must > 0
	*pStaMac		- the QSTA MAC
	*pStreamBuf		- the stream information buffers

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- no more stream

Note:
	1. if pStream->pTspec == NULL, the function will not copy
		TSPEC information.
	2. if pStream->pTclas[i] == NULL, the function will not
		copy TCLAS information.
	3. If you want to get all stream information, you shall call
		ACMP_StreamNumGet() first.
========================================================================
*/
ACM_FUNC_STATUS ACMP_StreamsGet(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN		UINT32					Category,
	ACM_PARAM_IN		UINT32					Type,
	ACM_PARAM_IN_OUT	UINT32					*pNumStm,
	ACM_PARAM_IN		UCHAR					*pStaMac,
	ACM_PARAM_OUT		ACM_STREAM_INFO			*pStreamBuf)
{
	ACM_PEER_DEV_LIST *pAcmDevList;
	ACM_STREAM *pStream, **ppAcmStmList;
	UINT32 NumWant, NumActualGot;
	UINT32 IdTidNum, IdLinkNum;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UCHAR MAC[ACM_MAC_ADDR_LEN];
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if ((*pNumStm) == 0)
		return ACM_RTN_FAIL;
	/* End of if */

	/* init */
	pStream = NULL;
	NumWant = *pNumStm;
	NumActualGot = 0;

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	/* copy stream information */
	switch(Category)
	{
		case ACM_SM_CATEGORY_REQ: /* requested list */
			pStream = ACMR_CB->TspecListReq.pHead;

			while((pStream != NULL) && (NumActualGot < NumWant))
			{
				ACM_STM_InfoCopy(&pStreamBuf[NumActualGot++], pStream);
				pStream = pStream->pNext;
			} /* End of while */
			break;

		case ACM_SM_CATEGORY_ACT: /* output links of 'all' peers */
			pAcmDevList = NULL;

			while(1)
			{
				/* get next device */
				if (ACM_PeerDeviceGetNext(pAd, &pAcmDevList, MAC) != ACM_RTN_OK)
					break;
				/* End of if */

				/* copy output and input TS information */
				ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
										pAd, MAC, ACM_PEER_TSPEC_OUTPUT_GET);

				if (ppAcmStmList == NULL)
					continue;
				/* End of if */

				for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
				{
					if (NumActualGot >= NumWant)
						break;
					/* End of if */

					if (ppAcmStmList[IdTidNum] != NULL)
					{
						ACM_STM_InfoCopy(&pStreamBuf[NumActualGot++],
										ppAcmStmList[IdTidNum]);
					} /* End of if */
				} /* End of for */
			}  /* End of while */
			break;

		case ACM_SM_CATEGORY_PEER: /* input and output links of a peer */
			/* 2 links: input and output TSPEC */
			for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
			{
				ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
										pAd, pStaMac, DirectionId[IdLinkNum]);

				if (ppAcmStmList == NULL)
					break;;
				/* End of if */

				for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
				{
					if (NumActualGot >= NumWant)
						break;
					/* End of if */

					if (ppAcmStmList[IdTidNum] != NULL)
					{
						if ((IdLinkNum == 1) &&
							(ppAcmStmList[IdTidNum]->pTspec->TsInfo.Direction == \
													ACM_DIRECTION_BIDIREC_LINK))
						{
							/* for bidirectional link, only copy one */
							continue;
						} /* End of if */

						ACM_STM_InfoCopy(&pStreamBuf[NumActualGot++],
										ppAcmStmList[IdTidNum]);
					} /* End of if */
				} /* End of for */
			} /* End of for */
			break;

		case ACM_SM_CATEGORY_ERR: /* failed list */
			pStream = ACMR_CB->TspecListFail.pHead;

			while((pStream != NULL) && (NumActualGot < NumWant))
			{
				ACM_STM_InfoCopy(&pStreamBuf[NumActualGot++], pStream);
				pStream = pStream->pNext;
			} /* End of while */
			break;

		default:
			goto LabelErr;
	} /* End of switch */

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	/* return actual got number */
	*pNumStm = NumActualGot;
	return ACM_RTN_OK;

LabelErr:
	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
	return ACM_RTN_FAIL;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! StreamsGet()\n"));
	return ACM_RTN_FAIL;
} /* End of ACMP_StreamsGet */


/*
========================================================================
Routine Description:
	Get the number of streams.

Arguments:
	pAd				- WLAN control block pointer
	Category		-	ACM_STM_CATEGORY_REQ,
						ACM_STM_CATEGORY_ACT,
						ACM_SM_CATEGORY_PEER,
						ACM_STM_CATEGORY_ERR
	Type			- ACM_ACCESS_POLICY_EDCA
	*pStaMac		- the QSTA MAC

Return Value:
	current number of streams

Note:
========================================================================
*/
UINT32 ACMP_StreamNumGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					Category,
	ACM_PARAM_IN	UINT32					Type,
	ACM_PARAM_IN	UCHAR					*pStaMac)
{
	ACM_PEER_DEV_LIST *pAcmDevList;
	ACM_STREAM *pStream, **ppAcmStmList;
	UINT32 NumStream, IdTidNum, IdLinkNum;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UCHAR MAC[ACM_MAC_ADDR_LEN];
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	NumStream = 0;

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, 0);

	switch(Category)
	{
		case ACM_SM_CATEGORY_REQ: /* requested list */
			pStream = ACMR_CB->TspecListReq.pHead;

			while(pStream != NULL)
			{
				NumStream ++;
				pStream = pStream->pNext;
			} /* End of while */
			break;

		case ACM_SM_CATEGORY_ACT: /* output links of all peers */
			pAcmDevList = NULL;

			while(1)
			{
				/* get next device */
				if (ACM_PeerDeviceGetNext(pAd, &pAcmDevList, MAC) != ACM_RTN_OK)
					break;
				/* End of if */

				/* copy output TS information */
				ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
										pAd, MAC, ACM_PEER_TSPEC_OUTPUT_GET);

				if (ppAcmStmList == NULL)
					continue;
				/* End of if */

				for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
				{
					if (ppAcmStmList[IdTidNum] != NULL)
						NumStream++;
					/* End of if */
				} /* End of for */
			}  /* End of while */
			break;

		case ACM_SM_CATEGORY_PEER: /* input and output links of a peer */
			for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
			{
				ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
										pAd, pStaMac, DirectionId[IdLinkNum]);

				if (ppAcmStmList == NULL)
					break;
				/* End of if */

				for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
				{
					if (ppAcmStmList[IdTidNum] != NULL)
					{
						if ((IdLinkNum == 1) &&
							(ppAcmStmList[IdTidNum]->pTspec->TsInfo.Direction == \
													ACM_DIRECTION_BIDIREC_LINK))
						{
							/* for bidirectional link, only copy once */
							continue;
						} /* End of if */

						NumStream++;
					} /* End of if */
				} /* End of for */
			} /* End of for */
			break;

		case ACM_SM_CATEGORY_ERR: /* failed list */
			pStream = ACMR_CB->TspecListFail.pHead;

			while(pStream != NULL)
			{
				NumStream ++;
				pStream = pStream->pNext;
			} /* End of while */
			break;
	} /* End of switch */

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	return NumStream;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! StreamNumGet()\n"));
	return 0;
} /* End of ACMP_StreamNumGet */


/*
========================================================================
Routine Description:
	Delete all activated TSPECs.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	1. Send a DELTS to the QSTA or QAP.
	2. Insert the activated TSPEC to the requested list.
	3. The TSPEC will be moved to the failed list when DELTS ACK
		is received or retry count is reached.
	4. Can not used in disassociation.
========================================================================
*/
VOID ACMP_TC_DeleteAll(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	ACM_STREAM **ppAcmStmList, *pStream;
	ACM_PEER_DEV_LIST *pAcmDevList;
	UINT32 IdTidNum, IdLinkNum;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UCHAR MAC[ACM_MAC_ADDR_LEN];
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pAcmDevList = NULL;

	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* clear all requested TSPEC if exists */
	ACM_TC_ReqAllFree(pAd);

	/* delete all output and input links for all device entries */
	while(1)
	{
		/* get next device entry */
		if (ACM_PeerDeviceGetNext(pAd, &pAcmDevList, MAC) != ACM_RTN_OK)
			break; /* no other device */
		/* End of if */

		/* delete all input and output streams */
		for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
		{
			ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
											pAd, MAC, DirectionId[IdLinkNum]);
			if (ppAcmStmList == NULL)
				break;
			/* End of if */

			for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
			{
				pStream = ppAcmStmList[IdTidNum];

				if (pStream != NULL)
				{
#ifdef CONFIG_AP_SUPPORT
					pStream->Cause = ACM_TC_CAUSE_DELETED_BY_QAP;
#endif // CONFIG_AP_SUPPORT //


					if (ACM_TC_Delete(pAd, pStream) == TRUE)
					{
						ACM_DELTS_SEND(pAd, pStream->pCdb,
										pStream, LabelSemErr);
					} /* End of if */

					/* empty the record */
					ppAcmStmList[IdTidNum] = NULL;
				} /* End of if */
			} /* End of for */
		} /* End of for */
	} /* End of while */

	/* free all backup device entries */
	ACM_PeerDeviceAllFree(pAd);

	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! TC_DeleteAll()\n"));
	return;
} /* End of ACMP_TC_DeleteAll */


/*
========================================================================
Routine Description:
	Delete a TSPEC silently.

Arguments:
	pAd				- WLAN control block pointer
	*pMacPeer		- the MAC of peer
	TID				- the TID of the TSPEC

Return Value:
	TRUE			- find it and delete it ok
	FALSE			- do not find it or delete it fail

Note:
	For QAP, the pMacPeer means the MAC of a station;
	For QSTA, the pMacPeer means the MAC of associated AP;

	No DELTS frame is sent.
========================================================================
*/
BOOLEAN ACMP_TC_DeleteOneSilently(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pMacPeer,
	ACM_PARAM_IN	UCHAR					TID)
{
	ACM_STREAM *pStream;
	ACM_TS_INFO TsInfo;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, FALSE);

	/* find the request */
	TsInfo.TSID = TID;

	pStream = ACM_TC_Find(pAd, pMacPeer, &TsInfo);
	if (pStream == NULL)
	{
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_msg> can not find the stream (TID=%d)!\n", TID));
		goto LabelNotFound;
	} /* End of if */

	/* delete the stream */
#ifdef CONFIG_AP_SUPPORT
	pStream->Cause = TSPEC_CAUSE_DELETED_BY_QAP;
#endif // CONFIG_AP_SUPPORT //


	ACM_TC_Destroy(pAd, pStream, 0);

	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return TRUE;

LabelNotFound:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);
	return FALSE;

LabelSemErr:
	return FALSE;
} /* End of ACMP_TC_DeleteOneSilently */


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Enable or disable all TSPEC rejection function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable

Return Value:
	None

Note:
	Only for QAP.
========================================================================
*/
VOID ACMP_TC_RejectCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable)
{
	if (FlgIsEnable)
	{
		ACM_MR_TSPEC_ALLOW();
	}
	else
	{
		/*
			We will not accept any new TSPEC request
			but we will keep old TSPECs.
		*/
		ACM_MR_TSPEC_DISALLOW();
	} /* End of if */
} /* End of ACMP_TC_RejectCtrl */
#endif // CONFIG_AP_SUPPORT //


/*
========================================================================
Routine Description:
	Enable or disable TSPEC timeout function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable

Return Value:
	None

Note:
	Only for QAP.
========================================================================
*/
VOID ACMP_TC_TimeoutCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable)
{
	ACM_CTRL_PARAM  *pEdcaParam;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	pEdcaParam->FlgIsTspecTimeoutEnable = FlgIsEnable;

	/* active stream check timer for any stream */
	ACMR_TIMER_ENABLE(ACMR_CB->FlgStreamAliveCheckEnable,
						ACMR_CB->TimerStreamAliveCheck,
						ACM_STREAM_CHECK_OFFSET);

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> TSPEC timeout mechanism flag = %d!\n", FlgIsEnable));
} /* End of ACMP_TC_TimeoutCtrl */




/* =========================== Global Function (STA) ======================== */



/*
========================================================================
Routine Description:
	Timer APIs are provided for WLAN module use.

Arguments:
	ACM_TIMER_API_PARAM

Return Value:
	None

Note:
========================================================================
*/
VOID ACMP_TR_TC_ReqCheck(ACM_TIMER_API_PARAM)
{
	ACM_TR_TC_ReqCheck(ACM_TIMER_API_DATA);
}


VOID ACMP_TR_STM_Check(ACM_TIMER_API_PARAM)
{
	ACM_TR_STM_Check(ACM_TIMER_API_DATA);
}


VOID ACMP_TR_TC_General(ACM_TIMER_API_PARAM)
{
	ACM_TR_TC_General(ACM_TIMER_API_DATA);
}


VOID ACMP_CMD_Timer_Data_Simulation(ACM_TIMER_API_PARAM)
{
	ACM_CMD_Timer_Data_Simulation(ACM_TIMER_API_DATA);
}


#ifdef CONFIG_STA_SUPPORT_SIM
/*
========================================================================
Routine Description:
	Send a TSPEC request to the QAP.

Arguments:
	pAd						- WLAN control block pointer
	*pCdb					- our STATION entry
	*pTspecSrc				- the requested TSPEC pointer
	TclasNum				- the number of TCLASS, max 5
	*pTclasSrc				- the requested TCLASS array pointer
	TclasProcessing			- 1: must match all TCLAS
	StreamType				- the stream type: WME stream

Return Value:
	ACM_RTN_OK				- request is accepted
	ACM_RTN_FAIL			- semaphore lock fail or others
	ACM_RTN_NULL_POINTER	- null pointer
	ACM_RTN_INVALID_PARAM	- invalid input parameters
	ACM_RTN_SEM_GET_ERR		- get semaphore fail
	ACM_RTN_FATAL_ERR		- can not call the func in error mode
	ACM_RTN_NO_FREE_TS		- no free TS ID or same TSID & Direction
	ACM_RTN_ALLOC_ERR		- TSPEC request structure allocation fail
	ACM_RTN_DISALLOW		- the request is not allowed

Note:
	1. Only for non-IBSS Station Mode.
	2. pTclasSrc is limited by ACM_TSPEC_TCLAS_MAX_NUM.
	3. DLP TSPEC is not allowed but DLP is allowed.
	4. *pTspecSrc & *pTclasSrc[] can not be freed in calling function.
	5. For WMM STA, the used TSPEC is the same.

	ACM_TG_CMT_SPEC_UNCLEAR_ON_RESERVED_FIELD
========================================================================
*/
ACM_FUNC_STATUS ACMP_WME_TC_Request(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	ACM_TSPEC			*pTspecSrc,
	ACM_PARAM_IN	UINT32				TclasNum,
	ACM_PARAM_IN	ACM_TCLAS			*pTclasSrc,
	ACM_PARAM_IN	UCHAR				TclasProcessing,
	ACM_PARAM_IN	UCHAR				StreamType,
	ACM_PARAM_IN	UINT16				DialogToken)
{
	ACM_CTRL_PARAM *pEdcaParam;
	ACM_TCLAS *pTclas;
	ACM_STREAM *pOldStreamIn, *pOldStreamOut, *pOldStreamDiffAc;
	ACM_STREAM *pStreamReq;
	ACM_FUNC_STATUS RtnCode, Status;
	UCHAR  UserPriority;
	ULONG  SplFlags;
	UCHAR *pFrameBuf;
	UINT32 FrameLen;
#ifdef ACM_CC_FUNC_TCLAS
	UINT32 IdTclasNum;
#endif // ACM_CC_FUNC_TCLAS //


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	pTclas = pTclasSrc;
	RtnCode = ACM_RTN_OK;
	UserPriority = ACM_UP_UNKNOWN;

	if (!ACM_MR_TSPEC_IS_ALLOWED(pAd))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> ACM is not allowed!\n"));
		return ACM_RTN_DISALLOW;
	} /* End of if */

	/* check if QSTA is in ASSOCIATION state */
	if (!ACMR_IS_ASSOCIATED(pAd))
	{
		/* QSTA yet associate to the QAP */
		/* QSTA can send ADDTS request only when it associates to a AP */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_err> Station does not yet associate to any AP!\n"));
		return ACM_RTN_FAIL;
	} /* End of if */


#ifndef ACM_CC_FUNC_SPEC_CHANGE_TG
	/* we can send out a TSPEC to change PS mode only */

	/* check if the ACM of all AC is disabled */
	if (ACMP_IsAnyACEnabled(pAd) != ACM_RTN_OK)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> ACM is disabled!\n"));
		return ACM_RTN_DISALLOW;
	} /* End of if */
#endif // ACM_CC_FUNC_SPEC_CHANGE_TG //

	/* sanity check for input parameters of WME STA */
	if (pTspecSrc == NULL)
		return ACM_RTN_NULL_POINTER;
	/* End of if */

	/* sanity check for tx rate */
	if ((pTspecSrc->MinPhyRate > 0) &&
		(pTspecSrc->MeanDataRate > 0) &&
		(pTspecSrc->MinPhyRate <= pTspecSrc->MeanDataRate))
	{
		/* minimum PHY rate must > mean data rate */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_err> Minimum Phy Rate > Mean Data Rate!\n"));
		return ACM_RTN_INVALID_PARAM;
	} /* End of if */

	if (((pTspecSrc->MinDataRate != 0) &&
		(pTspecSrc->MeanDataRate != 0) &&
		(pTspecSrc->MinDataRate > pTspecSrc->MeanDataRate)) ||
		((pTspecSrc->PeakDataRate != 0) &&
		(pTspecSrc->MeanDataRate != 0) &&
		(pTspecSrc->MeanDataRate > pTspecSrc->PeakDataRate)))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_err> Min/Mean Data Rate > Mean/Peak Data Rate!\n"));
		return ACM_RTN_INVALID_PARAM;
	} /* End of if */


#ifdef ACM_CC_FUNC_TCLAS
	/* maximum TCLASS number is limited */
	if (TclasNum > ACM_TSPEC_TCLAS_MAX_NUM)
		return ACM_RTN_INVALID_PARAM;
	/* End of if */

	/* check user priority and get the user priority */
	if ((TclasNum > 0) && (pTclas != NULL))
	{
		/* all TCLASS shall have the same user priority */
		for(IdTclasNum=0; IdTclasNum<TclasNum; IdTclasNum++)
		{
			if (pTclas != NULL)
			{
				if (UserPriority == ACM_UP_UNKNOWN)
					UserPriority = pTclas->UserPriority;
				else
				{
					if (pTclas->UserPriority != UserPriority)
						return ACM_RTN_INVALID_PARAM;
					/* End of if */
				} /* End of if */
			} /* End of if */

			/* check next TCLAS */
			pTclas ++;
		} /* End of for */
	}
	else
#endif // ACM_CC_FUNC_TCLAS //
	{
		/* no any TCLAS exists so use the UP of the TS Info */
		UserPriority = pTspecSrc->TsInfo.UP;
	} /* End of if */

#ifndef ACM_CC_FUNC_SPEC_CHANGE_TG
	/* we can send out a TSPEC to change PS mode only */

	/* check if ACM is needed for the AC */
	if (ACMR_CB->EdcaCtrlParam.FlgAcmStatus[ACM_MR_EDCA_AC(UserPriority)] == 0)
		return ACM_RTN_DISALLOW;
	/* End of if */
#endif // ACM_CC_FUNC_SPEC_CHANGE_TG //

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	/* check the TSID & Direction */
	Status = ACM_TC_RenegotiationCheck(pAd,
									ACMR_CLIENT_MAC(pCdb),
									UserPriority,
									&pTspecSrc->TsInfo,
									&pOldStreamIn,
									&pOldStreamOut,
									&pOldStreamDiffAc);

	if (Status == ACM_RTN_OK)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Already exist same TS! WME_TC_Request()\n"));
		RtnCode = ACM_RTN_EXIST;
		goto LabelErr;
	} /* End of if */

	/* allocate a TSPEC request structure */
	ACMR_MEM_ALLOC(pStreamReq, sizeof(ACM_STREAM), (ACM_STREAM *));
	if (pStreamReq == NULL)
	{
		RtnCode = ACM_RTN_ALLOC_ERR;
		goto LabelErr;
	} /* End of if */

	/* init the TSPEC request structure */
	ACMR_MEM_ZERO(pStreamReq, sizeof(ACM_STREAM));

	ACMR_MEM_ALLOC(pStreamReq->pTspec, sizeof(ACM_TSPEC), (ACM_TSPEC *));
	if (pStreamReq->pTspec == NULL)
	{
		ACMR_MEM_FREE(pStreamReq);
		RtnCode = ACM_RTN_ALLOC_ERR;
		goto LabelErr;
	} /* End of if */

	*pStreamReq->pTspec = *pTspecSrc;
	pTclas = pTclasSrc;

#ifdef ACM_CC_FUNC_TCLAS
	for(IdTclasNum=0; IdTclasNum<TclasNum; IdTclasNum++)
	{
		ACMR_MEM_ALLOC(pStreamReq->pTclas[IdTclasNum],
						sizeof(ACM_TCLAS), (ACM_TCLAS *));

		if (pStreamReq->pTclas[IdTclasNum] == NULL)
		{
			RtnCode = ACM_RTN_ALLOC_ERR;
			TclasNum = IdTclasNum+1;
			goto LabelErrAlloc;
		} /* End of if */

		*pStreamReq->pTclas[IdTclasNum] = *pTclas;
		pTclas ++; /* move to next TCLAS */
	} /* End of for */
#endif // ACM_CC_FUNC_TCLAS //

	pStreamReq->StreamType = StreamType;
	pStreamReq->Status = TSPEC_STATUS_REQUEST;
	pStreamReq->TimeoutAction = ACM_TC_TIMEOUT_ACTION_ADDTS_REQ;
	pStreamReq->TclasProcessing = TclasProcessing;
	pStreamReq->Retry = 0; /* no retry */
	ACM_STREAM_CDB_COPY(pStreamReq, pCdb);
	pStreamReq->UP = UserPriority;

	/* timeout unit: 100ms */
	pStreamReq->Timeout = ACM_ADDTS_REQUEST_TIMEOUT;
	pStreamReq->Timeout = pStreamReq->Timeout * 1000 / ACM_TIMEOUT_CHECK_BASE;

	if (DialogToken > 0)
		pStreamReq->DialogToken = DialogToken; /* debug use */
	else
	{
		TSPEC_DIALOG_TOKEN_GET(pAd, pStreamReq->DialogToken);
	} /* End of if */

	pStreamReq->AcmAcId  = ACM_MR_EDCA_AC(pStreamReq->UP);

	/* check if need to recover UAPSD field of TS Info */
	if (!pEdcaParam->FlgIsTspecUpasdEnable)
	{
		/*
			Does not support the modification of the Power Save settings via
			TSPEC.
		*/
		pStreamReq->pTspec->TsInfo.APSD = \
						pAd->CommonCfg.bACMAPSDBackup[pStreamReq->AcmAcId];

		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Recover APSD to %d! WME_TC_Request()\n",
					pStreamReq->pTspec->TsInfo.APSD));
	} /* End of if */

	/* timeout unit: 100ms */
	pStreamReq->TimeoutAddts = pStreamReq->Timeout;
	pStreamReq->TimeoutDelts = ACM_DELTS_TIMEOUT/ACM_TIMEOUT_CHECK_BASE;

	/* insert the new requested TSPEC to the request list */
	ACM_TC_ReqInsert(pAd, pStreamReq);

	/* check whether the check timer is enabled */
	ACMR_TIMER_ENABLE(ACMR_CB->FlgTspecReqCheckEnable,
						ACMR_CB->TimerTspecReqCheck,
						ACM_STREAM_CHECK_OFFSET);

	/* make up ADDTS Request frame */
	ACM_ADDREQ_MAKEUP(pAd, pCdb, pFrameBuf, FrameLen, pStreamReq, LabelSemErr);

	/* check inactivity timeout */
	if (pStreamReq->pTspec->InactivityInt == 0)
	{
		/*
			We can request a TSPEC with inactivity timeout = 0;
			but infinite inactivity timeout is not good I think,
			so give a default timeout for the TSPEC.
			After ACM_ADDREQ_MAKEUP()
		*/
		pStreamReq->pTspec->InactivityInt = ACM_WME_TSPEC_INACTIVITY_DEFAULT;
	} /* End of if */

	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> A request is created successfully! "
				"ACMP_WME_TC_Request()\n"));


	/* send a ADDTS Request frame */
	ACM_ADDREQ_SEND(pAd, pFrameBuf, FrameLen);

	return RtnCode;

LabelErr:
	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
	return RtnCode;

LabelSemErr:
	/* management semaphore get fail */
	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_err> SemaphoreLock! WME_TC_Request()\n"));
	return ACM_RTN_SEM_GET_ERR;

#ifdef ACM_CC_FUNC_TCLAS
LabelErrAlloc:
	/* release semaphore */
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	ACMR_MEM_FREE(pStreamReq->pTspec);

	for(IdTclasNum=0; IdTclasNum<TclasNum; IdTclasNum++)
		ACMR_MEM_FREE(pStreamReq->pTclas[IdTclasNum]);
	/* End of for */

	ACMR_MEM_FREE(pStreamReq);
	return RtnCode;
#endif // ACM_CC_FUNC_TCLAS //
} /* End of ACMP_WME_TC_Request */

#endif // CONFIG_STA_SUPPORT_SIM //




/* =========================== ASIC Function =========================== */

/*
========================================================================
Routine Description:
	Reset the medium time for the AC.

Arguments:
	pAd				- WLAN control block pointer
	AcId			- the AC ID (0 ~ 3)
	MediumTime		- the new medium time (unit: micro seconds)

Return Value:
	None

Note:
	1. If acm_time == 0, means the ACM for the AC will be disabled.
	2. special mode, when AcId == 0x1234, the settings will be
		applied for all ACs.
========================================================================
*/
STATIC VOID ACM_ASIC_ACM_Reset(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					AcId,
	ACM_PARAM_IN	UINT32					MediumTime)
{
	/* currently no ASIC setting to be set */
} /* End of ACM_ASIC_ACM_Reset */


/*
========================================================================
Routine Description:
	Enable ASIC channel busy time calculation.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: ENABLE; 0:DISABLE

Return Value:
	None

Note:
	1. If acm_time == 0, means the ACM for the AC will be disabled.
	2. special mode, when AcId == 0x1234, the settings will be
		applied for all ACs.
========================================================================
*/
STATIC VOID ACM_ASIC_ChanBusyEnable(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable)
{
	if (FlgIsEnable)
	{
		ACMR_CHAN_BUSY_DETECT_ENABLE(pAd);
	}
	else
	{
		ACMR_CHAN_BUSY_DETECT_DISABLE(pAd);
	} /* End of if */
} /* End of ACM_ASIC_ChanBusyEnable */


/*
========================================================================
Routine Description:
	Get the channel busy time in last TBTT.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	the channel busy time, unit: miscroseconds

Note:
========================================================================
*/
STATIC UINT32 ACM_ASIC_ChanBusyGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	UINT32 TimeBusy;


	TimeBusy = 0;
	ACMR_CHAN_BUSY_GET(pAd, TimeBusy);
	return TimeBusy;
} /* End of ACM_ASIC_ChanBusyGet */


/*
========================================================================
Routine Description:
	Reset the WLAN QOS ASIC EDCA setting.

Arguments:
	pAd					- WLAN control block pointer
	FlgIsDeltsAll		- 1: delete all TSPECs; 0: reserve all TSPECs

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_ASIC_EDCA_Reset(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsDeltsAll)
{
	/* no ASIC settings are needed to be set */

	/* delete all activated TSPEC */
	if (FlgIsDeltsAll == 1)
		ACMP_TC_DeleteAll(pAd);
	/* End of if */
} /* End of ACM_ASIC_EDCA_Reset */


/*
========================================================================
Routine Description:
	Translate TUs into microseconds.

Arguments:
	pAd				- WLAN control block pointer
	TU				- the time unit

Return Value:
	microseconds

Note:
========================================================================
*/
STATIC UINT32 ACM_ASIC_TU_Translate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					TU)
{
	UINT32 Unit;


	/* get a TU */
	Unit = 1024; /* 1024 micro second (us) */

	/* calculate TUs */
	TU *= Unit;
	return TU;
} /* End of ACM_ASIC_TU_Translate */




/* =========================== OTHER Function =========================== */

/*
========================================================================
Routine Description:
	Handle UAPSD enable or disable.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the client database
	StmAcId				- the AC ID (0 ~ 3)
	FlgTsAdd			- 1: add a TS; 0: delete a TS
	FlgIsApsdEnable		- 1: enable APSD in TSPEC

Return Value:
	None

Note:
	WMM-PS is optional for WMM-AC certification.

	RTMP_IRQ_LOCK will be used in the function so you can not put the function
	in any bottom half lock section.

	When FlgTsAdd == 0, FlgIsApsdEnable is not used.

	An uplink TSPEC plus a downlink TSPEC, or a bi-directional TSPEC with the
	APSD subfield set to 1 and the Schedule subfield set to 0, makes an AC both
	trigger- and delivery-enabled.

	An uplink TSPEC plus a downlink TSPEC, or a bi-directional TSPEC with the
	APSD and the Schedule subfields both set to 0, makes an AC neither
	trigger- nor delivery-enabled.

	ACM_TG_CMT_UAPSD_CHANGED_BY_TSPEC
========================================================================
*/
STATIC VOID ACM_APSD_Ctrl(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					AcId,
	ACM_PARAM_IN	UCHAR					Direction,
	ACM_PARAM_IN	UCHAR					FlgTsAdd,
	ACM_PARAM_IN	UCHAR					FlgIsApsdEnable)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
{
//	QUEUE_HEADER *pPsQueAc;
//	PNDIS_PACKET *pPktQued;
//	ULONG SplFlags;
	UCHAR FlgIsTrEnabled, FlgIsDeEnabled;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init new UAPSD state */
	FlgIsTrEnabled = pCdb->bAPSDCapablePerAC[AcId];
	FlgIsDeEnabled = pCdb->bAPSDDeliverEnabledPerAC[AcId];

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> ac%d, old ADD=%d, APSD=%d, TR=%d, DE=%d\n",
				AcId, FlgTsAdd, FlgIsApsdEnable, FlgIsTrEnabled,
                FlgIsDeEnabled));

	if (FlgTsAdd)
	{
		/* for addition use */

		/* new TSPEC replaces current one or both */
		if (Direction == ACM_DIRECTION_UP_LINK)
			FlgIsTrEnabled = FlgIsApsdEnable;
		else if (Direction == ACM_DIRECTION_DOWN_LINK)
			FlgIsDeEnabled = FlgIsApsdEnable;
		else
		{
			FlgIsTrEnabled = FlgIsApsdEnable;
			FlgIsDeEnabled = FlgIsApsdEnable;
		} /* End of if */
	}
	else
	{
		/* for deletion use */

		/* deleted TSPEC recovers current one or both */
		if (Direction == ACM_DIRECTION_UP_LINK)
			FlgIsTrEnabled = pCdb->bACMAPSDBackup[AcId];
		else if (Direction == ACM_DIRECTION_DOWN_LINK)
			FlgIsDeEnabled = pCdb->bACMAPSDBackupDeliverEnabled[AcId];
		else
		{
			FlgIsTrEnabled = pCdb->bACMAPSDBackup[AcId];
			FlgIsDeEnabled = pCdb->bACMAPSDBackupDeliverEnabled[AcId];
		} /* End of if */
	} /* End of if */

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> ac%d, new ADD=%d, APSD=%d, TR=%d, DE=%d\n",
				AcId, FlgTsAdd, FlgIsApsdEnable, FlgIsTrEnabled,
                FlgIsDeEnabled));

	/* check whether new UAPSD state is same as old UAPSD state */
	if ((FlgIsTrEnabled == pCdb->bAPSDCapablePerAC[AcId]) &&
		(FlgIsDeEnabled == pCdb->bAPSDDeliverEnabledPerAC[AcId]))
	{
		/* new UAPSD state is same as current UAPSD state */
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> No change for PS mode!\n"));
		return;
	} /* End of if */

	if (FlgIsTrEnabled)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> AC%d is trigger-enabled AC!\n", AcId));
	} /* End of if */

	if (FlgIsDeEnabled)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> AC%d is delivery-enabled AC!\n", AcId));
	} /* End of if */

	if (!FlgIsTrEnabled && !FlgIsDeEnabled)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> AC%d is legacy-PS AC!\n", AcId));
	} /* End of if */


	/* update UAPSD state */
	pCdb->bAPSDCapablePerAC[AcId] = FlgIsTrEnabled;
	pCdb->bAPSDDeliverEnabledPerAC[AcId] = FlgIsDeEnabled;

	/* update UAPSD control block information */
    if ((pCdb->bAPSDDeliverEnabledPerAC[QID_AC_BE] == 0) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_BK] == 0) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_VI] == 0) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_VO] == 0))
    {
        CLIENT_STATUS_CLEAR_FLAG(pCdb, fCLIENT_STATUS_APSD_CAPABLE);
    }
    else
    {
        CLIENT_STATUS_SET_FLAG(pCdb, fCLIENT_STATUS_APSD_CAPABLE);
    } /* End of if */

    if ((pCdb->bAPSDDeliverEnabledPerAC[QID_AC_BE] == 1) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_BK] == 1) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_VI] == 1) &&
        (pCdb->bAPSDDeliverEnabledPerAC[QID_AC_VO] == 1))
    {
        /* all AC are U-APSD delivery-enabled */
        pCdb->bAPSDAllAC = 1;

		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> all AC are UAPSD!\n"));
    }
    else
    {
        /* at least one AC is not U-APSD delivery-enabled */
        pCdb->bAPSDAllAC = 0;

		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> not all AC are UAPSD!\n"));
    } /* End of if */
}
#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


} /* End of ACM_APSD_Ctrl */


/*
========================================================================
Routine Description:
	Check whether bandwidth is enough to allow new stream.

Arguments:
	pAd					- WLAN control block pointer
	StmAcId				- the AC
	SI					- the service interval
	Policy				- ACM_ACCESS_POLICY_EDCA
	Direction			-	ACM_DIRECTION_UP_LINK,
							ACM_DIRECTION_DOWN_LINK,
							ACM_DIRECTION_DIRECT_LINK,
							ACM_DIRECTION_BIDIREC_LINK
	AcmTimeOld			- old used time of the same stream, unit: microsecond
	AcmTimeNew			- the requested time of new stream, unit: microsecond
	AcmTimeOldBi		- old used time for bidirectional, unit: microsecond
	*pTimeOffset		- the insufficient time when check result is fail
	*pDatlAc			- the borrowed AC ID, 0 ~ 3
	*pDatlBw			- the borrowed bandwidth from a AC, unit: microsecond

Return Value:
	ACM_RTN_OK			- allow
	ACM_RTN_FAIL		- doesnt allow due to total ACM time < Tcp,
						i.e. no any AC stream can be deleted to
						increase bandwidth
	ACM_RTN_INSUFFICIENT_BD_BUT_DEL_AC - doesnt allow but we maybe delete AC
										 streams to increase bandwidth

Note:
	pTimeOffset can be NULL;
	if pTimeOffset == NULL, no any output value will be set.

	Only used for QAP.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_BandwidthCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					StmAcId,
	ACM_PARAM_IN	UINT32					SI,
	ACM_PARAM_IN	UINT32					Policy,
	ACM_PARAM_IN	UINT32					Direction,
	ACM_PARAM_IN	UINT32					AcmTimeOld,
	ACM_PARAM_IN	UINT32					AcmTimeNew,
	ACM_PARAM_IN	UINT32					AcmTimeOldBi,
	ACM_PARAM_OUT	UINT32					*pTimeOffset,
	ACM_PARAM_OUT	UINT32					*pDatlAc,
	ACM_PARAM_OUT	UINT32					*pDatlBw)
{
	UINT32 TimeResidual;
	UINT32 TimeAc10;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	*pDatlAc = ACM_DATL_NO_BORROW;
	*pDatlBw = 0;

	/* check AcmTimeNew */
	if (AcmTimeNew > ACM_TIME_BASE)
		goto LabelErr; /* > time base */
	/* End of if */

	/* check whether Direction is bidirectional link */
	if (Direction == ACM_DIRECTION_BIDIREC_LINK)
	{
		/*
			double medium time for bidirectional because bidirectional
			link = uplink + dnlink

			Only for new time because
			AcmTimeOld = pOldStreamIn+pOldStreamOut+pOldStreamDiffAc

			But when old stream is bidirectional TSPEC for the same AC,
			we need to double the old time.
		*/
		AcmTimeNew = AcmTimeNew << 1;
		AcmTimeOld += AcmTimeOldBi;
	} /* End of if */

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Old time = %d, new time = %d\n", AcmTimeOld, AcmTimeNew));

	/* check whether new requested time <= old requested time */
	if (AcmTimeNew <= AcmTimeOld)
		return ACM_RTN_OK; /* bandwidth is enough */
	/* End of if */

	/* calculate reserved AC1/0 (best effort and background) time every sec */
	if ((StmAcId == ACM_EDCA_VI_AC_QUE_ID) ||
		(StmAcId == ACM_EDCA_VO_AC_QUE_ID))
	{
		/* only VI or VO ACM need to care about BE/BK reserved bandwidth */
		TimeAc10 = ACM_BANDWIDTH_CHECK_BASE * ACMR_CB->EdcaCtrlParam.BEK_MinNu;
		TimeAc10 /= ACMR_CB->EdcaCtrlParam.BEK_MinDe;
	}
	else
		TimeAc10 = 0;
	/* End of if */

	/* examine residual bandwidth */
	if (Policy == ACM_ACCESS_POLICY_EDCA)
	{
		/* EDCA stream */
		if (SI == 0)
		{
			/* no any HCCA stream exists */
			/* residual time = 1s - total ACM time */
			TimeResidual = ACM_BANDWIDTH_CHECK_BASE;
			TimeResidual -= ACMR_CB->EdcaCtrlParam.AcmTotalTime;
		}
		else
		{
			/* at lease one HCCA stream exists */
			/* residual time = 1s - total polled time - total ACM time */
			TimeResidual = SI; /* SI */
			TimeResidual = TimeResidual*ACM_BANDWIDTH_CHECK_BASE/SI;
			TimeResidual -= ACMR_CB->EdcaCtrlParam.AcmTotalTime;
		} /* End of if */

		/* substract reserved AC1/0 time from the residual time (1 second) */
		TimeResidual -= TimeAc10;

#ifdef ACM_CC_FUNC_MBSS
		/* substract time from other BSS */
		if (TimeResidual < ACMR_CB->MbssTotalUsedTime)
			TimeResidual = 0;
		else
			TimeResidual -= ACMR_CB->MbssTotalUsedTime;
		/* End of if */
#endif // ACM_CC_FUNC_MBSS //

		/* check whether residual time is enough for the new stream */
		if ((TimeResidual + AcmTimeOld) <= AcmTimeNew)
		{
			/* residual bandwidth is not enough for new EDCA stream */
			goto LabelErr;
		} /* End of if */

		/* also do dynamic ATL check */
		if (ACMR_CB->EdcaCtrlParam.FlgDatl)
		{
			if (ACM_DATL_Handle(pAd, StmAcId, AcmTimeOld, AcmTimeNew,
								pDatlAc, pDatlBw) != ACM_RTN_OK)
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> DATL not allow! BandwidthCheck()\n"));
				goto LabelErr;
			} /* End of if */
		} /* End of if */

		/* so bandwidth is enough */
		return ACM_RTN_OK;
	} /* End of if */

LabelErr:
	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Not enough bandwidth (new request time = %dus)\n",
				AcmTimeNew));
	return ACM_RTN_FAIL;
} /* End of ACM_BandwidthCheck */


/*
========================================================================
Routine Description:
	Check whether bandwidth is enough to allow new stream.

Arguments:
	pAd					- WLAN control block pointer
	DatlAcId			- the AC
	AcmTimeOld			- old used time of the same stream, unit: microsecond
	AcmTimeNew			- the requested time of new stream, unit: microsecond
	*pDatlAc			- the borrowed AC ID, 0 ~ 3
	*pDatlBw			- the borrowed bandwidth from a AC, unit: microsecond

Return Value:
	ACM_RTN_OK			- allow
	ACM_RTN_INSUFFICIENT_BD_BUT_DEL_AC - doesnt allow but we maybe delete AC
										 streams to increase bandwidth

Note:
	We only check available bandwidth of 'A' AC, not for all ACs.

	For example: if AC3 need more 100ms bandwidth, but residual bandwidth of
	AC2 is 80ms and residual bandwidth of AC1 is 60ms, we still NOT allow the
	TS.  We only check 100>80 and 100>60 so we can not allow the TS.
	We dont check 100<(80+60).
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_DATL_Handle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					StmAcId,
	ACM_PARAM_IN	UINT32					AcmTimeOld,
	ACM_PARAM_IN	UINT32					AcmTimeNew,
	ACM_PARAM_OUT	UINT32					*pDatlAc,
	ACM_PARAM_OUT	UINT32					*pDatlBw)
{
#define ACM_LMR_DATL_REAL_TIME_CAL(__Time, __Percent)	\
	__Time = __Percent;									\
	__Time *= AcmTotalTime;								\
	__Time /= 100;

	ACM_CTRL_PARAM *pEdcaParam;
	UINT32 *pTimeAcmCur;
	UCHAR *pBwMax, *pBwMin;
	INT32 AcmTotalTime;
	INT32 TimeOpCur, TimeOpMax, TimeOpOff;
	INT32 TimeAcCur, TimeAcMax, TimeAcMin, TimeAcOff;
	UINT32 BwBorAc[ACM_DEV_NUM_OF_AC][ACM_DEV_NUM_OF_AC];
	UINT32 IdAcNum, IdAcOther;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if ((AcmTimeOld != 0) && (AcmTimeNew <= AcmTimeOld))
		return ACM_RTN_OK;
	/* End of if */

	/* here, AcmTimeNew >= AcmTimeOld (AcmTimeOld maybe 0) */

	/* init */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	pTimeAcmCur = pEdcaParam->AcmAcTime;
	pBwMax = pEdcaParam->DatlBwMax; /* unit: percentage */
	pBwMin = pEdcaParam->DatlBwMin; /* unit: percentage */
	ACMR_MEM_COPY(BwBorAc, pEdcaParam->DatlBorAcBw, sizeof(BwBorAc));

	*pDatlAc = ACM_DATL_NO_BORROW;
	*pDatlBw = 0;

	/* get TOTAL allowed ACM time */
	AcmTotalTime = ACM_TIME_BASE;

	/* get new used time for the AC */
	TimeOpCur = pTimeAcmCur[StmAcId] + AcmTimeNew - AcmTimeOld;
	if (TimeOpCur < 0)
		return ACM_RTN_OK; /* should not be here */
	/* End of if */

	/* get maximum allowed ACM time for the AC */
	ACM_LMR_DATL_REAL_TIME_CAL(TimeOpMax, pBwMax[StmAcId]);

#ifdef ACM_CC_FUNC_MBSS
	TimeOpCur += ACMR_CB->MbssAcUsedTime[StmAcId];
#endif // ACM_CC_FUNC_MBSS //

	if (TimeOpCur <= TimeOpMax)
		return ACM_RTN_OK; /* yet exceed its max time so no borrow is needed */
	/* End of if */


	/* here, extra bandwidth is needed so calculating needed extra bandwidth */
	TimeOpOff = AcmTimeNew - AcmTimeOld; /* >= 0 */

	/* sub extra bandwidth by self available bandwidth */
	if ((INT32)(pTimeAcmCur[StmAcId]) < TimeOpMax)
	{
		if (TimeOpOff <= (TimeOpMax - (INT32)(pTimeAcmCur[StmAcId])))
		{
			/* current available bandwidth is enough */

			/*
				Should not be here because TimeOpCur will
				<= TimeOpMax, we will return above.
			*/
			return ACM_RTN_OK;
		} /* End of if */

		TimeOpOff -= (TimeOpMax - pTimeAcmCur[StmAcId]);
	} /* End of if */

	/*
		Check the needed extra bandwidth in the available bandwidth
		of other ACs.
	*/
	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
	{
		if (pEdcaParam->FlgAcmStatus[IdAcNum] == 0)
			continue; /* ACM is disabled */
		/* End of if */

		if (IdAcNum == StmAcId)
			continue; /* skip same AC */
		/* End of if */

		if (pBwMin[IdAcNum] >= pBwMax[IdAcNum])
			continue; /* no extra bandwidth can be borrow */
		/* End of if */

		/* calculate cur/max/min time for the AC, unit: microsecond */
		TimeAcCur = pTimeAcmCur[IdAcNum];
		ACM_LMR_DATL_REAL_TIME_CAL(TimeAcMax, pBwMax[IdAcNum]);
		ACM_LMR_DATL_REAL_TIME_CAL(TimeAcMin, pBwMin[IdAcNum]);

#ifdef ACM_CC_FUNC_MBSS
		TimeAcCur += ACMR_CB->MbssAcUsedTime[IdAcNum];
#endif // ACM_CC_FUNC_MBSS //

		/* no available bandwidth for the AC */
		if (TimeAcCur > TimeAcMax)
			continue;
		/* End of if */

		/*
			If cur used time > min threshold time, use cur used time as
			min threshold time.
		*/
		if (TimeAcCur > TimeAcMin)
			TimeAcMin = TimeAcCur;
		/* End of if */

		/* accumulate borrowed bandwidth for the AC i */
		for(IdAcOther=0; IdAcOther<ACM_DEV_NUM_OF_AC; IdAcOther++)
			TimeAcMin += BwBorAc[IdAcNum][IdAcOther];
		/* End of for */

		/* calculate residual bandwidth that can be borrowed */
		TimeAcOff = TimeAcMax - TimeAcMin;

		if (TimeOpOff <= TimeAcOff)
		{
			/* we can borrow the bandwidth from the AC */
			*pDatlAc = IdAcNum;
			*pDatlBw = TimeOpOff;

			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> borrow bandwidth %dus from AC%d!"
						"ACM_DATL_Handle()\n", TimeOpOff, IdAcNum));
			return ACM_RTN_OK;
		} /* End of if */

		/* bandwidth is not enough, check next one */
	} /* End of for */

	return ACM_RTN_INSUFFICIENT_BD_BUT_DEL_AC;
} /* End of ACM_DATL_Handle */


/*
========================================================================
Routine Description:
	Update ACM DATL information.

Arguments:
	pAd					- WLAN control block pointer
	StmAcId				- the AC
	AcmTimeOld			- old used time of the same stream, unit: microsecond
	AcmTimeNew			- the requested time of new stream, unit: microsecond
	DatlAcId			- the borrowed AC ID, 0 ~ 3
	DatlBandwidth		- the borrowed bandwidth from a AC, unit: microsecond

Return Value:
	None

Note:
	When AcmTimeNew > AcmTimeOld, DatlBandwidth will be the extra needed
	bandwidth (AcmTimeNew - AcmTimeOld).

	When AcmTimeNew <= AcmTimeOld, DatlBandwidth will be 0.
========================================================================
*/
STATIC VOID ACM_DATL_Update(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					StmAcId,
	ACM_PARAM_IN	UINT32					AcmTimeOld,
	ACM_PARAM_IN	UINT32					AcmTimeNew,
	ACM_PARAM_OUT	UINT32					DatlAcId,
	ACM_PARAM_OUT	UINT32					DatlBandwidth)
{
	ACM_CTRL_PARAM  *pEdcaParam;
	UINT32 IdAcNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);

	/* reduce time_offset from any borrow bandwidth of other AC */
	if ((AcmTimeOld != 0) && (AcmTimeOld > AcmTimeNew))
	{
		/* time_offset is the bandwidth we need to reclaim it to other AC */
		UINT32 time_offset = AcmTimeOld - AcmTimeNew;


		for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
		{
			if (IdAcNum == StmAcId)
				continue;
			/* End of if */

			if (time_offset > pEdcaParam->DatlBorAcBw[IdAcNum][StmAcId])
			{
				/* reclaim some time to the AC */
				time_offset -= pEdcaParam->DatlBorAcBw[IdAcNum][StmAcId];
				pEdcaParam->DatlBorAcBw[IdAcNum][StmAcId] = 0;

				/* check next AC */
			}
			else
			{
				/* reclaim all time to the AC */
				pEdcaParam->DatlBorAcBw[IdAcNum][StmAcId] -= time_offset;
				break;
			} /* End of if */
		} /* End of for */
	} /* End of if */

	/* accumulate new borrowed bandwidth */
	if (DatlAcId != ACM_DATL_NO_BORROW)
		pEdcaParam->DatlBorAcBw[DatlAcId][StmAcId] += DatlBandwidth;
	/* End of if */
} /* End of ACM_DATL_Update */


/*
========================================================================
Routine Description:
	Get extra data length for different entrypt mode.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA

Return Value:
	the extra data length

Note:
========================================================================
*/
STATIC UINT32 ACM_EncryptExtraLenGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb)
{
	UINT32 DataExtraLen;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (pCdb == NULL)
		return 0; /* error */
	/* End of if */

	if (ACMR_STA_ENCRYPT_MODE_GET(pCdb) == ACMR_ENCRYPT_WEP)
	{
		/* IV (4B) + ICV (4B) */
		DataExtraLen = 8;
	}
	else if (ACMR_STA_ENCRYPT_MODE_GET(pCdb) == ACMR_ENCRYPT_TKIP)
	{
		/* IV/KeyID (4B) + Extended IV (4B) + MIC (8B) + ICV (4B) */
		DataExtraLen = 20;
	}
	else if (ACMR_STA_ENCRYPT_MODE_GET(pCdb) == ACMR_ENCRYPT_AES)
	{
		/* CCMP Header (8B) + MIC (8B) */
		DataExtraLen = 16;
	}
	else
		DataExtraLen = 0;
	/* End of if */

	return DataExtraLen;
} /* End of ACM_EncryptExtraLenGet */


#ifdef CONFIG_STA_SUPPORT_SIM
/*
========================================================================
Routine Description:
	Make up a WME Setup Request frame to the QAP.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the peer
	*pBufFrame			- the packet buffer
	*pReqNew			- the requested TSPEC pointer

Return Value:
	Frame Length

Note:
	1. Use high priority queue to send.
	2. Only for QSTA mode (AP Client).
========================================================================
*/
STATIC UINT32 ACM_FrameAddtsReqMakeUp(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					*pBufFrame,
	ACM_PARAM_IN	ACM_STREAM				*pReqNew)
{
	ACMR_WLAN_HEADER	HdrActionFrame;
	ULONG				FrameLen;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	FrameLen = 0;

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> Make up a ADDTS Request...\n"));

	/* make the frame header */
	MgtMacHeaderInit(
					pAd, &HdrActionFrame, SUBTYPE_ACTION, 0,
					ACMR_AP_ADDR_GET(pAd),
					ACMR_CLIENT_MAC(pReqNew->pCdb));
	MakeOutgoingFrame(
					pBufFrame, &FrameLen,
					sizeof(ACMR_WLAN_HEADER), &HdrActionFrame,
					END_OF_ARGS);

	/* make the frame body */
	FrameLen += ACM_WME_ActionFrameBodyMake(
											pAd, pReqNew,
											(UCHAR *)&pBufFrame[FrameLen],
											ACM_ACTION_WME_SETUP_REQ,
											0);
	return FrameLen;
} /* End of ACM_FrameAddtsReqMakeUp */
#endif // CONFIG_STA_SUPPORT_SIM //




#ifdef CONFIG_AP_SUPPORT

/*
========================================================================
Routine Description:
	Make up a WME Teardown frame to the QSTA.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the peer, no use
	*pBufFrame		- the packet buffer
	*pStream		- the stream want to delete

Return Value:
	Frame Length

Note:
	1. Use high priority queue to send.
	2. No resource protection here.
========================================================================
*/
STATIC UINT32 ACM_FrameDeltsToStaMakeUp(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					*pBufFrame,
	ACM_PARAM_IN	ACM_STREAM				*pStream)
{
	ACMR_WLAN_HEADER	HdrActionFrame;
	ULONG				FrameLen;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	FrameLen = 0;

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> Make up a DELTS to QSTA...\n"));

	/* make the frame header */
	MgtMacHeaderInit(
					pAd, &HdrActionFrame, SUBTYPE_ACTION, 0,
					ACMR_CLIENT_MAC(pStream->pCdb),
					pAd->ApCfg.MBSSID[pStream->pCdb->apidx].Bssid);
	MakeOutgoingFrame(
					pBufFrame, &FrameLen,
					sizeof(ACMR_WLAN_HEADER), &HdrActionFrame,
					END_OF_ARGS);

	/* make the frame body */
	FrameLen += ACM_WME_ActionFrameBodyMake(
											pAd, pStream,
											(UCHAR *)&pBufFrame[FrameLen],
											ACM_ACTION_WME_TEAR_DOWN,
											0);
	return FrameLen;
} /* End of ACM_FrameDeltsToStaMakeUp */
#endif // CONFIG_AP_SUPPORT //


/*
========================================================================
Routine Description:
	Increase or decrease the link number counter.

Arguments:
	pAd					- WLAN control block pointer
	AccessPolicy		- ACM_ACCESS_POLICY_EDCA
	Dir					-	ACM_DIRECTION_UP_LINK,
							ACM_DIRECTION_DOWN_LINK,
							ACM_DIRECTION_DIRECT_LINK,
							ACM_DIRECTION_BIDIREC_LINK
	FlgIsAdd			- 1: increase by 1; 0: decrease by 1

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_LinkNumCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					AccessPolicy,
	ACM_PARAM_IN	UINT32					Dir,
	ACM_PARAM_IN	UINT32					FlgIsAdd)
{
	UINT32 *pAcLinkNum[ACM_DIRECTION_MAX] = \
				{	&ACMR_CB->EdcaCtrlParam.LinkNumUp,
					&ACMR_CB->EdcaCtrlParam.LinkNumDn,
					&ACMR_CB->EdcaCtrlParam.LinkNumDi,
					&ACMR_CB->EdcaCtrlParam.LinkNumBi };
	UINT32 *pNumStm;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (AccessPolicy == ACM_ACCESS_POLICY_EDCA)
		pNumStm = pAcLinkNum[Dir];
	else
	{
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> non-WMM is not supported! LinkNumCtrl()\n"));
		return;
	} /* End of if */

	if (FlgIsAdd == 1)
		(*pNumStm) ++; /* a new link is accepted */
	else
	{
		if ((*pNumStm) > 0)
			(*pNumStm) --; /* a old link is deleted */
		else
		{
			ACMR_DEBUG(ACMR_DEBUG_ERR,
						("acm_err> link number == 0! dir = %d "
						"ACM_LinkNumCtrl()\n", Dir));
			(*pNumStm) = 0; /* fix it */
		} /* End of if */
	} /* End of if */
} /* End of ACM_LinkNumCtrl */


/*
========================================================================
Routine Description:
	Set the minimum PHY Mode and MCS to the packet.

Arguments:
	pAd				- WLAN control block pointer
	*pStream		- the stream pointer

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
	We have checked the min physical rate should be the supported rate.
	So dont need to empty the non-supported rate in
	gAcmMCS_CCK[] or gAcmMCS_OFDM[].
========================================================================
*/
STATIC VOID ACM_PacketPhyModeMCSSet(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACM_STREAM				*pStream)
{
	UINT16 Rate[] = { /* non-11n rate */
		10, 20, 55, 60, 90, 110, 120, 180, 240, 360, 480, 540 };
	UINT32 MinPhyRate; /* unit: bps */
	UCHAR PhyModeMin, McsMin;
	UINT32 PreambleId, RateId;
	ACMR_STA_DB *pCdb;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (pStream->pTspec->MinPhyRate == 0)
		return;
	/* End of if */

	/* init */
	PhyModeMin = ACMR_PHY_NONE;
	McsMin = 0;
	PreambleId = 0; /* 0: long preamble, 1: short preamble */
	pCdb = pStream->pCdb;

	if (ACMR_STA_IS_SPREAMBLE(pAd, pCdb))
		PreambleId = 1;
	/* End of if */

	/* find the phy mode & mcs based on minimum physical rate */
	while(1)
	{
		MinPhyRate = pStream->pTspec->MinPhyRate;

		for(RateId=0; RateId<ACM_RATE_B_NUM; RateId++)
		{
			if (MinPhyRate == \
				((UINT32)gAcmMCS_CCK[PreambleId][RateId][1] * ACM_RATE_UNIT))
			{
				PhyModeMin = ACMR_PHY_CCK;
				McsMin = gAcmMCS_CCK[1][RateId][0];
				break;
			} /* End of if */
		} /* End of for */

		if (PhyModeMin == ACMR_PHY_NONE)
		{
			/* not found in CCK rate so try to find it in OFDM rate */
			for(RateId=0; RateId<ACM_RATE_G_NUM; RateId++)
			{
				if (MinPhyRate == \
					((UINT32)gAcmMCS_OFDM[RateId][1] * ACM_RATE_UNIT))
				{
					PhyModeMin = ACMR_PHY_OFDM;
					McsMin = gAcmMCS_OFDM[RateId][0];
					break;
				} /* End of if */
			} /* End of for */
		} /* End of if */

#ifdef ACM_CC_FUNC_11N
		if (PhyModeMin == ACMR_PHY_NONE)
		{
			BOOLEAN FlgIs2040;
			UINT32 IdGI;


			/* not found in OFDM rate so try to find it in N rate */
			FlgIs2040 = ACMR_IS_2040_STA(pCdb);

			for(IdGI=0; IdGI<2; IdGI++)
			{
				for(RateId=0; RateId<ACM_RATE_MAX_NUM_HT; RateId++)
				{
					if (MinPhyRate == \
						((UINT32)gAcmMCS_HT[FlgIs2040][0][RateId] * \
																ACM_RATE_UNIT))
					{
						/* always use regular GI */
						PhyModeMin = ACMR_PHY_HT;
						McsMin = RateId;
						break;
					} /* End of if */
				} /* End of for */
			} /* End of for */
		} /* End of if */
#endif // ACM_CC_FUNC_11N //

		if (PhyModeMin != ACMR_PHY_NONE)
		{
			/* correct MIN PHY rate */
			break;
		} /* End of if */


		/* should not be here */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> MIN PHY Rate %d bps is not "
					"our supported rate!\n",
					MinPhyRate));

		/* the rate does not belong to any specified rate */
		/* we need to choose one as the minimum physical rate */
		MinPhyRate = MinPhyRate / ACM_RATE_UNIT;

		/* try to find the correct rate in non-HT rates */
		for(RateId=0; RateId<ACM_RATE_MAX_NUM; RateId++)
		{
			if (MinPhyRate < Rate[RateId])
			{
				if (RateId > 0)
					RateId--;
				/* End of if */

				MinPhyRate = Rate[RateId];
				break;
			} /* End of if */
		} /* End of for */

#ifdef ACM_CC_FUNC_11N
		/* if not found, try to find the correct rate in HT rates */
		if (RateId == ACM_RATE_MAX_NUM)
		{
			BOOLEAN FlgIs2040;
			UINT32 McsRate, McsRateMin;
			UINT32 OffsetMin;


			FlgIs2040 = ACMR_IS_2040_STA(pCdb);
			McsRateMin = gAcmMCS_HT[FlgIs2040][0][0] * ACM_RATE_UNIT;
			OffsetMin = 0xFFFFFFFF;

			for(RateId=0; RateId<ACM_RATE_MAX_NUM_HT; RateId++)
			{
				McsRate = (UINT32)gAcmMCS_HT[FlgIs2040][0][RateId];
				McsRate *= ACM_RATE_UNIT;

				if (McsRate <= MinPhyRate)
				{
					if ((MinPhyRate - McsRate) < OffsetMin)
					{
						OffsetMin = (MinPhyRate - McsRate);
						McsRateMin = McsRate;
					} /* End of if */
				} /* End of if */
			} /* End of for */

			MinPhyRate = McsRateMin / ACM_RATE_UNIT;
		} /* End of if */
#endif // ACM_CC_FUNC_11N //

		/* adjust the minimum physical rate to the correct rate number */
		pStream->pTspec->MinPhyRate = MinPhyRate * ACM_RATE_UNIT;

		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Correct MIN PHY Rate to %d bps!\n",
					pStream->pTspec->MinPhyRate));
	} /* End of while */

	/* assign minimum phy mode & mcs to the stream */
	pStream->PhyModeMin = PhyModeMin;
	pStream->McsMin = McsMin;

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Min PHY mode = %d, Min MCS = %d\n",
				PhyModeMin, McsMin));
} /* End of ACM_PacketPhyModeMCSSet */


/*
========================================================================
Routine Description:
	Get the MAC address of next client data base.

Arguments:
	pAd				- WLAN control block pointer
	*pDevIndex		- search base, first = 0
	*pDevMac		- the MAC of the client

Return Value:
	ACM_RTN_OK		- get successfully
	ACM_RTN_FAIL	- no client can be got

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_PeerDeviceMacGetNext(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN_OUT	UINT32					*pDevIndex,
	ACM_PARAM_IN		UCHAR					*pDevMac)
{
	ACMR_STA_DB *pCdb;
	UINT32 IdDevNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pCdb = NULL; /* initial value must be NULL */

	/* search for all client entries */
	for(IdDevNum=(*pDevIndex); IdDevNum<ACMR_STA_MAX_NUM; IdDevNum++)
	{
		pCdb = ACMR_STA_GET(pAd, pCdb, IdDevNum);

		if (ACMR_STA_IS_VALID(pCdb))
		{
			/* the client entry exists */
			ACMR_MEM_MAC_COPY(pDevMac, ACMR_CLIENT_MAC(pCdb));

			/* return next client entry index */
			*pDevIndex = (IdDevNum+1);
			return ACM_RTN_OK;
		} /* End of if */
	} /* End of for */

	/* no any client exists */
	return ACM_RTN_FAIL;
} /* End of ACM_PeerDeviceMacGetNext */




/*
========================================================================
Routine Description:
	Mapping current station rate to ACM rate.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA

Return Value:
	ACM_PRE_TIME_ID_1M, etc.

Note:
========================================================================
*/
STATIC UCHAR ACM_Rate_Mapping(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb)
{
	UCHAR RateMapping[ACM_RATE_MAX_NUM][3] = {
		{ ACMR_RATE_DRV_1M,		ACM_RATE_1M,	ACM_PRE_TIME_ID_1M },
		{ ACMR_RATE_DRV_2M,		ACM_RATE_2M,	ACM_PRE_TIME_ID_2M },
		{ ACMR_RATE_DRV_5_5M,	ACM_RATE_5_5M,	ACM_PRE_TIME_ID_5_5M },
		{ ACMR_RATE_DRV_11M,	ACM_RATE_11M,	ACM_PRE_TIME_ID_11M },
		{ ACMR_RATE_DRV_6M,		ACM_RATE_6M,	ACM_PRE_TIME_ID_6M },
		{ ACMR_RATE_DRV_9M,		ACM_RATE_9M,	ACM_PRE_TIME_ID_9M },
		{ ACMR_RATE_DRV_12M,	ACM_RATE_12M,	ACM_PRE_TIME_ID_12M },
		{ ACMR_RATE_DRV_18M,	ACM_RATE_18M,	ACM_PRE_TIME_ID_18M },
		{ ACMR_RATE_DRV_24M,	ACM_RATE_24M,	ACM_PRE_TIME_ID_24M },
		{ ACMR_RATE_DRV_36M,	ACM_RATE_36M,	ACM_PRE_TIME_ID_36M },
		{ ACMR_RATE_DRV_48M,	ACM_RATE_48M,	ACM_PRE_TIME_ID_48M },
		{ ACMR_RATE_DRV_54M,	ACM_RATE_54M,	ACM_PRE_TIME_ID_54M } };
	UINT32 PhyMode, MCS;
	UINT32 RateIndex;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	RateIndex = ACM_RATE_MAX_NUM - 1; /* default use maximum rate */

	ACMR_CLIENT_PHY_MODE_MCS_GET(pCdb, PhyMode, MCS);

	if (PhyMode == ACMR_PHY_CCK)
	{
		/* CCK PHY */
		ACMR_CLIENT_CCK_RATE_INDEX_GET(MCS, RateIndex);
	}
	else
	{
		/* OFDM PHY */
		ACMR_CLIENT_OFDM_RATE_INDEX_GET(MCS, RateIndex);
	} /* End of if */

	return RateMapping[RateIndex][2];
} /* End of ACM_Rate_Mapping */


/*
========================================================================
Routine Description:
	Get the output or input TSPEC array list of the device.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the MAC of the client
	FlgIsOutputLink		- 1: output links; 0: input links

Return Value:
	ACM_RTN_OK			- get ok
	ACM_RTN_FAIL		- get fail

Note:
========================================================================
*/
STATIC UCHAR **ACM_StationTspecListGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pDevMac,
	ACM_PARAM_IN	BOOLEAN					FlgIsOutputLink)
{
	ACMR_STA_DB *pCdb;
	ACM_ENTRY_INFO *pStaAcmInfo;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pCdb = ACMR_STA_ENTRY_GET(pAd, pDevMac);
	if (pCdb == NULL)
	{
		/* maybe the peer device disassociated or we disassociated it */
		return NULL;
	} /* End of if */

	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);

	if (FlgIsOutputLink == TRUE)
		return pStaAcmInfo->pAcStmOut;
	/* End of if */

	return pStaAcmInfo->pAcStmIn;
} /* End of ACM_StationTspecListGet */


/*
========================================================================
Routine Description:
	Get the TSID from the packet from VLAN ID or DSCP.

Arguments:
	pAd				- WLAN control block pointer
	*pMbuf			- the QoS frame

Return Value:
	the TSID

Note:
========================================================================
*/
STATIC UCHAR ACM_TSID_Get(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_MBUF			*pMbuf)
{
	ACM_IPHDR *pIpHdr;
	PUCHAR pBufSrc;
	UCHAR TSID;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	TSID = 0; /* TSID = default BE */

	ACMR_PKT_DATA_GET(pMbuf, pBufSrc);

	if (((pBufSrc[12] << 8) + pBufSrc[13]) == 0x0800)
	{
		/* IPv4 packet */
		pIpHdr = (ACM_IPHDR *)&pBufSrc[14];
		TSID = pIpHdr->TOS >> 5; /* UP = DSCP / 8 */
	}
	else
	{
		/* VLAN priority is larger than DSCP */
		if (((pBufSrc[12] << 8) + pBufSrc[13]) == 0x8100)
		{
			/* VLAN packet */
			TSID = (pBufSrc[14] & 0xe0) >> 5;
		} /* End of if */
	} /* End of if */

	return TSID;
} /* End of ACM_TSID_Get */


/*
========================================================================
Routine Description:
	Get the physical transmit queue type.

Arguments:
	AcmAcId				- physical AC ID

Return Value:
	the physical transmit queue type

Note:
========================================================================
*/
STATIC UINT32 ACM_TxQueueTypeGet(
	ACM_PARAM_IN	UCHAR		AcmAcId)
{
	/* EDCA stream, AC0 ~ AC3 */
	switch(AcmAcId)
	{
		case 0:
			return ACMR_QID_AC_BE;	/* AC0 (BE) stream */

		case 1:
			return ACMR_QID_AC_BK;	/* AC1 (BK) stream */

		case 2:
			return ACMR_QID_AC_VI;	/* AC2 (VI) stream */

		case 3:
			return ACMR_QID_AC_VO;	/* AC3 (VO) stream */

		default: /* fatal error */
			ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> EDCA AcmAcId > 3! TxQueueTypeGet()\n"));
			return ACMR_QID_AC_BE;	/* default: AC0 (BE) stream */
	} /* End of switch */
} /* End of ACM_TxQueueTypeGet */




/* =========== Peer device management function =========== */

/*
========================================================================
Routine Description:
	Insert the peer device to the backup link list.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the MAC of the QSTA

Return Value:
	ACM_RTN_OK			- add ok
	ACM_RTN_FAIL		- add fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_PeerDeviceAdd(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pDevMac)
{
	ACM_PEER_DEV_LIST *pAcmStmList, *pStmLast;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pAcmStmList = ACMR_CB->pDevListPeer;

	/* check whether no any QSTA exists in the backup list */
	if (pAcmStmList == NULL)
	{
		ACMR_MEM_ALLOC(ACMR_CB->pDevListPeer,
						sizeof(ACM_PEER_DEV_LIST), (ACM_PEER_DEV_LIST *));

		if (ACMR_CB->pDevListPeer == NULL)
			goto LabelAllocFail;
		/* End of if */

		ACMR_CB->pDevListPeer->pPrev = NULL;
		ACMR_CB->pDevListPeer->pNext = NULL;
		ACMR_MEM_MAC_COPY(ACMR_CB->pDevListPeer->MAC, pDevMac);
		return ACM_RTN_OK;
	} /* End of if */

	/* search the peer device in the list and find the last one */
	while(pAcmStmList != NULL)
	{
		if (AMR_IS_SAME_MAC(pAcmStmList->MAC, pDevMac))
		{
			/* the peer device has already existed so no need to backup it */
			return ACM_RTN_OK;
		} /* End of if */

		pStmLast = pAcmStmList;
		pAcmStmList = pAcmStmList->pNext;
	} /* End of while */

	pAcmStmList = pStmLast;

	/* do not find so append the new peer device to the last one */
	ACMR_MEM_ALLOC(pStmLast, sizeof(ACM_PEER_DEV_LIST), (ACM_PEER_DEV_LIST *));
	if (pStmLast == NULL)
		goto LabelAllocFail;
	/* End of if */

	pAcmStmList->pNext = pStmLast;

	pStmLast->pPrev = pAcmStmList;
	pStmLast->pNext = NULL;
	ACMR_MEM_MAC_COPY(pStmLast->MAC, pDevMac);
	return ACM_RTN_OK;

LabelAllocFail:
	ACMR_DEBUG(ACMR_DEBUG_ERR, ("acm_err> Allocate peer device entry fail!\n"));
	return ACM_RTN_FAIL;
} /* End of ACM_PeerDeviceAdd */


/*
========================================================================
Routine Description:
	Delete the peer device from the backup link list.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the MAC of the QSTA

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_PeerDeviceDel(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pDevMac)
{
	ACM_PEER_DEV_LIST *pDevList, *pDevNext;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pDevList = ACMR_CB->pDevListPeer;

	while(1)
	{
		if (pDevList == NULL)
		{
			/* can not found the next peer device */
			break;
		} /* End of if */

		pDevNext = pDevList->pNext;

		if (AMR_IS_SAME_MAC(pDevList->MAC, pDevMac))
		{
			/* find it and delete it */
			if (pDevList->pPrev == NULL)
				ACMR_CB->pDevListPeer = pDevList->pNext;
			else
				(pDevList->pPrev)->pNext = pDevList->pNext;
			/* End of if */

			if (pDevList->pNext != NULL)
				(pDevList->pNext)->pPrev = pDevList->pPrev;
			/* End of if */

			ACMR_MEM_FREE(pDevList);

			/* not break, try to delete all redudant entries for the peer */
			/* should not occur */
		} /* End of if */

		pDevList = pDevNext;
	} /* End of while */
} /* End of ACM_PeerDeviceDel */


/*
========================================================================
Routine Description:
	Get next the peer device from the backup link list.

Arguments:
	pAd					- WLAN control block pointer
	**ppDevicePeer		- the last peer device
	*pDevMac			- the MAC of the QSTA

Return Value:
	ACM_RTN_OK			- get ok
	ACM_RTN_FAIL		- get fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_PeerDeviceGetNext(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACM_PEER_DEV_LIST		**ppDevicePeer,
	ACM_PARAM_IN	UCHAR					*pDevMac)
{
	ACM_PEER_DEV_LIST *pDevNext;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (*ppDevicePeer == NULL)
	{
		/* get first peer */
		pDevNext = ACMR_CB->pDevListPeer;

		if (pDevNext == NULL)
		{
			*ppDevicePeer = NULL;
			return ACM_RTN_FAIL;
		} /* End of if */
	}
	else
	{
		/* get next peer */
		pDevNext = *ppDevicePeer;

		if (pDevNext->pNext == NULL)
		{
			*ppDevicePeer = NULL;
			return ACM_RTN_FAIL;
		} /* End of if */

		pDevNext = pDevNext->pNext;
	} /* End of if */

	*ppDevicePeer = pDevNext;
	ACMR_MEM_MAC_COPY(pDevMac, pDevNext->MAC);
	return ACM_RTN_OK;
} /* End of ACM_PeerDeviceGetNext */


/*
========================================================================
Routine Description:
	Free all the peer device backup link list.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_PeerDeviceAllFree(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	ACM_PEER_DEV_LIST *pDevList, *pDevNext;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pDevList = ACMR_CB->pDevListPeer;

	while(pDevList)
	{
		pDevNext = pDevList->pNext;
		ACMR_MEM_FREE(pDevList);
		pDevList = pDevNext;
	} /* End of while */

	ACMR_CB->pDevListPeer = NULL;
} /* End of ACM_PeerDeviceAllFree */


/*
========================================================================
Routine Description:
	Maintain the peer device backup link list.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the MAC of the QSTA

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_PeerDeviceMaintain(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pDevMac)
{
	ACMR_STA_DB *pCdb;
	ACM_ENTRY_INFO *pStaAcmInfo;
	UINT32 IdTidNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* get device entry */
	pCdb = ACMR_STA_ENTRY_GET(pAd, pDevMac);
	if (pCdb == NULL)
		return;
	/* End of if */

	/* get ACM control block of the device */
	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);

	/* check if all TS are deleted for the device */
	for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
	{
		if (pStaAcmInfo->pAcStmOut[IdTidNum] != NULL)
		{
			/* at least one output TS exists so dont kill the entry */
			return;
		} /* End of if */

		if (pStaAcmInfo->pAcStmIn[IdTidNum] != NULL)
		{
			/* at least one input TS exists so dont kill the entry */
			return;
		} /* End of if */
	} /* End of for */

	/* no any TS exists so we delete the peer device record */
	ACM_PeerDeviceDel(pAd, pDevMac);
} /* End of ACM_PeerDeviceMaintain */




/* ========================= Stream Management Function ===================== */

/*
========================================================================
Routine Description:
	Check whether a stream is timeout due to inactivity or suspendsion.

Arguments:
	pAd					- WLAN control block pointer
	*pStream			- the activated stream

Return Value:
	TRUE				- Need to send out a DELTS frame
	FALSE				- No need to send out a DELTS frame

Note:
	We can not delete steams only after station disassociated.
	Because maybe one link timeout but other links do not timeout for a
	QSTA.
========================================================================
*/
STATIC BOOLEAN ACM_STM_IdleCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream)
{
	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* check current stream status */
	if ((pStream->Status != TSPEC_STATUS_ACTIVE) &&
		(pStream->Status != TSPEC_STATUS_ACTIVE_SUSPENSION))
	{
		/* the request will be deleted, do NOT need to care about it */
		return FALSE;
	} /* End of if */

	/* check inactivity timeout */
	if (pStream->pTspec->InactivityInt != ACM_TSPEC_INACTIVITY_DISABLE)
	{
		/* inactivity function is enabled */

		if (pStream->InactivityCur <= ACM_STREAM_CHECK_BASE)
		{
			/* stream is timeout so we need to delete the stream */
			if (pStream->InactivityCur > 0)
			{

				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> Inactivity timeout! TX a DELTS frame! "
							"(dir %d) ACM_STM_IdleCheck()!\n",
							pStream->pTspec->TsInfo.Direction));

				pStream->InactivityCur = 0;
				pStream->Cause = TSPEC_CAUSE_INACTIVITY_TIMEOUT;
				return ACM_TC_Delete(pAd, pStream);
			} /* End of if */
		}
		else
			pStream->InactivityCur -= ACM_STREAM_CHECK_BASE;
		/* End of if */
	} /* End of if */

	return FALSE;
} /* End of ACM_STM_IdleCheck */


/*
========================================================================
Routine Description:
	Copy the stream information.

Arguments:
	*pStreamSrc				- the source stream
	*pStreamInfoDst			- the destination buffer

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_STM_InfoCopy(
	ACM_PARAM_OUT	ACM_STREAM_INFO     *pStreamInfoDst,
	ACM_PARAM_IN	ACM_STREAM			*pStreamSrc)
{
	UINT32 IdTclasNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* copy information */
	ACM_TSPEC_COPY((&(pStreamInfoDst->Tspec)), pStreamSrc->pTspec);

	for(IdTclasNum=0; IdTclasNum<ACM_TSPEC_TCLAS_MAX_NUM; IdTclasNum++)
	{
		if (pStreamSrc->pTclas[IdTclasNum] != NULL)
		{
			ACM_TCLAS_COPY((&(pStreamInfoDst->Tclas[IdTclasNum])),
							pStreamSrc->pTclas[IdTclasNum]);
		} /* End of if */
	} /* End of for */

	ACMR_MEM_MAC_COPY(pStreamInfoDst->DevMac, pStreamSrc->StaMac);

	pStreamInfoDst->TclasProcessing	= pStreamSrc->TclasProcessing;
	pStreamInfoDst->StreamType		= pStreamSrc->StreamType;
	pStreamInfoDst->UP				= pStreamSrc->UP;
	pStreamInfoDst->Reserved1		= 0;
	pStreamInfoDst->Status			= pStreamSrc->Status;
	pStreamInfoDst->Cause			= pStreamSrc->Cause;
	pStreamInfoDst->AcmAcId			= pStreamSrc->AcmAcId;
	pStreamInfoDst->FlgOutLink		= pStreamSrc->FlgOutLink;
	pStreamInfoDst->Reserved2[0]	= 0;
	pStreamInfoDst->Reserved2[1]	= 0;
	pStreamInfoDst->Reserved2[2]	= 0;
	pStreamInfoDst->InactivityCur	= pStreamSrc->InactivityCur;
	pStreamInfoDst->SuspensionCur	= pStreamSrc->SuspensionCur;
	pStreamInfoDst->PhyModeMin		= pStreamSrc->PhyModeMin;
	pStreamInfoDst->McsMin			= pStreamSrc->McsMin;
} /* End of ACM_STM_InfoCopy */


/*
========================================================================
Routine Description:
	Check inactivity and suspension for all activated streams.

Arguments:
	Data				- WLAN control block pointer

Return Value:
	None

Note:
	This is a tasklet.
========================================================================
*/
STATIC VOID ACM_TASK_STM_Check(
	ACM_PARAM_IN	ULONG		Data)
{
	ACMR_PWLAN_STRUC pAd;
	ACM_CTRL_PARAM *pEdcaParam;
	ACM_PEER_DEV_LIST *pAcmDevList;
	ACM_STREAM **ppAcmStmList, *pStream;
	ACM_FUNC_STATUS RtnCode;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UCHAR MAC[ACM_MAC_ADDR_LEN], MAC_Last[ACM_MAC_ADDR_LEN];
	UCHAR UP;
	BOOLEAN FlgIsNeedToDel;
	UINT32 IdTidNum, IdLinkNum, AcId;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pAd = (ACMR_PWLAN_STRUC) Data;
	pEdcaParam = &ACMR_CB->EdcaCtrlParam;
	RtnCode = ACM_RTN_FAIL;
	FlgIsNeedToDel = FALSE;

	/* sanity check */
	if (pEdcaParam->FlgIsTspecTimeoutEnable == 0)
		return; /* no need to do TSPEC timeout check */
	/* End of if */

	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* check all output and input links for each peer */
	pAcmDevList = NULL;

	while(1)
	{
		/* get next device */
		if (ACM_PeerDeviceGetNext(pAd, &pAcmDevList, MAC) != ACM_RTN_OK)
			break;
		/* End of if */

		/*
			Check if we need to delete the device entry due to NO any output
			and input link for the peer device.
		*/
		if (FlgIsNeedToDel == TRUE)
			ACM_PeerDeviceDel(pAd, MAC_Last);
		/* End of if */

		/* check all output and input streams for the peer device */
		for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
		{
			ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
											pAd, MAC, DirectionId[IdLinkNum]);
			if (ppAcmStmList == NULL)
			{
				/*
					We can not delete the entry here or we can not find the
					pNext in ACM_PeerDeviceGetNext(), we need to use sta_p at
					next loop.
				*/
				ACMR_MEM_MAC_COPY(MAC_Last, MAC);
				FlgIsNeedToDel = TRUE;
				break;
			} /* End of if */

			for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
			{
				pStream = ppAcmStmList[IdTidNum];

				if (pStream != NULL)
				{
					if ((IdLinkNum == 1) &&
						(pStream->pTspec->TsInfo.Direction == \
													ACM_DIRECTION_BIDIREC_LINK))
					{
						/*
							For bidirectional link, we will check the one
							when IdLinkNum == 0, so we dont need to check twice.
						*/
						continue;
					} /* End of if */

					UP = pStream->pTspec->TsInfo.UP;
                    AcId = ACM_MR_EDCA_AC(UP);

					/* do NOT check NULL TSPEC */
                    if (ACMR_CB->EdcaCtrlParam.FlgAcmStatus[AcId] != 0)
					{
						RtnCode = ACM_RTN_OK;

						if (ACM_STM_IdleCheck(pAd, pStream) == TRUE)
						{
							/* the stream timeouts so we need to delete it */
							ACM_DELTS_SEND(pAd, pStream->pCdb,
											pStream, LabelSemErr);
						} /* End of if */
					} /* End of if */
				} /* End of if */
			} /* End of for */

			FlgIsNeedToDel = FALSE;
		} /* End of for */
	} /* End of while */

	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);

	/* check if no any activated stream exists */
	if (RtnCode == ACM_RTN_FAIL)
	{
#ifndef ACMR_HANDLE_IN_TIMER
		/* disable the stream activity & suspend check timer */
		ACMR_TIMER_DISABLE(ACMR_CB->FlgStreamAliveCheckEnable,
							ACMR_CB->TimerStreamAliveCheck);
#endif // ACMR_HANDLE_IN_TIMER //
	}
	else
	{
#ifdef ACMR_HANDLE_IN_TIMER
		/* schedule next timer */
		ACMR_TIMER_ENABLE(ACMR_CB->FlgStreamAliveCheckEnable,
							ACMR_CB->TimerStreamAliveCheck,
							ACM_STREAM_CHECK_OFFSET);
#endif // ACMR_HANDLE_IN_TIMER //
	} /* End of if */

	return;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! STM_TASK_Check()\n"));
	return;
} /* End of ACM_TASK_STM_Check */


/*
========================================================================
Routine Description:
	Check inactivity and suspension for all activated streams.

Arguments:
	Data			- WLAN control block pointer

Return Value:
	None

Note:
	1. Only for QAP Mode.
	2. Waked up every 100ms.
========================================================================
*/
VOID ACM_TR_STM_Check(
	ACM_PARAM_IN	ULONG		Data)
{
#ifndef ACMR_HANDLE_IN_TIMER
	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC) Data;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check for enable flag */
	if (ACMR_CB->FlgStreamAliveCheckEnable == 0)
	{
		ACMR_TIMER_DISABLE(ACMR_CB->FlgStreamAliveCheckEnable,
							ACMR_CB->TimerStreamAliveCheck);
		return;
	} /* End of if */

	/* inform TSPEC request check task */
	ACMR_TASK_ACTIVATE(ACMR_CB->TaskletStreamAliveCheck,
						ACMR_CB->TimerStreamAliveCheck,
						ACM_STREAM_CHECK_OFFSET);
#else

	ACM_TASK_STM_Check(Data);
#endif // ACMR_HANDLE_IN_TIMER //
} /* End of ACM_TR_STM_Check */




/* =========================== 11e TSPEC Function =========================== */

/*
========================================================================
Routine Description:
	Translate factor decimal part binary to decimal. (unit: 1/100)

Arguments:
	BIN				- the binary of decimal part

Return Value:
	the decimal

Note:
	Ex: 0b0001 1000 0000 0000 ==> 0.75
========================================================================
*/
UINT32 ACM_SurplusFactorDecimalBin2Dec(
	ACM_PARAM_IN	UINT32				BIN)
{
	UINT32 ValueMax, Bit1Index, Base, ValueDec, ValueCarry;
	UINT32 IdBitNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	ValueMax = 1;
	ValueMax <<= ACM_SURPLUS_DEC_BIT_NUM;

	Bit1Index = ValueMax>>1;
	Base = 0;
	ValueDec = 0;

	/* translate to decimal, unit: 1/(1<<ACM_SURPLUS_DEC_BIT_NUM) */
	for(IdBitNum=0; IdBitNum<ACM_SURPLUS_DEC_BIT_NUM; IdBitNum++)
	{
		if (BIN & Bit1Index)
			ValueDec += ValueMax >> Base;
		/* End of if */

		/* check next bit */
		Bit1Index >>= 1;
		Base ++;
	} /* End of for */

	ValueDec *= ACM_SURPLUS_DEC_BASE;
	ValueCarry = ValueDec * 10;
	ValueDec >>= ACM_SURPLUS_DEC_BIT_NUM;
	ValueDec >>= 1;
	ValueCarry >>= ACM_SURPLUS_DEC_BIT_NUM;
	ValueCarry >>= 1;

	/* check if we need to carry (>= 0.5) */
	if ((ValueCarry - ValueDec*10) >= 5)
		ValueDec ++;
	/* End of if */

	return ValueDec;
} /* End of ACM_SurplusFactorDecimalBin2Dec */


/*
========================================================================
Routine Description:
	Translate factor decimal part decimal to binary.

Arguments:
	DEC				- the deciaml of decimal part, 00 ~ 99 (base 1/100)

Return Value:
	the binary

Note:
	Ex: 0.75 ==> 0b0001 1000 0000 0000
========================================================================
*/
STATIC UINT32 ACM_SurplusFactorDecimalDec2Bin(
	ACM_PARAM_IN	UINT32				DEC)
{
	UINT32 ValueMax, Bin1Index, ValueBin;
	UINT32 NumBit, TempValueDec;
	UINT32 IdBitNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	Bin1Index = 1;
	Bin1Index <<= (ACM_SURPLUS_DEC_BIT_NUM-1);

	ValueBin = 0;
	DEC = DEC % 100; /* 0.01 ~ 0.99 */

	NumBit = 0;
	ValueMax = 100;

	/* translate to binary */
	for(IdBitNum=0; IdBitNum<ACM_SURPLUS_DEC_BIT_NUM; IdBitNum++)
	{
		DEC <<= 1;

		TempValueDec = DEC;
		TempValueDec /= ValueMax;
		if (TempValueDec)
		{
			ValueBin |= Bin1Index;
			DEC -= ValueMax;
		} /* End of if */

		Bin1Index >>= 1;
	} /* End of for */

	return ValueBin;
} /* End of ACM_SurplusFactorDecimalDec2Bin */


/*
========================================================================
Routine Description:
	Active a requested TSPEC and move it to the active table.

Arguments:
	pAd					- WLAN control block pointer
	*pStreamReq			- the requested TSPEC pointer

Return Value:
	ACM_RTN_OK			- active ok
	ACM_RTN_FAIL		- active fail

Note:
	1. for QAP, the dnlink link TSPEC will be moved to the active
		table; the bidirectional link TSPEC will be duplicated to
		the active table & the client data base; otherwise to the
		client data base;
	2. for QSTA, the uplink or direct link TSPEC will be moved to
		the active table; otherwise to the client data base;
	3. If the requested TSPEC is a negotiation TSPEC, the old
		TSPEC will be freed.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_Active(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq)
{
	ACMR_STA_DB *pCdb;
	ACM_ENTRY_INFO *pStaAcmInfo;
	UINT32 Direction, AccessPolicy;
	UCHAR FlgOutLink;
	UCHAR StmAcId;
	UCHAR TSID;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> TC_Active()!\n"));

	/* check whether current status of the request is DELETING */
	if ((pStreamReq->Status == TSPEC_STATUS_REQ_DELETING) ||
		(pStreamReq->Status == TSPEC_STATUS_ACT_DELETING))
	{
		/* rearrange the requested TSPEC */
		ACM_TC_Rearrange(pAd, pStreamReq, ACM_MAX_NUM_OF_DELTS_RETRY);
		return ACM_RTN_FAIL;
	} /* End of if */

	/* init */
	pCdb		= pStreamReq->pCdb;
	FlgOutLink	= 0; /* default input link */
	AccessPolicy= pStreamReq->pTspec->TsInfo.AccessPolicy;
	Direction	= pStreamReq->pTspec->TsInfo.Direction;
	StmAcId		= 0; /* 0: BE */

	/* determine if the link is output link */
	if (ACMR_IS_AP_MODE)
	{
		/* QAP mode */
		if ((Direction == ACM_DIRECTION_DOWN_LINK) ||
			(Direction == ACM_DIRECTION_BIDIREC_LINK))
		{
			FlgOutLink = 1;
		} /* End of if */
	}
	else
	{
		/* QSTA mode */
		if ((Direction == ACM_DIRECTION_UP_LINK) ||
			(Direction == ACM_DIRECTION_DIRECT_LINK) ||
			(Direction == ACM_DIRECTION_BIDIREC_LINK))
		{
			FlgOutLink = 1;
		} /* End of if */
	} /* End of if */

	/* backup the new stream */
	pStaAcmInfo  = ACMR_STA_ACM_PARAM_INFO(pCdb);
	TSID = pStreamReq->pTspec->TsInfo.TSID;

	if (FlgOutLink == 1)
	{
		/* assign actual AC queue ID */
		StmAcId = ACM_MR_EDCA_AC(pStreamReq->UP);
		pStreamReq->TxQueueType = ACM_TxQueueTypeGet(StmAcId);

		/* backup output stream by TSID */
		pStaAcmInfo->pAcStmOut[TSID] = (UCHAR *)pStreamReq;

		/* for bidirectional link, we also backup it in input array by TSID */
		/* for bidirectional link, FlgOutLink must be 1 */
		if (Direction == ACM_DIRECTION_BIDIREC_LINK)
			pStaAcmInfo->pAcStmIn[TSID] = (UCHAR *)pStreamReq;
		/* End of if */
	}
	else
	{
		/* no transmit queue for input link */
		pStreamReq->TxQueueType = ACM_TX_QUEUE_TYPE_NOT_EXIST;

		/* backup input stream by TSID */
		pStaAcmInfo->pAcStmIn[TSID] = (UCHAR *)pStreamReq;
	} /* End of if */

	/* change status to ACTIVE */
	pStreamReq->FlgOutLink = FlgOutLink;
	pStreamReq->Status = TSPEC_STATUS_ACTIVE;

	/* statustics counter */
	ACM_LINK_NUM_INCREASE(pAd, pStreamReq->pTspec->TsInfo.AccessPolicy, Direction);

	/* count number of TSPEC */
	ACM_NUM_OF_TSPEC_RECOUNT(pAd, pStreamReq->pCdb);

	return ACM_RTN_OK;
} /* End of ACM_TC_Active */


/*
========================================================================
Routine Description:
	Remove a activated TSPEC.

Arguments:
	pAd					- WLAN control block pointer
	*pStream			- the actived stream

Return Value:
	None

Note:
	"Activated" means the TSPEC is accepted, whatever dnlink or uplink or
	bidirectional link.
========================================================================
*/
STATIC VOID ACM_TC_ActRemove(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream)
{
	ACMR_STA_DB *pCdb;
	ACM_ENTRY_INFO *pStaAcmInfo;
	ACM_STREAM *pStmFree;
	BOOLEAN FlgIsNeedToFree;
	UCHAR TSID;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	pCdb = pStream->pCdb;
	if (pCdb == NULL)
		return;
	/* End of if */

	/* check if the request is from a actived stream */
	FlgIsNeedToFree = 0;

	if ((pStream->Status == TSPEC_STATUS_ACT_DELETING) ||
		(pStream->Status == TSPEC_STATUS_RENEGOTIATING))
	{
		/* pStream is in request list so we will free other copys in
			non-request lists */
		FlgIsNeedToFree = 1;
	} /* End of if */

	/* reset the backup array for the stream */
	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);
	TSID = pStream->pTspec->TsInfo.TSID;

	if (pStream->FlgOutLink)
		pStmFree = (ACM_STREAM *)pStaAcmInfo->pAcStmOut[TSID];
	else
		pStmFree = (ACM_STREAM *)pStaAcmInfo->pAcStmIn[TSID];
	/* End of if */

	pStaAcmInfo->pAcStmOut[TSID] = NULL;
	pStaAcmInfo->pAcStmIn[TSID] = NULL;

	if (pStmFree == NULL)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_err> TSPEC == NULL! TC_ActRemove()\n"));
	} /* End of if */

	/* maybe free it */
	if ((FlgIsNeedToFree) && (pStmFree != NULL))
		ACM_TC_Free(pAd, pStmFree);
	/* End of if */
} /* End of ACM_TC_ActRemove */


/*
========================================================================
Routine Description:
	Delete a activated TSPEC.

Arguments:
	pAd					- WLAN control block pointer
	*pStream			- the activated stream

Return Value:
	TRUE				- Need to send out a DELTS frame
	FALSE				- No need to send out a DELTS frame

Note:
	1. Send a DELTS to the QSTA or QAP.
	2. Insert the activated TSPEC to the requested list.
	3. The TSPEC will be moved to the failed list when DELTS ACK
		is received or retry count is reached.
========================================================================
*/
STATIC BOOLEAN ACM_TC_Delete(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream)
{
	ACM_STREAM *pTspecDup;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	if ((pStream->Status == TSPEC_STATUS_REQ_DELETING) ||
		(pStream->Status == TSPEC_STATUS_ACT_DELETING))
	{
		/* already in deleting state */
		return FALSE;
	} /* End of if */

	if ((pStream->Status == TSPEC_STATUS_REQUEST) ||
		(pStream->Status == TSPEC_STATUS_RENEGOTIATING))
	{
		/* only for QSTA mode: request TSPEC or renegotiate TSPEC */

		ACM_TC_Req_ADDTS2DELTS(pAd, pStream);
		goto label_check_timer_enable;
	} /* End of if */

	if ((pStream->Status == TSPEC_STATUS_ACTIVE) ||
		(pStream->Status == TSPEC_STATUS_ACTIVE_SUSPENSION))
	{
		/* QAP or QSTA mode: actived TSPEC */

		/*
			Duplicate the stream, because we can not insert the stream to the
			requested list directly; Or the pPrev & pNext will be modified in
			ACM_TC_ReqInsert().

			So we duplicate a same stream and put it to the request list.

			If deletion is successfully, we will move the stream and the
			duplicated one.
		*/
		pTspecDup = ACM_TC_Duplicate(pAd, pStream);
		if (pTspecDup == NULL)
		{
			/* no enough memory, delete the stream at next time */
			pStream->InactivityCur = ACM_STREAM_CHECK_BASE;
			return FALSE;
		} /* End of if */

		/* change its state to deleting mode */
		pTspecDup->Status			= TSPEC_STATUS_ACT_DELETING;
		pTspecDup->TimeoutAction	= ACM_TC_TIMEOUT_ACTION_DELTS;
		pTspecDup->Timeout			= pStream->TimeoutDelts;
		pTspecDup->Retry			= ACM_MAX_NUM_OF_DELTS_RETRY;

		/* insert the duplicated stream to the requested list */
		if (ACM_TC_ReqInsert(pAd, pTspecDup) != ACM_RTN_OK)
		{
			/* insert fail (maybe already exist), free duplicate one */
			ACM_TC_Free(pAd, pTspecDup);

			/* delete the stream at next time */
			pStream->InactivityCur = ACM_STREAM_CHECK_BASE;
		}
		else
		{
			/* insert ok so send a DELTS frame to peer */

			/*
				The new allocated stream is put into the request list so
				we dont need to use the stream to send DELTS, we can use
				pStream to send the DELTS frame.
			*/
			goto label_check_timer_enable;
		} /* End of if */
	} /* End of if */

	return FALSE;

label_check_timer_enable:
	/* enable request timeout check timer */
	ACMR_TIMER_ENABLE(ACMR_CB->FlgTspecReqCheckEnable,
						ACMR_CB->TimerTspecReqCheck,
						ACM_STREAM_CHECK_OFFSET);
	return TRUE;
} /* End of ACM_TC_Delete */


/*
========================================================================
Routine Description:
	Move TSPEC active the the fail list.

Arguments:
	pAd					- WLAN control block pointer
	*pStreamReq			- the TSPEC pointer
	FlgIsActiveExcluded	-	1: do not delete active TSPEC
							0: delete both request and active TSPEC

Return Value:
	None

Note:
	1. Put the TSPEC into the failed list. Not free it to OS kernel.
	2. If pStreamReq belongs to a active TSPEC, we will delete the
		both of request TSPEC and active TSPEC simulatenously.
========================================================================
*/
STATIC VOID ACM_TC_Destroy(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq,
	ACM_PARAM_IN	BOOLEAN				FlgIsActiveExcluded)
{
	ACM_TSPEC_REQ_LIST *pTspecFreedList;
	ACM_STREAM *pTspecFree;
	UINT32 Direction;
	UCHAR FlgIsActStm;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check whether the request exists in the failed list */
	FlgIsActStm = 0;
	pTspecFreedList = &ACMR_CB->TspecListFail;
	pTspecFree = pTspecFreedList->pHead;

	while(pTspecFree != NULL)
	{
		if (AMR_IS_SAME_POINTER(pTspecFree, pStreamReq))
		{
			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> already deleted! TC_Destroy()\n"));
			return; /* already exists (same pointer) */
		} /* End of if */

		pTspecFree = pTspecFree->pNext;
	} /* End of while */

	/*
		Remove the TSPEC from the requested list or the active table

		For Status == TSPEC_STATUS_ACT_DELETING, we should delete it in the
		requested list and the active table.
	*/
	if ((pStreamReq->Status == TSPEC_STATUS_REQUEST) ||
		(pStreamReq->Status == TSPEC_STATUS_REQ_DELETING) ||
		(pStreamReq->Status == TSPEC_STATUS_ACT_DELETING) ||
		(pStreamReq->Status == TSPEC_STATUS_RENEGOTIATING))
	{
		/* remove it from the requested linked list */
		ACM_TC_ReqRemove(pAd, pStreamReq);
	} /* End of if */

	if (FlgIsActiveExcluded == 0)
	{
		/* need also to delete active TSPEC */
		if ((pStreamReq->Status == TSPEC_STATUS_ACTIVE) ||
			(pStreamReq->Status == TSPEC_STATUS_ACTIVE_SUSPENSION) ||
			(pStreamReq->Status == TSPEC_STATUS_ACT_DELETING))
		{
			/* remove it from the active table linked list */
			FlgIsActStm = 1; /* this is a active stream */
			ACM_TC_ActRemove(pAd, pStreamReq);
		} /* End of if */
	} /* End of if */

	/* change current status to fail */
	pStreamReq->Status = TSPEC_STATUS_FAIL;

	/* insert it to the failed list */
	if (pTspecFreedList->TspecNum >= ACM_MAX_NUM_OF_FAIL_RSV_TSPEC)
	{
		/* free the first one (the oldest one) */
		pTspecFree = pTspecFreedList->pHead;

		if (pTspecFree == NULL)
		{
			/* fatal error: pHead == NULL but TspecNum > 0 */
			pTspecFreedList->TspecNum = 0;

			ACMR_DEBUG(ACMR_DEBUG_ERR,
						("acm_err> pTspecFree == NULL! TC_Destroy()\n"));
		}
		else
		{
			pTspecFreedList->pHead = pTspecFree->pNext;
			pTspecFree->pNext->pPrev = NULL;
			pTspecFreedList->TspecNum --;

			ACM_FREE_TS(pTspecFree);
		} /* End of if */
	} /* End of if */

	/* insert the new failed TSPEC to the last one */
	if (pTspecFreedList->pTail != NULL)
	{
		pTspecFreedList->pTail->pNext = pStreamReq;
		pStreamReq->pPrev = pTspecFreedList->pTail;
		pTspecFreedList->pTail = pStreamReq;
	}
	else
	{
		pTspecFreedList->pTail = pStreamReq;
		pStreamReq->pPrev = NULL;
	} /* End of if */

	if (pTspecFreedList->pHead == NULL)
		pTspecFreedList->pHead = pTspecFreedList->pTail;
	/* End of if */

	pStreamReq->pNext = NULL;
	pTspecFreedList->TspecNum ++;

	/* reclaim the used time of the stream */
	if (FlgIsActStm)
	{
		/* only for ACTIVE stream */
		ACM_EDCA_AllocatedTimeReturn(pAd, pStreamReq);

		/* statistics counter */
		Direction = pStreamReq->pTspec->TsInfo.Direction;
		ACM_LINK_NUM_DECREASE(pAd,
							pStreamReq->pTspec->TsInfo.AccessPolicy,
							Direction);

		/* delete peer device record if no any TSPEC exists for the peer */
		if (pStreamReq->pCdb != NULL)
			ACM_PeerDeviceMaintain(pAd, ACMR_CLIENT_MAC(pStreamReq->pCdb));
		/* End of if */

		/* recover the UAPSD state if the TSPEC is active TSPEC */
		if (pStreamReq->pCdb != NULL)
		{
			ACM_APSD_Ctrl(pAd,
						pStreamReq->pCdb,
						ACM_MR_EDCA_AC(pStreamReq->UP),
						Direction, 0, 0);
		} /* End of if */
	} /* End of if */

	/* count number of TSPEC */
	ACM_NUM_OF_TSPEC_RECOUNT(pAd, pStreamReq->pCdb);
} /* End of ACM_TC_Destroy */


/*
========================================================================
Routine Description:
	Move it to the fail list.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the QSTA sends the DELTS frame
	*pTsInfo			- the TS Info
	FlgIsFromSta		- 1: destroy from QSTA; 0: destroy from QAP

Return Value:
	ACM_RTN_OK			- destroy ok
	ACM_RTN_FAIL		- destroy fail
	ACM_RTN_SEM_GET_ERR	- get semaphore fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_DestroyBy_TS_Info(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo,
	ACM_PARAM_IN	UCHAR				FlgIsFromSta)
{
	ACM_STREAM *pStream;
	ACMR_STA_DB *pCdb;
	UCHAR StmAcId;
	UCHAR Direction;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pCdb = NULL;
	StmAcId = 0;
	Direction = ACM_DIRECTION_UP_LINK;

	/* semaphore protection */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	/* find the stream */
	pStream = ACM_TC_Find(pAd, pDevMac, pTsInfo);

	if (pStream == NULL)
	{
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> DEL a stream but can not find it! "
					"ACM_TC_DestroyBy_TS_Info()\n"));
		goto LabelDestroyOk; /* the stream does NOT exist */
	} /* End of if */

	/*
		Check if the stream is created by the QSTA, only the original QSTA
		can delete the stream, other QSTA can NOT delete.
	*/
	if (!(AMR_IS_SAME_MAC(ACMR_CLIENT_MAC(pStream->pCdb), pDevMac)))
		goto LabelErr;
	/* End of if */

	if (FlgIsFromSta == 1)
		pStream->Cause = TSPEC_CAUSE_DELETED_BY_QSTA;
	else
		pStream->Cause = TSPEC_CAUSE_DELETED_BY_QAP;
	/* End of if */

	pCdb = pStream->pCdb;
	StmAcId = pStream->AcmAcId;
	Direction = pStream->pTspec->TsInfo.Direction;

	ACM_TC_Destroy(pAd, pStream, 0);

LabelDestroyOk:
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	/* recover UAPSD state */
//	if (pStream != NULL)
//		ACM_APSD_Ctrl(pAd, pCdb, StmAcId, Direction, 0, 0);
	/* End of if */

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> DEL a stream! TC_DestroyBy_TS_Info()\n"));
	return ACM_RTN_OK;

LabelErr:
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
	return ACM_RTN_FAIL;

LabelSemErr:
	return ACM_RTN_SEM_GET_ERR;
} /* End of ACM_TC_DestroyBy_TS_Info */


/*
========================================================================
Routine Description:
	Free a stream and do NOT move the failed TSPEC to the fail list.

Arguments:
	pAd					- WLAN control block pointer
	*pStreamReq			- the TSPEC pointer

Return Value:
	None

Note:
	1. Free the TSPEC silently. Do NOT put it to the failed list.
========================================================================
*/
STATIC VOID ACM_TC_Discard(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq)
{
	UCHAR FlgIsActStm;
	UINT32 Direction;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	FlgIsActStm = 0;

	/*
		Remove the TSPEC from the requested list or the active table

		For Status == TSPEC_STATUS_ACT_DELETING, we should delete it in the
		requested list and the active table.
	*/
	if ((pStreamReq->Status == TSPEC_STATUS_REQUEST) ||
		(pStreamReq->Status == TSPEC_STATUS_REQ_DELETING) ||
		(pStreamReq->Status == TSPEC_STATUS_ACT_DELETING) ||
		(pStreamReq->Status == TSPEC_STATUS_RENEGOTIATING))
	{
		/* remove it from the requested list */
		ACM_TC_ReqRemove(pAd, pStreamReq);
	} /* End of if */

	if ((pStreamReq->Status == TSPEC_STATUS_ACTIVE) ||
		(pStreamReq->Status == TSPEC_STATUS_ACTIVE_SUSPENSION) ||
		(pStreamReq->Status == TSPEC_STATUS_ACT_DELETING))
	{
		/* also remove it from the active table */
		FlgIsActStm = 1;
		ACM_TC_ActRemove(pAd, pStreamReq);
	} /* End of if */

	/* reclaim the used time of the stream */
	Direction = pStreamReq->pTspec->TsInfo.Direction;

	if (FlgIsActStm == 1)
	{
		/* only for ACTIVE stream */
		ACM_EDCA_AllocatedTimeReturn(pAd, pStreamReq);

		/* statistics counter */
		ACM_LINK_NUM_DECREASE(pAd,
							pStreamReq->pTspec->TsInfo.AccessPolicy,
							Direction);

		/* delete peer device record if no any TSPEC for the peer */
		if (pStreamReq->pCdb != NULL)
			ACM_PeerDeviceMaintain(pAd, ACMR_CLIENT_MAC(pStreamReq->pCdb));
		/* End of if */
	} /* End of if */

	ACM_APSD_Ctrl(pAd, pStreamReq->pCdb,
					ACM_MR_EDCA_AC(pStreamReq->UP), Direction, 0, 0);

	/* free the stream */
	ACM_TC_Free(pAd, pStreamReq);

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Discard the TSPEC (out flag = %d) ok!\n",
                FlgIsActStm));

	/* count number of TSPEC */
	ACM_NUM_OF_TSPEC_RECOUNT(pAd, pStreamReq->pCdb);
} /* End of ACM_TC_Discard */


/*
========================================================================
Routine Description:
	Duplicate a stream.

Arguments:
	pAd					- WLAN control block pointer
	*pStream			- the source stream

Return Value:
	the duplicate stream

Note:
========================================================================
*/
STATIC ACM_STREAM *ACM_TC_Duplicate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream)
{
	ACM_STREAM *pTspecDup;
#ifdef ACM_CC_FUNC_TCLAS
	UINT32 IdTclasNum;
#endif // ACM_CC_FUNC_TCLAS //


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* allocate & copy the stream */
	ACMR_MEM_ALLOC(pTspecDup, sizeof(ACM_STREAM), (ACM_STREAM *));

	if (pTspecDup == NULL)
		return NULL;
	/* End of if */

	ACMR_MEM_COPY(pTspecDup, pStream, sizeof(ACM_STREAM));

	pTspecDup->pPrev = pTspecDup->pNext = NULL;

	/* allocate & copy TSPEC */
	ACMR_MEM_ALLOC(pTspecDup->pTspec, sizeof(ACM_TSPEC), (ACM_TSPEC *));

	if (pTspecDup->pTspec == NULL)
	{
		ACMR_MEM_FREE(pTspecDup);
		return NULL;
	} /* End of if */

	ACM_TSPEC_COPY(pTspecDup->pTspec, pStream->pTspec);

#ifdef ACM_CC_FUNC_TCLAS
	/* allocate & copy TCLAS */
	for(IdTclasNum=0; IdTclasNum<ACM_TSPEC_TCLAS_MAX_NUM; IdTclasNum++)
		pTspecDup->pTclas[IdTclasNum] = NULL;
	/* End of for */

	for(IdTclasNum=0; IdTclasNum<ACM_TSPEC_TCLAS_MAX_NUM; IdTclasNum++)
	{
		if (pStream->pTclas[IdTclasNum] != NULL)
		{
			ACMR_MEM_ALLOC(pTspecDup->pTclas[IdTclasNum],
							sizeof(ACM_TCLAS), (ACM_TCLAS *));

			if (pTspecDup->pTclas[IdTclasNum] == NULL)
				goto label_dup_err;
			/* End of if */

			ACM_TCLAS_COPY(pTspecDup->pTclas[IdTclasNum],
							pStream->pTclas[IdTclasNum]);
		} /* End of if */
	} /* End of for */
#endif // ACM_CC_FUNC_TCLAS //

	return pTspecDup;

#ifdef ACM_CC_FUNC_TCLAS
label_dup_err:
	ACM_FREE_TS(pTspecDup);
	return NULL;
#endif // ACM_CC_FUNC_TCLAS //
} /* End of ACM_TC_Duplicate */


/*
========================================================================
Routine Description:
	Find a stream by TS Info.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the QSTA
	*pTsInfo			- the TS Info

Return Value:
	the stream

Note:
	1. the search sequence must be REQ --> CDB
	2. we need STATION MAC information to compare because TS INFO can
		be the same for two different QSTAs.
	3. we only need pTsInfo->TSID to find a TS, other fields can be 0.
		So you can NOT use the function to check if same TS exists.
========================================================================
*/
STATIC ACM_STREAM *ACM_TC_Find(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo)
{
	ACM_STREAM *pStream;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (pDevMac == NULL)
		return NULL;
	/* End of if */

	/* search the TSPEC in the requested list */
	pStream = ACM_TC_FindInReq(pAd, pDevMac, pTsInfo);
	if (pStream != NULL)
		return pStream;
	/* End of if */

	/* search the TSPEC in the peer device database (input and output TSPEC) */
	pStream = ACM_TC_FindInPeer(pAd, pDevMac, pTsInfo);
	if (pStream != NULL)
		return pStream;
	/* End of if */

	return NULL;
} /* End of ACM_TC_Find */


/*
========================================================================
Routine Description:
	Find a stream in the peer record by TS Info.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the QSTA
	*pTsInfo			- the TS Info

Return Value:
	the stream

Note:
========================================================================
*/
STATIC ACM_STREAM *ACM_TC_FindInPeer(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo)
{
	ACM_STREAM *pStream;
	ACM_STREAM **ppAcmStmList;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UINT32 IdTidNum, IdLinkNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (pDevMac == NULL)
		return NULL;
	/* End of if */

	/* try to find it */
	for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
	{
		ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
										pAd, pDevMac, DirectionId[IdLinkNum]);
		if (ppAcmStmList == NULL)
			continue;
		/* End of if */

		for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
		{
			pStream = ppAcmStmList[IdTidNum];

			while(pStream != NULL)
			{
				if (ACM_IS_SAME_TS_INFOP(&pStream->pTspec->TsInfo, pTsInfo))
				{
					/* same TS INFO so we find it */
					return pStream;
				} /* End of if */

				/* only one TS for a direction/AC so the step can be skipped */
				/* pStream->pNext should be always NULL */
				pStream = pStream->pNext;
			} /* End of while */
		} /* End of for */
	} /* End of for */

	return NULL;
} /* End of ACM_TC_FindInPeer */


/*
========================================================================
Routine Description:
	Find a stream in the requested list by TS Info.

Arguments:
	pAd					- WLAN control block pointer
	*pDevMac			- the QSTA
	*pTsInfo			- the TS Info

Return Value:
	the stream

Note:
========================================================================
*/
STATIC ACM_STREAM *ACM_TC_FindInReq(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo)
{
	ACM_STREAM *pStream;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pStream = ACMR_CB->TspecListReq.pHead;

	while(pStream != NULL)
	{
		if ((AMR_IS_SAME_MAC(ACMR_CLIENT_MAC(pStream->pCdb), pDevMac)) &&
			(ACM_IS_SAME_TS_INFOP(&pStream->pTspec->TsInfo, pTsInfo)))
		{
			/* same peer and TS INFO so we find it */
			return pStream;
		} /* End of if */

		pStream = pStream->pNext;
	} /* End of while */

	return NULL;
} /* End of ACM_TC_FindInReq */


/*
========================================================================
Routine Description:
	Free a TSPEC.

Arguments:
	pAd					- WLAN control block pointer
	*pStream			- the stream

Return Value:
	ACM_RTN_OK			- free ok
	ACM_RTN_FAIL		- *pStream = NULL

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_Free(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream)
{
	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (pStream == NULL)
		return ACM_RTN_FAIL;
	/* End of if */

	/* free the TSPEC */
	ACM_FREE_TS(pStream);
	return ACM_RTN_OK;
} /* End of ACM_TC_Free */


/*
========================================================================
Routine Description:
	Get the user priority.

Arguments:
	*pTsInfo			- the TS Info element
	TclasNum			- the number of TCLASS, max 5
	*pTclas				- the requested TCLASS array pointer

Return Value:
	None

Note:
========================================================================
*/
STATIC UCHAR ACM_TC_UP_Get(
	ACM_PARAM_IN	ACM_TS_INFO				*pTsInfo,
	ACM_PARAM_IN	UINT32					TclasNum,
	ACM_PARAM_IN	ACM_TCLAS				*pTclas)
{
#ifdef ACM_CC_FUNC_TCLAS
	if (TclasNum > 0)
		return pTclas->UserPriority;
	/* End of if */
#endif // ACM_CC_FUNC_TCLAS //

	return pTsInfo->UP;
} /* End of ACM_TC_UP_Get */


/*
========================================================================
Routine Description:
	Rearrange a requested TSPEC in the request list.

Arguments:
	pAd					- WLAN control block pointer
	*pReqNew			- the requested TSPEC pointer
	Retry				- the retry count, base 0

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TC_Rearrange(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pReqNew,
	ACM_PARAM_IN	UINT16				Retry)
{
	ACM_STREAM *pStmPrev, *pStmNext;
	UINT32 TimeoutNew;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* reset the timeout & retry count */
	switch(pReqNew->Status)
	{
		case TSPEC_STATUS_REQUEST:
			/* the TSPEC is waiting for ADDTS response */
			TimeoutNew = pReqNew->TimeoutAddts;
			break;

		case TSPEC_STATUS_REQ_DELETING:
		case TSPEC_STATUS_ACT_DELETING:
			/* the TSPEC is waiting for DELTS response */
			TimeoutNew = pReqNew->TimeoutDelts;
			break;

		default:
			return; /* error status */
	} /* End of switch */

	pReqNew->Retry = Retry;


	/* check the number of requested TSPEC */
	if (ACMR_CB->TspecListReq.TspecNum <= 1)
	{
		pReqNew->Timeout = TimeoutNew;
		return; /* only a requested TSPEC, do NOT need to rearrange */
	} /* End of if */


	/* remove the requested TSPEC from the requested list */
	if (pReqNew->pPrev == NULL)
	{
		/* the TSPEC is the first one */
		pStmNext = pReqNew->pNext;

		if (pStmNext == NULL)
		{
			/* fatal error: only one requested TSPEC but TspecNum != 1 */
			/* prev = next = NULL */

			ACMR_CB->TspecListReq.TspecNum = 1; /* fix the Tspec number */
			pReqNew->Timeout = TimeoutNew;

			ACMR_DEBUG(ACMR_DEBUG_ERR,
						("acm_err> pNext == NULL! TC_Rearrange()\n"));
			return;
		} /* End of if */

		/* adjust the timeout for next request */
		pStmNext->Timeout += pReqNew->Timeout;
		pStmNext->pPrev = NULL;
		ACMR_CB->TspecListReq.pHead = pStmNext;
		ACMR_CB->TspecListReq.TspecNum --;
	}
	else
	{
		/* the TSPEC is not the first one */
		pStmPrev = pReqNew->pPrev;
		pStmNext = pReqNew->pNext;

		if (pStmNext == NULL)
		{
			/* the TSPEC is the last one so dont need to adjust timeout */
			pStmPrev->pNext = NULL;
			ACMR_CB->TspecListReq.pTail = pStmPrev;
			ACMR_CB->TspecListReq.TspecNum --;
		}
		else
		{
			/* the TSPEC is not the first one or the last one */
			pStmPrev->pNext = pStmNext;
			pStmNext->pPrev = pStmPrev;
			pStmNext->Timeout += pReqNew->Timeout;
			ACMR_CB->TspecListReq.TspecNum --;
		} /* End of if */
	} /* End of if */


	/* re-insert the requested TSPEC to the list */
	pReqNew->Timeout = TimeoutNew;
	ACM_TC_ReqInsert(pAd, pReqNew);
} /* End of ACM_TC_Rearrange */


/*
========================================================================
Routine Description:
	Check whether the requested TSPEC is a renegotiation TSPEC.

Arguments:
	pAd						- WLAN control block pointer
	*pDevMac				- the client MAC address
	UP						- the user priority of new stream
	*pTsInfo				- the TS Info element
	**ppStreamIn			- the old in TSPEC of same AC, can be NULL
	**ppStreamOut			- the old out TSPEC of same AC, can be NULL
	**ppStreamDifAc			- the old TSPEC of different AC, can be NULL

Return Value:
	ACM_RTN_OK 				- renegotiation TSPEC in active table or database
	ACM_RTN_RENO_IN_REQ_LIST- renegotiation TSPEC in the req list
	ACM_RTN_FAIL			- new TSPEC
	ACM_RTN_FATAL_ERR		- unexpected error occurred

Note:
	Maximum 2 TS can be existed in a AC, such as a uplink TS and a dnlink TS.

	04112008:
	Spec. metions that "The admission of any TS with the same TID as an
	existing TS deletes the existing TS and replaces it with the new TS,
	even if the TSs are in different ACs"

	So a new TSPEC can replace uplink TSPEC (same AC), dnlink TSPEC (same AC),
	bilink (same AC), uplink/dnlink/bilink (different AC, same TID)

	For example:
	(exist) 1) VI, TID3, bi
			2) VO, TID6, up
			3) VO, TID7, dn
	(new)	VO, TID3, bi

	==> delete all exist ones and use new one (VO, TID3, bi) in VO queue.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_RenegotiationCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	UCHAR				UP,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo,
	ACM_PARAM_OUT	ACM_STREAM			**ppStreamIn,
	ACM_PARAM_OUT	ACM_STREAM			**ppStreamOut,
	ACM_PARAM_OUT	ACM_STREAM			**ppStreamDifAc)
{
/* two TSPECs are the same when their TS Info are the same */
#define LMR_MEMCMP_RENE_TC(other_ts_info) \
	ACM_IS_SAME_TS_INFOP(&other_ts_info, pTsInfo)

	ACMR_STA_DB *pCdb;
	ACM_STREAM *pStreamReq;
	ACM_ENTRY_INFO *pStaAcmInfo;
	BOOLEAN FlgIsFindSameTspec;
	UINT32 IdTidNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	FlgIsFindSameTspec = FALSE;

	if (ppStreamIn != NULL)
		*ppStreamIn = NULL;
	/* End of if */
	if (ppStreamOut != NULL)
		*ppStreamOut = NULL;
	/* End of if */
	if (ppStreamDifAc != NULL)
		*ppStreamDifAc = NULL;
	/* End of if */

	pCdb = ACMR_STA_ENTRY_GET(pAd, pDevMac);
	if (pCdb == NULL)
		return ACM_RTN_FATAL_ERR;
	/* End of if */

	/* check wether no any requested TSPEC exists */
	if (ACMR_CB->TspecListReq.TspecNum <= 0)
	{
		/* no TSPEC requested list in QAP mode */
		goto label_active_check;
	} /* End of if */

	/* find the TSPEC in requested list */
	pStreamReq = ACMR_CB->TspecListReq.pHead;

	while(pStreamReq != NULL)
	{
		if (pStreamReq->pTspec == NULL)
		{
			/* fatal error: TSPEC pointer is NULL, delete it */
			ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> TSPEC = NULL! TC_RenegotiationCheck()\n"));
		}
		else
		{
			/* check whether TS Info element is same */
			if ((AMR_IS_SAME_MAC(ACMR_CLIENT_MAC(pStreamReq->pCdb), pDevMac)) &&
				(LMR_MEMCMP_RENE_TC(pStreamReq->pTspec->TsInfo)))
			{
				/* find the original TSPEC in the request list */
				return ACM_RTN_RENO_IN_REQ_LIST;
			} /* End of if */
		} /* End of if */

		/* check next outstanding requested TSPEC */
		pStreamReq = pStreamReq->pNext;
	} /* End of while */

label_active_check:
	/* check all existed output and input streams of the peer */
	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);

	/* find the TSPEC with same TID but different AC */
	for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
	{
		if (pStaAcmInfo->pAcStmOut[IdTidNum] != NULL)
		{
			pStreamReq = (ACM_STREAM *)pStaAcmInfo->pAcStmOut[IdTidNum];

			/* same TID but different same AC ID */
			if ((pStreamReq->pTspec->TsInfo.TSID == pTsInfo->TSID) &&
				(ACM_MR_EDCA_AC(pStreamReq->UP) != ACM_MR_EDCA_AC(UP)))
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> Same OUT TS! Exist TSID = %d "
							"dif UP = %d DIR = %d! "
							"New TSID = %d UP = %d DIR = %d! "
							"ACM_TC_RenegotiationCheck()\n",
							pStreamReq->pTspec->TsInfo.TSID,
							pStreamReq->UP,
							pStreamReq->pTspec->TsInfo.Direction,
							pTsInfo->TSID,
							UP,
							pTsInfo->Direction));

				FlgIsFindSameTspec = TRUE;

				if (ppStreamDifAc != NULL)
					*ppStreamDifAc = pStreamReq;
				/* End of if */

				/* only one possible stream with same TID and different AC */
				break;
			} /* End of if */
		} /* End of if */

		if (pStaAcmInfo->pAcStmIn[IdTidNum] != NULL)
		{
			pStreamReq = (ACM_STREAM *)pStaAcmInfo->pAcStmIn[IdTidNum];

			/* same TID but different same AC ID */
			if ((pStreamReq->pTspec->TsInfo.TSID == pTsInfo->TSID) &&
				(ACM_MR_EDCA_AC(pStreamReq->UP) != ACM_MR_EDCA_AC(UP)))
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> Same IN TS! Exist TSID = %d "
							"dif UP = %d DIR = %d! "
							"New TSID = %d UP = %d DIR = %d! "
							"ACM_TC_RenegotiationCheck()\n",
							pStreamReq->pTspec->TsInfo.TSID,
							pStreamReq->UP,
							pStreamReq->pTspec->TsInfo.Direction,
							pTsInfo->TSID,
							UP,
							pTsInfo->Direction));

				FlgIsFindSameTspec = TRUE;

				if (ppStreamDifAc != NULL)
					*ppStreamDifAc = pStreamReq;
				/* End of if */

				/* only one possible stream with same TID and different AC */
				break;
			} /* End of if */
		} /* End of if */
	} /* End of for */

	/* find the TSPEC with same TID or same AC */
	for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
	{
		if (pStaAcmInfo->pAcStmOut[IdTidNum] != NULL)
		{
			pStreamReq = (ACM_STREAM *)pStaAcmInfo->pAcStmOut[IdTidNum];

			/* skip ppStreamDifAc */
			if (ppStreamDifAc != NULL)
			{
				if (AMR_IS_SAME_POINTER(pStreamReq, (*ppStreamDifAc)))
					continue;
				/* End of if */
			} /* End of if */

			/* same TID or same AC ID */
			if (ACM_IS_SAME_TS(pStreamReq->pTspec->TsInfo.TSID,
								pTsInfo->TSID,
								pStreamReq->UP,
								UP,
								pStreamReq->pTspec->TsInfo.Direction,
								pTsInfo->Direction))
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> Same OUT TS! Exist TSID = %d "
							"UP = %d DIR = %d! "
							"New TSID = %d UP = %d DIR = %d! "
							"ACM_TC_RenegotiationCheck()\n",
							pStreamReq->pTspec->TsInfo.TSID,
							pStreamReq->UP,
							pStreamReq->pTspec->TsInfo.Direction,
							pTsInfo->TSID,
							UP,
							pTsInfo->Direction));

				FlgIsFindSameTspec = TRUE;

				if (ppStreamOut != NULL)
					*ppStreamOut = pStreamReq;
				/* End of if */

				if (pStreamReq->pTspec->TsInfo.Direction == \
													ACM_DIRECTION_BIDIREC_LINK)
				{
					/* for bi-directional link, check once is enough */
					return ACM_RTN_OK;
				} /* End of if */

				/* not return, try to find another input stream maybe */
			} /* End of if */
		} /* End of if */

		if (pStaAcmInfo->pAcStmIn[IdTidNum] != NULL)
		{
			pStreamReq = (ACM_STREAM *)pStaAcmInfo->pAcStmIn[IdTidNum];

			/* skip ppStreamDifAc */
			if (ppStreamDifAc != NULL)
			{
				if (AMR_IS_SAME_POINTER(pStreamReq, (*ppStreamDifAc)))
					continue;
				/* End of if */
			} /* End of if */

			/* same TID or same AC ID */
			if (ACM_IS_SAME_TS(pStreamReq->pTspec->TsInfo.TSID,
								pTsInfo->TSID,
								pStreamReq->UP,
								UP,
								pStreamReq->pTspec->TsInfo.Direction,
								pTsInfo->Direction))
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> Same IN TS! Exist TSID = %d "
							"UP = %d DIR = %d! "
							"New TSID = %d UP = %d DIR = %d! "
							"ACM_TC_RenegotiationCheck()\n",
							pStreamReq->pTspec->TsInfo.TSID,
							pStreamReq->UP,
							pStreamReq->pTspec->TsInfo.Direction,
							pTsInfo->TSID,
							UP,
							pTsInfo->Direction));

				FlgIsFindSameTspec = TRUE;

				if (ppStreamIn != NULL)
					*ppStreamIn = pStreamReq;
				/* End of if */

				if (pStreamReq->pTspec->TsInfo.Direction == \
													ACM_DIRECTION_BIDIREC_LINK)
				{
					/* for bi-directional link, check once is enough */
					return ACM_RTN_OK;
				} /* End of if */

				/* not return, try to find another output stream maybe */
			} /* End of if */
		} /* End of if */
	} /* End of for */

	if (FlgIsFindSameTspec == TRUE)
		return ACM_RTN_OK; /* find one */
	/* End of if */

	return ACM_RTN_FAIL;
} /* End of ACM_TC_RenegotiationCheck */


#ifdef ACM_CC_FUNC_REPLACE_RULE_TG
/*
========================================================================
Routine Description:
	Check whether the replacement TSPEC can be accepted.

Arguments:
	pAd						- WLAN control block pointer
	*pDevMac				- the client MAC address
	UP						- the user priority of new stream
	*pTsInfo				- the TS Info element

Return Value:
	ACM_RTN_OK 				- accept
	ACM_RTN_FAIL			- reject
	ACM_RTN_FATAL_ERR		- unexpected error occurred

Note:
	We can not accept a replacement TSPEC from QSTA if
	1. same TID, but not same AC; or
	2. same TID, same AC, but not same Direction;
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_ReplacementCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	UCHAR				UP,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo)
{
	ACMR_STA_DB *pCdb;
	ACM_STREAM *pStreamReq;
	ACM_TS_INFO *pTsInfoOld;
	ACM_ENTRY_INFO *pStaAcmInfo;
	UINT32 IdTidNum, IdDirNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pCdb = ACMR_STA_ENTRY_GET(pAd, pDevMac);
	if (pCdb == NULL)
		return ACM_RTN_FATAL_ERR;
	/* End of if */

	pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(pCdb);

	/* check for all TSPECs in input and output TSPECs */
	for(IdDirNum=0; IdDirNum<2; IdDirNum++)
	{
		for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
		{
			if (IdDirNum == 0)
				pStreamReq = (ACM_STREAM *)pStaAcmInfo->pAcStmOut[IdTidNum];
			else
				pStreamReq = (ACM_STREAM *)pStaAcmInfo->pAcStmIn[IdTidNum];
			/* End of if */

			if (pStreamReq != NULL)
			{
				pTsInfoOld = &pStreamReq->pTspec->TsInfo;

				/* same TID but different same AC ID */
				if ((pTsInfoOld->TSID == pTsInfo->TSID) &&
					(ACM_MR_EDCA_AC(pStreamReq->UP) != ACM_MR_EDCA_AC(UP)))
				{
					/* match condition 1 */
					return ACM_RTN_FAIL;
				} /* End of if */

			} /* End of if */
		} /* End of for */
	} /* End of for */

	return ACM_RTN_OK;
} /* End of ACM_TC_ReplacementCheck */
#endif // ACM_CC_FUNC_REPLACE_RULE_TG //


/*
========================================================================
Routine Description:
	Change ADDTS state to DELTS state.

Arguments:
	pAd					- WLAN control block pointer
	*pStreamReq			- the requested TSPEC pointer

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TC_Req_ADDTS2DELTS(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq)
{
	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* change its state to deleting mode */
	pStreamReq->Status = TSPEC_STATUS_REQ_DELETING;
	pStreamReq->TimeoutAction = ACM_TC_TIMEOUT_ACTION_DELTS;
	pStreamReq->Timeout = pStreamReq->TimeoutDelts;
	pStreamReq->Retry = ACM_MAX_NUM_OF_DELTS_RETRY;

	/* rearrange the requested TSPEC in the requested list */
	ACM_TC_Rearrange(pAd, pStreamReq, ACM_MAX_NUM_OF_DELTS_RETRY);
} /* End of ACM_TC_Req_ADDTS2DELTS */


/*
========================================================================
Routine Description:
	Check if another link or same link exists in the requested list.

Arguments:
	pAd					- WLAN control block pointer
	*pStream			- the requested or actived stream

Return Value:
	ACM_RTN_EXIST		- same link exists
	ACM_RTN_NOT_EXIST	- no same link exists

Note:
	Same link means same TS info and same QSTA MAC.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_ReqCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream)
{
	ACM_STREAM *pAcmStmList;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pAcmStmList = ACMR_CB->TspecListReq.pHead;

	while(pAcmStmList != NULL)
	{
		/* check whether they are from same QSTA */
		if (AMR_IS_SAME_POINTER(pAcmStmList->pCdb, pStream->pCdb))
		{
			/* check whether TSPEC is the same */
			if (ACM_IS_SAME_TSPEC(pAcmStmList->pTspec, pStream->pTspec))
				return ACM_RTN_EXIST;
			/* End of if */
		} /* End of if */

		pAcmStmList = pAcmStmList->pNext;
	} /* End of while */

	return ACM_RTN_NOT_EXIST;
} /* End of ACM_TC_ReqCheck */


/*
========================================================================
Routine Description:
	Free all requested TSPEC.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TC_ReqAllFree(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd)
{
	ACM_TSPEC_REQ_LIST *pStmReqList;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	pStmReqList = &ACMR_CB->TspecListReq;
	ACM_LIST_ALL_FREE(pAd, pStmReqList);
} /* End of ACM_TC_ReqAllFree */


/*
========================================================================
Routine Description:
	Free requested TSPEC for the peer device.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the destination device entry

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TC_ReqDeviceFree(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
 	ACM_PARAM_IN	ACMR_STA_DB			*pCdb)
{
	ACM_TSPEC_REQ_LIST *pStmReqList;
	ACM_STREAM *pStreamReq, *pStreamReqNext;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pStmReqList = &ACMR_CB->TspecListReq;
	pStreamReq = pStmReqList->pHead;

	/* loop for all request TSPEC */
	while(pStreamReq != NULL)
	{
		pStreamReqNext = pStreamReq->pNext;

		/* check whether same device MAC */
		if (AMR_IS_SAME_CDB(pStreamReq->pCdb, pCdb))
		{
			/* remove the request from the request list */
			ACM_TC_ReqRemove(pAd, pStreamReq);

			/* free it */
			ACM_TC_Free(pAd, pStreamReq);
		} /* End of if */

		pStreamReq = pStreamReqNext;
	} /* End of while */
} /* End of ACM_TC_ReqDeviceFree */


/*
========================================================================
Routine Description:
	Insert a requested TSPEC to the request list.

Arguments:
	pAd					- WLAN control block pointer
	*pReqNew			- the requested TSPEC pointer

Return Value:
	ACM_RTN_OK			- insert ok
	ACM_RTN_FAIL		- insert fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_ReqInsert(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pReqNew)
{
	ACM_TSPEC_REQ_LIST *pStmReqList;
	ACM_STREAM *pStmReq;
	ACM_STREAM *pStmSwapUse;
	UINT32 NumTimeout;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pStmReqList = &ACMR_CB->TspecListReq;
	pReqNew->pPrev = NULL;
	pReqNew->pNext = NULL;
	NumTimeout = 0;

	/* insert */
	if (pStmReqList->pHead == NULL)
	{
		/* no any outgoing requested TSPEC exists */
		pStmReqList->pHead = pReqNew;
		pStmReqList->pTail = pReqNew;
		pStmReqList->TspecNum = 1;
	}
	else
	{
		/* check if the request exists */
		if (ACM_TC_ReqCheck(pAd, pReqNew) == ACM_RTN_EXIST)
		{
			ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> req already exist!\n"));
			return ACM_RTN_FAIL; /* the request already exists */
		} /* End of if */

		/* at least one outgoing requested TSPEC exists */
		pStmReq = pStmReqList->pHead;

		while(pStmReq != NULL)
		{
			/* calculate timeout sum */
			NumTimeout += pStmReq->Timeout;

			if (pReqNew->Timeout <= NumTimeout)
			{
				/* insert the new requested TSPEC */

				/* adjust request timeout */
				pReqNew->Timeout -= (NumTimeout - pStmReq->Timeout);
				pStmReq->Timeout -= pReqNew->Timeout;

				if (pStmReq->pPrev == NULL)
				{
					/* the old request is the first one */
					pStmReqList->pHead = pReqNew;
					pReqNew->pNext = pStmReq;
					pStmReq->pPrev = pReqNew;
				}
				else
				{
					/* the old request is not the first one */
					pStmSwapUse = pStmReq->pPrev;
					pStmSwapUse->pNext = pReqNew;
					pReqNew->pPrev = pStmSwapUse;
					pReqNew->pNext = pStmReq;
					pStmReq->pPrev = pReqNew;
				} /* End of if */

				/* a new requested TSPEC is added */
				pStmReqList->TspecNum ++;
				return ACM_RTN_OK;
			} /* End of if */

			/* move to next requested TSPEC */
			pStmReq = pStmReq->pNext;
		} /* End of while */

		/* insert it to the last one */
		pStmReq = pStmReqList->pTail;
		pStmReq->pNext = pReqNew;
		pReqNew->pPrev = pStmReq;
		pStmReqList->pTail = pReqNew;

		pReqNew->Timeout -= NumTimeout;
		pStmReqList->TspecNum ++;
	} /* End of if */

	return ACM_RTN_OK;
} /* End of ACM_TC_ReqInsert */


/*
========================================================================
Routine Description:
	Remove a requested TSPEC from the request list.

Arguments:
	pAd					- WLAN control block pointer
	*pStreamReq			- the requested TSPEC pointer

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TC_ReqRemove(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq)
{
	ACM_TSPEC_REQ_LIST *pStmReqList;
	ACM_STREAM *pAcmStmList;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pStmReqList = &ACMR_CB->TspecListReq;
	pAcmStmList = pStmReqList->pHead;

	/* check whether the request exists in the requested list */
	while(pAcmStmList != NULL)
	{
		if (AMR_IS_SAME_POINTER(pAcmStmList, pStreamReq))
			break; /* find it */
		/* End of if */

		pAcmStmList = pAcmStmList->pNext;
	} /* End of while */

	if (pAcmStmList == NULL)
	{
		/* fatal error, can NOT find the request */
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> The stream is not in the requested list! "
					"TC_ReqRemove()\n"));
		return;
	} /* End of if */

	/* remove it */
	if (AMR_IS_SAME_POINTER(pStmReqList->pHead, pStreamReq))
	{
		/* the requested TSPEC is the first one */
		if (AMR_IS_SAME_POINTER(pStmReqList->pTail, pStreamReq))
		{
			/* the requested TSPEC is also the last one */
			pStmReqList->pHead = pStmReqList->pTail = NULL;
			pStmReqList->TspecNum = 0;
			return;
		} /* End of if */

		/* here, exist at least two requests */
		pStmReqList->pHead = pStreamReq->pNext;
		(pStreamReq->pNext)->pPrev = NULL;
		(pStreamReq->pNext)->Timeout += pStreamReq->Timeout;
		pStmReqList->TspecNum --;
		return;
	} /* End of if */

	if (AMR_IS_SAME_POINTER(pStmReqList->pTail, pStreamReq))
	{
		/* the requested TSPEC is the last one */
		pStmReqList->pTail = pStreamReq->pPrev;
		(pStreamReq->pPrev)->pNext = NULL;
		pStmReqList->TspecNum --;
		return;
	} /* End of if */

	/* the requested TSPEC is not either the first one or the last one */
	(pStreamReq->pPrev)->pNext = pStreamReq->pNext;
	(pStreamReq->pNext)->pPrev = pStreamReq->pPrev;
	(pStreamReq->pNext)->Timeout += pStreamReq->Timeout;
	pStmReqList->TspecNum --;
} /* End of ACM_TC_ReqRemove */


/*
========================================================================
Routine Description:
	Release all activated TSPECs without DELTS.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	Used in module remove only.
========================================================================
*/
STATIC VOID ACM_TC_ReleaseAll(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd)
{
	ACM_TSPEC_REQ_LIST *pTspecFreedList;
	ACM_STREAM **ppAcmStmList;
	ACM_PEER_DEV_LIST *pAcmDevList;
	UCHAR DirectionId[2] = \
					{ ACM_PEER_TSPEC_OUTPUT_GET, ACM_PEER_TSPEC_INPUT_GET };
	UCHAR MAC[ACM_MAC_ADDR_LEN];
	UINT32 IdTidNum, IdLinkNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	if (ACMR_ADAPTER_DB == NULL)
		return;
	/* End of if */

	/* clean all requested TSPEC if exists */
	ACM_TC_ReqAllFree(pAd);

	/* free all streams for all peer devices */
	pAcmDevList = NULL; /* get first device */

	while(1)
	{
		/* get next device */
		if (ACM_PeerDeviceGetNext(pAd, &pAcmDevList, MAC) != ACM_RTN_OK)
			break;
		/* End of if */

		/* delete all streams for device */
		for(IdLinkNum=0; IdLinkNum<2; IdLinkNum++)
		{
			ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
													pAd, MAC, DirectionId[IdLinkNum]);
			if (ppAcmStmList == NULL)
				break;
			/* End of if */

			for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
			{
				if (ppAcmStmList[IdTidNum] != NULL)
				{
					if ((IdLinkNum != 0) &&
						(ppAcmStmList[IdTidNum]->pTspec->TsInfo.Direction == \
													ACM_DIRECTION_BIDIREC_LINK))
					{
						/*
							We only backup one stream for IdLinkNum = 0
							for bi-dir link so we can not free it twice.
						*/
					}
					else
						ACM_TC_Free(pAd, ppAcmStmList[IdTidNum]);
					/* End of if */

					/* empty the record */
					ppAcmStmList[IdTidNum] = NULL;
				} /* End of if */
			} /* End of for */
		} /* End of for */
	} /* End of while */

	/* free all backup peer device entries */
	ACM_PeerDeviceAllFree(pAd);

	/* free all failed streams in the failed list */
	pTspecFreedList = &ACMR_CB->TspecListFail;
	ACM_LIST_ALL_FREE(pAd, pTspecFreedList);
} /* End of ACM_TC_ReleaseAll */


/*
========================================================================
Routine Description:
	Activate periodically.

Arguments:
	Data

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TASK_General(
	ACM_PARAM_IN	ULONG		Data)
{
	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC)Data;


#if defined(ACM_CC_FUNC_MBSS) || defined(ACM_CC_FUNC_CHAN_UTIL_MONITOR)
	ULONG SplFlags;


	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);
#endif // ACM_CC_FUNC_MBSS || ACM_CC_FUNC_CHAN_UTIL_MONITOR //

	pAd = pAd; /* avoid compile warning */

	WMM_ACM_FUNC_NAME_PRINT("IN");

#ifdef ACM_CC_FUNC_MBSS
	ACMR_CB->TimeoutMbssAcm ++;

	if (ACMR_CB->TimeoutMbssAcm >= ACM_MBSS_BW_ANNONCE_TIMEOUT_NUM)
	{
		ACM_TC_TASK_BwAnn(Data);

		ACMR_CB->TimeoutMbssAcm = 0;
	} /* End of if */
#endif // ACM_CC_FUNC_MBSS //

#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_CHAN_UTIL_MONITOR
	/* TODO: maybe we can check at least one TSPEC exist */
	if (ACMP_IsAnyACEnabled(pAd))
	{
		/* only do the function when at least one AC ACM is set */
		ACMR_CB->CU_MON_Timeout ++;

		if (ACMR_CB->CU_MON_Timeout >= ACM_CH_MON_TIMEOUT_NUM)
		{
			ACM_TC_TASK_CU_Mon(Data);

			ACMR_CB->CU_MON_Timeout = 0;
		} /* End of if */
	}
	else
	{
		/* reset all parameters */
		ACMR_CB->CU_MON_Timeout = 0;

		ACMR_CB->CU_MON_FlgLastMode = ACM_CU_MON_MODE_RECOVER;
		ACMR_CB->CU_MON_AdjustCount = 0;
		ACMR_CB->CU_MON_AdjustNum = 0;
		ACMR_CB->CU_MON_RecoverCount = 0;

		ACMR_AIFSN_DEFAULT_GET(pAd, ACMR_CB->CU_MON_AifsnAp, ACMR_CB->CU_MON_AifsnBss);

		ACMR_CB->CU_MON_FlgChangeNeed = 1;
	} /* End of if */
#endif // ACM_CC_FUNC_CHAN_UTIL_MONITOR //
#endif // CONFIG_AP_SUPPORT //

#if defined(ACM_CC_FUNC_MBSS) || defined(ACM_CC_FUNC_CHAN_UTIL_MONITOR)
	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);

LabelSemErr:
	ACMR_TIMER_ENABLE(ACMR_CB->FlgTimerGeneralEnable,
						ACMR_CB->TimerGeneral,
						ACM_TIMER_GENERAL_PERIOD_TIMEOUT);
	return;
#endif
} /* End of ACM_TASK_General */


/*
========================================================================
Routine Description:
	Check if TSPEC requests are timeout.

Arguments:
	Data

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_TASK_TC_ReqCheck(
	ACM_PARAM_IN	ULONG		Data)
{
#define TASK_LMR_STREAM_DESTROY(pAd, pStreamReq, FlgIsActiveExcluded)	\
	pCdb = pStreamReq->pCdb;											\
	StmAcId = pStreamReq->AcmAcId;										\
	ACM_TC_Destroy(pAd, pStreamReq, FlgIsActiveExcluded);

	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC)Data;
	ACM_TSPEC_REQ_LIST *pStmReqList;
	ACM_STREAM *pStreamReq, *pNext;
	ACMR_STA_DB *pCdb;
	UCHAR StmAcId;
	UCHAR *pFrameBuf;
	UINT32 FrameLen;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* get management semaphore */
	ACM_TSPEC_IRQ_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* check if no any requested TSPEC exists */
	pStmReqList = &ACMR_CB->TspecListReq;

	if (pStmReqList->TspecNum == 0)
		goto LabelDisableTimer;
	/* End of if */

	pStreamReq = pStmReqList->pHead;
	if (pStreamReq == NULL)
		goto LabelDisableTimer;
	/* End of if */

	/* sanity check for timeout of 1st requested TSPEC */
	if (pStreamReq->Timeout <= 0)
	{
		/* error: the timeout == 0 */
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> timeout of 1st TSPEC == 0! "
					"TC_TASK_ReqCheck()\n"));

		/* reset timeout */
		pStreamReq->Timeout = 1;
	} /* End of if */

	/* subtract 1 from timeout */
	pStreamReq->Timeout --;

	if (pStreamReq->Timeout > 0)
	{
		/* no timeout occurred */
		goto LabelSemRelease;
	} /* End of if */

	/* handle TSPEC timeout */
	while(pStreamReq != NULL)
	{
		/*
			Backup next requested stream (must put here), because pStreamReq
			maybe moved to the last one in the list by ACM_TC_Rearrange().

			If pStreamReq is moved to the last one, the pStreamReq->pNext
			will be NULL.
		*/
		pNext = pStreamReq->pNext;

		/* init */
		pCdb = NULL;
		StmAcId = 0;
		pFrameBuf = NULL;
		FrameLen = 0;

		/* check retry count */
		if (pStreamReq->Retry > 0)
		{
			/* subtract 1 from retry count */
			pStreamReq->Retry --;

			/* execute timeout action for the TSPEC (retransmit) */
			switch(pStreamReq->TimeoutAction)
			{
				case ACM_TC_TIMEOUT_ACTION_DELTS:
					/* re-send a DELTS frame! */
					ACMR_DEBUG(ACMR_DEBUG_TRACE,
								("acm_msg> DELTS timeout! TX a DELTS frame! "
								"TC_TASK_ReqCheck()\n"));
					ACM_DELTS_SEND(pAd, pStreamReq->pCdb, pStreamReq, LabelSemErr);
					ACM_TC_Rearrange(pAd, pStreamReq, pStreamReq->Retry);
					break;

				case ACM_TC_TIMEOUT_ACTION_ADDTS_REQ:

#ifdef CONFIG_AP_SUPPORT
					/* for QAP, the type is error type, so delete the request */
					TASK_LMR_STREAM_DESTROY(pAd, pStreamReq, 0);
#endif // CONFIG_AP_SUPPORT //
					break;

				default:
					/* error timeout action, delete it */
					ACMR_DEBUG(ACMR_DEBUG_TRACE,
								("acm_err> error timeout action! "
								"TASK_TC_Req_Check()\n"));
					TASK_LMR_STREAM_DESTROY(pAd, pStreamReq, 0);
					break;
			} /* End of switch */
		}
		else
		{
			/* reach retry limit count */
			switch(pStreamReq->TimeoutAction)
			{
				case ACM_TC_TIMEOUT_ACTION_DELTS:
				default:
					/* we do not yet take care about ACK frame of DELTS */


#ifdef CONFIG_AP_SUPPORT
					ACMR_DEBUG(ACMR_DEBUG_TRACE,
								("acm_msg> DELTS tx limit! "
								"Delete the request! "
								"TC_TASK_ReqCheck()\n"));

					/*
						We can not delete activated TSPEC after DELTS timeout
						in AP if PS STA does not use trigger frame or PS-Poll
						frame to get the DELTS frame.

						So we just only delete TSPEC request, not TSPEC active.

						EX: UAPSD of VO is enabled, all DELTS will be put
						in VO queue. DELTS will not sent from AP if STA does
						not send any trigger frame.
					*/
					TASK_LMR_STREAM_DESTROY(pAd, pStreamReq, 1);
#endif // CONFIG_AP_SUPPORT //
					break;

				case ACM_TC_TIMEOUT_ACTION_ADDTS_REQ:
					/* change to delts action in the requested list */
					ACMR_DEBUG(ACMR_DEBUG_TRACE,
								("acm_msg> ADDTS timeout! TX a DELTS frame! "
								"TC_TASK_ReqCheck()\n"));
					ACM_TC_Req_ADDTS2DELTS(pAd, pStreamReq);
					ACM_DELTS_SEND(pAd, pStreamReq->pCdb, pStreamReq, LabelSemErr);
					break;
			} /* End of switch */


			/* send ADDTS Request frame again */
			if (FrameLen > 0)
			{
				ACM_ADDREQ_SEND(pAd, pFrameBuf, FrameLen);
			} /* End of if */
		} /* End of if */

		/* check next requested TSPEC */
		pStreamReq = pNext;

		if ((pStreamReq != NULL) && (pStreamReq->Timeout > 0))
		{
			/* yet timeout for next request */
			break;
		} /* End of if */
	} /* End of while */

LabelSemRelease:
	/* release semaphore */
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);

#ifdef ACMR_HANDLE_IN_TIMER
	/* schedule next time */
	ACMR_TIMER_ENABLE(ACMR_CB->FlgTspecReqCheckEnable,
						ACMR_CB->TimerTspecReqCheck,
						ACM_STREAM_CHECK_OFFSET);
#endif // ACMR_HANDLE_IN_TIMER //
	return;

LabelDisableTimer:
	ACM_TSPEC_IRQ_UNLOCK(pAd, SplFlags, LabelSemErr);

#ifndef ACMR_HANDLE_IN_TIMER
	/* give up schedule next time */
	ACMR_TIMER_DISABLE(ACMR_CB->FlgTspecReqCheckEnable,
						ACMR_CB->TimerTspecReqCheck);
#endif // ACMR_HANDLE_IN_TIMER //
	return;

LabelSemErr:
	ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Semaphore Lock! TC_TASK_ReqCheck()\n"));

#ifdef ACMR_HANDLE_IN_TIMER
	/* schedule next time */
	ACMR_TIMER_ENABLE(ACMR_CB->FlgTspecReqCheckEnable,
						ACMR_CB->TimerTspecReqCheck,
						ACM_STREAM_CHECK_OFFSET);
#endif // ACMR_HANDLE_IN_TIMER //
	return;
} /* End of ACM_TASK_TC_ReqCheck */


/*
========================================================================
Routine Description:
	Get IP information from the frame.

Arguments:
	*pPkt				- the IP header
	*pTclas				- the IP information

Return Value:
	ACM_RTN_OK			- get ok
	ACM_RTN_FAIL		- get fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TCLAS_IP_INFO_Get(
	ACM_PARAM_IN	UCHAR			*pPkt,
	ACM_PARAM_OUT	ACM_TCLAS		*pTclas)
{
	ACM_IPHDR *pIpHdr;
	UINT16 Type;
	UINT16 *pPortSrc;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	Type = *(UINT16 *)pPkt;

	if (Type != ACMR_HTONS(0x0800)) /* 0800: IP packet type */
		return ACM_RTN_FAIL;
	/* End of if */

	pIpHdr = (ACM_IPHDR *)(pPkt+2);

	pTclas->Clasifier.IPv4.Version = pIpHdr->Version;
	pTclas->Clasifier.IPv4.IpSource = pIpHdr->AddrSrc;
	pTclas->Clasifier.IPv4.IpDest = pIpHdr->AddrDst;
	pTclas->Clasifier.IPv4.DSCP = pIpHdr->TOS;
	pTclas->Clasifier.IPv4.Protocol = pIpHdr->Protocol;

	pPortSrc = (UINT16 *)((UCHAR *)pIpHdr + (pIpHdr->IHL<<2));
	pTclas->Clasifier.IPv4.PortSource = *pPortSrc;
	pTclas->Clasifier.IPv4.PortDest = *(pPortSrc+1);
	return ACM_RTN_OK;
} /* End of ACM_TCLAS_IP_INFO_Get */


/*
========================================================================
Routine Description:
	Get VLAN information from the frame.

Arguments:
	*pPkt				- the frame
	*pVlanTag			- the VLAN Tag of the frame

Return Value:
	ACM_RTN_OK			- get ok
	ACM_RTN_FAIL		- get fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TCLAS_VLAN_INFO_Get(
	ACM_PARAM_IN	UCHAR			*pPkt,
	ACM_PARAM_OUT	UINT16			*pVlanTag)
{
	UINT16 Type;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	Type = *(UINT16 *)pPkt;

	if (Type != ACMR_HTONS(0x8100)) /* 8100: VLAN packet type */
		return ACM_RTN_FAIL;
	/* End of if */

	*pVlanTag = *(UINT16 *)(&pPkt[2]);
	return ACM_RTN_OK;
} /* End of ACM_TCLAS_VLAN_INFO_Get */


/*
========================================================================
Routine Description:
	Activate periodically.

Arguments:
	Data

Return Value:
	None

Note:
========================================================================
*/
VOID ACM_TR_TC_General(
	ACM_PARAM_IN	ULONG		Data)
{
#ifndef ACMR_HANDLE_IN_TIMER
	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC)Data;


	ACMR_TASK_ACTIVATE(ACMR_CB->TaskletGeneral,
						ACMR_CB->TimerGeneral,
						ACM_TIMER_GENERAL_PERIOD_TIMEOUT);
#else

	ACM_TASK_General(Data);
#endif // ACMR_HANDLE_IN_TIMER //
} /* End of ACM_TR_TC_General */


/*
========================================================================
Routine Description:
	Check whether TSPEC request is timeout.
	If timeout, move it to the failure list.

Arguments:
	Data

Return Value:
	None

Note:
	1. waked up every 100ms
	2. for example, the request list is
		timeout      = 0 --> 0 --> 4 --> 6 --> 2
		real timeout = 0ms, 0ms, 400ms, 1000ms, 1200ms
========================================================================
*/
VOID ACM_TR_TC_ReqCheck(
	ACM_PARAM_IN	ULONG		Data)
{
#ifndef ACMR_HANDLE_IN_TIMER
	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC)Data;


	/* sanity check for enable flag */
	if (ACMR_CB->FlgTspecReqCheckEnable == 0)
	{
		ACMR_TIMER_DISABLE(ACMR_CB->FlgTspecReqCheckEnable,
							ACMR_CB->TimerTspecReqCheck);
		return;
	} /* End of if */

	/* inform TSPEC request check task */
	ACMR_TASK_ACTIVATE(ACMR_CB->TaskletTspecReqCheck,
						ACMR_CB->TimerTspecReqCheck,
						ACM_STREAM_CHECK_OFFSET);
#else

	ACM_TASK_TC_ReqCheck(Data);
#endif // ACMR_HANDLE_IN_TIMER //
} /* End of ACM_TR_TC_ReqCheck */




/* ============================= Chan Util Monitor ====================== */
#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_CHAN_UTIL_MONITOR
/*
========================================================================
Routine Description:
	Adjust AIFSN of non-ACM AC when channel utilization is too high.

Arguments:
	Data

Return Value:
	None

Note:
	To achieve the maximum throughput and short delay, CUmax should be
	set in the range of 0.9 to 0.95 (RTS/CTS).
	Where CUmax is maximum channel utilization.

	Fast adjust and slow recover.
========================================================================
*/
STATIC VOID ACM_TC_TASK_CU_Mon(
	ACM_PARAM_IN	ULONG		Data)
{
	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC)Data;
	ACM_CTRL_PARAM *pEdcaParam;
	UINT32 CUmax;
	UINT32 IdAcNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pEdcaParam = &ACMR_CB->EdcaCtrlParam;
	CUmax = (255 * ACM_CH_MON_CUMAX)/100;

	/* handle new chan utilization */
	if (ACMR_CHAN_UTILIZATION_GET(pAd) >= CUmax)
	{
		ACMR_CB->CU_MON_AdjustCount ++;
		ACMR_CB->CU_MON_RecoverCount = 0;
	}
	else
	{
		ACMR_CB->CU_MON_AdjustCount = 0;
		ACMR_CB->CU_MON_RecoverCount ++;
	} /* End of if */

	/* check if we need to adjust AIFSN of non-ACM AC */
	if (ACMR_CB->CU_MON_AdjustCount >= ACM_CH_MON_ADJUST_NUM)
	{
		ACMR_CB->CU_MON_AdjustNum ++;

		if (ACMR_CB->CU_MON_AdjustNum < ACM_CH_MON_MAX_ADJUST)
		{
			ACMR_CB->CU_MON_FlgLastMode = ACM_CU_MON_MODE_ADJUST;

			for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
			{
				if (pEdcaParam->FlgAcmStatus[IdAcNum] == 0)
				{
					if (ACMR_CB->CU_MON_AifsnAp[IdAcNum] < 0x0e)
					{
						ACMR_CB->CU_MON_FlgChangeNeed = 1;
						ACMR_CB->CU_MON_AifsnAp[IdAcNum] += 2;
					} /* End of if */

					if (ACMR_CB->CU_MON_AifsnBss[IdAcNum] < 0x0e)
					{
						ACMR_CB->CU_MON_FlgChangeNeed = 1;
						ACMR_CB->CU_MON_AifsnBss[IdAcNum] += 2;
					} /* End of if */

					if (ACMR_CB->CU_MON_FlgChangeNeed)
					{
						ACMR_DEBUG(ACMR_DEBUG_TRACE,
								("acm_msg> chan busy, adjust AC%d...\n", IdAcNum));
					} /* End of if */
				} /* End of if */
			} /* End of for */
		} /* End of if */

		ACMR_CB->CU_MON_AdjustCount = 0; /* re-check */
	} /* End of if */

	/* check if we need to recover AIFSN of non-ACM AC */
	if (ACMR_CB->CU_MON_RecoverCount >= ACM_CH_MON_RECOVER_NUM)
	{
		UCHAR aifns_ap[ACM_DEV_NUM_OF_AC];
		UCHAR aifns_bss[ACM_DEV_NUM_OF_AC];


		ACMR_AIFSN_DEFAULT_GET(pAd, aifns_ap, aifns_bss);

		for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
		{
			if (pEdcaParam->FlgAcmStatus[IdAcNum] == 0)
			{
				if (ACMR_CB->CU_MON_AifsnAp[IdAcNum] > aifns_ap[IdAcNum])
				{
					ACMR_CB->CU_MON_FlgChangeNeed = 1;
					ACMR_CB->CU_MON_AifsnAp[IdAcNum] --;
				} /* End of if */

				if (ACMR_CB->CU_MON_AifsnBss[IdAcNum] > aifns_bss[IdAcNum])
				{
					ACMR_CB->CU_MON_FlgChangeNeed = 1;
					ACMR_CB->CU_MON_AifsnBss[IdAcNum] --;
				} /* End of if */

				if (ACMR_CB->CU_MON_FlgChangeNeed)
				{
					ACMR_DEBUG(ACMR_DEBUG_TRACE,
								("acm_msg> chan idle, recover AC%d...\n", IdAcNum));
				} /* End of if */
			} /* End of if */
		} /* End of for */

		ACMR_CB->CU_MON_AdjustNum = 0;
		ACMR_CB->CU_MON_RecoverCount = 0; /* re-check */
	} /* End of if */
} /* End of ACM_TC_TASK_CU_Mon */
#endif // ACM_CC_FUNC_CHAN_UTIL_MONITOR //
#endif // CONFIG_AP_SUPPORT //




/* ============================= MBSS function ========================== */
/*
========================================================================
Routine Description:
	Send a broadcast Bandwidth Annonce frame.

Arguments:
	pAd					- WLAN control block pointer
	FlgIsForceToSent	- 1: forece to send the frame

Return Value:
	None

Note:
	1. Carefully! This is a broadcast frame and must be non-encryption.
	2. The frame is only sent by AP. STA only forward it.
	3. AP A <--> AP B <--> AP C
		AP B only need to broadcast its used ACM time, no AP A used time.
		Because AP C does not see AP A.
========================================================================
*/
STATIC VOID ACM_FrameBwAnnSend(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsForceToSent)
{
#ifdef ACM_CC_FUNC_MBSS
#ifdef CONFIG_AP_SUPPORT
	ACM_CTRL_PARAM		*pEdcaParam;
	ACM_BW_ANN_FRAME	*pFrameBwAnn;
	ACMR_WLAN_HEADER	HdrActionFrame;
	ULONG				FrameLen;
	UCHAR				*pBufFrame;
	UCHAR				MAC_BC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	UINT32				IdAcNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* sanity check */
	pEdcaParam = &ACMR_CB->EdcaCtrlParam;

	if (FlgIsForceToSent == FALSE)
	{
		if (ACMR_CB->AcmTotalTimeOld == pEdcaParam->AcmTotalTime)
			return; /* same acm time, no need to announce */
		/* End of if */
	} /* End of if */

	/* init */
	FrameLen = 0;

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> Make up a BW ANN...\n"));

	/* allocate a frame buffer */
	if (MlmeAllocateMemory(pAd, &pBufFrame) != NDIS_STATUS_SUCCESS)
		return;
	/* End of if */

	/* make the frame header */
	MgtMacHeaderInit(
					pAd, &HdrActionFrame, SUBTYPE_ACTION, 0,
					MAC_BC,
					pAd->ApCfg.MBSSID[BSS0].Bssid);

	ACMR_MEM_MAC_COPY(HdrActionFrame.Addr3, MAC_BC); /* BSSID = broadcast */

	MakeOutgoingFrame(
					pBufFrame, &FrameLen,
					sizeof(ACMR_WLAN_HEADER), &HdrActionFrame,
					END_OF_ARGS);

	/* make the frame body */
	pFrameBwAnn = (ACM_BW_ANN_FRAME *)&pBufFrame[FrameLen];
	memset(pFrameBwAnn, 0, sizeof(ACM_BW_ANN_FRAME));

	pFrameBwAnn->Category = ACM_CATEGORY_WME;
	pFrameBwAnn->Action = ACM_ACTION_WME_BW_ANN;

	pFrameBwAnn->MBSS.Identifier = ACMR_CB->MbssIdentifier++;
	pFrameBwAnn->MBSS.Channel = ACMR_CHAN_GET(pAd);
	ACMR_MEM_MAC_COPY(pFrameBwAnn->MBSS.BSSID, ACMR_AP_ADDR_GET(pAd));

	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
		pFrameBwAnn->MBSS.UsedTime[IdAcNum] = pEdcaParam->AcmAcTime[IdAcNum];
	/* End of for */

	FrameLen += sizeof(ACM_BW_ANN_FRAME);

	/* send out the frame and free it */
	ACMR_MGMT_PKT_TX(pAd, pBufFrame, FrameLen);
	MlmeFreeMemory(pAd, pBufFrame);

	/* backup total acm used time */
	ACMR_CB->AcmTotalTimeOld = pEdcaParam->AcmTotalTime;
#endif // CONFIG_AP_SUPPORT //
#endif // ACM_CC_FUNC_MBSS //
} /* End of ACM_FrameBwAnnSend */


#ifdef ACM_CC_FUNC_MBSS
/*
========================================================================
Routine Description:
	Forward the bandwidth announce action frame.

Arguments:
	pAd					- WLAN control block pointer
	*pMblk				- the received frame
	PktLen				- the frame length

Return Value:
	None

Note:
	1. Only when ASSOC OK.
	2. If the source is from our AP, forward it to broadcast;
	3. If the source is not from our AP, forward it to our AP.
	4. TODO: Can not encrypt the frame.
========================================================================
*/
STATIC VOID ACM_MBSS_BwAnnForward(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pMblk,
	ACM_PARAM_IN	UINT32					PktLen)
{
	ACMR_WLAN_HEADER *pHeader;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (ACMR_IS_ASSOCIATED(pAd))
	{
		pHeader = (ACMR_WLAN_HEADER *)pMblk;

		if (ACMR_MAC_CMP(pHeader->Addr2, ACMR_AP_ADDR_GET(pAd)) != 0)
		{
			/* from other BSS so translating to unicast frame to our AP */
			ACMR_WLAN_PKT_RA_SET(pMblk, ACMR_AP_ADDR_GET(pAd));
			ACMR_WLAN_PKT_BSSID_SET(pMblk, ACMR_AP_ADDR_GET(pAd));
		} /* End of if */

		ACMR_WLAN_PKT_TA_SET(pMblk, ACMR_SELF_MAC_GET(pAd));
		ACMR_MGMT_PKT_TX(pAd, pMblk, PktLen);
	} /* End of if */
} /* End of ACM_MBSS_BwAnnForward */


/*
========================================================================
Routine Description:
	Handle the bandwidth announce action frame from other BSS.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- forward the frame
	ACM_RTN_FAIL	- do NOT forward the frame

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_MBSS_BwAnnHandle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pActFrame,
	ACM_PARAM_IN	UINT32					PktLen)
{
	ACM_BW_ANN_FRAME *pFrameAnn;
	ACM_MBSS_BW *pMbssNew, *pMbss;
	UINT32 IdMbssNum;
	UCHAR FlgIsNewAnn;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	pFrameAnn = (ACM_BW_ANN_FRAME *)pActFrame;
	pMbssNew = &pFrameAnn->MBSS;
	pMbss = &ACMR_CB->MBSS[0];
	FlgIsNewAnn = 0;

	/* learn it to our database */
	for(IdMbssNum=0; IdMbssNum<ACM_MBSS_BK_NUM; IdMbssNum++)
	{
		/* TODO: Need check SSID ? */
		if ((pMbssNew->Channel == pMbss->Channel) &&
			(memcmp(pMbssNew->BSSID, pMbss->BSSID, 6) == 0))
		{
			/* this is from same channel and BSSID */
			if (pMbssNew->Identifier != pMbss->Identifier)
			{
				/* new announce */
				if ((pMbssNew->UsedTime[0] != pMbss->UsedTime[0]) ||
					(pMbssNew->UsedTime[1] != pMbss->UsedTime[1]) ||
					(pMbssNew->UsedTime[2] != pMbss->UsedTime[2]) ||
					(pMbssNew->UsedTime[3] != pMbss->UsedTime[3]))
				{
					/* we regard as a new one only when different used time */
					FlgIsNewAnn = 1;
				} /* End of if */
			} /* End of if */

			break; /* find it */
		} /* End of if */

		pMbss ++; /* check next one */
	} /* End of for */

	if (IdMbssNum == ACM_MBSS_BK_NUM)
	{
		/* not found, new announce, try to find an empty entry */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> [mbss] New Ann from %02x:%02x:%02x:%02x:%02x:%02x\n",
					pMbssNew->BSSID[0], pMbssNew->BSSID[1],
					pMbssNew->BSSID[2], pMbssNew->BSSID[3],
					pMbssNew->BSSID[4], pMbssNew->BSSID[5]));

		pMbss = &ACMR_CB->MBSS[0];

		for(IdMbssNum=0; IdMbssNum<ACM_MBSS_BK_NUM; IdMbssNum++)
		{
			if (pMbss->Channel == 0)
				break;
			/* End of if */
		} /* End of for */

		if (IdMbssNum == ACM_MBSS_BK_NUM)
			return ACM_RTN_OK; /* My god! Too many BSS in the same channel */
		/* End of if */

		FlgIsNewAnn = 1;
	} /* End of if */

	/* update new used time */
	ACMR_MEM_COPY(pMbss, pMbssNew, sizeof(ACM_MBSS_BW));
	pMbss->Timeout = 0;

	/* calculate total used time */
	ACM_MBSS_BwReCalculate(pAd);

	if (FlgIsNewAnn)
	{
		/* need to forward the announce frame */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> [mbss] Ann Time %d %d %d %d!\n",
					pMbss->UsedTime[0], pMbss->UsedTime[1],
					pMbss->UsedTime[2], pMbss->UsedTime[3]));
		return ACM_RTN_OK;
	} /* End of if */

	/* this is duplicated announce frame */
	return ACM_RTN_FAIL;
} /* End of ACM_MBSS_BwAnnHandle */


/*
========================================================================
Routine Description:
	Re-calculate the used time for other BSS.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_MBSS_BwReCalculate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd)
{
	UINT32 IdMbssNum, IdAcNum;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	ACMR_CB->MbssTotalUsedTime = 0;

	for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
		ACMR_CB->MbssAcUsedTime[IdAcNum] = 0;
	/* End of for */

	for(IdMbssNum=0; IdMbssNum<ACM_MBSS_BK_NUM; IdMbssNum++)
	{
		if (memcmp(ACMR_CB->MBSS[IdMbssNum].BSSID, ACMR_SELF_MAC_GET(pAd), 6) != 0)
		{
			/* only for different BSS */
			if (ACMR_CB->MBSS[IdMbssNum].Channel != 0)
			{
				/* valid entry */
				for(IdAcNum=0; IdAcNum<ACM_DEV_NUM_OF_AC; IdAcNum++)
				{
					ACMR_CB->MbssTotalUsedTime += \
											ACMR_CB->MBSS[IdMbssNum].UsedTime[IdAcNum];

					ACMR_CB->MbssAcUsedTime[IdAcNum] += \
											ACMR_CB->MBSS[IdMbssNum].UsedTime[IdAcNum];
				} /* End of for */
			} /* End of if */
		} /* End of if */
	} /* End of for */
} /* End of ACM_MBSS_BwReCalculate */


/*
========================================================================
Routine Description:
	Broadcast our used bandwidth periodically.

Arguments:
	Data

Return Value:
	None

Note:
	We need to inform other BSS our used ACM time because we share same
	channel bandwidth.
========================================================================
*/
STATIC VOID ACM_TC_TASK_BwAnn(
	ACM_PARAM_IN	ULONG		Data)
{
	ACMR_PWLAN_STRUC pAd = (ACMR_PWLAN_STRUC)Data;
	UINT32 IdMbssNum;
	ULONG SplFlags;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* broadcast our bandwidth */
	ACM_FrameBwAnnSend(pAd, TRUE);

	/* get management semaphore */
	ACM_TSPEC_SEM_LOCK_CHK(pAd, SplFlags, LabelSemErr);

	/* check admission time timeout from other BSS */
	for(IdMbssNum=0; IdMbssNum<ACM_MBSS_BK_NUM; IdMbssNum++)
	{
		if (ACMR_CB->MBSS[IdMbssNum].Channel == 0)
			continue; /* check next one */
		/* End of if */

		if (++ACMR_CB->MBSS[IdMbssNum].Timeout >= ACM_MBSS_ENTRY_TIMEOUT)
		{
			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> [mbss] TimeOut %02x:%02x:%02x:%02x:%02x:%02x\n",
						ACMR_CB->MBSS[IdMbssNum].BSSID[0],
						ACMR_CB->MBSS[IdMbssNum].BSSID[1],
						ACMR_CB->MBSS[IdMbssNum].BSSID[2],
						ACMR_CB->MBSS[IdMbssNum].BSSID[3],
						ACMR_CB->MBSS[IdMbssNum].BSSID[4],
						ACMR_CB->MBSS[IdMbssNum].BSSID[5]));

			ACMR_MEM_ZERO(&ACMR_CB->MBSS[IdMbssNum], sizeof(ACM_MBSS_BW));
		} /* End of if */
	} /* End of for */

	/* re-calculate used time for other BSS */
	ACM_MBSS_BwReCalculate(pAd);

	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

LabelSemErr:
	return;
} /* End of ACM_TC_TASK_BwAnn */
#endif // ACM_CC_FUNC_MBSS //




/* ============================= TX time function ========================== */
/*
========================================================================
Routine Description:
	Calculate the QoS packet transmission time.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the connected client data base
	BodyLen				- the data length, not include WLAN header
	RateIndex			- transmission rate
	FlgIsGmode			- GMODE flag
	FlgIsCtsEnable		- CTS-self flag
	FlgIsRtsEnable		- RTS/CTS flag
	FlgIsSpreambleUsed	- Short preamble flag
	FlgIsNoAckUsed		- NO ACK flag
	TxopLimit			- TXOP limitation (microseconds)

	*pTimeNoData		- the tx time, not include data body
	*pTimeHeader		- the tx time, only include WLAN header
	*pTimeCtsSelf		- the tx time, CTS-self
	*pTimeRtsCts		- the tx time, RTS/CTS
	*pTimeAck			- the tx time, ACK

Return Value:
	transmission time (miscro second, us)

Note:
	1. Only for QoS packet.
	2. If you want to get pTimeHeader only, BodyLen must not be 0.
========================================================================
*/
UINT32 ACM_TX_TimeCal(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UCHAR				RateIndex,
	ACM_PARAM_IN	UCHAR				FlgIsGmode,
	ACM_PARAM_IN	UCHAR				FlgIsCtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsRtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsSpreambleUsed,
	ACM_PARAM_IN	UCHAR				FlgIsNoAckUsed,
	ACM_PARAM_IN	UINT32				TxopLimit,
	ACM_PARAM_OUT	UINT32				*pTimeNoData,
	ACM_PARAM_OUT	UINT32				*pTimeHeader,
	ACM_PARAM_OUT	UINT32				*pTimeCtsSelf,
	ACM_PARAM_OUT	UINT32				*pTimeRtsCts,
	ACM_PARAM_OUT	UINT32				*pTimeAck)
{
#define LMR_PREAMBL_TIME(__FlgIsGmode, __FlgIsSpreamble, __Time)	\
	{																\
		if (__FlgIsGmode == 0)										\
		{															\
			if (__FlgIsSpreamble == 0)								\
				__Time += TIME_LONG_PREAMBLE;						\
			else													\
				__Time += TIME_SHORT_PREAMBLE;						\
		}															\
		else														\
			__Time += 20;											\
	}

	UINT32 LenHeader;
	UINT32 RateId;
	UINT32 TxTime;
#ifndef ACM_CC_FUNC_SOFT_ACM
	UINT32 TxTimeFragment;
#endif // ACM_CC_FUNC_SOFT_ACM //
	UINT32 TimeHeader, TimeData, TimeFrag, TimeRtsCts;
	UINT32 TxTimeCtsSelf, TxTimeRtsCts, TxTimeAck;
	UINT32 DataExtraLen, LenFrag, LenLastFrag, NumFrag;
	UCHAR  FlgIsNeedHardwareFrag;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	LenHeader = ACMR_FME_QOS_HEADER_SIZE + 4; /* 4: FCS size */
	TxTime = 0;
	TimeRtsCts = 0;
	TxTimeCtsSelf = 0;
	TxTimeRtsCts = 0;
	TxTimeAck = 0;
	TimeHeader = 0;

	/* CTS-self */
	if (FlgIsCtsEnable == 1)
	{
		/* tx time += preamble + CTS-self + SIFS */
		TxTimeCtsSelf = 0;

		switch(RateIndex)
		{
			case ACM_RATE_54M:
				RateId = ACM_RATE_ID_54M;
				break;

			case ACM_RATE_48M:
				RateId = ACM_RATE_ID_48M;
				break;

			case ACM_RATE_36M:
				RateId = ACM_RATE_ID_36M;
				break;

			case ACM_RATE_24M:
				RateId = ACM_RATE_ID_24M;
				break;

			case ACM_RATE_18M:
				RateId = ACM_RATE_ID_18M;
				break;

			case ACM_RATE_12M:
				RateId = ACM_RATE_ID_12M;
				break;

			case ACM_RATE_9M:
				RateId = ACM_RATE_ID_9M;
				break;

			case ACM_RATE_6M:
				RateId = ACM_RATE_ID_6M;
				break;

			case ACM_RATE_11M:
				RateId = ACM_RATE_ID_11M;
				break;

			case ACM_RATE_5_5M:
				RateId = ACM_RATE_ID_5_5M;
				break;

			case ACM_RATE_2M:
				RateId = ACM_RATE_ID_2M;
				break;

			case ACM_RATE_1M:
				RateId = ACM_RATE_ID_1M;
				break;

			default:
				RateId = ACM_RATE_ID_1M;
				ACMR_DEBUG(ACMR_DEBUG_ERR,
							("acm_err> RateIndex Error! TX_TimeCal()\n"));
				break;
		} /* End of switch */

		LMR_PREAMBL_TIME(FlgIsGmode, FlgIsSpreambleUsed, TxTimeCtsSelf);
		TxTimeCtsSelf += \
					ACM_TX_TimePlcpCal(FRM_LENGTH_ACK, RateId, FlgIsGmode);
		TxTimeCtsSelf += TIME_SIFSG;

		if (pTimeCtsSelf != NULL)
			*pTimeCtsSelf = TxTimeCtsSelf;
		/* End of if */
	} /* End of if */


	/* RTS and CTS */
	if ((FlgIsCtsEnable == 0) && (FlgIsRtsEnable == 1))
	{
		/* tx time += preamble + RTS + SIFS + preamble + CTS + SIFS */
		LMR_PREAMBL_TIME(FlgIsGmode, FlgIsSpreambleUsed, TimeRtsCts);
		LMR_PREAMBL_TIME(FlgIsGmode, FlgIsSpreambleUsed, TimeRtsCts);

		switch(RateIndex)
		{
			case ACM_RATE_54M:
			case ACM_RATE_48M:
			case ACM_RATE_36M:
			case ACM_RATE_24M:
				TimeRtsCts += TIME_SIFSGx2;
				RateId = ACM_RATE_ID_24M;
				break;

			case ACM_RATE_18M:
			case ACM_RATE_12M:
				TimeRtsCts += TIME_SIFSGx2;
				RateId = ACM_RATE_ID_12M;
				break;

			case ACM_RATE_9M:
			case ACM_RATE_6M:
				TimeRtsCts += TIME_SIFSGx2;
				RateId = ACM_RATE_ID_6M;
				break;

			case ACM_RATE_11M:
				TimeRtsCts += TIME_SIFSx2;
				RateId = ACM_RATE_ID_11M;
				break;

			case ACM_RATE_5_5M:
				TimeRtsCts += TIME_SIFSx2;
				RateId = ACM_RATE_ID_5_5M;
				break;

			case ACM_RATE_2M:
				TimeRtsCts += TIME_SIFSx2;
				RateId = ACM_RATE_ID_2M;
				break;

			case ACM_RATE_1M:
				TimeRtsCts += TIME_SIFSx2;
				RateId = ACM_RATE_ID_1M;
				break;

			default:
				TimeRtsCts += TIME_SIFSx2;
				RateId = ACM_RATE_ID_1M;
				ACMR_DEBUG(ACMR_DEBUG_ERR,
							("acm_err> RateIndex Error! TX_TimeCal()\n"));
				break;
		} /* End of switch */

		TimeRtsCts += ACM_TX_TimePlcpCal(FRM_LENGTH_RTS, RateId, FlgIsGmode);
		TimeRtsCts += ACM_TX_TimePlcpCal(FRM_LENGTH_ACK, RateId, FlgIsGmode);

		if (pTimeRtsCts != NULL)
			*pTimeRtsCts = TimeRtsCts;
		/* End of if */
	} /* End of if */


	/* SIFS + preamble + ACK */
	if (FlgIsNoAckUsed == 0)
	{
		TxTimeAck = 0;
		LMR_PREAMBL_TIME(FlgIsGmode, FlgIsSpreambleUsed, TxTimeAck);

		switch(RateIndex)
		{
			case ACM_RATE_54M:
			case ACM_RATE_48M:
			case ACM_RATE_36M:
			case ACM_RATE_24M:
				TxTimeAck += TIME_SIFSG;
				RateId = ACM_RATE_ID_24M;
				break;

			case ACM_RATE_18M:
			case ACM_RATE_12M:
				TxTimeAck += TIME_SIFSG;
				RateId = ACM_RATE_ID_12M;
				break;

			case ACM_RATE_9M:
			case ACM_RATE_6M:
				TxTimeAck += TIME_SIFSG;
				RateId = ACM_RATE_ID_6M;
				break;

			case ACM_RATE_11M:
				TxTimeAck += TIME_SIFS;
				RateId = ACM_RATE_ID_11M;
				break;

			case ACM_RATE_5_5M:
				TxTimeAck += TIME_SIFS;
				RateId = ACM_RATE_ID_5_5M;
				break;

			case ACM_RATE_2M:
				TxTimeAck += TIME_SIFS;
				RateId = ACM_RATE_ID_2M;
				break;

			case ACM_RATE_1M:
				TxTimeAck += TIME_SIFS;
				RateId = ACM_RATE_ID_1M;
				break;

			default:
				TxTimeAck += TIME_SIFS;
				RateId = ACM_RATE_ID_1M;
				ACMR_DEBUG(ACMR_DEBUG_ERR,
							 ("acm_err> RateIndex Error! TX_TimeCal()\n"));
				break;
		} /* End of switch */

		/*
			EX: data size = 208B, rate = 54Mbps,
			TxTimeAck = 44us in A band.
		*/
		TxTimeAck += ACM_TX_TimePlcpCal(FRM_LENGTH_ACK, RateId, FlgIsGmode);
	}
	else
	{
		/* use NO ACK policy so ack time = 0 */
		TxTimeAck = 0;
	} /* End of if */

	if (pTimeAck != NULL)
		*pTimeAck = TxTimeAck;
	/* End of if */

	if (BodyLen > 0)
	{
		/* preamble + Data */
		switch(RateIndex)
		{
			case ACM_RATE_54M:
				RateId = ACM_RATE_ID_54M;
				break;

			case ACM_RATE_48M:
				RateId = ACM_RATE_ID_48M;
				break;

			case ACM_RATE_36M:
				RateId = ACM_RATE_ID_36M;
				break;

			case ACM_RATE_24M:
				RateId = ACM_RATE_ID_24M;
				break;

			case ACM_RATE_18M:
				RateId = ACM_RATE_ID_18M;
				break;

			case ACM_RATE_12M:
				RateId = ACM_RATE_ID_12M;
				break;

			case ACM_RATE_9M:
				RateId = ACM_RATE_ID_9M;
				break;

			case ACM_RATE_6M:
				RateId = ACM_RATE_ID_6M;
				break;

			case ACM_RATE_11M:
				RateId = ACM_RATE_ID_11M;
				break;

			case ACM_RATE_5_5M:
				RateId = ACM_RATE_ID_5_5M;
				break;

			case ACM_RATE_2M:
				RateId = ACM_RATE_ID_2M;
				break;

			case ACM_RATE_1M:
				RateId = ACM_RATE_ID_1M;
				break;

			default:
				RateId = ACM_RATE_ID_1M;
				ACMR_DEBUG(ACMR_DEBUG_ERR,
							("acm_err> RateIndex Error! TX_TimeCal()\n"));
				break;
		} /* End of switch */

		/* add data preamble tx time */
		/*
			EX: data size = 208B, rate = 54Mbps,
			LenHeader = 30B, TimeHeader = 28us in A band.
		*/
		LMR_PREAMBL_TIME(FlgIsGmode, FlgIsSpreambleUsed, TimeHeader);
		TimeHeader += ACM_TX_TimePlcpCal(LenHeader, RateId, FlgIsGmode);

		/* maybe software fragment */
		if (TxopLimit > 0)
		{
			/* check whether tx time > txop limit */
			LenFrag = ACMR_FRG_THRESH(pAd);

			if ((LenFrag == 0) || (BodyLen <= LenFrag))
			{
				/* no fragment is needed */
				LenFrag = BodyLen;
				FlgIsNeedHardwareFrag = 0;
			}
			else
			{
				/* fragment is needed */
				FlgIsNeedHardwareFrag = 1;
			} /* End of if */

			/* LenFrag = total len (no fragment) or fragment len */

			/* calculate data body transmission time */
			DataExtraLen = ACM_EncryptExtraLenGet(pAd, pCdb);
			TimeData = ACM_TX_TimePlcpCal(LenFrag+DataExtraLen, RateId, FlgIsGmode);

			/* calculate all transmission time */
			if (FlgIsRtsEnable == 1)
				TxTimeRtsCts = TimeRtsCts; /* need RTS/CTS */
			else
				TxTimeRtsCts = 0;
			/* End of if */

			/* pre-sum CTS-self "or" (not "and") RTS/CTS tx time */
			TxTime = TxTimeCtsSelf + TxTimeRtsCts;

			if (FlgIsNeedHardwareFrag == 1)
			{
				/*
					Calculate all fragment tx time because our ASIC can not
					distribute fragment to different TXOP.
				*/
				NumFrag = BodyLen/ LenFrag;
				TxTime += (TimeHeader + TimeData + TxTimeAck) * NumFrag;

				LenLastFrag = BodyLen % LenFrag;
				if (LenLastFrag > 0)
				{
					/* sum the last fragment tx time */
					TimeData = ACM_TX_TimePlcpCal(LenLastFrag+DataExtraLen,
													RateId, FlgIsGmode);
					TxTime += TimeHeader + TimeData + TxTimeAck;
				} /* End of if */
			}
			else
			{
				/* only a packet, no fragment */
				TxTime += (TimeHeader + TimeData + TxTimeAck);
			} /* End of if */


			/* check whether tx time > TXOP limitation */
#ifndef ACM_CC_FUNC_SOFT_ACM
			if (TxTime > TxopLimit)
			{
				/* we need to do software fragment, calculate residant TXOP */
				/* MUST no hardware fragment */
				TxTime = (TxTimeCtsSelf + TxTimeRtsCts +
						TimeHeader + TxTimeAck);

				if (TxopLimit > TxTime)
				{
					if (FlgIsNeedHardwareFrag == 0)
					{
						TxopLimit -= (TxTimeCtsSelf + TxTimeRtsCts +
									TimeHeader + TxTimeAck);
					}
					else
					{
						/* calculate a fragment tx time */
						TxTimeFragment = TimeHeader+TimeData+TxTimeAck;

						/* dont care about CTS-Self or RTS/CTS tx time */
						TxopLimit -= (TxTimeCtsSelf + TxTimeRtsCts);

						if (TxTimeFragment > TxopLimit)
						{
							/*
								A fragment tx time > TXOP limit
								so do need to do fragment, we must make a
								packet len <= fragment threshold.
							*/
							TxopLimit -= (TimeHeader + TxTimeAck);
						}
						else
						{
							/*
								We can not use the LenFrag as software fragment
								length, when >= 2 fragments can be transmitted
								in a TXOP, maybe another MSDU and our fragment
								is transmitted in a TXOP or our both fragments
								are transmitted in a TXOP, we can not predict
								the case.
							*/
							TxopLimit -= (TimeHeader + TxTimeAck);
						} /* End of if */
					} /* End of if */

					/* calculate packet len allowed to be transmitted in a TXOP */
					if (FlgIsGmode == 1)
					{
						LenFrag = TxopLimit>>2;
						LenFrag = (LenFrag<<1)*RateIndex;
						if (LenFrag > 22)
							LenFrag = (LenFrag-22)>>3;
						/* End of if */
					}
					else
						LenFrag = ((TxopLimit*RateIndex)>>4);
					/* End of if */

					if (LenFrag >= DataExtraLen)
					{
						LenFrag -= DataExtraLen;

						if (LenFrag > 1)
							LenFrag --; /* for safe */
						/* End of if */

						/* for software fragment, we dont need 128-double */
					} /* End of if */
				}
				else
				{
					/*
						Fatal error, txop limit is too small.
						So dont care txop limit and transmit the packet without
						fragment.
					*/
					LenFrag = BodyLen;
				} /* End of if */
			} /* End of if */
#endif // ACM_CC_FUNC_SOFT_ACM //

			/*
				Calculate MSDU exchange time, LenFrag is the packet length
				allowed to transmitted in a TXOP.
			*/
			if (LenFrag == BodyLen)
			{
				/* no fragment is needed */
				TimeData = ACM_TX_TimePlcpCal(BodyLen+DataExtraLen,
												RateId, FlgIsGmode);

				/* sum all tx time */
				TxTime = TxTimeCtsSelf + TxTimeRtsCts;
				TxTime += TimeHeader + TxTimeAck;
			}
			else
			{
				/* fragment is needed */
				/* LenFrag = a software fragment length in a TXOP */
				TimeFrag = ACM_TX_TimePlcpCal(LenFrag+DataExtraLen,
												RateId, FlgIsGmode);

				NumFrag = BodyLen/ LenFrag;
				LenLastFrag = BodyLen % LenFrag;

				if (LenFrag <= ACMR_RTS_THRESH(pAd))
					TxTimeRtsCts = 0; /* no RTS/CTS is needed for new len */
				/* End of if */

				/* add other fragment tx time except the tail fragment */
				if (NumFrag >= 1)
				{
					TimeData = (TxTimeCtsSelf + TxTimeRtsCts +
								TimeHeader + TimeFrag +
								TxTimeAck) * NumFrag;
				} /* End of if */

				/* add tail fragment tx time */
				if (LenLastFrag > 0)
				{
					TimeData += (TxTimeCtsSelf + TxTimeRtsCts +
								TimeHeader + TxTimeAck);
					TimeData += ACM_TX_TimePlcpCal(LenLastFrag+DataExtraLen,
													RateId, FlgIsGmode);
				} /* End of if */

				/* tx time is included in TimeData */
				TxTime = 0;
			} /* End of if */
		}
		else
		{
			/* add data tx time */
			/*
				EX: data size = 208B, rate = 54Mbps,
				TimeData without header = 32us in A band.
			*/
			if (BodyLen > 0)
			{
				DataExtraLen = ACM_EncryptExtraLenGet(pAd, pCdb);
				TimeData = ACM_TX_TimePlcpCal(BodyLen+DataExtraLen,
												RateId, FlgIsGmode);
			}
			else
				TimeData = 0;
			/* End of if */

			/* sum all tx time */
			TxTime = TxTimeCtsSelf + TxTimeRtsCts;
			TxTime += TimeHeader + TxTimeAck;
		} /* End of if */
	}
	else
		TimeData = 0;
	/* End of if */

	if (pTimeNoData != NULL)
		*pTimeNoData = TxTime;
	/* End of if */

	if (pTimeHeader != NULL)
		*pTimeHeader = TimeHeader;
	/* End of if */

	TxTime += TimeData;

	/*
		EX1: data size = 208B, rate = 54Mbps,
		TxTime = 104us in A band.

		EX2: data size = 208B, rate = 6Mbps,
		In A band, TxTimeAck = 60us, TimeHeader = 64us,
		TimeData = 284us, so TxTime = 64+284+60 = 408us.
	*/
	return TxTime;
} /* End of ACM_TX_TimeCal */


#ifdef ACM_CC_FUNC_11N
/*
========================================================================
Routine Description:
	Calculate the QoS packet transmission time for HT rate.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the connected client data base
	BodyLen				- the data length, not include WLAN header
	McsId				- transmission MCS
	BwId				- 20 or 20/40MHz
	GIId				- regular or short GI
	FlgIsRtsEnable		- RTS/CTS flag
	FlgIsNoAckUsed		- NO ACK flag
	FlgIsAmpud			- HT preamble time flag
	TxopLimit			- TXOP limitation (microseconds)

	*pTimeNoData		- the tx time, not include data body
	*pTimeHeader		- the tx time, only include WLAN header
	*pTimeAck			- the tx time, BLOCK ACK
	*pTimeDataHdrOnly	- the tx time, data + BLOCK ACK
	*pTimeDataOnly		- the tx time, data

Return Value:
	transmission time (miscro second, us)

Note:
	1. Only for QoS packet.
	2. No HT rate for RTS/CTS.
========================================================================
*/
UINT32 ACM_TX_TimeCalHT(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				McsId,
	ACM_PARAM_IN	UINT32				BwId,
	ACM_PARAM_IN	UINT32				GIId,
	ACM_PARAM_IN	BOOLEAN				FlgIsRtsEnable,
	ACM_PARAM_IN	BOOLEAN				FlgIsNoAckUsed,
	ACM_PARAM_IN	BOOLEAN				FlgIsAmpdu,
	ACM_PARAM_IN	UINT32				TxopLimit,
	ACM_PARAM_OUT	UINT32				*pTimeNoData,
	ACM_PARAM_OUT	UINT32				*pTimeHeader,
	ACM_PARAM_OUT	UINT32				*pTimeAck,
	ACM_PARAM_OUT	UINT32				*pTimeDataHdrOnly,
	ACM_PARAM_OUT	UINT32				*pTimeDataOnly)
{
	UINT32 LenHeader;
	UINT32 TxTime;
	UINT32 TimeHeader, TimeData;
	UINT32 TxTimeAck;
	UINT32 DataExtraLen;
	UINT32 Nss;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	LenHeader = ACMR_FME_QOS_N_HEADER_SIZE;
	TxTime = 0;
	TxTimeAck = 0;
	TimeHeader = 0;
	DataExtraLen = 0;

	if (McsId < 8)
		Nss = 1;
	else
	{
		if (McsId < 16)
			Nss = 2;
		else
		{
			if (McsId < 24)
				Nss = 3;
			else
			{
				if (McsId < 32)
					Nss = 4;
				else
					Nss = 1;
				/* End of if */
			} /* End of if */
		} /* End of if */
	} /* End of if */

	/* TODO: If in green-field mode, use RIFS */

	/* SIFS + ACK */
	if (FlgIsNoAckUsed == 0)
	{
		TxTimeAck = 0;

		LMR_PREAMBL_TIME(1, 0, TxTimeAck);

		TxTimeAck += TIME_SIFSG;

		TxTimeAck += ACM_TX_TimePlcpCal(
										FRM_LENGTH_BLOCK_ACK,
										ACM_RATE_ID_24M,
										1);
	}
	else
	{
		/* use NO ACK policy so ack time = 0 */
		TxTimeAck = 0;
	} /* End of if */

	if (pTimeAck != NULL)
		*pTimeAck = TxTimeAck;
	/* End of if */

	/* If FlgIsAmpdu == 1, means we do not care HT preamble time */

	/* preamble + header */
	TimeHeader = ACM_TX_TimePlcpCalHT(
										LenHeader,
										McsId,
										Nss,
										0,
										0,
										BwId,
										GIId,
										FlgIsAmpdu);

	/* add data tx time without preamble */
	if (BodyLen > 0)
	{
		if (pCdb != NULL)
			DataExtraLen = ACM_EncryptExtraLenGet(pAd, pCdb);
		/* End of if */

		TimeData = ACM_TX_TimePlcpCalHT(
										LenHeader+BodyLen+DataExtraLen,
										McsId,
										Nss,
										0,
										0,
										BwId,
										GIId,
										FlgIsAmpdu);

		TimeData -= TimeHeader; /* only data, no HT preamble */
	}
	else
		TimeData = 0;
	/* End of if */

	/* sum all tx time */
	TxTime = TimeHeader + TxTimeAck;

	if (pTimeNoData != NULL)
		*pTimeNoData = TxTime;
	/* End of if */

	if (pTimeHeader != NULL)
		*pTimeHeader = TimeHeader;
	/* End of if */

	if (pTimeDataHdrOnly != NULL)
	{
		if ((pCdb != NULL) && (TimeData > 0))
			DataExtraLen = ACM_EncryptExtraLenGet(pAd, pCdb);
		/* End of if */

		if ((BodyLen > 0) && (TimeData == 0))
		{
			/* calculate time */
			*pTimeDataHdrOnly = ACM_TX_TimePlcpCalHT(
											LenHeader+BodyLen+DataExtraLen,
											McsId,
											Nss,
											0,
											0,
											BwId,
											GIId,
											FlgIsAmpdu);
		}
		else
			*pTimeDataHdrOnly = TimeHeader + TimeData;
		/* End of if */
	} /* End of if */

	if (pTimeDataOnly != NULL)
	{
		if ((pCdb != NULL) && (TimeData > 0))
			DataExtraLen = ACM_EncryptExtraLenGet(pAd, pCdb);
		/* End of if */

		if ((BodyLen > 0) && (TimeData == 0))
		{
			/* calculate time */
			*pTimeDataOnly = ACM_TX_TimePlcpCalHT(
											BodyLen+DataExtraLen,
											McsId,
											Nss,
											0,
											0,
											BwId,
											GIId,
											FlgIsAmpdu);
		}
		else
			*pTimeDataOnly = TimeData;
		/* End of if */
	} /* End of if */

	TxTime += TimeData;

	if (FlgIsRtsEnable != 0)
	{
		/* add RTS/CTS 24Mbps time */
#ifdef ACM_CC_FUNC_AUX_TX_TIME
		UINT32 TimeRtsCts;

		ACM_TX_TimeCal(pAd, NULL,
						0,						/* BodyLen = 0 */
						ACM_RATE_24M,			/* RateIndex */
						1,						/* g mode */
						0,						/* cts-self */
						1,						/* rts/cts */
						0,						/* IdPreambleNum */
						1,						/* no ack */
						0,						/* txop limit */
						NULL,					/* no data tx time */
						NULL,					/* wlan header tx time */
						NULL,					/* cts self tx time */
						&TimeRtsCts,			/* rts/cts tx time */
						NULL);					/* ack tx time */
		TxTime += TimeRtsCts;
#else

		TxTime += gAcmTxTimeOthers[ACM_PRE_TIME_ID_24M][0][2];
#endif // ACM_CC_FUNC_AUX_TX_TIME //
	} /* End of if */

	return TxTime;
} /* End of ACM_TX_TimeCalHT */
#endif // ACM_CC_FUNC_11N //


/*
========================================================================
Routine Description:
	Calculate the QoS packet transmission time on the fly.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the connected client data base
	BodyLen				- the data length, not include WLAN header
	RateIndex			- transmission rate, such as ACM_PRE_TIME_ID_1M, etc.
	FlgIsCtsEnable		- CTS-self flag
	FlgIsRtsEnable		- RTS/CTS flag
	FlgIsSpreambleUsed	- Short preamble flag
	FlgIsNoAckUsed		- NO ACK flag

Return Value:
	transmission time (miscro second, us)

Note:
	1. Only for QoS packet.
========================================================================
*/
UINT32 ACM_TX_TimeCalOnFly(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UCHAR				RateIndex,
	ACM_PARAM_IN	UCHAR				FlgIsCtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsRtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsSpreambleUsed,
	ACM_PARAM_IN	UCHAR				FlgIsNoAckUsed)
{
	UINT32 TxTime;
#ifndef ACM_CC_FUNC_AUX_TX_TIME
	UINT32 LenDataId;
	UINT32 RateRtsCtsId;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	LenDataId = BodyLen >> ACM_PRE_TIME_DATA_SIZE_OFFSET;

	if (RateIndex > ACM_PRE_TIME_ID_54M)
		RateIndex = ACM_PRE_TIME_ID_54M;
	/* End of if */

//	if (LenDataId > 0)
//		LenDataId --; /* use small length */
	/* End of if */

	if (LenDataId > ACM_PRE_TIME_DATA_SIZE_NUM)
		LenDataId = ACM_PRE_TIME_DATA_SIZE_NUM;
	/* End of if */

	/* sum the time: cts-self + rts/cts + preamble + header + data + ack */
	TxTime = gAcmTxTimeOthers[RateIndex][FlgIsSpreambleUsed][ACM_PRE_TIME_HEADER];
	TxTime += gAcmTxTimeBody[RateIndex][LenDataId];

	if (FlgIsCtsEnable)
	{
		RateRtsCtsId = ACMR_PHY_RATE_ID_RTS_CTS_GET(pAd);
		TxTime += gAcmTxTimeOthers[RateRtsCtsId][FlgIsSpreambleUsed][ACM_PRE_TIME_CTS_SELF];
	} /* End of if */

	if (FlgIsRtsEnable)
	{
		RateRtsCtsId = ACMR_PHY_RATE_ID_RTS_CTS_GET(pAd);
		TxTime += gAcmTxTimeOthers[RateRtsCtsId][FlgIsSpreambleUsed][ACM_PRE_TIME_RTS_CTS];
	} /* End of if */

	if (FlgIsNoAckUsed == 0)
		TxTime += gAcmTxTimeOthers[RateIndex][FlgIsSpreambleUsed][ACM_PRE_TIME_ACK];
	/* End of if */
#else

	UCHAR RateMapping[ACM_RATE_MAX_NUM] = {
			ACM_RATE_1M,		ACM_RATE_2M,	ACM_RATE_5_5M,
			ACM_RATE_11M,		ACM_RATE_6M,	ACM_RATE_9M,
			ACM_RATE_12M,		ACM_RATE_18M,	ACM_RATE_24M,
			ACM_RATE_36M,		ACM_RATE_48M,	ACM_RATE_54M
		};
	UCHAR FlgIsGmode;


	if (RateIndex <= 3)
		FlgIsGmode = 0;
	else
		FlgIsGmode = 1;
	/* End of if */

	if (FlgIsCtsEnable)
		FlgIsRtsEnable = 0;
	/* End of if */

	TxTime = ACM_TX_TimeCal(pAd, NULL,
							0,						/* BodyLen = 0 */
							RateMapping[RateIndex],	/* RateIndex */
							FlgIsGmode,				/* g mode */
							FlgIsCtsEnable,			/* cts-self */
							FlgIsRtsEnable,			/* rts/cts */
							FlgIsSpreambleUsed,		/* IdPreambleNum */
							FlgIsNoAckUsed,			/* no ack */
							0,						/* txop limit */
							NULL,					/* no data tx time */
							NULL,					/* wlan header tx time */
							NULL,					/* cts self tx time */
							NULL,					/* rts/cts tx time */
							NULL);					/* ack tx time */
#endif // ACM_CC_FUNC_AUX_TX_TIME //

	return TxTime;
} /* End of ACM_TX_TimeCalOnFly */


#ifdef ACM_CC_FUNC_11N
/*
========================================================================
Routine Description:
	Calculate the QoS packet transmission time on the fly for HT rate.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the connected client data base
	*pStream			- the matched TSPEC
	Timestamp			- current Timestamp, unit: micro second
	BodyLen				- the data length, not include WLAN header
	McsId				- transmission rate, 0 ~ 31
	FlgIsNoAckUsed		- NO ACK flag

Return Value:
	transmission time (miscro second, us)

Note:
	1. Only for QoS packet.
========================================================================
*/
UINT32 ACM_TX_TimeCalOnFlyHT(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	ACM_STREAM			*pStream,
	ACM_PARAM_IN	UINT64				Timestamp,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				McsId,
	ACM_PARAM_IN	UCHAR				FlgIsNoAckUsed)
{

	UINT32 TxTime;
	UINT32 TimeOffset;
	UINT8 NumOfBaWinSize;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	TimeOffset = (UINT32)(Timestamp - pStream->TxTimestampMarkEnqueue);

	if (pStream->pTspec->TsInfo.AckPolicy == ACM_ACK_POLICY_BLOCK)
		NumOfBaWinSize = pStream->HT_BaWinSize;
	else
		NumOfBaWinSize = 0; /* not BLOCK ACK for the TSPEC */
	/* End of if */

	if (!ACMR_HT_IS_BA_BUILT(pCdb, pStream->UP))
		NumOfBaWinSize = 0; /* not yet build the BA session */
	/* End of if */

	if ((pStream->TxAmpduNumEnqueueHT == 0) ||
		(pStream->TxAmpduNumEnqueueHT > (UINT32)NumOfBaWinSize) ||
		(TimeOffset > pStream->TxTimeEnqueueHT))
	{

		/* sum the time: rts/cts + preamble + header + data + block ack */
		/* 802.11 header is included defaultly in ACM_TX_TimeCalHT() */
#ifdef ACM_CC_FUNC_AUX_TX_TIME
		TxTime = ACM_TX_TimeCalHT(pAd, NULL,
							BodyLen,		/* body len */
							McsId,					/* MCS Index */
							ACMR_IS_2040_STA(pCdb),	/* 20 or 20/40 MHz */
							0,				/* regular or short GI */
							1,				/* rts/cts */
							0,				/* no AMPDU or fist pkt in AMPDU */
							FlgIsNoAckUsed,	/* no ack */
							0xFFFFFFFF,		/* txop limit */
							NULL,			/* no data tx time */
							NULL,			/* wlan header tx time */
							NULL,			/* ack tx time */
							NULL,			/* data+hdr only tx time */
							NULL); 			/* data only tx time */
#else
		UINT32 LenDataId;

		LenDataId = (BodyLen+ACMR_FME_QOS_N_HEADER_SIZE);
		LenDataId >>= ACM_PRE_TIME_DATA_SIZE_OFFSET;

//		if (LenDataId > 0)
//			LenDataId --; /* use small length */
		/* End of if */

		if (LenDataId > ACM_PRE_TIME_DATA_SIZE_NUM)
			LenDataId = ACM_PRE_TIME_DATA_SIZE_NUM;
		/* End of if */

		/* add RTS/CTS time (maybe no RTS/CTS in the air) */
		TxTime = gAcmTxTimeOthers[ACM_PRE_TIME_ID_24M][0][2];

		/* add data time */
		TxTime += gAcmTxTimeBodyHT\
							[ACMR_IS_2040_STA(pCdb)][0][McsId][LenDataId][0];

		/* add block ack time */
		if (FlgIsNoAckUsed == 0)
			TxTime += gAcmTxTimeOthersHT;
		/* End of if */
#endif // ACM_CC_FUNC_AUX_TX_TIME //

		/* statistics */
#ifdef ACM_CC_FUNC_STATS
		if (pStream->TxAmpduNumEnqueueHT > 1)
		{
			ACM_CTRL_PARAM *pEdcaParam;
			ACM_STATISTICS *pStats;

			pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
			pStats = &pEdcaParam->Stats;

			ACM_STATS_COUNT_INC(pStats->AMPDU[pStream->TxAmpduNumEnqueueHT]);
		} /* End of if */
#endif // ACM_CC_FUNC_STATS //

		/* backup time record */
		if (pStream->TxAmpduNumEnqueueHT == 0)
			pStream->TxAmpduNumEnqueueHT ++;
		else
		{
			pStream->TxTimestampEnqueueHT = Timestamp;
			pStream->TxTimeEnqueueHT = TxTime;
			pStream->TxAmpduNumEnqueueHT = 0;
		} /* End of if */
	}
	else
	{
		/* AMPDU handle */
		/* skip PHY preamble & ACK time */
		/* ACMR_FME_QOS_N_HEADER_SIZE + 4 for AMPDU subframe header, 3 for padding */

#ifdef ACM_CC_FUNC_AUX_TX_TIME
		UINT32 TimeData;

		/* 802.11 header is included defaultly in ACM_TX_TimeCalHT() */
		TimeData = ACM_TX_TimeCalHT(pAd, NULL,
							BodyLen+4+3,	/* body len */
							McsId,			/* MCS Index */
							ACMR_IS_2040_STA(pCdb),	/* 20 or 20/40 MHz */
							0,				/* regular or short GI */
							1,				/* rts/cts */
							1,				/* AMPDU */
							FlgIsNoAckUsed,	/* no ack */
							0,				/* txop limit */
							NULL,			/* no data tx time */
							NULL,			/* wlan header tx time */
							NULL,			/* ack tx time */
							&TxTime,		/* data+hdr only tx time */
							NULL);			/* data only tx time */
#else
		UINT32 LenDataId;

		/* 802.11 header for each MPDU of A-MPDU */
		LenDataId = (ACMR_FME_QOS_N_HEADER_SIZE + BodyLen + 4 + 3);
		LenDataId = LenDataId >> ACM_PRE_TIME_DATA_SIZE_OFFSET;

//		if (LenDataId > 0)
//			LenDataId --; /* use small length */
		/* End of if */

		if (LenDataId > ACM_PRE_TIME_DATA_SIZE_NUM)
			LenDataId = ACM_PRE_TIME_DATA_SIZE_NUM;
		/* End of if */

		/* we only care about tx time for header + data body */
		TxTime = gAcmTxTimeBodyHT\
							[ACMR_IS_2040_STA(pCdb)][0][McsId][LenDataId][1];

		/* TODO: we need to minus the PHY preamble time */

#endif // ACM_CC_FUNC_AUX_TX_TIME //

		/*
			Need to add Minimum MPDU Start Spacing, suppose tMMSS = 4us.

			An HT STA shall not start the transmission of more than one MPDU
			within the time limit described in the Minimum MPDU Start Spacing
			field.

			To satisfy this requirement, the number of octets between the start
			of two consecutive MPDUs in an A-MPDU, measured at the PHY SAP,
			shall be equal or greater than:

			tMMSS * r / 8, where r = PHY Data Rate (unit: Mbps)
		*/
		TxTime += 4;

		/* maybe we can make up a AMPDU from the WLAN hardware */
		pStream->TxAmpduNumEnqueueHT ++;
	} /* End of if */

	return TxTime;
} /* End of ACM_TX_TimeCalOnFlyHT */
#endif // ACM_CC_FUNC_11N //


/*
========================================================================
Routine Description:
	Pre-Calculate the QoS packet transmission time.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	All possible tx time are kept in a local array.
	[0] duration id [1] cts-self tx time [2] rts/cts tx time
	[3] header time [4] ack tx time

	Do NOT include "data body tx time".

	For HT packet, suppose that
	1. RTS/CTS (11M) for each packet
	2. BLOCK ACK (11M) for each packet
========================================================================
*/
VOID ACM_TX_TimeCalPre(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd)
{
#ifndef ACM_CC_FUNC_AUX_TX_TIME
	UINT32 RateMap[ACM_RATE_MAX_NUM][3] = {
					{ ACM_RATE_1M,   0, ACM_RATE_ID_1M },
					{ ACM_RATE_2M,   0, ACM_RATE_ID_2M },
					{ ACM_RATE_5_5M, 0, ACM_RATE_ID_5_5M },
					{ ACM_RATE_11M,  0, ACM_RATE_ID_11M },
					{ ACM_RATE_6M,   1, ACM_RATE_ID_6M },
					{ ACM_RATE_9M,   1, ACM_RATE_ID_9M },
					{ ACM_RATE_12M,  1, ACM_RATE_ID_12M },
					{ ACM_RATE_18M,  1, ACM_RATE_ID_18M },
					{ ACM_RATE_24M,  1, ACM_RATE_ID_24M },
					{ ACM_RATE_36M,  1, ACM_RATE_ID_36M },
					{ ACM_RATE_48M,  1, ACM_RATE_ID_48M },
					{ ACM_RATE_54M,  1, ACM_RATE_ID_54M }};
	UINT32 TimeHeader, TimeCtsSelf, TimeRtsCts, TimeAck;
	UINT32 TimeDataAck;
	UINT32 IdRateNum, IdSizeNum, IdPreambleNum, Size;

#ifdef ACM_CC_FUNC_11N
	UINT32 TimeDatHdrOnly, TimeDataOnly;
	UINT32 IdBw, IdGI;
#endif // ACM_CC_FUNC_11N //


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* for all rates */
	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> calculate frame body tx time...\n"));

	for(IdRateNum=0; IdRateNum<ACM_RATE_MAX_NUM; IdRateNum++)
	{
		Size = ACM_PRE_TIME_DATA_SIZE_INTERVAL-1;

		for(IdSizeNum=0; IdSizeNum<ACM_PRE_TIME_DATA_SIZE_NUM; IdSizeNum++)
		{
			TimeDataAck = ACM_TX_TimeCal(pAd, NULL,
										Size,					/* body len */
										RateMap[IdRateNum][0],	/* RateIndex */
										RateMap[IdRateNum][1],	/* g mode */
										0,						/* cts-self */
										0,				/* rts/cts */
										0,				/* short IdPreambleNum */
										1,				/* no ack */
										0,				/* txop limit */
										NULL,			/* no data tx time */
										&TimeHeader,	/* wlan header tx time */
										NULL,			/* cts self tx time */
										NULL,			/* rts/cts tx time */
										NULL);			/* ack tx time */
			TimeDataAck -= TimeHeader;
			gAcmTxTimeBody[IdRateNum][IdSizeNum] = TimeDataAck;
			Size += ACM_PRE_TIME_DATA_SIZE_INTERVAL;
		} /* End of for */
	} /* End of for */

#ifdef ACM_CC_FUNC_11N
	for(IdBw=0; IdBw<2; IdBw++)
	{
		for(IdGI=0; IdGI<2; IdGI++)
		{
			for(IdRateNum=0; IdRateNum<ACM_RATE_MAX_NUM_HT; IdRateNum++)
			{
				Size = ACM_PRE_TIME_DATA_SIZE_INTERVAL-1;

				for(IdSizeNum=0;
					IdSizeNum<ACM_PRE_TIME_DATA_SIZE_NUM;
					IdSizeNum++)
				{
					/* not include RTS/CTS time */
					TimeDataAck = ACM_TX_TimeCalHT(pAd, NULL,
										Size,			/* body len */
										IdRateNum,		/* MCS Index */
										IdBw,			/* 20 or 20/40 MHz */
										IdGI,			/* regular or short GI */
										0,				/* rts/cts */
										1,				/* no ack */
										0,				/* not AMPDU */
										0,				/* txop limit */
										NULL,			/* no data tx time */
										NULL,			/* wlan header tx time */
										NULL,			/* ack tx time */
										&TimeDatHdrOnly,/* data+hdr only tx time */
										&TimeDataOnly);	/* data only tx time */

					gAcmTxTimeBodyHT\
						[IdBw][IdGI][IdRateNum][IdSizeNum][0] = TimeDataAck;
					gAcmTxTimeBodyHT\
						[IdBw][IdGI][IdRateNum][IdSizeNum][1] = TimeDatHdrOnly;
					gAcmTxTimeBodyHT\
						[IdBw][IdGI][IdRateNum][IdSizeNum][2] = TimeDataOnly;

					Size += ACM_PRE_TIME_DATA_SIZE_INTERVAL;
				} /* End of for */
			} /* End of for */
		} /* End of for */
	} /* End of for */
#endif // ACM_CC_FUNC_11N //

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> calculate other tx time...\n"));

	for(IdRateNum=0; IdRateNum<ACM_RATE_MAX_NUM; IdRateNum++)
	{
		/* short IdPreambleNum (0) and long IdPreambleNum (1) */
		for(IdPreambleNum=0; IdPreambleNum<2; IdPreambleNum++)
		{
			ACM_TX_TimeCal(pAd, NULL,
							1,						/* BodyLen = 1 */
							RateMap[IdRateNum][0],	/* RateIndex */
							RateMap[IdRateNum][1],	/* g mode */
							1,						/* cts-self */
							0,						/* rts/cts */
							IdPreambleNum,			/* IdPreambleNum */
							0,						/* no ack */
							0,						/* txop limit */
							NULL,					/* no data tx time */
							&TimeHeader,			/* wlan header tx time */
							&TimeCtsSelf,			/* cts self tx time */
							NULL,					/* rts/cts tx time */
							&TimeAck);				/* ack tx time */

			ACM_TX_TimeCal(pAd, NULL,
							0,						/* BodyLen = 0 */
							RateMap[IdRateNum][0],	/* RateIndex */
							RateMap[IdRateNum][1],	/* g mode */
							0,						/* cts-self */
							1,						/* rts/cts */
							IdPreambleNum,			/* IdPreambleNum */
							1,						/* no ack */
							0,						/* txop limit */
							NULL,					/* no data tx time */
							NULL,					/* wlan header tx time */
							NULL,					/* cts self tx time */
							&TimeRtsCts,			/* rts/cts tx time */
							NULL);					/* ack tx time */

			gAcmTxTimeOthers[IdRateNum][IdPreambleNum][0] = RateMap[IdRateNum][2];
			gAcmTxTimeOthers[IdRateNum][IdPreambleNum][1] = (UINT16)TimeCtsSelf;
			gAcmTxTimeOthers[IdRateNum][IdPreambleNum][2] = (UINT16)TimeRtsCts;
			gAcmTxTimeOthers[IdRateNum][IdPreambleNum][3] = (UINT16)TimeHeader;
			gAcmTxTimeOthers[IdRateNum][IdPreambleNum][4] = (UINT16)TimeAck;
		} /* End of for */
	} /* End of for */

#ifdef ACM_CC_FUNC_11N
	/* only for BLOCK ACK frame, 24Mbps */
	TimeDataAck = ACM_TX_TimeCalHT(pAd, NULL,
						FRM_LENGTH_BLOCK_ACK,		/* body len */
						0,							/* MCS Index */
						0,							/* 20 or 20/40 MHz */
						0,							/* regular or short GI */
						0,							/* rts/cts */
						0,							/* no ack */
						0,							/* not AMPDU */
						0,							/* txop limit */
						NULL,						/* no data tx time */
						NULL,						/* wlan header tx time */
						&TimeAck,					/* ack tx time */
						NULL,						/* data+hdr only tx time */
						NULL);						/* data only tx time */

	gAcmTxTimeOthersHT = (UINT16)TimeAck;
#endif // ACM_CC_FUNC_11N //

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> pre-calculattion ok!\n"));
#endif // ACM_CC_FUNC_AUX_TX_TIME //
} /* End of ACM_TX_TimeCalPre */


/*
========================================================================
Routine Description:
	Calculate the frame body transmission time.

Arguments:
	BodyLen			- frame body
	RateId			- frame tx time
	FlgIsGmode		- is G mode

Return Value:
	transmission time (miscro second, us)

Note:
========================================================================
*/
STATIC UINT16 ACM_TX_TimePlcpCal(
	ACM_PARAM_IN	UINT32	BodyLen,
	ACM_PARAM_IN	UINT32	RateId,
	ACM_PARAM_IN	UCHAR	FlgIsGmode)
{
	UINT32 PLCP;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	if (FlgIsGmode)
	{
		PLCP = (BodyLen << 3) + 22; /* need to add 22 bits in 11g */
		PLCP = (PLCP % (gAcmRateG[RateId]<<1)) ? (PLCP/(gAcmRateG[RateId]<<1))+1 :
											(PLCP/(gAcmRateG[RateId]<<1));
		return (PLCP << 2);
	} /* End of if */

	return (BodyLen << 4)/gAcmRateLegacy[RateId];
} /* End of ACM_TX_TimePlcpCal */


#ifdef ACM_CC_FUNC_11N
/*
========================================================================
Routine Description:
	Calculate the frame body transmission time for HT rate.

Arguments:
	BodyLen			- frame body
	McsId			- frame tx MCS
	Nss				- Number of Spatial Streams
	Ness			- Number of Extension Spatial Streams
	FlgIsGF			- 1: Greenfield mode
	FlgIs2040		- 1: support 20/40MHz
	FlgIsSGI		- 1: use short GI
	FlgIsOnlyData	- 1: only calcualte tx time for data, no preamble

Return Value:
	transmission time (miscro second, us)

Note:
========================================================================
*/
STATIC UINT16 ACM_TX_TimePlcpCalHT(
	ACM_PARAM_IN	UINT32	BodyLen,
	ACM_PARAM_IN	UINT32	McsId,
	ACM_PARAM_IN	UINT32	Nss,
	ACM_PARAM_IN	UINT32	Ness,
	ACM_PARAM_IN	BOOLEAN FlgIsGF,
	ACM_PARAM_IN	BOOLEAN	FlgIs2040,
	ACM_PARAM_IN	BOOLEAN	FlgIsSGI,
	ACM_PARAM_IN	BOOLEAN	FlgIsOnlyData)
{

	UINT32 T_LEG_PREAMBLE, T_L_SIG, T_HT_PREAMBLE, T_HT_SIG;
	UINT32 T_SYM, T_SYMS, N_SYM;
	UINT32 T_GF_HT_PREAMBLE;
	UINT32 TxTime;
	UINT32 N_DLTF[5] = { 1, 1, 2, 4, 4 };
	UINT32 N_ELTF[4] = { 0, 1, 2, 4 };
	UINT32 N_SYM_1_NUM; /* numerator of N_SYM */


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* init */
	T_LEG_PREAMBLE = 16;
	T_L_SIG = 4;
	T_HT_SIG = 8;
	T_SYM = 4;
	T_SYMS = 36; /* unit: 0.1us */
	TxTime = 0;

	/* calculate N_SYM */
	/* ex: 1538B, 1st MPDU of AMPDU, (1538 * 8 + 22)/1080 + 1 = 12 */
	/* STBC is not used, BCC is used */
	N_SYM = 1*((BodyLen << 3) + 16 + 6)/(1*gAcmRateNdbps[FlgIs2040][McsId])+1;

	/* calculate transmission time */
	if (FlgIsGF == 0)
	{
		/* ex: 1538B, 1st MPDU of AMPDU, 4 + 4 + 2*4 = 16us */
		T_HT_PREAMBLE = 4 + 4 + ((N_DLTF[Nss] + N_ELTF[Ness] - 1) << 2);

		/* ex: 1538B, 1st MPDU of AMPDU, 16 + 4 + 16 + 8 = 44us */
		if (FlgIsOnlyData == 0)
			TxTime = T_LEG_PREAMBLE + T_L_SIG + T_HT_PREAMBLE + T_HT_SIG;
		/* End of if */

		/* ex: 1538B, 1st MPDU of AMPDU, 4 * 12 = 48us */
		if (FlgIsSGI == 0)
		{
			TxTime += T_SYM * N_SYM;
		}
		else
		{
			N_SYM_1_NUM = (T_SYMS * N_SYM) / (T_SYM * 10);

			if ((T_SYMS * N_SYM) % (T_SYM * 10))
				N_SYM_1_NUM ++;
			/* End of if */

			TxTime += T_SYM * N_SYM_1_NUM;
		} /* End of if */
	}
	else
	{
		T_GF_HT_PREAMBLE = 8 + 4 + ((N_DLTF[Nss] + N_ELTF[Ness] - 1) << 2);

		if (FlgIsOnlyData == 0)
			TxTime = T_GF_HT_PREAMBLE + T_HT_SIG;
		/* End of if */

		if (FlgIsSGI == 0)
			TxTime += T_SYM * N_SYM;
		else
			TxTime += (T_SYMS * N_SYM) / 10;
		/* End of if */
	} /* End of if */

	return TxTime;
} /* End of ACM_TX_TimePlcpCalHT */
#endif // ACM_CC_FUNC_11N //




/* =========================== AP Function =========================== */

#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Handle the Action Frame transmitted from QSTA.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	pMblk			- the received frame
	PhyRate			- the physical tx rate for the frame

Return Value:
	None

Note:
	1. Only for QAP.
	2. PhyRate is used to calculate medium time or polled TXOP
		when min phy rate in the TSPEC is not assigned.
	3. No QoS Control field (2B) in the action management frame.
	4. No any packet free here.
========================================================================
*/
STATIC VOID ACM_ActionHandleByQAP(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	UCHAR				*pMblk,
	ACM_PARAM_IN	UINT32				PktLen,
	ACM_PARAM_IN	UINT32				PhyRate)
{
	ACM_WME_NOT_FRAME *pNotFrame;
	UCHAR *pActFrame;
	UCHAR Category, Action;
	UINT32 BodyLen;
	UCHAR StatusCode;
	UINT16 MediumTime;


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* points to the action frame WLAN header */
	pActFrame = (UCHAR *)pMblk;
	BodyLen = PktLen - ACMR_FME_LEG_HEADER_SIZE;

	/* get Category & Action field */
	pActFrame += ACMR_FME_LEG_HEADER_SIZE;
	Category = *pActFrame;
	Action = *(pActFrame+1);

	/* sanity check for peer */
#ifdef ACM_CC_FUNC_MBSS
	if ((Action != ACM_ACTION_WME_BW_ANN) && (pCdb == NULL))
#else
	if (pCdb == NULL)
#endif // ACM_CC_FUNC_MBSS //
		goto label_exit; /* wrong packet source */
	/* End of if */

	/* handle it by Category field */
	if (Category == ACM_CATEGORY_WME)
	{
#ifdef ACM_CC_FUNC_MBSS
		if (Action != ACM_ACTION_WME_BW_ANN)
#endif // ACM_CC_FUNC_MBSS //
		{
			if (!ACMR_IS_WMM_STA(pCdb))
			{
				ACMR_DEBUG(ACMR_DEBUG_ERR,
							("acm_err> A WME frame is RCV from a non-WME STA! "
							"Discard the frame! ActionHandleByQAP()\n"));
				goto label_exit; /* wrong packet type */
			} /* End of if */
		} /* End of if */

		/* WME action frame */
		switch(Action)
		{
			case ACM_ACTION_WME_SETUP_REQ: /* ADDTS Request */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> A WME Setup Request frame is received!\n"));

				ACM_WME_ActionHandle(pAd,
									pCdb,
									pActFrame,
									BodyLen,
									PhyRate,
									ACM_ACTION_WME_SETUP_REQ,
									&StatusCode,
									&MediumTime);
				break;

			case ACM_ACTION_WME_TEAR_DOWN: /* DELTS */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> A WME Teardown frame is received!\n"));

				ACM_WME_ActionHandle(pAd,
									pCdb,
									pActFrame,
									BodyLen,
									0, /* dont care phy rate */
									ACM_ACTION_WME_TEAR_DOWN,
									&StatusCode,
									&MediumTime);
				break;

#ifdef ACM_CC_FUNC_MBSS
			case ACM_ACTION_WME_BW_ANN: /* BW ANN */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> A WME BW ANN frame is received!\n"));

				ACM_MBSS_BwAnnHandle(pAd, pActFrame, BodyLen);

				/* not forward the public frame to other BSSs to avoid loop */
				goto label_exit;
#endif // ACM_CC_FUNC_MBSS //

			default:
				goto label_exit;
		} /* End of switch */

		/* check whether we need to reply a ADDTS Response frame;
			if TRUE, send out one */
		if (Action == ACM_ACTION_WME_SETUP_REQ)
		{
			/* if the ACM of the AC is disabled, we do not need to response */
			if (StatusCode != ACM_STATUS_CODE_PRIVATE_ACM_DISABLED)
			{
				UCHAR MacRa[6], MacTa[6];


				ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> prepare to response...\n"));

				/* init response frame */
				StatusCode = ACM_11E_WMM_StatusTranslate(StatusCode);

				pNotFrame = (ACM_WME_NOT_FRAME *)pActFrame;
				pNotFrame->Action = ACM_ACTION_WME_SETUP_RSP;
				pNotFrame->StatusCode = StatusCode;

				/*
					Test Event 1 found:
					we need to re-assign the allowed medium time.
				*/
				pNotFrame->ElmTspec.Tspec.MediumTime = MediumTime;

				/* swap DA & SA */
				ACMR_WLAN_PKT_RA_GET(pMblk, MacRa);
				ACMR_WLAN_PKT_TA_GET(pMblk, MacTa);
				ACMR_WLAN_PKT_RA_SET(pMblk, MacTa);
				ACMR_WLAN_PKT_TA_SET(pMblk, MacRa);

				/* send out it */
				ACM_ADDRSP_SEND(pAd, pMblk, PktLen);

				/* only test use */
				if (gAcmTestFlag)
					ACM_ADDRSP_SEND(pAd, pMblk, PktLen);
				/* End of if */

				ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> Send a WME Setup response (len=%d).\n", PktLen));
			} /* End of if */
		} /* End of if */

		/* send a private public ACTION frame to advise used ACM time in AP */
		ACM_FrameBwAnnSend(pAd, FALSE);
	} /* End of if */

label_exit:
	return;
} /* End of ACM_ActionHandleByQAP */


/*
========================================================================
Routine Description:
	Handle a TSPEC request from the QSTA.

Arguments:
	pAd						- WLAN control block pointer
	*pCdb					- the QSTA database
	StreamType				- the stream type: ACM_STREAM_TYPE_WIFI
	DialogToken				- the TSPEC ID
	*pTspec					- the requested TSPEC pointer
	TclasNum				- the number of TCLASS, max 5
	*pTclas[]				- the requested TCLASS array pointer
	TclasProcessing			- the TCLAS Processing element
	PhyRate					- the observed physical transmit rate (bps)
	*pStatusCode			- response status code
	*pMediumTime			- the allowed medium time

Return Value:
	ACM_RTN_OK				- request is accepted
	ACM_RTN_FAIL			- semaphore lock fail or others
	ACM_RTN_NULL_POINTER	- null pointer
	ACM_RTN_ALLOC_ERR		- TSPEC request structure allocation fail
	ACM_RTN_DISALLOW		- the request is not allowed

Note:
	1. Only for Root AP Mode.
	2. pTclasSrc is limited by ACM_TSPEC_TCLAS_MAX_NUM.
	3. DLP is not allowed because we can not monitor traffic
		on the direct link when inactivity timeout != 0.
		We allow DLP protocol but we dont allow DLP TSPEC.
	4. for uplink, dst_cdb_p == NULL means the destination STA is QAP;
		for direct link, dst_cdb must NOT be NULL;
		for dnlink, dst_cdb_p == NULL;
		for bidirectional link, dst_cdb_p = NULL.
	5. TSPEC parameters that must be non-zero to pass in ADDTS Rsp:
		(1) Mean Data Rate;
		(2) Nominal MSDU Size;
		(3) Maximum Service Interval;
		(4) Minimum PHY Rate.
	6. TCLAS number can be 0 for EDCA/WMM streams.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_TC_ReqHandle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				StreamType,
	ACM_PARAM_IN	UINT16				DialogToken,
	ACM_PARAM_IN	ACM_TSPEC			*pTspec,
	ACM_PARAM_IN	UINT32				TclasNum,
	ACM_PARAM_IN	ACM_TCLAS			*pTclas[],
	ACM_PARAM_IN	UCHAR				TclasProcessing,
	ACM_PARAM_IN	UINT32				PhyRate,
	ACM_PARAM_OUT	UCHAR				*pStatusCode,
	ACM_PARAM_OUT	UINT16				*pMediumTime)
{
	ACM_STREAM *pNewStream;
	ACM_STREAM *pOldStreamIn, *pOldStreamOut, *pOldStreamDiffAc;
	ACM_FUNC_STATUS RtnCode, Status;
	UCHAR StatusCode;
	UCHAR Policy, UserPriority;
	UCHAR StmAcId, ApsdAcId, FlgIsApsdEnable;
	UCHAR FlgIsSupRate;
	ULONG SplFlags;
#ifdef ACM_CC_FUNC_TCLAS
	UINT32 IdTclasNum;
#endif // ACM_CC_FUNC_TCLAS //
#ifdef ACM_CC_FUNC_HCCA
	UINT32 MinServInt;
#endif // ACM_CC_FUNC_HCCA //


	WMM_ACM_FUNC_NAME_PRINT("IN");

	/* ---- Sanity check for input parameters ---- */
	*pStatusCode = ACM_STATUS_CODE_UNSPECIFIED_FAILURE;

	if (!ACM_MR_TSPEC_IS_ALLOWED(pAd))
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> ACM is not allowed!\n"));
		return ACM_RTN_DISALLOW;
	} /* End of if */

	if (pTspec == NULL)
	{
		/* TSPEC element shall NOT be NULL */
		return ACM_RTN_NULL_POINTER;
	} /* End of if */

#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_ACL
	/* ACL check */
	if (ACM_MR_ACL_IS_ENABLED(pAd))
	{
		BOOLEAN FlgIsAllowed;

		ACM_LIST_ENTRY_GET(pAd, ACMR_CLIENT_MAC(pCdb), FlgIsAllowed, NULL);
		if (FlgIsAllowed == FALSE)
		{
			/* not in ACL list so we reject it */
			return ACM_RTN_DISALLOW;
		} /* End of if */
	}
#endif // ACM_CC_FUNC_ACL //
#endif // CONFIG_AP_SUPPORT //

	/* init */
	pOldStreamIn = NULL;
	pOldStreamOut = NULL;
	pOldStreamDiffAc = NULL;
	RtnCode = ACM_RTN_FAIL;
	StatusCode = ACM_STATUS_CODE_UNSPECIFIED_FAILURE;
	ApsdAcId = 0;
	FlgIsApsdEnable = 0;
	*pMediumTime = 0;
	FlgIsSupRate = 0;

	/* allocate a TSPEC request TS */
	ACMR_MEM_ALLOC(pNewStream, sizeof(ACM_STREAM), (ACM_STREAM *));

	if (pNewStream == NULL)
		return ACM_RTN_ALLOC_ERR;
	/* End of if */

	ACMR_MEM_ZERO(pNewStream, sizeof(ACM_STREAM));

	/* init client database pointer, Dialog Token, TSPEC, TCLASS, and
		TCLASS Processing */
	ACM_STREAM_CDB_COPY(pNewStream, pCdb);
	pNewStream->DialogToken = DialogToken;
	pNewStream->StreamType = StreamType;

	/* allocate/copy a TSPEC */
	ACMR_MEM_ALLOC(pNewStream->pTspec, sizeof(ACM_TSPEC), (ACM_TSPEC *));

	if (pNewStream->pTspec == NULL)
		goto LabelErrAlloc;
	/* End of if */

	ACM_TSPEC_COPY(pNewStream->pTspec, pTspec);

#ifdef ACM_CC_FUNC_TCLAS
	/* continue to check input TSPEC parameters */
	if ((TclasNum > 1) && (TclasProcessing == ACM_TCLAS_PROCESSING_NOT_EXIST))
	{
		/* ERR! no TCLAS Processing element is found when num of TCLAS >= 2 */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> No tclas processing element!\n"));
		StatusCode = ACM_STATUS_CODE_DECLINED;
		RtnCode = ACM_RTN_INVALID_PARAM;
		goto LabelRspErr;
	} /* End of if */
#endif // ACM_CC_FUNC_TCLAS //

	if (pTspec->TsInfo.AccessPolicy != ACM_ACCESS_POLICY_EDCA)
	{
		/* ERR! we do NOT support HCCA */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Access policy %d != EDCA %d!\n",
					pTspec->TsInfo.AccessPolicy,
					ACM_ACCESS_POLICY_EDCA));
		StatusCode = ACM_STATUS_CODE_DECLINED;
		RtnCode = ACM_RTN_INVALID_PARAM;
		goto LabelRspErr;
	} /* End of if */

#ifdef ACM_CC_FUNC_TCLAS
	if ((TclasProcessing != ACM_WME_TCLAS_PROCESSING_ALL) &&
		(TclasProcessing != ACM_WME_TCLAS_PROCESSING_ONE) &&
		(TclasProcessing != ACM_TCLAS_PROCESSING_NOT_EXIST))
	{
		/* ERR! invalid TCLAS Processing value in D6.0 */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Wrong tclas process value!\n"));
		RtnCode = ACM_RTN_INVALID_PARAM;
		goto LabelRspErr;
	} /* End of if */
#endif // ACM_CC_FUNC_TCLAS //


	/* ---- Create and init a stream structure ---- */


#ifdef ACM_CC_FUNC_TCLAS
	/* allocate/copy TCLAS */
	for(IdTclasNum=0; IdTclasNum<TclasNum; IdTclasNum++)
	{
		if (pTclas[IdTclasNum] != NULL)
		{
			ACMR_MEM_ALLOC(pNewStream->pTclas[IdTclasNum],
							sizeof(ACM_TCLAS), (ACM_TCLAS *));

			if (pNewStream->pTclas[IdTclasNum] == NULL)
			{
				RtnCode = ACM_RTN_ALLOC_ERR;
				goto LabelRspErr;
			} /* End of if */

			ACM_TCLAS_COPY(pNewStream->pTclas[IdTclasNum], pTclas[IdTclasNum]);
			continue;
		} /* End of if */

		/* OK! all TCLAS are copied */
		break;
	} /* End of for */
#endif // ACM_CC_FUNC_TCLAS //

	/* init other parameters */
	pNewStream->TclasProcessing = TclasProcessing;
	pNewStream->TimeoutDelts = ACM_DELTS_TIMEOUT/ACM_TIMEOUT_CHECK_BASE;


	/* ---- Sanity check for TSPEC parameters ---- */
	/* check source STA */
	if ((ACMR_IS_QSTA(pCdb) == 0) && (!ACMR_IS_WMM_STA(pCdb)))
	{
		/* ERR! source STA is not a QSTA and not a WMM STA */
		StatusCode = ACM_STATUS_CODE_DECLINED;
		goto LabelRspErr;
	} /* End of if */

	/* check TCLSS parameters and get user priority */
	UserPriority = ACM_UP_UNKNOWN;

#ifdef ACM_CC_FUNC_TCLAS
	for(IdTclasNum=0; IdTclasNum<TclasNum; IdTclasNum++)
	{
		if (pTclas[IdTclasNum] != NULL)
		{
			/* check classifier type */
			if (pTclas[IdTclasNum]->ClassifierType > ACM_TCLAS_TYPE_MAX)
			{
				/* classifier type is illegal */
				StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
				goto LabelRspErr;
			} /* End of if */

			/* check user priority, priority for each TCLAS must be the same */
			if (UserPriority == ACM_UP_UNKNOWN)
				UserPriority = pTclas[IdTclasNum]->UserPriority;
			else
			{
				if (pTclas[IdTclasNum]->UserPriority != UserPriority)
				{
					/* user priority for all TCLASS element shall be the same */
					StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
					goto LabelRspErr;
				} /* End of if */
			} /* End of if */
		} /* End of if */
	} /* End of for */
#endif // ACM_CC_FUNC_TCLAS //


	/* ---- Check for Direct Link TSPEC ---- */
	if (pTspec->TsInfo.Direction == ACM_DIRECTION_DIRECT_LINK)
	{
		/* DLP is not allowed */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Direct Link is not allowed!\n"));
		StatusCode = ACM_STATUS_CODE_DIRECT_LINK_IS_NOT_ALLOWED;
		goto LabelRspErr;
	} /* End of if */


	/* ---- Update the UP ---- */
	if (UserPriority == ACM_UP_UNKNOWN)
	{
		/* use the UP in the TSPEC if no any TCLAS exists */
		UserPriority = pTspec->TsInfo.UP;
	} /* End of if */

	pNewStream->UP = UserPriority;

	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> UP = %d\n", UserPriority));


	/* ---- non-ACM TSPEC check ---- */
	StmAcId = ACM_MR_EDCA_AC(UserPriority);

	/* we need to keep the TSPEC even it is a NULL TSPEC so mark the code */

	/* ---- Sanity check for TSPEC parameters (after non-ACM TSPEC) ---- */
	if (pTspec->MinPhyRate == 0)
	{
		/* use observed physical rate of the ADDTS request frame */
		pTspec->MinPhyRate = PhyRate;
	} /* End of if */

#ifdef ACM_CC_FUNC_TCLAS
	if ((pTspec->MinPhyRate == 0) ||
		(TclasNum > ACM_TSPEC_TCLAS_MAX_NUM))
#else
	if (pTspec->MinPhyRate == 0)
#endif // ACM_CC_FUNC_TCLAS //
	{
		/* in WMM ACM WIFI test spec., min phy rate can NOT be 0 */
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> min phy rate = 0 or tclas is too many!\n"));
		RtnCode = ACM_RTN_INVALID_PARAM;
		goto LabelRspErr;
	} /* End of if */

	ACM_TG_CMT_UAPSD_SETTING_ON_NON_ACM_AC;

	/* check if the TSPEC is for non-ACM AC */
#ifdef ACM_CC_FUNC_SPEC_CHANGE_TG
	if (ACMR_CB->EdcaCtrlParam.FlgAcmStatus[StmAcId] != ACM_FLG_FUNC_DISABLED)
#endif // ACM_CC_FUNC_SPEC_CHANGE_TG //
	{
		UINT32 MaxRate;


		ACM_TG_CMT_MIN_PHY_RATE;

		/* check min phy rate can not larger than maximum supported rate */
		ACMR_SUP_RATE_MAX_GET(pAd, pCdb, MaxRate);

		if (pTspec->MinPhyRate > MaxRate)
		{
			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> min phy rate %dbps > "
						"maximum supported rate %dbps!\n",
						pTspec->MinPhyRate, MaxRate));
			RtnCode = ACM_RTN_INVALID_PARAM;
			goto LabelRspErr;
		} /* End of if */

		/* check min phy rate must be one of supported rates non-11n */
#ifdef ACM_CC_FUNC_11N
		if (!ACMR_IS_11N_ONLY_SUPPORT(pAd, pCdb))
		/* if only 11n is supported, no need to check non-11n supported rate */
#endif // ACM_CC_FUNC_11N //
		{
			ACMR_SUP_RATE_CHECK(pAd, pTspec->MinPhyRate, FlgIsSupRate);
		}

		if (FlgIsSupRate == 0)
		{
#ifdef ACM_CC_FUNC_11N
			/* check if AP & STA support 11n mode */
			if (ACMR_IS_11N_SUPPORT(pAd) && ACMR_STA_IS_11N_SUPPORT(pCdb))
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> min phy rate is 11n rate.\n"));

				/* check min phy rate must be one of supported rates 11n */
				ACMR_SUP_RATE_CHECK_11N(pAd,
										pCdb,
										pTspec->MinPhyRate,
										FlgIsSupRate);

				/* set maximum BA Window Size */
				pNewStream->HT_BaWinSize = 64; /* maximum */
				ACMR_STA_MAX_BACK_NUM_GET(pAd, pCdb, pNewStream->HT_BaWinSize);
			} /* End of if */
#endif // ACM_CC_FUNC_11N //

			if (FlgIsSupRate == 0)
			{
				/* not supported rate */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> min phy rate %d is not a supported rate!\n",
							pTspec->MinPhyRate));
				RtnCode = ACM_RTN_INVALID_PARAM;
				goto LabelRspErr;
			} /* End of if */
		} /* End of if */

		ACM_TG_CMT_B0_TS_INFO;

		/* check minimum TSPEC parameters, reference to Annex H.1 in D6.0 */
		Policy = pTspec->TsInfo.AccessPolicy;

		if (Policy == ACM_ACCESS_POLICY_EDCA)
		{
			/* Contention Based CBR Traffic, EDCA */
			if ((pTspec->NominalMsduSize == 0) ||
#ifndef ACM_CC_FUNC_SPEC_CHANGE_TG
				(pTspec->MeanDataRate == 0) ||
#endif // ACM_CC_FUNC_SPEC_CHANGE_TG //
				(pTspec->SurplusBandwidthAllowance == 0) ||
				(pTspec->TsInfo.Aggregation != ACM_AGGREGATION_DISABLE) ||
				(pTspec->TsInfo.Schedule != ACM_SCHEDULE_NO))
			{
				/* some parameters are invalid */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> nominal msdu size = %d or "
							"mean data rate = %d or "
							"surplus bandwidth allowance = %d or "
							"aggregation = %d or "
							"schedule = %d!\n",
							pTspec->NominalMsduSize,
							pTspec->MeanDataRate,
							pTspec->SurplusBandwidthAllowance,
							pTspec->TsInfo.Aggregation,
							pTspec->TsInfo.Schedule));
				StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
				goto LabelRspErr;
			} /* End of if */
		}
		else
		{
			/* non-EDCA is not support */
			StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
			goto LabelRspErr;
		} /* End of if */

#ifdef ACM_CC_FUNC_HCCA
		/* check suspension interval */
		if ((pTspec->SuspensionInt != ACM_TSPEC_SUSPENSION_DISABLE) &&
			(pTspec->InactivityInt != ACM_TSPEC_INACTIVITY_DISABLE))
		{
			/* suspension and inactivity interval are enabled */
			if (pTspec->SuspensionInt >= pTspec->InactivityInt)
			{
				/* error: suspension interval > inactivity interval */
				StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
				goto LabelRspErr;
			} /* End of if */
		} /* End of if */

		/* check TS Info */
		if (pTspec->TsInfo.AckPolicy != ACM_ACK_POLICY_NORMAL)
		{
			ACM_TG_CMT_NO_ACK_POLICY;

			/*
				1. Block Ack state change is not supported for WMM ACM;
				2. No ACK is not supported for WMM ACM;
				3. No schedule is needed because no scheduled EDCA APSD
				is supported.
			*/
			StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
			goto LabelRspErr;
		} /* End of if */

		/* get QOS MIB database */
		if (pTspec->TsInfo.Aggregation == 1)
		{
			/* not support */
		} /* End of if */

		/* check service start time */
		if (pTspec->ServiceStartTime != 0)
		{
			/* not support */
		} /* End of if */

		/* check minimum service interval */
		if (pTspec->MinServInt != 0)
		{
			/* mininum service interval is enabled */
			if (pTspec->MinServInt < ACM_TSPEC_MIN_SERV_INT_LIMIT)
			{
				/* minimum service interval is too small */
				pTspec->MinServInt = ACM_TSPEC_MIN_SERV_INT_LIMIT;
				StatusCode = ACM_STATUS_CODE_SUGGESTED_TSPEC;
				goto LabelRspErr;
			} /* End of if */
		} /* End of if */

		if (pTspec->MinServInt > ACM_TSPEC_INACTIVITY_MIN)
			MinServInt = (pTspec->MinServInt<<1); /* for safe, *2 */
		else
			MinServInt = ACM_TSPEC_INACTIVITY_MIN;
		/* End of if */


		/* check minimum inactivity interval (must > minimum SI) */
		if (pTspec->InactivityInt != ACM_TSPEC_INACTIVITY_DISABLE)
		{
			/* inactivity interval is enabled */
			if (pTspec->InactivityInt < MinServInt)
			{
				/* inactivity interval is too small */
				pTspec->InactivityInt = MinServInt;
				StatusCode = ACM_STATUS_CODE_SUGGESTED_TSPEC;
				goto LabelRspErr;
			} /* End of if */
		} /* End of if */


		/* check minimum suspension interval */
		if (pTspec->SuspensionInt != ACM_TSPEC_SUSPENSION_DISABLE)
		{
			/* suspension interval is enabled */
			if (pTspec->SuspensionInt < ACM_TSPEC_SUSPENSION_MIN)
			{
				/* suspension interval is too small */
				pTspec->SuspensionInt = ACM_TSPEC_SUSPENSION_MIN;
				StatusCode = ACM_STATUS_CODE_SUGGESTED_TSPEC;
				goto LabelRspErr;
			} /* End of if */
		} /* End of if */
#endif // ACM_CC_FUNC_HCCA //
	} /* End of if */


	/* ---- Handle TSPEC ---- */
//#ifndef ACM_CC_FUNC_SPEC_CHANGE_TG
	/* adjust min phy rate and get minimum phy mode & mcs */
	ACM_PacketPhyModeMCSSet(pAd, pNewStream);
//#endif // ACM_CC_FUNC_SPEC_CHANGE_TG //

	if (pTspec->TsInfo.AccessPolicy == ACM_ACCESS_POLICY_EDCA)
	{
		/* no aggregation support for EDCA */
		if (pTspec->TsInfo.Aggregation == ACM_AGGREGATION_ENABLE)
		{
			pTspec->TsInfo.Aggregation = ACM_AGGREGATION_DISABLE;
			StatusCode = ACM_STATUS_CODE_SUGGESTED_TSPEC;
			goto LabelRspErr;
		} /* End of if */

		/* semaphore protection */
		RtnCode = ACM_RTN_SEM_GET_ERR;
		ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

		/* check if the request is a negotiated request */
		Status = ACM_TC_RenegotiationCheck(
											pAd,
											ACMR_CLIENT_MAC(pCdb),
											UserPriority,
											&pTspec->TsInfo,
											&pOldStreamIn,
											&pOldStreamOut,
											&pOldStreamDiffAc);

#ifdef ACM_CC_FUNC_REPLACE_RULE_TG

		if (Status == ACM_RTN_OK)
		{
			if (ACM_TC_ReplacementCheck(pAd,
										ACMR_CLIENT_MAC(pCdb),
										UserPriority,
										&pTspec->TsInfo) != ACM_RTN_OK)
			{
				ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> Reject the TSPEC due to "
							"replacement rule check!\n"));
				StatusCode = ACM_STATUS_CODE_INVALID_PARAMETERS;
				goto LabelRspErr;
			} /* End of if */
		} /* End of if */
#endif // ACM_CC_FUNC_REPLACE_RULE_TG //

		switch(Status)
		{
			case ACM_RTN_FAIL:
				/* this is a new request */
				ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> new TSPEC!\n"));
				break;

			case ACM_RTN_OK:
				/* this is negotiated request */
				ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> same TSPEC!\n"));
				pNewStream->ReNegotiation = 1; /* this is a nego TSPEC */
				break;

			case ACM_RTN_FATAL_ERR:
			case ACM_RTN_RENO_IN_REQ_LIST:
			default:
				/*
					Only these TSPEC whose Status == DELETING will exist in
					the requested list, means the old one is in DELETING state,
					we do NOT handle any negotiated request for this one.
				*/
				StatusCode = ACM_STATUS_CODE_UNSPECIFIED_FAILURE;
				goto label_rsp_err_sem;
		} /* End of switch */

		/*
			Assign TS ID:
			AcmAcId: for dnlink or bidirectional link, it is the physical TS
			or AC ID; otherwise it is the backup array ID.
		*/
		pNewStream->AcmAcId = ACM_MR_EDCA_AC(pNewStream->UP);

		/*
			Check ok so calculate needed medium time.

			Note: if no ACM is needed for the AC, ACM_EDCA_ReqHandle() will
			return error.
		*/
		StatusCode = ACM_EDCA_ReqHandle(pAd,
										pCdb,
										pNewStream,
										pOldStreamIn,
										pOldStreamOut,
										pOldStreamDiffAc);

		if (StatusCode == ACM_STATUS_CODE_SUCCESS)
		{
			/* add the device to the entry list */
			ACM_PeerDeviceAdd(pAd, ACMR_CLIENT_MAC(pCdb));

			/* get UAPSD parameters */
			ApsdAcId = pNewStream->AcmAcId;
			FlgIsApsdEnable = pTspec->TsInfo.APSD;

			/* assign return medium time */
			if (pTspec->TsInfo.Direction != ACM_DIRECTION_DOWN_LINK)
				*pMediumTime = pNewStream->pTspec->MediumTime;
			/* End of if */
		} /* End of if */

		if (StatusCode != ACM_STATUS_CODE_SUCCESS)
		{
			ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);
			goto LabelRspErr;
		} /* End of if */

		goto LabelOK;
	} /* End of if */


	/* ---- Handle non-EDCA TSPEC ---- */
	/* HCCA is not support */
	ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> HCCA is not supported!\n"));
	StatusCode = ACM_STATUS_CODE_DECLINED;
	RtnCode = ACM_RTN_DISALLOW;
	goto LabelRspErr;

LabelOK:
	/* active stream check timer for any stream */
	ACMR_TIMER_ENABLE(ACMR_CB->FlgStreamAliveCheckEnable,
						ACMR_CB->TimerStreamAliveCheck,
						ACM_STREAM_CHECK_OFFSET);

	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	if (ACMR_STA_IS_IN_ACTIVE_MODE(pCdb))
	{
		/* reset UAPSD state only in ACTIVE state */
		ACM_APSD_Ctrl(pAd, pCdb, ApsdAcId,
						pNewStream->pTspec->TsInfo.Direction,
						1, FlgIsApsdEnable);
	}
	else
	{
		/* mark a UAPSD unhandled flag for TSPEC */
		pNewStream->FlgUapsdHandleNeed = 1;
	} /* End of if */


	/* response TSPEC to QSTA with code = success */
	*pStatusCode = ACM_STATUS_CODE_SUCCESS;

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("\nacm_msg> A request is accepted (Status=%d)!\n",
                *pStatusCode));
	return ACM_RTN_OK;

label_rsp_err_sem:
	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

LabelRspErr:
LabelSemErr:
	/* response TSPEC to QSTA with error code */
	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> A request is rejected (%d)!\n", StatusCode));

LabelErrAlloc:
	/* free local allocated memory */
	ACM_FREE_TS(pNewStream);
	*pStatusCode = StatusCode;
	return RtnCode;
} /* End of ACM_TC_ReqHandle */

#endif // CONFIG_AP_SUPPORT //





#endif // WMM_ACM_SUPPORT //

/* End of acm_comm.c */
