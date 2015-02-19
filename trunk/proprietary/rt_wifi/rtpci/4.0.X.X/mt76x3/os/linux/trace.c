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
	trace.c
*/

#include "rt_config.h"

#ifdef CONFIG_TRACE_SUPPORT
#define CREATE_TRACE_POINTS
#include "os/trace.h"
#endif

static VOID TraceCrPseFrameInfo(RTMP_ADAPTER *pAd, UINT8 PID, UINT8 QID)
{
	UINT32 FirstFID, CurFID, NextFID, FrameNums = 0;
	UINT32 Value;
	UCHAR PseData[32];
	UINT32 DwIndex;

	RTMP_IO_READ32(pAd, C_GFF, &Value);
	Value &= ~GET_QID_MASK;
	Value |= SET_GET_QID(QID);
	Value &= ~GET_PID_MASK;
	Value |= SET_GET_PID(PID);
	RTMP_IO_WRITE32(pAd, C_GFF, Value);

	RTMP_IO_READ32(pAd, C_GFF, &Value);
	FirstFID = GET_FIRST_FID(Value);


	if (FirstFID == 0xfff)
	{
		return;
	}

	CurFID = FirstFID;
	FrameNums++;

	for (DwIndex = 0; DwIndex < 8; DwIndex++)
	{
		RTMP_IO_READ32(pAd, (MT_PCI_REMAP_ADDR_1 + (((CurFID) * 128)) + (DwIndex * 4)),
			&PseData[DwIndex * 4]);
	}

	TRACE_CR_PSE_FRAME_INFO(PID, QID, CurFID);
	TRACE_TX_MAC_LINFO((TMAC_TXD_L *)PseData);

	while (1)
	{
		RTMP_IO_READ32(pAd, C_GF, &Value);
		Value &= ~CURR_FID_MASK;
		Value |= SET_CURR_FID(CurFID);
		RTMP_IO_WRITE32(pAd, C_GF, Value);
		RTMP_IO_READ32(pAd, C_GF, &Value);
		NextFID = GET_RETURN_FID(Value);

		if (NextFID == 0xfff)
		{
			return;
		}
		else
		{
			CurFID = NextFID;
			
			for (DwIndex = 0; DwIndex < 8; DwIndex++)
			{
				RTMP_IO_READ32(pAd, (MT_PCI_REMAP_ADDR_1 + (((CurFID) * 128)) + (DwIndex * 4)),
					&PseData[DwIndex * 4]);
			}

			TRACE_CR_PSE_FRAME_INFO(PID, QID, CurFID);
			TRACE_TX_MAC_LINFO((TMAC_TXD_L *)PseData);
			
			FrameNums++;
		}

	}
}


VOID TraceCrPseInfo(RTMP_ADAPTER *pAd)
{
	TRACE_CR_PSE_INFO(pAd);
	
	/* HIF port frame information */
	TraceCrPseFrameInfo(pAd, 0, 0);
	TraceCrPseFrameInfo(pAd, 0, 1);
	TraceCrPseFrameInfo(pAd, 0, 2);

	/* MCU port frame information */
	TraceCrPseFrameInfo(pAd, 1, 0);
	TraceCrPseFrameInfo(pAd, 1, 1);
	TraceCrPseFrameInfo(pAd, 1, 2);
	TraceCrPseFrameInfo(pAd, 1, 3);
	
	/* WLAN port frame information */
	TraceCrPseFrameInfo(pAd, 2, 0);
	TraceCrPseFrameInfo(pAd, 2, 1);
	TraceCrPseFrameInfo(pAd, 2, 2);
	TraceCrPseFrameInfo(pAd, 2, 3);
	TraceCrPseFrameInfo(pAd, 2, 4);
	TraceCrPseFrameInfo(pAd, 2, 5);
	TraceCrPseFrameInfo(pAd, 2, 6);
	TraceCrPseFrameInfo(pAd, 2, 7);
	TraceCrPseFrameInfo(pAd, 2, 8);
	TraceCrPseFrameInfo(pAd, 2, 9);
	TraceCrPseFrameInfo(pAd, 2, 10);
	TraceCrPseFrameInfo(pAd, 2, 11);
	TraceCrPseFrameInfo(pAd, 2, 12);
	TraceCrPseFrameInfo(pAd, 2, 13);
}


VOID TraceWtblInfo(RTMP_ADAPTER *pAd, UINT32 wtbl_idx)
{
	INT idx, start_idx, end_idx, tok;
	UINT32 addr, val[16];
	struct wtbl_1_struc wtbl_1;
	struct wtbl_2_struc wtbl_2;
	union wtbl_3_struc wtbl_3;
	struct wtbl_4_struc wtbl_4;

	if (wtbl_idx == RESERVED_WCID) {
		start_idx = 0;
		end_idx = (MT_WTBL_SIZE - 1);
	} else if (wtbl_idx < MT_WTBL_SIZE) {
		start_idx = end_idx = wtbl_idx;
	} else {
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++)
	{
		/* Read WTBL 1 */
		NdisZeroMemory((UCHAR *)&wtbl_1, sizeof(struct wtbl_1_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];
		RTMP_IO_READ32(pAd, addr, &wtbl_1.wtbl_1_d0.word);
		RTMP_IO_READ32(pAd, addr + 4, &wtbl_1.wtbl_1_d1.word);
		RTMP_IO_READ32(pAd, addr + 8, &wtbl_1.wtbl_1_d2.word);
		RTMP_IO_READ32(pAd, addr + 12, &wtbl_1.wtbl_1_d3.word);
		RTMP_IO_READ32(pAd, addr + 16, &wtbl_1.wtbl_1_d4.word);
		TRACE_CR_WTBL1_INFO(pAd, &wtbl_1);

		/* Read WTBL 2 */
		NdisZeroMemory((UCHAR *)&wtbl_2, sizeof(struct wtbl_2_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[1] + idx * pAd->mac_ctrl.wtbl_entry_size[1];
		for (tok = 0; tok < sizeof(struct wtbl_2_struc) / 4; tok++) {
			RTMP_IO_READ32(pAd, addr + tok * 4, &val[tok]);
		}
		TRACE_CR_WTBL2_INFO(pAd, (struct wtbl_2_struc *)&val[0]);

		/* Read WTBL 3 */
		NdisZeroMemory((UCHAR *)&wtbl_3, sizeof(union wtbl_3_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[2] + idx * pAd->mac_ctrl.wtbl_entry_size[2];
		for (tok = 0; tok < sizeof(union wtbl_3_struc) / 4; tok++) {
			RTMP_IO_READ32(pAd, addr + tok * 4, &val[tok]);
		}
		//dump_wtbl_3_info(pAd, (union wtbl_3_struc *)&val[0]);

		/* Read WTBL 4 */
		NdisZeroMemory((UCHAR *)&wtbl_4, sizeof(struct wtbl_4_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[3] + idx * pAd->mac_ctrl.wtbl_entry_size[3];
		RTMP_IO_READ32(pAd, addr, &wtbl_4.ac0.word[0]);
		RTMP_IO_READ32(pAd, addr+4, &wtbl_4.ac0.word[1]);
		RTMP_IO_READ32(pAd, addr + 8, &wtbl_4.ac1.word[0]);
		RTMP_IO_READ32(pAd, addr + 12, &wtbl_4.ac1.word[1]);
		RTMP_IO_READ32(pAd, addr + 16, &wtbl_4.ac2.word[0]);
		RTMP_IO_READ32(pAd, addr + 20, &wtbl_4.ac2.word[1]);
		RTMP_IO_READ32(pAd, addr + 24, &wtbl_4.ac3.word[0]);
		RTMP_IO_READ32(pAd, addr + 28, &wtbl_4.ac3.word[1]);
		//dump_wtbl_4_info(pAd, &wtbl_4);
	}
}


INT32 TracePSTable(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
   INT i,j;
   UINT32 RegValue;
   ULONG DataRate=0;
   struct wtbl_entry tb_entry;
   union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
   UINT32  rPseRdTabAccessReg;  
   BOOLEAN pfgForce;
   UCHAR pucPort, pucQueue;
   INT Total_Packet_Number = 0 ;

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
           PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
           STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
           Total_Packet_Number = 0 ;

		if ((ent_type == ENTRY_NONE))
		{
			/* dump all MacTable entries */
			if (pEntry->EntryType == ENTRY_NONE) 
				continue;
		} 
		else 
		{
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry)) 
				&& (pEntry->Sst != SST_ASSOC))
				continue;
		}

		NdisZeroMemory(&tb_entry, sizeof(tb_entry));      
		if (mt_wtbl_get_entry234(pAd, pEntry->wcid, &tb_entry) == FALSE) 
		{
			 return FALSE;
		}
		RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0]+12, &dw3->word);

		//get PSE register

		//      rPseRdTabAccessReg.field.rd_kick_busy=1;
		//      rPseRdTabAccessReg.field.rd_tag=pEntry->wcid;
		rPseRdTabAccessReg = PSE_RTA_RD_KICK_BUSY |PSE_RTA_TAG(pEntry->wcid); 
		RTMP_IO_WRITE32(pAd, PSE_RTA,rPseRdTabAccessReg);

		do
		{          
			RTMP_IO_READ32(pAd,PSE_RTA,&rPseRdTabAccessReg);

			pfgForce = ( BOOLEAN ) GET_PSE_RTA_RD_RULE_F(rPseRdTabAccessReg);
			pucPort  = ( UCHAR )  GET_PSE_RTA_RD_RULE_PID(rPseRdTabAccessReg);
			pucQueue = ( UCHAR )  GET_PSE_RTA_RD_RULE_QID(rPseRdTabAccessReg);
		}      
		while ( GET_PSE_RTA_RD_KICK_BUSY(rPseRdTabAccessReg) == 1 );

		Total_Packet_Number = Total_Packet_Number + tr_entry->ps_queue.Number;
		for (j = 0; j < WMM_QUE_NUM; j++)
			Total_Packet_Number = Total_Packet_Number + tr_entry->tx_queue[j].Number;

		TRACE_PS_INFO(pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3], 
					  pEntry->Addr[4], pEntry->Addr[5],
					  pEntry->EntryType, pEntry->Aid, pEntry->func_tb_idx,
					  pEntry->PsMode, tr_entry->ps_state, dw3->field.i_psm,
					  dw3->field.du_i_psm, dw3->field.skip_tx, pfgForce,
					  pucPort, pucQueue, Total_Packet_Number, tr_entry->ps_queue.Number);
	}

	return TRUE;
}


INT32 TraceTRInfo(RTMP_ADAPTER *pAd)
{
	UINT32 Value;
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd))
	{
		ULONG irq_flags;
		UINT32 tbase[NUM_OF_TX_RING], tcnt[NUM_OF_TX_RING];
		UINT32 tcidx[NUM_OF_TX_RING], tdidx[NUM_OF_TX_RING];
		UINT32 rbase[NUM_OF_RX_RING], rcnt[NUM_OF_RX_RING];
		UINT32 rcidx[NUM_OF_RX_RING], rdidx[NUM_OF_RX_RING];
		UINT32 mbase[4] = {0}, mcnt[4] = {0}, mcidx[4] = {0}, mdidx[4] = {0};
		UINT32 sys_ctrl[4];
		UINT32 cr_int_src, cr_int_mask, cr_delay_int, cr_wpdma_glo_cfg;
		INT idx;
		
		RTMP_IRQ_LOCK(&pAd->irq_lock, irq_flags);
		for (idx = 0; idx < NUM_OF_TX_RING; idx++)
		{
			RTMP_IO_READ32(pAd, pAd->TxRing[idx].hw_desc_base, &tbase[idx]);
			RTMP_IO_READ32(pAd, pAd->TxRing[idx].hw_cnt_addr, &tcnt[idx]);
			RTMP_IO_READ32(pAd, pAd->TxRing[idx].hw_cidx_addr, &tcidx[idx]);
			RTMP_IO_READ32(pAd, pAd->TxRing[idx].hw_didx_addr, &tdidx[idx]);
		}

		for (idx = 0; idx < NUM_OF_RX_RING; idx++)
		{
			RTMP_IO_READ32(pAd, pAd->RxRing[idx].hw_desc_base, &rbase[idx]);
			RTMP_IO_READ32(pAd, pAd->RxRing[idx].hw_cnt_addr, &rcnt[idx]);
			RTMP_IO_READ32(pAd, pAd->RxRing[idx].hw_cidx_addr, &rcidx[idx]);
			RTMP_IO_READ32(pAd, pAd->RxRing[idx].hw_didx_addr, &rdidx[idx]);
		}

		RTMP_IO_READ32(pAd, pAd->MgmtRing.hw_desc_base, &mbase[0]);
		RTMP_IO_READ32(pAd, pAd->MgmtRing.hw_cnt_addr, &mcnt[0]);
		RTMP_IO_READ32(pAd, pAd->MgmtRing.hw_cidx_addr, &mcidx[0]);
		RTMP_IO_READ32(pAd, pAd->MgmtRing.hw_didx_addr, &mdidx[0]);

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
#ifdef CONFIG_ANDES_SUPPORT
			RTMP_IO_READ32(pAd, pAd->CtrlRing.hw_desc_base, &mbase[1]);
			RTMP_IO_READ32(pAd, pAd->CtrlRing.hw_cnt_addr, &mcnt[1]);
			RTMP_IO_READ32(pAd, pAd->CtrlRing.hw_cidx_addr, &mcidx[1]);
			RTMP_IO_READ32(pAd, pAd->CtrlRing.hw_didx_addr, &mdidx[1]);
#endif /* CONFIG_ANDES_SUPPORT */

			RTMP_IO_READ32(pAd, pAd->BcnRing.hw_desc_base, &mbase[2]);
			RTMP_IO_READ32(pAd, pAd->BcnRing.hw_cnt_addr, &mcnt[2]);
			RTMP_IO_READ32(pAd, pAd->BcnRing.hw_cidx_addr, &mcidx[2]);
			RTMP_IO_READ32(pAd, pAd->BcnRing.hw_didx_addr, &mdidx[2]);
			
			RTMP_IO_READ32(pAd, pAd->TxBmcRing.hw_desc_base, &mbase[3]);
			RTMP_IO_READ32(pAd, pAd->TxBmcRing.hw_cnt_addr, &mcnt[3]);
			RTMP_IO_READ32(pAd, pAd->TxBmcRing.hw_cidx_addr, &mcidx[3]);
			RTMP_IO_READ32(pAd, pAd->TxBmcRing.hw_didx_addr, &mdidx[3]);
		}
#endif /* MT_MAC */

		cr_int_src = cr_int_mask = cr_wpdma_glo_cfg = 0;
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			cr_int_src = MT_INT_SOURCE_CSR;
			cr_int_mask = MT_INT_MASK_CSR;
			cr_delay_int = MT_DELAY_INT_CFG;
			cr_wpdma_glo_cfg = MT_WPDMA_GLO_CFG;
		}
#endif /* MT_MAC */

		RTMP_IO_READ32(pAd, cr_int_src, &sys_ctrl[0]);
		RTMP_IO_READ32(pAd, cr_int_mask, &sys_ctrl[1]);
		RTMP_IO_READ32(pAd, cr_delay_int, &sys_ctrl[2]);
		RTMP_IO_READ32(pAd, cr_wpdma_glo_cfg, &sys_ctrl[3]);
		
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, irq_flags);
		
		for (idx = 0; idx < NUM_OF_TX_RING; idx++) {
			TRACE_CR_TR_INFO("tx ring", pAd->TxRing[idx].hw_desc_base, tbase[idx],  tcnt[idx],
							 tcidx[idx], tdidx[idx], pAd->TxRing[idx].TxSwFreeIdx);
		}

		for (idx = 0; idx < NUM_OF_RX_RING; idx++) {
			TRACE_CR_TR_INFO("rx ring", pAd->RxRing[idx].hw_desc_base, rbase[idx], rcnt[idx], 
								rcidx[idx], rdidx[idx], pAd->RxRing[idx].RxSwReadIdx);
		}

		TRACE_CR_TR_INFO("mgmt ring", pAd->MgmtRing.hw_desc_base, mbase[0], mcnt[0], mcidx[0], 
			mdidx[0], pAd->MgmtRing.TxSwFreeIdx);

#ifdef CONFIG_ANDES_SUPPORT

		TRACE_CR_TR_INFO("ctrl ring", pAd->CtrlRing.hw_desc_base, mbase[1], mcnt[1], mcidx[1], 
			mdidx[1], pAd->CtrlRing.TxSwFreeIdx);

#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			TRACE_CR_TR_INFO("bcn ring", pAd->BcnRing.hw_desc_base, mbase[2], mcnt[2], mcidx[2], 
				mdidx[2], pAd->BcnRing.TxSwFreeIdx);
		}

		TRACE_CR_TR_INFO("bmc ring", pAd->TxBmcRing.hw_desc_base, mbase[3], mcnt[3], mcidx[3], 
			mdidx[3], pAd->TxBmcRing.TxSwFreeIdx);
#endif /* MT_MAC */

		TRACE_CR_INTERRUPT_INFO(sys_ctrl[0], sys_ctrl[1], sys_ctrl[2], sys_ctrl[3], sys_ctrl[3] & 0x1,
								sys_ctrl[3] & 0x4, sys_ctrl[3] & 0x2, sys_ctrl[3] & 0x8);

	
	}
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */


	return TRUE;
}


INT32 TraceMIBInfo(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val, idx, ampdu_cnt[5];
	UINT32 msdr6, msdr7, msdr8, msdr9, msdr10, msdr16, msdr17, msdr18;
	UINT32 mbxsdr[4][3];
	UINT32 mbtcr[16], mbtbcr[16], mbrcr[16], mbrbcr[16];
	UINT32 btcr[4], btbcr[4], brcr[4], brbcr[4];
	
	RTMP_IO_READ32(pAd, MIB_MSCR, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("MIB Status Control=0x%x\n", mac_val));
	RTMP_IO_READ32(pAd, MIB_MPBSCR, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("MIB Per-BSS Status Control=0x%x\n", mac_val));

	RTMP_IO_READ32(pAd, MIB_MSDR6, &msdr6);
	RTMP_IO_READ32(pAd, MIB_MSDR7, &msdr7);
	RTMP_IO_READ32(pAd, MIB_MSDR8, &msdr8);
	RTMP_IO_READ32(pAd, MIB_MSDR9, &msdr9);
	RTMP_IO_READ32(pAd, MIB_MSDR10, &msdr10);
	RTMP_IO_READ32(pAd, MIB_MSDR16, &msdr16);
	RTMP_IO_READ32(pAd, MIB_MSDR17, &msdr17);
	RTMP_IO_READ32(pAd, MIB_MSDR18, &msdr18);
	DBGPRINT(RT_DEBUG_OFF, ("===Phy/Timing Related Counters===\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelIdleCnt=0x%x\n", msdr6 & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tCCA_NAV_Tx_Time=0x%x\n", msdr9 & 0xffffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tRx_MDRDY_CNT=0x%x\n", msdr10 & 0x3ffffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tPrim CCA Time=0x%x\n", msdr16 & 0xffffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tSec CCA Time=0x%x\n", msdr17 & 0xffffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tPrim ED Time=0x%x\n", msdr18 & 0xffffff));
	
	DBGPRINT(RT_DEBUG_OFF, ("===Tx Related Counters(Generic)===\n"));
	RTMP_IO_READ32(pAd, MIB_MSDR0, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tBeaconTxCnt=0x%x\n", mac_val));
	RTMP_IO_READ32(pAd, MIB_MDR0, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tTx 40MHz Cnt=0x%x\n", (mac_val >> 16) & 0xffff));
	RTMP_IO_READ32(pAd, MIB_MDR1, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tTx 80MHz Cnt=0x%x\n", mac_val& 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tTx 160MHz Cnt=0x%x\n", (mac_val >> 16) & 0xffff));
	
	DBGPRINT(RT_DEBUG_OFF, ("===AMPDU Related Counters===\n"));
	RTMP_IO_READ32(pAd, MIB_MSDR12, &ampdu_cnt[0]);
	RTMP_IO_READ32(pAd, MIB_MSDR14, &ampdu_cnt[1]);
	RTMP_IO_READ32(pAd, MIB_MSDR15, &ampdu_cnt[2]);
	RTMP_IO_READ32(pAd, MIB_MDR2, &ampdu_cnt[3]);
	RTMP_IO_READ32(pAd, MIB_MDR3, &ampdu_cnt[4]);
	DBGPRINT(RT_DEBUG_OFF, ("\tRx BA_Cnt=0x%x\n", ampdu_cnt[0] & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tTx AMPDU_Burst_Cnt=0x%x\n", (ampdu_cnt[0] >> 16 ) & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tTx AMPDU_Pkt_Cnt=0x%x\n", ampdu_cnt[1] & 0xffffff));	
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU SuccessCnt=0x%x\n", ampdu_cnt[2] & 0xffffff));	
	DBGPRINT(RT_DEBUG_OFF, ("\tTx Agg Range: \t1 \t2~5 \t6~15 \t16~\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \n", 
					(ampdu_cnt[3]) & 0xffff, (ampdu_cnt[3] >> 16) & 0xffff,
					(ampdu_cnt[4]) & 0xffff, (ampdu_cnt[4] >> 16) & 0xfff));

	DBGPRINT(RT_DEBUG_OFF, ("===Rx Related Counters(Generic)===\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tVector Overflow Drop Cnt=0x%x\n", (msdr6 >> 16 ) & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tVector Missmacth Cnt=0x%x\n", (msdr7 >> 16 ) & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tDelimiter Fail Cnt=0x%x\n", msdr8& 0xffff));
	RTMP_IO_READ32(pAd, MIB_MSDR4, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tRxFifoFullCnt=0x%x\n", mac_val & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxFCSErrCnt=0x%x\n", (mac_val >> 16 ) & 0xffff));
	RTMP_IO_READ32(pAd, MIB_MSDR5, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMPDUCnt=0x%x\n", mac_val & 0xffff));
	DBGPRINT(RT_DEBUG_OFF, ("\tPFDropCnt=0x%x\n", (mac_val >> 16 ) & 0x00ff));
	RTMP_IO_READ32(pAd, MIB_MSDR22, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tRx AMPDU Cnt=0x%x\n", mac_val & 0xffff));
	RTMP_IO_READ32(pAd, MIB_MSDR23, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("\tRx Total ByteCnt=0x%x\n", mac_val));

	for (idx = 0; idx < 4; idx++) {
		RTMP_IO_READ32(pAd, WTBL_BTCRn + idx * 4, &btcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_BTBCRn + idx * 4, &btbcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_BRCRn + idx * 4, &brcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_BRBCRn + idx * 4, &brbcr[idx]);
	}
	DBGPRINT(RT_DEBUG_OFF, ("===Per-BSS Related Tx/Rx Counters===\n"));
	DBGPRINT(RT_DEBUG_OFF, ("BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n"));
	for (idx = 0; idx < 4; idx++) {
		DBGPRINT(RT_DEBUG_OFF, ("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
					idx, (btcr[idx] >> 16) & 0xffff, btcr[idx] & 0xffff, btbcr[idx],
					(brcr[idx] >> 16) & 0xffff, brcr[idx] & 0xffff, brbcr[idx]));
	}


	for (idx = 0; idx < 4; idx++)
	{
		RTMP_IO_READ32(pAd, MIB_MB0SDR0 + idx * 0x10, &mbxsdr[idx][0]);
		RTMP_IO_READ32(pAd, MIB_MB0SDR1 + idx * 0x10, &mbxsdr[idx][1]);
		RTMP_IO_READ32(pAd, MIB_MB0SDR2 + idx * 0x10, &mbxsdr[idx][2]);
	}
	DBGPRINT(RT_DEBUG_OFF, ("===Per-MBSS Related MIB Counters===\n"));
	DBGPRINT(RT_DEBUG_OFF, ("BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2Cnt\n"));
	for (idx = 0; idx < 4; idx++) {
		DBGPRINT(RT_DEBUG_OFF, ("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x\n",
					idx, mbxsdr[idx][0], (mbxsdr[idx][0] >> 16) & 0xffff,
					mbxsdr[idx][1], (mbxsdr[idx][1] >> 16) & 0xffff,
					mbxsdr[idx][2], (mbxsdr[idx][2] >> 16) & 0xffff));
	}


	for (idx = 0; idx < 8; idx++) {
		RTMP_IO_READ32(pAd, WTBL_MBTCRn + idx * 4, &mbtcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_MBRCRn + idx * 4, &mbrcr[idx]);
	}
	
	for (idx = 0; idx < 16; idx++) {
		RTMP_IO_READ32(pAd, WTBL_MBTBCRn + idx * 4, &mbtbcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_MBRBCRn + idx * 4, &mbrbcr[idx]);
	}	
	DBGPRINT(RT_DEBUG_OFF, ("===Per-MBSS Related Tx/Rx Counters===\n"));
	DBGPRINT(RT_DEBUG_OFF, ("MBSSIdx   TxDataCnt  TxByteCnt  RxDataCnt  RxByteCnt\n"));
	for (idx = 0; idx < 16; idx++) {
		DBGPRINT(RT_DEBUG_OFF, ("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
					idx, 
					((idx % 2 == 1) ? (mbtcr[idx/2] >> 16) & 0xffff : mbtcr[idx/2] & 0xffff), 
					mbtbcr[idx],
					((idx % 2 == 1)  ? (mbrcr[idx/2] >> 16) & 0xffff : mbrcr[idx/2] & 0xffff), 
					mbrbcr[idx]));
	}
	
	return TRUE;
}


