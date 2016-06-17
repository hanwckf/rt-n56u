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
	andes_mt.c
*/

#include	"rt_config.h"




#ifdef RTMP_PCI_SUPPORT
INT32 AndesMTPciKickOutCmdMsg(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	FreeNum = GET_CTRLRING_FREENO(pAd);

	if (FreeNum == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
		__FUNCTION__, pAd->CtrlRing.TxCpuIdx, pAd->CtrlRing.TxDmaIdx, pAd->CtrlRing.TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->CtrlRingLock, &flags);

	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CtrlRingLock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pAd->CtrlRing.TxCpuIdx;

#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pAd->CtrlRing.Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
 	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)pAd->CtrlRing.Cell[SwIdx].AllocVa;
#endif

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	pAd->CtrlRing.Cell[SwIdx].pNdisPacket = net_pkt;
	pAd->CtrlRing.Cell[SwIdx].pNextNdisPacket = NULL;

	pAd->CtrlRing.Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA) , (SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pAd->CtrlRing.Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pAd->CtrlRing.Cell[SwIdx].AllocPa, TXD_SIZE);

   	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pAd->CtrlRing.TxCpuIdx, MGMT_RING_SIZE);

	if (msg->need_wait)
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
	else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CtrlRingLock, &flags);
		return -1;
	}

	RTMP_IO_WRITE32(pAd, pAd->CtrlRing.hw_cidx_addr, pAd->CtrlRing.TxCpuIdx);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CtrlRingLock, &flags);

	return ret;
}

#ifdef MT7615
INT32 AndesMTPciKickOutCmdMsg2(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
    RTMP_RING * pRing;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

    pRing = (RTMP_RING *)(&(pAd->FwDwloRing));
	FreeNum = GET_FWDWLORING_FREENO(pRing);

	if (FreeNum == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
		__FUNCTION__, pRing->TxCpuIdx, pRing->TxDmaIdx, pRing->TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pRing->RingLock, &flags);

	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->RingLock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pRing->TxCpuIdx;

#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pRing->Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
 	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)pRing->Cell[SwIdx].AllocVa;
#endif

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

    hex_dump("host 2 N9: Cmd:",GET_OS_PKT_DATAPTR(net_pkt),GET_OS_PKT_LEN(net_pkt));
	pRing->Cell[SwIdx].pNdisPacket = net_pkt;
	pRing->Cell[SwIdx].pNextNdisPacket = NULL;
 	pRing->Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA) , (SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pRing->Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pRing->Cell[SwIdx].AllocPa, TXD_SIZE);

   	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pRing->TxCpuIdx, MGMT_RING_SIZE);

	if (msg->need_wait)
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
	else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->RingLock, &flags);
		return -1;
	}

	RTMP_IO_WRITE32(pAd, pRing->hw_cidx_addr, pRing->TxCpuIdx);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->RingLock, &flags);

	return ret;
}
#endif /* MT7615 */
#endif /* RTMP_PCI_SUPPORT */

VOID AndesMTFillCmdHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	FW_TXD *fw_txd = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;

	if ((Ctl->Stage == FW_NO_INIT) || (Ctl->Stage == FW_DOWNLOAD) || (Ctl->Stage == ROM_PATCH_DOWNLOAD))
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, 12);
	else if (Ctl->Stage == FW_RUN_TIME)
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*fw_txd));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Unknown Control Stage(%d)\n", __FUNCTION__,
								Ctl->Stage));

	fw_txd->fw_txd_0.field.length = GET_OS_PKT_LEN(net_pkt);
	fw_txd->fw_txd_0.field.pq_id = msg->pq_id;
	fw_txd->fw_txd_1.field.cid = msg->cmd_type;
	fw_txd->fw_txd_1.field.pkt_type_id = PKT_ID_CMD;
	fw_txd->fw_txd_1.field.set_query = msg->set_query;
#ifdef MT7615
    fw_txd->fw_txd_1.field1.seq_num = msg->seq;
    fw_txd->fw_txd_1.field1.pkt_ft = 0x2;
#else
    fw_txd->fw_txd_1.field.seq_num = msg->seq;
#endif /* MT7615 */
	fw_txd->fw_txd_2.field.ext_cid =  msg->ext_cmd_type;

	if ((msg->cmd_type == EXT_CID) && ((msg->set_query == CMD_SET) || (msg->set_query == CMD_QUERY))
									&& (msg->need_rsp == TRUE))
	{
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NEED_ACK;
	} else {
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NO_NEED_ACK;
	}

	fw_txd->fw_txd_0.word = cpu2le32(fw_txd->fw_txd_0.word);
	fw_txd->fw_txd_1.word = cpu2le32(fw_txd->fw_txd_1.word);
	fw_txd->fw_txd_2.word = cpu2le32(fw_txd->fw_txd_2.word);

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_CMD_INFO(fw_txd->fw_txd_0.field.length, fw_txd->fw_txd_0.field.pq_id,
						fw_txd->fw_txd_1.field.cid, fw_txd->fw_txd_1.field.pkt_type_id,
						fw_txd->fw_txd_1.field.set_query, fw_txd->fw_txd_1.field.seq_num,
						fw_txd->fw_txd_2.field.ext_cid, fw_txd->fw_txd_2.field.ext_cid_option,
						(char *)(GET_OS_PKT_DATAPTR(net_pkt)), GET_OS_PKT_LEN(net_pkt));
#endif /* CONFIG_TRACE_SUPPORT */
}


INT32 CmdInitAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: address = %x, data = %x\n", __FUNCTION__, address, data));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _INIT_CMD_ACCESS_REG));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, INIT_CMD_ACCESS_REG, CMD_SET, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.ucSetQuery = 1;
	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdInitAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _INIT_EVENT_ACCESS_REG *access_reg = (struct _INIT_EVENT_ACCESS_REG *)data;

	NdisMoveMemory(msg->rsp_payload, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->rsp_payload)) = le2cpu32(*((UINT32 *)msg->rsp_payload));
}


INT32 CmdInitAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: address = %x\n", __FUNCTION__, address));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _INIT_CMD_ACCESS_REG));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, INIT_CMD_ACCESS_REG, CMD_QUERY, EXT_CMD_NA, TRUE, 0,
							TRUE, TRUE, 8, (CHAR *)data, CmdInitAccessRegReadCb);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.ucSetQuery = 0;
	access_reg.u4Address = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdReStartDLRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case WIFI_FW_DOWNLOAD_SUCCESS:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFI FW Download Success\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_PARAM:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Invalid Parameter\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_CRC:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Invalid CRC\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_DECRYPTION_FAIL:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Decryption Fail\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_UNKNOWN_CMD:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Unknown CMD\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_TIMEOUT:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Timeout\n", __FUNCTION__));
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Unknow Status(%d)\n", __FUNCTION__, Status));
			break;
	}
}


static VOID CmdSecKeyRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_SEC_ADDREMOVE_STRUC_T EvtSecKey;
	UINT32 Status;
	UINT32 WlanIndex;

	EvtSecKey = (struct _EVENT_SEC_ADDREMOVE_STRUC_T *)Data;

	Status = le2cpu32(EvtSecKey->u4Status);
	WlanIndex = le2cpu32(EvtSecKey->u4WlanIdx);

	if (Status != 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, error set key, wlan idx(%d), status: 0x%x\n", __FUNCTION__, WlanIndex, Status));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, wlan idx(%d), status: 0x%x\n", __FUNCTION__, WlanIndex, Status));
	}
}

static VOID CmdPsRetrieveRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{

	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsCapatibility;
	UINT32 Status;

    EvtPsCapatibility = (P_EXT_EVENT_AP_PS_RETRIEVE_T)Data;
        Status = le2cpu32(EvtPsCapatibility->u4Param1);

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Disable FW PS Supportstatus:%x !!!!!\n",__FUNCTION__,Status));

}

#ifdef MT_PS
#if defined(MT7603) || defined(MT7628)
static VOID CmdPsRetrieveStartRspFromCR(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	MAC_TABLE_ENTRY *pEntry;

	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsRetrieveStart;
	UINT32 WlanIdx;
	STA_TR_ENTRY *tr_entry;
	NDIS_STATUS token_status;
	unsigned char q_idx;
	struct tx_swq_fifo *ps_fifo_swq;
	UINT deq_qid;

   EvtPsRetrieveStart = (P_EXT_EVENT_AP_PS_RETRIEVE_T)Data;
   WlanIdx = le2cpu32(EvtPsRetrieveStart->u4Param1);

	if (!(VALID_WCID(WlanIdx))) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s INVALID_MAC_WCID(%d)\n", __FUNCTION__, WlanIdx));
		goto NEXT;
   }
	pEntry = &pAd->MacTab.Content[WlanIdx];

	if (IS_ENTRY_NONE(pEntry))
	{
		MtPsRedirectDisableCheck(pAd, WlanIdx);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s Entry(wcid=%d) left.\n", __FUNCTION__, WlanIdx));
		goto NEXT;
	}

	if (!(VALID_TR_WCID(WlanIdx))) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s INVALID_TR_WCID(%d)\n", __FUNCTION__, WlanIdx));
		goto NEXT;
	}
	tr_entry = &pAd->MacTab.tr_entry[WlanIdx];
	
	if (tr_entry->ps_state != APPS_RETRIEVE_START_PS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s Entry(wcid=%d) ps state(%d) is not APPS_RETRIEVE_START_PS\n", __FUNCTION__, WlanIdx, tr_entry->ps_state));	
		goto NEXT;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_PS, ("---->%s: Start to send TOKEN frames.\n", __FUNCTION__));
	tr_entry->ps_state = APPS_RETRIEVE_GOING;
	CheckSkipTX(pAd, pEntry);
	tr_entry->ps_qbitmap = 0;

	for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
	{
		UINT16 IsEmpty = IS_TXRING_EMPTY(pAd, q_idx);

		if (!IsEmpty)
		{
			token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);
			if (!token_status)
				tr_entry->ps_qbitmap |= (1 << q_idx);
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("%s(%d) Fail:	Send TOKEN Frame, AC=%d\n", __FUNCTION__, __LINE__, q_idx)); 
	}
	}

	if (tr_entry->ps_qbitmap == 0)
	{
		q_idx = QID_AC_VO;

		token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);
		
		if (!token_status)
			tr_entry->ps_qbitmap |= (1 << q_idx);
	}

	if (tr_entry->ps_qbitmap == 0)
	{
		tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
		tr_entry->token_enq_all_fail = TRUE;
		RTEnqueueInternalCmd(pAd, CMDTHREAD_PS_CLEAR, (VOID *)&WlanIdx, sizeof(UINT32));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN | DBG_FUNC_PS, ("%s(%d): (ps_state = %d) token_enq_all_fail!! ==> send CMDTHREAD_PS_CLEAR cmd.\n", 
			__FUNCTION__, __LINE__, tr_entry->ps_state)); 
	}
	else
	{
		tr_entry->token_enq_all_fail = FALSE;
	}

NEXT:
	ps_fifo_swq = &pAd->apps_cr_q;
	deq_qid = ps_fifo_swq->deqIdx;
	while (ps_fifo_swq->swq[deq_qid] != 0) {
		WlanIdx = ps_fifo_swq->swq[deq_qid];
		pEntry = &pAd->MacTab.Content[WlanIdx];
   tr_entry = &pAd->MacTab.tr_entry[WlanIdx];

		if (pEntry->PsMode == PWR_ACTIVE) {
			ps_fifo_swq->swq[deq_qid]  = 0;
			INC_RING_INDEX(ps_fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);	
			tr_entry->ps_state = APPS_RETRIEVE_IDLE;
			MtHandleRxPsPoll(pAd, &pEntry->Addr[0], WlanIdx, TRUE);
			deq_qid = ps_fifo_swq->deqIdx;
		} else {
			if (MtStartPSRetrieve(pAd, ps_fifo_swq->swq[deq_qid]) == TRUE) {
				ps_fifo_swq->swq[deq_qid]  = 0;
				INC_RING_INDEX(ps_fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);	
				tr_entry->ps_state = APPS_RETRIEVE_START_PS;
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("=======> Error %s(%d) wcid=%d,  tr_entry->ps_state=%d\n", __FUNCTION__, __LINE__, ps_fifo_swq->swq[deq_qid], tr_entry->ps_state)); 
			}
			break;			
		}
	}
}
#endif /* MT7603 || MT7628 */


static VOID CmdPsRetrieveStartRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
	{
	MAC_TABLE_ENTRY *pEntry;

	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsRetrieveStart;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	UINT32 WlanIdx;
	STA_TR_ENTRY *tr_entry;
	NDIS_STATUS token_status;
	unsigned char q_idx;

	EvtPsRetrieveStart = (P_EXT_EVENT_AP_PS_RETRIEVE_T)Data;
	WlanIdx = le2cpu32(EvtPsRetrieveStart->u4Param1);

	if (!(VALID_WCID(WlanIdx))) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s INVALID_MAC_WCID(WlanIndex)\n", __FUNCTION__));
      return;
   }
	pEntry = &pAd->MacTab.Content[WlanIdx];

	if (IS_ENTRY_NONE(pEntry))
	{
		MtPsRedirectDisableCheck(pAd, WlanIdx);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s Entry(wcid=%d) left.\n", __FUNCTION__, WlanIdx));
		return;
	}

	if (!(VALID_TR_WCID(WlanIdx))) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s INVALID_TR_WCID(WlanIndex)\n", __FUNCTION__));
	  	return;
	}
	tr_entry = &pAd->MacTab.tr_entry[WlanIdx];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_PS, ("---->%s: Start to send TOKEN frames.\n", __FUNCTION__));

   tr_entry->ps_state = APPS_RETRIEVE_GOING;

   tr_entry->ps_qbitmap = 0;

	for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
	{
		UINT16 IsEmpty = IS_TXRING_EMPTY(pAd, q_idx);

		if (!IsEmpty)
		{	
		token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);
		if (!token_status)
			tr_entry->ps_qbitmap |= (1 << q_idx);
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d) Fail:	Send TOKEN Frame, AC=%d\n", __FUNCTION__, __LINE__, q_idx));
	}
	}

	if (tr_entry->ps_qbitmap == 0)
	{
		q_idx = QID_AC_VO;
		token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);		
		
		if (!token_status)
			tr_entry->ps_qbitmap |= (1 << q_idx);
	}


	if (tr_entry->ps_qbitmap == 0)
	{
		tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
		tr_entry->token_enq_all_fail = TRUE;
		RTEnqueueInternalCmd(pAd, CMDTHREAD_PS_CLEAR, (VOID *)&WlanIdx, sizeof(UINT32));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN | DBG_FUNC_PS, ("%s(%d): (ps_state = %d) token_enq_all_fail!! ==> send CMDTHREAD_PS_CLEAR cmd.\n",
			__FUNCTION__, __LINE__, tr_entry->ps_state));
	}
	else
	{
		tr_entry->token_enq_all_fail = FALSE;
	}
}


static VOID CmdPsClearRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
        MAC_TABLE_ENTRY *pEntry;
	P_CMD_AP_PS_CLEAR_STRUC_T EvtPsClear;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	STA_TR_ENTRY *tr_entry;
	//struct wtbl_entry tb_entry;
	//union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	UINT32 WlanIndex;
	UINT32 q_idx = QID_AC_BE;

	EvtPsClear = (struct _CMD_AP_PS_CLEAR_STRUC_T *)Data;
	WlanIndex = le2cpu32(EvtPsClear->u4WlanIdx);
        pEntry = &ad->MacTab.Content[WlanIndex];
	tr_entry = &ad->MacTab.tr_entry[WlanIndex];


	if (ad->MacTab.tr_entry[WlanIndex].PsMode == PWR_ACTIVE)
      tr_entry->ps_state = APPS_RETRIEVE_IDLE;
   else
      tr_entry->ps_state = APPS_RETRIEVE_DONE;

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_PS, ("wcid=%d, Receive Event of CmdPsClear tr_entry->ps_state=%d\n", WlanIndex,tr_entry->ps_state));

	if(tr_entry->token_enq_all_fail)
	{
		tr_entry->token_enq_all_fail = FALSE;

		if (tr_entry->ps_queue.Number)
			MtEnqTxSwqFromPsQueue(ad, q_idx, tr_entry);

		for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
			tr_entry->TokenCount[q_idx] = tr_entry->tx_queue[q_idx].Number;
	}

#ifdef RTMP_MAC_PCI
#ifdef DOT11_N_SUPPORT
   SendRefreshBAR(ad, pEntry);
#endif /* DOT11_N_SUPPORT */
#endif /* RTMP_MAC_PCI */
   if (tr_entry->ps_state == APPS_RETRIEVE_IDLE)
      MtHandleRxPsPoll(ad, &pEntry->Addr[0], WlanIndex, TRUE);
#ifdef UAPSD_SUPPORT
	else
	{
		if (tr_entry->bEospNullSnd)
		{
			UINT32	AcQueId;

			tr_entry->bEospNullSnd = FALSE;
			/* sanity Check for UAPSD condition */
			if (tr_entry->EospNullUp >= 8)
				tr_entry->EospNullUp = 1; /* shout not be here */

			/* get the AC ID of incoming packet */
			AcQueId = WMM_UP2AC_MAP[tr_entry->EospNullUp];

			/* bQosNull = bEOSP = TRUE = 1 */

			/*
				Use management queue to tx QoS Null frame to avoid delay so
				us_of_frame is not used.
			*/

#ifdef MT_PS
			if (pEntry->i_psm == I_PSM_DISABLE)
			{
				MtSetIgnorePsm(ad, pEntry, I_PSM_ENABLE);
			}
#endif /* MT_PS */

			RtmpEnqueueNullFrame(ad, pEntry->Addr, pEntry->CurrTxRate,
								pEntry->Aid, pEntry->func_tb_idx, TRUE, TRUE, tr_entry->EospNullUp);

			RTMPDeQueuePacket(ad, TRUE, AcQueId, pEntry->wcid, 1);
#ifdef UAPSD_DEBUG
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: send a EOSP QoS Null frame!\n", __FUNCTION__));
#endif /* UAPSD_DEBUG */
		}
		else if (pEntry->UAPSDTxNum != 0)
		{
			RTMPDeQueuePacket(ad, TRUE, NUM_OF_TX_RING, pEntry->wcid, pEntry->UAPSDTxNum);
		}
	}
#endif /* UAPSD_SUPPORT */
}
#endif /* MT_PS */

static VOID CmdStartDLRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case WIFI_FW_DOWNLOAD_SUCCESS:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFI FW Download Success\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_PARAM:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Invalid Parameter\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_CRC:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Invalid CRC\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_DECRYPTION_FAIL:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Decryption Fail\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_UNKNOWN_CMD:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Unknown CMD\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_TIMEOUT:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi FW Download Timeout\n", __FUNCTION__));
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Unknow Status(%d)\n", __FUNCTION__, Status));
			break;
	}
}

INT32 CmdSecKeyReq(RTMP_ADAPTER *ad, UINT8 AddRemove, UINT8 Keytype, UINT8 *pAddr, UINT8 Alg, UINT8 KeyID, UINT8 KeyLen, UINT8 WlanIdx, UINT8 *KeyMaterial)
{
	struct cmd_msg *msg;
	struct _CMD_SEC_ADDREMOVE_KEY_STRUC_T CmdSecKey;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_SEC_ADDREMOVE_KEY_STRUC_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_SEC_ADDREMOVE_KEY, FALSE, 0, FALSE, TRUE, sizeof(struct _EVENT_SEC_ADDREMOVE_STRUC_T), NULL, CmdSecKeyRsp);

	memset(&CmdSecKey, 0x00, sizeof(CmdSecKey));

	CmdSecKey.ucAddRemove = AddRemove;
	CmdSecKey.ucKeyType = Keytype;
	memcpy(CmdSecKey.aucPeerAddr, pAddr, 6);
	CmdSecKey.ucAlgorithmId = Alg;
	CmdSecKey.ucKeyId = KeyID;
	CmdSecKey.ucKeyLen = KeyLen;
	memcpy(CmdSecKey.aucKeyMaterial, KeyMaterial, KeyLen);
	CmdSecKey.ucWlanIndex = WlanIdx;
	AndesAppendCmdMsg(msg, (char *)&CmdSecKey, sizeof(CmdSecKey));

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

INT32 CmdPsRetrieveReq(RTMP_ADAPTER *ad, UINT32 enable)
{
   	struct cmd_msg *msg;
      struct _CMD_AP_PS_RETRIEVE_T CmdPsCapatibility;

      int ret = 0;

      msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_AP_PS_RETRIEVE_T));

      if (!msg)
      {
         ret = NDIS_STATUS_RESOURCES;
         goto error;
      }

      AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_AP_PWR_SAVING_CAPABILITY, TRUE, 0, TRUE, TRUE, sizeof(struct _CMD_AP_PS_RETRIEVE_T), NULL, CmdPsRetrieveRsp);

      NdisZeroMemory(&CmdPsCapatibility, sizeof(CmdPsCapatibility));

      CmdPsCapatibility.u4Option=  cpu2le32(0);
      CmdPsCapatibility.u4Param1=  cpu2le32(enable);

      AndesAppendCmdMsg(msg, (char *)&CmdPsCapatibility, sizeof(CmdPsCapatibility));
      ret = AndesSendCmdMsg(ad, msg);

error:
      MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
   	return ret;
}

#ifdef MT_PS
INT32 CmdPsRetrieveStartReq(RTMP_ADAPTER *ad, UINT32 WlanIdx)
{
   struct cmd_msg *msg;
   struct _EXT_CMD_AP_PWS_START_T CmdApPwsStart;

   int ret = 0;

   /*how to handle memory allocate failure? */
   msg = AndesAllocCmdMsg(ad, sizeof(struct _EXT_CMD_AP_PWS_START_T));
   if (!msg)
   {
      ret = NDIS_STATUS_RESOURCES;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("%s:(ret = %d)\n", __FUNCTION__, ret));
      goto error;
   }

   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_PS, ("%s(%d): RTEnqueueInternalCmd comming!! WlanIdx: %x\n",__FUNCTION__, __LINE__,WlanIdx));

   AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_PS_RETRIEVE_START, TRUE, 0, TRUE, TRUE, sizeof(struct _EXT_CMD_AP_PWS_START_T), NULL, CmdPsRetrieveStartRsp);

   NdisZeroMemory(&CmdApPwsStart, sizeof(CmdApPwsStart));

   CmdApPwsStart.u4WlanIdx= cpu2le32(WlanIdx);

   AndesAppendCmdMsg(msg, (char *)&CmdApPwsStart, sizeof(CmdApPwsStart));
   msg->wcid = WlanIdx;
   ret = AndesSendCmdMsg(ad, msg);

error:
   	return ret;
}



INT32 CmdPsClearReq(RTMP_ADAPTER *ad, UINT32 wlanidx, BOOLEAN p_wait)
{
	struct cmd_msg *msg;
	struct _CMD_AP_PS_CLEAR_STRUC_T CmdPsClear;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_AP_PS_CLEAR_STRUC_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("%s:(ret = %d)\n", __FUNCTION__, ret));
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_PS, ("%s(%d): RTEnqueueInternalCmd comming!! WlanIdx: %x\n",__FUNCTION__, __LINE__,wlanidx));

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_PWR_SAVING, TRUE, 0, TRUE, TRUE, sizeof(struct _CMD_AP_PS_CLEAR_STRUC_T), NULL, CmdPsClearRsp);

	NdisZeroMemory(&CmdPsClear, sizeof(CmdPsClear));

	CmdPsClear.u4WlanIdx = cpu2le32(wlanidx);
	CmdPsClear.u4Status = cpu2le32(0);

	AndesAppendCmdMsg(msg, (char *)&CmdPsClear, sizeof(CmdPsClear));
	msg->wcid = wlanidx;
	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}
#endif /* MT_PS */

static INT32 CmdRestartDLReq(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_RESTART_DL_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdReStartDLRsp);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdPatchSemRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;

	Ctl->SemStatus = *Data;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Patch SEM Status=%d\n", Ctl->SemStatus));
}


static INT32 CmdPatchSemGet(RTMP_ADAPTER *ad, UINT32 Semaphore)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;

	msg = AndesAllocCmdMsg(ad, 4);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_PATCH_SEM_CONTROL, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdPatchSemRsp);

	/* Semaphore */
	value = cpu2le32(Semaphore);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

/* Nobody uses it currently*/

static VOID CmdAddrellLenRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case TARGET_ADDRESS_LEN_SUCCESS:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Request target address and length success\n", __FUNCTION__));
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow Status(%d)\n", __FUNCTION__, Status));
			break;
	}
}


static INT32 CmdAddressLenReq(RTMP_ADAPTER *ad, UINT32 address, UINT32 len, UINT32 data_mode)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Start address = %x, DL length = %d, Data mode = %x\n",
									address, len, data_mode));
	msg = AndesAllocCmdMsg(ad, 12);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (address == ROM_PATCH_START_ADDRESS)
		AndesInitCmdMsg(msg, P1_Q0, MT_PATCH_START_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdAddrellLenRsp);
	else
		AndesInitCmdMsg(msg, P1_Q0, MT_TARGET_ADDRESS_LEN_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdAddrellLenRsp);

	/* start address */
	value = cpu2le32(address);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* dl length */
	value = cpu2le32(len);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* data mode */
	value = cpu2le32(data_mode);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

static INT32 CmdFwScatter(RTMP_ADAPTER *ad, UINT8 *dl_payload, UINT32 dl_len, UINT32 count)
{
	struct cmd_msg *msg;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, dl_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, 0xC000, MT_FW_SCATTER, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	AndesAppendCmdMsg(msg, (char *)dl_payload, dl_len);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(scatter = %d, ret = %d)\n", __FUNCTION__, count, ret));
	return ret;
}

static INT32 CmdFwScatters(RTMP_ADAPTER *ad, UINT8 *image, UINT32 image_len)
{
	INT32 sent_len;
	UINT32 cur_len = 0, count = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	while (1)
	{
		INT32 sent_len_max = MT_UPLOAD_FW_UNIT - cap->cmd_header_len;
		sent_len = (image_len - cur_len) >=  sent_len_max ? sent_len_max : (image_len - cur_len);

		if (sent_len > 0) {
			ret = CmdFwScatter(ad, image + cur_len, sent_len, count);
			count++;
			if (ret)
				goto error;
			cur_len += sent_len;
		} else {
			break;
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdPatchFinishRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case WIFI_FW_DOWNLOAD_SUCCESS:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFI ROM Patch Download Success\n", __FUNCTION__));
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WiFi ROM Patch Fail (%d)\n", __FUNCTION__, Status));
			break;
	}
}


static INT32 CmdPatchFinishReq(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __FUNCTION__));

	msg = AndesAllocCmdMsg(ad, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_PATCH_FINISH_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdPatchFinishRsp);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static INT32 CmdFwStartReq(RTMP_ADAPTER *ad, UINT32 override, UINT32 address)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: override = %d, address = %d\n", __FUNCTION__, override, address));

	msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_FW_START_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdStartDLRsp);

	/* override */
	value = cpu2le32(override);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* entry point address */
	value = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdChPrivilege(RTMP_ADAPTER *ad, UINT8 Action, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream)
{
	struct cmd_msg *msg;
	struct _CMD_CH_PRIVILEGE_T ch_privilege;
	INT32 ret = 0;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (central_chl == 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: central channel = 0 is invalid\n", __FUNCTION__));
		return -1;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: control_chl = %d, central_chl = %d, BW = %d,	\
								TXStream = %d, RXStream = %d\n", __FUNCTION__,	\
							control_chl, central_chl, BW, TXStream, RXStream));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_CH_PRIVILEGE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, CMD_CH_PRIVILEGE, CMD_SET, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&ch_privilege, 0x00, sizeof(ch_privilege));

	ch_privilege.ucAction = Action;
	ch_privilege.ucPrimaryChannel = control_chl;

	if (BW == BAND_WIDTH_20)
	{
		ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCN;
	}
	else if (BW == BAND_WIDTH_40)
	{
		if (control_chl < central_chl)
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCA;
		else
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCB;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unknown bandwidth = %d\n", BW));
	}

	if (central_chl > 14)
		ch_privilege.ucRfBand =  CMD_CH_PRIV_BAND_A;
	else
		ch_privilege.ucRfBand = CMD_CH_PRIV_BAND_G;

	ch_privilege.ucRfChannelWidth = CMD_CH_PRIV_CH_WIDTH_20_40;

	ch_privilege.ucReqType = CMD_CH_PRIV_REQ_JOIN;

	AndesAppendCmdMsg(msg, (char *)&ch_privilege, sizeof(ch_privilege));

	if (IS_MT7603(ad) || IS_MT7628(ad) || IS_MT76x6(ad))
	{
		UINT32 Value;
		RTMP_IO_READ32(ad, RMAC_RMCR, &Value);

		if (Value & RMAC_RMCR_RX_STREAM_0)
			Ctl->RxStream0 = 1;

		if (Value & RMAC_RMCR_RX_STREAM_1)
			Ctl->RxStream1 = 1;

		Value |= RMAC_RMCR_RX_STREAM_0;
		Value |= RMAC_RMCR_RX_STREAM_1;
		RTMP_IO_WRITE32(ad, RMAC_RMCR, Value);
	}

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdMultipleMacRegAccessWriteCb(struct cmd_msg *msg,
											char *data, UINT16 len)
{
	EXT_EVENT_MULTI_CR_ACCESS_WR_T *EventMultiCRAccessWR
								= (EXT_EVENT_MULTI_CR_ACCESS_WR_T *)(data + 20);

	EventMultiCRAccessWR->u4Status = le2cpu32(EventMultiCRAccessWR->u4Status);

	if (EventMultiCRAccessWR->u4Status)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: fail\n", __FUNCTION__));
}


INT32 CmdMultipleMacRegAccessWrite(RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair,
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_MULTIPLE_REG_ACCESS,
				TRUE, 0, TRUE, TRUE, 32, NULL, CmdMultipleMacRegAccessWriteCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32(MAC_CR);
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MultiCR.u4Data = cpu2le32(RegPair[Index].Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: offset: = %x\n", __FUNCTION__,MultiCR.u4Addr));

		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdMultipleRfRegAccessWriteCb(struct cmd_msg *msg,
											char *data, UINT16 len)
{
	EXT_EVENT_MULTI_CR_ACCESS_WR_T *EventMultiCRAccessWR
								= (EXT_EVENT_MULTI_CR_ACCESS_WR_T *)(data + 20);

	EventMultiCRAccessWR->u4Status = le2cpu32(EventMultiCRAccessWR->u4Status);

	if (EventMultiCRAccessWR->u4Status)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: fail\n", __FUNCTION__));
}


INT32 CmdMultipleRfRegAccessWrite(RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair,
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_MULTIPLE_REG_ACCESS,
				TRUE, 0, TRUE, TRUE, 32, NULL, CmdMultipleRfRegAccessWriteCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32((RF_CR & 0xff) |
							((RegPair->WiFiStream & 0xffffff) << 8));
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MultiCR.u4Data = cpu2le32(RegPair[Index].Value);

		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdMultipleRfRegAccessReadCb(struct cmd_msg *msg,
											char *data, UINT16 len)
{
	UINT32 Index;
	UINT32 Num = (len -20) / sizeof(EXT_EVENT_MULTI_CR_ACCESS_RD_T);
	EXT_EVENT_MULTI_CR_ACCESS_RD_T *EventMultiCRAccessRD
								= (EXT_EVENT_MULTI_CR_ACCESS_RD_T *)(data + 20);
	MT_RF_REG_PAIR *RegPair = (MT_RF_REG_PAIR *)msg->rsp_payload;

	for (Index = 0; Index < Num; Index++)
	{
		RegPair->WiFiStream = (le2cpu32(EventMultiCRAccessRD->u4Type)
								& (0xffffff << 8)) >> 8;
		RegPair->Register = le2cpu32(EventMultiCRAccessRD->u4Addr);
		RegPair->Value = le2cpu32(EventMultiCRAccessRD->u4Data);

		EventMultiCRAccessRD++;
		RegPair++;
	}
}


INT32 CmdMultiPleRfRegAccessRead(RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair,
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_MULTIPLE_REG_ACCESS,
				TRUE, 0, TRUE, TRUE, (12 * Num) + 20, (char *)RegPair,
				CmdMultipleRfRegAccessReadCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32((RF_CR & 0xff) |
							((RegPair->WiFiStream & 0xffffff) << 8));
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);

		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdMultipleMacRegAccessReadCb(struct cmd_msg *msg,
											char *data, UINT16 len)
{
	UINT32 Index;
	UINT32 Num = (len - 20) / sizeof(EXT_EVENT_MULTI_CR_ACCESS_RD_T);
	EXT_EVENT_MULTI_CR_ACCESS_RD_T *EventMultiCRAccessRD
								= (EXT_EVENT_MULTI_CR_ACCESS_RD_T *)(data + 20);
	RTMP_REG_PAIR *RegPair = (RTMP_REG_PAIR *)msg->rsp_payload;

	for (Index = 0; Index < Num; Index++)
	{
		RegPair->Register = le2cpu32(EventMultiCRAccessRD->u4Addr);
		RegPair->Value = le2cpu32(EventMultiCRAccessRD->u4Data);

		EventMultiCRAccessRD++;
		RegPair++;
	}
}


INT32 CmdMultiPleMacRegAccessRead(RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair,
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_MULTIPLE_REG_ACCESS,
				TRUE, 0, TRUE, TRUE, (12 * Num) + 20, (char *)RegPair,
				CmdMultipleMacRegAccessReadCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32(MAC_CR);
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);

		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


INT32 CmdAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: address = %x, data = %x\n", __FUNCTION__, address, data));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_ACCESS_REG_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, CMD_ACCESS_REG, CMD_SET, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _CMD_ACCESS_REG_T *access_reg = (struct _CMD_ACCESS_REG_T *)data;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __FUNCTION__));

	NdisMoveMemory(msg->rsp_payload, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->rsp_payload)) = le2cpu32(*((UINT32 *)msg->rsp_payload));
}


INT32 CmdAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg;
	INT32 ret = 0;

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_ACCESS_REG_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, CMD_ACCESS_REG, CMD_QUERY, EXT_CMD_NA, TRUE, 0,
							TRUE, TRUE, 8, (CHAR *)data, CmdAccessRegReadCb);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.u4Address = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(pAd, msg);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: address = %x, value = %x\n", __FUNCTION__, address, *data));
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdRFRegAccessWrite(RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value)
{
	struct cmd_msg *msg;
	struct _CMD_RF_REG_ACCESS_T RFRegAccess;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: RFIdx = %d, Offset = %x, Value = %x\n", __FUNCTION__,\
										RFIdx, Offset, Value));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_RF_REG_ACCESS_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RF_REG_ACCESS, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&RFRegAccess, 0x00, sizeof(RFRegAccess));

	RFRegAccess.WiFiStream = cpu2le32(RFIdx);
	RFRegAccess.Address = cpu2le32(Offset);
	RFRegAccess.Data = cpu2le32(Value);

	AndesAppendCmdMsg(msg, (char *)&RFRegAccess, sizeof(RFRegAccess));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdRFRegAccessReadCb(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _CMD_RF_REG_ACCESS_T *RFRegAccess = (struct _CMD_RF_REG_ACCESS_T *)Data;

	NdisMoveMemory(msg->rsp_payload, &RFRegAccess->Data, Len - 8);
	*msg->rsp_payload = le2cpu32(*msg->rsp_payload);
}


INT32 CmdRFRegAccessRead(RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value)
{
	struct cmd_msg *msg;
	struct _CMD_RF_REG_ACCESS_T RFRegAccess;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: RFIdx = %d, Offset = %x\n", __FUNCTION__, RFIdx, Offset));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_RF_REG_ACCESS_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_RF_REG_ACCESS, TRUE, 0,
							TRUE, TRUE, 12, (CHAR *)Value, CmdRFRegAccessReadCb);

	memset(&RFRegAccess, 0x00, sizeof(RFRegAccess));

	RFRegAccess.WiFiStream = cpu2le32(RFIdx);
	RFRegAccess.Address = cpu2le32(Offset);

	AndesAppendCmdMsg(msg, (char *)&RFRegAccess, sizeof(RFRegAccess));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


VOID CmdIOWrite32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	RTMP_REG_PAIR RegPair;

	if (Ctl->Stage == FW_RUN_TIME)
	{
		RegPair.Register = Offset;
		RegPair.Value = Value;
		CmdMultipleMacRegAccessWrite(pAd, &RegPair, 1);
	}
	else
	{
		CmdInitAccessRegWrite(pAd, Offset, Value);
	}
}


VOID CmdIORead32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value)
{

	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	RTMP_REG_PAIR RegPair;

	if (Ctl->Stage == FW_RUN_TIME)
	{
		RegPair.Register = Offset;
		CmdMultiPleMacRegAccessRead(pAd, &RegPair, 1);
		*Value = RegPair.Value;
	}
	else
	{
		CmdInitAccessRegRead(pAd, Offset, Value);
	}
}

static VOID EventExtCmdResultMsgRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->ucExTenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->u4Status));

	RTMP_OS_TXRXHOOK_CALL(WLAN_CALIB_TEST_RSP,NULL,EventExtCmdResult->u4Status,pAd);
}

VOID EventExtCmdResultHandler(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->ucExTenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->u4Status));

	RTMP_OS_TXRXHOOK_CALL(WLAN_CALIB_TEST_RSP,NULL,EventExtCmdResult->u4Status,pAd);
}


INT32 CmdIcapOverLap(RTMP_ADAPTER *pAd, UINT32 IcapLen)
{
    struct cmd_msg *msg;
    struct _CMD_TEST_CTRL_T TestCtrl;
    INT32 ret = 0;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: IcapLen = %d\n", __FUNCTION__, IcapLen));

    msg = AndesAllocCmdMsg(pAd, sizeof(TestCtrl));

    if (!msg) {
        ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RF_TEST, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

    memset(&TestCtrl, 0x00, sizeof(TestCtrl));

    TestCtrl.ucAction = 0;

    TestCtrl.ucIcapLen = IcapLen;

	TestCtrl.u.u4OpMode = OPERATION_ICAP_OVERLAP;

    AndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));

    ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdRfTest(RTMP_ADAPTER *pAd, UINT8 Action, UINT8 Mode, UINT8 CalItem)
{
    struct cmd_msg *msg;
    struct _CMD_TEST_CTRL_T TestCtrl;
    INT32 ret = 0;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Action = %d Mode = %d CalItem = %d\n", __FUNCTION__, Action, Mode, CalItem));

    msg = AndesAllocCmdMsg(pAd, sizeof(TestCtrl));

    if (!msg) {
        ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RF_TEST, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

    memset(&TestCtrl, 0x00, sizeof(TestCtrl));

    TestCtrl.ucAction = Action;
    TestCtrl.u.u4OpMode = (UINT32)Mode;

    if (Action == ACTION_IN_RFTEST) {
        /* set Cal Items */
        TestCtrl.u.rRfATInfo.u4FuncIndex = 1;
        TestCtrl.u.rRfATInfo.u4FuncData = (UINT32)CalItem;
    }

    AndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));

    ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;

}

INT32 CmdRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 On)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_RADIO_ON_OFF_CTRL_T RadioOnOffCtrl;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: On = %d\n", __FUNCTION__, On));

	msg = AndesAllocCmdMsg(pAd, sizeof(RadioOnOffCtrl));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RADIO_ON_OFF_CTRL, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

	memset(&RadioOnOffCtrl, 0x00, sizeof(RadioOnOffCtrl));

	if (On == WIFI_RADIO_ON)
		RadioOnOffCtrl.ucWiFiRadioCtrl = WIFI_RADIO_ON;
	else if (On == WIFI_RADIO_OFF)
		RadioOnOffCtrl.ucWiFiRadioCtrl = WIFI_RADIO_OFF;
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknown state, On=%d\n", __FUNCTION__, On));

	AndesAppendCmdMsg(msg, (char *)&RadioOnOffCtrl, sizeof(RadioOnOffCtrl));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdWiFiRxDisable(RTMP_ADAPTER *pAd, UINT RxDisable)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_WIFI_RX_DISABLE_T WiFiRxDisable;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: WiFiRxDisable = %d\n", __FUNCTION__, RxDisable));

	if (RxDisable != WIFI_RX_DISABLE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error: %s: RxDisable = %d\n", __FUNCTION__, RxDisable));
		return ret;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(WiFiRxDisable));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_WIFI_RX_DISABLE, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

	memset(&WiFiRxDisable, 0x00, sizeof(WiFiRxDisable));

	WiFiRxDisable.ucWiFiRxDisableCtrl = RxDisable;

	AndesAppendCmdMsg(msg, (char *)&WiFiRxDisable, sizeof(WiFiRxDisable));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

INT32 CmdPmStateCtrl(RTMP_ADAPTER *pAd, UCHAR State, UCHAR Mode)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_PM_STATE_CTRL_T PmStateCtrl;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: PmStateCtrl, State = %x, Mode = %x\n", __func__, State, Mode));

	msg = AndesAllocCmdMsg(pAd, sizeof(PmStateCtrl));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_PM_STATE_CTRL, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

	memset(&PmStateCtrl, 0x00, sizeof(PmStateCtrl));

	PmStateCtrl.ucPmNumber = State;
	PmStateCtrl.ucPmState = Mode;

	AndesAppendCmdMsg(msg, (char *)&PmStateCtrl, sizeof(PmStateCtrl));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


#ifdef SINGLE_SKU_V2

enum {
	SKU_CCK_1_2=0,
	SKU_CCK_55_11,
	SKU_OFDM_6_9,
	SKU_OFDM_12_18,
	SKU_OFDM_24_36,
	SKU_OFDM_48,
	SKU_OFDM_54,
	SKU_HT20_0_8,
	SKU_HT20_32,
	SKU_HT20_1_2_9_10,
	SKU_HT20_3_4_11_12,
	SKU_HT20_5_13,
	SKU_HT20_6_14,
	SKU_HT20_7_15,
	SKU_HT40_0_8,
	SKU_HT40_32,
	SKU_HT40_1_2_9_10,
	SKU_HT40_3_4_11_12,
	SKU_HT40_5_13,
	SKU_HT40_6_14,
	SKU_HT40_7_15,
};
static VOID mt_FillSkuParameter(RTMP_ADAPTER *pAd,UINT8 channel,UINT8 *txPowerSku)
{
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UINT8 j;

	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		start_ch = ch->StartChannel;
		if ( channel >= start_ch )
		{
			for ( j = 0; j < ch->num; j++ )
			{
				if ( channel == ch->Channel[j] )
				{

					txPowerSku[SKU_CCK_1_2] 	= ch->PwrCCK[0] ? ch->PwrCCK[0] : 0xff;
					txPowerSku[SKU_CCK_55_11] 	= ch->PwrCCK[2] ? ch->PwrCCK[2] : 0xff;
					txPowerSku[SKU_OFDM_6_9] 	= ch->PwrOFDM[0] ? ch->PwrOFDM[0] : 0xff;
					txPowerSku[SKU_OFDM_12_18]= ch->PwrOFDM[2] ? ch->PwrOFDM[2] : 0xff;
					txPowerSku[SKU_OFDM_24_36]= ch->PwrOFDM[4] ? ch->PwrOFDM[4] : 0xff;
					txPowerSku[SKU_OFDM_48] 	= ch->PwrOFDM[6] ? ch->PwrOFDM[6] : 0xff;
					txPowerSku[SKU_OFDM_54] 	= ch->PwrOFDM[7] ? ch->PwrOFDM[7] : 0xff;
					txPowerSku[SKU_HT20_0_8] 	= ch->PwrHT20[0] ? ch->PwrHT20[0] : 0xff;
					/*MCS32 is a special rate will chose the max power, normally will be OFDM 6M */
					txPowerSku[SKU_HT20_32] 	=  ch->PwrOFDM[0] ? ch->PwrOFDM[0] : 0xff;
					txPowerSku[SKU_HT20_1_2_9_10] = ch->PwrHT20[1] ? ch->PwrHT20[1] : 0xff;
					txPowerSku[SKU_HT20_3_4_11_12] = ch->PwrHT20[3] ? ch->PwrHT20[3] : 0xff;
					txPowerSku[SKU_HT20_5_13] 	= ch->PwrHT20[5] ? ch->PwrHT20[5]  : 0xff;
					txPowerSku[SKU_HT20_6_14] 	= ch->PwrHT20[6] ? ch->PwrHT20[6]  : 0xff;
					txPowerSku[SKU_HT20_7_15] 	= ch->PwrHT20[7] ? ch->PwrHT20[7]  : 0xff;
					txPowerSku[SKU_HT40_0_8] 	= ch->PwrHT40[0] ? ch->PwrHT40[0]  : 0xff;
					/*MCS32 is a special rate will chose the max power, normally will be OFDM 6M */
					txPowerSku[SKU_HT40_32] 	=  ch->PwrOFDM[0] ? ch->PwrOFDM[0] : 0xff;
					txPowerSku[SKU_HT40_1_2_9_10] = ch->PwrHT40[1] ?  ch->PwrHT40[1] : 0xff;
					txPowerSku[SKU_HT40_3_4_11_12] = ch->PwrHT40[3] ?  ch->PwrHT40[3] : 0xff;
					txPowerSku[SKU_HT40_5_13] 	= ch->PwrHT40[5] ?  ch->PwrHT40[5] : 0xff;
					txPowerSku[SKU_HT40_6_14] 	= ch->PwrHT40[6] ?  ch->PwrHT40[6] : 0xff;
					txPowerSku[SKU_HT40_7_15] 	= ch->PwrHT40[7] ?  ch->PwrHT40[7] : 0xff;
					break;
				}
			}
		}
	}
}

#endif

INT32 CmdChannelSwitch(RTMP_ADAPTER *pAd, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream, BOOLEAN scan)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_CHAN_SWITCH_T CmdChanSwitch;
	INT32 ret = 0,i=0;


	if (central_chl == 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: central channel = 0 is invalid\n", __FUNCTION__));
		return -1;
	}


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: control_chl = %d, central_chl = %d, BW = %d,TXStream = %d, RXStream = %d, scan(%d)\n", __FUNCTION__,
							control_chl, central_chl, BW, TXStream, RXStream, scan));

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdChanSwitch));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_CHANNEL_SWITCH, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

	memset(&CmdChanSwitch, 0x00, sizeof(CmdChanSwitch));

	CmdChanSwitch.ucCtrlCh = control_chl;
	CmdChanSwitch.ucCentralCh = central_chl;
	CmdChanSwitch.ucBW = BW;
	CmdChanSwitch.ucTxStreamNum = TXStream;
	CmdChanSwitch.ucRxStreamNum = RXStream;
	/* ucSwitchReason is only applied on MT7636, other porject is don't care term */

	for(i=0;i<SKU_SIZE;i++)
	{
		CmdChanSwitch.aucTxPowerSKU[i]=0xff;
	}

#ifdef SINGLE_SKU_V2
	if  (pAd->SKUEn)
	mt_FillSkuParameter(pAd,central_chl,CmdChanSwitch.aucTxPowerSKU);
#endif
	AndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID EventExtNicCapability(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_NIC_CAPABILITY *ExtEventNicCapability = (EXT_EVENT_NIC_CAPABILITY *)Data;
	UINT32 Loop;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("The data code of firmware:"));

	for (Loop = 0; Loop < 16; Loop++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", ExtEventNicCapability->aucDateCode[Loop]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nThe version code of firmware:"));

	for (Loop = 0; Loop < 12; Loop++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", ExtEventNicCapability->aucVersionCode[Loop]));
	}
}


INT32 CmdNicCapability(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	INT32 ret = 0;

	msg = AndesAllocCmdMsg(pAd, 0);

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_NIC_CAPABILITY, TRUE, 0,
							TRUE, TRUE, 28, NULL, EventExtNicCapability);

	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdFwLog2Host(RTMP_ADAPTER *pAd, UINT8 FWLog2HostCtrl)
{

	struct cmd_msg *msg;
	INT32 Ret = 0;
	EXT_CMD_FW_LOG_2_HOST_CTRL_T CmdFwLog2HostCtrl;

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdFwLog2HostCtrl));

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: msg is NULL\n",__FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_FW_LOG_2_HOST,
						FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&CmdFwLog2HostCtrl, 0x00, sizeof(CmdFwLog2HostCtrl));

	CmdFwLog2HostCtrl.ucFwLog2HostCtrl = FWLog2HostCtrl;

	AndesAppendCmdMsg(msg, (char *)&CmdFwLog2HostCtrl,
									sizeof(CmdFwLog2HostCtrl));

	Ret = AndesSendCmdMsg(pAd, msg);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

#ifdef THERMAL_PROTECT_SUPPORT
static VOID EventThermalProtect(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_THERMAL_PROTECT_T *EvtThermalProtect;
	UINT8 HLType;
    UINT32 Ret;

	EvtThermalProtect = (EXT_EVENT_THERMAL_PROTECT_T *)Data;

	HLType = EvtThermalProtect->ucHLType;
	pAd->last_thermal_pro_temp = EvtThermalProtect->cCurrentTemp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: HLType = %d, CurrentTemp = %d\n", __FUNCTION__, HLType, pAd->last_thermal_pro_temp));

    RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, Ret);
	if(Ret != 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
	
	if (HLType == HIGH_TEMP_THD)
	{
		pAd->force_one_tx_stream = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Switch TX to 1 stram\n"));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Switch TX to 2 stram\n"));
		pAd->force_one_tx_stream = FALSE;
	}
    pAd->fgThermalProtectToggle = TRUE;

    RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
}


INT32 CmdThermalProtect(RTMP_ADAPTER *ad, UINT8 HighEn, CHAR HighTempTh, UINT8 LowEn, CHAR LowTempTh)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_THERMAL_PROTECT_T ThermalProtect;

	msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_THERMAL_PROTECT_T));

	if (!msg)
	{
         ret = NDIS_STATUS_RESOURCES;
         goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_THERMAL_PROTECT, TRUE, 0, TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

	NdisZeroMemory(&ThermalProtect, sizeof(ThermalProtect));

	ThermalProtect.ucHighEnable = HighEn;
	ThermalProtect.cHighTempThreshold = HighTempTh;
	ThermalProtect.ucLowEnable = LowEn;
	ThermalProtect.cLowTempThreshold = LowTempTh;

	ad->thermal_pro_high_criteria = HighTempTh;
	ad->thermal_pro_high_en = HighEn;
	ad->thermal_pro_low_criteria = LowTempTh;
	ad->thermal_pro_low_en = LowEn;

	AndesAppendCmdMsg(msg, (char *)&ThermalProtect, sizeof(ThermalProtect));
	ret = AndesSendCmdMsg(ad, msg);

error:
      MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
   	return ret;
}
#endif

INT32 CmdObtwDelta(struct _RTMP_ADAPTER *ad, BOOLEAN anyEnable, UINT8 *ObtwDeltaArray)
{
    struct cmd_msg *msg;
    INT32 ret = 0;
    EXT_CMD_OBTW_T ObtwDeltaCmd;

    msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_OBTW_T));

    if (!msg)
    {
         ret = NDIS_STATUS_RESOURCES;
         goto error;
    }

    AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_OBTW, FALSE, 0, FALSE, FALSE, 8, NULL, NULL);

    NdisZeroMemory(&ObtwDeltaCmd, sizeof(ObtwDeltaCmd));

    ObtwDeltaCmd.ucOption = anyEnable;
    NdisCopyMemory(ObtwDeltaCmd.ucOBTWDelta, ObtwDeltaArray, 21);

    AndesAppendCmdMsg(msg, (char *)&ObtwDeltaCmd, sizeof(ObtwDeltaCmd));
    ret = AndesSendCmdMsg(ad, msg);

error:
      MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
    return ret;
}

INT32 CmdTmrCal(RTMP_ADAPTER *ad, UINT8 enable, UINT8 band, UINT8 bw, UINT8 ant, UINT8 role)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_TMR_CAL_T TmrCal;

	msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_TMR_CAL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_TMR_CAL, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	NdisZeroMemory(&TmrCal, sizeof(TmrCal));

	TmrCal.ucEnable = enable;
	TmrCal.ucBand = band;
	TmrCal.ucBW = bw;
	TmrCal.ucAnt = ant;//only ant 0 support at present.
	TmrCal.ucRole = role;

	AndesAppendCmdMsg(msg, (char *)&TmrCal, sizeof(TmrCal));
	ret = AndesSendCmdMsg(ad, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

NTSTATUS CmdPowerOnWiFiSys(RTMP_ADAPTER *pAd)
{
	NTSTATUS status = 0;
#if defined(RTMP_PCI_SUPPORT)
#elif defined(RTMP_USB_SUPPORT)
	status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x04,
		0,
		0x01,
		NULL,
		0);
#endif

	return status;
}

static NDIS_STATUS AndesMTLoadFwMethod1(RTMP_ADAPTER *ad)
{
	UINT32 value, loop, dl_len;
	UINT32 ret = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (cap->load_code_method == BIN_FILE_METHOD) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
#ifdef RTMP_PCI_SUPPORT
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
#endif

	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load fw image from fw_header_image\n"));
		cap->FWImageName = cap->fw_header_image;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d)::pChipCap->fw_len(%d)\n", __FUNCTION__, __LINE__, cap->fw_len));

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image(fw_header_image), load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FW Version:"));
	for (loop = 0; loop < 10; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 29 + loop)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FW Build Date:"));
	for (loop = 0; loop < 15; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 19 + loop)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	dl_len = (*(cap->FWImageName + cap->fw_len - 1) << 24) |
				(*(cap->FWImageName + cap->fw_len - 2) << 16) |
				(*(cap->FWImageName + cap->fw_len -3) << 8) |
				*(cap->FWImageName + cap->fw_len - 4);

	dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\ndownload len = %d\n", dl_len));

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* switch to bypass mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(1);

#ifdef RTMP_PCI_SUPPORT
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(5);
#endif


	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	/* optional CMD procedure */
	/* CMD restart download flow request */
	RTMP_IO_READ32(ad, TOP_MISC2, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TOP_MSIC = %x\n", value));

	/* check ram code if running, if it is, need to do optional cmd procedure */
	if ((value & 0x02) == 0x02) {
		ret = CmdRestartDLReq(ad);

		if (ret)
			goto done;
	}

	/* check rom code if ready */
	loop = 0;

	do
	{
		RTMP_IO_READ32(ad, TOP_MISC2, &value);
		if ((value & 0x01) == 0x01 && !(value & 0x02) )
			break;
		RtmpOsMsDelay(1);
		loop++;
	} while (loop <= 500);

	if (loop > 500) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: rom code is not ready(TOP_MISC2 = %d)\n", __FUNCTION__, value));
		goto done;
	}

	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, FW_CODE_START_ADDRESS1, dl_len, TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName, dl_len);

	if (ret)
		goto done;

	/* 3. Firmware start negotiation phase */
	ret = CmdFwStartReq(ad, 1, FW_CODE_START_ADDRESS1);

	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		RTMP_IO_READ32(ad, TOP_MISC2, &value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TOP_MSIC = %x\n", value));
		if ((value & 0x02) == 0x02)
			break;

		RtmpOsMsDelay(1);
	}

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{
		Ctl->Stage = FW_RUN_TIME;
	}

done:

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* Switch to normal mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(0);
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(0);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value |= (1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~(1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	return ret;
}


static NDIS_STATUS AndesMTLoadFwMethod2(RTMP_ADAPTER *ad)
{
#define SW_SYN0 0x81021250

	UINT32 value, loop, ilm_dl_len, dlm_dl_len;
	UINT8 ilm_feature_set, dlm_feature_set;
	UINT8 ilm_chip_info, dlm_chip_info;
	UINT32 ilm_target_addr, dlm_target_addr;
	UINT32 ret = 0;
	NTSTATUS status;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (cap->load_code_method == BIN_FILE_METHOD) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
#ifdef RTMP_PCI_SUPPORT
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
#endif


	} else {
		cap->FWImageName = cap->fw_header_image;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d), cap->fw_len(%d)\n", __FUNCTION__, __LINE__, cap->fw_len));

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	ilm_target_addr = (*(cap->FWImageName + cap->fw_len - (33 + 36)) << 24) |
				(*(cap->FWImageName + cap->fw_len - (34 + 36)) << 16) |
				(*(cap->FWImageName + cap->fw_len -(35 + 36)) << 8) |
				*(cap->FWImageName + cap->fw_len - (36 + 36));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ILM target address = %x\n", ilm_target_addr));

	ilm_chip_info = *(cap->FWImageName + cap->fw_len - (32 + 36));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM chip information = %x\n", ilm_chip_info));

	ilm_feature_set = *(cap->FWImageName + cap->fw_len - (31 + 36));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM feature set = %x\n", ilm_feature_set));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM Build Date:"));

	for (loop = 0; loop < 8; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->FWImageName + cap->fw_len - (20 + 36) + loop)));

	ilm_dl_len = (*(cap->FWImageName + cap->fw_len - (1 + 36)) << 24) |
				(*(cap->FWImageName + cap->fw_len - (2 + 36)) << 16) |
				(*(cap->FWImageName + cap->fw_len -(3 + 36)) << 8) |
				*(cap->FWImageName + cap->fw_len - (4 + 36));

	ilm_dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM download len = %d\n", ilm_dl_len));

	dlm_target_addr = (*(cap->FWImageName + cap->fw_len - 33) << 24) |
				(*(cap->FWImageName + cap->fw_len - 34) << 16) |
				(*(cap->FWImageName + cap->fw_len - 35) << 8) |
				*(cap->FWImageName + cap->fw_len - 36);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DLM target address = %x\n", dlm_target_addr));

	dlm_chip_info = *(cap->FWImageName + cap->fw_len - 32);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nDLM chip information = %x\n", dlm_chip_info));

	dlm_feature_set = *(cap->FWImageName + cap->fw_len - 31);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nDLM feature set = %x\n", dlm_feature_set));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DLM Build Date:"));

	for (loop = 0; loop < 8; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 20 + loop)));

	dlm_dl_len = (*(cap->FWImageName + cap->fw_len - 1) << 24) |
				(*(cap->FWImageName + cap->fw_len - 2) << 16) |
				(*(cap->FWImageName + cap->fw_len - 3) << 8) |
				*(cap->FWImageName + cap->fw_len - 4);

	dlm_dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nDLM download len = %d\n", dlm_dl_len));

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* switch to bypass mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(1);

#ifdef RTMP_PCI_SUPPORT
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(5);
#endif


	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	/* optional CMD procedure */
	/* CMD restart download flow request */
	/* check if SW_SYN0 at init. state */
//#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	RTMP_IO_READ32(ad, SW_SYN0, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Current SW_SYN0(%d)\n", __FUNCTION__, value));

	if (value == 0x0)
	{
		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 1. power on WiFi SYS\n", __FUNCTION__));
		status = CmdPowerOnWiFiSys(ad);

		if (status)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: failed to power on WiFi SYS\n", __FUNCTION__));
			goto done;
		}

		/* poll SW_SYN0 == 1 */
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, SW_SYN0, &value);
			if (value == 0x1)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 1. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	}
	else if (value == 0x3)
	{
		/* restart cmd*/
		ret = CmdRestartDLReq(ad);

		if (ret)
			goto done;

		/* poll SW_SYN0 == 0 */
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, SW_SYN0, &value);
			if (value == 0x0)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}

		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 2. power on WiFi SYS\n", __FUNCTION__));
		status = CmdPowerOnWiFiSys(ad);

		if (status)
			goto done;

		/* poll SW_SYN0 == 1*/
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, SW_SYN0, &value);
			if (value == 0x1)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (SW_SYN0 = %d), ready to ILM/DLM DL...\n", __FUNCTION__, value));
	}
//#endif

	/*  ILM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, ilm_target_addr, ilm_dl_len,
				(ilm_feature_set & FW_FEATURE_SET_ENCRY) |
				(FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(ilm_feature_set))) |
				((ilm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
				TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading ilm firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName, ilm_dl_len);

	if (ret)
		goto done;

	/*  DLM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, dlm_target_addr, dlm_dl_len,
				(dlm_feature_set & FW_FEATURE_SET_ENCRY) |
				(FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(dlm_feature_set))) |
				((dlm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
				TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading dlm firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName + ilm_dl_len, dlm_dl_len);

	if (ret)
		goto done;

	/* 3. Firmware start negotiation phase */
	ret = CmdFwStartReq(ad, 0, 0);


#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		RTMP_IO_READ32(ad, SW_SYN0, &value);
		if ((value & 0x03) == 0x03)
			break;

		RtmpOsMsDelay(1);
	}

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{
		Ctl->Stage = FW_RUN_TIME;
	}
#endif


done:

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* Switch to normal mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(0);
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(0);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value |= (1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~(1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	return ret;
}

#ifdef MT7615
static NDIS_STATUS AndesMTLoadFwMethod3(RTMP_ADAPTER *ad)
{
	UINT32 value, loop, ilm_dl_len, dlm_dl_len;
	UINT8 ilm_feature_set, dlm_feature_set;
	UINT8 ilm_chip_info, dlm_chip_info;
	UINT32 ilm_target_addr, dlm_target_addr;
	UINT32 ret = 0;
	NTSTATUS status;
	UINT32	counter =0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (cap->load_code_method == BIN_FILE_METHOD) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
#ifdef RTMP_PCI_SUPPORT
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
#endif


	} else {
		cap->FWImageName = cap->fw_header_image;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d), cap->fw_len(%d)\n", __FUNCTION__, __LINE__, cap->fw_len));

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	ilm_target_addr = (*(cap->FWImageName + cap->fw_len - (33 + 36)) << 24) |
				(*(cap->FWImageName + cap->fw_len - (34 + 36)) << 16) |
				(*(cap->FWImageName + cap->fw_len -(35 + 36)) << 8) |
				*(cap->FWImageName + cap->fw_len - (36 + 36));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ILM target address = %x\n", ilm_target_addr));

	ilm_chip_info = *(cap->FWImageName + cap->fw_len - (32 + 36));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM chip information = %x\n", ilm_chip_info));

	ilm_feature_set = *(cap->FWImageName + cap->fw_len - (31 + 36));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM feature set = %x\n", ilm_feature_set));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM Build Date:"));

	for (loop = 0; loop < 8; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->FWImageName + cap->fw_len - (20 + 36) + loop)));

	ilm_dl_len = (*(cap->FWImageName + cap->fw_len - (1 + 36)) << 24) |
				(*(cap->FWImageName + cap->fw_len - (2 + 36)) << 16) |
				(*(cap->FWImageName + cap->fw_len -(3 + 36)) << 8) |
				*(cap->FWImageName + cap->fw_len - (4 + 36));

	ilm_dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM download len = %d\n", ilm_dl_len));

	dlm_target_addr = (*(cap->FWImageName + cap->fw_len - 33) << 24) |
				(*(cap->FWImageName + cap->fw_len - 34) << 16) |
				(*(cap->FWImageName + cap->fw_len - 35) << 8) |
				*(cap->FWImageName + cap->fw_len - 36);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DLM target address = %x\n", dlm_target_addr));

	dlm_chip_info = *(cap->FWImageName + cap->fw_len - 32);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nDLM chip information = %x\n", dlm_chip_info));

	dlm_feature_set = *(cap->FWImageName + cap->fw_len - 31);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nDLM feature set = %x\n", dlm_feature_set));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DLM Build Date:"));

	for (loop = 0; loop < 8; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 20 + loop)));

	dlm_dl_len = (*(cap->FWImageName + cap->fw_len - 1) << 24) |
				(*(cap->FWImageName + cap->fw_len - 2) << 16) |
				(*(cap->FWImageName + cap->fw_len - 3) << 8) |
				*(cap->FWImageName + cap->fw_len - 4);

	dlm_dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nDLM download len = %d\n", dlm_dl_len));

	/* optional CMD procedure */
	/* CMD restart download flow request */
	/* check if TOP_MISC2 at init. state */
	RTMP_IO_READ32(ad, TOP_MISC2, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Current TOP_MISC2(%d)\n", __FUNCTION__, value));

    if ((value & (BIT0|BIT1|BIT2)) == (BIT0|BIT1|BIT2))
	{
		/* restart cmd*/
		ret = CmdRestartDLReq(ad);

		if (ret)
			goto done;

		/* poll TOP_MISC2 == 0 */
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, TOP_MISC2, &value);
			if ((value & BIT0) == 0x0)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 1. TOP_MISC2 is not at init. state (TOP_MISC2 = %d)\n", __FUNCTION__, value));
			goto done;
		}

		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 2. power on WiFi SYS\n", __FUNCTION__));
		status = CmdPowerOnWiFiSys(ad); /* fonchi todo: check WiFi on is needed for 7615. */

		if (status)
			goto done;

		/* poll TOP_MISC2 == 1*/
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, TOP_MISC2, &value);
			if ((value & BIT0) == BIT0)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TOP_MISC2 is not at init. state (TOP_MISC2 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	}
    else if ((value & BIT0) == 0)
	{
		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 1. power on WiFi SYS\n", __FUNCTION__));
		status = CmdPowerOnWiFiSys(ad); /* fonchi todo: check that is WiFi on needed on 7615. */

		if (status)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: failed to power on WiFi SYS\n", __FUNCTION__));
			goto done;
		}

		/* poll TOP_MISC2 == 1 */
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, TOP_MISC2, &value);
			if ((value & BIT0) == BIT0)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 1. TOP_MISC2 is not at init. state (TOP_MISC2 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	}
    else
    {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (TOP_MISC2 = %d), ready to ILM/DLM DL...\n", __FUNCTION__, value));
	}

	/*  ILM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, ilm_target_addr, ilm_dl_len,
				(ilm_feature_set & FW_FEATURE_SET_ENCRY) |
				(FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(ilm_feature_set))) |
				((ilm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
				TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2. CmdAddressLenReq Command fail (ret = %d)\n", __FUNCTION__, ret));
		goto done;
    }

	/* 2. Loading ilm firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName, ilm_dl_len);

	if (ret)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 3. CmdFwScatters Command fail (ret = %d)\n", __FUNCTION__, ret));
        goto done;
    }

	/*  DLM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, dlm_target_addr, dlm_dl_len,
				(dlm_feature_set & FW_FEATURE_SET_ENCRY) |
				(FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(dlm_feature_set))) |
				((dlm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
				TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading dlm firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName + ilm_dl_len, dlm_dl_len);

	if (ret)
		goto done;

    /* fonchi todo: Need to download CR4 before N9 running. */

	/* 3. Firmware start negotiation phase */
	ret = CmdFwStartReq(ad, 0, 0);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		RTMP_IO_READ32(ad, TOP_MISC2, &value);
		if ((value & (BIT1 | BIT2)) == (BIT1 | BIT2))
			break;

		RtmpOsMsDelay(1);
	}

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{
		Ctl->Stage = FW_RUN_TIME;
	}
#endif

done:
/* fonchi todo: should done checking needed? */

	return ret;
}
#endif /* MT7615 */

NDIS_STATUS AndesMTLoadFw(RTMP_ADAPTER *pAd)
{
	UINT32 Ret;
	RTMP_CHIP_CAP *Cap = &pAd->chipCap;

	if (Cap->DownLoadType == DownLoadTypeA)
	{
		Ret = AndesMTLoadFwMethod1(pAd);
	}
	else if (Cap->DownLoadType == DownLoadTypeB)
	{
		Ret = AndesMTLoadFwMethod2(pAd);
	}
#ifdef MT7615
	else if (Cap->DownLoadType == DownLoadTypeC)
	{
		Ret = AndesMTLoadFwMethod3(pAd);
	}
#endif /* MT7615 */
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow Download type(%d)\n", __FUNCTION__, Cap->DownLoadType));
		Ret = -1;
	}

	return Ret;
}


NDIS_STATUS AndesMTLoadRomPatch(RTMP_ADAPTER *ad)
{
	UINT32 value, loop;
	UINT32 ret = 0;
	NTSTATUS status;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	RTMP_CHIP_OP *pChipOps = &ad->chipOps;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;
	UINT32 patch_len = 0, total_checksum = 0;
	if (cap->load_code_method == BIN_FILE_METHOD) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load rom patch image from /lib/firmware/%s\n", cap->rom_patch_bin_file_name));


	} else {
		cap->rom_patch = cap->rom_patch_header_image;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d), cap->rom_patch_len(%d)\n", __FUNCTION__, __LINE__, cap->rom_patch_len));

	if (!cap->rom_patch) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = ROM_PATCH_DOWNLOAD;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Build Date:"));

	for (loop = 0; loop < 16; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->rom_patch + loop)));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("platform = \n"));

	for (loop = 0; loop < 4; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->rom_patch + 16 + loop)));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("hw/sw version = \n"));

	for (loop = 0; loop < 4; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(cap->rom_patch + 20 + loop)));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("patch version = \n"));

	for (loop = 0; loop < 4; loop++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(cap->rom_patch + 24 + loop)));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	total_checksum = *(cap->rom_patch + 28) | (*(cap->rom_patch + 29) << 8);

	patch_len = cap->rom_patch_len - PATCH_INFO_SIZE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\ndownload len = %d\n", patch_len));

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* switch to bypass mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(1);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	/* check if SW_SYN0 at init. state */
	RTMP_IO_READ32(ad, SW_SYN0, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Current SW_SYN0(%d)\n", __FUNCTION__, value));

	if (value == 0x0)
	{
		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 1. power on WiFi SYS\n", __FUNCTION__));
		status = CmdPowerOnWiFiSys(ad);

		if (status) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: failed to power on WiFi SYS\n", __FUNCTION__));
			goto done;
		}

		/* poll SW_SYN0 == 1 */
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, SW_SYN0, &value);
			if (value == 0x1)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 1. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	}
	else if (value == 0x3)
	{
		/* restart cmd*/
		ret = CmdRestartDLReq(ad);

		if (ret) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): failed to CmdRestartDLReq\n", __FUNCTION__, __LINE__));
			goto done;
		}

		/* poll SW_SYN0 == 0 */
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, SW_SYN0, &value);
			if (value == 0x0)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}

		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 2. power on WiFi SYS\n", __FUNCTION__));
		status = CmdPowerOnWiFiSys(ad);

		if (status)
			goto done;

		/* poll SW_SYN0 == 1*/
		loop = 0;
		do
		{
			RTMP_IO_READ32(ad, SW_SYN0, &value);
			if (value == 0x1)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (SW_SYN0 = %d)\n", __FUNCTION__, value));
	}

	ret = CmdPatchSemGet(ad, GET_PATCH_SEM);

	if (ret)
		goto done;

	if (Ctl->SemStatus == 0)
	{
		Ctl->Stage = FW_NO_INIT;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPatch is not ready && get semaphore fail, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 1;
		goto done;
	}
	else if (Ctl->SemStatus == 1)
	{
		Ctl->Stage = FW_NO_INIT;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPatch is ready, continue to ILM/DLM DL, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 0;
		goto done;
	}
	else if (Ctl->SemStatus == 2)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPatch is not ready && get semaphore success, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 0;
	}
	else if (Ctl->SemStatus == 3)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRelease patch semaphore, SemStatus(%d), Error\n", Ctl->SemStatus));
		ret = 1;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nUnknown SemStatus(%d), Error\n", Ctl->SemStatus));
		ret = 1;
	}

	if (ret)
		goto done;

	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, ROM_PATCH_START_ADDRESS, patch_len, TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading rom patch image phase */
	ret = CmdFwScatters(ad, cap->rom_patch + PATCH_INFO_SIZE, patch_len);


	if (ret)
		goto done;

	/* 3. ROM patch start negotiation phase */
	ret = CmdPatchFinishReq(ad);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Send checksum req..\n"));

	pChipOps->AndesMTChkCrc(ad, patch_len);

	RtmpOsMsDelay(20);

	if (total_checksum != pChipOps->AndesMTGetCrc(ad)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("checksum fail!, local(0x%x) <> fw(0x%x)\n", total_checksum,
										pChipOps->AndesMTGetCrc(ad)));

		ret = NDIS_STATUS_FAILURE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("checksum=0x%x\n", pChipOps->AndesMTGetCrc(ad)));
	ret = CmdPatchSemGet(ad, REL_PATCH_SEM);

	if (Ctl->SemStatus == 3)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRelease patch semaphore, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 0;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRelease patch semaphore, SemStatus(%d), Error\n", Ctl->SemStatus));
		ret = 1;
	}

	if (ret)
		goto done;

done:

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* Switch to normal mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(0);
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(0);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value |= (1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~(1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	vErasePatch(ad);
#endif

	return ret;
}


INT32 AndesMTEraseFw(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = &pAd->chipCap;

	if (cap->load_code_method == BIN_FILE_METHOD) {

		if (cap->FWImageName)
			os_free_mem(NULL, cap->FWImageName);
			cap->FWImageName = NULL;
	}

	return 0;
}

INT32 AndesMTEraseRomPatch(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __FUNCTION__));

	if (cap->load_code_method == BIN_FILE_METHOD) {
		if (cap->rom_patch)
			os_free_mem(NULL, cap->rom_patch);
			cap->rom_patch = NULL;
	}

	return 0;
}

static VOID EventChPrivilegeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	UINT32 Value;

	if (IS_MT7603(pAd) || IS_MT7628(pAd)  || IS_MT76x6(pAd))
	{
		RTMP_IO_READ32(pAd, RMAC_RMCR, &Value);

		if (ctl->RxStream0 == 1)
			Value |= RMAC_RMCR_RX_STREAM_0;
		else
			Value &= ~RMAC_RMCR_RX_STREAM_0;

		if (ctl->RxStream1 == 1)
			Value |= RMAC_RMCR_RX_STREAM_1;
		else
			Value &= ~RMAC_RMCR_RX_STREAM_1;

		RTMP_IO_WRITE32(pAd, RMAC_RMCR, Value);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __FUNCTION__));
}

#ifdef CONFIG_STA_SUPPORT

#ifdef RTMP_MAC_SDIO
static VOID ExtEventSleepyNotifyHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	struct _EXT_EVENT_SLEEPY_NOTIFY_T *pExtEventSleepyNotify = (struct _EXT_EVENT_SLEEPY_NOTIFY_T *)Data;

	//printk("ExtEventSleepyNotifyHandler: %d\n", pExtEventSleepyNotify->ucSleepState);
	//printk("in_interrupt = %d ------------------------------\n", in_interrupt());

	extern INT32 MakeFWOwn(RTMP_ADAPTER *pAd);

	if (pExtEventSleepyNotify->ucSleepState)
		MakeFWOwn(pAd);

}
#endif

static VOID ExtEventBeaconLostHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	struct _EXT_EVENT_BEACON_LOSS_T *pExtEventBeaconLoss = (struct _EXT_EVENT_BEACON_LOSS_T *)Data;

	pAd->StaCfg.PwrMgmt.bBeaconLost = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::FW LOG, Beacon lost (%x:%x:%x:%x:%x:%x)\n",
								__FUNCTION__,
								pExtEventBeaconLoss->aucBssid[0],
								pExtEventBeaconLoss->aucBssid[1],
								pExtEventBeaconLoss->aucBssid[2],
								pExtEventBeaconLoss->aucBssid[3],
								pExtEventBeaconLoss->aucBssid[4],
								pExtEventBeaconLoss->aucBssid[5]));
}
#endif /*CONFIG_STA_SUPPORT*/


static VOID ExtEventFwLog2HostHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("FW LOG: %s\n", Data));
}

#ifdef MT7636_BTCOEX
static VOID ExtEventBTCoexHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UCHAR SubOpCode;
	MAC_TABLE_ENTRY *pEntry;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->ExtEventBTCoexHandler\n", Data));
	struct _EVENT_EXT_COEXISTENCE_T *coext_event_t = (struct _EVENT_EXT_COEXISTENCE_T *)Data;

	SubOpCode = coext_event_t->ucSubOpCode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SubOpCode: 0x%x\n", coext_event_t->ucSubOpCode));
	hex_dump("Coex Event payload ", coext_event_t->aucBuffer, Length);

	if (SubOpCode == 0x01)
	{
		struct _EVENT_COEX_CMD_RESPONSE_T *CoexResp = (struct _EVENT_COEX_CMD_RESPONSE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->Cmd_Resp=0x%x\n", CoexResp->u4Status));

	}
	else if (SubOpCode == 0x02)
	{
		struct _EVENT_COEX_REPORT_COEX_MODE_T *CoexReportMode= (struct _EVENT_COEX_REPORT_COEX_MODE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->SupportCoexMode=0x%x\n", CoexReportMode->u4SupportCoexMode));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->CurrentCoexMode=0x%x\n", CoexReportMode->u4CurrentCoexMode));
		pAd->BtCoexMode = ((CoexReportMode->u4CurrentCoexMode) & 0xFF);
		pAd->BtCoexLiveTime= ((CoexReportMode->u4CurrentCoexMode) & 0xFF0000) >> 15;
	}
	else if (SubOpCode == 0x03)
	{
		struct _EVENT_COEX_MASK_OFF_TX_RATE_T *CoexMaskTxRate= (struct _EVENT_COEX_MASK_OFF_TX_RATE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->MASK_OFF_TX_RATE=0x%x\n", CoexMaskTxRate->ucOn));

	}
	else if (SubOpCode == 0x04)
	{
		struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *CoexChgBaSize= (struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->Change_BA_Size ucOn=%d \n", CoexChgBaSize->ucOn));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->Change_BA_Size=0x%x\n", CoexChgBaSize->ucRXBASize));
		pEntry = &pAd->MacTab.Content[BSSID_WCID];


		if (CoexChgBaSize->ucOn == 1)
		{
			BA_REC_ENTRY *pBAEntry = NULL;
			UCHAR Idx;

			Idx = pEntry->BARecWcidArray[0];
			pBAEntry = &pAd->BATable.BARecEntry[Idx];
			pAd->BtCoexBASize = CoexChgBaSize->ucRXBASize;

			if (pBAEntry->BAWinSize > pAd->BtCoexBASize)
			{
				pAd->BtCoexOriBASize = pBAEntry->BAWinSize;
				pAd->CommonCfg.BACapability.field.RxBAWinLimit = pAd->BtCoexBASize;
				BAOriSessionTearDown(pAd, BSSID_WCID, 0, FALSE, FALSE);
				BARecSessionTearDown(pAd, BSSID_WCID, 0, FALSE);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("COEX: TDD mode: Set RxBASize to %d\n", pAd->BtCoexBASize));
				if (pEntry != NULL )
					BAOriSessionSetUp(pAd, pEntry, 0, 0, 100, TRUE);
			}
		}
		else
		{
				pAd->CommonCfg.BACapability.field.RxBAWinLimit = pAd->BtCoexOriBASize;
				BAOriSessionTearDown(pAd, BSSID_WCID, 0, FALSE, FALSE);
				BARecSessionTearDown(pAd, BSSID_WCID, 0, FALSE);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("COEX: TDD mode: Set RxBASize to %d\n", pAd->BtCoexOriBASize));
				if (pEntry != NULL )
					BAOriSessionSetUp(pAd, pEntry, 0, 0, 100, TRUE);

		}
	}
	else if (SubOpCode == 0x05)
	{
		struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *CoexLimitBeacon= (struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->COEX_LIMIT_BEACON_SIZE ucOn =%d\n", CoexLimitBeacon->ucOn));

	}
	else if (SubOpCode == 0x06)
	{
		struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *CoexExtendBTORoam= (struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--->EVENT_COEX_EXTEND_BNCTO_ROAMING ucOn =%d\n", CoexExtendBTORoam->ucOn));

	}
}
#endif

static VOID EventExtEventHandler(RTMP_ADAPTER *pAd, UINT8 ExtEID, UINT8 *Data, UINT32 Length)
{
	switch (ExtEID)
	{
		case EXT_EVENT_CMD_RESULT:
			EventExtCmdResultHandler(pAd, Data, Length);
			break;
		case EXT_EVENT_FW_LOG_2_HOST:
			ExtEventFwLog2HostHandler(pAd, Data, Length);
			break;
#ifdef MT_PS			
#if defined(MT7603) || defined(MT7628)
		case EXT_CMD_PS_RETRIEVE_START:
			CmdPsRetrieveStartRspFromCR(pAd, Data, Length);
			break;
#endif /* MT7603 || MT7628 */
#endif
#ifdef MT7636_BTCOEX
		case EXT_EVENT_BT_COEX:
			ExtEventBTCoexHandler(pAd, Data, Length);
			break;
#endif /*MT7636_BTCOEX*/
#ifdef THERMAL_PROTECT_SUPPORT
		case EXT_EVENT_THERMAL_PROTECT:
			EventThermalProtect(pAd, Data, Length);
			break;
#endif
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_SDIO
		case EXT_EVENT_SLEEPY_NOTIFY:
			ExtEventSleepyNotifyHandler(pAd, Data, Length);
			break;
#endif
		case EXT_EVENT_BEACON_LOSS:
			ExtEventBeaconLostHandler(pAd, Data, Length);
			break;
#endif /*CONFIG_STA_SUPPORT*/
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Unknown Ext Event(%x)\n", __FUNCTION__,
										ExtEID));
			break;
	}


}


static VOID UnsolicitedEventHandler(RTMP_ADAPTER *pAd, UINT8 EID, UINT8 ExtEID, UINT8 *Data, UINT32 Length)
{
	switch (EID)
	{
		case EVENT_CH_PRIVILEGE:
			EventChPrivilegeHandler(pAd, Data, Length);
			break;
		case EXT_EVENT:
			EventExtEventHandler(pAd, ExtEID, Data, Length);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Unknown Event(%x)\n", __FUNCTION__, EID));
			break;
	}
}


static VOID AndesMTRxProcessEvent(RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	struct cmd_msg *msg, *msg_tmp;
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	unsigned long flags;

	//event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_EVENT_INFO(event_rxd->fw_rxd_0.field.length, event_rxd->fw_rxd_0.field.pkt_type_id,
								event_rxd->fw_rxd_1.field.eid, event_rxd->fw_rxd_1.field.seq_num,
							event_rxd->fw_rxd_2.field.ext_eid, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd), event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd));
#endif



	if ((event_rxd->fw_rxd_1.field.seq_num == 0) ||
			(event_rxd->fw_rxd_2.field.ext_eid == EXT_EVENT_FW_LOG_2_HOST) ||
			(event_rxd->fw_rxd_2.field.ext_eid == EXT_EVENT_THERMAL_PROTECT)) {
		/* if have callback function */
		UnsolicitedEventHandler(pAd, event_rxd->fw_rxd_1.field.eid, event_rxd->fw_rxd_2.field.ext_eid, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd),
												event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd));
	}
	else
	{

#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
#endif
		DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
			if (msg->seq == event_rxd->fw_rxd_1.field.seq_num)
			{
			        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s (seq=%d)\n", __FUNCTION__, msg->seq));
				_AndesUnlinkCmdMsg(msg, &ctl->ackq);

#ifdef RTMP_PCI_SUPPORT
				RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
#endif

				if ((event_rxd->fw_rxd_1.field.eid == MT_FW_START_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == MT_RESTART_DL_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == MT_TARGET_ADDRESS_LEN_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == MT_PATCH_SEM_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == EVENT_ACCESS_REG))
				{
					msg->rsp_handler(msg, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd) - 4,
											event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd) + 4);

				}
				else if ((msg->rsp_payload_len == event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd))
						&& (msg->rsp_payload_len != 0))
				{
					if (msg->rsp_handler == NULL)
					{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): rsp_handler is NULL!!!! (cmd_type = 0x%x, ext_cmd_type = 0x%x)\n",
							__FUNCTION__, msg->cmd_type, msg->ext_cmd_type));
					}
					else
					{
						msg->rsp_handler(msg, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd),
												event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd));
					}
				}
				else if ((msg->rsp_payload_len == 0) &&
						(msg->rsp_handler == NULL) &&
						((event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd)) == 8))
				{
					/* Just need to wait for the command finished */
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("command response(ack) success\n"));
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("expect response len(%d), command response len(%zd) invalid\n",
									msg->rsp_payload_len, event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd)));
					msg->rsp_payload_len = event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd);
				}

				if (msg->need_wait) {
					RTMP_OS_COMPLETE(&msg->ack_done);
				} else {
					AndesFreeCmdMsg(msg);
				}

#ifdef RTMP_PCI_SUPPORT
				RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
#endif

				break;
			}
		}


#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
#endif
	}
}


VOID AndesMTRxEventHandler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	EVENT_RXD *event_rxd = (EVENT_RXD *)data;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __FUNCTION__));

	msg = AndesAllocCmdMsg(pAd, event_rxd->fw_rxd_0.field.length);

	if (!msg)
		return;

	AndesAppendCmdMsg(msg, (char *)data, event_rxd->fw_rxd_0.field.length);

	AndesMTRxProcessEvent(pAd, msg);

#ifdef RTMP_PCI_SUPPORT
	if (msg->net_pkt)
		RTMPFreeNdisPacket(pAd, msg->net_pkt);
#endif

	AndesFreeCmdMsg(msg);
}


#ifdef RTMP_PCI_SUPPORT
VOID AndesMTPciFwInit(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	Ctl->Stage = FW_NO_INIT;
	/* Enable Interrupt*/
	RTMP_IRQ_ENABLE(pAd);
	RT28XXDMAEnable(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
}


VOID AndesMTPciFwExit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	RT28XXDMADisable(pAd);
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
}
#endif /* RTMP_PCI_SUPPORT */






static VOID CmdEfuseBufferModeRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

static VOID CmdThemalSensorRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_GET_SENSOR_RESULT_T *EventExtCmdResult = (struct _EXT_EVENT_GET_SENSOR_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	EventExtCmdResult->u4SensorResult = le2cpu32(EventExtCmdResult->u4SensorResult);	
	NdisMoveMemory(msg->rsp_payload, &EventExtCmdResult->u4SensorResult, sizeof(EventExtCmdResult->u4SensorResult));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ThemalSensor = 0x%x\n", EventExtCmdResult->u4SensorResult));
}

static VOID CmdExtPmMgtBitRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

static VOID CmdExtPmStateCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

VOID CmdEfusBufferModeSet(RTMP_ADAPTER *pAd)
{

	struct cmd_msg *msg;
	EXT_CMD_EFUSE_BUFFER_MODE_T CmdEfuseBufferMode;
	int ret = 0;
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EFUSE_BUFFER_MODE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_EFUSE_BUFFER_MODE, TRUE, 0,TRUE, TRUE, 8, NULL, CmdEfuseBufferModeRsp);
	memset(&CmdEfuseBufferMode, 0x00, sizeof(CmdEfuseBufferMode));
	switch(pAd->eeprom_type){
	case EEPROM_EFUSE:
	{
		CmdEfuseBufferMode.ucSourceMode = EEPROM_MODE_EFUSE;
		CmdEfuseBufferMode.ucCount = 0;
	}
	break;
	case EEPROM_FLASH:
	{
		CmdEfuseBufferMode.ucSourceMode = EEPROM_MODE_BUFFER;
		if(pAd->chipOps.bufferModeEfuseFill)
		{
			pAd->chipOps.bufferModeEfuseFill(pAd,&CmdEfuseBufferMode);

		}else
		{
			/*force to efuse mode*/
			CmdEfuseBufferMode.ucSourceMode = EEPROM_MODE_EFUSE;
			CmdEfuseBufferMode.ucCount = 0;
		}

	}
	break;
	default:
		goto error;
	break;
	}

	AndesAppendCmdMsg(msg, (char *)&CmdEfuseBufferMode, sizeof(CmdEfuseBufferMode));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return;

}

VOID CmdSetTxPowerCtrl(RTMP_ADAPTER *pAd, UINT8 central_chl)
{

	struct cmd_msg *msg;
	EXT_CMD_TX_POWER_CTRL_T CmdTxPwrCtrl;
	int ret = 0;
	int i;
	UINT8 PwrPercentageDelta = 0;
	//struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TX_POWER_CTRL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_SET_TX_POWER_CTRL, TRUE, 0,
		TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);
	memset(&CmdTxPwrCtrl, 0x00, sizeof(CmdTxPwrCtrl));

	CmdTxPwrCtrl.ucCenterChannel = central_chl;
	CmdTxPwrCtrl.ucTSSIEnable = pAd->EEPROMImage[NIC_CONFIGURE_1_TOP];
	CmdTxPwrCtrl.ucTempCompEnable = pAd->EEPROMImage[NIC_CONFIGURE_1];

	CmdTxPwrCtrl.aucTargetPower[0] = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	CmdTxPwrCtrl.aucTargetPower[1] = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
#ifdef CONFIG_ATE
        /* Replace Target Power from QA Tool manual setting*/
	if (ATE_ON(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s,ATE set tx power\n", __FUNCTION__));
		CmdTxPwrCtrl.aucTargetPower[0] = pAd->ATECtrl.TxPower0;
		CmdTxPwrCtrl.aucTargetPower[1] = pAd->ATECtrl.TxPower1;
	}
#endif
	NdisCopyMemory(&CmdTxPwrCtrl.aucRatePowerDelta[0], &(pAd->EEPROMImage[TX_PWR_CCK_1_2M]), sizeof(CmdTxPwrCtrl.aucRatePowerDelta));
	CmdTxPwrCtrl.ucBWPowerDelta = pAd->EEPROMImage[G_BAND_20_40_BW_PWR_DELTA];
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[0], &(pAd->EEPROMImage[TX0_G_BAND_OFFSET_LOW]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[0]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[1], &(pAd->EEPROMImage[TX0_G_BAND_CHL_PWR_DELTA_MID]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[1]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[2], &(pAd->EEPROMImage[TX0_G_BAND_OFFSET_HIGH]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[2]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[5], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_HIGH]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[5]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[3], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELATE_LOW]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[3]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[4], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_MID]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[4]));
	NdisCopyMemory(&CmdTxPwrCtrl.aucTempCompPower[0], &(pAd->EEPROMImage[STEP_NUM_NEG_7]), sizeof(CmdTxPwrCtrl.aucTempCompPower));

	if (1)
	{	
		if (pAd->CommonCfg.TxPowerPercentage > 90)
		{
			PwrPercentageDelta = 0;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 60) /* reduce Pwr for 1 dB. */
		{
			PwrPercentageDelta = 1;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 30) /* reduce Pwr for 3 dB. */
		{
			PwrPercentageDelta = 3;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 15) /* reduce Pwr for 6 dB. */
		{
			PwrPercentageDelta = 6;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 9)	/* reduce Pwr for 9 dB. */
		{
			PwrPercentageDelta = 9;
		}
		else /* reduce Pwr for 12 dB. */
		{
			PwrPercentageDelta = 12;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PwrPercentageDelta = 0x%x\n", PwrPercentageDelta));
		CmdTxPwrCtrl.ucReserved = PwrPercentageDelta;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucCenterChannel=%x\n", CmdTxPwrCtrl.ucCenterChannel));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucTSSIEnable=%x \n", CmdTxPwrCtrl.ucTSSIEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucTempCompEnable=%x\n", CmdTxPwrCtrl.ucTempCompEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucTargetPower[0]=%x\n", CmdTxPwrCtrl.aucTargetPower[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucTargetPower[1]=%x\n", CmdTxPwrCtrl.aucTargetPower[1]));
	for(i=0; i<14;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucRatePowerDelta[%d]=%x\n", i, CmdTxPwrCtrl.aucRatePowerDelta[i]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.ucBWPowerDelta=%x \n",CmdTxPwrCtrl.ucBWPowerDelta));
	for(i=0;i<6;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucCHPowerDelta[%d]=%x\n", i, CmdTxPwrCtrl.aucCHPowerDelta[i]));
	for(i=0;i<17;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdTxPwrCtrl.aucTempCompPower[%d]=%x\n", i, CmdTxPwrCtrl.aucTempCompPower[i]));

	AndesAppendCmdMsg(msg, (char *)&CmdTxPwrCtrl, sizeof(CmdTxPwrCtrl));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));

}

VOID CmdGetThermalSensorResult(RTMP_ADAPTER *pAd, UINT8 ActionIdx, UINT32 *SensorResult)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_SENSOR_RESULT_T CmdSensorResult;
	INT32 ret;

	ret = 0;

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_SENSOR_RESULT_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
#ifdef CONFIG_ATE
	if(!ATE_ON(pAd)){
#endif
	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_GET_THEMAL_SENSOR,TRUE, 0,
							TRUE, TRUE, 8, (UINT8 *)SensorResult, CmdThemalSensorRsp);
#ifdef CONFIG_ATE
	}else{
		ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
		#ifdef CONFIG_QA
		if(ATECtrl->bQAEnabled){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Call to qa_agent\n", __FUNCTION__));
			AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_GET_THEMAL_SENSOR, TRUE, 0, TRUE, TRUE, 8, NULL, HQA_GetThermalValue_CB);
		}
		#else
		AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_GET_THEMAL_SENSOR,TRUE, 0,
							TRUE, TRUE, 8, (UINT8 *)SensorResult, CmdThemalSensorRsp);
		#endif
	}
#endif

	CmdSensorResult.ucActionIdx = ActionIdx;

	AndesAppendCmdMsg(msg, (char *)&CmdSensorResult, sizeof(CmdSensorResult));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
}

#ifdef MT7636_BTCOEX
INT AndesCoexOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Status)
{
	struct cmd_msg *msg;
	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_WIFI_STATUS_UPDATE_T coex_status;
	UINT32 SetBtWlanStatus;

	ret = 0;

	NdisZeroMemory(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	NdisZeroMemory(&coex_status, sizeof(COEX_WIFI_STATUS_UPDATE_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_BT_COEX, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	coext_t.ucSubOpCode= COEX_WIFI_STATUS_UPDATE;
	coex_status.u4WIFIStatus = 0x00;

	SetBtWlanStatus = pAd->BtWlanStatus;
	switch(Status)
	{
		case STATUS_RADIO_ON:
			SetBtWlanStatus |= COEX_STATUS_RADIO_ON;
			break;
		case STATUS_RADIO_OFF:
			SetBtWlanStatus &= ~( COEX_STATUS_RADIO_ON);
			break;
		case STATUS_SCAN_G_BAND:
			SetBtWlanStatus |= COEX_STATUS_SCAN_G_BAND;
			break;
		case STATUS_SCAN_G_BAND_END:
			SetBtWlanStatus &= ~( COEX_STATUS_SCAN_G_BAND);
			break;
		case STATUS_SCAN_A_BAND:
			SetBtWlanStatus |= COEX_STATUS_SCAN_A_BAND;
			break;
		case STATUS_SCAN_A_BAND_END:
			SetBtWlanStatus &= ~( COEX_STATUS_SCAN_A_BAND);
			break;
		case STATUS_LINK_UP:
			SetBtWlanStatus |= COEX_STATUS_LINK_UP;
			break;
		case STATUS_LINK_DOWN:
			SetBtWlanStatus &= ~( COEX_STATUS_LINK_UP);
			break;
		case STATUS_BT_OVER_WIFI:
			SetBtWlanStatus |= COEX_STATUS_BT_OVER_WIFI;
			break;
		default: /* fatal error */
			break;
	} /* End of switch */


	if (SetBtWlanStatus == pAd->BtWlanStatus)
		goto error;
	else
		pAd->BtWlanStatus = SetBtWlanStatus;

	coex_status.u4WIFIStatus = pAd->BtWlanStatus;
	/* Parameter */

	NdisMoveMemory(coext_t.aucData, &coex_status,sizeof(COEX_WIFI_STATUS_UPDATE_T));

	hex_dump("AndesBtCoex: ", &coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t,sizeof(EXT_CMD_COEXISTENCE_T));


	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

INT AndesCoexProtectionFrameOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Mode,
	IN UCHAR Rate)
{
	struct cmd_msg *msg;
	INT32 ret;
	int i;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_SET_PROTECTION_FRAME_T coex_proction;

	ret = 0;

	NdisZeroMemory(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	NdisZeroMemory(&coex_proction, sizeof(COEX_SET_PROTECTION_FRAME_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_BT_COEX, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	coext_t.ucSubOpCode= COEX_SET_PROTECTION_FRAME;
	coex_proction.ucProFrameMode = Mode;
	coex_proction.ucProFrameRate = Rate;

	NdisMoveMemory(coext_t.aucData, &coex_proction,sizeof(COEX_SET_PROTECTION_FRAME_T));

	hex_dump("AndesBtCoexProtection: ", &coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t,sizeof(EXT_CMD_COEXISTENCE_T));


	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT AndesCoexBSSInfo(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN Enable,
	IN UCHAR bQoS)
{
	struct cmd_msg *msg;
	INT32 ret;
	int i;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_UPDATE_BSS_INFO_T coex_bss_info;

	ret = 0;

	NdisZeroMemory(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	NdisZeroMemory(&coex_bss_info, sizeof(COEX_UPDATE_BSS_INFO_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_BT_COEX, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	coext_t.ucSubOpCode= COEX_UPDATE_BSS_INFO;

	if (Enable)
	{
		coex_bss_info.u4BSSPresence[0] = 0x1;
		coex_bss_info.u4IsQBSS[0] = bQoS;
	}
	else
	{
		coex_bss_info.u4BSSPresence[0] = 0x0;
		coex_bss_info.u4IsQBSS[0] = 0;
	}
	NdisMoveMemory(coext_t.aucData, &coex_bss_info,sizeof(COEX_UPDATE_BSS_INFO_T));

	hex_dump("AndesBtCoexProtection: ", &coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t,sizeof(EXT_CMD_COEXISTENCE_T));


	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}
#endif

#ifdef CONFIG_MULTI_CHANNEL

int CmdMccStart(struct _RTMP_ADAPTER *ad,
                    u8 channel_1st,
                    u8 channel_2nd,
                    unsigned int bw_1st,
                    unsigned int bw_2nd,
                    u8 central_1st_seg0,
                    u8 central_1st_seg1,
                    u8 central_2nd_seg0,
                    u8 central_2nd_seg1,
                    u8 role_1st,
                    u8 role_2nd,
                    u16 stay_time_1st,
                    u16 stay_time_2nd,
                    u16 idle_time,
                    u16 null_repeat_cnt,
                    u32 start_tsf)
{
	struct cmd_msg *msg;
    EXT_CMD_MCC_START_T mcc_start_msg;
	int ret;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\x1b[31m @@@@@ %s:channel(%u,%u), bw(%u,%u), role(%u,%u), cycle_time(%u,%u), wait_time(%u), null_cnt(%u), start_tsf(0x%ld) \x1b[m\n",
		__FUNCTION__, channel_1st, channel_2nd, bw_1st, bw_2nd,
		role_1st, role_2nd, stay_time_1st, stay_time_2nd,
		idle_time, null_repeat_cnt, start_tsf));

	msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_MCC_START_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_MCC_OFFLOAD_START, TRUE, 0,
		TRUE, TRUE, 0, NULL, NULL);


    NdisZeroMemory(&mcc_start_msg, sizeof(EXT_CMD_MCC_START_T));

    mcc_start_msg.u2IdleInterval = idle_time; /* ms */
        mcc_start_msg.ucRepeatCnt = null_repeat_cnt;
        mcc_start_msg.ucStartIdx = 0;
        mcc_start_msg.u4StartInstant = start_tsf;
        mcc_start_msg.u2FreePSEPageTh = 0x11; /* 0:  Disable PSE threshold check */
        mcc_start_msg.ucPreSwitchInterval = 0; /* for SDIO */

    mcc_start_msg.ucWlanIdx0 = 1; /* WTBL-0  ???  */
        mcc_start_msg.ucPrimaryChannel0 =  channel_1st;
        mcc_start_msg.ucCenterChannel0Seg0 = central_1st_seg0;
        mcc_start_msg.ucCenterChannel0Seg1 = central_1st_seg1;
        mcc_start_msg.ucBandwidth0 = bw_1st;
        mcc_start_msg.ucTrxStream0 = 0; /* 2T2R  */
        mcc_start_msg.u2StayInterval0 = stay_time_1st;
        mcc_start_msg.ucRole0 = role_1st;
        mcc_start_msg.ucOmIdx0 = 0; /* ??? */
        mcc_start_msg.ucBssIdx0 = 0; /* ??? */
        mcc_start_msg.ucWmmIdx0 = 0;

    mcc_start_msg.ucWlanIdx1 = 2; /* WTBL-1 ??? */
        mcc_start_msg.ucPrimaryChannel1 = channel_2nd;
        mcc_start_msg.ucCenterChannel1Seg0 = central_2nd_seg0;
        mcc_start_msg.ucCenterChannel1Seg1 = central_2nd_seg1;
        mcc_start_msg.ucBandwidth1 = bw_2nd;
        mcc_start_msg.ucTrxStream1 = 0; /* 2T2R */
        mcc_start_msg.u2StayInterval1 = stay_time_2nd;
        mcc_start_msg.ucRole1 = role_2nd;
        mcc_start_msg.ucOmIdx1 = 1; /* ??? */
        mcc_start_msg.ucBssIdx1 = 1; /* ??? */
        mcc_start_msg.ucWmmIdx1 = 1;


	AndesAppendCmdMsg(msg, (char *)&mcc_start_msg, sizeof(EXT_CMD_MCC_START_T));
	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}


int CmdMccStop(struct _RTMP_ADAPTER *ad, u8 parking_idx, u8 auto_resume_mode, u16 auto_resume_interval, u32 auto_resume_tsf)
{
	struct cmd_msg *msg;
    EXT_CMD_MCC_STOP_T mcc_stop_msg;
	int ret;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[32m @@@@@ %s:parking_channel_idx(%u), auto_resume_mode(%u), auto_resume_tsf(0x%08x) \x1b[m\n",
		__FUNCTION__, parking_idx, auto_resume_mode, auto_resume_tsf));

	msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_MCC_STOP_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_MCC_OFFLOAD_STOP, TRUE, 0,
		TRUE, TRUE, 0, NULL, NULL);



    NdisZeroMemory(&mcc_stop_msg, sizeof(EXT_CMD_MCC_STOP_T));

    mcc_stop_msg.ucParkIdx = parking_idx;
        mcc_stop_msg.ucAutoResumeMode = auto_resume_mode;
        mcc_stop_msg.u2AutoResumeInterval = auto_resume_interval;
        mcc_stop_msg.u4AutoResumeInstant = auto_resume_tsf;
        mcc_stop_msg.u2IdleInterval  = 0; /* no resume */
        mcc_stop_msg.u2StayInterval0 = 0; /* no resume */
        mcc_stop_msg.u2StayInterval1 = 0; /* no resume */


	AndesAppendCmdMsg(msg, (char *)&mcc_stop_msg, sizeof(EXT_CMD_MCC_STOP_T));

	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}


#endif /* CONFIG_MULTI_CHANNEL */




#ifdef RTMP_EFUSE_SUPPORT
static VOID CmdEfuseAccessReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	CMD_ACCESS_EFUSE_T *pCmdAccessEfuse = (CMD_ACCESS_EFUSE_T *)data;
	EFUSE_ACCESS_DATA_T *pEfuseValue=NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __FUNCTION__));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Address:%x,IsValied:%x\n",pCmdAccessEfuse->u4Address,pCmdAccessEfuse->u4Valid));

	pEfuseValue = (EFUSE_ACCESS_DATA_T*)msg->rsp_payload;

	*pEfuseValue->pIsValid = pCmdAccessEfuse->u4Valid;

	NdisMoveMemory(&pEfuseValue->pValue[0],&pCmdAccessEfuse->aucData[0] ,EFUSE_BLOCK_SIZE);
}



VOID CmdEfuseAccessRead(RTMP_ADAPTER *pAd, USHORT offset,PUCHAR pData, PUINT pIsValid)
{

	struct cmd_msg *msg;
	CMD_ACCESS_EFUSE_T CmdAccessEfuse;
	EFUSE_ACCESS_DATA_T efuseAccessData;
	int ret = 0;
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_ACCESS_EFUSE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	NdisZeroMemory(&efuseAccessData, sizeof(efuseAccessData));

	efuseAccessData.pValue = pData;
	efuseAccessData.pIsValid = pIsValid;

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_ID_EFUSE_ACCESS, TRUE, 0, TRUE, TRUE, sizeof(CMD_ACCESS_EFUSE_T), (char *)&efuseAccessData, CmdEfuseAccessReadCb);

	NdisZeroMemory(&CmdAccessEfuse, sizeof(CmdAccessEfuse));

	CmdAccessEfuse.u4Address = (offset/EFUSE_BLOCK_SIZE)*EFUSE_BLOCK_SIZE;

	AndesAppendCmdMsg(msg, (char *)&CmdAccessEfuse, sizeof(CmdAccessEfuse));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
}


static VOID CmdEfuseAccessWriteCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	CMD_ACCESS_EFUSE_T *pCmdAccessEfuse = (CMD_ACCESS_EFUSE_T *)data;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Address:%x,IsValied:%x\n",pCmdAccessEfuse->u4Address,pCmdAccessEfuse->u4Valid));

}


VOID CmdEfuseAccessWrite(RTMP_ADAPTER *pAd, USHORT offset,PUCHAR pData)
{

	struct cmd_msg *msg;
	CMD_ACCESS_EFUSE_T CmdAccessEfuse;
	int ret = 0;
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_ACCESS_EFUSE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_EFUSE_ACCESS, TRUE, 0,TRUE, TRUE, sizeof(CMD_ACCESS_EFUSE_T), NULL, CmdEfuseAccessWriteCb);

	NdisZeroMemory(&CmdAccessEfuse, sizeof(CmdAccessEfuse));

	NdisMoveMemory(&CmdAccessEfuse.aucData[0],&pData[0],EFUSE_BLOCK_SIZE);
	CmdAccessEfuse.u4Address = (offset/EFUSE_BLOCK_SIZE)*EFUSE_BLOCK_SIZE;

	AndesAppendCmdMsg(msg, (char *)&CmdAccessEfuse, sizeof(CmdAccessEfuse));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
}

static VOID CmdExtStaRecUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

INT32 CmdExtStaRecUpdate(RTMP_ADAPTER *pAd, UINT8 ucBssIndex, UINT8 ucWlanIdx, UINT32 u4EnableFeature)
{
	struct cmd_msg 			*msg = NULL;
	PMAC_TABLE_ENTRY		pEntry = &pAd->MacTab.Content[ucWlanIdx];
	CMD_STAREC_UPDATE_T	CmdStaRecUpdate = {0};
	INT32					Ret = 0;
	UINT8					i = 0;
	UINT8					ucTLVNumber = 0;

	msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_STA_REC + 100);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	for (i = 0; i < STA_REC_MAX_NUM; i++)
	{
		if (u4EnableFeature & (1 << i))
		{
			ucTLVNumber++;
		}
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_STAREC_UPDATE, TRUE, 0,TRUE, TRUE, 8, NULL, CmdExtStaRecUpdateRsp);

	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = ucBssIndex;
	CmdStaRecUpdate.ucWlanIdx = ucWlanIdx;
	CmdStaRecUpdate.u2TotalElementNum = ucTLVNumber;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, sizeof(CMD_STAREC_UPDATE_T));

	/* Fill RA related parameters */
	if (u4EnableFeature & (1 << STA_REC_AUTO_RATE_PARAMETER))
	{
		CMD_STAREC_AUTO_RATE_T CmdStaRecAutoRate = {0};

		/* Fill TLV format */
		CmdStaRecAutoRate.u2Tag = STA_REC_AUTO_RATE_PARAMETER;
		CmdStaRecAutoRate.u2Length = sizeof(CMD_STAREC_AUTO_RATE_T);
		CmdStaRecAutoRate.fgRaValid = TRUE;
		CmdStaRecAutoRate.ucRaTable = 4;
		CmdStaRecAutoRate.fgRaMcs32Supported = TRUE;
		CmdStaRecAutoRate.fgRaHtCapInfoGF = FALSE;
		NdisMoveMemory(&CmdStaRecAutoRate.rRaHtTransmitSetting, &pEntry->HTPhyMode, sizeof(HTTRANSMIT_SETTING_T));
		CmdStaRecAutoRate.aucRaHtCapMCSSet[0] = 0xff;
		CmdStaRecAutoRate.aucRaHtCapMCSSet[1] = 0xff;
		CmdStaRecAutoRate.aucRaHtCapMCSSet[2] = 0;
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAutoRate, sizeof(CMD_STAREC_AUTO_RATE_T));
	}

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

#endif /* RTMP_EFUSE_SUPPORT */

INT32 CmdEdcaParameterSet(RTMP_ADAPTER *pAd, CMD_EDCA_SET_T EdcaParam)
{
	struct cmd_msg *msg;
	INT32 ret=0,size=0;

	size = 4+sizeof(TX_AC_PARAM_T)*EdcaParam.ucTotalNum;

	msg = AndesAllocCmdMsg(pAd, size);

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_EDCA_SET, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	AndesAppendCmdMsg(msg, (char *)&EdcaParam,size);


	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}



 INT32 CmdSlotTimeSet(RTMP_ADAPTER *pAd, UINT8 SlotTime,UINT8 SifsTime,UINT8 RifsTime,UINT16 EifsTime)
 {
	struct cmd_msg *msg;
	INT32 ret=0;

	CMD_SLOT_TIME_SET_T cmdSlotTime;

	NdisZeroMemory(&cmdSlotTime,sizeof(CMD_SLOT_TIME_SET_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_SLOT_TIME_SET_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_SLOT_TIME_SET, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	cmdSlotTime.u2Eifs = EifsTime;
	cmdSlotTime.ucRifs = RifsTime;
	cmdSlotTime.ucSifs = SifsTime;
	cmdSlotTime.ucSlotTime = SlotTime;

	AndesAppendCmdMsg(msg, (char *)&cmdSlotTime,sizeof(CMD_SLOT_TIME_SET_T));


	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
 }

/* 7636 psm */
INT32 CmdExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit)
{
	struct cmd_msg *msg;
	EXT_CMD_PWR_MGT_BIT_T PwrMgtBitWifi = {0};
	INT32 Ret = 0;

// For now, only support SW control
#ifdef MT7628
    return 0;
#endif
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PWR_MGT_BIT_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	PwrMgtBitWifi.ucWlanIdx = ucWlanIdx;
	PwrMgtBitWifi.ucPwrMgtBit = ucPwrMgtBit;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:ucWlanIdx(%d), ucPwrMgtBit(%d)\n", __FUNCTION__, ucWlanIdx, ucPwrMgtBit));

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_PWR_MGT_BIT_WIFI, FALSE, 0,TRUE, TRUE, 8, NULL, CmdExtPmMgtBitRsp);

	AndesAppendCmdMsg(msg, (char *)&PwrMgtBitWifi, sizeof(PwrMgtBitWifi));

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


 /*1: enter, 2: exit specific PM state*/
INT32 CmdExtPmStateCtrl(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPmNumber, UINT8 ucPmState)
{
#ifdef CONFIG_STA_SUPPORT
	static UINT32	u4RfCr = 0;
    UINT32  u4TempRfCr = 0;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[ucWlanIdx];
#endif /*CONFIG_STA_SUPPORT*/
	struct cmd_msg *msg = NULL;
	EXT_CMD_PM_STATE_CTRL_T CmdPmStateCtrl = {0};
	INT32 Ret = 0;

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PM_STATE_CTRL_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Fill parameter here*/
	CmdPmStateCtrl.ucWlanIdx = cpu2le32(ucWlanIdx);
	CmdPmStateCtrl.ucPmNumber = cpu2le32(ucPmNumber);
	CmdPmStateCtrl.ucPmState = cpu2le32(ucPmState);
#ifdef CONFIG_STA_SUPPORT
	NdisMoveMemory(CmdPmStateCtrl.aucBssid, pEntry->PairwiseKey.BssId, MAC_ADDR_LEN);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(%x, %x, %x), (%d, %d)\n",
								__FUNCTION__,
								pEntry->PairwiseKey.BssId[0],
								pEntry->PairwiseKey.BssId[1],
								pEntry->PairwiseKey.BssId[2],
								pAd->MlmeAux.BeaconPeriod,
								pAd->MlmeAux.DtimPeriod));
#endif /*CONFIG_STA_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	if (ucPmState == ENTER_PM_STATE)
	{
        CmdPmStateCtrl.ucDtimPeriod = cpu2le32(pAd->MlmeAux.DtimPeriod);
        CmdPmStateCtrl.u2BcnInterval = cpu2le32(pAd->MlmeAux.BeaconPeriod);
        CmdPmStateCtrl.u4Aid = cpu2le32(pAd->StaActive.Aid);

		RTMP_IO_READ32(pAd, RMAC_RFCR, &u4RfCr);
        u4TempRfCr = u4RfCr;
        u4TempRfCr |= (DROP_PROBE_REQ | DROP_NOT_MY_BSSID | DROP_DIFF_BSSID_BCN);
        CmdPmStateCtrl.u4RxFilter = cpu2le32(u4TempRfCr);

        CmdPmStateCtrl.u4Feature = cpu2le32(PM_CMD_FEATURE_PSPOLL_OFFLOAD);
        CmdPmStateCtrl.ucOwnMacIdx = cpu2le32(0); //AsicSetDevMac
		// TODO: Hanmim 7636 psm
        CmdPmStateCtrl.ucWmmIdx = cpu2le32(0);      // 0: WMM1, 1: WMM2
        CmdPmStateCtrl.ucBcnLossCount = cpu2le32(5); // 2.5sec
        CmdPmStateCtrl.ucBcnSpDuration = cpu2le32(0);
	}
	else
	{
        CmdPmStateCtrl.u4RxFilter = cpu2le32(u4RfCr);
	}
#endif /*CONFIG_STA_SUPPORT*/

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_PM_STATE_CTRL, FALSE, 0,TRUE, TRUE, 8, NULL, CmdExtPmStateCtrlRsp);

	AndesAppendCmdMsg(msg, (char *)&CmdPmStateCtrl, sizeof(CmdPmStateCtrl));

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}
#ifdef LED_CONTROL_SUPPORT
INT AndesLedEnhanceOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR LedIdx,
	IN UCHAR on_time,
	IN UCHAR off_time,
	IN UCHAR Led_Parameter
	)
{
	struct cmd_msg *msg;
	CHAR *Pos, *pBuf;
	UINT32 VarLen;
	UINT32 arg0;
	INT32 ret;
	LED_ENHANCE led_enhance;
	
	msg = AndesAllocCmdMsg(pAd, sizeof(LED_NMAC_CMD));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_LED_CTRL, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	//memset(&CmdEfuseBufferMode, 0x00, sizeof(CmdEfuseBufferMode));

	
	/* Calibration ID and Parameter */
	VarLen = 8;
	arg0 = LedIdx;
	led_enhance.word = 0;
	led_enhance.field.on_time=on_time;
	led_enhance.field.off_time=off_time;
	led_enhance.field.tx_blink=2;		// 2 : data only
	if (IS_MT7628(pAd))
	{
		/* invert polarity for Open-Drain WLED */
		led_enhance.field.reverse_polarity=1;
		if (Led_Parameter == 1 || Led_Parameter == 0)
			Led_Parameter = ~(Led_Parameter) & 0x1;
	}
	led_enhance.field.idx = Led_Parameter;

	os_alloc_mem(pAd, (UCHAR **)&pBuf, VarLen);
	if (pBuf == NULL)
	{
		return NDIS_STATUS_RESOURCES;
	}

	NdisZeroMemory(pBuf, VarLen);
	
	Pos = pBuf;
	/* Parameter */
	
	NdisMoveMemory(Pos, &arg0, 4);
	NdisMoveMemory(Pos+4, &led_enhance, sizeof(led_enhance));
	

	Pos += 4;

	hex_dump("AndesLedOPEnhance: ", pBuf, VarLen);
	AndesAppendCmdMsg(msg, (char *)pBuf, VarLen);
	

	ret = AndesSendCmdMsg(pAd, msg);

	os_free_mem(NULL, pBuf);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;	
}
#endif

#ifdef RTMP_FLASH_SUPPORT
static VOID ReloadDPDByChannel(RTMP_ADAPTER *pAd, UINT8 channel, P_EXT_CMD_ID_LOAD_DPD_T pCmdLoadDPD)
{
	/* reload low channel  WF0: 0x314~0x357  WF1: 0x358~0x39B */
	if(channel >= 1 && channel <= 5)
	{		
		INT q = 0;
		
		for(q = 0; q < 17; q++)
		{
			UINT32 DPDValueWF0 = 0, DPDValueWF1 = 0;
			UINT16 tmp1 = 0, tmp2 = 0;
			rtmp_ee_flash_read(pAd, (0x314 + q*4), &tmp1);
			rtmp_ee_flash_read(pAd, (0x314 + q*4 + 2), &tmp2);
			DPDValueWF0 = (tmp2 << 16) | tmp1;
			pCmdLoadDPD->au4WF0CR[q] = DPDValueWF0;

			rtmp_ee_flash_read(pAd, (0x358 + q*4), &tmp1);
			rtmp_ee_flash_read(pAd, (0x358 + q*4 + 2), &tmp2);
			DPDValueWF1 = (tmp2 << 16) | tmp1;
			pCmdLoadDPD->au4WF1CR[q] = DPDValueWF1;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s: flash 0x%x = WF0[%d] = %x , flash 0x%x = WF1[%d] = %x\n"
			, __FUNCTION__, (0x314 + q*4),q, DPDValueWF0, (0x358 + q*4),q, DPDValueWF1));
		}
	}
	/* reload mid channel  WF0: 0x28C~0x2CF  WF1: 0x2D0~0x313 */
	else if(channel >= 6 && channel <= 10 )
	{
		INT q = 0;
		
		for(q = 0; q < 17; q++)
		{
			UINT32 DPDValueWF0 = 0, DPDValueWF1 = 0;
			UINT16 tmp1 = 0, tmp2 = 0;
			rtmp_ee_flash_read(pAd, (0x28C + q*4), &tmp1);
			rtmp_ee_flash_read(pAd, (0x28C + q*4 + 2), &tmp2);
			DPDValueWF0 = (tmp2 << 16) | tmp1;
			pCmdLoadDPD->au4WF0CR[q] = DPDValueWF0;

			rtmp_ee_flash_read(pAd, (0x2D0 + q*4), &tmp1);
			rtmp_ee_flash_read(pAd, (0x2D0 + q*4 + 2), &tmp2);
			DPDValueWF1 = (tmp2 << 16) | tmp1;
			pCmdLoadDPD->au4WF1CR[q] = DPDValueWF1;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s: flash 0x%x = WF0[%d] = %x , flash 0x%x = WF1[%d] = %x\n"
			, __FUNCTION__, (0x28C + q*4),q, DPDValueWF0, (0x2D0 + q*4),q, DPDValueWF1));
		}
	}
	/* reload mid channel  WF0: 0x204~0x247  WF1: 0x248~0x28B */
	else if(channel >= 11 && channel <= 14 )
	{
		INT q = 0;
		
		for(q = 0; q < 17; q++)
		{
			UINT32 DPDValueWF0 = 0, DPDValueWF1 = 0;
			UINT16 tmp1 = 0, tmp2 = 0;
			rtmp_ee_flash_read(pAd, (0x204 + q*4), &tmp1);
			rtmp_ee_flash_read(pAd, (0x204 + q*4 + 2), &tmp2);
			DPDValueWF0 = (tmp2 << 16) | tmp1;
			pCmdLoadDPD->au4WF0CR[q] = DPDValueWF0;

			rtmp_ee_flash_read(pAd, (0x248 + q*4), &tmp1);
			rtmp_ee_flash_read(pAd, (0x248 + q*4 + 2), &tmp2);
			DPDValueWF1 = (tmp2 << 16) | tmp1;
			pCmdLoadDPD->au4WF1CR[q] = DPDValueWF1;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s: flash 0x%x = WF0[%d] = %x , flash 0x%x = WF1[%d] = %x\n"
			, __FUNCTION__, (0x204 + q*4),q, DPDValueWF0, (0x248 + q*4),q, DPDValueWF1));
		}
	}
	return;
}
INT32 CmdLoadDPDDataFromFlash(RTMP_ADAPTER *pAd, UINT8 channel, UINT16 doReload)
{
	struct cmd_msg *msg;
	EXT_CMD_ID_LOAD_DPD_T CmdLoadDPD;
	INT32 ret = 0;

	if(channel < 1 || channel > 14)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Channel = %d (1~14), INSANE , return\n", __FUNCTION__, channel));
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
	("%s: Channel = %d, DoReload = %d\n", __FUNCTION__, channel, doReload));

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdLoadDPD));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_LOAD_DPD_FROM_FLASH, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResultMsgRsp);

	memset(&CmdLoadDPD, 0x00, sizeof(CmdLoadDPD));

	if(doReload != 0)
	{
		CmdLoadDPD.ucReload = 1;
		ReloadDPDByChannel(pAd, channel, &CmdLoadDPD);
	}
	else
	{
		CmdLoadDPD.ucReload = 0;
	}

	AndesAppendCmdMsg(msg, (char *)&CmdLoadDPD, sizeof(CmdLoadDPD));

	ret = AndesSendCmdMsg(pAd, msg);


error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}
#endif /*RTMP_FLASH_SUPPORT*/

