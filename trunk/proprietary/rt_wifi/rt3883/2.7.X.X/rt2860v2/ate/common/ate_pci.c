/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	ate_pci.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifdef RTMP_MAC_PCI

#include "rt_config.h"

/* 802.11 MAC Header, Type:Data, Length:24bytes + 6 bytes QOS/HTC + 2 bytes padding */
extern UCHAR TemplateFrame[32];

INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
	result = (GloCfg.field.TxDMABusy) ? TRUE : FALSE;

	return result;
}


INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
	result = (GloCfg.field.RxDMABusy) ? TRUE : FALSE;

	return result;
}


VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable)
{
	BOOLEAN value;
	ULONG WaitCnt;
	WPDMA_GLO_CFG_STRUC GloCfg;
	
	value = Enable > 0 ? 1 : 0;

	/* check if DMA is in busy mode or not. */
	WaitCnt = 0;

	while (TxDmaBusy(pAd) || RxDmaBusy(pAd))
	{
		RTMPusecDelay(10);

		if (WaitCnt++ > 100)
			break;
	}
	
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
	GloCfg.field.EnableTxDMA = value;
	GloCfg.field.EnableRxDMA = value;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);	/* abort all TX rings */
	RtmpOsMsDelay(5);

	return;
}


/*
========================================================================
	Routine Description:
		Write TxWI for ATE mode.
		
	Return Value:
		None
========================================================================
*/
VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC 	pOutTxWI,	
	IN	BOOLEAN			FRAG,	
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,		/* HW new a sequence. */
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,	
	IN	BOOLEAN			CfAck,	
	IN	HTTRANSMIT_SETTING	*pTransmit)
{
	TXWI_STRUC 		TxWI;
	PTXWI_STRUC 	pTxWI;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWISize);
	pTxWI = &TxWI;

	pTxWI->FRAG= FRAG;

	pTxWI->CFACK = CFACK;
	pTxWI->TS= InsTimestamp;
	pTxWI->AMPDU = AMPDU;
	pTxWI->ACK = Ack;
	pTxWI->txop= Txopmode;
	
	pTxWI->NSEQ = NSeq;

	if ( BASize >7 )
		BASize =7;
		
	pTxWI->BAWinSize = BASize;	
	pTxWI->WirelessCliID = WCID;
	pTxWI->MPDUtotalByteCount = Length; 
	pTxWI->PacketId = PID; 
	
	/* If CCK or OFDM, BW must be 20 */
	pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	pTxWI->ShortGI = pTransmit->field.ShortGI;
	pTxWI->STBC = pTransmit->field.STBC;
	
	pTxWI->MCS = pTransmit->field.MCS;
	pTxWI->PHYMODE = pTransmit->field.MODE;
	pTxWI->CFACK = CfAck;
	pTxWI->MIMOps = 0;
	pTxWI->MpduDensity = 0;

	pTxWI->PacketId = pTxWI->MCS;
	NdisMoveMemory(pOutTxWI, &TxWI, TXWISize);

    return;
}


/* 
==========================================================================
	Description:
		Setup Frame format.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT pos = 0;
	PTXD_STRUC pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	PNDIS_PACKET pPacket=NULL;
	PUCHAR pDest=NULL;
	PVOID AllocVa=NULL;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	HTTRANSMIT_SETTING	TxHTPhyMode;

	PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
	PTXWI_STRUC pTxWI = (PTXWI_STRUC) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	PUCHAR pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

#ifdef RALINK_QA
	PHEADER_802_11	pHeader80211;
#endif /* RALINK_QA */

	/* fill TxWI */
	TxHTPhyMode.field.BW = pATEInfo->TxWI.BW;
	TxHTPhyMode.field.ShortGI = pATEInfo->TxWI.ShortGI;
	TxHTPhyMode.field.STBC = pATEInfo->TxWI.STBC;
	TxHTPhyMode.field.MCS = pATEInfo->TxWI.MCS;
	TxHTPhyMode.field.MODE = pATEInfo->TxWI.PHYMODE;

	if (pATEInfo->bQATxStart == TRUE) 
	{
		/* always use QID_AC_BE and FIFO_EDCA */
		ATEWriteTxWI(pAd, pTxWI, pATEInfo->TxWI.FRAG, pATEInfo->TxWI.CFACK,
			pATEInfo->TxWI.TS, pATEInfo->TxWI.AMPDU, pATEInfo->TxWI.ACK,
			pATEInfo->TxWI.NSEQ, pATEInfo->TxWI.BAWinSize, 0,
			pATEInfo->TxWI.MPDUtotalByteCount, pATEInfo->TxWI.PacketId, 0, 0,
			pATEInfo->TxWI.txop/*IFS_HTTXOP*/, pATEInfo->TxWI.CFACK/*FALSE*/,
			&TxHTPhyMode);

#ifdef TXBF_SUPPORT
		if (IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			/* Must copy rsv bits to actual TxWI */
			pTxWI->rsv = pATEInfo->TxWI.rsv;
			pTxWI->iTxBF = pATEInfo->TxWI.iTxBF;	
			pTxWI->Sounding = pATEInfo->TxWI.Sounding;
			pTxWI->eTxBF = pATEInfo->TxWI.eTxBF;
			pTxWI->Autofallback = pATEInfo->TxWI.Autofallback;
			pTxWI->NDPSndBW = pATEInfo->TxWI.NDPSndBW;
			pTxWI->NDPSndRate = pATEInfo->TxWI.NDPSndRate;
		}
#endif /* TXBF_SUPPORT */
	}
	else
	{
		ATEWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE,  FALSE, FALSE, FALSE, 
			4, 0, pATEInfo->TxLength, 0, 0, 0, IFS_HTTXOP, FALSE, &TxHTPhyMode);

#ifdef TXBF_SUPPORT
		if (pATEInfo->bTxBF == 1)
		{
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				pTxWI->rsv = 0;
				pTxWI->iTxBF = pATEInfo->TxWI.iTxBF;	
				pTxWI->Sounding = (pATEInfo->txSoundingMode == 1 ? 1 : 0);
				pTxWI->eTxBF = pATEInfo->TxWI.eTxBF;
				pTxWI->Autofallback = pATEInfo->TxWI.Autofallback;
				pTxWI->NDPSndBW = pATEInfo->TxWI.BW;
				if (pATEInfo->txSoundingMode == 3)
					pTxWI->NDPSndRate = 2;
				else if (pATEInfo->txSoundingMode == 2)
					pTxWI->NDPSndRate = 1;
				else
					pTxWI->NDPSndRate = 0;
			}
		}
#endif /* TXBF_SUPPORT */
	}
	
	/* fill 802.11 header */
#ifdef RALINK_QA
	if (pATEInfo->bQATxStart == TRUE) 
	{
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize, pATEInfo->Header, pATEInfo->HLen);
	}
	else
#endif /* RALINK_QA */
	{
		pATEInfo->HLen = LENGTH_802_11;
#ifdef TXBF_SUPPORT
		TemplateFrame[0] = 0x08;	/* Data */
		TemplateFrame[1] = 0x00;
		if (pATEInfo->bTxBF && pATEInfo->txSoundingMode!=0)
		{
			/* QoS Data */
			pATEInfo->HLen = 32;
			TemplateFrame[0] = 0x88;
			TemplateFrame[1] = 0x80;
		
			switch (pATEInfo->txSoundingMode)
			{
			case 1:
				/* Data Sounding */
				TemplateFrame[28] = pAd->CommonCfg.ETxBfNoncompress? 0x80: 0xc0;
				TemplateFrame[29] = 0x00;	
				break;
			case 2:
			case 3:
				/* 2 or 3 Stream NDP */
				TemplateFrame[28] = pAd->CommonCfg.ETxBfNoncompress? 0x80: 0xc0;
				TemplateFrame[29] = 0x01;	/* NDP Announce */
				break;
			default:
				TemplateFrame[28] = TemplateFrame[29] = 0x0;
			}
		}
#endif /* TXBF_SUPPORT */
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize, TemplateFrame, pATEInfo->HLen);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 4, pATEInfo->Addr1, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 10, pATEInfo->Addr2, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 16, pATEInfo->Addr3, ETH_LENGTH_OF_ADDRESS);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_READ, FALSE);
#endif /* RT_BIG_ENDIAN */

	/* alloc buffer for payload */
#ifdef RALINK_QA
	if ((pATEInfo->bQATxStart == TRUE) && (pATEInfo->DLen != 0)) 
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pATEInfo->DLen + 0x100, FALSE, &AllocVa, &AllocPa);
	}
	else
#endif /* RALINK_QA */
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pATEInfo->TxLength, FALSE, &AllocVa, &AllocPa);
	}

	if (pPacket == NULL)
	{
		pATEInfo->TxCount = 0;
		DBGPRINT_ERR(("%s : fail to alloc packet space.\n", __FUNCTION__));
		return -1;
	}

	pTxRing->Cell[TxIdx].pNextNdisPacket = pPacket;
	pDest = (PUCHAR) AllocVa;

#ifdef RALINK_QA
	if ((pATEInfo->bQATxStart == TRUE) && (pATEInfo->DLen != 0)) 
	{
		GET_OS_PKT_LEN(pPacket) = pATEInfo->DLen;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pATEInfo->DLen;
#endif /* LIMUX */
	}
	else
#endif /* RALINK_QA */
	{
		GET_OS_PKT_LEN(pPacket) = pATEInfo->TxLength - LENGTH_802_11;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pATEInfo->TxLength - LENGTH_802_11;
#endif /* LINUX */
	}

	/* prepare frame payload */
#ifdef RALINK_QA
	if ((pATEInfo->bQATxStart == TRUE) && (pATEInfo->DLen != 0))
	{
		/* copy pattern to payload */
		if ((pATEInfo->PLen != 0))
		{
			for (pos = 0; pos < pATEInfo->DLen; pos += pATEInfo->PLen)
			{
				memcpy(GET_OS_PKT_DATAPTR(pPacket) + pos, pATEInfo->Pattern, pATEInfo->PLen);
			}
		}
	}
	else
#endif /* RALINK_QA */
	{
		for (pos = 0; pos < GET_OS_PKT_LEN(pPacket); pos++)
		{
			/* default payload is 0xA5 */
			pDest[pos] = pATEInfo->Payload;
		}
	}

	/* build Tx descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
    pDestTxD  = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
    TxD = *pDestTxD;
    pTxD = &TxD;
#endif /* !RT_BIG_ENDIAN */

#ifdef RALINK_QA
	if (pATEInfo->bQATxStart == TRUE)
	{
		/* prepare TxD */
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		/* build Tx descriptor */
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWISize + pATEInfo->HLen;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = GET_OS_PKT_LEN(pPacket);
		pTxD->LastSec0 = (pTxD->SDLen1 == 0) ? 1 : 0;
		pTxD->LastSec1 = 1;

		pDest = (PUCHAR)pTxWI;
		pDest += TXWISize;
		pHeader80211 = (PHEADER_802_11)pDest;
		
		/* modify sequence number... */
		if (pATEInfo->TxDoneCount == 0)
			pATEInfo->seq = pHeader80211->Sequence;
		else
			pHeader80211->Sequence = ++pATEInfo->seq;
	}
	else
#endif /* RALINK_QA */
	{
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		/* build Tx descriptor */
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow (pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWISize + LENGTH_802_11;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = GET_OS_PKT_LEN(pPacket);
		pTxD->LastSec1 = 1;
	}

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	return 0;
}

#endif /* RTMP_MAC_PCI */

