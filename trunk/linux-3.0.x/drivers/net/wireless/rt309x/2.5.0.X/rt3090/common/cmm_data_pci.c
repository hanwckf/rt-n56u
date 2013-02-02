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
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
/*
   All functions in this file must be PCI-depended, or you should out your function
	in other files.

*/
#include	"rt_config.h"


USHORT RtmpPCI_WriteSingleTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber)
{

	UCHAR			*pDMAHeaderBufVA;
	USHORT			TxIdx, RetTxIdx;
	PTXD_STRUC		pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	UINT32			BufBasePaLow;
	PRTMP_TX_RING	pTxRing;	
	USHORT			hwHeaderLen;

	//
	// get Tx Ring Resource
	// 
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	// copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer
	//hwHeaderLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);
	hwHeaderLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf, TXINFO_SIZE + TXWI_SIZE + hwHeaderLen);

	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

	//
	// build Tx Descriptor
	// 
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	TxD = *pDestTxD;
	pTxD = &TxD;
#endif
	NdisZeroMemory(pTxD, TXD_SIZE);

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = TXINFO_SIZE + TXWI_SIZE + hwHeaderLen; // include padding
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, PCI_DMA_TODEVICE);;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = 0;
	pTxD->LastSec1 = (bIsLast) ? 1 : 0;

	RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange((PUCHAR)(pDMAHeaderBufVA + TXINFO_SIZE), TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXINFO_SIZE + TXWI_SIZE), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);	
#endif // RT_BIG_ENDIAN //

	RetTxIdx = TxIdx;
	//
	// Update Tx index
	// 
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


USHORT RtmpPCI_WriteMultiTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			frameNum,
	OUT	USHORT			*FreeNumber)
{
	BOOLEAN bIsLast;
	UCHAR			*pDMAHeaderBufVA;
	USHORT			TxIdx, RetTxIdx;
	PTXD_STRUC		pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	UINT32			BufBasePaLow;
	PRTMP_TX_RING	pTxRing;	
	USHORT			hwHdrLen;
	UINT32			firstDMALen;

	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);		

	//
	// get Tx Ring Resource
	// 
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	if (frameNum == 0)
	{
		// copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer
		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			//hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-LENGTH_AMSDU_SUBFRAMEHEAD, 4)+LENGTH_AMSDU_SUBFRAMEHEAD;
			hwHdrLen = pTxBlk->MpduHeaderLen - LENGTH_AMSDU_SUBFRAMEHEAD + pTxBlk->HdrPadLen + LENGTH_AMSDU_SUBFRAMEHEAD;
		else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
			//hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-LENGTH_ARALINK_HEADER_FIELD, 4)+LENGTH_ARALINK_HEADER_FIELD;
			hwHdrLen = pTxBlk->MpduHeaderLen - LENGTH_ARALINK_HEADER_FIELD + pTxBlk->HdrPadLen + LENGTH_ARALINK_HEADER_FIELD;
		else
			//hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);
			hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		firstDMALen = TXINFO_SIZE + TXWI_SIZE + hwHdrLen;
	}
	else
	{
		firstDMALen = pTxBlk->MpduHeaderLen;
	}

	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf, firstDMALen); 
		
	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	
	//
	// build Tx Descriptor
	// 
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	TxD = *pDestTxD;
	pTxD = &TxD;
#endif
	NdisZeroMemory(pTxD, TXD_SIZE);

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; // include padding
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, PCI_DMA_TODEVICE);;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = 0;
	pTxD->LastSec1 = (bIsLast) ? 1 : 0;

	RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);

#ifdef RT_BIG_ENDIAN
	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA+ TXINFO_SIZE + TXWI_SIZE), DIR_WRITE, FALSE);
	
	if (frameNum != 0)
		RTMPWIEndianChange((PUCHAR)(pDMAHeaderBufVA + TXINFO_SIZE), TYPE_TXWI);
	
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //
	
	RetTxIdx = TxIdx;
	//
	// Update Tx index
	// 
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

	PTXWI_STRUC		pTxWI;
	PRTMP_TX_RING	pTxRing;

	//
	// get Tx Ring Resource
	// 
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	pTxWI = (PTXWI_STRUC) pTxRing->Cell[FirstTxIdx].DmaBuf.AllocVa;
	pTxWI->MPDUtotalByteCount = totalMPDUSize;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange((PUCHAR)pTxWI, TYPE_TXWI);
#endif // RT_BIG_ENDIAN //

}

USHORT	RtmpPCI_WriteFragTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			fragNum,
	OUT	USHORT			*FreeNumber)
{
	UCHAR			*pDMAHeaderBufVA;
	USHORT			TxIdx, RetTxIdx;
	PTXD_STRUC		pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	UINT32			BufBasePaLow;
	PRTMP_TX_RING	pTxRing;
	USHORT			hwHeaderLen;
	UINT32			firstDMALen;

	//
	// Get Tx Ring Resource
	// 
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	//
	// Copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer
	//
	//hwHeaderLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);
	hwHeaderLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

	firstDMALen = TXINFO_SIZE + TXWI_SIZE + hwHeaderLen;
	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf, firstDMALen); 
		

	//
	// Build Tx Descriptor
	// 
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	TxD = *pDestTxD;
	pTxD = &TxD;
#endif
	NdisZeroMemory(pTxD, TXD_SIZE);	
	
	if (fragNum == pTxBlk->TotalFragNum)
	{
		pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	}
	
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; // include padding
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, PCI_DMA_TODEVICE);
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = 0;
	pTxD->LastSec1 = 1;

	RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange((PUCHAR)(pDMAHeaderBufVA + TXINFO_SIZE), TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXINFO_SIZE + TXWI_SIZE), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
    WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);	
#endif // RT_BIG_ENDIAN //

	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;
	
	//
	// Update Tx index
	// 
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
	IN RTMP_ADAPTER 	*pAd, 
	IN UCHAR 			QueIdx,
	IN PNDIS_PACKET		pPacket,
	IN PUCHAR			pSrcBufVA,
	IN UINT 			SrcBufLen)
{
	PTXD_STRUC		pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	ULONG			SwIdx = pAd->MgmtRing.TxCpuIdx;
	
#ifdef RT_BIG_ENDIAN
    pDestTxD  = (PTXD_STRUC)pAd->MgmtRing.Cell[SwIdx].AllocVa;
    TxD = *pDestTxD;
    pTxD = &TxD;
    RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[SwIdx].AllocVa;
#endif

	pAd->MgmtRing.Cell[SwIdx].pNdisPacket = pPacket;
	pAd->MgmtRing.Cell[SwIdx].pNextNdisPacket = NULL;	
	
	RTMPWriteTxDescriptor(pAd, pTxD, TRUE, FIFO_MGMT);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 1;
	pTxD->DMADONE = 0;	
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = PCI_MAP_SINGLE(pAd, pSrcBufVA, SrcBufLen, 0, PCI_DMA_TODEVICE);
	pTxD->SDLen0 = SrcBufLen;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
    WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	
//==================================================================
/*	DBGPRINT_RAW(RT_DEBUG_TRACE, ("MLMEHardTransmit\n"));
	for (i = 0; i < (TXWI_SIZE+24); i++)
	{
	
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("%x:", *(pSrcBufVA+i)));
		if ( i%4 == 3)
			DBGPRINT_RAW(RT_DEBUG_TRACE, (" :: "));
		if ( i%16 == 15)
			DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n      "));
	}
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n      "));*/
//=======================================================================

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	// Increase TX_CTX_IDX, but write to register later.
	INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);

	RTMP_IO_WRITE32(pAd, TX_MGMTCTX_IDX,  pAd->MgmtRing.TxCpuIdx);

	return 0;
}




BOOLEAN  RTMPFreeTXDUponTxDmaDone(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			QueIdx)
{
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC	  pTxD;
#ifdef	RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
#endif
	PNDIS_PACKET  pPacket;
	UCHAR	FREE = 0;
	TXD_STRUC	TxD, *pOriTxD;
	//ULONG		IrqFlags;
	BOOLEAN			bReschedule = FALSE;


	ASSERT(QueIdx < NUM_OF_TX_RING);
	pTxRing = &pAd->TxRing[QueIdx];

	RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF, &pTxRing->TxDmaIdx);
	while (pTxRing->TxSwFreeIdx != pTxRing->TxDmaIdx)
	{
//		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#ifdef RALINK_ATE
#ifdef RALINK_QA
		PHEADER_802_11	pHeader80211;

		if ((ATE_ON(pAd)) && (pAd->ate.bQATxStart == TRUE))
		{
			if (pAd->ate.QID == QueIdx)
			{
				pAd->ate.TxDoneCount++;
				pAd->RalinkCounters.KickTxCount++;

				/* always use QID_AC_BE and FIFO_EDCA */
				ASSERT(pAd->ate.QID == 0);
				pAd->ate.TxAc0++;

				FREE++;
#ifndef RT_BIG_ENDIAN
				pTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
				pOriTxD = pTxD;
		        NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
				pTxD = &TxD;
#else
		        pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		        pOriTxD = pDestTxD ;
		        TxD = *pDestTxD;
		        pTxD = &TxD;
		        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;

				pHeader80211 = pTxRing->Cell[pTxRing->TxSwFreeIdx].DmaBuf.AllocVa + sizeof(TXWI_STRUC);
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
					INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);

					/* get TX_DTX_IDX again */
					RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);
					goto kick_out;
				}
				else if ((pAd->ate.TxStatus == 1)/* or (pAd->ate.bQATxStart == TRUE) ??? */ && (pAd->ate.TxDoneCount == pAd->ate.TxCount))
				{
					DBGPRINT(RT_DEBUG_TRACE,("all Tx is done\n"));

					// Tx status enters idle mode.
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

#ifndef RT_BIG_ENDIAN
	        	NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
#else
        		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
        		*pDestTxD = TxD;
#endif // RT_BIG_ENDIAN //

				INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
				continue;
			}
		}
#endif // RALINK_QA //
#endif // RALINK_ATE //

		// static rate also need NICUpdateFifoStaCounters() function.
		//if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
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
#endif // VENDOR_FEATURE1_SUPPORT //

		/* Note : If (pAd->ate.bQATxStart == TRUE), we will never reach here. */
		FREE++;
#ifndef RT_BIG_ENDIAN
                pTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		pOriTxD = pTxD;
                NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
		pTxD = &TxD;
#else
        pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
        pOriTxD = pDestTxD ;
        TxD = *pDestTxD;
        pTxD = &TxD;
        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		pTxD->DMADONE = 0;

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_SP_PacketCheck(pAd,
				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket,
				((UCHAR *)pTxRing->Cell[\
				pTxRing->TxSwFreeIdx].DmaBuf.AllocVa)+TXWI_SIZE);
		}
#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef RALINK_ATE
		/* Execution of this block is not allowed when ATE is running. */
		if (!(ATE_ON(pAd)))
#endif // RALINK_ATE //
		{
			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket;
			if (pPacket)
			{
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, PCI_DMA_TODEVICE);
				else
#endif // CONFIG_5VT_ENHANCE //
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			//Always assign pNdisPacket as NULL after clear
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, PCI_DMA_TODEVICE);
				else
#endif // CONFIG_5VT_ENHANCE //
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			//Always assign pNextNdisPacket as NULL after clear
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
		}

		pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;
		INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
		/* get tx_tdx_idx again */
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);
#ifdef RT_BIG_ENDIAN
        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
        *pDestTxD = TxD;
#else
        NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
#endif

#ifdef RALINK_ATE
#ifdef RALINK_QA
kick_out:
#endif // RALINK_QA //

		/*
			ATE_TXCONT mode also need to send some normal frames, so let it in.
			ATE_STOP must be changed not to be 0xff
			to prevent it from running into this block.
		*/
		if ((pAd->ate.Mode & ATE_TXFRAME) && (pAd->ate.QID == QueIdx))
		{
			// TxDoneCount++ has been done if QA is used.
			if (pAd->ate.bQATxStart == FALSE)
			{
				pAd->ate.TxDoneCount++;
			}
			if (((pAd->ate.TxCount - pAd->ate.TxDoneCount + 1) >= TX_RING_SIZE))
			{
				/* Note : We increase TxCpuIdx here, not TxSwFreeIdx ! */
				INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);
#ifndef RT_BIG_ENDIAN
				pTxD = (PTXD_STRUC) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
				pOriTxD = pTxD;
		        NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
				pTxD = &TxD;
#else
		        pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
		        pOriTxD = pDestTxD ;
		        TxD = *pDestTxD;
		        pTxD = &TxD;
		        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;
#ifndef RT_BIG_ENDIAN
        		NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
#else
        		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
        		*pDestTxD = TxD;
#endif
				// kick Tx-Ring
				RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QueIdx * RINGREG_DIFF, pAd->TxRing[QueIdx].TxCpuIdx);
				pAd->RalinkCounters.KickTxCount++;
			}
		}
#endif // RALINK_ATE //
//         RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
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
	IN	PRTMP_ADAPTER	pAd,
	IN	INT_SOURCE_CSR_STRUC TxRingBitmap)
{
//	UCHAR			Count = 0;
    unsigned long	IrqFlags;
	BOOLEAN			bReschedule = FALSE;
	
	// Make sure Tx ring resource won't be used by other threads
	//NdisAcquireSpinLock(&pAd->TxRingLock);
	 
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

	// Make sure to release Tx ring resource
	//NdisReleaseSpinLock(&pAd->TxRingLock);
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	
	// Dequeue outgoing frames from TxSwQueue[] and process it
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
    TXD_STRUC       TxD;
#endif
	PNDIS_PACKET pPacket;
//	int 		 i;
	UCHAR	FREE = 0;
	PRTMP_MGMT_RING pMgmtRing = &pAd->MgmtRing;

	NdisAcquireSpinLock(&pAd->MgmtRingLock);

	RTMP_IO_READ32(pAd, TX_MGMTDTX_IDX, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
        TxD = *pDestTxD;
        pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
#endif
		pTxD->DMADONE = 0;
		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket;

        if (pPacket == NULL)
            continue;

#ifdef CONFIG_AP_SUPPORT

#define LMR_FRAME_GET()	(GET_OS_PKT_DATAPTR(pPacket)+TXWI_SIZE)

#ifdef UAPSD_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket,
					LMR_FRAME_GET());
		}
#endif // UAPSD_AP_SUPPORT //

#ifdef WMM_ACM_SUPPORT
		{
		    HEADER_802_11  *pHeader;

			/* handle Power Save ADDTS Response */
			pHeader = (HEADER_802_11 *)(LMR_FRAME_GET());

			if ((pHeader->FC.Type == BTYPE_MGMT) &&
				(pHeader->FC.SubType == SUBTYPE_ACTION))
			{
				ACMP_PsRspDeltsSentOutHandle(pAd,
									MacTableLookup(pAd, pHeader->Addr1),
									((UCHAR *)pHeader)+sizeof(HEADER_802_11));
			}
		}
#endif // WMM_ACM_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket = NULL;
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
VOID	RTMPHandleTBTTInterrupt(
	IN PRTMP_ADAPTER pAd)
{
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pAd->OpMode == OPMODE_AP)
	{
		ReSyncBeaconTime(pAd);

#ifdef WORKQUEUE_BH	
		schedule_work(&pObj->tbtt_work);
#else
		tasklet_hi_schedule(&pObj->tbtt_task);
#endif // WORKQUEUE_BH //

		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandlePreTBTTInterrupt::Channel Switching...(%d/%d)\n", pAd->CommonCfg.RadarDetect.CSCount, pAd->CommonCfg.RadarDetect.CSPeriod));
			
			pAd->CommonCfg.RadarDetect.CSCount++;
			if (pAd->CommonCfg.RadarDetect.CSCount >= pAd->CommonCfg.RadarDetect.CSPeriod)
			{
#ifdef DFS_HARDWARE_SUPPORT
				pAd->CommonCfg.DFSAPRestart=1;
				schedule_dfs_task(pAd);
#else
				APStop(pAd);
				APStartUp(pAd);
#endif // DFS_HARDWARE_SUPPORT //
			}
		}
	}
	else
#endif // CONFIG_AP_SUPPORT //
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
		APUpdateAllBeaconFrame(pAd);
	}
	else
#endif // CONFIG_AP_SUPPORT //
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandlePreTBTTInterrupt...\n"));
		}
	}


}

VOID	RTMPHandleRxCoherentInterrupt(
	IN	PRTMP_ADAPTER	pAd)
{
	WPDMA_GLO_CFG_STRUC	GloCfg;

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

	// clear MCU Int source register.
	RTMP_IO_WRITE32(pAd, 0x7024, 0);

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
	if (pAd->CommonCfg.bIEEE80211H)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		
		// pulse radar signal Int.
		if (McuIntSrc & 0x40)
		{
			tasklet_hi_schedule(&pObj->pulse_radar_detect_task);
		}

		// width radar signal Int.
		if(((pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
				|| (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56))
			&& (McuIntSrc & 0x80))
		{
			tasklet_hi_schedule(&pObj->width_radar_detect_task);
		}

		// long pulse radar signal detection.
		if(((pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
				|| (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
				|| (JapRadarType(pAd) == JAP_W56))
		&& (McuIntSrc & 0x10))
		{
			tasklet_hi_schedule(&pObj->width_radar_detect_task);
		}
	}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //
#ifdef CARRIER_DETECTION_SUPPORT
	if ((pAd->CommonCfg.CarrierDetect.Enable)
		&& (McuIntSrc & 0x04))
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		
		tasklet_hi_schedule(&pObj->carrier_sense_task);
	}
#endif // CARRIER_DETECTION_SUPPORT //
}
#endif // CONFIG_AP_SUPPORT //

PNDIS_PACKET GetPacketFromRxRing(
	IN		PRTMP_ADAPTER	pAd,
	OUT		PRT28XX_RXD_STRUC	pSaveRxD,
	OUT		BOOLEAN			*pbReschedule,
	IN OUT	UINT32			*pRxPending)
{
	PRXD_STRUC				pRxD;
#ifdef RT_BIG_ENDIAN
	PRXD_STRUC				pDestRxD;
	RXD_STRUC				RxD;
#endif
	PNDIS_PACKET			pRxPacket = NULL;
	PNDIS_PACKET			pNewPacket;
	PVOID					AllocVa;
	NDIS_PHYSICAL_ADDRESS	AllocPa;
	BOOLEAN					bReschedule = FALSE;
	RTMP_DMACB				*pRxCell;

	RTMP_SEM_LOCK(&pAd->RxRingLock);

	if (*pRxPending == 0)
	{
		// Get how may packets had been received
		RTMP_IO_READ32(pAd, RX_DRX_IDX , &pAd->RxRing.RxDmaIdx);

		if (pAd->RxRing.RxSwReadIdx == pAd->RxRing.RxDmaIdx)
		{
			// no more rx packets
			bReschedule = FALSE;
			goto done;
		}

		// get rx pending count
		if (pAd->RxRing.RxDmaIdx > pAd->RxRing.RxSwReadIdx)
			*pRxPending = pAd->RxRing.RxDmaIdx - pAd->RxRing.RxSwReadIdx;
		else
			*pRxPending	= pAd->RxRing.RxDmaIdx + RX_RING_SIZE - pAd->RxRing.RxSwReadIdx;

	}

	pRxCell = &pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx];

#ifdef RT_BIG_ENDIAN
	pDestRxD = (PRXD_STRUC) pRxCell->AllocVa;
	RxD = *pDestRxD;
	pRxD = &RxD;
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	// Point to Rx indexed rx ring descriptor
	pRxD = (PRXD_STRUC) pRxCell->AllocVa;
#endif

	if (pRxD->DDONE == 0)
	{
		*pRxPending = 0;
		// DMAIndx had done but DDONE bit not ready
		bReschedule = TRUE;
		goto done;
	}


	// return rx descriptor
	NdisMoveMemory(pSaveRxD, pRxD, RXD_SIZE);

	pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, RX_BUFFER_AGGRESIZE, FALSE, &AllocVa, &AllocPa);

	if (pNewPacket)
	{
		// unmap the rx buffer
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
					 pRxCell->DmaBuf.AllocSize, PCI_DMA_FROMDEVICE);
		pRxPacket = pRxCell->pNdisPacket;

		pRxCell->DmaBuf.AllocSize	= RX_BUFFER_AGGRESIZE;
		pRxCell->pNdisPacket		= (PNDIS_PACKET) pNewPacket;
		pRxCell->DmaBuf.AllocVa	= AllocVa;
		pRxCell->DmaBuf.AllocPa	= AllocPa;
		/* update SDP0 to new buffer of rx packet */
		pRxD->SDP0 = AllocPa;
	}
	else 
	{
		//DBGPRINT(RT_DEBUG_TRACE,("No Rx Buffer\n"));
		pRxPacket = NULL;
		bReschedule = TRUE;
	}

	pRxD->DDONE = 0;

	// had handled one rx packet
	*pRxPending = *pRxPending - 1;	

	// update rx descriptor and kick rx 
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	INC_RING_INDEX(pAd->RxRing.RxSwReadIdx, RX_RING_SIZE);

	pAd->RxRing.RxCpuIdx = (pAd->RxRing.RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pAd->RxRing.RxSwReadIdx-1);
	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);

done:
	RTMP_SEM_UNLOCK(&pAd->RxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;
}


NDIS_STATUS MlmeHardTransmitTxRing(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	QueIdx,
	IN	PNDIS_PACKET	pPacket)
{
	PACKET_INFO 	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	PTXD_STRUC		pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	PHEADER_802_11	pHeader_802_11;
	BOOLEAN 		bAckRequired, bInsertTimestamp;
	ULONG			SrcBufPA;
	//UCHAR			TxBufIdx;
	UCHAR			MlmeRate;
	ULONG			SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;
	PTXWI_STRUC 	pFirstTxWI;
	//ULONG	i;
	//HTTRANSMIT_SETTING	MlmeTransmit;   //Rate for this MGMT frame.
	ULONG	 FreeNum;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;


	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);


	if (pSrcBufVA == NULL)
	{
		// The buffer shouldn't be NULL
		return NDIS_STATUS_FAILURE;
	}

	// Make sure MGMT ring resource won't be used by other threads
	//NdisAcquireSpinLock(&pAd->TxRingLock);

	FreeNum = GET_TXRING_FREENO(pAd, QueIdx);

	if (FreeNum == 0)
	{
		//NdisReleaseSpinLock(&pAd->TxRingLock);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;

#ifndef RT_BIG_ENDIAN
	pTxD  = (PTXD_STRUC) pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
#else
    pDestTxD  = (PTXD_STRUC)pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
    TxD = *pDestTxD;
    pTxD = &TxD;
    RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

	if (pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket)
	{
		DBGPRINT(RT_DEBUG_OFF, ("MlmeHardTransmit Error\n"));
		//NdisReleaseSpinLock(&pAd->TxRingLock);
		return NDIS_STATUS_FAILURE;
	}


	pFirstTxWI	=(PTXWI_STRUC)pSrcBufVA;
	
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXWI_SIZE);
	if (pHeader_802_11->Addr1[0] & 0x01)
	{
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	}
	else
	{
		MlmeRate = pAd->CommonCfg.MlmeRate;
	}
	
	if ((pHeader_802_11->FC.Type == BTYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

	// Verify Mlme rate for a / g bands.
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) // 11A band
		MlmeRate = RATE_6;

	//
	// Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
	// Snice it's been set to 0 while on MgtMacHeaderInit
	// By the way this will cause frame to be send on PWR_SAVE failed.
	//
	//
	// In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame
	
	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == BTYPE_CNTL) // must be PS-POLL
	{
		bAckRequired = FALSE;
	}
	else // BTYPE_MGMT or BTYPE_DATA(must be NULL frame)
	{
		if (pHeader_802_11->Addr1[0] & 0x01) // MULTICAST, BROADCAST
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
	// Before radar detection done, mgmt frame can not be sent but probe req
	// Because we need to use probe req to trigger driver to send probe req in passive scan
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
		//NdisReleaseSpinLock(&pAd->TxRingLock);
		return (NDIS_STATUS_FAILURE);
	}


#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif
	//
	// fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
	// should always has only one ohysical buffer, and the whole frame size equals
	// to the first scatter buffer size
	//

	// Initialize TX Descriptor
	// For inter-frame gap, the number is for this frame and next frame
	// For MLME rate, we will fix as 2Mb to match other vendor's implement
//	pAd->CommonCfg.MlmeTransmit.field.MODE = 1;
	
// management frame doesn't need encryption. so use RESERVED_WCID no matter u are sending to specific wcid or not.
	// Only beacon use Nseq=TRUE. So here we use Nseq=FALSE.
	if (pMacEntry == NULL)
	{
	RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
		0, RESERVED_WCID, (SrcBufLen - TXWI_SIZE), PID_MGMT, 0,  (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE, &pAd->CommonCfg.MlmeTransmit);
	}
	else
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE,
					bInsertTimestamp, FALSE, bAckRequired, FALSE,
					0, pMacEntry->Aid, (SrcBufLen - TXWI_SIZE),
					pMacEntry->MaxHTPhyMode.field.MCS, 0,
					(UCHAR)pMacEntry->MaxHTPhyMode.field.MCS,
					IFS_BACKOFF, FALSE, &pMacEntry->MaxHTPhyMode);
	}

	pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket = pPacket;
	pAd->TxRing[QueIdx].Cell[SwIdx].pNextNdisPacket = NULL;
//	pFirstTxWI->MPDUtotalByteCount = SrcBufLen - TXWI_SIZE;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange((PUCHAR)pFirstTxWI, TYPE_TXWI);
#endif
	SrcBufPA = PCI_MAP_SINGLE(pAd, pSrcBufVA, SrcBufLen, 0, PCI_DMA_TODEVICE);


	RTMPWriteTxDescriptor(pAd, pTxD, TRUE, FIFO_EDCA);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 1;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = SrcBufPA;
	pTxD->DMADONE = 0;

#ifdef RT_BIG_ENDIAN
    RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
    WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

   	// Increase TX_CTX_IDX, but write to register later.
	INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);

	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QueIdx*0x10,  pAd->TxRing[QueIdx].TxCpuIdx);

   	// Make sure to release MGMT ring resource
//	NdisReleaseSpinLock(&pAd->TxRingLock);

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS MlmeDataHardTransmit(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	QueIdx,
	IN	PNDIS_PACKET	pPacket)
{
	if ((pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
		)
	{
		return NDIS_STATUS_FAILURE;
	}

	return MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
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
VOID RTMPWriteTxDescriptor(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXD_STRUC		pTxD,
	IN	BOOLEAN 		bWIV,
	IN	UCHAR			QueueSEL)
{
	//
	// Always use Long preamble before verifiation short preamble functionality works well.
	// Todo: remove the following line if short preamble functionality works
	//
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	pTxD->WIV	= (bWIV) ? 1: 0;
	pTxD->QSEL= (QueueSEL);
	//RT2860c??  fixed using EDCA queue for test...  We doubt Queue1 has problem.  2006-09-26 Jan
	//pTxD->QSEL= FIFO_EDCA;
	if (pAd->bGenOneHCCA == TRUE)
		pTxD->QSEL= FIFO_HCCA;
	pTxD->DMADONE = 0;
}
