/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt_main_dev.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include "rt_config.h"


#ifdef CONFIG_APSTA_MIXED_SUPPORT
UINT32 CW_MAX_IN_BITS;
#endif // CONFIG_APSTA_MIXED_SUPPORT //

/*---------------------------------------------------------------------*/
/* Private Variables Used                                              */
/*---------------------------------------------------------------------*/

PSTRING mac = "";		   // default 00:00:00:00:00:00
PSTRING hostname = "";		   // default CMPC
module_param (mac, charp, 0);
MODULE_PARM_DESC (mac, "rt28xx: wireless mac addr");

#ifdef OS_ABL_SUPPORT

UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif // OS_ABL_SUPPORT //


/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/

// public function prototype
int rt28xx_close(IN struct net_device *net_dev);
int rt28xx_open(struct net_device *net_dev);

// private function prototype
static INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);




static struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev);

/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int MainVirtualIF_close(IN struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	// Sanity check for pAd
	if (pAd == NULL)
		return 0; // close ok

	netif_carrier_off(pAd->net_dev);
	netif_stop_queue(pAd->net_dev);

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = FALSE;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		// kick out all STAs behind the bss.
		MbssKickOutStas(pAd, MAIN_MBSSID, REASON_DISASSOC_INACTIVE);
	}

	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);
#endif // CONFIG_AP_SUPPORT //



#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef QOS_DLS_SUPPORT
		// send DLS-TEAR_DOWN message, 
		if (pAd->CommonCfg.bDLSCapable)
		{
			UCHAR i;

			// tear down local dls table entry
			for (i=0; i<MAX_NUM_OF_INIT_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}

			// tear down peer dls table entry
			for (i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}
			RTMP_MLME_HANDLER(pAd);
		}
#endif // QOS_DLS_SUPPORT //

		if (INFRA_ON(pAd) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);

			if (MsgElem)
			{
			COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
			DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;

			MsgElem->Machine = ASSOC_STATE_MACHINE;
			MsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			// Prevent to connect AP again in STAMlmePeriodicExec
			pAd->MlmeAux.AutoReconnectSsidLen= 32;
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, MsgElem);
			kfree(MsgElem);
			}

			RTMPusecDelay(1000);
		}

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		// send wireless event to wpa_supplicant for infroming interface down.
		RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //

		if (pAd->StaCfg.pWpsProbeReqIe)
		{
			kfree(pAd->StaCfg.pWpsProbeReqIe);
			pAd->StaCfg.pWpsProbeReqIe = NULL;
			pAd->StaCfg.WpsProbeReqIeLen = 0;
		}

		if (pAd->StaCfg.pWpaAssocIe)
		{
			kfree(pAd->StaCfg.pWpaAssocIe);
			pAd->StaCfg.pWpaAssocIe = NULL;
			pAd->StaCfg.WpaAssocIeLen = 0;
		}
#endif // WPA_SUPPLICANT_SUPPORT //


	}
#endif // CONFIG_STA_SUPPORT //

	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0; // close ok
}

/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int MainVirtualIF_open(IN struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	// Sanity check for pAd
	if (pAd == NULL)
		return 0; // close ok

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = TRUE;
#endif // CONFIG_AP_SUPPORT //
	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	// increase MODULE use count
	RT_MOD_INC_USE_COUNT();

	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	return 0;
}

/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
extern VOID BG_FTPH_Remove(VOID);
#endif
#endif
int rt28xx_close(IN PNET_DEV dev)
{
	struct net_device * net_dev = (struct net_device *)dev;
	RTMP_ADAPTER	*pAd = NULL;
	UINT32			i = 0;
	BOOLEAN                 Cancelled;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	DBGPRINT(RT_DEBUG_TRACE, ("===> rt28xx_close\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif // BG_FT_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	Cancelled = FALSE;
	// Sanity check for pAd
	if (pAd == NULL)
		return 0; // close ok

#ifdef WMM_ACM_SUPPORT
	/* must call first */
	ACMP_Release(pAd);
#endif // WMM_ACM_SUPPORT //


#ifdef RTMP_RBUS_SUPPORT
#ifdef RT3XXX_ANTENNA_DIVERSITY_SUPPORT
	RT3XXX_AntDiversity_Fini(pAd);
#endif // RT3XXX_ANTENNA_DIVERSITY_SUPPORT //
#endif // RTMP_RBUS_SUPPORT //

#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif // WDS_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef PCIE_PS_SUPPORT
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_CLOSE);
#endif // PCIE_PS_SUPPORT //

		// If dirver doesn't wake up firmware here,
		// NICLoadFirmware will hang forever when interface is up again.
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        {      
		    AsicForceWakeup(pAd, TRUE);
        }


		//MlmeRadioOff(pAd);
#ifdef RTMP_MAC_PCI
		pAd->bPCIclkOff = FALSE;    
#endif // RTMP_MAC_PCI //
	}
#endif // CONFIG_STA_SUPPORT //

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	RtmpOsMsDelay(20); /* wait for disconnect requests transmitted */

	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RTMPusecDelay(1000);
		}
	}
	

#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{

#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)
		{
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}
#endif // DOT11N_DRAFT3 //

		// PeriodicTimer already been canceled by MlmeHalt() API.
		//RTMPCancelTimer(&pAd->PeriodicTimer,	&Cancelled);
	}
#endif // CONFIG_AP_SUPPORT //

	// Stop Mlme state machine
	MlmeHalt(pAd);
	
	// Close net tasklets
	RtmpNetTaskExit(pAd);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		MacTableReset(pAd);
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif // MAT_SUPPORT //
#ifdef LED_CONTROL_SUPPORT
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif // LED_CONTROL_SUPPORT //
		MlmeRadioOff(pAd);
	}
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif // MAT_SUPPORT //

#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif // CLIENT_WDS //
		// Shutdown Access Point function, release all related resources 
		APShutdown(pAd);

		// Free BssTab & ChannelInfo tabbles.
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
	}
#endif // CONFIG_AP_SUPPORT //

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);



#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		INT ap_idx;
		for (ap_idx = 0; ap_idx < pAd->ApCfg.BssidNum; ap_idx++)
			WscStop(pAd, FALSE, &pAd->ApCfg.MBSSID[ap_idx].WscControl);
#ifdef APCLI_SUPPORT
		WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[BSS0].WscControl);
#endif // APCLI_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
				FALSE,
#endif // CONFIG_AP_SUPPORT //
				&pAd->StaCfg.WscControl);
#endif // CONFIG_STA_SUPPORT //

#ifdef OLD_DH_KEY
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	    	WSC_VFREE_KEY_MEM(pAd->ApCfg.MBSSID[0].WscControl.pPubKeyMem, pAd->ApCfg.MBSSID[0].WscControl.pSecKeyMem);
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WSC_VFREE_KEY_MEM(pAd->StaCfg.WscControl.pPubKeyMem, pAd->StaCfg.WscControl.pSecKeyMem);
#endif // CONFIG_STA_SUPPORT //
#endif // OLD_DH_KEY //

	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Stop(pAd);
#endif // WSC_INCLUDED //

	// Close kernel threads
	RtmpMgmtTaskExit(pAd);

#ifdef RTMP_MAC_PCI
	{
#ifdef RTMP_PCI_SUPPORT
			BOOLEAN brc;
#endif

			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			{
				RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			}

			// Receive packets to clear DMA index after disable interrupt. 
			//RTMPHandleRxDoneInterrupt(pAd);
			// put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all 
			// register access before Radio off.

#ifdef RTMP_PCI_SUPPORT
			brc=RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);

//In  solution 3 of 3090F, the bPCIclkOff will be set to TRUE after calling RT28xxPciAsicRadioOff
#ifdef PCIE_PS_SUPPORT
			pAd->bPCIclkOff = FALSE;    
#endif // PCIE_PS_SUPPORT //

			if (brc==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
			}
#endif // RTMP_PCI_SUPPORT //
	}
	

/*
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}

	// Disable Rx, register value supposed will remain after reset
	NICIssueReset(pAd);
*/
#endif // RTMP_MAC_PCI //

	// Free IRQ
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
#ifdef RTMP_MAC_PCI
		// Deregister interrupt function
		RtmpOSIRQRelease(net_dev);
#endif // RTMP_MAC_PCI //
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}

#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	// Free Ring or USB buffers
	RTMPFreeTxRxRingMemory(pAd);
#endif // RESOURCE_PRE_ALLOC //

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef DOT11_N_SUPPORT
	// Free BA reorder resource
	ba_reordering_resource_release(pAd);
#endif // DOT11_N_SUPPORT //
	
#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

/*+++Modify by woody to solve the bulk fail+++*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef DOT11Z_TDLS_SUPPORT
		TDLS_Table_Destory(pAd);
#endif // DOT11Z_TDLS_SUPPORT //
	}
#endif // CONFIG_STA_SUPPORT //

#ifdef VENDOR_FEATURE2_SUPPORT
	printk("Number of Packet Allocated = %d\n", pAd->NumOfPktAlloc);
	printk("Number of Packet Freed = %d\n", pAd->NumOfPktFree);
#endif // VENDOR_FEATURE2_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt28xx_close\n"));
	return 0; // close ok
} /* End of rt28xx_close */


/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
========================================================================
*/

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
extern VOID BG_FTPH_Init(VOID);
#endif
#endif

int rt28xx_open(IN PNET_DEV dev)
{				 
	struct net_device * net_dev = (struct net_device *)dev;
	PRTMP_ADAPTER pAd = NULL;
	int retval = 0;
 	//POS_COOKIE pObj;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	// Sanity check for pAd
	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -1;
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		CW_MAX_IN_BITS = 6;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		CW_MAX_IN_BITS = 10;
	}
#endif // CONFIG_APSTA_MIXED_SUPPORT //

#if WIRELESS_EXT >= 12
	if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MAIN) 
	{
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if (pAd->OpMode == OPMODE_AP)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_ap_iw_handler_def;
#endif // CONFIG_APSTA_MIXED_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		if (pAd->OpMode == OPMODE_STA)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_iw_handler_def;
#endif // CONFIG_STA_SUPPORT //
	}
#endif // WIRELESS_EXT >= 12 //

	// Request interrupt service routine for PCI device
	// register the interrupt routine with the os
	/*
		AP Channel auto-selection will be run in rt28xx_init(),
		so we must reqister IRQ hander here.
	*/
	RtmpOSIRQRequest(net_dev);

	// Init IRQ parameters stored in pAd
	RTMP_IRQ_INIT(pAd);
	
	// Chip & other init
	if (rt28xx_init(pAd, mac, hostname) == FALSE)
		goto err;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RT_CFG80211_REINIT(pAd);
	RT_CFG80211_CRDA_REG_RULE_APPLY(pAd);
#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //

#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //

	// Enable Interrupt
	RTMP_IRQ_ENABLE(pAd);

	// Now Enable RxTx
	RTMPEnableRxTx(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

	{
	UINT32 reg = 0;
	RTMP_IO_READ32(pAd, 0x1300, &reg);  // clear garbage interrupts
	if (reg);
	DBGPRINT(RT_DEBUG_TRACE, ("0x1300 = %08x\n", reg));
	}

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
        RTMPInitPCIeLinkCtrlValue(pAd);
#endif // PCIE_PS_SUPPORT //

#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif // BG_FT_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#ifdef VENDOR_FEATURE2_SUPPORT
	printk("Number of Packet Allocated in open = %d\n", pAd->NumOfPktAlloc);
	printk("Number of Packet Freed in open = %d\n", pAd->NumOfPktFree);
#endif // VENDOR_FEATURE2_SUPPORT //

	return (retval);

err:
//+++move from rt28xx_init() to here.
	RtmpOSIRQRelease(net_dev);
//---move from rt28xx_init() to here.

	return (-1);
} /* End of rt28xx_open */


PNET_DEV RtmpPhyNetDevInit(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_OS_NETDEV_OP_HOOK *pNetDevHook)
{
	struct net_device	*net_dev = NULL;
//	NDIS_STATUS		Status;
	
	net_dev = RtmpOSNetDevCreate(pAd, INT_MAIN, 0, sizeof(PRTMP_ADAPTER), INF_MAIN_DEV_NAME);
	if (net_dev == NULL)
	{
		printk("RtmpPhyNetDevInit(): creation failed for main physical net device!\n");
		return NULL;
	}

	NdisZeroMemory((unsigned char *)pNetDevHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	pNetDevHook->open = MainVirtualIF_open;
	pNetDevHook->stop = MainVirtualIF_close;
	pNetDevHook->xmit = rt28xx_send_packets;
#ifdef IKANOS_VX_1X0
	pNetDevHook->xmit = IKANOS_DataFramesTx;
#endif // IKANOS_VX_1X0 //
	pNetDevHook->ioctl = rt28xx_ioctl;
	pNetDevHook->priv_flags = INT_MAIN;
	pNetDevHook->get_stats = RT28xx_get_ether_stats;

	pNetDevHook->needProtcted = FALSE;

#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
	pNetDevHook->get_wstats = rt28xx_get_wireless_stats;
#endif

#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
	if (pAd->OpMode == OPMODE_STA)
	{
		pNetDevHook->iw_handler = (void *)&rt28xx_iw_handler_def;
	}
#endif //WIRELESS_EXT >= 12
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
	if (pAd->OpMode == OPMODE_AP)
	{
		pNetDevHook->iw_handler = &rt28xx_ap_iw_handler_def;
	}
#endif //WIRELESS_EXT >= 12
#endif // CONFIG_APSTA_MIXED_SUPPORT //

	RTMP_OS_NETDEV_SET_PRIV(net_dev, pAd);
	pAd->net_dev = net_dev;
	
#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = net_dev;
#endif // CONFIG_AP_SUPPORT //

	return net_dev;
}


/*
========================================================================
Routine Description:
    The entry point for Linux kernel sent packet to our driver.

Arguments:
    sk_buff *skb		the pointer refer to a sk_buffer.

Return Value:
    0					

Note:
	This function is the entry point of Tx Path for Os delivery packet to 
	our driver. You only can put OS-depened & STA/AP common handle procedures 
	in here.
========================================================================
*/
int rt28xx_packet_xmit(struct sk_buff *skb)
{
	struct net_device *net_dev = skb->dev;
	PRTMP_ADAPTER pAd = NULL;
	PNDIS_PACKET pPacket = (PNDIS_PACKET) skb;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	/* RT2870STA does this in RTMPSendPackets() */
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
		return 0;
	}
#endif // RALINK_ATE //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		// Drop send request since we are in monitor mode
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}
#endif // CONFIG_STA_SUPPORT //

        // EapolStart size is 18
	if (skb->len < 14)
	{
		//printk("bad packet size: %d\n", pkt->len);
		hex_dump("bad packet", skb->data, skb->len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		goto done;
	}

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#if !defined (CONFIG_RA_NAT_NONE)
	if (ra_sw_nat_hook_tx != NULL)
	{
		ra_sw_nat_hook_tx(pPacket, 0);
	}
#endif
#endif

#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);
	if (*(int*)(skb->cb) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
	}
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APSendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		STASendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
	}
#endif // CONFIG_STA_SUPPORT //

done:

	return 0;
}


/*
========================================================================
Routine Description:
    Send a packet to WLAN.

Arguments:
    skb_p           points to our adapter
    dev_p           which WLAN network interface

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
static int rt28xx_send_packets(
	IN struct sk_buff 		*skb_p, 
	IN struct net_device 	*net_dev)
{
	RTMP_ADAPTER *pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	if (!(RTMP_OS_NETDEV_STATE_RUNNING(net_dev)))
	{
		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}

	NdisZeroMemory((PUCHAR)&skb_p->cb[CB_OFF], 15);
	RTMP_SET_PACKET_NET_DEVICE_MBSSID(skb_p, MAIN_MBSSID);

	MEM_DBG_PKT_ALLOC_INC(pAd);

	return rt28xx_packet_xmit(skb_p);
}


#if WIRELESS_EXT >= 12
// This function will be called when query /proc
struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev)
{
	PRTMP_ADAPTER pAd = NULL;
#ifdef CONFIG_AP_SUPPORT 
	PMAC_TABLE_ENTRY pMacEntry = NULL;
#endif // CONFIG_AP_SUPPORT //

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef APCLI_SUPPORT
		if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_APCLI)
		{
			INT ApCliIdx = ApCliIfLookUp(pAd, (PUCHAR)net_dev->dev_addr);
			if ((ApCliIdx >= 0) && VALID_WCID(pAd->ApCfg.ApCliTab[ApCliIdx].MacTabWCID))
				pMacEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[ApCliIdx].MacTabWCID];
		}
		else
#endif // APCLI_SUPPORT //
		{
			/*
				only AP client support wireless stats function.
				return NULL pointer for all other cases.
			*/
			pMacEntry = &pAd->MacTab.Content[0];
		}
	}
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_get_wireless_stats --->\n"));

	//check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return NULL;	
	
	pAd->iw_stats.status = 0; // Status - device dependent for now

	// link quality
#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
		pAd->iw_stats.qual.qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
#endif // CONFIG_STA_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		if (pMacEntry != NULL)
			pAd->iw_stats.qual.qual = ((pMacEntry->ChannelQuality * 12)/10 + 10);
	}
#endif // CONFIG_AP_SUPPORT //

	if(pAd->iw_stats.qual.qual > 100)
		pAd->iw_stats.qual.qual = 100;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		pAd->iw_stats.qual.level =
			RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0,
							pAd->StaCfg.RssiSample.LastRssi1,
							pAd->StaCfg.RssiSample.LastRssi2);
	}
#endif // CONFIG_STA_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		if (pMacEntry != NULL)
			pAd->iw_stats.qual.level =
				RTMPMaxRssi(pAd, pMacEntry->RssiSample.LastRssi0,
								pMacEntry->RssiSample.LastRssi1,
								pMacEntry->RssiSample.LastRssi2);
	}
#endif // CONFIG_AP_SUPPORT //

	pAd->iw_stats.qual.noise = pAd->BbpWriteLatch[66]; // noise level (dBm)
	
	pAd->iw_stats.qual.noise += 256 - 143;
	pAd->iw_stats.qual.updated = 1;     // Flags to know if updated
#ifdef IW_QUAL_DBM
	pAd->iw_stats.qual.updated |= IW_QUAL_DBM;	// Level + Noise are dBm
#endif // IW_QUAL_DBM //

	pAd->iw_stats.discard.nwid = 0;     // Rx : Wrong nwid/essid
	pAd->iw_stats.miss.beacon = 0;      // Missed beacons/superframe
	
	DBGPRINT(RT_DEBUG_TRACE, ("<--- rt28xx_get_wireless_stats\n"));
	return &pAd->iw_stats;
}
#endif // WIRELESS_EXT //


#ifdef WORKQUEUE_BH
void tbtt_workq(struct work_struct *work)
#else
void tbtt_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
//#define MAX_TX_IN_TBTT		(16)

#ifdef CONFIG_AP_SUPPORT
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, tbtt_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif // WORKQUEUE_BH //

#ifdef RTMP_MAC_PCI
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef AP_QLOAD_SUPPORT
		/* update channel utilization */
		QBSS_LoadUpdate(pAd, 0);
#endif // AP_QLOAD_SUPPORT //

	}
#endif // RTMP_MAC_PCI //

	if (pAd->OpMode == OPMODE_AP)
	{
		//
		// step 7 - if DTIM, then move backlogged bcast/mcast frames from PSQ to TXQ whenever DtimCount==0
#ifdef RTMP_MAC_PCI
		// NOTE: This updated BEACON frame will be sent at "next" TBTT instead of at cureent TBTT. The reason is
		//       because ASIC already fetch the BEACON content down to TX FIFO before driver can make any
		//       modification. To compenstate this effect, the actual time to deilver PSQ frames will be
		//       at the time that we wrapping around DtimCount from 0 to DtimPeriod-1
		if (pAd->ApCfg.DtimCount == 0)
#endif // RTMP_MAC_PCI //
		{
			PQUEUE_ENTRY    pEntry;
			BOOLEAN			bPS = FALSE;
			UINT 			count = 0;
			unsigned long 		IrqFlags;

//			NdisAcquireSpinLock(&pAd->MacTabLock);
//			NdisAcquireSpinLock(&pAd->TxSwQueueLock);
			
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head)
			{
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (MAX_PACKETS_IN_QUEUE + MAX_PACKETS_IN_MCAST_PS_QUEUE))
				{
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					//if(pAd->MacTab.McastPsQueue.Number)
					if (count)
					{
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
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
			
			
//			NdisReleaseSpinLock(&pAd->TxSwQueueLock);
//			NdisReleaseSpinLock(&pAd->MacTabLock);
			if (pAd->MacTab.McastPsQueue.Number == 0)
			{			
                UINT bss_index;

                /* clear MCAST/BCAST backlog bit for all BSS */
                for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
                /* End of for */
			}
			pAd->MacTab.PsQIdleCount = 0;

			// Dequeue outgoing framea from TxSwQueue0..3 queue and process it
            if (bPS == TRUE) 
			{
				RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
		}
	}
#endif // CONFIG_AP_SUPPORT //
}

INT rt28xx_ioctl(
	IN	PNET_DEV	net_dev, 
	IN	OUT	struct ifreq	*rq, 
	IN	INT					cmd)
{
	RTMP_ADAPTER	*pAd = NULL;
	INT				ret = 0;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		ret = rt28xx_ap_ioctl(net_dev, rq, cmd);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		ret = rt28xx_sta_ioctl(net_dev, rq, cmd);
	}
#endif // CONFIG_STA_SUPPORT //

	return ret;
}


/*
    ========================================================================

    Routine Description:
        return ethernet statistics counter

    Arguments:
        net_dev                     Pointer to net_device

    Return Value:
        net_device_stats*

    Note:

    ========================================================================
*/
static struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = NULL;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	if (pAd)
	{

		pAd->stats.rx_packets = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
		pAd->stats.tx_packets = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;

		pAd->stats.rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
		pAd->stats.tx_bytes = pAd->RalinkCounters.TransmittedByteCount;

		pAd->stats.rx_errors = pAd->Counters8023.RxErrors;
		pAd->stats.tx_errors = pAd->Counters8023.TxErrors;

		pAd->stats.rx_dropped = 0;
		pAd->stats.tx_dropped = 0;

	    pAd->stats.multicast = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;   // multicast packets received
	    pAd->stats.collisions = pAd->Counters8023.OneCollision + pAd->Counters8023.MoreCollisions;  // Collision packets

	    pAd->stats.rx_length_errors = 0;
	    pAd->stats.rx_over_errors = pAd->Counters8023.RxNoBuffer;                   // receiver ring buff overflow
	    pAd->stats.rx_crc_errors = 0;//pAd->WlanCounters.FCSErrorCount;     // recved pkt with crc error
	    pAd->stats.rx_frame_errors = pAd->Counters8023.RcvAlignmentErrors;          // recv'd frame alignment error
	    pAd->stats.rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   // recv'r fifo overrun
	    pAd->stats.rx_missed_errors = 0;                                            // receiver missed packet

	    // detailed tx_errors
	    pAd->stats.tx_aborted_errors = 0;
	    pAd->stats.tx_carrier_errors = 0;
	    pAd->stats.tx_fifo_errors = 0;
	    pAd->stats.tx_heartbeat_errors = 0;
	    pAd->stats.tx_window_errors = 0;

	    // for cslip etc
	    pAd->stats.rx_compressed = 0;
	    pAd->stats.tx_compressed = 0;
		
		return &pAd->stats;
	}
	else
    	return NULL;
}


BOOLEAN RtmpPhyNetDevExit(
	IN RTMP_ADAPTER *pAd, 
	IN PNET_DEV net_dev)
{


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	// remove all AP-client virtual interfaces.
	RT28xx_ApCli_Remove(pAd);
#endif // APCLI_SUPPORT //

#ifdef WDS_SUPPORT
	// remove all WDS virtual interfaces.
	RT28xx_WDS_Remove(pAd);
#endif // WDS_SUPPORT //

#ifdef MBSS_SUPPORT
	RT28xx_MBSS_Remove(pAd);
#endif // MBSS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef INF_PPA_SUPPORT
	if (ppa_hook_directpath_register_dev_fn && pAd->PPAEnable==TRUE) 
	{
		UINT status;
		status=ppa_hook_directpath_register_dev_fn(&pAd->g_if_id, pAd->net_dev, NULL, PPA_F_DIRECTPATH_REGISTER);
		DBGPRINT(RT_DEBUG_TRACE, ("unregister PPA:g_if_id=%d status=%d\n",pAd->g_if_id,status));
	}
	kfree(pAd->pDirectpathCb); 
#endif // INF_PPA_SUPPORT //

	// Unregister network device
	if (net_dev != NULL)
	{
		printk("RtmpOSNetDevDetach(): RtmpOSNetDeviceDetach(), dev->name=%s!\n", net_dev->name);
		RtmpOSNetDevDetach(net_dev);
	}

	return TRUE;
	
}


/*******************************************************************************

	Device IRQ related functions.
	
 *******************************************************************************/
int RtmpOSIRQRequest(IN PNET_DEV pNetDev)
{
	struct net_device *net_dev = pNetDev;
	PRTMP_ADAPTER pAd = NULL;
	int retval = 0;
	
	GET_PAD_FROM_NET_DEV(pAd, pNetDev);	
	
	ASSERT(pAd);
	
#ifdef RTMP_PCI_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
	{
		POS_COOKIE _pObj = (POS_COOKIE)(pAd->OS_Cookie);
		RTMP_MSI_ENABLE(pAd);	
		retval = request_irq(_pObj->pci_dev->irq,  rt2860_interrupt, SA_SHIRQ, (net_dev)->name, (net_dev));
		if (retval != 0) 
			printk("RT2860: request_irq  ERROR(%d)\n", retval);
	}
#endif // RTMP_PCI_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
		if ((retval = request_irq(net_dev->irq, rt2860_interrupt, IRQF_SHARED, net_dev->name ,net_dev))) 
#else
		if ((retval = request_irq(net_dev->irq,rt2860_interrupt, SA_INTERRUPT, net_dev->name ,net_dev))) 
#endif
		{
			printk("RT2860: request_irq  ERROR(%d)\n", retval);
		}
	}
#endif // RTMP_RBUS_SUPPORT //

	return retval; 
	
}


int RtmpOSIRQRelease(IN PNET_DEV pNetDev)
{
	struct net_device *net_dev = pNetDev;
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	
	
	ASSERT(pAd);
	
#ifdef RTMP_PCI_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
	{ 
		POS_COOKIE pObj = (POS_COOKIE)(pAd->OS_Cookie);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
		synchronize_irq(pObj->pci_dev->irq);
#endif
		free_irq(pObj->pci_dev->irq, (net_dev));
		RTMP_MSI_DISABLE(pAd);
	}
#endif // RTMP_PCI_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
		synchronize_irq(net_dev->irq);
#endif
		free_irq(net_dev->irq, (net_dev));
	}
#endif // RTMP_RBUS_SUPPORT //

	return 0;
}


