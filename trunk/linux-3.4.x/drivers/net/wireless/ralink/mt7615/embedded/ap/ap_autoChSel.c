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
 
    Abstract:
*/


#include "rt_config.h"
#include "ap_autoChSel.h"


extern UCHAR ZeroSsid[32];

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;
#ifdef AP_SCAN_SUPPORT 
extern INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev);
#endif/*AP_SCAN_SUPPORT*/

#ifdef DOT11_VHT_AC 
struct vht_ch_layout{
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};
#endif/* DOT11_VHT_AC */

static inline INT GetABandChOffset(
	IN INT Channel)
{
#ifdef A_BAND_SUPPORT
	if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
	    (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
	{
		return 1;
	}
	else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
	{
		return -1;
	}
#endif /* A_BAND_SUPPORT */
	return 0;
}

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel)
{
	UCHAR i;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if(pBssInfoTab == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pBssInfoTab equal NULL.\n"));
		return (ULONG)BSS_NOT_FOUND;
	}

	for (i = 0; i < pBssInfoTab->BssNr; i++) 
	{
		if ((((pBssInfoTab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			((pBssInfoTab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(pBssInfoTab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, pBssInfoTab->BssEntry[i].Ssid, pBssInfoTab->BssEntry[i].SsidLen) ||
			(NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) || 
			(NdisEqualMemory(pBssInfoTab->BssEntry[i].Ssid, ZeroSsid, pBssInfoTab->BssEntry[i].SsidLen))))
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}

static inline VOID AutoChBssEntrySet(
	OUT BSSENTRY *pBss, 
	IN PUCHAR pBssid, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen, 
	IN UCHAR Channel,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi)
{
	COPY_MAC_ADDR(pBss->Bssid, pBssid);
	if (SsidLen > 0 && SsidLen <= MAX_LEN_OF_SSID)
	{
		/* 
			For hidden SSID AP, it might send beacon with SSID len equal to 0,
			Or send beacon /probe response with SSID len matching real SSID length,
			but SSID is all zero. such as "00-00-00-00" with length 4.
			We have to prevent this case overwrite correct table
		*/
		if (NdisEqualMemory(Ssid, ZeroSsid, SsidLen) == 0)
		{
			NdisMoveMemory(pBss->Ssid, Ssid, SsidLen);
			pBss->SsidLen = SsidLen;
		}
	}

	pBss->Channel = Channel;
	pBss->ExtChOffset = ExtChOffset;
	pBss->Rssi = Rssi;

	return;
}


VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch_index,
	IN ChannelSel_Alg Alg)
{

	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);

	if(pAutoChCtrl->pChannelInfo != NULL)
	{
		UINT32 BusyTime;

		if (Alg == ChannelAlgCCA)
		{
			UINT32 cca_cnt = AsicGetCCACnt(pAd);

			pAd->RalinkCounters.OneSecFalseCCACnt += cca_cnt;
			pAutoChCtrl->pChannelInfo->FalseCCA[ch_index] = cca_cnt;
		}

		/*
			do busy time statistics for primary channel
			scan time 200ms, beacon interval 100 ms
		*/
		BusyTime = AsicGetChBusyCnt(pAd, 0);
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / 200;

#ifdef AP_QLOAD_SUPPORT
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / AUTO_CHANNEL_SEL_TIMEOUT;
#endif /* AP_QLOAD_SUPPORT */
	}
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAutoChCtrl->pChannelInfo equal NULL.\n"));

	return;
}

static inline INT GetChIdx(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	INT Idx;

	Idx = -1;
	for (Idx = 0; Idx < pAd->ChannelListNum; Idx++)
	{
		if (Channel == pAd->ChannelList[Idx].Channel)
			break;
	}

	return Idx;
}

static inline VOID AutoChannelSkipListSetDirty(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR i;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	for (i=0; i < pAd->ApCfg.AutoChannelSkipListNum ; i++)
	{
			UCHAR channel_idx = GetChIdx(pAd, pAd->ApCfg.AutoChannelSkipList[i]);
			if ( channel_idx != pAd->ChannelListNum )
			{
				pAutoChCtrl->pChannelInfo->SkipList[channel_idx] = TRUE;
			}
	}
}

static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;

	for (i=0; i < pAd->ApCfg.AutoChannelSkipListNum ; i++)
	{
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i])
		{
			result = TRUE;
			break;
		}
	}
	return result;
}

static inline BOOLEAN BW40_ChannelCheck(
	IN UCHAR ch)
{
	INT i;
	BOOLEAN result = TRUE;
	UCHAR NorBW40_CH[] = {140, 165};
	UCHAR NorBW40ChNum = sizeof(NorBW40_CH) / sizeof(UCHAR);

	for (i=0; i<NorBW40ChNum; i++)
	{
		if (ch == NorBW40_CH[i])
		{
			result = FALSE;
			break;
		}
	}

	return result;
}

static inline UCHAR SelectClearChannelRandom(RTMP_ADAPTER *pAd)
{
	UCHAR cnt, ch = 0, i, RadomIdx;
	/*BOOLEAN bFindIt = FALSE;*/
	UINT8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
	
	if (pAd->CommonCfg.bIEEE80211H)
	{
		cnt = 0;
		
		/* Filter out an available channel list */
		for (i = 0; i < pAd->ChannelListNum; i++)
		{
			/* Check DFS channel RemainingTimeForUse */
			if (pAd->ChannelList[i].RemainingTimeForUse)
				continue;

			/* Check skip channel list */
			if (AutoChannelSkipListCheck(pAd, pAd->ChannelList[i].Channel) == TRUE)
				continue;

#ifdef DOT11_N_SUPPORT
			/* Check N-group of BW40 */
			if (cfg_ht_bw == BW_40 &&
				!(pAd->ChannelList[i].Flags & CHANNEL_40M_CAP))
				continue;
#endif /* DOT11_N_SUPPORT */

			/* Store available channel to temp list */
			TempChList[cnt++] = pAd->ChannelList[i].Channel;
		}

		/* Randomly select a channel from temp list */
		if (cnt)
		{
			RadomIdx = RandomByte2(pAd)%cnt;
			ch = TempChList[RadomIdx];
		}
		else
		{
			ch = get_channel_by_reference(pAd, 1);
		}
		
	}
	else
	{
		ch = pAd->ChannelList[RandomByte2(pAd)%pAd->ChannelListNum].Channel;
		if (ch == 0)
			ch = FirstChannel(pAd);
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): Select Channel %d\n", __FUNCTION__, ch));
	return ch;

}

/* 
	==========================================================================
	Description:
        This routine calaulates the dirtyness of all channels by the 
        CCA value  and Rssi. Store dirtyness to pChannelInfo strcut.
		This routine is called at iwpriv cmmand or initialization. It chooses and returns 
		a good channel whith less interference.
	Return:
		ch -  channel number that
	NOTE:
	==========================================================================
 */
static inline UCHAR SelectClearChannelCCA(RTMP_ADAPTER *pAd)
{
	#define CCA_THRESHOLD (100)
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	INT ch = 1, channel_idx, BssTab_idx;
	BSSENTRY *pBss;
	UINT32 min_dirty, min_falsecca;
	int candidate_ch;
	UCHAR  ExChannel[2] = {0}, candidate_ExChannel[2] = {0};	
	UCHAR base;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

	if(pBssInfoTab == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pBssInfoTab equal NULL.\n"));
		return (FirstChannel(pAd));
	}

	if(pChannelInfo == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pChannelInfo equal NULL.\n"));
		return (FirstChannel(pAd));
	}

	for (BssTab_idx = 0; BssTab_idx < pBssInfoTab->BssNr; BssTab_idx++)
	{
		pBss = &(pBssInfoTab->BssEntry[BssTab_idx]);
		channel_idx = GetChIdx(pAd, pBss->Channel);
		if (channel_idx < 0 || channel_idx >= MAX_NUM_OF_CHANNELS+1)
			continue;


		if (pBss->Rssi >= RSSI_TO_DBM_OFFSET-50)
		{
			/* high signal >= -50 dbm */
			pChannelInfo->dirtyness[channel_idx] += 50;
		}
		else if (pBss->Rssi <= RSSI_TO_DBM_OFFSET-80)
		{
			/* low signal <= -80 dbm */
			pChannelInfo->dirtyness[channel_idx] += 30;
		}
		else
		{
			/* mid signal -50 ~ -80 dbm */
			pChannelInfo->dirtyness[channel_idx] += 40;
		}

		pChannelInfo->dirtyness[channel_idx] += 40;

		{
			INT BelowBound;
			INT AboveBound;
			INT loop;

			switch(pBss->ExtChOffset)
			{
				case EXTCHA_ABOVE:
					BelowBound = pChannelInfo->IsABand ? 1 : 4;
					AboveBound = pChannelInfo->IsABand ? 2 : 8;
					break;

				case EXTCHA_BELOW:
					BelowBound = pChannelInfo->IsABand ? 2 : 8;
					AboveBound = pChannelInfo->IsABand ? 1 : 4;
					break;

				default:
					BelowBound = pChannelInfo->IsABand ? 1 : 4;
					AboveBound = pChannelInfo->IsABand ? 1 : 4;
					break;
			}

			/* check neighbor channel */
			for (loop = (channel_idx+1); loop <= (channel_idx+AboveBound); loop++)
			{
				if (loop >= MAX_NUM_OF_CHANNELS)
					break;

				if (pAd->ChannelList[loop].Channel - pAd->ChannelList[loop-1].Channel > 4)
					break;

				pChannelInfo->dirtyness[loop] += ((9 - (loop - channel_idx)) * 4);
			}

            /* check neighbor channel */
			for (loop=(channel_idx-1); loop >= (channel_idx-BelowBound); loop--)
			{
                if (loop < 0 || loop >= MAX_NUM_OF_CHANNELS)
                    break;

                if (pAd->ChannelList[(loop+1) % MAX_NUM_OF_CHANNELS].Channel - pAd->ChannelList[loop % MAX_NUM_OF_CHANNELS].Channel > 4)
                    continue;

                pChannelInfo->dirtyness[loop] +=
                    ((9 - (channel_idx - loop)) * 4);
			}
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" ch%d bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			pBss->Channel, pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));
	}
			
	AutoChannelSkipListSetDirty(pAd);	
	
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));
	for (channel_idx = 0; channel_idx < pAd->ChannelListNum; channel_idx++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Channel %d : Dirty = %ld, False CCA = %u, Busy Time = %u, Skip Channel = %s\n",
					pAd->ChannelList[channel_idx].Channel,
					pChannelInfo->dirtyness[channel_idx],
					pChannelInfo->FalseCCA[channel_idx],
#ifdef AP_QLOAD_SUPPORT
					pChannelInfo->chanbusytime[channel_idx],
#else
					0,
#endif /* AP_QLOAD_SUPPORT */
					(pChannelInfo->SkipList[channel_idx] == TRUE) ? "TRUE" : "FALSE"));
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));

	min_dirty = min_falsecca = 0xFFFFFFFF;

	/* 
	 * Rule 1. Pick up a good channel that False_CCA =< CCA_THRESHOLD 
	 *		   by dirtyness
	 */
	candidate_ch = -1;
	
	for (channel_idx = 0; channel_idx < pAd->ChannelListNum; channel_idx++)
	{
		if (pChannelInfo->SkipList[channel_idx] == TRUE)
			continue;

		if (pChannelInfo->FalseCCA[channel_idx] <= CCA_THRESHOLD)
		{
			UINT32 dirtyness = pChannelInfo->dirtyness[channel_idx];
			ch = pAd->ChannelList[channel_idx].Channel;

#ifdef AP_QLOAD_SUPPORT
			/* QLOAD ALARM */
			/* when busy time of a channel > threshold, skip it */
			/* TODO: Use weight for different references to do channel selection */
			if (QBSS_LoadIsBusyTimeAccepted(pAd,
				pChannelInfo->chanbusytime[channel_idx]) == FALSE)
			{
				/* check next one */
				continue;
			}
#endif /* AP_QLOAD_SUPPORT */

#ifdef DOT11_N_SUPPORT
			/*
				User require 40MHz Bandwidth.
				In the case, ignor all channel
				doesn't support 40MHz Bandwidth.
			*/
			if ((cfg_ht_bw == BW_40)
				&& (pChannelInfo->IsABand && (GetABandChOffset(ch) == 0)))
				continue;

			/*
				Need to Consider the dirtyness of extending channel
				in 40 MHz bandwidth channel.
			*/
			if (cfg_ht_bw == BW_40)
			{
				if (pAutoChCtrl->pChannelInfo->IsABand)
				{
					if (((channel_idx + GetABandChOffset(ch)) >=0)
						&& ((channel_idx + GetABandChOffset(ch)) < pAd->ChannelListNum))
					{
						INT ChOffsetIdx = channel_idx + GetABandChOffset(ch);
						dirtyness += pChannelInfo->dirtyness[ChOffsetIdx];
					}
				}
				else
				{
					UCHAR ExChannel_idx = 0;
					if (pAd->ChannelList[channel_idx].Channel == 14)
					{
						dirtyness = 0xFFFFFFFF;
						break;
					}
					else
					{
						NdisZeroMemory(ExChannel, sizeof(ExChannel));
						if (((channel_idx - 4) >=0) && ((channel_idx - 4) < pAd->ChannelListNum))
						{
							dirtyness += pChannelInfo->dirtyness[channel_idx - 4];
							ExChannel[ExChannel_idx++] = pAd->ChannelList[channel_idx - 4].Channel;
					    }

						if (((channel_idx + 4) >=0) && ((channel_idx + 4) < pAd->ChannelListNum))
						{
						    dirtyness += pChannelInfo->dirtyness[channel_idx + 4];
						    ExChannel[ExChannel_idx++] = pAd->ChannelList[channel_idx + 4].Channel;
						}
					}
				}
			}
#endif /* DOT11_N_SUPPORT */

			if ((min_dirty > dirtyness))
			{
				min_dirty = dirtyness;
				candidate_ch = channel_idx;
				NdisMoveMemory(candidate_ExChannel, ExChannel, 2);
			}
		}
	}

	if (candidate_ch >= 0)
	{
		ch = pAd->ChannelList[candidate_ch].Channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 1 CCA value : Min Dirtiness (Include extension channel) ==> Select Channel %d \n", ch));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Min Dirty = %u\n", min_dirty));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ExChannel = %d , %d\n", candidate_ExChannel[0], candidate_ExChannel[1]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BW        = %s\n", (cfg_ht_bw == BW_40)? "40" : "20"));
		return ch;
	}

	/*
	 * Rule 2. Pick up a good channel that False_CCA > CCA_THRESHOLD 
	 *		   by FalseCCA (FalseCCA + Dirtyness)
	 */
	candidate_ch = -1;
	for (channel_idx = 0; channel_idx < pAd->ChannelListNum; channel_idx++)
	{
		if (pChannelInfo->SkipList[channel_idx] == TRUE)
			continue;
		
		if (pChannelInfo->FalseCCA[channel_idx] > CCA_THRESHOLD)
		{
			UINT32 falsecca = pChannelInfo->FalseCCA[channel_idx] + pChannelInfo->dirtyness[channel_idx];
			ch = pAd->ChannelList[channel_idx].Channel;

#ifdef DOT11_N_SUPPORT
			if ((cfg_ht_bw == BW_40)
				&& (pChannelInfo->IsABand && (GetABandChOffset(ch) == 0)))
				continue;
#endif /* DOT11_N_SUPPORT */

			if ((GetABandChOffset(ch) != 0)
					&& ((channel_idx + GetABandChOffset(ch)) >=0)
					&& ((channel_idx + GetABandChOffset(ch)) < pAd->ChannelListNum))
			{
				INT ChOffsetIdx = channel_idx + GetABandChOffset(ch);
				falsecca += (pChannelInfo->FalseCCA[ChOffsetIdx] +
							pChannelInfo->dirtyness[ChOffsetIdx]);
			}

#ifdef AP_QLOAD_SUPPORT
			/* QLOAD ALARM */
			/* when busy time of a channel > threshold, skip it */
			/* TODO: Use weight for different references to do channel selection */
			if (QBSS_LoadIsBusyTimeAccepted(pAd,
				pChannelInfo->chanbusytime[channel_idx]) == FALSE)
			{
				/* check next one */
				continue;
			}
#endif /* AP_QLOAD_SUPPORT */

			if ((min_falsecca > falsecca))
			{
				min_falsecca = falsecca;
				candidate_ch = channel_idx;
			}
		}
	}

	if (candidate_ch >= 0)
	{
		ch = pAd->ChannelList[candidate_ch].Channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 2 CCA value : Min False CCA value ==> Select Channel %d, min falsecca = %d \n", ch, min_falsecca));
		return	ch;
	}

	base = RandomByte2(pAd);
	for (channel_idx=0 ; channel_idx < pAd->ChannelListNum ; channel_idx++)
	{
		ch = pAd->ChannelList[(base + channel_idx) % pAd->ChannelListNum].Channel;
	
		if (AutoChannelSkipListCheck(pAd, ch))
			continue;
		
		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
			&& (pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, ch))
			continue;

		break;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 3 CCA value : Randomly Select ==> Select Channel %d\n", ch));
	return ch;
}

static inline UCHAR SelectClearChannelBusyTime(
	IN PRTMP_ADAPTER pAd)
{

	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	UINT32 SubGroupMaxBusyTime, SubGroupMaxBusyTimeChIdx, MinBusyTime;
	UINT32 SubGroupMinBusyTime, SubGroupMinBusyTimeChIdx,ChannelIdx, StartChannelIdx, Temp1, Temp2;
	INT	i, j, GroupNum, CandidateCh1 = 0, CandidateChIdx1, base;
#ifdef DOT11_VHT_AC
  UINT32 MinorMinBusyTime;   
  INT CandidateCh2, CandidateChIdx2;
#endif/* DOT11_VHT_AC */		         
	PUINT32 pSubGroupMaxBusyTimeTable = NULL;
	PUINT32 pSubGroupMaxBusyTimeChIdxTable = NULL;
	PUINT32 pSubGroupMinBusyTimeTable = NULL;
	PUINT32 pSubGroupMinBusyTimeChIdxTable = NULL;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

       MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__)); 

       	if(pChannelInfo == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pChannelInfo equal NULL.\n"));
		return (FirstChannel(pAd));
		
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("====================================================================\n"));
	for (ChannelIdx=0; ChannelIdx < pAd->AutoChSelCtrl.ChannelListNum;ChannelIdx++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel %3d : Busy Time = %6u, Skip Channel = %s, BwCap = %s\n",
		pAd->AutoChSelCtrl.AutoChSelChList[ChannelIdx].Channel, pChannelInfo->chanbusytime[ChannelIdx],					 
					(pAd->AutoChSelCtrl.AutoChSelChList[ChannelIdx].SkipChannel == TRUE) ? "TRUE" : "FALSE",
					(pAd->AutoChSelCtrl.AutoChSelChList[ChannelIdx].BwCap == TRUE)?"TRUE" : "FALSE"));
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("====================================================================\n"));

       /*Initialization*/ 
       SubGroupMaxBusyTimeChIdx = 0;
	SubGroupMaxBusyTime = pChannelInfo->chanbusytime[SubGroupMaxBusyTimeChIdx];
       SubGroupMinBusyTimeChIdx = 0;
	SubGroupMinBusyTime = pChannelInfo->chanbusytime[SubGroupMinBusyTimeChIdx];    
	StartChannelIdx = SubGroupMaxBusyTimeChIdx + 1;
	GroupNum = 0;
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMaxBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMaxBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMinBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMinBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));    
	NdisZeroMemory(pSubGroupMaxBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMaxBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMinBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMinBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
    
	for (ChannelIdx = StartChannelIdx; ChannelIdx < pAd->AutoChSelCtrl.ChannelListNum; ChannelIdx++)
	{
		/*Compare the busytime with each other in the same group*/
		if (pAd->AutoChSelCtrl.AutoChSelChList[ChannelIdx].CentralChannel == pAd->AutoChSelCtrl.AutoChSelChList[ChannelIdx-1].CentralChannel)
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                      ("pChannelInfo->chanbusytime[%d] = %d, SubGroupMaxBusyTime = %d, SubGroupMinBusyTime = %d\n",
			ChannelIdx, pChannelInfo->chanbusytime[ChannelIdx], SubGroupMaxBusyTime, SubGroupMinBusyTime));
            
			if (pChannelInfo->chanbusytime[ChannelIdx] > SubGroupMaxBusyTime)
			{
				SubGroupMaxBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMaxBusyTimeChIdx = ChannelIdx;		
			}
                      else if (pChannelInfo->chanbusytime[ChannelIdx] < SubGroupMinBusyTime) 
                      {
				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;                                
                       }
                      
                       MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("SubGroupMaxBusyTime = %d, SubGroupMaxBusyTimeChIdx = %d,SubGroupMinBusyTime = %d SubGroupMinBusyTimeChIdx = %d\n", 
            SubGroupMaxBusyTime, SubGroupMaxBusyTimeChIdx, SubGroupMinBusyTime, SubGroupMinBusyTimeChIdx)); 
                       
			/*Fill in the group table in order for the last group*/ 			    
			if (ChannelIdx == (pAd->AutoChSelCtrl.ChannelListNum - 1))
			{
				pSubGroupMaxBusyTimeTable[GroupNum] = SubGroupMaxBusyTime;
				pSubGroupMaxBusyTimeChIdxTable[GroupNum] = SubGroupMaxBusyTimeChIdx;  
				pSubGroupMinBusyTimeTable[GroupNum] = SubGroupMinBusyTime;
				pSubGroupMinBusyTimeChIdxTable[GroupNum] = SubGroupMinBusyTimeChIdx;
                
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                ("SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
				GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
				GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]));  
                
				GroupNum++;
			}
		}
		else
		{
		        /*Fill in the group table*/
			pSubGroupMaxBusyTimeTable[GroupNum] = SubGroupMaxBusyTime;
			pSubGroupMaxBusyTimeChIdxTable[GroupNum] = SubGroupMaxBusyTimeChIdx;
			pSubGroupMinBusyTimeTable[GroupNum] = SubGroupMinBusyTime;
			pSubGroupMinBusyTimeChIdxTable[GroupNum] = SubGroupMinBusyTimeChIdx; 
            
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
            GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
            GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]));
            
			GroupNum++;
            
			/*Fill in the group table in order for the last group in case of BW20*/
			if ((ChannelIdx == (pAd->AutoChSelCtrl.ChannelListNum - 1)) 
				&& (pAd->AutoChSelCtrl.AutoChSelChList[ChannelIdx].Bw == BW_20))
			{
				pSubGroupMaxBusyTimeTable[GroupNum] = pChannelInfo->chanbusytime[ChannelIdx];;
				pSubGroupMaxBusyTimeChIdxTable[GroupNum] = ChannelIdx;
				pSubGroupMinBusyTimeTable[GroupNum] = pChannelInfo->chanbusytime[ChannelIdx];;
				pSubGroupMinBusyTimeChIdxTable[GroupNum] = ChannelIdx; 
                
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
				GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
				GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]));
                
				GroupNum++;
			}
			else 
			{
				/*Reset indices in order to start checking next group*/
				SubGroupMaxBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMaxBusyTimeChIdx = ChannelIdx;
				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;                
			}
		}
	}

	for (i = 0; i < GroupNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
               ("SubGroupMaxBusyTimeTable[%d] = %d, pSubGroupMaxBusyTimeChIdxTable[%d] = %d, "
		 "SubGroupMinBusyTimeTable[%d] = %d, pSubGroupMinBusyTimeChIdxTable[%d] = %d\n",
        i, pSubGroupMaxBusyTimeTable[i], i, pSubGroupMaxBusyTimeChIdxTable[i],
        i, pSubGroupMinBusyTimeTable[i], i, pSubGroupMinBusyTimeChIdxTable[i]));
	}
    
	/*Sort max_busy_time group table from the smallest to the biggest one  */
	for (i = 0; i < GroupNum; i++)
	{
		for (j = 1; j < (GroupNum-i); j++)
		{            
			if (pSubGroupMaxBusyTimeTable[i] > pSubGroupMaxBusyTimeTable[i+j])
			{
			       /*Swap pSubGroupMaxBusyTimeTable[i] for pSubGroupMaxBusyTimeTable[i+j]*/
				Temp1 = pSubGroupMaxBusyTimeTable[i+j];
				pSubGroupMaxBusyTimeTable[i+j] = pSubGroupMaxBusyTimeTable[i];
				pSubGroupMaxBusyTimeTable[i] = Temp1;
                
			       /*Swap pSubGroupMaxBusyTimeChIdxTable[i] for pSubGroupMaxBusyTimeChIdxTable[i+j]*/                              
				Temp2 = pSubGroupMaxBusyTimeChIdxTable[i+j];
				pSubGroupMaxBusyTimeChIdxTable[i+j] = pSubGroupMaxBusyTimeChIdxTable[i];
				pSubGroupMaxBusyTimeChIdxTable[i] = Temp2; 
                
			       /*Swap pSubGroupMinBusyTimeTable[i] for pSubGroupMinBusyTimeTable[i+j]*/
				Temp1 = pSubGroupMinBusyTimeTable[i+j];
				pSubGroupMinBusyTimeTable[i+j] = pSubGroupMinBusyTimeTable[i];
				pSubGroupMinBusyTimeTable[i] = Temp1;
                
			       /*Swap pSubGroupMinBusyTimeChIdxTable[i] for pSubGroupMinBusyTimeChIdxTable[i+j]*/                              
				Temp2 = pSubGroupMinBusyTimeChIdxTable[i+j];
				pSubGroupMinBusyTimeChIdxTable[i+j] = pSubGroupMinBusyTimeChIdxTable[i];
				pSubGroupMinBusyTimeChIdxTable[i] = Temp2;                
			}
		}	
	}
    
	for (i = 0; i < GroupNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
               ("SubGroupMaxBusyTimeTable[%d] = %d, pSubGroupMaxBusyTimeChIdxTable[%d] = %d, "
         "SubGroupMinBusyTimeTable[%d] = %d, pSubGroupMinBusyTimeChIdxTable[%d] = %d\n",
        i, pSubGroupMaxBusyTimeTable[i], i, pSubGroupMaxBusyTimeChIdxTable[i],
        i, pSubGroupMinBusyTimeTable[i], i, pSubGroupMinBusyTimeChIdxTable[i]));
	}

#ifdef DOT11_VHT_AC
        /*Return channel in case of VHT BW80+80*/
	 if ((pAd->CommonCfg.vht_bw == VHT_BW_8080) 
               && (cfg_ht_bw == BW_40)
         && (GroupNum > 2)
         && (WMODE_CAP_AC(pAd->AutoChSelCtrl.PhyMode) == TRUE))
	{
		MinBusyTime = pSubGroupMaxBusyTimeTable[0];
		MinorMinBusyTime = pSubGroupMaxBusyTimeTable[1];
        
               /*Select primary channel, whose busy time is minimum in the group*/
		CandidateChIdx1 = pSubGroupMinBusyTimeChIdxTable[0];
		CandidateCh1 = pAd->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Channel;
        
               /*Select secondary VHT80 central channel*/
		CandidateChIdx2 = pSubGroupMaxBusyTimeChIdxTable[1];
		CandidateCh2 = pAd->AutoChSelCtrl.AutoChSelChList[CandidateChIdx2].Channel;
		pAd->CommonCfg.vht_cent_ch2 = vht_cent_ch_freq((UCHAR)CandidateCh2, VHT_BW_80);
        
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
	    ("Rule 3 Channel Busy time value : Select Primary Channel %d \n", CandidateCh1));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("Rule 3 Channel Busy time value : Select Secondary Central Channel %d \n", pAd->CommonCfg.vht_cent_ch2));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("Rule 3 Channel Busy time value : Min Channel Busy = %u\n", MinBusyTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("Rule 3 Channel Busy time value : MinorMin Channel Busy = %u\n", MinorMinBusyTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("Rule 3 Channel Busy time value : BW = %s\n","80+80"));
        
		os_free_mem(pSubGroupMaxBusyTimeTable);
		os_free_mem(pSubGroupMaxBusyTimeChIdxTable);
 		os_free_mem(pSubGroupMinBusyTimeTable);
		os_free_mem(pSubGroupMinBusyTimeChIdxTable);         
        
		goto ReturnCh;
	}
#endif/*DOT11_VHT_AC*/

	if (GroupNum > 0) 
        {
		MinBusyTime = pSubGroupMaxBusyTimeTable[0];
        
               /*Select primary channel, whose busy time is minimum in the group*/
		CandidateChIdx1 = pSubGroupMinBusyTimeChIdxTable[0];
		CandidateCh1 = pAd->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Channel;
        
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("Rule 3 Channel Busy time value : Select Primary Channel %d \n", CandidateCh1));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("Rule 3 Channel Busy time value : Min Channel Busy = %u\n", MinBusyTime));          
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("Rule 3 Channel Busy time value : BW = %s\n", (pAd->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Bw== BW_160) ? "160"
			:(pAd->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Bw== BW_80)? "80" 
			:(pAd->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Bw == BW_40)?"40":"20"));
        
		os_free_mem(pSubGroupMaxBusyTimeTable);
		os_free_mem(pSubGroupMaxBusyTimeChIdxTable);
 		os_free_mem(pSubGroupMinBusyTimeTable);
		os_free_mem(pSubGroupMinBusyTimeChIdxTable);       
        
        goto ReturnCh;
	}	

        base = RandomByte2(pAd);
    
	for (ChannelIdx=0 ; ChannelIdx < pAd->AutoChSelCtrl.ChannelListNum; ChannelIdx++)
	{
		CandidateCh1 = pAd->AutoChSelCtrl.AutoChSelChList[(base + ChannelIdx) % pAd->AutoChSelCtrl.ChannelListNum].Channel;
	
		if (AutoChannelSkipListCheck(pAd, CandidateCh1))
			continue;
		
		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
			&& (pAd->AutoChSelCtrl.IsABand == TRUE)
			&& RadarChannelCheck(pAd, CandidateCh1))
			continue;

		break;
	}

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
    ("Randomly Select : Select Channel %d\n", CandidateCh1));

ReturnCh:    
    
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));

    return CandidateCh1;
    
}

/* 
	==========================================================================
	Description:
        This routine calaulates the dirtyness of all channels by the dirtiness value and 
        number of AP in each channel and stores in pChannelInfo strcut.
		This routine is called at iwpriv cmmand or initialization. It chooses and returns 
		a good channel whith less interference.
	Return:
		ch -  channel number that
	NOTE:
	==========================================================================
 */
static inline UCHAR SelectClearChannelApCnt(RTMP_ADAPTER *pAd)
{
    /*PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab; */

	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	/*BSSENTRY *pBss; */
	UCHAR channel_index = 0,dirty,base = 0;
	UCHAR final_channel = 0;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);	
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);
 
	if(pChannelInfo == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pChannelInfo equal NULL.\n"));
		return (FirstChannel(pAd));
	}
	
	/* Calculate Dirtiness */
	
	for (channel_index=0 ; channel_index < pAd->ChannelListNum ; channel_index++)
	{
		if (pChannelInfo->ApCnt[channel_index] > 0)
	    {
		    INT ll;
		    pChannelInfo->dirtyness[channel_index] += 30;

            /*5G */
		    if (pChannelInfo->IsABand)
		    {
			    int Channel = pAd->ChannelList[channel_index].Channel;
				
			    /*Make secondary channel dirty */
			    if(op_ht_bw == BW_40)
			    {
					if (Channel > 14)
					{
						if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel== 60) 
							|| (Channel == 100) || (Channel == 108) || (Channel == 116) 
							|| (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
						{
							if (channel_index + 1 < MAX_NUM_OF_CHANNELS)
								if(pAd->ChannelList[channel_index+1].Channel - pAd->ChannelList[channel_index].Channel == 4)
									pChannelInfo->dirtyness[channel_index+1] += 1;
						}
						else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || 
							(Channel == 64) || (Channel == 104) || (Channel == 112) ||
								(Channel == 120) || (Channel == 128) || (Channel == 136) || 
								(Channel== 153) || (Channel == 161))
						{
							if(channel_index - 1 >= 0)
								if(pAd->ChannelList[channel_index].Channel - pAd->ChannelList[channel_index-1].Channel == 4)
									pChannelInfo->dirtyness[channel_index-1] += 1;
						}
					}
				}
			}
			/*2.4G */
			if (!pChannelInfo->IsABand)
			{
				int ChanOffset = 0;

				if((op_ht_bw == BW_40)&&
				(ext_cha== EXTCHA_BELOW))
				{
				/*	
					BW is 40Mhz
					the distance between two channel to prevent interference
					is 4 channel width plus 4 channel width (secondary channel)
				*/
					ChanOffset = 8;
				}
				else
				{
				/*
					BW is 20Mhz
					The channel width of 2.4G band is 5Mhz.
					The distance between two channel to prevent interference is 4 channel width
				*/
					ChanOffset = 4;
				}
					
				for (ll = channel_index + 1; ll < (channel_index + ChanOffset + 1); ll++)
				{
					if (ll < MAX_NUM_OF_CHANNELS)
						pChannelInfo->dirtyness[ll]++;
				}

				if((op_ht_bw == BW_40)&&
					(ext_cha == EXTCHA_ABOVE))
				{
					/* BW is 40Mhz */
					ChanOffset = 8;
				}
				else
				{
					/* BW is 20Mhz */
					ChanOffset = 4;
				}

				for (ll = channel_index - 1; ll > (channel_index - ChanOffset - 1); ll--)
				{
					if (ll >= 0 && ll < MAX_NUM_OF_CHANNELS+1)
						pChannelInfo->dirtyness[ll]++;
				}
			}
    	}       
   }/* Calculate Dirtiness */

	AutoChannelSkipListSetDirty(pAd);
	
   MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));
   for (channel_index=0 ; channel_index < pAd->ChannelListNum ; channel_index++)
   /* debug messages */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Channel %d : Dirty = %ld, ApCnt=%ld, Busy Time = %d, Skip Channel = %s\n", 
				pAd->ChannelList[channel_index].Channel,
				pChannelInfo->dirtyness[channel_index], 
				pChannelInfo->ApCnt[channel_index],
#ifdef AP_QLOAD_SUPPORT
				pChannelInfo->chanbusytime[channel_index],
#else
				0,
#endif /* AP_QLOAD_SUPPORT */
				(pChannelInfo->SkipList[channel_index] == TRUE) ? "TRUE" : "FALSE"));
   MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));
   
   pAd->ApCfg.AutoChannel_Channel = 0;
	
	/* RULE 1. pick up a good channel that no one used */
	
	for (channel_index=0 ; channel_index < pAd->ChannelListNum ; channel_index++)
	{
		if (pChannelInfo->SkipList[channel_index] == TRUE)
			continue;
		
	     if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
				&&(pChannelInfo->IsABand == TRUE)
				&& RadarChannelCheck(pAd, pAd->ChannelList[channel_index].Channel))
			continue;	

#ifdef AP_QLOAD_SUPPORT
		/* QLOAD ALARM */
		if (QBSS_LoadIsBusyTimeAccepted(pAd,
			pChannelInfo->chanbusytime[channel_index]) == FALSE)
			continue;
#endif /* AP_QLOAD_SUPPORT */
		 
		if (pChannelInfo->dirtyness[channel_index] == 0) break;
	}
	if (channel_index < pAd->ChannelListNum)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rule 1 APCnt : dirtiness == 0 (no one used and no interference) ==> Select Channel %d\n", pAd->ChannelList[channel_index].Channel));

		return pAd->ChannelList[channel_index].Channel;
	}

	/* RULE 2. if not available, then co-use a channel that's no interference (dirtyness=30) */
	/* RULE 3. if not available, then co-use a channel that has minimum interference (dirtyness=31,32) */
	for (dirty = 30; dirty <= 32; dirty++)
	{
		BOOLEAN candidate[MAX_NUM_OF_CHANNELS+1], candidate_num=0;
		UCHAR min_ApCnt = 255;
		final_channel = 0;	
		
		NdisZeroMemory(candidate, MAX_NUM_OF_CHANNELS+1);
		for (channel_index=0 ; channel_index < pAd->ChannelListNum ; channel_index++)
		{
			if (pChannelInfo->SkipList[channel_index] == TRUE)
				continue;
			
			if (pChannelInfo->dirtyness[channel_index] == dirty) 
			{ 
				candidate[channel_index]=TRUE; 
				candidate_num++; 
			}
		}
		/* if there's more than 1 candidate, pick up the channel with minimum RSSI */
		if (candidate_num)
		{
			for (channel_index=0 ; channel_index < pAd->ChannelListNum ; channel_index++)
			{

#ifdef AP_QLOAD_SUPPORT
				/* QLOAD ALARM */
				/* when busy time of a channel > threshold, skip it */
				/* TODO: Use weight for different references to do channel selection */
				if (QBSS_LoadIsBusyTimeAccepted(pAd,
					pChannelInfo->chanbusytime[channel_index]) == FALSE)
				{
					/* check next one */
					continue;
				}
#endif /* AP_QLOAD_SUPPORT */

				if (candidate[channel_index] && (pChannelInfo->ApCnt[channel_index] < min_ApCnt))
				{

					if((op_ht_bw == BW_40)
						&& (BW40_ChannelCheck(pAd->ChannelList[channel_index].Channel) == FALSE))
						continue;

					if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
							&&(pChannelInfo->IsABand == TRUE)
							&& RadarChannelCheck(pAd, pAd->ChannelList[channel_index].Channel))
						continue;

					final_channel = pAd->ChannelList[channel_index].Channel;
					min_ApCnt = pChannelInfo->ApCnt[channel_index];
				}
			}
			if (final_channel != 0)
			{				
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rule 2 APCnt : minimum APCnt with  minimum interference(dirtiness: 30~32) ==> Select Channel %d\n", final_channel));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" Dirtiness = %d ,  Min ApCnt = %d\n", dirty, min_ApCnt));
				return final_channel;
			}
		}
	}
	/* RULE 3. still not available, pick up the random channel */
	base = RandomByte2(pAd);

	for (channel_index=0 ; channel_index < pAd->ChannelListNum ; channel_index++)
	{		
		final_channel = pAd->ChannelList[(base + channel_index) % pAd->ChannelListNum].Channel;
		
		if (AutoChannelSkipListCheck(pAd, final_channel))
			continue;
		
		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
			&&(pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, final_channel))
				continue;			

		break;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Rule 3 APCnt : Randomly Select  ==> Select Channel %d\n",final_channel));
	return final_channel;
	
}

ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen, 
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi)
{
	ULONG	Idx;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pBssInfoTab equal NULL.\n"));
		return BSS_NOT_FOUND;
	}

	Idx = AutoChBssSearchWithSSID(pAd, pBssid, (PUCHAR)Ssid, SsidLen, ChannelNo);
	if (Idx == BSS_NOT_FOUND) 
	{
		if (pBssInfoTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
			return BSS_NOT_FOUND;
		Idx = pBssInfoTab->BssNr;
		AutoChBssEntrySet(&pBssInfoTab->BssEntry[Idx % MAX_LEN_OF_BSS_TABLE], pBssid, Ssid, SsidLen,
							ChannelNo, ExtChOffset, Rssi);
		pBssInfoTab->BssNr++;
	} 
	else
	{
		AutoChBssEntrySet(&pBssInfoTab->BssEntry[Idx % MAX_LEN_OF_BSS_TABLE], pBssid, Ssid, SsidLen,
							ChannelNo, ExtChOffset, Rssi);
	}

	return Idx;
}

void AutoChBssTableDestroy(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl;
	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	if(!pAutoChCtrl)
	{
		return;
	}
	if (pAutoChCtrl->pBssInfoTab)
	{
		os_free_mem(pAutoChCtrl->pBssInfoTab);
		pAutoChCtrl->pBssInfoTab = NULL;
	}
	return;
}


static VOID AutoChBssTableReset(RTMP_ADAPTER *pAd, UINT8 band)
{
	
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;
	
	if (pBssInfoTab)
		NdisZeroMemory(pBssInfoTab, sizeof(BSSINFO));
	else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("pAutoChCtrl->pBssInfoTab equal NULL.\n"));
	}

	return;
}


void AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd)
{
	BSSINFO *pBssInfoTab = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	
	os_alloc_mem(pAd, (UCHAR **)&pBssInfoTab, sizeof(BSSINFO));
	if (pBssInfoTab) {
		NdisZeroMemory(pBssInfoTab, sizeof(BSSINFO));
		pAutoChCtrl->pBssInfoTab = pBssInfoTab;
	}
	else
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s Fail to alloc memory for pAutoChCtrl->pBssInfoTab", 
					__FUNCTION__));
	}
	return;
}


void ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	
	if (pAutoChCtrl->pChannelInfo)
	{
		os_free_mem( pAutoChCtrl->pChannelInfo);
		pAutoChCtrl->pChannelInfo = NULL;
	}
	return;
}


static VOID ChannelInfoReset(RTMP_ADAPTER *pAd, UINT8 band)
{	
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	CHANNELINFO *ch_info = pAutoChCtrl->pChannelInfo;
	
	if (ch_info)
		NdisZeroMemory(ch_info, sizeof(CHANNELINFO));
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("pChannelInfo equal NULL, band:%d\n", band));

	return;
}


void ChannelInfoInit(
	IN PRTMP_ADAPTER pAd)
{
	CHANNELINFO *ch_info = NULL;	
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	
	os_alloc_mem(pAd, (UCHAR **)&ch_info, sizeof(CHANNELINFO));
	if (ch_info) 
	{
		os_zero_mem(ch_info, sizeof(CHANNELINFO));
		pAutoChCtrl->pChannelInfo = ch_info;
	}
	else
	{
		pAutoChCtrl->pChannelInfo = NULL;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s Fail to alloc memory for pAd->pChannelInfo", __FUNCTION__));
	}
	return ;
}

/* 
	==========================================================================
	Description:
		This routine sets the current PhyMode for calculating 
		the dirtyness.
	Return:
		none
	NOTE:
	==========================================================================
 */
void CheckPhyModeIsABand(RTMP_ADAPTER *pAd)
{
	
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	UCHAR PhyMode = HcGetRadioPhyMode(pAd);
	pAutoChCtrl->pChannelInfo->IsABand = (WMODE_CAP_5G(PhyMode)) ? TRUE : FALSE;

	return;
}


UCHAR SelectBestChannel(RTMP_ADAPTER *pAd, ChannelSel_Alg Alg)
{
	UCHAR ch = 0;

	/* init RadioCtrl.pChannelInfo->IsABand */
	CheckPhyModeIsABand(pAd);
    
#ifdef MICROWAVE_OVEN_SUPPORT
	if (Alg == ChannelAlgCCA)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
#endif /* MICROWAVE_OVEN_SUPPORT */
    
	switch ( Alg )
	{
		case ChannelAlgRandom:
		case ChannelAlgApCnt:
			ch = SelectClearChannelApCnt(pAd);
			break;
		case ChannelAlgCCA:
			ch = SelectClearChannelCCA(pAd);
			break;
		case ChannelAlgBusyTime:
			ch = SelectClearChannelBusyTime(pAd);
			break;	
		default:
			ch = SelectClearChannelBusyTime(pAd);
			break;
	}

	RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, ch);

	return ch;

}

VOID APAutoChannelInit(RTMP_ADAPTER *pAd)
{
	UINT32 BusyTime; 

	/* reset bss table */
	AutoChBssTableReset(pAd, 0);

	/* clear Channel Info */
	ChannelInfoReset(pAd, 0);

	/* init RadioCtrl.pChannelInfo->IsABand */
	CheckPhyModeIsABand(pAd);

	pAd->ApCfg.current_channel_index = 0;
 		
	/* read clear for primary channel */
	BusyTime = AsicGetChBusyCnt(pAd, 0);
}

/* 
	==========================================================================
	Description:
		This routine is called at initialization. It returns a channel number
		that complies to regulation domain and less interference with current
		enviornment.
	Return:
		ch -  channel number that
	NOTE:
		The retruned channel number is guaranteed to comply to current regulation
		domain that recorded in pAd->CommonCfg.CountryRegion
        Usage: 
               1.) iwpriv ra0 set AutoChannelSel=1
                   Ues the number of AP and inference status to choose
               2.) iwpriv ra0 set AutoChannelSel=2
                   Ues the False CCA count and Rssi to choose
	==========================================================================
 */
UCHAR APAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand)
{
	UCHAR ch = 0;
    
	if (pAd->phy_op && pAd->phy_op->AutoCh)
		ch = pAd->phy_op->AutoCh(pAd, Alg, IsABand);	
		
	return ch;
}

UCHAR MTAPAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand)
{

	UCHAR ch = 0, i, count = 0;    
	UINT32 BusyTime;

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

	/* Init some structures before doing AutoChannelSelect() */
	APAutoChannelInit(pAd);

	if (( Alg == ChannelAlgRandom ) && (IsABand== TRUE))
	{   /*for Dfs */
		ch = SelectClearChannelRandom(pAd);
	}
	else
	{
#ifdef MICROWAVE_OVEN_SUPPORT
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
		AsicMeasureFalseCCA(pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

               /* Re-arrange channel list and fill in channel properties for auto-channel selection*/
		AutoChSelBuildChannelList(pAd, IsABand);

	       if (IsABand)
	       {
	               pAd->AutoChSelCtrl.IsABand = TRUE;
	       		for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum5G; i++)
	       		{
	       			if ((pAd->AutoChSelCtrl.AutoChSel5GChList[i].SkipChannel == TRUE)
				 	||(pAd->AutoChSelCtrl.AutoChSel5GChList[i].BwCap== FALSE))
                {   
				 	continue;
                }    
				else
				{
					pAd->AutoChSelCtrl.AutoChSelChList[count].Channel = pAd->AutoChSelCtrl.AutoChSel5GChList[i].Channel;
					pAd->AutoChSelCtrl.AutoChSelChList[count].Bw = pAd->AutoChSelCtrl.AutoChSel5GChList[i].Bw;
					pAd->AutoChSelCtrl.AutoChSelChList[count].BwCap = pAd->AutoChSelCtrl.AutoChSel5GChList[i].BwCap;
					pAd->AutoChSelCtrl.AutoChSelChList[count].CentralChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[i].CentralChannel;
					pAd->AutoChSelCtrl.AutoChSelChList[count].SkipChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[i].SkipChannel;
					pAd->AutoChSelCtrl.AutoChSelChList[count].Flags = pAd->AutoChSelCtrl.AutoChSel5GChList[i].Flags;
					count++;	
				}		
	       		}
		   	pAd->AutoChSelCtrl.ChannelListNum = count;
	       	}
	       else
	       {
	               pAd->AutoChSelCtrl.IsABand = FALSE; 
			for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum2G; i++)
	       		{
	       			if ((pAd->AutoChSelCtrl.AutoChSel2GChList[i].SkipChannel == TRUE)
				 	||(pAd->AutoChSelCtrl.AutoChSel2GChList[i].BwCap== FALSE))
                {   
				 	continue;
                }    
				else
				{
					pAd->AutoChSelCtrl.AutoChSelChList[count].Channel = pAd->AutoChSelCtrl.AutoChSel2GChList[i].Channel;
					pAd->AutoChSelCtrl.AutoChSelChList[count].Bw = pAd->AutoChSelCtrl.AutoChSel2GChList[i].Bw;
					pAd->AutoChSelCtrl.AutoChSelChList[count].BwCap = pAd->AutoChSelCtrl.AutoChSel2GChList[i].BwCap;
					pAd->AutoChSelCtrl.AutoChSelChList[count].CentralChannel = pAd->AutoChSelCtrl.AutoChSel2GChList[i].CentralChannel;
					pAd->AutoChSelCtrl.AutoChSelChList[count].SkipChannel = pAd->AutoChSelCtrl.AutoChSel2GChList[i].SkipChannel;
					pAd->AutoChSelCtrl.AutoChSelChList[count].Flags = pAd->AutoChSelCtrl.AutoChSel2GChList[i].Flags;
					count++;	
				}		
	       		}
		   	pAd->AutoChSelCtrl.ChannelListNum = count;
	       	}
           
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: IsABand = %d, ChannelListNum = %d\n", __FUNCTION__, IsABand, pAd->AutoChSelCtrl.ChannelListNum));  

		for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum; i++)
		{
			ULONG wait_time = 200; /* Wait for 200 ms at each channel. */

			AutoChSelSwitchChannel(pAd, &pAd->AutoChSelCtrl.AutoChSelChList[i], TRUE);
            
			AsicLockChannel(pAd, pAd->AutoChSelCtrl.AutoChSelChList[i].Channel);/*Do nothing */
            
			pAd->ApCfg.current_channel_index = i;

			pAd->ApCfg.AutoChannel_Channel = pAd->AutoChSelCtrl.AutoChSelChList[i].Channel;
			
			/* Read-Clear reset Channel busy time counter */
			BusyTime = AsicGetChBusyCnt(pAd, 0);
            
#ifdef AP_QLOAD_SUPPORT
			/* QLOAD ALARM, ever alarm from QLOAD module */
			if (QLOAD_DOES_ALARM_OCCUR(pAd))
				wait_time = 400;
#endif /* AP_QLOAD_SUPPORT */

			OS_WAIT(wait_time);

			UpdateChannelInfo(pAd, i,Alg);
		}

		ch = SelectBestChannel(pAd, Alg);

        }
    
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));

	return ch;
}

UCHAR RLTAPAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand)    
{
	UCHAR ch = 0, i;
	UINT32	BusyTime;
	UCHAR	BandIdx = 0;    

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));
  
	/* passive scan channel 1-14. collect statistics */
	
	/*
		In the autochannel select case. AP didn't get channel yet.
		So have no way to determine which Band AP used by channel number.
	*/

        bbp_set_bw(pAd,BW_20,BandIdx);  
	/* Init some structures before doing AutoChannelSelect() */
	APAutoChannelInit(pAd);

	if (( Alg == ChannelAlgRandom ) && (IsABand== TRUE))
	{   /*for Dfs */
		ch = SelectClearChannelRandom(pAd);
	}
	else
	{

#ifdef MICROWAVE_OVEN_SUPPORT
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
		AsicMeasureFalseCCA(pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

		/*find RSSI in each channel */
		for (i=0; i<pAd->ChannelListNum; i++)
		{
			ULONG wait_time = 200; /* wait for 200 ms at each channel. */

			AsicSwitchChannel(pAd, pAd->ChannelList[i].Channel, TRUE);
            
			AsicLockChannel(pAd, pAd->ChannelList[i].Channel);/*do nothing */
            
			pAd->ApCfg.current_channel_index = i;

			pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[i].Channel;
			
			/* Read-Clear reset Channel busy time counter */
			BusyTime = AsicGetChBusyCnt(pAd, 0);
#ifdef AP_QLOAD_SUPPORT
			/* QLOAD ALARM, ever alarm from QLOAD module */
			if (QLOAD_DOES_ALARM_OCCUR(pAd))
				wait_time = 400;
#endif /* AP_QLOAD_SUPPORT */
			OS_WAIT(wait_time);

			UpdateChannelInfo(pAd, i,Alg);
		}

		ch = SelectBestChannel(pAd, Alg);
	}
		
	return ch;
}

UCHAR RTMPAPAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand)     
{
	UCHAR ch = 0, i;
	UINT32	BusyTime;
	UCHAR	BandIdx = 0;    

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));   
	/* passive scan channel 1-14. collect statistics */
	
	/*
		In the autochannel select case. AP didn't get channel yet.
		So have no way to determine which Band AP used by channel number.
	*/
	
        bbp_set_bw(pAd,BW_20,BandIdx);   
	/* Init some structures before doing AutoChannelSelect() */
	APAutoChannelInit(pAd);

	if (( Alg == ChannelAlgRandom ) && (IsABand== TRUE))
	{   /*for Dfs */
		ch = SelectClearChannelRandom(pAd);
	}
	else
	{

#ifdef MICROWAVE_OVEN_SUPPORT
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
		AsicMeasureFalseCCA(pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

		/*find RSSI in each channel */
		for (i=0; i<pAd->ChannelListNum; i++)
		{
			ULONG wait_time = 200; /* wait for 200 ms at each channel. */

			AsicSwitchChannel(pAd, pAd->ChannelList[i].Channel, TRUE);
			AsicLockChannel(pAd, pAd->ChannelList[i].Channel);/*do nothing */
			pAd->ApCfg.current_channel_index = i;

			pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[i].Channel;
			
			/* Read-Clear reset Channel busy time counter */
			BusyTime = AsicGetChBusyCnt(pAd, 0);
#ifdef AP_QLOAD_SUPPORT
			/* QLOAD ALARM, ever alarm from QLOAD module */
			if (QLOAD_DOES_ALARM_OCCUR(pAd))
				wait_time = 400;
#endif /* AP_QLOAD_SUPPORT */
			OS_WAIT(wait_time);

			UpdateChannelInfo(pAd, i,Alg);
		}

		ch = SelectBestChannel(pAd, Alg);
	}
		
	return ch;
}

/*
   ==========================================================================
   Description:
       Update channel for wdev which is supported for A-band or G-band.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelUpdateChannel(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN BOOLEAN IsABand)
{
	INT32 i;
	UINT8 ExtChaDir;
	struct wifi_dev *pwdev;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pwdev = &pAd->ApCfg.MBSSID[i].wdev;

		if (!pwdev) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("\x1b[41m %s(): wdev%d is NULL !!\x1b[m \n", __FUNCTION__, i));
			continue;
		} else {
			if (IsABand) {
				if (WMODE_CAP_5G(pwdev->PhyMode))
					pwdev->channel = Channel;
			} else {
				if (WMODE_CAP_2G(pwdev->PhyMode)) {
					/* Update primary channel in wdev */
					pwdev->channel = Channel;
					/* Query ext_cha in wdev */
					ExtChaDir = wlan_config_get_ext_cha(pwdev);

					/* Check current extension channel */
					if (!ExtChCheck(pAd, Channel, ExtChaDir)) {
						if (ExtChaDir == EXTCHA_BELOW)
							ExtChaDir = EXTCHA_ABOVE;
						else
							ExtChaDir = EXTCHA_BELOW;

						/* Update ext_cha in wdev */
						wlan_config_set_ext_cha(pwdev, ExtChaDir);
					}
				}
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("\x1b[41m %s(): Update channel for wdev%d for this band PhyMode = %d,Channel = %d \x1b[m \n",
		__FUNCTION__, i, pwdev->PhyMode, pwdev->channel));
	}
}

/*
   ==========================================================================
   Description:
       Switch channel for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelSwitchChannel(
    IN RTMP_ADAPTER *pAd, 
    IN PAUTOCH_SEL_CH_LIST pAutoChSelChList, 
    IN BOOLEAN bScan)
{
    #define SINGLE_BAND   0
	UINT8 RxStream;
	UINT8 TxStream;
	MT_SWITCH_CHANNEL_CFG SwChCfg;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

    os_zero_mem(&SwChCfg,sizeof(MT_SWITCH_CHANNEL_CFG));
    
	SwChCfg.Bw = pAutoChSelChList->Bw;
	SwChCfg.bScan = bScan;
	SwChCfg.CentralChannel = pAutoChSelChList->CentralChannel;
    
	if(pAd->CommonCfg.dbdc_mode == SINGLE_BAND)
    {   
		SwChCfg.BandIdx = 0;
		TxStream = pAd->Antenna.field.TxPath;
		RxStream = pAd->Antenna.field.RxPath;
    } 
	else if(pAd->AutoChSelCtrl.IsABand)
    {   
		SwChCfg.BandIdx = 1;
		TxStream = pAd->dbdc_5G_tx_stream;
		RxStream = pAd->dbdc_5G_rx_stream;
    }    
	else
    {   
		SwChCfg.BandIdx = 0;
		TxStream = pAd->dbdc_2G_tx_stream;
		RxStream = pAd->dbdc_2G_rx_stream;
    }    

	SwChCfg.RxStream = RxStream;
	SwChCfg.TxStream = TxStream;
	SwChCfg.ControlChannel = pAutoChSelChList->Channel;
    
#ifdef DOT11_VHT_AC
	SwChCfg.ControlChannel2 = (SwChCfg.Bw == BW_8080)?pAd->CommonCfg.vht_cent_ch2 : 0;
#endif /* DOT11_VHT_AC */

#ifdef MT_DFS_SUPPORT 
    SwChCfg.bDfsCheck = DfsSwitchCheck(pAd, SwChCfg.ControlChannel); 		
#endif

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
	("%s : Scan:%d, CenChannel:%d, BandIdx:%d, RxStream:%d, TxStream:%d, Bw:%d, ControlChannel:%d, ControlChannel2:%d\n",
	__FUNCTION__, SwChCfg.bScan, SwChCfg.CentralChannel, SwChCfg.BandIdx, SwChCfg.RxStream, SwChCfg.TxStream, SwChCfg.Bw, SwChCfg.ControlChannel, SwChCfg.ControlChannel2));

    MtAsicSwitchChannel(pAd,SwChCfg);
    
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
} 

/*
   ==========================================================================
   Description:
       Build channel list for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelList(
    IN RTMP_ADAPTER *pAd, 
    IN BOOLEAN IsABand)
{
    #define EXT_ABOVE     1
    #define EXT_BELOW    -1       
    INT  i, j;
#ifdef DOT11_VHT_AC   
    INT k, count, idx;   
    struct vht_ch_layout * vht_ch_80M = get_ch_array(80);
    struct vht_ch_layout * vht_ch_160M = get_ch_array(160);
#endif/* DOT11_VHT_AC */    
	struct wifi_dev *pwdev;
	UCHAR cfg_ht_bw;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

	for (i = 0; i < pAd->ApCfg.BssidNum; i++)
	{
	    pwdev = &pAd->ApCfg.MBSSID[i].wdev;
		cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
		
	    if (IsABand && WMODE_CAP_5G(pwdev->PhyMode))
	    {
	        os_zero_mem(pAd->AutoChSelCtrl.AutoChSel5GChList, MAX_NUM_OF_CHANNELS * sizeof(AUTOCH_SEL_CH_LIST));
	        
	        AutoChSelBuildChannelListFor5G(pAd);
		        
			/*Check for skip-channel list*/
	        for (j = 0 ; j < pAd->AutoChSelCtrl.ChannelListNum5G; j++)
			{
				pAd->AutoChSelCtrl.AutoChSel5GChList[j].SkipChannel = AutoChannelSkipListCheck(pAd, pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel);
#ifdef BACKGROUND_SCAN_SUPPORT			
				if (pAd->BgndScanCtrl.SkipDfsChannel) {
					pAd->AutoChSelCtrl.AutoChSel5GChList[j].SkipChannel = RadarChannelCheck(pAd, pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel);
				}
#endif /* BACKGROUND_SCAN_SUPPORT */			
				
			}
	        
			for (j = 0 ; j < pAd->AutoChSelCtrl.ChannelListNum5G; j++)
			{
				if (cfg_ht_bw == BW_20)
				{
	                pAd->AutoChSelCtrl.AutoChSel5GChList[j].Bw = BW_20;
	                pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap = TRUE;
	                pAd->AutoChSelCtrl.AutoChSel5GChList[j].CentralChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel;
				}
	            
#ifdef DOT11_N_SUPPORT                                                        
				else if (((cfg_ht_bw == BW_40) 
#ifdef DOT11_VHT_AC                                                 
	                       &&(pAd->CommonCfg.vht_bw == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
	                      )&& N_ChannelGroupCheck(pAd, pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel))
				{
					 pAd->AutoChSelCtrl.AutoChSel5GChList[j].Bw = BW_40;
	                 
					 /*Check that if there is a secondary channel in current BW40-channel group for BW40 capacity.*/
					 if ((GetABandChOffset(pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel) == EXT_ABOVE)
				 	     && (pAd->AutoChSelCtrl.AutoChSel5GChList[j+1].Channel == (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel + 4)))
					 {
					     pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap = TRUE;
					 }
					else if ((GetABandChOffset(pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel) == EXT_BELOW)
					      && (pAd->AutoChSelCtrl.AutoChSel5GChList[j - 1].Channel == (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel - 4)))
					 {
					     pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap = TRUE;
					 }
					 else
					 {
					     pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap = FALSE;
					 }
					 
					 /*Check that whether there is a skip-channel in current BW40-channel group*/
					 /*If there is a skip-channel in BW40-channel group, just also skip secondary channel*/
					 if (pAd->AutoChSelCtrl.AutoChSel5GChList[j].SkipChannel == TRUE)
					 {
					     if ((GetABandChOffset(pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel) == EXT_ABOVE)
						      && (pAd->AutoChSelCtrl.AutoChSel5GChList[j+1].Channel == (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel + 4)))
					     {
						     pAd->AutoChSelCtrl.AutoChSel5GChList[j+1].SkipChannel = TRUE;
					     }	
					     else if ((GetABandChOffset(pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel) == EXT_BELOW)
					           && (pAd->AutoChSelCtrl.AutoChSel5GChList[j - 1].Channel == (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel - 4)))
					     {
							 pAd->AutoChSelCtrl.AutoChSel5GChList[j-1].SkipChannel = TRUE;
						 }	
					 }
	                 
	                 /*Fill in central-channel parameter*/ 
					 if (GetABandChOffset(pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel) == EXT_ABOVE)
	                 {            
					     pAd->AutoChSelCtrl.AutoChSel5GChList[j].CentralChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel+ 2;
	                 }
	                 else
	                 {
					     pAd->AutoChSelCtrl.AutoChSel5GChList[j].CentralChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel- 2;
	                 }    
				}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC    
				else if (((pAd->CommonCfg.vht_bw == VHT_BW_80) || (pAd->CommonCfg.vht_bw == VHT_BW_8080))  
						   && vht80_channel_group(pAd, pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel))
				{
					 pAd->AutoChSelCtrl.AutoChSel5GChList[j].Bw = BW_80;
					 idx = 0;
					 count = 0 ;
	                 
					 /*Find out VHT BW80 channel group for current channel*/
					 while (vht_ch_80M[idx].ch_up_bnd != 0)
					 {
					     if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
							 (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel <= vht_ch_80M[idx].ch_up_bnd))
					     {
					         break;
					     }
					     idx++;
					 }
	                 
					 if (vht_ch_80M[idx].ch_up_bnd != 0)
					 {
					     /*Count for secondary channels in current VHT BW80 channel group*/
					     for (k = 1; k < 4; k++)
					     {
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel <= vht_ch_80M[idx].ch_up_bnd))
					         {
					             count++;
					         }	
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel <= vht_ch_80M[idx].ch_up_bnd))
					         {           
					             count++;	
					         }
					     }
					     if(count == 3)
					     {
					         pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap = TRUE;
					     }
					 }	
	                 
					 /*Check that whether there is a skip-channel in BW80-channel group*/
					 /*If there is a skip-channel in BW80-channel group, just also skip secondary channels*/
					 if (pAd->AutoChSelCtrl.AutoChSel5GChList[j].SkipChannel == TRUE)
					 {
					     for (k = 1; k < 4; k++)
					     {
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel <= vht_ch_80M[idx].ch_up_bnd))
					         {
					             pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].SkipChannel = TRUE;
					         }	
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel <= vht_ch_80M[idx].ch_up_bnd))
					         {           
					             pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].SkipChannel = TRUE;
					         }
					     }
					 }	   
					 pAd->AutoChSelCtrl.AutoChSel5GChList[j].CentralChannel = vht_cent_ch_freq (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel, VHT_BW_80);                                       	 
				}
	                              
				else if ((pAd->CommonCfg.vht_bw == VHT_BW_160)
	                      && vht80_channel_group(pAd, pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel))
				{
					 pAd->AutoChSelCtrl.AutoChSel5GChList[j].Bw = BW_160;
					 idx = 0;
					 count = 0 ;
	                 
					 /*Find out VHT BW160 channel group for current channel*/
					 while (vht_ch_160M[idx].ch_up_bnd != 0)
					 {
					     if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
							 (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel <= vht_ch_160M[idx].ch_up_bnd))
					     {
					         break;
					     }
					     idx++;
					 }
					 if (vht_ch_160M[idx].ch_up_bnd != 0)
					 {
					     /*Count for secondary channels in current VHT BW160 channel group*/
					     for (k = 1; k < 8; k++)
					     {
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel <= vht_ch_160M[idx].ch_up_bnd))
					         {
					             count++;
					         }	
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
								     (pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel <= vht_ch_160M[idx].ch_up_bnd))
					         {           
					             count++;	
					         }
					     }
					     if(count == 7)
					     {
					         pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap = TRUE;
					     }
					 }
	                 
					 /*Check that whether there is a skip-channel in BW160-channel group*/
					 /*If there is a skip-channel in BW160-channel group, just also skip secondary channels*/
					 if (pAd->AutoChSelCtrl.AutoChSel5GChList[j].SkipChannel == TRUE)
					 {
					     for (k = 1; k < 8; k++)
					     {
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].Channel <= vht_ch_160M[idx].ch_up_bnd))
					         {
					             pAd->AutoChSelCtrl.AutoChSel5GChList[j+k].SkipChannel = TRUE;
					         }	
					         if ((pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
								 (pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].Channel <= vht_ch_160M[idx].ch_up_bnd))
					         {           
					             pAd->AutoChSelCtrl.AutoChSel5GChList[j-k].SkipChannel = TRUE;
					         }
					     }
					 }
					 pAd->AutoChSelCtrl.AutoChSel5GChList[j].CentralChannel = vht_cent_ch_freq (pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel, VHT_BW_160); 
				}
#endif /* DOT11_VHT_AC */
			}

			for (j = 0 ; j < pAd->AutoChSelCtrl.ChannelListNum5G; j++)
			{
	            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	            ("%s:  PrimChannel =  %3d, CenChannel = %3d, BW= %d, BwCap= %d, SkipChannel= %d\n",__FUNCTION__,
	            pAd->AutoChSelCtrl.AutoChSel5GChList[j].Channel, pAd->AutoChSelCtrl.AutoChSel5GChList[j].CentralChannel,
	            pAd->AutoChSelCtrl.AutoChSel5GChList[j].Bw, pAd->AutoChSelCtrl.AutoChSel5GChList[j].BwCap,
	            pAd->AutoChSelCtrl.AutoChSel5GChList[j].SkipChannel));
			}
			break;
	    } 
	    else if (WMODE_CAP_2G(pwdev->PhyMode))
	    {
	        os_zero_mem(pAd->AutoChSelCtrl.AutoChSel2GChList, MAX_NUM_OF_CHANNELS * sizeof(AUTOCH_SEL_CH_LIST));

	        AutoChSelBuildChannelListFor2G(pAd);
		        
	        /*Check for skip-channel list*/
	        for (j = 0 ; j < pAd->AutoChSelCtrl.ChannelListNum2G; j++)
	        {
	            pAd->AutoChSelCtrl.AutoChSel2GChList[j].SkipChannel = AutoChannelSkipListCheck(pAd, pAd->AutoChSelCtrl.AutoChSel2GChList[j].Channel);
	        }	
	        
	        for (j = 0 ; j < pAd->AutoChSelCtrl.ChannelListNum2G; j++)
	        {
	            /*2.4G only support BW20 for auto-channel selection*/
	            pAd->AutoChSelCtrl.AutoChSel2GChList[j].Bw = BW_20;
	            pAd->AutoChSelCtrl.AutoChSel2GChList[j].CentralChannel = pAd->AutoChSelCtrl.AutoChSel2GChList[j].Channel;

	            if (cfg_ht_bw == BW_20)
	                pAd->AutoChSelCtrl.AutoChSel2GChList[j].BwCap = TRUE;

#ifdef DOT11_N_SUPPORT
	            else if ((cfg_ht_bw == BW_40)
	                   && N_ChannelGroupCheck(pAd, pAd->AutoChSelCtrl.AutoChSel2GChList[j].Channel))
	            {	            
	                pAd->AutoChSelCtrl.AutoChSel2GChList[j].BwCap = TRUE;
	            }
#endif /* DOT11_N_SUPPORT */
	            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	            ("%s:  PrimChannel =  %3d, CenChannel = %3d, BW= %d, BwCap= %d, SkipChannel= %d\n",__FUNCTION__,
	            pAd->AutoChSelCtrl.AutoChSel2GChList[j].Channel, pAd->AutoChSelCtrl.AutoChSel2GChList[j].CentralChannel,
	            pAd->AutoChSelCtrl.AutoChSel2GChList[j].Bw, pAd->AutoChSelCtrl.AutoChSel2GChList[j].BwCap,
	            pAd->AutoChSelCtrl.AutoChSel2GChList[j].SkipChannel));
	        }	
			break;
	    }
	}
	
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}
		
/*
   ==========================================================================
   Description:
       Build channel list for 2.4G according to 1) Country Region 2) RF IC type for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelListFor2G(
    IN RTMP_ADAPTER *pAd)
{
    INT i = 0;
        
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

    for (i = 0; i <= pAd->AutoChSelCtrl.ChannelListNum2G; i++)
    {
        pAd->AutoChSelCtrl.AutoChSel2GChList[i].Channel = pAd->ChannelList[i].Channel;
    }

    for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum2G; i++)
    {         
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s : Ch = %3d\n", __FUNCTION__, pAd->AutoChSelCtrl.AutoChSel2GChList[i].Channel));
    } 

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
   ==========================================================================
   Description:
       Build channel list for 5G according to 1) Country Region 2) RF IC type for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelListFor5G(
    IN RTMP_ADAPTER *pAd)
{
    UCHAR StartChIdx5G = 0;
    INT i = 0;
    INT j = 0;    
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

    if (pAd->AutoChSelCtrl.ChannelListNum2G)
    {
        StartChIdx5G = pAd->AutoChSelCtrl.ChannelListNum2G;
    }
        
    for (i = StartChIdx5G; i < pAd->ChannelListNum; i++)
    {
        //pAd->AutoChSelCtrl.AutoChSel5GChList[i-StartChIdx5G].Channel = pAd->ChannelList[i].Channel;
        if(CheckNonOccupancyChannel(pAd, pAd->ChannelList[i].Channel))
		    pAd->AutoChSelCtrl.AutoChSel5GChList[j++].Channel = pAd->ChannelList[i].Channel;   	
    }

    //pAd->AutoChSelCtrl.ChannelListNum5G = pAd->ChannelListNum - pAd->AutoChSelCtrl.ChannelListNum2G;
	pAd->AutoChSelCtrl.ChannelListNum5G = j;

    for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum5G; i++)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s : Ch = %3d\n",__FUNCTION__, pAd->AutoChSelCtrl.AutoChSel5GChList[i].Channel));
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/* 
	==========================================================================
	Description:

	Return:
		ScanChIdx - Channel index which is mapping to related channel to be scanned.
	Note:
		return -1 if no more next channel
	==========================================================================
 */
CHAR AutoChSelFindScanChIdx(
	IN RTMP_ADAPTER *pAd, 
	IN CHAR LastScanChIdx)
{
	CHAR ScanChIdx = -1;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));
    
	if (LastScanChIdx == -1)
    {   
		ScanChIdx = 0;
    }    
	else
    {   
		ScanChIdx = LastScanChIdx + 1;
        
        if (ScanChIdx >= pAd->AutoChSelCtrl.ChannelListNum)
        {
            ScanChIdx = -1;
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
    ("%s : LastScanChIdx = %d, ScanChIdx = %d, ChannelListNum = %d\n"
    ,__FUNCTION__, LastScanChIdx, ScanChIdx, pAd->AutoChSelCtrl.ChannelListNum));
    
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
    
	return ScanChIdx;
}

/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID AutoChSelScanNextChannel(
    IN RTMP_ADAPTER *pAd,
    IN struct wifi_dev *pwdev)
{
    RALINK_TIMER_STRUCT *ScanTimer = &pAd->AutoChSelCtrl.AutoChScanTimer;
    CHAR Idx = pAd->AutoChSelCtrl.ScanChIdx;
    UINT32 BusyTime;
    ULONG wait_time = 200; /* Wait for 200 ms at each channel. */
    UCHAR NewCh;
    INT ret;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));
    
#ifdef AP_QLOAD_SUPPORT
	/* QLOAD ALARM, ever alarm from QLOAD module */
	if (QLOAD_DOES_ALARM_OCCUR(pAd))
    {   
		wait_time = 400;
    }    
#endif /* AP_QLOAD_SUPPORT */
    
    if (pAd->AutoChSelCtrl.ScanChIdx == -1)
    {
		NewCh = SelectClearChannelBusyTime(pAd);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s : Current channel = %d , selected new channel = %d\n",__FUNCTION__, pwdev->channel, NewCh));   

#ifdef AP_SCAN_SUPPORT        
		scan_ch_restore(pAd, OPMODE_AP, pwdev); /* Restore original channel */ 
#endif /* AP_SCAN_SUPPORT */

		if (NewCh != pwdev->channel)
		{
			BOOLEAN IsABand = pAd->AutoChSelCtrl.IsABand;
				
			AutoChSelUpdateChannel(pAd, NewCh, IsABand);
			
		    ret = rtmp_set_channel(pAd, pwdev, NewCh);
		    if (!ret)
		    {
		        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		        ("%s : Fail to set channel !! \n",__FUNCTION__));    
		    }
		}

		/* Update current state from listen state to idle. */
		pAd->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
    }
    else
    {
		/* Update current state from idle state to listen. */
		pAd->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_LISTEN;
		    
		AutoChSelSwitchChannel(pAd, &pAd->AutoChSelCtrl.AutoChSelChList[Idx], TRUE);
		    
		AsicLockChannel(pAd, pAd->AutoChSelCtrl.AutoChSelChList[Idx].Channel);/* Do nothing */

		/* Read-Clear reset Channel busy time counter */
		BusyTime = AsicGetChBusyCnt(pAd, 0);

		RTMPSetTimer(ScanTimer, wait_time); 
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
    ==========================================================================
    Description:
        Auto-channel selection SCAN req state machine procedure
    ==========================================================================
 */
VOID AutoChSelScanReqAction(
    IN RTMP_ADAPTER *pAd, 
    IN MLME_QUEUE_ELEM *pElem)
{
    BOOLEAN	Cancelled;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

    RTMPCancelTimer(&pAd->AutoChSelCtrl.AutoChScanTimer, &Cancelled);

    AutoChSelScanNextChannel(pAd, pAd->AutoChSelCtrl.pScanReqwdev);
        
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
    ==========================================================================
    Description:
        Auto-channel selection SCAN timeout state machine procedure
    ==========================================================================
 */
VOID AutoChSelScanTimeoutAction(
    IN RTMP_ADAPTER *pAd, 
    IN MLME_QUEUE_ELEM *pElem)
{
    CHAR Idx = pAd->AutoChSelCtrl.ScanChIdx;
    
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));

    UpdateChannelInfo(pAd, Idx, ChannelAlgBusyTime);

    pAd->AutoChSelCtrl.ScanChIdx = AutoChSelFindScanChIdx(pAd, Idx);
    
    AutoChSelScanNextChannel(pAd, pAd->AutoChSelCtrl.pScanReqwdev);
    
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
    ==========================================================================
    Description:
        Scan start handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelScanStart(
    IN RTMP_ADAPTER *pAd,
    IN struct wifi_dev *pwdev)
{	
    UCHAR i, count = 0;
    
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));
    
    pAd->AutoChSelCtrl.ScanChIdx = 0; /* Start from first channel */

    pAd->AutoChSelCtrl.pScanReqwdev = pwdev;

    if (WMODE_CAP_5G(pwdev->PhyMode))
	{
	    pAd->AutoChSelCtrl.IsABand = TRUE;

		if (pAd->AutoChSelCtrl.ChannelListNum5G == 0)
			AutoChSelBuildChannelList(pAd, pAd->AutoChSelCtrl.IsABand);
        
	    for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum5G; i++)
	    {
	        if ((pAd->AutoChSelCtrl.AutoChSel5GChList[i].SkipChannel == TRUE)
				||(pAd->AutoChSelCtrl.AutoChSel5GChList[i].BwCap== FALSE))
	        {   
			    continue;
	        }    
	        else
	        {
			    pAd->AutoChSelCtrl.AutoChSelChList[count].Channel = pAd->AutoChSelCtrl.AutoChSel5GChList[i].Channel;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].Bw = pAd->AutoChSelCtrl.AutoChSel5GChList[i].Bw;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].BwCap = pAd->AutoChSelCtrl.AutoChSel5GChList[i].BwCap;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].CentralChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[i].CentralChannel;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].SkipChannel = pAd->AutoChSelCtrl.AutoChSel5GChList[i].SkipChannel;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].Flags = pAd->AutoChSelCtrl.AutoChSel5GChList[i].Flags;
			    count++;	
	        }		
	    }
	    pAd->AutoChSelCtrl.ChannelListNum = count;
	}
	else
	{
	    pAd->AutoChSelCtrl.IsABand = FALSE; 

		if (pAd->AutoChSelCtrl.ChannelListNum2G == 0)
			AutoChSelBuildChannelList(pAd, pAd->AutoChSelCtrl.IsABand);
		
	    for (i=0; i < pAd->AutoChSelCtrl.ChannelListNum2G; i++)
	    {
	        if ((pAd->AutoChSelCtrl.AutoChSel2GChList[i].SkipChannel == TRUE)
				||(pAd->AutoChSelCtrl.AutoChSel2GChList[i].BwCap== FALSE))
	        {   
				 	continue;
	        }    
	        else
	        {
			    pAd->AutoChSelCtrl.AutoChSelChList[count].Channel = pAd->AutoChSelCtrl.AutoChSel2GChList[i].Channel;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].Bw = pAd->AutoChSelCtrl.AutoChSel2GChList[i].Bw;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].BwCap = pAd->AutoChSelCtrl.AutoChSel2GChList[i].BwCap;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].CentralChannel = pAd->AutoChSelCtrl.AutoChSel2GChList[i].CentralChannel;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].SkipChannel = pAd->AutoChSelCtrl.AutoChSel2GChList[i].SkipChannel;
			    pAd->AutoChSelCtrl.AutoChSelChList[count].Flags = pAd->AutoChSelCtrl.AutoChSel2GChList[i].Flags;
			    count++;	
	        }		
	    }
        
	    pAd->AutoChSelCtrl.ChannelListNum = count;
	}
           
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
	("%s: IsABand = %d, ChannelListNum = %d\n", __FUNCTION__, pAd->AutoChSelCtrl.IsABand, pAd->AutoChSelCtrl.ChannelListNum)); 
    
	MlmeEnqueue(pAd, AUTO_CH_SEL_STATE_MACHINE, AUTO_CH_SEL_SCAN_REQ, 0, NULL, 0);

    RTMP_MLME_HANDLER(pAd);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
    ==========================================================================
    Description:
        Scan timeout handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));
    
	MlmeEnqueue(pAd, AUTO_CH_SEL_STATE_MACHINE, AUTO_CH_SEL_SCAN_TIMEOUT, 0, NULL, 0);

    RTMP_MLME_HANDLER(pAd);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
   ==========================================================================
   Description:
        Auto-channel selection state machine.
   Parameters:
		Sm - pointer to the state machine 
   NOTE:
        The state machine is classified as follows:
        a. AUTO_CH_SEL_SCAN_IDLE
        b. AUTO_CH_SEL_SCAN_LISTEN
   ==========================================================================
 */
VOID AutoChSelStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s----------------->\n",__FUNCTION__));
    
    StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AUTO_CH_SEL_SCAN_MAX_STATE, AUTO_CH_SEL_SCAN_MAX_MSG,
                    (STATE_MACHINE_FUNC)Drop, AUTO_CH_SEL_SCAN_IDLE, AUTO_CH_SEL_MACHINE_BASE);

    /* Scan idle state */
    StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_IDLE, AUTO_CH_SEL_SCAN_REQ, (STATE_MACHINE_FUNC)AutoChSelScanReqAction);

    /* Scan listen state */
	StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_LISTEN, AUTO_CH_SEL_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)AutoChSelScanTimeoutAction);

    RTMPInitTimer(pAd, &pAd->AutoChSelCtrl.AutoChScanTimer, GET_TIMER_FUNCTION(AutoChSelScanTimeout), pAd, FALSE);
    
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<-----------------\n",__FUNCTION__));
}

/*
   ==========================================================================
   Description:
        Init for auto-channel selection scan-timer.
   NOTE:
   ==========================================================================
 */
VOID AutoChSelInit(
	IN PRTMP_ADAPTER pAd)
{
    AutoChSelStateMachineInit(pAd, &pAd->AutoChSelCtrl.AutoChScanStatMachine, pAd->AutoChSelCtrl.AutoChScanFunc);        
}

/*
   ==========================================================================
   Description:
        Release auto-channel selection scan-timer.
   NOTE:
   ==========================================================================
 */
VOID AutoChSelRelease(
	IN PRTMP_ADAPTER pAd)
{
    BOOLEAN Cancelled;
    RTMPReleaseTimer(&pAd->AutoChSelCtrl.AutoChScanTimer, &Cancelled);	   
}

#ifdef AP_SCAN_SUPPORT
/*
   ==========================================================================
   Description:
       trigger Auto Channel Selection every period of ACSCheckTime.

   NOTE:
		This function is called in a 1-sec mlme periodic check.
		Do ACS only on one HW band at a time. 
		Do ACS only when no clients is associated.
   ==========================================================================
 */
VOID AutoChannelSelCheck(RTMP_ADAPTER *pAd)
{
	UINT8 i, WdevBand, Band, BandNum = HcGetAmountOfBand(pAd);
	BOOLEAN Result = 1;
	MAC_TABLE_ENTRY *pEntry = NULL;	
	struct wifi_dev *pwdev = NULL;

	
	/* Do nothing if ACSCheckTime is not configured or AP is doing channel scanning */
	for (Band = 0; Band < BandNum; Band++) 
		Result &= (pAd->ApCfg.ACSCheckTime[Band] == 0);

	if (Result || ApScanRunning(pAd))
		return;
	else 
	{
		for (Band = 0; Band < BandNum; Band++)
		{
			if (pAd->ApCfg.ACSCheckTime[Band] != 0)
				pAd->ApCfg.ACSCheckCount[Band]++;
		}
	}
	
	for (Band = 0; Band < BandNum; Band++)
	{
		if (pAd->ApCfg.ACSCheckCount[Band] > pAd->ApCfg.ACSCheckTime[Band])
		{
			/* Reset Counter */
			pAd->ApCfg.ACSCheckCount[Band] = 0;

			/* Do Auto Channel Selection only when no client is associated in current band */   
			if (pAd->MacTab.Size != 0)
			{
				for (i = 0; i < pAd->MacTab.Size; i++)
				{
					pEntry = &pAd->MacTab.Content[i];
					WdevBand = HcGetBandByWdev(pEntry->wdev);

					if (Band == WdevBand)
					{
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				       ("\x1b[41m%s(): Ignore ACS checking because has associated clients in current band%d\x1b[m\n",
				       __FUNCTION__, Band));

						return;
					}
				}
			}
			/* Start for ACS checking and do it only on one HW band at a time. */
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				       ("%s(): Scanning channels for channel selection.\n", __FUNCTION__));
				
				if (pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime)
				{
					for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					{
						pwdev = &pAd->ApCfg.MBSSID[i].wdev;
						WdevBand = HcGetBandByWdev(pwdev);
						
						if (Band == WdevBand)
						{
							AutoChSelScanStart(pAd, pwdev);
							break;
						}	
					}
				}
				else
				{
					ApSiteSurvey(pAd, NULL, SCAN_PASSIVE, TRUE);
				}
				
				return;
			}
		}
	}
}
#endif /* AP_SCAN_SUPPORT */

