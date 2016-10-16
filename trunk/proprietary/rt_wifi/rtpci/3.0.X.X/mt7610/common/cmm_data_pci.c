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


#ifdef DBG
VOID dump_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
	DBGPRINT(RT_DEBUG_OFF, ("TxD:\n"));
	
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr0=0x%x\n", pTxD->SDPtr0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen0=0x%x\n", pTxD->SDLen0));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec0=0x%x\n", pTxD->LastSec0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr1=0x%x\n", pTxD->SDPtr1));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen1=0x%x\n", pTxD->SDLen1));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec1=0x%x\n", pTxD->LastSec1));
	DBGPRINT(RT_DEBUG_OFF, ("\tDMADONE=0x%x\n", pTxD->DMADONE));
	DBGPRINT(RT_DEBUG_OFF, ("\tBurst=0x%x\n", pTxD->Burst));
}


VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	DBGPRINT(RT_DEBUG_OFF, ("RxD:\n"));
	
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr0=0x%x\n", pRxD->SDP0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen0=0x%x\n", pRxD->SDL0));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec0=0x%x\n", pRxD->LS0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr1=0x%x\n", pRxD->SDP1));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen1=0x%x\n", pRxD->SDL1));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec1=0x%x\n", pRxD->LS1));
	DBGPRINT(RT_DEBUG_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}


VOID dumpTxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
	
	int index;
	
	ASSERT(ring_idx < NUM_OF_TX_RING);
	pDescRing = (RTMP_DMABUF *)pAd->TxDescRing[ring_idx].AllocVa;
	
	pTxRing = &pAd->TxRing[ring_idx];
	for (index = 0; index < TX_RING_SIZE; index++)
	{
		pTxD = (TXD_STRUC *)pTxRing->Cell[index].AllocVa;
		hex_dump("Dump TxDesc", (UCHAR *)pTxD, sizeof(TXD_STRUC));
		dump_txd(pAd, pTxD);
		pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[index].AllocVa + sizeof(TXD_STRUC));
		hex_dump("Dump TxInfo", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
		dump_txinfo(pAd, pTxInfo);
	}				
}


VOID dumpRxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	RTMP_RX_RING *pRxRing;
	RXD_STRUC *pRxD;
	int index;


	pDescRing = (RTMP_DMABUF *)pAd->RxDescRing.AllocVa;
	
	pRxRing = &pAd->RxRing;
	for (index = 0; index < RX_RING_SIZE; index++)
	{
		pRxD = (RXD_STRUC *)pRxRing->Cell[index].AllocVa;
		hex_dump("Dump RxDesc", (UCHAR *)pRxD, sizeof(TXD_STRUC));
		dump_rxd(pAd, pRxD);
	}				
}
#endif /* DBG */

static VOID ral_write_txinfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN TXINFO_STRUC *pTxInfo,
	IN BOOLEAN bWiv,
	IN UCHAR QueueSel)
{
#ifdef RLT_MAC
	struct _TXINFO_NMAC_PKT *nmac_info;

	nmac_info = (struct _TXINFO_NMAC_PKT *)pTxInfo;
#ifdef HDR_TRANS_SUPPORT
	if (pTxBlk && pTxBlk->NeedTrans)
		nmac_info->pkt_80211 = 0;	/* 802.3 MAC header */
	else
#endif /* HDR_TRANS_SUPPORT */
	nmac_info->pkt_80211 = 1;

	nmac_info->info_type = 0;
	nmac_info->d_port = 0;
	nmac_info->cso = 0;
	nmac_info->tso = 0;
#endif /* RLT_MAC */

	pTxInfo->TxInfoWIV = (bWiv) ? 1: 0;
	pTxInfo->TxInfoQSEL = (pAd->bGenOneHCCA == TRUE) ? FIFO_HCCA : QueueSel;


}



/*
	========================================================================
	
	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
		
	Arguments:
		pTxD		Pointer to transmit descriptor
		Ack 		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs 		Setting for IFS gap
		Rate		Setting for transmit rate
		Service 	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003
		
	Return Value:
		None
		
	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID ral_write_txd(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN TXD_STRUC *pTxD,
	IN TXINFO_STRUC *pTxInfo,
	IN BOOLEAN bWIV,
	IN UCHAR QueueSEL)
{
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	ral_write_txinfo(pAd, pTxBlk, pTxInfo, bWIV, QueueSEL);
	
	pTxD->DMADONE = 0;
}

/* IRQL = DISPATCH_LEVEL */
VOID ComposeNullFrame(
	IN PRTMP_ADAPTER pAd)
{
	NdisZeroMemory(&pAd->NullFrame, sizeof (HEADER_802_11));
	pAd->NullFrame.FC.Type = BTYPE_DATA;
	pAd->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
	pAd->NullFrame.FC.ToDs = 1;
	COPY_MAC_ADDR(pAd->NullFrame.Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->NullFrame.Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pAd->NullFrame.Addr3, pAd->CommonCfg.Bssid);
}


USHORT RtmpPCI_WriteSingleTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN BOOLEAN bIsLast,
	OUT USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
	TXWI_STRUC *pTxWI;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT8 *temp;
#endif
	UINT32 BufBasePaLow;
	PRTMP_TX_RING pTxRing;
	USHORT hwHeaderLen, inf_hdr_len;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;

	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/
	/*hwHeaderLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
	hwHeaderLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen + TSO_SIZE;
	inf_hdr_len = TXWISize + hwHeaderLen;
	NdisMoveMemory(pDMAHeaderBufVA,
					(UCHAR *)(pTxBlk->HeaderBuf + TXINFO_SIZE),
					inf_hdr_len);
	pTxWI = (TXWI_STRUC *)pDMAHeaderBufVA;

	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif
	//pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
	//NdisZeroMemory(pTxD, TXD_SIZE);

	pTxD->Burst = 0;
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = inf_hdr_len;	/* include padding*/
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;

	ral_write_txd(pAd, pTxBlk, pTxD, pTxInfo, FALSE, FIFO_EDCA);

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	DBGPRINT(RT_DEBUG_OFF, ("%s():pTxD->SDPtr0=0x%x, SrcBufPtr=0x%p!\n", __FUNCTION__, pTxD->SDPtr0, pTxBlk->pSrcBufData));
	
	hex_dump("DataTxD", pTxRing->Cell[TxIdx].AllocVa, TXD_SIZE);
	dump_txd(pAd, (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa);

	hex_dump("DataTxInfo", pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC), sizeof(TXINFO_STRUC));
	dump_txinfo(pAd, (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC)));

	hex_dump("DataPktHdr", pDMAHeaderBufVA, inf_hdr_len);
	hex_dump("DataPktInfo", pTxBlk->pSrcBufData, pTxD->SDLen1);
}
//---Add by shiang for debug
#endif

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
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
	TXINFO_STRUC *pTxInfo;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;	
	USHORT hwHdrLen;
	UINT32 firstDMALen;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);		

	
	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	if (frameNum == 0)
	{
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/
		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-LENGTH_AMSDU_SUBFRAMEHEAD, 4)+LENGTH_AMSDU_SUBFRAMEHEAD;*/
			hwHdrLen = pTxBlk->MpduHeaderLen - LENGTH_AMSDU_SUBFRAMEHEAD + pTxBlk->HdrPadLen + LENGTH_AMSDU_SUBFRAMEHEAD;
		else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
			/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-LENGTH_ARALINK_HEADER_FIELD, 4)+LENGTH_ARALINK_HEADER_FIELD;*/
			hwHdrLen = pTxBlk->MpduHeaderLen - LENGTH_ARALINK_HEADER_FIELD + pTxBlk->HdrPadLen + LENGTH_ARALINK_HEADER_FIELD;
		else
			/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
			hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		firstDMALen = TXWISize + hwHdrLen;
	}
	else
	{
		firstDMALen = pTxBlk->MpduHeaderLen;
	}

	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + TXINFO_SIZE, firstDMALen); 
		
	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	
	
	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif
	//pTxInfo = (TXINFO_STRUC *)((UCHAR *)pTxD + sizeof(TXD_STRUC));
	//NdisZeroMemory(pTxD, TXD_SIZE);

	pTxD->Burst = 0;
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;

	ral_write_txd(pAd, pTxBlk, pTxD, pTxInfo, FALSE, FIFO_EDCA);

#ifdef RT_BIG_ENDIAN
	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	
	if (frameNum != 0)
		RTMPWIEndianChange(pAd, (PUCHAR)pDMAHeaderBufVA, TYPE_TXWI);
	
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
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	USHORT			totalMPDUSize,
	IN	USHORT			FirstTxIdx)
{

	TXWI_STRUC *pTxWI;
	PRTMP_TX_RING	pTxRing;

	
	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	pTxWI = (TXWI_STRUC *) pTxRing->Cell[FirstTxIdx].DmaBuf.AllocVa;
	pTxWI->TxWIMPDUByteCnt = totalMPDUSize;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
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
	TXINFO_STRUC *pTxInfo;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
	USHORT hwHeaderLen;
	UINT32 firstDMALen;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* Get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	
	/* Copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/
	
	/*hwHeaderLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
	hwHeaderLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

	firstDMALen = TXWISize + hwHeaderLen;
	NdisMoveMemory(pDMAHeaderBufVA, (UCHAR *)(pTxBlk->HeaderBuf + TXINFO_SIZE), firstDMALen); 
		
	/* Build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif
	//pTxInfo = (TXINFO_STRUC *)((UCHAR *)pTxD + sizeof(TXD_STRUC));
	//NdisZeroMemory(pTxD, TXD_SIZE);	
	
	if (fragNum == pTxBlk->TotalFragNum)
	{
		pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	}
	
	pTxD->Burst = 0;
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	if (pTxD->SDLen1 > 0)
	{
		pTxD->LastSec0 = 0;
		pTxD->LastSec1 = 1;
	}
	else
	{
		pTxD->LastSec0 = 1;
		pTxD->LastSec1 = 0;
	}
	ral_write_txd(pAd, pTxBlk, pTxD, pTxInfo, FALSE, FIFO_EDCA);

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, pDMAHeaderBufVA, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);	
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;
	
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
	TXINFO_STRUC *pTxInfo;
	ULONG SwIdx = pAd->MgmtRing.TxCpuIdx;
	INT pkt_len = SrcBufLen - TXINFO_SIZE;

	//NdisZeroMemory(pAd->MgmtRing.Cell[SwIdx].AllocVa, TXD_SIZE);
#ifdef RT_BIG_ENDIAN
	pDestTxD = (PTXD_STRUC)pAd->MgmtRing.Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#else
	pTxD = (TXD_STRUC *)pAd->MgmtRing.Cell[SwIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pAd->MgmtRing.Cell[SwIdx].AllocVa + sizeof(TXD_STRUC));
#endif
	//pTxInfo = (TXINFO_STRUC *)(((UCHAR *)pTxD) + sizeof(TXD_STRUC));

	pAd->MgmtRing.Cell[SwIdx].pNdisPacket = pPacket;
	pAd->MgmtRing.Cell[SwIdx].pNextNdisPacket = NULL;

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = PCI_MAP_SINGLE(pAd, (pSrcBufVA + TXINFO_SIZE), pkt_len, 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen0 = pkt_len;

	ral_write_txd(pAd, NULL, pTxD, pTxInfo, TRUE, FIFO_MGMT);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((UCHAR *)pTxD, TYPE_TXD);
	WriteBackToDescriptor((UCHAR *)pDestTxD, (UCHAR *)pTxD, FALSE, TYPE_TXD);
#endif

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	TXWI_STRUC *pTxWI = (TXWI_STRUC *)(pSrcBufVA + TXINFO_SIZE);
	
	DBGPRINT(RT_DEBUG_OFF, ("%s():pTxD->SDPtr0=0x%x, SrcBufPtr=0x%p, SwIdx=%ld\n", __FUNCTION__,
							pTxD->SDPtr0, (UCHAR *)(pSrcBufVA + TXINFO_SIZE), SwIdx));

	hex_dump("MgmtTxDInfo", pAd->MgmtRing.Cell[SwIdx].AllocVa, TXD_SIZE);
	dump_txd(pAd, (TXD_STRUC *)pAd->MgmtRing.Cell[SwIdx].AllocVa);
	dump_txinfo(pAd, (TXINFO_STRUC *)(pAd->MgmtRing.Cell[SwIdx].AllocVa + sizeof(TXD_STRUC)));
	dumpTxWI(pAd, pTxWI);
	hex_dump("MgmtPktInfo", (pSrcBufVA + TXINFO_SIZE), pTxD->SDLen0);
}
//---Add by shiang for debug
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pSrcBufVA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pAd->MgmtRing.Cell[SwIdx].AllocPa, TXD_SIZE);

	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);

#ifdef RLT_MAC
	RTMP_IO_WRITE32(pAd, TX_MGMT_CIDX,  pAd->MgmtRing.TxCpuIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	RTMP_IO_WRITE32(pAd, TX_MGMTCTX_IDX,  pAd->MgmtRing.TxCpuIdx);
#endif /* RTMP_MAC */
	return 0;
}




BOOLEAN  RTMPFreeTXDUponTxDmaDone(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx)
{
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
#endif
	PNDIS_PACKET pPacket;
	UCHAR FREE = 0;
#ifdef RT_BIG_ENDIAN
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	BOOLEAN bReschedule = FALSE;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	ASSERT(QueIdx < NUM_OF_TX_RING);
	if (QueIdx >= NUM_OF_TX_RING)
		return FALSE;

	pTxRing = &pAd->TxRing[QueIdx];

#ifdef RLT_MAC
	RTMP_IO_READ32(pAd, TX_RING_DIDX + QueIdx * RINGREG_DIFF, &pTxRing->TxDmaIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF, &pTxRing->TxDmaIdx);
#endif /* RTMP_MAC */
	while (pTxRing->TxSwFreeIdx != pTxRing->TxDmaIdx)
	{
#ifdef RALINK_ATE
#ifdef RALINK_QA
		PHEADER_802_11	pHeader80211;

		if ((ATE_ON(pAd)) && (pAd->ate.bQATxStart == TRUE) && (pAd->ate.QID == QueIdx))
		{
			pAd->ate.TxDoneCount++;
			pAd->RalinkCounters.KickTxCount++;

			/* always use QID_AC_BE and FIFO_EDCA */
			ASSERT(pAd->ate.QID == 0);
			pAd->ate.TxAc0++;

			FREE++;
#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
#else
			pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
			pTxD = (PTXD_STRUC)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
/*			pTxD->DMADONE = 0; */

			pHeader80211 = (PHEADER_802_11)((UCHAR *)(pTxRing->Cell[pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + TXWISize);
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (PUCHAR)pHeader80211, DIR_READ, FALSE);
#endif
			pHeader80211->Sequence = ++pAd->ate.seq;
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (PUCHAR)pHeader80211, DIR_WRITE, FALSE);
#endif

			if  ((pAd->ate.bQATxStart == TRUE) && (pAd->ate.Mode & ATE_TXFRAME) && (pAd->ate.TxDoneCount < pAd->ate.TxCount))
			{
				pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
				pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
				pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);

				INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);

				/* get TX_DTX_IDX again */
#ifdef RLT_MAC
				RTMP_IO_READ32(pAd, TX_RING_DIDX + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
				RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);
#endif /* RTMP_MAC */
				goto kick_out;
			}
			else if ((pAd->ate.TxStatus == 1)/* or (pAd->ate.bQATxStart == TRUE) ??? */ && (pAd->ate.TxDoneCount == pAd->ate.TxCount))
			{
				DBGPRINT(RT_DEBUG_TRACE,("all Tx is done\n"));

				/* Tx status enters idle mode.*/
				pAd->ate.TxStatus = 0;
			}
			else if (!(pAd->ate.Mode & ATE_TXFRAME))
			{
				/* not complete sending yet, but someone press the Stop TX botton */
				DBGPRINT(RT_DEBUG_INFO,("not complete sending yet, but someone pressed the Stop TX bottom\n"));
				DBGPRINT(RT_DEBUG_INFO,("pAd->ate.Mode = 0x%02x\n", pAd->ate.Mode));
			}
			else
			{
				DBGPRINT(RT_DEBUG_OFF,("pTxRing->TxSwFreeIdx = %d\n", pTxRing->TxSwFreeIdx));
			}

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif /* RT_BIG_ENDIAN */

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);

			INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
			continue;
		}
#endif /* RALINK_QA */
#endif /* RALINK_ATE */

#ifdef VENDOR_FEATURE1_SUPPORT
		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateRx >= 4)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateRx = 0;
		}
#else
		NICUpdateFifoStaCounters(pAd);
#endif /* VENDOR_FEATURE1_SUPPORT */

		/* Note : If (pAd->ate.bQATxStart == TRUE), we will never reach here. */
		FREE++;
#ifndef RT_BIG_ENDIAN
		pTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
#else
		pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (PTXD_STRUC)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
/*		pTxD->DMADONE = 0; */

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_SP_PacketCheck(pAd,
				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket,
				((UCHAR *)pTxRing->Cell[pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + TXWISize);
		}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef RALINK_ATE
		/* Execution of this block is not allowed when ATE is running. */
		if (!(ATE_ON(pAd)))
#endif /* RALINK_ATE */
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
#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC && skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC && skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE ))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
		}

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);

		pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;
		INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
		/* get tx_tdx_idx again */
#ifdef RLT_MAC
		RTMP_IO_READ32(pAd, TX_RING_DIDX + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);
#endif /* RTMP_MAC */

#ifdef RT_BIG_ENDIAN
		//RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		//*pDestTxD = TxD;
		//NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif

#ifdef RALINK_ATE
#ifdef RALINK_QA
kick_out:
#endif /* RALINK_QA */

		/*
			ATE_TXCONT mode also need to send some normal frames, so let it in.
			ATE_STOP must be changed not to be 0xff
			to prevent it from running into this block.
		*/
		if ((pAd->ate.Mode & ATE_TXFRAME) && (pAd->ate.QID == QueIdx))
		{
			/* TxDoneCount++ has been done if QA is used.*/
			if (pAd->ate.bQATxStart == FALSE)
			{
				pAd->ate.TxDoneCount++;
			}
			if (((pAd->ate.TxCount - pAd->ate.TxDoneCount + 1) >= TX_RING_SIZE))
			{
				/* Note : We increase TxCpuIdx here, not TxSwFreeIdx ! */

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxCpuIdx].AllocPa, RXD_SIZE);

				INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);
#ifndef RT_BIG_ENDIAN
				pTxD = (PTXD_STRUC) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
#else
		        pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
				NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
				pTxD = (PTXD_STRUC)&tx_hw_info[0];
		        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
        		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxCpuIdx].AllocPa, RXD_SIZE);

				/* kick Tx-Ring*/
#ifdef RLT_MAC
				RTMP_IO_WRITE32(pAd, TX_RING_CIDX + QueIdx * RINGREG_DIFF, pAd->TxRing[QueIdx].TxCpuIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
				RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QueIdx * RINGREG_DIFF, pAd->TxRing[QueIdx].TxCpuIdx);
#endif /* RTMP_MAC */
				pAd->RalinkCounters.KickTxCount++;
			}
		}
#endif /* RALINK_ATE */
	}


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
	IN UINT32 int_source)
{
    unsigned long	IrqFlags;
	BOOLEAN			bReschedule = FALSE;
	INT_SOURCE_CSR_STRUC TxRingBitmap = (INT_SOURCE_CSR_STRUC)int_source;
	
	/* Make sure Tx ring resource won't be used by other threads*/
	/*NdisAcquireSpinLock(&pAd->TxRingLock);*/
	 
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

	if (TxRingBitmap.field.Ac0DmaDone)
		bReschedule = RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BE);

	if (TxRingBitmap.field.HccaDmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_HCCA);

	if (TxRingBitmap.field.Ac3DmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_VO);

	if (TxRingBitmap.field.Ac2DmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_VI);

	if (TxRingBitmap.field.Ac1DmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BK);

	/* Make sure to release Tx ring resource*/
	/*NdisReleaseSpinLock(&pAd->TxRingLock);*/
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	
	/* Dequeue outgoing frames from TxSwQueue[] and process it*/
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);

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
VOID	RTMPHandleMgmtRingDmaDoneInterrupt(
	IN	PRTMP_ADAPTER	pAd)
{
	PTXD_STRUC	 pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
/*	int 		 i;*/
	UCHAR	FREE = 0;
	PRTMP_MGMT_RING pMgmtRing = &pAd->MgmtRing;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	NdisAcquireSpinLock(&pAd->MgmtRingLock);
	
#ifdef RLT_MAC
	RTMP_IO_READ32(pAd, TX_MGMT_DIDX, &pMgmtRing->TxDmaIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	RTMP_IO_READ32(pAd, TX_MGMTDTX_IDX, &pMgmtRing->TxDmaIdx);
#endif /* RTMP_MAC */
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
        //TxD = *pDestTxD;
        //pTxD = &TxD;
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (PTXD_STRUC)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
#endif

		//pTxD->DMADONE = 0;
		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

#define LMR_FRAME_GET()	(GET_OS_PKT_DATAPTR(pPacket) + TXWISize)

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket, LMR_FRAME_GET());
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	       // RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	       //WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
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
VOID	RTMPHandleTBTTInterrupt(
	IN PRTMP_ADAPTER pAd)
{
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pAd->OpMode == OPMODE_AP)
	{
		ReSyncBeaconTime(pAd);

		RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);

#ifdef A_BAND_SUPPORT
		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			ChannelSwitchingCountDownProc(pAd);
		}
#endif
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
		}
	}
}


/*
	========================================================================

	Routine Description:
	Arguments:
		pAd 		Pointer to our adapter. Rewrite beacon content before next send-out.

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID	RTMPHandlePreTBTTInterrupt(
	IN PRTMP_ADAPTER pAd)
{
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef RT_CFG80211_SUPPORT
		RT_CFG80211_BEACON_TIM_UPDATE(pAd);
#else /* RT_CFG80211_SUPPORT */
		APUpdateAllBeaconFrame(pAd);
#endif /* !RT_CFG80211_SUPPORT */
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandlePreTBTTInterrupt...\n"));
		}
	}

}

VOID RTMPHandleRxCoherentInterrupt(
	IN	PRTMP_ADAPTER	pAd)
{
	WPDMA_GLO_CFG_STRUC GloCfg;

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("====> pAd is NULL, return.\n"));
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPHandleRxCoherentInterrupt \n"));
	
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG , &GloCfg.word);
	GloCfg.field.EnTXWriteBackDDONE = 0;
	GloCfg.field.EnableRxDMA = 0;
	GloCfg.field.EnableTxDMA = 0;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_HCCA);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	RTMPEnableRxTx(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPHandleRxCoherentInterrupt \n"));
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPHandleMcuInterrupt(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 McuIntSrc = 0;
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


PNDIS_PACKET RxRingDeQueue(
	IN RTMP_ADAPTER *pAd,
	OUT RX_BLK *pRxBlk,
	OUT BOOLEAN *pbReschedule,
	INOUT UINT32 *pRxPending,
	BOOLEAN *bCmdRspPacket,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
	RXINFO_STRUC *pRxInfo;	
	RXINFO_STRUC RxInfo;
#endif
	PNDIS_PACKET pRxPacket = NULL, pNewPacket;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell;
	NDIS_SPIN_LOCK *pRxRingLock = NULL;
	RTMP_RX_RING *pRxRing = NULL;

#ifdef RLT_MAC
	if (RxRingNo == 1)
	{
		pRxRingLock = &pAd->RxRing1Lock;
		pRxRing = &pAd->RxRing1;
	}
	else
#endif /* RLT_MAC */
	{
		pRxRingLock = &pAd->RxRingLock;
		pRxRing = &pAd->RxRing;
	}

	RTMP_SEM_LOCK(pRxRingLock);

	if (*pRxPending == 0)
	{
		/* Get how may packets had been received*/
#ifdef RLT_MAC
		if (RxRingNo == 1)
		{
			RTMP_IO_READ32(pAd, RX_RING1_DIDX, &pRxRing->RxDmaIdx);
		}
		else
		{
			RTMP_IO_READ32(pAd, RX_RING_DIDX, &pRxRing->RxDmaIdx);
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		RTMP_IO_READ32(pAd, RX_DRX_IDX, &pRxRing->RxDmaIdx);
#endif /* RTMP_MAC */
		if (pRxRing->RxSwReadIdx == pRxRing->RxDmaIdx)
		{
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count*/
		if (pRxRing->RxDmaIdx > pRxRing->RxSwReadIdx)
			*pRxPending = pRxRing->RxDmaIdx - pRxRing->RxSwReadIdx;
		else
			*pRxPending	= pRxRing->RxDmaIdx + RX_RING_SIZE - pRxRing->RxSwReadIdx;
	}

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
	/* Point to Rx indexed rx ring descriptor*/
	pRxD = (PRXD_STRUC) pRxCell->AllocVa;
#endif

	if (pRxD->DDONE == 0)
	{
		*pRxPending = 0;
		/* DMAIndx had done but DDONE bit not ready*/
		bReschedule = TRUE;
		goto done;
	}

	/* return rx descriptor*/
	NdisMoveMemory(&pRxBlk->hw_rx_info[0], pRxD, RXD_SIZE);


#ifdef WLAN_SKB_RECYCLE
	{
		struct sk_buff *skb;
		
		skb = __skb_dequeue_tail(&pAd->rx0_recycle);
		if (unlikely(skb==NULL))
		{
			pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, 
													((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, 
													RX_BUFFER_AGGRESIZE, 
													FALSE, 
													&pRxCell->DmaBuf.AllocVa, 
													&pRxCell->DmaBuf.AllocPa);
		}
		else
		{
			pNewPacket = OSPKT_TO_RTPKT(skb);
			pRxCell->DmaBuf.AllocVa = GET_OS_PKT_DATAPTR(pNewPacket);
			pRxCell->DmaBuf.AllocPa = PCI_MAP_SINGLE_DEV(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pRxCell->DmaBuf.AllocVa, RX_BUFFER_AGGRESIZE,  -1, RTMP_PCI_DMA_FROMDEVICE);
		}
	}
#else
	pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, RX_BUFFER_AGGRESIZE, FALSE, &pRxCell->DmaBuf.AllocVa, &pRxCell->DmaBuf.AllocPa);
#endif /* WLAN_SKB_RECYCLE */

	if (pNewPacket)
	{
		/* unmap the rx buffer*/
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
					 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		pRxPacket = pRxCell->pNdisPacket;
#ifdef RTMP_MAC
		pRxBlk->pRxInfo = &pRxBlk->hw_rx_info[RXINFO_OFFSET];
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		pRxBlk->pRxFceInfo = (RXFCE_INFO *)&pRxBlk->hw_rx_info[RXINFO_OFFSET];
		pRxBlk->pRxInfo = (RXINFO_STRUC *)GET_OS_PKT_DATAPTR(pRxPacket);
#ifdef RT_BIG_ENDIAN
		NdisMoveMemory(&RxInfo, pRxBlk->pRxInfo, RXINFO_SIZE);
		pRxInfo = &RxInfo;
		//RTMPDescriptorEndianChange((PUCHAR)pRxInfo, TYPE_RXINFO);
		(*(UINT32*)pRxInfo) = le2cpu32(*(UINT32*)pRxInfo);
		NdisMoveMemory(pRxBlk->pRxInfo, pRxInfo, RXINFO_SIZE);
#endif /* RT_BIG_ENDIAN */
		SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + RXINFO_SIZE);
		SET_OS_PKT_LEN(pRxPacket, GET_OS_PKT_LEN(pRxPacket) - RXINFO_SIZE);
#endif /* RLT_MAC */

		pRxCell->DmaBuf.AllocSize = RX_BUFFER_AGGRESIZE;
		pRxCell->pNdisPacket = (PNDIS_PACKET) pNewPacket;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		/* update SDP0 to new buffer of rx packet */
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;

#ifdef RX_DMA_SCATTER
		pRxD->SDL0 = RX_BUFFER_AGGRESIZE;
#endif /* RX_DMA_SCATTER */
	}
	else 
	{
		pRxPacket = NULL;
		bReschedule = TRUE;
	}

	*pRxPending = *pRxPending - 1;	

#ifndef CACHE_LINE_32B
	pRxD->DDONE = 0;
	/* update rx descriptor and kick rx */
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);

	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
#ifdef RLT_MAC
	if (RxRingNo == 1)
	{
		RTMP_IO_WRITE32(pAd, RX_RING1_CIDX, pRxRing->RxCpuIdx);
	}
	else
	{
		RTMP_IO_WRITE32(pAd, RX_RING_CIDX, pRxRing->RxCpuIdx);
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pRxRing->RxCpuIdx);
#endif /* RTMP_MAC */
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
		PRXD_STRUC pDestRxDLast;
#endif
		/* 16B-align */

		/* update last BD 32B-align, DMA Done bit = 0 */
		pRxRing->Cell[pRxRing->RxSwReadIdx].LastBDInfo.DDONE = 0;
#ifdef RT_BIG_ENDIAN
		pRxCellLast = &pRxRing->Cell[pRxRing->RxSwReadIdx - 1];
		pDestRxDLast = (PRXD_STRUC) pRxCellLast->AllocVa;
		RTMPDescriptorEndianChange((PUCHAR)&pRxRing->Cell[pRxRing->RxSwReadIdx].LastBDInfo, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxDLast, (PUCHAR)&pRxRing->Cell[pRxRing->RxSwReadIdx].LastBDInfo, FALSE, TYPE_RXD);
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
		INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);
		pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
#ifdef RLT_MAC
		if (RxRingNo == 1)
		{
			RTMP_IO_WRITE32(pAd, RX_RING1_CIDX, pRxRing->RxCpuIdx);
		}
		else
		{
			RTMP_IO_WRITE32(pAd, RX_RING_CIDX, pRxRing->RxCpuIdx);
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pRxRing->RxCpuIdx);
#endif /* RTMP_MAC */
	}
	else
	{
		/* 32B-align */
		/* do not set DDONE bit and backup it */
		if (pRxRing->RxSwReadIdx >= (RX_RING_SIZE-1))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("Please change RX_RING_SIZE to mutiple of 2!\n"));

			/* flush cache from current BD */
			RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);

			/* update SW read and CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);
			pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
#ifdef RLT_MAC
			if (RxRingNo == 1)
			{
				RTMP_IO_WRITE32(pAd, RX_RING1_CIDX, pRxRing->RxCpuIdx);
			}
			else
			{
				RTMP_IO_WRITE32(pAd, RX_RING_CIDX, pRxRing->RxCpuIdx);
			}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
			RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pRxRing->RxCpuIdx);
#endif /* RTMP_MAC */
		}
		else
		{
			/* backup current BD */
			pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx + 1];
			pRxCell->LastBDInfo = *pRxD;

			/* update CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);
		}
	}
#endif /* CACHE_LINE_32B */

done:
	RTMP_SEM_UNLOCK(pRxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;
}


PNDIS_PACKET GetPacketFromRxRing(
	IN RTMP_ADAPTER *pAd,
	OUT RX_BLK *pRxBlk,
	OUT BOOLEAN *pbReschedule,
	INOUT UINT32 *pRxPending,
	BOOLEAN *bCmdRspPacket)
{
	/*
		Get packet from RxRing0.
	*/
	return RxRingDeQueue(pAd, pRxBlk, pbReschedule, pRxPending, bCmdRspPacket, 0);
}


NDIS_STATUS MlmeHardTransmitTxRing(RTMP_ADAPTER *pAd, UCHAR QueIdx, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	BOOLEAN bAckRequired, bInsertTimestamp;
	ULONG SrcBufPA;
	UCHAR MlmeRate;
	ULONG SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;
	TXWI_STRUC *pFirstTxWI;
	ULONG FreeNum;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

	FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
	if (FreeNum == 0)
		return NDIS_STATUS_FAILURE;

	SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;

#ifdef RT_BIG_ENDIAN
	pDestTxD  = (PTXD_STRUC)pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
	NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&hw_hdr_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD  = (PTXD_STRUC) pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
#endif
	pTxInfo = (TXINFO_STRUC *)((UCHAR *)pTxD + sizeof(TXD_STRUC));

	if (pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket)
	{
		DBGPRINT(RT_DEBUG_OFF, ("MlmeHardTransmit Error\n"));
		return NDIS_STATUS_FAILURE;
	}


	
	pFirstTxWI =(TXWI_STRUC *)(pSrcBufVA + TXINFO_SIZE);
	pHeader_802_11 = (PHEADER_802_11)(pSrcBufVA + TXINFO_SIZE + TXWISize + TSO_SIZE);
	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;
	
	if ((pHeader_802_11->FC.Type == BTYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

	/* Verify Mlme rate for a / g bands.*/
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/
	
	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */
	
	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == BTYPE_CNTL) /* must be PS-POLL*/
		bAckRequired = FALSE;
	else /* BTYPE_MGMT or BTYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST*/
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
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/* Before radar detection done, mgmt frame can not be sent but probe req*/
	/* Because we need to use probe req to trigger driver to send probe req in passive scan*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->Dot11_H.RDMode != RD_NORMAL_MODE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
		return (NDIS_STATUS_FAILURE);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif
	
	/*
		Fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one ohysical buffer, and the whole frame size equals
		to the first scatter buffer size
	*/
	

	/*
		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/
/*	pAd->CommonCfg.MlmeTransmit.field.MODE = 1;*/
	
/* management frame doesn't need encryption. so use RESERVED_WCID no matter u are sending to specific wcid or not */
	/* Only beacon use Nseq=TRUE. So here we use Nseq=FALSE.*/
	if (pMacEntry == NULL)
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
						0, RESERVED_WCID, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE), PID_MGMT, 0,
						(UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE,
						&pAd->CommonCfg.MlmeTransmit);
	}
	else
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE,
					bInsertTimestamp, FALSE, bAckRequired, FALSE,
					0, pMacEntry->Aid, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE),
					pMacEntry->MaxHTPhyMode.field.MCS, 0,
					(UCHAR)pMacEntry->MaxHTPhyMode.field.MCS,
					IFS_BACKOFF, FALSE, &pMacEntry->MaxHTPhyMode);
	}

	pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket = pPacket;
	pAd->TxRing[QueIdx].Cell[SwIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pFirstTxWI, TYPE_TXWI);
#endif
	SrcBufPA = PCI_MAP_SINGLE(pAd, (pSrcBufVA + TXINFO_SIZE), (SrcBufLen - TXINFO_SIZE), 0, RTMP_PCI_DMA_TODEVICE);

	ral_write_txd(pAd, NULL, pTxD, pTxInfo, TRUE, FIFO_EDCA);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = (SrcBufLen - TXINFO_SIZE);
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = SrcBufPA;
	pTxD->DMADONE = 0;

#ifdef DBG
//+++Add by shiang for debug
if (0){
	DBGPRINT(RT_DEBUG_OFF, ("%s():pTxD->SDPtr0=0x%x, SrcBufPtr=0x%p, TxRing=%d, SwIdx=%ld\n", __FUNCTION__,
							pTxD->SDPtr0, (UCHAR *)(pSrcBufVA + TXINFO_SIZE), QueIdx, SwIdx));

	hex_dump("MgmtTxDInfo", pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa, TXD_SIZE);
	dump_txd(pAd, (TXD_STRUC *)pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa);
	dump_txinfo(pAd, (TXINFO_STRUC *)(pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa + sizeof(TXD_STRUC)));
	hex_dump("MgmtPktInfo", (pSrcBufVA + TXINFO_SIZE), pTxD->SDLen0);
}
//---Add by shiang for debug
#endif

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pAd->TxRing[QueIdx].Cell[SwIdx].AllocPa, TXD_SIZE);

   	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);

#ifdef RLT_MAC
	RTMP_IO_WRITE32(pAd, TX_RING_CIDX + QueIdx*0x10,  pAd->TxRing[QueIdx].TxCpuIdx);
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QueIdx*0x10,  pAd->TxRing[QueIdx].TxCpuIdx);
#endif /* RTMP_MAC */

	return NDIS_STATUS_SUCCESS;
}

#ifdef RLT_MAC
BOOLEAN RxRing1DoneInterruptHandle(RTMP_ADAPTER *pAd) 
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	RXD_STRUC *pRxD = NULL;
	RXINFO_STRUC *pRxInfo = NULL;
	UCHAR *pData = NULL;
	RXWI_STRUC *pRxWI = NULL;
	PNDIS_PACKET pRxPacket = NULL;
	PHEADER_802_11 pHeader = NULL;
	RX_BLK rxblk, *pRxBlk = NULL;
	UINT8 RXWISize = pAd->chipCap.RXWISize;
	RXFCE_INFO *pFceInfo;
	BOOLEAN bCmdRspPacket = FALSE;

#ifdef LINUX
#endif /* LINUX */

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1)
	{

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_HALT_IN_PROGRESS)) || 
			!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			break;
		}

#ifdef RTMP_MAC_PCI
		//if (RxProcessed++ > MAX_RX_PROCESS_CNT)
		if (RxProcessed++ > 32)
		{
			bReschedule = TRUE;
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

		pRxBlk = &rxblk;
		
		pRxPacket = RxRingDeQueue(pAd, pRxBlk, &bReschedule, &RxPending, &bCmdRspPacket, 1);

		if ((pRxPacket == NULL) && !bCmdRspPacket)
			break;

		if ((pRxPacket == NULL) && bCmdRspPacket)
		{
			INC_RING_INDEX(pAd->RxRing1.RxSwReadIdx, RX_RING_SIZE);
			continue;
		}

		/* get rx descriptor and data buffer */
		pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];
		pFceInfo = rxblk.pRxFceInfo;
		pRxInfo = rxblk.pRxInfo;
		pData = GET_OS_PKT_DATAPTR(pRxPacket);
		pRxWI = (RXWI_STRUC *)pData;
		pHeader = (PHEADER_802_11)(pData + RXWISize);

		DBGPRINT(RT_DEBUG_TRACE,("%s: 1.\n", __FUNCTION__));
		
		if (pFceInfo->info_type == CMD_PACKET)
		{
			DBGPRINT(RT_DEBUG_TRACE,("%s: Receive command packet.\n", __FUNCTION__));
			CmdRspEventCallbackHandle(pAd, (PUCHAR)pFceInfo);
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
		
	}
	return bReschedule;
}

VOID	RTMPHandleTxRing8DmaDoneInterrupt(
	IN RTMP_ADAPTER *pAd)
{
	PTXD_STRUC	 pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	UCHAR	FREE = 0;
	RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;

	NdisAcquireSpinLock(&pAd->CtrlRingLock);
	
	RTMP_IO_READ32(pAd, TX_CTRL_DIDX, &pCtrlRing->TxDmaIdx);
	while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (PTXD_STRUC) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif

		//pTxD->DMADONE = 0;
		pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}
	NdisReleaseSpinLock(&pAd->CtrlRingLock);
}

#endif /* RLT_MAC */

