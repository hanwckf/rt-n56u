/*

*/

#ifndef __RTMP_INF_PCIRBS_H__
#define __RTMP_INF_PCIRBS_H__

#define fRTMP_ADAPTER_NEED_STOP_TX	0

#ifdef PCI_MSI_SUPPORT
#define RTMP_OS_IRQ_RELEASE(_pAd, _NetDev)								\
{																			\
	POS_COOKIE pObj = (POS_COOKIE)(_pAd->OS_Cookie);						\
	RtmpOSIRQRelease(_NetDev, _pAd->infType, pObj->pci_dev, &_pAd->HaveMsi);\
}
#else
#define RTMP_OS_IRQ_RELEASE(_pAd, _NetDev)								\
{																			\
	POS_COOKIE pObj = (POS_COOKIE)(_pAd->OS_Cookie);						\
	RtmpOSIRQRelease(_NetDev, _pAd->infType, pObj->pci_dev, NULL);			\
}
#endif /* PCI_MSI_SUPPORT */


/* ----------------- TX Related MACRO ----------------- */
#define RTMP_START_DEQUEUE(pAd, QueIdx, irqFlags,deq_info)		do{}while(0)
#define RTMP_STOP_DEQUEUE(pAd, QueIdx, irqFlags)		do{}while(0)


#define RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) \
		((freeNum) >= (ULONG)(pTxBlk->TotalFragNum + RTMP_GET_PACKET_FRAGMENTS(pPacket) + 3)) /* rough estimate we will use 3 more descriptor. */
#define RTMP_RELEASE_DESC_RESOURCE(pAd, QueIdx)	\
		do{}while(0)

#ifdef WFA_VHT_PF
#define NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, freeNum, _TxFrameType) \
		((((pAd->force_amsdu && (_TxFrameType == TX_AMSDU_FRAME)) || (freeNum != (TX_RING_SIZE-1))) && (pAd->TxSwQueue[QueIdx].Number == 0)) || \
		 (freeNum<3)\
		)
#else
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
#define NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, freeNum, _TxFrameType) \
		((((pAd->force_amsdu && (_TxFrameType == TX_RALINK_FRAME)) || (freeNum != (TX_RING_SIZE-1))) && (pAd->TxSwQueue[QueIdx].Number == 0)) || \
		 (freeNum<3)\
		)
#else
#define NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, freeNum, _TxFrameType) \
		(((freeNum != (TX_RING_SIZE-1)) && (pAd->TxSwQueue[QueIdx].Number == 0)) || (freeNum<3))
		/*(((freeNum) != (TX_RING_SIZE-1)) && (pAd->TxSwQueue[QueIdx].Number == 1)) */
#endif
#endif /* WFA_VHT_PF */

#define HAL_KickOutMgmtTx(_pAd, _QueIdx, _pPacket, _pSrcBufVA, _SrcBufLen)	\
			RtmpPCIMgmtKickOut(_pAd, _QueIdx, _pPacket, _pSrcBufVA, _SrcBufLen)

#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
		/* RtmpPCI_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)*/

#define HAL_WriteTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)	\
			RtmpPCI_WriteSingleTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber) \
			RtmpPCI_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber)

#define HAL_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)	\
			RtmpPCI_WriteMultiTxResource(pAd, pTxBlk, frameNum, pFreeNumber)

#define HAL_FinalWriteTxResource(_pAd, _pTxBlk, _TotalMPDUSize, _FirstTxIdx)	\
			RtmpPCI_FinalWriteTxResource(_pAd, _pTxBlk, _TotalMPDUSize, _FirstTxIdx)

#define HAL_LastTxIdx(_pAd, _QueIdx,_LastTxIdx) \
			/*RtmpPCIDataLastTxIdx(_pAd, _QueIdx,_LastTxIdx)*/

#define HAL_KickOutTx(_pAd, _pTxBlk, _QueIdx)	\
			HIF_IO_WRITE32((_pAd), (_pAd)->PciHif.TxRing[(_QueIdx)].hw_cidx_addr, (_pAd)->PciHif.TxRing[(_QueIdx)].TxCpuIdx)

#define HAL_KickOutTxBMC(_pAd, _pTxBlk, _QueIdx)	\
			HIF_IO_WRITE32((_pAd), (_pAd)->TxBmcRing.hw_cidx_addr, (_pAd)->TxBmcRing.TxCpuIdx)

#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
			MiniportMMRequest(_pAd, _QueIdx, _pNullFrame, _frameLen)

#define GET_TXRING_FREENO(_pAd, _QueIdx) \
	((_pAd->PciHif.TxRing[_QueIdx].TxSwFreeIdx > _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx)	? \
			(_pAd->PciHif.TxRing[_QueIdx].TxSwFreeIdx - _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx - 1) \
			 :	\
			(_pAd->PciHif.TxRing[_QueIdx].TxSwFreeIdx + TX_RING_SIZE - _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx - 1))

#if defined(MT7615) || defined(MT7622)
// TODO: shiang-MT7615, fix me after the cmd ring and N9 is ready for Beacon handling!!
#define GET_MGMTRING_FREENO(_pAd,_RingIdx) 	GET_TXRING_FREENO(_pAd,_RingIdx)
#define GET_BCNRING_FREENO(_pAd,_RingIdx)    GET_TXRING_FREENO(_pAd,_RingIdx)	
#else
#define GET_TXRING_FREENO(_pAd, _QueIdx) \
	((_pAd->PciHif.TxRing[_QueIdx].TxSwFreeIdx > _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx)	? \
			(_pAd->PciHif.TxRing[_QueIdx].TxSwFreeIdx - _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx - 1) \
			 :	\
			(_pAd->PciHif.TxRing[_QueIdx].TxSwFreeIdx + TX_RING_SIZE - _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx - 1))

#define GET_MGMTRING_FREENO(_pAd,_RingIdx) \
	(_pAd->MgmtRing.TxSwFreeIdx > _pAd->MgmtRing.TxCpuIdx)	? \
			(_pAd->MgmtRing.TxSwFreeIdx - _pAd->MgmtRing.TxCpuIdx - 1) \
			 :	\
			(_pAd->MgmtRing.TxSwFreeIdx + MGMT_RING_SIZE - _pAd->MgmtRing.TxCpuIdx - 1);

#ifdef MT_MAC
#define GET_BCNRING_FREENO(_pAd,_RingIdx) \
	(_pAd->BcnRing.TxSwFreeIdx > _pAd->BcnRing.TxCpuIdx)	? \
			(_pAd->BcnRing.TxSwFreeIdx - _pAd->BcnRing.TxCpuIdx - 1) \
			 :	\
			(_pAd->BcnRing.TxSwFreeIdx + BCN_RING_SIZE - _pAd->BcnRing.TxCpuIdx - 1);
#endif /* MT_MAC */
#endif /* defined(MT7615) || defined(MT7622) */

#define IS_TXRING_EMPTY(_pAd, _QueIdx)\
	(_pAd->PciHif.TxRing[_QueIdx].TxDmaIdx == _pAd->PciHif.TxRing[_QueIdx].TxCpuIdx)	? 1: 0;



#define GET_RXRING_PENDINGNO(_pAd, _QueIdx) \
			(_pAd->PciHif.RxRing[_QueIdx].RxDmaIdx > _pAd->PciHif.RxRing[_QueIdx].RxSwReadIdx) ?	\
			(_pAd->PciHif.RxRing[_QueIdx].RxDmaIdx - _pAd->PciHif.RxRing[_QueIdx].RxSwReadIdx) \
			:	\
			(_pAd->PciHif.RxRing[_QueIdx].RxDmaIdx + _pAd->PciHif.RxRing[_QueIdx].RxRingSize - _pAd->PciHif.RxRing[_QueIdx].RxSwReadIdx)



#define IS_RXRING_FULL(_pAd, _QueIdx)\
	(_pAd->PciHif.RxRing[_QueIdx].RxDmaIdx == _pAd->PciHif.RxRing[_QueIdx].RxCpuIdx)	? 1: 0;


#ifdef CONFIG_ANDES_SUPPORT
#define GET_CTRLRING_FREENO(_pAd) \
	(_pAd->CtrlRing.TxSwFreeIdx > _pAd->CtrlRing.TxCpuIdx)	? \
			(_pAd->CtrlRing.TxSwFreeIdx - _pAd->CtrlRing.TxCpuIdx - 1) \
			 :	\
			(_pAd->CtrlRing.TxSwFreeIdx + MGMT_RING_SIZE - _pAd->CtrlRing.TxCpuIdx - 1);

#if defined(MT7615) || defined(MT7622)
#define GET_FWDWLORING_FREENO(_Ring) \
	((_Ring)->TxSwFreeIdx > (_Ring)->TxCpuIdx)	? \
			((_Ring)->TxSwFreeIdx - (_Ring)->TxCpuIdx - 1) \
			 :	\
			((_Ring)->TxSwFreeIdx + (_Ring)->ring_size - (_Ring)->TxCpuIdx - 1);
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */


struct _RTMP_ADAPTER;
struct _TX_BLK;

USHORT RtmpPCI_WriteSingleTxResource(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN BOOLEAN bIsLast,
	OUT USHORT *FreeNumber);

USHORT RtmpPCI_WriteMultiTxResource(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN UCHAR frameNum,
	OUT USHORT *FreeNumber);

USHORT RtmpPCI_WriteFragTxResource(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN UCHAR fragNum,
	OUT	USHORT *FreeNumber);

VOID RtmpPCI_FinalWriteTxResource(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN USHORT totalMPDUSize,
	IN USHORT FirstTxIdx);

int RtmpPCIMgmtKickOut(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN PUCHAR pSrcBufVA,
	IN UINT SrcBufLen);

#endif /* __RTMP_INF_PCIRBS_H__ */

