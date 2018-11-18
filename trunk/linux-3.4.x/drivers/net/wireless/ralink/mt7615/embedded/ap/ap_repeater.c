/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_repeater.c

    Abstract:
    Support MAC Repeater function.

    Revision History:
    Who             		When              What
    --------------  ----------      ----------------------------------------------
    Arvin				11-16-2012      created
*/

#ifdef MAC_REPEATER_SUPPORT

#include "rt_config.h"

#define OUI_LEN	3
UCHAR VENDOR_DEFINED_OUI_ADDR[][OUI_LEN] = 
						{{0x02, 0x0C, 0x43},
						{0x02, 0x0C, 0xE7},
						{0x02, 0x0A, 0x00}};
static UCHAR  rept_vendor_def_oui_table_size = (sizeof(VENDOR_DEFINED_OUI_ADDR) / sizeof(UCHAR[OUI_LEN]));

/* IOCTL */
INT Show_ReptTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int CliIdx;
	RTMP_CHIP_CAP *cap = &pAd->chipCap;

	RETURN_ZERO_IF_PAD_NULL(pAd);
	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	printk("---------------------------------\n");
	printk("--------pRepeaterCliPool --------\n");
	printk("---------------------------------\n");
	printk("\n%-3s%-4s%-5s%-4s%-4s%-5s%-6s%-5s%-5s%-5s%-5s%-5s%-19s%-19s%-19s%-19s\n",
		   "AP", "CLI", "WCID", "En", "Vld", "bEth", "Block", "Conn", "CTRL", "SYNC", "AUTH", "ASSO","REAL_MAC","FAKE_MAC","MUAR_MAC","MUAR_ROOT");
	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
	{
		PREPEATER_CLIENT_ENTRY 		pReptCliEntry;
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		printk("%-3d", pReptCliEntry->MatchApCliIdx);
		printk("%-4d", pReptCliEntry->MatchLinkIdx);
		printk("%-5d", pReptCliEntry->MacTabWCID);
		printk("%-4d", pReptCliEntry->CliEnable);
		printk("%-4d", pReptCliEntry->CliValid);
		printk("%-5d", pReptCliEntry->bEthCli);
		printk("%-6d", pReptCliEntry->bBlockAssoc);
		printk("%-5d", pReptCliEntry->CliConnectState);
		printk("%-5lu", pReptCliEntry->CtrlCurrState);
		printk("%-5lu", pReptCliEntry->SyncCurrState);
		printk("%-5lu", pReptCliEntry->AuthCurrState);
		printk("%-5lu", pReptCliEntry->AssocCurrState);
		if ((memcmp(pAd->MonitorAddr,pReptCliEntry->OriginalAddress,MAC_ADDR_LEN) == 0) && (pReptCliEntry->CliEnable)) {
#define RED(_text)  "\033[1;31m"_text"\033[0m"
			printk(RED("%02X:%02X:%02X:%02X:%02X:%02X  "),
				   pReptCliEntry->OriginalAddress[0], pReptCliEntry->OriginalAddress[1], pReptCliEntry->OriginalAddress[2],
				   pReptCliEntry->OriginalAddress[3], pReptCliEntry->OriginalAddress[4], pReptCliEntry->OriginalAddress[5]);
		} else {
			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
				   pReptCliEntry->OriginalAddress[0], pReptCliEntry->OriginalAddress[1], pReptCliEntry->OriginalAddress[2],
				   pReptCliEntry->OriginalAddress[3], pReptCliEntry->OriginalAddress[4], pReptCliEntry->OriginalAddress[5]);
		}
		printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
			   pReptCliEntry->CurrentAddress[0], pReptCliEntry->CurrentAddress[1], pReptCliEntry->CurrentAddress[2],
			   pReptCliEntry->CurrentAddress[3], pReptCliEntry->CurrentAddress[4], pReptCliEntry->CurrentAddress[5]);
		//read muar cr MAR0,MAR1
		{
			//UINT32	mar_val;
			RMAC_MAR0_STRUC mar0_val;
			RMAC_MAR1_STRUC mar1_val;
			memset(&mar0_val,0x0,sizeof(mar0_val));
			memset(&mar1_val,0x0,sizeof(mar1_val));
			mar1_val.field.access_start = 1;
			mar1_val.field.multicast_addr_index = pReptCliEntry->MatchLinkIdx*2;
			/* Issue a read command */
			HW_IO_WRITE32(pAd, RMAC_MAR1, (UINT32)mar1_val.word);
			/* wait acess complete*/
			do {
				HW_IO_READ32(pAd, RMAC_MAR1, (UINT32*)&mar1_val);
				//delay
			} while(mar1_val.field.access_start == 1);

			HW_IO_READ32(pAd, RMAC_MAR0, (UINT32*)&mar0_val);
			printk("%02x:%02x:%02x:%02x:%02x:%02x  ",
				(UINT8)(mar0_val.addr_31_0 & 0x000000ff),
				(UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
				(UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
				(UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
				(UINT8)mar1_val.field.addr_39_32,
				(UINT8)mar1_val.field.addr_47_40
			);
			memset(&mar0_val,0x0,sizeof(mar0_val));
			memset(&mar1_val,0x0,sizeof(mar1_val));
			mar1_val.field.access_start = 1;
			mar1_val.field.multicast_addr_index = pReptCliEntry->MatchLinkIdx*2+1;
			/* Issue a read command */
			HW_IO_WRITE32(pAd, RMAC_MAR1, (UINT32)mar1_val.word);
			/* wait acess complete*/
			do {
				HW_IO_READ32(pAd, RMAC_MAR1, (UINT32*)&mar1_val);
				//delay
			} while(mar1_val.field.access_start == 1);

			HW_IO_READ32(pAd, RMAC_MAR0, (UINT32*)&mar0_val);
			printk("%02x:%02x:%02x:%02x:%02x:%02x\n",
				   (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
				   (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
				   (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
				   (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
				   (UINT8)mar1_val.field.addr_39_32,
				   (UINT8)mar1_val.field.addr_47_40
				  );
		}
	}
	return TRUE;
}

/* End of IOCTL */

VOID ApCliAuthTimeoutExt(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
    PRTMP_ADAPTER pAd;
    USHORT ifIndex = 0;

    MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
                ("Repeater Cli AUTH - AuthTimeout\n"));

    pAd = pRepeaterCliEntry->pAd;
    ifIndex = pRepeaterCliEntry->MatchLinkIdx + REPT_MLME_START_IDX;

    MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
            ("(%s) ifIndex = %d, CliIdx = %d !!!\n",
                __FUNCTION__,
                pRepeaterCliEntry->MatchApCliIdx,
                pRepeaterCliEntry->MatchLinkIdx));


    MlmeEnqueue(pAd,
                APCLI_AUTH_STATE_MACHINE,
                APCLI_MT2_AUTH_TIMEOUT,
                0,
                NULL,
                ifIndex);
    RTMP_MLME_HANDLER(pAd);

    return;
}

DECLARE_TIMER_FUNCTION(ApCliAuthTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAuthTimeoutExt);

/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters
    ==========================================================================
 */
VOID ApCliAssocTimeoutExt(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
    PRTMP_ADAPTER pAd;
    struct wifi_dev *wdev;
    USHORT ifIndex = 0;

    MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Repeater Cli ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT\n"));

    pAd = pRepeaterCliEntry->pAd;
    wdev = pRepeaterCliEntry->wdev;
    ifIndex = pRepeaterCliEntry->MatchLinkIdx + REPT_MLME_START_IDX;

    MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" (%s) ifIndex = %d, CliIdx = %d !!!\n",
                    __FUNCTION__, pRepeaterCliEntry->MatchApCliIdx, pRepeaterCliEntry->MatchLinkIdx));

    MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_ASSOC_TIMEOUT, 0, NULL, ifIndex);
    RTMP_MLME_HANDLER(pAd);

    return;
}

DECLARE_TIMER_FUNCTION(ApCliAssocTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAssocTimeoutExt);

static VOID ReptCompleteInit(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
}

static VOID ReptLinkDownComplete(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	RTMP_OS_COMPLETE(&pReptEntry->free_ack);
}

VOID ReptWaitLinkDown(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	if(pReptEntry->CliEnable && !RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pReptEntry->free_ack,REPT_WAIT_TIMEOUT))
	{
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
		("(%s) ApCli Rept[%d] can't done.\n", __FUNCTION__, pReptEntry->MatchLinkIdx));
	}
}



VOID CliLinkMapInit(RTMP_ADAPTER *pAd)
{
    UCHAR MbssIdx;
    MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap;
    struct wifi_dev *cli_link_wdev = &pAd->ApCfg.ApCliTab[0].wdev;//default bind to apcli0
    struct wifi_dev *mbss_link_wdev;
	int		apcli_idx;

    NdisAcquireSpinLock(&pAd->ApCfg.CliLinkMapLock);
    for (MbssIdx = 0; MbssIdx < HW_BEACON_MAX_NUM; MbssIdx++)
    {
        mbss_link_wdev = &pAd->ApCfg.MBSSID[MbssIdx].wdev;
        pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];
		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			for (apcli_idx=0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++) {
				cli_link_wdev = &pAd->ApCfg.ApCliTab[apcli_idx].wdev;
				if (mbss_link_wdev->channel <= 14) { //2.4G
					if (cli_link_wdev->channel <= 14 ) { //2.4G
						pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
						pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
					}
				} else { //5G
					if (cli_link_wdev->channel > 14 ) { //5G
						pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
						pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
					}
				}
			}
		} else {
			pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
			pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
		}
    }
    NdisReleaseSpinLock(&pAd->ApCfg.CliLinkMapLock);
}

VOID RepeaterCtrlInit(RTMP_ADAPTER *pAd)
{
    RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
    UCHAR MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(pChipCap);
    UINT32 Ret = FALSE;
    UCHAR i;
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    UINT32 PoolMemSize;

    NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

    if (pAd->ApCfg.bMACRepeaterEn == TRUE)
    {
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
                MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
                ("%s, wrong State\n", __FUNCTION__));
        return;
    }

    PoolMemSize = sizeof(REPEATER_CLIENT_ENTRY) * MaxNumChipRept;
    Ret = os_alloc_mem(NULL,
                        (UCHAR **)&pAd->ApCfg.pRepeaterCliPool,
                        PoolMemSize);
    if (Ret != NDIS_STATUS_SUCCESS)
    {
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                (" Alloc memory for pRepeaterCliPool failed.\n"));
        return;
    }
    os_zero_mem(pAd->ApCfg.pRepeaterCliPool, PoolMemSize);

    PoolMemSize = sizeof(REPEATER_CLIENT_ENTRY_MAP) * MaxNumChipRept;
    Ret = os_alloc_mem(NULL,
                        (UCHAR **)&pAd->ApCfg.pRepeaterCliMapPool,
                        PoolMemSize);
    if (Ret != NDIS_STATUS_SUCCESS)
    {
        if (pAd->ApCfg.pRepeaterCliPool)
            os_free_mem(pAd->ApCfg.pRepeaterCliPool);

        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                (" Alloc memory for pRepeaterCliMapPool failed.\n"));
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
        return;
    }

    os_zero_mem(pAd->ApCfg.pRepeaterCliMapPool, PoolMemSize);

    /*initialize RepeaterEntryPool*/
    for (i = 0; i < MaxNumChipRept; i++)
    {
        pReptEntry = &pAd->ApCfg.pRepeaterCliPool[i];
        pReptEntry->CliConnectState = REPT_ENTRY_DISCONNT;
        pReptEntry->CliEnable= FALSE;
        pReptEntry->CliValid= FALSE;
        pReptEntry->bEthCli = FALSE;
        pReptEntry->pAd = pAd;
        pReptEntry->MatchApCliIdx = 0;
        pReptEntry->MatchLinkIdx = i;
        pReptEntry->AuthCurrState = APCLI_CTRL_DISCONNECTED;
        pReptEntry->AssocCurrState = APCLI_ASSOC_IDLE;
        pReptEntry->bss_info_argument.ucBssIndex = 0xff;
        pReptEntry->AuthCurrState = APCLI_AUTH_REQ_IDLE;
		ReptCompleteInit(pReptEntry);

//         /* timer init */
//         RTMPInitTimer(pAd,
//                         &pReptEntry->ApCliAssocTimer,
//                         GET_TIMER_FUNCTION(ApCliAssocTimeoutExt),
//                         pReptEntry, FALSE);
//
//         /* timer init */
//         RTMPInitTimer(pAd, &pReptEntry->ApCliAuthTimer,
//                         GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), pReptEntry, FALSE);
    }

    pAd->ApCfg.RepeaterCliSize = 0;
    os_zero_mem(&pAd->ApCfg.ReptControl, sizeof(REPEATER_CTRL_STRUCT));
    pAd->ApCfg.bMACRepeaterEn = TRUE;

    NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

VOID RepeaterCtrlExit(RTMP_ADAPTER *pAd)
{
    //TODO: check whole repeater control release.
    int wait_cnt = 0;
	/* 
		Add MacRepeater Entry De-Init Here, and let "iwpriv ra0 set MACRepeaterEn=0"
		can do this instead of "iwpriv apcli0 set ApCliEnable=0"
	*/
	UCHAR CliIdx;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	RTMP_CHIP_CAP *cap = &pAd->chipCap;

	if (pAd->ApCfg.bMACRepeaterEn)
	{
		for(CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
		{
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
			/*disconnect the ReptEntry which is bind on the CliLink*/
			if (pReptEntry->CliEnable)
			{
				RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
				pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
				MlmeEnqueue(pAd,
							APCLI_CTRL_STATE_MACHINE,
							APCLI_CTRL_DISCONNECT_REQ,
							0,
							NULL,
							(REPT_MLME_START_IDX + CliIdx));
				RTMP_MLME_HANDLER(pAd);
				ReptWaitLinkDown(pReptEntry);
			}
		}
	}
    while (pAd->ApCfg.RepeaterCliSize > 0)
    {
        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF,
                ("%s, wait entry to be deleted\n", __FUNCTION__));
        OS_WAIT(10);
        wait_cnt++;

        if (wait_cnt > 1000)
            break;
    }

    NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

    if (pAd->ApCfg.bMACRepeaterEn == FALSE)
    {
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
                ("%s, wrong State\n", __FUNCTION__));
        return;
    }

    pAd->ApCfg.bMACRepeaterEn = FALSE;

    if (pAd->ApCfg.pRepeaterCliMapPool != NULL)
    {
        os_free_mem(pAd->ApCfg.pRepeaterCliMapPool);
        pAd->ApCfg.pRepeaterCliMapPool = NULL;
    }
    if (pAd->ApCfg.pRepeaterCliPool != NULL)
    {
        os_free_mem(pAd->ApCfg.pRepeaterCliPool);
        pAd->ApCfg.pRepeaterCliPool = NULL;
    }
    NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN PVOID pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;

    COPY_MAC_ADDR(tempMAC, pAddr);
    HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	//NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	if (bIsPad == TRUE) {
		NdisAcquireSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	} else {
		NdisAcquireSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);
	}

	if (bRealMAC == TRUE)
	{
		if (bIsPad == TRUE)
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		else
			pMapEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash) + HashIdx) ;
		
		while (pMapEntry)
		{
			pEntry = pMapEntry->pReptCliEntry;
			if (pEntry)
			{
				if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
					break;
				else {
					pEntry = NULL;
					pMapEntry = pMapEntry->pNext;
				}
			} else {
				pMapEntry = pMapEntry->pNext;
			}
		}
	}
	else
	{
		if (bIsPad == TRUE)
			pEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptCliHash[HashIdx];
		else
			pEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->CliHash) + HashIdx) ;
			
		while (pEntry)
		{
			if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->CurrentAddress, tempMAC))
				break;
			else
				pEntry = pEntry->pNext;
		}
	}
	//NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	if (bIsPad == TRUE) {
		NdisReleaseSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	} else {
		NdisReleaseSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);
	}

	return pEntry;
}

BOOLEAN RTMPQueryLookupRepeaterCliEntryMT(
	IN PVOID pData,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{

	REPEATER_CLIENT_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
                            ("%s:: %02x:%02x:%02x:%02x:%02x:%02x\n", 
							__FUNCTION__,
							pAddr[0],
							pAddr[1],
							pAddr[2], 
							pAddr[3],
							pAddr[4],
							pAddr[5]));
	
	
	pEntry = RTMPLookupRepeaterCliEntry(pData, FALSE, pAddr, bIsPad);
	
	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
                            ("%s:: not the repeater client\n", __FUNCTION__));
		return FALSE;
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
                    ("%s:: is the repeater client\n", __FUNCTION__));
		return TRUE;
	}
}

UINT32 ReptTxPktCheckHandler(
    RTMP_ADAPTER *pAd,
    IN struct wifi_dev *cli_link_wdev,
    IN PNDIS_PACKET pPacket,
    OUT UCHAR *pWcid)
{
    PUCHAR pSrcBufVA = NULL;
    PACKET_INFO PacketInfo;
    UINT SrcBufLen;
    STA_TR_ENTRY *tr_entry;
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    APCLI_STRUCT *pApCliEntry = cli_link_wdev->func_dev;
    MAC_TABLE_ENTRY *pMacEntry = NULL;
    struct wifi_dev *mbss_wdev = NULL;
    MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;

	RETURN_ZERO_IF_PAD_NULL(pAd);

    RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

    pReptEntry = RTMPLookupRepeaterCliEntry(
                            pAd,
                            TRUE,
                            (pSrcBufVA + MAC_ADDR_LEN),
                            TRUE);
    if (pReptEntry  && pReptEntry->CliValid)
    {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		if ((pReptEntry->MatchApCliIdx != pApCliEntry->ifIndex) &&					
			(wf_fwd_check_active_hook && wf_fwd_check_active_hook()))
		{
			UCHAR apCliIdx, CliIdx;
			apCliIdx = pReptEntry->MatchApCliIdx;
			CliIdx = pReptEntry->MatchLinkIdx;

			pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_CHANGE_APCLI_IF;
			MlmeEnqueue(pAd,
						APCLI_CTRL_STATE_MACHINE,
						APCLI_CTRL_DISCONNECT_REQ,
						0,
						NULL,
						(REPT_MLME_START_IDX + CliIdx));
			RTMP_MLME_HANDLER(pAd);
			return INSERT_REPT_ENTRY;
		}
#endif /* CONFIG_WIFI_PKT_FWD */		
	
        *pWcid = pReptEntry->MacTabWCID;
        return REPEATER_ENTRY_EXIST;
    }
    else
    {
        //check SA valid.
        if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN))
        {
            tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];
            if ((tr_entry) &&
                (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
            {
				RTMP_CHIP_CAP *cap = &pAd->chipCap;

				if (pApCliEntry->InsRepCmdCount > GET_MAX_REPEATER_ENTRY_NUM(cap))
					return INSERT_REPT_ENTRY;

                pMacEntry = MacTableLookup(pAd, (pSrcBufVA + MAC_ADDR_LEN));
                if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
                {
					STA_TR_ENTRY *sta_tr_entry;
					sta_tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
					
					if ((sta_tr_entry) &&
							(sta_tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
								 (" wireless client is not ready !!!\n"));
						return INSERT_REPT_ENTRY;
					}
                    mbss_wdev = pMacEntry->wdev;
                    pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[mbss_wdev->func_idx];

                    if (
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
						(wf_fwd_check_active_hook && wf_fwd_check_active_hook()) ||
#endif /* CONFIG_WIFI_PKT_FWD */						
						(pMbssToCliLinkMap->cli_link_wdev == cli_link_wdev))
                    {
                        HW_ADD_REPT_ENTRY(pAd, cli_link_wdev, (pSrcBufVA + MAC_ADDR_LEN));
                        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
                                ("pMacEntry connect to mbss idx:%d, use CliLink:%d to RootAP\n",
                                    mbss_wdev->func_idx, cli_link_wdev->func_idx));
                        return INSERT_REPT_ENTRY;
                    }
                }
                else
                /*SA is not in mac table, pkt should from upper layer or eth.*/
                {
                    /*
                        TODO: Carter, if more than one apcli/sta,
                        the eth pkt or upper layer pkt connecting rule should be refined.
                    */
                    HW_ADD_REPT_ENTRY(pAd, cli_link_wdev, (pSrcBufVA + MAC_ADDR_LEN));
                    MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
                            ("pAddr %x %x %x %x %x %x: use CliLink:%d to RootAP\n",
                                        PRINT_MAC((pSrcBufVA + MAC_ADDR_LEN)),
                                        cli_link_wdev->func_idx));
                    return INSERT_REPT_ENTRY;
                }
            }
        }
    }
    return USE_CLI_LINK_INFO;
}

VOID RTMPInsertRepeaterEntry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PUCHAR pAddr)
{
	INT CliIdx, idx;
	UCHAR HashIdx;
	//BOOLEAN Cancelled;
	UCHAR tempMAC[MAC_ADDR_LEN];
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL, pCurrEntry = NULL;
	INT pValid_ReptCliIdx;
	PREPEATER_CLIENT_ENTRY_MAP pReptCliMap;
    RTMP_CHIP_CAP *cap = &pAd->chipCap;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
            ("%s.\n", __FUNCTION__));

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.RepeaterCliSize >= GET_MAX_REPEATER_ENTRY_NUM(cap))
	{
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                (" Repeater Client Full !!!\n"));
		return;
	}

	pValid_ReptCliIdx = GET_MAX_REPEATER_ENTRY_NUM(cap);
	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
	{
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

		if ((pReptCliEntry->CliEnable) &&
				(MAC_ADDR_EQUAL(pReptCliEntry->OriginalAddress, pAddr) ||
				 MAC_ADDR_EQUAL(pReptCliEntry->CurrentAddress, pAddr))
		   )
		{
            NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
                    ("\n  receive mac :%02x:%02x:%02x:%02x:%02x:%02x !!!\n",
                                                PRINT_MAC(pAddr)));

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
                    (" duplicate Insert !!!\n"));
			return ;
		}
		if ((pReptCliEntry->CliEnable == FALSE) && (pValid_ReptCliIdx == GET_MAX_REPEATER_ENTRY_NUM(cap))) {
			pValid_ReptCliIdx = CliIdx;
		}
	}
	CliIdx = pValid_ReptCliIdx;
	if (CliIdx >= GET_MAX_REPEATER_ENTRY_NUM(cap))
	{
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                ("Repeater Pool Full !!!\n"));
		return ;
	}

	pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	pReptCliMap = &pAd->ApCfg.pRepeaterCliMapPool[CliIdx];

	/* ENTRY PREEMPTION: initialize the entry */
    /* timer init */
    RTMPInitTimer(pAd,
                    &pReptCliEntry->ApCliAssocTimer,
                    GET_TIMER_FUNCTION(ApCliAssocTimeoutExt),
                    pReptCliEntry, FALSE);

    /* timer init */
    RTMPInitTimer(pAd, &pReptCliEntry->ApCliAuthTimer,
                    GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), pReptCliEntry, FALSE);

	pReptCliEntry->CtrlCurrState = APCLI_CTRL_DISCONNECTED;
	pReptCliEntry->AuthCurrState = APCLI_AUTH_REQ_IDLE;
	pReptCliEntry->AssocCurrState = APCLI_ASSOC_IDLE;
	pReptCliEntry->CliConnectState = REPT_ENTRY_DISCONNT;
	pReptCliEntry->LinkDownReason = APCLI_LINKDOWN_NONE;
	pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
	pReptCliEntry->CliValid = FALSE;
	pReptCliEntry->bEthCli = FALSE;
	pReptCliEntry->MacTabWCID = 0xFF;
#ifdef FAST_EAPOL_WAR
	if (pReptCliEntry->pre_entry_alloc == TRUE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected condition,check it (pReptCliEntry->pre_entry_alloc=%d)\n", __FUNCTION__,pReptCliEntry->pre_entry_alloc));
	}
	pReptCliEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	pReptCliEntry->AuthReqCnt = 0;
	pReptCliEntry->AssocReqCnt = 0;
	pReptCliEntry->CliTriggerTime = 0;
	pReptCliEntry->pNext = NULL;
    pReptCliEntry->wdev = wdev;
    pReptCliEntry->MatchApCliIdx = wdev->func_idx;
    pReptCliEntry->BandIdx = HcGetBandByWdev(wdev);
	pReptCliMap->pReptCliEntry = pReptCliEntry;
	pReptCliMap->pNext = NULL;

	COPY_MAC_ADDR(pReptCliEntry->OriginalAddress, pAddr);
	COPY_MAC_ADDR(tempMAC, pAddr);

	if (pAd->ApCfg.MACRepeaterOuiMode == CASUALLY_DEFINE_MAC_ADDR)
	{
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                ("todo !!!\n"));
	}
	else if (pAd->ApCfg.MACRepeaterOuiMode == VENDOR_DEFINED_MAC_ADDR_OUI)
	{
		INT IdxToUse, i;
		UCHAR checkMAC[MAC_ADDR_LEN];
		COPY_MAC_ADDR(checkMAC, pAddr);

		for (idx = 0; idx < rept_vendor_def_oui_table_size; idx++)
		{
			if (RTMPEqualMemory(VENDOR_DEFINED_OUI_ADDR[idx], pAddr, OUI_LEN))
				continue;
			else
			{
				NdisCopyMemory(checkMAC, VENDOR_DEFINED_OUI_ADDR[idx], OUI_LEN);

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				{
					if (MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[i].wdev.if_addr, checkMAC))
						break;			
				}

				if (i >= pAd->ApCfg.BssidNum)
					break;
			}
		}
		/*
	            If there is a matched one can be used
	            otherwise, use the first one.
       	 */
		if (idx >= 0 && idx < rept_vendor_def_oui_table_size)
			IdxToUse = idx;
		else
			IdxToUse = 0;
		NdisCopyMemory(tempMAC, VENDOR_DEFINED_OUI_ADDR[IdxToUse], OUI_LEN);
	}
	else
	{
		NdisCopyMemory(tempMAC, wdev->if_addr, OUI_LEN);
	}

	COPY_MAC_ADDR(pReptCliEntry->CurrentAddress, tempMAC);
	pReptCliEntry->CliEnable = TRUE;
	pReptCliEntry->CliConnectState = REPT_ENTRY_CONNTING;
	pReptCliEntry->pNext = NULL;
	NdisGetSystemUpTime(&pReptCliEntry->CliTriggerTime);

    pAd->ApCfg.RepeaterCliSize++;
    NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

    /* Before add into muar table, config Band binding. */
    HcAddRepeaterEntry(wdev, CliIdx);

	AsicInsertRepeaterEntry(pAd, CliIdx, tempMAC);

	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);
	if (pAd->ApCfg.ReptCliHash[HashIdx] == NULL)
	{
		pAd->ApCfg.ReptCliHash[HashIdx] = pReptCliEntry;
	}
	else
	{
		pCurrEntry = pAd->ApCfg.ReptCliHash[HashIdx];
		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;
		pCurrEntry->pNext = pReptCliEntry;
	}

	HashIdx = MAC_ADDR_HASH_INDEX(pReptCliEntry->OriginalAddress);
	if (pAd->ApCfg.ReptMapHash[HashIdx] == NULL)
		pAd->ApCfg.ReptMapHash[HashIdx] = pReptCliMap;
	else
	{
		PREPEATER_CLIENT_ENTRY_MAP pCurrMapEntry;

		pCurrMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];

		while (pCurrMapEntry->pNext != NULL)
			pCurrMapEntry = pCurrMapEntry->pNext;
		pCurrMapEntry->pNext = pReptCliMap;
	}

    /*
        FIXME:
            if apcli is removed afterward,
            the state machine massage should be reviewed.
    */
	NdisZeroMemory(&ApCliCtrlMsg, sizeof(APCLI_CTRL_MSG_STRUCT));
	ApCliCtrlMsg.Status = MLME_SUCCESS;
	COPY_MAC_ADDR(&ApCliCtrlMsg.SrcAddr[0], tempMAC);
	ApCliCtrlMsg.BssIdx = wdev->func_idx;
	ApCliCtrlMsg.CliIdx = CliIdx;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_MT2_AUTH_REQ,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, wdev->func_idx);
    RTMP_MLME_HANDLER(pAd);
}

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR func_tb_idx,
	IN UCHAR CliIdx)
{
	USHORT HashIdx;
	REPEATER_CLIENT_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	BOOLEAN bVaild = TRUE;
    BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" %s.CliIdx=%d\n", __FUNCTION__,CliIdx));

    AsicRemoveRepeaterEntry(pAd, CliIdx);

    NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	pEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	/* move NULL check here, to prevent pEntry NULL dereference */
	if (pEntry == NULL)
	{
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                ("%s - pEntry is NULL !!!\n",__FUNCTION__));
		return;
	}

    if (pEntry->CliEnable == FALSE)
    {
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
        MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                ("%s - CliIdx:%d Enable is FALSE already\n",
                        __FUNCTION__, CliIdx));
        return;
    }
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" %s.CliIdx=%d,orig addr = %02X:%02X:%02X:%02X:%02X:%02X,fake addr = %02X:%02X:%02X:%02X:%02X:%02X, bandIdx = %d\n",
						__FUNCTION__,CliIdx,
						PRINT_MAC(pEntry->OriginalAddress),
						PRINT_MAC(pEntry->CurrentAddress),
						pEntry->BandIdx));

	/*Release OMAC Idx*/
	HcDelRepeaterEntry(pEntry->wdev,CliIdx);

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->CurrentAddress);

	pPrevEntry = NULL;
	pProbeEntry = pAd->ApCfg.ReptCliHash[HashIdx];

	ASSERT(pProbeEntry);

	if (pProbeEntry == NULL)
	{
		bVaild = FALSE;
		goto done;
	}

	if (pProbeEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeEntry == pEntry)
			{
				if (pPrevEntry == NULL)
				{
					pAd->ApCfg.ReptCliHash[HashIdx] = pEntry->pNext;
				}
				else
				{
					pPrevEntry->pNext = pEntry->pNext;
				}
				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeEntry != NULL);

	if (pProbeEntry == NULL)
	{
		bVaild = FALSE;
		goto done;
	}

	pMapEntry = &pAd->ApCfg.pRepeaterCliMapPool[CliIdx];

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->OriginalAddress);

	pPrevMapEntry = NULL;
	pProbeMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
	ASSERT(pProbeMapEntry);
	if (pProbeMapEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeMapEntry == pMapEntry)
			{
				if (pPrevMapEntry == NULL)
				{
					pAd->ApCfg.ReptMapHash[HashIdx] = pMapEntry->pNext;
				}
				else
				{
					pPrevMapEntry->pNext = pMapEntry->pNext;
				}
				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}
	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);

	/* move this NULL check to earlier , fix klockwork issue
	if (pEntry == NULL)
	{
       	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
        	return;
	}
	*/
done:
    RTMPReleaseTimer(&pEntry->ApCliAuthTimer, &Cancelled);
    RTMPReleaseTimer(&pEntry->ApCliAssocTimer, &Cancelled);
#ifdef FAST_EAPOL_WAR
	if (pEntry->pre_entry_alloc == TRUE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected condition,check it (pEntry->pre_entry_alloc=%d)\n", __FUNCTION__,pEntry->pre_entry_alloc));
	}
	pEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	pEntry->CliConnectState = REPT_ENTRY_DISCONNT;
	pEntry->CliValid = FALSE;
	pEntry->CliEnable = FALSE;

	if (bVaild == TRUE)
		pAd->ApCfg.RepeaterCliSize--;

	ReptLinkDownComplete(pEntry);
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	return;
}

#if defined(RTMP_MAC) || defined(RLT_MAC)
MAC_TABLE_ENTRY *RTMPInsertRepeaterMacEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN  UCHAR apIdx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	UINT i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	BOOLEAN Cancelled;
	ASIC_SEC_INFO Info = {0};

	RETURN_ZERO_IF_PAD_NULL(pAd);
	if (pAd->MacTab.Size >= GET_MAX_UCAST_NUM(pAd))
		return NULL;

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);

	i = (MAX_NUMBER_OF_MAC + ((MAX_EXT_MAC_ADDR_SIZE + 1) * (apIdx - MIN_NET_DEVICE_FOR_APCLI)));

	if (cliIdx != 0xFF)
		i  = i + cliIdx + 1;

	/* fix klockwork issue, to prevent MacTab.Content array out of bound */
	if (i >= MAX_LEN_OF_MAC_TABLE){
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Unexpected index %d > %d !\n", __FUNCTION__,i,MAX_LEN_OF_MAC_TABLE));
		return NULL;
	}


	/* pick up the first available vacancy*/
	if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
	{
		pEntry = &pAd->MacTab.Content[i];

		/* ENTRY PREEMPTION: initialize the entry */
		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

		if (CleanAll == TRUE)
		{
			pEntry->MaxSupportedRate = RATE_11;
			pEntry->CurrTxRate = RATE_11;
			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		if (apIdx >= MIN_NET_DEVICE_FOR_APCLI)
		{
			SET_ENTRY_APCLI(pEntry);
		}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		pEntry->wdev = wdev;
		pEntry->wcid = i;
		//SET_ENTRY_AP(pEntry);//Carter, why set Apcli Entry then set to AP entry?
		pAd->MacTab.tr_entry[i].isCached = FALSE;
		//tr_entry->isCached = FALSE;
		pEntry->bIAmBadAtheros = FALSE;

#ifdef TXBF_SUPPORT
#ifndef MT_MAC
		if (pAd->chipCap.FlgHwTxBfCap)
			RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, FALSE);
#endif
#endif /* TXBF_SUPPORT */

		pEntry->pAd = pAd;
		pEntry->CMTimerRunning = FALSE;
		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
		pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
		pEntry->func_tb_idx = (apIdx - MIN_NET_DEVICE_FOR_APCLI);

		if (IS_ENTRY_APCLI(pEntry))
			pEntry->apidx = (apIdx - MIN_NET_DEVICE_FOR_APCLI);

		pEntry->pMbss = NULL;

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			pEntry->SecConfig.AKMMap = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.SecConfig.AKMMap;
			pEntry->SecConfig.PairwiseCipher = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.SecConfig.PairwiseCipher;

			if (IS_AKM_OPEN(pEntry->SecConfig.AKMMap)
				||IS_AKM_SHARED(pEntry->SecConfig.AKMMap),
				||IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))
			{
				pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			}
			else
			{
				pEntry->WpaState = AS_PTKSTART;
				pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			}
		}
#endif /* APCLI_SUPPORT */

		pEntry->GTKState = REKEY_NEGOTIATING;
		pEntry->PairwiseKey.KeyLen = 0;
		pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		pAd->MacTab.tr_entry[i].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		//pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

		pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
		COPY_MAC_ADDR(pEntry->Addr, pAddr);

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			COPY_MAC_ADDR(pEntry->bssid, pAddr);
		}
#endif // APCLI_SUPPORT //

		pEntry->Sst = SST_NOT_AUTH;
		pEntry->AuthState = AS_NOT_AUTH;
		pEntry->Aid = (USHORT)i;
		pEntry->CapabilityInfo = 0;
		pEntry->PsMode = PWR_ACTIVE;
		pAd->MacTab.tr_entry[i].PsQIdleCount = 0;
		//pEntry->PsQIdleCount = 0;
		pEntry->NoDataIdleCount = 0;
		pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
		pEntry->ContinueTxFailCnt = 0;
		pEntry->TimeStamp_toTxRing = 0;
		// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
		pAd->MacTab.tr_entry[i].PsMode = PWR_ACTIVE;
		pAd->MacTab.tr_entry[i].NoDataIdleCount = 0;
		pAd->MacTab.tr_entry[i].ContinueTxFailCnt = 0;
		pAd->MacTab.tr_entry[i].LockEntryTx = FALSE;
		pAd->MacTab.tr_entry[i].TimeStamp_toTxRing = 0;

		pAd->MacTab.Size ++;

		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
		Info.Wcid = (UCHAR)i;

		/* Set key material to Asic */
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);

		/* Add this entry into ASIC RX WCID search table */
		RTMP_STA_ENTRY_ADD(pAd, pEntry->wcid,pEntry->Addr,FALSE, TRUE);

#ifdef TXBF_SUPPORT
		if (pAd->chipCap.FlgHwTxBfCap)
			NdisAllocateSpinLock(pAd, &pEntry->TxSndgLock);
#endif /* TXBF_SUPPORT */

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s - allocate entry #%d, Aid = %d, Total= %d\n",__FUNCTION__, i, pEntry->Aid, pAd->MacTab.Size));

	}
	else
	{
		pEntry = &pAd->MacTab.Content[i];
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s - exist entry #%d, Aid = %d, Total= %d\n", __FUNCTION__, i, pEntry->Aid, pAd->MacTab.Size));
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return pEntry;
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
		{
			pAd->MacTab.Hash[HashIdx] = pEntry;
		}
		else
		{
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}

	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	//rtmp_tx_burst_set(pAd);

	return pEntry;
}
#endif /* RTMP_MAC || RLT_MAC  */

VOID RTMPRepeaterReconnectionCheck(
	IN PRTMP_ADAPTER pAd)
{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	INT i;
	PCHAR	pApCliSsid, pApCliCfgSsid;
	UCHAR	CfgSsidLen;
	NDIS_802_11_SSID Ssid;
	ULONG timeDiff[MAX_APCLI_NUM];

	if (pAd->ApCfg.bMACRepeaterEn &&
		pAd->ApCfg.MACRepeaterOuiMode == VENDOR_DEFINED_MAC_ADDR_OUI &&
		pAd->ScanCtrl.PartialScan.bScanning == FALSE)
	{
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			if (!APCLI_IF_UP_CHECK(pAd, i) ||
				(pAd->ApCfg.ApCliTab[i].Enable == FALSE))
				continue;

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, (" %s(): i=%d,%d,%d,%d,%d,%d\n",
					 __FUNCTION__, (int)i,
					 (int)ApScanRunning(pAd),
					(int)pAd->ApCfg.ApCliAutoConnectRunning[i],
					(int)pAd->ApCfg.ApCliAutoConnectType[i],
					(int)pAd->ApCfg.bPartialScanEnable[i],
					(int)(pAd->Mlme.OneSecPeriodicRound%23)));

			if (ApScanRunning(pAd))
				continue;
			if (pAd->ApCfg.ApCliAutoConnectRunning[i] != FALSE)
				continue;
			if (pAd->ApCfg.ApCliTab[i].AutoConnectFlag == FALSE)
				continue;
			pApCliSsid = pAd->ApCfg.ApCliTab[i].Ssid;
			pApCliCfgSsid = pAd->ApCfg.ApCliTab[i].CfgSsid;
			CfgSsidLen = pAd->ApCfg.ApCliTab[i].CfgSsidLen;
			if ((pAd->ApCfg.ApCliTab[i].CtrlCurrState < APCLI_CTRL_AUTH ||
				!NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
				pAd->ApCfg.ApCliTab[i].CfgSsidLen > 0)
			{
				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->ApCfg.ApCliIssueScanTime[i]))
					timeDiff[i] = (pAd->Mlme.Now32 - pAd->ApCfg.ApCliIssueScanTime[i]);
				else
					timeDiff[i] = (pAd->ApCfg.ApCliIssueScanTime[i] - pAd->Mlme.Now32);
				//will trigger scan after 23 sec
				if (timeDiff[i] <= RTMPMsecsToJiffies(23000))
					continue;
				
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" %s(): Scan channels for AP (%s)\n",
							__FUNCTION__, pApCliCfgSsid));
				pAd->ApCfg.ApCliAutoConnectRunning[i] = TRUE;
				if (pAd->ApCfg.bPartialScanEnable[i]) {
					pAd->ApCfg.bPartialScanning[i] = TRUE;
					pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.ApCliTab[i].wdev;
					pAd->ScanCtrl.PartialScan.bScanning = TRUE;
				}
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				NdisGetSystemUpTime(&pAd->ApCfg.ApCliIssueScanTime[i]);
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->ApCfg.ApCliTab[i].wdev);
			}
		}
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
}

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
    RTMP_CHIP_CAP *cap = &pAd->chipCap;

	if (pAd->ApCfg.RepeaterCliSize >= GET_MAX_REPEATER_ENTRY_NUM(cap))
		return FALSE;

	if(IS_MULTICAST_MAC_ADDR(pAddr))
		return FALSE;

	if(IS_BROADCAST_MAC_ADDR(pAddr))
		return FALSE;

	pEntry = RepeaterInvaildMacLookup(pAd, pAddr);

	if (pEntry)
		return FALSE;
	else
		return TRUE;
}

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	ULONG HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];

	while (pEntry)
	{
		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
		{
			break;
		}
		else
			pEntry = pEntry->pNext;
	}

	if (pEntry && pEntry->bInsert)
		return pEntry;
	else
		return NULL;
}

VOID InsertIgnoreAsRepeaterEntryTable(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	UCHAR HashIdx, idx = 0;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pCurrEntry = NULL;

	RETURN_IF_PAD_NULL(pAd);

	if (pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize >= MAX_IGNORE_AS_REPEATER_ENTRY_NUM)
		return;

	if (MAC_ADDR_EQUAL(pAddr, ZERO_MAC_ADDR))
		return;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	for (idx = 0; idx< MAX_IGNORE_AS_REPEATER_ENTRY_NUM; idx++)
	{
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
		{
			if (pEntry->bInsert)
			{
				NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				return;
			}
		}

		/* pick up the first available vacancy*/
		if (pEntry && pEntry->bInsert == FALSE)
		{
			NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
			COPY_MAC_ADDR(pEntry->MacAddr, pAddr);
			pEntry->entry_idx = idx;
			pEntry->bInsert = TRUE;
			break;
		}
	}

	/* add this entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		pEntry->pNext = NULL;
		if (pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] == NULL)
		{
			pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] = pEntry;
		}
		else
		{
			pCurrEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}

		/* move inside to make sure pEntry is not null while printing */
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" Store Invaild MacAddr = %02x:%02x:%02x:%02x:%02x:%02x. !!!\n",
				PRINT_MAC(pEntry->MacAddr)));
	}



	pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return;
}

BOOLEAN RepeaterRemoveIngoreEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR idx,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pPrevEntry, *pProbeEntry;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

	if (pEntry)
	{
		if(pEntry->bInsert){
			pPrevEntry = NULL;
			pProbeEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];
			ASSERT(pProbeEntry);
			if (pProbeEntry != NULL)
			{
				/* update Hash list*/
				do
				{
					if (pProbeEntry == pEntry)
					{
						if (pPrevEntry == NULL)
						{
							pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] = pEntry->pNext;
						}
						else
						{
							pPrevEntry->pNext = pEntry->pNext;
						}
						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);
			}
			/* not found !!!*/
			ASSERT(pProbeEntry != NULL);

			pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize--;
		}

		/* move inside to make sure pEntry is not NULL */
		NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
		pEntry->bInsert = FALSE;
	}

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return TRUE;
}

VOID RepeaterLinkMonitor(RTMP_ADAPTER *pAd)
{
    REPEATER_CLIENT_ENTRY *ReptPool = pAd->ApCfg.pRepeaterCliPool;
    REPEATER_CLIENT_ENTRY *pReptCliEntry = NULL;
    RTMP_CHIP_CAP *cap = &pAd->chipCap;
    UCHAR Wcid = 0;
    STA_TR_ENTRY *tr_entry = NULL;
    APCLI_CTRL_MSG_STRUCT msg;
    UCHAR CliIdx;

    if ((pAd->ApCfg.bMACRepeaterEn) && (ReptPool != NULL))
    {
        for(CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
        {
            pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

            if (pReptCliEntry->CliEnable)
            {
                Wcid = pReptCliEntry->MacTabWCID;

                tr_entry = &pAd->MacTab.tr_entry[Wcid];
                if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) &&
                    RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pReptCliEntry->CliTriggerTime + (5 * OS_HZ))))
                {
					if (pReptCliEntry->CtrlCurrState == APCLI_CTRL_DISCONNECTED) {
						HW_REMOVE_REPT_ENTRY(pAd, pReptCliEntry->MatchApCliIdx, CliIdx);
					} else {
						if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
							continue;
						pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_REPTLM_TRIGGER_TOO_LONG;
						NdisZeroMemory(&msg, sizeof(APCLI_CTRL_MSG_STRUCT));
						msg.BssIdx = pReptCliEntry->MatchApCliIdx;
						msg.CliIdx = CliIdx;
						MlmeEnqueue(pAd,
									APCLI_CTRL_STATE_MACHINE,
									APCLI_CTRL_DISCONNECT_REQ,
									sizeof(APCLI_CTRL_MSG_STRUCT),
									&msg,
									REPT_MLME_START_IDX + CliIdx);
						RTMP_MLME_HANDLER(pAd);
					}
                }
            }
        }
    }
}

INT Show_Repeater_Cli_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	ULONG DataRate=0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	ADD_HT_INFO_IE *addht;
	
	if(!wdev)
		return FALSE;
	
	addht = wlan_operate_get_addht(wdev);

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	printk("\n");

#ifdef DOT11_N_SUPPORT
	printk("HT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode);
	printk("\n");
#endif /* DOT11_N_SUPPORT */

	printk("\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1",
		   "RSSI2", "RSSI3", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry &&
                (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
            && (pEntry->Sst == SST_ASSOC) && (pEntry->bReptCli))
		{
			DataRate=0;
			getRate(pEntry->HTPhyMode, &DataRate);

			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
					pEntry->ReptCliAddr[0], pEntry->ReptCliAddr[1], pEntry->ReptCliAddr[2],
					pEntry->ReptCliAddr[3], pEntry->ReptCliAddr[4], pEntry->ReptCliAddr[5]);

			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d-%d", (int)pEntry->apidx, pEntry->func_tb_idx);
			printk("%-4d", (int)pEntry->PsMode);
			printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			printk("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
			printk("%-7d", pEntry->RssiSample.AvgRssi[0]);
			printk("%-7d", pEntry->RssiSample.AvgRssi[1]);
			printk("%-7d", pEntry->RssiSample.AvgRssi[2]);
			printk("%-7d", pEntry->RssiSample.AvgRssi[3]);
			printk("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			printk("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
			printk("%-6d", pEntry->HTPhyMode.field.MCS);
			printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-6d", pEntry->HTPhyMode.field.STBC);
			printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			printk("%-7d", (int)DataRate);
			printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
			printk("\n");
		}
	}

	return TRUE;
}

VOID UpdateMbssCliLinkMap(
            RTMP_ADAPTER *pAd,
            UCHAR MbssIdx,
            struct wifi_dev *cli_link_wdev,
            struct wifi_dev *mbss_link_wdev)
{
    MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;

    NdisAcquireSpinLock(&pAd->ApCfg.CliLinkMapLock);
    pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];
    pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
    pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
    NdisReleaseSpinLock(&pAd->ApCfg.CliLinkMapLock);

    return;
}

#endif /* MAC_REPEATER_SUPPORT */

