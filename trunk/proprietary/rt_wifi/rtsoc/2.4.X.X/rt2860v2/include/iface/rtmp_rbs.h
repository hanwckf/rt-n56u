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
	rtmp_rbs.h

    Abstract:
 	Ralink SoC Internal Bus related definitions and data dtructures

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#ifndef __RTMP_RBUS_H__
#define __RTMP_RBUS_H__

#ifdef LINUX
#include <linux/skbuff.h>
#endif // LINUX //


/*************************************************************************
  *
  *	Device hardware/ Interface related definitions.
  *
  ************************************************************************/  

#define RTMP_MAC_IRQ_NUM		SURFBOARDINT_WLAN


/*************************************************************************
  *
  *	EEPROM Related definitions
  *
  ************************************************************************/
#if defined(CONFIG_RALINK_RT3050_1T1R)
#if defined(CONFIG_RALINK_RT3350)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT3350_AP_1T1R_V1_0.bin"
#else
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT3050_AP_1T1R_V1_0.bin"
#endif // CONFIG_RALINK_RT3350 //
#endif // CONFIG_RALINK_RT3050_1T1R //

#if defined(CONFIG_RALINK_RT3051_1T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT3051_AP_1T2R_V1_0.bin"
#endif // CONFIG_RALINK_RT3051_1T2R //

#if defined(CONFIG_RALINK_RT3052_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT3052_AP_2T2R_V1_1.bin"
#endif // CONFIG_RALINK_RT3052_2T2R //

#if defined(CONFIG_RALINK_RT3883_3T3R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT3883_AP_3T3R_V0_1.bin"
#endif // CONFIG_RALINK_RT3883_3T3R //

#if defined(CONFIG_RALINK_RT3662_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT3662_AP_2T2R_V0_0.bin"
#endif // CONFIG_RALINK_RT3662_2T2R //

#if defined(CONFIG_RT2860V2_2850)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT2880_RT2850_AP_2T3R_V1_6.bin"
#endif // CONFIG_RT2860V2_2850 //

#ifndef EEPROM_DEFAULT_FILE_PATH
// RFIC 2820
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/Wireless/RT2880_RT2820_AP_2T3R_V1_6.bin"
#endif // EEPROM_DEFAULT_FILE_PATH //

#if defined (CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif

#define NVRAM_OFFSET				0x30000
#define RF_OFFSET					0x40000



#ifdef LINUX
/*************************************************************************
  *
  *	Device Tx/Rx related definitions.
  *
  ************************************************************************/
#if !defined(CONFIG_RA_NAT_NONE)
/* bruce+ */

extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
#endif

#ifdef DFS_SUPPORT
// TODO: Check these functions.
extern void unregister_tmr_service(void);
extern void request_tmr_service(int, void *, void *);

#endif // DFS_SUPPORT //


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#ifndef SA_SHIRQ
#define SA_SHIRQ IRQF_SHARED
#endif
#endif

#endif // LINUX //

#define CMDTHREAD_CHAN_RESCAN					0x0D730101
#define CMDTHREAD_REG_HINT						0x0D730102
#define CMDTHREAD_REG_HINT_11D					0x0D730103
#define CMDTHREAD_SCAN_END						0x0D730104
#define CMDTHREAD_CONNECT_RESULT_INFORM			0x0D730105

#endif // __RTMP_RBUS_H__ //

