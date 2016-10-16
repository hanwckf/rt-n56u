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
	rt_profile.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#include "rt_config.h"

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
#include <linux/foe_hook.h>
#endif

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../net/nat/hw_nat/frame_engine.h"
#endif


struct dev_type_name_map{
	INT type;
	RTMP_STRING *prefix[2];
};


#define SECOND_INF_MAIN_DEV_NAME	"rai"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME	"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME	"moni"

#define xdef_to_str(s)			def_to_str(s)
#define def_to_str(s)			#s

#define FIRST_AP_PROFILE_PATH		"/etc/Wireless/RT2860/RT2860AP.dat"
#define FIRST_CHIP_ID			xdef_to_str(CONFIG_RT_FIRST_CARD)

#define SECOND_AP_PROFILE_PATH		"/etc/Wireless/iNIC/iNIC_ap.dat"
#define SECOND_CHIP_ID			xdef_to_str(CONFIG_RT_SECOND_CARD)

static struct dev_type_name_map prefix_map[] =
{
	{INT_MAIN,		{INF_MAIN_DEV_NAME, SECOND_INF_MAIN_DEV_NAME}},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{INT_MBSSID,		{INF_MBSSID_DEV_NAME, SECOND_INF_MBSSID_DEV_NAME}},
#endif /* MBSS_SUPPORT */
#ifdef APCLI_SUPPORT
	{INT_APCLI,		{INF_APCLI_DEV_NAME, SECOND_INF_APCLI_DEV_NAME}},
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	{INT_WDS,		{INF_WDS_DEV_NAME, SECOND_INF_WDS_DEV_NAME}},
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_SNIFFER_SUPPORT
	{INT_MONITOR,		{INF_MONITOR_DEV_NAME, SECOND_INF_MONITOR_DEV_NAME}},
#endif /* CONFIG_SNIFFER_SUPPORT */

	{0},
};


INT get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	INT idx = 0;
#if (CONFIG_RT_FIRST_CARD == 7603) && \
    (CONFIG_RT_SECOND_CARD == 7603)
	INT first_card = 0, second_card = 0;

	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);
	DBGPRINT(RT_DEBUG_TRACE, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x\n", first_card, second_card, pAd->MACVersion));

	if (IS_RT8592(pAd))
		idx = 0;
	else if (IS_RT5392(pAd) || IS_MT76x0(pAd) || IS_MT76x2(pAd))
		idx = 1;
#endif

	pAd->dev_idx = idx;

	return idx;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	do {
		map = &prefix_map[type_idx];
		if (map->type == dev_type) {
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): dev_idx = %d, dev_name_prefix=%s\n",
						__FUNCTION__, dev_idx, map->prefix[dev_idx]));
			return map->prefix[dev_idx];
		}
		type_idx++;
	} while (prefix_map[type_idx].type != 0);

	return NULL;
}


static UCHAR *get_dev_profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#if (CONFIG_RT_FIRST_CARD == 7603) && \
    (CONFIG_RT_SECOND_CARD == 7603)
			INT card_idx = get_dev_config_idx(pAd);
			if (card_idx == 0)
			{
				src = FIRST_AP_PROFILE_PATH;
			}
			else
			if (card_idx == 1)
			{
				src = SECOND_AP_PROFILE_PATH;
			}
			else
#endif
			{
				src = AP_PROFILE_PATH;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef MULTIPLE_CARD_SUPPORT
	src = (RTMP_STRING *)pAd->MC_FileName;
#endif /* MULTIPLE_CARD_SUPPORT */

	return src;
}

void RTMPPreReadParametersHook(RTMP_ADAPTER *pAd)
{


    return;
}

NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src = NULL;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	INT retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE, fsize;
	RTMP_STRING *buffer = NULL;

#ifdef HOSTAPD_SUPPORT
	int i;
#endif /*HOSTAPD_SUPPORT */

	src = get_dev_profile(pAd);
	if (src && *src)
	{
		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Open file \"%s\" failed!\n", src));
		}
		else 
		{
#ifndef OS_ABL_SUPPORT
			// TODO: need to roll back when convert into OSABL code
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
				fsize = (ULONG)file_inode(srcf)->i_size;
#else
				fsize = (ULONG)srcf->f_dentry->d_inode->i_size;
#endif
				if (buf_size < (fsize + 1))
					buf_size = fsize + 1;
#endif /* OS_ABL_SUPPORT */
			os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);
			if (buffer) {
				memset(buffer, 0x00, buf_size);
				retval =RtmpOSFileRead(srcf, buffer, buf_size - 1);
				if (retval > 0)
				{
					RTMPSetProfileParameters(pAd, buffer);
					retval = NDIS_STATUS_SUCCESS;
				}
				else
					DBGPRINT(RT_DEBUG_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));
				os_free_mem(NULL, buffer);
			} else 
				retval = NDIS_STATUS_FAILURE;

			if (RtmpOSFileClose(srcf) != 0)
			{
				retval = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
			}
		}
		
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

#ifdef HOSTAPD_SUPPORT
	for (i = 0; i < pAd->ApCfg.BssidNum; i++)
	{
		pAd->ApCfg.MBSSID[i].Hostapd = Hostapd_Diable;
		DBGPRINT(RT_DEBUG_TRACE, ("Reset ra%d hostapd support=FLASE", i));
	}
#endif /*HOSTAPD_SUPPORT */

#ifdef SINGLE_SKU_V2
	RTMPSetSingleSKUParameters(pAd);
#endif /* SINGLE_SKU_V2 */

	return (retval);

}


void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state)
{	
	pAd->IndicateMediaState = media_state;

#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		else
		{							
			RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0); 		
		}	
#endif /* SYSTEM_LOG_SUPPORT */
}


void tbtt_tasklet(unsigned long data)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, tbtt_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
		PRTMP_ADAPTER pAd = (RTMP_ADAPTER *)data;
#endif /* WORKQUEUE_BH */

#ifdef RTMP_MAC_PCI
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef AP_QLOAD_SUPPORT
		/* update channel utilization */
		QBSS_LoadUpdate(pAd, 0);
#endif /* AP_QLOAD_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
		RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	}
#endif /* RTMP_MAC_PCI */

	if (pAd->OpMode == OPMODE_AP)
	{
		/* step 7 - if DTIM, then move backlogged bcast/mcast frames from PSQ to TXQ whenever DtimCount==0 */
#ifdef RTMP_MAC_PCI
		/* 
			NOTE:
			This updated BEACON frame will be sent at "next" TBTT instead of at cureent TBTT. The reason is
			because ASIC already fetch the BEACON content down to TX FIFO before driver can make any
			modification. To compenstate this effect, the actual time to deilver PSQ frames will be
			at the time that we wrapping around DtimCount from 0 to DtimPeriod-1
		*/
		if (pAd->ApCfg.DtimCount == 0)
#endif /* RTMP_MAC_PCI */
		{	
#ifdef RTMP_MAC_PCI
			QUEUE_ENTRY *pEntry;	
			QUEUE_ENTRY *pTail;
			UINT count = 0;
			unsigned long IrqFlags;			
#endif /* RTMP_MAC_PCI */
		
#ifndef MT_MAC
			BOOLEAN bPS = FALSE;
#endif
			
#ifdef MT_MAC
			UINT apidx = 0, deq_cnt = 0;
			
			if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->MacTab.fAnyStationInPsm == TRUE))
			{
#ifdef USE_BMC
				#define MAX_BMCCNT 16
				int max_bss_cnt = 0;
				UINT mac_val = 0;

				/* BMC Flush */
				mac_val = 0x7fff0001;
                if (AsicSetBmcQCR(
				
                        pAd,
                        BMC_FLUSH,
                        CR_WRITE,
                        apidx,
                        &mac_val) == FALSE)
				
				{
					return;
				}
				
				if ((pAd->ApCfg.BssidNum == 0) || (pAd->ApCfg.BssidNum == 1))
				{
					max_bss_cnt = 0xf;
				}
				else
				{
					max_bss_cnt = MAX_BMCCNT / pAd->ApCfg.BssidNum;
				}
#endif
				for(apidx=0;apidx<pAd->ApCfg.BssidNum;apidx++)		
            	{
	                BSS_STRUCT *pMbss;
					UINT wcid = 0;
					STA_TR_ENTRY *tr_entry = NULL;
			
					pMbss = &pAd->ApCfg.MBSSID[apidx];
				
					wcid = pMbss->wdev.tr_tb_idx;
					tr_entry = &pAd->MacTab.tr_entry[wcid]; 
				
					if (tr_entry->tx_queue[QID_AC_BE].Head != NULL)
					{
#ifdef USE_BMC
						UINT bmc_cnt = 0;

                        if (AsicSetBmcQCR(pAd, BMC_CNT_UPDATE, CR_READ, apidx, &mac_val)
                            == FALSE)
                        {
                            return;
                        }
						if ((apidx >= 0) && (apidx <= 4))
						{
							if (apidx == 0)
								bmc_cnt = mac_val & 0xf;
							else 
								bmc_cnt = (mac_val >> (12+ (4*apidx))) & 0xf;
						}
						else if ((apidx >= 5) && (apidx <= 12))
						{
							bmc_cnt = (mac_val >> (4*(apidx-5))) & 0xf;
						}
						else if ((apidx >=13) && (apidx <= 15))
						{
							bmc_cnt = (mac_val >> (4*(apidx-13))) & 0xf;
						}

						
						if (bmc_cnt >= max_bss_cnt)
							deq_cnt = 0;
						else
							deq_cnt = max_bss_cnt - bmc_cnt;
						
						if (tr_entry->tx_queue[QID_AC_BE].Number <= deq_cnt)
#endif /* USE_BMC */
							deq_cnt = tr_entry->tx_queue[QID_AC_BE].Number;


#ifdef RTMP_MAC_PCI
						RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
						pEntry = tr_entry->tx_queue[QID_AC_BE].Head;
						pTail = tr_entry->tx_queue[QID_AC_BE].Tail;

						/* Only last pkt no need to mark the more bit */
						if (pEntry!=NULL && (tr_entry->tx_queue[QID_AC_BE].Number >=2) && deq_cnt >=2) 
						{
							count = 1; //first pkt
							
							while((pEntry!=pTail) && (count<deq_cnt))
							{
								RTMP_SET_PACKET_MOREDATA(QUEUE_ENTRY_TO_PACKET(pEntry), TRUE);
								pEntry = pEntry->Next;
								count ++;
							}
						}
						RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

						if (deq_cnt)
						{
							RTMPDeQueuePacket(pAd, FALSE, QID_AC_BE, wcid, deq_cnt);
#ifdef DATA_QUEUE_RESERVE
							if (pAd->bDump)
								DBGPRINT(RT_DEBUG_WARN, (" %s: bss:%u, deq_cnt = %u\n", __FUNCTION__, apidx, deq_cnt));
#endif /* DATA_QUEUE_RESERVE */	
						}
					}
			
					if (WLAN_MR_TIM_BCMC_GET(apidx) == 0x01)
					{
						if  ( (tr_entry->tx_queue[QID_AC_BE].Head == NULL) && 
							(tr_entry->EntryType == ENTRY_CAT_MCAST))
						{
							WLAN_MR_TIM_BCMC_CLEAR(tr_entry->func_tb_idx);	/* clear MCAST/BCAST TIM bit */
							DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_UAPSD, ("%s: clear MCAST/BCAST TIM bit \n", __FUNCTION__));
						}
					}
				}
#ifdef USE_BMC				
				/* BMC start */
                mac_val = 0x7fff0001;
                if (AsicSetBmcQCR(pAd, BMC_ENABLE, CR_WRITE, apidx, &mac_val)
                    == FALSE)
                {
                    return;
                }
#endif				
			}
#else /* MT_MAC */
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head)
			{
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (pAd->TxSwQMaxLen + MAX_PACKETS_IN_MCAST_PS_QUEUE))
				{
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					/*if(pAd->MacTab.McastPsQueue.Number) */
					if (count)
					{
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
						RTMP_SET_PACKET_TXTYPE(pEntry, TX_LEGACY_FRAME);
					}
					InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
					count++;
				}
				else
				{
					break;
				}
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			

			if (pAd->MacTab.McastPsQueue.Number == 0)
			{			
		                UINT bss_index;

                		/* clear MCAST/BCAST backlog bit for all BSS */
				for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
			}
			pAd->MacTab.PsQIdleCount = 0;

			if (bPS == TRUE) 
			{
				// TODO: shiang-usw, modify the WCID_ALL to pMBss->tr_entry because we need to tx B/Mcast frame here!!
				RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
#endif /* !MT_MAC */			
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

#ifdef INF_PPA_SUPPORT
static INT process_nbns_packet(
	IN PRTMP_ADAPTER 	pAd, 
	IN struct sk_buff 		*skb)
{
	UCHAR *data;
	USHORT *eth_type;

	data = (UCHAR *)eth_hdr(skb);
	if (data == 0)
	{
		data = (UCHAR *)skb->data;
		if (data == 0)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error\n", __FUNCTION__));
			return 1;
		}
	}
	
	eth_type = (USHORT *)&data[12];
	if (*eth_type == cpu_to_be16(ETH_P_IP))
	{
		INT ip_h_len;
		UCHAR *ip_h;
		UCHAR *udp_h;
		USHORT dport, host_dport;
	    
		ip_h = data + 14;
		ip_h_len = (ip_h[0] & 0x0f)*4;

		if (ip_h[9] == 0x11) /* UDP */
		{
			udp_h = ip_h + ip_h_len;
			memcpy(&dport, udp_h + 2, 2);
			host_dport = ntohs(dport);
			if ((host_dport == 67) || (host_dport == 68)) /* DHCP */
			{
				return 0;          
			}
		}
	}
    	else if ((data[12] == 0x88) && (data[13] == 0x8e)) /* EAPOL */
	{ 
		return 0;
    	}
	return 1;
}
#endif /* INF_PPA_SUPPORT */

#if defined (CONFIG_WIFI_PKT_FWD)
struct net_device *rlt_dev_get_by_name(const char *name)
{
	struct net_device *dev;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	dev = dev_get_by_name(&init_net, name);
#else
	dev = dev_get_by_name(name);
#endif

	if (dev)
		dev_put(dev);
	
	return dev;

}


VOID ApCliLinkCoverRxPolicy(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	OUT BOOLEAN *DropPacket)
{
	VOID *opp_band_tbl = NULL;
	VOID *band_tbl = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pInvalidEntry = NULL;
	REPEATER_CLIENT_ENTRY *pOtherBandReptEntry = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
	UCHAR *pPktHdr = NULL;
	UCHAR isLinkValid;
	
	pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);

	if (wf_fwd_feedback_map_table)
		wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);
	
	if (opp_band_tbl == NULL)
		return;

	if (IS_GROUP_MAC(pPktHdr)) {
		pInvalidEntry = RepeaterInvaildMacLookup(pAd, pPktHdr+6);
		pOtherBandReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pPktHdr+6, FALSE, &isLinkValid);

		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL)) {
			DBGPRINT(RT_DEBUG_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
			*DropPacket = TRUE;
		}
	}
}
#endif /* CONFIG_WIFI_PKT_FWD */
void announce_802_3_packet(
	IN VOID *pAdSrc,
	IN PNDIS_PACKET pNetPkt,
	IN UCHAR OpMode)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct sk_buff *pRxPkt;

#ifdef DOT11V_WNM_SUPPORT
#endif /* DOT11V_WNM_SUPPORT */

	ASSERT(pNetPkt);
	MEM_DBG_PKT_FREE_INC(pNetPkt);

	pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		if (RTMP_MATPktRxNeedConvert(pAd, pRxPkt->dev))
		{
#if defined (CONFIG_WIFI_PKT_FWD)
			BOOLEAN	 need_drop = FALSE;

			ApCliLinkCoverRxPolicy(pAd, pNetPkt, &need_drop);
			
			if (need_drop == TRUE) {
				RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
				return;
			}
#endif /* CONFIG_WIFI_PKT_FWD */
			RTMP_MATEngineRxHandle(pAd, pNetPkt, 0);
		}
#endif /* MAT_SUPPORT */
	}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	/* Push up the protocol stack */
#ifdef CONFIG_AP_SUPPORT
#if defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570)
	{
		extern int (*pToUpperLayerPktSent)(PNDIS_PACKET *pSkb);
		pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
		pToUpperLayerPktSent(pNetPkt);
		return;
	}
#endif /* defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570) */
#endif /* CONFIG_AP_SUPPORT */

#ifdef IKANOS_VX_1X0
	{
		IKANOS_DataFrameRx(pAd, pNetPkt);
		return;
	}
#endif /* IKANOS_VX_1X0 */

#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE)) 
		{
			INT retVal, ret = 0;
			UINT ppa_flags = 0;
			
			retVal = process_nbns_packet(pAd, pNetPkt);
			
			if (retVal > 0)
			{
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pNetPkt, pNetPkt->len, ppa_flags);
				if (ret == 0)
				{
					pRxPkt = NULL;
					return;
				}
				netif_rx(pRxPkt);
			}
			else if (retVal == 0)
			{
				pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
				netif_rx(pRxPkt);
			}
			else
			{
				dev_kfree_skb_any(pRxPkt);
				MEM_DBG_PKT_FREE_INC(pAd);
			}
		}	
		else
		{
			pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
			netif_rx(pRxPkt);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */

#ifdef CONFIG_RT2880_BRIDGING_ONLY
	PACKET_CB_ASSIGN(pRxPkt, 22) = 0xa8;
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
	if(ra_classifier_hook_rx!= NULL)
	{
		ra_classifier_hook_rx(pNetPkt, classifier_cur_cycle);
	}
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#if !defined (CONFIG_RA_NAT_NONE)
	/*
	 * ra_sw_nat_hook_rx return 1 --> continue
	 * ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
	*/
	if (ra_sw_nat_hook_rx != NULL)
	{
		pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
		FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_EXTIF;
		if (ra_sw_nat_hook_rx(pRxPkt))
		{
			FOE_MAGIC_TAG(pRxPkt) = 0;
			netif_rx(pRxPkt);
		}
		
		return;
	}
#endif
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	if (BG_FTPH_PacketFromApHandle(pNetPkt) == 0)
		return;
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
	if (ra_sw_nat_hook_set_magic)
		ra_sw_nat_hook_set_magic(pNetPkt, FOE_MAGIC_WLAN);

	if (ra_sw_nat_hook_rx != NULL) 
	{
		if (ra_sw_nat_hook_rx(pNetPkt) == 0)
			return;
	}
#endif

#if defined (CONFIG_WIFI_PKT_FWD)
		{
			struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pRxPkt);
			PNET_DEV rx_dev = RtmpOsPktNetDevGet(pRxPkt);

			/* all incoming packets should set CB to mark off which net device received and in which band */
			if ((rx_dev == rlt_dev_get_by_name("rai0")) || (rx_dev == rlt_dev_get_by_name("apclii0")) ||
				(rx_dev == rlt_dev_get_by_name("rai1")) || (rx_dev == rlt_dev_get_by_name("apclii1"))) {
				RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G);

				if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_CLIENT);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_AP);
			} else {
				RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_2G);
			
				if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_CLIENT);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_AP);
			}

			if (wf_fwd_rx_hook != NULL)
			{
				struct ethhdr *mh = eth_hdr(pRxPkt);
				int ret = 0;
				
				if ((mh->h_dest[0] & 0x1) == 0x1)
				{
					if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5)) 
					{
#ifdef MAC_REPEATER_SUPPORT
						if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
							(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source) == TRUE)) {
							RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
							return;
						}
						else
#endif /* MAC_REPEATER_SUPPORT */
						{
							VOID *opp_band_tbl = NULL;
							VOID *band_tbl = NULL;

							if (wf_fwd_feedback_map_table)
								wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);
							
							
							if ((opp_band_tbl != NULL) 
								&& MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)) {
								RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
								return;
							}
						}
					}
				}
				
				ret = wf_fwd_rx_hook(pRxPkt);

				if (ret == 0) {
					return;
				} else if (ret == 2) {
					RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
#endif /* CONFIG_WIFI_PKT_FWD */

	netif_rx(pRxPkt);
}


#ifdef CONFIG_SNIFFER_SUPPORT

INT Monitor_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_OPEN, 0, dev_p, 0);


	//Monitor_Open(pAd,dev_p);

	return 0;
}

INT Monitor_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_CLOSE, 0, dev_p, 0);
	//Monitor_Close(pAd,dev_p);

	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0;
}


VOID RT28xx_Monitor_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;	

	/* init operation functions */
	NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = Monitor_VirtualIF_Open;
	netDevOpHook.stop = Monitor_VirtualIF_Close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	DBGPRINT(RT_DEBUG_OFF, ("%s: %d !!!!####!!!!!!\n",__FUNCTION__, __LINE__)); 
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_INIT,	0, &netDevOpHook, 0);

}
VOID RT28xx_Monitor_Remove(VOID *pAd)
{

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_REMOVE, 0, NULL, 0);
	
}

void STA_MonPktSend(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	PNET_DEV pNetDev;
	PNDIS_PACKET pRxPacket;
	PHEADER_802_11 pHeader;
	USHORT DataSize;
	UINT32 MaxRssi;
	UINT32 timestamp = 0;
	CHAR RssiForRadiotap = 0;
	UCHAR L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, AMPDU, STBC, RSSI1;
	UCHAR BssMonitorFlag11n, Channel, CentralChannel = 0;
	UCHAR *pData, *pDevName;
	UCHAR sniffer_type = pAd->sniffer_ctl.sniffer_type;
	UCHAR sideband_index = 0;

	ASSERT(pRxBlk->pRxPacket);
	
	if (pRxBlk->DataSize < 10) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too small! (%d)\n", __FUNCTION__, pRxBlk->DataSize));
		goto err_free_sk_buff;
	}

	if (sniffer_type == RADIOTAP_TYPE) {
		if (pRxBlk->DataSize + sizeof(struct mtk_radiotap_header) > RX_BUFFER_AGGRESIZE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(struct mtk_radiotap_header)));
			goto err_free_sk_buff;
		}
	}

	if (sniffer_type == PRISM_TYPE) {
		if (pRxBlk->DataSize + sizeof(wlan_ng_prism2_header) > RX_BUFFER_AGGRESIZE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(wlan_ng_prism2_header)));
			goto err_free_sk_buff;
		}
	}

	MaxRssi = RTMPMaxRssi(pAd,
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2));

	if (sniffer_type == RADIOTAP_TYPE){
		RssiForRadiotap = RTMPMaxRssi(pAd,
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2));
	}

	pNetDev = pAd->monitor_ctrl.monitor_wdev.if_dev;  //send packet to mon0
	//pNetDev = get_netdev_from_bssid(pAd, BSS0);
	pRxPacket = pRxBlk->pRxPacket;
	pHeader = pRxBlk->pHeader;
	pData = pRxBlk->pData;
	DataSize = pRxBlk->DataSize;
	L2PAD = pRxBlk->pRxInfo->L2PAD;
	PHYMODE = pRxBlk->rx_rate.field.MODE;
	BW = pRxBlk->rx_rate.field.BW;

	ShortGI = pRxBlk->rx_rate.field.ShortGI;
	MCS = pRxBlk->rx_rate.field.MCS;
	LDPC = pRxBlk->rx_rate.field.ldpc;
	if (pAd->chipCap.hif_type == HIF_RLT)
		LDPC_EX_SYM = pRxBlk->ldpc_ex_sym;
	else
		LDPC_EX_SYM = 0;
		
	AMPDU = pRxBlk->pRxInfo->AMPDU;
	STBC = pRxBlk->rx_rate.field.STBC;
	RSSI1 = pRxBlk->rx_signal.raw_rssi[1];
	//if(pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12] != 0)
#ifdef RLT_MAC
	NdisCopyMemory(&timestamp,&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12],4);
#endif
#ifdef MT_MAC
	timestamp = pRxBlk->TimeStamp;
#endif

	BssMonitorFlag11n = 0;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	BssMonitorFlag11n = (pAd->StaCfg.BssMonitorFlag & MONITOR_FLAG_11N_SNIFFER);
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	pDevName = (UCHAR *)RtmpOsGetNetDevName(pAd->net_dev);
	Channel = pAd->CommonCfg.Channel;

	if (BW == BW_20)
		CentralChannel = Channel;
	else if (BW == BW_40)
	CentralChannel = pAd->CommonCfg.CentralChannel;
#ifdef DOT11_VHT_AC
	else if (BW == BW_80)
		CentralChannel = pAd->CommonCfg.vht_cent_ch; 
#endif /* DOT11_VHT_AC */

#ifdef DOT11_VHT_AC
	if (BW == BW_80) {
		sideband_index = vht_prim_ch_idx(CentralChannel, Channel);
	}
#endif /* DOT11_VHT_AC */

#ifdef SNIFFER_MIB_CMD
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	SNIFFER_MAC_CTRL *pMACEntry;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	BOOLEAN  check = FALSE;
	
	RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);

	if (psniffer_mib_ctrl->MAC_ListNum == 0) // no set mac to sniffer, means sniffer all
	{
		check = TRUE;
		RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);
		goto report_upper;
	}
	
	DlListForEach(pMACEntry, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
	{
		//only conpare addr1, addr2 , no addr3
		if (NdisEqualMemory(pHeader->Addr1, pMACEntry->MACAddr, MAC_ADDR_LEN) 
			|| NdisEqualMemory(pHeader->Addr2, pMACEntry->MACAddr, MAC_ADDR_LEN)
			)
		{
#ifdef ALL_NET_EVENT
			PSNIFFER_MAC_NOTIFY_T pMACEntryNotify = &pMACEntry->rNotify;
			if(pMACEntryNotify->i4RxPacketConut >= SNIFFER_MAC_MAX_RX_PACKET_COUNT)
			{
				BOOLEAN fgCancelled;
				INT32 i4AvgRssi = pMACEntryNotify->i4RssiAccum/pMACEntryNotify->i4RxPacketConut;
				UCHAR bssid[MAC_ADDR_LEN] = {0}; //report zero bssid, to prevent too many check
				wext_send_event(pNetDev,pMACEntry->MACAddr,bssid,Channel,i4AvgRssi,FBT_LINK_STA_FOUND_NOTIFY);
				
				// Clear data and reset timer for next collection.
				NdisZeroMemory(pMACEntryNotify,sizeof(*pMACEntryNotify));
				RTMPCancelTimer(&pMACEntry->rNotifyTimer, &fgCancelled);
				RTMPSetTimer(&pMACEntry->rNotifyTimer, SNIFFER_MAC_TIMEOUT);
			}
			else
			{
				RSSI_SAMPLE rRssiSample;
				NdisZeroMemory(&rRssiSample,sizeof(rRssiSample));
				rRssiSample.AvgRssi[0] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0);
				rRssiSample.AvgRssi[1] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1);
				rRssiSample.AvgRssi[2] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2);
					
				pMACEntryNotify->pNetDev = pNetDev;
				pMACEntryNotify->u4Channel = Channel;
				pMACEntryNotify->i4RssiAccum += RTMPAvgRssi(pAd,&rRssiSample);
				pMACEntryNotify->i4RxPacketConut++;
			}
#endif /* ALL_NET_EVENT */
			check = TRUE;
		}
		//DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC [%d]=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,i++, PRINT_MAC(pMACEntry->MACAddr)));
	}
	RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);

	report_upper:
		
	if (check == FALSE)
	{
		goto err_free_sk_buff;
	}
}
#endif /* SNIFFER_MIB_CMD */

	if (sniffer_type == RADIOTAP_TYPE) {
		send_radiotap_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
									  L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, 
									  AMPDU, STBC, RSSI1, pDevName, Channel, CentralChannel,
							 		  sideband_index, RssiForRadiotap,timestamp);
	}

	if (sniffer_type == PRISM_TYPE) {
		send_prism_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
						L2PAD, PHYMODE, BW, ShortGI, MCS, AMPDU, STBC, RSSI1,
						BssMonitorFlag11n, pDevName, Channel, CentralChannel,
						MaxRssi);
	}

	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}
#endif /* CONFIG_SNIFFER_SUPPORT */


extern NDIS_SPIN_LOCK TimerSemLock;

VOID RTMPFreeAdapter(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	int index;	

	os_cookie=(POS_COOKIE)pAd->OS_Cookie;
#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
	if (pAd->eebuf && (pAd->eebuf != pAd->EEPROMImage))
	{
		os_free_mem(NULL, pAd->eebuf);
		pAd->eebuf = NULL;
	}
#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */
	NdisFreeSpinLock(&pAd->MgmtRingLock);
	
#ifdef RTMP_MAC_PCI
	for (index = 0; index < NUM_OF_RX_RING; index++)
		NdisFreeSpinLock(&pAd->RxRingLock[index]);

	NdisFreeSpinLock(&pAd->McuCmdLock);
#endif /* RTMP_MAC_PCI */

#if defined(RT3290) || defined(RLT_MAC)
	NdisFreeSpinLock(&pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

	for (index =0 ; index < NUM_OF_TX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}
	
	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef RTMP_MAC_PCI
	NdisFreeSpinLock(&pAd->LockInterrupt);
#ifdef CONFIG_ANDES_SUPPORT
	NdisFreeSpinLock(&pAd->CtrlRingLock);
#endif

#ifdef MT_MAC
	NdisFreeSpinLock(&pAd->BcnRingLock);
#endif /* MT_MAC */

	NdisFreeSpinLock(&pAd->tssi_lock);
#endif /* RTMP_MAC_PCI */


#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); /* OS_ABL_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef DOT11_N_SUPPORT
	NdisFreeSpinLock(&pAd->mpdu_blk_pool.lock);
#endif /* DOT11_N_SUPPORT */

	if (pAd->iw_stats)
	{
		os_free_mem(NULL, pAd->iw_stats);
		pAd->iw_stats = NULL;
	}

	NdisFreeSpinLock(&TimerSemLock);

#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
    if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->OpMode == OPMODE_AP))
	{
        BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
        ASSERT(pMbss);
		if (pMbss) {
			wdev_bcn_buf_deinit(pAd, &pMbss->bcn_buf);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
			return;
		}
	}
#endif
#endif
#ifdef CONFIG_ATE
#endif /* CONFIG_ATE */

	RTMP_OS_FREE_TIMER(pAd);
	RTMP_OS_FREE_LOCK(pAd);
	RTMP_OS_FREE_TASKLET(pAd);
	RTMP_OS_FREE_TASK(pAd);
	RTMP_OS_FREE_SEM(pAd);
	RTMP_OS_FREE_ATOMIC(pAd);

	RtmpOsVfree(pAd); /* pci_free_consistent(os_cookie->pci_dev,sizeof(RTMP_ADAPTER),pAd,os_cookie->pAd_pa); */
	if (os_cookie)
		os_free_mem(NULL, os_cookie);
}



int RTMPSendPackets(
	IN NDIS_HANDLE dev_hnd,
	IN PPNDIS_PACKET pkt_list,
	IN UINT pkt_cnt,
	IN UINT32 pkt_total_len,
	IN RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	struct wifi_dev *wdev = (struct wifi_dev *)dev_hnd;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = pkt_list[0];

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if(pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): pAd is NULL!\n", __FUNCTION__));
		return 0;
	}

	INC_COUNTER64(pAd->WlanCounters.TransmitCountFrmOs);

	if (!pPacket)
		return 0;

#ifdef WSC_NFC_SUPPORT
        {
                struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pPacket);
                USHORT protocol = 0;
                protocol = ntohs(pRxPkt->protocol);
                if (protocol == 0x6605)
                {
                        NfcParseRspCommand(pAd, pRxPkt->data, pRxPkt->len);
                        RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
                        return 0;
                }
                else
                {
                        printk("%04x\n", protocol);
                }
        }
#endif /* WSC_NFC_SUPPORT */


	if (pkt_total_len < 14)
	{
		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pPacket), pkt_total_len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}

#ifdef CONFIG_ATE
	// TODO: shiang-usw, can remove this?
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}
#endif /* CONFIG_ATE */


#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);
	if (*(int*)(GET_OS_PKT_CB(pPacket)) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
	}
#endif /* CONFIG_5VT_ENHANCE */


#if defined (CONFIG_WIFI_PKT_FWD)
	if (wf_fwd_tx_hook != NULL)
	{
		if (wf_fwd_tx_hook(pPacket) == 1)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}
	}
#endif /* CONFIG_WIFI_PKT_FWD */

	return wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1, wdev);
}


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Driver pre-Ioctl for AP.

Arguments:
	pAdSrc			- WLAN control block pointer
	pCB				- the IOCTL parameters

Return Value:
	NDIS_STATUS_SUCCESS	- IOCTL OK
	Otherwise			- IOCTL fail

Note:
========================================================================
*/
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB)
{
	RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pCB;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	INT cmd = 0xff;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if((pConfig->priv_flags == INT_MAIN) && !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		if (pConfig->pCmdData == NULL)
			return Status;
		if (RtPrivIoctlSetVal() == pConfig->CmdId_RTPRIV_IOCTL_SET)
		{
			if (TRUE
#ifdef CONFIG_APSTA_MIXED_SUPPORT
				&& (strstr(pConfig->pCmdData, "OpMode") == NULL)
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef SINGLE_SKU
				&& (strstr(pConfig->pCmdData, "ModuleTxpower") == NULL)
#endif /* SINGLE_SKU */
			)
			{
				return -ENETDOWN;
			}
		}
		else
			return -ENETDOWN;
    }
	

    /* determine this ioctl command is comming from which interface. */
    if (pConfig->priv_flags == INT_MAIN)
    {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		
    }
    else if (pConfig->priv_flags == INT_MBSSID)
    {
		pObj->ioctl_if_type = INT_MBSSID;
/*    	if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MBSSID */
		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) /* sample */
    	{
	        for (index = 1; index < pAd->ApCfg.BssidNum; index++)
	    	{
	    	    if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pConfig->net_dev)
	    	    {
	    	        pObj->ioctl_if = index;
	    	        break;
	    	    }
	    	}
	    	
	        /* Interface not found! */
	        if(index == pAd->ApCfg.BssidNum)
	            return -ENETDOWN;
            
	    }
	    else    /* ioctl command from I/F(ra0) */
	    {
    	    pObj->ioctl_if = MAIN_MBSSID;
	    }
        MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
    }
#ifdef WDS_SUPPORT
	else if (pConfig->priv_flags == INT_WDS)
	{
		pObj->ioctl_if_type = INT_WDS;
		for(index = 0; index < MAX_WDS_ENTRY; index++)
		{
			if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pConfig->net_dev)
			{
				pObj->ioctl_if = index;
				break;
			}
			
			if(index == MAX_WDS_ENTRY)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find wds I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
	}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (pConfig->priv_flags == INT_APCLI)
	{
		pObj->ioctl_if_type = INT_APCLI;
		for (index = 0; index < MAX_APCLI_NUM; index++)
		{
			if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == pConfig->net_dev)
			{
				pObj->ioctl_if = index;
				break;
			}

			if(index == MAX_APCLI_NUM)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find Apcli I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif /* APCLI_SUPPORT */
    else
    {
	/* DBGPRINT(RT_DEBUG_WARN, ("IOCTL is not supported in WDS interface\n")); */	
    	return -EOPNOTSUPP;
    }

	pConfig->apidx = pObj->ioctl_if;
	
	return Status;
}


VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	RTMP_STRING *msg)
{
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
	}
}


VOID IAPP_L2_UpdatePostCtrl(RTMP_ADAPTER *pAd, UINT8 *mac_p, INT wdev_idx)
{
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
VOID AP_WDS_KeyNameMakeUp(
	IN	RTMP_STRING *pKey,
	IN	UINT32						KeyMaxSize,
	IN	INT							KeyId)
{
	snprintf(pKey, KeyMaxSize, "Wds%dKey", KeyId);
}
#endif /* WDS_SUPPORT */

