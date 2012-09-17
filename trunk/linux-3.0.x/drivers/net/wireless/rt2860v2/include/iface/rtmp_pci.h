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
	rtmp_pci.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_PCI_H__
#define __RTMP_PCI_H__

#define RT28XX_HANDLE_DEV_ASSIGN(handle, dev_p)				\
	((POS_COOKIE)handle)->pci_dev = dev_p;


#ifdef LINUX
// set driver data
#define RT28XX_DRVDATA_SET(_a)			pci_set_drvdata(_a, net_dev);

#define RT28XX_PUT_DEVICE(dev_p)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#ifndef SA_SHIRQ
#define SA_SHIRQ IRQF_SHARED
#endif
#endif

#ifdef PCI_MSI_SUPPORT
#define RTMP_MSI_ENABLE(_pAd) \
	{     POS_COOKIE _pObj = (POS_COOKIE)(_pAd->OS_Cookie); \
		(_pAd)->HaveMsi = pci_enable_msi(_pObj->pci_dev) == 0 ? TRUE : FALSE; \
	}

#define RTMP_MSI_DISABLE(_pAd) \
	{     POS_COOKIE _pObj = (POS_COOKIE)(_pAd->OS_Cookie); \
		if (_pAd->HaveMsi == TRUE) \
			pci_disable_msi(_pObj->pci_dev); \
		_pAd->HaveMsi = FALSE;  \
	}
#else
#define RTMP_MSI_ENABLE(_pAd)		do{}while(0)
#define RTMP_MSI_DISABLE(_pAd)		do{}while(0)
#endif // PCI_MSI_SUPPORT //

#define RTMP_PCI_DEV_UNMAP()										\
{	if (net_dev->base_addr)	{								\
		iounmap((void *)(net_dev->base_addr));				\
		release_mem_region(pci_resource_start(dev_p, 0),	\
							pci_resource_len(dev_p, 0)); }	\
	if (net_dev->irq) pci_release_regions(dev_p); }


#define PCI_REG_READ_WORD(pci_dev, offset, Configuration)   \
    if (pci_read_config_word(pci_dev, offset, &reg16) == 0)     \
        Configuration = le2cpu16(reg16);                        \
    else                                                        \
        Configuration = 0;

#define PCI_REG_WIRTE_WORD(pci_dev, offset, Configuration)  \
    reg16 = cpu2le16(Configuration);                        \
    pci_write_config_word(pci_dev, offset, reg16);

#endif // LINUX //

#define CMDTHREAD_CHAN_RESCAN					0x0D730101
#define CMDTHREAD_REG_HINT						0x0D730102
#define CMDTHREAD_REG_HINT_11D					0x0D730103
#define CMDTHREAD_SCAN_END						0x0D730104
#define CMDTHREAD_CONNECT_RESULT_INFORM			0x0D730105

#endif // __RTMP_PCI_H__ //
