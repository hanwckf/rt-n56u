#ifdef MTK_LICENSE
/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_hif_usb.h
*/
#endif /* MTK_LICENSE */
#ifndef __MT_HIF_USB_H__
#define __MT_HIF_USB_H__

#define MT_HIF_BASE 0x4000

/* UDMA register */
#define UDMA_BASE		0x50029000
#define UDMA_WLCFG_1	(0x)
#define UDMA_RESET		(UDMA_BASE + 0x14)
#define UDMA_WLCFG_0	(UDMA_BASE + 0x18)
#define UDMA_WLCFG_0_TX_BT_SIZE_MASK (0x07 << 27)
#define UDMA_WLCFG_0_TX_BT_SIZE(p) (((p) & 0x07) << 27)
#define UDMA_WLCFG_0_RX_BT_SIZE_MASK (0x07 << 24)
#define UDMA_WLCFG_0_RX_BT_SIZE(p) (((p) & 0x07) << 24)
#define UDMA_WLCFG_0_TX_EN_MASK (0x1 << 23)
#define UDMA_WLCFG_0_TX_EN(p) (((p) & 0x1) << 23)
#define UDMA_WLCFG_0_RX_EN_MASK (0x1 << 22)
#define UDMA_WLCFG_0_RX_EN(p) (((p) & 0x1) << 22)
#define UDMA_WLCFG_0_RX_AGG_EN_MASK (0x1 << 21)
#define UDMA_WLCFG_0_RX_AGG_EN(p) (((p) & 0x1) << 21)
#define UDMA_WLCFG_0_LPK_EN_MASK (0x1 << 20)
#define UDMA_WLCFG_0_LPK_EN(p) (((p) & 0x1) << 20)
#define UDMA_WLCFG_0_RX_MPSZ_PAD0_MASK (0x1 << 18)
#define UDMA_WLCFG_0_RX_MPSZ_PAD0(p) (((p) & 0x1) << 18)
#define UDMA_WLCFG_0_RX_AGG_LMT_MASK (0xff << 8)
#define UDMA_WLCFG_0_RX_AGG_LMT(p) (((p) & 0xff) << 8)
#define UDMA_WLCFG_0_RX_AGG_TO_MASK (0xff << 0)
#define UDMA_WLCFG_0_RX_AGG_TO(p) (((p) & 0xff) << 0)

#define STOP_DROP_EPOUT	(0x80)

#endif /* __MT_HIF_USB_H__ */
