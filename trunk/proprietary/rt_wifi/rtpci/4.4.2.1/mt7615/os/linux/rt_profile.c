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

#ifdef RTMP_UDMA_SUPPORT
#include "rt_udma.h"
#endif/*RTMP_UDMA_SUPPORT*/

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
#include <linux/foe_hook.h>
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */


#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../net/nat/hw_nat/frame_engine.h"
#endif

/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
/* get br-lan's netmask */
#include <linux/inetdevice.h> 
#include <linux/netdevice.h> 
#endif

#define BSSID_WCID_TO_REMOVE 1

struct dev_type_name_map{
	INT type;
	RTMP_STRING *prefix[MAX_NUM_OF_INF];
};

#if defined(RT_CFG80211_SUPPORT)
#define SECOND_INF_MAIN_DEV_NAME		"wlani"
#define SECOND_INF_MBSSID_DEV_NAME	"wlani"
#else
#define SECOND_INF_MAIN_DEV_NAME		"rai"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#endif
#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME		"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME		"moni"
#define SECOND_INF_MSTA_DEV_NAME    "rai"

#if defined(RT_CFG80211_SUPPORT)
#define THIRD_INF_MAIN_DEV_NAME		"wlane"
#define THIRD_INF_MBSSID_DEV_NAME	"wlane"
#else
#define THIRD_INF_MAIN_DEV_NAME		"rae"
#define THIRD_INF_MBSSID_DEV_NAME	"rae"
#endif
#define THIRD_INF_WDS_DEV_NAME		"wdse"
#define THIRD_INF_APCLI_DEV_NAME	"apclie"
#define THIRD_INF_MESH_DEV_NAME		"meshe"
#define THIRD_INF_P2P_DEV_NAME		"p2pe"
#define THIRD_INF_MONITOR_DEV_NAME		"mone"
#define THIRD_INF_MSTA_DEV_NAME    "rae"


#define xdef_to_str(s)   def_to_str(s)
#define def_to_str(s)    #s

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)) && defined (CONFIG_ANDROID)
#define FIRST_EEPROM_FILE_PATH	"/data/router/etc_ro/Wireless/RT2860/"
#define FIRST_AP_PROFILE_PATH		"/data/router/etc/Wireless/RT2860/RT2860.dat"
#define FIRST_STA_PROFILE_PATH      "/data/router/etc/Wireless/RT2860/RT2860.dat"
#define FIRST_CHIP_ID	xdef_to_str(MT_FIRST_CARD)


#define SECOND_EEPROM_FILE_PATH	"/data/router/etc_ro/Wireless/iNIC/"
#define SECOND_AP_PROFILE_PATH	"/data/router/etc/Wireless/iNIC/iNIC_ap.dat"
#define SECOND_STA_PROFILE_PATH "/data/router/etc/Wireless/iNIC/iNIC_sta.dat"

#define SECOND_CHIP_ID	xdef_to_str(MT_SECOND_CARD)

#define THIRD_EEPROM_FILE_PATH	"/data/router/etc_ro/Wireless/WIFI3/"
#define THIRD_AP_PROFILE_PATH	"/data/router/etc/Wireless/WIFI3/RT2870AP.dat"
#define THIRD_STA_PROFILE_PATH "/data/router/etc/Wireless/WIFI3/RT2870AP.dat"

#define THIRD_CHIP_ID	xdef_to_str(MT_THIRD_CARD)

#else
#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/"
#define FIRST_AP_PROFILE_PATH		"/etc/Wireless/RT2860/RT2860AP.dat"
#define FIRST_STA_PROFILE_PATH      "/etc/Wireless/RT2860/RT2860AP.dat"
#define FIRST_CHIP_ID	xdef_to_str(MT_FIRST_CARD)

#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/"
#define SECOND_AP_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap.dat"
#define SECOND_STA_PROFILE_PATH "/etc/Wireless/iNIC/iNIC_sta.dat"

#define SECOND_CHIP_ID	xdef_to_str(MT_SECOND_CARD)

#define THIRD_EEPROM_FILE_PATH	"/etc_ro/Wireless/WIFI3/"
#define THIRD_AP_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP.dat"
#define THIRD_STA_PROFILE_PATH "/etc/Wireless/WIFI3/RT2870AP.dat"

#define THIRD_CHIP_ID	xdef_to_str(MT_THIRD_CARD)
#endif /* CONFIG_ANDROID */

static struct dev_type_name_map prefix_map[] =
{
	{INT_MAIN, 		{INF_MAIN_DEV_NAME, SECOND_INF_MAIN_DEV_NAME,THIRD_INF_MAIN_DEV_NAME}},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{INT_MBSSID, 	{INF_MBSSID_DEV_NAME, SECOND_INF_MBSSID_DEV_NAME,THIRD_INF_MBSSID_DEV_NAME}},
#endif /* MBSS_SUPPORT */
#ifdef APCLI_SUPPORT
	{INT_APCLI, 		{INF_APCLI_DEV_NAME, SECOND_INF_APCLI_DEV_NAME,THIRD_INF_APCLI_DEV_NAME}},
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	{INT_WDS, 		{INF_WDS_DEV_NAME, SECOND_INF_WDS_DEV_NAME,THIRD_INF_WDS_DEV_NAME}},
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	{0},
};

#ifdef MULTI_PROFILE
INT multi_profile_check(struct _RTMP_ADAPTER *ad, CHAR *final);
#endif /*MULTI_PROFILE*/

struct dev_id_name_map{
	INT chip_id;
	RTMP_STRING *chip_name;
};

static const struct dev_id_name_map id_name_list[]=
{
	{7610, "7610, 7610e 7610u"},

};

INT get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	INT idx = -1;
#if defined(MT_FIRST_CARD) && defined(MT_SECOND_CARD)
	INT first_card = 0, second_card = 0;

	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x\n", first_card, second_card, pAd->MACVersion));

	if (IS_RT8592(pAd))
		idx = 0;
	else if (IS_RT5392(pAd) || IS_MT76x0(pAd) || IS_MT76x2(pAd))
		idx = 1;
#endif /* defined(MT_FIRST_CARD) && defined(MT_SECOND_CARD) */

#if defined(MT_SECOND_CARD)
	if(IS_MT7637E(pAd))
		idx = 1;
#endif /* MT_SECOND_CARD */

	if (idx == -1)
#ifdef MULTI_INF_SUPPORT
		idx = multi_inf_get_idx((VOID *) pAd);
	else
#endif /* MULTI_INF_SUPPORT */
		idx = 0;


#if defined(MT_SECOND_CARD)
#if defined(CONFIG_FIRST_IF_MT7603E)
/*
	MT7603(ra0) + MT7615 (rai0) combination
*/
	if(IS_MT7615(pAd))
		idx = 1;
#endif /* defined(CONFIG_FIRST_IF_MT7603E) */
#endif /* defined(MT_SECOND_CARD) */

	pAd->dev_idx = idx;

	return idx;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid dev_idx(%d)!\n",
					__FUNCTION__, dev_idx));
		return NULL;
	}

	do {
		map = &prefix_map[type_idx];
		if (map->type == dev_type) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): dev_idx = %d, dev_name_prefix=%s\n",
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
#if defined(MT_FIRST_CARD) || defined(MT_SECOND_CARD) || defined(MT_THIRD_CARD)
        INT card_idx = get_dev_config_idx(pAd);
#endif /* MT_FIRST_CARD || MT_SECOND_CARD */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef MT_FIRST_CARD
			if (card_idx == 0)
			{
				src = FIRST_AP_PROFILE_PATH;
			}
			else
#endif /* MT_FIRST_CARD */
#ifdef MT_SECOND_CARD
			if (card_idx == 1)
			{
				src = SECOND_AP_PROFILE_PATH;
			}
			else
#endif /* MT_SECOND_CARD */
#ifdef MT_THIRD_CARD
			if (card_idx == 2)
			{
				src = THIRD_AP_PROFILE_PATH;
			}
			else
#endif /* MT_THIRD_CARD */

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

NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src = NULL;
	RTMP_OS_FD_EXT srcf;
	INT retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	RTMP_STRING *buffer = NULL;

#ifdef HOSTAPD_SUPPORT
	int i;
#endif /*HOSTAPD_SUPPORT */

	os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);
	if (!buffer) {
		return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(buffer, buf_size);
	/*if support multi-profile merge it*/
#ifdef MULTI_PROFILE
	if(multi_profile_check(pAd,buffer) == NDIS_STATUS_SUCCESS){
		RTMPSetProfileParameters(pAd, buffer);
		retval = NDIS_STATUS_SUCCESS;
	}else
#endif /*MULTI_PROFILE*/
	{
		src = get_dev_profile(pAd);
		if (src && *src)
		{
			srcf = os_file_open(src,O_RDONLY,0);

			if (srcf.Status)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", src));
			}
			else
			{
#ifndef OS_ABL_SUPPORT
				// TODO: need to roll back when convert into OSABL code
				if (srcf.fsize!= 0 && buf_size < (srcf.fsize + 1))
				{
					buf_size = srcf.fsize  + 1;
				}
#endif /* OS_ABL_SUPPORT */
					retval =os_file_read(srcf, buffer, buf_size - 1);
					if (retval > 0)
					{
						RTMPSetProfileParameters(pAd, buffer);
						retval = NDIS_STATUS_SUCCESS;
					}
					else
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));

				if (os_file_close(srcf) != 0)
				{
					retval = NDIS_STATUS_FAILURE;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
				}
			}

		}
	}

#ifdef HOSTAPD_SUPPORT
	for (i = 0; i < pAd->ApCfg.BssidNum; i++)
	{
		pAd->ApCfg.MBSSID[i].Hostapd=Hostapd_Diable;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Reset ra%d hostapd support=FLASE", i));
	}
#endif /*HOSTAPD_SUPPORT */

	os_free_mem(buffer);
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
		    UINT wcid = BSSID_WCID_TO_REMOVE;  //Pat: TODO
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[wcid].Addr, BSS0, 0);
		}
		else
		{
		    UINT wcid = BSSID_WCID_TO_REMOVE;  //Pat: TODO
			RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[wcid].Addr, BSS0, 0);
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

#ifdef RT_CFG80211_P2P_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#else
	if (pAd->OpMode == OPMODE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
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
#if defined(RTMP_MAC) || defined(RLT_MAC)
			QUEUE_ENTRY *pEntry;
			BOOLEAN bPS = FALSE;
			UINT count = 0;
			unsigned long IrqFlags;
#endif /* RTMP_MAC || RLT_MAC */

#ifdef MT_MAC
			UINT apidx = 0, deq_cnt = 0;
#ifdef USE_BMC
			UINT mac_val = 0;
#endif /* USE_BMC */
			if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->MacTab.fAnyStationInPsm == TRUE))
			{
#ifdef USE_BMC
				#define MAX_BMCCNT 16
				int fcnt = 0, max_bss_cnt = 0;

				/* BMC Flush */
				mac_val = 0x7fff0001;
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR1, mac_val);

				for (fcnt=0;fcnt<100;fcnt++)
				{
				RTMP_IO_READ32(pAd, ARB_BMCQCR1, &mac_val);
					if (mac_val == 0)
						break;
				}

				if (fcnt == 100)
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: flush not complete, flush cnt=%d\n", __FUNCTION__, fcnt));
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
#ifdef USE_BMC
					UINT bmc_cnt = 0;
#endif /* USE_BMC */
					STA_TR_ENTRY *tr_entry = NULL;

					pMbss = &pAd->ApCfg.MBSSID[apidx];

					wcid = pMbss->wdev.tr_tb_idx;
					tr_entry = &pAd->MacTab.tr_entry[wcid];

					if (tr_entry->tx_queue[QID_AC_BE].Head != NULL)
					{
#ifdef USE_BMC
						if (apidx <= 4)
						{
							RTMP_IO_READ32(pAd, ARB_BMCQCR2, &mac_val);
							if (apidx == 0)
								bmc_cnt = mac_val & 0xf;
							else
								bmc_cnt = (mac_val >> (12+ (4*apidx))) & 0xf;
						}
						else if ((apidx >= 5) && (apidx <= 12))
						{
							RTMP_IO_READ32(pAd, ARB_BMCQCR3, &mac_val);
							bmc_cnt = (mac_val >> (4*(apidx-5))) & 0xf;
						}
						else if ((apidx >=13) && (apidx <= 15))
						{
							RTMP_IO_READ32(pAd, ARB_BMCQCR3, &mac_val);
							bmc_cnt = (mac_val >> (4*(apidx-13))) & 0xf;
						}
						else
						{
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: apidx(%d) not support\n", __FUNCTION__, apidx));
		                    return;
						}

						if (bmc_cnt >= max_bss_cnt)
							deq_cnt = 0;
						else
							deq_cnt = max_bss_cnt - bmc_cnt;

						if (tr_entry->tx_queue[QID_AC_BE].Number <= deq_cnt)
#endif /* USE_BMC */
							deq_cnt = tr_entry->tx_queue[QID_AC_BE].Number;

						RTMPDeQueuePacket(pAd, FALSE, QID_AC_BE, wcid, deq_cnt);

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: bss:%d, deq_cnt = %d\n", __FUNCTION__, apidx, deq_cnt));
					}

					if (WLAN_MR_TIM_BCMC_GET(apidx) == 0x01)
					{
						if  ( (tr_entry->tx_queue[QID_AC_BE].Head == NULL) &&
							(tr_entry->EntryType == ENTRY_CAT_MCAST))
						{
							WLAN_MR_TIM_BCMC_CLEAR(tr_entry->func_tb_idx);	/* clear MCAST/BCAST TIM bit */
							MTWF_LOG(DBG_CAT_PS, CATPS_UAPSD, DBG_LVL_WARN, ("%s: clear MCAST/BCAST TIM bit \n", __FUNCTION__));
						}
					}
				}
#ifdef USE_BMC
				/* BMC start */
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR0, 0x7fff0001);
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
				RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error\n", __FUNCTION__));
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

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
struct net_device *rlt_dev_get_by_name(const char *name)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	return dev_get_by_name(&init_net, name);
#else
	return dev_get_by_name(name);
#endif
}

VOID ApCliLinkCoverRxPolicy(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	OUT BOOLEAN *DropPacket)
{
#ifdef MAC_REPEATER_SUPPORT
	void *opp_band_tbl =NULL;
	void *band_tbl =NULL;
	void *other_band_tbl = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pInvalidEntry = NULL;
	REPEATER_CLIENT_ENTRY *pOtherBandReptEntry = NULL;
	REPEATER_CLIENT_ENTRY *pAnotherBandReptEntry = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
	UCHAR *pPktHdr = NULL ;

	pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);

	if (wf_fwd_feedback_map_table)
		wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

	if ((opp_band_tbl == NULL) && (other_band_tbl == NULL))
		return;

	if (IS_GROUP_MAC(pPktHdr)) {
		pInvalidEntry = RepeaterInvaildMacLookup(pAd, pPktHdr+6);

		if (opp_band_tbl != NULL)
			pOtherBandReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pPktHdr+6, FALSE);

		if (other_band_tbl != NULL)
			pAnotherBandReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pPktHdr+6, FALSE);

		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL) || (pAnotherBandReptEntry != NULL)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
			*DropPacket = TRUE;
		}
	}
#endif /* MAC_REPEATER_SUPPORT */	
}
#endif /* CONFIG_WIFI_PKT_FWD */

/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
#if (!defined(CONFIG_RA_NAT_NONE)) || ( defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI) )
int GetBrLanNetMask(
	IN	RTMP_ADAPTER *pAd)
{ 
	struct net *net= &init_net;
	struct net_device *pNetDev;
	struct in_ifaddr *if_info; 
    	struct in_device *in_dev;

	/* old kernerl older than 2.6.21 didn't have for_each_netdev()*/
#ifndef for_each_netdev
	for(pNetDev=dev_base; pNetDev!=NULL; pNetDev=pNetDev->next)
#else
	for_each_netdev(net, pNetDev)
#endif
	{
		if (pNetDev->priv_flags == IFF_EBRIDGE)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Bridge Addr = %s!!!\n",
			pNetDev->name));
			break;
		}
	}
	in_dev = (struct in_device *)pNetDev->ip_ptr; 
	if( !in_dev )
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" in_dev is NULL!\n"));
		return 0
	}
	// get in_ifaddr
	if_info = in_dev->ifa_list; 
	if( if_info )
	{
		pAd->BrLanIpAddr = if_info->ifa_local;
		pAd->BrLanMask = if_info->ifa_mask;	
		pAd->isInitBrLan = 1;
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Device %s IP: %08X   mask: %08X\n","br-lan",pAd->BrLanIpAddr,pAd->BrLanMask));
	return 0;
}
#endif
#endif

void announce_802_3_packet(
	IN VOID *pAdSrc,
	IN PNDIS_PACKET pPacket,
	IN UCHAR OpMode)
{
	RTMP_ADAPTER *pAd = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
#if( defined(WH_EZ_SETUP) && (defined (CONFIG_WIFI_PKT_FWD) || defined (CONFIG_WIFI_PKT_FWD_MODULE)))
	BOOLEAN bypass_rx_fwd = FALSE;
#endif
	/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
#if (!defined(CONFIG_RA_NAT_NONE)) || ( defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI) )
	BOOLEAN 	 isToLan = FALSE;
#endif
#endif

	pAd =  (RTMP_ADAPTER *)pAdSrc;
	//MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=>%s(): OpMode=%d\n", __FUNCTION__, OpMode));
#ifdef DOT11V_WNM_SUPPORT
#endif /* DOT11V_WNM_SUPPORT */
	ASSERT(pPacket);
	MEM_DBG_PKT_FREE_INC(pPacket);

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		if (RTMP_MATPktRxNeedConvert(pAd, RtmpOsPktNetDevGet(pRxPkt))) {
#if defined (CONFIG_WIFI_PKT_FWD)
		if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) {
			BOOLEAN	 need_drop = FALSE;

			ApCliLinkCoverRxPolicy(pAd, pPacket, &need_drop);
			
			if (need_drop == TRUE) {
				RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
				return;
			}
		}
#endif /* CONFIG_WIFI_PKT_FWD */
			RTMP_MATEngineRxHandle(pAd, pRxPkt, 0);
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
		RtmpOsPktProtocolAssign(pRxPkt);
		pToUpperLayerPktSent(pRxPkt);
		return;
	}
#endif /* defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570) */
#endif /* CONFIG_AP_SUPPORT */

#ifdef IKANOS_VX_1X0
	{
		IKANOS_DataFrameRx(pAd, pRxPkt);
		return;
	}
#endif /* IKANOS_VX_1X0 */

#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE))
		{
			INT retVal, ret = 0;
			UINT ppa_flags = 0;

			retVal = process_nbns_packet(pAd, pRxPkt);

			if (retVal > 0)
			{
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);
				if (ret == 0)
				{
					pRxPkt = NULL;
					return;
				}
				RtmpOsPktRcvHandle(pRxPkt);
			}
			else if (retVal == 0)
			{
				RtmpOsPktProtocolAssign(pRxPkt);
				RtmpOsPktRcvHandle(pRxPkt);
			}
			else
			{
				dev_kfree_skb_any(pRxPkt);
				MEM_DBG_PKT_FREE_INC(pAd);
			}
		}
		else
		{
			RtmpOsPktProtocolAssign(pRxPkt);
			RtmpOsPktRcvHandle(pRxPkt);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */

#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
#if (!defined(CONFIG_RA_NAT_NONE)) || ( defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI) )
	/* hwnat optimize */
	if ( (ra_sw_nat_hook_rx != NULL) && pAd->LanNatSpeedUpEn )
	{

		UINT16		i,protoType, protoType_ori, sKipLen=14;
		PUCHAR		pPktHdr = NULL, pLayerHdr = NULL;
		UINT32		pSrcIP,pDesIP;
		
		struct wifi_dev *wdev = pAd->wdev_list[pAd->HwnatCurWdevIdx];
		/* wdev should not be apcli or Null*/
		if( (wdev!=NULL)
#ifdef APCLI_SUPPORT
			&& (wdev->wdev_type != WDEV_TYPE_APCLI)
#endif /* APCLI_SUPPORT */
			)
		{
			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RecvIF Name=(%s)\n",wdev->if_dev->name ));
			pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);
			
			if ( pPktHdr )
			{
				// Get the upper layer protocol type of this 802.3 pkt. 
				protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12));
				protoType = OS_NTOHS(protoType_ori);
				// handle 802.1q enabled packet. Skip the VLAN tag field to get the protocol type. 
				if (protoType == 0x8100)
				{
					protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12 + 4));
					protoType = OS_NTOHS(protoType_ori);
					sKipLen = 14 + 4;
				}
				// handle 802.2 enabled packet. Skip LLC field to get the protocol type. 
				else if( protoType < 1536 ) 
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("protoType < 1536 pPktHdr: "));
					for( i = 0 ; i < 30 ; i++ )
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02X ",*(pPktHdr+i)));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
					
					if (pPktHdr[14] == 0xAA && pPktHdr[15] == 0xAA && pPktHdr[16] == 0x03)
					{
						protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12 + 8));
						protoType = OS_NTOHS(protoType_ori);
						sKipLen = 14 + 8;
					} 
				}
				//get the real protocol type
				pLayerHdr = pPktHdr + sKipLen;
				// if protoType == protoType and pSrcIP&mask == pDesIP&mask, not do hwnat; else do nothing
				switch( protoType )
				{
					case ETH_P_IP:
						pSrcIP = *((UINT32 *)(pLayerHdr + 12));
						pDesIP = *((UINT32 *)(pLayerHdr + 16));
						/* get br-lan netmask */
						if( pAd->isInitBrLan == 0 )
							GetBrLanNetMask( pAd );
							
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pSrcIP=(%d.%d.%d.%d) %08X&%08X \n", pSrcIP&0xff, (pSrcIP>>8)&0xff, (pSrcIP>>16)&0xff, (pSrcIP>>24)&0xff , pSrcIP,pAd->BrLanMask) );
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pDesIP=(%d.%d.%d.%d) %08X&%08X \n", pDesIP&0xff, (pDesIP>>8)&0xff, (pDesIP>>16)&0xff, (pDesIP>>24)&0xff , pDesIP,pAd->BrLanMask) );
						if( ( pSrcIP & pAd->BrLanMask ) == ( pDesIP & pAd->BrLanMask ) )
							isToLan = TRUE;
						
						break;
					case ETH_P_ARP:
					case ETH_P_PPP_DISC:
					case ETH_P_PPP_SES:
					case ETH_P_IPV6:
					default:
						isToLan = FALSE;
						break;
				}
				
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("isToLan = %d \n", isToLan ));
			}
		}
	}
#endif
#endif

	{
#ifdef CONFIG_RT2880_BRIDGING_ONLY
		PACKET_CB_ASSIGN(pRxPkt, 22) = 0xa8;
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
		if(ra_classifier_hook_rx!= NULL)
		{
			unsigned int flags;

			RTMP_IRQ_LOCK(&pAd->page_lock, flags);
			ra_classifier_hook_rx(pRxPkt, classifier_cur_cycle);
			RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
		}
#endif /* CONFIG_RA_CLASSIFIER */

#if !defined(CONFIG_RA_NAT_NONE)
	/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
	if( !isToLan )
#endif
	{

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
		RtmpOsPktNatMagicTag(pRxPkt);
#endif

		/* bruce+
			ra_sw_nat_hook_rx return 1 --> continue
			ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
		*/
		if (ra_sw_nat_hook_rx!= NULL)
		{
			unsigned int flags;

			RtmpOsPktProtocolAssign(pRxPkt);

			RTMP_IRQ_LOCK(&pAd->page_lock, flags);
			if(ra_sw_nat_hook_rx(pRxPkt))
			{
				FOE_MAGIC_TAG(RTPKT_TO_OSPKT(pRxPkt)) = 0;
				RtmpOsPktRcvHandle(pRxPkt);
			}
			RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
			return;
		}
	}
#else
		{
#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
			RtmpOsPktNatNone(pRxPkt);
#endif /* CONFIG_RA_HW_NAT */
		}
#endif /* CONFIG_RA_NAT_NONE */
	}


#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
		if (BG_FTPH_PacketFromApHandle(pRxPkt) == 0)
			return;
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef TRACELOG_TCP_PKT
        if (RTMPIsTcpAckPkt(pRxPkt))
            pAd->u4TcpRxAckCnt++;
#endif

#ifdef REDUCE_TCP_ACK_SUPPORT
        ReduceAckUpdateDataCnx(pAd,pRxPkt);
#endif

#ifdef RTMP_UDMA_SUPPORT
	if(mt_udma_pkt_send(pAd, pRxPkt) == 0)
		return;
#endif/*RTMP_UDMA_SUPPORT*/

        RtmpOsPktProtocolAssign(pRxPkt);

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
	/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
	if( !isToLan )
#endif
	{
		if (ra_sw_nat_hook_set_magic)
			ra_sw_nat_hook_set_magic(pRxPkt, FOE_MAGIC_WLAN);

		if (ra_sw_nat_hook_rx != NULL)
		{
			if (ra_sw_nat_hook_rx(pRxPkt) == 0)
				return;
		}
	}
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) 
		{
			struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pRxPkt);
 			if (RTMP_IS_PACKET_AP_APCLI(pOsRxPkt))
 			{	
				if (wf_fwd_rx_hook != NULL)
				{
					struct ethhdr *mh = eth_hdr(pRxPkt);
					int ret = 0;
					
					if ((mh->h_dest[0] & 0x1) == 0x1)
					{ 
						if (RTMP_IS_PACKET_APCLI(pOsRxPkt))
						{
#ifdef MAC_REPEATER_SUPPORT
							if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
								(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source, TRUE) == TRUE)) {
								//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
								//    ("announce_802_3_packet: drop rx pkt by RTMPQueryLookupRepeaterCliEntryMT check\n"));
								RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
								return;
							}
							else
#endif /* MAC_REPEATER_SUPPORT */
							{
								VOID *opp_band_tbl = NULL;
								VOID *band_tbl = NULL;
								VOID *other_band_tbl = NULL;


								if (wf_fwd_feedback_map_table)
									wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

								if (band_tbl != NULL)
								{
									if (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr), mh->h_source) ||
										((((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr_DBDC !=NULL) &&
										MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr_DBDC), mh->h_source)))
										{
										//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
										//    ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table band_tbl check of source addr against Wdev_ifAddr\n"));
											RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
											return;
										}
								}
								if (opp_band_tbl != NULL)
								{
									if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)) ||
										((((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr_DBDC !=NULL) &&
										(MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr_DBDC), mh->h_source))))
										{
										//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
										//    ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table opp_band_tbl check of source addr against Wdev_ifAddr\n"));
											RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
											return;
										}
								}							

								if (other_band_tbl != NULL)
								{
									if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr), mh->h_source)) ||
										((((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr_DBDC !=NULL) &&
										(MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr_DBDC), mh->h_source))))
										{
										//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
										// ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table other_band_tbl check of source addr against Wdev_ifAddr\n"));
											RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
											return;
										}
								}
							}
						}
					}
#ifdef WH_EZ_SETUP
					if (IS_EZ_SETUP_ENABLED(pAd->wdev_list[pAd->CurWdevIdx]))
					{
						struct wifi_dev *wdev = pAd->wdev_list[pAd->CurWdevIdx];
#ifdef EZ_MOD_SUPPORT
						//interface_info_t other_band_config;			
						if (ez_need_bypass_rx_fwd(wdev)
							/*wdev->wdev_type == WDEV_TYPE_APCLI && ez_get_other_band_info(pAd,wdev, &other_band_config)
							&& !wdev->ez_security.this_band_info.shared_info.link_duplicate 
							&& !MAC_ADDR_EQUAL(other_band_config.cli_peer_ap_mac ,ZERO_MAC_ADDR)*/)
#else
						interface_info_t other_band_config; 		
						if (wdev->wdev_type == WDEV_TYPE_APCLI && ez_get_other_band_info(pAd,wdev, &other_band_config)
							&& !wdev->ez_security.this_band_info.shared_info.link_duplicate 
							&& !MAC_ADDR_EQUAL(other_band_config.cli_peer_ap_mac ,ZERO_MAC_ADDR))
#endif
						{
							bypass_rx_fwd = TRUE;
						} else {
							bypass_rx_fwd = FALSE;
						}
					} else
					{
						bypass_rx_fwd = FALSE;
					}
					if (!bypass_rx_fwd) {
#endif		
					
					ret = wf_fwd_rx_hook(pRxPkt);

					if (ret == 0) {
						//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
						//    ("announce_802_3_packet: wf_fwd_rx_hook returned 0\n"));
						return;
					} else if (ret == 2) {
						//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
						//    ("announce_802_3_packet: wf_fwd_rx_hook returned 2\n"));
						RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
						return;
					}
#ifdef WH_EZ_SETUP		
				}
#endif			
				}
					
 			}
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("No CB Packet RTMP_IS_PACKET_AP_APCLI(%d)\n", RTMP_IS_PACKET_AP_APCLI(pOsRxPkt)));
				
	}
#endif /* CONFIG_WIFI_PKT_FWD */

		RtmpOsPktRcvHandle(pRxPkt);
}




VOID RTMPFreeGlobalUtility(VOID)
{
	/*do nothing for now*/
}

VOID RTMPFreeAdapter(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	int index;
	os_cookie=(POS_COOKIE)pAd->OS_Cookie;


#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheDeinit(&pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */

#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
	/* only if in MULTIPLE_CARD the eebuf be allocated not static */
	if (pAd->eebuf  /*&& (pAd->eebuf != pAd->chipCap.EEPROM_DEFAULT_BIN)*/)
	{
		os_free_mem(pAd->eebuf);
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


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
    if (pAd->ApCfg.bMACRepeaterEn == TRUE)
    {
        AsicSetReptFuncEnable(pAd, FALSE);
    }

    NdisFreeSpinLock(&pAd->ApCfg.CliLinkMapLock);
    NdisFreeSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	NdisFreeSpinLock(&pAd->ApCfg.InsertReptCmdLock);
#endif
#endif
#endif

#ifdef CONFIG_FWOWN_SUPPORT
	NdisFreeSpinLock(&pAd->DriverOwnLock);
#endif /* CONFIG_FWOWN_SUPPORT */

#if defined(RT3290) || defined(RLT_MAC)
	NdisFreeSpinLock(&pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

    NdisFreeSpinLock(&pAd->BssInfoIdxBitMapLock);
	NdisFreeSpinLock(&pAd->WdevListLock);

	for (index =0 ; index < NUM_OF_TX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}

	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef MT_MAC
    NdisFreeSpinLock(&pAd->BcnRingLock);
#endif /* MT_MAC */

#ifdef RTMP_MAC_PCI
	NdisFreeSpinLock(&pAd->LockInterrupt);
#ifdef CONFIG_ANDES_SUPPORT
	NdisFreeSpinLock(&pAd->CtrlRingLock);
#ifdef MT7615
    NdisFreeSpinLock(&pAd->FwDwloRing.RingLock);
#endif /* MT7615 */
#endif

	NdisFreeSpinLock(&pAd->tssi_lock);
#endif /* RTMP_MAC_PCI */


#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); /* OS_ABL_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef DOT11_N_SUPPORT
	NdisFreeSpinLock(&pAd->mpdu_blk_pool.lock);
#endif /* DOT11_N_SUPPORT */

#ifdef GREENAP_SUPPORT
        NdisFreeSpinLock(&pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */

	if (pAd->iw_stats)
	{
		os_free_mem(pAd->iw_stats);
		pAd->iw_stats = NULL;
	}
	if (pAd->stats)
	{
		os_free_mem(pAd->stats);
		pAd->stats = NULL;
	}

#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
    if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->OpMode == OPMODE_AP))
	{
        BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
        ASSERT(pMbss);
		if (pMbss) {
			bcn_buf_deinit(pAd, &pMbss->wdev.bcn_buf);
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
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

	RTMPFreeHifAdapterBlock(pAd);

	RtmpOsVfree(pAd); /* pci_free_consistent(os_cookie->pci_dev,sizeof(RTMP_ADAPTER),pAd,os_cookie->pAd_pa); */
	if (os_cookie)
		os_free_mem(os_cookie);

	RTMPFreeGlobalUtility();
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

	if (!wdev->sys_handle)
	{
		ASSERT(wdev->sys_handle);
		return 0;
	}
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INC_COUNTER64(pAd->WlanCounters[0].TransmitCountFrmOs);

	if (!pPacket)
		return 0;

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
	}
#endif /* WSC_NFC_SUPPORT */

#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);
	if (*(int*)(GET_OS_PKT_CB(pPacket)) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
	}
#endif /* CONFIG_5VT_ENHANCE */


#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED(wdev)
	//hex_dump("RTMPSendPackets: Eth Hdr: ",GET_OS_PKT_DATAPTR(pPacket),14)
	/*if( MAC_ADDR_IS_GROUP( ( (PUCHAR)GET_OS_PKT_DATAPTR(pPacket) ) ) ){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("RTMPSendPackets: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x\nEth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
		wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
		((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[0],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[1],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[2],
		((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[3],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[4],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[5],
		((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[6],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[7],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[8],
		((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[9],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[10],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[11],
		((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[12],((PUCHAR)GET_OS_PKT_DATAPTR(pPacket))[13]));
	}*/
	
#ifdef EZ_API_SUPPORT	
#ifdef EZ_MOD_SUPPORT
	 && (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD) 
#else
	 && (wdev->ez_security.ez_api_mode != CONNECTION_OFFLOAD) 
#endif
#endif	 
	 && (wdev->wdev_type == WDEV_TYPE_APCLI) )
	{
#ifdef EZ_MOD_SUPPORT	
		if (ez_handle_send_packets(wdev, pPacket) == 0)
			return 0;
#else
		UCHAR *pDestAddr = GET_OS_PKT_DATAPTR(pPacket);

#ifdef EZ_DUAL_BAND_SUPPORT
		if((wdev->ez_security.ez_loop_chk_timer_running) && (wdev->ez_security.first_loop_check) &&  (MAC_ADDR_IS_GROUP(pDestAddr))){ // only source runs timer, so role chk not required
		    EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			    ("APCLi=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : Drop Tx Pkt as Loop Check triggered by this source\n",
		    	wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}
#endif

		//hex_dump("RTMPSendPackets: Eth Hdr: ",pDestAddr,14);

		if(pDestAddr && MAC_ADDR_IS_GROUP(pDestAddr)){ // group packet
			//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			//    ("RTMPSendPackets: APCLi=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x: Group packet on ApCli interface\n",
			//	  wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));

			if( ez_apcli_tx_grp_pkt_drop(wdev, (struct sk_buff *)pPacket) == TRUE)
			{
				//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("APCLi=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : ApCli will drop this other band rcvd pkt\n",
					//wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return 0;
			}			
		}
		else if(pDestAddr && (!MAC_ADDR_IS_GROUP(pDestAddr))){
			ez_apcli_uni_tx_on_dup_link(wdev,(struct sk_buff *)pPacket);
		}
#endif
	}
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) {
		if (wf_fwd_tx_hook != NULL)
		{
			if (wf_fwd_tx_hook(pPacket) == 1)
			{
				//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				//    ("RTMPSendPackets: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : wf_fwd_tx_hook indicated Packet DROP\n",
				//	   wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return 0;
			}
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

	if((pConfig->priv_flags == INT_MAIN) && !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
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

    pObj->pSecConfig = NULL;

    /* determine this ioctl command is comming from which interface. */
    if (pConfig->priv_flags == INT_MAIN)
    
    {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		pObj->pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;		
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
			pObj->pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;
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
				pObj->pSecConfig = &pAd->WdsTab.WdsEntry[index].wdev.SecConfig;
				break;
			}

			if(index == MAX_WDS_ENTRY)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): can not find wds I/F\n", __FUNCTION__));
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
				pObj->pSecConfig = &pAd->ApCfg.ApCliTab[index].wdev.SecConfig;
				break;
			}

			if(index == MAX_APCLI_NUM)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): can not find Apcli I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif /* APCLI_SUPPORT */
    else
    {
	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("IOCTL is not supported in WDS interface\n")); */
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
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

