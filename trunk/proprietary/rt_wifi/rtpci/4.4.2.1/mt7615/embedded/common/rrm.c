/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifdef DOT11K_RRM_SUPPORT

#include "rt_config.h"

/*
static CHAR ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
*/


static VOID RRM_QuietOffsetTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

static VOID RRM_QuietTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(RRM_QuietOffsetTimeout);
DECLARE_TIMER_FUNCTION(RRM_QuietTimeout);

BUILD_TIMER_FUNCTION(RRM_QuietOffsetTimeout);
BUILD_TIMER_FUNCTION(RRM_QuietTimeout);


static VOID RRM_QuietOffsetTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	INT idx;
	PRRM_CONFIG pRrmCfg = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
		if (&pRrmCfg->QuietCB.QuietOffsetTimer == pTimer)
			break;
	}

	/* start Quiet . */
	if (idx == pAd->ApCfg.BssidNum)
		return;

	pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
	pRrmCfg->QuietCB.QuietState = RRM_QUIET_SILENT;
	RTMPSetTimer(&pRrmCfg->QuietCB.QuietTimer,
		pRrmCfg->QuietCB.QuietDuration);

	return;
}

static VOID RRM_QuietTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	INT idx;
	PRRM_CONFIG pRrmCfg = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
		if (&pRrmCfg->QuietCB.QuietTimer == pTimer)
			break;
	}

	/* stop Quiet . */
	if (idx == pAd->ApCfg.BssidNum)
		return;

	pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
	pRrmCfg->QuietCB.QuietState = RRM_QUIET_IDLE;

	return;
}

void RRM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT loop;
	RTMP_STRING *macptr;

	/* RRMEnable */
	if (RTMPGetKeyParameter("RRMEnable", tmpbuf, 255, buffer, TRUE))
	{
		for (loop=0, macptr = rstrtok(tmpbuf,";");
				(macptr && loop < MAX_MBSSID_NUM(pAd));
					macptr = rstrtok(NULL,";"), loop++)
		{
			LONG Enable;
			Enable = simple_strtol(macptr, 0, 10);
			pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable =
				(Enable > 0) ? TRUE : FALSE;

			pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnableSet = TRUE;
			
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s::(bDot11kRRMEnable[%d]=%d)\n",
				__FUNCTION__, loop,
				pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable));
		}
	}
	else 
	{
		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
			pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable = FALSE;
	}

	return;
}


INT Set_Dot11kRRM_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	ULONG Value;

	if (ifIndex >= pAd->ApCfg.BssidNum)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Unknow If index (%d)", ifIndex));
		return -1;
	}

	Value = (UINT) simple_strtol(arg, 0, 10);
	pAd->ApCfg.MBSSID[ifIndex].RrmCfg.bDot11kRRMEnable =
		(BOOLEAN)(Value) == 0 ? FALSE : TRUE;

	return 1;
}

INT Set_BeaconReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Loop;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UINT Aid = 1;
	UINT ArgIdx;
	RTMP_STRING *thisChar;

	RRM_MLME_BCN_REQ_INFO BcnReq;

	ArgIdx = 0;
	NdisZeroMemory(&BcnReq, sizeof(RRM_MLME_BCN_REQ_INFO));

	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		switch(ArgIdx)
		{
			case 0:	/* Aid. */
				Aid = (UINT8) simple_strtol(thisChar, 0, 16);
				if (!VALID_WCID(Aid))
				{
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s: unknow sta of Aid(%d)\n", __FUNCTION__, Aid));
					return TRUE;
				}
				break;

			case 1: /* Meausre Duration. */
				BcnReq.MeasureDuration = (UINT8) simple_strtol(thisChar, 0, 10);

			case 2: /* Regulator Class */
				BcnReq.RegulatoryClass = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 3: /* BSSID */
				if(strlen(thisChar) != 17)
				{
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
						("%s: invalid value BSSID.\n", 	__FUNCTION__));
					return TRUE;
				}

				if(strlen(thisChar) == 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
				{
					RTMP_STRING *value;
					for (Loop=0, value = rstrtok(thisChar,":"); value; value = rstrtok(NULL,":"))
					{
						if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
							return FALSE;  /*Invalid */

						AtoH(value, &BcnReq.Bssid[Loop++], 1);
					}

					if(Loop != 6)
						return TRUE;
				}
				break;

			case 4: /* SSID */
				BcnReq.pSsid = (PUINT8)thisChar;
				BcnReq.SsidLen = strlen(thisChar);
				break;

			case 5: /* measure channel */
				BcnReq.MeasureCh = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 6: /* measure mode. */
				BcnReq.MeasureMode = (UINT8) simple_strtol(thisChar, 0, 10);
				if (BcnReq.MeasureMode > RRM_BCN_REQ_MODE_BCNTAB)
				{
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
						("%s: invalid Measure Mode. %d\n", 	__FUNCTION__, BcnReq.MeasureMode));
					return TRUE;
				}
				break;
			case 7: /* regulatory class. */
				{
					RTMP_STRING *RegClassString;
					int RegClassIdx;

					RegClassIdx = 0;
					while ((RegClassString = strsep((char **)&thisChar, "+")) != NULL)
					{
						BcnReq.ChRepRegulatoryClass[RegClassIdx] =
							(UINT8) simple_strtol(RegClassString, 0, 10);
						RegClassIdx++;
					}
				}
				break;
			case 8: /* Channel Report  List. */
				{
					RTMP_STRING *ChIdString;
					int ChId;

					ChId = 0;
					while ((ChIdString = strsep((char **)&thisChar, "!")) != NULL)
					{
						BcnReq.ChRepList[ChId] =
							(UINT8) simple_strtol(ChIdString, 0, 10);
						ChId++;
					}
				}			
				break;
		}
		ArgIdx++;
	}	

	if (ArgIdx < 7 || ArgIdx > 9)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s: invalid args (%d).\n", __FUNCTION__, ArgIdx));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("eg: iwpriv ra0 set BcnReq=<Aid>-<Duration>-<RegulatoryClass>-<BSSID>-<SSID>-<MeasureCh>-<MeasureMode>-<ChRegClass>-<ChReptList>\n"));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("eg: iwpriv ra0 set BcnReq=1-50-12-FF:FF:FF:FF:FF:FF-WiFi1-255-1-12+1-1!6!36!48\n"));
		
		return TRUE;
	}


	BcnReq.BcnReqCapFlag.field.ReportCondition = TRUE;
	if (BcnReq.MeasureCh == 255)
		BcnReq.BcnReqCapFlag.field.ChannelRep = TRUE;
	else
		BcnReq.BcnReqCapFlag.field.ChannelRep = FALSE;

	RRM_EnqueueBcnReq(pAd, Aid, ifIndex, &BcnReq);

	return TRUE;
}

INT Set_LinkMeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UINT Aid = 1;

	Aid = simple_strtol(arg, 0, 10);
	RRM_EnqueueLinkMeasureReq(pAd, Aid, ifIndex);

	return TRUE;
}

INT Set_TxStreamMeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UINT Aid = 1;
	UINT ArgIdx;
	RTMP_STRING *thisChar;

	RRM_MLME_TRANSMIT_REQ_INFO TransmitReq;
	PMAC_TABLE_ENTRY pMacEntry;

	ArgIdx = 0;
	NdisZeroMemory(&TransmitReq, sizeof(RRM_MLME_TRANSMIT_REQ_INFO));

	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		switch(ArgIdx)
		{
			case 0:	/* Aid. */
				Aid = (UINT8) simple_strtol(thisChar, 0, 10);
				if (!VALID_WCID(Aid))
				{
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s: unknow sta of Aid(%d)\n", __FUNCTION__, Aid));
					return TRUE;
				}
				break;

			case 1: /* DurationMandotory. */
				TransmitReq.bDurationMandatory =
					((UINT16)simple_strtol(thisChar, 0, 10) > 0 ? TRUE : FALSE);
				break;

			case 2: /* Measure Duration */
				TransmitReq.MeasureDuration = (UINT16)simple_strtol(thisChar, 0, 10);
				break;

			case 3: /* TID */
				TransmitReq.Tid = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 4: /* Bin 0 Range */
				TransmitReq.BinRange = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 5: /* Averange Condition */
				TransmitReq.ArvCondition =
					((UINT8) simple_strtol(thisChar, 0, 10)) > 0 ? 1 : 0;
				break;

			case 6: /* Consecutive Condition */
				TransmitReq.ConsecutiveCondition =
					((UINT8) simple_strtol(thisChar, 0, 10)) > 0 ? 1 : 0;
				break;

			case 7: /* Delay Condition */
				TransmitReq.DelayCondition =
					((UINT8) simple_strtol(thisChar, 0, 10)) > 0 ? 1 : 0;
				break;

			case 8: /* Averange Error Threshold */
				TransmitReq.AvrErrorThreshold =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 9: /* Consecutive Error Threshold */
				TransmitReq.ConsecutiveErrorThreshold =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 10: /* Delay Threshold */
				TransmitReq.DelayThreshold =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 11: /* Measure counter  */
				TransmitReq.MeasureCnt =
					(UINT8) simple_strtol(thisChar, 0, 10);

				break;

			case 12: /* Trigger time out */
				TransmitReq.TriggerTimeout =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;
			
		}
		ArgIdx++;
	}	

	if ((ArgIdx != 13) && (ArgIdx != 5))
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s: invalid args (%d).\n", __FUNCTION__, ArgIdx));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("eg: iwpriv ra0 set txreq=<Aid>-<DurationMandortory>-<Duration>-<TID>-<BinRange>[-<AvrCond>-<ConsecutiveCond>-<DealyCond>-<AvrErrorThreshold>-<ConsecutiveErrorThreshold>-<DelayThreshold>-<MeasureCnt>-<TriggerTimeout>]\n"));
		return TRUE;
	}

	if (ArgIdx == 5)
		TransmitReq.bTriggerReport = 0;
	else
		TransmitReq.bTriggerReport = 1;

	pMacEntry = &pAd->MacTab.Content[Aid];
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s::Aid=%d, PeerMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__FUNCTION__, Aid,	pMacEntry->Addr[0], pMacEntry->Addr[1],
		pMacEntry->Addr[2], pMacEntry->Addr[3], pMacEntry->Addr[4], pMacEntry->Addr[5]));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("Duration=%d, Tid=%d, Bin 0 Range=%d\n",
		TransmitReq.MeasureDuration, TransmitReq.Tid, TransmitReq.BinRange));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("ArvCondition=%d, ConsecutiveCondition=%d, DelayCondition=%d\n",
		TransmitReq.ArvCondition, TransmitReq.ConsecutiveCondition, TransmitReq.DelayCondition));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("AvrErrorThreshold=%d, ConsecutiveErrorThreshold=%d\n",
		TransmitReq.AvrErrorThreshold, TransmitReq.ConsecutiveErrorThreshold));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("DelayThreshold=%d\n", TransmitReq.DelayThreshold));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("MeasureCnt=%d, TriggerTimeout=%d\n",
		TransmitReq.MeasureCnt, TransmitReq.TriggerTimeout));

	RRM_EnqueueTxStreamMeasureReq(pAd, Aid, ifIndex, &TransmitReq);

	return TRUE;
}

INT Set_RRM_Selftest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Cmd = 1;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR StaAddr[MAC_ADDR_LEN] = {0x00,0x0c,0x43,0x00,0x00,0x00};

	Cmd = simple_strtol(arg, 0, 10);

	switch(Cmd)
	{
		case 1: /* insert a STA for RRM Beacon Report request testing. */
			pEntry = MacTableInsertEntry(pAd, StaAddr, &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);
			pEntry->Sst = SST_ASSOC;
			pEntry->CapabilityInfo |= RRM_CAP_BIT;
			pEntry->RrmEnCap.field.BeaconActiveMeasureCap = 1;
			break;
	}
	return TRUE;
}

INT RRM_InfoDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT loop;

	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++)
	{
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[loop].wdev;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("%d: bDot11kRRMEnable=%d\n",
			loop, pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Regulator Class="));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("%d ",
			get_regulatory_class(pAd,wdev->channel,wdev->PhyMode,wdev)));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("\n"));	
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Country Code=%s\n",
		pAd->CommonCfg.CountryCode));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Power Constraint=%d\n",
		pAd->CommonCfg.PwrConstraint));

#ifdef DBDC_MODE
    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Regulator TxPowerPercentage=(%ld, %ld)\n",
        pAd->CommonCfg.TxPowerPercentage[BAND0], pAd->CommonCfg.TxPowerPercentage[BAND1]));
#else
    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Regulator TxPowerPercentage=%ld\n",
        pAd->CommonCfg.TxPowerPercentage[BAND0]));
#endif

	return TRUE;
}

VOID RRM_CfgInit(
	IN PRTMP_ADAPTER pAd)
{
	INT loop;
	PRRM_CONFIG pRrmCfg;

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
	{
		pRrmCfg = &pAd->ApCfg.MBSSID[loop].RrmCfg;
		pRrmCfg->QuietCB.QuietPeriod = RRM_DEFAULT_QUIET_PERIOD;
		pRrmCfg->QuietCB.QuietDuration = RRM_DEFAULT_QUIET_DURATION;
		pRrmCfg->QuietCB.QuietOffset = RRM_DEFAULT_QUIET_OFFSET;

		RTMPInitTimer(pAd, &pRrmCfg->QuietCB.QuietOffsetTimer, GET_TIMER_FUNCTION(RRM_QuietOffsetTimeout), pAd, FALSE);
		RTMPInitTimer(pAd, &pRrmCfg->QuietCB.QuietTimer, GET_TIMER_FUNCTION(RRM_QuietTimeout), pAd, FALSE);

		pRrmCfg->QuietCB.QuietState = RRM_QUIET_IDLE;
		pRrmCfg->QuietCB.CurAid = 1;

		if (pRrmCfg->bDot11kRRMEnableSet == FALSE)
			pRrmCfg->bDot11kRRMEnable = FALSE; //set to default off
			
		pRrmCfg->bDot11kRRMNeighborRepTSFEnable = FALSE;
		init_rrm_capabilities(pRrmCfg, &pAd->ApCfg.MBSSID[loop]);
	}

	return;
}

VOID RRM_QuietUpdata(
	IN PRTMP_ADAPTER pAd)
{
	INT loop;
	PRRM_CONFIG pRrmCfg;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;

	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++)
	{
		mbss = &pAd->ApCfg.MBSSID[loop];
		wdev = &mbss->wdev;

		if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
			!(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev))))
		{
			/* the interface is down */
			continue;
		}

		pRrmCfg = &pAd->ApCfg.MBSSID[loop].RrmCfg;

		if (pRrmCfg->QuietCB.QuietCnt == pRrmCfg->QuietCB.QuietPeriod)
		{
			/* issue Bcn Report Request to STAs supported RRM-Bcn Report. */
			/* Voice-enterpise doesn't require it in testing-event3 so remove it. */
			/*RRM_BcnReortQuery(pAd, loop, pRrmCfg); */
		}

		if ((pRrmCfg->QuietCB.QuietCnt == 0)
			&& (pRrmCfg->QuietCB.QuietState == RRM_QUIET_IDLE))
		{
			if (pRrmCfg->QuietCB.QuietOffset != 0)
			{
				/* Start QuietOffsetTimer. */
				RTMPSetTimer(&pRrmCfg->QuietCB.QuietOffsetTimer,
					pRrmCfg->QuietCB.QuietOffset);
			}
			else
			{
				/* Start Quiet Timer. */
				pRrmCfg->QuietCB.QuietState = RRM_QUIET_SILENT;

				RTMPSetTimer(&pRrmCfg->QuietCB.QuietTimer,
					pRrmCfg->QuietCB.QuietDuration);
			}
		}

		RRM_QUIET_CNT_DEC(pRrmCfg->QuietCB.QuietCnt, pRrmCfg->QuietCB.QuietPeriod);
	}
	return;
}

VOID RRM_PeerNeighborReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	PMAC_TABLE_ENTRY pEntry;
	UINT8 DialogToken;
	PCHAR pSsid = NULL;
	UINT8 SsidLen = 0;
	CHAR ssidbuf[MAX_LEN_OF_SSID+1];

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s::\n", __FUNCTION__));

	/* skip Category and action code. */
	pFramePtr += 2;

	pEntry = MacTableLookup(pAd, pFr->Hdr.Addr2);
	if (!pEntry || (pEntry->Sst != SST_ASSOC))
		return;

	if (RRM_PeerNeighborReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &pSsid, &SsidLen))
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("DialogToken=%x\n", DialogToken));
		snprintf(ssidbuf, sizeof(ssidbuf),"%s", pSsid);
		ssidbuf[SsidLen]='\0';
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("pSsid=%s\n", pSsid));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("SsidLen=%d\n", SsidLen));
		RRM_EnqueueNeighborRep(pAd, pEntry, DialogToken, pSsid, SsidLen);
	}
	return;
}

VOID RRM_BeaconReportHandler(
	IN PRTMP_ADAPTER pAd,
	IN PRRM_BEACON_REP_INFO pBcnRepInfo,
	IN LONG Length)
{
	CHAR Rssi;
	USHORT LenVIE = 0;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	//UCHAR VarIE[MAX_VIE_LEN];
	UCHAR *VarIE = NULL;		/*Wframe-larger-than=1024 warning  removal*/
	ULONG Idx = BSS_NOT_FOUND;
	/*
	 	if peer response mesurement pilot frame, pVIE->Length should be init.
	 	Sofar we don't know the mesurement pilot frame format, so we use below flag to avoid call BssTableSetEntry() instead of init. pVIE->Length
	*/
	BOOLEAN bFrameBody = FALSE;
	LONG RemainLen = Length;
	PRRM_BEACON_REP_INFO pBcnRep;
	PUINT8 ptr;
	RRM_BEACON_REP_INFO_FIELD BcnReqInfoField;
	UINT32 Ptsf;
	BCN_IE_LIST *ie_list = NULL;

	os_alloc_mem(NULL, (UCHAR **)&VarIE, 1024);
	if (VarIE == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s(): Alloc VarIE failed!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(VarIE, 1024);

	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL) {
		if (VarIE != NULL) {
			os_free_mem(VarIE);
		}
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s(): Alloc ie_list failed!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));

	ptr = (PUINT8)pBcnRepInfo;

	pBcnRep = (PRRM_BEACON_REP_INFO)ptr;
	Ptsf = le2cpu32(pBcnRep->ParentTSF);
	BcnReqInfoField.word = pBcnRep->RepFrameInfo; 

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s:: ReqClass=%d, Channel=%d\n",
		__FUNCTION__, pBcnRep->RegulatoryClass, pBcnRep->ChNumber));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("Bssid=%02x:%02x:%02x:%02x:%02x:%02x, FrameType=%s\n",
						PRINT_MAC(pBcnRep->Bssid), (BcnReqInfoField.field.ReportFrameType == 0) ?"beacon, probe resp" :"measurement pilot" ));

	Rssi = pBcnRep->RCPI + pAd->BbpRssiToDbmDelta;

	RemainLen -= sizeof(RRM_BEACON_REP_INFO);
	ptr += sizeof(RRM_BEACON_REP_INFO);

	/* check option sub element IE. */
	while (RemainLen > 0)
	{
		PRRM_SUBFRAME_INFO pRrmSubFrame;
		pRrmSubFrame = (PRRM_SUBFRAME_INFO)ptr;

		switch(pRrmSubFrame->SubId)
		{
			case 1:
				if (BcnReqInfoField.field.ReportFrameType == 0) // 0 ==> beacon or probe response frame , 1 ==> Measurement Pilot frame
				{
					/* Init Variable IE structure */
					pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
					pVIE->Length = 0;

					bFrameBody = PeerBeaconAndProbeRspSanity(pAd,
								pRrmSubFrame->Oct,
								pRrmSubFrame->Length,
								pBcnRep->ChNumber,
								ie_list,
								&LenVIE,
								pVIE,
								FALSE,
								TRUE);

					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s:: bFrameBody=%d\n", __FUNCTION__, bFrameBody));
	
				}
				break;

			case 221:
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s:: SubIe: ID=%x, Len=%d\n",
					__FUNCTION__, pRrmSubFrame->SubId, pRrmSubFrame->Length));
				break;
		}

		RemainLen -= (pRrmSubFrame->Length + 2);
		ptr += (pRrmSubFrame->Length + 2);

		/* avoid infinite loop. */
		if (pRrmSubFrame->Length == 0)
			break;
	}

	if (NdisEqualMemory(pBcnRep->Bssid, ie_list->Bssid, MAC_ADDR_LEN) == FALSE)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s():BcnReq->BSSID=%02x:%02x:%02x:%02x:%02x:%02x not equal ie_list->Bssid=%02x:%02x:%02x:%02x:%02x:%02x!\n",
			__FUNCTION__,PRINT_MAC(pBcnRep->Bssid),PRINT_MAC(ie_list->Bssid)));

		if (NdisEqualMemory(ie_list->Bssid, ZERO_MAC_ADDR, MAC_ADDR_LEN))
		{
			COPY_MAC_ADDR(&ie_list->Addr2[0], pBcnRep->Bssid);
			COPY_MAC_ADDR(&ie_list->Bssid[0], pBcnRep->Bssid);	
		}

	}

	if (ie_list->Channel == 0)
		ie_list->Channel = pBcnRep->ChNumber;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s():ie_list->Channel=%d\n",
			__FUNCTION__,ie_list->Channel));

#ifdef AP_SCAN_SUPPORT
	if (bFrameBody)
	{

		ie_list->FromBcnReport = TRUE;
	
		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE);

		if (Idx != BSS_NOT_FOUND)
		{
			BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[Idx];
			NdisMoveMemory(pBssEntry->PTSF, (PUCHAR)&Ptsf, 4);
			pBssEntry->RegulatoryClass = pBcnRep->RegulatoryClass;
			pBssEntry->CondensedPhyType = BcnReqInfoField.field.CondensePhyType;
			pBssEntry->RSNI = pBcnRep->RSNI;
		}
	}
#endif /* AP_SCAN_SUPPORT */
	
	if (ie_list != NULL)
	{
		os_free_mem(ie_list);
	}

	if (VarIE != NULL) {
		os_free_mem(VarIE);
	}

	return;
}
void RRM_measurement_report_to_host(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
#define USE_BEACON_REPORT_FOR_NEIGHBOR_REPORT
VOID RRM_PeerMeasureRepAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
#ifdef USE_BEACON_REPORT_FOR_NEIGHBOR_REPORT
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	ULONG MsgLen = Elem->MsgLen;
	//PMEASURE_REQ_ENTRY pDialogEntry;
	PMAC_TABLE_ENTRY pEntry;
	UINT8 DialogToken;
#endif
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s::\n", __FUNCTION__));
	RRM_measurement_report_to_host(pAd, Elem);
		
#ifdef USE_BEACON_REPORT_FOR_NEIGHBOR_REPORT
	/* skip Category and action code. */
	pFramePtr += 2;
	MsgLen -= 2;

	/* get DialogToken. */
	NdisMoveMemory(&DialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	/*
		Not a autonomous measure report (non zero DialogToken).
		check the dialog token field.
		drop it if the dialog token doesn't match.
	*/
	do
	{
		PEID_STRUCT eid_ptr;
		MEASURE_REPORT_MODE ReportMode;
		UINT8 ReportType;
		PRRM_BEACON_REP_INFO pMeasureRep;

		/* Is the STA associated. Dorp the Measure report if it's not. */
		pEntry = MacTableLookup(pAd, pFr->Hdr.Addr2);
		if (!pEntry || (pEntry->Sst != SST_ASSOC))
			break;


		eid_ptr = (PEID_STRUCT)pFramePtr;
		while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
		{
			switch(eid_ptr->Eid)
			{
				case IE_MEASUREMENT_REPORT:
					{
						LONG BcnRepLen = (LONG)eid_ptr->Len - 3;
						NdisMoveMemory(&ReportMode, eid_ptr->Octet + 1, 1);
						NdisMoveMemory(&ReportType, eid_ptr->Octet + 2, 1);
						pMeasureRep = (PVOID)(eid_ptr->Octet + 3);
						if (ReportType == RRM_MEASURE_SUBTYPE_BEACON)
							RRM_BeaconReportHandler(pAd, pMeasureRep,
								BcnRepLen);
					}
					break;

				default:
					break;
			}
			eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
		}
	} while(FALSE);
#endif
	return;
}

int rrm_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	p_rrm_command_t p_rrm_command = NULL;
	int status = NDIS_STATUS_FAILURE;
	
	os_alloc_mem(NULL, (UCHAR **)&p_rrm_command, wrq->u.data.length);
	if (p_rrm_command == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("!!!(%s) : no memory!!!\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	copy_from_user(p_rrm_command, wrq->u.data.pointer, wrq->u.data.length);

	switch(p_rrm_command->command_id)
		{

			case OID_802_11_RRM_CMD_ENABLE:
				if (p_rrm_command->command_len == 0x1){
					status = Set_Dot11kRRM_Enable(pAd, p_rrm_command->command_body[0]);
				} else { 
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Unexpected command len (%d)", p_rrm_command->command_len));
					status = NDIS_STATUS_FAILURE;
				}
				break;

			case OID_802_11_RRM_CMD_CAP:
				if (p_rrm_command->command_len == 0x8){
					status = set_rrm_capabilities(pAd, p_rrm_command->command_body);
				} else { 
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Unexpected command len (%d)", p_rrm_command->command_len));
					status = NDIS_STATUS_FAILURE;
				}
				break;

			case OID_802_11_RRM_CMD_SEND_BEACON_REQ:
					status = rrm_send_beacon_req(pAd, (p_bcn_req_data_t)p_rrm_command->command_body);
				break;
		}
	os_free_mem(p_rrm_command);
	return status;
}
VOID APUpdateBeaconFrame(RTMP_ADAPTER *pAd, INT apidx);
VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd,struct _BSS_STRUCT *mbss);

int Set_Dot11kRRM_Enable(RTMP_ADAPTER *pAd, UINT8 enable)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[ifIndex]; 

	if (ifIndex >= pAd->ApCfg.BssidNum)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Unknow If index (%d)", ifIndex));
		return NDIS_STATUS_FAILURE;
	}

	pMbss->RrmCfg.bDot11kRRMEnable = (BOOLEAN)(enable) == 0 ? FALSE : TRUE;

	if (pMbss->RrmCfg.bDot11kRRMEnable == TRUE)
        pMbss->CapabilityInfo |= RRM_CAP_BIT;
	else
		pMbss->CapabilityInfo &= ~RRM_CAP_BIT;

	ApUpdateCapabilityAndErpIe(pAd, pMbss);
	UpdateBeaconHandler(pAd, &pMbss->wdev,IE_CHANGE);
	return NDIS_STATUS_SUCCESS;
}

int set_rrm_capabilities(RTMP_ADAPTER *pAd, UINT8 *rrm_capabilities)
{
	
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss; 
	PRRM_CONFIG pRrmCfg;	

	if (ifIndex >= pAd->ApCfg.BssidNum)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Unknow If index (%d)", ifIndex));
		return NDIS_STATUS_FAILURE;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	pRrmCfg = &pMbss->RrmCfg;
	NdisCopyMemory((void *)&pRrmCfg->rrm_capabilities, (void *)rrm_capabilities, sizeof(RRM_EN_CAP_IE));

	pRrmCfg->rrm_capabilities.word &= pRrmCfg->max_rrm_capabilities.word;

	if (pMbss->RrmCfg.bDot11kRRMEnable == TRUE)
    	UpdateBeaconHandler(pAd, &pMbss->wdev,IE_CHANGE);

	return NDIS_STATUS_SUCCESS;
}


int rrm_send_beacon_req(RTMP_ADAPTER *pAd, p_bcn_req_data_t p_bcn_req_data)
{
	
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss; 
	
	pMbss = &pAd->ApCfg.MBSSID[ifIndex]; 
	//PRRM_CONFIG pRrmCfg;	
	if (ifIndex >= pAd->ApCfg.BssidNum)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Unknow If index (%d)", ifIndex));
		return NDIS_STATUS_FAILURE;
	}
	return RRM_EnqueueBcnReqAction(pAd, pMbss->mbss_idx, p_bcn_req_data);
}
void init_rrm_capabilities(PRRM_CONFIG pRrmCfg, BSS_STRUCT *pMBss)
{
	pRrmCfg->max_rrm_capabilities.word = 0;
	pRrmCfg->max_rrm_capabilities.field.NeighborRepCap = 1;
	pRrmCfg->max_rrm_capabilities.field.BeaconPassiveMeasureCap = 1;
	pRrmCfg->max_rrm_capabilities.field.BeaconActiveMeasureCap = 1;
	pRrmCfg->max_rrm_capabilities.field.BeaconTabMeasureCap = 1;
	pRrmCfg->max_rrm_capabilities.field.APChannelReportCap = 1;
	pRrmCfg->max_rrm_capabilities.field.BeaconMeasureReportCndCap = 1;
	pRrmCfg->rrm_capabilities.word = pRrmCfg->max_rrm_capabilities.word;
}
extern int DebugLevel;

void RRM_measurement_report_to_host(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	ULONG MsgLen = Elem->MsgLen;
	UINT8 DialogToken;
	p_rrm_event_t p_rrm_event;
	p_bcn_rsp_data_t p_bcn_rsp_data;
	
	MsgLen -= sizeof(HEADER_802_11);
	
	/* skip category and action code. */
	pFramePtr += 2;
	MsgLen -= 2;
	DialogToken = *pFramePtr;
	
	pFramePtr += 1;
	MsgLen -= 1;
	os_alloc_mem(NULL, (UCHAR **)&p_rrm_event, sizeof(rrm_event_t)+ sizeof(bcn_rsp_data_t) + MsgLen);
	if (p_rrm_event == NULL)
		{
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("!!!(%s) : no memory!!!\n", __FUNCTION__));
			return;
		}
		

	p_rrm_event->event_id = 1;	
	p_rrm_event->event_len = sizeof(bcn_rsp_data_t) + MsgLen;
	p_bcn_rsp_data = (p_bcn_rsp_data_t)(&p_rrm_event->event_body[0]);
	p_bcn_rsp_data->dialog_token = DialogToken;
	p_bcn_rsp_data->ifindex = 1;
	p_bcn_rsp_data->bcn_rsp_len = MsgLen;
	COPY_MAC_ADDR(p_bcn_rsp_data->peer_address, pFr->Hdr.Addr2);
	memcpy(&p_bcn_rsp_data->bcn_rsp[0], pFramePtr, MsgLen);
	RtmpOSWrielessEventSend(
				pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				OID_802_11_RRM_EVENT,
				NULL,
				(UCHAR *) p_rrm_event,
				sizeof(rrm_event_t) + sizeof(bcn_rsp_data_t) + MsgLen);
	os_free_mem(p_rrm_event);
}

#endif /* DOT11K_RRM_SUPPORT */

