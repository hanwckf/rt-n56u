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
	mt7628.h

    Abstract:
	2*2 Wireless Chip SoC

    Revision History:
    Who			When			What
    ---------	----------		----------------------------------------------
	Carter.Chen		20130912		Initial version
 */

#ifndef __MT7628_H__
#define __MT7628_H__

#ifdef MT7628

#ifndef RTMP_RBUS_SUPPORT
#error "For RT3883, you should define the compile flag -DRTMP_RBUS_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT3883, you should define the compile flag -DRTMP_MAC_PCI"
#endif

#include "mcu/andes_core.h"
#include "phy/mt_rf.h"

struct _RTMP_ADAPTER;

/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */
#define RTMP_MAC_CSR_ADDR	0xB0300000
#define RTMP_FLASH_BASE_ADDR	0xbc000000


#if defined (CONFIG_MTK_MT7628)
#define PROCREG_DIR             "mt7628"
#endif

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt7628_init(struct _RTMP_ADAPTER *pAd);
void mt7628_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7628_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7628_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
int mt7628_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7628_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7628_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7628_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7628_set_ed_cca(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);


#endif /* MT7628 */
#endif /*__MT7628_H__ */
/* End of mt7628.h */

