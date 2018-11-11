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
		cmm_data_pci.c

	Abstract:

	Note:
		All functions in this file must be PCI-depended, or you should move
		your functions to other files.

	Revision History:
	Who          When          What
	---------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

#ifdef RX_SCATTER
static INT rx_scatter_info(
    IN RTMP_RX_RING *pRxRing,
    IN RXD_STRUC *pRxD,
    OUT UINT *pPktSize);

static INT rx_scatter_gather(
	IN RTMP_ADAPTER *pAd,
    IN RTMP_RX_RING *pRxRing,
    IN RTMP_DMACB *pRxCell,
    IN RXD_STRUC *pRxD,
    IN UINT scatterCnt,
    IN UINT headRoom,
    IN UINT pktSize);
#endif /* RX_SCATTER */

VOID dump_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("TxD:\n"));

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr0=0x%x\n", pTxD->SDPtr0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDLen0=0x%x\n", pTxD->SDLen0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tLastSec0=0x%x\n", pTxD->LastSec0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr1=0x%x\n", pTxD->SDPtr1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDLen1=0x%x\n", pTxD->SDLen1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tLastSec1=0x%x\n", pTxD->LastSec1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tDMADONE=0x%x\n", pTxD->DMADONE));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tBurst=0x%x\n", pTxD->Burst));
}


VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RxD:\n"));

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
				pRxD->SDP0, pRxD->SDL0, pRxD->LS0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr1/SDLen1/LastSec1=0x%x/0x%x/0x%x\n",
				pRxD->SDP1, pRxD->SDL1, pRxD->LS1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}


VOID dumpTxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	int index;

	ASSERT(ring_idx < NUM_OF_TX_RING);
	pDescRing = (RTMP_DMABUF *)pAd->PciHif.TxDescRing[ring_idx].AllocVa;

	pTxRing = &pAd->PciHif.TxRing[ring_idx];
	for (index = 0; index < TX_RING_SIZE; index++)
	{
		pTxD = (TXD_STRUC *)pTxRing->Cell[index].AllocVa;
		hex_dump("Dump TxDesc", (UCHAR *)pTxD, sizeof(TXD_STRUC));
		dump_txd(pAd, pTxD);
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT) {
			TXINFO_STRUC *pTxInfo;

			pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[index].AllocVa + sizeof(TXD_STRUC));
			hex_dump("Dump TxInfo", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
			dump_txinfo(pAd, pTxInfo);
		}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}
}


BOOLEAN MonitorTxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	if (pAd->TxDMACheckTimes < 10)
	{
		/* Check if TX DMA busy */
		MAC_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);

		if ((Value & TX_DMA_BUSY) == TX_DMA_BUSY)
		{
			/* Check TX FIFO if have space */
			MAC_IO_WRITE32(pAd, 0x4244, 0x98000000);
			MAC_IO_READ32(pAd, 0x4244, &Value);
			if ((Value & (1 << 8)) == 0)
			{
				pAd->TxDMACheckTimes = 0;
				return FALSE;
			}
			else
			{
				pAd->TxDMACheckTimes++;
				return FALSE;
			}
		}
		else
		{
			pAd->TxDMACheckTimes = 0;
			return FALSE;
		}
	}
	else
	{
		pAd->TxDMACheckTimes = 0;
		return TRUE;
	}
}


BOOLEAN MonitorRxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	if (pAd->RxDMACheckTimes < 10)
	{
		/* Check if RX DMA busy */
		MAC_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);

		if ((Value & RX_DMA_BUSY) == RX_DMA_BUSY)
		{
			/* Check RX FIFO if have data */
			MAC_IO_WRITE32(pAd, 0x4244, 0x28000000);
			MAC_IO_READ32(pAd, 0x4244, &Value);
			if ((Value & (1 << 8)) == 0)
			{
				pAd->RxDMACheckTimes = 0;
				return FALSE;
			}
			else
			{
				pAd->RxDMACheckTimes++;
				return FALSE;
			}
		}
		else
		{
			pAd->RxDMACheckTimes = 0;
			return FALSE;
		}
	}
	else
	{
		pAd->RxDMACheckTimes = 0;
		return TRUE;
	}
}


VOID dumpRxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	RTMP_RX_RING *pRxRing;
	RXD_STRUC *pRxD;
	int index;
	UINT16 RxRingSize = (ring_idx == 0) ? RX_RING_SIZE : RX1_RING_SIZE;

	pDescRing = (RTMP_DMABUF *)pAd->PciHif.RxDescRing[0].AllocVa;

	pRxRing = &pAd->PciHif.RxRing[0];

	for (index = 0; index < RxRingSize; index++)
	{
		pRxD = (RXD_STRUC *)pRxRing->Cell[index].AllocVa;
		hex_dump("Dump RxDesc", (UCHAR *)pRxD, sizeof(RXD_STRUC));
		dump_rxd(pAd, pRxD);
	}
}


#if defined(RTMP_MAC) || defined(RLT_MAC)
static VOID ral_write_txinfo(
	IN RTMP_ADAPTER *pAd,
	IN TXINFO_STRUC *pTxInfo,
	IN BOOLEAN bWiv,
	IN UCHAR QueueSel)
{
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXINFO_NMAC_PKT *nmac_info = (struct _TXINFO_NMAC_PKT *)pTxInfo;

#ifdef HDR_TRANS_SUPPORT
		if (pTxBlk && TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
			nmac_info->pkt_80211 = 0;	/* 802.3 MAC header */
		else
#endif /* HDR_TRANS_SUPPORT */
			nmac_info->pkt_80211 = 1;
		nmac_info->info_type = 0;
		nmac_info->d_port = 0;
		nmac_info->cso = 0;
		nmac_info->tso = 0;
		nmac_info->wiv = (bWiv) ? 1: 0;
		nmac_info->QSEL = (pAd->bGenOneHCCA == TRUE) ? FIFO_HCCA : QueueSel;

	}
#endif /* RLT_MAC */


#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXINFO_OMAC *omac_info = (struct _TXINFO_OMAC *)pTxInfo;

		omac_info->WIV = (bWiv) ? 1: 0;
		omac_info->QSEL = (pAd->bGenOneHCCA == TRUE) ? FIFO_HCCA : QueueSel;

	}
#endif /* RTMP_MAC */
}


#ifdef RLT_MAC
static VOID rlt_update_txinfo(
	IN RTMP_ADAPTER *pAd,
	IN TXINFO_STRUC *pTxInfo,
	IN TX_BLK *pTxBlk)
{
}
#endif /* RLT_MAC */


VOID ral_write_txd(
	IN RTMP_ADAPTER *pAd,
	IN TXD_STRUC *pTxD,
	IN TX_BLK *txblk,
	IN BOOLEAN bWIV,
	IN UCHAR QueueSEL)
{
	TXINFO_STRUC *pTxInfo = (TXINFO_STRUC *)(pTxD + 1);


	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	ral_write_txinfo(pAd, pTxInfo, bWIV, QueueSEL);

	pTxD->DMADONE = 0;
}

#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


#ifdef MT_MAC
VOID mt_write_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
//+++Add by shiang for jeffrey debug
#ifdef LINUX
	wmb();
#endif /* LINUX */
//---Add by shiang for jeffrey debug
	pTxD->DMADONE = 0;
}
#endif /* MT_MAC */


VOID ComposePsPoll(
	IN	RTMP_ADAPTER *pAd,
	IN	PPSPOLL_FRAME pPsPollFrame,
	IN	USHORT	Aid,
	IN	UCHAR *pBssid,
	IN	UCHAR *pTa)
{
	NdisZeroMemory(pPsPollFrame, sizeof (PSPOLL_FRAME));
	pPsPollFrame->FC.Type = FC_TYPE_CNTL;
	pPsPollFrame->FC.SubType = SUBTYPE_PS_POLL;
	pPsPollFrame->Aid = Aid | 0xC000;
	COPY_MAC_ADDR(pPsPollFrame->Bssid, pBssid);
	COPY_MAC_ADDR(pPsPollFrame->Ta, pTa);
}


USHORT write_first_buf(RTMP_ADAPTER *pAd, TX_BLK *txblk, UCHAR *dma_buf)
{
	UINT tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len; // TXWISize + TSO_SIZE
	USHORT first_buf_len;

	first_buf_len =  tx_hw_hdr_len + txblk->MpduHeaderLen + txblk->HdrPadLen - txblk->hw_rsv_len;

if (TX_BLK_TEST_FLAG(txblk, fTX_DumpPkt) == TRUE) {
	printk("%s(): first_buf_len=%d, HwHdrLen=%d, pTxBlk->MpduHdrLen=%d, HdrPadLen=%d, HwRsvLen=%d\n",
		__FUNCTION__, first_buf_len, tx_hw_hdr_len, txblk->MpduHeaderLen,
		txblk->HdrPadLen, txblk->hw_rsv_len);
}


	NdisMoveMemory(dma_buf,
					(UCHAR *)(txblk->HeaderBuf + txblk->hw_rsv_len),
					first_buf_len);

	return first_buf_len;
}


USHORT RtmpPCI_WriteSingleTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN BOOLEAN bIsLast,
	OUT USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	UINT32 BufBasePaLow;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	RTMP_TX_RING *pTxRing;

#ifdef MT_MAC
#ifdef USE_BMC
	if (pTxBlk->QueIdx == QID_BMC)
	{
		pTxRing = &pAd->TxBmcRing;
		TxIdx = pAd->TxBmcRing.TxCpuIdx;
	}
	else
#endif /* USE_BMC */
#endif
	{
		/* get Tx Ring Resource*/
		pTxRing = &pAd->PciHif.TxRing[pTxBlk->QueIdx];
		TxIdx = pAd->PciHif.TxRing[pTxBlk->QueIdx].TxCpuIdx;
	}

	pDMAHeaderBufVA = (UCHAR *)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

#ifdef CUT_THROUGH
    if (!CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
#endif

#ifdef CUT_THROUGH
	if (CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
		pTxRing->Cell[TxIdx].token_id = pTxBlk->TxTokenID[0];
#endif /* CUT_THROUGH */
	
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	pTxRing->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
	pTxD->SDPtr1 = pTxRing->Cell[TxIdx].PacketPa;
#ifdef CUT_THROUGH
    if (CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
    		pTxD->SDLen1 = pTxBlk->SrcBufLen < pAd->chipCap.CtParseLen ? pTxBlk->SrcBufLen : pAd->chipCap.CtParseLen;
    else
#endif /* CUT_THROUGH */
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, pTxBlk, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMPWIEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TXWI);
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + pAd->chipCap.tx_hw_hdr_len), DIR_WRITE, FALSE);
	}
#endif
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


USHORT RtmpPCI_WriteMultiTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR frameNum,
	OUT	USHORT *FreeNumber)
{
	BOOLEAN bIsLast;
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
	UINT32 firstDMALen = 0;
	ASSERT((pTxBlk->TxFrameType == TX_AMSDU_FRAME || pTxBlk->TxFrameType == TX_RALINK_FRAME));
	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);

	/* get Tx Ring Resource */
	pTxRing = &pAd->PciHif.TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->PciHif.TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);


	if (frameNum == 0)
	{

		USHORT hwHdrLen;
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer */
		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			hwHdrLen = pTxBlk->MpduHeaderLen - AMSDU_SUBHEAD_LEN + pTxBlk->HdrPadLen + AMSDU_SUBHEAD_LEN;
		else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
			hwHdrLen = pTxBlk->MpduHeaderLen - ARALINK_HEADER_LEN + pTxBlk->HdrPadLen + ARALINK_HEADER_LEN;

		hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		firstDMALen = pAd->chipCap.TXWISize + hwHdrLen;
	}
	else
	{
		firstDMALen = pTxBlk->MpduHeaderLen;
	}

	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + TXINFO_SIZE, firstDMALen);

#ifdef CUT_THROUGH
    if (!CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
#endif

#ifdef CUT_THROUGH
	if (CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
		pTxRing->Cell[TxIdx].token_id = pTxBlk->TxTokenID[0];
#endif /* CUT_THROUGH */
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	pTxRing->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	/* build Tx Descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
	pTxD->SDPtr1 = pTxRing->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, pTxBlk, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16  TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		if (frameNum == 0)
			RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
		if (frameNum != 0)
			MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
	}
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	if (frameNum != 0)
		RTMPWIEndianChange(pAd, (PUCHAR)pDMAHeaderBufVA, TYPE_TXWI);
	}
#endif
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


VOID RtmpPCI_FinalWriteTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN USHORT totalMPDUSize,
	IN USHORT FirstTxIdx)
{
	RTMP_TX_RING *pTxRing;
	UCHAR *tmac_info;

	/* get Tx Ring Resource*/
	pTxRing = &pAd->PciHif.TxRing[pTxBlk->QueIdx];

	if (FirstTxIdx >= TX_RING_SIZE)
		return;
	tmac_info = (UCHAR *)pTxRing->Cell[FirstTxIdx].DmaBuf.AllocVa;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;

		pTxWI->TXWI_N.MPDUtotalByteCnt = totalMPDUSize;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;

		pTxWI->TXWI_O.MPDUtotalByteCnt = totalMPDUSize;
	}
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;

		txd_s->TxD0.TxByteCount = totalMPDUSize;
	}
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16  TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, TmacLen);
	}
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
	}
#endif
#endif /* RT_BIG_ENDIAN */
}


USHORT	RtmpPCI_WriteFragTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR fragNum,
	OUT	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
#if defined(RTMP_MAC) || defined(RLT_MAC)
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#endif

	/* Get Tx Ring Resource*/
	pTxRing = &pAd->PciHif.TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->PciHif.TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	/* Build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	if (fragNum == pTxBlk->TotalFragNum)
	{

#ifdef CUT_THROUGH
		if (!CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
			pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
#endif

#ifdef CUT_THROUGH
		if (CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
			pTxRing->Cell[TxIdx].token_id = pTxBlk->TxTokenID[0];
#endif /* CUT_THROUGH */

		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	}

	pTxD->SDPtr0 = BufBasePaLow;
	/* Copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer, including padding */
	pTxD->SDLen0 = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (pTxD->SDLen1 ? 1 : 0);
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, pTxBlk, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16  TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
	}

#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
	RTMPWIEndianChange(pAd, pDMAHeaderBufVA, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	}
#endif
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index */
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


#if defined(MT7615) || defined(MT7622)
#ifdef FAST_PATH_TXQ
int RtmpPCIMgmtKickOut(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pSrcBufVA,
	IN UINT SrcBufLen)
{
	struct FastPathTxQueElement *FPTxElement;

	RTMP_SEM_LOCK(&pAd->FastPathTxFreeQueLock);
	FPTxElement = DlListFirst(&pAd->FastPathTxFreeQue, struct FastPathTxQueElement, List);

	if (!FPTxElement)
	{
		RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,("%s: Not available element in FastPathTxFreeQue\n", __FUNCTION__));
		return 0;
	}

	DlListDel(&FPTxElement->List);
	pAd->FPTxElementFreeNum--;

	if (pAd->FPTxElementFreeNum < pAd->MinFPTxElementFreeNum)
		pAd->MinFPTxElementFreeNum = pAd->FPTxElementFreeNum;

	RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);

	RTMP_SET_PACKET_MGMT_PKT(pPacket, 1);

	FPTxElement->QueIdx = QueIdx;
	FPTxElement->pPacket = pPacket;

	RTMP_SEM_LOCK(&pAd->MgmtQueLock);
	pAd->MgmtQueNum++;
	DlListAddTail(&pAd->MgmtQue, &FPTxElement->List);
	RTMP_SEM_UNLOCK(&pAd->MgmtQueLock);

	if (pAd->bFastPathTaskSchedulable)
	{
		RTMP_NET_TASK_DATA_ASSIGN(&pAd->FastPathDequeTask, (ULONG)(pAd));
		RTMP_OS_TASKLET_SCHE(&pAd->FastPathDequeTask);
	}

	return 0;
}
#else
int RtmpPCIMgmtKickOut(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pSrcBufVA,
	IN UINT SrcBufLen)
{
	TX_BLK tx_blk, *pTxBlk;

	NdisZeroMemory((UCHAR *)&tx_blk, sizeof(TX_BLK));
	pTxBlk = &tx_blk;

	RTMP_SET_PACKET_MGMT_PKT(pPacket, 1);
	if (RTMP_GET_PACKET_TXTYPE(pPacket) == TX_MCAST_FRAME)
		pTxBlk->TxFrameType = TX_MCAST_FRAME;
	else
		pTxBlk->TxFrameType = TX_LEGACY_FRAME;
	pTxBlk->TotalFrameNum = 1;
	/* The real fragment number maybe vary*/
	pTxBlk->TotalFragNum = 1;
	pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(pPacket);
	pTxBlk->QueIdx = QueIdx;
	pTxBlk->pPacket = pPacket;
	pTxBlk->wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
#ifdef VENDOR_FEATURE1_SUPPORT
	pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /* VENDOR_FEATURE1_SUPPORT */

	InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));

#ifdef CUT_THROUGH
	CutThroughPktTx(pAd, pTxBlk);
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
#endif /* CUT_THROUGH */

	return 0;
}
#endif
#else
/*
	Must be run in Interrupt context
	This function handle PCI specific TxDesc and cpu index update and kick the packet out.
 */
int RtmpPCIMgmtKickOut(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pSrcBufVA,
	IN UINT SrcBufLen)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	UCHAR tx_hw_info[TXD_SIZE];
	TXD_STRUC *pDestTxD;
#endif
	UCHAR *frm_buf;
	UINT32 SwIdx;
	INT pkt_len;
	RTMP_DMACB *dma_buf;

#ifdef MT_MAC
#endif /* MT_MAC */
	{
		frm_buf = pSrcBufVA + TXINFO_SIZE;
		pkt_len = SrcBufLen - TXINFO_SIZE;
		SwIdx = pAd->MgmtRing.TxCpuIdx;
		dma_buf = &pAd->MgmtRing.Cell[SwIdx];
	}

#ifdef RT_BIG_ENDIAN
	pDestTxD = (TXD_STRUC *)dma_buf->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD = (TXD_STRUC *)dma_buf->AllocVa;
#endif

#ifdef CUT_THROUGH
    if (!CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
	dma_buf->pNdisPacket = pPacket;
#endif

	dma_buf->pNextNdisPacket = NULL;
	dma_buf->PacketPa = PCI_MAP_SINGLE(pAd, frm_buf, pkt_len, 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->SDPtr0 = dma_buf->PacketPa;
	pTxD->SDLen0 = pkt_len;
	pTxD->LastSec0 = 1;
	pTxD->SDPtr1 = 0;
	pTxD->LastSec1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, NULL, TRUE, FIFO_MGMT);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((UCHAR *)pTxD, TYPE_TXD);
	WriteBackToDescriptor((UCHAR *)pDestTxD, (UCHAR *)pTxD, FALSE, TYPE_TXD);
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pSrcBufVA, SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_buf->AllocPa, TXD_SIZE);
	{
		/* Increase TX_CTX_IDX, but write to register later.*/
		INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);
		HIF_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr,  pAd->MgmtRing.TxCpuIdx);
	}

	return 0;
}
#endif /* defined(MT7615) || defined(MT7622) */


#ifdef CONFIG_AP_SUPPORT
static VOID trPsTokenUpdate(RTMP_ADAPTER *pAd,UINT QueIdx,PNDIS_PACKET pPacket)
{
	struct tx_swq_fifo *fifo_swq;
	STA_TR_ENTRY *tr_entry;
	INT enq_idx;
	UCHAR wcid;

	wcid = RTMP_GET_PACKET_WCID(pPacket);

	if(wcid < MAX_LEN_OF_TR_TABLE)
	{

		tr_entry = &pAd->MacTab.tr_entry[wcid];

		if (tr_entry->tx_queue[QueIdx].Number == 0)
		{
			tr_entry->TokenCount[QueIdx] = 0;
		} else
		if ((tr_entry->ps_state == APPS_RETRIEVE_IDLE) && (tr_entry->TokenCount[QueIdx] > 0))
		{
			fifo_swq = &pAd->tx_swq[QueIdx];
			enq_idx = fifo_swq->enqIdx;

			if ((fifo_swq->swq[enq_idx] == 0) && (tr_entry->enq_cap))
			{
				fifo_swq->swq[enq_idx] = tr_entry->wcid;
				INC_RING_INDEX(fifo_swq->enqIdx, TX_SWQ_FIFO_LEN);
				TR_TOKEN_COUNT_DEC(tr_entry, QueIdx);
			}
		}
	}
}

#endif

BOOLEAN RTMPFreeTXDUponTxDmaDone(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN UCHAR RingIdx,
	BOOLEAN ForceFree)
{
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	UCHAR FREE = 0;
	BOOLEAN bReschedule = FALSE;
#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 0)
#ifdef CUT_THROUGH
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	ULONG HwCnt;
#endif
#endif

#ifdef MT_MAC
	if (QueIdx == QID_BMC)
		pTxRing = &pAd->TxBmcRing;
	else
#endif /* MT_MAC */
	{
		ASSERT(RingIdx < NUM_OF_TX_RING);
		if (RingIdx >= NUM_OF_TX_RING)
		{
        	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,("%s: RingIdx=%d\n", __FUNCTION__, RingIdx));
			return FALSE;
		}
		pTxRing = &pAd->PciHif.TxRing[RingIdx];
	}

	pTxRing->TxDMADoneCnt++;

	if (pTxRing->TxDMADoneCnt < pTxRing->TxDMADoneCntResetMark)
	{
		return TRUE;
	}
	else
	{
		pTxRing->TxDMADoneCnt = 0;
	}
#if (CFG_CPU_LOADING_DMADONE == 1)
    RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);
    pTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);

    while (pTxD->DMADONE)
    {
        pTxD->DMADONE = 0;
#else //(CFG_CPU_LOADING_DMADONE == 1)

#ifdef CUT_THROUGH_DBG
	pAd->IoReadTx++;
#endif

	do
	{
#endif //(CFG_CPU_LOADING_DMADONE == 1)
        /*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		{
			return FALSE;
		}
		#endif /* ERR_RECOVERY */
		if (++pAd->FifoUpdateRx >= FIFO_STAT_READ_PERIOD)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateRx = 0;
		}

		/* Note : If (pAd->ate.bQATxStart == TRUE), we will never reach here. */
		FREE++;
#ifndef RT_BIG_ENDIAN
		pTxD = (TXD_STRUC *)(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
#else
		pDestTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
		if (!pTxD->DMADONE) {
			break;
		}

		pTxD->DMADONE = 0; 

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_SP_PacketCheck(pAd,
				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket,
				((UCHAR *)pTxRing->Cell[\
				pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + pAd->chipCap.TXWISize);
#endif /* UAPSD_SUPPORT */
#else
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_SP_PacketCheck(pAd,
				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket,
				((UCHAR *)pTxRing->Cell[pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + pAd->chipCap.TXWISize);
		}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		/* Execution of this block is not allowed when ATE is running. */
		if ( 1
#ifdef CONFIG_ATE
            && !(ATE_ON(pAd))
#endif /* CONFIG_ATE */
#ifdef CUT_THROUGH /* leave packet free to TxD-Free Notification handler. */
            && !CUT_THROUGH_TX_ENABL(pAd->PktTokenCb)
#endif /* CUT_THROUGH */
		)
		{
			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket;
			if (pPacket)
			{
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);

#ifdef CONFIG_AP_SUPPORT
				if (QueIdx < WMM_QUE_NUM)
				{
					trPsTokenUpdate(pAd,QueIdx,pPacket);
				}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC &&
					skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket = NULL;
			}

			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
#if defined(MT7615) || defined(MT7622)
                            if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
                                MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
                                                    ("%s():ERR!!7615 shall not has one TxD with 2 pkt buffer!\n",
                                                    __FUNCTION__));
                            }
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);

#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC &&
					skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE ))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			}
		}

#ifdef CUT_THROUGH
		pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket = NULL;
		pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#else
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, TXD_SIZE);
		pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
#endif /* CUT_THROUGH */

#ifdef MT_MAC
		if (QueIdx == QID_BMC)
			pAd->RalinkCounters.OneSecDmaDoneCount[QID_AC_BE]++;
		else
#endif /* MT_MAC */
			pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx]++;

		INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);


#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		//*pDestTxD = TxD;
		NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif
#if (CFG_CPU_LOADING_DMADONE == 1)
        RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);
        pTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
#endif

	} while (pTxRing->TxSwFreeIdx != pTxRing->TxCpuIdx);

#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 0)
#ifdef CUT_THROUGH
    if(NUM_OF_TX_RING > RingIdx)
    {
	    HwCnt = GET_TXRING_FREENO(pAd, RingIdx);
	    if ((HwCnt > pktTokenCb->TxRingHighWaterMark) &&
			    cut_through_tx_state(pktTokenCb, NO_ENOUGH_FREE_TX_RING, RingIdx))
	    {
		    cut_through_tx_flow_block(pktTokenCb, NULL, NO_ENOUGH_FREE_TX_RING, FALSE, RingIdx);
	    }
    }
#endif
#endif

	return  bReschedule;
}


/*
	========================================================================

	Routine Description:
		Process TX Rings DMA Done interrupt, running in DPC level

	Arguments:
		Adapter 	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
BOOLEAN	RTMPHandleTxRingDmaDoneInterrupt(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_TX_DONE_MASK tx_mask)
{
	unsigned long	IrqFlags;
	BOOLEAN bReschedule = FALSE;


	/* Make sure Tx ring resource won't be used by other threads*/
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT && !IS_MT7615(pAd) && !IS_MT7622(pAd)) {
		if (tx_mask & TX_AC0_DONE)
			bReschedule = RTMPFreeTXDUponTxDmaDone(pAd,QID_AC_BK, QID_AC_BK, FALSE);

		if (tx_mask & TX_AC1_DONE)
			bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd,QID_AC_BE, QID_AC_BE, FALSE);
	}else
#endif /*MT_MAC*/
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT || IS_MT7615(pAd) \
            || IS_MT7622(pAd))
	{
		if (tx_mask & TX_AC0_DONE)
			bReschedule = RTMPFreeTXDUponTxDmaDone(pAd,QID_AC_BE, QID_AC_BE, FALSE);

		if (tx_mask & TX_AC1_DONE)
			bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd,QID_AC_BK, QID_AC_BK, FALSE);
	}

	if ((tx_mask & TX_HCCA_DONE) && (NUM_OF_TX_RING > QID_HCCA))
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd,QID_HCCA, QID_HCCA, FALSE);

	if ((tx_mask & TX_AC3_DONE) && (NUM_OF_TX_RING > QID_AC_VO))
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd,QID_AC_VO, QID_AC_VO, FALSE);

	if ((tx_mask & TX_AC2_DONE) && (NUM_OF_TX_RING > QID_AC_VI))
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd,QID_AC_VI, QID_AC_VI, FALSE);
#ifdef MT_MAC
	if (tx_mask & TX_BMC_DONE)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd,QID_BMC, QID_BMC, FALSE);
#endif /* MT_MAC */

	/* Make sure to release Tx ring resource*/
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);

#ifdef FAST_PATH_TXQ
	if (pAd->bFastPathTaskSchedulable && ((pAd->FastPathTxQueNum > 0) || (pAd->MgmtQueNum> 0)))
	{
		RTMP_NET_TASK_DATA_ASSIGN(&pAd->FastPathDequeTask, (ULONG)(pAd));
		RTMP_OS_TASKLET_SCHE(&pAd->FastPathDequeTask);
	}
#endif

	return  bReschedule;
}


/*
	========================================================================

	Routine Description:
		Process MGMT ring DMA done interrupt, running in DPC level

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPHandleMgmtRingDmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	UCHAR	FREE = 0;
	RTMP_MGMT_RING *pMgmtRing = &pAd->MgmtRing;

	NdisAcquireSpinLock(&pAd->MgmtRingLock);

	HIF_IO_READ32(pAd, pMgmtRing->hw_didx_addr, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
        //TxD = *pDestTxD;
        //pTxD = &TxD;
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
#endif

		pTxD->DMADONE = 0;
		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

#define LMR_FRAME_GET()	(GET_OS_PKT_DATAPTR(pPacket) + pAd->chipCap.TXWISize)

#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket,
					LMR_FRAME_GET(), FALSE);
#else
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket, LMR_FRAME_GET(), FALSE);
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		if (pPacket &&
			((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT)))
		{
			HEADER_802_11  *pHeader;
			pHeader = (HEADER_802_11 *)(GET_OS_PKT_DATAPTR(pPacket)+ TXINFO_SIZE + pAd->chipCap.TXWISize);
			CFG80211_SendMgmtFrameDone(pAd, pHeader->Sequence, TRUE);
		}
#endif

#ifdef CONFIG_ATE
	if(ATE_ON(pAd))
		MT_ATETxControl(pAd, 0xFF, pPacket);
#endif
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
#if defined(MT7615) || defined(MT7622)
                    if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
                        MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
                                            ("%s():ERR!!7615 shall not has one TxD with 2 pkt buffer!\n",
                                            __FUNCTION__));
                    }
#endif /* defined(MT7615) || defined(MT7622) */
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}
	NdisReleaseSpinLock(&pAd->MgmtRingLock);

}


/*
	========================================================================

	Routine Description:
	Arguments:
		Adapter 	Pointer to our adapter. Dequeue all power safe delayed braodcast frames after beacon.

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID RTMPHandleTBTTInterrupt(RTMP_ADAPTER *pAd)
{
//+++Add by Carter
#ifdef MT_MAC
	UINT32 en_cr, stat_cr;

	stat_cr = 0x0;
	en_cr = 0x0;

	printk("%s\n", __func__);

	RTMP_IO_READ32(pAd, HWISR3, &stat_cr);
	RTMP_IO_READ32(pAd, HWIER3, &en_cr);

	/* disable the interrupt source */
	RTMP_IO_WRITE32(pAd, HWIER3, (~stat_cr & en_cr));

	/* write 1 to clear */
	RTMP_IO_WRITE32(pAd, HWISR3, stat_cr);
	RTMP_IO_WRITE32(pAd, HWIER3, en_cr);
#endif /* MT_MAC */
//---Add by Carter

#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type != HIF_MT)
			ReSyncBeaconTime(pAd);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			/* disable mask */

		}
#endif /*MT_MAC */

		RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);

		if ((HcIsRfSupport(pAd,RFIC_5GHZ))
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			ChannelSwitchingCountDownProc(pAd);
		}
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
	}
}


VOID RTMPHandleTTTTInterrupt(RTMP_ADAPTER *pAd)
{
//+++Add by Carter
#ifdef MT_MAC
	volatile UINT32 en_cr, stat_cr;

	printk("%s\n", __func__);

	stat_cr = 0x00000000L;
	en_cr = 0x00000000L;
#endif
}

/*
	========================================================================

	Routine Description:
	Arguments:
		pAd 		Pointer to our adapter. Rewrite beacon content before next send-out.

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID RTMPHandlePreTBTTInterrupt(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
        UpdateBeaconHandler(
            pAd,
            NULL,
            PRETBTT_UPDATE);
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
	}

#ifdef MT_MAC
	/* use pretbtt to check DTIM and dequeue bmc packets */
//#ifdef USE_BMC
	if (pAd->chipCap.hif_type == HIF_MT) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);
	}
//#endif
#endif /*MT_MAC */

}

#if defined (MT_MAC) && defined (CONFIG_AP_SUPPORT)
VOID RTMPHandlePreTTTTInterrupt(RTMP_ADAPTER *pAd)
{
	printk("%s\n", __func__);
	/* only AP MODE support TTTT now, p2p go don't take care of this. */
	if (pAd->OpMode == OPMODE_AP)
	{
		APMakeAllTimFrame(pAd);
	}
}
#endif

VOID RTMPHandleRxCoherentInterrupt(RTMP_ADAPTER *pAd)
{
	if (pAd == NULL)
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("====> pAd is NULL, return.\n"));
		return;
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("==> RTMPHandleRxCoherentInterrupt \n"));

#if defined(MT7603) || defined(MT7628)|| defined(MT7637) ||  defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7603, fix me for upper crash case!!
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7637(pAd) || IS_MT7615(pAd) || IS_MT7622(pAd)) {
		return;
	}
#endif /* MT7603*/

	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);

	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_HCCA);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	RTMPEnableRxTx(pAd);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<== RTMPHandleRxCoherentInterrupt \n"));
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPHandleMcuInterrupt(RTMP_ADAPTER *pAd)
{
	UINT32 McuIntSrc = 0;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return;
	}

	RTMP_IO_READ32(pAd, 0x7024, &McuIntSrc);

	/* check mac 0x7024 */
#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCIE &&
		(McuIntSrc & (1<<1)) && /*bit_1: carr_status interrupt */
		(pAd->CommonCfg.CarrierDetect.Enable == TRUE))
	{
		RTMPHandleRadarInterrupt(pAd);
	}
#endif /* CARRIER_DETECTION_SUPPORT */

	/* clear MCU Int source register.*/
	RTMP_IO_WRITE32(pAd, 0x7024, 0);

}
#endif /* CONFIG_AP_SUPPORT */


#ifdef RLT_MAC


UINT32 RLTFillRxBlkAndPacketProcess(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket)
{
	struct _RXWI_NMAC *rxwi_n;
	UCHAR *buf_ptr;
#ifdef RT_BIG_ENDIAN
	RXINFO_STRUC *pRxInfo;
	RXINFO_STRUC RxInfo;
#endif /* RT_BIG_ENDIAN */

	pRxBlk->Flags = 0;
	pRxBlk->pRxFceInfo = (RXFCE_INFO *)&pRxBlk->hw_rx_info[RXINFO_OFFSET];
	pRxBlk->pRxInfo = (RXINFO_STRUC *)GET_OS_PKT_DATAPTR(pRxPacket);
#ifdef RT_BIG_ENDIAN
	NdisMoveMemory(&RxInfo, pRxBlk->pRxInfo, RAL_RXINFO_SIZE);
	pRxInfo = &RxInfo;
	(*(UINT32*)pRxInfo) = le2cpu32(*(UINT32*)pRxInfo);
	NdisMoveMemory(pRxBlk->pRxInfo, pRxInfo, RAL_RXINFO_SIZE);
#endif /* RT_BIG_ENDIAN */

	rxwi_n = (struct _RXWI_NMAC *)(GET_OS_PKT_DATAPTR(pRxPacket) + RAL_RXINFO_SIZE);
	pRxBlk->pRxWI = (RXWI_STRUC *)(rxwi_n);
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd , (UCHAR *)pRxBlk->pRxWI, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */

	pRxBlk->MPDUtotalByteCnt = rxwi_n->MPDUtotalByteCnt;
	pRxBlk->wcid = rxwi_n->wcid;
	pRxBlk->key_idx = rxwi_n->key_idx;
	pRxBlk->bss_idx = rxwi_n->bss_idx;
	pRxBlk->TID = rxwi_n->tid;
	pRxBlk->DataSize = rxwi_n->MPDUtotalByteCnt;

	pRxBlk->rx_rate.field.MODE = rxwi_n->phy_mode;
	pRxBlk->rx_rate.field.MCS = rxwi_n->mcs;
	pRxBlk->rx_rate.field.ldpc = rxwi_n->ldpc;
	pRxBlk->rx_rate.field.BW = rxwi_n->bw;
	pRxBlk->rx_rate.field.STBC = rxwi_n->stbc;
	pRxBlk->rx_rate.field.ShortGI = rxwi_n->sgi;
	pRxBlk->rx_signal.raw_rssi[0] = rxwi_n->rssi[0];
	pRxBlk->rx_signal.raw_rssi[1] = rxwi_n->rssi[1];
	pRxBlk->rx_signal.raw_rssi[2] = rxwi_n->rssi[2];
	{
		pRxBlk->rx_signal.raw_snr[0] = rxwi_n->bbp_rxinfo[0];
		pRxBlk->rx_signal.raw_snr[1] = rxwi_n->bbp_rxinfo[1];
		pRxBlk->rx_signal.raw_snr[2] = rxwi_n->bbp_rxinfo[2];
	}
	pRxBlk->rx_signal.freq_offset = rxwi_n->bbp_rxinfo[4];
	pRxBlk->ldpc_ex_sym = rxwi_n->ldpc_ex_sym;

	SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + RAL_RXINFO_SIZE + pAd->chipCap.RXWISize);
	SET_OS_PKT_LEN(pRxPacket, GET_OS_PKT_LEN(pRxPacket) - RAL_RXINFO_SIZE - pAd->chipCap.RXWISize);

	return TRUE;
}
#endif /* RLT_MAC */


#ifdef RTMP_MAC
INT32 RTMPFillRxBlkAndPacketProcess(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket)
{
	struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)(GET_OS_PKT_DATAPTR(pRxPacket));

	pRxBlk->Flags = 0;
	pRxBlk->pRxInfo = (RXINFO_STRUC *)(&pRxBlk->hw_rx_info[RXINFO_OFFSET]);
	pRxBlk->pRxWI = (RXWI_STRUC *)rxwi_o;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd , (UCHAR *)pRxBlk->pRxWI, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */

	pRxBlk->MPDUtotalByteCnt = rxwi_o->MPDUtotalByteCnt;
	pRxBlk->wcid = rxwi_o->wcid;
	pRxBlk->key_idx = rxwi_o->key_idx;
	pRxBlk->bss_idx = rxwi_o->bss_idx;
	pRxBlk->TID = rxwi_o->tid;
	pRxBlk->DataSize = rxwi_o->MPDUtotalByteCnt;

	pRxBlk->rx_rate.field.MODE = rxwi_o->phy_mode;
	pRxBlk->rx_rate.field.MCS = rxwi_o->mcs;
	pRxBlk->rx_rate.field.BW = rxwi_o->bw;
	pRxBlk->rx_rate.field.STBC = rxwi_o->stbc;
	pRxBlk->rx_rate.field.ShortGI = rxwi_o->sgi;
	pRxBlk->rx_signal.raw_rssi[0] = rxwi_o->RSSI0;
	pRxBlk->rx_signal.raw_rssi[1] = rxwi_o->RSSI1;
	pRxBlk->rx_signal.raw_rssi[2] = rxwi_o->RSSI2;
	pRxBlk->rx_signal.raw_snr[0] = rxwi_o->SNR0;
	pRxBlk->rx_signal.raw_snr[1] = rxwi_o->SNR1;
	pRxBlk->rx_signal.raw_snr[2] = rxwi_o->SNR2;
	pRxBlk->rx_signal.freq_offset = rxwi_o->FOFFSET;

	SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + pAd->chipCap.RXWISize);
	SET_OS_PKT_LEN(pRxPacket, GET_OS_PKT_LEN(pRxPacket) - pAd->chipCap.RXWISize);

	return TRUE;
}
#endif /* RTMP_MAC */

#ifdef RX_SCATTER
static INT rx_scatter_info(
    IN RTMP_RX_RING *pRxRing,
    IN RXD_STRUC *pRxD,
    OUT UINT *pPktSize)
{
#ifdef RT_BIG_ENDIAN
    RXD_STRUC *pDestRxD;
    UCHAR rx_hw_info[RXD_SIZE];
#endif

    UINT LoopCnt;
    INT32 RxCellIdx;

    RTMP_DMACB *pCurRxCell = NULL;
    RXD_STRUC *pCurRxD;

    if (pRxRing == NULL || pRxD == NULL)
        return FALSE;

    LoopCnt = 0;

    *pPktSize = 0;
    RxCellIdx =  pRxRing->RxSwReadIdx;

    /* walk through rx-ring and find the rx-cell content LS0 to be 1. */
    do
    {
        pCurRxCell = &pRxRing->Cell[RxCellIdx];

        /* flush dcache if no consistent memory is supported */
        RTMP_DCACHE_FLUSH(pCurRxCell->AllocPa, RXD_SIZE);

#ifdef RT_BIG_ENDIAN
        pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
        /* RxD = *pDestRxD; */
        NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
        pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
        RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
        /* Point to Rx indexed rx ring descriptor */
        pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif

        if (pCurRxD->DDONE == 0)
        {
        	LoopCnt = 0;
            break;
        }

        *pPktSize += pCurRxD->SDL0;

        LoopCnt++;

        /* find the last pice of rx scattering. */
        if (pCurRxD->LS0 == 1)
        {
            break;
        }
		INC_RING_INDEX(RxCellIdx, pRxRing->RxRingSize);
		
    } while (TRUE);

    /* if LS0 is not find, return fail to stop GetPacketFromRxRing. */
    return LoopCnt;
}

static INT rx_scatter_gather(
	IN RTMP_ADAPTER *pAd,
    IN RTMP_RX_RING *pRxRing,
    IN RTMP_DMACB *pRxCell,
    IN RXD_STRUC *pRxD,
    IN UINT scatterCnt,
    IN UINT headRoom,
    IN UINT pktSize)
{
#ifdef RT_BIG_ENDIAN
    RXD_STRUC *pDestRxD;
    UCHAR rx_hw_info[RXD_SIZE];
#endif

    UINT LoopCnt;
    INT32 RxCellIdx;

	RTMP_DMACB *pCurRxCell = NULL;
    RXD_STRUC *pCurRxD;
	PNDIS_PACKET pRxCellPacket;

    if (pRxRing == NULL || pRxCell == NULL || pRxD == NULL)
        return FALSE;

    /* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
    pRxCellPacket = pRxCell->pNdisPacket;
    if (pRxCellPacket == NULL)
    {
        return FALSE;
    }

    OS_PKT_TAIL_BUF_EXTEND(pRxCellPacket, pRxD->SDL0);
	pRxCell->pNdisPacket = ExpandPacket(NULL, pRxCellPacket, headRoom, pktSize);

    if (pRxCell->pNdisPacket != NULL)
        pRxCell->DmaBuf.AllocVa = GET_OS_PKT_DATAPTR(pRxCell->pNdisPacket);
    else
        pRxCell->DmaBuf.AllocVa = NULL;

    if (pRxCell->pNdisPacket == NULL)
        return TRUE;

    RxCellIdx =  pRxRing->RxSwReadIdx;
    for (LoopCnt=0; LoopCnt<(scatterCnt - 1); LoopCnt++)
    {
        INC_RING_INDEX(RxCellIdx, pRxRing->RxRingSize);
        pCurRxCell = &pRxRing->Cell[RxCellIdx];

        /* return tokens of all rx scatter-gather cells. */
        PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa - headRoom,
                            pCurRxCell->DmaBuf.AllocSize,
                            RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->DmaBuf.AllocPa - headRoom, pCurRxCell->DmaBuf.AllocSize);

#ifdef RT_BIG_ENDIAN
        pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
        /* RxD = *pDestRxD; */
        NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
        pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
        RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
        /* Point to Rx indexed rx ring descriptor */
        pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
        memcpy(OS_PKT_TAIL_BUF_EXTEND(pRxCell->pNdisPacket, pCurRxD->SDL0),
            (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

        /* update done bit of all rx scatter-gather cells to zero. */
		pCurRxCell->DmaBuf.AllocPa =
		    PCI_MAP_SINGLE_DEV(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
		        pCurRxCell->DmaBuf.AllocVa - headRoom, pCurRxCell->DmaBuf.AllocSize,
		        -1, RTMP_PCI_DMA_FROMDEVICE);
        pCurRxCell->DmaBuf.AllocPa += headRoom;
        pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
        pCurRxD->SDL0 = pRxRing->RxBufferSize;
        pCurRxD->DDONE = 0;

        /* update rx descriptor */
#ifdef RT_BIG_ENDIAN
        RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
        WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
    }

    /* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
    pRxRing->RxSwReadIdx = RxCellIdx;

    return TRUE;
}

#ifdef  CONFIG_WIFI_BUILD_SKB
PNDIS_PACKET ExpandPacketBuffer(
	IN VOID *pReserved,
	IN PNDIS_PACKET pPacket,
	IN UINT32 subpkt_len,
	IN UINT32 ext_head_len,
	IN UINT32 ext_tail_len)
{
	struct sk_buff *pkt;
	UINT32	tmp_len;
	UINT32	padding_len;


	padding_len = SKB_BUF_HEADROOM_RSV;
	
	tmp_len = SKB_DATA_ALIGN(ext_head_len + ext_tail_len) 
		+ padding_len + SKB_BUF_TAILROOM_RSV;
	
	DEV_ALLOC_FRAG(pkt, (tmp_len));

	if (pkt && pPacket)
		os_move_mem(((PVOID)pkt + ext_head_len + padding_len),
			((PVOID)pPacket + padding_len), subpkt_len);
	if (pPacket) {
		DEV_FREE_FRAG_BUF(pPacket);
	}

	if (pkt == NULL) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
			("Extend Rx buffer %d size packet failed! drop pkt.\n",
			(ext_head_len + ext_tail_len)));
			return NULL;
	}
	return OSPKT_TO_RTPKT(pkt);
}

/*
    ========================================================================
    Routine Description:
        assemble scattered buffers into one buffer

    Arguments:
		pAd 		- Adapter pointer
		scatterCnt	- Enable / Disable Piggy-Back
		headRoom	- headRoom size
		pktSize		- total pkt size

    Return Value:
        TRUE:  assemble successfully, or buffer allocate failed.
        FALSE: assemble Failed.

    ========================================================================
*/

static INT rx_scatter_gather_buf_only(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_RX_RING *pRxRing,
	IN RTMP_DMACB *pRxCell,
	IN RXD_STRUC *pRxD,
	IN UINT scatterCnt,
	IN UINT headRoom,
	IN UINT pktSize)
{
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	UINT LoopCnt;
	INT32 RxCellIdx;
	UINT32 buf_idx;


	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
	PNDIS_PACKET pRxCellPacket;

	if (pRxRing == NULL || pRxCell == NULL || pRxD == NULL)
	return FALSE;

	/* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
	pRxCellPacket = pRxCell->pNdisPacket;
	if (pRxCellPacket == NULL)
	{
		return FALSE;
	}


	buf_idx = pRxD->SDL0;



	pRxCell->pNdisPacket = ExpandPacketBuffer(NULL, pRxCellPacket,
					buf_idx, headRoom, pktSize);
	buf_idx += (headRoom + SKB_BUF_HEADROOM_RSV);

	if (pRxCell->pNdisPacket != NULL) {
		pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket)
						+ SKB_BUF_HEADROOM_RSV;
	}
	else
		pRxCell->DmaBuf.AllocVa = NULL;

	if (pRxCell->pNdisPacket == NULL)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
		("%s, Allocate buffer error, RxSwReadIdx[%d]\n", __FUNCTION__,
		pRxRing->RxSwReadIdx));
		return TRUE;
	}

	RxCellIdx =  pRxRing->RxSwReadIdx;
	for (LoopCnt=0; LoopCnt<(scatterCnt - 1); LoopCnt++)
	{
		INC_RING_INDEX(RxCellIdx, pRxRing->RxRingSize);
		pCurRxCell = &pRxRing->Cell[RxCellIdx];

		/* return tokens of all rx scatter-gather cells. */
		PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa - headRoom,
					pCurRxCell->DmaBuf.AllocSize,
					RTMP_PCI_DMA_FROMDEVICE);
			/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH((pCurRxCell->DmaBuf.AllocPa - headRoom),
			pCurRxCell->DmaBuf.AllocSize);

#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
		NdisMoveMemory((pRxCell->pNdisPacket + buf_idx),
		    (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

		buf_idx += pCurRxD->SDL0;

		/* update done bit of all rx scatter-gather cells to zero. */
		pCurRxCell->DmaBuf.AllocPa =
		    PCI_MAP_SINGLE_DEV(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pCurRxCell->DmaBuf.AllocVa - headRoom,
			pCurRxCell->DmaBuf.AllocSize,
			        -1, RTMP_PCI_DMA_FROMDEVICE);
		pCurRxCell->DmaBuf.AllocPa += headRoom;
		pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
		pCurRxD->SDL0 = pRxRing->RxBufferSize;
		pCurRxD->DDONE = 0;

		/* update rx descriptor */
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD,
			(PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
	}

	/* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
	pRxRing->RxSwReadIdx = RxCellIdx;

	return TRUE;
}
#endif /* CONFIG_WIFI_BUILD_SKB */
#endif /* RX_SCATTER */

PNDIS_PACKET GetPacketFromRxRing(	
	IN RTMP_ADAPTER *pAd,
	OUT RX_BLK **_pRxBlk,
	OUT BOOLEAN *pbReschedule,
	INOUT UINT32 *pRxPending,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	RTMP_RX_RING *pRxRing;
	NDIS_SPIN_LOCK *pRxRingLock;
	PNDIS_PACKET pRxPacket = NULL, pNewPacket;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell=NULL;
	RX_BLK *pRxBlk = NULL;
#ifdef CUT_THROUGH
	UINT16 TokenId = PKT_TOKEN_ID_INVALID;
    PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)(pAd->PktTokenCb);
#endif /* CUT_THROUGH */


#ifdef RX_SCATTER
	UINT scatterCnt = 1;
	UINT pktScatterGatherSize = 0;
#endif /* RX_SCATTER */
	UINT pkt_buf_size = 0;

	if (!_pRxBlk) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
       		 ("%s:() Error! _pRxBlk is null\n", __FUNCTION__));
		bReschedule = TRUE;
		return NULL;
	}
    pRxBlk = *_pRxBlk;
	pRxRing = &pAd->PciHif.RxRing[RxRingNo];
	pRxRingLock = &pAd->RxRingLock[RxRingNo];
	RTMP_SEM_LOCK(pRxRingLock);

	pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);

#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	/* RxD = *pDestRxD; */
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif


	if (pRxD->DDONE == 0)
	{
		/* DMAIndx had done but DDONE bit not ready */
#if (CFG_CPU_LOADING_DMADONE == 1)
        HIF_IO_READ32(pAd, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
        if (pRxRing->RxSwReadIdx != pRxRing->RxDmaIdx) {
            bReschedule = TRUE;
        } else {
            bReschedule = FALSE;
            pRxPacket = NULL;
        }
#else //(CFG_CPU_LOADING_DMADONE == 1)
		bReschedule = FALSE;

#endif //(CFG_CPU_LOADING_DMADONE == 1)
		goto done;
	}

	*pRxPending = 1;

#ifdef RX_SCATTER
	if (pAd->chipCap.RxDMAScatter == RX_DMA_SCATTER_ENABLE)
	{
		if ((scatterCnt = rx_scatter_info(pRxRing, pRxD, &pktScatterGatherSize)) < 1)
		{
			bReschedule = TRUE;
			goto done;
		}
		pkt_buf_size = pktScatterGatherSize;
	}
	else
#endif /* RX_SCATTER */
	{
		pkt_buf_size = pRxD->SDL0;
	}
#ifdef WLAN_SKB_RECYCLE
	{
		struct sk_buff *skb = __skb_dequeue_tail(&pAd->rx0_recycle);

		if (unlikely(skb == NULL))
			pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pRxCell->DmaBuf.AllocSize, FALSE, &AllocVa, &AllocPa);
		else
		{
			pNewPacket = OSPKT_TO_RTPKT(skb);
			AllocVa = GET_OS_PKT_DATAPTR(pNewPacket);
			AllocPa = PCI_MAP_SINGLE_DEV(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, AllocVa, pRxCell->DmaBuf.AllocSize,  -1, RTMP_PCI_DMA_FROMDEVICE);
		}
	}
#else
#ifdef  CONFIG_WIFI_BUILD_SKB
	if (RxRingNo == 0)
		pNewPacket = RTMP_AllocateRxPacketBufferOnly(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pRxCell->DmaBuf.AllocSize, FALSE, &AllocVa, &AllocPa);
	else
#endif /* CONFIG_WIFI_BUILD_SKB */
		pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pRxCell->DmaBuf.AllocSize, FALSE, &AllocVa, &AllocPa);
#endif /* WLAN_SKB_RECYCLE */

	if (pNewPacket && !pAd->RxRest)
	{
#ifdef CUT_THROUGH
		if (CUT_THROUGH_RX_ENABL(pktTokenCb) && (RxRingNo == 0))
		{
			if (((pktTokenCb->RxFlowBlockState & NO_ENOUGH_FREE_RX_TOKEN) != NO_ENOUGH_FREE_RX_TOKEN)
				&& (pktTokenCb->rx_id_list.list->FreeTokenCnt < pktTokenCb->RxTokenLowWaterMark))
			{
				pktTokenCb->RxFlowBlockState |= NO_ENOUGH_FREE_RX_TOKEN;
			}
			else if (((pktTokenCb->RxFlowBlockState & NO_ENOUGH_FREE_RX_TOKEN) == NO_ENOUGH_FREE_RX_TOKEN)
					&& (pktTokenCb->rx_id_list.list->FreeTokenCnt > pktTokenCb->RxTokenHighWaterMark))
			{
				pktTokenCb->RxFlowBlockState &= ~NO_ENOUGH_FREE_RX_TOKEN;
			}

			if ((pktTokenCb->RxFlowBlockState & NO_ENOUGH_FREE_RX_TOKEN) == NO_ENOUGH_FREE_RX_TOKEN)
			{
				RELEASE_NDIS_PACKET(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}
#endif /* CUT_THROUGH */

#ifdef CUT_THROUGH
		if (CUT_THROUGH_RX_ENABL(pktTokenCb) && (RxRingNo == 0))
		{
			PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa - sizeof(RX_BLK),
			pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa - sizeof(RX_BLK), pRxCell->DmaBuf.AllocSize);
		}
		else
#endif /* CUT_THROUGH */
		{
			PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
			pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		}

#ifdef RX_SCATTER
		if ((pAd->chipCap.RxDMAScatter == RX_DMA_SCATTER_ENABLE)
			&& (scatterCnt > 1))
		{
			UINT headRoom = 0;
#ifdef CUT_THROUGH
			if (CUT_THROUGH_RX_ENABL(pktTokenCb) && (RxRingNo == 0))
				headRoom = sizeof(RX_BLK);
#endif /* CUT_THROUGH */
#ifdef  CONFIG_WIFI_BUILD_SKB
			if ((RxRingNo == 0)) {
				if (rx_scatter_gather_buf_only(pAd, pRxRing,
					pRxCell, pRxD, scatterCnt, headRoom,
	 				pktScatterGatherSize) == FALSE)
				{
					bReschedule = TRUE;
					goto done;
				}
			} else {
#endif /* CONFIG_WIFI_BUILD_SKB*/
				if (rx_scatter_gather(pAd, pRxRing, pRxCell,
	 				pRxD, scatterCnt, headRoom,
	 				pktScatterGatherSize) == FALSE)
				{
					bReschedule = TRUE;
					goto done;
				}
#ifdef  CONFIG_WIFI_BUILD_SKB
			}
#endif /* CONFIG_WIFI_BUILD_SKB*/
#ifdef CUT_THROUGH
		if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (RxRingNo == 0))
				cut_through_rx_pkt_assign(pAd->PktTokenCb, pRxCell->token_id, pRxCell->pNdisPacket);
#endif /* CUT_THROUGH */
		}
#endif /* RX_SCATTER */

#ifdef  CONFIG_WIFI_BUILD_SKB
		if ((RxRingNo == 0)) {
			void *rx_data;
			rx_data = pRxCell->pNdisPacket;
			if (rx_data) {
				DEV_BUILD_SKB(pRxPacket, rx_data,
					pkt_buf_size);

				if (!pRxPacket) {
					DEV_FREE_FRAG_BUF(rx_data);
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL,
						DBG_LVL_WARN,
						("%s, build_skb return NULL\n",
						__func__));
				}
				else {
					DEV_SKB_PTR_ADJUST(pRxPacket,
							pkt_buf_size, rx_data);
				}
			}
			else 
				pRxPacket = NULL;
		}
		else
#endif /* CONFIG_WIFI_BUILD_SKB */
			pRxPacket = pRxCell->pNdisPacket;
#ifdef CUT_THROUGH
		if (CUT_THROUGH_RX_ENABL(pktTokenCb) && (RxRingNo == 0))
		{
			TokenId = cut_through_rx_enq(pAd->PktTokenCb, pNewPacket, TOKEN_RX);

			if (pRxCell->DmaBuf.AllocVa != NULL)
			{
				*_pRxBlk = (RX_BLK *)((UINT8 *)pRxCell->DmaBuf.AllocVa - sizeof(RX_BLK));
				pRxBlk = *_pRxBlk;
			}
			pRxBlk->token_id = pRxCell->token_id;
		}
#endif /* CUT_THROUGH */

#ifndef RT_BIG_ENDIAN
		/* return rx descriptor */
		NdisMoveMemory(&pRxBlk->hw_rx_info[0], pRxD, RXD_SIZE);
#endif /* RT_BIG_ENDIAN */

		if (pRxPacket)
		{
			AsicFillRxBlkAndPktProcess(pAd, pRxBlk, pRxPacket);
		}

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;

#ifdef CUT_THROUGH
		if (RxRingNo == 0 && CUT_THROUGH_RX_ENABL(pAd->PktTokenCb))
		{
			pRxCell->DmaBuf.AllocSize = pRxRing->RxBufferSize + sizeof(RX_BLK);
			pRxCell->DmaBuf.AllocVa += sizeof(RX_BLK);
			pRxCell->DmaBuf.AllocPa += sizeof(RX_BLK);
			OS_PKT_RESERVE(pRxCell->pNdisPacket, sizeof(RX_BLK));
			pRxCell->token_id = TokenId;
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa - sizeof(RX_BLK), pRxCell->DmaBuf.AllocSize);
		}
		else
#endif /* CUT_THROUGH */
		{
			pRxCell->DmaBuf.AllocSize = pRxRing->RxBufferSize;
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		}

		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = pRxRing->RxBufferSize;

#ifdef CUT_THROUGH
		if (RxRingNo == 0 && CUT_THROUGH_RX_ENABL(pAd->PktTokenCb))
		{
			pRxD->SDP1 = cpu2le32((((UINT16)pRxCell->token_id) & 0x0000ffff));
		}

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) && RxRingNo==0)
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it!!len=%d\n", __FUNCTION__,pRxBlk->DataSize));
			if (pRxPacket) {
				if(pRxBlk->DataSize == 0)
				{
					SET_OS_PKT_LEN(pRxPacket,pRxD->SDL0-64);
				}

				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;				
				bReschedule = TRUE;
			}
		}
#endif
	}
	else
	{
		if (pNewPacket)	{
#ifdef  CONFIG_WIFI_BUILD_SKB
			if (RxRingNo == 0) {
				DEV_FREE_FRAG_BUF(pNewPacket);
			}
			else {
#endif /* CONFIG_WIFI_BUILD_SKB */
				RELEASE_NDIS_PACKET(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
#ifdef  CONFIG_WIFI_BUILD_SKB
			}
#endif /* CONFIG_WIFI_BUILD_SKB */
		}
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

#ifdef  CONFIG_WIFI_BUILD_SKB
		if ((RxRingNo == 0)) {
			void *rx_data;
				
			rx_data = pRxCell->pNdisPacket;
			if (rx_data) {
				DEV_BUILD_SKB(pRxPacket, rx_data,
					pkt_buf_size);

				if (!pRxPacket) {
					DEV_FREE_FRAG_BUF(rx_data);
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL,
						DBG_LVL_WARN,
						("%s, build_skb return NULL\n",
						__func__));
				}
				else {
					DEV_SKB_PTR_ADJUST(pRxPacket,
						pkt_buf_size, rx_data);
				}
			}
			else 
				pRxPacket = NULL;
		}
		else
#endif /* CONFIG_WIFI_BUILD_SKB */
			pRxPacket = pRxCell->pNdisPacket;

		if (pRxPacket)
		{
			pkt_alloc_fail_handle(pAd, pRxBlk, pRxPacket);
		}
			
		pRxCell->DmaBuf.AllocPa = PCI_MAP_SINGLE_DEV(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pRxCell->DmaBuf.AllocVa, pRxCell->DmaBuf.AllocSize,  -1, RTMP_PCI_DMA_FROMDEVICE);

		pAd->RxResetDropCount++;

		if (pAd->RxResetDropCount > 10)
		{
			pAd->RxRest = 0;
			pAd->RxResetDropCount = 0;
		}

		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = pRxRing->RxBufferSize;
		pRxPacket = NULL;
		bReschedule = TRUE;
	}

#ifndef CACHE_LINE_32B
	pRxD->DDONE = 0;

	/* update rx descriptor and kick rx */
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	INC_RING_INDEX(pRxRing->RxSwReadIdx, pRxRing->RxRingSize);
	pRxRing->sw_read_idx_inc++;
	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0)
        ? (pRxRing->RxRingSize - 1) : (pRxRing->RxSwReadIdx - 1);

#ifdef CUT_THROUGH_DBG
		if (RxRingNo == 0)
			pAd->IoWriteRx++;
		else
			pAd->IoWriteRx1++;
#endif

#ifdef  CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if ((RxRingNo == 0) && *pRxPending > 0) {
		prefetch(pRxRing->Cell[pRxRing->RxSwReadIdx].pNdisPacket);
	}
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

#else /* CACHE_LINE_32B */
	/*
		Because our RXD_SIZE is 16B, but if the cache line size is 32B, we
		will suffer a problem as below:

		1. We flush RXD 0, start address of RXD 0 is 32B-align.
			Nothing occurs.
		2. We flush RXD 1, start address of RXD 1 is 16B-align.
			Because cache line size is 32B, cache must flush 32B, cannot flush
			16B only, so RXD0 and RXD1 will be flushed.
			But when traffic is busy, maybe RXD0 is updated by MAC, i.e.
			DDONE bit is 1, so when the cache flushs RXD0, the DDONE bit will
			be cleared to 0.
		3. Then when we handle RXD0 in the future, we will find the DDONE bit
			is 0 and we will wait for MAC to set it to 1 forever.
	*/
	if (pRxRing->RxSwReadIdx & 0x01)
	{
		RTMP_DMACB *pRxCellLast;
#ifdef RT_BIG_ENDIAN
		RXD_STRUC *pDestRxDLast;
#endif
		/* 16B-align */

		/* update last BD 32B-align, DMA Done bit = 0 */
		pRxCell->LastBDInfo.DDONE = 0;
#ifdef RT_BIG_ENDIAN
		pRxCellLast = &pRxRing->Cell[pRxRing->RxSwReadIdx - 1];
		pDestRxDLast = (RXD_STRUC *) pRxCellLast->AllocVa;
		RTMPDescriptorEndianChange((PUCHAR)&pRxCell->LastBDInfo, TYPE_RXD);
		WriteBackToDescriptor((UCHAR *)pDestRxDLast, (UCHAR *)&pRxCell->LastBDInfo, FALSE, TYPE_RXD);
#endif

		/* update current BD 16B-align, DMA Done bit = 0 */
		pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

		/* flush cache from last BD */
		RTMP_DCACHE_FLUSH(pRxCellLast->AllocPa, 32); /* use RXD_SIZE should be OK */

		/* update SW read and CPU index */
		INC_RING_INDEX(pRxRing->RxSwReadIdx, pRxRing->RxRingSize);
        pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0)
            ? (pRxRing->RxRingSize - 1) : (pRxRing->RxSwReadIdx - 1);
		HIF_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
	}
	else
	{
		/* 32B-align */
		/* do not set DDONE bit and backup it */
		if (pRxRing->RxSwReadIdx >= (pRxRing->RxRingSize - 1))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					("Please change RxRingSize to mutiple of 2!\n"));

			/* flush cache from current BD */
			RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);

			/* update SW read and CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, pRxRing->RxRingSize);
            pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0)
                ? (pRxRing->RxRingSize - 1) : (pRxRing->RxSwReadIdx - 1);
			HIF_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		}
		else
		{
			/* backup current BD */
			pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx + 1];
			pRxCell->LastBDInfo = *pRxD;

			/* update CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, pRxRing->RxRingSize);
		}
	}
#endif /* CACHE_LINE_32B */

done:
	RTMP_SEM_UNLOCK(pRxRingLock);
	*pbReschedule = bReschedule;

	return pRxPacket;
}


NDIS_STATUS MlmeHardTransmitTxRing(RTMP_ADAPTER *pAd, UCHAR QueIdx, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA, *tmac_info, *frm_buf;
	UINT SrcBufLen, frm_len;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	PFRAME_BAR pBar = NULL;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate, wcid, tx_rate;
	UINT32 SwIdx;
	ULONG FreeNum;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	HTTRANSMIT_SETTING *transmit, TransmitSetting;
	MAC_TX_INFO mac_info;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	UCHAR hwQueIdx = QueIdx;
#ifdef MAC_REPEATER_SUPPORT
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    UCHAR cliIdx = 0xFF;
#endif
	struct dev_rate_info *rate;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + tx_hw_hdr_len);

#ifdef CONFIG_AP_SUPPORT
	pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);

#ifdef MAC_REPEATER_SUPPORT
    if ((pMacEntry != NULL) && (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)))
    {
        cliIdx = RTMP_GET_PKT_REPT_CLI_IDX(pPacket);
        if (cliIdx != 0xff) 
		{/*repeater case case*/
            pReptEntry = &pAd->ApCfg.pRepeaterCliPool[cliIdx];
            if ((pReptEntry != NULL) && ((pReptEntry->CliEnable == TRUE) && (pReptEntry->CliValid == TRUE)))
                pMacEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
        } else { /*apcli case*/
			if (IS_ENTRY_REPEATER(pMacEntry)) {
				UCHAR apcli_wcid = 0;
				if (pMacEntry->wdev && (pMacEntry->wdev->func_idx < pAd->ApCfg.ApCliNum) ) {
					apcli_wcid = pAd->ApCfg.ApCliTab[pMacEntry->wdev->func_idx].MacTabWCID;
				} else { // use default apcli0
					apcli_wcid = pAd->ApCfg.ApCliTab[0].MacTabWCID;
				}
				pMacEntry = &pAd->MacTab.Content[apcli_wcid];
			}
		}
    }
#endif

	if (pMacEntry) {
		wdev = pMacEntry->wdev;
		if (!wdev)
			return NDIS_STATUS_FAILURE;
	} else {
		wdev = WdevSearchByAddress(pAd, pHeader_802_11->Addr2);
		if (!wdev)
			return NDIS_STATUS_FAILURE;
	}
#endif /*CONFIG_AP_SUPPORT*/


#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd)|| IS_MT7622(pAd))
	{
		hwQueIdx = HcGetTxRingIdx(pAd,wdev);
	}
#endif /* defined(MT7615) || defined(MT7622) */
	rate = &wdev->rate;

/*
    TODO:
    Carter, MiniportMMRequest check this already, isn't it?
    why check it again?
    redundant code?
*/
	FreeNum = GET_TXRING_FREENO(pAd,hwQueIdx);
	if (FreeNum == 0)
		return NDIS_STATUS_FAILURE;

	SwIdx = pAd->PciHif.TxRing[hwQueIdx].TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].AllocVa;
	NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&hw_hdr_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD  = (TXD_STRUC *) pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].AllocVa;
#endif

	if (pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].pNdisPacket)
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("MlmeHardTransmit Error\n"));
		return NDIS_STATUS_FAILURE;
	}


#ifdef MT_MAC
	frm_buf = pSrcBufVA;
	frm_len = SrcBufLen;
	tmac_info = pSrcBufVA;
#else
	frm_buf = (UCHAR *)(pSrcBufVA + TXINFO_SIZE);
	frm_len = SrcBufLen - TXINFO_SIZE;
	tmac_info =(UCHAR *)(pSrcBufVA + TXINFO_SIZE);
#endif /* MT7603 */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s(): pSrcBufVA=0x%p, pHeader_802_11=0x%p, tmac_info=%p, tx_hw_hdr_len=%d\n",
			__FUNCTION__, pSrcBufVA, pHeader_802_11, tmac_info, tx_hw_hdr_len));

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

//	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
//		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)) ||
//		((pHeader_802_11->FC.Type == FC_TYPE_CNTL) &&
//		(pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ)))
//	{
//		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
//	}

	/* Verify Mlme rate for a/g bands.*/
	if (wdev)
	{
		if ((wdev->channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		{
			MlmeRate = RATE_6;
			transmit = &rate->MlmeTransmit;
			transmit->field.MCS = MCS_RATE_6;
			transmit->field.MODE = MODE_OFDM;
		}
	}

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/

	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */

	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL)
	{
		if (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ)
		{
			pBar = (PFRAME_BAR)(pSrcBufVA + tx_hw_hdr_len);
			bAckRequired = TRUE;
		}
		else
		{
			bAckRequired = FALSE;
		}
#ifdef VHT_TXBF_SUPPORT
		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, (SrcBufLen - TXINFO_SIZE - pAd->chipCap.TXWISize - TSO_SIZE));
#endif /* VHT_TXBF_SUPPORT*/
	}
	else /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST */
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP)
			{
				bInsertTimestamp = TRUE;
				bAckRequired = FALSE;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
				/* Find which MBSSID to be send this probeRsp */
				UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

				if (!(apidx >= pAd->ApCfg.BssidNum) &&
				     (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
				     (rate->MlmeTransmit.field.MODE == MODE_CCK) &&
				     (rate->MlmeTransmit.field.MCS == RATE_1))
				{
					TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
				}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/* Before radar detection done, mgmt frame can not be sent but probe req*/
	/* Because we need to use probe req to trigger driver to send probe req in passive scan*/
	if(wdev)
	{
		if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE) && (wdev->channel > 14)))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
			return (NDIS_STATUS_FAILURE);
		}
	}	

	/*
		Fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one ohysical buffer, and the whole frame size equals
		to the first scatter buffer size

		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/

/* management frame doesn't need encryption. so use RESERVED_WCID no matter u are sending to specific wcid or not */
	/* Only beacon use Nseq=TRUE. So here we use Nseq=FALSE.*/
	if (pMacEntry == NULL)
	{
		if (((pHeader_802_11->FC.Type == FC_TYPE_CNTL) && (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ)) ||
			((pHeader_802_11->FC.Type == FC_TYPE_MGMT) && (pHeader_802_11->FC.SubType == SUBTYPE_ACTION)))
		{
			return NDIS_STATUS_FAILURE;
		}

		wcid = RESERVED_WCID;
		if (wdev)
		{
			GET_GroupKey_WCID(wdev, wcid);
			if (wcid >= MAX_LEN_OF_MAC_TABLE)
			{
				if ((wdev->wdev_type == WDEV_TYPE_AP) ||
					(wdev->wdev_type == WDEV_TYPE_GO) ||
					(wdev->wdev_type == WDEV_TYPE_ADHOC))
				{
					return NDIS_STATUS_FAILURE;
				}
				else if ((wdev->wdev_type == WDEV_TYPE_STA) ||
					(wdev->wdev_type == WDEV_TYPE_GC) ||
					(wdev->wdev_type == WDEV_TYPE_APCLI))
				{
					/*because sta role has no bssinfo before linkup with rootap. use a temp idx here.*/
					wcid = 0;
				}
			}
		}
	}
	else
	{
		wcid = pMacEntry->wcid;
	}
	tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
	transmit = &rate->MlmeTransmit;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.FRAG = FALSE;

	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;

	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.Ack = bAckRequired;
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = wcid;
	mac_info.TID = 0;
#ifdef MT_MAC
	mac_info.wmm_set = HcGetWmmIdx(pAd,wdev);
	mac_info.q_idx = QueIdx;
    if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry))
        mac_info.OmacIdx = pAd->MacTab.tr_entry[pMacEntry->wcid].OmacIdx;
    else
        mac_info.OmacIdx = wdev->OmacIdx;

	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.Length = (SrcBufLen - tx_hw_hdr_len);
	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;
		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;
#if defined(MT7615) || defined(MT7622)
		if (pHeader_802_11->FC.SubType == SUBTYPE_BEACON)
			mac_info.q_idx = TxQ_IDX_BCN0;
#else
		if (pHeader_802_11->FC.SubType == SUBTYPE_BEACON)
			mac_info.q_idx = Q_IDX_BCN;
#endif /* defined(MT7615) || defined(MT7622) */
		mac_info.PID = PID_MGMT;

#ifdef DOT11W_PMF_SUPPORT
		PMF_PerformTxFrameAction(pAd, pHeader_802_11, SrcBufLen, tx_hw_hdr_len, &mac_info.prot);
#endif
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_DATA_NULL:
				mac_info.hdr_len = 24;
                tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
                transmit = &rate->MlmeTransmit;
				break;
			case SUBTYPE_QOS_NULL:
				mac_info.hdr_len = 26;
				tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
                transmit = &rate->MlmeTransmit;
				break;
			default:
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
						__FUNCTION__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
				hex_dump("DataFrame", frm_buf, frm_len);
				break;
		}
		mac_info.WCID = wcid;
		if (pMacEntry && pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt)
			mac_info.PID = PID_PS_DATA;
		 else
			mac_info.PID = PID_MGMT;
	}
	else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL)
	{
		switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_BLOCK_ACK_REQ:
				mac_info.PID = PID_CTL_BAR;
				mac_info.hdr_len = 16;
				mac_info.SpeEn = 0;
				mac_info.TID = pBar->BarControl.TID;
				if (wdev->channel > 14)
				{ /* 5G */
					TransmitSetting.field.MODE = MODE_OFDM;
				}
				else
				{ /* 2.4G */
					TransmitSetting.field.MODE = MODE_CCK;
				}
				TransmitSetting.field.BW = BW_20;
				TransmitSetting.field.STBC = 0;
				TransmitSetting.field.ShortGI = 0;
				TransmitSetting.field.MCS = 0;
				TransmitSetting.field.ldpc = 0;
				transmit = &TransmitSetting;
				break;
			default:
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
                    ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) "
                        "send to MgmtRing, need to assign the length!\n",
						__FUNCTION__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
				hex_dump("Control Frame", frm_buf, frm_len);
				break;
		}
	}
#else
	mac_info.Length = (SrcBufLen - TXINFO_SIZE - pAd->chipCap.TXWISize - TSO_SIZE);
	mac_info.PID = PID_MGMT;
#endif

	mac_info.TxRate = tx_rate;
	mac_info.Txopmode = IFS_BACKOFF;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;

	/* PCI use Miniport to send NULL frame and need to add NULL frame TxS control here to enter PSM */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MT_MAC
		if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
			&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)))
		{
#ifdef WH_EZ_SETUP		
			if(IS_EZ_SETUP_ENABLED(pMacEntry->wdev))
				mac_info.PsmBySw = 1;
#endif			
			if ((pMacEntry != NULL) && (IS_ENTRY_APCLI(pMacEntry)
#ifdef MAC_REPEATER_SUPPORT
				|| IS_ENTRY_REPEATER(pMacEntry)
#endif /* MAC_REPEATER_SUPPORT */
				))
			{
				/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
				if (IS_MT7615(pAd) || IS_MT7622(pAd))
					mac_info.IsAutoRate = TRUE;
			}
		}
#endif /* MT_MAC */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (write_tmac_info(pAd, tmac_info, &mac_info, transmit) == NDIS_STATUS_FAILURE) {
	 return (NDIS_STATUS_FAILURE);}
	RTMP_SET_PACKET_WDEV(pPacket,wdev->wdev_idx);


#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
	}
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
	}
#endif
#endif

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		HAL_KickOutMgmtTx(pAd, mac_info.q_idx, pPacket, pSrcBufVA, SrcBufLen);
		return NDIS_STATUS_SUCCESS;
	}
#endif /* defined(MT7615) || defined(MT7622) */

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef SPECIFIC_TX_POWER_SUPPORT
	if (IS_RT6352(pAd) && (pMacEntry == NULL)) {
		TXWI_STRUC *pFirstTxWI = (TXWI_STRUC *)tmac_info;

		pFirstTxWI->TxPwrAdj = TxPwrAdj;
	}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].pNdisPacket = pPacket;
	pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
#endif
#endif

#ifdef VHT_TXBF_SUPPORT
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL && pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Send VhtNDPA to peer(wcid=%d, pMacEntry=%p) with dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
					__FUNCTION__, wcid, pMacEntry, get_phymode_str(transmit->field.MODE),
					get_bw_str(transmit->field.BW),
					(transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf)));

		hex_dump("VHT NDPA frame raw data", pSrcBufVA, SrcBufLen);
	}
#endif /* VHT_TXBF_SUPPORT */

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif

	pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].PacketPa =  PCI_MAP_SINGLE(pAd, frm_buf, frm_len, 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = frm_len;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pAd->PciHif.TxRing[hwQueIdx].Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, NULL, TRUE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
	{
		RTMP_TX_RING *ring = &pAd->PciHif.TxRing[hwQueIdx];
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
		RTMP_DCACHE_FLUSH(ring->Cell[SwIdx].AllocPa, TXD_SIZE);

	   	/* Increase TX_CTX_IDX, but write to register later.*/
		INC_RING_INDEX(ring->TxCpuIdx, TX_RING_SIZE);
		HIF_IO_WRITE32(pAd,ring->hw_cidx_addr,ring->TxCpuIdx);
	}
	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_ANDES_SUPPORT
BOOLEAN RxRing1DoneInterruptHandle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	RXD_STRUC *pRxD = NULL;
	RXINFO_STRUC *pRxInfo = NULL;
	UCHAR *pData = NULL;
	PNDIS_PACKET pRxPacket = NULL;
	PHEADER_802_11 pHeader = NULL;
	RX_BLK rxblk, *pRxBlk = NULL;
	UINT8 RXWISize = pAd->chipCap.RXWISize;
#ifdef RLT_MAC
	RXFCE_INFO *pFceInfo;
#endif /* RLT_MAC */
	RTMP_RX_RING *pRxRing = &pAd->PciHif.RxRing[HIF_RX_IDX1];

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1)
	{
		if (/*RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF) ) ||*/
			!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			break;
		}

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

#ifdef RTMP_MAC_PCI
		if (RxProcessed++ > MAX_RX1_PROCESS_CNT)
		{
			bReschedule = TRUE;

#ifdef CUT_THROUGH_DBG
			RxProcessed--;
			if ((RxProcessed >= 1) && (RxProcessed <= 64))
				pAd->Rx1MaxProcessCntA++;
			else if ((RxProcessed >= 65) && (RxProcessed <= 128))
				pAd->Rx1MaxProcessCntB++;
			else if ((RxProcessed >= 129) && (RxProcessed <= 192))
				pAd->Rx1MaxProcessCntC++;
			else if ((RxProcessed >= 193) && (RxProcessed <= 256))
				pAd->Rx1MaxProcessCntD++;
#endif
			break;
		}
#endif /* RTMP_MAC_PCI */

		/*
			1. allocate a new data packet into rx ring to replace received packet
				then processing the received packet
			2. the callee must take charge of release of packet
			3. As far as driver is concerned, the rx packet must
				a. be indicated to upper layer or
				b. be released if it is discarded
		*/

		os_zero_mem(&rxblk,sizeof(RX_BLK));
		pRxBlk = &rxblk;

		pRxPacket = GetPacketFromRxRing(pAd, &pRxBlk, &bReschedule, &RxPending, 1);

		if (pRxPacket == NULL)
		{
#ifdef CUT_THROUGH_DBG
			if ((RxProcessed >= 1) && (RxProcessed <= 64))
				pAd->Rx1MaxProcessCntA++;
			else if ((RxProcessed >= 65) && (RxProcessed <= 128))
				pAd->Rx1MaxProcessCntB++;
			else if ((RxProcessed >= 129) && (RxProcessed <= 192))
				pAd->Rx1MaxProcessCntC++;
			else if ((RxProcessed >= 193) && (RxProcessed <= 256))
				pAd->Rx1MaxProcessCntD++;
#endif
			break;
		}

		/* get rx descriptor and data buffer */
		pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];
		pRxInfo = rxblk.pRxInfo;
		pData = GET_OS_PKT_DATAPTR(pRxPacket);
		pHeader = (PHEADER_802_11)(pData + RXWISize);

#ifdef RLT_MAC
		pFceInfo = rxblk.pRxFceInfo;
		if (pFceInfo->info_type == CMD_PACKET)
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, ("%s: Receive command packet.\n", __FUNCTION__));
			PciRxCmdMsgComplete(pAd, pFceInfo);
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
#endif /* RLT_MAC */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			if ((rxblk.DataSize == 0) && (pRxPacket)) {
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				continue;
			}
		}
#endif /* MT_MAC */
	}

	if (pRxRing->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		pRxRing->sw_read_idx_inc = 0;
	}

	return bReschedule;
}


VOID RTMPHandleTxRing8DmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
/*	int 		 i;*/
	UCHAR	FREE = 0;
	RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;

	NdisAcquireSpinLock(&pAd->CtrlRingLock);

	HIF_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);
	while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif

		pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
			continue;
		}

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&pAd->CtrlRingLock);
}

#if defined(MT7615) || defined(MT7622)
BOOLEAN RTMPHandleFwDwloCmdRingDmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	UCHAR	FREE = 0;
	RTMP_RING *pRing = (RTMP_RING *)&pAd->FwDwloRing;

	NdisAcquireSpinLock(&pRing->RingLock);

	HIF_IO_READ32(pAd, pRing->hw_didx_addr, &pRing->TxDmaIdx);
	while (pRing->TxSwFreeIdx!= pRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pRing->Cell[pRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pRing->Cell[pRing->TxSwFreeIdx].AllocVa);
#endif

		pPacket = pRing->Cell[pRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pRing->Cell[pRing->TxSwFreeIdx].pNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRing->Cell[pRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&pRing->RingLock);

    return FALSE;
}
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */


#ifdef MT_MAC
VOID RTMPHandleBcnDmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;
	UCHAR *tmac_info = NULL;

	RTMP_SEM_LOCK(&pAd->BcnRingLock);

	HIF_IO_READ32(pAd, pBcnRing->hw_didx_addr, &pBcnRing->TxDmaIdx);
	while (pBcnRing->TxSwFreeIdx!= pBcnRing->TxDmaIdx)
	{
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
#endif

		pPacket = pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket;
		if (pPacket == NULL)
		{
			INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);
			continue;
		}

		PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
		tmac_info = GET_OS_PKT_DATAPTR(pPacket);
		pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif

	}

	RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
}
#endif /* MT_MAC */



#ifdef MT_MAC
#ifdef CUT_THROUGH
INT dump_txp_info(RTMP_ADAPTER *pAd, CR4_TXP_MSDU_INFO *txp_info)
{
	INT cnt;


	hex_dump("CT_TxP_Info Raw Data: ", (UCHAR *)txp_info, sizeof(CR4_TXP_MSDU_INFO));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXP_Fields:\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\ttype_and_flags=0x%x\n", txp_info->type_and_flags));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tmsdu_token=%d\n", txp_info->msdu_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tbss_index=%d\n", txp_info->bss_index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tbuf_num=%d\n", txp_info->buf_num));

	for (cnt = 0; cnt < txp_info->buf_num; cnt++)
	{
		if (cnt >= MAX_BUF_NUM_PER_PKT)
			break;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\tbuf_ptr=0x%x\n", txp_info->buf_ptr[cnt]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\tbuf_len=%d(0x%x)\n", txp_info->buf_len[cnt], txp_info->buf_len[cnt]));
	}

	return 0;
}


INT write_txp_info_data(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	CR4_TXP_MSDU_INFO *cr4_txp_msdu_info = (CR4_TXP_MSDU_INFO *)buf;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT16 token;
	struct wifi_dev *wdev;
	ULONG HwCnt;
	UCHAR BssInfoIdx = 0;
	UCHAR RingIdx = pTxBlk->QueIdx;

	if (pTxBlk->wdev_idx < WDEV_NUM_MAX)
	{
		wdev = pAd->wdev_list[pTxBlk->wdev_idx];
	}
	else
	{
		wdev = pAd->wdev_list[0];
	}

#if defined(MT7615) || defined(MT7622)
	RingIdx = HcGetTxRingIdx(pAd,wdev);
#endif

	HwCnt = GET_TXRING_FREENO(pAd,RingIdx);

	if (HwCnt <= pktTokenCb->TxRingLowWaterMark)
	{
		RTMPFreeTXDUponTxDmaDone(pAd, pTxBlk->QueIdx,RingIdx, TRUE);
		HwCnt = GET_TXRING_FREENO(pAd,RingIdx);

		if (HwCnt == 0)
		{
#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 0)
			cut_through_tx_flow_block(pktTokenCb, wdev->if_dev, NO_ENOUGH_FREE_TX_RING, TRUE, RingIdx);

			pktTokenCb->TxRingFullCnt++;
#endif

			return FALSE;
		}
	}


	if(pktTokenCb->tx_id_list.token_inited != TRUE || pktTokenCb->tx_id_list.list==NULL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s(): tx_id_list.token is not prepared!\n",__FUNCTION__));
		return FALSE;

	}

	if (pktTokenCb->tx_id_list.list->FreeTokenCnt < pktTokenCb->TxTokenLowWaterMark)

	{
#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 0)
		RxRing1DoneInterruptHandle(pAd);

		if (pktTokenCb->tx_id_list.list->FreeTokenCnt < pktTokenCb->TxTokenLowWaterMark)
		{
			cut_through_tx_flow_block(pktTokenCb, wdev->if_dev, NO_ENOUGH_FREE_TX_TOKEN, TRUE, RingIdx);

			pktTokenCb->TxTokenFullCnt++;

			return FALSE;
		}
#else
        return FALSE;
#endif
	}

	NdisZeroMemory(cr4_txp_msdu_info, sizeof(CR4_TXP_MSDU_INFO));

#ifdef CONFIG_HOTSPOT_R2
	/* Inform CR4 to bypass ProxyARP check on this packet */
	if(RTMP_IS_PACKET_DIRECT_TX(pTxBlk->pPacket))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_HSR2_TX;
#endif /* HOTSPOT_SUPPORT_R2 */

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_CT_WithTxD))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_APPLY_TXD;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_NONE_CIPHER_FRAME;

	cr4_txp_msdu_info->buf_ptr[0] = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	
	if (RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) {
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_MGN_FRAME;
		token = cut_through_tx_enq(pktTokenCb, pTxBlk->pPacket, TOKEN_TX_MGT, 
					cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	}
	else {
		token = cut_through_tx_enq(pktTokenCb, pTxBlk->pPacket, TOKEN_TX_DATA,
					cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	}

#ifdef MAC_REPEATER_SUPPORT
	if (pTxBlk->pMacEntry && IS_ENTRY_REPEATER(pTxBlk->pMacEntry))
	{
		cr4_txp_msdu_info->rept_wds_wcid = pTxBlk->pMacEntry->wcid;
	    }
	else
	/*TODO: WDS case.*/
#endif
#ifdef MWDS
    if (pTxBlk->pMacEntry && IS_ENTRY_MWDS(pTxBlk->pMacEntry))
    {
        cr4_txp_msdu_info->rept_wds_wcid = pTxBlk->pMacEntry->wcid;
    }
    else
#endif /* MWDS */
	{
	    cr4_txp_msdu_info->rept_wds_wcid = 0xff;
	}

	BssInfoIdx = wdev->bss_info_argument.ucBssIndex;

	if (token == PKT_TOKEN_ID_INVALID) {
		PCI_UNMAP_SINGLE(pAd, cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket), RTMP_PCI_DMA_TODEVICE);
		return FALSE;
	}
	cr4_txp_msdu_info->msdu_token = token;
	cr4_txp_msdu_info->bss_index = BssInfoIdx;
	cr4_txp_msdu_info->buf_num = 1; // os get scatter.
	cr4_txp_msdu_info->buf_len[0] = pTxBlk->SrcBufLen;
#ifdef RT_BIG_ENDIAN
	cr4_txp_msdu_info->msdu_token = cpu2le16(cr4_txp_msdu_info->msdu_token );
	cr4_txp_msdu_info->buf_ptr[0] = cpu2le32(cr4_txp_msdu_info->buf_ptr[0]);
	cr4_txp_msdu_info->buf_len[0] = cpu2le16(cr4_txp_msdu_info->buf_len[0]);
	cr4_txp_msdu_info->type_and_flags = cpu2le16(cr4_txp_msdu_info->type_and_flags);
#endif
	pTxBlk->TxTokenID[0] = cr4_txp_msdu_info->msdu_token;
	pTxBlk->MpduHeaderLen += sizeof(CR4_TXP_MSDU_INFO);

	return TRUE;
}


#ifdef MANUAL_MU
static UINT32 mu_pkt_tx_cnt = 0;
static UINT32 fake_pkt_tx_cnt = 0;
#endif /* MANUAL_MU */
INT CutThroughPktTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	USHORT freeCnt = 1;
	QUEUE_ENTRY *pQEntry;
	UINT32 WdevIdx = pTxBlk->wdev_idx;
#ifdef MANUAL_MU
	PNDIS_PACKET fake_pkt = NULL;
	UINT32 fake_hw_rsv_len = 0, fake_mpdu_hdr_len = 0, fake_hdr_pad_len = 0, fake_wifi_hdr_len = 0;
#endif /* MANUAL_MU */
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
	struct wifi_dev *wdev = pAd->wdev_list[WdevIdx];
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;

	ASSERT(pTxBlk);

	if(!wdev || pktTokenCb->tx_id_list.token_inited != TRUE)
	{
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): wdev not enable yet! or token not inited!, token=%p,%p,%pS\n",
				__FUNCTION__,pktTokenCb,&pktTokenCb->tx_id_list.token_inited,OS_TRACE));

		return 0;
	}

	/*for get wmm set from TxBlk*/
	pTxBlk->wmm_set = HcGetWmmIdx(pAd,wdev);

	if (pTxBlk->TxFrameType == TX_OFFLOAD_FRAME)
	{
		/* offload both TxD/header translation to chip */
		write_tmac_info_ct(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

#if defined(MT7615) || defined(MT7622)
		pTxBlk->QueIdx= HcGetTxRingIdx(pAd,wdev);
#endif

		if (write_txp_info_data(pAd, &pTxBlk->HeaderBuf[pAd->chipCap.tx_hw_hdr_len], pTxBlk) == FALSE)
		{
			return NETDEV_TX_BUSY;
		}

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Ring Idx = %d\n",__FUNCTION__,pTxBlk->QueIdx));

		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);
	}
	else if (RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket))
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_OffloadFillTxBlkInfo(pAd, pTxBlk) != TRUE)
		{
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("<--%s(%d): ##########Fail#########\n", __FUNCTION__, __LINE__));
			return 0;
		}

		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

		NdisCopyMemory(&pTxBlk->HeaderBuf[0], pTxBlk->pSrcBufData, pAd->chipCap.tx_hw_hdr_len);
		pTxBlk->pSrcBufData += pAd->chipCap.tx_hw_hdr_len;
		pTxBlk->SrcBufLen -= pAd->chipCap.tx_hw_hdr_len;
		pTxBlk->MpduHeaderLen = 0;
		pTxBlk->wifi_hdr_len = 0;
		pTxBlk->HdrPadLen = 0;
		pTxBlk->hw_rsv_len = 0;

#if defined(MT7615) || defined(MT7622)
		pTxBlk->QueIdx= HcGetTxRingIdx(pAd,wdev);
#endif

		if (write_txp_info_data(pAd, &pTxBlk->HeaderBuf[pAd->chipCap.tx_hw_hdr_len], pTxBlk) == FALSE)
		{
			return NETDEV_TX_BUSY;
		}

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Ring Idx = %d\n",__FUNCTION__,pTxBlk->QueIdx));

		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);
	}
	else if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():pSrcBufData=0x%p, pSrcBufHeader=0x%p, SrcBufLen=%d\n",
				__FUNCTION__, pTxBlk->pSrcBufData, pTxBlk->pSrcBufHeader, pTxBlk->SrcBufLen));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():802.3 DataFrm, MpduHdrL=%d,WFHdrL=%d,HdrPadL=%d,HwRsvL=%d, wdev_idx=%d\n",
				__FUNCTION__, pTxBlk->MpduHeaderLen, pTxBlk->wifi_hdr_len, pTxBlk->HdrPadLen,
				pTxBlk->hw_rsv_len, pTxBlk->wdev_idx));

		if (write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk) == NDIS_STATUS_FAILURE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}
#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, &pTxBlk->HeaderBuf[0], TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif

#if defined(MT7615) || defined(MT7622)
		pTxBlk->QueIdx= HcGetTxRingIdx(pAd,wdev);
#endif

		if (write_txp_info_data(pAd, &pTxBlk->HeaderBuf[pAd->chipCap.tx_hw_hdr_len], pTxBlk) == FALSE)
		{
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Ring Idx = %d\n",__FUNCTION__,pTxBlk->QueIdx));

		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);
	}
	else
	{
		INT dot11_meta_hdr_len, tx_hw_hdr_len;
		PNDIS_PACKET pkt;
		NDIS_STATUS status;
		UCHAR *dot11_hdr;
		TX_BLK NewTxBlk, *pNewTxBlk = &NewTxBlk;

		NdisCopyMemory(pNewTxBlk, pTxBlk, sizeof(*pTxBlk));

		dot11_meta_hdr_len = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;
		tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
		dot11_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():DataFrm, MpduHdrL=%d,WFHdrL=%d,HdrPadL=%d,HwRsvL=%d, NeedCopyHdrLen=%d\n",
				__FUNCTION__, pTxBlk->MpduHeaderLen, pTxBlk->wifi_hdr_len, pTxBlk->HdrPadLen,
				pTxBlk->hw_rsv_len, dot11_meta_hdr_len));

		status = RTMPAllocateNdisPacket(pAd, &pkt,
						dot11_hdr, dot11_meta_hdr_len,
						pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);

		if (status != NDIS_STATUS_SUCCESS)
		{
			printk("%s():AllocNdisPkt failed!\n", __FUNCTION__);
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}

		if ((pTxBlk->FragIdx == TX_FRAG_ID_NO) || (pTxBlk->FragIdx == TX_FRAG_ID_LAST))
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);


#ifdef VENDOR_FEATURE1_SUPPORT
		pNewTxBlk->HeaderBuf = (UCHAR *)pNewTxBlk->HeaderBuffer;
#endif
		pNewTxBlk->pPacket = pkt;
		pNewTxBlk->pSrcBufData = GET_OS_PKT_DATAPTR(pkt);
		pNewTxBlk->SrcBufLen = GET_OS_PKT_LEN(pkt);

		TX_BLK_SET_FLAG(pNewTxBlk, fTX_CT_WithTxD);

		if (write_tmac_info_Data(pAd, &pNewTxBlk->HeaderBuf[0], pNewTxBlk) == NDIS_STATUS_FAILURE) {
			RELEASE_NDIS_PACKET(pAd, pNewTxBlk->pPacket, NDIS_STATUS_SUCCESS);
			return 0;
		}
#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, &pNewTxBlk->HeaderBuf[0], TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
		pNewTxBlk->MpduHeaderLen = 0;
		pNewTxBlk->wifi_hdr_len = 0;
		pNewTxBlk->HdrPadLen = 0;
		pNewTxBlk->hw_rsv_len = 0;

#if defined(MT7615) || defined(MT7622)
		pNewTxBlk->QueIdx= HcGetTxRingIdx(pAd,wdev);
		pTxBlk->QueIdx= pNewTxBlk->QueIdx; /* in case QueIdx will be used later in KickOutTx */
#endif

		if (write_txp_info_data(pAd, &pNewTxBlk->HeaderBuf[pAd->chipCap.tx_hw_hdr_len], pNewTxBlk) == FALSE)
		{
			RELEASE_NDIS_PACKET(pAd, pNewTxBlk->pPacket, NDIS_STATUS_SUCCESS);
			return 0;
		}
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, pNewTxBlk->pSrcBufData, DIR_WRITE, FALSE);
#endif
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Ring Idx = %d\n",__FUNCTION__,pNewTxBlk->QueIdx));
		HAL_WriteTxResource(pAd, pNewTxBlk, TRUE, &freeCnt);

	}

#ifdef CONFIG_AP_SUPPORT
	pEntry = &pAd->MacTab.Content[(UCHAR)RTMP_GET_PACKET_WCID(pTxBlk->pPacket)];
	INC_COUNTER64(pEntry->TxPackets);
	pEntry->TxBytes += pTxBlk->SrcBufLen;
	pEntry->OneSecTxBytes += pTxBlk->SrcBufLen;
	pEntry->one_sec_tx_pkts++;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CUT_THROUGH_DBG
	pAd->IoWriteTx++;
#endif
	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	return 0;
}


#ifdef CUT_THROUGH_FULL_OFFLOAD
#ifdef FAST_PATH_TXQ
INT32 FullOffloadFrameTx(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, UCHAR QueIdx, UCHAR UserPriority)
{

	struct FastPathTxQueElement *FPTxElement;
#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 1)
    UINT32 wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
    struct wifi_dev *wdev = pAd->wdev_list[wdev_idx];
#endif

	if (GET_OS_PKT_LEN(pPacket) <= LENGTH_802_3) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,("%s: 802.3 Length too small in FastPathTxFreeQue\n", __FUNCTION__));
		return 0;
	}
	
#ifdef RTMP_UDMA_SUPPORT
	/* Remove the 4-byte vlan header from tagged packets */
	if (((GET_OS_PKT_DATAPTR(pPacket))[12] == 0x81) && ((GET_OS_PKT_DATAPTR(pPacket))[13] == 0x00))
	{
		memmove((GET_OS_PKT_DATAPTR(pPacket)) + 4, (GET_OS_PKT_DATAPTR(pPacket)), 12);
		skb_pull(pPacket, 4);
	}
#endif	/* RTMP_UDMA_SUPPORT */
	RTMP_SEM_LOCK(&pAd->FastPathTxFreeQueLock);
	FPTxElement = DlListFirst(&pAd->FastPathTxFreeQue, struct FastPathTxQueElement, List);

	if (!FPTxElement)
	{
		RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		pAd->FPTxElementFullNum++;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,("%s: Not available element in FastPathTxFreeQue\n", __FUNCTION__));
		return 0;
	}

	DlListDel(&FPTxElement->List);
	pAd->FPTxElementFreeNum--;
	
	if (pAd->FPTxElementFreeNum < pAd->MinFPTxElementFreeNum)
		pAd->MinFPTxElementFreeNum = pAd->FPTxElementFreeNum;
	
	RTMP_SEM_UNLOCK(&pAd->FastPathTxFreeQueLock);

#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 1)
    if (pAd->FPTxElementFreeNum < FP_TX_FREE_LOWER_BOUND) {
        cut_through_tx_flow_block(pAd->PktTokenCb, wdev->if_dev, NO_ENOUGH_FREE_TX_ELEM, TRUE, HcGetTxRingIdx(pAd,wdev));
        pAd->fp_txBlocked = TRUE;
    }
#endif

	FPTxElement->QueIdx = QueIdx;
	FPTxElement->UserPriority = UserPriority;
	FPTxElement->pPacket = pPacket;

#ifdef LINUX
	skb_orphan(pPacket);
#endif

	RTMP_SEM_LOCK(&pAd->FastPathTxQueLock);
	pAd->FastPathTxQueNum++;
	DlListAddTail(&pAd->FastPathTxQue, &FPTxElement->List);
	RTMP_SEM_UNLOCK(&pAd->FastPathTxQueLock);

	if (pAd->bFastPathTaskSchedulable)
	{
		RTMP_NET_TASK_DATA_ASSIGN(&pAd->FastPathDequeTask, (ULONG)(pAd));
		RTMP_OS_TASKLET_SCHE(&pAd->FastPathDequeTask);
	}
	return 0;

}


static struct FastPathTxQueElement *GetFPMgmtQueElement(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	struct FastPathTxQueElement *FPTxElement;

	RTMP_SPIN_LOCK(&pAd->MgmtQueLock);
	FPTxElement = DlListFirst(&pAd->MgmtQue, struct FastPathTxQueElement, List);

	if (FPTxElement)
	{
		DlListDel(&FPTxElement->List);
		pAd->MgmtQueNum--;

		if (RTMP_GET_PACKET_TXTYPE(FPTxElement->pPacket) == TX_MCAST_FRAME)
			pTxBlk->TxFrameType = TX_MCAST_FRAME;
		else
			pTxBlk->TxFrameType = TX_LEGACY_FRAME;

		pTxBlk->wdev_idx = RTMP_GET_PACKET_WDEV(FPTxElement->pPacket);
	}

	RTMP_SPIN_UNLOCK(&pAd->MgmtQueLock);

	if (FPTxElement)
	{
		return FPTxElement;
	}

	return NULL;
}

static struct FastPathTxQueElement *GetFPTxQueElement(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	struct FastPathTxQueElement *FPTxElement;

	RTMP_SPIN_LOCK(&pAd->FastPathTxQueLock);
	FPTxElement = DlListFirst(&pAd->FastPathTxQue, struct FastPathTxQueElement, List);

	if (FPTxElement)
	{
		DlListDel(&FPTxElement->List);
		pAd->FastPathTxQueNum--;
		pTxBlk->TxFrameType = TX_OFFLOAD_FRAME;
	}

	RTMP_SPIN_UNLOCK(&pAd->FastPathTxQueLock);

	if (FPTxElement)
	{
		return FPTxElement;
	}

	return NULL;
}

#ifdef CONFIG_TX_DELAY
static struct FastPathTxQueElement *FirstFPElement(RTMP_ADAPTER *pAd)
{
	struct FastPathTxQueElement *FPTxElement = NULL;
	RTMP_SPIN_LOCK(&pAd->FastPathTxQueLock);
	FPTxElement = DlListFirst(&pAd->FastPathTxQue, struct FastPathTxQueElement, List);
	RTMP_SPIN_UNLOCK(&pAd->FastPathTxQueLock);

	return FPTxElement;
}

enum hrtimer_restart que_agg_timeout(struct hrtimer *timer)
{
	RTMP_ADAPTER *pAd = container_of(timer, struct _RTMP_ADAPTER, que_agg_timer);

	if (pAd->bFastPathTaskSchedulable
		&& (pAd->FastPathTxQueNum > 0)) {
		RTMP_NET_TASK_DATA_ASSIGN(&pAd->FastPathDequeTask, (ULONG)(pAd));
		RTMP_OS_TASKLET_SCHE(&pAd->FastPathDequeTask);
	
		pAd->force_deq = TRUE;
	}

	return HRTIMER_NORESTART;
}
#endif

VOID FastPathDequeBh(ULONG Param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)Param;
	TX_BLK tx_blk, *pTxBlk;
	UCHAR Wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	struct FastPathTxQueElement *FPTxElement;
	INT Ret = 0;
	BOOLEAN KickRing0 = FALSE, KickRing1 = FALSE;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
#ifdef APCLI_SUPPORT
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	UCHAR *pMacAddr = NULL;
#endif
#ifdef MAT_SUPPORT
	PUCHAR pSrcBufVA = NULL;
	PNDIS_PACKET pPacket;
	PACKET_INFO PacketInfo;
	PNDIS_PACKET convertPkt = NULL;
#endif

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_FP_BH(__FUNCTION__);
#endif

#ifdef CONFIG_TX_DELAY
	if ((pAd->que_agg_en) &&
		(pAd->FastPathTxQueNum > 0) && (pAd->MgmtQueNum == 0) &&
		 (pAd->FastPathTxQueNum < pAd->TxProcessBatchCnt) &&
		(!pAd->force_deq)) {

		FPTxElement = FirstFPElement(pAd);

		if (FPTxElement) {
			if ((GET_OS_PKT_LEN(FPTxElement->pPacket) >= pAd->min_pkt_len) &&
				(GET_OS_PKT_LEN(FPTxElement->pPacket) <= MAX_AGG_PKT_LEN)) {
	
				if (!hrtimer_active(&pAd->que_agg_timer))
					hrtimer_start(&pAd->que_agg_timer, ktime_set(0, pAd->que_agg_timeout_value),  HRTIMER_MODE_REL);
		
				return;
			}
		}
	}
#endif

	while (pAd->bFastPathTaskSchedulable)
	{
		if(!pktTokenCb->tx_id_list.token_inited)
		{
			break;
		}

		NdisZeroMemory((UCHAR *)&tx_blk, sizeof(TX_BLK));
		pTxBlk = &tx_blk;

		FPTxElement = GetFPMgmtQueElement(pAd, pTxBlk);
		if (FPTxElement)
		{
			goto pkt_handle;
		}


		FPTxElement = GetFPTxQueElement(pAd, pTxBlk);
		if (!FPTxElement)
		{
			break;
		}

pkt_handle:
		pTxBlk->TotalFrameNum = 1;
		pTxBlk->TotalFragNum = 1;
		pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(FPTxElement->pPacket);
		pTxBlk->pPacket = FPTxElement->pPacket;
		pTxBlk->QueIdx = FPTxElement->QueIdx;

		Wcid = RTMP_GET_PACKET_WCID(FPTxElement->pPacket);
#ifdef MWDS
        pMacEntry = &pAd->MacTab.Content[Wcid];
#endif
		/*if wcid is out of MAC table size, free it*/
		if(Wcid >= MAX_LEN_OF_MAC_TABLE)
		{
		    MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                     ("%s(): WCID is invalid\n", __FUNCTION__));
			RELEASE_NDIS_PACKET(pAd, FPTxElement->pPacket, NDIS_STATUS_SUCCESS);

			FPTxElement->pPacket = NULL;
			RTMP_SPIN_LOCK(&pAd->FastPathTxFreeQueLock);
			DlListAddTail(&pAd->FastPathTxFreeQue, &FPTxElement->List);
			pAd->FPTxElementFreeNum++;
			RTMP_SPIN_UNLOCK(&pAd->FastPathTxFreeQueLock);
			break;
		}

#ifdef VENDOR_FEATURE1_SUPPORT
		pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /* VENDOR_FEATURE1_SUPPORT */

		InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(FPTxElement->pPacket));


#ifdef MWDS
        pTxBlk->pMacEntry = pMacEntry;/*hook pMacEntry for fill txp.*/
#endif

#ifdef APCLI_SUPPORT
        pPacket = pTxBlk->pPacket;
#ifndef MWDS
        pMacEntry = &pAd->MacTab.Content[Wcid];
#endif
        if ((IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
            (pTxBlk->TxFrameType == TX_OFFLOAD_FRAME))
        {
 #ifdef MWDS  
            if (IS_MWDS_OPMODE_APCLI(pMacEntry))
            {
                convertPkt = FALSE;
                pMacAddr = NULL;
            }
            else
#endif /* MWDS */
#ifdef MAT_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
            if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn))
            {
                UCHAR tmpIdx;

                pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;
                if(pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR)
                {
                    tmpIdx = REPT_MLME_START_IDX + pMacEntry->MatchReptCliIdx;
                    convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
                    pMacAddr = &pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress[0];
                }
            }
            else
#endif /* MAC_REPEATER_SUPPORT */
            {
                /* For each tx packet, update our MAT convert engine databases.*/
                convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pTxBlk->OpMode);
                pMacAddr = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx].wdev.if_addr[0];
            }

            if(convertPkt)
            {
                RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
                pPacket = convertPkt;
                RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
                pTxBlk->pPacket = convertPkt;
		        FPTxElement->pPacket = convertPkt;
            }

#ifdef HDR_TRANS_TX_SUPPORT
            if ((pMacAddr != NULL)
 #ifdef MWDS
                || IS_MWDS_OPMODE_APCLI(pMacEntry)
 #endif /* MWDS */
            )
            {
                /*cuz the pkt will be TX_HDR by hw, change the 802.3 sa, too.*/
                if(pMacAddr)
                {
                    pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
#ifdef MWDS
		            if (!(IS_ENTRY_MWDS(pMacEntry)))
#endif /* MWDS */
                        NdisMoveMemory(pSrcBufVA+6, pMacAddr, MAC_ADDR_LEN);
                }
            }
            else
            {
                MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("%s(): pMacAddr == NULL)\n", __FUNCTION__));

               	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
				FPTxElement->pPacket = NULL;
				RTMP_SPIN_LOCK(&pAd->FastPathTxFreeQueLock);
				DlListAddTail(&pAd->FastPathTxFreeQue, &FPTxElement->List);
		        pAd->FPTxElementFreeNum++;
				RTMP_SPIN_UNLOCK(&pAd->FastPathTxFreeQueLock);

				break;
            }
#endif
#endif /* MAT_SUPPORT */
            pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
#ifndef MWDS
            pTxBlk->pMacEntry = pMacEntry;/*hook pMacEntry for fill txp.*/
#endif
            TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
#ifdef MAC_REPEATER_SUPPORT
            if (pMacEntry->bReptCli)
                pMacEntry->ReptCliIdleCount = 0;
#endif
        }
#endif /* APCLI_SUPPORT */

		RTMP_OffloadFillTxBlkInfo(pAd, pTxBlk);

		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

		if ((!pAd->chipCap.BATriggerOffload) &&
					(pTxBlk->TxFrameType == TX_OFFLOAD_FRAME))
		{
			tr_entry = &pAd->MacTab.tr_entry[Wcid];
			RTMP_BASetup(pAd, tr_entry, FPTxElement->UserPriority);
		}

		RTMP_SPIN_LOCK(&pAd->irq_lock);
		Ret = CutThroughPktTx(pAd, pTxBlk);
		RTMP_SPIN_UNLOCK(&pAd->irq_lock);
#ifdef CONFIG_AP_SUPPORT
		if (!Ret) {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[Wcid])) {
				if (pAd->MacTab.Content[Wcid].pMbss) {
					pAd->MacTab.Content[Wcid].pMbss->TxCount++;
					pAd->MacTab.Content[Wcid].pMbss->TransmittedByteCount += RTPKT_TO_OSPKT(pTxBlk->pPacket)->len;
				}
			}
#ifdef APCLI_SUPPORT
			else if (IS_ENTRY_APCLI(&pAd->MacTab.Content[Wcid]) || 
			IS_ENTRY_REPEATER(&pAd->MacTab.Content[Wcid])) {
				if (pAd->MacTab.Content[Wcid].pApCli) {
					((APCLI_STRUCT*)(pAd->MacTab.Content[Wcid].pApCli))->TxCount++;
					((APCLI_STRUCT*)(pAd->MacTab.Content[Wcid].pApCli))->TransmittedByteCount += RTPKT_TO_OSPKT(pTxBlk->pPacket)->len;
				}
			}
#endif
		} else {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[Wcid])) {
				if (pAd->MacTab.Content[Wcid].pMbss) {
					pAd->MacTab.Content[Wcid].pMbss->TxDropCount++;
					pAd->MacTab.Content[Wcid].pMbss->TxErrorCount++;
				}
#ifdef APCLI_SUPPORT
				else if (IS_ENTRY_APCLI(&pAd->MacTab.Content[Wcid]) ||
				IS_ENTRY_REPEATER(&pAd->MacTab.Content[Wcid])) {
					if (pAd->MacTab.Content[Wcid].pApCli) {
						((APCLI_STRUCT*)(pAd->MacTab.Content[Wcid].pApCli))->TxDropCount++;
						((APCLI_STRUCT*)(pAd->MacTab.Content[Wcid].pApCli))->TxErrorCount++;
					}
				}
#endif
			}
		}
#endif
		if (pTxBlk->QueIdx == 0 && !KickRing0)
			KickRing0 = TRUE;
		else if (pTxBlk->QueIdx == 1 && !KickRing1)
			KickRing1 = TRUE;

		if (!Ret)
		{
			FPTxElement->pPacket = NULL;
			RTMP_SPIN_LOCK(&pAd->FastPathTxFreeQueLock);
			DlListAddTail(&pAd->FastPathTxFreeQue, &FPTxElement->List);
			pAd->FPTxElementFreeNum++;
			RTMP_SPIN_UNLOCK(&pAd->FastPathTxFreeQueLock);
		}
		else
		if(pktTokenCb->tx_id_list.token_inited==true)
		{
			if (pTxBlk->TxFrameType == TX_OFFLOAD_FRAME)
			{
				RTMP_SPIN_LOCK(&pAd->FastPathTxQueLock);
				pAd->FastPathTxQueNum++;
				DlListAdd(&pAd->FastPathTxQue, &FPTxElement->List);
				RTMP_SPIN_UNLOCK(&pAd->FastPathTxQueLock);
			}
			else if ((pTxBlk->TxFrameType == TX_MCAST_FRAME)
						|| (pTxBlk->TxFrameType == TX_LEGACY_FRAME))
			{
				RTMP_SPIN_LOCK(&pAd->MgmtQueLock);
				pAd->MgmtQueNum++;
				DlListAdd(&pAd->MgmtQue, &FPTxElement->List);
				RTMP_SPIN_UNLOCK(&pAd->MgmtQueLock);
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown TxFrameType = %d\n", __FUNCTION__, pTxBlk->TxFrameType));
			}

			break;
		}

#ifdef CONFIG_TRACE_SUPPORT
		if (pTxBlk->QueIdx == 0)
		{
			if (pTxBlk->TxFrameType == TX_OFFLOAD_FRAME)
				TRACE_TX0_DATA("tx0_data");
			else
				TRACE_TX0_MGMT("tx0 mgmt");
		}
		else if (pTxBlk->QueIdx == 1)
		{

			if (pTxBlk->TxFrameType == TX_OFFLOAD_FRAME)
				TRACE_TX1_DATA("tx1_data");
			else
				TRACE_TX1_MGMT("tx1 mgmt");
		}
#endif
	}

	RTMP_SPIN_LOCK(&pAd->irq_lock);

	if (KickRing0)
	{
		HAL_KickOutTx(pAd, pTxBlk, 0);
	}

	if (KickRing1)
	{
		HAL_KickOutTx(pAd, pTxBlk, 1);
	}
	
	RTMP_SPIN_UNLOCK(&pAd->irq_lock);

#if (CFG_CPU_LOADING_REDUCE_TXELEM_FULL == 1)
    if (pAd->fp_txBlocked &&
        pAd->FPTxElementFreeNum > FP_TX_FREE_UPPER_BOUND) {
        cut_through_tx_flow_block(pAd->PktTokenCb, NULL, NO_ENOUGH_FREE_TX_ELEM, FALSE, pTxBlk->QueIdx);
        pAd->fp_txBlocked = FALSE;
    }
#endif

	if (pAd->bFastPathTaskSchedulable
		&& (pAd->FastPathTxQueNum > 0 || pAd->MgmtQueNum > 0))
	{
		RTMP_NET_TASK_DATA_ASSIGN(&pAd->FastPathDequeTask, (ULONG)(pAd));
		RTMP_OS_TASKLET_SCHE(&pAd->FastPathDequeTask);
	}

#ifdef CONFIG_TX_DELAY
	pAd->force_deq = FALSE;
#endif
}
#else
INT32 FullOffloadFrameTx(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, UCHAR QueIdx, UCHAR UserPriority)
{
	TX_BLK tx_blk, *pTxBlk;
	UCHAR wcid = RTMP_GET_PACKET_WCID(pPacket);
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
	INT32 Ret = 0;
	ULONG IrqFlags;

	if (GET_OS_PKT_LEN(pPacket) <= LENGTH_802_3) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,("%s: 802.3 Length too small in FastPathTxFreeQue\n", __FUNCTION__));
		return 0;
	}
	
	NdisZeroMemory((UCHAR *)&tx_blk, sizeof(TX_BLK));

	pTxBlk = &tx_blk;
	pTxBlk->TxFrameType = TX_OFFLOAD_FRAME;
	pTxBlk->TotalFrameNum = 1;

	pTxBlk->TotalFrameNum = 1;
	pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(pPacket);
	pTxBlk->pPacket = pPacket;
	pTxBlk->QueIdx = QueIdx;

#ifdef VENDOR_FEATURE1_SUPPORT
	pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /*  VENDOR_FEATURE1_SUPPORT */

	InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));

	if (!RTMP_OffloadFillTxBlkInfo(pAd, pTxBlk))
	{
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("<--%s(%d): ##########Fail#########\n", __FUNCTION__, __LINE__));
		return FALSE;
	}

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (!pAd->chipCap.BATriggerOffload)
	{
		RTMP_BASetup(pAd, tr_entry, UserPriority);
	}

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	Ret = CutThroughPktTx(pAd, pTxBlk);
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	return Ret;
}
#endif
#endif /* CUT_THROUGH_FULL_OFFLOAD */

#endif /* CUT_THROUGH */
#endif /* MT_MAC */


VOID rtmp_hif_data_init(RTMP_ADAPTER *pAd)
{
	return ;
}


NDIS_STATUS RTMPFreeHifAdapterBlock(RTMP_ADAPTER *pAd)
{
	INT index;

	for (index = 0; index < NUM_OF_RX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->RxRingLock[index]);
	}

	NdisFreeSpinLock(&pAd->McuCmdLock);
	NdisFreeSpinLock(&pAd->LockInterrupt);
#ifdef CONFIG_ANDES_SUPPORT
	NdisFreeSpinLock(&pAd->CtrlRingLock);
#if defined(MT7615) || defined(MT7622)
	NdisFreeSpinLock(&pAd->FwDwloRing.RingLock);
#endif /* defined(MT7615) || defined(MT7622) */
#endif
	NdisFreeSpinLock(&pAd->tssi_lock);


	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS RTMPInitHifAdapterBlock(RTMP_ADAPTER *pAd)
{
	INT index;
	for (index = 0; index < NUM_OF_RX_RING; index++)
	{
		NdisAllocateSpinLock(pAd, &pAd->RxRingLock[index]);
	}

#ifdef CONFIG_ANDES_SUPPORT
	NdisAllocateSpinLock(pAd, &pAd->CtrlRingLock);
#if defined(MT7615) || defined(MT7622)
	NdisAllocateSpinLock(pAd, &pAd->FwDwloRing.RingLock);
	pAd->FwDwloRing.ring_size = CTL_RING_SIZE;
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

	NdisAllocateSpinLock(pAd, &pAd->tssi_lock);
	NdisAllocateSpinLock(pAd, &pAd->LockInterrupt);

	return NDIS_STATUS_SUCCESS;
}

