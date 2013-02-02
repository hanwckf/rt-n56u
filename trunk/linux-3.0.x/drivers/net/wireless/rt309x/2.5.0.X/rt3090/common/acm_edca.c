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

	All related WMM ACM EDCA function body.

***************************************************************************/

#define MODULE_WMM_ACM
#include "rt_config.h"

#ifdef WMM_ACM_SUPPORT

/* IEEE802.11E related include files */
#include "acm_extr.h" /* used for other modules */
#include "acm_comm.h" /* used for edca/wmm */
#include "acm_edca.h" /* used for edca/wmm */



/* ----- EDCA External Variable (only used in ACM module) ----- */
extern UCHAR gTCLAS_Elm_Len[];

#ifdef ACM_MEMORY_TEST
extern UINT32 gACM_MEM_Alloc_Num;
extern UINT32 gACM_MEM_Free_Num;
#endif // ACM_MEMORY_TEST //


/* ----- EDCA Private Variable ----- */
/* AC0 (BE), AC1 (BK), AC2 (VI), AC3 (VO); AC3 > AC2 > AC0 > AC1 */
/* DSCP = (DSCP >> 3) & 0x07 */
UCHAR gEDCA_UP_AC[8] = { 0, 1, 1, 0, 2, 2, 3, 3 };

/* AC0 vs. UP0, AC1 vs. UP1, AC2 vs. UP4, AC3 vs. UP6 */
UCHAR gEDCA_AC_UP[4] = { 0, 1, 4, 6 };

/* DSCP(0) -> Priority 0 (AC_BE)
   DSCP(1) -> Priority 1 (AC_BK)
   DSCP(2) -> Priority 2 (AC_BK)
   DSCP(3) -> Priority 3 (AC_BE)
   DSCP(4) -> Priority 4 (AC_VI)
   DSCP(5) -> Priority 5 (AC_VI)
   DSCP(6) -> Priority 6 (AC_VO)
   DSCP(7) -> Priority 7 (AC_VO) */
UCHAR gEDCA_UP_DSCP[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };





/* ====================== Public Function ============================= */

/*
========================================================================
Routine Description:
	Change current EDCA Information.

Arguments:
	pAd				- WLAN control block pointer
	CpNu			- the numerator of Contention Period,
						if 0, no any update for CpNu
	CpDe			- the denominator of Contention Period
						if 0, no any update for CpDe
	BeNu			- the numerator of Best Effort percentage,
						if 0, no any update for BeNu
	BeDe			- the denominator of Best Effort percentage
						if 0, no any update for BeDe

Return Value:
	ACM_RTN_OK		- change ok
	ACM_RTN_FAIL	- change fail

Note:
	1. CpNu/CpDe is the percentage of EDCA in 1 second.
	2. BeNu/BeDe is the percentage of Best Effort streams in 1 second.
	3. The function will not delete any stream if residunt
		bandwidth is not enough for (CpNu/CpDe)*SI or (BeNu/BeDe).
	4. New (CpNu/CpDe) or (BeNu/BeDe) will be valid after bandwidth is enough.
	5. If the old ACM is enabled and the new ACM is disabled,
		the function will not deleted these streams use the AC.
========================================================================
*/
ACM_FUNC_STATUS ACM_EDCA_InfomationChange(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT16					CpNu,
	ACM_PARAM_IN	UINT16					CpDe,
	ACM_PARAM_IN	UINT16					BeNu,
	ACM_PARAM_IN	UINT16					BeDe)
{
	ACM_CTRL_PARAM *pEdcaParam;
	ULONG SplFlags;


	/* init */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);

	/* change EDCA parameters */
	ACM_TSPEC_SEM_LOCK_CHK_RTN(pAd, SplFlags, LabelSemErr, ACM_RTN_FAIL);

	/* copy new settings to new parameters */
	if ((CpNu > 0) && (CpDe > 0))
	{
		pEdcaParam->CP_MinNu = CpNu;
		pEdcaParam->CP_MinDe = CpDe;
	} /* End of if */

	if ((BeNu > 0) && (BeDe > 0))
	{
		pEdcaParam->BEK_MinNu = BeNu;
		pEdcaParam->BEK_MinDe = BeDe;
	} /* End of if */

	ACM_TSPEC_SEM_UNLOCK(pAd, LabelSemErr);

	/* delete all streams */
	ACMP_TC_DeleteAll(pAd);
	return ACM_RTN_OK;

LabelSemErr:
	return ACM_RTN_FAIL;
} /* End of ACM_EDCA_InfomationChange */


/*
========================================================================
Routine Description:
	Check whether the element is WME Information.

Arguments:
	*pElm			- the element
	SubType			- the sub type

Return Value:
	ACM_RTN_OK		- check ok
	ACM_RTN_FAIL	- check fail

Note:
	No semaphore protection is needed.
========================================================================
*/
ACM_FUNC_STATUS ACM_WME_ELM_Check(
	ACM_PARAM_IN	UCHAR				*pElm,
	ACM_PARAM_IN	UCHAR				SubType)
{
	UCHAR ElmLen[3] = { ACM_ELM_WME_INFO_LEN,
						ACM_ELM_WME_PARAM_LEN,
						ACM_ELM_WME_TSPEC_LEN };


	if ((pElm[0] == ACM_ELM_WME_ID) &&
		(pElm[1] == ElmLen[SubType]) &&
		(pElm[2] == ACM_WME_OUI_0) &&
		(pElm[3] == ACM_WME_OUI_1) &&
		(pElm[4] == ACM_WME_OUI_2) &&
		(pElm[5] == ACM_WME_OUI_TYPE) &&
		(pElm[6] == SubType) &&
		(pElm[7] == ACM_WME_OUI_VERSION))
	{
		return ACM_RTN_OK;
	} /* End of if */

	return ACM_RTN_FAIL;
} /* End of ACM_WME_ELM_Check */




#ifdef CONFIG_STA_SUPPORT_SIM
/*
========================================================================
Routine Description:
	Fill a TSPEC element to a buffer.

Arguments:
	pAd				- WLAN control block pointer
	*pBuffer		- the buffer
	*pTspec11e		- the current TSPEC

Return Value:
	filled element length

Note:
========================================================================
*/
UINT32 ACMP_WME_TSPEC_ElementFill(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pBuffer,
	ACM_PARAM_IN	ACM_TSPEC				*pTspec11e)
{
	ACM_ELM_WME_TSPEC *pElmTspec;
	ACM_WME_TSPEC *pTspecWme;
	ACM_WME_TS_INFO *pInfo;


	/* sanity check */
	if ((pBuffer == NULL) || (pTspec11e == NULL))
		return 0;
	/* End of if */

	/* TSPEC element */
	pElmTspec = (ACM_ELM_WME_TSPEC *)pBuffer;
	pElmTspec->ElementId = ACM_ELM_WME_ID;
	pElmTspec->Length = ACM_ELM_WME_TSPEC_LEN;

	/* init OUI field */
	pElmTspec->OUI[0] = ACM_WME_OUI_0;
	pElmTspec->OUI[1] = ACM_WME_OUI_1;
	pElmTspec->OUI[2] = ACM_WME_OUI_2;
	pElmTspec->OUI_Type = ACM_WME_OUI_TYPE;
	pElmTspec->OUI_SubType = ACM_WME_OUI_SUBTYPE_TSPEC;
	pElmTspec->Version = ACM_WME_OUI_VERSION;

	/* init TS Info field */
	pTspecWme = &pElmTspec->Tspec;
	ACMR_MEM_ZERO(pTspecWme, sizeof(ACM_WME_TSPEC));
	pInfo = &pElmTspec->Tspec.TsInfo;

	pInfo->TID = pTspec11e->TsInfo.TSID;
	pInfo->Direction = pTspec11e->TsInfo.Direction;
	pInfo->UP = pTspec11e->TsInfo.UP;
	pInfo->PSB = pTspec11e->TsInfo.APSD;
	pInfo->One = 1; /* always 1 */

#ifdef ACM_CC_FUNC_11N
	pInfo->Reserved2 = pTspec11e->TsInfo.AckPolicy;
#endif // ACM_CC_FUNC_11N //

	/* init TSPEC parameters */
	pTspecWme->NominalMsduSize = pTspec11e->NominalMsduSize;
	pTspecWme->MaxMsduSize = pTspec11e->MaxMsduSize;
	pTspecWme->MinServInt = pTspec11e->MinServInt;
	pTspecWme->MaxServInt = pTspec11e->MaxServInt;
	pTspecWme->InactivityInt = pTspec11e->InactivityInt;
	pTspecWme->SuspensionInt = pTspec11e->SuspensionInt;
	pTspecWme->ServiceStartTime = pTspec11e->ServiceStartTime;
	pTspecWme->MinDataRate = pTspec11e->MinDataRate;
	pTspecWme->MeanDataRate = pTspec11e->MeanDataRate;
	pTspecWme->PeakDataRate = pTspec11e->PeakDataRate;
	pTspecWme->MaxBurstSize = pTspec11e->MaxBurstSize;
	pTspecWme->DelayBound = pTspec11e->DelayBound;
	pTspecWme->MinPhyRate = pTspec11e->MinPhyRate;
	pTspecWme->SurplusBandwidthAllowance = \
							pTspec11e->SurplusBandwidthAllowance;

	return (sizeof(ACM_ELM_WME_TSPEC));
} /* End of ACMP_WME_TSPEC_ElementFill */
#endif // CONFIG_STA_SUPPORT_SIM //




/* ====================== Private Function (EDCA) (AP) ====================== */

/*
========================================================================
Routine Description:
	Reclaim a EDCA used ACM time after a actived stream is deleted.

Arguments:
	pAd				- WLAN control block pointer
	*pStream		- the EDCA stream

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_EDCA_AllocatedTimeReturn(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACM_STREAM				*pStream)
{
#define ACM_LMR_TIME_DECREASE(time, value)	\
	if (time >= value) time -= value; else time = 0;
#define ACM_LMR_TIME_INCREASE(time, value)	\
	time += value;

	ACM_CTRL_PARAM *pEdca;
	ACM_TSPEC *pTspec;
	UINT32 TimeUsed, AcId;
	UINT32 Direction;


	/* init */
	pEdca = &ACMR_CB->EdcaCtrlParam;
	pTspec = pStream->pTspec;

	/* check if the stream is EDCA */
	if (pTspec->TsInfo.AccessPolicy != ACM_ACCESS_POLICY_EDCA)
		return;
	/* End of if */

	/* get AC ID */
	AcId = ACM_MR_EDCA_AC(pStream->UP);

	/* reclaim used time */
	TimeUsed = pTspec->MediumTime << 5; /* unit: microsecond */

	if (TimeUsed == 0)
		return;
	/* End of if */

	ACM_LMR_TIME_DECREASE(pEdca->AcmTotalTime, TimeUsed);
	ACM_LMR_TIME_DECREASE(pEdca->AcmAcTime[AcId], TimeUsed);

	Direction = pTspec->TsInfo.Direction;

#ifdef CONFIG_AP_SUPPORT
	if (Direction == ACM_DIRECTION_BIDIREC_LINK)
	{
		/* for bidirectional link, used time = dnlink + uplink;
			so we need to decrease it twice */
		ACM_LMR_TIME_DECREASE(pEdca->AcmTotalTime, TimeUsed);
		ACM_LMR_TIME_DECREASE(pEdca->AcmAcTime[AcId], TimeUsed);
	} /* End of if */
#endif // CONFIG_AP_SUPPORT //

	/* for main link or bidirectional link, we shall reset the CSR */
	if ((pStream->FlgOutLink == 1) ||
		(Direction == ACM_DIRECTION_BIDIREC_LINK))
	{
		ACM_LMR_TIME_DECREASE(pEdca->AcmOutTime[AcId], TimeUsed);

		/* modify CSR setting */
		ACM_ASIC_ACM_Reset(pAd, AcId, pEdca->AcmOutTime[AcId]);
	} /* End of if */

	/* update DATL time */
	if (pEdca->FlgDatl)
		ACM_DATL_Update(pAd, AcId, TimeUsed, 0, ACM_DATL_NO_BORROW, 0);
	/* End of if */

	/* update available ACM time */
	TimeUsed = pEdca->AcmTotalTime;
#ifdef ACM_CC_FUNC_MBSS
	TimeUsed += ACMR_CB->MbssTotalUsedTime;
#endif // ACM_CC_FUNC_MBSS //

#ifdef CONFIG_AP_SUPPORT
	ACMR_AVAIL_ACM_TIME_UPDATE(pAd, TimeUsed);
#endif // CONFIG_AP_SUPPORT //
} /* End of ACM_EDCA_AllocatedTimeReturn */


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Delete a lower priority ACM AC stream to free bandwidth.

Arguments:
	pAd				- WLAN control block pointer
	TimeOff			- the needed free time (us)

Return Value:
	ACM_RTN_OK		- delete successfullly
	ACM_RTN_FAIL	- no any AC stream is deleted

Note:
	Used in QAP.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_EDCA_Lower_UP_Del(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					TimeOff)
{
	ACM_STREAM **ppAcmStmList, *pStream;
	ACM_STREAM *pStreamDel;
	UINT32 DevIndex;
	UCHAR MAC[6];
	UINT32 MediumTime, MediumTimeMax;
	UINT32 IdTidNum;


	/* check whether no any ACM stream exists */
	if (ACMR_CB->EdcaCtrlParam.AcmTotalTime == 0)
		return ACM_RTN_FAIL; /* no ACM AC stream can be deleted */
	/* End of if */

	/* init */
	pStreamDel = NULL;
	DevIndex = 0;
	MediumTimeMax = 0;

	/* try to find a non-bidirectional ACM AC whose medium time > TimeOff */
	/* first, search in client database */
	TimeOff = (TimeOff >> 5) + 1;	/* change to 32 miscroseconds */
									/* 1 is for roundUp */

	while(1)
	{
		/* get a associated STATION MAC from our database */
		if (ACM_PeerDeviceMacGetNext(pAd, &DevIndex, MAC) != ACM_RTN_OK)
			break;
		/* End of if */

		/* get its WMM TS list */
		ppAcmStmList = (ACM_STREAM **)ACM_StationTspecListGet(
											pAd, MAC, ACM_PEER_TSPEC_INPUT_GET);

		/* try to find a TS that its medium time >= TimeOff */
		for(IdTidNum=0; IdTidNum<ACM_STA_TID_MAX_NUM; IdTidNum++)
		{
			pStream = ppAcmStmList[IdTidNum];

			if ((pStream != NULL) &&
				(pStream->pTspec->TsInfo.Direction != \
													ACM_DIRECTION_BIDIREC_LINK))
			{
				MediumTime = pStream->pTspec->MediumTime;

				if (MediumTime >= TimeOff)
				{
					/* find one stream so delete it */
					pStream->Cause = TSPEC_CAUSE_BANDWIDTH;
					ACM_TC_Delete(pAd, pStream);
					return ACM_RTN_OK;
				} /* End of if */

				if (MediumTime > MediumTimeMax)
				{
					/* backup the stream with max medium time */
					MediumTimeMax = MediumTime;
					pStreamDel = pStream;
				} /* End of if */
			} /* End of if */
		} /* End of for */
	} /* End of while */

	if (pStreamDel == NULL)
		return ACM_RTN_FAIL; /* no stream can be deleted */
	/* End of if */

	/* find it and delete the stream */
	pStreamDel->Cause = TSPEC_CAUSE_BANDWIDTH;
	ACM_TC_Delete(pAd, pStreamDel);
	return ACM_RTN_OK;
} /* End of ACM_EDCA_Lower_UP_Del */


/*
========================================================================
Routine Description:
	Handle a EDCA TSPEC request from the QSTA.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the source QSTA
	*pNewStream			- the requested TSPEC
	*pOldStreamIn		- the old same in TSPEC with same AC
	*pOldStreamOut		- the old same out TSPEC with same AC
	*pOldStreamDiffAc	- the old same TSPEC with different AC

Return Value:
	Status Code

Note:
	1. Admission Control Mechanism for EDCA.
========================================================================
*/
STATIC UCHAR ACM_EDCA_ReqHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACM_STREAM				*pNewStream,
	ACM_PARAM_IN	ACM_STREAM				*pOldStreamIn,
	ACM_PARAM_IN	ACM_STREAM				*pOldStreamOut,
	ACM_PARAM_IN	ACM_STREAM				*pOldStreamDiffAc)
{
	/* unit: 100K bps */
	UINT16 RateMapping[ACM_RATE_MAX_NUM][2] = {
			{ 540, ACM_RATE_54M }, { 480, ACM_RATE_48M },
			{ 360, ACM_RATE_36M }, { 240, ACM_RATE_24M },
			{ 180, ACM_RATE_18M }, { 120, ACM_RATE_12M },
			{  90, ACM_RATE_9M  }, {  60, ACM_RATE_6M },
			{ 110, ACM_RATE_11M }, {  55, ACM_RATE_5_5M },
			{  20, ACM_RATE_2M  }, {  10, ACM_RATE_1M } };
	ACM_TSPEC *pTspec;
	ACM_FUNC_STATUS RtnCode;
	UINT32 SBA_Int, SBA_Dec;
	UINT32 PPS;           	/* unit: packet per sec */
	UINT32 MpduExgTime;		/* unit: microseconds */
	UINT32 MediumTime;		/* unit: 32 microseconds */
	UINT32 MediumTimeOld;
	UINT32 AcmTimeOldBi;
	UINT32 AcId;
	UCHAR  Direction;
	UINT16 NormSize;
	UINT32 RateIndex, TxopLimit, TxopLimitDn, TxopLimitUp;
	UCHAR  FlgIsSpreambleUsed, FlgIsCtsEnable, FlgIsRtsEnable, FlgIsGmode;
	UINT32 DatlAcId, DatlBandwidth;
	UINT32 MediumTimeTemp, IdPhyRateNum;


	/* check whether ACM is needed for the AC */
	AcId = ACM_MR_EDCA_AC(pNewStream->UP);

#ifndef ACM_CC_FUNC_SPEC_CHANGE_TG
	if (ACMR_CB->EdcaCtrlParam.FlgAcmStatus[AcId] == ACM_FLG_FUNC_DISABLED)
		return ACM_STATUS_CODE_PRIVATE_ACM_DISABLED; /* no ACM is needed */
	/* End of if */
#endif // ACM_CC_FUNC_SPEC_CHANGE_TG //

	pTspec = pNewStream->pTspec;
	Direction = pTspec->TsInfo.Direction;
	MediumTime = 0;
	MediumTimeOld = 0;
	AcmTimeOldBi = 0;

	/* get minimum TXOP, maybe a packet tx time will excess the TXOP */
	/* currently we do not use the field, TxopLimit */
	TxopLimitDn = ACMR_TXOP_AP_GET(pAd, AcId);
	TxopLimitUp = ACMR_TXOP_BSS_GET(pAd, AcId);


	TxopLimit = 0;


	/* parse the surplus bandwidth allowance */
	/* Surplus Bandwidth Allowance = SBA_Int(3-bit).SBA_Dec(13-bit) */
	SBA_Int = pTspec->SurplusBandwidthAllowance >> ACM_SURPLUS_DEC_BIT_NUM;
	SBA_Dec = pTspec->SurplusBandwidthAllowance & ACM_SURPLUS_DEC_BITMAP;
	SBA_Dec = ACM_SurplusFactorDecimalBin2Dec(SBA_Dec);

	ACMR_DEBUG(ACMR_DEBUG_TRACE,
			("acm_msg> Surplus Bandwidth Allowance = %d.%02d!\n",
			SBA_Int, SBA_Dec));

	/* In WMM ACM Test Plan, the range for SBA should exclude 1.0 and 8.0 */
	if (SBA_Int > ACM_SURPLUS_INT_MAX)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> Surplus Bandwidth Allowance = %d.%d > %d.0!\n",
				SBA_Int, SBA_Dec, ACM_SURPLUS_INT_MAX));
		return ACM_STATUS_CODE_WMM_INVALID_PARAMETERS;
	} /* End of if */

	/* calculate pps */
	/* bit 15 = 1: means fix size; 0: means nominal size */
	/* TODO: if NormSize >> MeanDataRate or maximum WLAN packet size (1548B) ? */
	PPS = 1;
	NormSize = pTspec->NominalMsduSize;

	if (NormSize > 0)
	{
		NormSize = NormSize & 0x7FFF;
		PPS = ((pTspec->MeanDataRate>>3)/NormSize) + 1;
		if (PPS <= 1)
		{
			pTspec->NominalMsduSize = (pTspec->MeanDataRate)>>3;
			NormSize = pTspec->NominalMsduSize;
		} /* End of if */
	} /* End of if */

	/* calculate MPDU exchange time */

	/* check if ACM is enabled, only allocate bandwidth for enabled ACM */
	if (ACMR_CB->EdcaCtrlParam.FlgAcmStatus[AcId] != ACM_FLG_FUNC_DISABLED)
	{
#ifdef ACM_CC_FUNC_11N
		if (pNewStream->PhyModeMin >= ACMR_PHY_HT)
		{
			/* for HT minimum physical rate, includes RTS/CTS time */
			/* TODO: for AMSDU, if no legacy station connects, no RTS/CTS */
			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> minimal MCS = %d, 20/40MHz Flag = %d!\n",
						pNewStream->McsMin, ACMR_IS_2040_STA(pCdb)));

			MpduExgTime = ACM_TX_TimeCalHT(
										pAd,
										pCdb,
										NormSize,
										pNewStream->McsMin,
										ACMR_IS_2040_STA(pCdb),
										0, /* always use regular GI */
										1, /* RTS/CTS */
										0, /* has ACK */
										0, /* no AMPDU */
										TxopLimit,
										NULL, NULL, NULL, NULL, NULL);
		}
		else
#endif // ACM_CC_FUNC_11N //
		{
			/* find the rate ID of the minimum physical rate */
			RateIndex = pTspec->MinPhyRate/100000;
			for(IdPhyRateNum=0; IdPhyRateNum<ACM_RATE_MAX_NUM; IdPhyRateNum++)
			{
				if (RateMapping[IdPhyRateNum][0] == RateIndex)
				{
					RateIndex = RateMapping[IdPhyRateNum][1];
					break;
				} /* End of if */
			} /* End of for */

			if (IdPhyRateNum == ACM_RATE_MAX_NUM)
			{
				/* should not be here so using default 1M rate */
				RateIndex = ACM_RATE_1M;
			} /* End of if */

			/* get CTS-self & B/G mode flag */
			FlgIsCtsEnable = 0; /* no CTS-self function */

			if ((RateIndex == ACM_RATE_1M) || (RateIndex == ACM_RATE_2M) ||
				(RateIndex == ACM_RATE_5_5M) || (RateIndex == ACM_RATE_11M))
			{
				/* CCK rate */
				FlgIsGmode = 0;
			}
			else
			{
				/* OFDM rate */
				FlgIsGmode = 1;
			} /* End of if */

			/* get short or long preamble flag */
			FlgIsSpreambleUsed = ACMR_STA_IS_SPREAMBLE(pAd, pCdb);

			/* get rts/cts enable/disable flag */
			if ((NormSize > ACMR_RTS_THRESH(pAd)) && (FlgIsCtsEnable == 0))
				FlgIsRtsEnable = 1;
			else
				FlgIsRtsEnable = 0;
			/* End of if */

			MpduExgTime = ACM_TX_TimeCal(
										pAd,
										pCdb,
										NormSize,
										RateIndex,
										FlgIsGmode,
										FlgIsCtsEnable,
										FlgIsRtsEnable,
										FlgIsSpreambleUsed,
										pTspec->TsInfo.AckPolicy,
										TxopLimit,
										NULL, NULL, NULL, NULL, NULL);
		} /* End of if */

		/* calculate the medium time, unit: microseconds */
		MediumTime = SBA_Int * PPS * MpduExgTime;

		ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> norm_size = %d, pps = %d, MpduExgTime = %d, medium time = %d!\n",
				NormSize, PPS, MpduExgTime, MediumTime));

		if (SBA_Dec > 0)
		{
			/*
				Avoid MediumTime * SBA_Dec > 0xFFFFFFFF because MediumTime
				is only unsigned (32-bit).
			*/
			MediumTimeTemp = 0xFFFFFFFF/MediumTime;
			if (SBA_Dec <= MediumTimeTemp)
			{
				/* medium time += 0.SBA_Dec * pps * MpduExgTime */
				MediumTime += (SBA_Dec*PPS*MpduExgTime)/ACM_SURPLUS_DEC_BASE;
			} /* End of if */
		} /* End of if */

		MediumTime &= 0xFFFFFFE0; /* unit: microseconds */
		MediumTime += 32; /* extra 32us for roundUp */
		MediumTimeOld = 0;

		/* calculate old used ACM time */
		if (pOldStreamIn != NULL)
			MediumTimeOld += pOldStreamIn->pTspec->MediumTime << 5;
		/* End of if */

		if (pOldStreamOut != NULL)
		{
			MediumTimeOld += pOldStreamOut->pTspec->MediumTime << 5;

			if (Direction == ACM_DIRECTION_BIDIREC_LINK)
				AcmTimeOldBi = pOldStreamOut->pTspec->MediumTime << 5;
			/* End of if */
		} /* End of if */

		if (pOldStreamDiffAc != NULL)
			MediumTimeOld += pOldStreamDiffAc->pTspec->MediumTime << 5;
		/* End of if */

		/* check whether current bandwidth is enough */
		RtnCode = ACM_BandwidthCheck(
									pAd,
									AcId,
									0,
									pTspec->TsInfo.AccessPolicy,
									Direction,
									MediumTimeOld,	/* old used time */
									MediumTime,		/* new used time */
									AcmTimeOldBi,	/* old used time */
									NULL,
									&DatlAcId,
									&DatlBandwidth);

		if (RtnCode != ACM_RTN_OK)
			return ACM_STATUS_CODE_ASSOC_DENIED_INSUFFICIENT_BANDWIDTH;
		/* End of if */
	} /* End of if */

	/* the new stream is allowed */
	pTspec->MediumTime = MediumTime>>5; /* transfer to unit: 32us */

	pNewStream->Status = TSPEC_STATUS_ACTIVE;
	pNewStream->InactivityCur = pTspec->InactivityInt;
#ifdef ACM_CC_FUNC_HCCA
	pNewStream->SuspensionCur = pTspec->SuspensionInt;
#else
	pNewStream->SuspensionCur = 0;
#endif // ACM_CC_FUNC_HCCA //

	/* free old TSPEC */
	if (pOldStreamIn != NULL)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> Free in stream!\n"));
		ACM_TC_Discard(pAd, pOldStreamIn);
	} /* End of if */

	if (pOldStreamOut != NULL)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> Free out stream!\n"));
		ACM_TC_Discard(pAd, pOldStreamOut);
	} /* End of if */

	if (pOldStreamDiffAc != NULL)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE, ("acm_msg> Free dif stream!\n"));
		ACM_TC_Discard(pAd, pOldStreamDiffAc);
	} /* End of if */

	/* active/add the new TSPEC */
	if (ACM_TC_Active(pAd, pNewStream) == ACM_RTN_OK)
	{
		if (MediumTime > 0)
		{
			if (ACMR_CB->EdcaCtrlParam.FlgDatl != 0)
			{
				if (MediumTime <= MediumTimeOld)
				{
					/*
						Get the needed extra bandwidth for new medium time
						because we will not handle the case above, we need to
						handle the case here before ACM_EDCA_Param_ACM_Update()
					*/
					ACM_DATL_Handle(pAd, AcId, 0, MediumTime,
									&DatlAcId, &DatlBandwidth);
				} /* End of if */
			} /* End of if */

			/* update used time */
			ACM_EDCA_Param_ACM_Update(
									pAd,
									pNewStream->AcmAcId,
									Direction,
									pNewStream->UP,
									MediumTime,
									0,
									DatlAcId,
									DatlBandwidth);
		} /* End of if */

		RtnCode = ACM_STATUS_CODE_SUCCESS;
	}
	else
		RtnCode = ACM_STATUS_CODE_UNSPECIFIED_FAILURE;
	/* End of if */

	/* update to CSR */
	if ((Direction == ACM_DIRECTION_DOWN_LINK) ||
		(Direction == ACM_DIRECTION_BIDIREC_LINK))
	{
		/*
			When the Direction is donwlink or bidirectional link,
			we maybe reset ASIC ACM control registers.

			Currently no any related register is needed to re-set.
		*/
		AcId = pNewStream->AcmAcId;

		ACM_ASIC_ACM_Reset(
						pAd,
						AcId,
						ACMR_CB->EdcaCtrlParam.AcmOutTime[AcId]);
	} /* End of if */

	return RtnCode;
} /* End of ACM_EDCA_ReqHandle */
#endif // CONFIG_AP_SUPPORT //


/*
========================================================================
Routine Description:
	Update new ACM medium time for EDCA mechanism.

Arguments:
	pAd				- WLAN control block pointer
	AcmAcId			- the AC for the stream (0 ~ 3)
	Direction		- the Direction of the stream
	UP				- the user priority
	AcmTimeNew		- new medium time of the stream (unit: microseconds)
	AcmTimeOld		- old medium time of the stream (unit: microseconds)
	DatlAcId		- the borrowed AC ID, 0 ~ 3
	DatlBandwidth	- the borrowed bandwidth from a AC (unit: microsecond)

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_EDCA_Param_ACM_Update(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				AcmAcId,
	ACM_PARAM_IN	UCHAR				Direction,
	ACM_PARAM_IN	UCHAR				UP,
	ACM_PARAM_IN	UINT32				AcmTimeNew,
	ACM_PARAM_IN	UINT32				AcmTimeOld,
	ACM_PARAM_IN	UINT32				DatlAcId,
	ACM_PARAM_IN	UINT32				DatlBandwidth)
{
	ACM_CTRL_PARAM  *pEdcaParam;
	UINT32 TimeNewDn, TimeOldDn;
	UINT32 TimeNewUp, TimeOldUp;
	UINT32 TimeUsed;
	UINT32 AcId;


	/* init */
	pEdcaParam = &(ACMR_CB->EdcaCtrlParam);
	TimeNewDn = 0;
	TimeOldDn = 0;
	TimeNewUp = 0;
	TimeOldUp = 0;
	AcId = AcmAcId;

	switch(Direction)
	{
		case ACM_DIRECTION_UP_LINK: /* uplink */
		case ACM_DIRECTION_DIRECT_LINK:
			TimeNewUp = AcmTimeNew;
			TimeOldUp = AcmTimeOld;

#ifdef CONFIG_AP_SUPPORT
			/* for uplink in QAP, AcmAcId is not AC ID; it is minor link;
				so we need to re-get AC ID from UP of the uplink */
			AcId = ACM_MR_EDCA_AC(UP);
#endif // CONFIG_AP_SUPPORT //
			break;

		case ACM_DIRECTION_DOWN_LINK: /* dnlink */
			TimeNewDn = AcmTimeNew;
			TimeOldDn = AcmTimeOld;

			break;

		case ACM_DIRECTION_BIDIREC_LINK: /* dnlink + uplink */
			TimeNewDn = TimeNewUp = AcmTimeNew;
			TimeOldDn = TimeOldUp = AcmTimeOld;
			break;
	} /* End of switch */

	if ((TimeNewDn == TimeOldDn) && (TimeNewUp == TimeOldUp))
		return; /* same time, do NOT need to update */
	/* End of if */


	/* accumulate new allocated time */
#ifdef CONFIG_AP_SUPPORT
	pEdcaParam->AcmTotalTime += TimeNewDn + TimeNewUp;
	pEdcaParam->AcmOutTime[AcId] += TimeNewDn;
	pEdcaParam->AcmAcTime[AcId] += TimeNewDn + TimeNewUp;
#endif // CONFIG_AP_SUPPORT //


	/* substract old medium time from total ACM time parameters */
#ifdef CONFIG_AP_SUPPORT
	if (pEdcaParam->AcmTotalTime >= (TimeOldDn + TimeOldUp))
		pEdcaParam->AcmTotalTime -= (TimeOldDn + TimeOldUp);
#endif // CONFIG_AP_SUPPORT //
	else
	{
		pEdcaParam->AcmTotalTime = 0; /* fatal error */

		ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Used total ACM time < stream medium time! "
				"EDCA_Param_ACM_Update()\n"));
	} /* End of if */

#ifdef CONFIG_AP_SUPPORT
	if (pEdcaParam->AcmOutTime[AcId] >= TimeOldDn)
		pEdcaParam->AcmOutTime[AcId] -= TimeOldDn;
#endif // CONFIG_AP_SUPPORT //
	else
	{
		pEdcaParam->AcmOutTime[AcId] = 0; /* fatal error */

		ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Used ACM time < stream medium time! "
				"EDCA_Param_ACM_Update()\n"));
	} /* End of if */


	/* update the ACM used time of the AC */
#ifdef CONFIG_AP_SUPPORT
	if (pEdcaParam->AcmAcTime[AcId] >= (TimeOldDn + TimeOldUp))
		pEdcaParam->AcmAcTime[AcId] -= (TimeOldDn + TimeOldUp);
#endif // CONFIG_AP_SUPPORT //
	else
	{
		pEdcaParam->AcmAcTime[AcId] = 0; /* fatal error */

		ACMR_DEBUG(ACMR_DEBUG_ERR,
				("acm_err> Used AC ACM time < stream medium time! "
				"EDCA_Param_ACM_Update()\n"));
	} /* End of if */


	/* update DATL time only in QAP */
	if (pEdcaParam->FlgDatl)
	{
		ACM_DATL_Update(pAd, AcId, AcmTimeOld, AcmTimeNew,
						DatlAcId, DatlBandwidth);
	} /* End of if */


	/* update available ACM time */
	TimeUsed = pEdcaParam->AcmTotalTime;
#ifdef ACM_CC_FUNC_MBSS
	TimeUsed += ACMR_CB->MbssTotalUsedTime;
#endif // ACM_CC_FUNC_MBSS //

#ifdef CONFIG_AP_SUPPORT
	ACMR_AVAIL_ACM_TIME_UPDATE(pAd, TimeUsed);
#endif // CONFIG_AP_SUPPORT //
} /* End of ACM_EDCA_Param_ACM_Update */




/* ====================== Private Function (WMM) =========================== */

#ifdef ACM_CC_FUNC_WMM

/*
========================================================================
Routine Description:
	Translate 11e status code to WME status code.

Arguments:
	StatusCode		- 11e status code

Return Value:
	WME status code

Note:
	Only 3 status code for WMM ACM.

	WLAN_STATUS_CODE_WME_INVALID_PARAM	- invalid TSPEC parameters
	WLAN_STATUS_CODE_WME_ACM_ACCEPTED	- accept
	WLAN_STATUS_CODE_WME_REFUSED		- refuse due to insufficient BW
========================================================================
*/
STATIC UCHAR ACM_11E_WMM_StatusTranslate(
	ACM_PARAM_IN	UCHAR		StatusCode)
{
	ACMR_DEBUG(ACMR_DEBUG_TRACE,
				("acm_msg> 11e status code = %d\n", StatusCode));

	if (StatusCode == ACM_STATUS_CODE_INVALID_PARAMETERS)
		return WLAN_STATUS_CODE_WME_INVALID_PARAM;
	/* End of if */

	if (StatusCode == ACM_STATUS_CODE_SUCCESS)
		return WLAN_STATUS_CODE_WME_ACM_ACCEPTED;
	/* End of if */

	return WLAN_STATUS_CODE_WME_REFUSED;
} /* End of ACM_11E_WMM_StatusTranslate */



/*
========================================================================
Routine Description:
	Translate WME TSPEC & TCLAS to 11e TSPEC & TCLAS.

Arguments:
	*pPktElm			- the TSPEC related element in the packet buffer
	BodyLen				- the action frame length
	*pETspec			- the 11e TSPEC
	*pTclas				- the 11e TCLAS
	*pTclasNum			- the number of TCLAS
	*pTclasProcessing	- the TCLAS PROCESSING value

Return Value:
	ACM_RTN_OK			- translate ok
	ACM_RTN_FAIL		- translate fail

Note:
	Internally we use 11e TSPEC, not WMM TSPEC.
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_WME_11E_TSPEC_TCLAS_Translate(
	ACM_PARAM_IN	UCHAR					*pPktElm,
	ACM_PARAM_IN	UINT32					BodyLen,
	ACM_PARAM_IN	ACM_TSPEC				*pETspec,
	ACM_PARAM_IN	ACM_TCLAS				**pTclas,
	ACM_PARAM_IN	UINT32					*pTclasNum,
	ACM_PARAM_IN	UCHAR					*pTclasProcessing)
{
	ACM_WME_TSPEC *pTspec;
	ACM_ELM_WME_TCLAS *pElmTclas;
	ACM_ELM_WME_TCLAS_PROCESSING *pElmTclasProcessing;
	UCHAR *pElm;
	UCHAR ElmID, ElmLen, ElmSubID;
	UCHAR TclasType;
	UINT32 IdTclasNum;


	/* init */
	pTspec = NULL;

	for(IdTclasNum=0; IdTclasNum<ACM_TSPEC_TCLAS_MAX_NUM; IdTclasNum++)
		pTclas[IdTclasNum] = NULL;
	/* End of for */

	pElm = (UCHAR *)pPktElm;

	*pTclasNum = 0;
	*pTclasProcessing = ACM_TCLAS_PROCESSING_NOT_EXIST;

	BodyLen -= 4; /* skip Category, action, DialogToken, & StatusCode */

	/* parsing TSPEC, TCLASS, & TCLASS Processing elements */
	while(BodyLen > 0)
	{
		ElmID = *pElm;
		ElmLen = *(pElm+1);

		if (BodyLen < (UINT32)(ACM_ELM_ID_LEN_SIZE+ElmLen))
		{
			/* fatal error, packet size is not enough */
			ACMR_DEBUG(ACMR_DEBUG_ERR,
						("acm_err> packet length %d is too small %d! "
						"WME_11E_TSPEC_TCLAS_Translate()\n",
						BodyLen, (ACM_ELM_ID_LEN_SIZE+ElmLen)));
			goto LabelParseErr;
		} /* End of if */

		/* not check *(pElm+1) = element length and
			not check *(pElm+6) = WMM sub element ID */
		if ((ElmID != ACM_ELM_WME_ID) ||
			(*(pElm+2) != ACM_WME_OUI_0) ||
			(*(pElm+3) != ACM_WME_OUI_1) ||
			(*(pElm+4) != ACM_WME_OUI_2) ||
			(*(pElm+5) != ACM_WME_OUI_TYPE) ||
			(*(pElm+7) != ACM_WME_OUI_VERSION))
		{
			/* not WMM element so check next element */
			pElm += (ACM_ELM_ID_LEN_SIZE+ElmLen);
			BodyLen -= (ACM_ELM_ID_LEN_SIZE+ElmLen);
			continue;
		} /* End of if */

		ElmSubID = *(pElm+ACM_WME_OUI_ID_OFFSET);

		switch(ElmSubID)
		{
			case ACM_WME_OUI_SUBTYPE_TSPEC: /* TSPEC element */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> find a WMM TSPEC element! "
							"WME_11E_TSPEC_TCLAS_Translate()\n"));

				pTspec = &((ACM_ELM_WME_TSPEC *)pElm)->Tspec;

				if (ACM_WME_11E_TSPEC_Translate(pTspec,
												pETspec) != ACM_RTN_OK)
				{
					goto LabelParseErr;
				} /* End of if */
				break;

			case ACM_WSM_OUI_SUBTYPE_TCLAS: /* TCLASS element */
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> find a WMM TCLAS element! "
							"WME_11E_TSPEC_TCLAS_Translate()\n"));

				/* sanity check for TCLAS number & element length */
				if ((*pTclasNum) >= ACM_TCLAS_MAX_NUM)
					goto LabelParseErr;
				/* End of if */

				/* skip element id/len, OUI header, user priority */
				TclasType = *(pElm+2+ACM_WME_OUI_HDR_LEN+1);

				switch(TclasType)
				{
					case ACM_TCLAS_TYPE_ETHERNET:
						if (ElmLen != ACM_TCLAS_TYPE_WME_ETHERNET_LEN)
							goto LabelParseErr;
						/* End of if */
						break;

					case ACM_TCLAS_TYPE_IP_V4:
						if (ElmLen != ACM_TCLAS_TYPE_WME_IP_V4_LEN)
							goto LabelParseErr;
						/* End of if */
						break;

					case ACM_TCLAS_TYPE_8021DQ:
						if (ElmLen != ACM_TCLAS_TYPE_WME_8021DQ_LEN)
							goto LabelParseErr;
						/* End of if */
						break;

					default:
						goto LabelParseErr;
				} /* End of switch */

				pElmTclas = (ACM_ELM_WME_TCLAS *)pElm;
				pTclas[(*pTclasNum)++] = &pElmTclas->Tclas;
				break;

			case ACM_WSM_OUI_SUBTYPE_TCLAS_PROCESSING: /* TCLASS Processing */
				if (ElmLen != ACM_ELM_WME_TCLAS_PROCESSING_LEN)
					goto LabelParseErr;
				/* End of if */

				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> find a WMM TCLAS PROCESSING element! "
							"WME_11E_TSPEC_TCLAS_Translate()\n"));

				pElmTclasProcessing = (ACM_ELM_WME_TCLAS_PROCESSING *)pElm;
				*pTclasProcessing = pElmTclasProcessing->Processing;
				break;
		} /* End of switch */

		/* check next element */
		pElm += (2+ElmLen); /* 2: Element ID & Element Length */
		BodyLen -= (2+ElmLen); /* 2: Element ID & Element Length */
	} /* End of while */

	return ACM_RTN_OK;

LabelParseErr:
	return ACM_RTN_FAIL;
} /* End of ACM_WME_11E_TSPEC_TCLAS_Translate */


/*
========================================================================
Routine Description:
	Translate WME TSPEC to 11e TSPEC.

Arguments:
	*pWTspec		- the 'W'ME TSPEC
	*pETspec		- the 11'e' TSPEC

Return Value:
	ACM_RTN_OK		- translate ok
	ACM_RTN_FAIL	- translate fail

Note:
========================================================================
*/
STATIC ACM_FUNC_STATUS ACM_WME_11E_TSPEC_Translate(
	ACM_PARAM_IN	ACM_WME_TSPEC			*pWTspec,
	ACM_PARAM_IN	ACM_TSPEC				*pETspec)
{
	ACM_TS_INFO *pInfo;


	/* init */
	pInfo = &pETspec->TsInfo;

	/* translate WMM TSPEC to 11e TSPEC */
	ACMR_MEM_ZERO((UCHAR *)pETspec, sizeof(ACM_TSPEC));

	/* init TS Info field */
	pInfo->TrafficType = pWTspec->TsInfo.Reserved4;
	pInfo->TSID = pWTspec->TsInfo.TID;

	pInfo->Direction = pWTspec->TsInfo.Direction;
	pInfo->AccessPolicy = pWTspec->TsInfo.One;
	pInfo->Aggregation = pWTspec->TsInfo.Zero1;
	pInfo->APSD = pWTspec->TsInfo.PSB;
	pInfo->UP = pWTspec->TsInfo.UP;
	pInfo->AckPolicy = pWTspec->TsInfo.Reserved2;

	/* in WMM ACM TG, we need to check bit16 ~ 23 and bit8 == 0,
		and we will not	use schedule field, so we set (bit16 ~ 23 | bit8)
		to the field */
	pInfo->Schedule = pWTspec->TsInfo.Reserved1 |
						pWTspec->TsInfo.Reserved3;

	/* init TSPEC parameters */
	pETspec->NominalMsduSize = pWTspec->NominalMsduSize;
	pETspec->MaxMsduSize = pWTspec->MaxMsduSize;
	pETspec->MinServInt = pWTspec->MinServInt;
	pETspec->MaxServInt = pWTspec->MaxServInt;
	if (pWTspec->InactivityInt == 0)
	{
		/* can not be 0 so use default timeout */
		pETspec->InactivityInt = ACM_WME_TSPEC_INACTIVITY_DEFAULT;
	}
	else
		pETspec->InactivityInt = pWTspec->InactivityInt;
	/* End of if */

	pETspec->SuspensionInt = pWTspec->SuspensionInt;
	pETspec->ServiceStartTime = pWTspec->ServiceStartTime;
	pETspec->MinDataRate = pWTspec->MinDataRate;
	pETspec->MeanDataRate = pWTspec->MeanDataRate;
	pETspec->PeakDataRate = pWTspec->PeakDataRate;

	/* if you want to issue NULL TSPEC, Min = Mean = Peak = 0 */
	if (pETspec->MeanDataRate == 0)
	{
		if (pETspec->PeakDataRate != 0)
			pETspec->MeanDataRate = pETspec->PeakDataRate;
		else
		{
			if (pETspec->MinDataRate != 0)
				pETspec->MeanDataRate = pETspec->MinDataRate;
			/* End of if */
		} /* End of if */
	} /* End of if */

	pETspec->MaxBurstSize = pWTspec->MaxBurstSize;
	pETspec->DelayBound = pWTspec->DelayBound;
	pETspec->MinPhyRate = pWTspec->MinPhyRate;
	pETspec->SurplusBandwidthAllowance = \
										pWTspec->SurplusBandwidthAllowance;
	pETspec->MediumTime = pWTspec->MediumTime;
	return ACM_RTN_OK;
} /* End of ACM_WME_11E_TSPEC_Translate */


/*
========================================================================
Routine Description:
	Make a WME action frame body.

Arguments:
	pAd				- WLAN control block pointer
	*pStream		- the stream
	*pPkt			- the frame body pointer
	Action			- action
	StatusCode		- status code, used when action = response

Return Value:
	ACM_RTN_OK		- insert ok
	ACM_RTN_FAIL	- insert fail

Note:
========================================================================
*/
STATIC UINT32 ACM_WME_ActionFrameBodyMake(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream,
	ACM_PARAM_IN	UCHAR				*pPkt,
	ACM_PARAM_IN	UCHAR				Action,
	ACM_PARAM_IN	UCHAR				StatusCode)
{
	ACM_WME_NOT_FRAME *pFrameBody;
	ACM_ELM_WME_TSPEC *pElmTspec;
	ACM_ELM_WME_TCLAS_PROCESSING *pElmTspecProcessing;
	ACM_WME_TS_INFO *pInfo;
	ACM_WME_TSPEC *pTspec;
	UCHAR *pElmTclas;
	UINT32 BodyLen, Len;
	UINT32 IdTclasNum;


	/* sanity check for type */
	if (Action > ACM_ACTION_WME_TEAR_DOWN)
		return 0;
	/* End of if */

	/* init frame body */
	pFrameBody = (ACM_WME_NOT_FRAME *)pPkt;
	pFrameBody->Category = ACM_CATEGORY_WME;
	pFrameBody->Action   = Action;

	if (Action != ACM_ACTION_WME_TEAR_DOWN)
		pFrameBody->DialogToken = pStream->DialogToken;
	else
		pFrameBody->DialogToken = 0; /* always 0 for DELTS */
	/* End of if */

	pFrameBody->StatusCode = StatusCode;
	BodyLen = 4;

	/* TSPEC element */
	pElmTspec = &pFrameBody->ElmTspec;
	pElmTspec->ElementId = ACM_ELM_WME_ID;
	pElmTspec->Length = ACM_ELM_WME_TSPEC_LEN;

	/* init OUI field */
	pElmTspec->OUI[0] = ACM_WME_OUI_0;
	pElmTspec->OUI[1] = ACM_WME_OUI_1;
	pElmTspec->OUI[2] = ACM_WME_OUI_2;
	pElmTspec->OUI_Type = ACM_WME_OUI_TYPE;
	pElmTspec->OUI_SubType = ACM_WME_OUI_SUBTYPE_TSPEC;
	pElmTspec->Version = ACM_WME_OUI_VERSION;

	/* init TS Info field */
	pTspec = &pFrameBody->ElmTspec.Tspec;
	ACMR_MEM_ZERO(pTspec, sizeof(ACM_WME_TSPEC));
	pInfo = &pFrameBody->ElmTspec.Tspec.TsInfo;

	pInfo->TID = pStream->pTspec->TsInfo.TSID;
	pInfo->Direction = pStream->pTspec->TsInfo.Direction;
	pInfo->UP = pStream->pTspec->TsInfo.UP;
	pInfo->PSB = pStream->pTspec->TsInfo.APSD;
	pInfo->One = 1; /* always 1 */

#ifdef ACM_CC_FUNC_11N
	pInfo->Reserved2 = pStream->pTspec->TsInfo.AckPolicy;
#endif // ACM_CC_FUNC_11N //

	/* init TSPEC parameters */
	pTspec->NominalMsduSize = pStream->pTspec->NominalMsduSize;
	pTspec->MaxMsduSize = pStream->pTspec->MaxMsduSize;
	pTspec->MinServInt = pStream->pTspec->MinServInt;
	pTspec->MaxServInt = pStream->pTspec->MaxServInt;
	pTspec->InactivityInt = pStream->pTspec->InactivityInt;
	pTspec->SuspensionInt = pStream->pTspec->SuspensionInt;
	pTspec->ServiceStartTime = pStream->pTspec->ServiceStartTime;
	pTspec->MinDataRate = pStream->pTspec->MinDataRate;
	pTspec->MeanDataRate = pStream->pTspec->MeanDataRate;
	pTspec->PeakDataRate = pStream->pTspec->PeakDataRate;
	pTspec->MaxBurstSize = pStream->pTspec->MaxBurstSize;
	pTspec->DelayBound = pStream->pTspec->DelayBound;
	pTspec->MinPhyRate = pStream->pTspec->MinPhyRate;
	pTspec->SurplusBandwidthAllowance = \
								pStream->pTspec->SurplusBandwidthAllowance;

	if (pTspec->TsInfo.Direction != ACM_DIRECTION_DOWN_LINK)
	{
		/* we need to fill medium time if the link is not downlink-only */
		pTspec->MediumTime = pStream->pTspec->MediumTime;
	} /* End of if */

	BodyLen += (ACM_ELM_ID_LEN_SIZE+pElmTspec->Length);

	/* TCLASS element */
	pElmTclas = pFrameBody->Tclas;

	for(IdTclasNum=0; IdTclasNum<ACM_TSPEC_TCLAS_MAX_NUM; IdTclasNum++)
	{
		if (pStream->pTclas[IdTclasNum] != NULL)
		{
			*pElmTclas++ = ACM_ELM_WME_ID;
			Len = ACM_TCLAS_LEN_GET(pStream->pTclas[IdTclasNum]->ClassifierType);
			*pElmTclas++ = Len;

			*pElmTclas++ = ACM_WME_OUI_0;
			*pElmTclas++ = ACM_WME_OUI_1;
			*pElmTclas++ = ACM_WME_OUI_2;
			*pElmTclas++ = ACM_WME_OUI_TYPE;
			*pElmTclas++ = ACM_WSM_OUI_SUBTYPE_TCLAS;
			*pElmTclas++ = ACM_WME_OUI_VERSION;

			ACMR_MEM_COPY(pElmTclas,
						pStream->pTclas[IdTclasNum],
						Len-ACM_WME_OUI_HDR_LEN);

			pElmTclas += (Len-ACM_WME_OUI_HDR_LEN);

			BodyLen += (ACM_ELM_ID_LEN_SIZE+Len);
			continue; /* check next TCLAS */
		} /* End of if */

		break; /* no more TCLAS exists */
	} /* End of for */

	/* TCLASS Processing element */
	if (pStream->pTclas[0] != NULL)
	{
		/*
			TCLAS PROCESSING element exists only when at least one TCLAS
			element exists.
		*/
		if (pStream->TclasProcessing != ACM_TCLAS_PROCESSING_NOT_EXIST)
		{
			pElmTspecProcessing = (ACM_ELM_WME_TCLAS_PROCESSING *)pElmTclas;
			BodyLen += (ACM_ELM_ID_LEN_SIZE+ACM_ELM_WME_TCLAS_PROCESSING_LEN);

			pElmTspecProcessing->ElementId = ACM_ELM_WME_ID;
			pElmTspecProcessing->Length = ACM_ELM_WME_TCLAS_PROCESSING_LEN;

			pElmTspecProcessing->OUI[0] = ACM_WME_OUI_0;
			pElmTspecProcessing->OUI[1] = ACM_WME_OUI_1;
			pElmTspecProcessing->OUI[2] = ACM_WME_OUI_2;
			pElmTspecProcessing->OUI_Type = ACM_WME_OUI_TYPE;
			pElmTspecProcessing->OUI_SubType = ACM_WSM_OUI_SUBTYPE_TCLAS_PROCESSING;
			pElmTspecProcessing->Version = ACM_WME_OUI_VERSION;

			pElmTspecProcessing->Processing = pStream->TclasProcessing;
		} /* End of if */
	} /* End of if */

	return BodyLen;
} /* End of ACM_WME_ActionFrameBodyMake */


/*
========================================================================
Routine Description:
	Handle a WME action frame.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	*pFrameBody		- the action frame body
	BodyLen			- the length of action frame body
	PhyRate			- the physical tx rate for the frame, bps
	Action			- Setup request, response, or teardown
	*pStatusCode	- response status code
	*pMediumTime	- the allowed medium time

Return Value:
	None

Note:
========================================================================
*/
STATIC VOID ACM_WME_ActionHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pFrameBody,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				PhyRate,
	ACM_PARAM_IN	UCHAR				Action,
	ACM_PARAM_OUT	UCHAR				*pStatusCode,
	ACM_PARAM_OUT	UINT16				*pMediumTime)
{
	ACM_WME_NOT_FRAME *pNotFrame;
	ACM_TCLAS *pTclas[ACM_TSPEC_TCLAS_MAX_NUM];
	ACM_TSPEC Tspec;
	UINT32 TclasNum;
	UCHAR TclasProcessing;
	UCHAR StatusCode;
	ACM_FUNC_STATUS RtnCode;


	/* init */
	pNotFrame = (ACM_WME_NOT_FRAME *)pFrameBody;
	TclasNum = 0;
	TclasProcessing = ACM_TCLAS_PROCESSING_NOT_EXIST;
	StatusCode = ACM_STATUS_CODE_SUCCESS;

	/* sanity check for input parameters */
	if (Action > ACM_ACTION_WME_TEAR_DOWN)
	{
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> Error action type = %d! "
					"WME_ActionHandle()\n", Action));
		return;
	} /* End of if */

	if (ACM_WME_ELM_Check((UCHAR *)&pNotFrame->ElmTspec,
							ACM_WME_OUI_SUBTYPE_TSPEC) != ACM_RTN_OK)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Element check error! "
					"WME_ActionHandle()\n"));
		return; /* TSPEC element error */
	} /* End of if */

	if (BodyLen < ACM_NOT_FRAME_BODY_LEN)
	{
		ACMR_DEBUG(ACMR_DEBUG_TRACE,
					("acm_msg> Frame length is not enough! "
					"WME_ActionHandle()\n"));
		return; /* error! < minimum action frame length */
	} /* End of if */

	/* translate WME TSPEC to 11e TSPEC */
	if (ACM_WME_11E_TSPEC_TCLAS_Translate(
										(UCHAR *)&pNotFrame->ElmTspec,
										BodyLen,
										&Tspec,
										pTclas,
										&TclasNum,
										&TclasProcessing) != ACM_RTN_OK)
	{
		ACMR_DEBUG(ACMR_DEBUG_ERR,
					("acm_err> Translate TSPEC fail! "
					"WME_ActionHandle()\n"));
		return; /* translate fail */
	} /* End of if */

	/* handle it by action */
	switch(Action)
	{
#ifdef CONFIG_AP_SUPPORT
		case ACM_ACTION_WME_SETUP_REQ:
			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> A WME Request Frame is RCV! "
						"WME_ActionHandle()\n"));

			RtnCode = ACM_TC_ReqHandle(
									pAd, pCdb, ACM_STREAM_TYPE_WIFI,
									pNotFrame->DialogToken, &Tspec,
									TclasNum, pTclas,
									TclasProcessing,
									PhyRate, &StatusCode, pMediumTime);

			if (RtnCode != ACM_RTN_OK)
			{
				ACMR_DEBUG(ACMR_DEBUG_TRACE,
							("acm_msg> A WME Setup request is not allowed %d! "
							"WME_ActionHandle()\n", RtnCode));
			} /* End of if */
			break;
#endif // CONFIG_AP_SUPPORT //


		case ACM_ACTION_WME_TEAR_DOWN:
			ACMR_DEBUG(ACMR_DEBUG_TRACE,
						("acm_msg> A WME Tear down is RCV! "
						"DEL the stream! WME_ActionHandle()\n"));

			ACM_TC_DestroyBy_TS_Info(
									pAd,
									ACMR_CLIENT_MAC(pCdb),
									&Tspec.TsInfo,
									ACMR_IS_AP_MODE);
			break;

		default:
			/* should not be here */
			break;
	} /* End of switch */

	/* upadte status code */
	*pStatusCode = StatusCode;
} /* End of ACM_WME_ActionHandle */




/* ====================== Private Function (WMM) (AP) ====================== */

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

#endif // ACM_CC_FUNC_WMM //

#endif // WMM_ACM_SUPPORT //

/* End of acm_edca.c */

