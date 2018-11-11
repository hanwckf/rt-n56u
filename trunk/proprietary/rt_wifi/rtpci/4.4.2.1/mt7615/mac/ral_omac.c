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

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"



static UCHAR *txwi_txop_str[]={"HT_TXOP", "PIFS", "SIFS", "BACKOFF", "Invalid"};
#define TXWI_TXOP_STR(_x)	((_x) <= 3 ? txwi_txop_str[(_x)]: txwi_txop_str[4])
VOID dump_rtmp_txwi(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI)
{
	struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;

	ASSERT((sizeof(struct _TXWI_OMAC) == pAd->chipCap.TXWISize));

	if (pAd->chipCap.TXWISize != (sizeof(struct _TXWI_OMAC)))
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():sizeof(struct _TXWI_OMAC)=%d, pAd->chipCap.TXWISize=%d\n",
					__FUNCTION__, sizeof(struct _TXWI_OMAC), pAd->chipCap.TXWISize));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPHYMODE=%d(%s)\n", txwi_o->PHYMODE, get_phymode_str(txwi_o->PHYMODE)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tiTxBF=%d\n", txwi_o->iTxBF));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSounding=%d\n", txwi_o->Sounding));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\teTxBF=%d\n", txwi_o->eTxBF));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSTBC=%d\n", txwi_o->STBC));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tShortGI=%d\n", txwi_o->ShortGI));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBW=%d(%sMHz)\n", txwi_o->BW, get_bw_str(txwi_o->BW)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMCS=%d\n", txwi_o->MCS));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxOP=%d(%s)\n", txwi_o->txop, TXWI_TXOP_STR(txwi_o->txop)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMpduDensity=%d\n", txwi_o->MpduDensity));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU=%d\n", txwi_o->AMPDU));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTS=%d\n", txwi_o->TS));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCF-ACK=%d\n", txwi_o->CFACK));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMIMO-PS=%d\n", txwi_o->MIMOps));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", txwi_o->FRAG));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPID=%d\n", txwi_o->PacketId));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMPDUtotalByteCnt=%d\n", txwi_o->MPDUtotalByteCnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWCID=%d\n", txwi_o->wcid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBAWinSize=%d\n", txwi_o->BAWinSize));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNSEQ=%d\n", txwi_o->NSEQ));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tACK=%d\n", txwi_o->ACK));
}


VOID dump_rtmp_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI)
{
	struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)pRxWI;

	ASSERT((sizeof(struct _RXWI_OMAC) == pAd->chipCap.RXWISize));

	if (pAd->chipCap.RXWISize != (sizeof(struct _RXWI_OMAC)))
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():sizeof(struct _RXWI_OMAC)=%d, pAd->chipCap.RXWISize=%d\n",
					__FUNCTION__, sizeof(struct _RXWI_OMAC), pAd->chipCap.RXWISize));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWCID=%d\n", rxwi_o->wcid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMPDUtotalByteCnt=%d\n", rxwi_o->MPDUtotalByteCnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPhyMode=%d(%s)\n", rxwi_o->phy_mode, get_phymode_str(rxwi_o->phy_mode)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMCS=%d\n", rxwi_o->mcs));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBW=%d\n", rxwi_o->bw));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSGI=%d\n", rxwi_o->sgi));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSTBC=%d\n", rxwi_o->stbc));


	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSequence=%d\n", rxwi_o->SEQUENCE));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", rxwi_o->FRAG));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTID=%d\n", rxwi_o->tid));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tkey_idx=%d\n", rxwi_o->key_idx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBSS_IDX=%d\n", rxwi_o->bss_idx));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRSSI=%d:%d:%d\n", rxwi_o->RSSI0, rxwi_o->RSSI1, rxwi_o->RSSI2));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSNR=%d:%d:%d\n", rxwi_o->SNR0, rxwi_o->SNR1, rxwi_o->SNR2));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFreqOffset=%d\n", rxwi_o->FOFFSET));
}


#ifdef RTMP_MAC_PCI
VOID rtmp_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	DELAY_INT_CFG_STRUC IntCfg;
	UINT32 phy_addr, offset;
	INT i;


	/*
		Write Tx Ring base address registers 
		
		The Tx Ring arrangement:
		RingIdx	SwRingIdx	AsicPriority	WMM QID
		0 		TxSw0		L			QID_AC_BE
		1		TxSw1		L			QID_AC_BK
		2		TxSw2		L			QID_AC_VI
		3		TxSw3		L			QID_AC_VO

		4		HCCA		M			-
		5		MGMT		H			-

		Ring 0~3 for TxChannel 0
		Ring 6~9 for TxChannel 1
	*/
	for (i = 0; i < NUM_OF_TX_RING; i++) {
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(pAd->PciHif.TxRing[i].Cell[0].AllocPa);
		pAd->PciHif.TxRing[i].TxSwFreeIdx = 0;
		pAd->PciHif.TxRing[i].TxCpuIdx = 0;
		pAd->PciHif.TxRing[i].hw_desc_base = TX_BASE_PTR0 + offset;
		pAd->PciHif.TxRing[i].hw_cidx_addr = TX_CTX_IDX0 + offset;
		pAd->PciHif.TxRing[i].hw_didx_addr = TX_DTX_IDX0 + offset;
		HIF_IO_WRITE32(pAd, pAd->PciHif.TxRing[i].hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, pAd->PciHif.TxRing[i].hw_cidx_addr, pAd->TxRing[i].TxCpuIdx);
		HIF_IO_WRITE32(pAd, TX_MAX_CNT0 + offset, TX_RING_SIZE);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
					i, pAd->PciHif.TxRing[i].hw_desc_base, phy_addr, TX_RING_SIZE));
	}

	/* init MGMT ring index pointer */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->MgmtRing.Cell[0].AllocPa);
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;
	pAd->MgmtRing.hw_desc_base = TX_BASE_PTR5;
	pAd->MgmtRing.hw_cidx_addr = TX_MGMTCTX_IDX;
	pAd->MgmtRing.hw_didx_addr = TX_MGMTDTX_IDX;
	HIF_IO_WRITE32(pAd, pAd->MgmtRing.hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr, pAd->MgmtRing.TxCpuIdx);
	HIF_IO_WRITE32(pAd, TX_MGMTMAX_CNT, MGMT_RING_SIZE);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_MGMT[0x%x]: Base=0x%x, Cnt=%d!\n",
					pAd->MgmtRing.hw_desc_base, phy_addr, MGMT_RING_SIZE));

	/* Init RX Ring Base/Size/Index pointer CSR */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->RxRing[0].Cell[0].AllocPa);
	pAd->PciHif.RxRing[0].RxSwReadIdx = 0;
	pAd->PciHif.RxRing[0].RxCpuIdx = RX_RING_SIZE - 1;
	pAd->PciHif.RxRing[0].hw_desc_base = RX_BASE_PTR;
	pAd->PciHif.RxRing[0].hw_cidx_addr = RX_CRX_IDX;
	pAd->PciHif.RxRing[0].hw_didx_addr = RX_DRX_IDX;
	HIF_IO_WRITE32(pAd, pAd->PciHif.RxRing[0].hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd, pAd->PciHif.RxRing[0].hw_cidx_addr, pAd->RxRing[0].RxCpuIdx);
	HIF_IO_WRITE32(pAd, RX_MAX_CNT, RX_RING_SIZE);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->RX_RING[0x%x]: Base=0x%x, Cnt=%d\n",
				pAd->PciHif.RxRing[0].hw_desc_base, phy_addr, RX_RING_SIZE));

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	AsicWaitPDMAIdle(pAd, 100, 1000);
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
	
	IntCfg.word = 0;
	HIF_IO_WRITE32(pAd, DELAY_INT_CFG, IntCfg.word);
}
#endif /* RTMP_MAC_PCI */

