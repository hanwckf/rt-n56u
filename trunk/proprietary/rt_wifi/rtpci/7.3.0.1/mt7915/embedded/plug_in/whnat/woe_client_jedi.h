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

	Module Name: wifi_offload
	woe_client_jedi.h
*/


#ifndef _WOE_CLIENT_JEDI_H_
#define _WOE_CLIENT_JEDI_H_

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <os/rt_linux_txrx_hook.h>


#ifdef MT7915
#include "woe_mt7915.h"
#endif

extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl);
#ifdef MULTI_INF_SUPPORT
/*EXPORT symbol from wifi drvier*/
extern int multi_inf_get_idx(VOID *pAd);
#endif /*MULTI_INF_SUPPORT*/

#define WIFI_RING_OFFSET		0x10
#define WIFI_TX_RING_SIZE		(2048)
#define WIFI_PDMA_TXD_SIZE		(TXD_SIZE)
#define WIFI_TX_1ST_BUF_SIZE	128
#define WIFI_RX1_RING_SIZE		(512)
#define WIFI_TX_BUF_SIZE		1900

#define WIFI_TXD_INIT(_txd) (((struct _TXD_STRUC *) _txd)->DMADONE = DMADONE_DONE)

#endif /*_WOE_CLIENT_JEDI_H_*/
