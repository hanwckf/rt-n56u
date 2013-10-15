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
	rt2880.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT2880_H__
#define __RT2880_H__


#ifndef RTMP_RBUS_SUPPORT
#error "For RT2880, you should define the compile flag -DRTMP_RBUS_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT2880, you should define the compile flag -DRTMP_MAC_PCI"
#endif

#ifdef LINUX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
#include <asm/rt2880/rt_mmap.h>
#define RTMP_MAC_CSR_ADDR RALINK_11N_MAC_BASE
#else

#ifdef CONFIG_RALINK_RT2880_SHUTTLE
#undef RTMP_MAC_CSR_ADDR
#define RTMP_MAC_CSR_ADDR 0xA0600000
#endif /* CONFIG_RALINK_RT2880_SHUTTLE */

#ifdef CONFIG_RALINK_RT2880_MP
#undef RTMP_MAC_CSR_ADDR
#define RTMP_MAC_CSR_ADDR 0xA0480000
#endif /* CONFIG_RALINK_RT2880_MP */

#ifndef RTMP_MAC_CSR_ADDR  
#error "Please Choice Chip Version (Shuttle/MP)"
#endif

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21) */

#else

#ifdef CONFIG_RALINK_RT2880_SHUTTLE
#undef RTMP_MAC_CSR_ADDR
#define RTMP_MAC_CSR_ADDR 0xA0600000
#endif /* CONFIG_RALINK_RT2880_SHUTTLE */

#ifdef CONFIG_RALINK_RT2880_MP
#undef RTMP_MAC_CSR_ADDR
#define RTMP_MAC_CSR_ADDR 0xA0480000
#endif /* CONFIG_RALINK_RT2880_MP */

#ifndef RTMP_MAC_CSR_ADDR  
#error "Please Choice Chip Version (Shuttle/MP)"
#endif
#endif /* LINUX */

#define RT2860_CSR_ADDR_PCI		0xC0000000 /* RT2880 PCI */

#define RTMP_FLASH_BASE_ADDR	0xbfc00000

struct _RTMP_ADAPTER;
VOID RT2880_Init(IN struct _RTMP_ADAPTER *pAd);

/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */


#endif /*__RT2880_H__ */

