
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
	==========================================================================
	Description:
		
	Parametrs:
	
	Return	: None.
	==========================================================================
 */
VOID RRM_ScanResultFix(BSS_ENTRY *pBssEntry)
{
			if (pBssEntry->Channel > 14) // 5G case
			{
				if (pBssEntry->HtCapabilityLen != 0) // HT or Higher case
				{
#ifdef DOT11_VHT_AC				
					if (pBssEntry->vht_cap_len != 0)
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				}
				else // OFDM case
				{
					pBssEntry->CondensedPhyType = 4;
				}
			}
			else // 2.4G case
			{

				if (pBssEntry->HtCapabilityLen != 0) //HT case
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) //ERP case
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4)// OFDM case (1,2,5.5,11 for CCK 4 Rates)
					pBssEntry->CondensedPhyType = 4;

				/* no CCK's definition in spec. */
			}

			/* calculate new op_class if not get from scan */
			if (!pBssEntry->RegulatoryClass) {
			    if (pBssEntry->Channel > 14) {
				if (pBssEntry->CondensedPhyType == 9) {					/* VHT 36 - 161 80MHz */
				    pBssEntry->RegulatoryClass = 128;
				} else if (pBssEntry->Channel >= 36 && pBssEntry->Channel <= 48) {	/* HT 36 - 48 20/40MHz */
				    switch(pBssEntry->AddHtInfo.AddHtInfo.ExtChanOffset)
				    {
					case EXTCHA_ABOVE:
						pBssEntry->RegulatoryClass = 116;
						break;
					case EXTCHA_BELOW:
						pBssEntry->RegulatoryClass = 117;
						break;
					default:
						pBssEntry->RegulatoryClass = 115;
						break;
				    }
				} else if (pBssEntry->Channel >= 52 && pBssEntry->Channel <= 64) {	/* HT 52 - 64 20/40MHz */
				    switch(pBssEntry->AddHtInfo.AddHtInfo.ExtChanOffset)
				    {
					case EXTCHA_ABOVE:
						pBssEntry->RegulatoryClass = 119;
						break;
					case EXTCHA_BELOW:
						pBssEntry->RegulatoryClass = 120;
						break;
					default:
						pBssEntry->RegulatoryClass = 118;
						break;
				    }
				} else if (pBssEntry->Channel >= 149) {					/* HT >= 149 20/40MHz */
				    switch(pBssEntry->AddHtInfo.AddHtInfo.ExtChanOffset)
				    {
					case EXTCHA_ABOVE:
						pBssEntry->RegulatoryClass = 126;
						break;
					case EXTCHA_BELOW:
						pBssEntry->RegulatoryClass = 127;
						break;
					default:
						pBssEntry->RegulatoryClass = 124;
						break;
				    }
				} else {
						pBssEntry->RegulatoryClass = 128;
				}
			    } else { /* 2.4GHz mode */
				if (pBssEntry->CondensedPhyType == 7) {
				    switch(pBssEntry->AddHtInfo.AddHtInfo.ExtChanOffset)
				    {
					case EXTCHA_ABOVE:
						pBssEntry->RegulatoryClass = 83;
						break;
					case EXTCHA_BELOW:
						pBssEntry->RegulatoryClass = 84;
						break;
					default:
						pBssEntry->RegulatoryClass = 81;
						break;
				    }
				} else {
					pBssEntry->RegulatoryClass = 81;
				}
			    }
			}

			/* if not get beacon period from scan - use default 100ms */
			if (!pBssEntry->BeaconPeriod)
				pBssEntry->BeaconPeriod = 100;
}

/*
	==========================================================================
	Description:
		
	Parametrs:
	
	Return	: None.
	==========================================================================
 */
BOOLEAN RRM_PeerNeighborReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PCHAR *pSsid,
	OUT PUINT8 pSsidLen)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PMAC_TABLE_ENTRY pEntry;
	PRRM_CONFIG pRrmCfg;
	UINT8 PeerMeasurementType;

	if ((Fr == NULL) || (pDialogToken == NULL))
		return result;

	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code. */
	pFramePtr = Fr->Octet;
	pFramePtr += 2;
	MsgLen -= 2;

	pEntry = MacTableLookup(pAd, Fr->Hdr.Addr2);
	if (pEntry == NULL || pEntry->func_tb_idx > pAd->ApCfg.BssidNum)
	{
		return result;
	}

	pRrmCfg = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].RrmCfg;
	*pSsid = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid;
	*pSsidLen = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen;

	result = TRUE;
	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case RRM_NEIGHBOR_REQ_SSID_SUB_ID:
				*pSsid = (PCHAR)eid_ptr->Octet;
				*pSsidLen = eid_ptr->Len;
                break;
        		case RRM_NEIGHBOR_REQ_MEASUREMENT_REQUEST_SUB_ID:
	        		DBGPRINT(RT_DEBUG_TRACE, ("%s - Got STA Measurement Request\n",__FUNCTION__));
				pRrmCfg->PeerMeasurementToken = eid_ptr->Octet[0];
				PeerMeasurementType = eid_ptr->Octet[2];
				switch(PeerMeasurementType)
				{
					case RRM_MEASURE_SUBTYPE_LCI:
						pRrmCfg->bPeerReqLCI = TRUE;
						DBGPRINT(RT_DEBUG_TRACE, ("%s - STA Request LCI Measurement Report\n",__FUNCTION__));
					break;
					case RRM_MEASURE_SUBTYPE_LOCATION_CIVIC:
						pRrmCfg->bPeerReqCIVIC = TRUE;
						DBGPRINT(RT_DEBUG_TRACE, ("%s - STA Request CIVIC Measurement Report\n",__FUNCTION__));
					break;
					default:
						DBGPRINT(RT_DEBUG_TRACE, ("unknown PeerMeasurementType: %d\n",PeerMeasurementType));
				}
                break;


			case RRM_NEIGHBOR_REQ_VENDOR_SUB_ID:
                break;

			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;	
}


BOOLEAN RRM_PeerMeasureReportSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REPORT_INFO pMeasureReportInfo,
	OUT PVOID *pMeasureRep)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PUCHAR pFramePtr;

	if (Fr == NULL || pMeasureReportInfo == NULL)
		return result;

    	pFramePtr = Fr->Octet;

	/* skip 802.11 header. */
	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code. */
	pFramePtr += 2;
	MsgLen -= 2;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_MEASUREMENT_REPORT:
				NdisMoveMemory(&pMeasureReportInfo->Token, eid_ptr->Octet, 1);
				NdisMoveMemory(&pMeasureReportInfo->ReportMode, eid_ptr->Octet + 1, 1);
				NdisMoveMemory(&pMeasureReportInfo->ReportType, eid_ptr->Octet + 2, 1);
				*pMeasureRep = (PVOID)(eid_ptr->Octet + 3);
				result = TRUE;
                break;

			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}

#endif /* DOT11K_RRM_SUPPORT */

