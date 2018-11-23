#ifdef MTK_LICENSE
/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/
#endif /* MTK_LICENSE */
#define RTMP_MODULE_OS

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"


extern USB_DEVICE_ID rtusb_dev_id[];
extern INT const rtusb_usb_id_len;

#ifdef MEM_ALLOC_INFO_SUPPORT
extern MEM_INFO_LIST MemInfoList;
extern MEM_INFO_LIST PktInfoList;
#endif /*MEM_ALLOC_INFO_SUPPORT*/

static BOOLEAN USBDevConfigInit(struct usb_device *dev, struct usb_interface *intf, VOID *pAd);

#ifndef PF_NOFREEZE
#define PF_NOFREEZE  0
#endif

#ifdef PROFILE_PATH_DYNAMIC
static char *profilePath = STA_PROFILE_PATH;
module_param(profilePath, charp, S_IRUGO);
#endif /* PROFILE_PATH_DYNAMIC */

#ifdef OS_ABL_SUPPORT
/* USB complete handlers in LINUX */
RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkOutDataPacketComplete = NULL;
RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkOutMLMEPacketComplete = NULL;
RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkOutNullFrameComplete = NULL;
RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkOutRTSFrameComplete = NULL;
RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkOutPsPollComplete = NULL;
RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkRxComplete = NULL;
//RTMP_DRV_USB_COMPLETE_HANDLER RtmpDrvUsbBulkCmdRspEventComplete = NULL;

USBHST_STATUS RTUSBBulkOutDataPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkOutDataPacketComplete((VOID *)pURB);
}

USBHST_STATUS RTUSBBulkOutMLMEPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkOutMLMEPacketComplete((VOID *)pURB);
}

USBHST_STATUS RTUSBBulkOutNullFrameComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkOutNullFrameComplete((VOID *)pURB);
}

USBHST_STATUS RTUSBBulkOutRTSFrameComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkOutRTSFrameComplete((VOID *)pURB);
}

USBHST_STATUS RTUSBBulkOutPsPollComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkOutPsPollComplete((VOID *)pURB);
}

USBHST_STATUS RTUSBBulkRxComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkRxComplete((VOID *)pURB);
}

#ifdef MT_MAC
USBHST_STATUS RTUSBBulkOutBCNPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkOutBCNPacketComplete((VOID *)pURB);
}
#endif

#ifndef RTMP_ANDES_JAY
USBHST_STATUS RTUSBBulkCmdRspEventComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	RtmpDrvUsbBulkCmdRspEventComplete((VOID *)pURB);
}
#endif


VOID RtmpNetOpsInit(VOID *pDrvNetOpsSrc)
{
	RTMP_NET_ABL_OPS *pDrvNetOps = (RTMP_NET_ABL_OPS *)pDrvNetOpsSrc;


	pDrvNetOps->RtmpNetUsbBulkOutDataPacketComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkOutDataPacketComplete;
	pDrvNetOps->RtmpNetUsbBulkOutMLMEPacketComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkOutMLMEPacketComplete;
	pDrvNetOps->RtmpNetUsbBulkOutNullFrameComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkOutNullFrameComplete;
	pDrvNetOps->RtmpNetUsbBulkOutRTSFrameComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkOutRTSFrameComplete;
	pDrvNetOps->RtmpNetUsbBulkOutPsPollComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkOutPsPollComplete;
	pDrvNetOps->RtmpNetUsbBulkRxComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkRxComplete;
#ifdef MT_MAC
    pDrvNetOps->RtmpNetUsbBulkOutBCNPacketComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkOutBCNPacketComplete;
#endif
#ifndef RTMP_ANDES_JAY
	pDrvNetOps->RtmpNetUsbBulkCmdRspEventComplete = (RTMP_DRV_USB_COMPLETE_HANDLER)RTUSBBulkCmdRspEventComplete;
#endif /*RTMP_ANDES_JAY */
}


VOID RtmpNetOpsSet(VOID *pDrvNetOpsSrc)
{
	RTMP_NET_ABL_OPS *pDrvNetOps = (RTMP_NET_ABL_OPS *)pDrvNetOpsSrc;


	RtmpDrvUsbBulkOutDataPacketComplete = pDrvNetOps->RtmpDrvUsbBulkOutDataPacketComplete;
	RtmpDrvUsbBulkOutMLMEPacketComplete = pDrvNetOps->RtmpDrvUsbBulkOutMLMEPacketComplete;
	RtmpDrvUsbBulkOutNullFrameComplete = pDrvNetOps->RtmpDrvUsbBulkOutNullFrameComplete;
	RtmpDrvUsbBulkOutRTSFrameComplete = pDrvNetOps->RtmpDrvUsbBulkOutRTSFrameComplete;
	RtmpDrvUsbBulkOutPsPollComplete = pDrvNetOps->RtmpDrvUsbBulkOutPsPollComplete;
	RtmpDrvUsbBulkRxComplete = pDrvNetOps->RtmpDrvUsbBulkRxComplete;
	//RtmpDrvUsbBulkCmdRspEventComplete = pDrvNetOps->RtmpDrvUsbBulkCmdRspEventComplete;
}
#endif /* OS_ABL_SUPPORT */

	
VOID rtusb_reject_pending_pkts(VOID *pAd)
{
	/* clear PS packets */
	/* clear TxSw packets */
}


VOID rtusb_vendor_specific_check(struct usb_device *dev, VOID *pAd)
{


	RT_CMD_USB_MORE_FLAG_CONFIG Config = { dev->descriptor.idVendor,
										dev->descriptor.idProduct };
	RTMP_DRIVER_USB_MORE_FLAG_SET(pAd, &Config);
}

/** 
 * @addtogroup hif HIF operation
 * @{
 * @name USB initialzation
 * @{
 */

static int rt2870_probe(
	struct usb_interface *intf,
	struct usb_device *usb_dev,
	const USB_DEVICE_ID *dev_id,
	VOID **ppAd)
{
	struct net_device *net_dev = NULL;
	VOID *pAd = (VOID *) NULL;
	INT status, rv;
	PVOID handle;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	ULONG OpMode;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("===>rt2870_probe()!\n"));
	
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));
	if (handle == NULL)
	{
		printk("rt2870_probe(): Allocate memory for os handle failed!\n");
		return -ENOMEM;
	}
	memset(handle, 0, sizeof(struct os_cookie));

	((POS_COOKIE)handle)->pUsb_Dev = usb_dev;
	((POS_COOKIE)handle)->pDev = &(usb_dev->dev);

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	((POS_COOKIE)handle)->intf = intf;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	/* set/get operators to/from DRIVER module */
#ifdef OS_ABL_FUNC_SUPPORT
	/* get DRIVER operations */
	RtmpNetOpsInit(pRtmpDrvNetOps);
	RTMP_DRV_OPS_FUNCTION(pRtmpDrvOps, pRtmpDrvNetOps, NULL, NULL);
	RtmpNetOpsSet(pRtmpDrvNetOps);
#endif /* OS_ABL_FUNC_SUPPORT */

	rv = RTMPAllocAdapterBlock(handle, &pAd);
	if (rv != NDIS_STATUS_SUCCESS) 
	{
		os_free_mem(handle);
		goto err_out;
	}

	if (USBDevConfigInit(usb_dev, intf, pAd) == FALSE)
		goto err_out_free_radev;

	RTMP_DRIVER_USB_INIT(pAd, usb_dev, dev_id->driver_info);
	
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;
	
	/* Here are the net_device structure with usb specific parameters. */

#ifdef PROFILE_PATH_DYNAMIC
	RTMP_DRIVER_STA_PROFILEPATH_SET(pAd, profilePath);
#endif /* PROFILE_PATH_DYNAMIC */

	RTMP_DRIVER_CHIP_PREPARE(pAd);

	/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	status = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);
	if (status != 0)
		goto err_out_free_netdev;

/*#ifdef KTHREAD_SUPPORT */

	*ppAd = pAd;

#ifdef INF_PPA_SUPPORT
	RTMP_DRIVER_INF_PPA_INIT(pAd);
#endif /* INF_PPA_SUPPORT */

#ifdef PRE_ASSIGN_MAC_ADDR
{
	UCHAR PermanentAddress[MAC_ADDR_LEN];
	RTMP_DRIVER_MAC_ADDR_GET(pAd, &PermanentAddress[0]);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("%s():MAC Addr - %02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, PermanentAddress[0], PermanentAddress[1],
				PermanentAddress[2],PermanentAddress[3],
				PermanentAddress[4],PermanentAddress[5]));

	/* Set up the Mac address */
	RtmpOSNetDevAddrSet(OpMode, net_dev, &PermanentAddress[0], NULL);
}
#endif /* PRE_ASSIGN_MAC_ADDR */

#ifdef EXT_BUILD_CHANNEL_LIST
	RTMP_DRIVER_SET_PRECONFIG_VALUE(pAd);
#endif /* EXT_BUILD_CHANNEL_LIST */

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("<===rt2870_probe()!\n"));

	return 0;

	/* --------------------------- ERROR HANDLE --------------------------- */	
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);
#ifdef MEM_ALLOC_INFO_SUPPORT
	{
		UINT32 memalctotal, pktalctotal;
		memalctotal = ShowMemAllocInfo();
		pktalctotal = ShowPktAllocInfo();
		if((memalctotal != 0) || (pktalctotal != 0))
		{
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error: Memory leak!!\n"));
			ASSERT(0);
		}
		MIListExit(&MemInfoList);
		MIListExit(&PktInfoList);
	}
#endif /* MEM_ALLOC_INFO_SUPPORT */

err_out_free_radev:
	RTMPFreeAdapter(pAd);
	
err_out:
	*ppAd = NULL;

	return -1;
	
}


/*
========================================================================
Routine Description:
    Release allocated resources.

Arguments:
    *dev				Point to the PCI or USB device
	pAd					driver control block pointer

Return Value:
    None

Note:
========================================================================
*/
static void rt2870_disconnect(struct usb_device *dev, VOID *pAd)
{
	struct net_device *net_dev;


	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("rtusb_disconnect: unregister usbnet usb-%s-%s\n",
				dev->bus->bus_name, dev->devpath));
	if (!pAd)
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)	/* kernel 2.4 series */
		while(MOD_IN_USE > 0)
		{
			MOD_DEC_USE_COUNT;
		}
#else
		usb_put_dev(dev);
#endif /* LINUX_VERSION_CODE */

		printk("rtusb_disconnect: pAd == NULL!\n");
		return;
	}
/*	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST); */
	RTMP_DRIVER_NIC_NOT_EXIST_SET(pAd);

	/* for debug, wait to show some messages to /proc system */
	udelay(1);


	RTMP_DRIVER_NET_DEV_GET(pAd, &net_dev);

	RtmpPhyNetDevExit(pAd, net_dev);

	/* FIXME: Shall we need following delay and flush the schedule?? */
	udelay(1);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)	/* kernel 2.4 series */
#else
	flush_scheduled_work();
#endif /* LINUX_VERSION_CODE */
	udelay(1);

	/* free the root net_device */
//	RtmpOSNetDevFree(net_dev);

	RtmpRaDevCtrlExit(pAd);

	/* free the root net_device */
	RtmpOSNetDevFree(net_dev);

	/* release a use of the usb device structure */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)	/* kernel 2.4 series */
	while(MOD_IN_USE > 0)
	{
		MOD_DEC_USE_COUNT;
	}
#else
	usb_put_dev(dev);
#endif /* LINUX_VERSION_CODE */
	udelay(1);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, (" RTUSB disconnect successfully\n"));

#ifdef MEM_ALLOC_INFO_SUPPORT
	{
		UINT32 memalctotal, pktalctotal;
		memalctotal = ShowMemAllocInfo();
		pktalctotal = ShowPktAllocInfo();
		if((memalctotal != 0) || (pktalctotal != 0))
		{
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error: Memory leak!!\n"));
			ASSERT(0);
		}
		MIListExit(&MemInfoList);
		MIListExit(&PktInfoList);
	}
#endif /* MEM_ALLOC_INFO_SUPPORT */
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
/**************************************************************************/
/* tested for kernel 2.4 series                                                                                       */
/**************************************************************************/
static BOOLEAN USBDevConfigInit(struct usb_device *dev, struct usb_interface *intf, VOID *pAd)
{
	struct usb_interface_descriptor *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	ULONG BulkOutIdx;
	UINT32 i;
	RT_CMD_USB_DEV_CONFIG Config, *pConfig = &Config;

	iface_desc = &intf->altsetting[0];

	/* get # of enpoints */
	pConfig->NumberOfPipes = iface_desc->bNumEndpoints;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("NumEndpoints=%d\n", iface_desc->bNumEndpoints));		 

	/* Configure Pipes */
	endpoint = &iface_desc->endpoint[0];
	BulkOutIdx = 0;

	for(i=0; i<pConfig->NumberOfPipes; i++)
	{
		if ((endpoint[i].bmAttributes == USB_ENDPOINT_XFER_BULK) && 
			((endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN))
		{
			pConfig->BulkInEpAddr = endpoint[i].bEndpointAddress;
			pConfig->BulkInMaxPacketSize = endpoint[i].wMaxPacketSize;

			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("BULK IN MaximumPacketSize = %d\n", pConfig->BulkInMaxPacketSize));
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("EP address = 0x%2x  \n", endpoint[i].bEndpointAddress));
		}
		else if ((endpoint[i].bmAttributes == USB_ENDPOINT_XFER_BULK) && 
				((endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT))
		{
			/* There are 6 bulk out EP. EP6 highest priority. */
			/* EP1-4 is EDCA.  EP5 is HCCA. */
			pConfig->BulkOutEpAddr[BulkOutIdx++] = endpoint[i].bEndpointAddress;
			pConfig->BulkOutMaxPacketSize = endpoint[i].wMaxPacketSize;

			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("BULK OUT MaximumPacketSize = %d\n", pConfig->BulkOutMaxPacketSize));
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("EP address = 0x%2x  \n", endpoint[i].bEndpointAddress));
		}
	}

	if (!(pConfig->BulkInEpAddr && pConfig->BulkOutEpAddr[0])) 
	{
		printk("Could not find both bulk-in and bulk-out endpoints\n");
		return FALSE;
	}

	pConfig->pConfig = dev->config;
	RTMP_DRIVER_USB_CONFIG_INIT(pAd, pConfig);
	rtusb_vendor_specific_check(dev, pAd);

	return TRUE;
	
}


static void *rtusb_probe(struct usb_device *dev, UINT inf, const USB_DEVICE_ID *id)
{
	struct usb_interface *usb_if;
	VOID *pAd;
	int rv;

	/* get the active interface descriptor */
	usb_if = &dev->actconfig->interface[inf];

	/* call generic probe procedure. */
	rv = rt2870_probe(usb_if, dev, id, &pAd);
	if (rv != 0)
		pAd = NULL;
	
	return (void *)pAd;
}


/*Disconnect function is called within exit routine */
static void rtusb_disconnect(struct usb_device *dev, void *ptr)
{
	rt2870_disconnect(dev, ptr);
}


struct usb_driver rtusb_driver = {
	.name = RTMP_DRV_NAME,
	.probe = rtusb_probe,
	.disconnect = rtusb_disconnect,
	.id_table = rtusb_dev_id,
};

#else	/* else if we are kernel 2.6 series */


/**************************************************************************/
/**************************************************************************/
/*tested for kernel 2.6series */
/**************************************************************************/
/**************************************************************************/

#ifdef CONFIG_PM
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10)
#define pm_message_t u32
#endif

static int rtusb_suspend(struct usb_interface *intf, pm_message_t state)
{
	struct net_device *net_dev;
	VOID *pAd = usb_get_intfdata(intf);

#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	UCHAR Flag;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s():=>autosuspend\n", __FUNCTION__));
	{
/*	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)) */
		RTMP_DRIVER_ADAPTER_END_DISSASSOCIATE(pAd);
		RTMP_DRIVER_ADAPTER_IDLE_RADIO_OFF_TEST(pAd, &Flag);
		if(!Flag)
		{
			/*RT28xxUsbAsicRadioOff(pAd); */
			RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_OFF(pAd);
		}
	}
	/*RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND); */
	RTMP_DRIVER_ADAPTER_SUSPEND_SET(pAd);
	return 0;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */


	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("%s()=>\n", __FUNCTION__));
/*	net_dev = pAd->net_dev; */
	RTMP_DRIVER_NET_DEV_GET(pAd, &net_dev);
	netif_device_detach(net_dev);

	RTMP_DRIVER_USB_SUSPEND(pAd, netif_running(net_dev));

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("<=%s()\n", __FUNCTION__));
	return 0;
}


static int rtusb_resume(struct usb_interface *intf)
{
	struct net_device *net_dev;
	VOID *pAd = usb_get_intfdata(intf);

#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	INT 		pm_usage_cnt;
	UCHAR Flag;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	pm_usage_cnt = atomic_read(&intf->pm_usage_cnt);	
#else
	pm_usage_cnt = intf->pm_usage_cnt;
#endif

	if(pm_usage_cnt  <= 0)
		usb_autopm_get_interface(intf);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s():=>autosuspend\n", __FUNCTION__));

	/*RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND); */
	RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(pAd);

		/*RT28xxUsbAsicRadioOn(pAd); */
		RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_ON(pAd);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s(): <=autosuspend\n", __FUNCTION__));

	return 0;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */


	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("%s()=>\n", __FUNCTION__));

/*	pAd->PM_FlgSuspend = 0; */
	RTMP_DRIVER_USB_RESUME(pAd);

/*	net_dev = pAd->net_dev; */
	RTMP_DRIVER_NET_DEV_GET(pAd, &net_dev);
	netif_device_attach(net_dev);


	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("<=%s()\n", __FUNCTION__));
	return 0;
}
#endif /* CONFIG_PM */


static BOOLEAN USBDevConfigInit(struct usb_device *dev, struct usb_interface *intf, VOID *pAd)
{
	struct usb_host_interface *iface_desc;
	ULONG BulkOutIdx;
	ULONG BulkInIdx;
	UINT32 i;
	RT_CMD_USB_DEV_CONFIG Config, *pConfig = &Config;

	/* get the active interface descriptor */
	iface_desc = intf->cur_altsetting;

	/* get # of enpoints  */
	pConfig->NumberOfPipes = iface_desc->desc.bNumEndpoints;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("NumEndpoints=%d\n", iface_desc->desc.bNumEndpoints));		  

	/* Configure Pipes */
	BulkOutIdx = 0;
	BulkInIdx = 0;

	for (i = 0; i < pConfig->NumberOfPipes; i++)
	{ 
		if ((iface_desc->endpoint[i].desc.bmAttributes == USB_ENDPOINT_XFER_BULK) && 
			((iface_desc->endpoint[i].desc.bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN))
		{
			if (BulkInIdx < 2)
			{
				pConfig->BulkInEpAddr[BulkInIdx++] = iface_desc->endpoint[i].desc.bEndpointAddress;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
				pConfig->BulkInMaxPacketSize = le2cpu16(iface_desc->endpoint[i].desc.wMaxPacketSize);
#else
				pConfig->BulkInMaxPacketSize = iface_desc->endpoint[i].desc.wMaxPacketSize;
#endif /* LINUX_VERSION_CODE */

				MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("BULK IN MaxPacketSize = %d\n", pConfig->BulkInMaxPacketSize));
				MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("EP address = 0x%2x\n", iface_desc->endpoint[i].desc.bEndpointAddress));
				} 
				else
				{
					MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Bulk IN endpoint nums large than 2\n"));
				}
			}
			else if ((iface_desc->endpoint[i].desc.bmAttributes == USB_ENDPOINT_XFER_BULK) && 
					((iface_desc->endpoint[i].desc.bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT))
			{
				if (BulkOutIdx < 6)
				{
				/* there are 6 bulk out EP. EP6 highest priority. */
				/* EP1-4 is EDCA.  EP5 is HCCA. */
				pConfig->BulkOutEpAddr[BulkOutIdx++] = iface_desc->endpoint[i].desc.bEndpointAddress;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
				pConfig->BulkOutMaxPacketSize = le2cpu16(iface_desc->endpoint[i].desc.wMaxPacketSize);
#else
				pConfig->BulkOutMaxPacketSize = iface_desc->endpoint[i].desc.wMaxPacketSize;
#endif

				MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("BULK OUT MaxPacketSize = %d\n", pConfig->BulkOutMaxPacketSize));
				MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE, ("EP address = 0x%2x  \n", iface_desc->endpoint[i].desc.bEndpointAddress));
			}
			else
			{
				MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Bulk Out endpoint nums large than 6\n"));
			}
		}
	}

	if (!(pConfig->BulkInEpAddr && pConfig->BulkOutEpAddr[0])) 
	{
		printk("%s: Could not find both bulk-in and bulk-out endpoints\n", __FUNCTION__);
		return FALSE;
	}

	pConfig->pConfig = &dev->config->desc;
	usb_set_intfdata(intf, pAd);
	RTMP_DRIVER_USB_CONFIG_INIT(pAd, pConfig);
	rtusb_vendor_specific_check(dev, pAd);    
	
	return TRUE;
	
}


static int rtusb_probe(struct usb_interface *intf, const USB_DEVICE_ID *id)
{	
	VOID *pAd;
	struct usb_device *dev;
	int rv;

	dev = interface_to_usbdev(intf);
	dev = usb_get_dev(dev);
	
	rv = rt2870_probe(intf, dev, id, &pAd);
	if (rv != 0)
		usb_put_dev(dev);
#ifdef IFUP_IN_PROBE
	else
	{
		if (VIRTUAL_IF_UP(pAd) != 0)
		{
			pAd = usb_get_intfdata(intf);
			usb_set_intfdata(intf, NULL);
			rt2870_disconnect(dev, pAd);
			rv = -ENOMEM;
		}
	}
#endif /* IFUP_IN_PROBE */	
	return rv;
}


static void rtusb_disconnect(struct usb_interface *intf)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	VOID *pAd;

	pAd = usb_get_intfdata(intf);
#ifdef IFUP_IN_PROBE	
	VIRTUAL_IF_DOWN(pAd);
#endif /* IFUP_IN_PROBE */	
	usb_set_intfdata(intf, NULL);	

	rt2870_disconnect(dev, pAd);

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	printk("rtusb_disconnect usb_autopm_put_interface \n");
	usb_autopm_put_interface(intf);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)	 
	printk("%s() => pm_usage_cnt %d \n", __FUNCTION__,
			atomic_read(&intf->pm_usage_cnt));
#else
	printk("%s() => pm_usage_cnt %d \n", __FUNCTION__, intf->pm_usage_cnt);
#endif
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
	
}


struct usb_driver rtusb_driver = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
	.owner = THIS_MODULE,
#endif	
	.name = RTMP_DRV_NAME,
	.probe = rtusb_probe,
	.disconnect = rtusb_disconnect,
	.id_table = rtusb_dev_id,

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	.supports_autosuspend = 1,
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
	.suspend = rtusb_suspend,
	.resume = rtusb_resume,
#endif /* CONFIG_PM */
};
#endif /* LINUX_VERSION_CODE */


INT __init rtusb_init(void)
{
	printk("rtusb init %s --->\n", RTMP_DRV_NAME);
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	/*add for initial hook callback function linking list*/
	RTMP_OS_TXRXHOOK_INIT();
	return usb_register(&rtusb_driver);
}


VOID __exit rtusb_exit(void)
{
	usb_deregister(&rtusb_driver);	
	printk("<--- rtusb exit\n");
}

/** @} */
/** @} */
#ifndef MULTI_INF_SUPPORT

module_init(rtusb_init);
module_exit(rtusb_exit);

/* Following information will be show when you run 'modinfo' */
/* *** If you have a solution for the bug in current version of driver, please mail to me. */
/* Otherwise post to forum in ralinktech's web site(www.ralinktech.com) and let all users help you. *** */
MODULE_AUTHOR("Ralink");
MODULE_DESCRIPTION("Ralink Wireless Lan Linux Driver");
MODULE_LICENSE("GPL");


#endif /* MULTI_INF_SUPPORT */

