/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_apcli.c

    Abstract:
    Support AP-Client function.

    Note:
    1. Call RT28xx_ApCli_Init() in init function and
       call RT28xx_ApCli_Remove() in close function

    2. MAC of ApCli-interface is initialized in RT28xx_ApCli_Init()

    3. ApCli index (0) of different rx packet is got in
       APHandleRxDoneInterrupt() by using FromWhichBSSID = pEntry->apidx;
       Or FromWhichBSSID = BSS0;

    4. ApCli index (0) of different tx packet is assigned in
       MBSS_VirtualIF_PacketSend() by using RTMP_SET_PACKET_NET_DEVICE_MBSSID()
    5. ApCli index (0) of different interface is got in APHardTransmit() by using
       RTMP_GET_PACKET_IF()

    6. ApCli index (0) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    8. The number of ApCli only can be 1

	9. apcli convert engine subroutines, we should just take care data packet.
    Revision History:
    Who             When            What
    --------------  ----------      ----------------------------------------------
    Shiang, Fonchi  02-13-2007      created
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"


/*
========================================================================
Routine Description:
    Init AP-Client function.

Arguments:
    ad_p            points to our adapter
    main_dev_p      points to the main BSS network interface

Return Value:
    None

Note:
	1. Only create and initialize virtual network interfaces.
	2. No main network interface here.
========================================================================
*/
VOID RT28xx_ApCli_Init(
	IN PRTMP_ADAPTER 		ad_p,
	IN PNET_DEV				main_dev_p)
{
	PNET_DEV	new_dev_p;
//	VIRTUAL_ADAPTER *apcli_ad_p;
	INT apcli_index;
	RTMP_OS_NETDEV_OP_HOOK	netDevOpHook;
	APCLI_STRUCT	*pApCliEntry;
	
	/* sanity check to avoid redundant virtual interfaces are created */
	if (ad_p->flg_apcli_init != FALSE)
		return;


	/* init */
	for(apcli_index = 0; apcli_index < MAX_APCLI_NUM; apcli_index++)
		ad_p->ApCfg.ApCliTab[apcli_index].dev = NULL;

	/* create virtual network interface */
	for(apcli_index = 0; apcli_index < MAX_APCLI_NUM; apcli_index++)
	{
		new_dev_p = RtmpOSNetDevCreate(ad_p, INT_APCLI, apcli_index, sizeof(PRTMP_ADAPTER), INF_APCLI_DEV_NAME);
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, ad_p);

		pApCliEntry = &ad_p->ApCfg.ApCliTab[apcli_index];
		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(pApCliEntry->CurrentAddress, ad_p->CurrentAddress);

#ifdef NEW_MBSSID_MODE
		if (ad_p->ApCfg.BssidNum > 0 || MAX_MESH_NUM > 0) 
		{
			/* 	
				Refer to HW definition - 
					Bit1 of MAC address Byte0 is local administration bit 
					and should be set to 1 in extended multiple BSSIDs'
					Bit3~ of MAC address Byte0 is extended multiple BSSID index.
			 */ 
			pApCliEntry->CurrentAddress[0] += 2; 	
			pApCliEntry->CurrentAddress[0] += (((ad_p->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
		}
#else
		pApCliEntry->CurrentAddress[ETH_LENGTH_OF_ADDRESS - 1] =
			(pApCliEntry->CurrentAddress[ETH_LENGTH_OF_ADDRESS - 1] + ad_p->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
#endif // NEW_MBSSID_MODE //

		/* init operation functions */
		NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
		netDevOpHook.open = ApCli_VirtualIF_Open;
		netDevOpHook.stop = ApCli_VirtualIF_Close;
		netDevOpHook.xmit = ApCli_VirtualIF_PacketSend;
		netDevOpHook.ioctl = ApCli_VirtualIF_Ioctl;
		netDevOpHook.priv_flags = INT_APCLI; /* we are virtual interface */
		netDevOpHook.needProtcted = TRUE;
		NdisMoveMemory(&netDevOpHook.devAddr[0], &pApCliEntry->CurrentAddress[0], MAC_ADDR_LEN);
		
		/* register this device to OS */
		RtmpOSNetDevAttach(new_dev_p, &netDevOpHook);

		/* backup our virtual network interface */
		pApCliEntry->dev = new_dev_p;
        
#ifdef WSC_AP_SUPPORT
		pApCliEntry->WscControl.pAd = ad_p;        
		if (pApCliEntry->WscControl.WscEnrolleePinCode == 0)
			pApCliEntry->WscControl.WscEnrolleePinCode = GenerateWpsPinCode(ad_p, TRUE, 0);
		NdisZeroMemory(pApCliEntry->WscControl.EntryAddr, MAC_ADDR_LEN);
		pApCliEntry->WscControl.WscConfigMethods= 0x008C;
		WscGenerateUUID(ad_p, &pApCliEntry->WscControl.Wsc_Uuid_E[0], &pApCliEntry->WscControl.Wsc_Uuid_Str[0], 0, FALSE);
        WscInit(ad_p, TRUE, apcli_index);
#endif // WSC_AP_SUPPORT //
	} /* End of for */

	ad_p->flg_apcli_init = TRUE;

}

/*
========================================================================
Routine Description:
    Open a virtual network interface.

Arguments:
    dev_p           which WLAN network interface

Return Value:
    0: open successfully
    otherwise: open fail

Note:
========================================================================
*/
INT ApCli_VirtualIF_Open(
	IN PNET_DEV		dev_p)
{
	UCHAR ifIndex;
	PRTMP_ADAPTER pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	// increase MODULE use count
	RT_MOD_INC_USE_COUNT();

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].dev == dev_p)
		{
			RTMP_OS_NETDEV_START_QUEUE(dev_p);
			ApCliIfUp(pAd);
		}
	}

	return 0;
} /* End of ApCli_VirtualIF_Open */


/*
========================================================================
Routine Description:
    Close a virtual network interface.

Arguments:
    dev_p           which WLAN network interface

Return Value:
    0: close successfully
    otherwise: close fail

Note:
========================================================================
*/
INT ApCli_VirtualIF_Close(
	IN	PNET_DEV	dev_p)
{
	UCHAR ifIndex;
	PRTMP_ADAPTER pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].dev == dev_p)
		{
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
			
			// send disconnect-req to sta State Machine.
			if (pAd->ApCfg.ApCliTab[ifIndex].Enable)
			{
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
			}
			break;
		}
	}

	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0;
} /* End of ApCli_VirtualIF_Close */


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
INT ApCli_VirtualIF_PacketSend(
	IN PNDIS_PACKET 	skb_p, 
	IN PNET_DEV			dev_p)
{
	RTMP_ADAPTER *ad_p;
	PAPCLI_STRUCT pApCli;
	INT apcliIndex;



	ad_p = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(ad_p);

#ifdef RALINK_ATE
	if (ATE_ON(ad_p))
	{
		RELEASE_NDIS_PACKET(ad_p, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}
#endif // RALINK_ATE //

	if ((RTMP_TEST_FLAG(ad_p, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
		(RTMP_TEST_FLAG(ad_p, fRTMP_ADAPTER_RADIO_OFF))          ||
		(RTMP_TEST_FLAG(ad_p, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
	{
		/* wlan is scanning/disabled/reset */
		RELEASE_NDIS_PACKET(ad_p, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}

	if (!(RTMP_OS_NETDEV_STATE_RUNNING(dev_p)))
	{
		/* the interface is down */
		RELEASE_NDIS_PACKET(ad_p, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}


	pApCli = (PAPCLI_STRUCT)&ad_p->ApCfg.ApCliTab;

	for(apcliIndex = 0; apcliIndex < MAX_APCLI_NUM; apcliIndex++)
	{
		if (pApCli[apcliIndex].Valid != TRUE)
			continue;

		/* find the device in our ApCli list */
		if (pApCli[apcliIndex].dev == dev_p)
		{
			/* ya! find it */
			ad_p->RalinkCounters.PendingNdisPacketCount ++;
			RTMP_SET_PACKET_MOREDATA(skb_p, FALSE);
			RTMP_SET_PACKET_NET_DEVICE_APCLI(skb_p, apcliIndex);
			SET_OS_PKT_NETDEV(skb_p, ad_p->net_dev);

			/* transmit the packet */
			return rt28xx_packet_xmit(RTPKT_TO_OSPKT(skb_p));
		}
    }

	RELEASE_NDIS_PACKET(ad_p, skb_p, NDIS_STATUS_FAILURE);

	return 0;
} /* End of ApCli_VirtualIF_PacketSend */


/*
========================================================================
Routine Description:
    IOCTL to WLAN.

Arguments:
    dev_p           which WLAN network interface
    rq_p            command information
    cmd             command ID

Return Value:
    0: IOCTL successfully
    otherwise: IOCTL fail

Note:
    SIOCETHTOOL     8946    New drivers use this ETHTOOL interface to
                            report link failure activity.
========================================================================
*/
INT ApCli_VirtualIF_Ioctl(
	IN PNET_DEV				dev_p, 
	IN OUT struct ifreq 	*rq_p, 
	IN INT 					cmd)
{
	RTMP_ADAPTER *ad_p;

	ad_p = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(ad_p);

	if (!RTMP_TEST_FLAG(ad_p, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	/* do real IOCTL */
	return (rt28xx_ioctl(dev_p, rq_p, cmd));
} /* End of ApCli_VirtualIF_Ioctl */

#endif // APCLI_SUPPORT //
