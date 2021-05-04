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
	mt_mac_pci.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __MAC_PCI_H__
#define __MAC_PCI_H__

#include "rtmp_type.h"
#include "phy/phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"



/* ----------------- Interface Related MACRO ----------------- */

typedef enum _RTMP_TX_DONE_MASK {
	TX_AC0_DONE = 0,
	TX_AC1_DONE = 1,
	TX_AC2_DONE = 2,
	TX_AC3_DONE = 3,
	TX_HCCA_DONE = 4,
	TX_MGMT_DONE = 5,
	TX_BMC_DONE = 6,
} RTMP_TX_DONE_MASK;


/* For RTMPPCIePowerLinkCtrlRestore () function */
#define RESTORE_HALT		1
#define RESTORE_WAKEUP		2
#define RESTORE_CLOSE           3

#define PowerSafeCID		1
#define PowerRadioOffCID	2
#define PowerWakeCID		3
#define CID0MASK		0x000000ff
#define CID1MASK		0x0000ff00
#define CID2MASK		0x00ff0000
#define CID3MASK		0xff000000

struct _RTMP_ADAPTER;
enum _RTMP_TX_DONE_MASK;

VOID RTMPHandleMgmtRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandlePreTBTTInterrupt(struct _RTMP_ADAPTER *pAd);

void RTMPHandleTwakeupInterrupt(struct _RTMP_ADAPTER *pAd);

/*pci state contrl*/
USHORT	pci_write_frag_tx_resource(struct _RTMP_ADAPTER *pAd,
									  struct _TX_BLK *pTxBlk,
									  UCHAR fragNum,
									  USHORT *FreeNumber);

VOID pci_inc_resource_full_cnt(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
VOID pci_dec_resource_full_cnt(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_get_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_get_all_resource_state(struct _RTMP_ADAPTER *pAd);
INT pci_set_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state);
UINT32 pci_check_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);

UINT32 pci_get_tx_resource_free_num_nolock(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
UINT32 pci_get_rx_resource_pending_num(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);
BOOLEAN pci_is_tx_resource_empty(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);

VOID mtd_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

VOID mt_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

#endif /*__MAC_PCI_H__ */

