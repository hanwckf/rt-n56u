
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
#define RTMP_MODULE_OS

#ifdef WDS_SUPPORT

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

struct rtnl_link_stats64 *
RT28xx_get_wds_ether_stats64(PNET_DEV net_dev, struct rtnl_link_stats64 *stats);

/* Register WDS interface */
VOID RT28xx_WDS_Init(
	IN VOID				*pAd,
	IN PNET_DEV			net_dev)
{

	RTMP_OS_NETDEV_OP_HOOK	netDevOpHook;

	NdisZeroMemory((PUCHAR)&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = WdsVirtualIF_open;
	netDevOpHook.stop = WdsVirtualIF_close;
	netDevOpHook.xmit = WdsVirtualIFSendPackets;
	netDevOpHook.ioctl = WdsVirtualIF_ioctl;
	netDevOpHook.get_stats = RT28xx_get_wds_ether_stats64;
	NdisMoveMemory(&netDevOpHook.devAddr[0], RTMP_OS_NETDEV_GET_PHYADDR(net_dev), MAC_ADDR_LEN);
	DBGPRINT(RT_DEBUG_TRACE, ("The new WDS interface MAC = %02X:%02X:%02X:%02X:%02X:%02X\n", 
				PRINT_MAC(netDevOpHook.devAddr)));

	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_INIT,
						0, &netDevOpHook, 0);
}


INT WdsVirtualIFSendPackets(
	IN PNDIS_PACKET			pPktSrc, 
	IN PNET_DEV				pDev)
{


	MEM_DBG_PKT_ALLOC_INC(pPktSrc);

	if(!(RTMP_OS_NETDEV_STATE_RUNNING(pDev)))
	{
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, pPktSrc, NDIS_STATUS_FAILURE);
		return 0;
	} /* End of if */

	return WDS_PacketSend(pPktSrc, pDev, rt28xx_packet_xmit);
}


INT WdsVirtualIF_open(
	IN	PNET_DEV dev)
{
	VOID			*pAd;
#ifdef RTL865X_SOC
	INT				index;
	unsigned int 	linkid;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> VirtualIF_open\n", RTMP_OS_NETDEV_GET_DEVNAME(dev)));


	pAd = RTMP_OS_NETDEV_GET_PRIV(dev);
	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	
	RTMP_OS_NETDEV_START_QUEUE(dev);
	return 0;
}


INT WdsVirtualIF_close(
	IN PNET_DEV dev)
{
	VOID		*pAd;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> VirtualIF_close\n", RTMP_OS_NETDEV_GET_DEVNAME(dev)));


	pAd = RTMP_OS_NETDEV_GET_PRIV(dev);

	/*RTMP_OS_NETDEV_CARRIER_OFF(dev);*/
	RTMP_OS_NETDEV_STOP_QUEUE(dev);
	
	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0;
}


INT WdsVirtualIF_ioctl(
	IN PNET_DEV net_dev, 
	IN OUT VOID *rq, 
	IN INT cmd)
{
	VOID *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev); /*RTMP_OS_NETDEV_GET_PRIV(pVirtualAd->RtmpDev); */
	
/*	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE)) */
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("VirtualIF_ioctl(%s)::Network is down!\n", RTMP_OS_NETDEV_GET_DEVNAME(net_dev)));
		return -ENETDOWN;
	}

	return rt28xx_ioctl(net_dev, rq, cmd);
}


VOID RT28xx_WDS_Remove(
	IN VOID				*pAd)
{

	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_REMOVE, 0, NULL, 0);
}

#endif /* WDS_SUPPORT */
