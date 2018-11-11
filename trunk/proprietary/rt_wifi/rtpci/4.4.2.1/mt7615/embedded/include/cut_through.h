/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmp.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#include "rtmp_type.h"
#include "rtmp_os.h"
#include "mac/mac_mt/dmac/mt_dmac.h"
#include "rtmp_timer.h"


#ifndef ___CUT_THROUGH_H__
#define ___CUT_THROUGH_H__

#ifdef CUT_THROUGH_DBG
#define TIME_SLOT_NUMS 10
#endif
typedef enum _CT_MSDU_INFO_FLAG{
	CT_INFO_APPLY_TXD = BIT(0),
	HIF_PKT_FLAGS_COPY_HOST_TXD_ALL = BIT(1),
	CT_INFO_MGN_FRAME = BIT(2),
	CT_INFO_NONE_CIPHER_FRAME = BIT(3),
	CT_INFO_HSR2_TX = BIT(4), 
}CT_MSDU_INFO_FLAG;


typedef struct GNU_PACKED _CR4_TXP_MSDU_INFO {
#define MAX_BUF_NUM_PER_PKT 6
    UINT16 type_and_flags;  
    UINT16 msdu_token;
    UINT8 bss_index;
    UINT8 rept_wds_wcid;//if not rept/wds entry, leave to 0xff. 2015-June3 discussion.
    UINT8 reserved;
    UINT8 buf_num;
    UINT32 buf_ptr[MAX_BUF_NUM_PER_PKT];
    UINT16 buf_len[MAX_BUF_NUM_PER_PKT];
} CR4_TXP_MSDU_INFO;


#define CUT_THROUGH_TYPE_TX 1
#define CUT_THROUGH_TYPE_RX 2
#define CUT_THROUGH_TYPE_BOTH (CUT_THROUGH_TYPE_TX | CUT_THROUGH_TYPE_RX)

#define PKT_TX_TOKEN_ID_MAX	4095 /* token ID in range of 0~4095 */
#define PKT_TOKEN_ID_INVALID	PKT_TX_TOKEN_ID_MAX + 1
#define PKT_TX_TOKEN_ID_CNT	PKT_TX_TOKEN_ID_MAX + 1 /* number of token IDs */
#define PKT_TX_TOKEN_ID_ARAY	PKT_TX_TOKEN_ID_CNT + 1 /* Array space */

#define CUT_THROUGH_TX_ENABL(_ptr) ((((PKT_TOKEN_CB*)(_ptr))->cut_through_type & CUT_THROUGH_TYPE_TX) == CUT_THROUGH_TYPE_TX)
#define CUT_THROUGH_RX_ENABL(_ptr) ((((PKT_TOKEN_CB*)(_ptr))->cut_through_type & CUT_THROUGH_TYPE_RX) == CUT_THROUGH_TYPE_RX)

enum {
	TOKEN_NONE,
	TOKEN_TX_DATA,
	TOKEN_TX_MGT,
	TOKEN_RX,
};


typedef struct _PKT_TOKEN_ENTRY{
	PNDIS_PACKET pkt_buf;
	UINT32 rxDone;
	UINT32 InOrder;
	UINT32 Drop;
	UINT8 Type;
	LONG startTime;
	LONG endTime;
	NDIS_PHYSICAL_ADDRESS pkt_phy_addr;
	size_t pkt_len;	
#ifdef CONFIG_HOTSPOT_R2
	BOOLEAN Reprocessed;
#endif /* CONFIG_HOTSPOT_R2 */
} PKT_TOKEN_ENTRY;

/*
    queue operation behavior:
    1. If id_head != id_tail
        has free token
    2. if id_head == id_tail
        empty and no free token
*/
typedef struct _PKT_TOKEN_LIST {
    INT16 id_head; /* Index for first use-able token in free_id[] */
    INT16 id_tail; /* Index for first free_id[] to store recycled token */
    UINT16 free_id[PKT_TX_TOKEN_ID_ARAY];
    PKT_TOKEN_ENTRY pkt_token[PKT_TX_TOKEN_ID_CNT];
	UINT32 FreeTokenCnt;
	UINT32 TotalTxUsedTokenCnt;
	UINT32 TotalTxBackTokenCnt;
	UINT32 TotalTxTokenEventCnt;
	UINT32 TotalTxTokenCnt;
#ifdef CUT_THROUGH_DBG
	UINT32 UsedTokenCntRec[TIME_SLOT_NUMS];
	UINT32 UsedTokenCnt;
	UINT32 BackTokenCntRec[TIME_SLOT_NUMS];
	UINT32 BackTokenCnt;
	UINT32 DropPktCnt;
	UINT32 DropPktCntRec[TIME_SLOT_NUMS];
	UINT32 FreeAgg0_31;
	UINT32 FreeAgg0_31Rec[TIME_SLOT_NUMS];
	UINT32 FreeAgg32_63;
	UINT32 FreeAgg32_63Rec[TIME_SLOT_NUMS];
	UINT32 FreeAgg64_95;
	UINT32 FreeAgg64_95Rec[TIME_SLOT_NUMS];
	UINT32 FreeAgg96_127;
	UINT32 FreeAgg96_127Rec[TIME_SLOT_NUMS];
#endif
} PKT_TOKEN_LIST;

typedef struct _PKT_TOKEN_QUEUE {
    BOOLEAN token_inited;
    PKT_TOKEN_LIST *list;
	NDIS_SPIN_LOCK token_id_lock;
} PKT_TOKEN_QUEUE;


enum {
	NO_ENOUGH_FREE_TX_TOKEN = (1 << 0),
	NO_ENOUGH_FREE_TX_RING = (1 << 1),
#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 1)
    NO_ENOUGH_FREE_TX_ELEM = (1 << 2),
#endif
};

enum {
	NO_ENOUGH_FREE_RX_TOKEN = (1 << 0),
};

typedef struct _PKT_TOKEN_CB {
	INT cut_through_type;
    VOID *pAd;
    PKT_TOKEN_QUEUE tx_id_list;
	NDIS_SPIN_LOCK rx_order_notify_lock;
    PKT_TOKEN_QUEUE rx_id_list;
	UINT32 TxRingLowWaterMark;
	UINT32 TxRingHighWaterMark;
	UINT32 TxTokenLowWaterMark;
	UINT32 TxTokenHighWaterMark;
	UINT8 TxFlowBlockState[NUM_OF_TX_RING];
	NDIS_SPIN_LOCK TxBlockLock[NUM_OF_TX_RING];
	DL_LIST TxBlockDevList[NUM_OF_TX_RING];
	UINT32 RxTokenLowWaterMark;
	UINT32 RxTokenHighWaterMark;
	UINT8 RxFlowBlockState;
	UINT32 TxRingFullCnt;
	UINT32 TxTokenFullCnt;
#ifdef CUT_THROUGH_DBG
	RALINK_TIMER_STRUCT TokenHistoryTimer;
	UINT8 TimeSlot;
#endif
} PKT_TOKEN_CB;

typedef struct _TX_BLOCK_DEV {
	DL_LIST list;
	PNET_DEV NetDev;
} TX_BLOCK_DEV;


PNDIS_PACKET cut_through_rx_deq(PKT_TOKEN_CB *pktTokenCb, UINT16 token, UINT8 *Type);
UINT16 cut_through_rx_enq(PKT_TOKEN_CB *pktTokenCb, PNDIS_PACKET pkt, UINT8 Type);
VOID cut_through_rx_pkt_assign(PKT_TOKEN_CB *pktTokenCb, UINT16 token, PNDIS_PACKET pkt);
PNDIS_PACKET cut_through_tx_deq(PKT_TOKEN_CB *pktTokenCb, UINT16 token, UINT8 *Type);
UINT16 cut_through_tx_enq(PKT_TOKEN_CB *pktTokenCb, PNDIS_PACKET pkt, UINT8 Type, NDIS_PHYSICAL_ADDRESS pkt_phy_addr, size_t pkt_len);
BOOLEAN cut_through_tx_state(PKT_TOKEN_CB *pktTokenCb, UINT8 State, UINT8 RingIdx);
INT32 cut_through_tx_flow_block(PKT_TOKEN_CB *pktTokenCb, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
UINT cut_through_rx_in_order(PKT_TOKEN_CB *pktTokenCb, UINT16 token);
UINT cut_through_rx_drop(PKT_TOKEN_CB *pktTokenCb, UINT16 token);
UINT cut_through_rx_rxdone(PKT_TOKEN_CB *pktTokenCb, UINT16 token);
INT cut_through_rx_mark_token_info(PKT_TOKEN_CB *pktTokenCb, UINT16 token, UINT8 drop);
INT cut_through_rx_mark_rxdone(PKT_TOKEN_CB *pktTokenCb, UINT16 token);
LONG cut_through_inorder_time(PKT_TOKEN_CB *pktTokenCb, UINT16 token);
INT cut_through_deinit(PKT_TOKEN_CB **ppktTokenCb);
INT cut_through_init(VOID **ppktTokenCb, VOID *pAd);
INT cut_through_set_mode(PKT_TOKEN_CB *pktTokenCb, UINT mode);
INT cut_through_get_mode(PKT_TOKEN_CB *pktTokenCb);

VOID dump_ct_token_list(PKT_TOKEN_CB *tokenCb, INT type);
VOID FastPathDequeBh(ULONG Param);

#endif /* ___CUT_THROUGH_H__ */

