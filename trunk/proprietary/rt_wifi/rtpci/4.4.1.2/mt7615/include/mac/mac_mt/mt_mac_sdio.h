#ifdef MTK_LICENSE
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
	mt_mac_sdio.h
*/
#endif /* MTK_LICENSE */
#ifndef __MAC_SDIO_H__
#define __MAC_SDIO_H__

#include "rtmp_type.h"
#include "phy/phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"

#define RTMP_START_DEQUEUE(pAd, QueIdx, irqFlags,deq_info)				\
			{													\
				RTMP_IRQ_LOCK(&pAd->DeQueueLock[QueIdx], irqFlags);		\
				if (pAd->DeQueueRunning[QueIdx])						\
				{														\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
					deq_info.q_max_cnt[QueIdx] = 0; \
					deq_info.pkt_cnt = 0; \
					MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_TRACE, ("DeQueueRunning[%d]= TRUE!\n", QueIdx));		\
					continue;											\
				}														\
				else													\
				{														\
					pAd->DeQueueRunning[QueIdx] = TRUE;					\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
				}														\
			}

#define RTMP_STOP_DEQUEUE(pAd, QueIdx, irqFlags)						\
			do{															\
				RTMP_IRQ_LOCK(&pAd->DeQueueLock[QueIdx], irqFlags);		\
				pAd->DeQueueRunning[QueIdx] = FALSE;					\
				RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);	\
			}while(0)

#define	RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) \
	(MtSDIOTxResourceRequest(pAd, pTxBlk->QueIdx, freeNum, GET_OS_PKT_LEN(pPacket)) == NDIS_STATUS_SUCCESS)

#define RTMP_RELEASE_DESC_RESOURCE(pAd, QueIdx)			\
		do{}while(0)

#define NEED_QUEUE_BACK_FOR_AGG(_pAd, _QueIdx, _freeNum, _TxFrameType) 	0	

#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
	MtSDIO_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)	\
	MtSDIO_WriteSingleTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)


#define HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber) \
	MtSDIO_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber)

#define HAL_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)	\
	MtSDIO_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)

#define HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)	\
	MtSDIO_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)

#define HAL_LastTxIdx(pAd, QueIdx,TxIdx) \
	MtSDIODataLastTxIdx(pAd, QueIdx,TxIdx)

#define HAL_KickOutTx(pAd, pTxBlk, QueIdx)	\
	MtSDIODataKickOut(pAd, pTxBlk, QueIdx)

#define HAL_KickOutMgmtTx(_pAd, _QueIdx, _pPacket, _pSrcBufVA, _SrcBufLen)	\
	MtSDIOMgmtKickOut(_pAd, _pSrcBufVA, _SrcBufLen)

#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
	MtSDIONullFrameKickOut(_pAd, _QueIdx, _pNullFrame, _frameLen)

#define GET_TXRING_FREENO(_pAd, _QueIdx)	MTSDIOTxQueryPageResource(_pAd, _QueIdx)
#define GET_MGMTRING_FREENO(_pAd,_RingIdx)			(_pAd->MgmtRing.TxSwFreeIdx)


#define RTMP_OS_IRQ_RELEASE(_pAd, _NetDev)

VOID BeaconUpdateExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);


/*Don't implement it for SDIO*/
#define RTMP_ASIC_INTERRUPT_DISABLE(_pAd)

#define RTMP_ASIC_INTERRUPT_ENABLE(_pAd)

#define RTMP_IRQ_ENABLE(_pAd)

#define RTMP_IRQ_DISABLE(_pAd)


#endif /*__MAC_SDIO_H__ */

