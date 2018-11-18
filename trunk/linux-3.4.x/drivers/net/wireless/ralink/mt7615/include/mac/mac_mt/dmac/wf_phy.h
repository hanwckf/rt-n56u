/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wf_phy.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_PHY_H__
#define __WF_PHY_H__

#define WF_PHY_BASE			0x10000

#define PHY_PHYSYS_CTRL		(WF_PHY_BASE + 0x0000) /* 0x82070000 */
#define PHY_PHYCK_CTRL			(WF_PHY_BASE + 0x0004) /* 0x82070004 */

#define PHY_BAND0_PHY_CTRL_0	(WF_PHY_BASE + 0x0200) /* 0x82070200 */
#define RO_BAND0_PHYCTRL_STS0	(WF_PHY_BASE + 0x020C) /* 0x8207020C */
#define RO_BAND0_PHYCTRL_STS1	(WF_PHY_BASE + 0x0210) /* 0x82070210 */
#define RO_BAND0_PHYCTRL_STS2	(WF_PHY_BASE + 0x0214) /* 0x82070214 */
#define RO_BAND0_PHYCTRL_STS4	(WF_PHY_BASE + 0x021C) /* 0x8207021C */
#define RO_BAND0_PHYCTRL_STS5	(WF_PHY_BASE + 0x0220) /* 0x82070220 */
#define RO_BAND0_PHYCTRL_STS	(WF_PHY_BASE + 0x0230) /* 0x82070230 */



#define PHY_BAND1_PHY_CTRL_0	(WF_PHY_BASE + 0x0400) /* 0x82070400 */
#define RO_BAND1_PHYCTRL_STS0	(WF_PHY_BASE + 0x040C) /* 0x8207040C */
#define RO_BAND1_PHYCTRL_STS1	(WF_PHY_BASE + 0x0410) /* 0x82070410 */
#define RO_BAND1_PHYCTRL_STS2	(WF_PHY_BASE + 0x0414) /* 0x82070414 */
#define RO_BAND1_PHYCTRL_STS4	(WF_PHY_BASE + 0x041C) /* 0x8207041C */
#define RO_BAND1_PHYCTRL_STS5	(WF_PHY_BASE + 0x0420) /* 0x82070420 */
#define RO_BAND1_PHYCTRL_STS	(WF_PHY_BASE + 0x0430) /* 0x82070430 */


#define PHY_BAND0_PHYMUX_5	(WF_PHY_BASE + 0x0614) /* 0x82070614 */
#define PHY_BAND0_PHYMUX_6	(WF_PHY_BASE + 0x0618) /* 0x82070618 */
#define PHY_BAND0_PHYMUX_23	(WF_PHY_BASE + 0x065c) /* 0x8207065c */
#define PHY_BAND1_PHYMUX_5	(WF_PHY_BASE + 0x0814) /* 0x82070814 */
#define PHY_BAND1_PHYMUX_6	(WF_PHY_BASE + 0x0818) /* 0x82070818 */
#define PHY_BAND1_PHYMUX_23	(WF_PHY_BASE + 0x085c) /* 0x8207085c */
#define PHY_BAND0_PHY_CCA       (WF_PHY_BASE + 0x0618) /*  0x82070618 */
#define PHY_BAND1_PHY_CCA       (WF_PHY_BASE + 0x0818) /*  0x82070818 */

#define CR_DBGSGD_MODE (WF_PHY_BASE + 0x0C04) /*  0x82070C04 */ 

#define PHY_TXFD_1				(WF_PHY_BASE + 0x8004) /* 0x82078004 */

#define PHY_RXTD_RXFE_01_B0	(WF_PHY_BASE + 0x2004) /* 0x82072004 */
#define PHY_RXTD_RXFE_01_B1	(WF_PHY_BASE + 0x2804) /* 0x82072804 */

#define PHY_RXTD_0				(WF_PHY_BASE + 0x2200) /* 0x82072200 */
#define PHY_RXTD_12                        (WF_PHY_BASE + 0x2230) /* 0x82072230 */
#define PHY_MIN_PRI_PWR              (WF_PHY_BASE + 0x229C) /* 0x8207229C */
#define BAND1_PHY_MIN_PRI_PWR       (WF_PHY_BASE + 0x0084) /* 0x82070084 */
#define PHY_RXTD2_10                      (WF_PHY_BASE + 0x2a28) /* 0x82072a28 */
#define PHY_RXTD2_0				(WF_PHY_BASE + 0x2a00) /* 0x82072a00 */

#define PHY_LTFSYNC_6			(WF_PHY_BASE + 0x22f4) /* 0x820722f4 */

#define PHY_RXTD_CCKPD_3		(WF_PHY_BASE + 0x2300) /* 0x82072300 */
#define PHY_RXTD_CCKPD_4		(WF_PHY_BASE + 0x2304) /* 0x82072304 */
#define PHY_RXTD_CCKPD_6		(WF_PHY_BASE + 0x230c) /* 0x8207230c */
#define PHY_RXTD_CCKPD_7		(WF_PHY_BASE + 0x2310) /* 0x82072310 */
#define PHY_RXTD_CCKPD_8        	(WF_PHY_BASE + 0x2314) /* 0x82072314 */

#define RO_BAND0_RXTD_DEBUG0		(WF_PHY_BASE + 0x227c) /* 0x8207227c */
#define RO_BAND0_RXTD_DEBUG4		(WF_PHY_BASE + 0x228c) /* 0x8207228c */
#define RO_BAND0_RXTD_DEBUG6		(WF_PHY_BASE + 0x2294) /* 0x82072294 */


#define PHY_RXTD1_0				(WF_PHY_BASE + 0x2600) /* 0x82072600 */
#define PHY_RXTD1_1				(WF_PHY_BASE + 0x2604) /* 0x82072604 */
#define PHY_RXTD1_4				(WF_PHY_BASE + 0x2610) /* 0x82072610 */

#define PHY_RXTD_43				(WF_PHY_BASE + 0x22ac) /* 0x820722ac */
#define PHY_RXTD_44				(WF_PHY_BASE + 0x22b0) /* 0x820722b0 */
#define PHY_RXTD_56				(WF_PHY_BASE + 0x2728) /* 0x82072728 */
#define PHY_RXTD_58				(WF_PHY_BASE + 0x2730) /* 0x82072730 */

#define PHY_RXTD_BAND0_AGC_23_RX0	(WF_PHY_BASE + 0x215c) /* 0x8207215c */
#define PHY_RXTD_BAND0_AGC_23_RX1	(WF_PHY_BASE + 0x255c) /* 0x8207255c */
#define RO_BAND0_AGC_DEBUG_0		(WF_PHY_BASE + 0x21a0) /* 0x820721a0 */
#define RO_BAND0_AGC_DEBUG_2		(WF_PHY_BASE + 0x21A8) /* 0x820721A8 */
#define RO_BAND0_AGC_DEBUG_4		(WF_PHY_BASE + 0x21B0) /* 0x820721B0 */
#define RO_BAND0_AGC_DEBUG_6		(WF_PHY_BASE + 0x21b8) /* 0x820721b8 */
#define RO_BAND1_AGC_DEBUG_2		(WF_PHY_BASE + 0x29A8) /* 0x820729A8 */
#define RO_BAND1_AGC_DEBUG_4		(WF_PHY_BASE + 0x29B0) /* 0x820729B0 */

#define PHY_FSD_CTRL_1				(WF_PHY_BASE + 0x50d8) /* 0x820750d8 */

#define PHY_TX_BAND0_WF0_CR_TXFE_3	(WF_PHY_BASE + 0x8408) /* 0x82078408 */
#define PHY_TX_BAND0_WF1_CR_TXFE_3	(WF_PHY_BASE + 0x8420) /* 0x82078420 */
#define PHY_TX_BAND1_WF0_CR_TXFE_3	(WF_PHY_BASE + 0x8438) /* 0x82078438 */
#define PHY_TX_BAND1_WF1_CR_TXFE_3	(WF_PHY_BASE + 0x8450) /* 0x82078450 */

#define PHY_CTRL_TSSI_9					(WF_PHY_BASE + 0x9c24) /* 0x82079c24 */
#define PHY_CTRL_WF1_TSSI_9			(WF_PHY_BASE + 0x9d24) /* 0x82079d24 */
#define PHY_CTRL_WF2_TSSI_9			(WF_PHY_BASE + 0x9e24) /* 0x82079e24 */
#define PHY_CTRL_WF3_TSSI_9			(WF_PHY_BASE + 0x9f24) /* 0x82079f24 */


#ifdef PALLADIUM
#define CR_CM_TOP_CTRL						(WF_PHY_BASE + 0xf000) /* 0x8207f000 */
#define CR_CM1_TOP_CTRL					(WF_PHY_BASE + 0xf100) /* 0x8207f100 */
#define CR_CM2_TOP_CTRL					(WF_PHY_BASE + 0xf200) /* 0x8207f200 */
#define CR_CM3_TOP_CTRL					(WF_PHY_BASE + 0xf300) /* 0x8207f300 */
#define CR_CM_TOP_RST_BITS					31
#define CR_CM_TOP_RST_MASK				(0x1)
#define CR_CM_INTF_RST_BITS				30
#define CR_CM_INTF_RST_MASK				(0x1)
#define CR_AFE_BAND_BITS					29
#define CR_AFE_BAND_MASK					(0x1)
#define CR_AFE_CBW_BITS					27
#define CR_AFE_CBW_MASK					(0x3)
#define CR_CH_RX_DBM_BITS					16
#define CR_CH_RX_DBM_MASK					(0x7ff)
#define CR_CM_INTF_EN_BITS					15
#define CR_CM_INTF_EN_MASK				(0x1)
#define CR_AFE_AWGN_NOISE_EN_BITS		14
#define CR_AFE_AWGN_NOISE_EN_MASK		(0x1)
#define CR_CH_TYPE_BITS						8
#define CR_CH_TYPE_MASK					(0x3f)
#define CR_TRANSPOSE_MODE_EN_BITS		7
#define CR_TRANSPOSE_MODE_EN_MASK		(0x1)
#define CR_N_RX_BITS						4
#define CR_N_RX_MASK						(0x7)
#define CR_N_TX_BITS						1
#define CR_N_TX_MASK						(0x7)
#define CR_CM_LOAD_EN_BITS					0
#define CR_CM_LOAD_EN_MASK				(0x1)


#define CR_CM_INTF_T1_CTRL					(WF_PHY_BASE + 0xf004) /* 0x8207f004 */
#define CR_CM1_INTF_T1_CTRL				(WF_PHY_BASE + 0xf104) /* 0x8207f104 */
#define CR_CM2_INTF_T1_CTRL				(WF_PHY_BASE + 0xf204) /* 0x8207f204 */
#define CR_CM3_INTF_T1_CTRL				(WF_PHY_BASE + 0xf304) /* 0x8207f304 */

#define CR_CM_INTF_T2_CTRL					(WF_PHY_BASE + 0xf008) /* 0x8207f008 */
#define CR_CM1_INTF_T2_CTRL				(WF_PHY_BASE + 0xf108) /* 0x8207f108 */
#define CR_CM2_INTF_T2_CTRL				(WF_PHY_BASE + 0xf208) /* 0x8207f208 */
#define CR_CM3_INTF_T2_CTRL				(WF_PHY_BASE + 0xf308) /* 0x8207f308 */

#define CR_CM_INTF_FREQ_CTRL				(WF_PHY_BASE + 0xf00c) /* 0x8207f00c */
#define CR_CM1_INTF_FREQ_CTRL				(WF_PHY_BASE + 0xf10c) /* 0x8207f10c */
#define CR_CM2_INTF_FREQ_CTRL				(WF_PHY_BASE + 0xf20c) /* 0x8207f20c */
#define CR_CM3_INTF_FREQ_CTRL				(WF_PHY_BASE + 0xf30c) /* 0x8207f30c */


#define CR_CM_CH_SEED_CTRL					(WF_PHY_BASE + 0xf010) /* 0x8207f010 */
#define CR_CM1_CH_SEED_CTRL				(WF_PHY_BASE + 0xf120) /* 0x8207f110 */
#define CR_CM2_CH_SEED_CTRL				(WF_PHY_BASE + 0xf220) /* 0x8207f210 */
#define CR_CM3_CH_SEED_CTRL				(WF_PHY_BASE + 0xf320) /* 0x8207f310 */
#define CR_CH_ID_BITS						0
#define CR_CH_ID_MASK						(0xffffffff)


#define CR_CM_AFE_SEED_CTRL				(WF_PHY_BASE + 0xf014) /* 0x8207f014 */
#define CR_CM1_AFE_SEED_CTRL				(WF_PHY_BASE + 0xf114) /* 0x8207f114 */
#define CR_CM2_AFE_SEED_CTRL				(WF_PHY_BASE + 0xf214) /* 0x8207f214 */
#define CR_CM3_AFE_SEED_CTRL				(WF_PHY_BASE + 0xf314) /* 0x8207f314 */
#define CR_AFE_SEED_BITS					0
#define CR_AFE_SEED_MASK					(0xffffffff)


#define CR_CM_INTF_SEED_CTRL				(WF_PHY_BASE + 0xf018) /* 0x8207f018 */
#define CR_CM1_INTF_SEED_CTRL				(WF_PHY_BASE + 0xf118) /* 0x8207f118 */
#define CR_CM2_INTF_SEED_CTRL				(WF_PHY_BASE + 0xf218) /* 0x8207f218 */
#define CR_CM3_INTF_SEED_CTRL				(WF_PHY_BASE + 0xf318) /* 0x8207f318 */

#define CR_CM_INTF_SEED_WS_CTRL			(WF_PHY_BASE + 0xf01c) /* 0x8207f01c */
#define CR_CM1_INTF_SEED_WS_CTRL			(WF_PHY_BASE + 0xf11c) /* 0x8207f11c */
#define CR_CM2_INTF_SEED_WS_CTRL			(WF_PHY_BASE + 0xf21c) /* 0x8207f21c */
#define CR_CM3_INTF_SEED_WS_CTRL			(WF_PHY_BASE + 0xf31c) /* 0x8207f31c */

#define CR_CM_MAIN_TAP_0_CTRL				(WF_PHY_BASE + 0xf020) /* 0x8207f020 */
#define CR_CM1_MAIN_TAP_0_CTRL			(WF_PHY_BASE + 0xf120) /* 0x8207f120 */
#define CR_CM2_MAIN_TAP_0_CTRL			(WF_PHY_BASE + 0xf220) /* 0x8207f220 */
#define CR_CM3_MAIN_TAP_0_CTRL			(WF_PHY_BASE + 0xf320) /* 0x8207f320 */

#define CR_CM_DC_CTRL						(WF_PHY_BASE + 0xf0c0) /* 0x8207f0c0 */
#define CR_CM1_DC_CTRL						(WF_PHY_BASE + 0xf1c0) /* 0x8207f1c0 */
#define CR_CM2_DC_CTRL						(WF_PHY_BASE + 0xf2c0) /* 0x8207f2c0 */
#define CR_CM3_DC_CTRL						(WF_PHY_BASE + 0xf3c0) /* 0x8207f3c0 */
#define CR_AFE_DC_NOISE_EN_BITS			31
#define CR_AFE_DC_NOISE_EN_MASK			(0x1)
#define CR_CM_DC_VAL_BITS					0
#define CR_CM_DC_VAL_MASK					(0x7ff)


#define CR_CM_INTF_EN_SYNC_MODE_CTRL		(WF_PHY_BASE + 0xf0c4) /* 0x8207f0c4 */
#define CR_CM1_INTF_EN_SYNC_MODE_CTRL	(WF_PHY_BASE + 0xf1c4) /* 0x8207f1c4 */
#define CR_CM2_INTF_EN_SYNC_MODE_CTRL	(WF_PHY_BASE + 0xf2c4) /* 0x8207f2c4 */
#define CR_CM3_INTF_EN_SYNC_MODE_CTRL	(WF_PHY_BASE + 0xf3c4) /* 0x8207f3c4 */

#define CR_CM_INTF_T3_CTRL					(WF_PHY_BASE + 0xf0c8) /* 0x8207f0c8 */
#define CR_CM1_INTF_T3_CTRL				(WF_PHY_BASE + 0xf1c8) /* 0x8207f1c8 */
#define CR_CM2_INTF_T3_CTRL				(WF_PHY_BASE + 0xf2c8) /* 0x8207f2c8 */
#define CR_CM3_INTF_T3_CTRL				(WF_PHY_BASE + 0xf3c8) /* 0x8207f3c8 */


#endif /* PALLADIUM */


#endif /* __WF_PHY_H__ */
