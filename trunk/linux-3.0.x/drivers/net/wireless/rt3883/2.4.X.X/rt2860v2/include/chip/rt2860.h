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
	rt2860.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT2860_H__
#define __RT2860_H__

#ifdef RT2860
#include "chip/mac_pci.h"

#ifndef RTMP_PCI_SUPPORT
#error "For RT2860, you should define the compile flag -DRTMP_PCI_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT2880, you should define the compile flag -DRTMP_MAC_PCI"
#endif

//
// Device ID & Vendor ID, these values should match EEPROM value
//
#define NIC2860_PCI_DEVICE_ID	0x0601
#define NIC2860_PCIe_DEVICE_ID	0x0681
#define NIC2760_PCI_DEVICE_ID	0x0701		// 1T/2R Cardbus ???
#define NIC2790_PCIe_DEVICE_ID  0x0781		// 1T/2R miniCard


#define VEN_AWT_PCIe_DEVICE_ID	0x1059
#define VEN_AWT_PCI_VENDOR_ID		0x1A3B

#define EDIMAX_PCI_VENDOR_ID		0x1432

#endif // RT2860 //

#endif //__RT2860_H__ //
