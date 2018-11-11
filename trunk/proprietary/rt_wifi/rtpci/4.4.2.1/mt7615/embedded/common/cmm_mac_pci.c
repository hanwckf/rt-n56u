/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"


static INT desc_ring_alloc(RTMP_ADAPTER *pAd, RTMP_DMABUF *pDescRing, INT size)
{
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;

	pDescRing->AllocSize = size;
	RtmpAllocDescBuf(pci_dev,
				0,
				pDescRing->AllocSize,
				FALSE,
				&pDescRing->AllocVa,
				&pDescRing->AllocPa);

	if (pDescRing->AllocVa == NULL)
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
		return ERRLOG_OUT_OF_SHARED_MEMORY;
	}

	/* Zero init this memory block*/
	NdisZeroMemory(pDescRing->AllocVa, size);

	return 0;
}

#ifdef RESOURCE_PRE_ALLOC
static INT desc_ring_free(RTMP_ADAPTER *pAd, RTMP_DMABUF *pDescRing)
{
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;

	if (pDescRing->AllocVa)
	{
		RtmpFreeDescBuf(pci_dev,
						pDescRing->AllocSize,
						pDescRing->AllocVa,
						pDescRing->AllocPa);
	}
	NdisZeroMemory(pDescRing, sizeof(RTMP_DMABUF));

	return TRUE;
}


VOID RTMPResetTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	int index, j;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;

	rtmp_tx_swq_exit(pAd, WCID_ALL);

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->PciHif.TxRing[index];
		for (j=0; j< TX_RING_SIZE; j++)
		{
			dma_cb = &pTxRing->Cell[j];
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *)(dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = dma_cb->pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, dma_cb->PacketPa, GET_OS_PKT_LEN(pPacket), RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNdisPacket as NULL after clear*/
			dma_cb->pNdisPacket = NULL;

			pPacket = dma_cb->pNextNdisPacket;
			if (pPacket)
			{
#if defined(MT7615) || defined(MT7622)
                            if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
                                MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
                                                    ("%s():ERR!!7615 shall not has one TxD with 2 pkt buffer!\n",
                                                    __FUNCTION__));
                            }
#endif /* defined(MT7615) || defined(MT7622) */
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			dma_cb->pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}

#ifdef CONFIG_ANDES_SUPPORT
	{
		RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;

		HIF_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxCpuIdx)
		{

			if(pCtrlRing->TxSwFreeIdx >= CTL_RING_SIZE)
			{
				/*should not happen*/
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> TxSwFreeIdx is out of range! %d\n", pCtrlRing->TxSwFreeIdx));
				break;
			}

#ifdef RT_BIG_ENDIAN
	        pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;

			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
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
			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
	}
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		// TODO: shiang-MT7615, we don't need bcn ring now!
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	if (1 /*pAd->chipCap.hif_type == HIF_MT*/)
	{
		RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;

		HIF_IO_READ32(pAd, pBcnRing->hw_didx_addr, &pBcnRing->TxDmaIdx);

		while (pBcnRing->TxSwFreeIdx!= pBcnRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);
				continue;
			}

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket;
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
			pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
	}
#endif /* MT_MAC */

	for (index = 0; index < NUM_OF_RX_RING; index++)
	{
		UINT16 RxRingSize;

		if (index == 0)
		{
			RxRingSize = RX_RING_SIZE;
		}
		else
		{
			RxRingSize = RX1_RING_SIZE;
		}

		for (j = RxRingSize - 1 ; j >= 0; j--)
		{
			dma_cb = &pAd->PciHif.RxRing[index].Cell[j];
			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket))
			{
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
#ifdef  CONFIG_WIFI_BUILD_SKB
				if (index == 0) {
					DEV_FREE_FRAG_BUF(dma_cb->pNdisPacket);
				}
				else {
#endif /* CONFIG_WIFI_BUILD_SKB */
					RELEASE_NDIS_PACKET(
						pAd, dma_cb->pNdisPacket, 
						NDIS_STATUS_SUCCESS);
#ifdef  CONFIG_WIFI_BUILD_SKB
				}
#endif /* CONFIG_WIFI_BUILD_SKB */
			}
		}
		NdisZeroMemory(pAd->PciHif.RxRing[index].Cell, RxRingSize * sizeof(RTMP_DMACB));
	}
#ifdef MT_MAC
#ifdef CUT_THROUGH
    if (pAd->PktTokenCb)
    {
        cut_through_deinit((PKT_TOKEN_CB **)&pAd->PktTokenCb);
    }
#endif /* CUT_THROUGH */
#endif /* MT_MAC */    

	if (pAd->FragFrame.pFragPacket)
	{
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);
}


VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	INT num;
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));


	/* Free Rx/Mgmt Desc buffer*/
	for (num = 0; num < NUM_OF_RX_RING; num++)
		desc_ring_free(pAd, &pAd->PciHif.RxDescRing[num]);

	desc_ring_free(pAd, &pAd->MgmtDescRing);
#ifdef CONFIG_ANDES_SUPPORT
	desc_ring_free(pAd, &pAd->CtrlDescRing);
#if defined(MT7615) || defined(MT7622)
    desc_ring_free(pAd, &pAd->FwDwloRing.DescRing);
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	desc_ring_free(pAd, &pAd->BcnDescRing);

	if (pAd->TxBmcBufSpace.AllocVa)
	{
		RTMP_FreeFirstTxBuffer(pci_dev,
								pAd->TxBmcBufSpace.AllocSize,
								FALSE, pAd->TxBmcBufSpace.AllocVa,
								pAd->TxBmcBufSpace.AllocPa);
	}
	NdisZeroMemory(&pAd->TxBmcBufSpace, sizeof(RTMP_DMABUF));

	desc_ring_free(pAd, &pAd->TxBmcDescRing);
#endif /* MT_MAC */

	/* Free 1st TxBufSpace and TxDesc buffer*/
	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
		if (pAd->PciHif.TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(pci_dev,
									pAd->PciHif.TxBufSpace[num].AllocSize,
									FALSE, pAd->PciHif.TxBufSpace[num].AllocVa,
									pAd->PciHif.TxBufSpace[num].AllocPa);
		}
		NdisZeroMemory(&pAd->PciHif.TxBufSpace[num], sizeof(RTMP_DMABUF));

		desc_ring_free(pAd, &pAd->PciHif.TxDescRing[num]);
	}

#ifdef MT_MAC
#ifdef CUT_THROUGH
	    if (pAd->PktTokenCb)
	    {
		cut_through_deinit((PKT_TOKEN_CB **)&pAd->PktTokenCb);
	    }
#endif /* CUT_THROUGH */
#endif /* MT_MAC */

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}


NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	INT num, index;
	ULONG RingBasePaHigh, RingBasePaLow;
	VOID *RingBaseVa;
	RTMP_TX_RING *pTxRing;
	RTMP_RX_RING *pRxRing;
	RTMP_DMABUF *pDmaBuf, *pDescRing;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;


	/* Initialize All Tx Ring Descriptors and associated buffer memory*/
	/* (5 TX rings = 4 ACs + 1 HCCA)*/
	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
		ULONG BufBasePaHigh, BufBasePaLow;
		VOID *BufBaseVa;

		/* memory zero the  Tx ring descriptor's memory */
		pDescRing = &pAd->PciHif.TxDescRing[num];
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(pAd->PciHif.TxBufSpace[num].AllocVa, pAd->PciHif.TxBufSpace[num].AllocSize);
		/* Save PA & VA for further operation */
		BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->PciHif.TxBufSpace[num].AllocPa);
		BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->PciHif.TxBufSpace[num].AllocPa);
		BufBaseVa = pAd->PciHif.TxBufSpace[num].AllocVa;

		/* linking Tx Ring Descriptor and associated buffer memory */
		pTxRing = &pAd->PciHif.TxRing[num];
		for (index = 0; index < TX_RING_SIZE; index++)
		{
			dma_cb = &pTxRing->Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Tx Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
			RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->SDPtr0 = BufBasePaLow;
            pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* advance to next TxBuf address */
			BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
			BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxRing[%d]: total %d entry initialized\n", num, index));
	}

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		// TODO: shiang-MT7615, we don't need Mgmt ring now!
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("MgmtRing: Not Required!\n"));
	}
	else
#endif
	{
		/* Initialize MGMT Ring and associated buffer memory */
		pDescRing = &pAd->MgmtDescRing;
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		for (index = 0; index < MGMT_RING_SIZE; index++)
		{
			dma_cb = &pAd->MgmtRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init MGMT Ring Size, Va, Pa variables */
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address */
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in MgmtRing for scatter-gather case */
		}
	}

#ifdef CONFIG_ANDES_SUPPORT
	/* Initialize CTRL Ring and associated buffer memory */
	pDescRing = &pAd->CtrlDescRing;
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
	RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
	RingBaseVa = pDescRing->AllocVa;
	NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
	for (index = 0; index < CTL_RING_SIZE; index++)
	{
		dma_cb = &pAd->CtrlRing.Cell[index];
		dma_cb->pNdisPacket = NULL;
		dma_cb->pNextNdisPacket = NULL;
		/* Init Ctrl Ring Size, Va, Pa variables */
		dma_cb->AllocSize = TXD_SIZE;
		dma_cb->AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

		/* link the pre-allocated TxBuf to TXD */
		pTxD = (TXD_STRUC *)dma_cb->AllocVa;
		pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

		/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
	}
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("CtrlRing: total %d entry initialized\n", index));

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		/* Initialize firmware download Ring */
		pDescRing = &pAd->FwDwloRing.DescRing;
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		for (index = 0; index < CTL_RING_SIZE; index++)
		{
			dma_cb = &pAd->FwDwloRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init firmware download ring Size, Va, Pa variables */
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address */
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in FwDwloRing for scatter-gather case */
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("FwDwloRing: total %d entry initialized\n", index));
	}
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */


#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		// TODO: shiang-MT7615, we don't need bcn ring now!
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("BcnRing: Not Required!\n"));
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	if (1 /*pAd->chipCap.hif_type == HIF_MT*/)
	{
		/* Initialize CTRL Ring and associated buffer memory */
		ULONG BufBasePaHigh, BufBasePaLow;
		VOID *BufBaseVa;

		pDescRing = &pAd->BcnDescRing;
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,  ("TX_BCN DESC %p size = %lu\n",
					pDescRing->AllocVa, pDescRing->AllocSize));
		for (index = 0; index < BCN_RING_SIZE; index++)
		{
			dma_cb = &pAd->BcnRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Ctrl Ring Size, Va, Pa variables */
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address */
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
		}

		/* memory zero the  Tx BMC ring descriptor's memory */
		pDescRing = &pAd->TxBmcDescRing;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(pAd->TxBmcBufSpace.AllocVa, pAd->TxBmcBufSpace.AllocSize);
		/* Save PA & VA for further operation */
		BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBmcBufSpace.AllocPa);
		BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBmcBufSpace.AllocPa);
		BufBaseVa = pAd->TxBmcBufSpace.AllocVa;

		/* linking Tx Ring Descriptor and associated buffer memory */
		pTxRing = &pAd->TxBmcRing;
		for (index = 0; index < TX_RING_SIZE; index++)
		{
			dma_cb = &pTxRing->Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Tx Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
			RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->SDPtr0 = BufBasePaLow;
            pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* advance to next TxBuf address */
			BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
			BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
		}
	}


#ifdef CUT_THROUGH
    if (!pAd->PktTokenCb)
    {
        cut_through_init(&pAd->PktTokenCb, pAd);
    }    
#endif /* CUT_THROUGH */    
#endif /* MT_MAC */

	/* Initialize Rx Ring and associated buffer memory */
	for (num = 0; num < NUM_OF_RX_RING; num++)
	{
		UINT16 RxRingSize;
		UINT16 RxBufferSize;

		pDescRing = &pAd->PciHif.RxDescRing[num];
		pRxRing = &pAd->PciHif.RxRing[num];

		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,  ("RX[%d] DESC %p size = %lu\n",
					num, pDescRing->AllocVa, pDescRing->AllocSize));

		/* Save PA & VA for further operation */
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		if (num == 0)
		{
			RxRingSize = RX_RING_SIZE;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		}
		else
		{
			RxRingSize = RX1_RING_SIZE;
			RxBufferSize = RX1_BUFFER_SIZE;
		}

		/* Linking Rx Ring and associated buffer memory */
		for (index = 0; index < RxRingSize; index++)
		{
			dma_cb = &pRxRing->Cell[index];
			/* Init RX Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = RXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);;

			/* Offset to next ring descriptor address */
			RingBasePaLow += RXD_SIZE;
			RingBaseVa = (PUCHAR)RingBaseVa + RXD_SIZE;

			/* Setup Rx associated Buffer size & allocate share memory */
			pDmaBuf = &dma_cb->DmaBuf;
#ifdef CUT_THROUGH
			if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb))
				pDmaBuf->AllocSize = RxBufferSize + sizeof(RX_BLK);
			else
#endif /* CUT_THROUGH */
			pDmaBuf->AllocSize = RxBufferSize;
#ifdef  CONFIG_WIFI_BUILD_SKB
			if (num == 0) {
				pPacket = RTMP_AllocateRxPacketBufferOnly(
					pAd,
					((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					pDmaBuf->AllocSize,
					FALSE,
					&pDmaBuf->AllocVa,
					&pDmaBuf->AllocPa);
			}
			else {
#endif /* CONFIG_WIFI_BUILD_SKB */
				pPacket = RTMP_AllocateRxPacketBuffer(
					pAd,
					((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					pDmaBuf->AllocSize,
					FALSE,
					&pDmaBuf->AllocVa,
					&pDmaBuf->AllocPa);
#ifdef  CONFIG_WIFI_BUILD_SKB
			}
#endif /* CONFIG_WIFI_BUILD_SKB*/
			/* keep allocated rx packet */
			dma_cb->pNdisPacket = pPacket;

			/* Error handling*/
			if (pDmaBuf->AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate RxRing's 1st buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block */
			//NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize); 

#ifdef CUT_THROUGH
			if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (num == 0))
			{
				pDmaBuf->AllocVa += sizeof(RX_BLK);
				pDmaBuf->AllocPa += sizeof(RX_BLK);
				OS_PKT_RESERVE(pPacket, sizeof(RX_BLK));
			}

			/* Only Rx Ring 0 need token! */
			if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (num == 0))
				dma_cb->token_id = cut_through_rx_enq(pAd->PktTokenCb, pPacket, TOKEN_RX);
#endif /* CUT_THROUGH */

			/* Write RxD buffer address & allocated buffer length */
			pRxD = (RXD_STRUC *)dma_cb->AllocVa;
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			pRxD->DDONE = 0;
			pRxD->SDL0 = RxBufferSize;

#ifdef CUT_THROUGH
            if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (num == 0))
            {
                pRxD->SDP1 = cpu2le32((((UINT32)dma_cb->token_id) & 0x0000ffff));
            }
#endif /* CUT_THROUGH */


#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
		}
	}

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
	if (pAd->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	/* Initialize all transmit related software queues */
	rtmp_tx_swq_init(pAd);
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		/* Init TX rings index pointer */
		pAd->PciHif.TxRing[index].TxSwFreeIdx = 0;
		pAd->PciHif.TxRing[index].TxCpuIdx = 0;
	}

	/* Init RX Ring index pointer */
	for (index = 0; index < NUM_OF_RX_RING; index++)
	{
		UINT16 RxRingSize;
		UINT16 RxBufferSize;

		if (index == 0)
		{
			RxRingSize = RX_RING_SIZE;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		}
		else
		{
			RxRingSize = RX1_RING_SIZE;
			RxBufferSize = RX1_BUFFER_SIZE;
		}
		pAd->PciHif.RxRing[index].RxSwReadIdx = 0;
		pAd->PciHif.RxRing[index].RxCpuIdx = RxRingSize - 1;
		pAd->PciHif.RxRing[index].RxRingSize = RxRingSize;
		pAd->PciHif.RxRing[index].RxBufferSize = RxBufferSize;
	}

	/* init MGMT ring index pointer */
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer */
	pAd->CtrlRing.TxSwFreeIdx = 0;
	pAd->CtrlRing.TxCpuIdx = 0;
#endif /* CONFIG_ANDES_SUPPORT */

	pAd->PrivateInfo.TxRingFullCnt = 0;

	return Status;

}


/*
	========================================================================

	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT num;
	ULONG ErrorValue = 0;
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("-->RTMPAllocTxRxRingMemory\n"));
	do
	{
		/*
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes.
		*/
		for (num = 0; num < NUM_OF_TX_RING; num++)
		{
			/* Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)*/
			desc_ring_alloc(pAd, &pAd->PciHif.TxDescRing[num], TX_RING_SIZE * TXD_SIZE);
			if (pAd->PciHif.TxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxRing[%d]: total %d bytes allocated\n",
						num, (INT)pAd->PciHif.TxDescRing[num].AllocSize));

			/* Allocate all 1st TXBuf's memory for this TxRing */
			pAd->PciHif.TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				num,
				pAd->PciHif.TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->PciHif.TxBufSpace[num].AllocVa,
				&pAd->PciHif.TxBufSpace[num].AllocPa);

			if (pAd->PciHif.TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			// TODO: shiang-MT7615, fix me!!
		}
		else
#endif /* defined(MT7615) || defined(MT7622) */
		{
			/* Alloc MGMT ring desc buffer except Tx ring allocated eariler */
			desc_ring_alloc(pAd, &pAd->MgmtDescRing, MGMT_RING_SIZE * TXD_SIZE);
			if (pAd->MgmtDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
                                ("MGMT Ring: total %d bytes allocated\n",
					(INT)pAd->MgmtDescRing.AllocSize));
		}

#ifdef CONFIG_ANDES_SUPPORT
		/* Alloc CTRL ring desc buffer except Tx ring allocated eariler */
		desc_ring_alloc(pAd, &pAd->CtrlDescRing, CTL_RING_SIZE * TXD_SIZE);
		if (pAd->CtrlDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
                            ("CTRL Ring: total %d bytes allocated\n",
					(INT)pAd->CtrlDescRing.AllocSize));

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			/* Alloc firmware download ring desc buffer except Tx ring allocated eariler */
			desc_ring_alloc(pAd, &pAd->FwDwloRing.DescRing, pAd->FwDwloRing.ring_size * TXD_SIZE);
			if (pAd->FwDwloRing.DescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
						("FwDwlo Ring: total %d bytes allocated\n",
						(INT)pAd->FwDwloRing.DescRing.AllocSize));
		}
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			// TODO: shiang-MT7615, fix me!!
		}
		else
#endif /* defined(MT7615) || defined(MT7622) */
		if (1 /*pAd->chipCap.hif_type == HIF_MT*/) {
			/* Alloc Beacon ring desc buffer */
			desc_ring_alloc(pAd, &pAd->BcnDescRing, BCN_RING_SIZE * TXD_SIZE);
			if (pAd->BcnDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
                                ("Beacon Ring: total %d bytes allocated\n",
						(INT)pAd->BcnDescRing.AllocSize));

			/* Allocate Tx ring descriptor's memory (BMC)*/
			desc_ring_alloc(pAd, &pAd->TxBmcDescRing, TX_RING_SIZE * TXD_SIZE);
			if (pAd->TxBmcDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
                                ("TxBmcRing: total %d bytes allocated\n",
						(INT)pAd->TxBmcDescRing.AllocSize));

			/* Allocate all 1st TXBuf's memory for this TxRing */
			pAd->TxBmcBufSpace.AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				0,
				pAd->TxBmcBufSpace.AllocSize,
				FALSE,
				&pAd->TxBmcBufSpace.AllocVa,
				&pAd->TxBmcBufSpace.AllocPa);

			if (pAd->TxBmcBufSpace.AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			if (Status == NDIS_STATUS_RESOURCES)
				break;
		}

#ifdef CUT_THROUGH
		cut_through_init(&pAd->PktTokenCb, pAd);
#endif /* CUT_THROUGH */

#endif /* MT_MAC */

		/* Alloc RX ring desc memory except Tx ring allocated eariler */
		for (num = 0; num < NUM_OF_RX_RING; num++)
		{
			UINT16 RxRingSize;

			if (num == 0)
			{
				RxRingSize = RX_RING_SIZE;
			}
			else
			{
				RxRingSize = RX1_RING_SIZE;
			}

			desc_ring_alloc(pAd, &pAd->PciHif.RxDescRing[num],
								RxRingSize * RXD_SIZE);

			if (pAd->PciHif.RxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("Rx[%d] Ring: total %d bytes allocated\n",
						num, (INT)pAd->PciHif.RxDescRing[num].AllocSize));
		}
	} while (FALSE);


	if (Status != NDIS_STATUS_SUCCESS)
	{
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
                        ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));

	return Status;
}

#else
/*
	========================================================================

	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	ULONG RingBasePaHigh, RingBasePaLow;
	PVOID RingBaseVa;
	INT index, num;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	RTMP_TX_RING *pTxRing;
	RTMP_DMABUF *pDmaBuf;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket;


	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));

	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);


	do
	{
		/*
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes
		*/
		for (num=0; num<NUM_OF_TX_RING; num++)
		{
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;

			/*
				Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)
			*/
			desc_ring_alloc(pAd, &pAd->PciHif.TxDescRing[num], TX_RING_SIZE * TXD_SIZE);
			if (pAd->PciHif.TxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			pDmaBuf = &pAd->PciHif.TxDescRing[num];
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxDescRing[%p]: total %d bytes allocated\n",
					pDmaBuf->AllocVa, (INT)pDmaBuf->AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDmaBuf->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			RingBaseVa = pDmaBuf->AllocVa;

			/*
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			pAd->PciHif.TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				num,
				pAd->PciHif.TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->PciHif.TxBufSpace[num].AllocVa,
				&pAd->PciHif.TxBufSpace[num].AllocPa);

			if (pAd->PciHif.TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pAd->PciHif.TxBufSpace[num].AllocVa, pAd->PciHif.TxBufSpace[num].AllocSize);

			/* Save PA & VA for further operation*/
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->PciHif.TxBufSpace[num].AllocPa);
			BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->PciHif.TxBufSpace[num].AllocPa);
			BufBaseVa = pAd->PciHif.TxBufSpace[num].AllocVa;

			/*
				Initialize Tx Ring Descriptor and associated buffer memory
			*/
			pTxRing = &pAd->PciHif.TxRing[num];
			for (index = 0; index < TX_RING_SIZE; index++)
			{
				dma_cb = &pTxRing->Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init Tx Ring Size, Va, Pa variables */
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

				/* Setup Tx Buffer size & address. only 802.11 header will store in this space*/
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
				RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

				/* link the pre-allocated TxBuf to TXD */
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->SDPtr0 = BufBasePaLow;
                pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* advance to next TxBuf address */
				BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxRing[%d]: total %d entry allocated\n", num, index));
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		/*
			Allocate MGMT ring descriptor's memory except Tx ring which allocated eariler
		*/
		desc_ring_alloc(pAd, &pAd->MgmtDescRing, MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->MgmtDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("MgmtDescRing[%p]: total %d bytes allocated\n",
				pAd->MgmtDescRing.AllocVa, (INT)pAd->MgmtDescRing.AllocSize));

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->MgmtDescRing.AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(pAd->MgmtDescRing.AllocPa);
		RingBaseVa = pAd->MgmtDescRing.AllocVa;

		/*
			Initialize MGMT Ring and associated buffer memory
		*/
		for (index = 0; index < MGMT_RING_SIZE; index++)
		{
			dma_cb = &pAd->MgmtRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init MGMT Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in MgmtRing for scatter-gather case*/
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("MGMT Ring: total %d entry allocated\n", index));

#ifdef CONFIG_ANDES_SUPPORT
		/*
			Allocate CTRL ring descriptor's memory except Tx ring which allocated eariler
		*/
		desc_ring_alloc(pAd, &pAd->CtrlDescRing, CTL_RING_SIZE * TXD_SIZE);
		if (pAd->CtrlDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("CtrlDescRing[%p]: total %d bytes allocated\n",
				pAd->CtrlDescRing.AllocVa, (INT)pAd->CtrlDescRing.AllocSize));

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->CtrlDescRing.AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->CtrlDescRing.AllocPa);
		RingBaseVa = pAd->CtrlDescRing.AllocVa;

		/*
			Initialize CTRL Ring and associated buffer memory
		*/
		for (index = 0; index < CTL_RING_SIZE; index++)
		{
			dma_cb = &pAd->CtrlRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init CTRL Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
		}
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("CTRL Ring: total %d entry allocated\n", index));

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			/*
				Allocate firmware download ring descriptor's memory except Tx ring which allocated eariler
			*/
			desc_ring_alloc(pAd, &pAd->FwDwloRing.DescRing, pAd->FwDwloRing.ring_size * TXD_SIZE);
			if (pAd->FwDwloRing.DescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("FwDwloRing.DescRing[%p]: total %d bytes allocated\n",
					pAd->FwDwloRing.DescRing.AllocVa, (INT)pAd->FwDwloRing.DescRing.AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->FwDwloRing.DescRing.AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->FwDwloRing.DescRing.AllocPa);
			RingBaseVa = pAd->FwDwloRing.DescRing.AllocVa;

			/*
				Initialize firmware download ring and associated buffer memory
			*/
			for (index = 0; index < CTL_RING_SIZE; index++)
			{
				dma_cb = &pAd->FwDwloRing.Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init firmware download Ring Size, Va, Pa variables*/
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);

				/* Offset to next ring descriptor address*/
				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* link the pre-allocated TxBuf to TXD*/
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

				/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("FwDwlo Ring: total %d entry allocated\n", index));
		}
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */


#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			// TODO: shiang-MT7615, we don't need bcn ring now!
		}
	else
#endif /* defined(MT7615) || defined(MT7622) */
		if (1/* pAd->chipCap.hif_type == HIF_MT */) {
			/* Allocate Beacon ring descriptor's memory */
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;

			desc_ring_alloc(pAd, &pAd->BcnDescRing, BCN_RING_SIZE * TXD_SIZE);
			if (pAd->BcnDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("BcnDescRing[%p]: total %d bytes allocated\n",
					pAd->BcnDescRing.AllocVa, (INT)pAd->BcnDescRing.AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->BcnDescRing.AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->BcnDescRing.AllocPa);
			RingBaseVa = pAd->BcnDescRing.AllocVa;

			/* Initialize Beacon Ring and associated buffer memory */
			for (index = 0; index < BCN_RING_SIZE; index++)
			{
				dma_cb = &pAd->BcnRing.Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init CTRL Ring Size, Va, Pa variables*/
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);

				/* Offset to next ring descriptor address*/
				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* link the pre-allocated TxBuf to TXD*/
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

				/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("Bcn Ring: total %d entry allocated\n", index));

			/*
				Allocate Tx ring descriptor's memory (BMC)
			*/
			desc_ring_alloc(pAd, &pAd->TxBmcDescRing, TX_RING_SIZE * TXD_SIZE);
			if (pAd->TxBmcDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			pDmaBuf = &pAd->TxBmcDescRing;
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("TxBmcDescRing[%p]: total %d bytes allocated\n",
					pDmaBuf->AllocVa, (INT)pDmaBuf->AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDmaBuf->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			RingBaseVa = pDmaBuf->AllocVa;

			/*
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			pAd->TxBmcBufSpace.AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				0,
				pAd->TxBmcBufSpace.AllocSize,
				FALSE,
				&pAd->TxBmcBufSpace.AllocVa,
				&pAd->TxBmcBufSpace.AllocPa);

			if (pAd->TxBmcBufSpace.AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pAd->TxBmcBufSpace.AllocVa, pAd->TxBmcBufSpace.AllocSize);

			/* Save PA & VA for further operation*/
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBmcBufSpace.AllocPa);
			BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBmcBufSpace.AllocPa);
			BufBaseVa = pAd->TxBmcBufSpace.AllocVa;

			/*
				Initialize Tx Ring Descriptor and associated buffer memory
			*/
			pTxRing = &pAd->TxBmcRing;
			for (index = 0; index < TX_RING_SIZE; index++)
			{
				dma_cb = &pTxRing->Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init Tx Ring Size, Va, Pa variables */
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

				/* Setup Tx Buffer size & address. only 802.11 header will store in this space*/
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
				RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

				/* link the pre-allocated TxBuf to TXD */
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->SDPtr0 = BufBasePaLow;
                pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* advance to next TxBuf address */
				BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}

			if (Status == NDIS_STATUS_RESOURCES)
				break;
		}

#ifdef CUT_THROUGH
		cut_through_init(&pAd->PktTokenCb, pAd);
#endif /* CUT_THROUGH */

#endif /* MT_MAC */

		for (num = 0; num < NUM_OF_RX_RING; num++)
		{
			UINT16 RxRingSize;
			UINT16 RxBufferSize;

			if (num == 0)
			{
				RxRingSize = RX_RING_SIZE;
				RxBufferSize = RX_BUFFER_AGGRESIZE;
			}
			else
			{
				RxRingSize = RX1_RING_SIZE;
				RxBufferSize = RX1_BUFFER_SIZE;
			}
			/* Alloc RxRingDesc memory except Tx ring allocated eariler */
			desc_ring_alloc(pAd, &pAd->PciHif.RxDescRing[num], RxRingSize * RXD_SIZE);
			
			if (pAd->PciHif.RxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RxDescRing[%p]: total %d bytes allocated\n",
						pAd->PciHif.RxDescRing[num].AllocVa, (INT)pAd->PciHif.RxDescRing[num].AllocSize));

			/* Initialize Rx Ring and associated buffer memory */
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->PciHif.RxDescRing[num].AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->PciHif.RxDescRing[num].AllocPa);
			RingBaseVa = pAd->PciHif.RxDescRing[num].AllocVa;
			for (index = 0; index < RxRingSize; index++)
			{
				dma_cb = &pAd->PciHif.RxRing[num].Cell[index];
				/* Init RX Ring Size, Va, Pa variables*/
				dma_cb->AllocSize = RXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

				/* Offset to next ring descriptor address */
				RingBasePaLow += RXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;

				/* Setup Rx associated Buffer size & allocate share memory*/
				pDmaBuf = &dma_cb->DmaBuf;
#ifdef CUT_THROUGH
                if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb))
    				pDmaBuf->AllocSize = RxBufferSize + sizeof(RX_BLK);
                else
#endif /* CUT_THROUGH */
				pDmaBuf->AllocSize = RxBufferSize;
#ifdef  CONFIG_WIFI_BUILD_SKB
				if (num == 0) {
					pPacket =
						RTMP_AllocateRxPacketBufferOnly(
						pAd,
					((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
						pDmaBuf->AllocSize,
						FALSE,
						&pDmaBuf->AllocVa,
						&pDmaBuf->AllocPa);
				}
				else {
#endif /* CONFIG_WIFI_BUILD_SKB */
					pPacket = RTMP_AllocateRxPacketBuffer(
					pAd,
					((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					pDmaBuf->AllocSize,
					FALSE,
					&pDmaBuf->AllocVa,
					&pDmaBuf->AllocPa);
#ifdef  CONFIG_WIFI_BUILD_SKB
				}
#endif /* CONFIG_WIFI_BUILD_SKB*/

				/* keep allocated rx packet */
				dma_cb->pNdisPacket = pPacket;
				if (pDmaBuf->AllocVa == NULL)
				{
					ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
					MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate RxRing's 1st buffer\n"));
					Status = NDIS_STATUS_RESOURCES;
					break;
				}

				/* Zero init this memory block*/
				NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

#ifdef CUT_THROUGH
                if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (num == 0))
                {
                    pDmaBuf->AllocVa += sizeof(RX_BLK);
                    pDmaBuf->AllocPa += sizeof(RX_BLK);
                    OS_PKT_RESERVE(pPacket, sizeof(RX_BLK));
                }

                if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (num == 0))
                    dma_cb->token_id = cut_through_rx_enq(pAd->PktTokenCb, pPacket, TOKEN_RX);
#endif /* CUT_THROUGH */

				/* Write RxD buffer address & allocated buffer length*/
				pRxD = (RXD_STRUC *)dma_cb->AllocVa;
				pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
				pRxD->SDL0 = RxBufferSize;
				pRxD->DDONE = 0;
#ifdef CUT_THROUGH
                if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (num == 0))
                    pRxD->SDP1 = cpu2le32((((UINT32)dma_cb->token_id) & 0x0000ffff));
#endif /* CUT_THROUGH */

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
			}

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("Rx[%d] Ring: total %d entry allocated\n", num, index));
		}
	}	while (FALSE);

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
	if (pAd->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	if (Status != NDIS_STATUS_SUCCESS)
	{
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	/*
		Initialize all transmit related software queues
	*/

	/* Init TX rings index pointer*/
	rtmp_tx_swq_init(pAd);
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		pAd->PciHif.TxRing[index].TxSwFreeIdx = 0;
		pAd->PciHif.TxRing[index].TxCpuIdx = 0;
	}

	/* init MGMT ring index pointer*/
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer*/
	pAd->CtrlRing.TxSwFreeIdx = 0;
	pAd->CtrlRing.TxCpuIdx = 0;
#endif /* CONFIG_ANDES_SUPPORT */

	/* Init RX Ring index pointer*/
	for(index = 0; index < NUM_OF_RX_RING; index++)
	{
		UINT16 RxRingSize;
		UINT16 RxBufferSize;

		if (index == 0)
		{
			RxRingSize = RX_RING_SIZE;
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		}
		else
		{
			RxRingSize = RX1_RING_SIZE;
			RxBufferSize = RX1_BUFFER_SIZE;
		}

		pAd->PciHif.RxRing[index].RxSwReadIdx = 0;
		pAd->PciHif.RxRing[index].RxCpuIdx = RxRingSize - 1;
		pAd->PciHif.RxRing[index].RxRingSize = RxRingSize;
		pAd->PciHif.RxRing[index].RxBufferSize = RxBufferSize;
	}

	pAd->PrivateInfo.TxRingFullCnt = 0;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}


VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	int index, num , j;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	RTMP_DMACB *dma_cb;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	rtmp_tx_swq_exit(pAd, WCID_ALL);

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->PciHif.TxRing[index];

		for (j=0; j< TX_RING_SIZE; j++)
		{
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
#ifdef CUT_THROUGH
				UINT8 Type;
                            if (CUT_THROUGH_TX_ENABL(pAd->PktTokenCb))
                                cut_through_tx_deq(pAd->PktTokenCb, pTxRing->Cell[j].token_id, &Type);
#endif /* CUT_THROUGH */

				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNdisPacket as NULL after clear*/
			pTxRing->Cell[j].pNdisPacket = NULL;
			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			if (pPacket)
			{
#if defined(MT7615) ||defined(MT7622)
                            if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
                                MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
                                                    ("%s():ERR!!7615 shall not has one TxD with 2 pkt buffer!\n",
                                                    __FUNCTION__));
                            }
#endif /* defined(MT7615) || defined(MT7622) */

				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}

#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		// TODO: shiang-MT7615, we don't need mgmt ring now!
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	{
		RTMP_MGMT_RING *pMgmtRing = &pAd->MgmtRing;
		NdisAcquireSpinLock(&pAd->MgmtRingLock);

		HIF_IO_READ32(pAd, pMgmtRing->hw_didx_addr, &pMgmtRing->TxDmaIdx);
		while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
	        pDestTxD = (TXD_STRUC *) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
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
			INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
		NdisReleaseSpinLock(&pAd->MgmtRingLock);
	}

#ifdef CONFIG_ANDES_SUPPORT
	{
		RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;

		NdisAcquireSpinLock(&pAd->CtrlRingLock);
		HIF_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
	        pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);
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
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, CTL_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
		NdisReleaseSpinLock(&pAd->CtrlRingLock);
	}
#endif /* CONFIG_ANDES_SUPPORT */

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		// TODO: shiang-MT7615, we don't need bcn ring now!
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	if (1 /*pAd->chipCap.hif_type == HIF_MT*/)
	{
		RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;
		RTMP_DMACB *dma_cell = &pAd->BcnRing.Cell[0];

        RTMP_SEM_LOCK(&pAd->BcnRingLock);
		//NdisAcquireSpinLock(&pAd->BcnRingLock);
		HIF_IO_READ32(pAd, pBcnRing->hw_didx_addr, &pBcnRing->TxDmaIdx);

		while (pBcnRing->TxSwFreeIdx!= pBcnRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *) (dma_cell[pBcnRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (dma_cell[pBcnRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = dma_cell[pBcnRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);
				continue;
			}

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			dma_cell[pBcnRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = dma_cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			dma_cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
        RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
		//NdisReleaseSpinLock(&pAd->BcnRingLock);

		/* Free Tx BMC Ring Packet*/
		pTxRing = &pAd->TxBmcRing;

		for (j=0; j< TX_RING_SIZE; j++)
		{
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNdisPacket as NULL after clear*/
			pTxRing->Cell[j].pNdisPacket = NULL;
			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}

    	if (pAd->TxBmcBufSpace.AllocVa)
		{
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxBmcBufSpace.AllocSize, FALSE, pAd->TxBmcBufSpace.AllocVa, pAd->TxBmcBufSpace.AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBmcBufSpace, sizeof(RTMP_DMABUF));

    	if (pAd->TxBmcDescRing.AllocVa)
		{
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxBmcDescRing.AllocSize, pAd->TxBmcDescRing.AllocVa, pAd->TxBmcDescRing.AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBmcDescRing, sizeof(RTMP_DMABUF));
	}

#endif /* MT_MAC */

	for (j = 0; j < NUM_OF_RX_RING; j++)
	{
		UINT16 RxRingSize;

		if (j == 0)
		{
			RxRingSize = RX_RING_SIZE;
		}
		else
		{
			RxRingSize = RX1_RING_SIZE;
		}


		for (index = RxRingSize - 1 ; index >= 0; index--)
		{
			dma_cb = &pAd->PciHif.RxRing[j].Cell[index];
			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket))
			{
#ifdef CUT_THROUGH
				UINT8 Type;
                            if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb) && (j == 0))
                                cut_through_rx_deq(pAd->PktTokenCb, dma_cb->token_id, &Type);
#endif /* CUT_THROUGH */

				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa,
									dma_cb->DmaBuf.AllocSize,
									RTMP_PCI_DMA_FROMDEVICE);
#ifdef  CONFIG_WIFI_BUILD_SKB
				if (j == 0) {
					DEV_FREE_FRAG_BUF(dma_cb->pNdisPacket);
				}
				else {
#endif /* CONFIG_WIFI_BUILD_SKB */
					RELEASE_NDIS_PACKET(pAd,
						dma_cb->pNdisPacket,
						NDIS_STATUS_SUCCESS);
#ifdef  CONFIG_WIFI_BUILD_SKB
				}
#endif /* CONFIG_WIFI_BUILD_SKB */
			}
		}

		NdisZeroMemory(pAd->PciHif.RxRing[j].Cell, RxRingSize * sizeof(RTMP_DMACB));

		if (pAd->PciHif.RxDescRing[j].AllocVa)
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
								pAd->PciHif.RxDescRing[j].AllocSize,
								pAd->PciHif.RxDescRing[j].AllocVa,
								pAd->PciHif.RxDescRing[j].AllocPa);

		NdisZeroMemory(&pAd->PciHif.RxDescRing[j], sizeof(RTMP_DMABUF));
	}

#ifdef MT_MAC
#ifdef CUT_THROUGH
	cut_through_deinit((PKT_TOKEN_CB **)&pAd->PktTokenCb);
#endif /* CUT_THROUGH */
#endif /* MT_MAC */

	if (pAd->MgmtDescRing.AllocVa)
	{
		RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->MgmtDescRing.AllocSize, pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->MgmtDescRing, sizeof(RTMP_DMABUF));

#ifdef CONFIG_ANDES_SUPPORT
	if (pAd->CtrlDescRing.AllocVa)
	{
		RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->CtrlDescRing.AllocSize, pAd->CtrlDescRing.AllocVa, pAd->CtrlDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->CtrlDescRing, sizeof(RTMP_DMABUF));

#if defined(MT7615) || defined(MT7622)
    if (pAd->FwDwloRing.DescRing.AllocVa)
    {
        RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->FwDwloRing.DescRing.AllocSize,
            pAd->FwDwloRing.DescRing.AllocVa, pAd->FwDwloRing.DescRing.AllocPa);
    }
    NdisZeroMemory(&pAd->FwDwloRing.DescRing, sizeof(RTMP_DMABUF));
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
    	if (pAd->PciHif.TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->PciHif.TxBufSpace[num].AllocSize, FALSE, pAd->PciHif.TxBufSpace[num].AllocVa, pAd->PciHif.TxBufSpace[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->PciHif.TxBufSpace[num], sizeof(RTMP_DMABUF));

    	if (pAd->PciHif.TxDescRing[num].AllocVa)
		{
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->PciHif.TxDescRing[num].AllocSize, pAd->PciHif.TxDescRing[num].AllocVa, pAd->PciHif.TxDescRing[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->PciHif.TxDescRing[num], sizeof(RTMP_DMABUF));
	}

	if (pAd->FragFrame.pFragPacket)
	{
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}

#endif /* RESOURCE_PRE_ALLOC */


VOID AsicInitTxRxRing(RTMP_ADAPTER *pAd)
{
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		rlt_asic_init_txrx_ring(pAd);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		rtmp_asic_init_txrx_ring(pAd);
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		mt_asic_init_txrx_ring(pAd);
#endif /* MT_MAC */
}


/*
	========================================================================

	Routine Description:
		Reset NIC Asics. Call after rest DMA. So reset TX_CTX_IDX to zero.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPRingCleanUp(RTMP_ADAPTER *pAd, UCHAR RingType)
{
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD, TxD;
	RXD_STRUC *pDestRxD, RxD;
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	RTMP_TX_RING *pTxRing;
	ULONG IrqFlags;
	int i, ring_id;


	/*
		We have to clean all descriptors in case some error happened with reset
	*/
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,("RTMPRingCleanUp(RingIdx=%d, Pending-NDIS=%ld)\n", RingType, pAd->RalinkCounters.PendingNdisPacketCount));
	switch (RingType)
	{
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
		case QID_HCCA:
			if (NUM_OF_TX_RING <= RingType)
			{
				break;
			}

			pTxRing = &pAd->PciHif.TxRing[RingType];

			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			for (i=0; i<TX_RING_SIZE; i++) /* We have to scan all TX ring*/
			{
				pTxD  = (TXD_STRUC *) pTxRing->Cell[i].AllocVa;

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNdisPacket = NULL;
				}

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
#if defined(MT7615) || defined(MT7622)
                                if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
                                        MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
                                                            ("%s():ERR!!7615 shall not has one TxD with 2 pkt buffer!\n",
                                                            __FUNCTION__));
                                }
#endif /* defined(MT7615) || defined(MT7622) */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNextNdisPacket = NULL;
				}
			}
			HIF_IO_READ32(pAd,pTxRing->hw_didx_addr,&pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			HIF_IO_WRITE32(pAd,pTxRing->hw_cidx_addr,pTxRing->TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

			/* TODO: need to check */
			//rtmp_tx_swq_exit(pAd, WCID_ALL);
			break;

		case QID_MGMT:
			RTMP_IRQ_LOCK(&pAd->MgmtRingLock, IrqFlags);
			for (i=0; i<MGMT_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *) pAd->MgmtRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->MgmtRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->MgmtRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
				pAd->MgmtRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

 			HIF_IO_READ32(pAd, pAd->MgmtRing.hw_didx_addr, &pAd->MgmtRing.TxDmaIdx);
			pAd->MgmtRing.TxSwFreeIdx = pAd->MgmtRing.TxDmaIdx;
			pAd->MgmtRing.TxCpuIdx = pAd->MgmtRing.TxDmaIdx;
			HIF_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr, pAd->MgmtRing.TxCpuIdx);

 			RTMP_IRQ_UNLOCK(&pAd->MgmtRingLock, IrqFlags);
			pAd->RalinkCounters.MgmtRingFullCount = 0;
			break;

		case QID_RX:
			for (ring_id = 0; ring_id < NUM_OF_RX_RING; ring_id++)
			{
				RTMP_RX_RING *pRxRing;
				NDIS_SPIN_LOCK *lock;
				UINT16 RxRingSize;
				UINT16 RxBufferSize;

				pRxRing = &pAd->PciHif.RxRing[ring_id];
				lock = &pAd->RxRingLock[ring_id];

				if (ring_id == 0)
				{
					RxRingSize = RX_RING_SIZE;
					RxBufferSize = RX_BUFFER_AGGRESIZE;
				}
				else
				{
					RxRingSize = RX1_RING_SIZE;
					RxBufferSize = RX1_BUFFER_SIZE;
				}

				RTMP_IRQ_LOCK(lock, IrqFlags);
				for (i = 0; i < RxRingSize; i++)
				{
#ifdef RT_BIG_ENDIAN
					pDestRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
					RxD = *pDestRxD;
					pRxD = &RxD;
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
					/* Point to Rx indexed rx ring descriptor*/
					pRxD  = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

					pRxD->DDONE = 0;
					pRxD->SDL0 = RxBufferSize;

#ifdef RT_BIG_ENDIAN
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
					WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif /* RT_BIG_ENDIAN */
				}

				HIF_IO_READ32(pAd, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
				pRxRing->RxSwReadIdx = pRxRing->RxDmaIdx;
				pRxRing->RxCpuIdx = ((pRxRing->RxDmaIdx == 0) ? (RxRingSize - 1) : (pRxRing->RxDmaIdx-1));
				HIF_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);

				RTMP_IRQ_UNLOCK(lock, IrqFlags);
			}
			break;

#ifdef CONFIG_ANDES_SUPPORT
		case QID_CTRL:
			RTMP_IRQ_LOCK(&pAd->CtrlRingLock, IrqFlags);

			for (i=0; i < CTL_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *) pAd->CtrlRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->CtrlRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->CtrlRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->CtrlRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->CtrlRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->CtrlRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			HIF_IO_READ32(pAd, pAd->CtrlRing.hw_didx_addr, &pAd->CtrlRing.TxDmaIdx);
			pAd->CtrlRing.TxSwFreeIdx = pAd->CtrlRing.TxDmaIdx;
			pAd->CtrlRing.TxCpuIdx = pAd->CtrlRing.TxDmaIdx;
			HIF_IO_WRITE32(pAd, pAd->CtrlRing.hw_cidx_addr, pAd->CtrlRing.TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->CtrlRingLock, IrqFlags);
			break;
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
		case QID_BCN:
#if defined(MT7615) || defined(MT7622)
			if (IS_MT7615(pAd) || IS_MT7622(pAd))
			{
				// TODO: shiang-MT7615, we don't need bcn ring now!
				break;
			}
#endif /* defined(MT7615) || defined(MT7622) */

			RTMP_IRQ_LOCK(&pAd->BcnRingLock, IrqFlags);
			for (i = 0; i < BCN_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *)pAd->BcnRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->BcnRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->BcnRing.Cell[i].pNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					//RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->BcnRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->BcnRing.Cell[i].pNextNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
					//RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->BcnRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			HIF_IO_READ32(pAd, pAd->BcnRing.hw_didx_addr, &pAd->BcnRing.TxDmaIdx);
			pAd->BcnRing.TxSwFreeIdx = pAd->BcnRing.TxDmaIdx;
			pAd->BcnRing.TxCpuIdx = pAd->BcnRing.TxDmaIdx;
			HIF_IO_WRITE32(pAd, pAd->BcnRing.hw_cidx_addr, pAd->BcnRing.TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->BcnRingLock, IrqFlags);

			break;
		case QID_BMC:

			for (i = 0; i < TX_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *)pAd->TxBmcRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->TxBmcRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->TxBmcRing.Cell[i].pNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->TxBmcRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->TxBmcRing.Cell[i].pNextNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->TxBmcRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			HIF_IO_READ32(pAd, pAd->TxBmcRing.hw_didx_addr, &pAd->TxBmcRing.TxDmaIdx);
			pAd->TxBmcRing.TxSwFreeIdx = pAd->TxBmcRing.TxDmaIdx;
			pAd->TxBmcRing.TxCpuIdx = pAd->TxBmcRing.TxDmaIdx;
			HIF_IO_WRITE32(pAd, pAd->TxBmcRing.hw_cidx_addr, pAd->TxBmcRing.TxCpuIdx);

			break;
#endif /* MT_MAC */

		default:
			break;
	}
}


VOID DumpPseInfo(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;
	UINT32 Index;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);

	/* PSE Infomation */
	for (Index = 0; Index < 30720; Index++)
	{
		RemapBase = GET_REMAP_2_BASE(0xa5000000 + Index * 4) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa5000000 + Index * 4);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("Offset[0x%x] = 0x%x\n", 0xa5000000 + Index * 4, Value));
	}

	/* Frame linker */
	for (Index  = 0; Index < 1280; Index++)
	{
		RemapBase = GET_REMAP_2_BASE(0xa00001b0) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b0);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~0xfff;
		Value |= (Index & 0xfff);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

		RemapBase = GET_REMAP_2_BASE(0xa00001b4) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b4);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("Frame Linker(0x%x) = 0x%x\n", Index, Value ));

	}

	/* Page linker */
	for (Index = 0; Index < 1280; Index++)
	{
		RemapBase = GET_REMAP_2_BASE(0xa00001b8) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b8);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~(0xfff << 16);
		Value |= ((Index & 0xfff) << 16);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

		RemapBase = GET_REMAP_2_BASE(0xa00001b8) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b8);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("Page Linker(0x%x) = 0x%x\n", Index, (Value & 0xfff)));
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
}



/***************************************************************************
  *
  *	register related procedures.
  *
  **************************************************************************/
/*
========================================================================
Routine Description:
    Disable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMADisable(RTMP_ADAPTER *pAd)
{
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
}


/*
========================================================================
Routine Description:
    Enable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMAEnable(RTMP_ADAPTER *pAd)
{
	AsicWaitPDMAIdle(pAd, 200, 1000);

	RtmpusecDelay(50);

	AsicSetWPDMA(pAd, PDMA_TX_RX, TRUE);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<== %s(): WPDMABurstSIZE = %d\n", __FUNCTION__, pAd->chipCap.WPDMABurstSIZE));
}


BOOLEAN AsicCheckCommanOk(RTMP_ADAPTER *pAd, UCHAR Command)
{
	BOOLEAN status = FALSE;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return FALSE;
	}




	return status;
}


#ifdef MT_MAC
VOID MtUpdateBeaconToAsic(
    IN RTMP_ADAPTER     *pAd,
    IN VOID             *wdev_void,
    IN ULONG            FrameLen,
    IN ULONG            UpdatePos,
    IN UCHAR            UpdatePktType)
{
    struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BCN_BUF_STRUC *bcn_buf = NULL;
	UCHAR *buf;
	INT len;
	PNDIS_PACKET pkt = NULL;
#if defined(MT7615) || defined(MT7622)
	PNDIS_PACKET bcn_pkt = NULL;
#endif
	UINT32 WdevIdx = 0;
    USHORT FreeNum = 0;
    NDIS_SPIN_LOCK *ring_lock = &pAd->BcnRingLock;
    ULONG irqFlags;

#if defined(MT7615) || defined(MT7622)
    UCHAR qIdx = 0;
    NDIS_STATUS Status;
#endif /* defined(MT7615) || defined(MT7622) */

    ASSERT(wdev != NULL);
	if (!wdev) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): wdev is NULL!\n", __FUNCTION__));
		return;
	}
    bcn_buf = &wdev->bcn_buf;
    WdevIdx = wdev->wdev_idx;

	if (!bcn_buf) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): bcn_buf is NULL!\n", __FUNCTION__));
		return;
	}

	if (bcn_buf->BeaconPkt) {
#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
            ring_lock = &pAd->irq_lock;
			Status = RTMPAllocateNdisPacket(pAd, &bcn_pkt, NULL, 0,
										GET_OS_PKT_DATAPTR(bcn_buf->BeaconPkt),
										(UINT)(FrameLen+pAd->chipCap.tx_hw_hdr_len));
			if (Status != NDIS_STATUS_SUCCESS) {
				printk("%s():Cannot alloc bcn pkt buf!\n", __FUNCTION__);
				return;
			}
			pkt = bcn_pkt;
		}
		else
#endif /* defined(MT7615) || defined(MT7622) */
		{
			pkt = bcn_buf->BeaconPkt;
		}

		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + pAd->chipCap.tx_hw_hdr_len;

		SET_OS_PKT_LEN(pkt, len);
#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif /* RT_BIG_ENDIAN */

		RTMP_SET_PACKET_WDEV(pkt,WdevIdx);

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			UCHAR RingIdx = HcGetTxRingIdx(pAd,wdev);
            qIdx = HcGetBcnQueueIdx(pAd,wdev);

            RTMP_IRQ_LOCK(ring_lock, irqFlags);
            /*
                Carter, MT7615 use tx_ring to send bcn,
                ugly check RingFreeNum here.
            */
            FreeNum = GET_BCNRING_FREENO(pAd,RingIdx);
            if (FreeNum <= 0) {
                MTWF_LOG(
                    DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s(): FreeNum is not enough\n",
                        __FUNCTION__)
                );
                RTMP_IRQ_UNLOCK(ring_lock, irqFlags);
                //Free pkt here, due to lack of ring resource.
                RELEASE_NDIS_PACKET(pAd, bcn_pkt, NDIS_STATUS_FAILURE);
                return;
            }

			HAL_KickOutMgmtTx(pAd, qIdx, pkt, buf, len);
			RTMP_IRQ_UNLOCK(ring_lock, irqFlags);
		}
#else
		{
            RTMP_IRQ_LOCK(ring_lock, irqFlags);

            FreeNum = GET_BCNRING_FREENO(pAd, 0);
            if (FreeNum <= 0) {
                MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                                ("%s(): FreeNum is not enough\n", __FUNCTION__));
                RTMP_IRQ_UNLOCK(ring_lock, irqFlags);
                return;
            }

			/* Now do hardware-depened kick out.*/
			HAL_KickOutMgmtTx(pAd, Q_IDX_BCN, pkt, buf, len);
			bcn_buf->bcn_state = BCN_TX_WRITE_TO_DMA;
			RTMP_IRQ_UNLOCK(ring_lock, irqFlags);
		}
#endif /* defined(MT7615) || defined(MT7622) */
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): BeaconPkt is NULL!\n", __FUNCTION__));
	}
}
#endif /* MT_MAC */


#if defined(RTMP_MAC) || defined(RLT_MAC)
/*
========================================================================
Routine Description:
    Write Beacon buffer to Asic.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtUpdateBeaconToAsic(
    IN RTMP_ADAPTER     *pAd,
    IN struct wifi_dev  *wdev,
    IN ULONG            FrameLen,
    IN ULONG            UpdatePos,
    IN UCHAR            UpdatePktType)
{
	ULONG CapInfoPos = 0;
	UCHAR *ptr, *ptr_update, *ptr_capinfo;
	UINT i;
	BOOLEAN bBcnReq = FALSE;
	UCHAR bcn_idx = 0;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	INT wr_bytes = 1;
	UCHAR *pBeaconFrame, *tmac_info;
	UCHAR tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;

	if (IS_MT76x2(pAd))
		wr_bytes = 4;

#ifdef CONFIG_AP_SUPPORT
	if (apidx < pAd->ApCfg.BssidNum &&
		apidx < HW_BEACON_MAX_NUM &&
	       (pAd->OpMode == OPMODE_AP)
		)
	{
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[apidx];
		bcn_idx = pMbss->wdev.bcn_buf.BcnBufIdx;
		CapInfoPos = pMbss->wdev.bcn_buf.cap_ie_pos;
//		bBcnReq = BeaconTransmitRequired(pAd, apidx, pMbss);
        bBcnReq = BeaconTransmitRequired(pAd, &pMbss->wdev);

		if (wr_bytes > 1) {
			CapInfoPos = (CapInfoPos & (~(wr_bytes - 1)));
			UpdatePos = (UpdatePos & (~(wr_bytes - 1)));
		}

		tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->wdev.bcn_buf.BeaconPkt);
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);

		ptr_capinfo = (PUCHAR)(pBeaconFrame + CapInfoPos);
		ptr_update  = (PUCHAR)(pBeaconFrame + UpdatePos);
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s():Invalid Interface\n", __FUNCTION__));
		return;
	}

	if (pAd->BeaconOffset[bcn_idx] == 0) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s():Invalid BcnOffset[%d]\n",
					__FUNCTION__, bcn_idx));
		return;
	}

	if (bBcnReq == FALSE)
	{
		/* when the ra interface is down, do not send its beacon frame */
		/* clear all zero */
		for(i=0; i < TXWISize; i+=4)
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + i, 0x00, 4);
	}
	else
	{
		ptr = tmac_info;

#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif
		for (i=0; i < TXWISize; i+=4)  /* 16-byte TXWI field*/
		{
			UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + i, longptr, 4);
			ptr += 4;
		}

		/* Update CapabilityInfo in Beacon*/
		for (i = CapInfoPos; i < (CapInfoPos+2); i += wr_bytes)
		{
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *((UINT32 *)ptr_capinfo), wr_bytes);
			ptr_capinfo += wr_bytes;
		}

		if (FrameLen > UpdatePos)
		{
			for (i = UpdatePos; i < (FrameLen); i += wr_bytes)
			{
				UINT32 longptr =  *ptr_update + (*(ptr_update+1)<<8) + (*(ptr_update+2)<<16) + (*(ptr_update+3)<<24);
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, longptr, wr_bytes);
				ptr_update += wr_bytes;
			}
		}
	}
}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


VOID RT28xx_UpdateBeaconToAsic(
    IN RTMP_ADAPTER     *pAd,
    IN VOID             *wdev_void,
    IN ULONG            FrameLen,
    IN ULONG            UpdatePos,
    IN UCHAR            UpdatePktType)
{
#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT)
    {
        MtUpdateBeaconToAsic(pAd, wdev_void, FrameLen, UpdatePos, UpdatePktType);
        return;
    }
    else
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
    {
        RtUpdateBeaconToAsic(pAd, wdev_void, FrameLen, UpdatePos, UpdatePktType);
        return;
    }
#endif
    return;
}



/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to wake up mode from power save mode.
		Both RadioOn and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF : call this function is from Radio Off to Radio On.  Need to restore PCI host value.
		Level = other value : normal wake up function.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOn(RTMP_ADAPTER *pAd, UCHAR Level)
{

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;


	/* 2. Send wake up command.*/
	AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);
	pAd->bPCIclkOff = FALSE;
	/* 2-1. wait command ok.*/
	AsicCheckCommanOk(pAd, PowerWakeCID);
    	RTMP_ASIC_INTERRUPT_ENABLE(pAd);

    	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    	if (Level == GUI_IDLE_POWER_SAVE)
    	{
 /*2009/06/09: AP and stations need call the following function*/
 			/* add by johnli, RF power sequence setup, load RF normal operation-mode setup*/

		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

		if (pChipOps->AsicReverseRfFromSleepMode)
		{
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);

		}
		else
		{
	    		/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			}
#endif /* CONFIG_AP_SUPPORT */
	}
    	}
        return TRUE;

}


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to power save mode.
		Both RadioOff and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF  : GUI Radio Off mode
		Level = DOT11POWERSAVE  : 802.11 power save mode
		Level = RTMP_HALT  : When Disable device.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOff(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Level,
	IN USHORT TbttNumToNextWakeUp)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
    UINT32 RxDmaIdx, RxCpuIdx;
#endif
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

#if defined(RTMP_MAC) || defined(RLT_MAC)
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s ===> Lv= %d, TxCpuIdx = %d, TxDmaIdx = %d. RxCpuIdx = %d, RxDmaIdx = %d.\n",
					__FUNCTION__, Level,pAd->PciHif.TxRing[0].TxCpuIdx, pAd->PciHif.TxRing[0].TxDmaIdx,
					pAd->PciHif.RxRing[0].RxCpuIdx, pAd->PciHif.RxRing[0].RxDmaIdx));

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

	if (Level == DOT11POWERSAVE)
	{
		/* Check Rx DMA busy status, if more than half is occupied, give up this radio off.*/
		HIF_IO_READ32(pAd, pAd->PciHif.RxRing[0].hw_didx_addr, &RxDmaIdx);
		HIF_IO_READ32(pAd, pAd->PciHif.RxRing[0].hw_cidx_addr, &RxCpuIdx);
		if ((RxDmaIdx > RxCpuIdx) && ((RxDmaIdx - RxCpuIdx) > RX_RING_SIZE/3))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s(): return1. RxDmaIdx=%d, RxCpuIdx=%d\n",
						__FUNCTION__, RxDmaIdx, RxCpuIdx));
			return FALSE;
		}
		else if ((RxCpuIdx >= RxDmaIdx) && ((RxCpuIdx - RxDmaIdx) < RX_RING_SIZE/3))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s(): return2. RxDmaIdx=%d, RxCpuIdx=%d\n",
						__FUNCTION__, RxDmaIdx, RxCpuIdx));
			return FALSE;
		}
	}

    /* Once go into this function, disable tx because don't want too many packets in queue to prevent HW stops.*/
	/*pAd->bPCIclkOffDisableTx = TRUE;*/
	/*pAd->bPCIclkOffDisableTx = FALSE;*/
    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);


	/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
	{
		UCHAR Channel =HcGetRadioChannel(pAd);
		AsicTurnOffRFClk(pAd, Channel);
	}

	if (Level != RTMP_HALT)
	{
		UINT32 AutoWakeupInt = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			AutoWakeupInt = RLT_AutoWakeupInt;
#endif /* RLT_MAC*/
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			AutoWakeupInt = RTMP_AutoWakeupInt;
#endif /* RTMP_MAC */
		/*
			Change Interrupt bitmask.
			When PCI clock is off, don't want to service interrupt.
		*/
		HIF_IO_WRITE32(pAd, INT_MASK_CSR, AutoWakeupInt);
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}

	HIF_IO_WRITE32(pAd, pAd->PciHif.RxRing[0].hw_cidx_addr, pAd->PciHif.RxRing[0].RxCpuIdx);
	/*  2. Send Sleep command */
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
	/* send POWER-SAVE command to MCU. high-byte = 1 save power as much as possible. high byte = 0 save less power*/
	AsicSendCommandToMcu(pAd, SLEEP_MCU_CMD, PowerSafeCID, 0xff, 0x1, FALSE);



/* Disable for stability. If PCIE Link Control is modified for advance power save, re-covery this code segment.*/
/*RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x1280);*/
/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);*/

	if (Level == DOT11POWERSAVE)
	{
		AUTO_WAKEUP_STRUC	AutoWakeupCfg;
		/*RTMPSetTimer(&pAd->Mlme.PsPollTimer, 90);*/

		/* we have decided to SLEEP, so at least do it for a BEACON period.*/
		if (TbttNumToNextWakeUp == 0)
			TbttNumToNextWakeUp = 1;

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

		/* 1. Set auto wake up timer.*/
		AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;
		AutoWakeupCfg.field.EnableAutoWakeup = 1;
		AutoWakeupCfg.field.AutoLeadTime = LEAD_TIME;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	}


    	/*pAd->bPCIclkOffDisableTx = FALSE;*/

#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	return TRUE;
}


/*
========================================================================
Routine Description:
	Get a pci map buffer.

Arguments:
	pAd				- WLAN control block pointer
	*ptr			- Virtual address or TX control block
	size			- buffer size
	sd_idx			- 1: the ptr is TX control block
	direction		- RTMP_PCI_DMA_TODEVICE or RTMP_PCI_DMA_FROMDEVICE

Return Value:
	the PCI map buffer

Note:
========================================================================
*/
ra_dma_addr_t RtmpDrvPciMapSingle(
	IN RTMP_ADAPTER *pAd,
	IN VOID *ptr,
	IN size_t size,
	IN INT sd_idx,
	IN INT direction)
{
	ra_dma_addr_t SrcBufPA = 0;

	if (sd_idx == 1)
	{
		TX_BLK *pTxBlk = (TX_BLK *)(ptr);

		if (pTxBlk->SrcBufLen && !pTxBlk->DmaMapping)
		{
			SrcBufPA = linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, 0, direction);
		
			pTxBlk->DmaMapping = TRUE;
			pTxBlk->SrcBufPA = SrcBufPA;
		}
		else if (pTxBlk->SrcBufLen && pTxBlk->DmaMapping)
		{
			return pTxBlk->SrcBufPA;
		}
		else
		{
			return (ra_dma_addr_t)0x0;
		}
	}
	else
	{
		SrcBufPA = linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					ptr, size, 0, direction);
	}


	if (dma_mapping_error(&((POS_COOKIE)(pAd->OS_Cookie))->pci_dev->dev, SrcBufPA))
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s: dma mapping error\n", __FUNCTION__));
		return (ra_dma_addr_t)0x0;
	}
	 else
	{
		return SrcBufPA;
	}
}


int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value)
{
	// TODO: shiang-7603
	if (ad->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	if (base == 0x40)
		RTMP_IO_WRITE32(ad, 0x10000 + offset, value);
	else if (base == 0x41)
		RTMP_IO_WRITE32(ad, offset, value);
	else
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("illegal base = %x\n", base));

	return 0;
}


int read_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 *value)
{
	// TODO: shiang-7603
	if (ad->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	if (base == 0x40) {
		RTMP_IO_READ32(ad, 0x10000 + offset, value);
	} else if (base == 0x41) {
		RTMP_IO_READ32(ad, offset, value);
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("illegal base = %x\n", base));
	}
	return 0;
}


INT rtmp_irq_init(RTMP_ADAPTER *pAd)
{
	unsigned long irqFlags;
	UINT32 reg_mask = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		reg_mask = (RLT_DELAYINTMASK) |(RLT_RxINT|RLT_TxDataInt|RLT_TxMgmtInt);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		reg_mask = ((RTMP_DELAYINTMASK) |(RTMP_RxINT|RTMP_TxDataInt|RTMP_TxMgmtInt)) & ~(0x03);
#endif /* RTMP_MAC */

#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			reg_mask = (MT_CoherentInt | MT_MacInt | MT_INT_RX_DLY | MT_INT_T2_DONE | MT_INT_T3_DONE);
#if defined(ERR_RECOVERY) || defined(CONFIG_FWOWN_SUPPORT)
	            reg_mask |= MT_McuCommand;
#endif /* ERR_RECOVERY || CONFIG_FWOWN_SUPPORT */	
            reg_mask |= MT_FW_CLR_OWN_INT;
		}
		else
#endif /* defined(MT7615) || defined(MT7622) */
		reg_mask = ((MT_DELAYINTMASK) |(MT_RxINT|MT_TxDataInt|MT_TxMgmtInt)|MT_INT_BMC_DLY);
	}
#endif /* MT_MAC */

	RTMP_INT_LOCK(&pAd->irq_lock, irqFlags);
	pAd->PciHif.IntEnableReg = reg_mask;
#if (CFG_CPU_LOADING_RXRING_DLY == 1)
    pAd->PciHif.IntEnableReg &= ~(MT_INT_RX_DATA | MT_INT_RX_CMD);
    pAd->PciHif.IntEnableReg |= MT_INT_RX_DLY;
#endif
#if (CFG_CPU_LOADING_DISABLE_TXDONE0 == 1)
	pAd->PciHif.IntEnableReg &= ~MT_INT_AC0_DLY;
#endif
	pAd->PciHif.intDisableMask = 0;
	pAd->PciHif.IntPending = 0;
	RTMP_INT_UNLOCK(&pAd->irq_lock, irqFlags);

#ifdef INT_STATISTIC

	pAd->INTCNT = 0;
#ifdef MT_MAC
	pAd->INTWFMACINT0CNT = 0;
	pAd->INTWFMACINT1CNT = 0;
	pAd->INTWFMACINT2CNT = 0;
	pAd->INTWFMACINT3CNT = 0;
	pAd->INTWFMACINT4CNT = 0;
	pAd->INTBCNDLY = 0;
	pAd->INTBMCDLY = 0;
#endif
	pAd->INTTxCoherentCNT = 0;
	pAd->INTRxCoherentCNT = 0;
	pAd->INTFifoStaFullIntCNT = 0;
	pAd->INTMGMTDLYCNT =0;
	pAd->INTRXDATACNT =0;
	pAd->INTRXCMDCNT =0;
	pAd->INTHCCACNT =0;
	pAd->INTAC3CNT =0;
	pAd->INTAC2CNT =0;
	pAd->INTAC1CNT =0;
	pAd->INTAC0CNT =0;

	pAd->INTPreTBTTCNT =0;
	pAd->INTTBTTIntCNT =0;
	pAd->INTGPTimeOutCNT =0;
	pAd->INTAutoWakeupIntCNT =0;
#endif

	return 0;
}

#ifdef CONFIG_FWOWN_SUPPORT
#define	FW_OWN_POLLING_COUNTER	300
INT32 MtAcquirePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset)
{
	ULONG Flags = 0;
	INT32 Ret = 0;
	UINT32 Counter = 0;
           
	if (Offset == HIF_FUN_CAP) 
	{
		// These registers are accessible when Low Power 
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Access HIF_FUN_CAP, return\n", __FUNCTION__));	
		return 1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);

	pAd->bCRAccessing++;

	if (pAd->bDrvOwn)
	{
		// all registers are accessible
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Current is DrverOnw, return\n", __FUNCTION__));	
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);

	/* Write any value to HIF_SYS_REV clear FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wirte any value to HIF_SYS_REV to clear FW own\n",__FUNCTION__));
	HIF_IO_WRITE32(pAd, HIF_SYS_REV, 1);

	/* Poll driver own status */
	while (Counter < FW_OWN_POLLING_COUNTER)	
	{
		RtmpusecDelay(100);
		if (pAd->bDrvOwn == TRUE)
			break;						
		Counter++;		
	};

	if (pAd->bDrvOwn)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Successed to clear FW own\n", __FUNCTION__));	
		Ret = 1;
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Faile to clear FW own\n", __FUNCTION__));
		Ret = 0;
		RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);
		pAd->bCRAccessing--;	//will not continue accessing HW
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
	}


	return Ret;
}


// Drv give up own
void MtReleasePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset)
{
	ULONG Flags = 0;
    	
	if (Offset == HIF_FUN_CAP) 
	{
		// These registers are accessible when Low Power 
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Access HIF_FUN_CAP, return\n", __FUNCTION__));	
		return;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);
	pAd->bCRAccessing--;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
                    
	return;
}

VOID FwOwn(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7637)
#ifdef MT7615
	if(MTK_REV_GTE(pAd, MT7615, MT7615E1) && MTK_REV_LT(pAd, MT7615, MT7615E3))
	{
		/* Wirte any value to HIF_FUN_CAP to set FW own */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wirte any value to HIF_FUN_CAP to set FW own\n",__FUNCTION__));
		HIF_IO_WRITE32(pAd, HIF_FUN_CAP, 1);
		pAd->bDrvOwn = FALSE;
	}
	else
#endif /* MT7615 */
	{
#ifdef MT7615
		if (pAd->bDrvOwn == FALSE)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Return since already in Fw Own...\n",__FUNCTION__));
			return;
		}

		HIF_IO_WRITE32(pAd, MT_CFG_LPCR_HOST, MT_HOST_SET_OWN);
		pAd->bDrvOwn = FALSE;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Set Fw Own\n",__FUNCTION__));
#endif /* MT7615 */
	}
#endif /* MT7615 || MT7637 */

#ifdef MT7622

	/* Wirte any value to HIF_FUN_CAP to set FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wirte any value to HIF_FUN_CAP to set FW own\n",__FUNCTION__));
	HIF_IO_WRITE32(pAd, HIF_FUN_CAP, 1);
	pAd->bDrvOwn = FALSE;

	return;
#endif /* MT7622 */
}
#ifdef MT7615
static VOID n9_wdt_reset(RTMP_ADAPTER *pAd)
{
#define WDT_SWRST_CR_PA 0x81080044
#define MCU_POWER_ON 0x01
#define HOST_TRIGGER_WDT_SWRST 0x1209

    UINT32 top_clock_gen0_value = 0;
    UINT32 top_clock_gen1_value = 0;
    UINT32 top_misc_value = 0;
    UINT32 origonal_remap_cr_value = 0;
    UINT32 remap_cr_record_base_address = 0;
    UINT32 offset_between_target_and_remap_cr_base = 0;

    /* switch hclk to XTAL source, 0x80021100[1:0] = 2'b00 */
    HW_IO_READ32(pAd, TOP_CKGEN0, &top_clock_gen0_value);
    top_clock_gen0_value &= ~(BIT0 | BIT1);
    HW_IO_WRITE32(pAd, TOP_CKGEN0, top_clock_gen0_value);

    /* Set HCLK divider to 1:1, 0x80021104[1:0] = 2'b00 */
    HW_IO_READ32(pAd, TOP_CKGEN1, &top_clock_gen1_value);
    top_clock_gen1_value &= ~(BIT0 | BIT1);
    HW_IO_WRITE32(pAd, TOP_CKGEN1, top_clock_gen1_value);

    /* disable HIF can be reset by WDT 0x80021130[30]=1'b0 */
    HW_IO_READ32(pAd, TOP_MISC, &top_misc_value);
    top_misc_value &= ~(BIT30);
    HW_IO_WRITE32(pAd, TOP_MISC, top_misc_value);

    /* enable WDT reset mode and trigger WDT reset 0x81080044 = 0x1209 */
    /* keep the origonal remap cr1 value for restore */
    HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &origonal_remap_cr_value);
    /* do PCI-E remap for CR4 PDMA physical base address to 0x40000 */
    HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, WDT_SWRST_CR_PA);

    HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &remap_cr_record_base_address);

    if ((WDT_SWRST_CR_PA  - remap_cr_record_base_address) > REMAP_1_OFFSET_MASK)
{
        /* restore the origonal remap cr1 value */
        HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
        
    }
    offset_between_target_and_remap_cr_base =
        ((WDT_SWRST_CR_PA  - remap_cr_record_base_address) & REMAP_1_OFFSET_MASK);

    RTMP_IO_WRITE32(pAd, MT_PCI_REMAP_ADDR_1 + offset_between_target_and_remap_cr_base,
                                                        HOST_TRIGGER_WDT_SWRST);
    /* restore the origonal remap cr1 value */
    HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::N9 WDT reset down\n", __FUNCTION__));
}
#endif

INT32 DriverOwn(RTMP_ADAPTER *pAd)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
#if defined(MT7615) || defined(MT7637)
#ifdef MT7615
#define MAX_RETRY_CNT 4

	UINT32 retrycnt = 0;

	if(MTK_REV_GTE(pAd, MT7615, MT7615E1) && MTK_REV_LT(pAd, MT7615, MT7615E3))
	{
		UINT32		Counter = 0;

		/* Write any value to HIF_SYS_REV clear FW own */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Wirte any value to HIF_SYS_REV to clear FW own\n",__FUNCTION__));
		HIF_IO_WRITE32(pAd, HIF_SYS_REV, 1);

		/* Poll driver own status */
		while (Counter < FW_OWN_POLLING_COUNTER)	
		{
			RtmpusecDelay(1000);
			if (pAd->bDrvOwn == TRUE)
				break;						
			Counter++;		
		};

		if (pAd->bDrvOwn)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Successed to clear FW own\n", __FUNCTION__));			
		}
		else
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Fail to clear FW own (%d)\n", __FUNCTION__, Counter));
			Ret = NDIS_STATUS_FAILURE;
		}		
	}
	else
#endif /* MT7615 */
	{
		UINT32 counter=0;
		UINT32 Value;


#ifdef MT7615
		do {
			retrycnt++;
		if (pAd->bDrvOwn == TRUE)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Return since already in Driver Own...\n",__FUNCTION__));
				return Ret;
		}
		
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Try to Clear FW Own...\n",__FUNCTION__));
		/* Write CR to get driver own */
		HIF_IO_WRITE32(pAd, MT_CFG_LPCR_HOST, MT_HOST_CLR_OWN);

		/* Poll driver own status */
		counter = 0;
		while (counter < FW_OWN_POLLING_COUNTER)	
		{
			RtmpusecDelay(1000);
			if (pAd->bDrvOwn == TRUE)
				break;						
			counter++;		
		};

		if (counter == FW_OWN_POLLING_COUNTER)
		{
			HIF_IO_READ32(pAd, MT_CFG_LPCR_HOST, &Value);

			if (!(Value & MT_HOST_SET_OWN))
				pAd->bDrvOwn = TRUE;
		}

		if (pAd->bDrvOwn)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Success to clear FW Own\n", __FUNCTION__));			
		}
		else
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Fail to clear FW Own (%d)\n", __FUNCTION__, counter));
				if (retrycnt >= MAX_RETRY_CNT)
					Ret = NDIS_STATUS_FAILURE;
				else
					n9_wdt_reset(pAd);
		}
		} while (pAd->bDrvOwn == FALSE && retrycnt < MAX_RETRY_CNT);
#endif /* MT7615 */
	}
#endif /* MT7615 || MT7637 */	

#ifdef MT7622
	UINT32		Counter = 0;

	/* Write any value to HIF_SYS_REV clear FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Wirte any value to HIF_SYS_REV to clear FW own\n",__FUNCTION__));
	HIF_IO_WRITE32(pAd, HIF_SYS_REV, 1);

	/* Poll driver own status */
	while (Counter < FW_OWN_POLLING_COUNTER)	
	{
		RtmpusecDelay(1000);
		if (pAd->bDrvOwn == TRUE)
			break;						
		Counter++;		
	};

	if (pAd->bDrvOwn)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Successed to clear FW own\n", __FUNCTION__));			
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Fail to clear FW own (%d)\n", __FUNCTION__, Counter));
		Ret = NDIS_STATUS_FAILURE;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	return Ret;
}

#if defined(MT7615) || defined(MT7622)
INT32 MakeFWOwn(RTMP_ADAPTER *pAd)
{
	ULONG Flags = 0;
	INT32 Ret = 0;
	UINT32 Value;
	UINT32 retry = 100;

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);

	if (!pAd->bDrvOwn)
	{
		// It is already FW own, do nothing
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()::Current is FwOwn, return\n", __FUNCTION__));			
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}
    
	if (pAd->bSetFWOwnRunning)
	{
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}

	pAd->bSetFWOwnRunning = 1;

	//Make sure no CR access and enter FwOwn with Sleep Notify
	while(pAd->bCRAccessing & retry)
	{
		RtmpusecDelay(100);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wait CRAccessing %d\n", pAd->bCRAccessing));
		retry--;
	}
	
	if (retry == 0)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("**************WARNING******************\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can not set FW own! gbDrvOwn=%d\n", pAd->bDrvOwn));
		pAd->bSetFWOwnRunning = 0;
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);
		return 1;
	}
	
	/* Wirte any value to HIF_FUN_CAP to set FW own */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Wirte any value to HIF_FUN_CAP to set FW own\n",__FUNCTION__));
	HIF_IO_WRITE32(pAd, HIF_FUN_CAP, 1);
	pAd->bDrvOwn = FALSE;

	HIF_IO_READ32(pAd, 0x4014, &Value);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Get 0x4014=0x%x\n",__FUNCTION__, Value));

	pAd->bSetFWOwnRunning = 0;

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);

	return Ret;	
}
#endif /* defined(MT7615) || defined(MT7622) */	
#endif /* CONFIG_FWOWN_SUPPORT */

/*handle specific CR and forwarding to platform handle*/

VOID pci_io_read32(RTMP_ADAPTER *ad,UINT32 addr,UINT32 *value)
{
	if(ad->chipOps.hif_io_read32){
		if(ad->chipOps.hif_io_read32(ad,addr,value) >= 0)
			return;
	}

	RTMP_IO_READ32(ad,addr,value);
}

VOID pci_io_write32(RTMP_ADAPTER *ad,UINT32 addr,UINT32 value)
{
	if(ad->chipOps.hif_io_write32){
		if(ad->chipOps.hif_io_write32(ad,addr,value) >= 0)
			return;
	}

	RTMP_IO_WRITE32(ad,addr,value);
}
#endif /* RTMP_MAC_PCI */
