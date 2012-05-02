
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
    ap_wds.c

    Abstract:
    Support WDS function.

    Revision History:
    Who       When            What
    ------    ----------      ----------------------------------------------
    Fonchi    02-13-2007      created
*/

#ifdef WDS_SUPPORT

#include "rt_config.h"

static struct net_device_stats *RT28xx_get_wds_ether_stats(
    IN  struct net_device *net_dev);


// Register WDS interface
VOID RT28xx_WDS_Init(
	IN PRTMP_ADAPTER pAd,
	IN PNET_DEV net_dev)
{
	INT index;
	PNET_DEV pWdsNetDev;
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;
	
	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_wds_init != FALSE)
		return;

	for(index = 0; index < MAX_WDS_ENTRY; index++)
	{
		pWdsNetDev = RtmpOSNetDevCreate(pAd, INT_WDS, index, sizeof(PRTMP_ADAPTER), INF_WDS_DEV_NAME);
		if (pWdsNetDev == NULL)
		{
			/* allocation fail, exit */
			DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (WDS)...\n"));
			break;
		}
		else
		{
			PMAC_TABLE_ENTRY pWdsEntry;
			pWdsEntry = &pAd->MacTab.Content[pAd->WdsTab.WdsEntry[index].MacTabMatchWCID];
			DBGPRINT(RT_DEBUG_TRACE, ("The new WDS interface MAC = %02X:%02X:%02X:%02X:%02X:%02X\n", 
					PRINT_MAC(pWdsEntry->Addr)));
		}

		NdisZeroMemory(&pAd->WdsTab.WdsEntry[index].WdsCounter, sizeof(WDS_COUNTER));
		RTMP_OS_NETDEV_SET_PRIV(pWdsNetDev, pAd);
		pAd->WdsTab.WdsEntry[index].PhyMode = 0xff;
		pAd->WdsTab.WdsEntry[index].dev = pWdsNetDev;
		NdisZeroMemory((PUCHAR)&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
		netDevOpHook.open = WdsVirtualIF_open;
		netDevOpHook.stop = WdsVirtualIF_close;
		netDevOpHook.xmit = WdsVirtualIFSendPackets;
		netDevOpHook.ioctl = WdsVirtualIF_ioctl;
		netDevOpHook.priv_flags = INT_WDS; /* we are virtual interface */
		netDevOpHook.needProtcted = TRUE;
		netDevOpHook.get_stats = RT28xx_get_wds_ether_stats;
		NdisMoveMemory(&netDevOpHook.devAddr[0], RTMP_OS_NETDEV_GET_PHYADDR(net_dev), MAC_ADDR_LEN);
		DBGPRINT(RT_DEBUG_TRACE, ("The new WDS interface MAC = %02X:%02X:%02X:%02X:%02X:%02X\n", 
					PRINT_MAC(netDevOpHook.devAddr)));

		// Register this device
		RtmpOSNetDevAttach(pWdsNetDev, &netDevOpHook);

		pAd->WdsTab.WdsEntry[index].PhyMode = 0xff;
		pAd->WdsTab.WdsEntry[index].dev = pWdsNetDev;
	}

	pAd->flg_wds_init = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("Total Allocate %d WDS interfaces!\n", index));
	
}


INT WdsVirtualIFSendPackets(
	IN PNDIS_PACKET pSkb, 
	IN PNET_DEV dev)
{
	UCHAR i;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = (PNDIS_PACKET) pSkb;

	pAd = (PRTMP_ADAPTER) RTMP_OS_NETDEV_GET_PRIV(dev);


#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}
#endif // RALINK_ATE //

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))          ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}
	
	if (!(RTMP_OS_NETDEV_STATE_RUNNING(dev)))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}

	for (i = 0; i < MAX_WDS_ENTRY; i++)
	{
		if (ValidWdsEntry(pAd, i) && (pAd->WdsTab.WdsEntry[i].dev == dev))
		{
			RTMP_SET_PACKET_NET_DEVICE_WDS(pSkb, i);
			SET_OS_PKT_NETDEV(pSkb, pAd->net_dev);

			return rt28xx_packet_xmit(pSkb);
		}
	}

	
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

	return 0;
}


INT WdsVirtualIF_open(
	IN	PNET_DEV dev)
{
	PRTMP_ADAPTER	pAd;
#ifdef RTL865X_SOC
	INT				index;
	unsigned int 	linkid;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> VirtualIF_open\n", RTMP_OS_NETDEV_GET_DEVNAME(dev)));


	pAd = RTMP_OS_NETDEV_GET_PRIV(dev);
	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	// increase MODULE use count
	RT_MOD_INC_USE_COUNT();
	
	RTMP_OS_NETDEV_START_QUEUE(dev);
	return 0;
}


INT WdsVirtualIF_close(
	IN PNET_DEV dev)
{
	PRTMP_ADAPTER	pAd;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> VirtualIF_close\n", RTMP_OS_NETDEV_GET_DEVNAME(dev)));


	pAd = RTMP_OS_NETDEV_GET_PRIV(dev);

	RTMP_OS_NETDEV_CARRIER_OFF(pAd->net_dev);
	RTMP_OS_NETDEV_STOP_QUEUE(dev);
	
	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0;
}


INT WdsVirtualIF_ioctl(
	IN PNET_DEV net_dev, 
	IN OUT struct ifreq *rq, 
	IN INT cmd)
{
	RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev); //RTMP_OS_NETDEV_GET_PRIV(pVirtualAd->RtmpDev);
	
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("VirtualIF_ioctl(%s)::Network is down!\n", RTMP_OS_NETDEV_GET_DEVNAME(net_dev)));
		return -ENETDOWN;
	}

	return rt28xx_ioctl(net_dev, rq, cmd);
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
static struct net_device_stats *RT28xx_get_wds_ether_stats(
    IN  struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = NULL;
	INT WDS_apidx = 0,index;

	if (net_dev) {
		GET_PAD_FROM_NET_DEV(pAd, net_dev);
	}

	if (net_dev->priv_flags == INT_WDS)
	{
		if (pAd)
		{
			//struct net_device_stats	stats;
			for(index = 0; index < MAX_WDS_ENTRY; index++)
			{
				if (pAd->WdsTab.WdsEntry[index].dev == net_dev)
				{
					WDS_apidx = index;

					break;
				}
				
				if(index == MAX_WDS_ENTRY)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl can not find wds I/F\n"));
					return NULL;
				}
			}

			pAd->stats.rx_packets = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.ReceivedFragmentCount.QuadPart;
			pAd->stats.tx_packets = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TransmittedFragmentCount.QuadPart;

			pAd->stats.rx_bytes = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.ReceivedByteCount;
			pAd->stats.tx_bytes = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TransmittedByteCount;

			pAd->stats.rx_errors = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxErrors;
			pAd->stats.tx_errors = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TxErrors;

			pAd->stats.rx_dropped = 0;
			pAd->stats.tx_dropped = 0;

	  		pAd->stats.multicast = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.MulticastReceivedFrameCount.QuadPart;   // multicast packets received
	  		pAd->stats.collisions = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.OneCollision + pAd->WdsTab.WdsEntry[index].WdsCounter.MoreCollisions;  // Collision packets
	  
	  		pAd->stats.rx_length_errors = 0;
	  		pAd->stats.rx_over_errors = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxNoBuffer;                   // receiver ring buff overflow
	  		pAd->stats.rx_crc_errors = 0;//pAd->WlanCounters.FCSErrorCount;     // recved pkt with crc error
	  		pAd->stats.rx_frame_errors = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RcvAlignmentErrors;          // recv'd frame alignment error
	  		pAd->stats.rx_fifo_errors = pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxNoBuffer;                   // recv'r fifo overrun
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
	else

    		return NULL;
}

#endif // WDS_SUPPORT //
