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
    rbus_main_dev.c

    Abstract:
    Create and register network interface for RBUS based chipsets in linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include "rt_config.h"


static struct net_device *rt2880_dev = NULL;


VOID __exit rt2880_module_exit(VOID);
int __init rt2880_module_init(VOID);


module_init(rt2880_module_init);
module_exit(rt2880_module_exit);

int rt2880_module_init(VOID)
{
	struct  net_device		*net_dev = NULL;
	ULONG				csr_addr;
	INT					rv;
	PVOID				*handle = NULL;
	RTMP_ADAPTER			*pAd = NULL;
	unsigned int			dev_irq;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook;
	

	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2880_probe\n"));	


//RtmpRaBusInit============================================
	// map physical address to virtual address for accessing register
	csr_addr = (unsigned long)RTMP_MAC_CSR_ADDR;
	dev_irq = RTMP_MAC_IRQ_NUM;
	

//RtmpDevInit==============================================
	// Allocate RTMP_ADAPTER adapter structure
	handle = kmalloc(sizeof(struct os_cookie) , GFP_KERNEL);
	if (!handle)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory for os_cookie failed!\n"));
		goto err_out;
	}

	NdisZeroMemory(handle, sizeof(struct os_cookie));

	rv = RTMPAllocAdapterBlock(handle, &pAd);
	if (rv != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" RTMPAllocAdapterBlock !=  NDIS_STATUS_SUCCESS\n"));
		kfree(handle);
		
		goto err_out;
	}
	// Here are the RTMP_ADAPTER structure with rbus-bus specific parameters.
	pAd->CSRBaseAddress = (PUCHAR)csr_addr;
	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_RBUS);


//NetDevInit==============================================
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;

	// Here are the net_device structure with pci-bus specific parameters.
	net_dev->irq = dev_irq;			// Interrupt IRQ number
	net_dev->base_addr = csr_addr;		// Save CSR virtual address and irq to device structure
	((POS_COOKIE)handle)->pci_dev = NULL;

#ifdef CONFIG_STA_SUPPORT
    pAd->StaCfg.OriDevType = net_dev->type;
#endif // CONFIG_STA_SUPPORT //


	
//All done, it's time to register the net device to kernel.
	// Register this device
	rv = RtmpOSNetDevAttach(net_dev, &netDevHook);
	if (rv)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("failed to call RtmpOSNetDevAttach(), rv=%d!\n", rv));
		goto err_out_free_netdev;
	}

	// due to we didn't have any hook point when do module remove, we use this static as our hook point.
	rt2880_dev = net_dev;

	wl_proc_init();

	DBGPRINT(RT_DEBUG_TRACE, ("%s: at CSR addr 0x%1lx, IRQ %d. \n", net_dev->name, (ULONG)csr_addr, net_dev->irq));

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2880_probe\n"));

	return 0;

err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);

err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);
		
err_out:
	return -ENODEV;
	
}


VOID rt2880_module_exit(VOID)
{
	struct net_device   *net_dev = rt2880_dev;
	RTMP_ADAPTER *pAd;


	if (net_dev == NULL)
		return;
	
	pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	if (pAd != NULL)
	{
#ifdef WLAN_LED
	//extern RALINK_TIMER_STRUCT LedCheckTimer;
	//extern unsigned char CheckTimerEbl;
	{
		BOOLEAN  Cancelled;
		RTMPCancelTimer(&LedCheckTimer, &Cancelled);
		CheckTimerEbl=0;
	}
#endif // WLAN_LED //

		RtmpPhyNetDevExit(pAd, net_dev);
		RtmpRaDevCtrlExit(pAd);
	}
	else
	{
		RtmpOSNetDevDetach(net_dev);
	}
	
	// Free the root net_device.
	RtmpOSNetDevFree(net_dev);
	
#if defined(CONFIG_RA_CLASSIFIER)&&(!defined(CONFIG_RA_CLASSIFIER_MODULE)) 	 
	proc_ptr = proc_ralink_wl_video; 	 
	if(ra_classifier_release_func!=NULL) 	 
		ra_classifier_release_func(); 	 
#endif

	wl_proc_exit();
}

MODULE_LICENSE("Proprietary");
