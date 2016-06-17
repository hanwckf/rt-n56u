/*
 *************************************************************************** 
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2012, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cfg80211_inf.c

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
	YF Luo		06-28-2012		Init version
		        12-26-2013		Integration of NXTC	
*/
#define RTMP_MODULE_OS

#include "rt_config.h" 
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if defined (HE_BD_CFG80211_SUPPORT) && defined (BD_KERNEL_VER)
#undef  LINUX_VERSION_CODE
#define LINUX_VERSION_CODE KERNEL_VERSION(2,6,39)
#endif /* HE_BD_CFG80211_SUPPORT && BD_KERNEL_VER */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
#ifdef RT_CFG80211_SUPPORT

extern INT ApCliAllowToSendPacket(
	RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
	PNDIS_PACKET pPacket, UCHAR *pWcid);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, VOID *pData)
{
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT newType, oldType;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM *pVifParm;		
	pVifParm = (CMD_RTPRIV_IOCTL_80211_VIF_PARM *)pData;	

	newType = pVifParm->newIfType;
	oldType = pVifParm->oldIfType;

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE

	APCLI_STRUCT	*pApCliEntry;
	struct wifi_dev *wdev;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT apidx = 1;
	CHAR tr_tb_idx = MAX_LEN_OF_MAC_TABLE + apidx;


	printk(" CFG80211DRV_OpsChgVirtualInf  newType %d  oldType %d \n",newType,oldType);
   if (strcmp(pVifParm->net_dev->name, "p2p0") == 0) 
   {
 
	switch (newType)
	{
		case RT_CMD_80211_IFTYPE_MONITOR:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211 I/F Monitor Type\n"));
			//RTMP_OS_NETDEV_SET_TYPE_MONITOR(new_dev_p);	
			break;

			case RT_CMD_80211_IFTYPE_STATION:

			RTMP_CFG80211_RemoveVifEntry(pAd,cfg80211_ctrl->dummy_p2p_net_dev);
			RTMP_CFG80211_AddVifEntry(pAd, cfg80211_ctrl->dummy_p2p_net_dev, newType);

			AsicSetBssid(pAd, pAd->cfg80211_ctrl.P2PCurrentAddress, 0x1); 
			AsicSetBssid(pAd, pAd->CurrentAddress, 0x0); 

			break;


		case RT_CMD_80211_IFTYPE_P2P_CLIENT:

			pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev.if_dev = NULL;
			pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
			pApCliEntry->wdev.if_dev= cfg80211_ctrl->dummy_p2p_net_dev;
			wdev = &pApCliEntry->wdev;

			wdev->wdev_type = WDEV_TYPE_STA;
			wdev->func_dev = pApCliEntry;
			wdev->sys_handle = (void *)pAd;


			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_Open\n"));
	
			RTMP_CFG80211_RemoveVifEntry(pAd,cfg80211_ctrl->dummy_p2p_net_dev);
			/* TX */
			RTMP_CFG80211_AddVifEntry(pAd, cfg80211_ctrl->dummy_p2p_net_dev, newType);
			wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->tx_pkt_handle = APSendPacket;

			/* RX */
			wdev->rx_pkt_allowed = sta_rx_pkt_allow;
			wdev->rx_pkt_foward = sta_rx_fwd_hnd;
			
			RTMP_OS_NETDEV_SET_PRIV(cfg80211_ctrl->dummy_p2p_net_dev, pAd);
			RTMP_OS_NETDEV_SET_WDEV(cfg80211_ctrl->dummy_p2p_net_dev, wdev);
			COPY_MAC_ADDR(wdev->if_addr,pAd->cfg80211_ctrl.P2PCurrentAddress);
			if (rtmp_wdev_idx_reg(pAd, wdev) < 0) 
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Assign wdev idx for %s failed, free net device!\n",
								__FUNCTION__,RTMP_OS_NETDEV_GET_DEVNAME(cfg80211_ctrl->dummy_p2p_net_dev)));
				RtmpOSNetDevFree(cfg80211_ctrl->dummy_p2p_net_dev);
				break;
			}
			

			pAd->flg_apcli_init = TRUE;
			ApCli_Open(pAd, cfg80211_ctrl->dummy_p2p_net_dev);

//			COPY_MAC_ADDR(wdev->if_addr,pAd->cfg80211_ctrl.P2PCurrentAddress);
			AsicSetBssid(pAd, pAd->CurrentAddress, 0x0); 


			break;

		case RT_CMD_80211_IFTYPE_P2P_GO:

	//		pNetDevOps->priv_flags = INT_P2P;
			/* The Behivaor in SetBeacon Ops */
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			wdev->wdev_type = WDEV_TYPE_AP;
			wdev->func_dev = &pAd->ApCfg.MBSSID[apidx];
			wdev->func_idx = apidx;	
			wdev->sys_handle = (void *)pAd;
			wdev->if_dev = cfg80211_ctrl->dummy_p2p_net_dev;
			
			/* BC/MC Handling */
			wdev->tr_tb_idx = tr_tb_idx;
			tr_tb_set_mcast_entry(pAd, tr_tb_idx, wdev);

			RTMP_CFG80211_RemoveVifEntry(pAd,cfg80211_ctrl->dummy_p2p_net_dev);
			/* TX */
			RTMP_CFG80211_AddVifEntry(pAd, cfg80211_ctrl->dummy_p2p_net_dev, newType);
//			RT_MOD_INC_USE_COUNT();

			/* TX */
			wdev->tx_pkt_allowed = ApAllowToSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->tx_pkt_handle = APSendPacket;

			/* RX */
			wdev->rx_pkt_allowed = ap_rx_pkt_allow;
            		wdev->rx_pkt_foward = ap_rx_foward_handle;	
			wdev->rx_ps_handle = ap_rx_ps_handle;
//			RTMP_OS_NETDEV_START_QUEUE(cfg80211_ctrl->dummy_p2p_net_dev);
			RTMP_OS_NETDEV_SET_PRIV(cfg80211_ctrl->dummy_p2p_net_dev, pAd);
			RTMP_OS_NETDEV_SET_WDEV(cfg80211_ctrl->dummy_p2p_net_dev, wdev);
			
            		wdev_bcn_buf_init(pAd, &pAd->ApCfg.MBSSID[apidx].bcn_buf);
	
			RTMP_OS_NETDEV_SET_PRIV(cfg80211_ctrl->dummy_p2p_net_dev, pAd);
      		       RTMP_OS_NETDEV_SET_WDEV(cfg80211_ctrl->dummy_p2p_net_dev, wdev);

			if (rtmp_wdev_idx_reg(pAd, wdev) < 0)
            		{
	       	     MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Assign wdev idx for %s failed, free net device!\n",
	                                            __FUNCTION__,RTMP_OS_NETDEV_GET_DEVNAME(cfg80211_ctrl->dummy_p2p_net_dev)));
	        	    RtmpOSNetDevFree(cfg80211_ctrl->dummy_p2p_net_dev);
	        	    break;
          		}

			COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pAd->cfg80211_ctrl.P2PCurrentAddress);
			COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pAd->cfg80211_ctrl.P2PCurrentAddress);
			AsicSetBssid(pAd, pAd->CurrentAddress, 0x0); 

			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown CFG80211 I/F Type (%d)\n", newType));
	}
   }
	if ((newType == RT_CMD_80211_IFTYPE_STATION) &&
	   (oldType == RT_CMD_80211_IFTYPE_P2P_CLIENT) && (pAd->flg_apcli_init == TRUE))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_Close\n"));
		CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
//		RT_MOD_DEC_USE_COUNT();
		pAd->flg_apcli_init = FALSE;
		RT_MOD_INC_USE_COUNT();

		printk("iverson ApCli_Close \n");
		AsicSetBssid(pAd, pAd->cfg80211_ctrl.P2PCurrentAddress, 0x1); 
		AsicSetBssid(pAd, pAd->CurrentAddress, 0x0); 
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_STATION;

		return ApCli_Close(pAd, cfg80211_ctrl->dummy_p2p_net_dev);

	}
	else if ((newType == RT_CMD_80211_IFTYPE_STATION) &&
	   (oldType == RT_CMD_80211_IFTYPE_P2P_GO))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("GOi_Close\n"));

		wdev_bcn_buf_deinit(pAd, &pAd->ApCfg.MBSSID[apidx].bcn_buf);
		AsicSetBssid(pAd, pAd->cfg80211_ctrl.P2PCurrentAddress, 0x1); 
		AsicSetBssid(pAd, pAd->CurrentAddress, 0x0); 

//		rtmp_wdev_idx_unreg(pAd, Wdev);
//		Wdev->if_dev = NULL;

	}


#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */



#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#ifndef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	/* After P2P NEGO phase, the device type may be change from GC to GO 
	   or no change. We remove the GC in VIF list if nego as GO case.
	 */
	if ((newType == RT_CMD_80211_IFTYPE_P2P_GO) &&
	   (oldType == RT_CMD_80211_IFTYPE_P2P_CLIENT))
	{
		RTMP_CFG80211_VirtualIF_CancelP2pClient(pAd);
	}
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */	

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE

	CFG80211DBG(DBG_LVL_TRACE, ("80211> @@@ Change from %u  to %u Mode\n",oldType,newType));

	pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_DISABLE;
	if (newType == RT_CMD_80211_IFTYPE_P2P_CLIENT)
	{
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_CLI_UP;
	
	}
	else if (newType == RT_CMD_80211_IFTYPE_P2P_GO)
	{
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_GO_UP;
	}
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */

	/* Change Device Type */
#ifdef CONFIG_STA_SUPPORT	
	if (newType == RT_CMD_80211_IFTYPE_ADHOC)
	{
#ifdef DOT11_N_SUPPORT
		SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
		pAd->StaCfg.BssType = BSS_ADHOC;
	}	
	else
#endif /* CONFIG_STA_SUPPORT */	 
	if ((newType == RT_CMD_80211_IFTYPE_STATION) ||
		(newType == RT_CMD_80211_IFTYPE_P2P_CLIENT))
	{
		CFG80211DBG(DBG_LVL_TRACE, ("80211> Change the Interface to STA Mode\n"));

#ifdef CONFIG_STA_SUPPORT		
		if ((oldType == RT_CMD_80211_IFTYPE_ADHOC) && 
             (newType == RT_CMD_80211_IFTYPE_STATION))
		{
			/* DeviceType Change from adhoc to infra, 
			   only in StaCfg. 
               CFG Todo: It should not bind by device. 
			 */
			pAd->StaCfg.BssType = BSS_INFRA;
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
			CFG80211DRV_DisableApInterface(pAd);
#endif /* CONFIG_AP_SUPPORT */
			
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_STATION;
	}
	else if ((newType == RT_CMD_80211_IFTYPE_AP) ||
		     (newType == RT_CMD_80211_IFTYPE_P2P_GO))
	{
		CFG80211DBG(DBG_LVL_TRACE, ("80211> Change the Interface to AP Mode\n"));		
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
	}	
#ifdef CONFIG_STA_SUPPORT	
	else if (newType == RT_CMD_80211_IFTYPE_MONITOR)
	{
		/* set packet filter */
		Set_NetworkType_Proc(pAd, "Monitor");

		if (pVifParm->MonFilterFlag != 0)
		{
			UINT32 Filter = 0;

#ifndef MT_MAC
			if (pAd->chipCap.hif_type != HIF_MT) {
				RTMP_IO_READ32(pAd, RX_FILTR_CFG, &Filter);

				if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_FCSFAIL) == RT_CMD_80211_FILTER_FCSFAIL)
				{
					Filter = Filter & (~0x01);
				}
				else
				{
					Filter = Filter | 0x01;
				}
	
				if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_PLCPFAIL) == RT_CMD_80211_FILTER_PLCPFAIL)
				{
					Filter = Filter & (~0x02);
				}
				else
				{
					Filter = Filter | 0x02;
				}	
	
				if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_CONTROL) == RT_CMD_80211_FILTER_CONTROL)
				{
					Filter = Filter & (~0xFF00);
				}
				else
				{
					Filter = Filter | 0xFF00;
				}	
	
				if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_OTHER_BSS) == RT_CMD_80211_FILTER_OTHER_BSS)
				{
					Filter = Filter & (~0x08);
				}
				else
				{
					Filter = Filter | 0x08;
				}

				RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, Filter);
			}
#endif /* MT_MAC */
			pVifParm->MonFilterFlag = Filter;
		} 
	} 
#endif /*CONFIG_STA_SUPPORT*/
	if ((newType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
	   (newType == RT_CMD_80211_IFTYPE_P2P_GO))
	{
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pVifParm->net_dev->dev_addr);
	}
	else
	{
#ifdef RT_CFG80211_P2P_SUPPORT
        pCfg80211_ctrl->bP2pCliPmEnable = FALSE;
        pCfg80211_ctrl->bPreKeepSlient = FALSE;
        pCfg80211_ctrl->bKeepSlient = FALSE;
        pCfg80211_ctrl->NoAIndex = MAX_LEN_OF_MAC_TABLE;
        pCfg80211_ctrl->MyGOwcid = MAX_LEN_OF_MAC_TABLE;
        pCfg80211_ctrl->CTWindows= 0;   /* CTWindows and OppPS parameter field */
#endif /* RT_CFG80211_P2P_SUPPORT */
	}

	if(pCfg80211_ctrl == NULL)
		CFG80211DBG(DBG_LVL_TRACE, ("(%s)pCfg80211_ctrl is null",__FUNCTION__));

	return TRUE;
}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_VIF_SET *pVifInfo;	
	pVifInfo = (CMD_RTPRIV_IOCTL_80211_VIF_SET *)pData;

	/* Already one VIF in list */
	if (pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn)
		return FALSE;
	
	pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = TRUE;
	RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType);
	return TRUE;
}

BOOLEAN RTMP_CFG80211_VIF_ON(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	return pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn;
}


static 
PCFG80211_VIF_DEV RTMP_CFG80211_FindVifEntry_ByMac(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       	pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL)
	{
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			return pDevEntry;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
	
	return NULL;	
}

PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       	pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL)
	{
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev;
		
		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
	
	return NULL;	
}

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       	pDevEntry = NULL;
	RT_LIST_ENTRY		        *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL)
	{
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev->ieee80211_ptr;
		
		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
	
	return NULL;	
}

VOID RTMP_CFG80211_AddVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_VIF_DEV pNewVifDev = NULL;
	
	os_alloc_mem(NULL, (UCHAR **)&pNewVifDev, sizeof(CFG80211_VIF_DEV));
	if (pNewVifDev)
	{
		NdisZeroMemory(pNewVifDev, sizeof(CFG80211_VIF_DEV));

		pNewVifDev->pNext = NULL;
		pNewVifDev->net_dev = pNewNetDev;
		pNewVifDev->devType = DevType;
		NdisZeroMemory(pNewVifDev->CUR_MAC, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->CUR_MAC, pNewNetDev->dev_addr, MAC_ADDR_LEN);

		insertTailList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, (RT_LIST_ENTRY *)pNewVifDev);	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Add CFG80211 VIF Device, Type: %d.\n", pNewVifDev->devType));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error in alloc mem in New CFG80211 VIF Function.\n"));
	}
}

VOID RTMP_CFG80211_RemoveVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)RTMP_CFG80211_FindVifEntry_ByMac(pAd, pNewNetDev);	
	
	if (pListEntry)
	{
		delEntryList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, pListEntry);
		os_free_mem(NULL, pListEntry);	
		pListEntry = NULL;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error in RTMP_CFG80211_RemoveVifEntry.\n"));
	}
}

PNET_DEV RTMP_CFG80211_VirtualIF_Get(
	IN VOID                 *pAdSrc
)
{
	//PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	//return pAd->Cfg80211VifDevSet.Cfg80211VifDev[0].net_dev;
	return NULL;
}

static INT CFG80211_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAdSrc;
	
	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ===> %d,%s\n", __FUNCTION__, dev_p->ifindex, 
						RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	//if (VIRTUAL_IF_UP(pAd) != 0)
	//	return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();

	if ((dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT) 
#ifdef CFG80211_MULTI_STA	
		|| (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION)
#endif /* CFG80211_MULTI_STA */		
	   )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211_VirtualIF_Open\n"));
		pAd->flg_apcli_init = TRUE;
		ApCli_Open(pAd, dev_p);
		return 0;
	}

	RTMP_OS_NETDEV_START_QUEUE(dev_p);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <=== %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	return 0;
}

static INT CFG80211_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAdSrc;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

	if ((dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
#ifdef CFG80211_MULTI_STA	 
	    || (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION)
#endif /* CFG80211_MULTI_STA */
	   )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211_VirtualIF_Close\n"));
		CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning)
			pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;

		RT_MOD_DEC_USE_COUNT();
		return ApCli_Close(pAd, dev_p);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
	
	if (netif_carrier_ok(dev_p))
		netif_carrier_off(dev_p);

	if (INFRA_ON(pAd))
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
	else if (ADHOC_ON(pAd))
		AsicEnableIbssSync(pAd);
	else
		AsicDisableSync(pAd);

	//VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();
	return 0;
}

static INT CFG80211_PacketSend(PNDIS_PACKET pPktSrc, PNET_DEV pDev, RTMP_NET_PACKET_TRANSMIT Func)
{
	PRTMP_ADAPTER pAd;
	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);

	/* To Indicate from Which VIF */
	switch (pDev->ieee80211_ptr->iftype)
	{
		case RT_CMD_80211_IFTYPE_AP:
			RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
			break;

		case RT_CMD_80211_IFTYPE_P2P_GO:;
			if(!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) 
			{
		        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Drop the Packet due P2P GO not in ready state\n"));
		        RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
				return 0;
			}
			RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
			break;	

		case RT_CMD_80211_IFTYPE_P2P_CLIENT:
		case RT_CMD_80211_IFTYPE_STATION:
			RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
			//printk("%s: tx ==> %d\n", __FUNCTION__, RTMP_GET_PACKET_OPMODE(pPktSrc));
			break;				

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unknown CFG80211 I/F Type(%d)\n", pDev->ieee80211_ptr->iftype));	
			RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
			return 0;
	}	

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG80211 Packet Type  [%s](%d)\n", 
					pDev->name, pDev->ieee80211_ptr->iftype));

	return Func(RTPKT_TO_OSPKT(pPktSrc));
}

static INT CFG80211_VirtualIF_PacketSend(
	struct sk_buff *skb, PNET_DEV dev_p)
{
	struct wifi_dev *wdev;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s ---> %d\n", __FUNCTION__, dev_p->ieee80211_ptr->iftype));

	if(!(RTMP_OS_NETDEV_STATE_RUNNING(dev_p)))
	{
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, skb, NDIS_STATUS_FAILURE);
		return 0;
	}

	/* The device not ready to send packt. */
	wdev = RTMP_OS_NETDEV_GET_WDEV(dev_p);
	ASSERT(wdev);
	if (!wdev) return -1;

	NdisZeroMemory((PUCHAR)&skb->cb[CB_OFF], 26);
	MEM_DBG_PKT_ALLOC_INC(skb);

	return CFG80211_PacketSend(skb, dev_p, rt28xx_packet_xmit);
}

static INT CFG80211_VirtualIF_Ioctl(
	IN PNET_DEV				dev_p, 
	IN OUT VOID 			*rq_p, 
	IN INT 					cmd)
{

	RTMP_ADAPTER *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s --->\n", __FUNCTION__));

	return rt28xx_ioctl(dev_p, rq_p, cmd);

} 

VOID RTMP_CFG80211_VirtualIF_Init(
	IN VOID 		*pAdSrc,
	IN CHAR			*pDevName,
	IN UINT32                DevType)
{

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	return;
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV	new_dev_p;
	APCLI_STRUCT	*pApCliEntry;
	struct wifi_dev *wdev;
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	UINT apidx = MAIN_MBSSID;
#endif /*RT_CFG80211_P2P_SUPPORT*/

	CHAR tr_tb_idx = MAX_LEN_OF_MAC_TABLE + apidx;
	CHAR preIfName[12];
	UINT devNameLen = strlen(pDevName);
	UINT preIfIndex = pDevName[devNameLen-1] - 48;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	struct wireless_dev *pWdev;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;

	memset(preIfName, 0, sizeof(preIfName));
	NdisCopyMemory(preIfName, pDevName, devNameLen-1);

	pNetDevOps=&netDevHook;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ---> (%s, %s, %d)\n", __FUNCTION__, pDevName, preIfName, preIfIndex));

	/* init operation functions and flags */
	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_VirtualIF_Open;	     /* device opem hook point */
	netDevHook.stop = CFG80211_VirtualIF_Close;	     /* device close hook point */
	netDevHook.xmit = CFG80211_VirtualIF_PacketSend; /* hard transmit hook point */
	netDevHook.ioctl = CFG80211_VirtualIF_Ioctl;	 /* ioctl hook point */

#if WIRELESS_EXT >= 12
	//netDevHook.iw_handler = (void *)&rt28xx_ap_iw_handler_def;
#endif /* WIRELESS_EXT >= 12 */
		
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), preIfName);
	
	if (new_dev_p == NULL)
	{
		/* allocation fail, exit */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Allocate network device fail (CFG80211)...\n"));
		return;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Register CFG80211 I/F (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
	}	
	
	new_dev_p->destructor =  free_netdev;
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	pNetDevOps->needProtcted = TRUE;

	NdisMoveMemory(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);

#ifdef MT_MAC
	INT32 Value;
    UCHAR MacByte = 0;

    //TODO: shall we make choosing which byte to be selectable???
    Value = 0x00000000L;
    RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);//read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.
    Value = Value | (0x2 << 29);//Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address
    RTMP_IO_WRITE32(pAd, LPON_BTEIR, Value);
    MacByte = Value >> 29;
	
	pNetDevOps->devAddr[0] |= 0x2;
	
    switch (MacByte) {
	    case 0x1: /* choose bit[23:20]*/
	            pNetDevOps->devAddr[2] = (pNetDevOps->devAddr[2] = pNetDevOps->devAddr[2] & 0x0f);
	            break;
	    case 0x2: /* choose bit[31:28]*/
	            pNetDevOps->devAddr[3] = (pNetDevOps->devAddr[3] = pNetDevOps->devAddr[3] & 0x0f);
	            break;
	    case 0x3: /* choose bit[39:36]*/
	            pNetDevOps->devAddr[4] = (pNetDevOps->devAddr[4] = pNetDevOps->devAddr[4] & 0x0f);
	            break;
	    case 0x4: /* choose bit [47:44]*/
	            pNetDevOps->devAddr[5] = (pNetDevOps->devAddr[5] = pNetDevOps->devAddr[5] & 0x0f);
	            break;
	    default: /* choose bit[15:12]*/
	            pNetDevOps->devAddr[1] = (pNetDevOps->devAddr[1] = pNetDevOps->devAddr[1] & 0x0f);
	            break;
    }

	AsicSetDevMac(pAd, pNetDevOps->devAddr, 0x1);//set own_mac to HWBSSID1
#else
	//CFG_TODO
	/* 	 
		Bit1 of MAC address Byte0 is local administration bit 
		and should be set to 1 in extended multiple BSSIDs'
		Bit3~ of MAC address Byte0 is extended multiple BSSID index.
	*/	
	if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
		pNetDevOps->devAddr[0] += 2; /* NEW BSSID */
	else
	{
#ifdef P2P_ODD_MAC_ADJUST
		if (pNetDevOps->devAddr[5] & 0x01 == 0x01)
			pNetDevOps->devAddr[5] -= 1;
		else
#endif /* P2P_ODD_MAC_ADJUST */
		pNetDevOps->devAddr[5] += FIRST_MBSSID;
	}		
#endif /* MT_MAC */	

	switch (DevType)
	{
#ifdef CONFIG_STA_SUPPORT	
		case RT_CMD_80211_IFTYPE_MONITOR:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211 I/F Monitor Type\n"));
			//RTMP_OS_NETDEV_SET_TYPE_MONITOR(new_dev_p);	
			break;

		case RT_CMD_80211_IFTYPE_P2P_CLIENT:
		case RT_CMD_80211_IFTYPE_STATION:
			pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
			wdev = &pApCliEntry->wdev;
			wdev->wdev_type = WDEV_TYPE_STA;
			wdev->func_dev = pApCliEntry;
			wdev->func_idx = MAIN_MBSSID;
			wdev->sys_handle = (void *)pAd;
			wdev->if_dev = new_dev_p;
			/* TX */
			wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->tx_pkt_handle = APSendPacket;

			/* RX */
			wdev->rx_pkt_allowed = sta_rx_pkt_allow;
			wdev->rx_pkt_foward = sta_rx_fwd_hnd;

			RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
			RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
			if (rtmp_wdev_idx_reg(pAd, wdev) < 0) 
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Assign wdev idx for %s failed, free net device!\n",
								__FUNCTION__,RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
				RtmpOSNetDevFree(new_dev_p);
				break;
			}
			
			/* init MAC address of virtual network interface */
			COPY_MAC_ADDR(wdev->if_addr, pNetDevOps->devAddr);
			break;
#endif /*CONFIG_STA_SUPPORT*/
		case RT_CMD_80211_IFTYPE_P2P_GO:
			/* Only ForceGO init from here, 
			   Nego as GO init on AddBeacon Ops.
			 */
			pNetDevOps->priv_flags = INT_P2P;
			/* The Behivaor in SetBeacon Ops */
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			wdev->wdev_type = WDEV_TYPE_AP;
			wdev->func_dev = &pAd->ApCfg.MBSSID[apidx];
			wdev->func_idx = apidx;	
			wdev->sys_handle = (void *)pAd;
			wdev->if_dev = new_dev_p;
			
			/* BC/MC Handling */
			wdev->tr_tb_idx = tr_tb_idx;
			tr_tb_set_mcast_entry(pAd, tr_tb_idx, wdev);

			/* TX */
			wdev->tx_pkt_allowed = ApAllowToSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->tx_pkt_handle = APSendPacket;

			/* RX */
			wdev->rx_pkt_allowed = ap_rx_pkt_allow;
            wdev->rx_pkt_foward = ap_rx_foward_handle;	
			wdev->rx_ps_handle = ap_rx_ps_handle;
			
			/* for concurrent purpose */
			wdev->hw_bssid_idx = CFG_GO_BSSID_IDX;
			
            wdev_bcn_buf_init(pAd, &pAd->ApCfg.MBSSID[apidx].bcn_buf);

			RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
            RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
			if (rtmp_wdev_idx_reg(pAd, wdev) < 0)
            {
	            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Assign wdev idx for %s failed, free net device!\n",
	                                            __FUNCTION__,RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
	            RtmpOSNetDevFree(new_dev_p);
	            break;
            }

			COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pNetDevOps->devAddr);
			COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pNetDevOps->devAddr);
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown CFG80211 I/F Type (%d)\n", DevType));
	}

	//CFG_TODO : should be move to VIF_CHG
	if ((DevType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
	   (DevType == RT_CMD_80211_IFTYPE_P2P_GO))
	{
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pNetDevOps->devAddr);
	}
	
	pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
	
	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));	
	pWdev->netdev = new_dev_p;
	pWdev->iftype = DevType;
		
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
	COPY_MAC_ADDR(pWdev->address, pNetDevOps->devAddr);	
#endif /* LINUX_VERSION_CODE: 3.7.0 */

	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);

	//AsicSetBssid(pAd, pAd->CurrentAddress, 0x0);
	//AsicSetBssid(pAd, wdev->if_addr, 0x1); 
		
	/* Record the pNetDevice to Cfg80211VifDevList */
	RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s <---\n", __FUNCTION__));
}

VOID RTMP_CFG80211_VirtualIF_Remove(
	IN  VOID 				 *pAdSrc,
	IN	PNET_DEV			  dev_p,
	IN  UINT32                DevType)
{

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	BOOLEAN isGoOn = FALSE;	
	struct wifi_dev *wdev;
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	UINT apidx = MAIN_MBSSID;
#endif /*RT_CFG80211_P2P_SUPPORT*/


	if (dev_p)
	{
		pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = FALSE;
		RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
		RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		isGoOn = RTMP_CFG80211_VIF_P2P_GO_ON(pAd);
			
		if (isGoOn)
		{
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
			struct wifi_dev *pwdev = &pMbss->wdev;
			if (pAd->Mlme.bStartScc == TRUE)
			{
				pAd->Mlme.bStartScc = FALSE;
				AsicSwitchChannel(pAd, pAd->StaCfg.wdev.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->StaCfg.wdev.CentralChannel);	
				bbp_set_bw(pAd, pAd->StaCfg.wdev.bw);
			}
	
			pwdev->channel = 0; 
			pwdev->CentralChannel= 0; 
			pwdev->bw = 0; 
			pwdev->extcha = EXTCHA_NONE;

/*after p2p cli connect , neet to change to default configure*/
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA  = EXTCHA_BELOW;
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
			pAd->CommonCfg.HT_Disable = 0;
			SetCommonHT(pAd);

			wdev_bcn_buf_deinit(pAd, &pAd->ApCfg.MBSSID[apidx].bcn_buf);
			RtmpOSNetDevDetach(dev_p);
			rtmp_wdev_idx_unreg(pAd, wdev);
			wdev->if_dev = NULL;
		}
		else 
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		if (pAd->flg_apcli_init)
		{
			wdev = &pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev;

			//actually not mcc still need to check this!
			if (pAd->Mlme.bStartScc == TRUE)
			{
				printk("GC remove & switch to Infra BW = %d  pAd->StaCfg.wdev.CentralChannel %d \n",pAd->StaCfg.wdev.bw,pAd->StaCfg.wdev.CentralChannel);
				pAd->Mlme.bStartScc = FALSE;
				AsicSwitchChannel(pAd, pAd->StaCfg.wdev.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->StaCfg.wdev.CentralChannel);					
				bbp_set_bw(pAd, pAd->StaCfg.wdev.bw);
			}
			
			wdev->CentralChannel = 0;
			wdev->channel= 0;
			wdev->bw = HT_BW_20;
			wdev->extcha = EXTCHA_NONE;
	

			OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
			cfg80211_disconnected(dev_p, 0, NULL, 0, GFP_KERNEL);
			
			NdisZeroMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, MAC_ADDR_LEN);
			RtmpOSNetDevDetach(dev_p);
			rtmp_wdev_idx_unreg(pAd, wdev);
			pAd->flg_apcli_init = FALSE;
			wdev->if_dev = NULL;
		}
		else /* Never Opened When New Netdevice on */
		{
			RtmpOSNetDevDetach(dev_p);
		}

		if (dev_p->ieee80211_ptr)
		{
			kfree(dev_p->ieee80211_ptr);
			dev_p->ieee80211_ptr = NULL;
		} 		
	}	 
} 

VOID RTMP_CFG80211_AllVirtualIF_Remove(
	IN VOID 		*pAdSrc)
{
    	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;    
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	
	if (pCacheList->size <= 0 )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s <-- Vif list is empty\n", __FUNCTION__));
		return;
	}

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while ((pDevEntry != NULL) && (pCacheList->size != 0))
	{   
		RtmpOSNetDevProtect(1);
		RTMP_CFG80211_VirtualIF_Remove(pAd, pDevEntry->net_dev, pDevEntry->net_dev->ieee80211_ptr->iftype);
		RtmpOSNetDevProtect(0);

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

#ifdef RT_CFG80211_P2P_SUPPORT
BOOLEAN RTMP_CFG80211_VIF_P2P_GO_ON(VOID *pAdSrc)
{	
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE	
	PNET_DEV pNetDev = NULL;

	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO)) != NULL))
    	return TRUE;    
	else	
		return FALSE;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE	
	if(CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.P2POpStatusFlags, CFG_P2P_GO_UP))
		return TRUE;
	else
		return FALSE;
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */				

	return FALSE;
}

BOOLEAN RTMP_CFG80211_VIF_P2P_CLI_ON(VOID *pAdSrc)
{
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNetDev = NULL;
		
    if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
        ((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_CLIENT)) != NULL))
    	return TRUE;
    else
    	return FALSE;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE	
	if(CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.P2POpStatusFlags, CFG_P2P_CLI_UP))
		return TRUE;
	else
		return FALSE;
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
	
	return FALSE;
}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID RTMP_CFG80211_VirtualIF_CancelP2pClient(VOID *pAdSrc)
{
        PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
    	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
    	PCFG80211_VIF_DEV               pDevEntry = NULL;
    	RT_LIST_ENTRY *pListEntry = NULL;

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> %s\n", __FUNCTION__));

    	pListEntry = pCacheList->pHead;
    	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
    	while (pDevEntry != NULL)
    	{
        	if (pDevEntry->devType == RT_CMD_80211_IFTYPE_P2P_CLIENT)
        	{
                	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==> RTMP_CFG80211_VirtualIF_CancelP2pClient HIT.\n"));
                        	pDevEntry->devType = RT_CMD_80211_IFTYPE_P2P_GO;
			rtmp_wdev_idx_unreg(pAd, &pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev);
                        break;
        	}

        	pListEntry = pListEntry->pNext;
        	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
    	}

        pAd->flg_apcli_init = FALSE;
        pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev.if_dev = NULL;

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== %s\n", __FUNCTION__));
}

static INT CFG80211_DummyP2pIf_Open(
	IN PNET_DEV		dev_p)
{
	struct wireless_dev *wdev = dev_p->ieee80211_ptr;
#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	VOID *pAdSrc;
	printk("CFG80211_DummyP2pIf_Open=======> Open\n");
	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */


	if (!wdev)
			return -EINVAL;	
			
	wdev->wiphy->interface_modes |= (BIT(NL80211_IFTYPE_P2P_CLIENT)
		| BIT(NL80211_IFTYPE_P2P_GO));		

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	RT_MOD_INC_USE_COUNT();

	RTMP_OS_NETDEV_START_QUEUE(dev_p);
    AsicSetBssid(pAd, pAd->cfg80211_ctrl.P2PCurrentAddress, 0x1);
    AsicSetDevMac(pAd, pAd->cfg80211_ctrl.P2PCurrentAddress, 0x1);

#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
        wdev->wiphy->interface_modes |=  BIT(RT_CMD_80211_IFTYPE_P2P_DEVICE);
#endif /* LINUX_VERSION_CODE: 3.7.0 */
	
#ifndef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	wdev->iftype = RT_CMD_80211_IFTYPE_P2P_CLIENT;
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */


	return 0;
}

static INT CFG80211_DummyP2pIf_Close(
	IN PNET_DEV		dev_p)
{
	struct wireless_dev *wdev = dev_p->ieee80211_ptr;

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	VOID *pAdSrc;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	BOOLEAN isGoOn = FALSE;	
	UINT apidx = 1;
	struct wifi_dev *Wdev;


#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */



	if (!wdev)
			return -EINVAL;

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	if (pAd->flg_apcli_init)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_Close\n"));
		CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
//		RT_MOD_DEC_USE_COUNT();
		ApCli_Close(pAd, dev_p);
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	RTMP_OS_NETDEV_STOP_QUEUE(dev_p);


	if (INFRA_ON(pAd))
		AsicEnableBssSync(pAd,100);
	else if (ADHOC_ON(pAd))
		AsicEnableIbssSync(pAd);
	else
		AsicDisableSync(pAd);

	//VIRTUAL_IF_DOWN(pAd);

	if (cfg80211_ctrl->dummy_p2p_net_dev)
	{
//iverson
		if (isGoOn)
		{
			Wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			wdev_bcn_buf_deinit(pAd, &pAd->ApCfg.MBSSID[apidx].bcn_buf);
			rtmp_wdev_idx_unreg(pAd, Wdev);
			Wdev->if_dev = NULL;
		}
		else 
		if (pAd->flg_apcli_init)
		{
			Wdev = &pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev;

			OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
			cfg80211_disconnected(dev_p, 0, NULL, 0, GFP_KERNEL);
			CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);		
			NdisZeroMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, MAC_ADDR_LEN);
//			RtmpOSNetDevDetach(dev_p);
			rtmp_wdev_idx_unreg(pAd, Wdev);
			pAd->flg_apcli_init = FALSE;
	//		Wdev->if_dev = NULL;
		}
	
	}
	AsicSetBssid(pAd, pAd->cfg80211_ctrl.P2PCurrentAddress, 0x1); 
	AsicSetBssid(pAd, pAd->CurrentAddress, 0x0); 

#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

	wdev->wiphy->interface_modes = (wdev->wiphy->interface_modes)
					& (~(BIT(NL80211_IFTYPE_P2P_CLIENT)
					| BIT(NL80211_IFTYPE_P2P_GO)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
					| BIT(RT_CMD_80211_IFTYPE_P2P_DEVICE)
#endif /* LINUX_VERSION_CODE: 3.7.0 */
				       ));
#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	RT_MOD_DEC_USE_COUNT();
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

	return 0;
}

static INT CFG80211_DummyP2pIf_Ioctl(
	IN PNET_DEV				dev_p, 
	IN OUT VOID 			*rq_p, 
	IN INT 					cmd)
{
	RTMP_ADAPTER *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s --->\n", __FUNCTION__));

	return rt28xx_ioctl(dev_p, rq_p, cmd);

}

static INT CFG80211_DummyP2pIf_PacketSend(
#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	struct sk_buff *skb, 
	PNET_DEV dev_p)
#else
	IN PNDIS_PACKET 	skb_p, 
	IN PNET_DEV			dev_p)
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */
{

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE

	struct wifi_dev *wdev;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ---> %d\n", __FUNCTION__, dev_p->ieee80211_ptr->iftype));

	if(!(RTMP_OS_NETDEV_STATE_RUNNING(dev_p)))
{
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, skb, NDIS_STATUS_FAILURE);
	return 0;
}

	/* The device not ready to send packt. */
	wdev = RTMP_OS_NETDEV_GET_WDEV(dev_p);
	ASSERT(wdev);
	if (!wdev) return -1;

	NdisZeroMemory((PUCHAR)&skb->cb[CB_OFF], 26);
	MEM_DBG_PKT_ALLOC_INC(skb);

	return CFG80211_PacketSend(skb, dev_p, rt28xx_packet_xmit);

#else
	return 0;
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

}

VOID RTMP_CFG80211_DummyP2pIf_Remove(
	IN VOID 		*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	PNET_DEV dummy_p2p_net_dev = (PNET_DEV)cfg80211_ctrl->dummy_p2p_net_dev;
	struct wifi_dev *wdev = &cfg80211_ctrl->dummy_p2p_wdev;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s =====> \n", __FUNCTION__));
	RtmpOSNetDevProtect(1);
	if (dummy_p2p_net_dev)
	{

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
		RTMP_CFG80211_RemoveVifEntry(pAd, dummy_p2p_net_dev);
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

		RTMP_OS_NETDEV_STOP_QUEUE(dummy_p2p_net_dev);

#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
		if (netif_carrier_ok(dummy_p2p_net_dev))
			netif_carrier_off(dummy_p2p_net_dev);

#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */
		RtmpOSNetDevDetach(dummy_p2p_net_dev);

		rtmp_wdev_idx_unreg(pAd, wdev);
        wdev->if_dev = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0))
		if (dummy_p2p_net_dev->ieee80211_ptr)
        {
        	kfree(dummy_p2p_net_dev->ieee80211_ptr);
            dummy_p2p_net_dev->ieee80211_ptr = NULL;
        }
#endif

		RtmpOSNetDevProtect(0);		
		RtmpOSNetDevFree(dummy_p2p_net_dev);	
		RtmpOSNetDevProtect(1);		
		
		cfg80211_ctrl->flg_cfg_dummy_p2p_init = FALSE;
	}
	RtmpOSNetDevProtect(0);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s <=====\n", __FUNCTION__));
}
	
VOID RTMP_CFG80211_DummyP2pIf_Init(
	IN VOID 		*pAdSrc)
{
#define INF_CFG80211_DUMMY_P2P_NAME "p2p"

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV	new_dev_p;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;
	UINT preIfIndex = 0;
	struct wireless_dev *pWdev;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s =====> \n", __FUNCTION__));
	if (cfg80211_ctrl->flg_cfg_dummy_p2p_init != FALSE)
		return;

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE 
	cfg80211_ctrl->P2POpStatusFlags	= CFG_P2P_DISABLE;
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE*/

	cfg80211_ctrl->bP2pCliPmEnable = FALSE;
	cfg80211_ctrl->bPreKeepSlient = FALSE;
	cfg80211_ctrl->bKeepSlient = FALSE;
	cfg80211_ctrl->NoAIndex = MAX_LEN_OF_MAC_TABLE;
	cfg80211_ctrl->MyGOwcid = MAX_LEN_OF_MAC_TABLE;
	cfg80211_ctrl->CTWindows = 0;	/* CTWindows and OppPS parameter field */
	
	pNetDevOps=&netDevHook;

	/* init operation functions and flags */
	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_DummyP2pIf_Open;	         /* device opem hook point */
	netDevHook.stop = CFG80211_DummyP2pIf_Close;	     /* device close hook point */
	netDevHook.xmit = CFG80211_DummyP2pIf_PacketSend;    /* hard transmit hook point */
	netDevHook.ioctl = CFG80211_DummyP2pIf_Ioctl;	     /* ioctl hook point */	
	
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), INF_CFG80211_DUMMY_P2P_NAME);

	if (new_dev_p == NULL)
	{
		/* allocation fail, exit */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Allocate network device fail (CFG80211: Dummy P2P IF)...\n"));
		return;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Register CFG80211 I/F (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
	}

	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	NdisMoveMemory(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	pNetDevOps->needProtcted = TRUE;
	
#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE

#ifdef MT_MAC
	INT32 Value;
        UCHAR MacByte = 0;

        //TODO: shall we make choosing which byte to be selectable???
        Value = 0x00000000L;
        RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);//read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.
        Value = Value | (0x2 << 29);//Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address
        RTMP_IO_WRITE32(pAd, LPON_BTEIR, Value);
        MacByte = Value >> 29;
	
	pNetDevOps->devAddr[0] |= 0x2;
	
                        switch (MacByte) {
                                case 0x1: /* choose bit[23:20]*/
                                        pNetDevOps->devAddr[2] = (pNetDevOps->devAddr[2] = pNetDevOps->devAddr[2] & 0x0f);
                                        break;
                                case 0x2: /* choose bit[31:28]*/
                                        pNetDevOps->devAddr[3] = (pNetDevOps->devAddr[3] = pNetDevOps->devAddr[3] & 0x0f);
                                        break;
                                case 0x3: /* choose bit[39:36]*/
                                        pNetDevOps->devAddr[4] = (pNetDevOps->devAddr[4] = pNetDevOps->devAddr[4] & 0x0f);
                                        break;
                                case 0x4: /* choose bit [47:44]*/
                                        pNetDevOps->devAddr[5] = (pNetDevOps->devAddr[5] = pNetDevOps->devAddr[5] & 0x0f);
                                        break;
                                default: /* choose bit[15:12]*/
                                        pNetDevOps->devAddr[1] = (pNetDevOps->devAddr[1] = pNetDevOps->devAddr[1] & 0x0f);
                                        break;
                        }

	AsicSetDevMac(pAd, pNetDevOps->devAddr, 0x1);//set own_mac to HWBSSID1
#else
	//CFG_TODO
	/* 	 
		Bit1 of MAC address Byte0 is local administration bit 
		and should be set to 1 in extended multiple BSSIDs'
		Bit3~ of MAC address Byte0 is extended multiple BSSID index.
	*/	
	if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
		pNetDevOps->devAddr[0] += 2; /* NEW BSSID */
	else
	{
#ifdef P2P_ODD_MAC_ADJUST
		if (pNetDevOps->devAddr[5] & 0x01 == 0x01)
			pNetDevOps->devAddr[5] -= 1;
		else
#endif /* P2P_ODD_MAC_ADJUST */
		pNetDevOps->devAddr[5] += FIRST_MBSSID;
	}		
#endif /* MT_MAC */	
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pNetDevOps->devAddr);

#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

	
	pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
	if (unlikely(!pWdev)) 
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Could not allocate wireless device\n"));
		return;
	} else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("alloc p2p dummy wdev(0x%x)\n",pWdev));


	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));	
	pWdev->netdev = new_dev_p;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
	pWdev->iftype = RT_CMD_80211_IFTYPE_P2P_DEVICE;	
#else
	pWdev->iftype = RT_CMD_80211_IFTYPE_P2P_CLIENT;
#endif /* LINUX_VERSION_CODE: 3.7.0 */
		/* interface_modes move from IF open to init */

	struct wifi_dev *wdev = NULL;
	wdev = &cfg80211_ctrl->dummy_p2p_wdev;
	wdev->wdev_type = WDEV_TYPE_STA;
	wdev->sys_handle = (void *) pAd;
	wdev->if_dev = new_dev_p;

	COPY_MAC_ADDR(wdev->if_addr, pNetDevOps->devAddr);

	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
        RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
	if (rtmp_wdev_idx_reg(pAd, wdev) < 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("===============> fail register the wdev for dummy p2p\n"));
	}
	
	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps); 
	cfg80211_ctrl->dummy_p2p_net_dev = new_dev_p;
	cfg80211_ctrl->flg_cfg_dummy_p2p_init = TRUE;
#ifdef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
	AsicSetBssid(pAd, pNetDevOps->devAddr, 0x1); 
	/* Record the pNetDevice to Cfg80211VifDevList */
	RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, pWdev->iftype);

#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s <=====\n", __FUNCTION__));
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

#ifdef CFG80211_MULTI_STA
BOOLEAN RTMP_CFG80211_MULTI_STA_ON(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
        PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	if (!pNewNetDev) 
		return FALSE;

	pListEntry = pCacheList->pHead;
        pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
        while (pDevEntry != NULL)
        {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN)
		    && (pNewNetDev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION))
                        return TRUE;

                pListEntry = pListEntry->pNext;
                pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
        }
	
	return FALSE;	
}

VOID RTMP_CFG80211_MutliStaIf_Init(VOID *pAdSrc)
{
	printk("%s()\n", __FUNCTION__);
#define INF_CFG80211_MULTI_STA_NAME "muti-sta0"
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	vifInfo.vifType = RT_CMD_80211_IFTYPE_STATION;
     	vifInfo.vifNameLen = strlen(INF_CFG80211_MULTI_STA_NAME);
        NdisZeroMemory(vifInfo.vifName, sizeof(vifInfo.vifName));
        NdisCopyMemory(vifInfo.vifName, INF_CFG80211_MULTI_STA_NAME, vifInfo.vifNameLen);
	
	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" %s() VIF Add error.\n", __FUNCTION__));
		return;
	}

	cfg80211_ctrl->multi_sta_net_dev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_STATION);
        cfg80211_ctrl->flg_cfg_multi_sta_init = TRUE;

}

VOID RTMP_CFG80211_MutliStaIf_Remove(VOID *pAdSrc)
{
	printk("%s()\n", __FUNCTION__);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	
	if (cfg80211_ctrl->multi_sta_net_dev)
	{
		RtmpOSNetDevProtect(1);
		RTMP_DRIVER_80211_VIF_DEL(pAd, cfg80211_ctrl->multi_sta_net_dev, 
					RT_CMD_80211_IFTYPE_STATION);
		RtmpOSNetDevProtect(0);
		cfg80211_ctrl->flg_cfg_multi_sta_init = FALSE;
	}
}
#endif /* CFG80211_MULTI_STA */
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE: 2.6.28 */

