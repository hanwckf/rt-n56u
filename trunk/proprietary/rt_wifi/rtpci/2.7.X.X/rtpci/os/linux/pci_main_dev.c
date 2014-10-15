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
    pci_main_dev.c

    Abstract:
    Create and register network interface for PCI based chipsets in Linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#define RTMP_MODULE_OS

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
#define __devinit
#define __devinitdata
#define __devexit
#define __devexit_p
#endif

/* */
/* Function declarations */
/* */
/*extern int rt28xx_close(IN struct net_device *net_dev); */
/*extern int rt28xx_open(struct net_device *net_dev); */

static VOID __devexit rt2860_remove_one(struct pci_dev *pci_dev);
static INT __devinit rt2860_probe(struct pci_dev *pci_dev, const struct pci_device_id  *ent);
static void __exit rt2860_cleanup_module(void);
static int __init rt2860_init_module(void);


#ifdef CONFIG_PM
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10)
#define pm_message_t u32
#endif

static int rt2860_suspend(struct pci_dev *pci_dev, pm_message_t state);
static int rt2860_resume(struct pci_dev *pci_dev);
#endif
#endif /* CONFIG_PM */

/* */
/* Ralink PCI device table, include all supported chipsets */
/* */
static struct pci_device_id rt2860_pci_tbl[] __devinitdata =
{
	/* Do not remove this "default" device ID for bringing up empty efuse ! */
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3091_PCIe_DEVICE_ID)},
#ifdef RT3090
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3090_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3091_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3092_PCIe_DEVICE_ID)},
#endif /* RT3090 */
#ifdef RT35xx
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3062_PCI_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3562_PCI_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3060_PCI_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3592_PCIe_DEVICE_ID)},
	{PCI_DEVICE(EDIMAX_PCI_VENDOR_ID, 0x7711)},
	{PCI_DEVICE(EDIMAX_PCI_VENDOR_ID, 0x7722)},
#endif /* RT35xx */
#ifdef RT3390
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3390_PCIe_DEVICE_ID)},
#endif /* RT3390 */
#ifdef RT3593
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3593_PCI_OR_PCIe_DEVICE_ID)},
#endif /* RT3593 */
#ifdef RT5390
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5390_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC539F_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5392_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5362_PCI_DEVICE_ID)},
	{PCI_DEVICE(DLINK_PCI_VENDOR_ID, 0x3C05)},
#endif /* RT5390 */
#ifdef RT5592
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5592_PCIe_DEVICE_ID)},
#endif /* RT5592 */
    {0,}		/* terminate list */
};

MODULE_DEVICE_TABLE(pci, rt2860_pci_tbl);


/* */
/* Our PCI driver structure */
/* */
static struct pci_driver rt2860_driver =
{
    name:       RTMP_DRV_NAME,
    id_table:   rt2860_pci_tbl,
    probe:      rt2860_probe,
#if LINUX_VERSION_CODE >= 0x20412
    remove:     __devexit_p(rt2860_remove_one),
#else
    remove:     __devexit(rt2860_remove_one),
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#ifdef CONFIG_PM
	suspend:	rt2860_suspend,
	resume:		rt2860_resume,
#endif
#endif
};


/***************************************************************************
 *
 *	PCI device initialization related procedures.
 *
 ***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#ifdef CONFIG_PM

VOID RT2860RejectPendingPackets(
	IN	VOID	*pAd)
{
	/* clear PS packets */
	/* clear TxSw packets */
}

static int rt2860_suspend(
	struct pci_dev *pci_dev,
	pm_message_t state)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;
	INT32 retval = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_suspend()\n"));

	if (net_dev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("net_dev == NULL!\n"));
	}
	else
	{
		ULONG IfNum;

		GET_PAD_FROM_NET_DEV(pAd, net_dev);

		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);
		if (IfNum > 0)
		{
			/* avoid users do suspend after interface is down */

			/* stop interface */
			netif_carrier_off(net_dev);
			netif_stop_queue(net_dev);

			/* mark device as removed from system and therefore no longer available */
			netif_device_detach(net_dev);

			/* mark halt flag */
/*			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
/*			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF); */
			RTMP_DRIVER_PCI_SUSPEND(pAd);

			/* take down the device */
			rt28xx_close((PNET_DEV)net_dev);

			RT_MOD_DEC_USE_COUNT();
		}
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	/* reference to http://vovo2000.com/type-lab/linux/kernel-api/linux-kernel-api.html */
	/* enable device to generate PME# when suspended */
	/* pci_choose_state(): Choose the power state of a PCI device to be suspended */
	retval = pci_enable_wake(pci_dev, pci_choose_state(pci_dev, state), 1);
	/* save the PCI configuration space of a device before suspending */
	pci_save_state(pci_dev);
	/* disable PCI device after use */
	pci_disable_device(pci_dev);

	retval = pci_set_power_state(pci_dev, pci_choose_state(pci_dev, state));
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_suspend()\n"));
	return retval;
}

static int rt2860_resume(
	struct pci_dev *pci_dev)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	INT32 retval;


	/* set the power state of a PCI device */
	/* PCI has 4 power states, DO (normal) ~ D3(less power) */
	/* in include/linux/pci.h, you can find that */
	/* #define PCI_D0          ((pci_power_t __force) 0) */
	/* #define PCI_D1          ((pci_power_t __force) 1) */
	/* #define PCI_D2          ((pci_power_t __force) 2) */
	/* #define PCI_D3hot       ((pci_power_t __force) 3) */
	/* #define PCI_D3cold      ((pci_power_t __force) 4) */
	/* #define PCI_UNKNOWN     ((pci_power_t __force) 5) */
	/* #define PCI_POWER_ERROR ((pci_power_t __force) -1) */
	retval = pci_set_power_state(pci_dev, PCI_D0);

	/* restore the saved state of a PCI device */
	pci_restore_state(pci_dev);

	/* initialize device before it's used by a driver */
	if (pci_enable_device(pci_dev))
	{
		printk("pci enable fail!\n");
		return 0;
	}
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_resume()\n"));

	if (net_dev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("net_dev == NULL!\n"));
	}
	else
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL)
	{
		ULONG IfNum;

		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);
		if (IfNum > 0)
/*		if (VIRTUAL_IF_NUM(pAd) > 0) */
		{
			/* mark device as attached from system and restart if needed */
			netif_device_attach(net_dev);

			if (rt28xx_open((PNET_DEV)net_dev) != 0)
			{
				/* open fail */
				DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_resume()\n"));
				return 0;
			}

			/* increase MODULE use count */
			RT_MOD_INC_USE_COUNT();

/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF); */
			RTMP_DRIVER_PCI_RESUME(pAd);

			netif_start_queue(net_dev);
			netif_carrier_on(net_dev);
			netif_wake_queue(net_dev);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_resume()\n"));
	return 0;
}
#endif /* CONFIG_PM */
#endif


static INT __init rt2860_init_module(VOID)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	return pci_register_driver(&rt2860_driver);
#else
	return pci_module_init(&rt2860_driver);
#endif
}


/* */
/* Driver module unload function */
/* */
static VOID __exit rt2860_cleanup_module(VOID)
{
    pci_unregister_driver(&rt2860_driver);
}

module_init(rt2860_init_module);
module_exit(rt2860_cleanup_module);


/* */
/* PCI device probe & initialization function */
/* */
static INT __devinit   rt2860_probe(
    IN  struct pci_dev              *pci_dev, 
    IN  const struct pci_device_id  *pci_id)
{
	VOID 				*pAd = NULL;
	struct  net_device	*net_dev;
	PVOID				handle;
	PSTRING				print_name;
	ULONG				csr_addr;
	INT rv = 0;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook;
	ULONG					OpMode;

	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_probe\n"));

/*PCIDevInit============================================== */
	/* wake up and enable device */
	if ((rv = pci_enable_device(pci_dev))!= 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Enable PCI device failed, errno=%d!\n", rv));
		return rv;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	print_name = (PSTRING)pci_name(pci_dev);
#else
	print_name = pci_dev->slot_name;
#endif /* LINUX_VERSION_CODE */

	if ((rv = pci_request_regions(pci_dev, print_name)) != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Request PCI resource failed, errno=%d!\n", rv));
		goto err_out;
	}
	
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long) ioremap(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));
	if (!csr_addr)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
					print_name, (ULONG)pci_resource_len(pci_dev, 0), (ULONG)pci_resource_start(pci_dev, 0)));
		goto err_out_free_res;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: at 0x%lx, VA 0x%lx, IRQ %d. \n",  print_name, 
					(ULONG)pci_resource_start(pci_dev, 0), (ULONG)csr_addr, pci_dev->irq));
	}

	/* Set DMA master */
	pci_set_master(pci_dev);


/*RtmpDevInit============================================== */
	/* Allocate RTMP_ADAPTER adapter structure */
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));
	if (handle == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Allocate memory for os handle failed!\n", __FUNCTION__));
		goto err_out_iounmap;
	}
	memset(handle, 0, sizeof(struct os_cookie));

	((POS_COOKIE)handle)->pci_dev = pci_dev;

#ifdef OS_ABL_FUNC_SUPPORT
{
	RTMP_PCI_CONFIG PciConfig;
	PciConfig.ConfigVendorID = PCI_VENDOR_ID;
	/* get DRIVER operations */
	RTMP_DRV_OPS_FUNCTION(pRtmpDrvOps, NULL, &PciConfig, NULL);
}
#endif /* OS_ABL_FUNC_SUPPORT */

	rv = RTMPAllocAdapterBlock(handle, &pAd);	/* we may need the pci_dev for allocate structure of "RTMP_ADAPTER" */
	if (rv != NDIS_STATUS_SUCCESS) 
		goto err_out_iounmap;
	/* Here are the RTMP_ADAPTER structure with pci-bus specific parameters. */
	RTMP_DRIVER_PCI_CSR_SET(pAd, csr_addr);

	RTMP_DRIVER_PCIE_INIT(pAd, pci_dev);

/*NetDevInit============================================== */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;
	
	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = pci_dev->irq;		/* Interrupt IRQ number */
	net_dev->base_addr = csr_addr;		/* Save CSR virtual address and irq to device structure */
	pci_set_drvdata(pci_dev, net_dev);	/* Set driver data */
	

/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */
#ifdef RT_CFG80211_SUPPORT
{
/*	pAd->pCfgDev = &(pci_dev->dev); */
/*	pAd->CFG80211_Register = CFG80211_Register; */
/*	RTMP_DRIVER_CFG80211_INIT(pAd, pci_dev); */

	/*
		In 2.6.32, cfg80211 register must be before register_netdevice();
		We can not put the register in rt28xx_open();
		Or you will suffer NULL pointer in list_add of
		cfg80211_netdev_notifier_call().
	*/
	CFG80211_Register(pAd, &(pci_dev->dev), net_dev);
}
#endif /* RT_CFG80211_SUPPORT */

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	rv = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);
	if (rv)
		goto err_out_free_netdev;


/*#ifdef KTHREAD_SUPPORT */
#ifdef PRE_ASSIGN_MAC_ADDR
	UCHAR PermanentAddress[MAC_ADDR_LEN];
	RTMP_DRIVER_MAC_ADDR_GET(pAd, &PermanentAddress[0]);
	DBGPRINT(RT_DEBUG_TRACE, ("@%s MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, PermanentAddress[0], PermanentAddress[1],PermanentAddress[2],PermanentAddress[3],PermanentAddress[4],PermanentAddress[5]));
	/* Set up the Mac address */
	RtmpOSNetDevAddrSet(OpMode, net_dev, &PermanentAddress[0], NULL);
#endif /* PRE_ASSIGN_MAC_ADDR */

	wl_proc_init();

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_probe\n"));

	return 0; /* probe ok */


	/* --------------------------- ERROR HANDLE --------------------------- */
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);
	
err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);

err_out_iounmap:
	iounmap((void *)(csr_addr));
	release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0)); 
	
err_out_free_res:
	pci_release_regions(pci_dev);
	
err_out:
	pci_disable_device(pci_dev);

	DBGPRINT(RT_DEBUG_ERROR, ("<=== rt2860_probe failed with rv = %d!\n", rv));

	return -ENODEV; /* probe fail */
}


static VOID __devexit rt2860_remove_one(
    IN  struct pci_dev  *pci_dev)
{
	PNET_DEV	net_dev = pci_get_drvdata(pci_dev);
	VOID		*pAd = NULL;
	ULONG		csr_addr = net_dev->base_addr; /* pAd->CSRBaseAddress; */
	
	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	
	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_remove_one\n"));

	if (pAd != NULL)
	{
		/* Unregister/Free all allocated net_device. */
		RtmpPhyNetDevExit(pAd, net_dev);

		/* Unmap CSR base address */
		iounmap((char *)(csr_addr));
		
		/* release memory region */
		release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));

#ifdef RT_CFG80211_SUPPORT
		RTMP_DRIVER_80211_UNREGISTER(pAd, net_dev);
#endif /* RT_CFG80211_SUPPORT */

		/* Free RTMP_ADAPTER related structures. */
		RtmpRaDevCtrlExit(pAd);
	}
	else
	{
		/* Unregister network device */
		RtmpOSNetDevDetach(net_dev);

		/* Unmap CSR base address */
		iounmap((char *)(net_dev->base_addr));

		/* release memory region */
		release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));
	}

	/* Free the root net_device */
	RtmpOSNetDevFree(net_dev);

	wl_proc_exit();
}

