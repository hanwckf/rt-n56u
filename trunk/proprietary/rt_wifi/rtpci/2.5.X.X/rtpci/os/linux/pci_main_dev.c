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

#include "rt_config.h"
#include <linux/pci.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
#define __devinit
#define __devinitdata
#define __devexit
#define __devexit_p
#endif

//
// Function declarations
//
extern int rt28xx_close(IN struct net_device *net_dev);
extern int rt28xx_open(struct net_device *net_dev);

static VOID __devexit rt2860_remove_one(struct pci_dev *pci_dev);
static INT __devinit rt2860_probe(struct pci_dev *pci_dev, const struct pci_device_id  *ent);
static void __exit rt2860_cleanup_module(void);
static int __init rt2860_init_module(void);

 static VOID RTMPInitPCIeDevice(
    IN  struct pci_dev   *pci_dev,
    IN PRTMP_ADAPTER     pAd);

#ifdef CONFIG_PM
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10)
#define pm_message_t u32
#endif

static int rt2860_suspend(struct pci_dev *pci_dev, pm_message_t state);
static int rt2860_resume(struct pci_dev *pci_dev);
#endif
#endif // CONFIG_PM //

//
// Ralink PCI device table, include all supported chipsets
//
static struct pci_device_id rt2860_pci_tbl[] __devinitdata =
{
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3091_PCIe_DEVICE_ID)},
#ifdef RT3090
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3090_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3091_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3092_PCIe_DEVICE_ID)},
#endif // RT3090 //
#ifdef RT3390
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC3390_PCIe_DEVICE_ID)},
#endif // RT3390 //
#ifdef RT5390
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5390_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC539F_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5392_PCIe_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5362_PCI_DEVICE_ID)},
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC5360_PCI_DEVICE_ID)},
#endif // RT3593 //
    {0,}		// terminate list
};

MODULE_DEVICE_TABLE(pci, rt2860_pci_tbl);


//
// Our PCI driver structure
//
static struct pci_driver rt2860_driver =
{
    name:       "rt2860",
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
	IN	PRTMP_ADAPTER	pAd)
{
	// clear PS packets
	// clear TxSw packets
}

static int rt2860_suspend(
	struct pci_dev *pci_dev,
	pm_message_t state)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)NULL;
	INT32 retval = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_suspend()\n"));

	if (net_dev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("net_dev == NULL!\n"));
	}
	else
	{
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		if (VIRTUAL_IF_NUM(pAd) > 0)
		{
			// avoid users do suspend after interface is down

			// stop interface
			netif_carrier_off(net_dev);
			netif_stop_queue(net_dev);

			// mark device as removed from system and therefore no longer available
			netif_device_detach(net_dev);

			// mark halt flag
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

			// take down the device
			rt28xx_close((PNET_DEV)net_dev);

			RT_MOD_DEC_USE_COUNT();
		}
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	// reference to http://vovo2000.com/type-lab/linux/kernel-api/linux-kernel-api.html
	// enable device to generate PME# when suspended
	// pci_choose_state(): Choose the power state of a PCI device to be suspended
	retval = pci_enable_wake(pci_dev, pci_choose_state(pci_dev, state), 1);
	// save the PCI configuration space of a device before suspending
	pci_save_state(pci_dev);
	// disable PCI device after use 
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
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)NULL;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	INT32 retval;


	// set the power state of a PCI device
	// PCI has 4 power states, DO (normal) ~ D3(less power)
	// in include/linux/pci.h, you can find that
	// #define PCI_D0          ((pci_power_t __force) 0)
	// #define PCI_D1          ((pci_power_t __force) 1)
	// #define PCI_D2          ((pci_power_t __force) 2)
	// #define PCI_D3hot       ((pci_power_t __force) 3)
	// #define PCI_D3cold      ((pci_power_t __force) 4)
	// #define PCI_UNKNOWN     ((pci_power_t __force) 5)
	// #define PCI_POWER_ERROR ((pci_power_t __force) -1)
	retval = pci_set_power_state(pci_dev, PCI_D0);

	// restore the saved state of a PCI device
	pci_restore_state(pci_dev);

	// initialize device before it's used by a driver
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
		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		if (VIRTUAL_IF_NUM(pAd) > 0)
		{
			// mark device as attached from system and restart if needed
			netif_device_attach(net_dev);

			if (rt28xx_open((PNET_DEV)net_dev) != 0)
			{
				// open fail
				DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_resume()\n"));
				return 0;
			}

			// increase MODULE use count
			RT_MOD_INC_USE_COUNT();

			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

			netif_start_queue(net_dev);
			netif_carrier_on(net_dev);
			netif_wake_queue(net_dev);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_resume()\n"));
	return 0;
}
#endif // CONFIG_PM //
#endif


static INT __init rt2860_init_module(VOID)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	return pci_register_driver(&rt2860_driver);
#else
    return pci_module_init(&rt2860_driver);
#endif
}


//
// Driver module unload function
//
static VOID __exit rt2860_cleanup_module(VOID)
{
    pci_unregister_driver(&rt2860_driver);
}

module_init(rt2860_init_module);
module_exit(rt2860_cleanup_module);


//
// PCI device probe & initialization function
//
#ifdef RT3090
static int twoPortM_read_proc(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
        int len;

        len = sprintf(page, "%d\n", 0x1);

        len -= off;
        *start = page + off;

        if (len > count)
                len = count;
        else
                *eof = 1;

        if (len < 0)
                len = 0;

        return len;
}

static int twoPortM_write_proc(struct file *file, const char *buffer,
        unsigned long count, void *data)
{
        char val_string[32];

        if (count > sizeof(val_string) - 1)
                return -EINVAL;

        if (copy_from_user(val_string, buffer, count))
                return -EFAULT;

        val_string[count] = '\0';

      //  memset((char *)&(atm_p->MIB_II), 0, sizeof(atmMIB_II_t));

        return count;
}
#endif // RT3090 //
static INT __devinit   rt2860_probe(
    IN  struct pci_dev              *pci_dev, 
    IN  const struct pci_device_id  *pci_id)
{
	PRTMP_ADAPTER 		pAd = (PRTMP_ADAPTER)NULL;
	struct  net_device		*net_dev;
	PVOID				handle;
	PSTRING				print_name;
	ULONG				csr_addr;
	INT rv = 0;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook;
#ifdef RT30xx
	static UCHAR MemReset=0;
#endif // RT3090 //
	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_probe\n"));

//PCIDevInit==============================================
	// wake up and enable device
	if ((rv = pci_enable_device(pci_dev))!= 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Enable PCI device failed, errno=%d!\n", rv));
		return rv;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	print_name = pci_dev ? (PSTRING)pci_name(pci_dev) : "rt2860";
#else
	print_name = pci_dev ? pci_dev->slot_name : "rt2860";
#endif // LINUX_VERSION_CODE //

	if ((rv = pci_request_regions(pci_dev, print_name)) != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Request PCI resource failed, errno=%d!\n", rv));
		goto err_out;
	}
	
	// map physical address to virtual address for accessing register
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

	// Set DMA master
	pci_set_master(pci_dev);


//RtmpDevInit==============================================
	// Allocate RTMP_ADAPTER adapter structure
	handle = kmalloc(sizeof(struct os_cookie), GFP_KERNEL);
	if (handle == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Allocate memory for os handle failed!\n", __FUNCTION__));
		goto err_out_iounmap;
	}

	memset(handle, 0, sizeof(struct os_cookie));

	((POS_COOKIE)handle)->pci_dev = pci_dev;
	
	rv = RTMPAllocAdapterBlock(handle, &pAd);	//shiang: we may need the pci_dev for allocate structure of "RTMP_ADAPTER"
	if (rv != NDIS_STATUS_SUCCESS) 
		goto err_out_iounmap;
	// Here are the RTMP_ADAPTER structure with pci-bus specific parameters.
	pAd->CSRBaseAddress = (PUCHAR)csr_addr;
	DBGPRINT(RT_DEBUG_ERROR, ("pAd->CSRBaseAddress =0x%lx, csr_addr=0x%lx!\n", (ULONG)pAd->CSRBaseAddress, csr_addr));
#ifdef RT30xx
	if (MemReset==0)
	{
		UINT32 MacCsr0 = 0;
		UCHAR Index = 0;
		BOOLEAN NeedResetChips=FALSE;
		MemReset=1;
		do 
		{
			RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);

			if ((MacCsr0 != 0x00) && (MacCsr0 != 0xFFFFFFFF))
				break;
		
			RTMPusecDelay(10);
		} while (Index++ < 100);
		if (((MacCsr0&0xFFFF0000)==0x3090) ||((MacCsr0&0xFFFF0000)==0x3390))
		{
			if ((MacCsr0&0x0000FFFF) < 0x3213)
			NeedResetChips=TRUE;
		}
		
		if (((MacCsr0&0xFFFF0000)==0x3071) )
		{
			if ((MacCsr0&0x0000FFFF) < 0x21C)
			NeedResetChips=TRUE;
		}
		
		if (NeedResetChips==TRUE)
		{
	  		struct proc_dir_entry *tsarm_proc;
	  		
			RTMP_IO_READ32(pAd, 0x5C0,&MacCsr0);
			MacCsr0|=0x400000;
			RTMP_IO_WRITE32(pAd, 0x5C0,MacCsr0);
			OS_WAIT(1000);
		
	  		/* Send reset PCI device event */
	  		//RTMPSendWirelessEvent(pAd, IW_RESETPCI_EVENT_FLAG, NULL, BSS0, 0);
	  
	  		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			tsarm_proc = create_proc_entry("TwoM", 0, NULL);
			if (tsarm_proc) {
				tsarm_proc->read_proc = twoPortM_read_proc;
				tsarm_proc->write_proc = twoPortM_write_proc;
			}
	  		goto err_out_free_radev;
		}
	}
#endif // RT3090 //
	RTMPInitPCIeDevice(pci_dev, pAd);
	
//NetDevInit==============================================
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;
	
	// Here are the net_device structure with pci-bus specific parameters.
	net_dev->irq = pci_dev->irq;		// Interrupt IRQ number
	net_dev->base_addr = csr_addr;		// Save CSR virtual address and irq to device structure
	pci_set_drvdata(pci_dev, net_dev);	// Set driver data
	

//All done, it's time to register the net device to linux kernel.
	// Register this device
#ifdef RT_CFG80211_SUPPORT
	pAd->pCfgDev = &(pci_dev->dev);
	pAd->CFG80211_Register = CFG80211_Register;
#endif // RT_CFG80211_SUPPORT //

	rv = RtmpOSNetDevAttach(net_dev, &netDevHook);
	if (rv)
		goto err_out_free_netdev;


#ifdef KTHREAD_SUPPORT
	init_waitqueue_head(&pAd->cmdQTask.kthread_q);
#ifdef WSC_INCLUDED
	init_waitqueue_head(&pAd->wscTask.kthread_q);
#endif
#endif // KTHREAD_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2860_probe\n"));

	return 0; // probe ok


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
	RTMP_ADAPTER	*pAd = NULL;
	ULONG			csr_addr = net_dev->base_addr; // pAd->CSRBaseAddress;
	
	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	
    DBGPRINT(RT_DEBUG_TRACE, ("===> rt2860_remove_one\n"));

	if (pAd != NULL)
	{
		// Unregister/Free all allocated net_device.
		RtmpPhyNetDevExit(pAd, net_dev);

		// Unmap CSR base address
		iounmap((char *)(csr_addr));
		
		// release memory region
		release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));

#ifdef RT_CFG80211_SUPPORT
		CFG80211_UnRegister(pAd, net_dev);
#endif // RT_CFG80211_SUPPORT //

		// Free RTMP_ADAPTER related structures.
		RtmpRaDevCtrlExit(pAd);
		
	}
	else
	{
		// Unregister network device
		RtmpOSNetDevDetach(net_dev);

		// Unmap CSR base address
		iounmap((char *)(net_dev->base_addr));

		// release memory region
		release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));
	}

	// Free the root net_device
	RtmpOSNetDevFree(net_dev);

#ifdef VENDOR_FEATURE4_SUPPORT
{
	extern ULONG OS_NumOfMemAlloc, OS_NumOfMemFree;
	DBGPRINT(RT_DEBUG_TRACE, ("OS_NumOfMemAlloc = %ld, OS_NumOfMemFree = %ld\n",
			OS_NumOfMemAlloc, OS_NumOfMemFree));
}
#endif // VENDOR_FEATURE4_SUPPORT //
}
 


/***************************************************************************
 *
 *	PCIe device initialization related procedures.
 *
 ***************************************************************************/
 static VOID RTMPInitPCIeDevice(
    IN  struct pci_dev   *pci_dev,
    IN PRTMP_ADAPTER     pAd)
{
	USHORT  device_id;
	POS_COOKIE pObj;
#ifdef RT3090
	USHORT  subDev_id, subVendor_id;
#endif // RT3090 //

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pci_read_config_word(pci_dev, PCI_DEVICE_ID, &device_id);
#ifndef RT_BIG_ENDIAN
	device_id = le2cpu16(device_id);
#endif /* !RT_BIG_ENDIAN */
	pObj->DeviceID = device_id;
#ifdef RT3090
        pci_read_config_word(pci_dev, PCI_SUBSYSTEM_VENDOR_ID, &subVendor_id);
        subVendor_id = le2cpu16(subVendor_id);

        pci_read_config_word(pci_dev, PCI_SUBSYSTEM_ID, &subDev_id);
        subDev_id = le2cpu16(subDev_id);
#endif // RT3090 //
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
	if (
#ifdef RT3090
		(device_id == NIC3090_PCIe_DEVICE_ID) ||
		(device_id == NIC3091_PCIe_DEVICE_ID) ||
		(device_id == NIC3092_PCIe_DEVICE_ID) ||
#endif // RT3090 //
#ifdef RT3390
	(device_id ==  NIC3390_PCIe_DEVICE_ID)||
#endif // RT3390 //
#ifdef RT5390
	(device_id ==  NIC5390_PCIe_DEVICE_ID)||
	(device_id ==  NIC539F_PCIe_DEVICE_ID)||
	(device_id ==  NIC5392_PCIe_DEVICE_ID)||
	(device_id ==  NIC5362_PCI_DEVICE_ID)||
	(device_id ==  NIC5360_PCI_DEVICE_ID)||
#endif // RT5390 //

		 0)
	{
		UINT32 MacCsr0 = 0, Index= 0;
		do 
		{
			RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);

			if ((MacCsr0 != 0x00) && (MacCsr0 != 0xFFFFFFFF))
				break;

			RTMPusecDelay(10);
		} while (Index++ < 100);

		// Support advanced power save after 2892/2790.
		// MAC version at offset 0x1000 is 0x2872XXXX/0x2870XXXX(PCIe, USB, SDIO).
		if ((MacCsr0&0xffff0000) != 0x28600000)
		{
#ifdef PCIE_PS_SUPPORT			
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
#endif // PCIE_PS_SUPPORT //
#ifdef RT3090
			if ((subVendor_id == 0x1462) && (subDev_id == 0x891A))
				RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N);
			else
				RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N);
#endif // RT3090 //
			RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_PCIE);
			return;
		}
		

	}
	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_PCI);
}

