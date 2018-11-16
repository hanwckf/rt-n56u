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
	cmm_data.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#include "rt_config.h"


UCHAR	SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
UCHAR	EAPOL[] = {0x88, 0x8e};
UCHAR   TPID[] = {0x81, 0x00}; /* VLAN related */

UCHAR	IPX[] = {0x81, 0x37};
UCHAR	APPLE_TALK[] = {0x80, 0xf3};


//  UserPriority To AccessCategory mapping
UCHAR WMM_UP2AC_MAP[8] = {QID_AC_BE, QID_AC_BK,
							QID_AC_BK, QID_AC_BE,
							QID_AC_VI, QID_AC_VI,
							QID_AC_VO, QID_AC_VO};



#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
UCHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9}; /* spec. 1.14 OUI */
#endif /* defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) */

#ifdef DBG_DIAGNOSE
VOID dbg_diag_deque_log(RTMP_ADAPTER *pAd)
{
	struct dbg_diag_info *diag_info;
	UCHAR QueIdx = 0;
	UCHAR RingNum =4;

	diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];

#if defined(MT7615) || defined(MT7622)
	RingNum = 2;
#endif

#ifdef RTMP_MAC_PCI
#ifdef DBG_TX_RING_DEPTH
    if ((pAd->DiagStruct.diag_cond & DIAG_COND_TX_RING_DEPTH) != DIAG_COND_TX_RING_DEPTH)
    {
		for (QueIdx = 0; QueIdx < RingNum /* 4 */; QueIdx++)
		{
			UINT free_cnt, desc_num;
			UINT32 dma_id, cpu_id, hw_cnt;

			free_cnt = GET_TXRING_FREENO(pAd, QueIdx);
			HIF_IO_READ32(pAd,pAd->PciHif.TxRing[QueIdx].hw_didx_addr,&dma_id);
			HIF_IO_READ32(pAd,pAd->PciHif.TxRing[QueIdx].hw_cidx_addr,&cpu_id);

			if (dma_id > cpu_id)
				hw_cnt = TX_RING_SIZE - dma_id + cpu_id;
			else if (cpu_id > dma_id)
				hw_cnt  = cpu_id - dma_id;
			else
				hw_cnt = ((free_cnt > 0) ? 0 : TX_RING_SIZE);

			hw_cnt = ((hw_cnt <=15) ? hw_cnt : 15);
			diag_info->TxDescCnt[QueIdx][hw_cnt]++;
		}
    }
#endif /* DBG_TX_RING_DEPTH */

#ifdef MT_MAC
#ifdef DBG_PSE_DEPTH
    if ((pAd->DiagStruct.diag_cond & DIAG_COND_PSE_DEPTH) != DIAG_COND_PSE_DEPTH)
	{
		UINT32 mac_val;
		RTMP_IO_READ32(pAd, 0x8148, &mac_val);
		mac_val = ((mac_val & 0xfff) / 13);
		if (mac_val > 48)
			mac_val = 49;
		diag_info->pse_pg_cnt[mac_val]++;
	}
#endif /* DBG_PSE_DEPTH */
#endif /* MT_MAC */
#endif /* RTMP_MAC_PCI */

#ifdef DBG_TXQ_DEPTH
    if ((pAd->DiagStruct.diag_cond & DIAG_COND_TXQ_DEPTH) != DIAG_COND_TXQ_DEPTH)
    {
		if ((pAd->DiagStruct.wcid > 0) &&  (pAd->DiagStruct.wcid < MAX_LEN_OF_TR_TABLE)) {
			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pAd->DiagStruct.wcid];
			UCHAR swq_cnt;

			for (QueIdx = 0; QueIdx < 4; QueIdx++)
			{
				if (tr_entry->tx_queue[QueIdx].Number <= 7)
					swq_cnt = tr_entry->tx_queue[QueIdx].Number;
				else
					swq_cnt = 8;
				diag_info->TxSWQueCnt[QueIdx][swq_cnt]++;
			}
		}
    }
#endif /* DBG_TXQ_DEPTH */
}
#endif /* DBG_DIAGNOSE */


VOID dump_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Dump RX_BLK Structure:\n"));

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\tHW rx info:\n"));
	hex_dump("RawData", &pRxBlk->hw_rx_info[0], RXD_SIZE);

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\tData Pointer info:\n"));

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tpRxInfo=0x%p\n", pRxBlk->pRxInfo));
		dump_rxinfo(pAd, pRxBlk->pRxInfo);
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tpRxFceInfo=0x%p\n", pRxBlk->pRxFceInfo));
			dumpRxFCEInfo(pAd, pRxBlk->pRxFceInfo);
		}
#endif /* RLT_MAC */
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tpRxWI=0x%p\n", pRxBlk->pRxWI));
		dump_rmac_info(pAd, (UCHAR *)pRxBlk->pRxWI);
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\trmac_info=0x%p\n", pRxBlk->rmac_info));
		dump_rmac_info(pAd, pRxBlk->rmac_info);
	}
#endif /* MT_MAC */

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tpRxPacket=0x%p, MPDUtotalByteCnt=%d\n", pRxBlk->pRxPacket, pRxBlk->MPDUtotalByteCnt));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tpData=0x%p\n", pRxBlk->pData));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tDataSize=%d\n", pRxBlk->DataSize));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tFlags=0x%x\n", pRxBlk->Flags));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tUserPriority=%d\n", pRxBlk->UserPriority));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tOpMode=%d\n", pRxBlk->OpMode));

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\tMirror Info from RMAC Info:\n"));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tWCID=%d\n", pRxBlk->wcid));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tTID=%d\n", pRxBlk->TID));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tKey_idx=%d\n", pRxBlk->key_idx));
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tBSS_IDX=%d\n", pRxBlk->bss_idx));
#if defined(RLT_MAC) || defined(RTMP_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tPhyMode=%d(%s)\n",
					pRxBlk->rx_rate.field.MODE,
					get_phymode_str(pRxBlk->rx_rate.field.MODE)));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tMCS=%d\n", pRxBlk->rx_rate.field.MCS));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tBW=%d\n", pRxBlk->rx_rate.field.BW));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tSGI=%d\n", pRxBlk->rx_rate.field.ShortGI));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tSTBC=%d\n", pRxBlk->rx_rate.field.STBC));
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tLDPC=%d\n", pRxBlk->rx_rate.field.ldpc));
#endif /* RLT_MAC */
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tRSSI=%d:%d:%d\n",
						pRxBlk->rx_signal.raw_rssi[0], pRxBlk->rx_signal.raw_rssi[1],
						pRxBlk->rx_signal.raw_rssi[2]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tSNR=%d:%d:%d\n",
						pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1],
						pRxBlk->rx_signal.raw_snr[2]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tFreqOffset=%d\n",
						pRxBlk->rx_signal.freq_offset));
	}
#endif /* defined(RLT_MAC) || defined(RTMP_MAC) */

	//hex_dump("Dump RxPacket in dump_rxblk", (UCHAR *)pRxBlk->pHeader, pRxBlk->MPDUtotalByteCnt > 512 ? 512 : pRxBlk->MPDUtotalByteCnt);
}


VOID dump_txblk(RTMP_ADAPTER *pAd, TX_BLK *txblk)
{

	//hex_dump("TxBlk Raw Data", (UCHAR *)txblk, sizeof(TX_BLK));
	printk("TxBlk Info\n");
	printk("\twdev=%p\n", txblk->wdev);
	printk("\twdev_idx=%d\n", txblk->wdev_idx);
	printk("\tWCID=%d\n", txblk->Wcid);
	printk("\tWMM_Idx=%d\n", txblk->WmmIdx);
	printk("\tQueIdx=%d\n", txblk->QueIdx);
#if defined(MT7615) || defined(MT7622)
	printk("\tWMM_Set=%d\n", txblk->wmm_set);
#endif /* defined(MT7615) || defined(MT7622) */

	printk("\tpMacEntry=%p\n", txblk->pMacEntry);
	if (txblk->pMacEntry) {
		printk("\t\tpMacEntry->wcid=%d\n", txblk->pMacEntry->wcid);
		printk("\t\tpMacEntry->tr_tb_idx=%d\n", txblk->pMacEntry->tr_tb_idx);
	}
	printk("\tTR_Entry=%p\n", txblk->tr_entry);
	if (txblk->tr_entry) {
		printk("\t\tTR_Entry->wcid=%d\n", txblk->tr_entry->wcid);
	}
	printk("\tOpMode=%d\n", txblk->OpMode);
	printk("\tTxFrameType=%d\n", txblk->TxFrameType);
	printk("\tTotalFragNum=%d\n", txblk->TotalFragNum);
	printk("\tUserPriority=%d\n", txblk->UserPriority);

}


#ifdef MT_MAC
/*
    1'b0: the related GROUP is not present
    1'b1: the related GROUP is present

    bit[0]: indicates GROUP1 (DW8~DW11)
    bit[1]: indicates GROUP2 (DW12~DW13)
    bit[2]: indicates GROUP3 (DW14~DW19)
    bit[3]: indicates GROUP4 (DW4~DW7)
*/
//#define RMAC_INFO_GRP_1_SIZE    16
//#define RMAC_INFO_GRP_2_SIZE    8
//#define RMAC_INFO_GRP_3_SIZE    24
//#define RMAC_INFO_GRP_4_SIZE    16

static INT32 RMACInfoGrpToLen[]={
        /* 0: base only */
        RMAC_INFO_BASE_SIZE,
        /* 1: [bit 0] base + group 1 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE,
        /* 2: [bit 1] base + group 2 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE,
        /* 3: [bit 0 + bit 1] base + group 1 + group 2 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE,
        /* 4: [bit 2] base + group 3 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_3_SIZE,
        /* 5: [bit 0 + bit 2] base + group 1 + group 3 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_3_SIZE,
        /* 6: [bit 1 + bit 2] base + group 2 + group 3 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE,
        /* 7: [bit 0 + bit 1 + bit 2] base + group 1 + group 2 + group 3 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE,
        /* 8: [bit 3 ] base + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 9: [bit 0 + bit 3 ] base + group 1 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 10: [bit 1 + bit 3 ] base + group 2 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 11: [bit 0 + bit 1 + bit 3 ] base + group 1 + group 2 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 12: [bit 2 + bit 3 ] base + group 3 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 13: [bit 0 + bit 2 + bit 3 ] base + group 1 + group 3 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 14: [bit 1 + bit 2 + bit 3 ] base + group 2 + group 3 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
        /* 15: [bit 0 + bit 1 + bit 2 + bit 3 ] base + group 1 + group 2 + group 3 + group 4 */
        RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
};


#ifdef MT_MAC
VOID ParseRxVStat(RTMP_ADAPTER *pAd, RX_STATISTIC_RXV *rx_stat, UCHAR *Data)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;
	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;
#if defined(MT7615) || defined(MT7622)
	INT16 foe = 0;
	UINT32 i = 0;
	UINT8 cbw = 0;
	UINT32 foe_const = 0;
#endif
	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);


#if defined(MT7615) || defined(MT7622)
	if (RXV1_1ST_CYCLE->TxMode == MODE_CCK) {
		foe = (RXV1_5TH_CYCLE->MISC1 &0x7ff);
		foe = (foe*1000)>>11;
	} else {
		cbw = RXV1_1ST_CYCLE->FrMode;
		foe_const = ((1<<(cbw + 1))&0xf)*10000;
		foe = (RXV1_5TH_CYCLE->MISC1 &0xfff);

		if( foe >= 2048)
		    foe = foe - 4096; 
		foe = (foe*foe_const)>>15;
	}

	rx_stat->FreqOffsetFromRx = foe;
	rx_stat->RCPI[0] = RXV1_4TH_CYCLE->RCPI0;
	rx_stat->RCPI[1] = RXV1_4TH_CYCLE->RCPI1;
	rx_stat->RCPI[2] = RXV1_4TH_CYCLE->RCPI2;
	rx_stat->RCPI[3] = RXV1_4TH_CYCLE->RCPI3;
	rx_stat->FAGC_RSSI_IB[0] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[0] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[1] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[1] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[2] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[2] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[3] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[3] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->SNR[0] = (RXV1_5TH_CYCLE->MISC1 >> 19) - 16;

	for(i = 0; i < 4; i++) {
		if (rx_stat->FAGC_RSSI_IB[i] >= 128)
			rx_stat->FAGC_RSSI_IB[i] -=256;
		if (rx_stat->FAGC_RSSI_WB[i] >= 128)
			rx_stat->FAGC_RSSI_WB[i] -=256;
	}
#else
	rx_stat->FreqOffsetFromRx = RXV1_5TH_CYCLE->FoE;
	rx_stat->RCPI[0] = RXV1_3TH_CYCLE->Rcpi0;
	rx_stat->RCPI[1] = RXV1_3TH_CYCLE->Rcpi1;
	rx_stat->SNR[0] = RXV1_5TH_CYCLE->LTF_SNR0;
	rx_stat->SNR[1] = RXV2_2ND_CYCLE->OfdmLtfSNR1;
	rx_stat->RSSI[0] = RXV1_3TH_CYCLE->Rcpi0/2 - 110;
	rx_stat->RSSI[1] = RXV1_3TH_CYCLE->Rcpi1/2 - 110;
#endif /* defined(MT7615) || defined(MT7622) */
}

VOID ParseRxVPacket(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	BOOLEAN bMgmtFr = TRUE;

	if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL)
	{
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)Data;
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 4);

#if defined(MT7615) || defined(MT7622)
		if (((FRAME_CONTROL *)RxBlk->FC)->Type == FC_TYPE_DATA)
		{
			bMgmtFr = FALSE;
		}
#endif /* defined(MT7615) || defined(MT7622) */
	}
	else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV)
	{
		RXV_DWORD0 *DW0 = NULL;
		RXV_DWORD1 *DW1 = NULL;
		RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
		RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
		RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;
		RX_STATISTIC_RXV *rx_stat = &pAd->rx_stat_rxv;

		DW0 = (RXV_DWORD0 *)Data;
		DW1 = (RXV_DWORD1 *)(Data + 4);
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);

		RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
		RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
		RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);

		RxBlk->rxv2_cyc1 = *(UINT32 *)RXV2_1ST_CYCLE;
		RxBlk->rxv2_cyc2 = *(UINT32 *)RXV2_2ND_CYCLE;
		RxBlk->rxv2_cyc3 = *(UINT32 *)RXV2_3TH_CYCLE;

#ifdef MT7615
        if (RXV1_2ND_CYCLE->NstsField == 1)
        {
            /* Parser Condition number */
            if (pAd->cn_cnt < 10)
            {
                UINT32 row_data = (RxBlk->rxv2_cyc2 >> 20) & 0xFFF;
                pAd->rxv2_cyc3[pAd->cn_cnt++] = row_data/4;
                if (pAd->cn_cnt == 10 && !ATE_ON(pAd))
                {
                    UINT32 value;

                    RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x66c, &value);
                    value &= ~0xF0;
                    RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x66c, value);

                    RTMP_IO_READ32(pAd, ARB_RQCR, &value);
                    value &= ~ARB_RQCR_RXV_R_EN;
                    if (pAd->CommonCfg.dbdc_mode)
                    {
                        value &= ~ARB_RQCR_RXV1_R_EN;
                    }
                    RTMP_IO_WRITE32(pAd, ARB_RQCR, value);
                }
            }
        }
#endif


		if (pAd->parse_rxv_stat_enable)
		{
			ParseRxVStat(pAd, rx_stat, Data);
		}
               
#ifdef CONFIG_ATE
		if (ATE_ON(pAd))
		{
			MT_ATEUpdateRxStatistic(pAd, TESTMODE_RXV, Data);
		}
#endif /* CONFIG_ATE */
	}
	else
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): invalid Type %u\n", __FUNCTION__, Type));
		return; /* return here to avoid dereferencing NULL pointer below */
	}

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
#ifdef AIR_MONITOR
        MAC_TABLE_ENTRY *pEntry = NULL;

        if (VALID_WCID(RxBlk->wcid))
			pEntry = &pAd->MacTab.Content[RxBlk->wcid];
#endif /* AIR_MONITOR */

		if ((bMgmtFr == TRUE)
#ifdef AIR_MONITOR
            || ((pAd->MntEnable == TRUE) && (pEntry && IS_ENTRY_MONITOR(pEntry)))
#endif /* AIR_MONITOR */
            )
		{
			RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 12);

			RxBlk->rx_signal.raw_rssi[0] = (RXV1_4TH_CYCLE->RCPI0 - 220) / 2;
			RxBlk->rx_signal.raw_rssi[1] = (RXV1_4TH_CYCLE->RCPI1 - 220) / 2;
			RxBlk->rx_signal.raw_rssi[2] = (RXV1_4TH_CYCLE->RCPI2 - 220) / 2;
			RxBlk->rx_signal.raw_rssi[3] = (RXV1_4TH_CYCLE->RCPI3 - 220) / 2;
		}
	}
#endif /* defined(MT7615) || defined(MT7622) */

	RxBlk->rx_rate.field.MODE = RXV1_1ST_CYCLE->TxMode;
	RxBlk->rx_rate.field.MCS = RXV1_1ST_CYCLE->TxRate;
	RxBlk->rx_rate.field.ldpc = RXV1_1ST_CYCLE->HtAdCode;
	RxBlk->rx_rate.field.BW = RXV1_1ST_CYCLE->FrMode;
	RxBlk->rx_rate.field.STBC = RXV1_1ST_CYCLE->HtStbc ? 1:0;
	RxBlk->rx_rate.field.ShortGI = RXV1_1ST_CYCLE->HtShortGi;
#if defined(MT7615) || defined(MT7622)
	if ((RxBlk->rx_rate.field.MODE == MODE_VHT) && (RxBlk->rx_rate.field.STBC == 0))
	{
		RxBlk->rx_rate.field.MCS |= ((RXV1_2ND_CYCLE->NstsField & 0x3) << 4);
	}
#endif /* defined(MT7615) || defined(MT7622) */

}


static inline INT32 mt_rx_info_2_blk(
								RTMP_ADAPTER *pAd,
								RX_BLK *pRxBlk,
								PNDIS_PACKET pRxPacket,
								INT32 pkt_type)
{
    UCHAR *RMACInfo, *Pos;
    INT32 RMACInfoLen;
    struct _RXD_BASE_STRUCT *rx_base;
	RXD_GRP4_STRUCT *RxdGrp4 = NULL;
	RXD_GRP1_STRUCT *RxdGrp1 = NULL;
	RXD_GRP2_STRUCT *RxdGrp2 = NULL;
	RXD_GRP3_STRUCT *RxdGrp3 = NULL;
	UCHAR *FC = NULL;
	UINT16 temp_fc,fn_sn;

    pRxBlk->pRxInfo = (RXINFO_STRUC *)(&pRxBlk->hw_rx_info[RXINFO_OFFSET]);
	RMACInfo = (UCHAR *)(GET_OS_PKT_DATAPTR(pRxPacket));
	Pos = RMACInfo;
    pRxBlk->rmac_info = RMACInfo;
    rx_base = (struct _RXD_BASE_STRUCT *)RMACInfo;

	Pos += RMAC_INFO_BASE_SIZE;

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP4)
	{
		RxdGrp4 = (RXD_GRP4_STRUCT *)Pos;
		/*RxdGrp4 have mac addr that dont need to swap */
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((UCHAR*)RxdGrp4,0);
#endif
		FC = Pos;
		Pos += RMAC_INFO_GRP_4_SIZE;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP1)
	{
		RxdGrp1 = (RXD_GRP1_STRUCT *)Pos;
		Pos += RMAC_INFO_GRP_1_SIZE;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP2)
	{
		RxdGrp2 = (RXD_GRP2_STRUCT *)Pos;
		Pos += RMAC_INFO_GRP_2_SIZE;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP3)
	{
		RxdGrp3 = (RXD_GRP3_STRUCT *)Pos;
		Pos += RMAC_INFO_GRP_3_SIZE;
	}

    RMACInfoLen = RMACInfoGrpToLen[rx_base->RxD0.RfbGroupVld];

#ifdef RT_BIG_ENDIAN
	if ((RMACInfoLen - 4) > 0)
	{
		MTMacInfoEndianChange(pAd, RMACInfo, TYPE_RMACINFO, RMACInfoLen);
    }
#endif /* RT_BIG_ENDIAN */

    //dump_rmac_info(pAd, RMACInfo);

    pRxBlk->MPDUtotalByteCnt = rx_base->RxD0.RxByteCnt - RMACInfoLen;

	if (rx_base->RxD1.HdrOffset == 1) {
        pRxBlk->MPDUtotalByteCnt -= 2;
        RMACInfoLen += 2;
    }

    pRxBlk->DataSize = pRxBlk->MPDUtotalByteCnt;
    pRxBlk->wcid = rx_base->RxD2.RxDWlanIdx;
    pRxBlk->bss_idx = rx_base->RxD1.RxDBssidIdx;
    pRxBlk->key_idx = rx_base->RxD1.KeyId;
    pRxBlk->TID = rx_base->RxD2.RxDTid;

#if defined(MT7615) || defined(MT7622)
    if (IS_MT7615(pAd) || IS_MT7622(pAd))
    {
#ifdef HDR_TRANS_RX_SUPPORT
		if (rx_base->RxD1.HdrTranslation)
			RX_BLK_SET_FLAG(pRxBlk, fRX_HDR_TRANS);

		if ((rx_base->RxD1.HdrTranslation && ((rx_base->RxD0.RfbGroupVld & RXS_GROUP4) == 0)) ||
			((rx_base->RxD1.HdrTranslation == 0) && (rx_base->RxD0.RfbGroupVld & RXS_GROUP4) && (rx_base->RxD0.PktType != RMAC_RX_PKT_TYPE_RX_DUP_RFB)) ||
			(rx_base->RxD2.HdrTransFail == 1))
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                    ("%s(): Error! HdrTrans=%d, GrpVld=%d, HdrTransFail=%d\n",
					__FUNCTION__, rx_base->RxD1.HdrTranslation,
					rx_base->RxD0.RfbGroupVld,
					rx_base->RxD2.HdrTransFail));
		}
#endif /* HDR_TRANA_RX_SUPPORT */

		pRxBlk->pRxInfo->U2M = (rx_base->RxD1.a1_type == 0x1 ? 1 : 0);
		pRxBlk->pRxInfo->Mcast = (rx_base->RxD1.a1_type == 0x2 ? 1 : 0);
		pRxBlk->pRxInfo->Bcast = (rx_base->RxD1.a1_type == 0x3 ? 1 : 0);
	}
#else
	{
		pRxBlk->pRxInfo->U2M = rx_base->RxD1.UcastToMe;
		pRxBlk->pRxInfo->Mcast = rx_base->RxD1.Mcast;
		pRxBlk->pRxInfo->Bcast = rx_base->RxD1.Bcast;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	pRxBlk->AmsduState = rx_base->RxD1.PayloadFmt;
	pRxBlk->DeAmsduFail = rx_base->RxD2.DeAmsduFail;

	pRxBlk->pRxInfo->FRAG = rx_base->RxD2.FragFrm;
    pRxBlk->pRxInfo->NULLDATA = rx_base->RxD2.NullFrm;
    pRxBlk->pRxInfo->DATA = !(rx_base->RxD2.NonDataFrm);
    pRxBlk->pRxInfo->HTC = rx_base->RxD1.HTC;
    pRxBlk->pRxInfo->AMPDU = !(rx_base->RxD2.NonAmpduFrm);
    pRxBlk->pRxInfo->L2PAD = 0;
    pRxBlk->pRxInfo->AMSDU = 0; // TODO:
    pRxBlk->pRxInfo->CipherErr = rx_base->RxD2.IcvErr | (rx_base->RxD2.TkipMicErr << 1);/* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
    pRxBlk->pRxInfo->Crc = rx_base->RxD2.FcsErr;
    pRxBlk->pRxInfo->MyBss = ((rx_base->RxD1.RxDBssidIdx == 0xf) ? 0 : 1);
    pRxBlk->pRxInfo->Decrypted = 0; // TODO:

#ifdef SNIFFER_MT7615
    if (IS_MT7615(pAd)) {
        OS_PKT_RESERVE(pRxPacket, RMACInfoLen);
        OS_PKT_TAIL_BUF_EXTEND(pRxPacket, pRxBlk->MPDUtotalByteCnt);
    }
    else {
#endif
    SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + RMACInfoLen);
    SET_OS_PKT_LEN(pRxPacket, pRxBlk->MPDUtotalByteCnt);
#ifdef SNIFFER_MT7615
    }
#endif
	pRxBlk->pRxPacket = pRxPacket;
	pRxBlk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket);

#ifdef HDR_TRANS_RX_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
	{
		struct wifi_dev *wdev = NULL;

        if (!FC)
        {
           return 0;
        }
		pRxBlk->FC = FC;
		temp_fc = *((UINT16 *)FC);
		fn_sn = *((UINT16 *)(FC + 8));
		
#ifdef RT_BIG_ENDIAN
		temp_fc= le2cpu16(temp_fc);
		fn_sn = le2cpu16(fn_sn);
#endif
		pRxBlk->FN = fn_sn & 0x000f;
		pRxBlk->SN = (fn_sn & 0xfff0) >> 4;

		wdev = WdevSearchByWcid(pAd, pRxBlk->wcid);

		if (wdev == NULL)
			wdev = WdevSearchByOmacIdx(pAd, pRxBlk->bss_idx);

		if (wdev == NULL)
			return 0;

		if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 0) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 0))
		{
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr2 = pRxBlk->pData + 6;
			pRxBlk->Addr3 = wdev->bssid;
		}
		else if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 0) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 1))
		{
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr2 = wdev->bssid;
			pRxBlk->Addr3 = pRxBlk->pData + 6;

		}
		else if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 1) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 0))
		{
			pRxBlk->Addr1 = wdev->bssid;
			pRxBlk->Addr2 = pRxBlk->pData + 6;
			pRxBlk->Addr3 = pRxBlk->pData;

		}
		else
		{
			pRxBlk->Addr1 = wdev->if_addr;
			pRxBlk->Addr2 = FC + 2;
			pRxBlk->Addr3 = pRxBlk->pData;
			pRxBlk->Addr4 = pRxBlk->pData + 6;
		}


	}
	else
#endif
	{
		pRxBlk->FC = pRxBlk->pData;
		temp_fc = *((UINT16 *)(pRxBlk->FC));
		fn_sn = *((UINT16 *)(pRxBlk->FC + 22));
#ifdef RT_BIG_ENDIAN
		temp_fc = le2cpu16(temp_fc);
		fn_sn = le2cpu16(fn_sn);
#endif

		pRxBlk->Duration = *((UINT16 *)(pRxBlk->pData + 2));
#ifdef RT_BIG_ENDIAN
		pRxBlk->Duration = le2cpu16(pRxBlk->Duration);
#endif
		if ((((FRAME_CONTROL *)&temp_fc)->Type == FC_TYPE_MGMT) || (((FRAME_CONTROL *)&temp_fc)->Type == FC_TYPE_DATA))
		{
			pRxBlk->FN = fn_sn & 0x000f;
			pRxBlk->SN = (fn_sn & 0xfff0) >> 4;
		}
		pRxBlk->Addr1 = pRxBlk->pData + 4;
		pRxBlk->Addr2 = pRxBlk->pData + 10;
		pRxBlk->Addr3 = pRxBlk->pData + 16;
		if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 1) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 1))
			pRxBlk->Addr4 = pRxBlk->pData + 24;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP3)
		ParseRxVPacket(pAd, RMAC_RX_PKT_TYPE_RX_NORMAL, pRxBlk, (UCHAR *)RxdGrp3);

#ifdef RX_CUT_THROUGH
	if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb))
		RTMP_SET_RMACLEN(pRxPacket, RMACInfoLen);
#endif /* RX_CUT_THROUGH */

    if (((FRAME_CONTROL *)pRxBlk->FC)->SubType == SUBTYPE_AUTH)
    {
        /*
                If HW already decrypted this packet, SW doesn't need to decrypt again.
                @20150708
        */
        if ((rx_base->RxD2.SecMode != 0) &&
            (rx_base->RxD2.CipherMis == 0) &&
            (rx_base->RxD2.CipherLenMis == 0))
        {
            pRxBlk->pRxInfo->Decrypted = 1;
        }

        MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): SecMode = 0x%x, CipherMis = %d, CipherLenMis = %d, TkipMicErr = %d, IcvErr = %d\n", __FUNCTION__,
			rx_base->RxD2.SecMode, rx_base->RxD2.CipherMis, rx_base->RxD2.CipherLenMis, rx_base->RxD2.TkipMicErr, rx_base->RxD2.IcvErr));
    }

#ifdef MT7615
    if (IS_MT7615(pAd)) {
        if (pRxBlk->pRxInfo->U2M == 1)
        {
            TaTidRecAndCmp(pAd, rx_base, pRxBlk->SN);
        }
    }
#endif

    return RMACInfoLen;
}
#endif /* MT_MAC */


#define NUM_TYPE_STRING 8
UCHAR *rx_pkt_type_string [NUM_TYPE_STRING] = {
    "RMAC_RX_PKT_TYPE_RX_TXS", "RMAC_RX_PKT_TYPE_RX_TXRXV", "RMAC_RX_PKT_TYPE_RX_NORMAL",
    "RMAC_RX_PKT_TYPE_RX_DUP_RFB", "RMAC_RX_PKT_TYPE_RX_TMR", "Undefine Type 0x5",
    "Undefine Type 0x6", "RMAC_RX_PKT_TYPE_RX_EVENT"
};


#ifdef CUT_THROUGH
#define GET_TX_FREE_TOKEN_ID(_ptr, _idx) le2cpu16(*(UINT16 *)(((UINT8 *)(_ptr)) + 2 * (_idx)))
#define GET_RX_ORDER_TOKEN_ID(_ptr, _idx) le2cpu16(*(UINT16 *)(((UINT8 *)(_ptr)) + 4 * (_idx)))
#define GET_RX_ORDER_DROP(_ptr, _idx) ((*(UINT8 *)(((UINT8 *)(_ptr)) + 4 * (_idx) + 3)) & 0x01)

static VOID EventTxFreeNotifyHandler(RTMP_ADAPTER *pAd, UINT8 *dataPtr, UINT32 TxDCnt, UINT32 Len)
{
    UINT loop;
    PNDIS_PACKET pkt;
    UINT16 *token_ptr, token_id;
	PKT_TOKEN_CB *pktTokenCb = pAd->PktTokenCb;
	UINT8 Type;

    if (dataPtr == NULL)
    {
        ASSERT(0);
        return;
    }

	token_ptr = (UINT16 *)dataPtr;

#ifdef CUT_THROUGH_DBG
	if ((TxDCnt >= 0) && (TxDCnt < 32))
		pktTokenCb->tx_id_list.list->FreeAgg0_31++;
	else if ((TxDCnt >= 32) && (TxDCnt < 64))
		pktTokenCb->tx_id_list.list->FreeAgg32_63++;
	else if ((TxDCnt >= 64) && (TxDCnt < 96))
		pktTokenCb->tx_id_list.list->FreeAgg64_95++;
	else if ((TxDCnt >= 96) && (TxDCnt < 128))
		pktTokenCb->tx_id_list.list->FreeAgg96_127++;
#endif


	pktTokenCb->tx_id_list.list->TotalTxTokenEventCnt++;
	pktTokenCb->tx_id_list.list->TotalTxTokenCnt += TxDCnt;

    for (loop = 0; loop < TxDCnt; loop++)
    {
        if (loop * 2 > Len)
        {
            MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
                ("%s: token number len mismatch TxD Cnt=%d Len=%d\n",
                __FUNCTION__, loop, Len));
			hex_dump("EventTxFreeNotifyHandlerErrorFrame", dataPtr, Len);
            break;
        }

		token_id = *((UINT16 *)token_ptr);
#ifdef RT_BIG_ENDIAN
		token_id = cpu2le16(token_id);
#endif
		token_ptr++;
#ifdef CONFIG_HOTSPOT_R2
		/* already handled , do nothing */
		if(pktTokenCb->tx_id_list.list->pkt_token[token_id].Reprocessed)
		{
			pktTokenCb->tx_id_list.list->pkt_token[token_id].Reprocessed = FALSE;
			continue;
		}
#endif /* CONFIG_HOTSPOT_R2 */

		pkt = cut_through_tx_deq(pAd->PktTokenCb, token_id, &Type);
		/* debug only: measure the time interval between Pkt sended to free-notity comed back. */
		NdisGetSystemUpTime(&pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime);
		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_PROFILE, DBG_LVL_TRACE,
				("%s: tx time token_id[%d] = %ld %ld %ld OS_HZ=%d\n",
				__FUNCTION__, token_id,
				pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime -
				pktTokenCb->tx_id_list.list->pkt_token[token_id].startTime,
				pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime,
				pktTokenCb->tx_id_list.list->pkt_token[token_id].startTime,
				OS_HZ));

		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE,
				("%s: token_id[%d] = 0x%x, try to free TxPkt(%p)\n",
				__FUNCTION__, loop, token_id, pkt));

#ifdef CONFIG_ATE
		if(ATE_ON(pAd))
			MT_ATETxControl(pAd, 0xFF, pkt);
#endif
		if (pkt != NULL)
		{
#ifdef CONFIG_ATE
			if(ATE_ON(pAd))
			{
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
			else
#endif
			{
				if (Type == TOKEN_TX_DATA)
				{
					RELEASE_NDIS_PACKET_IRQ(pAd, pkt, NDIS_STATUS_SUCCESS);
				}
				else
				{
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
				}
			}
		}
		else
		{
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
				("%s: Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
				__FUNCTION__, loop, token_id));
			hex_dump("EventTxFreeNotifyHandlerNullPktPtrFrame", dataPtr, Len);
		}
	}

	if ((pktTokenCb->tx_id_list.list->FreeTokenCnt > pktTokenCb->TxTokenHighWaterMark) &&
				cut_through_tx_state(pktTokenCb, NO_ENOUGH_FREE_TX_TOKEN, NUM_OF_TX_RING))
	{
		cut_through_tx_flow_block(pktTokenCb, NULL, NO_ENOUGH_FREE_TX_TOKEN, FALSE, NUM_OF_TX_RING);
	}
}


static VOID EventRxOrderNotifyNotifyHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 RxDCnt, UINT32 Len)
{
    UINT loop;
    UINT16 token_id;
	UINT16 drop;
	PKT_TOKEN_CB *pktTokenCb = pAd->PktTokenCb;
#ifdef RX_CUT_THROUGH
	UINT8 Type;
#endif

    if (!CUT_THROUGH_RX_ENABL(pktTokenCb))
    {
		MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR, ("%s(): CutThrough Rx is disabled but still get RxReorderNotify!\n", __FUNCTION__));
		ASSERT(FALSE);
		return;
    }

    if (Data == NULL)
    {
        ASSERT(FALSE);
        return;
    }

#ifdef CUT_THROUGH_DBG
	if ((RxDCnt >= 0) && (RxDCnt < 32))
		pktTokenCb->rx_id_list.list->FreeAgg0_31++;
	else if ((RxDCnt >= 32) && (RxDCnt < 64))
		pktTokenCb->rx_id_list.list->FreeAgg32_63++;
	else if ((RxDCnt >= 64) && (RxDCnt < 96))
		pktTokenCb->rx_id_list.list->FreeAgg64_95++;
	else if ((RxDCnt >= 96) && (RxDCnt < 128))
		pktTokenCb->rx_id_list.list->FreeAgg96_127++;
#endif

	for (loop = 0; loop < RxDCnt; loop++)
    {
        if (loop * 4 > Len)
        {
            MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
                ("%s: token number len mismatch RxD Cnt=%d Len=%d\n",
                __FUNCTION__, loop, Len));
            break;
        }

        token_id = GET_RX_ORDER_TOKEN_ID(Data, loop);
		drop = GET_RX_ORDER_DROP(Data, loop);

        MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s: token_id[%d] = 0x%x\n",
            __FUNCTION__, loop, token_id));

#ifdef RX_CUT_THROUGH
		RTMP_SEM_LOCK(&((PKT_TOKEN_CB *)(pAd->PktTokenCb))->rx_order_notify_lock);
        if (cut_through_rx_mark_token_info(pAd->PktTokenCb, token_id, drop) &&
				cut_through_rx_rxdone(pAd->PktTokenCb, token_id))
        {
			PNDIS_PACKET pkt;
			RX_BLK *pRxBlk;

			pkt = cut_through_rx_deq(pAd->PktTokenCb,token_id, &Type);

			RTMP_SEM_UNLOCK(&((PKT_TOKEN_CB *)(pAd->PktTokenCb))->rx_order_notify_lock);
			if (pkt)
			{
				if (drop)
				{
#ifdef CUT_THROUGH_DBG
					pktTokenCb->rx_id_list.list->DropPktCnt++;
#endif
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_RESOURCES);
				}
				else
				{
					pRxBlk = (RX_BLK *)((UINT8 *)GET_OS_PKT_DATAPTR(pkt) - sizeof(RX_BLK) - RTMP_GET_RMACLEN(pkt));
					rx_packet_process(pAd, pkt, pRxBlk);
					MTWF_LOG(DBG_CAT_TOKEN, TOKEN_PROFILE, DBG_LVL_TRACE, ("%s: inorder time[%d] = %ld\n",
							__FUNCTION__, token_id, cut_through_inorder_time(pAd->PktTokenCb, token_id)));
				}
			}
        }
		else
		{
			RTMP_SEM_UNLOCK(&((PKT_TOKEN_CB *)(pAd->PktTokenCb))->rx_order_notify_lock);
		}
#endif /* RX_CUT_THROUGH */
    }

    return;
}
#endif /* CUT_THROUGH */


UINT32 rxv_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
	RMAC_RXD_0_TXRXV *rxv = (RMAC_RXD_0_TXRXV *)(GET_OS_PKT_DATAPTR(rx_packet));
	UCHAR *ptr;
	INT idx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,
            ("RxV Report: Number=%d, ByteCnt=%d\n",
	                rxv->RxvCnt, rxv->RxByteCnt));

#if (ENABLE_RXD_LOG)

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("RxV Report: Number=%d, ByteCnt=%d\n",
    	                rxv->RxvCnt, rxv->RxByteCnt));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_0_RxV: 0x%x\n", *((UINT32 *)rxv + 0)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_1_RxV: 0x%x\n", *((UINT32 *)rxv + 1)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_2_RxV: 0x%x\n", *((UINT32 *)rxv + 2)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_3_RxV: 0x%x\n", *((UINT32 *)rxv + 3)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_4_RxV: 0x%x\n", *((UINT32 *)rxv + 4)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_5_RxV: 0x%x\n", *((UINT32 *)rxv + 5)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_6_RxV: 0x%x\n", *((UINT32 *)rxv + 6)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_7_RxV: 0x%x\n", *((UINT32 *)rxv + 7)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_8_RxV: 0x%x\n", *((UINT32 *)rxv + 8)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_9_RxV: 0x%x\n", *((UINT32 *)rxv + 9)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_10_RxV: 0x%x\n", *((UINT32 *)rxv+ 10)));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("RMAC_RXD_11_RxV: 0x%x\n", *((UINT32 *)rxv+ 11)));
#endif


	if (rxv->RxByteCnt != (rxv->RxvCnt * 44 + 4))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("ReceivedByteCnt not equal rxv_entry required!\n"));
	}
	else
	{
		ptr = (UCHAR *)(GET_OS_PKT_DATAPTR(rx_packet) + 4);

#ifdef RT_BIG_ENDIAN
		if ((rxv->RxByteCnt - 4) > 0)
		{
			MTMacInfoEndianChange(pAd, ptr, TYPE_RMACINFO, rxv->RxByteCnt);
		}
#endif /* RT_BIG_ENDIAN */

		for (idx = 0; idx < rxv->RxvCnt; idx++)
		{
			ParseRxVPacket(pAd, RMAC_RX_PKT_TYPE_RX_TXRXV, rx_blk, ptr);
			ptr += 44;
		}
	}

	return TRUE;
}


INT32 txs_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
    RMAC_RXD_0_TXS *txs = (RMAC_RXD_0_TXS *)(GET_OS_PKT_DATAPTR(rx_packet));
    UCHAR *ptr;
    INT idx, txs_entry_len = 20;
#if defined(MT7615) || defined(MT7622)
    BOOLEAN dump_all = FALSE;
#endif /* defined(MT7615) || defined(MT7622) */

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                        ("TxS Report: Number=%d, ByteCnt=%d\n",
                        txs->TxSCnt, txs->TxSRxByteCnt));
#if defined(MT7615) || defined(MT7622)
	if (txs->TxSRxByteCnt == 0)
	{
        return TRUE;
    }
#endif
#if (ENABLE_RXD_LOG)

MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("TxS Report:TxSPktType =%d, Number=%d, ByteCnt=%d\n",
    txs->TxSPktType, txs->TxSCnt, txs->TxSRxByteCnt));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_0_TXS: 0x%x\n", *((UINT32 *)txs + 0)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_1_TXS: 0x%x\n", *((UINT32 *)txs + 1)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_2_TXS: 0x%x\n", *((UINT32 *)txs + 2)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_3_TXS: 0x%x\n", *((UINT32 *)txs + 3)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_4_TXS: 0x%x\n", *((UINT32 *)txs + 4)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_5_TXS: 0x%x\n", *((UINT32 *)txs + 5)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_6_TXS: 0x%x\n", *((UINT32 *)txs + 6)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_7_TXS: 0x%x\n", *((UINT32 *)txs + 7)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_8_TXS: 0x%x\n", *((UINT32 *)txs + 8)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_9_TXS: 0x%x\n", *((UINT32 *)txs + 9)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_10_TXS: 0x%x\n", *((UINT32 *)txs+ 10)));
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("RMAC_RXD_11_TXS: 0x%x\n", *((UINT32 *)txs+ 11)));
#endif


#if defined(MT7615) || defined(MT7622)
    if (IS_MT7615(pAd) || IS_MT7622(pAd))
        txs_entry_len = 28; // sizeof(TXS_STRUC); // 28 bytes
#endif

    if (txs->TxSRxByteCnt != (txs->TxSCnt * txs_entry_len + 4))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                            ("ReceivedByteCnt not equal txs_entry required!\n"));
    }
    else
    {
        ptr = ((UCHAR *)(GET_OS_PKT_DATAPTR(rx_packet))) + 4;
        for (idx = 0; idx < txs->TxSCnt; idx++)
        {
            TXS_STRUC *txs_entry = (TXS_STRUC *)ptr;
	   TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
#ifdef RT_BIG_ENDIAN
            TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
            TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
            TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
            *(((UINT32 *)TxSD0)) = SWAP32(*(((UINT32 *)TxSD0)));
            *(((UINT32 *)TxSD1)) = SWAP32(*(((UINT32 *)TxSD1)));
            *(((UINT32 *)TxSD2)) = SWAP32(*(((UINT32 *)TxSD2)));
            *(((UINT32 *)TxSD3)) = SWAP32(*(((UINT32 *)TxSD3)));
#endif /* RT_BIG_ENDIAN */

#if defined(MT7615) || defined(MT7622)
            if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
                INT32 stat;

#ifdef RT_BIG_ENDIAN
                {
                    TXS_D_5 *TxSD5 = &txs_entry->TxSD5;
                    TXS_D_6 *TxSD6 = &txs_entry->TxSD6;

                    *(((UINT32 *)TxSD5)) = SWAP32(*(((UINT32 *)TxSD5)));
                    *(((UINT32 *)TxSD6)) = SWAP32(*(((UINT32 *)TxSD6)));
                }
#endif /* RT_BIG_ENDIAN */

                stat = ParseTxSPacket(pAd, TxSD0->TxS_PId, TxSD0->TxSFmt, ptr);
                if (stat == -1)
                    dump_all = TRUE;
            }
#endif /* defined(MT7615) || defined(MT7622) */


            ptr += txs_entry_len;
        }

#if defined(MT7615) || defined(MT7622)
        if (dump_all == TRUE) {
            printk("%s(): Previous received TxS has error, dump raw data!\n", __FUNCTION__);
            printk("\tTxS Report: Cnt=%d, ByteCnt=%d\n", txs->TxSCnt, txs->TxSRxByteCnt);
            hex_dump("Raw data", ((UCHAR *)(GET_OS_PKT_DATAPTR(rx_packet))),
                                txs->TxSRxByteCnt > 400 ? 400 : txs->TxSRxByteCnt);
        }
#endif /* defined(MT7615) || defined(MT7622) */
    }

    return TRUE;
}


UINT32 rx_mcu_event_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{

#if (ENABLE_RXD_LOG)
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: \n",__FUNCTION__));
#endif


	if (ad->chipOps.rx_event_handler != NULL)
	{
#ifdef UNIFY_FW_CMD
		ad->chipOps.rx_event_handler(ad,
		GET_OS_PKT_DATAPTR(rx_packet) + sizeof(RXD_BASE_STRUCT));
#else
		ad->chipOps.rx_event_handler(ad, GET_OS_PKT_DATAPTR(rx_packet));
#endif /* UNIFY_FW_CMD */
	}

	if (rx_blk)
		RX_BLK_SET_FLAG(rx_blk, fRX_CMD_RSP);

	return TRUE;
}


#ifdef MT_PS
BOOLEAN rx_pkt_retrieve_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet, UINT32 *rx_hw_hdr_len)
{
	BOOLEAN need_return = FALSE;
	struct _RXD_BASE_STRUCT *rx_base;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)(GET_OS_PKT_DATAPTR(rx_packet));
	TMAC_TXD_1 *txd_1 = &txd_l->TxD1;
#ifdef RT_BIG_ENDIAN
	*(((UINT32 *)txd_1)) = SWAP32(*(((UINT32 *)txd_1)));
#endif /* RT_BIG_ENDIAN */

	UCHAR padding = txd_1->HdrPad & 0x03;
	UCHAR hdr_info = txd_1->HdrInfo *2;
	UCHAR *da, *sa;
	STA_TR_ENTRY *tr_entry;
	MAC_TABLE_ENTRY *pEntry;
	unsigned long IrqFlags;
	UINT32 q_idx = QID_AC_BE;
	HEADER_802_11 *pWifi_hdr;
	UCHAR *qos_p;
#ifdef UAPSD_SUPPORT
			UCHAR ac_idx = 0;
#endif /* UAPSD_SUPPORT */

	tr_entry = &ad->MacTab.tr_entry[txd_1->WlanIdx];
	pEntry = &ad->MacTab.Content[txd_1->WlanIdx];
	rx_base = (struct _RXD_BASE_STRUCT *)rxd_0;
	pWifi_hdr = (HEADER_802_11 *)(GET_OS_PKT_DATAPTR(rx_packet) + sizeof(TMAC_TXD_L));

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(ad, (PUCHAR)pWifi_hdr, DIR_READ, FALSE);
#endif

	MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("parse_rx_packet_type wlan_idx=%d,%d,%d,0x%x\n",
				txd_1->WlanIdx, hdr_info, padding, tr_entry->EntryType));

	if ((pWifi_hdr->FC.Type == FC_TYPE_CNTL) ||
		(pWifi_hdr->FC.Type == FC_TYPE_MGMT))
	{
		need_return = TRUE;
		goto done;
	}

	da = pWifi_hdr->Addr1;
	sa = pWifi_hdr->Addr2;
	qos_p = ((UCHAR *)pWifi_hdr) + sizeof(HEADER_802_11);

#ifdef UAPSD_SUPPORT
	/*
		Sanity Check for UAPSD condition for correct QoS index.
	*/
	if (qos_p[0] >= 8)
		qos_p[0] = 1; /* shout not be here */

	/* get the AC ID of incoming packet */
	ac_idx = WMM_UP2AC_MAP[qos_p[0]];
#endif /* UAPSD_SUPPORT */

	if ((tr_entry->ps_state == APPS_RETRIEVE_GOING)
		|| (tr_entry->ps_state == APPS_RETRIEVE_START_PS))
	{
		if (qos_p[1] == PS_RETRIEVE_TOKEN) //retrive_token : 0x76
		{
			rx_blk->DataSize = 0;
			tr_entry->ps_qbitmap &= (~(1 << qos_p[0])) ;
			if (tr_entry->ps_qbitmap == 0x0)
			{
				UINT32 wlan_idx = 0;
				int for_qid;

				if (tr_entry->ps_queue.Number)
				{
					MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								("(wcid=%d) put ps_queue packets(Number=%d) to tx_queue.\n",
								txd_1->WlanIdx, tr_entry->ps_queue.Number));
					MtEnqTxSwqFromPsQueue(ad, q_idx, tr_entry);
				}

				for (for_qid = 0; for_qid < WMM_QUE_NUM; for_qid++)
				{
					tr_entry->TokenCount[for_qid] = tr_entry->tx_queue[for_qid].Number;
				}
				tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
				MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							("(ps_state = %d)Receives all RMAC_RX_PKT_TYPE_RETRIEVE packets and send CMDTHREAD_PS_CLEAR cmd.\n",
							tr_entry->ps_state));
				wlan_idx = pEntry->wcid;
				RTMP_PS_RETRIVE_CLEAR(ad,pEntry->wcid);
			}
		}
		else
		{
			INT packet_length = 0;

			if ((pWifi_hdr->FC.SubType == SUBTYPE_DATA_NULL) || (pWifi_hdr->FC.SubType == SUBTYPE_QOS_NULL))
				return 0;

			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("da=(%02x:%02x:%02x:%02x:%02x:%02x), sa=(%02x:%02x:%02x:%02x:%02x:%02x), qos_p->ac%d, qos_p->value=%d, Sequence=%d txd_1->wlan_idx=%d\n"
						, PRINT_MAC(da), PRINT_MAC(sa), qos_p[0], qos_p[1], pWifi_hdr->Sequence,txd_1->WlanIdx));

			packet_length = rx_base->RxD0.RxByteCnt-sizeof(TMAC_TXD_L) - padding - hdr_info - sizeof(SNAP_802_1H);
			if (packet_length <= 0)
			{
				need_return = TRUE;
				goto done;
			}
			*rx_hw_hdr_len = sizeof(TMAC_TXD_L);
			SET_OS_PKT_DATAPTR(rx_packet, GET_OS_PKT_DATAPTR(rx_packet) + sizeof(TMAC_TXD_L) + padding + hdr_info + sizeof(SNAP_802_1H));
			SET_OS_PKT_LEN(rx_packet, packet_length);

			OS_PKT_HEAD_BUF_EXTEND(rx_packet, MAC_ADDR_LEN + MAC_ADDR_LEN);
			NdisCopyMemory(GET_OS_PKT_DATAPTR(rx_packet) + MAC_ADDR_LEN, sa, MAC_ADDR_LEN);
			NdisCopyMemory(GET_OS_PKT_DATAPTR(rx_packet), da, MAC_ADDR_LEN);

			RTMP_SET_PACKET_WCID(rx_packet, txd_1->WlanIdx);

			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("parse_rx_packet_type  not token txd_1->wlan_idx: %x, rx_packet addr: %x rx_wcid: %x\n",
				txd_1->WlanIdx, (u32)rx_packet,RTMP_GET_PACKET_WCID(rx_packet)));

			RTMP_SET_PACKET_WDEV(rx_packet, tr_entry->wdev->wdev_idx);

			/* sanity Check for UAPSD condition */
			if (qos_p[0] >= 8)
				qos_p[0] = 1; /* shout not be here */

			/* get the AC ID of incoming packet */
			q_idx = WMM_UP2AC_MAP[qos_p[0]];
			RTMP_IRQ_LOCK(&ad->irq_lock, IrqFlags);
			InsertTailQueue(&tr_entry->ps_queue, PACKET_TO_QUEUE_ENTRY(rx_packet));
			RTMP_IRQ_UNLOCK(&ad->irq_lock, IrqFlags);
#ifdef UAPSD_SUPPORT
			if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(&ad->MacTab.Content[tr_entry->wcid], q_idx))
			{
				/*
				1. the station is UAPSD station;
				2. one of AC is non-UAPSD (legacy) AC;
				3. the destinated AC of the packet is UAPSD AC.
				*/
				/* So we can not set TIM bit due to one of AC is legacy AC */
			}
			else
#endif /* UAPSD_SUPPORT */
			{
#ifdef CONFIG_AP_SUPPORT
				WLAN_MR_TIM_BIT_SET(ad, tr_entry->func_tb_idx, tr_entry->wcid);
#endif /* CONFIG_AP_SUPPORT */
			}
			RX_BLK_SET_FLAG(rx_blk, fRX_RETRIEVE);
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("da=(%02x:%02x:%02x:%02x:%02x:%02x), sa=(%02x:%02x:%02x:%02x:%02x:%02x), qos_p->ac%d, qos_p->value=%d, Sequence=%d txd_1->wlan_idx=%d, ps_state = %d\n"
					, PRINT_MAC(da), PRINT_MAC(sa), qos_p[0], qos_p[1], pWifi_hdr->Sequence,txd_1->WlanIdx, tr_entry->ps_state));
	}


done:
	return need_return;
}
#endif /* MT_PS */


BOOLEAN tmr_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{
    TMR_FRM_STRUC *tmr = (TMR_FRM_STRUC *)(GET_OS_PKT_DATAPTR(rx_packet));
	/*Tmr pkt comes to Host directly, there are some minor calculation need to do.*/
    TmrReportParser(ad, tmr, FALSE, 0); /* TMRv1.0 TOAE calibration result need to leverage EXT_EVENT_ID_TMR_CAL */
    return TRUE;
}


#ifdef CUT_THROUGH
#define TX_FREE_NOTIFY 0
#define RX_REORDER_NOTIFY 1
#define TXRX_NOTE_GET_EVENT_TYPE(_ptr) ((UINT8)(((UINT32)(((UINT8 *)(_ptr)) + 3) & 0x1e000000) >> 25))
#define TXRX_NOTE_GET_TXDCNT(_ptr) (le2cpu16((UINT16)(((UINT32)(((UINT8 *)(_ptr)) + 2) & 0x01ff0000) >> 16)))
#define TXRX_NOTE_GET_LEN(_ptr) (le2cpu16((UINT16)(((UINT32)(((UINT8 *)(_ptr)) + 1) & 0x0000ffff))))
#define TXRX_NOTE_GET_DATA(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))
#define TXRX_NOTE_GET_TOKEN_LIST(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))

UINT32 tx_free_event_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{
	UINT8 *ptr = GET_OS_PKT_DATAPTR(rx_packet);
	UINT32 dw0 = *(UINT32 *)ptr;
	UINT8 *tokenList;
	UINT8 tokenCnt;
	UINT16 rxByteCnt;
	UINT8 report_type;

	rxByteCnt = (dw0 & 0xffff);
	tokenCnt = ((dw0 & (0x7f << 16)) >> 16) & 0x7f;
	report_type = ((dw0 & (0x3f << 23)) >> 23) & 0x3f;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s: DW0=0x%08x, rxByteCnt=%d,tokenCnt= %d, ReportType=%d\n",
				__FUNCTION__, dw0, rxByteCnt, tokenCnt, report_type));

	if (tokenCnt > 0x7f)
	{
		printk("Invalid tokenCnt(%d)!\n", tokenCnt);
		return FALSE;
	}

	switch (report_type)
	{
		case TX_FREE_NOTIFY:
			if ((tokenCnt * 2 + 8) != rxByteCnt)
			{
				printk("tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", tokenCnt, rxByteCnt);
				hex_dump("TxFreeNotifyEventMisMatchFrame", ptr, rxByteCnt);
				return FALSE;
			}
			tokenList = TXRX_NOTE_GET_TOKEN_LIST(ptr);
			EventTxFreeNotifyHandler(ad, tokenList, tokenCnt, rxByteCnt - 8);
			break;

		case RX_REORDER_NOTIFY:
			if ((tokenCnt * 4 + 8) != rxByteCnt)
			{
				printk("tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", tokenCnt, rxByteCnt);
				hex_dump("EventRxOrderNotifyNotifyHandler_RawData", ptr, rxByteCnt);
				return FALSE;
			}
			tokenList = TXRX_NOTE_GET_TOKEN_LIST(ptr);
			EventRxOrderNotifyNotifyHandler(ad, tokenList, tokenCnt, rxByteCnt - 8);
			break;

		default:
			printk("Invalid type(%d)!\n", report_type);
			break;
	}

	return TRUE;
}
#endif /* CUT_THROUGH */


static UINT32 sanity_and_get_packet_type(RTMP_ADAPTER *ad, RX_BLK *rx_blk, PNDIS_PACKET rx_packet)
{
	union _RMAC_RXD_0_UNION *rxd_0;

	rxd_0 = (union _RMAC_RXD_0_UNION *)(GET_OS_PKT_DATAPTR(rx_packet));

#ifdef RT_BIG_ENDIAN
	mt_rmac_d0_endian_change(&rxd_0->word);
#endif /* RT_BIG_ENDIAN */

#if defined(RTMP_PCI_SUPPORT) && !defined(RX_SCATTER)
	if (RMAC_RX_PKT_RX_BYTE_COUNT(rxd_0->word) > RX_BUFFER_AGGRESIZE)
	{
		RXD_STRUC *pRxD = (RXD_STRUC *)&rx_blk->hw_rx_info[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		    ("%s():drop invalid RxD length = 0x%x packet, SDL0=%d\n", 
			__FUNCTION__, RMAC_RX_PKT_RX_BYTE_COUNT(rxd_0->word), 
                                                        pRxD->SDL0));

		return -1;
	}
#endif

	return RMAC_RX_PKT_TYPE(rxd_0->word);
}	

UINT32 pkt_alloc_fail_handle(RTMP_ADAPTER *ad, RX_BLK *rx_blk, PNDIS_PACKET rx_packet)
{
	UINT32 rx_pkt_type;
	
	rx_blk->Flags = 0;
	rx_pkt_type = sanity_and_get_packet_type(ad, rx_blk, rx_packet);

	switch (rx_pkt_type) {
#ifdef CUT_THROUGH
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		if (CUT_THROUGH_TX_ENABL(ad->PktTokenCb))
			tx_free_event_handler(ad, rx_blk, rx_packet);
		else
			printk("%s(): CUT_THROUGH_TX_ENABL is FALSE but receive TxFreeEventNotify!\n", __FUNCTION__);
		break;
	}
#endif /* CUT_THROUGH */

	return 0;
}

UINT32 MTFillRxBlkAndPacketProcess(RTMP_ADAPTER *ad, RX_BLK *rx_blk, PNDIS_PACKET rx_packet)
{
	UINT32 rx_hw_hdr_len = 0;
	UINT32 rx_pkt_type;

	rx_blk->Flags = 0;

	rx_pkt_type = sanity_and_get_packet_type(ad, rx_blk, rx_packet);

	switch (rx_pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_NORMAL:
	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		rx_hw_hdr_len = mt_rx_info_2_blk(ad, rx_blk, rx_packet, rx_pkt_type);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
#if (ENABLE_RXD_LOG)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:RMAC_RX_PKT_TYPE_RX_TXRXV\n", __FUNCTION__));
#endif
		rxv_handler(ad, rx_blk, rx_packet);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXS:
#if (ENABLE_RXD_LOG)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:RMAC_RX_PKT_TYPE_RX_TXS\n", __FUNCTION__));
#endif
		txs_handler(ad, rx_blk, rx_packet);
		break;

	case RMAC_RX_PKT_TYPE_RX_EVENT:
#if (ENABLE_RXD_LOG)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:RMAC_RX_PKT_TYPE_RX_EVENT\n", __FUNCTION__));
#endif
		rx_mcu_event_handler(ad, rx_blk, rx_packet);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
	{
		BOOLEAN need_return = FALSE;
		need_return = tmr_handler(ad, rx_blk, rx_packet);
		if (need_return == TRUE)
			return 0;
		else
			break;
	}

#ifdef CUT_THROUGH
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		if (CUT_THROUGH_TX_ENABL(ad->PktTokenCb))
			tx_free_event_handler(ad, rx_blk, rx_packet);
		else
			printk("%s(): CUT_THROUGH_TX_ENABL is FALSE but receive TxFreeEventNotify!\n", __FUNCTION__);
		break;
#endif /* CUT_THROUGH */

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Invalid PktType:%d\n", __FUNCTION__, rx_pkt_type));
		break;
	}


	if (rx_hw_hdr_len == 0) {
		rx_blk->DataSize = 0;
		rx_blk->ReleaseTheBlk = TRUE;
	}

	return rx_hw_hdr_len;
}
#endif /* MT_MAC */


#ifdef DOT11_N_SUPPORT
VOID RTMP_BASetup(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, UINT8 UPriority)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[tr_entry->wcid];

	if (pAd->CommonCfg.BACapability.field.AutoBA == FALSE)
		return;

	// TODO: shiang-usw, fix me for pEntry, we should replace this paramter as tr_entry!
	if ((tr_entry && tr_entry->EntryType != ENTRY_CAT_MCAST && VALID_UCAST_ENTRY_WCID(pAd, tr_entry->wcid)) &&
		(pEntry->NoBADataCountDown == 0) && IS_HT_STA(pEntry))
	{
		BOOLEAN isRalink = CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);


		if (((pEntry->TXBAbitmap & (1<<UPriority)) == 0)
			/* && ((pEntry->BADeclineBitmap & (1 << UPriority)) == 0) */
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			&& (!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
			&& ((isRalink || IS_ENTRY_MESH(pEntry) || IS_ENTRY_WDS(pEntry))
				|| (IS_NO_SECURITY(&pEntry->SecConfig)
				|| IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher)
				|| IS_CIPHER_CCMP256(pEntry->SecConfig.PairwiseCipher)
				|| IS_CIPHER_GCMP128(pEntry->SecConfig.PairwiseCipher)
				|| IS_CIPHER_GCMP256(pEntry->SecConfig.PairwiseCipher)
#ifdef WAPI_SUPPORT
				|| IS_CIPHER_WPI_SMS4(pEntry->SecConfig.PairwiseCipher)
#endif /* WAPI_SUPPORT */
				))
			)
		{
			BAOriSessionSetUp(pAd, pEntry, UPriority, 0, 10, FALSE);
		}
	}
}
#endif /* DOT11_N_SUPPORT */


/*
	========================================================================

	Routine Description:
		API for MLME to transmit management frame to AP (BSS Mode)
	or station (IBSS Mode)

	Arguments:
		pAd Pointer to our adapter
		pData		Pointer to the outgoing 802.11 frame
		Length		Size of outgoing management frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
#define MAX_DATAMM_RETRY	3

NDIS_STATUS MiniportMMRequest(RTMP_ADAPTER *pAd, UCHAR QueIdx, UCHAR *pData, UINT Length)
{
	PNDIS_PACKET pPacket;
	NDIS_STATUS Status = NDIS_STATUS_FAILURE;
	ULONG FreeNum;
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	BOOLEAN bUseDataQ = FALSE, FlgDataQForce = FALSE, FlgIsLocked = FALSE;
	int retryCnt = 0;
	BOOLEAN FlgIsCheckPS = FALSE;
#if defined(MT7615) || defined(MT7622)
	HEADER_802_11 *pHead = (HEADER_802_11*)pData;
	struct wifi_dev  *wdev = WdevSearchByAddress(pAd,pHead->Addr2);
#endif /* defined(MT7615) || defined(MT7622) */
	UCHAR RingIdx;
#if defined(MT7615) || defined(MT7622)
#ifdef MAC_REPEATER_SUPPORT
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    /* if we cannot found wdev from A2, it might comes from Rept entry.
        cause rept must bind the bssid of apcli_link,
        search A3(Bssid) to find the corresponding wdev.
    */
    if ((wdev == NULL) && (pAd->ApCfg.bMACRepeaterEn))
    {
        pReptEntry = RTMPLookupRepeaterCliEntry(
                                                pAd,
                                                FALSE,
                                                pHead->Addr2,
                                                TRUE);
        if (pReptEntry != NULL)
            wdev = pReptEntry->wdev;
        else
        {
            pReptEntry = RTMPLookupRepeaterCliEntry(
                                                pAd,
                                                TRUE,
                                                pHead->Addr2,
                                                TRUE);

            if (pReptEntry != NULL)
                wdev = pReptEntry->wdev;
        }

        if (wdev == NULL)
        {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s(): can not find registered wdev. %02x:%02x:%02x:%02x:%02x:%02x\n",
					 __FUNCTION__,PRINT_MAC(pHead->Addr2)));
            return NDIS_STATUS_FAILURE;
        }
    }
#endif
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP
    if(!IS_SINGLE_CHIP_DBDC(pAd)) {
        if ((wdev != NULL) && IS_EZ_SETUP_ENABLED(wdev)) {
#ifdef EZ_MOD_SUPPORT
			ez_acquire_lock(pAd, NULL,EZ_MINIPORT_LOCK);
#else
			RTMP_SEM_LOCK(&pAd->ez_miniport_lock);
#endif
		}
    }
#endif
#endif
	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	if ((QueIdx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG)
	{
		bUseDataQ = TRUE;
		QueIdx &= (~MGMT_USE_QUEUE_FLAG);
	}

#ifndef MT_PS
#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x1) {
		if (pAd->fpga_ctl.tx_kick_cnt > 0) {
			if (pAd->fpga_ctl.tx_kick_cnt < 0xffff)
				pAd->fpga_ctl.tx_kick_cnt--;
		}
		else
			return NDIS_STATUS_FAILURE;

		QueIdx = 0;
		//bUseDataQ = TRUE;
	}
#endif /* CONFIG_FPGA_MODE */
#endif /* MT_PS */

	if ((QueIdx & MGMT_USE_PS_FLAG) == MGMT_USE_PS_FLAG)
	{
		FlgIsCheckPS = TRUE;
		QueIdx &= (~MGMT_USE_PS_FLAG);
	}

#ifdef RTMP_MAC_PCI
	if (pAd->MACVersion == 0x28600100)
	{
		/* do not care about the version */
		QueIdx = (bUseDataQ ==TRUE ? QueIdx : 3);
		bUseDataQ = TRUE;
	}

	if ((bUseDataQ)
#if defined(MT7615) || defined(MT7622)
        /*
            becuse of MT7615 use same Tx ring to transmit all pkt.
            shall share same lock.
        */
        || (IS_MT7615(pAd) || IS_MT7622(pAd))
#endif /*defined(MT7615) || defined(MT7622) */
    )
	{
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		FlgIsLocked = TRUE;
		retryCnt = MAX_DATAMM_RETRY;
	}
#endif /* RTMP_MAC_PCI */

	do
	{
		/* Reset is in progress, stop immediately*/
		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
			 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
		)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		if (wdev != NULL) {
		    if (IsHcRadioCurStatOffByWdev(wdev)) {
			Status = NDIS_STATUS_FAILURE;
			break;
		    }
		} else {
		    Status = NDIS_STATUS_FAILURE;
		    break;
		}



#if defined(MT7615) || defined(MT7622)
		RingIdx = HcGetTxRingIdx(pAd,wdev);
#else
		RingIdx = QueIdx;
#endif
		/* Check Free priority queue*/
		/* Since we use PBF Queue2 for management frame.  Its corresponding DMA ring should be using TxRing.*/
#ifdef RTMP_MAC_PCI
		if (bUseDataQ)
		{
			/* free Tx(QueIdx) resources */
			RTMPFreeTXDUponTxDmaDone(pAd,QueIdx,RingIdx, TRUE);
			FreeNum = GET_TXRING_FREENO(pAd,RingIdx);
		}
		else
#endif /* RTMP_MAC_PCI */
		{
			FreeNum = GET_MGMTRING_FREENO(pAd,RingIdx);
		}

		if ((FreeNum > 0) || IS_MT7615(pAd))
		{
			INT hw_len;
			UCHAR rtmpHwHdr[40];

			hw_len = pAd->chipCap.tx_hw_hdr_len;
			ASSERT((sizeof(rtmpHwHdr) > hw_len));

			NdisZeroMemory(&rtmpHwHdr, hw_len);
			Status = RTMPAllocateNdisPacket(pAd, &pPacket, (UCHAR *)&rtmpHwHdr[0], hw_len, pData, Length);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
				break;
			}

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
			UAPSD_MR_QOS_NULL_HANDLE(pAd, pData, pPacket);
#endif /* UAPSD_SUPPORT */
#else
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
	        if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
			{
				UAPSD_MR_QOS_NULL_HANDLE(pAd, pData, pPacket);
			}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef RTMP_MAC_PCI
			if (bUseDataQ)
			{
				FlgDataQForce = TRUE;
				retryCnt--;
			}
#endif /* RTMP_MAC_PCI */

#if defined(MT7615) || defined(MT7622)
#ifdef MAC_REPEATER_SUPPORT
            RTMP_CLEAN_PKT_REPT_CLI_IDX(pPacket);
            if ((pReptEntry != NULL) && (pReptEntry->CliEnable == TRUE))
                RTMP_SET_PKT_REPT_CLI_IDX(pPacket, pReptEntry->MatchLinkIdx);
#endif
#endif

			Status = MlmeHardTransmit(pAd, QueIdx, pPacket, FlgDataQForce, FlgIsLocked, FlgIsCheckPS, wdev->channel);
			if (Status != NDIS_STATUS_SUCCESS && pPacket)
			{
				RELEASE_NDIS_PACKET(pAd, pPacket, Status);
			}
			retryCnt = 0;
		}
		else
		{
			pAd->RalinkCounters.MgmtRingFullCount++;
#ifdef RTMP_MAC_PCI
			if (bUseDataQ)
			{
				retryCnt--;
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("retryCnt %d\n", retryCnt));
				if (retryCnt == 0)
				{
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Qidx(%d), not enough space in DataRing, MgmtRingFullCount=%ld!\n",
											QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
				}
			}
			else
#endif /* RTMP_MAC_PCI */
			{

                                retryCnt--;
                                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                                                ("retryCnt %d\n", retryCnt));

				if (retryCnt == 0) {
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Qidx(%d), not enough space in MgmtRing, MgmtRingFullCount=%ld!\n",
								QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
				}
			}

		}
	} while (retryCnt > 0);


#ifdef RTMP_MAC_PCI
    if ((bUseDataQ)
#if defined(MT7615) || defined(MT7622)
        /*
            becuse of MT7615 use same Tx ring to transmit all pkt.
            shall share same lock.
        */
        || (IS_MT7615(pAd) || IS_MT7622(pAd))
#endif
    )
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP
	if(!IS_SINGLE_CHIP_DBDC(pAd)) {
		if ((wdev != NULL) && IS_EZ_SETUP_ENABLED(wdev)) {
#ifdef EZ_MOD_SUPPORT
			ez_release_lock(pAd, NULL, EZ_MINIPORT_LOCK);
#else
			RTMP_SEM_UNLOCK(&pAd->ez_miniport_lock);
#endif
		}
	}
#endif
#endif
	return Status;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		FlgIsDeltsFrame 1: the frame is a DELTS frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
void AP_QueuePsActionPacket(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN PNDIS_PACKET pPacket,
	IN BOOLEAN FlgIsDeltsFrame,
	IN BOOLEAN FlgIsLocked,
	IN UCHAR MgmtQid)
{
#ifdef UAPSD_SUPPORT
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
	PNDIS_PACKET DuplicatePkt = NULL;
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
#endif /* UAPSD_SUPPORT */
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	/* Note: for original mode of 4 AC are UAPSD, if station want to change
			the mode of a AC to legacy PS, we dont know where to put the
			response;
			1. send the response;
			2. but the station is in ps mode, so queue the response;
			3. we should queue the reponse to UAPSD queue because the station
				is not yet change its mode to legacy ps AC;
			4. so AP should change its mode to legacy ps AC only when the station
				sends a trigger frame and we send out the reponse;
			5. the mechanism is too complicate; */

#ifdef UAPSD_SUPPORT
	/*
		If the frame is action frame and the VO is UAPSD, we can not send the
		frame to VO queue, we need to send to legacy PS queue; or the frame
		maybe not got from QSTA.
	*/
/*    if ((pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) &&*/
/*		(FlgIsDeltsFrame == 0))*/
	if (pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid])
	{
		/* queue the management frame to VO queue if VO is deliver-enabled */
		MTWF_LOG(DBG_CAT_PS, CATPS_UAPSD, DBG_LVL_TRACE, ("ps> mgmt to UAPSD queue %d ... (IsDelts: %d)\n",
				MgmtQid, FlgIsDeltsFrame));

#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
		if (!pMacEntry->bAPSDAllAC)
		{
			/* duplicate one packet to legacy PS queue */
			RTMP_SET_PACKET_UAPSD(pPacket, 0, MgmtQid);
			DuplicatePkt = DuplicatePacket(wdev->if_dev, pPacket);
		}
		else
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		{
			RTMP_SET_PACKET_UAPSD(pPacket, 1, MgmtQid);
		}

		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, MgmtQid, FALSE);

		if (pMacEntry->bAPSDAllAC)
		{
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
		}
		else
		{
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
			/* duplicate one packet to legacy PS queue */

			/*
				Sometimes AP will send DELTS frame to STA but STA will not
				send any trigger frame to get the DELTS frame.
				We must force to send it so put another one in legacy PS
				queue.
			*/
			if (DuplicatePkt != NULL)
			{
				pPacket = DuplicatePkt;
				goto Label_Legacy_PS;
			}
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		}
	}
	else
#endif /* UAPSD_SUPPORT */
	{
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
Label_Legacy_PS:
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("ps> mgmt to legacy ps queue... (%d)\n", FlgIsDeltsFrame));

		if (tr_entry->ps_queue.Number >= MAX_PACKETS_IN_PS_QUEUE ||
			rtmp_enq_req(pAd, pPacket, MgmtQid, tr_entry, FlgIsLocked, NULL) == FALSE)
		{
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(%d): WLAN_TX_DROP, pPacket=%p, QueIdx=%d, ps_queue_num=%d, wcid=%d\n",
					__FUNCTION__, __LINE__, pPacket, MgmtQid, tr_entry->ps_queue.Number, tr_entry->wcid));
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
			return;
		}
		else
		{
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
		}
	}
}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS MlmeHardTransmit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN BOOLEAN FlgDataQForce,
	IN	BOOLEAN			FlgIsLocked,
	IN	BOOLEAN			FlgIsCheckPS,
	IN	UCHAR	Channel)
{
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
	HEADER_802_11 *pHeader_802_11;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
#endif /* CONFIG_AP_SUPPORT */
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
#ifdef MAC_REPEATER_SUPPORT
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    UCHAR   cliIdx = 0xff;
#endif


	if ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
		&& (Channel > 14)
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif /* CONFIG_AP_SUPPORT */
#endif /* CARRIER_DETECTION_SUPPORT */
		)
	{
		return NDIS_STATUS_FAILURE;
	}

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
		return NDIS_STATUS_FAILURE;
	}
#endif /* ERR_RECOVERY */


#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x1) {
		if (pAd->fpga_ctl.tx_kick_cnt > 0) {
			if (pAd->fpga_ctl.tx_kick_cnt < 0xffff)  {
				pAd->fpga_ctl.tx_kick_cnt--;
			}
		} else {
			return NDIS_STATUS_FAILURE;
		}
	}
#endif /* CONFIG_FPGA_MODE */

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT
	// TODO: shiang-7603
	pHeader_802_11 = (HEADER_802_11 *) (pSrcBufVA + tx_hw_hdr_len);

	/*
		Section 11.2.1.1 STA Power Management modes of IEEE802.11-2007:
		The Power Managment bit shall not be set in any management frame,
		except an Action frame.

		So in the 'baseline' test plan
		(Wi-Fi 802.11 WPA2, WPA, WEP Interoperability Test Plan),
		Section 2.2.6, the following Requirement:
        APs shall ignore the power save bit in any received Authenticate and
		(Re) Associate, and shall assume that the station is awake for the
		response.
	*/

	/*
		IEEE802.11, 11.2.1.4 AP operation during the contention period f)
		A single buffered MSDU or management frame for a STA in the PS mode shall
		be forwarded to the STA after a PS-Poll has been received from that STA.
		The More Data field shall be set to indicate the presence of further
		buffered MSDUs or "management frames" for the polling STA.
	*/

	/*
		IEEE802.11e, 11.2.1.4 Power management with APSD,
		An unscheduled SP ends after the QAP has attempted to transmit at least
		one MSDU or MMPDU associated with a delivery-enabled AC and destined for
		the non-AP QSTA, but no more than the number indicated in the Max SP
		Length field if the field has a nonzero value.
	*/

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA) ||
		(pHeader_802_11->FC.Type == FC_TYPE_MGMT))
	{
		if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) || (pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL))
			pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);

#ifdef MAC_REPEATER_SUPPORT
        if ((pEntry != NULL) && (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)))
        {
            cliIdx = RTMP_GET_PKT_REPT_CLI_IDX(pPacket);
            if (cliIdx != 0xff) { /*repeater case*/
                pReptEntry = &pAd->ApCfg.pRepeaterCliPool[cliIdx];
                if ((pReptEntry != NULL) &&
                    ((pReptEntry->CliEnable == TRUE) && (pReptEntry->CliValid == TRUE)))
                    pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
            } else { /*apcli case*/
				if (IS_ENTRY_REPEATER(pEntry)) {
					UCHAR apcli_wcid = 0;
					if (pEntry->wdev && (pEntry->wdev->func_idx < pAd->ApCfg.ApCliNum) ) {
						apcli_wcid = pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx].MacTabWCID;
					} else { // use default apcli0
						apcli_wcid = pAd->ApCfg.ApCliTab[0].MacTabWCID;
					}
					pEntry = &pAd->MacTab.Content[apcli_wcid];
				}
			}
        }
#endif
	}

#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
	if ((pEntry != NULL)
		&& (pEntry->func_tb_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_QUIET(pAd, pEntry->func_tb_idx))
	{
		return NDIS_STATUS_FAILURE;
	}
#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */


	if ((pEntry != NULL) &&
		(pEntry->PsMode == PWR_SAVE) &&
		(((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_DATA_NULL) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL)) ||
		((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION)) ||
		((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION_NO_ACK)) ||
			(FlgIsCheckPS == 1)))
	{
		/* the peer is in PS mode, we need to queue the management frame */
		UINT8 FlgIsDeltsFrame = 0, MgmtQid = QID_AC_VO;

		/*
			1. Data & Not QoS Null, or
			2. Management & Action, or
			3. Management & Action No ACK;
		*/
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA in ps mode, queue the mgmt frame\n"));
		RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);
		RTMP_SET_PACKET_MGMT_PKT(pPacket, 1); /* is management frame */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 0); /* default to management queue */


#ifdef RT_CFG80211_P2P_SUPPORT
		if(pEntry->wdev->wdev_type == WDEV_TYPE_GO)
		{
			//RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, MAIN_MBSSID);
			RTMP_SET_PACKET_OPMODE(pPacket, OPMODE_AP);
		}
#endif /* RT_CFG80211_P2P_SUPPORT */

		if (FlgDataQForce == TRUE)
			RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 1); /* force to data queue */

		if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
		{
			FRAME_ADDBA_REQ *pFrameBa = (FRAME_ADDBA_REQ *)pHeader_802_11;
			if (pFrameBa->Category == CATEGORY_BA)
				MgmtQid = QueIdx;
		}


#if defined(CONFIG_HOTSPOT_R2) || defined (CONFIG_DOT11V_WNM)
		if (((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
            (pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC)) ||
			((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
            (pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH)))
        {
			RTMP_SET_PACKET_DISASSOC(pPacket, 1);
			pEntry->IsKeep = 1;
		}
#endif /* CONFIG_HOTSPOT_R2 */

		AP_QueuePsActionPacket(pAd, pEntry, pPacket, FlgIsDeltsFrame,
								FlgIsLocked, MgmtQid);
		return NDIS_STATUS_SUCCESS;
	}
    else
#endif /* CONFIG_AP_SUPPORT */
    {
#ifdef RTMP_MAC_PCI
    	if (FlgDataQForce == TRUE)
    		return MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
    	else
#endif /* RTMP_MAC_PCI */
		{
#ifdef WH_EZ_SETUP		
			if((pEntry && IS_EZ_SETUP_ENABLED(pEntry->wdev))
				&& (pHeader_802_11->FC.Type == FC_TYPE_DATA) 
				&& (pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) 
				&& (pHeader_802_11->FC.PwrMgmt == PWR_SAVE))
			{
				return MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);				
			}
			else
#endif	//WH_EZ_SETUP			
			{
    			return MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);
			}
		}	
    }
}

#ifdef WH_EZ_SETUP
extern UCHAR dmac_wmm_aci_2_hw_ac_que[4][4];
#endif

NDIS_STATUS MlmeHardTransmitMgmtRing(RTMP_ADAPTER *pAd, UCHAR QueIdx, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA, *tmac_info;
	UINT SrcBufLen;
	HEADER_802_11 *pHeader_802_11;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR PID, wcid, tx_rate;
	HTTRANSMIT_SETTING *transmit;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	MAC_TX_INFO mac_info;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	UCHAR prot = 0;
	UCHAR apidx = 0;
	ULONG Flags = 0;
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
	UCHAR action_oui[4];
#endif /* defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) */
    NDIS_SPIN_LOCK *ring_lock = &pAd->MgmtRingLock;
#ifdef MAC_REPEATER_SUPPORT
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    UCHAR cliIdx = 0xFF;
#endif
	struct dev_rate_info *rate;

    RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
	{
		/* The buffer shouldn't be NULL*/
		return NDIS_STATUS_FAILURE;
	}

#if defined(MT7615) || defined(MT7622)
    // TODO: Shiang-MT7615, fix me for this!
    if (IS_MT7615(pAd) || IS_MT7622(pAd))
        ring_lock = &pAd->irq_lock;
#endif /* defined(MT7615) || defined(MT7622) */

    /* Make sure MGMT ring resource won't be used by other threads*/
#if defined(MT7615) || defined(MT7622)
    /*
        Carter, because of irq_lock in MT7615 is locked at MiniportMMRequest already.
        lock it again will cause double lock.
    */
    if (IS_MT7615(pAd) || IS_MT7622(pAd))
        ;
    else
#endif /* defined(MT7615) || defined(MT7622) */
        RTMP_IRQ_LOCK(ring_lock, Flags);

#ifdef MT_MAC
	// TODO: shiang-7603
    if (pAd->chipCap.hif_type == HIF_MT)
    {
	tmac_info = pSrcBufVA;
    }
    else
#endif /* MT_MAC */
    {
	tmac_info = pSrcBufVA + TXINFO_SIZE;
    }

	pHeader_802_11 = (HEADER_802_11 *) (pSrcBufVA + tx_hw_hdr_len);

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
    	pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);

#ifdef MAC_REPEATER_SUPPORT
        if ((pMacEntry != NULL) && (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)))
        {
            cliIdx = RTMP_GET_PKT_REPT_CLI_IDX(pPacket);
            if (cliIdx != 0xff)
            { /*repeater case*/
                pReptEntry = &pAd->ApCfg.pRepeaterCliPool[cliIdx];
                if ((pReptEntry != NULL) &&
                    ((pReptEntry->CliEnable == TRUE) && (pReptEntry->CliValid == TRUE)))
                {
                    pMacEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
                }
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
		if (pMacEntry){
			/*for probe response should follow A2 address*/
			if(pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP ||
			pHeader_802_11->FC.SubType == SUBTYPE_ASSOC_RSP ||
			pHeader_802_11->FC.SubType == SUBTYPE_AUTH ||
			pHeader_802_11->FC.SubType == SUBTYPE_REASSOC_RSP) {
				wdev = WdevSearchByAddress(pAd, pHeader_802_11->Addr2);
			}
			if (!wdev)
				wdev = pMacEntry->wdev;
		} else {
			wdev = WdevSearchByAddress(pAd, pHeader_802_11->Addr2);
		}
		if (!wdev)
			return NDIS_STATUS_FAILURE;
	}
#endif

	/*get rate info from wdev*/
	rate = &wdev->rate;

	/* Verify Mlme rate for a / g bands.*/
	if ((wdev->channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;



#ifdef RT_CFG80211_P2P_SUPPORT
// TODO: this patch would effect infra/adhoc station mlme data rate. @20140728
//	INT wdev_idx = rtmp_wdev_idx_find_by_p2p_ifaddr(pAd, pHeader_802_11->Addr2);

//	if (wdev_idx > 0)
	{
		if (rate->MlmeTransmit.field.MODE == MODE_CCK)
		{
			rate->MlmeTransmit.field.MODE = MODE_OFDM;
			rate->MlmeTransmit.field.MCS = MCS_RATE_6;
		}
	}
#endif /* RT_CFG80211_SUPPORT */

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/

			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;



#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(pPacket);
#endif /* CONFIG_AP_SUPPORT */


	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) /* must be PS-POLL*/
	{
		bAckRequired = FALSE;

#ifdef VHT_TXBF_SUPPORT
		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
		{
			pHeader_802_11->Duration = 100;
		}
#endif /* VHT_TXBF_SUPPORT */
	}
	else /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST*/
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
#ifdef SOFT_SOUNDING
			if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
				&& pMacEntry && (pMacEntry->snd_reqired == TRUE))
			{
				bAckRequired = FALSE;
				pHeader_802_11->Duration = 0;
			}
			else
#endif /* SOFT_SOUNDING */
			{
				bAckRequired = TRUE;
				pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
				if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT))
				{
					bInsertTimestamp = TRUE;
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Response*/
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
					/* Find which MBSSID to be send this probeRsp */
					UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

					if ( !(apidx >= pAd->ApCfg.BssidNum) &&
					     (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
					     (rate->MlmeTransmit.field.MODE == MODE_CCK) &&
					     (rate->MlmeTransmit.field.MCS == RATE_1))
					{
						TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
					}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				}
				else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT))
				{
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				}
				else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) &&
						 (pMacEntry == NULL))
				{
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence >0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE) && (wdev->channel > 14)))
	{
#if defined(MT7615) || defined(MT7622)
    /*
        Carter, because of irq_lock in MT7615 is locked/unlocked at MiniportMMRequest.
        don't need to unlock it here.
    */
        if (IS_MT7615(pAd) || IS_MT7622(pAd))
            ;
        else
#endif /* defined(MT7615) || defined(MT7622) */
            RTMP_IRQ_UNLOCK(ring_lock, Flags);
		return NDIS_STATUS_FAILURE;
	}

	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size

		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement

		management frame doesn't need encryption.
		so use RESERVED_WCID no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;


#ifdef DOT11W_PMF_SUPPORT
	PMF_PerformTxFrameAction(pAd, pHeader_802_11, SrcBufLen, tx_hw_hdr_len, &prot);
#endif

	if (pMacEntry == NULL)
	{
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

		tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
		transmit = &rate->MlmeTransmit;
#ifdef VHT_TXBF_SUPPORT
		if (pAd->NDPA_Request)
		{
			transmit->field.MODE = MODE_VHT;
			transmit->field.MCS = MCS_RATE_6;
		}
#endif
	}
	else
	{
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
		/* P2P Test Case 6.1.12, only OFDM rate can be captured by sniffer */
		if(
#ifdef RT_CFG80211_P2P_SUPPORT
		   (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
#endif /* RT_CFG80211_P2P_SUPPORT */
			((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)))
		{
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:: Using Low Rate to send QOS NULL!!\n", __FUNCTION__));
			pMacEntry->MaxHTPhyMode.field.MODE = 1;
			pMacEntry->MaxHTPhyMode.field.MCS = MCS_RATE_6;
		}
#endif /* P2P_SUPPORT || RT_CFG80211_SUPPORT */

		wcid = pMacEntry->wcid;
		tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
		transmit = &rate->MlmeTransmit;
	}

    NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	if(prot)
		mac_info.prot = prot;

	if (prot == 2)
		mac_info.bss_idx = apidx;

	mac_info.FRAG = FALSE;

	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;

	mac_info.Ack = bAckRequired;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;

	mac_info.WCID = wcid;
#ifdef MT_MAC
    mac_info.Type = pHeader_802_11->FC.Type;
    mac_info.SubType = pHeader_802_11->FC.SubType;

#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
	/* For  MT STA LP control, use H/W control mode for PM bit */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		mac_info.PsmBySw = 1;
	else
		mac_info.PsmBySw = 0;
#else
	mac_info.PsmBySw = 1;
#endif /* CONFIG_STA_SUPPORT && CONFIG_PM_BIT_HW_MODE */

    /* check if the pkt is Tmr frame. */
#ifdef TMR_VERIFY
    if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
        (pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
    {
        FRAME_FTM_ACTION *pFrameTmr = (FRAME_FTM_ACTION *)pHeader_802_11;
        if (pFrameTmr->Category == CATEGORY_PUBLIC)
            mac_info.IsTmr = TRUE;
    }
#else
#endif /* TMR_VERIFY */

	mac_info.Length = (SrcBufLen - tx_hw_hdr_len);
	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;
		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;
    }
    else if (pHeader_802_11->FC.Type == FC_TYPE_DATA)
    {
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
				{
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
						__FUNCTION__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
					hex_dump("DataFrame", (char *)pHeader_802_11, 24);
				}
				break;
		}

		if (pMacEntry && pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt)
			PID = PID_PS_DATA;

		mac_info.WCID = wcid;
	}
	else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL)
	{
		switch (pHeader_802_11->FC.SubType)
		{
			case SUBTYPE_PS_POLL:
				mac_info.hdr_len = sizeof (PSPOLL_FRAME);
				tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
				transmit = &rate->MlmeTransmit;
			break;

			default:
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
																		__FUNCTION__,
																		pHeader_802_11->FC.Type,
																		pHeader_802_11->FC.SubType));
			break;
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame send to MgmtRing, need to assign the length!\n",
					__FUNCTION__));
	}

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
	if (pHeader_802_11->Octet[0] == MT2_PEER_PUBLIC_CATE)
	{
		NdisCopyMemory(&action_oui[0], &pHeader_802_11->Octet[0] + 2, 4);
		SET_GO_NEGO_CONFIRM_TX_PID(pAd, &action_oui[0], PID);
	}
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */

#else
	mac_info.Length = (SrcBufLen - TXINFO_SIZE - pAd->chipCap.TXWISize - TSO_SIZE);
#endif

#ifdef TMR_VERIFY
    if (pAd->tmr_rate == 1) {
		transmit = &rate->MlmeTransmit;
        transmit->field.MODE = MODE_OFDM;
        transmit->field.BW = BW_20;
        transmit->field.MCS = MCS_RATE_6;
        tx_rate = transmit->field.MCS;
    }
    else if (pAd->tmr_rate == 2) {
		transmit = &rate->MlmeTransmit;
        transmit->field.MODE = MODE_HTMIX;
        transmit->field.BW = BW_20;
        transmit->field.MCS = MCS_0;
        tx_rate = transmit->field.MCS;
    }
    else if (pAd->tmr_rate == 3) {
		transmit = &rate->MlmeTransmit;
        transmit->field.MODE = MODE_HTMIX;
        transmit->field.BW = BW_40;
        transmit->field.MCS = MCS_0;
        tx_rate = transmit->field.MCS;
    }
    else {
        transmit->field.MODE = MODE_CCK;
        transmit->field.MCS = RATE_1;
        transmit->field.BW = BW_20;
        tx_rate = transmit->field.MCS;
    }
#endif /* TMR_VARIFY */

	mac_info.PID = PID;
	mac_info.TID = 0;
	mac_info.TxRate = tx_rate;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;

#ifdef MT_MAC
    mac_info.wmm_set = HcGetWmmIdx(pAd,wdev);
    mac_info.q_idx = HcGetMgmtQueueIdx(pAd,wdev);
#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev)
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT) 
		&& (pHeader_802_11->FC.SubType == SUBTYPE_ACTION)
		&& !(MAC_ADDR_EQUAL(pHeader_802_11->Addr1,ZERO_MAC_ADDR))
		){
		//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Action : Addr1:  %02x %02x %02x %02x %02x %02x  Addr2: %02x %02x %02x %02x %02x %02x, WMMIDX=%d \n",
		//	PRINT_MAC(pHeader_802_11->Addr1), 
		//	PRINT_MAC(pHeader_802_11->Addr2),HcGetWmmIdx(pAd,wdev)));
		mac_info.q_idx = dmac_wmm_aci_2_hw_ac_que[HcGetWmmIdx(pAd,wdev)][QID_AC_BE];
	}
 #endif   
	if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry))
        mac_info.OmacIdx = pAd->MacTab.tr_entry[pMacEntry->wcid].OmacIdx;
    else
        mac_info.OmacIdx = wdev->OmacIdx;
#endif /* MT_MAC */
    MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                        ("%s(): %d, WMMSET=%d,QId=%d\n",
                        __FUNCTION__,__LINE__,mac_info.wmm_set ,mac_info.q_idx));

#ifdef CONFIG_MULTI_CHANNEL
	if (pAd->Mlme.bStartMcc == TRUE)
	{
		if((NdisEqualMemory(pAd->cfg80211_ctrl.P2PCurrentAddress,pHeader_802_11->Addr2, MAC_ADDR_LEN))
			|| (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP))
			mac_info.q_idx =Q_IDX_AC10;
		else
			mac_info.q_idx = Q_IDX_AC0;
	}
#endif /* CONFIG_MULTI_CHANNEL */

	/* PCI use Miniport to send NULL frame and need to add NULL frame TxS control here to enter PSM */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef WH_EZ_SETUP
		
	if (IS_EZ_SETUP_ENABLED(wdev) 
#ifdef EZ_MOD_SUPPORT		
		&& wdev->ez_driver_params.need_tx_satus
#else
	    && wdev->ez_security.ez_action_type != ACTION_TYPE_DELAY_DISCONNECT
#endif
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT) 
		&& (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) 
		&& ((pHeader_802_11->Octet[0] == CATEGORY_PUBLIC))){
#ifdef EZ_MOD_SUPPORT		
		wdev->ez_driver_params.need_tx_satus = FALSE;
#endif
		mac_info.PID = PID_P2P_ACTION;
		TxSTypeCtlPerPkt(pAd, mac_info.PID, TXS_FORMAT0, FALSE, TRUE, FALSE, TXS_DUMP_REPEAT);
	}	
#endif

#ifdef MT_MAC
	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)))
	{
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

#ifdef SOFT_SOUNDING
	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
		&& pMacEntry && (pMacEntry->snd_reqired == TRUE))
	{
		wcid = RESERVED_WCID;
		tx_rate = (UCHAR)pMacEntry->snd_rate.field.MCS;
		transmit = &pMacEntry->snd_rate;

		mac_info.Txopmode = IFS_PIFS;

		if (write_tmac_info(pAd, tmac_info, &mac_info, transmit) == NDIS_STATUS_FAILURE) {
#if defined(MT7615) || defined(MT7622)
			if (IS_MT7615(pAd) || IS_MT7622(pAd))
				;
			else
#endif /* defined(MT7615) || defined(MT7622) */
			RTMP_IRQ_UNLOCK(ring_lock, Flags);
			return NDIS_STATUS_FAILURE;
		}

		pMacEntry->snd_reqired = FALSE;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Kick Sounding to %02x:%02x:%02x:%02x:%02x:%02x, dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
					__FUNCTION__, PRINT_MAC(pMacEntry->Addr),
					get_phymode_str(transmit->field.MODE),
					get_bw_str(transmit->field.BW),
					(transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf)));
	}
	else
#endif /* SOFT_SOUNDING */
	{
		mac_info.Txopmode = IFS_BACKOFF;
		
		if (write_tmac_info(pAd, tmac_info, &mac_info, transmit) == NDIS_STATUS_FAILURE) {
#if defined(MT7615) || defined(MT7622)
			if (IS_MT7615(pAd) || IS_MT7622(pAd))
				;
			else
#endif /* defined(MT7615) || defined(MT7622) */
			RTMP_IRQ_UNLOCK(ring_lock, Flags);
			return NDIS_STATUS_FAILURE;
		}

#ifdef SPECIFIC_TX_POWER_SUPPORT
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pMacEntry == NULL) {
			TXWI_STRUC *pFirstTxWI = (TXWI_STRUC *)tmac_info;

#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
	        		pFirstTxWI->TXWI_O.TxPwrAdj = TxPwrAdj;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
			if (pAd->chipCap.hif_type == HIF_RLT)
				pFirstTxWI->TXWI_N.TxPwrAdj = TxPwrAdj;
#endif /* RLT_MAC */
		}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	}

//---Add by shiang for debug
//+++Add by shiang for debug
if ((pHeader_802_11->FC.Type == FC_TYPE_CNTL) && (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA))
{
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Send VhtNDPA to peer(wcid=%d, pMacEntry=%p) with Mode=%d, txRate=%d, BW=%d\n",
				__FUNCTION__, wcid, pMacEntry, transmit->field.MODE, tx_rate, transmit->field.BW));
	hex_dump("VHT_NDPA raw data", pSrcBufVA, SrcBufLen);
	dump_tmac_info(pAd, tmac_info);
}

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


	/* Now do hardware-depened kick out.*/

	RTMP_SET_PACKET_WDEV(pPacket,wdev->wdev_idx);

//    /* if we are going to send out FTM action. enable CR to report TMR report.*/
//    if ((pAd->pTmrCtrlStruct != NULL) && (pAd->pTmrCtrlStruct->TmrEnable != TMR_DISABLE))
//    {
//        if (mac_info.IsTmr == TRUE)
//        {
//            /* Leo: already set at TmrCtrlInit() MtSetTmrCR(pAd, TMR_INITIATOR); */
//            pAd->pTmrCtrlStruct->TmrState = SEND_OUT;
//        }
//    }

	HAL_KickOutMgmtTx(pAd, mac_info.q_idx, pPacket, pSrcBufVA, SrcBufLen);

#if defined(CONFIG_HOTSPOT_R2) || defined (CONFIG_DOT11V_WNM)
	if (RTMP_GET_PACKET_DISASSOC(pPacket))
	{
		if (((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
	        (pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC)) ||
			((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
            (pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH)))
	    {
    	    pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
    	}
		if ((pMacEntry) && (pMacEntry->IsKeep == 1))
			MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
	}
#endif /* CONFIG_HOTSPOT_R2 */

	/* Make sure to release MGMT ring resource*/
/*	if (!IrqState)*/
#if defined(MT7615) || defined(MT7622)
    /*
        Carter, because of irq_lock in MT7615 is locked/unlocked at MiniportMMRequest.
        don't need to unlock it here.
    */
    if (IS_MT7615(pAd) || IS_MT7622(pAd))
        ;
    else
#endif /* defined(MT7615) || defined(MT7622) */
        RTMP_IRQ_UNLOCK(ring_lock, Flags);

	return NDIS_STATUS_SUCCESS;
}


/********************************************************************************

	New DeQueue Procedures.

 ********************************************************************************/
#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags) 				\
			do{													\
				if (bIntContext == FALSE)						\
				RTMP_IRQ_LOCK((lock), IrqFlags);		\
			}while(0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
			do{													\
				if (bIntContext == FALSE)						\
					RTMP_IRQ_UNLOCK((lock), IrqFlags);	\
			}while(0)


#define MCAST_WCID_TO_REMOVE 0 //Pat:


/*
	========================================================================
	Tx Path design algorithm:
		Basically, we divide the packets into four types, Broadcast/Multicast, 11N Rate(AMPDU, AMSDU, Normal), B/G Rate(ARALINK, Normal),
		Specific Packet Type. Following show the classification rule and policy for each kinds of packets.
				Classification Rule=>
					Multicast: (*addr1 & 0x01) == 0x01
					Specific : bDHCPFrame, bARPFrame, bEAPOLFrame, etc.
					11N Rate : If peer support HT
								(1).AMPDU  -- If TXBA is negotiated.
								(2).AMSDU  -- If AMSDU is capable for both peer and ourself.
											*). AMSDU can embedded in a AMPDU, but now we didn't support it.
								(3).Normal -- Other packets which send as 11n rate.

					B/G Rate : If peer is b/g only.
								(1).ARALINK-- If both of peer/us supprot Ralink proprietary Aggregation and the TxRate is large than RATE_6
								(2).Normal -- Other packets which send as b/g rate.
					Fragment:
								The packet must be unicast, NOT A-RALINK, NOT A-MSDU, NOT 11n, then can consider about fragment.

				Classified Packet Handle Rule=>
					Multicast:
								No ACK, 		pTxBlk->bAckRequired = FALSE;
								No WMM, 		pTxBlk->bWMM = FALSE;
								No piggyback,   pTxBlk->bPiggyBack = FALSE;
								Force LowRate,  pTxBlk->bForceLowRate = TRUE;
					Specific :	Basically, for specific packet, we should handle it specifically, but now all specific packets are use
									the same policy to handle it.
								Force LowRate,  pTxBlk->bForceLowRate = TRUE;

					11N Rate :
								No piggyback,	pTxBlk->bPiggyBack = FALSE;

								(1).AMSDU
									pTxBlk->bWMM = TRUE;
								(2).AMPDU
									pTxBlk->bWMM = TRUE;
								(3).Normal

					B/G Rate :
								(1).ARALINK

								(2).Normal
	========================================================================
*/
static UCHAR TxPktClassification(RTMP_ADAPTER *pAd, PNDIS_PACKET  pPacket, TX_BLK *pTxBlk)
{
	UCHAR TxFrameType = TX_LEGACY_FRAME;
	UCHAR Wcid;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	if (RTMP_GET_PACKET_TXTYPE(pPacket) == TX_MCAST_FRAME)
		return TX_MCAST_FRAME;

	/* Handle for unicast packets */
	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	tr_entry = &pAd->MacTab.tr_entry[Wcid];
	if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		pMacEntry = &pAd->MacTab.Content[Wcid];
	else
		pMacEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];

	pTxBlk->wdev = tr_entry->wdev;

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		TxFrameType = TX_LEGACY_FRAME;

		if ((RTMP_GET_PACKET_FRAGMENTS(pPacket) > 1)
#ifdef DOT11_N_SUPPORT
			 && ((pMacEntry->TXBAbitmap & (1 << (RTMP_GET_PACKET_UP(pPacket)))) == 0)
#endif /* DOT11_N_SUPPORT */
		)
		{
			TxFrameType = TX_FRAG_FRAME;
		}

		return TxFrameType;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	/* It's a specific packet need to force low rate, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame*/
	if ((RTMP_GET_PACKET_TXTYPE(pPacket) == TX_LEGACY_FRAME) || (pMacEntry->PsMode == PWR_SAVE))
		TxFrameType = TX_LEGACY_FRAME;
#ifdef DOT11_N_SUPPORT
	else if (IS_HT_RATE(pMacEntry))
	{

#ifdef VHT_TXBF_SUPPORT
		// TODO: shiang-usw, we should use cb here instead of mark the data type!
		if (pMacEntry->TxSndgType == SNDG_TYPE_NDP && IS_VHT_RATE(pMacEntry))
		{
//#error
			TxFrameType = TX_NDPA_FRAME;
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Err!! fix me for this!!\n", __FUNCTION__));
		}
#endif

		if (RTMP_GET_PACKET_MOREDATA(pPacket) || (pMacEntry->PsMode == PWR_SAVE))
			TxFrameType = TX_LEGACY_FRAME;
		else
#ifdef UAPSD_SUPPORT
		if (RTMP_GET_PACKET_EOSP(pPacket))
			TxFrameType = TX_LEGACY_FRAME;
		else
#endif /* UAPSD_SUPPORT */
#ifdef WFA_VHT_PF
		if (pAd->force_amsdu == TRUE)
			return TX_AMSDU_FRAME;
		else
#endif /* WFA_VHT_PF */
		if ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) != 0)
			return TX_AMPDU_FRAME;
		else if(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED)
		)
			return TX_AMSDU_FRAME;
	}
#endif /* DOT11_N_SUPPORT */
	else
	{	/* it's a legacy b/g packet.*/
		if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE) && pAd->CommonCfg.bAggregationCapable) &&
			(RTMP_GET_PACKET_TXRATE(pPacket) >= RATE_6) &&
			(!(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)))
		)
		{
			TxFrameType = TX_RALINK_FRAME;
		}
	}

	if ((RTMP_GET_PACKET_FRAGMENTS(pPacket) > 1)
		 && (TxFrameType == TX_LEGACY_FRAME)
#ifdef DOT11_N_SUPPORT
		&& ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) == 0)
#endif /* DOT11_N_SUPPORT */
		)
		TxFrameType = TX_FRAG_FRAME;

	return TxFrameType;
}


BOOLEAN RTMP_FillTxBlkInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UCHAR qid;
	UCHAR ack_policy;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
#ifdef TX_PKT_SG
	NdisMoveMemory( &pTxBlk->pkt_info, &PacketInfo, sizeof(PacketInfo));
#endif /* TX_PKT_SG */
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);

	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
#ifdef CONFIG_AP_SUPPORT
	pTxBlk->pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);

#ifdef WAPI_SUPPORT
	/* Check if this is an WPI data frame*/
	if ((RTMPIsWapiCipher(pAd, pTxBlk->wdev_idx) == TRUE) &&
		 (RTMP_GET_PACKET_WAI(pTxBlk->pPacket) == FALSE))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bWPIDataFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWPIDataFrame);
#endif /* WAPI_SUPPORT */

	if (pTxBlk->tr_entry->EntryType == ENTRY_CAT_MCAST)
	{
		pTxBlk->pMacEntry = NULL;
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		{
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff))
				pTxBlk->pTransmit = &pAd->CommonCfg.MCastPhyMode;
			else
#endif /* MCAST_RATE_SPECIFIC */
                        {
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;
	                        if(pTxBlk->wdev->channel > 14)
				{
					pTxBlk->pTransmit->field.MODE = MODE_OFDM;
					pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
				}
                        }
		}

		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));
		if (RTMP_GET_PACKET_MOREDATA(pPacket))
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
		}
	}
	else
	{
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
        if (!pTxBlk->pMacEntry)
        {
            MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                ("%s():Err!! pTxBlk->pMacEntry is NULL!!\n", __FUNCTION__));
            return FALSE;
        }
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;

		pMacEntry = pTxBlk->pMacEntry;
#ifdef CONFIG_AP_SUPPORT
		if(!pMacEntry)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __FUNCTION__));
		else
			pTxBlk->pMbss = pMacEntry->pMbss;
#endif /* CONFIG_AP_SUPPORT */

//YF MCC
#ifdef MULTI_WMM_SUPPORT
    	if (IS_ENTRY_APCLI(pMacEntry))
    	{
			pTxBlk->QueIdx = EDCA_WMM1_AC0_PIPE;
			printk("%s(): QIdx %d\n", __FUNCTION__, pTxBlk->QueIdx);
        }
#endif /* MULTI_WMM_SUPPORT */

		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
#ifdef MULTI_WMM_SUPPORT
		if (pTxBlk->QueIdx >= EDCA_WMM1_AC0_PIPE)
		{
			qid = pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE;
		}
		else
#endif /* MULTI_WMM_SUPPORT */
		{
			qid = pTxBlk->QueIdx;
		}
		ack_policy = pAd->CommonCfg.AckPolicy[qid];
		if(pTxBlk->wdev){
			ack_policy = wlan_config_get_ack_policy(pTxBlk->wdev,qid);
		}
		if (ack_policy != NORMAL_ACK)
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		else
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);


		{
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
			if (pTxBlk->OpMode == OPMODE_AP)
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT*/
			{
#ifdef WDS_SUPPORT
				if(IS_ENTRY_WDS(pMacEntry))
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWDSEntry);
				}
				else
#endif /* WDS_SUPPORT */
#ifdef MWDS
				if(IS_ENTRY_MWDS(pMacEntry))
				{
#ifdef APCLI_SUPPORT
					if(IS_MWDS_OPMODE_APCLI(pMacEntry))
					{
						TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
						if(!RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
							TX_BLK_SET_FLAG(pTxBlk, fTX_bMWDSFrame);

                        if(pMacEntry->func_tb_idx < MAX_APCLI_NUM)
						    pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
					}
					else
#endif /* APCLI_SUPPORT */
					if(IS_MWDS_OPMODE_AP(pMacEntry))
					{
						if (!RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
							TX_BLK_SET_FLAG(pTxBlk, fTX_bMWDSFrame);
					}
				}
				else
#endif /* MWDS */
#ifdef APCLI_SUPPORT
				if (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))
				{
#ifdef MAT_SUPPORT
					PNDIS_PACKET apCliPkt = NULL;
                    UCHAR *pMacAddr = NULL;

#ifdef MAC_REPEATER_SUPPORT
					if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn))
					{
						UCHAR tmpIdx;

						pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;
						if(pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR)
						{
							tmpIdx = REPT_MLME_START_IDX + pMacEntry->MatchReptCliIdx;
							// TODO: shiang-lock, fix ME!
							apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
                            pMacAddr = &pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress[0];
						}
					}
					else
#endif /* MAC_REPEATER_SUPPORT */
					{
						/* For each tx packet, update our MAT convert engine databases.*/
						/* CFG_TODO */
						apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pTxBlk->OpMode);
                        pMacAddr = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx].wdev.if_addr[0];
					}

					if(apCliPkt)
					{
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
						pPacket = apCliPkt;
						RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
						pTxBlk->pPacket = apCliPkt;
					}

                    if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
                    {
                        /*cuz the pkt will be TX_HDR by hw, change the 802.3 sa, too.*/
                        PUCHAR pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
                        NdisMoveMemory(pSrcBufVA+6, pMacAddr, MAC_ADDR_LEN);
                    }
#endif /* MAT_SUPPORT */
					pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);

				}
				else
#endif /* APCLI_SUPPORT */
#ifdef CLIENT_WDS
				if (IS_ENTRY_CLIWDS(pMacEntry))
				{
					PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
					PUCHAR pSA = GET_OS_PKT_DATAPTR(pPacket) + MAC_ADDR_LEN;
					if (((pMacEntry->func_tb_idx < MAX_MBSSID_NUM(pAd))
						&& !MAC_ADDR_EQUAL(pSA, pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].Bssid))
						|| !MAC_ADDR_EQUAL(pDA, pMacEntry->Addr)
						)
					{
						TX_BLK_SET_FLAG(pTxBlk, fTX_bClientWDSFrame);
					}
				}
				else
#endif /* CLIENT_WDS */
				if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
				{ }
				else
					return FALSE;

				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
#endif /* CONFIG_AP_SUPPORT */

		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME)
		{
			if ( ((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				&& (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				) ||
				((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			)
			{	/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

#ifdef WAPI_SUPPORT
				/* 	According to WAPIA certification description, WAI packets can not
					include QoS header */
				if (RTMP_GET_PACKET_WAI(pTxBlk->pPacket))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
				}
#endif /* WAPI_SUPPORT */
#ifdef DOT11_N_SUPPORT
				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry)
				{
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pTxBlk->pMacEntry is NULL!!\n", __FUNCTION__));
				}
				else if (IS_HT_STA(pTxBlk->pMacEntry) &&
					(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
					((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
				}
#endif /* DOT11_N_SUPPORT */
			}

#ifdef DOT11_N_SUPPORT
			if (!pMacEntry)
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __FUNCTION__));
			}
			else if ( (IS_HT_RATE(pMacEntry) == FALSE) &&
				(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)))
			{	/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}
#endif /* DOT11_N_SUPPORT */

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
			}
#ifdef UAPSD_SUPPORT
			if (RTMP_GET_PACKET_EOSP(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);
			}
#endif /* UAPSD_SUPPORT */
		}
		else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);
		}
		if (!pMacEntry)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __FUNCTION__));
		else
			pMacEntry->DebugTxCount++;
	}


	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;

	return TRUE;
}

#ifdef CUT_THROUGH
BOOLEAN RTMP_OffloadFillTxBlkInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	pTxBlk->wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);

	return TRUE;
}
#endif /* CUT_THROUGH */

BOOLEAN CanDoAggregateTransmit(RTMP_ADAPTER *pAd, NDIS_PACKET *pPacket, TX_BLK *pTxBlk)
{
	int minLen = LENGTH_802_3;
    int wcid = RTMP_GET_PACKET_WCID(pPacket);

	/*MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Check if can do aggregation! TxFrameType=%d!\n", pTxBlk->TxFrameType));*/

    if (IS_ENTRY_MCAST(&pAd->MacTab.Content[wcid]))
        return FALSE;

	if (RTMP_GET_PACKET_DHCP(pPacket) ||
		RTMP_GET_PACKET_EAPOL(pPacket) ||
		RTMP_GET_PACKET_WAI(pPacket)
	)
		return FALSE;

	/* Make sure the first packet has non-zero-length data payload */
	if (RTMP_GET_PACKET_VLAN(pPacket))
		minLen += LENGTH_802_1Q; /* VLAN tag */
	else if (RTMP_GET_PACKET_LLCSNAP(pPacket))
		minLen += 8; /* SNAP hdr Len*/
	if (minLen >= GET_OS_PKT_LEN(pPacket))
		return FALSE;

	if ((pTxBlk->TxFrameType == TX_AMSDU_FRAME) &&
		((pTxBlk->TotalFrameLen + GET_OS_PKT_LEN(pPacket))> (RX_BUFFER_AGGRESIZE - 100)))
	{	/* For AMSDU, allow the packets with total length < max-amsdu size*/
		return FALSE;
	}

	if ((pTxBlk->TxFrameType == TX_RALINK_FRAME) &&
		(pTxBlk->TxPacketList.Number == 2))
	{	/* For RALINK-Aggregation, allow two frames in one batch.*/
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	/* CFG_TODO */
	if ((MAC_ADDR_EQUAL(GET_OS_PKT_DATAPTR(pTxBlk->pPacket), GET_OS_PKT_DATAPTR(pPacket)))
	    && (pAd->OpMode == OPMODE_AP)) /* unicast to same STA*/
		return TRUE;
	else
#endif /* CONFIG_AP_SUPPORT */
		return FALSE;

}


VOID rtmp_sta_txq_dump(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, INT qidx)
{
	unsigned long IrqFlags;
	QUEUE_ENTRY *entry;
	INT cnt = 0;

	if ((tr_entry == NULL) || qidx >= WMM_QUE_NUM) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s():Invalid entry(%p) or qidx(%d)\n",
					__FUNCTION__, tr_entry, qidx));
		return;
	}

	RTMP_IRQ_LOCK(&pAd->irq_lock /*&tr_entry->txq_lock[qidx]*/, IrqFlags);
	entry = tr_entry->tx_queue[qidx].Head;
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump TxQ[%d] of TR_ENTRY(ID:%d, MAC:%02x:%02x:%02x:%02x:%02x:%02x): %s, enq_cap=%d\n",
				qidx, tr_entry->wcid, PRINT_MAC(tr_entry->Addr),
				entry == NULL ? "Empty" : "HasEntry", tr_entry->enq_cap));
	while(entry != NULL) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 0x%p ", entry));
		cnt++;
		entry = entry->Next;
		if (entry == NULL) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}
		if (cnt > tr_entry->tx_queue[qidx].Number) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						__FUNCTION__, qidx, tr_entry->tx_queue[qidx].Number));
		}
	};
	RTMP_IRQ_UNLOCK(&pAd->irq_lock /* &tr_entry->txq_lock[qidx]*/, IrqFlags);
}


VOID rtmp_tx_swq_dump(RTMP_ADAPTER *pAd, INT qidx)
{
	ULONG IrqFlags;
	UINT deq_id, enq_id, cnt = 0;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	deq_id = pAd->tx_swq[qidx].deqIdx;
	enq_id = pAd->tx_swq[qidx].enqIdx;
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\nDump TxSwQ[%d]: DeqIdx=%d, EnqIdx=%d, %s\n",
							qidx, deq_id, enq_id,
							(pAd->tx_swq[qidx].swq[deq_id] == 0 ? "Empty" : "HasEntry")));
	for (; deq_id != enq_id; (deq_id =  (deq_id == (TX_SWQ_FIFO_LEN - 1) ? 0 : deq_id+1)))
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d ", pAd->tx_swq[qidx].swq[deq_id]));
		cnt++;
		if (cnt > TX_SWQ_FIFO_LEN) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s(): Buggy here? force break! deq_id=%d, enq_id=%d\n",
						__FUNCTION__, deq_id, enq_id));
		}
	}
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
}


INT rtmp_tx_swq_init(RTMP_ADAPTER *pAd)
{
	ULONG IrqFlags;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	NdisZeroMemory(pAd->tx_swq, sizeof(pAd->tx_swq));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	return TRUE;
}


INT rtmp_tx_swq_exit(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	UCHAR ring_idx, wcid_start, wcid_end;
	STA_TR_ENTRY *tr_entry;
	ULONG IrqFlags;
	PNDIS_PACKET pPacket;
	QUEUE_ENTRY *pEntry;
	QUEUE_HEADER *pQueue;

	if (wcid == WCID_ALL) {
		wcid_start = 0;
		wcid_end = MAX_LEN_OF_TR_TABLE - 1;
	} else {
		if (wcid < MAX_LEN_OF_TR_TABLE)
			wcid_start = wcid_end = wcid;
		else
		{
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Invalid WCID[%d]\n",
						__FUNCTION__, wcid));
			return FALSE;
		}
	}

	for (wcid = wcid_start; wcid <= wcid_end; wcid++) {
		tr_entry = &pAd->MacTab.tr_entry[wcid];

		if (IS_ENTRY_NONE(tr_entry))
			continue;

		// TODO: shiang-usw, protect "tr_entry->enq_cap" here !!
		for (ring_idx = 0; ring_idx < WMM_QUE_NUM; ring_idx++)
		{
			RTMP_IRQ_LOCK(&pAd->irq_lock /* &tr_entry->txq_lock[ring_idx] */, IrqFlags);
			pQueue = &tr_entry->tx_queue[ring_idx];
			while (pQueue->Head)
			{
				pEntry = RemoveHeadQueue(pQueue);
				TR_ENQ_COUNT_DEC(tr_entry);
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
				if (pPacket)
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock /* &tr_entry->txq_lock[ring_idx] */, IrqFlags);
		}

		RTMP_IRQ_LOCK(&pAd->irq_lock /* &tr_entry->ps_queue_lock */, IrqFlags);
		pQueue = &tr_entry->ps_queue;
		while (pQueue->Head)
		{
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
		RTMP_IRQ_UNLOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
	}

	return TRUE;
}


INT rtmp_enq_req(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR qidx, STA_TR_ENTRY *tr_entry, BOOLEAN FlgIsLocked,QUEUE_HEADER *pPktQueue)
{
	unsigned long IrqFlags = 0;
	BOOLEAN enq_done = FALSE;
	INT enq_idx = 0;
	struct tx_swq_fifo *fifo_swq;
	UCHAR occupied_wcid = 0;
	QUEUE_ENTRY *pEntry;
	UINT capCount = 0;
	PNDIS_PACKET tmpPkt;

	ASSERT(qidx < WMM_QUE_NUM);
	ASSERT((tr_entry->wcid != 0));


	fifo_swq = &pAd->tx_swq[qidx];
	if (FlgIsLocked == FALSE)
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);


	if ((tr_entry->enqCount > SQ_ENQ_NORMAL_MAX)
		&&(tr_entry->tx_queue[qidx].Number > SQ_ENQ_RESERVE_PERAC))
	{
		occupied_wcid = fifo_swq->swq[enq_idx];
		enq_done = FALSE;
		goto enq_end;
	}

	if(pkt)
	{
	enq_idx = fifo_swq->enqIdx;
		if ((fifo_swq->swq[enq_idx] == 0) && (tr_entry->enq_cap))
		{
		InsertTailQueueAc(pAd, tr_entry, &tr_entry->tx_queue[qidx],
							PACKET_TO_QUEUE_ENTRY(pkt));
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
			tr_entry->TotalPageCount[qidx] += (INT16)(MTSDIOTxGetPageCount(GET_OS_PKT_LEN(pkt), FALSE));
#endif /* TC_PAGE_BASED_DEMAND */
#if DEBUG_ADAPTIVE_QUOTA
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wcid %d q %d pkt len %d TotalPageCount %d\n",
				__FUNCTION__, tr_entry->wcid, qidx, GET_OS_PKT_LEN(pkt),
				tr_entry->TotalPageCount[qidx]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */
		fifo_swq->swq[enq_idx] = tr_entry->wcid;
		INC_RING_INDEX(fifo_swq->enqIdx, TX_SWQ_FIFO_LEN);
			TR_ENQ_COUNT_INC(tr_entry);
		enq_done = TRUE;
		} else
	        {
		occupied_wcid = fifo_swq->swq[enq_idx];
		enq_done = FALSE;
			goto enq_end;
		}
	}

	/*check soft queue is enough*/
	capCount = (fifo_swq->enqIdx >=fifo_swq->deqIdx) ? (TX_SWQ_FIFO_LEN-fifo_swq->enqIdx+fifo_swq->deqIdx) : (fifo_swq->deqIdx-fifo_swq->enqIdx);
	/*insert full queue to soft queue*/
	if(pPktQueue && pPktQueue->Number <= capCount)
	{
		while(pPktQueue->Head)
		{
			pEntry = RemoveHeadQueue(pPktQueue);
			tmpPkt =  QUEUE_ENTRY_TO_PACKET(pEntry);

			enq_idx = fifo_swq->enqIdx;
			if ((fifo_swq->swq[enq_idx] == 0) && (tr_entry->enq_cap)) {
				InsertTailQueueAc(pAd, tr_entry, &tr_entry->tx_queue[qidx],tmpPkt);
				fifo_swq->swq[enq_idx] = tr_entry->wcid;
				INC_RING_INDEX(fifo_swq->enqIdx, TX_SWQ_FIFO_LEN);
				TR_ENQ_COUNT_INC(tr_entry);
				enq_done = TRUE;
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
				tr_entry->TotalPageCount[qidx] += (INT16)(MTSDIOTxGetPageCount(GET_OS_PKT_LEN(tmpPkt), FALSE));
#endif /* TC_PAGE_BASED_DEMAND */
#if DEBUG_ADAPTIVE_QUOTA
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wcid %d q %d pkt len %d TotalPageCount %d\n",
					__func__, tr_entry->wcid, qidx, GET_OS_PKT_LEN(tmpPkt),
					tr_entry->TotalPageCount[qidx]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */
			} else {
				occupied_wcid = fifo_swq->swq[enq_idx];
				enq_done = FALSE;
				goto enq_end;
			}
		}
	}

enq_end:
	if (FlgIsLocked == FALSE)
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():EnqPkt(%p) for WCID(%d) to tx_swq[%d].swq[%d] %s\n",
				__FUNCTION__, pkt, tr_entry->wcid, qidx, enq_idx,
				(enq_done ? "success" : "fail")));

	if (enq_done == FALSE) {
#ifdef DBG_DIAGNOSE
		if ((pAd->DiagStruct.inited) && (pAd->DiagStruct.wcid == tr_entry->wcid))
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].enq_fall_cnt[qidx]++;
#endif /* DBG_DIAGNOSE */
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t FailedCause =>OccupiedWCID:%d,EnqCap:%d\n",
					occupied_wcid, tr_entry->enq_cap));
	}
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
	else
	{
        /* Soumik: may need to move the to _RTMPDeQueuePacket()
         * for AP mode to help account for the demand due to
         * delivery of PS buffered frame. */
		MTAdaptiveResourceCheckFastAdjustment(pAd, tr_entry->wcid, qidx);
		MTAdaptiveResourceAllocation(pAd, WCID_ALL, NUM_OF_TX_RING);
	}
#endif

	if (0) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():EnqSuccess!\n", __FUNCTION__));
		rtmp_tx_swq_dump(pAd, qidx);
		rtmp_sta_txq_dump(pAd, tr_entry, qidx);
	}

	return enq_done;
}


INT rtmp_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info)
{
	CHAR deq_qid = 0, start_q, end_q;
	UCHAR deq_wcid;
	struct tx_swq_fifo *fifo_swq;
	STA_TR_ENTRY *tr_entry = NULL;

	if (!info->inited) {
		if (info->target_que < WMM_QUE_NUM) {
			info->start_q = info->target_que;
			info->end_q = info->target_que;
		}
		else
		{
			info->start_q = (WMM_QUE_NUM - 1);
			info->end_q = 0;
		}
		info->cur_q = info->start_q;

		if (info->target_wcid < MAX_LEN_OF_TR_TABLE) {
			info->pkt_cnt = cnt;
			info->full_qid[0] = FALSE;
			info->full_qid[1] = FALSE;
			info->full_qid[2] = FALSE;
			info->full_qid[3] = FALSE;
		} else {
			info->q_max_cnt[0] = cnt;
			info->q_max_cnt[1] = cnt;
			info->q_max_cnt[2] = cnt;
			info->q_max_cnt[3] = cnt;
		}
		info->inited = 1;
	}

	start_q = info->cur_q;
	end_q = info->end_q;

	/* wcid first */
	if (info->target_wcid < MAX_LEN_OF_TR_TABLE) {
		if (info->pkt_cnt <= 0) {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		deq_wcid = info->target_wcid;
		if (info->target_que >= WMM_QUE_NUM) {
			tr_entry = &pAd->MacTab.tr_entry[deq_wcid];
			for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
				if (info->full_qid[deq_qid] == FALSE && tr_entry->tx_queue[deq_qid].Number)
					break;
			}
		} else if (info->full_qid[info->target_que] == FALSE) {
			deq_qid = info->target_que;
		} else {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		if (deq_qid >= 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
		} else {
			info->status = NDIS_STATUS_FAILURE;
		}

		goto done;
	}

	/*
		Start from high queue,
		Don't need get "pAd->irq_lock" here becuse already get it by caller
	*/
	for (deq_qid = start_q; deq_qid >= end_q; deq_qid--)
	{
		fifo_swq = &pAd->tx_swq[deq_qid];
		deq_wcid = fifo_swq->swq[fifo_swq->deqIdx];

		if (deq_wcid == 0) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_LOUD,
						("%s():tx_swq[%d] emtpy!\n", __FUNCTION__, deq_qid));
			info->q_max_cnt[deq_qid] = 0;
			continue;
		}

		tr_entry = &pAd->MacTab.tr_entry[deq_wcid];

		/* If any stations are in Psm, AP will skip this swq and increase deqIdx */
		if (tr_entry->EntryType == ENTRY_CAT_MCAST)
		{	/* Broadcast/Multicast */
			if (pAd->MacTab.fAnyStationInPsm)
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_LOUD,
							("!!!B/MCast entry and AP in PSM\n"));

				fifo_swq->swq[fifo_swq->deqIdx] = 0;
				INC_RING_INDEX(fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);
				continue;
			}
		}
		else
		{	/* unicast */
			if ((tr_entry->PsMode == PWR_SAVE)
#ifdef MT_PS
			||((tr_entry->ps_state != APPS_RETRIEVE_IDLE) && (tr_entry->ps_state != APPS_RETRIEVE_DONE))
#endif /* MT_PS */
			)
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("!!!STA in PSM\n"));

				fifo_swq->swq[fifo_swq->deqIdx] = 0;
				INC_RING_INDEX(fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);
				if (tr_entry->tx_queue[deq_qid].Number > 0)
					TR_TOKEN_COUNT_INC(tr_entry, deq_qid);
				continue;
			}
		}

		if (info->q_max_cnt[deq_qid] > 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
			info->pkt_cnt = info->q_max_cnt[deq_qid];
			break;
		} else {
			/* Queue Quota full, go next! */
		}
	}

	if (deq_qid < end_q) {
		info->cur_q = deq_qid;
		info->status = NDIS_STATUS_FAILURE;
	}

done:
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s(): DeqReq %s, Start/End/Cur Queue=%d/%d/%d\n",
				__FUNCTION__,
				(info->status == NDIS_STATUS_SUCCESS ? "success" : "fail"),
				info->start_q, info->end_q, info->cur_q));

	if (info->status == NDIS_STATUS_SUCCESS) {
		tr_entry = &pAd->MacTab.tr_entry[info->cur_wcid];

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("\tdeq_info=>wcid:%d, qidx:%d, pkt_cnt:%d, q_max_cnt=%d, QueuedNum=%d\n",
					info->cur_wcid, info->cur_q, info->pkt_cnt, info->q_max_cnt[deq_qid],
					tr_entry->tx_queue[info->cur_q].Number));
	} else {
		info->status = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("\tdeq_info=>wcid:%d, qidx:%d, pkt_cnt:%d\n",
					info->cur_wcid, info->cur_q, info->pkt_cnt));
	}

#ifdef DBG
	if (0){
		if (deq_qid >= 0) {
			// cannot call this here because we already get lock but this function will try lock again!
			rtmp_tx_swq_dump(pAd, deq_qid);

			if (deq_wcid < MAX_LEN_OF_TR_TABLE)
				rtmp_sta_txq_dump(pAd, &pAd->MacTab.tr_entry[deq_wcid], deq_qid);
		}
	}
#endif /* DBG */

	return TRUE;
}


INT rtmp_deq_report(RTMP_ADAPTER *pAd, struct dequeue_info *info)
{
	UINT tx_cnt = info->deq_pkt_cnt, qidx = info->cur_q;
	struct tx_swq_fifo *fifo_swq;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():Success DeQ(QId=%d) for WCID(%d), PktCnt=%d, TxSWQDeQ/EnQ ID=%d/%d\n",
				__FUNCTION__, info->cur_q, info->cur_wcid, info->deq_pkt_cnt,
				pAd->tx_swq[qidx].deqIdx, pAd->tx_swq[qidx].enqIdx));

	if ((qidx < 4) && (tx_cnt > 0)) {
		fifo_swq = &pAd->tx_swq[qidx];
		do {
			if (fifo_swq->swq[fifo_swq->deqIdx]  == info->cur_wcid) {
				fifo_swq->swq[fifo_swq->deqIdx] = 0;
				INC_RING_INDEX(fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);
				tx_cnt--;
			} else
				break;
		}while (tx_cnt != 0);

		if (info->q_max_cnt[qidx] > 0)
			info->q_max_cnt[qidx] -= info->deq_pkt_cnt;

		if (info->target_wcid < MAX_LEN_OF_TR_TABLE)
			info->pkt_cnt -= info->deq_pkt_cnt;

		//rtmp_tx_swq_dump(pAd, qidx);
		//rtmp_sta_txq_dump(pAd, &pAd->MacTab.tr_entry[info->wcid], qidx);
	}

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("After DeqReport, tx_swq D/EQIdx=%d/%d, deq_info.q_max_cnt/pkt_cnt=%d/%d\n",
				pAd->tx_swq[qidx].deqIdx, pAd->tx_swq[qidx].enqIdx,
				info->q_max_cnt[qidx] , info->pkt_cnt));

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
INT deq_mgmt_frame(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR qIdx, BOOLEAN bLocked)
{
	NDIS_STATUS Status;

#ifdef RTMP_MAC_PCI
	if (RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(pkt) == 1)
	{
		//DEQUEUE_LOCK(&pAd->irq_lock, bLocked, IrqFlags);
		Status = MlmeHardTransmitTxRing(pAd,qIdx,pkt);
		//DEQUEUE_UNLOCK(&pAd->irq_lock, bLocked, IrqFlags);
	}
	else
#endif /* RTMP_MAC_PCI */
		Status = MlmeHardTransmitMgmtRing(pAd,qIdx,pkt);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("tx queued mgmt frame error!\n"));
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	}

	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */

#define ENTRY_RETRY_INTERVAL	(100 * OS_HZ / 1000)
static inline BOOLEAN traffic_jam_chk(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry)
{
	BOOLEAN drop_it = FALSE;

	if (!IS_ENTRY_NONE(tr_entry))
	{
		ULONG Now32;

		NdisGetSystemUpTime(&Now32);

#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
		if (IS_ENTRY_WDS(tr_entry))
		{
			if (tr_entry->LockEntryTx &&
				RTMP_TIME_BEFORE(Now32, tr_entry->TimeStamp_toTxRing + WDS_ENTRY_RETRY_INTERVAL))
				drop_it = TRUE;
		}
		else
#endif /* WDS_SUPPORT */
		if (tr_entry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
		{
			if(RTMP_TIME_BEFORE(Now32, tr_entry->TimeStamp_toTxRing + ENTRY_RETRY_INTERVAL))
				drop_it = TRUE;
		}
		else
#endif /* CONFIG_AP_SUPPORT */
		{
			tr_entry->TimeStamp_toTxRing = Now32;
		}
	}

	return drop_it;
}


INT deq_packet_gatter(RTMP_ADAPTER *pAd, struct dequeue_info *deq_info, TX_BLK *pTxBlk, BOOLEAN in_hwIRQ)
{
	STA_TR_ENTRY *tr_entry;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	ULONG hwd_cnt;
	PQUEUE_ENTRY qEntry = NULL;
	PNDIS_PACKET pPacket;
	PQUEUE_HEADER pQueue;
	UCHAR QueIdx = deq_info->cur_q;
	UCHAR wcid = deq_info->cur_wcid;
	UCHAR RingIdx = QueIdx;
	struct wifi_dev *wdev;

	tr_entry = &pAd->MacTab.tr_entry[wcid];
	if (VALID_UCAST_ENTRY_WCID(pAd, wcid))
		pMacEntry = &pAd->MacTab.Content[wcid];
	else
		pMacEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];

	wdev = tr_entry->wdev;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-->%s(): deq_info->wcid=%d, qidx=%d!\n",
				__FUNCTION__, wcid, QueIdx));

	deq_info->deq_pkt_cnt = 0;

#if defined(MT7615) || defined(MT7622)
	RingIdx = HcGetTxRingIdx(pAd,pMacEntry->wdev);
#endif

	do {
		hwd_cnt = GET_TXRING_FREENO(pAd,RingIdx);
		pQueue = &tr_entry->tx_queue[QueIdx];

dequeue:
		if ((qEntry = pQueue->Head) != NULL)
		{
			qEntry = RemoveHeadQueue(pQueue);
			TR_ENQ_COUNT_DEC(tr_entry);
			pPacket = QUEUE_ENTRY_TO_PACKET(qEntry);

			ASSERT(RTMP_GET_PACKET_WCID(pPacket) == wcid);

			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("-->%s(): GetPacket, wcid=%d, deq_pkt_cnt=%d, TotalFrameNum=%d\n",
						__FUNCTION__, wcid, deq_info->deq_pkt_cnt, pTxBlk->TotalFrameNum));

#ifdef CONFIG_AP_SUPPORT
			// TODO: shiang-usw, for mgmt frame enqueue to a dataQ but still go through mgmtRing here, is that good??
			if (RTMP_GET_PACKET_MGMT_PKT(pPacket)) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Call deq_mgmt_frame()!\n", __FUNCTION__));
#if defined(MT7615) || defined(MT7622)
				// TODO: Shiang-MT7615, fix me!!
				if (IS_MT7615(pAd) || IS_MT7622(pAd))
				{
					printk("%s(): May deadlock here because MgmtTx will try to acquire irq_lock but we already locked it!free this pkt\n",
						__FUNCTION__);
					hex_dump("DroppedMgmtPktInDeQueueFunc",
								GET_OS_PKT_DATAPTR(pPacket),
								GET_OS_PKT_LEN(pPacket));

					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				else
#endif /* defined(MT7615) || defined(MT7622) */
				deq_mgmt_frame(pAd, pPacket, QueIdx, in_hwIRQ);
				deq_info->deq_pkt_cnt++;

				goto dequeue;
			}
#endif /* CONFIG_AP_SUPPORT */

			// TODO: shiang-usw, remove this check to other place!!
			if (traffic_jam_chk(pAd, tr_entry) == TRUE) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): traffic jam detected! free pkt!\n", __FUNCTION__));
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

				deq_info->deq_pkt_cnt++;

				goto dequeue;
			}

			pTxBlk->TxFrameType = TxPktClassification(pAd, pPacket, pTxBlk);
			if (pTxBlk->TxFrameType & (TX_RALINK_FRAME | TX_AMSDU_FRAME)) {
				if (pTxBlk->TotalFrameNum == 0) {
					struct tx_swq_fifo *tx_swq = &pAd->tx_swq[QueIdx];
					BOOLEAN tx_swq_empty = FALSE;
					UINT diff = 0;

					if (tx_swq->enqIdx > tx_swq->deqIdx)
						diff = tx_swq->enqIdx - tx_swq->deqIdx;
					else if (tx_swq->enqIdx < tx_swq->deqIdx)
						diff = TX_SWQ_FIFO_LEN - tx_swq->deqIdx + tx_swq->enqIdx;

					if (diff == 1)
						tx_swq_empty = TRUE;

					if ((tx_swq_empty == TRUE) &&
						(NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx,
									hwd_cnt, pTxBlk->TxFrameType)))
					{
						InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
						TR_ENQ_COUNT_INC(tr_entry);
						deq_info->q_max_cnt[QueIdx] = 0;
						break;
					}
				}
				else
				{
					if (CanDoAggregateTransmit(pAd, pPacket, pTxBlk) == FALSE) {
						InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
						TR_ENQ_COUNT_INC(tr_entry);
						goto start_kick;
					}
				}
			}


			/*Note: RTMP_HAS_ENOUGH_FREE_DESC will use pTxBlk->QueIdx when USB mode*/
			pTxBlk->QueIdx = QueIdx;

#ifdef MT_MAC
#ifdef USE_BMC
			if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->MacTab.fAnyStationInPsm == 1) && (tr_entry->EntryType == ENTRY_CAT_MCAST))
			{
				pTxBlk->QueIdx = QID_BMC;
			}
#endif /* USE_BMC */
#endif

			/* Early check to make sure we have enoguh Tx Resource. */
			if (!RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, hwd_cnt, pPacket))
			{
				pAd->PrivateInfo.TxRingFullCnt++;
				InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
				TR_ENQ_COUNT_INC(tr_entry);

				if (deq_info->target_wcid < MAX_LEN_OF_TR_TABLE) {
					deq_info->full_qid[QueIdx] = TRUE;
				} else {
				deq_info->q_max_cnt[QueIdx] = 0;
				}
#ifdef DBG_DIAGNOSE
				if (pAd->DiagStruct.inited && pAd->DiagStruct.wcid == pTxBlk->Wcid) {
					struct dbg_diag_info *diag_info;

					diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
					diag_info->deq_fail_no_resource_cnt[QueIdx]++;
				}
#endif /* DBG_DIAGNOSE */

#ifdef RTMP_MAC_PCI
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): No free TxD(hwd_cnt=%ld, SWIDx=%d, CIDX=%d)\n",
							__FUNCTION__, hwd_cnt, pAd->PciHif.TxRing[QueIdx].TxSwFreeIdx,
							pAd->PciHif.TxRing[QueIdx].TxCpuIdx));
#else
                		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): deque break beacuse no free txResource\n", __FUNCTION__));
#endif /* RTMP_MAC_PCI */

				break;
			}

#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
			tr_entry->TotalPageCount[QueIdx] -= (INT16)(MTSDIOTxGetPageCount(GET_OS_PKT_LEN(pPacket), FALSE));
			if (tr_entry->TotalPageCount[QueIdx] < 0)
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): wcid %d queue %d negative page count %d\n",
							__func__, tr_entry->wcid, QueIdx, tr_entry->TotalPageCount[QueIdx]));
			}
#if DEBUG_ADAPTIVE_QUOTA
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wcid %d q %d pkt len %d TotalPageCount %d\n",
				__func__, tr_entry->wcid, QueIdx, GET_OS_PKT_LEN(pPacket),
				tr_entry->TotalPageCount[QueIdx]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#endif /* TC_PAGE_BASED_DEMAND */
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */

			pTxBlk->TotalFrameNum++;
			/* The real fragment number maybe vary*/
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);
			pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);

			if (pTxBlk->TotalFrameNum == 1) {
				pTxBlk->pPacket = pPacket;
				pTxBlk->wdev = wdev;
				pTxBlk->tr_entry = tr_entry;
				pTxBlk->pMacEntry = pMacEntry;
#ifdef VENDOR_FEATURE1_SUPPORT
				pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /* VENDOR_FEATURE1_SUPPORT */
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
				pTxBlk->OpMode = RTMP_GET_PACKET_OPMODE(pPacket);
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			}

			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
		}
		else
		{
			if (pTxBlk->TxPacketList.Number == 0)
			{
				deq_info->deq_pkt_cnt++;
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s():Try deQ a empty Q. pTxBlk.TxPktList.Num=%d, deq_info.pkt_cnt=%d\n",
					__FUNCTION__, pTxBlk->TxPacketList.Number, deq_info->pkt_cnt));
				break;
			}
		}

		if ((pTxBlk->TxFrameType & (TX_RALINK_FRAME | TX_AMSDU_FRAME)) &&
			(pTxBlk->TotalFrameNum == 1) && (pQueue->Head != NULL)) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("%s(): ForAMSDU/ARalink, try dequeue more pkt!\n",
						__FUNCTION__));
			goto dequeue;
		}

start_kick:
		if (pTxBlk->TxFrameType & (TX_RALINK_FRAME | TX_AMSDU_FRAME))
		{
			if (pTxBlk->TxPacketList.Number == 1)
				pTxBlk->TxFrameType = TX_LEGACY_FRAME;
		}

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s():pTxBlk.TxPktList.Num=%d, deq_info.pkt_cnt=%d\n",
					__FUNCTION__, pTxBlk->TxPacketList.Number, deq_info->pkt_cnt));
		break;
	} while(pTxBlk->TxPacketList.Number < deq_info->pkt_cnt);

	if (pTxBlk->TxPacketList.Number > 0) {
		deq_info->deq_pkt_cnt += pTxBlk->TxPacketList.Number;
		return TRUE;
	}
	else
		return FALSE;
}


/*
	========================================================================

	Routine Description:
		To do the enqueue operation and extract the first item of waiting
		list. If a number of available shared memory segments could meet
		the request of extracted item, the extracted item will be fragmented
		into shared memory segments.

	Arguments:
		pAd Pointer to our adapter
		pQueue		Pointer to Waiting Queue

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/

inline VOID _RTMPDeQueuePacket(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN in_hwIRQ,
	IN UCHAR QIdx,
	IN INT wcid,
	IN INT max_cnt)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT Count = 0, round  = 0;
	TX_BLK TxBlk, *pTxBlk = &TxBlk;
	UCHAR QueIdx = 0;
	unsigned long	IrqFlags = 0;
	struct dequeue_info deq_info = {0};


#ifdef DBG_DIAGNOSE
	if (pAd->DiagStruct.inited)
		dbg_diag_deque_log(pAd);
#endif /* DBG_DIAGNOSE */

	//NdisZeroMemory((UCHAR *)&deq_info, sizeof(deq_info));

	deq_info.target_wcid = ((wcid == WCID_ALL) ? MAX_LEN_OF_TR_TABLE : wcid);
	deq_info.target_que = QIdx;

	do
	{
		// TODO: shiang-usw, for another two options "IS_P2P_ABSENCE(pAd)" and RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE)
		// TODO: we need to take care that for per-entry based control "tr_entry->deq_cap"
		if (RTMP_TEST_FLAG(pAd, TX_FLAG_STOP_DEQUEUE))
			break;

#ifdef ERR_RECOVERY
        if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
            break;
        }
#endif /* ERR_RECOVERY */


		round++;

		DEQUEUE_LOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

		rtmp_deq_req(pAd, max_cnt, &deq_info);

		DEQUEUE_UNLOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

		if (deq_info.status == NDIS_STATUS_FAILURE) {
			break;
		}

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): deq_info:cur_wcid=%d, cur_qidx=%d, pkt_cnt=%d, pkt_bytes=%d\n",
					__FUNCTION__, deq_info.cur_wcid, deq_info.cur_q, deq_info.pkt_cnt, deq_info.pkt_bytes));

		QueIdx = deq_info.cur_q;
		NdisZeroMemory((UCHAR *)pTxBlk, sizeof(TX_BLK));

		RTMP_START_DEQUEUE(pAd, QueIdx, IrqFlags,deq_info);

		DEQUEUE_LOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

		deq_packet_gatter(pAd, &deq_info, pTxBlk, in_hwIRQ);

		DEQUEUE_UNLOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);


		if (pTxBlk->TotalFrameNum) {
			ASSERT(pTxBlk->wdev);
			if (pTxBlk->wdev)
				ASSERT(pTxBlk->wdev->wdev_hard_tx);

			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
				DEQUEUE_LOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

			if (pTxBlk->wdev && pTxBlk->wdev->wdev_hard_tx) {
				pTxBlk->wdev->wdev_hard_tx(pAd, pTxBlk);
			}
			else
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():ERR! pTxBlk->wdev=%p, wdev_hard_tx=%p\n",
							__FUNCTION__, pTxBlk->wdev,
							pTxBlk->wdev ? pTxBlk->wdev->wdev_hard_tx : NULL));

#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					Status = APHardTransmit(pAd, pTxBlk);
#endif /* CONFIG_AP_SUPPORT */
			}
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
				DEQUEUE_UNLOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

			Count += pTxBlk->TotalFrameNum;
		}

		RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);

		DEQUEUE_LOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

		rtmp_deq_report(pAd, &deq_info);

		DEQUEUE_UNLOCK(&pAd->irq_lock, in_hwIRQ, IrqFlags);

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): deq_packet_gatter %s, TotalFrmNum=%d\n",
					__FUNCTION__, (pTxBlk->TotalFrameNum > 0 ? "success" : "fail"),
					pTxBlk->TotalFrameNum));

#ifdef RTMP_MAC_PCI
		if (++pAd->FifoUpdateDone >= FIFO_STAT_READ_PERIOD)
		{
			// TODO: shiang-usw, check this because of REG access here!!
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateDone = 0;
		}
#endif /* RTMP_MAC_PCI */

#ifdef DBG
		if (round >= 1024) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():ForceToBreak!!Buggy here?\n", __FUNCTION__));
			break;
		}
#endif /* DBG */

	}while(1);

#ifdef DBG_DEQUE
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("--->%s():DeQueueRule:WCID[%d], Que[%d]\n",
				__FUNCTION__, deq_info.target_wcid, deq_info.target_que));
#endif /* DBG_DEQUE */

#ifdef DBG_DIAGNOSE
	if (pAd->DiagStruct.inited) {
		struct dbg_diag_info *diag_info;

		diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
		diag_info->deq_called++;
		diag_info->deq_round += round;
		if (Count < 8)
			diag_info->deq_cnt[Count]++;
		else
			diag_info->deq_cnt[8]++;
	}
#endif /* DBG_DIAGNOSE */


#ifdef BLOCK_NET_IF
	ASSERT((QueIdx < WMM_NUM_OF_AC));
	if ((pAd->blockQueueTab[QueIdx].SwTxQueueBlockFlag == TRUE)
		&& /* (pAd->TxSwQueue[QueIdx].Number < 1)*/
		(pAd->tx_swq[QueIdx].enqIdx == pAd->tx_swq[QueIdx].deqIdx)
	)
	{
		releaseNetIf(&pAd->blockQueueTab[QueIdx]);
	}
#endif /* BLOCK_NET_IF */
}





/*Entry point for DeQueuePacket*/
VOID RTMPDeQueuePacket(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN in_hwIRQ,
	IN UCHAR QIdx,
	IN INT wcid,
	IN INT max_cnt)
{

	_RTMPDeQueuePacket(pAd,in_hwIRQ,QIdx,wcid,max_cnt);
}


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pAd 	Pointer to our adapter
		Rate			Transmit rate
		Size			Frame size in units of byte

	Return Value:
		Duration number in units of usec

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
USHORT RTMPCalcDuration(RTMP_ADAPTER *pAd, UCHAR Rate, ULONG Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) /* CCK*/
	{
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	/* 72+24 preamble+plcp*/
		else
			Duration = 192; /* 144+48 preamble+plcp*/

		Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);
		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration ++;
	}
	else if (Rate <= RATE_LAST_OFDM_RATE)/* OFDM rates*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
		Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	}
	else	/*mimo rate*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
	}

	return (USHORT)Duration;
}


/*
	NOTE: we do have an assumption here, that Byte0 and Byte1
		always reasid at the same scatter gather buffer
 */
static inline VOID Sniff2BytesFromNdisBuffer(
	IN PNDIS_BUFFER buf,
	IN UCHAR offset,
	OUT UCHAR *p0,
	OUT UCHAR *p1)
{
	UCHAR *ptr = (UCHAR *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
BOOLEAN is_gratuitous_arp(UCHAR *pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP)
	{
		/* 
 		 * Check if Gratuitous ARP, Sender IP equal Target IP
 		 */
		SenderIP = Pos + 14;
		TargetIP = Pos + 24;
		if (NdisCmpMemory(SenderIP, TargetIP, 4) == 0) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
						("The Packet is GratuitousARP\n"));
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN is_dad_packet(RTMP_ADAPTER *pAd, UCHAR *pData)
{
	UCHAR *pSenderIP = pData + 16;
#ifdef MAC_REPEATER_SUPPORT	
	UCHAR *pSourceMac = pData + 10;
#endif /* MAC_REPEATER_SUPPORT */	
	UCHAR *pDestMac = pData + 20;
	UCHAR ZERO_IP_ADDR[4] = {0x00, 0x00, 0x00, 0x00};
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/* 
	* Check if DAD packet
	*/
	if (
#ifdef MAC_REPEATER_SUPPORT
		(RTMPLookupRepeaterCliEntry(pAd, FALSE, pSourceMac, TRUE) != NULL) &&
#endif /* MAC_REPEATER_SUPPORT */			
		((MAC_ADDR_EQUAL(pDestMac, BROADCAST_ADDR) == TRUE) ||
		(MAC_ADDR_EQUAL(pDestMac, ZERO_MAC_ADDR) == TRUE)) &&
		(RTMPEqualMemory(pSenderIP, ZERO_IP_ADDR, 4) == TRUE)) {

		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("DAD found, and do not send this packet\n"));
		return TRUE;
	}

	return FALSE;
}

BOOLEAN is_looping_packet(RTMP_ADAPTER *pAd, NDIS_PACKET *pPacket)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	UCHAR Byte0, Byte1;

	if (!pAd)
		return FALSE;

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);
	
	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	
	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500)
	{
		/*
			802.3, 802.3 LLC:
				DestMAC(6) + SrcMAC(6) + Lenght(2) +
				DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
		{
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
		}
		else
		{
			return FALSE;;
		}
	}
	
	if (TypeLen == ETH_TYPE_ARP)
	{
		/* AP's tx shall check DAD.*/
		if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
			return TRUE;
	}

	return FALSE;
}

void set_wf_fwd_cb(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, struct wifi_dev *wdev)
{	
	struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pPacket);
	RTMP_CLEAN_PACKET_BAND(pOsRxPkt);
	RTMP_CLEAN_PACKET_RECV_FROM(pOsRxPkt);

	if (wdev->channel >= H_CHANNEL_BIGGER_THAN)
	{
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G_H);
		if (wdev->wdev_type == WDEV_TYPE_AP)
		{
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_H_AP);
		}
		else if ((wdev->wdev_type == WDEV_TYPE_APCLI) || (wdev->wdev_type == WDEV_TYPE_REPEATER)) 
		{
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_H_CLIENT);
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("No Setting RTMP_SET_PACKET_RECV_FROM(5G_H) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type));
		}


	}
	else if (wdev->channel > 14)
	{
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G);
		if (wdev->wdev_type == WDEV_TYPE_AP)
		{
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_AP);
		}
		else if ((wdev->wdev_type == WDEV_TYPE_APCLI) || (wdev->wdev_type == WDEV_TYPE_REPEATER)) 
		{
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_CLIENT);
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("No Setting RTMP_SET_PACKET_RECV_FROM(5G) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type));
		}

	}
	else
	{
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_2G);

		if (wdev->wdev_type == WDEV_TYPE_AP)
		{
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_AP);
		}
		else if ((wdev->wdev_type == WDEV_TYPE_APCLI) || (wdev->wdev_type == WDEV_TYPE_REPEATER)) 
		{
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_CLIENT);
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("No Setting RTMP_SET_PACKET_RECV_FROM(2G) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type));
		}
	}	

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("set_wf_fwd_cb: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x=> BandFrom:0x%x, RecvdFrom: 0x%x\n",
	//	wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
	//	RTMP_GET_PACKET_BAND(pOsRxPkt),RTMP_GET_PACKET_RECV_FROM(pOsRxPkt)));

}

#endif /* CONFIG_WIFI_PKT_FWD */

VOID CheckQosMapUP(RTMP_ADAPTER *pAd,PMAC_TABLE_ENTRY pEntry, UCHAR DSCP, PUCHAR pUserPriority)
{				
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2)
	UCHAR i = 0, find_up = 0, dscpL = 0, dscpH = 0;
	BSS_STRUCT *pMbss = NULL;
	STA_TR_ENTRY *tr_entry = NULL;	

	if (pEntry == NULL || pEntry->wdev == NULL)
	{
		return;
	}
	else
	{
		//pAd = (RTMP_ADAPTER *)pEntry->pAd;
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	}

	if (IS_ENTRY_CLIENT(tr_entry))
		pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;
	else
		return;

	if (pEntry->QosMapSupport && pMbss->HotSpotCtrl.QosMapEnable) {
		for (i=0;i<(pEntry->DscpExceptionCount/2);i++) {
			if ((pEntry->DscpException[i] & 0xff) == DSCP) {
				*pUserPriority = (pEntry->DscpException[i]>>8) & 0xff;
				find_up = 1;
				break;
			}
		}
					
		if (!find_up) {
			for (i=0;i<8;i++) {
				dscpL = pEntry->DscpRange[i] & 0xff;
				dscpH = (pEntry->DscpRange[i]>>8) & 0xff;	
				if ((DSCP <= dscpH) && (DSCP >= dscpL)) {
					*pUserPriority = i;
					break;
				}
			}
		}
	}
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2) */
}


/*
	Check the Ethernet Frame type, and set RTMP_SET_PACKET_SPECIFIC flags
	Here we set the PACKET_SPECIFIC flags(LLC, VLAN, DHCP/ARP, EAPOL).
*/
BOOLEAN RTMPCheckEtherType(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN struct wifi_dev *wdev,
	OUT UCHAR *pUserPriority,
	OUT UCHAR *pQueIdx)
{
	UINT16 TypeLen;
	UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
	
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[tr_entry->wcid];

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);

	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);

	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;
	if (TypeLen <= 1500)
	{
		/*
			802.3, 802.3 LLC:
				DestMAC(6) + SrcMAC(6) + Lenght(2) +
				DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
		{
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
		} else {
			return FALSE;
		}
	}

	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (TypeLen == ETH_TYPE_VLAN)
	{
#ifdef CONFIG_AP_SUPPORT
		/*
			802.3 VLAN packets format:

			DstMAC(6B) + SrcMAC(6B)
			+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes)
			+ Length/Type(2B)
			+ data payload (42-1500 bytes)
			+ FCS(4B)

			VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* No matter unicast or multicast, discard it if not my VLAN packet. */
		if (wdev->VLAN_VID != 0)
		{
			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & 0x0FFF; /* 12 bit */
			if (vlan_id != wdev->VLAN_VID) {
MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():failed for VLAN_ID(vlan_id=%d, VLAN_VID=%d)\n",
					__FUNCTION__, vlan_id, wdev->VLAN_VID));
				return FALSE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);

#ifdef RTMP_UDMA_SUPPORT
		/*patch for vlan_udma TOS*/
		//vlanpriority = (*pSrcBuf & 0xe0) >> 5;
		
		if (pAd->CommonCfg.bUdmaFlag  == TRUE) 
		{								
			if (TypeLen == ETH_TYPE_IPv4)	/*For IPV4 packet*/
			{
				up = (*(pSrcBuf + 5) & 0xe0) >> 5;
			}
			else if(TypeLen == ETH_TYPE_IPv6) /*For IPV6 Packet*/
			{
				up = ((*(pSrcBuf +4)) & 0x0e) >> 1;
				
			}
		}
		else		
#endif	/* RTMP_UDMA_SUPPORT */
		{
			/* only use VLAN tag as UserPriority setting */
			up = (*pSrcBuf & 0xe0) >> 5;
		}
		CheckQosMapUP(pAd, pMacEntry, (*pSrcBuf & 0xfc) >> 2, &up);

		pSrcBuf += LENGTH_802_1Q; /* Skip the VLAN Header.*/
	}
	else if (TypeLen == ETH_TYPE_IPv4)
	{
		/*
			0       4       8          14  15                      31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen           |
			+---+----+-----+----+---------------+
			Ver    - 4bit Internet Protocol version number.
			IHL    - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN   - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
		*/
		up = (*(pSrcBuf + 1) & 0xe0) >> 5;
		CheckQosMapUP(pAd, pMacEntry, (*(pSrcBuf+1) & 0xfc) >> 2, &up);
	}
	else if (TypeLen == ETH_TYPE_IPv6)
	{
		/*
			0       4       8        12     16                      31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label                   |
			+---+----+----+--------------------+
			Ver           - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
		*/
		up = ((*pSrcBuf) & 0x0e) >> 1;
		CheckQosMapUP(pAd, pMacEntry, ((*pSrcBuf & 0x0f) << 2)|((*(pSrcBuf+1)) & 0xc0) >> 6, &up);
	}


	switch (TypeLen)
	{
		case ETH_TYPE_IPv4:
			{
				UINT32 pktLen = GET_OS_PKT_LEN(pPacket);

				ASSERT((pktLen > (ETH_HDR_LEN + IP_HDR_LEN)));	/* 14 for ethernet header, 20 for IP header*/
				RTMP_SET_PACKET_IPV4(pPacket, 1);
				if (*(pSrcBuf + 9) == IP_PROTO_UDP)
				{
					UINT16 srcPort, dstPort;

					pSrcBuf += IP_HDR_LEN;
					srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
					dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf+2)));

					if ((srcPort==0x44 && dstPort==0x43) || (srcPort==0x43 && dstPort==0x44))
					{	/*It's a BOOTP/DHCP packet*/
						RTMP_SET_PACKET_DHCP(pPacket, 1);
						RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
					}


#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_DOT11V_WNM
					WNMIPv4ProxyARPCheck(pAd, pPacket, srcPort, dstPort, pSrcBuf);
#endif
#ifdef CONFIG_HOTSPOT
					{
						USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);
						if (!HSIPv4Check(pAd, &Wcid, pPacket, pSrcBuf, srcPort, dstPort))
						return FALSE;
					}
#endif
#endif
				}
			}
			break;
		case ETH_TYPE_ARP:
			{
#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_DOT11V_WNM
				BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;
				if (pMbss->WNMCtrl.ProxyARPEnable)
				{
					/* Check if IPv4 Proxy ARP Candidate from DS */
					if (IsIPv4ProxyARPCandidate(pAd, pSrcBuf - 2))
					{
						BOOLEAN FoundProxyARPEntry;
						FoundProxyARPEntry = IPv4ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE);
						if (!FoundProxyARPEntry)
							MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Can not find proxy entry\n"));

						return FALSE;
					}
				}
#endif
#ifdef CONFIG_HOTSPOT
				if (pMbss->HotSpotCtrl.HotSpotEnable)
				{
					if (!pMbss->HotSpotCtrl.DGAFDisable)
					{
						if (IsGratuitousARP(pAd, pSrcBuf - 2, pSrcBuf-14, pMbss))
							return FALSE;
					}
				}

#endif
#endif
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
				if (wdev->wdev_type == WDEV_TYPE_AP) {
					/* AP's tx shall check DAD.*/
					if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
						return FALSE;
				}
#endif /* CONFIG_WIFI_PKT_FWD */


				RTMP_SET_PACKET_DHCP(pPacket, 1);
				RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
			}
			break;
		case ETH_P_IPV6:
			{
#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_DOT11V_WNM
				BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;
				WNMIPv6ProxyARPCheck(pAd, pPacket, pSrcBuf);
				if (pMbss->WNMCtrl.ProxyARPEnable)
				{
					/* Check if IPv6 Proxy ARP Candidate from DS */
					if (IsIPv6ProxyARPCandidate(pAd, pSrcBuf - 2))
					{
						BOOLEAN FoundProxyARPEntry;
						FoundProxyARPEntry = IPv6ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE);
						if (!FoundProxyARPEntry)
							MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Can not find IPv6 proxy entry\n"));

						return FALSE;
					}
				}
#endif
#ifdef CONFIG_HOTSPOT
				if (pMbss->HotSpotCtrl.HotSpotEnable)
				{
					if (!pMbss->HotSpotCtrl.DGAFDisable)
					{
						if (IsUnsolicitedNeighborAdver(pAd, pSrcBuf - 2))
							return FALSE;
					}
				}
#endif
				/*
					Check if DHCPv6 Packet, and Convert group-address DHCP
					packets to individually-addressed 802.11 frames
 				 */
#endif
				/* return AC_BE if packet is not IPv6 */
				if ((*pSrcBuf & 0xf0) != 0x60)
					up = 0;
			}
			break;
		case ETH_TYPE_EAPOL:
			RTMP_SET_PACKET_EAPOL(pPacket, 1);
			RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
			break;
#ifdef WAPI_SUPPORT
		case ETH_TYPE_WAI:
			RTMP_SET_PACKET_WAI(pPacket, 1);
			RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
			break;
#endif /* WAPI_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		case 0x890d:
			{
				RTMP_SET_PACKET_TDLS_MMPDU(pPacket, 1);
				RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
				up = 5;
			}
			break;
#endif /* DOT11Z_TDLS_SUPPORT */

		default:
			break;
	}

#ifdef VENDOR_FEATURE1_SUPPORT
	RTMP_SET_PACKET_PROTOCOL(pPacket, TypeLen);
#endif /* VENDOR_FEATURE1_SUPPORT */

	/* have to check ACM bit. downgrade UP & QueIdx before passing ACM*/
	/* NOTE: AP doesn't have to negotiate TSPEC. ACM is controlled purely via user setup, not protocol handshaking*/
	/*
		Under WMM ACM control, we dont need to check the bit;
		Or when a TSPEC is built for VO but we will change priority to
		BE here and when we issue a BA session, the BA session will
		be BE session, not VO session.
	*/
	if (pAd->CommonCfg.APEdcaParm[0].bACM[WMM_UP2AC_MAP[up]])
		up = 0;


	/*
		Set WMM when
		1. wdev->bWmmCapable == TRUE
		2. Receiver's capability
			a). bc/mc packets
				->Need to get UP for IGMP use
			b). unicast packets
				-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
		3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr
	*/
	if ((wdev->bWmmCapable == TRUE) && (up <= 7))
	{
		*pUserPriority = up;
		*pQueIdx = WMM_UP2AC_MAP[up];
	}

	return TRUE;
}


#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	UINT32	ex_head = 0, ex_tail = 0;
	UCHAR 	NumberOfFrag = RTMP_GET_PACKET_FRAGMENTS(pTxBlk->pPacket);

#ifdef WAPI_SUPPORT
	if (pTxBlk->CipherAlg == CIPHER_SMS4)
		ex_tail = LEN_WPI_MIC;
	else
#endif /* WAPI_SUPPORT */
	if (pTxBlk->CipherAlg == CIPHER_AES)
		ex_tail = LEN_CCMP_MIC;

	ex_tail = (NumberOfFrag * ex_tail);

	pTxBlk->pPacket = ExpandPacket(pAd, pTxBlk->pPacket, ex_head, ex_tail);
	if (pTxBlk->pPacket == NULL)
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: out of resource.\n", __FUNCTION__));
		return FALSE;
	}
	RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	return TRUE;
}


VOID RTMPUpdateSwCacheCipherInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHdr)
{
	HEADER_802_11 *pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader_802_11 = (HEADER_802_11 *) pHdr;
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry && pHeader_802_11->FC.Wep &&
		CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))
	{
		PCIPHER_KEY pKey = &pMacEntry->PairwiseKey;

		TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

		pTxBlk->CipherAlg = pKey->CipherAlg;
		pTxBlk->pKey = pKey;
#ifdef WAPI_SUPPORT
		pTxBlk->KeyIdx = pMacEntry->usk_id;

		/* TSC increment pre encryption transmittion */
		if (pKey->CipherAlg == CIPHER_SMS4)
			inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 2);
		else
#endif /* WAPI_SUPPORT */
		if ((pKey->CipherAlg == CIPHER_WEP64) || (pKey->CipherAlg == CIPHER_WEP128))
			inc_iv_byte(pKey->TxTsc, LEN_WEP_TSC, 1);
		else if ((pKey->CipherAlg == CIPHER_TKIP) || (pKey->CipherAlg == CIPHER_AES))
			inc_iv_byte(pKey->TxTsc, LEN_WPA_TSC, 1);

	}

}


INT tx_sw_encrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *pHeaderBufPtr, HEADER_802_11 *wifi_hdr)
{
	UCHAR iv_offset = 0, ext_offset = 0;

	/*
		If original Ethernet frame contains no LLC/SNAP,
		then an extra LLC/SNAP encap is required
	*/
	EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
					    pTxBlk->pExtraLlcSnapEncap);

	/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
	if (pTxBlk->pExtraLlcSnapEncap) {
		/* Reserve the front 8 bytes of data for LLC header */
		pTxBlk->pSrcBufData -= LENGTH_802_1_H;
		pTxBlk->SrcBufLen += LENGTH_802_1_H;

		NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);
	}

	/* Construct and insert specific IV header to MPDU header */
	RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
						   pTxBlk->KeyIdx,
						   pTxBlk->pKey->TxTsc,
						   pHeaderBufPtr, &iv_offset);
	pHeaderBufPtr += iv_offset;
	// TODO: shiang-MT7603, for header Len, shall we take care that??
	pTxBlk->MpduHeaderLen += iv_offset;

	/* Encrypt the MPDU data by software */
	RTMPSoftEncryptionAction(pAd,
							 pTxBlk->CipherAlg,
							 (UCHAR *)wifi_hdr,
							pTxBlk->pSrcBufData,
							pTxBlk->SrcBufLen,
							pTxBlk->KeyIdx,
							pTxBlk->pKey, &ext_offset);
	pTxBlk->SrcBufLen += ext_offset;
	pTxBlk->TotalFrameLen += ext_offset;

	return TRUE;
}
#endif /* SOFT_ENCRYPT */

#ifdef IP_ASSEMBLY

/*for cache usage to improve throughput*/
static IP_ASSEMBLE_DATA *pCurIpAsmData[NUM_OF_TX_RING];

 static INT rtmp_IpAssembleDataCreate(RTMP_ADAPTER *pAd,UCHAR queId,IP_ASSEMBLE_DATA **ppIpAsmbData,UINT id,UINT fragSize)
{
	ULONG now = 0;
	IP_ASSEMBLE_DATA *pIpAsmbData = NULL;
	DL_LIST *pAssHead = &pAd->assebQueue[queId];

	os_alloc_mem(NULL,(UCHAR**)&pIpAsmbData,sizeof(IP_ASSEMBLE_DATA));

	*ppIpAsmbData = pIpAsmbData;

	if(pIpAsmbData==NULL)
	{
		return NDIS_STATUS_FAILURE;
	}

	InitializeQueueHeader(&pIpAsmbData->queue);
	NdisGetSystemUpTime(&now);
	pIpAsmbData->identify = id;
	pIpAsmbData->fragSize = fragSize;
	pIpAsmbData->createTime = now;

	DlListAdd(pAssHead,&pIpAsmbData->list);

	return NDIS_STATUS_SUCCESS;
}


static VOID rtmp_IpAssembleDataDestory(RTMP_ADAPTER *pAd,IP_ASSEMBLE_DATA *pIpAsmbData)
{
	PQUEUE_ENTRY pPktEntry;
	PNDIS_PACKET pPkt;

	/*free queue packet*/
	while (1)
	{
		pPktEntry = RemoveHeadQueue(&pIpAsmbData->queue);
		if (pPktEntry == NULL)
		{
			break;
		}
		pPkt = QUEUE_ENTRY_TO_PACKET(pPktEntry);
		RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);
	}
	/*remove from list*/
	DlListDel(&pIpAsmbData->list);
	/*free data*/
	os_free_mem(pIpAsmbData);
}


static IP_ASSEMBLE_DATA* rtmp_IpAssembleDataSearch(RTMP_ADAPTER *pAd, UCHAR queIdx, UINT identify)
{
	DL_LIST *pAssHead = &pAd->assebQueue[queIdx];
	IP_ASSEMBLE_DATA *pAssData = NULL;

	DlListForEach(pAssData,pAssHead,struct ip_assemble_data,list)
	{
		if(pAssData->identify == identify)
		{
			return pAssData;
		}
	}
	return NULL;
}


static VOID rtmp_IpAssembleDataUpdate(RTMP_ADAPTER *pAd)
{
	DL_LIST *pAssHead = NULL;
	IP_ASSEMBLE_DATA *pAssData = NULL,*pNextAssData=NULL;
	INT i=0;
	ULONG now = 0;
	QUEUE_HEADER *pAcQueue = NULL;

	NdisGetSystemUpTime(&now);
	for(i=0;i<NUM_OF_TX_RING;i++)
	{
		pAssHead = &pAd->assebQueue[i];
		DlListForEachSafe(pAssData,pNextAssData,pAssHead,struct ip_assemble_data,list)
		{
			pAcQueue = &pAssData->queue;
			if ((pAcQueue->Number != 0) && (RTMP_TIME_AFTER(now, (pAssData->createTime) + (1000 * OS_HZ))))
			{
				if(pCurIpAsmData[i] == pAssData)
				{
					pCurIpAsmData[i] = NULL;
				}
				rtmp_IpAssembleDataDestory(pAd,pAssData);
			}
		}
	}
}


INT rtmp_IpAssembleHandle(RTMP_ADAPTER *pAd,STA_TR_ENTRY *pTrEntry , PNDIS_PACKET pPacket,UCHAR queIdx,PACKET_INFO packetInfo)
{
	IP_ASSEMBLE_DATA *pIpAsmData = NULL;
	/*define local variable*/
	IP_V4_HDR *pIpv4Hdr, Ipv4Hdr;
	IP_FLAGS_FRAG_OFFSET *pFlagsFragOffset, flagsFragOffset;
	UINT fragSize = 0;
	QUEUE_HEADER *pAcQueue = NULL;
	UINT32 fragCount = 0;


	/*check all timer of assemble for ageout */
	rtmp_IpAssembleDataUpdate(pAd);

	/*is not ipv4 packet*/
	if (!RTMP_GET_PACKET_IPV4(pPacket))
	{
		/*continue to do normal path*/
		return NDIS_STATUS_INVALID_DATA;
	}

	pFlagsFragOffset = (IP_FLAGS_FRAG_OFFSET *) (packetInfo.pFirstBuffer + LENGTH_802_3 + 6);
	flagsFragOffset.word = ntohs(pFlagsFragOffset->word);


	/*is not fragment packet*/
	if(flagsFragOffset.field.flags_more_frag == 0 && flagsFragOffset.field.frag_offset == 0)
	{
		/*continue to do normal path*/
		return NDIS_STATUS_INVALID_DATA;
	}

	/*get ipv4 */
	pIpv4Hdr = (IP_V4_HDR *) (packetInfo.pFirstBuffer + LENGTH_802_3);
	Ipv4Hdr.identifier = ntohs(pIpv4Hdr->identifier);
	Ipv4Hdr.tot_len = ntohs(pIpv4Hdr->tot_len);
	Ipv4Hdr.ihl = pIpv4Hdr->ihl;
	fragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);

	/* check if 1st fragment */
	if ((flagsFragOffset.field.flags_more_frag == 1) && (flagsFragOffset.field.frag_offset == 0))
	{
		/*check current queue is exist this id packet or not*/
		pIpAsmData = rtmp_IpAssembleDataSearch(pAd,queIdx,Ipv4Hdr.identifier);
		/*if not exist, create it*/
		if(!pIpAsmData)
		{
			rtmp_IpAssembleDataCreate(pAd,queIdx,&pIpAsmData,Ipv4Hdr.identifier,fragSize);
			if(!pIpAsmData)
			{
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}
		}

		/*store to  cache */
		pCurIpAsmData[queIdx] = pIpAsmData;
		/*insert packet*/
		pAcQueue = &pIpAsmData->queue;
		InsertTailQueue(pAcQueue, PACKET_TO_QUEUE_ENTRY(pPacket));

	} else
	{
		/*search assemble data from identify and cache first*/
		if(pCurIpAsmData[queIdx] && (pCurIpAsmData[queIdx]->identify == Ipv4Hdr.identifier))
		{
			pIpAsmData = pCurIpAsmData[queIdx];
		}else
		{
			pIpAsmData = rtmp_IpAssembleDataSearch(pAd,queIdx,Ipv4Hdr.identifier);

			/*not create assemble, should drop*/
			if(!pIpAsmData)
			{
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}
			/*update cache*/
			pCurIpAsmData[queIdx] = pIpAsmData;
		}
		pAcQueue = &pIpAsmData->queue;
		InsertTailQueue(pAcQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
		/* check if last fragment */
		if (pIpAsmData && (flagsFragOffset.field.flags_more_frag == 0) && (flagsFragOffset.field.frag_offset != 0))
		{
			/*fragment packets gatter and check*/
			fragCount = ((flagsFragOffset.field.frag_offset * 8) / (pIpAsmData->fragSize)) + 1;

			if (pAcQueue->Number != fragCount)
			{
				rtmp_IpAssembleDataDestory(pAd,pIpAsmData);
				pCurIpAsmData[queIdx] = NULL;
				return NDIS_STATUS_FAILURE;
			}

			/* move backup fragments to software queue */
			if (rtmp_enq_req(pAd, NULL, queIdx, pTrEntry,FALSE,pAcQueue) == FALSE)
			{
				rtmp_IpAssembleDataDestory(pAd,pIpAsmData);
				pCurIpAsmData[queIdx] = NULL;
				return NDIS_STATUS_FAILURE;
			}
			rtmp_IpAssembleDataDestory(pAd,pIpAsmData);
			pCurIpAsmData[queIdx] = NULL;
		}

	}
	return NDIS_STATUS_SUCCESS;
}
#endif


BOOLEAN tx_burst_rule_check(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
    if (pAd->CommonCfg.bEnableTxBurst == FALSE)
        return FALSE;

    if (pAd->MacTab.Size >= 1) 
        return TRUE;
    else
	return FALSE;

#ifdef APCLI_SUPPORT
    if ((wdev->wdev_type == WDEV_TYPE_APCLI) || (wdev->wdev_type == WDEV_TYPE_REPEATER)) 
    {
	UINT apcliNum = 0, ifIndex = 0;
	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
	    APCLI_STRUCT *apcli_entry;
	    PMAC_TABLE_ENTRY pEntry;
            STA_TR_ENTRY *tr_entry;
            UINT Wcid = 0;
            
	    apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
            Wcid = apcli_entry->MacTabWCID;	   	

	    if (!VALID_WCID(Wcid))
                continue;

	    pEntry = &pAd->MacTab.Content[Wcid];
            tr_entry = &pAd->MacTab.tr_entry[Wcid];

	    apcliNum++;
	}

	if (apcliNum == 1)
        {
	    if (pAd->MacTab.Size == 1)
	    {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Hit only one apcli turn on TxOP\n"));
	        return TRUE;
	    }
#ifdef MAC_REPEATER_SUPPORT
            else if ((pAd->ApCfg.RepeaterCliSize == 1) &&
                ((pAd->ApCfg.RepeaterCliSize + apcliNum) == pAd->MacTab.Size))
            {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Hit only one rept turn on TxOP\n"));
                return TRUE;
            }
            else {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                        ("%s, MacTab.Size=%x, RepeaterCliSize=%x, apcliNum=%x\n",
                         __FUNCTION__,
                         pAd->MacTab.Size, pAd->ApCfg.RepeaterCliSize, apcliNum));
            }
#endif /* MAC_REPEATER_SUPPORT */		
	}
    else {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                ("%s, apcliNum=%x\n", __FUNCTION__, apcliNum));
    }
    }
#endif /* APCLI_SUPPORT */	 
	
    return FALSE;
}


INT rtmp_tx_burst_set(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef DOT11_N_SUPPORT
#ifdef MT_MAC
    BOOLEAN enable = FALSE;

    if (wdev == NULL)
        return TRUE;

	if (pAd->chipCap.hif_type == HIF_MT)
	{
		/* Not RDG, update the TxOP else keep the default RDG's TxOP */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == FALSE)
		{
                    enable = tx_burst_rule_check(pAd, wdev);
                    RTMP_SET_TX_BURST(pAd, wdev, enable);
		}
	}
#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */

	return TRUE;
}

VOID enable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
                             UINT8 ac_type, UINT8 prio, UINT16 level)
{
	if (wdev == NULL)
		return ;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s, prio=%d, level=0x%x, <caller: %pS>\n",
				__FUNCTION__, prio, level,
				__builtin_return_address(0)));
		HW_SET_TX_BURST(pAd, wdev, ac_type, prio, level, 1);
	}
#endif /* MT_MAC */
}

VOID disable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
                              UINT8 ac_type, UINT8 prio, UINT16 level)
{
	if (wdev == NULL)
		return ;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s, prio=%d, level=0x%x, <caller: %pS>\n",
				__FUNCTION__, prio, level,
				__builtin_return_address(0)));
		HW_SET_TX_BURST(pAd, wdev, ac_type, prio, level, 0);
	}
#endif /* MT_MAC */
}

UINT8 query_tx_burst_prio(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT8 i, prio = PRIO_DEFAULT;
	UINT32 prio_bitmap = 0;

	if (wdev == NULL)
		return prio;
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		prio_bitmap = wdev->bss_info_argument.prio_bitmap;
		for (i=0; i<MAX_PRIO_NUM; i++) {
			if (prio_bitmap & (1 << i))
				prio = i;
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s, curr: prio=%d, txop=0x%x, <caller: %pS>\n",
				__FUNCTION__, prio, wdev->bss_info_argument.txop_level[prio],
				__builtin_return_address(0)));
	}
#endif /* MT_MAC */
	return prio;
}


INT TxOPUpdatingAlgo(RTMP_ADAPTER *pAd)
{
	UCHAR UpdateTxOP = 0xFF;
	UINT32 TxTotalByteCnt = pAd->TxTotalByteCnt;
	UINT32 RxTotalByteCnt = pAd->RxTotalByteCnt;

	if ((TxTotalByteCnt == 0) || (RxTotalByteCnt == 0))
	{
		/* Avoid to divide 0, when doing the traffic calculating */
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Not expected one of them is 0, TxTotalByteCnt = %lu, RxTotalByteCnt = %lu\n", (ULONG)TxTotalByteCnt, (ULONG)RxTotalByteCnt));
	}
	else if ((pAd->MacTab.Size == 1)
			&& (pAd->CommonCfg.ManualTxop == 0)
			&& pAd->CommonCfg.bEnableTxBurst
			&& (pAd->chipCap.TxOPScenario == 1))
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == TRUE)
		{
			UpdateTxOP = 0x80; // Tx/Rx/Bi
		}
		else if ((((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20) > pAd->CommonCfg.ManualTxopThreshold)
		{ /* TxopThreshold unit is Mbps */
			if (TxTotalByteCnt > RxTotalByteCnt)
			{
				if ((TxTotalByteCnt/RxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound)
				{ /* Boundary unit is Ratio */
					UpdateTxOP = 0x60; // Tx
				}
				else if ((TxTotalByteCnt/RxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound)
				{
					//UpdateTxOP = 0x0; // Bi
					UpdateTxOP = (pAd->MacTab.fCurrentStaBw40) ? 0x0 : 0x60;
				}
				else
			        {
					// No change TxOP
				}
			}
			else
			{
				if ((RxTotalByteCnt/TxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound)
				{ /* Boundary unit is Ratio */
					UpdateTxOP = 0x0; // Rx
				}
				else if ((RxTotalByteCnt/TxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound)
				{
					//UpdateTxOP = 0x0; // Bi
					UpdateTxOP = (pAd->MacTab.fCurrentStaBw40) ? 0x0 : 0x60;
				}
				else
			        {
					// No change TxOP
				}
			}
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Current TP=%lu < Threshold(%lu), turn-off TxOP\n",
						(ULONG)(((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20), (ULONG)pAd->CommonCfg.ManualTxopThreshold));
			UpdateTxOP = 0x0;
		}
	}
	else if (pAd->MacTab.Size > 1)
	{
		UpdateTxOP = 0x0;
	}

	if (UpdateTxOP != 0xFF &&
			UpdateTxOP != pAd->CurrEdcaParam[WMM_PARAM_AC_1].u2Txop)
	{
		AsicUpdateTxOP(pAd, WMM_PARAM_AC_1, UpdateTxOP);
	}

	pAd->TxTotalByteCnt = 0;
	pAd->RxTotalByteCnt = 0;

	return TRUE;
}


VOID ComposeNullFrame(
		RTMP_ADAPTER *pAd,
		PHEADER_802_11 pNullFrame,
		UCHAR *pAddr1,
		UCHAR *pAddr2,
		UCHAR *pAddr3)
{
	NdisZeroMemory(pNullFrame, sizeof (HEADER_802_11));
	pNullFrame->FC.Type = FC_TYPE_DATA;
	pNullFrame->FC.SubType = SUBTYPE_DATA_NULL;
	pNullFrame->FC.ToDs = 1;
	COPY_MAC_ADDR(pNullFrame->Addr1, pAddr1);
	COPY_MAC_ADDR(pNullFrame->Addr2, pAddr2);
	COPY_MAC_ADDR(pNullFrame->Addr3, pAddr3);
}

VOID mt_detect_wmm_traffic(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_PACKET	*pPacket,
	IN UCHAR UserPriority,
	IN UCHAR FlgIsOutput)
{
#ifdef MT7615
	UINT16 TypeLen = 0;
	UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
#endif /* MT7615 */

	if (!pAd)
		return;

#ifdef MT7615
	if (IS_MT7615(pAd))
	{
		pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
		ASSERT(pSrcBuf);

		/* get Ethernet protocol field and skip the Ethernet Header */
		TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

		pSrcBuf += LENGTH_802_3;
		if (TypeLen <= 1500)
		{
			/*
				802.3, 802.3 LLC:
					DestMAC(6) + SrcMAC(6) + Lenght(2) +
					DSAP(1) + SSAP(1) + Control(1) +
				if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
					=> + SNAP (5, OriginationID(3) + etherType(2))
				else
					=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
			*/
			if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
			{
				Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
				TypeLen = (USHORT)((Byte0 << 8) + Byte1);
				pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
			}
			else
			{
				return;
			}
		}

		if (TypeLen == ETH_TYPE_IPv4)
		{
			/*
				0		4		8		   14  15					   31(Bits)
				+---+----+-----+----+---------------+
				|Ver |	IHL |DSCP |ECN |	TotalLen		   |
				+---+----+-----+----+---------------+
				Ver    - 4bit Internet Protocol version number.
				IHL    - 4bit Internet Header Length
				DSCP - 6bit Differentiated Services Code Point(TOS)
				ECN   - 2bit Explicit Congestion Notification
				TotalLen - 16bit IP entire packet length(include IP hdr)
			*/
			up = (*(pSrcBuf + 1) & 0xe0) >> 5;
		}
		else if (TypeLen == ETH_TYPE_IPv6)
		{
			/*
				0		4		8		 12 	16						31(Bits)
				+---+----+----+----+---------------+
				|Ver | TrafficClas |  Flow Label				   |
				+---+----+----+--------------------+
				Ver 		  - 4bit Internet Protocol version number.
				TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
			*/
			up = ((*pSrcBuf) & 0x0e) >> 1;	

			if ((*pSrcBuf & 0xf0) != 0x60)
				up = 0;
		}

		/* count packets which priority is more than BE */
		if (up > 3)
			pAd->OneSecondnonBEpackets++;
	}
#endif /* MT7615 */
}

VOID mt_dynamic_wmm_be_tx_op(
	IN RTMP_ADAPTER *pAd,
	IN ULONG nonBEpackets)
{
#ifdef MT7615
	if (IS_MT7615(pAd))
	{
		struct wifi_dev *wdev;
#if defined (APCLI_SUPPORT) || defined (CONFIG_STA_SUPPORT)
		INT idx;
#endif /* APCLI_SUPPORT */

		if (pAd->OneSecondnonBEpackets > nonBEpackets)
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE))
			{
#ifdef APCLI_SUPPORT
				for (idx = 0; idx < MAX_APCLI_NUM; idx++) 
				{
					wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
	
					if ((wdev) && (pAd->ApCfg.ApCliTab[idx].Valid == TRUE))
					{
						enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
					}
				}
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

				if (wdev)
				{
					enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
				}
#endif /* CONFIG_AP_SUPPORT */

#ifdef PKT_BUDGET_CTRL_SUPPORT
				HW_SET_PBC_CTRL(pAd,NULL,NULL,PBC_TYPE_WMM);
				//MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: OneSecondnonBEpackets: %lu\n", __FUNCTION__, pAd->OneSecondnonBEpackets));
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
			}
		}
		else
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) == 0)
			{
#ifdef APCLI_SUPPORT
				for (idx = 0; idx < MAX_APCLI_NUM; idx++) 
				{
					wdev = &pAd->ApCfg.ApCliTab[idx].wdev;

					if ((wdev) && (pAd->ApCfg.ApCliTab[idx].Valid == TRUE))
					{
						disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
					}
				}
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

				if (wdev)
				{
					disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
				}
#endif /* CONFIG_AP_SUPPORT */

#ifdef PKT_BUDGET_CTRL_SUPPORT
				HW_SET_PBC_CTRL(pAd,NULL,NULL,PBC_TYPE_NORMAL);
				//MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: OneSecondnonBEpackets: %lu\n", __FUNCTION__, pAd->OneSecondnonBEpackets));
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
			}
		}

		pAd->OneSecondnonBEpackets = 0;
	}
#endif /* MT7615 */	
}

UINT32 Get_OBSS_AirTime(
        IN	PRTMP_ADAPTER	pAd,
        IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.OBSSAirtime[BandIdx];
}


UINT32 Get_My_Tx_AirTime(
        IN	PRTMP_ADAPTER	pAd,
        IN      UCHAR                 BandIdx)
{
	return pAd->OneSecMibBucket.MyTxAirtime[BandIdx];
}

UINT32 Get_My_Rx_AirTime(
        IN	PRTMP_ADAPTER	pAd,
        IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.MyRxAirtime[BandIdx];	
}

UINT32 Get_EDCCA_Time(
        IN	PRTMP_ADAPTER	pAd,
        IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.EDCCAtime[BandIdx];
}

VOID CCI_ACI_scenario_maintain(
        IN	PRTMP_ADAPTER	pAd)
{

        UCHAR   i = 0;
        UCHAR   j = 0;
        UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
        UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
        UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
        UINT32  EDCCATime[DBDC_BAND_NUM] = {0};
        UCHAR   ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
        UCHAR   MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
        UCHAR   EdccaOccupyPercentage[DBDC_BAND_NUM] = {0};
        UCHAR   MyTxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
        UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
        struct wifi_dev *pWdev=NULL;
        UCHAR BandIdx;
        
		if (concurrent_bands > DBDC_BAND_NUM) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("It should not happen!!!!!"));
			concurrent_bands = DBDC_BAND_NUM;
		}	
		
        for(i=0;i<concurrent_bands;i++)
        {        
                ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
                MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
                MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);
                EDCCATime[i] = Get_EDCCA_Time(pAd, i);
                
                if (ObssAirTime[i] != 0)
                        ObssAirOccupyPercentage[i] = (ObssAirTime[i]*100)/ONE_SEC_2_US; 

                if (MyTxAirTime[i] != 0 ||MyRxAirTime [i] != 0 )
                        MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) *100)/ONE_SEC_2_US; 
                
                if (EDCCATime[i] != 0)
                        EdccaOccupyPercentage[i] = (EDCCATime[i] * 100) / ONE_SEC_2_US;

                        if (MyTxAirTime[i]+MyRxAirTime [i] != 0) 
                                MyTxAirOccupyPercentage[i] = (MyTxAirTime[i] * 100) / (MyTxAirTime[i] + MyRxAirTime[i]);

                //MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                //        ("Band%d OBSSAirtime=%d, MyAirtime=%d, EdccaTime=%d, MyTxAirtime=%d, MyRxAirtime=%d\n",
                //        i, ObssAirOccupyPercentage[i], MyAirOccupyPercentage[i], EdccaOccupyPercentage[i], MyTxAirTime[i], MyRxAirTime[i]));

                if ((ObssAirOccupyPercentage[i] > OBSS_OCCUPY_PERCENT_HIGH_TH) 
                        && (ObssAirOccupyPercentage[i] + MyAirOccupyPercentage[i] > ALL_AIR_OCCUPY_PERCENT )
                        && MyTxAirOccupyPercentage[i] > TX_RATIO_TH
                        && MyAirOccupyPercentage[i] >My_OCCUPY_PERCENT ) {
                        if (pAd->CCI_ACI_TxOP_Value[i] == 0) {
                                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                                        ("CCI detected !!!!! Apply TxOP=FE \n"));

                                for (j=0;j<WDEV_NUM_MAX;j++) {
                                        pWdev = pAd->wdev_list[j];
                                        if (!pWdev) {
                                                continue;
                                        }
                                        BandIdx = HcGetBandByWdev(pWdev);
                                        if (BandIdx == i) {
                                                pAd->CCI_ACI_TxOP_Value[i] = TXOP_FE;
                                                enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_FE);
                                                break;
                                        }
                                }
                        }                     
                } else if ((EdccaOccupyPercentage[i] > OBSS_OCCUPY_PERCENT_HIGH_TH) 
                        && (EdccaOccupyPercentage[i] + MyAirOccupyPercentage[i] > ALL_AIR_OCCUPY_PERCENT )
                        && MyTxAirOccupyPercentage[i] > TX_RATIO_TH
                        && MyAirOccupyPercentage[i] >My_OCCUPY_PERCENT ) {
                        if (pAd->CCI_ACI_TxOP_Value[i] == 0) {
                                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                                        ("ACI detected !!!!! Apply TxOP=FE \n"));

                                for (j=0;j<WDEV_NUM_MAX;j++) {
                                        pWdev = pAd->wdev_list[j];
                                        if (!pWdev) {
                                                continue;
                                        }
                                        BandIdx = HcGetBandByWdev(pWdev);
                                        if (BandIdx == i) {
                                                pAd->CCI_ACI_TxOP_Value[i] = TXOP_FE;
                                                enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_FE);
                                                break;
                                        }
                                }
                        }                        
                } else {   
                        if (pAd->CCI_ACI_TxOP_Value[i] != 0) {
                                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                                        ("NO CCI /ACI detected !!!!! Apply TxOP=0 \n"));
                                for (j=0;j<WDEV_NUM_MAX;j++) {
                                        pWdev = pAd->wdev_list[j];
                                        if (!pWdev) {
                                                continue;
                                        }
                                        BandIdx = HcGetBandByWdev(pWdev);
                                        if (BandIdx == i) {
                                                pAd->CCI_ACI_TxOP_Value[i] = TXOP_0;
                                                disable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_0);
                                                break;
                                        }
                                }
                        }
                }        
        
                //Reset_OBSS_AirTime(pAd,i);
        }
        

}

#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
VOID Mumimo_scenario_maintain(
        IN	PRTMP_ADAPTER	pAd)
{
#ifdef CFG_SUPPORT_MU_MIMO
	UCHAR               ucWlanIdx = 0, ucWdevice = 0;
    UCHAR               ucMUNum = 0;
    struct wifi_dev     *pWdev = NULL;
    PMAC_TABLE_ENTRY    pEntry;
    BOOL                fgHitMUTxOPCondition = FALSE ;

    // Find out the MU CAP in the STA Rec List
    for (ucWlanIdx = 1; ucWlanIdx <= pAd->MacTab.Size; ucWlanIdx++) {

        pEntry = &pAd->MacTab.Content[ucWlanIdx];

        if(pEntry->rStaRecBf.fgSU_MU) {
            ucMUNum++;
        }

        if(ucMUNum >= 2) {
            fgHitMUTxOPCondition = TRUE;
            break;
        }
    }

    // If Hit Trigger Condition, assign TxOP=0xC0 for 3/4 MU-MIMO Peak
    if(fgHitMUTxOPCondition) {
        if(pAd->MUMIMO_TxOP_Value != TXOP_C0) {
            for (ucWdevice = 0; ucWdevice < WDEV_NUM_MAX; ucWdevice++) {

                pWdev = pAd->wdev_list[ucWdevice];
                if (!pWdev) {
                    continue;
                }

                pAd->MUMIMO_TxOP_Value = TXOP_C0;
                enable_tx_burst(pAd, pWdev, AC_BE, PRIO_MU_MIMO, TXOP_C0);
            }
        }
    }
    else {
        if(pAd->MUMIMO_TxOP_Value != TXOP_0) {
            for (ucWdevice = 0; ucWdevice < WDEV_NUM_MAX; ucWdevice++) {

                pWdev = pAd->wdev_list[ucWdevice];
                if (!pWdev) {
                    continue;
                }

                pAd->MUMIMO_TxOP_Value = TXOP_0;
                disable_tx_burst(pAd, pWdev, AC_BE, PRIO_MU_MIMO, TXOP_0);
            }
        }
    }

    //MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
    //    ("MU TxOP = %d, MU STA = %d, MU CW Min %d\n", pAd->MUMIMO_TxOP_Value, ucMUNum, u4EdcaCWmin));

    return;
#else
    return;
#endif
}
#endif /* MT_MAC && VHT_TXBF_SUPPORT */

/** @} */
/** @} */
