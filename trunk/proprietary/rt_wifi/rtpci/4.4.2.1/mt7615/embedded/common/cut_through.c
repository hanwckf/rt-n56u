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
		cut_through.c

	Abstract:

	Note:
		All functions in this file must be PCI-depended, or you should move
		your functions to other files.

	Revision History:
	Who          When          What
	---------    ----------    ----------------------------------------------
*/

#include "rtmp_comm.h"
#include "cut_through.h"
#include "os/rt_linux_cmm.h"
#include "os/rt_drv.h"
#include "rt_os_util.h"
#include "rt_config.h"


#define INC_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}


VOID dump_ct_token_list(PKT_TOKEN_CB *tokenCb, INT type)
{
    PKT_TOKEN_QUEUE *token_q;
    PKT_TOKEN_LIST *token_list = NULL;
    INT idx;
    BOOLEAN dump;

    os_alloc_mem(NULL, (UCHAR **)&token_list, sizeof(PKT_TOKEN_LIST));

    if (!token_list)
    {
        printk("%s():alloc memory failed\n", __FUNCTION__);
        return;
    }

    if ((type & CUT_THROUGH_TYPE_TX) == CUT_THROUGH_TYPE_TX)
    {
        token_q= &tokenCb->tx_id_list;
    }
    else if ((type & CUT_THROUGH_TYPE_RX) == CUT_THROUGH_TYPE_RX)
    {
        token_q= &tokenCb->rx_id_list;
    }
    else
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Unkown type(%d)\n", __FUNCTION__, type));
        return;
    }

    RTMP_SEM_LOCK(&token_q->token_id_lock);
    if (token_q->token_inited == TRUE)
    {
        NdisCopyMemory(token_list, token_q->list, sizeof(PKT_TOKEN_LIST));
        dump = TRUE;
    }
    else
    {
        dump = FALSE;
    }
    RTMP_SEM_UNLOCK(&token_q->token_id_lock);


    if (dump == TRUE)
    {
        INT cnt = 0;

        printk("CutThrough Tx Token Queue Status:\n");
        printk("\tFree ID Head/Tail = %d/%d\n", token_list->id_head, token_list->id_tail);
        printk("\tFree ID Pool List:\n");
        for (idx = 0; idx < PKT_TX_TOKEN_ID_ARAY; idx++)
        {
            if (token_list->free_id[idx] != PKT_TOKEN_ID_INVALID)
            {
                if ((cnt % 8) == 0)
                    printk("\t\t");
                printk("ID[%d]=%d ", idx, token_list->free_id[idx]);
                if ((cnt % 8) == 7)
                    printk("\n");
                cnt++;
            }
        }
        printk("\tPkt Token Pool List:\n");
        for (idx = 0; idx < PKT_TX_TOKEN_ID_CNT; idx++)
        {
            if ((token_list->pkt_token[idx].pkt_buf != NULL) ||
                (token_list->pkt_token[idx].InOrder) ||
                (token_list->pkt_token[idx].rxDone))
            {
                printk("\t\tPktToken[%d]=0x%p, InOrder/rxDone=%d/%d\n",
                idx, token_list->pkt_token[idx].pkt_buf,
                token_list->pkt_token[idx].InOrder,
                token_list->pkt_token[idx].rxDone);
            }
        }
    }
    else
    {
        if ((type & CUT_THROUGH_TYPE_TX) == CUT_THROUGH_TYPE_TX)
        {
            printk("CutThrough TX Token Queue not init yet!\n");
        }
        else if ((type & CUT_THROUGH_TYPE_RX) == CUT_THROUGH_TYPE_RX)
        {
            printk("CutThrough RX Token Queue not init yet!\n");
        }
    }

    os_free_mem(token_list);
}


static INT cut_through_token_list_destroy(
    PKT_TOKEN_CB *pktTokenCb,
    PKT_TOKEN_QUEUE *token_q,
    INT type)
{
	INT idx;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);

	if (token_q->token_inited == TRUE)
	{

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): %p,%p\n",
			__FUNCTION__, token_q,&token_q->token_inited));
		
		RTMP_SEM_LOCK(&token_q->token_id_lock);
		token_q->token_inited = FALSE;
		RTMP_SEM_UNLOCK(&token_q->token_id_lock);
		
		for (idx = 0; idx < PKT_TX_TOKEN_ID_CNT; idx++)
		{
			PKT_TOKEN_ENTRY *entry = &token_q->list->pkt_token[idx];
			if (entry->pkt_buf) {
					
				if (type == CUT_THROUGH_TYPE_TX) {
					PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, 
										entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
				}

				RELEASE_NDIS_PACKET(pktTokenCb->pAd, entry->pkt_buf, NDIS_STATUS_FAILURE);
			}
		}
		os_free_mem(token_q->list);
		token_q->list = NULL;
		NdisFreeSpinLock(&token_q->token_id_lock);
	}

	return TRUE;
}


static INT cut_through_token_list_init(
    PKT_TOKEN_CB *pktTokenCb,
    PKT_TOKEN_QUEUE *token_q)
{
	PKT_TOKEN_LIST *token_list;
	INT idx;

	if (token_q->token_inited == FALSE) {
		NdisAllocateSpinLock(pktTokenCb->pAd, &token_q->token_id_lock);
		os_alloc_mem(pktTokenCb->pAd, (UCHAR **)&token_q->list, sizeof(PKT_TOKEN_LIST));
		if (token_q->list == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s(): AllocMem failed!\n", __FUNCTION__));
			NdisFreeSpinLock(&token_q->token_id_lock);
			return FALSE;
		}

		NdisZeroMemory(token_q->list, sizeof(PKT_TOKEN_LIST));
		token_list = token_q->list;
		token_list->id_head = 0;
		token_list->id_tail = PKT_TX_TOKEN_ID_CNT;
		for (idx = 0; idx < PKT_TX_TOKEN_ID_CNT; idx++) {
			token_list->free_id[idx] = idx;
		}
		token_list->free_id[PKT_TX_TOKEN_ID_CNT] = PKT_TOKEN_ID_INVALID;
		token_list->FreeTokenCnt = PKT_TX_TOKEN_ID_CNT;
		token_list->TotalTxUsedTokenCnt = 0;
		token_list->TotalTxBackTokenCnt = 0;
		token_list->TotalTxTokenEventCnt = 0;
		token_list->TotalTxTokenCnt = 0;

#ifdef CUT_THROUGH_DBG
		token_list->UsedTokenCnt = 0;
		token_list->BackTokenCnt = 0;
		token_list->FreeAgg0_31 = 0;
		token_list->FreeAgg32_63 = 0;
		token_list->FreeAgg64_95 = 0;
		token_list->FreeAgg96_127 = 0;
		token_list->DropPktCnt = 0;

		for (idx = 0; idx < TIME_SLOT_NUMS; idx++)
		{
			token_list->UsedTokenCntRec[idx] = 0;
			token_list->BackTokenCntRec[idx] = 0;
			token_list->FreeAgg0_31Rec[idx] = 0;
			token_list->FreeAgg32_63Rec[idx] = 0;
			token_list->FreeAgg64_95Rec[idx] = 0;
			token_list->FreeAgg96_127Rec[idx] = 0;
			token_list->DropPktCntRec[idx] = 0;
		}
#endif

		token_q->token_inited = TRUE;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): TokenList inited done!id_head/tail=%d/%d\n",
					__FUNCTION__, token_list->id_head, token_list->id_tail));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): %p,%p\n",
			__FUNCTION__, token_q,&token_q->token_inited));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): TokenList already inited!shall not happened!\n",
					__FUNCTION__));
		if (!token_q->list) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s(): TokenList is NULL!\n", __FUNCTION__));
			return FALSE;
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("\tlist.id_head=%d, list.id_tail=%d\n",
					token_q->list->id_head, token_q->list->id_tail));
	}

	return TRUE;
}

PNDIS_PACKET cut_through_rx_deq(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token,
	UINT8 *Type)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PNDIS_PACKET pkt_buf = NULL;
#ifdef CUT_THROUGH_DBG
	INT head[2] = {-1, -1}, tail[2] = {-1, -1};
#endif /* CUT_THROUGH_DBG */

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

	RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				PKT_TOKEN_ENTRY *entry = &token_list->pkt_token[token];

				pkt_buf = entry->pkt_buf;
				*Type = entry->Type;
				if (pkt_buf == NULL) {
					MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): buggy here? token ID(%d) without pkt!\n",
								__FUNCTION__, token));

					RTMP_SEM_UNLOCK(&token_q->token_id_lock);
					return pkt_buf;
				}
				entry->pkt_buf = NULL;
				entry->InOrder = FALSE;
				entry->rxDone= FALSE;
				entry->Drop = FALSE;
				entry->Type = TOKEN_NONE;
				token_list->free_id[token_list->id_tail] = token;
#ifdef CUT_THROUGH_DBG
				head[0] = token_list->id_head;
				tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
				INC_INDEX(token_list->id_tail, PKT_TX_TOKEN_ID_ARAY);
				token_list->FreeTokenCnt++;
				token_list->TotalTxBackTokenCnt++;
#ifdef CUT_THROUGH_DBG
				token_list->BackTokenCnt++;
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
			}
			else
			{
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */

	return pkt_buf;
}


UINT16 cut_through_rx_enq(
	PKT_TOKEN_CB *pktTokenCb,
	PNDIS_PACKET pkt,
	UINT8 Type)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	UINT16 idx = 0, token = PKT_TOKEN_ID_INVALID;
#ifdef CUT_THROUGH_DBG
	INT head[2] = {-1, -1}, tail[2] = {-1, -1};
#endif /* CUT_THROUGH_DBG */

	ASSERT(pkt);
	ASSERT(token_list);

	RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_q->list) {
#ifdef CUT_THROUGH_DBG
			head[0] = token_list->id_head;
			tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */

			idx = token_list->id_head;
			token = token_list->free_id[idx];
			if (token <= PKT_TX_TOKEN_ID_MAX) {
				if (token_list->pkt_token[token].pkt_buf)
				{
					RELEASE_NDIS_PACKET(pktTokenCb->pAd, token_list->pkt_token[token].pkt_buf, NDIS_STATUS_FAILURE);
				}

				token_list->pkt_token[token].pkt_buf = pkt;
				token_list->pkt_token[token].Type = Type;
				token_list->free_id[idx] = PKT_TOKEN_ID_INVALID;
				INC_INDEX(token_list->id_head, PKT_TX_TOKEN_ID_ARAY);
#ifdef CUT_THROUGH_DBG
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
				token_list->UsedTokenCnt++;
#endif /* CUT_THROUGH_DBG */
				token_list->FreeTokenCnt--;
				token_list->TotalTxUsedTokenCnt++;
			} else {
				token = PKT_TOKEN_ID_INVALID;
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */

	if (0) {
		printk("%s():Dump latest Free TokenList\n", __FUNCTION__);
		for  (idx =0; idx <PKT_TX_TOKEN_ID_ARAY; idx++) {
			printk("\ttoken_list->free_id[%d]=%d\n", idx, token_list->free_id[idx]);
			if (idx < PKT_TX_TOKEN_ID_CNT)
				printk("\ttoken_list->pkt_token[%d].pkt_buf=0x%p\n", idx, token_list->pkt_token[idx].pkt_buf);
		}
	}

	return token;
}


BOOLEAN cut_through_tx_state(PKT_TOKEN_CB *pktTokenCb, UINT8 State, UINT8 RingIdx)
{
	UINT8 Idx;

	if (RingIdx == NUM_OF_TX_RING)
	{
		for (Idx = 0; Idx < NUM_OF_TX_RING; Idx++)
		{
			RTMP_SEM_LOCK(&pktTokenCb->TxBlockLock[Idx]);
			if ((pktTokenCb->TxFlowBlockState[Idx] & State) == State)
			{
				RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[Idx]);
				return TRUE;
			}
			RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[Idx]);
		}
	}
	else
	{
		RTMP_SEM_LOCK(&pktTokenCb->TxBlockLock[RingIdx]);
		if ((pktTokenCb->TxFlowBlockState[RingIdx] & State) == State)
		{
			RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
			return TRUE;
		}
		RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
	}

	return FALSE;
}

VOID cut_through_rx_pkt_assign(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token,
    PNDIS_PACKET pkt)
{
    PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
#ifdef CUT_THROUGH_DBG
	INT head[2] = {-1, -1}, tail[2] = {-1, -1};
#endif /* CUT_THROUGH_DBG */

	ASSERT(pkt);
	ASSERT(token_list);

	RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_q->list) {
			if (token <= PKT_TX_TOKEN_ID_MAX) {
    			token_list->pkt_token[token].pkt_buf = pkt;
		    }
	    }
    }

    RTMP_SEM_UNLOCK(&token_q->token_id_lock);
    return;
}


INT32 cut_through_tx_flow_block(PKT_TOKEN_CB *pktTokenCb, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx)
{
	INT32 Ret = 0;

	if (Block == TRUE)
	{
		TX_BLOCK_DEV *TxBlockDev = NULL;

		RTMP_SEM_LOCK(&pktTokenCb->TxBlockLock[RingIdx]);
		pktTokenCb->TxFlowBlockState[RingIdx] |= State;

		DlListForEach(TxBlockDev, &pktTokenCb->TxBlockDevList[RingIdx], TX_BLOCK_DEV, list)
		{
			if (TxBlockDev->NetDev == NetDev)
			{
				RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
				return Ret;
			}
		}

		os_alloc_mem(NULL, (PUCHAR *)&TxBlockDev, sizeof(*TxBlockDev));

		if (!TxBlockDev)
		{
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
						("can not allocate TX_BLOCK_DEV\n"));
			RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
			return -1;
		}
		TxBlockDev->NetDev = NetDev;

		DlListAddTail(&pktTokenCb->TxBlockDevList[RingIdx], &TxBlockDev->list);

		RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
		RTMP_OS_NETDEV_STOP_QUEUE(NetDev);
	}
	else
	{
		TX_BLOCK_DEV *TxBlockDev = NULL;

		if (RingIdx == NUM_OF_TX_RING)
		{
			UINT8 Idx = 0;
			for (Idx = 0; Idx < NUM_OF_TX_RING; Idx++)
			{
				RTMP_SEM_LOCK(&pktTokenCb->TxBlockLock[Idx]);
				pktTokenCb->TxFlowBlockState[Idx] &= ~State;

				if (pktTokenCb->TxFlowBlockState[Idx] != 0)
				{
					RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[Idx]);
					continue;
				}

				while (1)
				{
					TxBlockDev = DlListFirst(&pktTokenCb->TxBlockDevList[Idx], TX_BLOCK_DEV, list);

					if (!TxBlockDev)
						break;

					DlListDel(&TxBlockDev->list);

					RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
					os_free_mem(TxBlockDev);
				}
				RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[Idx]);
			}
		}
		else
		{
			RTMP_SEM_LOCK(&pktTokenCb->TxBlockLock[RingIdx]);
			pktTokenCb->TxFlowBlockState[RingIdx] &= ~State;

			if (pktTokenCb->TxFlowBlockState[RingIdx] != 0)
			{
				RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
				return Ret;
			}

			while (1)
			{
				TxBlockDev = DlListFirst(&pktTokenCb->TxBlockDevList[RingIdx], TX_BLOCK_DEV, list);

				if (!TxBlockDev)
					break;

				DlListDel(&TxBlockDev->list);

				RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
				os_free_mem(TxBlockDev);
			}
			RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[RingIdx]);
		}
	}

	return Ret;
}

PNDIS_PACKET cut_through_tx_deq(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token,
	UINT8 *Type)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->tx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PNDIS_PACKET pkt_buf = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);
#ifdef CUT_THROUGH_DBG
	INT head[2] = {-1, -1}, tail[2] = {-1, -1};
#endif /* CUT_THROUGH_DBG */

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

	RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				PKT_TOKEN_ENTRY *entry = &token_list->pkt_token[token];

				pkt_buf = entry->pkt_buf;
				*Type = entry->Type;
				if (pkt_buf == NULL) {
					MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): buggy here? token ID(%d) without pkt!\n",
								__FUNCTION__, token));

					RTMP_SEM_UNLOCK(&token_q->token_id_lock);
					return pkt_buf;
				}
				entry->pkt_buf = NULL;
				PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
				entry->InOrder = FALSE;
				entry->rxDone= FALSE;
				entry->Drop = FALSE;
				entry->Type = TOKEN_NONE;
				token_list->free_id[token_list->id_tail] = token;
#ifdef CUT_THROUGH_DBG
				head[0] = token_list->id_head;
				tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
				INC_INDEX(token_list->id_tail, PKT_TX_TOKEN_ID_ARAY);
				token_list->FreeTokenCnt++;
				token_list->TotalTxBackTokenCnt++;
#ifdef CUT_THROUGH_DBG
				token_list->BackTokenCnt++;
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
			}
			else
			{
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */

	return pkt_buf;
}


UINT16 cut_through_tx_enq(
	PKT_TOKEN_CB *pktTokenCb,
	PNDIS_PACKET pkt,
	UINT8 Type,
	NDIS_PHYSICAL_ADDRESS pkt_phy_addr,
	size_t pkt_len)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->tx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	UINT16 idx = 0, token = PKT_TOKEN_ID_INVALID;
#ifdef CUT_THROUGH_DBG
	INT head[2] = {-1, -1}, tail[2] = {-1, -1};
#endif /* CUT_THROUGH_DBG */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);
	PKT_TOKEN_ENTRY *entry = NULL;

	ASSERT(pkt);
	ASSERT(token_list);

	RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_q->list) {
#ifdef CUT_THROUGH_DBG
			head[0] = token_list->id_head;
			tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */

			idx = token_list->id_head;
			token = token_list->free_id[idx];
			if (token <= PKT_TX_TOKEN_ID_MAX) {
				entry = &token_list->pkt_token[token];
				if (entry->pkt_buf)
				{
					PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pktTokenCb->pAd, entry->pkt_buf, NDIS_STATUS_FAILURE);
				}

				entry->pkt_buf = pkt;
				entry->Type = Type;
				entry->pkt_phy_addr = pkt_phy_addr;
				entry->pkt_len = pkt_len;
				token_list->free_id[idx] = PKT_TOKEN_ID_INVALID;
				INC_INDEX(token_list->id_head, PKT_TX_TOKEN_ID_ARAY);
#ifdef CUT_THROUGH_DBG
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
				token_list->UsedTokenCnt++;
#endif /* CUT_THROUGH_DBG */
				token_list->FreeTokenCnt--;
				token_list->TotalTxUsedTokenCnt++;
			} else {
				token = PKT_TOKEN_ID_INVALID;
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */

	if (0) {
		printk("%s():Dump latest Free TokenList\n", __FUNCTION__);
		for  (idx =0; idx <PKT_TX_TOKEN_ID_ARAY; idx++) {
			printk("\ttoken_list->free_id[%d]=%d\n", idx, token_list->free_id[idx]);
			if (idx < PKT_TX_TOKEN_ID_CNT)
				printk("\ttoken_list->pkt_token[%d].pkt_buf=0x%p\n", idx, token_list->pkt_token[idx].pkt_buf);
		}
	}

#ifdef CUT_THROUGH_DBG
	NdisGetSystemUpTime(&pktTokenCb->tx_id_list.list->pkt_token[token].startTime);
#endif
	
	return token;
}


UINT cut_through_rx_in_order(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry;
  	UINT inOrder = FALSE;

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

    RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				entry = &token_list->pkt_token[token];
    				inOrder = entry->InOrder;
			} else {
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->inOrder = %d\n",
				__FUNCTION__, token, inOrder));
#endif /* CUT_THROUGH_DBG */
    return inOrder;
}


UINT cut_through_rx_drop(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry;
  	UINT Drop = FALSE;

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

    RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				entry = &token_list->pkt_token[token];
    				Drop = entry->Drop;
			} else {
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->Drop = %d\n",
				__FUNCTION__, token, Drop));
#endif /* CUT_THROUGH_DBG */
    return Drop;
}


INT cut_through_rx_mark_token_info(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token,
	UINT8 Drop)
{
    PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
    PKT_TOKEN_ENTRY *entry = NULL;
	INT32 Ret = FALSE;

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

    RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				entry = &token_list->pkt_token[token];
    			if (Drop)
					entry->Drop = TRUE;
				else
					entry->InOrder = TRUE;
				NdisGetSystemUpTime(&entry->endTime);
				Ret = TRUE;
			} else {
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->inOrder=%d\n",
				__FUNCTION__, token, (entry != NULL ? entry->InOrder : 0xffffffff) ));
#endif /* CUT_THROUGH_DBG */
	return Ret;
}


INT cut_through_rx_mark_rxdone(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token)
{
    PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
    PKT_TOKEN_ENTRY *entry = NULL;
	INT32 Ret = FALSE;

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

    RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				entry = &token_list->pkt_token[token];
				entry->rxDone= TRUE;
				NdisGetSystemUpTime(&entry->startTime);
				Ret = TRUE;
			} else {
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->rxDone = %d\n",
				__FUNCTION__, token, (entry != NULL ? entry->rxDone : 0xffffffff)));
#endif /* CUT_THROUGH_DBG */
    return Ret;
}


LONG cut_through_inorder_time(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token)
{
    PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
    PKT_TOKEN_LIST *token_list = token_q->list;
    PKT_TOKEN_ENTRY *entry = NULL;
    LONG timer_interval = 0x7ffffff;

    ASSERT(token < PKT_TX_TOKEN_ID_CNT);

    RTMP_SEM_LOCK(&token_q->token_id_lock);
    if (token_q->token_inited == TRUE)
	{
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				entry = &token_list->pkt_token[token];
				timer_interval = entry->startTime - entry->endTime;
			} else {
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->inOrder=%d\n",
				__FUNCTION__, token, (entry != NULL ? entry->InOrder : 0xffffffff) ));
#endif /* CUT_THROUGH_DBG */
    return timer_interval;
}


UINT cut_through_rx_rxdone(
    PKT_TOKEN_CB *pktTokenCb,
    UINT16 token)
{
    PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
    PKT_TOKEN_ENTRY *entry;
    UINT rxDone = FALSE;

	ASSERT(token < PKT_TX_TOKEN_ID_CNT);

	RTMP_SEM_LOCK(&token_q->token_id_lock);
	if (token_q->token_inited == TRUE)
	{
		if (token_list) {
			if (token < PKT_TX_TOKEN_ID_CNT) {
				entry = &token_list->pkt_token[token];
    			rxDone = entry->rxDone;
			} else {
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __FUNCTION__, token));
			}
		}
	}
	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->rxDone = %d\n",
				__FUNCTION__, token, rxDone));
#endif /* CUT_THROUGH_DBG */
    return rxDone;
}


INT cut_through_deinit(PKT_TOKEN_CB **ppPktTokenCb)
{
	PKT_TOKEN_CB *pktTokenCb;
	TX_BLOCK_DEV *TxBlockDev = NULL;
	UINT32 Index;

#ifdef FAST_PATH_TXQ
	struct FastPathTxQueElement *FPTxElement;
#endif

	RTMP_ADAPTER *pAd;

#ifdef CUT_THROUGH_DBG
	BOOLEAN Cancelled;
#endif
	
	pktTokenCb = *ppPktTokenCb;
	
	if (pktTokenCb == NULL)
	{
		return TRUE;
	}

	pAd = (RTMP_ADAPTER *)pktTokenCb->pAd;

#ifdef FAST_PATH_TXQ
	pAd->bFastPathTaskSchedulable = FALSE;
	RTMP_OS_TASKLET_KILL(&pAd->FastPathDequeTask);
#endif
	cut_through_token_list_destroy(pktTokenCb, &pktTokenCb->tx_id_list, CUT_THROUGH_TYPE_TX);
	cut_through_token_list_destroy(pktTokenCb, &pktTokenCb->rx_id_list, CUT_THROUGH_TYPE_RX);
	
#ifdef CUT_THROUGH_DBG
	RTMPReleaseTimer(&pktTokenCb->TokenHistoryTimer, &Cancelled);
#endif

	NdisFreeSpinLock(&pktTokenCb->rx_order_notify_lock);

#ifdef FAST_PATH_TXQ
	while (1)
	{
		RTMP_SEM_LOCK(&pAd->FastPathTxQueLock);
		FPTxElement = DlListFirst(&pAd->FastPathTxQue, struct FastPathTxQueElement, List);

		if (!FPTxElement)
		{
			RTMP_SEM_UNLOCK(&pAd->FastPathTxQueLock);
			break;
		}
				
		DlListDel(&FPTxElement->List);
		RTMP_SEM_UNLOCK(&pAd->FastPathTxQueLock);

		RELEASE_NDIS_PACKET(pktTokenCb->pAd, FPTxElement->pPacket, NDIS_STATUS_FAILURE);
		FPTxElement->pPacket = NULL;
		
		RTMP_SEM_LOCK(&pAd->FastPathTxFreeQueLock);
		DlListAddTail(&pAd->FastPathTxFreeQue, &FPTxElement->List);
		pAd->FPTxElementFreeNum++;
		RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);
	}
	
	while (1)
	{
		RTMP_SEM_LOCK(&pAd->MgmtQueLock);
		FPTxElement = DlListFirst(&pAd->MgmtQue, struct FastPathTxQueElement, List);

		if (!FPTxElement)
		{
			RTMP_SEM_UNLOCK(&pAd->MgmtQueLock);
			break;
		}
				
		DlListDel(&FPTxElement->List);
		RTMP_SEM_UNLOCK(&pAd->MgmtQueLock);

		RELEASE_NDIS_PACKET(pktTokenCb->pAd, FPTxElement->pPacket, NDIS_STATUS_FAILURE);
		FPTxElement->pPacket = NULL;
		
		RTMP_SEM_LOCK(&pAd->FastPathTxFreeQueLock);
		DlListAddTail(&pAd->FastPathTxFreeQue, &FPTxElement->List);
		pAd->FPTxElementFreeNum++;
		RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);
	}

	while (1)
	{	
		RTMP_SEM_LOCK(&pAd->FastPathTxFreeQueLock);
		FPTxElement = DlListFirst(&pAd->FastPathTxFreeQue, struct FastPathTxQueElement, List);
		
		if (!FPTxElement)
		{
			RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);
			break;
		}

		DlListDel(&FPTxElement->List);
		pAd->FPTxElementFreeNum--;
		RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);

		os_free_mem(FPTxElement);
	}

#ifdef CONFIG_TX_DELAY
	hrtimer_cancel(&pAd->que_agg_timer);	
#endif
	NdisFreeSpinLock(&pAd->FastPathTxQueLock);
	NdisFreeSpinLock(&pAd->MgmtQueLock);
	NdisFreeSpinLock(&pAd->FastPathTxFreeQueLock);
#endif

	/* wake up netif queue once it is in stopped state.*/
	for (Index = 0; Index < NUM_OF_TX_RING; Index++)
	{
		RTMP_SEM_LOCK(&pktTokenCb->TxBlockLock[Index]);
		while (1)
		{
			TxBlockDev = DlListFirst(&pktTokenCb->TxBlockDevList[Index], TX_BLOCK_DEV, list);

			if (!TxBlockDev)
				break;

			DlListDel(&TxBlockDev->list);

			RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
			os_free_mem(TxBlockDev);
		}
		RTMP_SEM_UNLOCK(&pktTokenCb->TxBlockLock[Index]);
		NdisFreeSpinLock(&pktTokenCb->TxBlockLock[Index]);
	}

	if (pktTokenCb != NULL)
	{
		os_free_mem((VOID *)pktTokenCb);
		*ppPktTokenCb = NULL;
		return FALSE;
	}

	return TRUE;
}

#ifdef CUT_THROUGH_DBG
DECLARE_TIMER_FUNCTION(TokenHistoryExec);

VOID TokenHistoryExec(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)FunctionContext;
	PKT_TOKEN_LIST *tx_list = pktTokenCb->tx_id_list.list;
	PKT_TOKEN_LIST *rx_list = pktTokenCb->rx_id_list.list;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);

	tx_list->UsedTokenCntRec[pktTokenCb->TimeSlot] = tx_list->UsedTokenCnt;
	tx_list->UsedTokenCnt = 0;
	tx_list->BackTokenCntRec[pktTokenCb->TimeSlot] = tx_list->BackTokenCnt;
	tx_list->BackTokenCnt = 0;

	tx_list->FreeAgg0_31Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg0_31;
	tx_list->FreeAgg0_31 = 0;
	tx_list->FreeAgg32_63Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg32_63;
	tx_list->FreeAgg32_63 = 0;
	tx_list->FreeAgg64_95Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg64_95;
	tx_list->FreeAgg64_95 = 0;
	tx_list->FreeAgg96_127Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg96_127;
	tx_list->FreeAgg96_127 = 0;

	rx_list->UsedTokenCntRec[pktTokenCb->TimeSlot] = rx_list->UsedTokenCnt;
	rx_list->UsedTokenCnt = 0;
	rx_list->BackTokenCntRec[pktTokenCb->TimeSlot] = rx_list->BackTokenCnt;
	rx_list->BackTokenCnt = 0;
	
	rx_list->FreeAgg0_31Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg0_31;
	rx_list->FreeAgg0_31 = 0;
	rx_list->FreeAgg32_63Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg32_63;
	rx_list->FreeAgg32_63 = 0;
	rx_list->FreeAgg64_95Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg64_95;
	rx_list->FreeAgg64_95 = 0;
	rx_list->FreeAgg96_127Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg96_127;
	rx_list->FreeAgg96_127 = 0;
	
	rx_list->DropPktCntRec[pktTokenCb->TimeSlot] = rx_list->DropPktCnt;
	rx_list->DropPktCnt = 0;

	pAd->IsrTxCntRec[pktTokenCb->TimeSlot] = pAd->IsrTxCnt;
	pAd->IsrTxCnt = 0;
	pAd->IsrRxCntRec[pktTokenCb->TimeSlot] = pAd->IsrRxCnt;
	pAd->IsrRxCnt = 0;
	pAd->IsrRx1CntRec[pktTokenCb->TimeSlot] = pAd->IsrRx1Cnt;
	pAd->IsrRx1Cnt = 0;

	pAd->IoReadTxRec[pktTokenCb->TimeSlot] = pAd->IoReadTx;
	pAd->IoReadTx = 0;

	pAd->IoWriteTxRec[pktTokenCb->TimeSlot] = pAd->IoWriteTx;
	pAd->IoWriteTx = 0;

	pAd->IoReadRxRec[pktTokenCb->TimeSlot] = pAd->IoReadRx;
	pAd->IoReadRx = 0;

	pAd->IoWriteRxRec[pktTokenCb->TimeSlot] = pAd->IoWriteRx;
	pAd->IoWriteRx = 0;

	pAd->IoReadRx1Rec[pktTokenCb->TimeSlot] = pAd->IoReadRx1;
	pAd->IoReadRx1 = 0;

	pAd->IoWriteRx1Rec[pktTokenCb->TimeSlot] = pAd->IoWriteRx1;
	pAd->IoWriteRx1 = 0;

	pAd->MaxProcessCntRxRecA[pktTokenCb->TimeSlot] = pAd->RxMaxProcessCntA;
	pAd->RxMaxProcessCntA = 0;	
	
	pAd->MaxProcessCntRxRecB[pktTokenCb->TimeSlot] = pAd->RxMaxProcessCntB;
	pAd->RxMaxProcessCntB = 0;	
	
	pAd->MaxProcessCntRxRecC[pktTokenCb->TimeSlot] = pAd->RxMaxProcessCntC;
	pAd->RxMaxProcessCntC = 0;	
	
	pAd->MaxProcessCntRxRecD[pktTokenCb->TimeSlot] = pAd->RxMaxProcessCntD;
	pAd->RxMaxProcessCntD = 0;	
	
	pAd->MaxProcessCntRx1RecA[pktTokenCb->TimeSlot] = pAd->Rx1MaxProcessCntA;
	pAd->Rx1MaxProcessCntA = 0;
	
	pAd->MaxProcessCntRx1RecB[pktTokenCb->TimeSlot] = pAd->Rx1MaxProcessCntB;
	pAd->Rx1MaxProcessCntB = 0;
	
	pAd->MaxProcessCntRx1RecC[pktTokenCb->TimeSlot] = pAd->Rx1MaxProcessCntC;
	pAd->Rx1MaxProcessCntC = 0;
	
	pAd->MaxProcessCntRx1RecD[pktTokenCb->TimeSlot] = pAd->Rx1MaxProcessCntD;
	pAd->Rx1MaxProcessCntD = 0;

	pktTokenCb->TimeSlot++;
	pktTokenCb->TimeSlot = pktTokenCb->TimeSlot % TIME_SLOT_NUMS;
}
BUILD_TIMER_FUNCTION(TokenHistoryExec);
#endif


INT cut_through_init(VOID **ppPktTokenCb, VOID *pAd)
{
	PKT_TOKEN_CB *pktTokenCb;
#ifdef FAST_PATH_TXQ
	struct FastPathTxQueElement *FPTxElement;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)pAd;
#endif
	ULONG Flags;
	UINT32 Index;

	os_alloc_mem(pAd, (UCHAR **)&pktTokenCb, sizeof(PKT_TOKEN_CB));
	if (pktTokenCb == NULL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s os_alloc_mem fail\n",
				__FUNCTION__));
        return FALSE;
	}

	NdisZeroMemory(pktTokenCb, sizeof(PKT_TOKEN_CB));
	pktTokenCb->pAd = pAd;

	NdisAllocateSpinLock(pktTokenCb->pAd, &pktTokenCb->rx_order_notify_lock);
	ad->wrong_wlan_idx_num = 0;

#ifdef FAST_PATH_TXQ
	NdisAllocateSpinLock(ad, &ad->FastPathTxQueLock);
	NdisAllocateSpinLock(ad, &ad->MgmtQueLock);
	DlListInit(&ad->FastPathTxQue);
	DlListInit(&ad->MgmtQue);
	NdisAllocateSpinLock(ad, &ad->FastPathTxFreeQueLock);
	DlListInit(&ad->FastPathTxFreeQue);

	ad->FastPathTxQueNum = 0;
#ifdef CONFIG_TX_DELAY
	ad->TxProcessBatchCnt = 4;
	ad->force_deq = FALSE;
	ad->que_agg_en = FALSE;
	ad->que_agg_timeout_value = QUE_AGG_TIMEOUT;
	ad->min_pkt_len = MIN_AGG_PKT_LEN;
	hrtimer_init(&ad->que_agg_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ad->que_agg_timer.function = que_agg_timeout;
#endif
	ad->FPTxElementFullNum = 0;
	ad->MgmtQueNum = 0;
	ad->FPTxElementFreeNum = 0;
#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 1)
	ad->fp_txBlocked = 0;
#endif

	for (Index = 0; Index < FP_TX_FREE_NUM; Index++)
	{
		os_alloc_mem(NULL, (PUCHAR *)&FPTxElement, sizeof(*FPTxElement));
		
		if (!FPTxElement) 
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("can not allocate FPTxElement\n"));
			return FALSE;
		}

		RTMP_SEM_LOCK(&ad->FastPathTxFreeQueLock);	
		DlListAddTail(&ad->FastPathTxFreeQue, &FPTxElement->List);
		ad->FPTxElementFreeNum++;
		RTMP_SEM_UNLOCK(&ad->FastPathTxFreeQueLock);
	}

	ad->MinFPTxElementFreeNum = ad->FPTxElementFreeNum;
#endif

	cut_through_token_list_init(pktTokenCb, &pktTokenCb->tx_id_list);
	cut_through_token_list_init(pktTokenCb, &pktTokenCb->rx_id_list);

#ifdef RX_CUT_THROUGH
	pktTokenCb->cut_through_type = CUT_THROUGH_TYPE_BOTH;
#else
	pktTokenCb->cut_through_type = CUT_THROUGH_TYPE_TX;
#endif
	*ppPktTokenCb = pktTokenCb;

	pktTokenCb->TxRingLowWaterMark = 5;
	pktTokenCb->TxRingHighWaterMark = 5;
	pktTokenCb->TxTokenLowWaterMark = 211;
	pktTokenCb->TxTokenHighWaterMark = pktTokenCb->TxTokenLowWaterMark + 5;  
	pktTokenCb->RxTokenLowWaterMark = 5;
	pktTokenCb->RxTokenHighWaterMark = pktTokenCb->RxTokenLowWaterMark * 2;
	pktTokenCb->RxFlowBlockState = 0;

	for (Index = 0; Index < NUM_OF_TX_RING; Index++)
	{
		pktTokenCb->TxFlowBlockState[Index] = 0;
		NdisAllocateSpinLock(pAd, &pktTokenCb->TxBlockLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&pktTokenCb->TxBlockLock[Index], &Flags);
		DlListInit(&pktTokenCb->TxBlockDevList[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pktTokenCb->TxBlockLock[Index], &Flags);
	}

	pktTokenCb->TxRingFullCnt = 0;
	pktTokenCb->TxTokenFullCnt = 0;

#ifdef FAST_PATH_TXQ
	RTMP_OS_TASKLET_INIT(ad, &ad->FastPathDequeTask, FastPathDequeBh, (unsigned long)ad);
	ad->bFastPathTaskSchedulable = TRUE;
#endif

#ifdef CUT_THROUGH_DBG
	pktTokenCb->TimeSlot = 0;
	RTMPInitTimer(pAd, &pktTokenCb->TokenHistoryTimer, GET_TIMER_FUNCTION(TokenHistoryExec), pktTokenCb, TRUE);
	RTMPSetTimer(&pktTokenCb->TokenHistoryTimer, 1000);
	
	ad->IsrTxCnt = 0;
	ad->IsrRxCnt = 0;
	ad->IsrRx1Cnt = 0;
	ad->IoReadTx = 0;
	ad->IoWriteTx = 0;
	ad->IoReadRx = 0;
	ad->IoWriteRx = 0;
	ad->IoReadRx1 = 0;
	ad->IoWriteRx1 = 0;
	ad->RxMaxProcessCntA = 0;
	ad->RxMaxProcessCntB = 0;
	ad->RxMaxProcessCntC = 0;
	ad->RxMaxProcessCntD = 0;
	ad->Rx1MaxProcessCntA = 0;
	ad->Rx1MaxProcessCntB = 0;
	ad->Rx1MaxProcessCntC = 0;
	ad->Rx1MaxProcessCntD = 0;
	ad->RxDropPacket = 0;
	
	for (Index = 0; Index < TIME_SLOT_NUMS; Index++)
	{
		ad->IsrTxCntRec[Index] = 0;
		ad->IsrRxCntRec[Index] = 0;
		ad->IsrRx1CntRec[Index] = 0;
		ad->IoReadTxRec[Index] = 0;
		ad->IoWriteTxRec[Index] = 0;
		ad->IoReadRxRec[Index] = 0;
		ad->IoWriteRxRec[Index] = 0;
		ad->IoReadRx1Rec[Index] = 0;
		ad->IoWriteRx1Rec[Index] = 0;
		ad->MaxProcessCntRxRecA[Index] = 0;
		ad->MaxProcessCntRxRecB[Index] = 0;
		ad->MaxProcessCntRxRecC[Index] = 0;
		ad->MaxProcessCntRxRecD[Index] = 0;
		ad->MaxProcessCntRx1RecA[Index] = 0;
		ad->MaxProcessCntRx1RecB[Index] = 0;
		ad->MaxProcessCntRx1RecC[Index] = 0;
		ad->MaxProcessCntRx1RecD[Index] = 0;
	}
#endif

    return TRUE;
}


INT cut_through_set_mode(
    PKT_TOKEN_CB *pktTokenCb,
    UINT mode)
{
    if (pktTokenCb == NULL)
        return 0;

	if (mode <= CUT_THROUGH_TYPE_BOTH)
		pktTokenCb->cut_through_type = mode;
	else
		pktTokenCb->cut_through_type = 0;


    return 0;
}


INT cut_through_get_mode(
    PKT_TOKEN_CB *pktTokenCb)
{
    return pktTokenCb->cut_through_type;
}


INT Set_CtLowWaterMark_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 val;
    PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB*)pAd->PktTokenCb;

    val = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("Set CtLowWaterMark: %d\n",val));


	pktTokenCb->TxTokenLowWaterMark = val;
	pktTokenCb->TxTokenHighWaterMark = pktTokenCb->TxTokenLowWaterMark * 2;  
	pktTokenCb->RxTokenLowWaterMark = val;
	pktTokenCb->RxTokenHighWaterMark = pktTokenCb->RxTokenLowWaterMark * 2;

	return TRUE;
}

