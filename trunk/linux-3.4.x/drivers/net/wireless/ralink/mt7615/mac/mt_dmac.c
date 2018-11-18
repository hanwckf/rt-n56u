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


const UCHAR wmm_aci_2_hw_ac_queue[] =
{
		TxQ_IDX_AC1, /* 0: QID_AC_BE, WMM0 */
		TxQ_IDX_AC0, /* 1: QID_AC_BK */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3:QID_AC_VO */
		TxQ_IDX_AC11, /*4:QID_AC_BE, WMM1 */
		TxQ_IDX_AC10,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
		TxQ_IDX_AC21,/*8:QID_AC_BE, WMM2 */
		TxQ_IDX_AC20,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
		TxQ_IDX_AC31,/*12:QID_AC_BE, WMM3 */
		TxQ_IDX_AC30,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
		TxQ_IDX_ALTX0, /*16*/
		TxQ_IDX_BMC0,
		TxQ_IDX_BCN0,
		TxQ_IDX_PSMP0,
		TxQ_IDX_ALTX1, /*20*/
		TxQ_IDX_BMC1,
		TxQ_IDX_BCN1,
		TxQ_IDX_PSMP1,
};


VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxInfo Fields:\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA=%d\n", pRxInfo->BA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}


static char *hdr_fmt_str[]={
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};


static char *p_idx_str[]={"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"WMM0_AC0", "WMM0_AC1", "WMM0_AC2", "WMM0_AC3",
								"WMM1_AC0", "WMM1_AC1", "WMM1_AC2", "WMM1_AC3",
								"WMM2_AC0", "WMM2_AC1", "WMM2_AC2", "WMM2_AC3",
								"WMM3_AC0", "WMM3_AC1", "WMM3_AC2", "WMM3_AC3",
								"Band0_ALTX", "Band0_BMC", "Band0_BNC", "Band0_PSMP",
								"Band1_ALTX", "Band1_BMC", "Band1_BNC", "Band1_PSMP",
								"Invalid"};
static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "PDA", "Invalid"};
static char *pkt_ft_str[] = {"cut_through", "store_forward", "cmd", "PDA_FW_Download"};

VOID dump_tmac_info(RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;
	TMAC_TXD_0 *txd_0 = (TMAC_TXD_0 *)tmac_info;
	TMAC_TXD_1 *txd_1 = (TMAC_TXD_1 *)(tmac_info + sizeof(TMAC_TXD_0));
	TMAC_TXD_7_PP *txd_7 = (PTMAC_TXD_7_PP)&txd_s->TxD7;
	UCHAR q_idx = 0;

	hex_dump("TMAC_Info Raw Data: ", (UCHAR *)tmac_info, pAd->chipCap.TXWISize);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_TXD Fields:\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_0:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPortID=%d(%s)\n", txd_0->p_idx,
				txd_0->p_idx < 2 ? p_idx_str[txd_0->p_idx] : "Invalid"));
	if (txd_0->p_idx == P_IDX_LMAC)
		q_idx = txd_0->q_idx % 0x18;
	else
		q_idx = ((txd_0->q_idx == TxQ_IDX_MCU_PDA) ? txd_0->q_idx : ( txd_0->q_idx % 0x4));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueID=0x%x(%s %s)\n", txd_0->q_idx,
				(txd_0->p_idx == P_IDX_LMAC ? "LMAC" : "MCU"),
				txd_0->p_idx == P_IDX_LMAC ? q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxByteCnt=%d\n", txd_0->TxByteCount));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_1:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\twlan_idx=%d\n", txd_1->wlan_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrFmt=%d(%s)\n",
				txd_1->hdr_format,
				txd_1->hdr_format < 4 ? hdr_fmt_str[txd_1->hdr_format] : "N/A"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrInfo=0x%x\n", txd_1->hdr_info));
	switch (txd_1->hdr_format)
	{
		case TMI_HDR_FT_NON_80211:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_MRD),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_EOSP),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_RMVL),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_VLAN),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_ETYP)));
			break;
		case TMI_HDR_FT_CMD:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tRsvd=0x%x\n", txd_1->hdr_info));
			break;
		case TMI_HDR_FT_NOR_80211:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tHeader Len=%d(WORD)\n",
						txd_1->hdr_info & TMI_HDR_INFO_2_MASK_LEN));
			break;
		case TMI_HDR_FT_ENH_80211:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tEOSP=%d, AMS=%d\n",
						txd_1->hdr_info & (1<< TMI_HDR_INFO_3_BIT_EOSP),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_3_BIT_AMS)));
			break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxDFormatType=%d(%s format)\n", txd_1->ft,
				txd_1->ft == TMI_FT_LONG ? "Long - 8 DWORD" : "Short - 3 DWORD"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttxd_len=%d page(%d DW)\n",
				txd_1->txd_len == 0 ? 1 : 2, (txd_1->txd_len + 1) * 16));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrPad=%d(Padding Mode: %s, padding bytes: %d)\n",
				txd_1->hdr_pad, ((txd_1->hdr_pad & (TMI_HDR_PAD_MODE_TAIL << 1)) ? "tail": "head"),
				((txd_1->hdr_pad & 0x1) ? 2: 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tUNxV=%d\n", txd_1->UNxV));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tUTxB_AMSDU_C=%d\n", txd_1->UTxB_AMSDU_C));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", txd_1->tid));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpkt_ft=%d(%s)\n",
				txd_1->pkt_ft, pkt_ft_str[txd_1->pkt_ft]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\town_mac=%d\n", txd_1->OwnMacAddr));

	if (txd_s->TxD1.ft == TMI_FT_LONG)
	{
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)tmac_info;
		TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
		TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
		TMAC_TXD_4 *txd_4 = &txd_l->TxD4;
		TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
		TMAC_TXD_6 *txd_6 = &txd_l->TxD6;

		txd_7 = (PTMAC_TXD_7_PP)&txd_l->TxD7;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_2:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsub_type=%d\n", txd_2->sub_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfrm_type=%d\n", txd_2->frm_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDP=%d\n", txd_2->ndp));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDPA=%d\n", txd_2->ndpa));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSounding=%d\n", txd_2->sounding));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRTS=%d\n", txd_2->rts));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbc_mc_pkt=%d\n", txd_2->bc_mc_pkt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBIP=%d\n", txd_2->bip));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDuration=%d\n", txd_2->duration));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHE(HTC Exist)=%d\n", txd_2->htc_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFRAG=%d\n", txd_2->frag));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tReamingLife/MaxTx time=%d\n", txd_2->max_tx_time));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_offset=%d\n", txd_2->pwr_offset));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tba_disable=%d\n", txd_2->ba_disable));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttiming_measure=%d\n", txd_2->timing_measure));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate=%d\n", txd_2->fix_rate));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_3:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNoAck=%d\n", txd_3->no_ack));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPF=%d\n", txd_3->protect_frm));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_cnt=%d\n", txd_3->tx_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tremain_tx_cnt=%d\n", txd_3->remain_tx_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn=%d\n", txd_3->sn));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_vld=%d\n", txd_3->pn_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn_vld=%d\n", txd_3->sn_vld));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_4:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_low=0x%x\n", txd_4->pn_low));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_5:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_host=%d\n", txd_5->tx_status_2_host));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_mcu=%d\n", txd_5->tx_status_2_mcu));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_fmt=%d\n", txd_5->tx_status_fmt));
		if (txd_5->tx_status_2_host || txd_5->tx_status_2_mcu)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpid=%d\n", txd_5->pid));
		}

		if (txd_2->fix_rate)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tda_select=%d\n", txd_5->da_select));
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_mgmt=0x%x\n", txd_5->pwr_mgmt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_high=0x%x\n", txd_5->pn_high));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_6:\n"));
		if (txd_2->fix_rate)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate_mode=%d\n", txd_6->fix_rate_mode));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGI=%d(%s)\n", txd_6->gi, txd_6->gi == 0? "LONG" : "SHORT"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tldpc=%d(%s)\n", txd_6->ldpc, txd_6->ldpc == 0 ? "BCC" : "LDPC"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxBF=%d\n", txd_6->TxBF));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_rate=0x%x\n", txd_6->tx_rate));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tant_id=%d\n", txd_6->ant_id));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdyn_bw=%d\n", txd_6->dyn_bw));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbw=%d\n", txd_6->bw));
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_7:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\thif_err=0x%x\n", txd_7->hif_err));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpse_fid=0x%x\n", txd_7->pse_fid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tspe_idx=0x%x\n", txd_7->spe_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsw_tx_time=%d\n", txd_7->sw_tx_time));
}


static char *rmac_info_type_str[]={
		"TXS",
		"RXV",
		"RxNormal",
		"DupRFB",
		"TMR",
		"Unknown",
};

static inline char *rxd_pkt_type_str(INT pkt_type)
{
	if (pkt_type <= 0x04)
	{
		return rmac_info_type_str[pkt_type];
	}
	else
	{
		return rmac_info_type_str[5];
	}
}


VOID dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(RXD_BASE_STRUCT));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxData_BASE:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPktType=%d(%s)\n",
					rxd_base->RxD0.PktType,
					rxd_pkt_type_str(rxd_base->RxD0.PktType)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGroupValid=%x\n", rxd_base->RxD0.RfbGroupVld));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxByteCnt=%d\n", rxd_base->RxD0.RxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIP/UT=%d/%d\n", rxd_base->RxD0.IpChkSumOffload, rxd_base->RxD0.UdpTcpChkSumOffload));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->RxD0.EthTypeOffset));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
				rxd_base->RxD1.HTC,
				(rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
				(rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
				rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
				rxd_base->RxD1.BcnWithBMcst, rxd_base->RxD1.BcnWithUCast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tKeyID=%d\n", rxd_base->RxD1.KeyId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChFrequency=%x\n", rxd_base->RxD1.ChFreq));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdearLength(MAC)=%d\n", rxd_base->RxD1.MacHdrLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerOffset(HO)=%d\n", rxd_base->RxD1.HdrOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerTrans(H)=%d\n", rxd_base->RxD1.HdrTranslation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->RxD1.PayloadFmt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID=%d\n", rxd_base->RxD1.RxDBssidIdx));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlanIndex=%d\n", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", rxd_base->RxD2.RxDTid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSW BIT=%d\n", rxd_base->RxD2.SwBit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFCE Error(FC)=%d\n", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Mismatch(CM)=%d\n", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Length Mismatch(CLM)=%d\n", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tICV Err(I)=%d\n", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTKIP MIC Err(T)=%d\n", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLength Mismatch(LM)=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDeAMSDU Fail(DAF)=%d\n", rxd_base->RxD2.DeAmsduFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tExceedMax Rx Length(EL)=%d\n", rxd_base->RxD2.ExMaxRxLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrTransFail(HTF)=%d\n", rxd_base->RxD2.HdrTransFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tInterested Frame(INTF)=%d\n", rxd_base->RxD2.InterestedFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNull Frame(NULL)=%d\n", rxd_base->RxD2.NullFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon Data Frame(NDATA)=%d\n", rxd_base->RxD2.NonDataFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon-AMPDU Subframe(NASF)=%d\n", rxd_base->RxD2.NonAmpduSfrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon AMPDU(NAMP)=%d\n", rxd_base->RxD2.NonAmpduFrm));

	if (rxd_base->RxD0.RfbGroupVld)
	{
		// TODO: dump group info!!
	}
}

VOID dump_rmac_info_for_ICVERR(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;
	int LogDbgLvl = DBG_LVL_ERROR;

	if (pAd->chipCap.hif_type != HIF_MT) {
		return;
	}

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	if (pkt_type != RMAC_RX_PKT_TYPE_RX_NORMAL)
	{
		return;
	}

#ifdef WH_EZ_SETUP    
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		LogDbgLvl = DBG_LVL_TRACE;
#endif

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tHTC/UC2ME/MC/BC=%d/%d/%d/%d",
				rxd_base->RxD1.HTC,
				(rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
				(rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
				rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", WlanIndex=%d", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", SEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tFCE Error(FC)=%d", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", CM=%d", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", CLM=%d", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", I=%d", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", T=%d", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", LM=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));

}


VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info;
}


VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//RXV_FRM_STRUC *rxv_frm = (RXV_FRM_STRUC *)rmac_info;
}


VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info;
}


VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info;
}


VOID dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RMAC_RXD Header Format :%s\n",
				rxd_pkt_type_str(pkt_type)));
	switch (pkt_type)
	{
		case RMAC_RX_PKT_TYPE_RX_TXS:
			dump_rmac_info_txs(pAd, rmac_info);
			break;
		case RMAC_RX_PKT_TYPE_RX_TXRXV:
			dump_rmac_info_rxv(pAd, rmac_info);
			break;
		case RMAC_RX_PKT_TYPE_RX_NORMAL:
			dump_rmac_info_normal(pAd, rmac_info);
			break;
		case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
			dump_rmac_info_rfb(pAd, rmac_info);
			break;
		case RMAC_RX_PKT_TYPE_RX_TMR:
			dump_rmac_info_tmr(pAd, rmac_info);
			break;
		default:
			break;
	}
}


VOID DumpTxSFormat(RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	//TXS_D_4 *TxSD4 = &txs_entry->TxSD4;
	//TXS_D_5 *TxSD5 = &txs_entry->TxSD5;
	//TXS_D_6 *TxSD6 = &txs_entry->TxSD6;

	if (Format == TXS_FORMAT0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\tType=TimeStamp/FrontTime Mode\n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t\tTXSFM=%d, TXS2M/H=%d/%d, FixRate=%d, TxRate/BW=0x%x/%d\n",
					TxSD0->TxSFmt, TxSD0->TxS2M, TxSD0->TxS2H,
					TxSD0->TxS_FR, TxSD0->TxRate, TxSD1->TxS_TxBW));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail=%d/%d/%d/%d/%d/%d, PS=%d, Pid=%d\n",
                                    TxSD0->ME, TxSD0->RE, TxSD0->LE, TxSD0->BE, TxSD0->TxOp,
                                    TxSD0->BAFail, TxSD0->PSBit, TxSD0->TxS_PId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t\tTid=%d, AntId=%d, ETxBF/ITxBf=%d/%d\n",
                                    TxSD1->TxS_Tid, TxSD1->TxS_AntId,
                                    TxSD1->TxS_ETxBF, TxSD1->TxS_ITxBF));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t\tTxPwrdBm=0x%x, FinalMPDU=0x%x, AMPDU=0x%x\n",
                                    TxSD1->TxPwrdBm, TxSD1->TxS_FianlMPDU, TxSD1->TxS_AMPDU));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
                                    TxSD2->TxS_TxDelay, TxSD2->TxS_RxVSN, TxSD2->TxS_WlanIdx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\t\tSN=0x%x, MPDU TxCnt=%d, LastTxRateIdx=%d\n",
                                    TxSD3->type_0.TxS_SN, TxSD3->type_0.TxS_MpduTxCnt,
                                    TxSD3->type_0.TxS_LastTxRateIdx));
	}
	else if (Format == TXS_FORMAT1)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\tType=Noisy/RCPI Mode\n"));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: Unknown TxSFormat(%d)\n", __FUNCTION__, Format));
	}
}


INT dump_dmac_mib_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val, mac_val1, idx, band_idx = 0, band_offset = 0, ampdu_cnt[7];
	UINT32 msdr6, msdr7, msdr8, msdr9, msdr10, msdr16, msdr17, msdr18, msdr19, msdr20, msdr21;
	UINT32 mbxsdr[4][4];
	UINT32 mbtcr[16], mbtbcr[16], mbrcr[16], mbrbcr[16];
	UINT32 btcr[4], btbcr[4], brcr[4], brbcr[4];
    UINT32 mu_cnt[5];

	for (band_idx = 0; band_idx < 2; band_idx++)
	{
    if(arg != NULL && band_idx != simple_strtoul(arg, 0, 10))
    {
        continue;
    }
		band_offset = 0x200 * band_idx;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band %d MIB Status\n", band_idx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));

		RTMP_IO_READ32(pAd, MIB_M0SCR0 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MIB Status Control=0x%x\n", mac_val));
		RTMP_IO_READ32(pAd, MIB_M0PBSCR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MIB Per-BSS Status Control=0x%x\n", mac_val));

		RTMP_IO_READ32(pAd, MIB_M0SDR6 + band_offset, &msdr6);
		RTMP_IO_READ32(pAd, MIB_M0SDR7 + band_offset, &msdr7);
		RTMP_IO_READ32(pAd, MIB_M0SDR8 + band_offset, &msdr8);
		RTMP_IO_READ32(pAd, MIB_M0SDR9 + band_offset, &msdr9);
		RTMP_IO_READ32(pAd, MIB_M0SDR10 + band_offset, &msdr10);
		RTMP_IO_READ32(pAd, MIB_M0SDR16 + band_offset, &msdr16);
		RTMP_IO_READ32(pAd, MIB_M0SDR17 + band_offset, &msdr17);
		RTMP_IO_READ32(pAd, MIB_M0SDR18 + band_offset, &msdr18);
		RTMP_IO_READ32(pAd, MIB_M0SDR19 + band_offset, &msdr19);
		RTMP_IO_READ32(pAd, MIB_M0SDR20 + band_offset, &msdr20);
		RTMP_IO_READ32(pAd, MIB_M0SDR21 + band_offset, &msdr21);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Phy/Timing Related Counters===\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannelIdleCnt=0x%x\n", msdr6 & 0xffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCCA_NAV_Tx_Time=0x%x\n", msdr9 & 0xffffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRx_MDRDY_CNT=0x%x\n", msdr10 & 0x3ffffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tCCK_MDRDY=0x%x, OFDM_MDRDY=0x%x, OFDM_GREEN_MDRDY=0x%x\n",
					msdr19 & 0x3ffffff, msdr20 & 0x3ffffff, msdr21 & 0x3ffffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPrim CCA Time=0x%x\n", msdr16 & 0xffffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSec CCA Time=0x%x\n", msdr17 & 0xffffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPrim ED Time=0x%x\n", msdr18 & 0xffffff));


		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Tx Related Counters(Generic)===\n"));
		RTMP_IO_READ32(pAd, MIB_M0SDR0 + band_offset, &mac_val);
#ifdef MT7615
#ifdef CONFIG_AP_SUPPORT
		if (band_idx == DBDC_BAND0) 
		{
			mac_val += pAd->BcnCheckInfo.totalbcncnt0;
			pAd->BcnCheckInfo.totalbcncnt0 = 0;
		}
		else if (band_idx == DBDC_BAND1)
		{
			mac_val += pAd->BcnCheckInfo.totalbcncnt1;
			pAd->BcnCheckInfo.totalbcncnt1 = 0;
		}
#endif
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBeaconTxCnt=0x%x\n", (mac_val & 0xffff)));
		RTMP_IO_READ32(pAd, MIB_M0DR0 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx 40MHz Cnt=0x%x\n", (mac_val >> 16) & 0xffff));
		RTMP_IO_READ32(pAd, MIB_M0DR1 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx 80MHz Cnt=0x%x\n", mac_val& 0xffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx 160MHz Cnt=0x%x\n", (mac_val >> 16) & 0xffff));


		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===AMPDU Related Counters===\n"));
		RTMP_IO_READ32(pAd, MIB_M0SDR12 + band_offset, &ampdu_cnt[0]);
		RTMP_IO_READ32(pAd, MIB_M0SDR14 + band_offset, &ampdu_cnt[1]);
		RTMP_IO_READ32(pAd, MIB_M0SDR15 + band_offset, &ampdu_cnt[2]);
		RTMP_IO_READ32(pAd, MIB_M0DR2 + band_offset, &ampdu_cnt[3]);
		RTMP_IO_READ32(pAd, MIB_M0DR3 + band_offset, &ampdu_cnt[4]);
		RTMP_IO_READ32(pAd, MIB_M0DR4 + band_offset, &ampdu_cnt[5]);
		RTMP_IO_READ32(pAd, MIB_M0DR5 + band_offset, &ampdu_cnt[6]);
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRx BA_Cnt=0x%x\n", ampdu_cnt[0] & 0xffff));
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx AMPDU_Burst_Cnt=0x%x\n", (ampdu_cnt[0] >> 16 ) & 0xffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx AMPDU_Pkt_Cnt=0x%x\n", ampdu_cnt[0]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx AMPDU_MPDU_Pkt_Cnt=0x%x\n", ampdu_cnt[1]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU SuccessCnt=0x%x\n", ampdu_cnt[2] & 0xffffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx Agg Range: \t1 \t2~5 \t6~15 \t16~22 \t23~33 \t34~49 \t50~57 \t58~64\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
						(ampdu_cnt[3]) & 0xffff, (ampdu_cnt[3] >> 16) & 0xffff,
						(ampdu_cnt[4]) & 0xffff, (ampdu_cnt[4] >> 16) & 0xffff,
						(ampdu_cnt[5]) & 0xffff, (ampdu_cnt[5] >> 16) & 0xffff,
						(ampdu_cnt[6]) & 0xffff, (ampdu_cnt[6] >> 16) & 0xffff));


        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===MU Related Counters===\n"));
        MAC_IO_READ32(pAd, MIB_M0SDR34, &mu_cnt[0]);
        MAC_IO_READ32(pAd, MIB_M0DR8, &mu_cnt[1]);
        MAC_IO_READ32(pAd, MIB_M0DR9, &mu_cnt[2]);
        MAC_IO_READ32(pAd, MIB_M0DR10, &mu_cnt[3]);
        MAC_IO_READ32(pAd, MIB_M0DR11, &mu_cnt[4]);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMUBF_TX_COUNT=0x%x\n", mu_cnt[0] & 0xffff));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMU_TX_MPDU_COUNT(Ok+Fail)=0x%x\n", mu_cnt[1]));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[2]));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMU_TO_SU_PPDU_COUNT=0x%x\n", mu_cnt[3] & 0xffff));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[4]));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Rx Related Counters(Generic)===\n"));
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tVector Overflow Drop Cnt=0x%x\n", (msdr6 >> 16 ) & 0xffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tVector Missmacth Cnt=0x%x\n", msdr7 & 0xffff));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDelimiter Fail Cnt=0x%x\n", msdr8& 0xffff));
		RTMP_IO_READ32(pAd, MIB_M0SDR3 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxFCSErrCnt=0x%x\n", (mac_val & 0xffff)));
		RTMP_IO_READ32(pAd, MIB_M0SDR4 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxFifoFullCnt=0x%x\n", (mac_val & 0xffff)));
		RTMP_IO_READ32(pAd, MIB_M0SDR11 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxLenMismatch=0x%x\n", (mac_val & 0xffff)));
		RTMP_IO_READ32(pAd, MIB_M0SDR5 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxMPDUCnt=0x%x\n", (mac_val & 0xffff)));
		RTMP_IO_READ32(pAd, MIB_M0SDR29 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPFDropCnt=0x%x\n", (mac_val & 0x00ff)));
		RTMP_IO_READ32(pAd, MIB_M0SDR22 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRx AMPDU Cnt=0x%x\n", mac_val));
		// TODO: shiang-MT7615, is MIB_M0SDR23 used for Rx total byte count for all or just AMPDU only???
		RTMP_IO_READ32(pAd, MIB_M0SDR23 + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRx Total ByteCnt=0x%x\n", mac_val));
	}

	for (idx = 0; idx < 4; idx++) {
		RTMP_IO_READ32(pAd, WTBL_BTCRn + idx * 4, &btcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_BTBCRn + idx * 4, &btbcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_BRCRn + idx * 4, &brcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_BRBCRn + idx * 4, &brbcr[idx]);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Per-BSS Related Tx/Rx Counters===\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n"));
	for (idx = 0; idx < 4; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
					idx, (btcr[idx] >> 16) & 0xffff, btcr[idx] & 0xffff, btbcr[idx],
					(brcr[idx] >> 16) & 0xffff, brcr[idx] & 0xffff, brbcr[idx]));
	}


	for (idx = 0; idx < 4; idx++)
	{
		RTMP_IO_READ32(pAd, MIB_MB0SDR0 + idx * 0x10, &mbxsdr[idx][0]);
		RTMP_IO_READ32(pAd, MIB_MB0SDR1 + idx * 0x10, &mbxsdr[idx][1]);
		RTMP_IO_READ32(pAd, MIB_MB0SDR2 + idx * 0x10, &mbxsdr[idx][2]);
		RTMP_IO_READ32(pAd, MIB_MB0SDR3 + idx * 0x10, &mbxsdr[idx][3]);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Per-MBSS Related MIB Counters===\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2/3Cnt\n"));
	for (idx = 0; idx < 4; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x/0x%x\n",
					idx, mbxsdr[idx][0], (mbxsdr[idx][0] >> 16) & 0xffff,
					mbxsdr[idx][1], (mbxsdr[idx][1] >> 16) & 0xffff,
					mbxsdr[idx][2], (mbxsdr[idx][2] >> 16) & 0xffff,
					mbxsdr[idx][3] & 0xffff));
	}


	for (idx = 0; idx < 8; idx++) {
		RTMP_IO_READ32(pAd, WTBL_MBTCRn + idx * 4, &mbtcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_MBRCRn + idx * 4, &mbrcr[idx]);
	}

	for (idx = 0; idx < 16; idx++) {
		RTMP_IO_READ32(pAd, WTBL_MBTBCRn + idx * 4, &mbtbcr[idx]);
		RTMP_IO_READ32(pAd, WTBL_MBRBCRn + idx * 4, &mbrbcr[idx]);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Per-MBSS Related Tx/Rx Counters===\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MBSSIdx   TxDataCnt  TxByteCnt  RxDataCnt  RxByteCnt\n"));
	for (idx = 0; idx < 16; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
					idx,
					((idx % 2 == 1) ? (mbtcr[idx/2] >> 16) & 0xffff : mbtcr[idx/2] & 0xffff),
					mbtbcr[idx],
					((idx % 2 == 1)  ? (mbrcr[idx/2] >> 16) & 0xffff : mbrcr[idx/2] & 0xffff),
					mbrbcr[idx]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Dummy delimiter insertion result ===\n"));

	RTMP_IO_READ32(pAd, MIB_M0DR6, &mac_val);
	RTMP_IO_READ32(pAd, MIB_M0DR7, &mac_val1);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Range1 = %d\t Range2 = %d\t Range3 = %d\t Range4 = %d\n", GET_TX_DDLMT_RNG1_CNT(mac_val), GET_TX_DDLMT_RNG2_CNT(mac_val), GET_TX_DDLMT_RNG3_CNT(mac_val1), GET_TX_DDLMT_RNG4_CNT(mac_val1)));

#ifdef TRACELOG_TCP_PKT
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TCP RxAck = %d\t TxData = %d",
                          pAd->u4TcpRxAckCnt, pAd->u4TcpTxDataCnt));
        pAd->u4TcpRxAckCnt = 0;
        pAd->u4TcpTxDataCnt = 0;
#endif //TRACELOG_TCP_PKT

        return TRUE;
}


INT dump_dmac_pse_data(RTMP_ADAPTER *pAd, UINT32 StartFID, UINT32 FrameNums)
{
	UINT16 CurFID;
	UINT8 Index, DwIndex;
	UINT32 Value;
	UINT32 PseData[4 * 8];

	CurFID = StartFID;
	for (Index = 0; Index < FrameNums; Index++)
	{
		for (DwIndex = 0; DwIndex < 8; DwIndex++)
		{
			RTMP_IO_READ32(pAd, (MT_PCI_REMAP_ADDR_1 + (((CurFID) * 128)) + (DwIndex * 4)),
				&PseData[DwIndex * 4]);
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FID:0x%x\n", CurFID));

#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, (UCHAR *)(PseData), TYPE_TMACINFO, 32);
#endif
		dump_tmac_info(pAd, (UCHAR *)PseData);

#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, (UCHAR *)(PseData), TYPE_TMACINFO, 32);
#endif
		RTMP_IO_READ32(pAd, C_GF, &Value);
		Value &= ~CURR_FID_MASK;
		Value |= SET_CURR_FID(CurFID);
		RTMP_IO_WRITE32(pAd, C_GF, Value);
		RTMP_IO_READ32(pAd, C_GF, &Value);
		CurFID = GET_RETURN_FID(Value);

		if (CurFID == 0xfff)
			break;
	}

	return TRUE;
}


VOID Update_Mib_Bucket_One_Sec(RTMP_ADAPTER *pAd)
{
	//UINT32  CrValue;

	UCHAR   i=0, j=0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	
	for(i=0;i<concurrent_bands;i++) {
		if (pAd->OneSecMibBucket.Enabled[i] == TRUE) {
			pAd->OneSecMibBucket.ChannelBusyTime[i] = 0;
			pAd->OneSecMibBucket.OBSSAirtime[i] = 0;
			pAd->OneSecMibBucket.MyTxAirtime[i] = 0;
			pAd->OneSecMibBucket.MyRxAirtime[i] = 0;
			pAd->OneSecMibBucket.EDCCAtime[i] =  0;
			pAd->OneSecMibBucket.MdrdyCount[i] = 0;
			pAd->OneSecMibBucket.PdCount[i] = 0;
					
			
			for (j=0; j<2; j++) {
				pAd->OneSecMibBucket.ChannelBusyTime[i] += pAd->MsMibBucket.ChannelBusyTime[i][j];
				pAd->OneSecMibBucket.OBSSAirtime[i] += pAd->MsMibBucket.OBSSAirtime[i][j];
				pAd->OneSecMibBucket.MyTxAirtime[i] += pAd->MsMibBucket.MyTxAirtime[i][j];
				pAd->OneSecMibBucket.MyRxAirtime[i] += pAd->MsMibBucket.MyRxAirtime[i][j];
				pAd->OneSecMibBucket.EDCCAtime[i] += pAd->MsMibBucket.EDCCAtime[i][j];
				pAd->OneSecMibBucket.MdrdyCount[i] += pAd->MsMibBucket.MdrdyCount[i][j];
				pAd->OneSecMibBucket.PdCount[i] += pAd->MsMibBucket.PdCount[i][j];
			}	
				
			
		}
	}
        
}

VOID Update_Mib_Bucket_500Ms(RTMP_ADAPTER *pAd)
{
	UINT32  CrValue;

	UCHAR   i=0;
	UCHAR	CurrIdx = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	pAd->MsMibBucket.CurIdx ++ ;

	if (pAd->MsMibBucket.CurIdx >=2)
		pAd->MsMibBucket.CurIdx = 0;

	CurrIdx = pAd->MsMibBucket.CurIdx;

	
	for(i=0;i<concurrent_bands;i++) {
		if (pAd->MsMibBucket.Enabled == TRUE) {
			//Channel Busy Time
			HW_IO_READ32(pAd, MIB_M0SDR16 +(i*BandOffset), &CrValue);
			pAd->MsMibBucket.ChannelBusyTime[i][CurrIdx] = CrValue;
			//OBSS Air time
			HW_IO_READ32(pAd, RMAC_MIBTIME5 + i*4, &CrValue);
			pAd->MsMibBucket.OBSSAirtime[i][CurrIdx] = CrValue;
			//My Tx Air time
			HW_IO_READ32(pAd, MIB_M0SDR36 + (i*BandOffset), &CrValue);
			pAd->MsMibBucket.MyTxAirtime[i][CurrIdx] = CrValue;
			//My Rx Air time
			HW_IO_READ32(pAd, MIB_M0SDR37 + (i*BandOffset), &CrValue);
			pAd->MsMibBucket.MyRxAirtime[i][CurrIdx] = CrValue;
			//EDCCA time
			HW_IO_READ32(pAd, MIB_M0SDR18 + (i*BandOffset), &CrValue);
			pAd->MsMibBucket.EDCCAtime[i][CurrIdx] = CrValue;
			//Reset OBSS Air time        
			HW_IO_READ32(pAd, RMAC_MIBTIME0, &CrValue);
			CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
			CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
			HW_IO_WRITE32(pAd, RMAC_MIBTIME0, CrValue);

			HW_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS0 + (i*BandOffset), &CrValue); //PD count
			pAd->MsMibBucket.PdCount[i][CurrIdx] = CrValue;
			HW_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS5 + (i*BandOffset), &CrValue); // MDRDY count
			pAd->MsMibBucket.MdrdyCount[i][CurrIdx] = CrValue;

			HW_IO_READ32(pAd, PHY_BAND0_PHYMUX_5 + (i*BandOffset), &CrValue);
			CrValue &= 0xff8fffff;
			HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5 + (i*BandOffset), CrValue); //Reset
			CrValue |= 0x500000; 
			HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5 + (i*BandOffset), CrValue); //Enable
		}
	}
        
        
}

static INT ple_pg_cnt[]={256, 512, 512, 1024, 768, 1536, 1024, 2048, 1280, 2560, 1536, 3072, 1792, 3584, 0, 0};
INT dump_dmac_pse_info(RTMP_ADAPTER *pAd)
{
    UINT32 pse_buf_ctrl, pg_sz, pg_num;
    UINT32 pse_stat, pg_flow_ctrl[16] = {0};
    UINT32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail;
    UINT32 max_q, min_q, rsv_pg, used_pg;
    INT32 i;

    HW_IO_READ32(pAd, PSE_PBUF_CTRL, &pse_buf_ctrl);

    HW_IO_READ32(pAd, PSE_QUEUE_EMPTY, &pse_stat);

    HW_IO_READ32(pAd, PSE_FREEPG_CNT, &pg_flow_ctrl[0]);
    HW_IO_READ32(pAd, PSE_FREEPG_HEAD_TAIL, &pg_flow_ctrl[1]);
    HW_IO_READ32(pAd, PSE_PG_HIF0_GROUP, &pg_flow_ctrl[2]);
    HW_IO_READ32(pAd, PSE_HIF0_PG_INFO, &pg_flow_ctrl[3]);
    HW_IO_READ32(pAd, PSE_PG_HIF1_GROUP, &pg_flow_ctrl[4]);
    HW_IO_READ32(pAd, PSE_HIF1_PG_INFO, &pg_flow_ctrl[5]);
    HW_IO_READ32(pAd, PSE_PG_CPU_GROUP, &pg_flow_ctrl[6]);
    HW_IO_READ32(pAd, PSE_CPU_PG_INFO, &pg_flow_ctrl[7]);
    HW_IO_READ32(pAd, PSE_PG_LMAC0_GROUP, &pg_flow_ctrl[8]);
    HW_IO_READ32(pAd, PSE_LMAC0_PG_INFO, &pg_flow_ctrl[9]);
    HW_IO_READ32(pAd, PSE_PG_LMAC1_GROUP, &pg_flow_ctrl[10]);
    HW_IO_READ32(pAd, PSE_LMAC1_PG_INFO, &pg_flow_ctrl[11]);
    HW_IO_READ32(pAd, PSE_PG_LMAC2_GROUP, &pg_flow_ctrl[12]);
    HW_IO_READ32(pAd, PSE_LMAC2_PG_INFO, &pg_flow_ctrl[13]);
	HW_IO_READ32(pAd, PSE_PG_PLE_GROUP, &pg_flow_ctrl[14]);
	HW_IO_READ32(pAd, PSE_PLE_PG_INFO, &pg_flow_ctrl[15]);
	
    //Configuration Info
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("PSE Configuration Info:\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tPacket Buffer Control(0x82068014): 0x%08x\n", pse_buf_ctrl));
    pg_sz = (pse_buf_ctrl & (0x1<<31))>>31;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 256 : 128)));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tPage Offset=%d(in unit of 64KB)\n", GET_PSE_PBUF_OFFSET(pse_buf_ctrl)));
    pg_num = PSE_GET_TOTAL_PAGE_CFG(pse_buf_ctrl);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tConfigured Total Page=%d(%d pages)\n", pg_num, (pg_num < 14 ? ple_pg_cnt[pg_num] : 0)));
    pg_num = (pse_buf_ctrl & PSE_TOTAL_PAGE_NUM_MASK);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tAvailable Total Page=%d pages\n", pg_num));

    //Page Flow Control
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("PSE Page Flow Control:\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tFree page counter(0x82068100): 0x%08x\n", pg_flow_ctrl[0]));
    fpg_cnt = pg_flow_ctrl[0] & 0xfff;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe toal page number of free=0x%03x\n",fpg_cnt));
    ffa_cnt = (pg_flow_ctrl[0] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe free page numbers of free for all=0x%03x\n",ffa_cnt));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tFree page head and tail(0x82068104): 0x%08x\n", pg_flow_ctrl[1]));
    fpg_head = pg_flow_ctrl[1] & 0xfff;
    fpg_tail = (pg_flow_ctrl[1] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n",fpg_tail ,fpg_head));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of HIF0 group(0x82068110): 0x%08x\n", pg_flow_ctrl[2]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tHIF0 group page status(0x82068114): 0x%08x\n", pg_flow_ctrl[3]));
    min_q = pg_flow_ctrl[2] & 0xfff;
    max_q = (pg_flow_ctrl[2] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[3] & 0xfff;
    used_pg = (pg_flow_ctrl[3] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n",used_pg ,rsv_pg));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of HIF1 group(0x82068118): 0x%08x\n", pg_flow_ctrl[4]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tHIF1 group page status(0x8206811c): 0x%08x\n", pg_flow_ctrl[5]));
    min_q = pg_flow_ctrl[4] & 0xfff;
    max_q = (pg_flow_ctrl[4] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[5] & 0xfff;
    used_pg = (pg_flow_ctrl[5] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n",used_pg ,rsv_pg));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of CPU group(0x82068150): 0x%08x\n", pg_flow_ctrl[6]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tCPU group page status(0x82068154): 0x%08x\n", pg_flow_ctrl[7]));
    min_q = pg_flow_ctrl[6] & 0xfff;
    max_q = (pg_flow_ctrl[6] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[7] & 0xfff;
    used_pg = (pg_flow_ctrl[7] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n",used_pg ,rsv_pg));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of LMAC0 group(0x82068170): 0x%08x\n", pg_flow_ctrl[8]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tLMAC0 group page status(0x82068174): 0x%08x\n", pg_flow_ctrl[9]));
    min_q = pg_flow_ctrl[8] & 0xfff;
    max_q = (pg_flow_ctrl[8] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[9] & 0xfff;
    used_pg = (pg_flow_ctrl[9] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n",used_pg ,rsv_pg));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of LMAC1 group(0x82068178): 0x%08x\n", pg_flow_ctrl[10]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tLMAC1 group page status(0x8206817c): 0x%08x\n", pg_flow_ctrl[11]));
    min_q = pg_flow_ctrl[10] & 0xfff;
    max_q = (pg_flow_ctrl[10] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[11] & 0xfff;
    used_pg = (pg_flow_ctrl[11] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n",used_pg ,rsv_pg));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of LMAC2 group(0x82068180): 0x%08x\n", pg_flow_ctrl[11]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tLMAC2 group page status(0x82068184): 0x%08x\n", pg_flow_ctrl[12]));
    min_q = pg_flow_ctrl[12] & 0xfff;
    max_q = (pg_flow_ctrl[12] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[13] & 0xfff;
    used_pg = (pg_flow_ctrl[13] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n",used_pg ,rsv_pg));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tReserved page counter of PLE group(0x82068190): 0x%08x\n", pg_flow_ctrl[14]));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tPLE group page status(0x82068194): 0x%08x\n", pg_flow_ctrl[15]));
    min_q = pg_flow_ctrl[14] & 0xfff;
    max_q = (pg_flow_ctrl[14] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n",max_q ,min_q));
    rsv_pg = pg_flow_ctrl[15] & 0xfff;
    used_pg = (pg_flow_ctrl[15] & (0xfff << 16)) >> 16;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n",used_pg ,rsv_pg));


    //Queue Empty Status
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("PSE Queue Empty Status:\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\tQUEUE_EMPTY(0x820680b0): 0x%08x\n", pse_stat));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tCPU Q0/1/2/3 empty=%d/%d/%d/%d\n",
                pse_stat & 0x1, ((pse_stat & 0x2) >> 1),
                ((pse_stat & 0x4) >> 2), ((pse_stat & 0x8) >> 3)));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tHIF Q0/1 empty=%d/%d\n",
                ((pse_stat & (0x1 << 16)) >> 16), ((pse_stat & (0x1 << 17)) >> 17)));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tLMAC TX Q empty=%d\n",
                ((pse_stat & (0x1 << 24)) >> 24)));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\t\tRLS_Q empty=%d\n",
                ((pse_stat & (0x1 << 31)) >> 31)));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nonempty Q info:\n"));
	for(i = 0;i < 31;i++)
    {
        if(((pse_stat & (0x1 << i)) >> i) == 0)
        {
                UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

            if(i < 4)
            {
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                                ("\tCPU Q%d: ", i));
                fl_que_ctrl[0] |= (0x1 << 14);
                fl_que_ctrl[0] |= (i << 8);
            }
            else if(i == 16)
            {
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHIF Q0: "));
                fl_que_ctrl[0] |= (0x0 << 14);
                fl_que_ctrl[0] |= (0x0 << 8);
            }
            else if(i == 17)
            {
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHIF  Q1: "));
                fl_que_ctrl[0] |= (0x0 << 14);
                fl_que_ctrl[0] |= (0x1 << 8);
            }
            else if(i == 24)
            {
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tLMAC TX Q: "));
                fl_que_ctrl[0] |= (0x2 << 14);
                fl_que_ctrl[0] |= (0x0 << 8);
            }
            else if(i == 31)
            {
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRLS Q: "));
                fl_que_ctrl[0] |= (0x3 << 14);
                fl_que_ctrl[0] |= (i << 8);
            }
            else
            {
                continue;
            }
            fl_que_ctrl[0] |= (0x1 << 31);
            HW_IO_WRITE32(pAd, PSE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
            HW_IO_READ32(pAd, PSE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
            HW_IO_READ32(pAd, PSE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);

            hfid = fl_que_ctrl[1] & 0xfff;
            tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
            pktcnt = fl_que_ctrl[2] & 0xfff;

            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("tail/head fid = 0x%03x/0x%03x, pkt cnt = %x\n",
                                tfid, hfid, pktcnt));
        }
    }

	return TRUE;
}


static RTMP_REG_PAIR mac_cr_seg[]={
	{0x20000, 0x20010}, /* WF_CFG */
	{WF_TRB_BASE, 0x21040}, /* WF_CFG */
	{WF_AGG_BASE, 0x21240}, /* WF_CFG */
	{WF_ARB_BASE, 0x21440}, /* WF_CFG */
	{0,0},
};


VOID dump_mt_mac_cr(RTMP_ADAPTER *pAd)
{
	INT index = 0;
	UINT32 mac_val, mac_addr, seg_s, seg_e;

	while (mac_cr_seg[index].Register != 0)
	{
		seg_s = mac_cr_seg[index].Register;
		seg_e = mac_cr_seg[index].Value;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WF_CFG Segment(Start=0x%x, End=0x%x)\n",
					seg_s, seg_e));
		for (mac_addr = seg_s; mac_addr < seg_e; mac_addr += 4)
		{
			MAC_IO_READ32(pAd, mac_addr, &mac_val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC[0x%x] = 0x%x\n", mac_addr, mac_val));
		}

		index++;
	};
}


INT mt_mac_fifo_stat_update(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
					__FUNCTION__, __LINE__));

	return FALSE;
}


VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1)
{
	// TODO: shiang-7603
		return;
}


/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
static VOID NICUpdateAmpduRawCounters(RTMP_ADAPTER *pAd,UCHAR BandIdx)
{
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;
	COUNTER_802_11 *wlanCounter;
	UINT32 mac_val,ampdu_range_cnt[4];
	UINT32 Offset = 0x200*BandIdx;


	wlanCounter = &pAd->WlanCounters[BandIdx];

	MAC_IO_READ32(pAd, MIB_M0SDR14+Offset, &AmpduTxCount);
	AmpduTxCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd, MIB_M0SDR15+Offset, &AmpduTxSuccessCount);
	AmpduTxSuccessCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd, MIB_M0DR2+Offset, &mac_val);
	ampdu_range_cnt[0] = mac_val & 0xffff;
	ampdu_range_cnt[1] =  (mac_val >> 16 ) & 0xffff;
	MAC_IO_READ32(pAd, MIB_M0DR3+Offset, &mac_val);
	ampdu_range_cnt[2] = mac_val & 0xffff;
	ampdu_range_cnt[3] =  (mac_val >> 16 ) & 0xffff;


#ifdef STATS_COUNT_SUPPORT
	wlanCounter->AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	wlanCounter->AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif /* STATS_COUNT_SUPPORT */
	wlanCounter->TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	wlanCounter->TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	wlanCounter->TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	wlanCounter->TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];

}

VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32 OldValue,i;
	UINT32 rx_err_cnt, fcs_err_cnt, mdrdy_cnt = 0, fcs_err_cnt_band1 = 0, mdrdy_cnt_band1 = 0;
	UINT32 sig_err = 0;
	//UINT32 TxSuccessCount = 0, TxRetryCount = 0;
#ifdef DBG_DIAGNOSE
	UINT32 bss_tx_cnt;
	UINT32 TxFailCount = 0;
#endif /* DBG_DIAGNOSE */
#ifdef COMPOS_WIN
       COUNTER_MTK *pPrivCounters;
#else
	COUNTER_RALINK *pPrivCounters;
#endif
	COUNTER_802_11 *wlanCounter;
	COUNTER_802_3 *dot3Counters;

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
		return;
	}
#endif /* ERR_RECOVERY */


#ifdef COMPOS_WIN
    wlanCounter = &pAd->Counter.WlanCounters;
    	pPrivCounters = &pAd->Counter.MTKCounters;
    	dot3Counters = &pAd->Counter.Counters8023;
#else
	pPrivCounters = &pAd->RalinkCounters;
        wlanCounter = &pAd->WlanCounters[0];
        dot3Counters = &pAd->Counters8023;
#endif /* COMPOS_WIN */




#ifdef DBG_DIAGNOSE
	MAC_IO_READ32(pAd, MIB_MB0SDR1, &mac_val);
	TxFailCount = (mac_val >> 16) & 0xffff;
	// TODO: shiang, now only check BSS0
	MAC_IO_READ32(pAd, WTBL_BTCRn, &bss_tx_cnt);
	bss_tx_cnt = (bss_tx_cnt >> 16) & 0xffff;
#endif /* DBG_DIAGNOSE */
	MAC_IO_READ32(pAd, MIB_M0SDR3, &rx_err_cnt);
	fcs_err_cnt = rx_err_cnt & 0xffff;
	MAC_IO_READ32(pAd, MIB_M0SDR4, &rx_err_cnt);

	if (pAd->parse_rxv_stat_enable)
	{   
		MAC_IO_READ32(pAd,MIB_M0SDR10,&mdrdy_cnt);
#ifdef MT7615
		mdrdy_cnt = (mdrdy_cnt & 0x3FFFFFF); /* [25:0] Mac Mdrdy*/ 
#endif

		MAC_IO_READ32(pAd, MIB_M1SDR3, &fcs_err_cnt_band1);
#ifdef MT7615
		fcs_err_cnt_band1 = (fcs_err_cnt_band1 & 0xffff); /* [15:0] FCS ERR */ 
#endif

		MAC_IO_READ32(pAd, MIB_M1SDR10, &mdrdy_cnt_band1); 
#ifdef MT7615
		mdrdy_cnt_band1 = (mdrdy_cnt_band1 & 0x3FFFFFF); /* [25:0] Mac Mdrdy*/ 
#endif
	}


	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;

	if (pAd->parse_rxv_stat_enable)
	{
		pAd->AccuOneSecRxBand0FcsErrCnt += fcs_err_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand0MdrdyCnt += mdrdy_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1FcsErrCnt += fcs_err_cnt_band1; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1MdrdyCnt += mdrdy_cnt_band1; /*Used for rx_statistic*/
	}
       
#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue= pAd->WlanCounters[0].FCSErrorCount.u.LowPart;
	pAd->WlanCounters[0].FCSErrorCount.u.LowPart += fcs_err_cnt; /* >> 7);*/
	if (pAd->WlanCounters[0].FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters[0].FCSErrorCount.u.HighPart++;
#endif /* STATS_COUNT_SUPPORT */

	/* Add FCS error count to private counters*/
	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
	OldValue = pPrivCounters->RealFcsErrCount.u.LowPart;
	pPrivCounters->RealFcsErrCount.u.LowPart += fcs_err_cnt;
	if (pPrivCounters->RealFcsErrCount.u.LowPart < OldValue)
		pPrivCounters->RealFcsErrCount.u.HighPart++;

	dot3Counters->RxNoBuffer += (rx_err_cnt & 0xffff);

	for(i=0;i<DBDC_BAND_NUM;i++)
	{
		NICUpdateAmpduRawCounters(pAd,i);
	}

#ifdef CONFIG_QA
    if (pAd->ATECtrl.bQAEnabled == TRUE){
		//Modify Rx stat structure
        //pAd->ATECtrl.rx_stat.RxMacFCSErrCount += fcs_err_cnt;
		MT_ATEUpdateRxStatistic(pAd, 3, wlanCounter);
	}
#endif

#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct *pDiag;
		UINT8 ArrayCurIdx, i;
		struct dbg_diag_info *diag_info;

		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;

		if (pDiag->inited == 0)
		{
			NdisZeroMemory(pDiag, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->wcid = 0;
			pDiag->inited = 1;
            pDiag->diag_cond = 0;
		}
		else
		{
			diag_info = &pDiag->diag_info[ArrayCurIdx];

			if ((pDiag->wcid == 0) && (pAd->MacTab.Size > 0)) {
				UCHAR idx;
				MAC_TABLE_ENTRY *pEntry;

				for (idx = 1; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++)
				{
					pEntry = &pAd->MacTab.Content[idx];
					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst==SST_ASSOC))
					{
						pDiag->wcid = idx;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): DBG_DIAGNOSE Start to monitor the SwQ depth of WCID[%d], ArrayCurIdx=%d\n",
									__FUNCTION__, pDiag->wcid, pDiag->ArrayCurIdx));
						break;
					}
				}
			}

			/* Tx*/
			diag_info->TxFailCnt = TxFailCount;
#ifdef DBG_TX_AGG_CNT
			diag_info->TxAggCnt = AmpduTxCount;
			diag_info->TxNonAggCnt = bss_tx_cnt - AmpduTxCount; // only useful when only one BSS case

			diag_info->TxAMPDUCnt[0] = ampdu_range_cnt[0];
			diag_info->TxAMPDUCnt[1] = ampdu_range_cnt[1];
			diag_info->TxAMPDUCnt[2] = ampdu_range_cnt[2];
			diag_info->TxAMPDUCnt[3] = ampdu_range_cnt[3];
#endif /* DBG_TX_AGG_CNT */

			diag_info->RxCrcErrCnt = fcs_err_cnt;

			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;

			NdisZeroMemory(&pDiag->diag_info[ArrayCurIdx], sizeof(pDiag->diag_info[ArrayCurIdx]));

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
			{
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
			}
		}
	}
#endif /* DBG_DIAGNOSE */

#ifdef MT7615
	PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS1, &sig_err);
	PHY_IO_WRITE32(pAd, RO_BAND0_PHYCTRL_STS1, 0);
	pPrivCounters->SigErrCckCnt += sig_err >> 16;

	PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS2, &sig_err);
	PHY_IO_WRITE32(pAd, RO_BAND0_PHYCTRL_STS2, 0);
	pPrivCounters->SigErrOfdmCnt += sig_err >> 16;

	PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS4, &fcs_err_cnt);
	PHY_IO_WRITE32(pAd, RO_BAND0_PHYCTRL_STS4, 0);
	pPrivCounters->FcsErrCckCnt += fcs_err_cnt >> 16;
	pPrivCounters->FcsErrOfdmCnt += fcs_err_cnt & 0xFFFF;
#endif

	return;
}


/*
	========================================================================

	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/
VOID NicResetRawCounters(RTMP_ADAPTER *pAd)
{
	return;
}


UCHAR tmi_rate_map_cck_lp[]={
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_sp[]={
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_ofdm[]={
	TMI_TX_RATE_OFDM_6M,
	TMI_TX_RATE_OFDM_9M,
	TMI_TX_RATE_OFDM_12M,
	TMI_TX_RATE_OFDM_18M,
	TMI_TX_RATE_OFDM_24M,
	TMI_TX_RATE_OFDM_36M,
	TMI_TX_RATE_OFDM_48M,
	TMI_TX_RATE_OFDM_54M,
};


#define TMI_TX_RATE_CCK_VAL(_mcs) \
	((TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_OFDM_VAL(_mcs) \
	((TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_HT_VAL(_mode, _mcs, _stbc) \
	(((_stbc) << TMI_TX_RATE_BIT_STBC) |\
	 ((_mode) << TMI_TX_RATE_BIT_MODE) | \
	 (_mcs))

#define TMI_TX_RATE_VHT_VAL(_nss, _mcs, _stbc) \
		(((_stbc) << TMI_TX_RATE_BIT_STBC) |\
		 (((_nss -1) & (TMI_TX_RATE_MASK_NSS)) << TMI_TX_RATE_BIT_NSS) | \
		 (TMI_TX_RATE_MODE_VHT << TMI_TX_RATE_BIT_MODE) | \
		 (_mcs))


UINT16 tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
{
	UINT16 tmi_rate = 0, mcs_id = 0;

	stbc = (stbc== TRUE) ? 1 : 0;
	switch (mode)
	{
		case MODE_CCK:
			if (preamble)
            {
                if (mcs < sizeof(tmi_rate_map_cck_lp)/sizeof(tmi_rate_map_cck_lp[0]))
                    mcs_id = tmi_rate_map_cck_lp[mcs];
            }
            else
            {
                if (mcs < sizeof(tmi_rate_map_cck_sp)/sizeof(tmi_rate_map_cck_sp[0]))
				    mcs_id = tmi_rate_map_cck_sp[mcs];
            }
			tmi_rate = (TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (mcs_id);
			break;
		case MODE_OFDM:
            if (mcs < sizeof(tmi_rate_map_ofdm)/sizeof(tmi_rate_map_ofdm[0]))
            {
    			mcs_id = tmi_rate_map_ofdm[mcs];
    			tmi_rate = (TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (mcs_id);
            }
			break;
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			tmi_rate = ((USHORT)(stbc << TMI_TX_RATE_BIT_STBC)) |
					(((nss -1) & TMI_TX_RATE_MASK_NSS) << TMI_TX_RATE_BIT_NSS) |
					((USHORT)(mode << TMI_TX_RATE_BIT_MODE)) |
					((USHORT)(mcs));
//			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): mode=%d, mcs=%d, stbc=%d converted tmi_rate=0x%x\n",
//						__FUNCTION__, mode, mcs, stbc, tmi_rate));
			break;
		case MODE_VHT:
			tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid mode(mode=%d)\n",
						__FUNCTION__, mode));
			break;
	}

	return tmi_rate;
}


UCHAR get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
{
	UINT8 nsts = 1;

	switch (phy_mode)
	{
		case MODE_VHT:
			if (stbc && (vht_nss == 1))
			{
				nsts ++;
            }
			else
			{
				nsts = vht_nss;
			}
			break;
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			if (mcs != 32)
			{
				nsts += (mcs >> 3);
				if (stbc && (nsts == 1))
				{
					nsts ++;
				}
			}
			break;
		case MODE_CCK:
		case MODE_OFDM:
		default:
			break;
	}

	return nsts;
}

// TODO: shiang-MT7615, fix me!
#ifndef WH_EZ_SETUP
static
#endif
UCHAR dmac_wmm_aci_2_hw_ac_que[4][4] =
{
	{
		TxQ_IDX_AC1, /* 0: QID_AC_BE */
		TxQ_IDX_AC0, /* 1: QID_AC_BK */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3:QID_AC_VO */
	},
	{
		TxQ_IDX_AC11, /*:QID_AC_BE */
		TxQ_IDX_AC10,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
	},
	{
		TxQ_IDX_AC21,
		TxQ_IDX_AC20,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
	},
	{
		TxQ_IDX_AC31,
		TxQ_IDX_AC30,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
	}
};


INT MtWriteTMacInfo(RTMP_ADAPTER *pAd, UCHAR *buf, TMAC_INFO *TxInfo)
{
    TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
    TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
    TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
    TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
    TMAC_TXD_2 *txd_2 = NULL;
    TMAC_TXD_3 *txd_3 = NULL;
    TMAC_TXD_5 *txd_5 = NULL;
    TMAC_TXD_6 *txd_6 = NULL;
    TMAC_TXD_7 *txd_7 = NULL;
    UCHAR txd_size;
    UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0,expBf=0,impBf=0, vht_nss = 1;
    TX_RADIO_SET_T *pTxRadioSet = NULL;

    /*DWORD 0*/
    txd_0->p_idx = TxInfo->PortIdx;
    txd_0->q_idx = TxInfo->QueIdx;
    if ((TxInfo->QueIdx < 4) && (TxInfo->WmmSet < 4))
    {
        txd_0->q_idx= dmac_wmm_aci_2_hw_ac_que[TxInfo->WmmSet][TxInfo->QueIdx];
    }
    else
    {
        // TODO: shiang-usw, consider about MCC case!
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                            ("%s(): Non-WMM Q, TxInfo->WmmSet/QueIdx(%d/%d)!\n",
                            __FUNCTION__, TxInfo->WmmSet, TxInfo->QueIdx));

        txd_0->q_idx = TxInfo->QueIdx;
    }



    /*DWORD 1*/
    txd_1->wlan_idx = TxInfo->Wcid;

    if (txd_1->wlan_idx > 127) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
		goto error;
	}

    if (!TxInfo->NeedTrans)
    {
        txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
        TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0,TxInfo->WifiHdrLen, 0, txd_1->hdr_info);
        // TODO: depends on QoS to decide if need to padding
        if (TxInfo->HdrPad)
        {
            txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
        }
    }
    else
    {
        txd_1->hdr_format = TMI_HDR_FT_NON_80211;
        TMI_HDR_INFO_VAL(txd_1->hdr_format, TxInfo->MoreData,TxInfo->Eosp, 1, TxInfo->VlanFrame, TxInfo->EtherFrame,
        TxInfo->WifiHdrLen, 0, txd_1->hdr_info);
        if (TxInfo->HdrPad)
        {
            txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
        }
    }

    txd_1->tid = TxInfo->UserPriority;
    txd_1->OwnMacAddr = TxInfo->OwnMacIdx;

    if (TxInfo->LongFmt== FALSE)
    {
        txd_1->ft = TMI_FT_SHORT;
        txd_size = sizeof(TMAC_TXD_S);
        txd_7 = &txd_s->TxD7;
    }
    else
    {
        txd_2 = &txd_l->TxD2;
        txd_3 = &txd_l->TxD3;
        txd_5 = &txd_l->TxD5;
        txd_6 = &txd_l->TxD6;
        txd_7 = &txd_l->TxD7;

        txd_1->ft = TMI_FT_LONG;
        txd_size = sizeof(TMAC_TXD_L);

        pTxRadioSet = &TxInfo->TxRadioSet;
        ldpc = pTxRadioSet->Ldpc;
        mcs = pTxRadioSet->RateCode;
        sgi = pTxRadioSet->ShortGI;
        stbc = pTxRadioSet->Stbc;
        phy_mode = pTxRadioSet->PhyMode;
        bw = pTxRadioSet->CurrentPerPktBW;
        expBf = pTxRadioSet->EtxBFEnable;
        impBf = pTxRadioSet->ItxBFEnable;
		vht_nss = TxInfo->VhtNss?TxInfo->VhtNss:1;
        nss = get_nsts_by_mcs(phy_mode, mcs, stbc, vht_nss);
        /*DW2*/
        txd_2->max_tx_time = TxInfo->MaxTxTime;
        txd_2->bc_mc_pkt = TxInfo->BmcPkt;
        txd_2->fix_rate = TxInfo->FixRate;
        txd_2->frm_type = TxInfo->FrmType;
        txd_2->sub_type = TxInfo->SubType;

        if (TxInfo->NeedTrans)
        {
            txd_2->htc_vld = 0;
        }

        txd_2->frag = TxInfo->FragIdx;
        txd_2->timing_measure = TxInfo->TimingMeasure;;
        txd_2->ba_disable = TxInfo->BaDisable;
        txd_2->pwr_offset = TxInfo->PowerOffset;

        /*DW3*/
        txd_3->remain_tx_cnt = TxInfo->RemainTxCnt;
        txd_3->sn = TxInfo->Sn;
        txd_3->no_ack = ( TxInfo->bAckRequired? 0 : 1);
        txd_3->protect_frm = (TxInfo->CipherAlg != CIPHER_NONE) ? 1:0;

        /*DW5*/
        txd_5->pid = TxInfo->Pid;
        txd_5->tx_status_fmt = TxInfo->TxSFmt;
        txd_5->tx_status_2_host = TxInfo->TxS2Host;
        txd_5->tx_status_2_mcu = TxInfo->TxS2Mcu;
        if(TxInfo->NeedTrans)
        {
            txd_5->da_select = TMI_DAS_FROM_MPDU;
        }
        //txd_5->BarSsnCtrl = TxInfo->BarSsnCtrl;
        /* For  MT STA LP control, use H/W control mode for PM bit */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
        txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
#else
        txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
#endif /* CONFIG_STA_SUPPORT && CONFIG_PM_BIT_HW_MODE */

        /* DW6 */
        if (txd_2->fix_rate == 1)
        {
            txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
            // TODO: MT7615 fix me! the AntPri is 8 bits, but currently hardware "ant_id" can support up to 12 bits!!
            txd_6->ant_id = TxInfo->AntPri;
            // TODO: MT7615, fix me! how to support SpeExtEnable in MT7615??
            //txd_6->spe_en = TxInfo->SpeEn;
            txd_6->bw = ((1 << 2) |bw);
            txd_6->dyn_bw = 0;
            txd_6->TxBF = ((impBf | expBf) ? 1 : 0);
            txd_6->ldpc = ldpc;
            txd_6->gi = sgi;

            if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD)
            {
                preamble = TxInfo->TxRadioSet.Premable;
                txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
            }
        }
    }

    /* DWORD 7 */
    // TODO: MT7615 fix me! for spe_idx, ant_idx, and how to mapping to AntPri/SpeExtEnable
    txd_7->spe_idx = TxInfo->AntPri;
    txd_7->pp_ref_subtype= TxInfo->SubType;
    txd_7->pp_ref_type= TxInfo->FrmType;

    txd_0->TxByteCount= txd_size +TxInfo->PktLen;

	return NDIS_STATUS_SUCCESS;

error:
	dump_tmac_info(pAd, buf);
	return NDIS_STATUS_FAILURE;
}


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
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

	See also : BASmartHardTransmit()    !!!

	========================================================================
*/
INT write_tmac_info(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO *info,
	IN HTTRANSMIT_SETTING *pTransmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UCHAR stbc, bw, mcs, nss = 1, sgi, phy_mode, ldpc = 0, preamble = LONG_PREAMBLE;
	UCHAR to_mcu = FALSE, q_idx = info->q_idx;
	TMAC_TXD_L txd;
	TMAC_TXD_0 *txd_0 = &txd.TxD0;
	TMAC_TXD_1 *txd_1 = &txd.TxD1;
	TMAC_TXD_2 *txd_2 = &txd.TxD2;
	TMAC_TXD_3 *txd_3 = &txd.TxD3;
	TMAC_TXD_5 *txd_5 = &txd.TxD5;
	TMAC_TXD_6 *txd_6 = &txd.TxD6;
	TMAC_TXD_7 *txd_7 = &txd.TxD7;
	INT txd_size = sizeof(TMAC_TXD_S);
	STA_TR_ENTRY *tr_entry = NULL;
#ifdef CONFIG_AP_SUPPORT
    struct wifi_dev *wdev = NULL;
#endif

	if (VALID_UCAST_ENTRY_WCID(pAd, info->WCID))
		mac_entry = &pAd->MacTab.Content[info->WCID];

	NdisZeroMemory(&txd, sizeof(TMAC_TXD_L));

	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	phy_mode = pTransmit->field.MODE;
	bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);

#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_ATE
	if (!ATE_ON(pAd))
#endif
	{
		if (mac_entry && !IS_ENTRY_NONE(mac_entry))
		{
			UCHAR MaxMcs_1ss;

#ifdef DOT11_VHT_AC
			if (IS_VHT_STA(mac_entry))
				MaxMcs_1ss = 9;
			else
#endif /* DOT11_VHT_AC */
				MaxMcs_1ss = 7; 
							
			if ((pAd->CommonCfg.bMIMOPSEnable) && (mac_entry->MmpsMode == MMPS_STATIC)
				&& (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > MaxMcs_1ss))
				mcs = MaxMcs_1ss;
		}
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		info->AMPDU = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */

	/* DWORD 0 */
	txd_0->p_idx = (to_mcu ? P_IDX_MCU : P_IDX_LMAC);
	if (q_idx < WMM_QUE_NUM)
		txd_0->q_idx = dmac_wmm_aci_2_hw_ac_que[info->wmm_set][q_idx];
	else
		txd_0->q_idx = q_idx;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                        ("%s(%d): TxBlk->wmm_set= %d, QID = %d\n",
                        __FUNCTION__, __LINE__,info->wmm_set,txd_0->q_idx));


	/* DWORD 1 */
	txd_1->wlan_idx = info->WCID;

	if (txd_1->wlan_idx > 127) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
		goto error;
	}

	txd_1->ft = TMI_FT_LONG;
	txd_1->txd_len = 0;
	txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
	TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, info->hdr_len, 0, txd_1->hdr_info);
	if (info->hdr_pad)  // TODO: depends on QoS to decide if need to padding
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;


	txd_1->tid = info->TID;
	if (info->IsOffloadPkt == TRUE)
    {
        txd_1->pkt_ft = TMI_PKT_FT_MCU_FW;
    }
    else
    {
	   txd_1->pkt_ft = TMI_PKT_FT_HIF_CT; // cut-through
    }
	txd_1->OwnMacAddr = info->OmacIdx;

    /*
      repeater entry doesn't have real wdev could bind.
      so reference the OmacIdx which is stored in tr_entry.

      other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
    */
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry))
	{
	    tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
		txd_1->OwnMacAddr = tr_entry->OmacIdx;
	}
	else if (mac_entry && !IS_ENTRY_NONE(mac_entry) && !IS_ENTRY_MCAST(mac_entry))
	{
        txd_1->OwnMacAddr = mac_entry->wdev->OmacIdx;
	}

	if (txd_1->ft == TMI_FT_LONG)
	{
		txd_size = sizeof(TMAC_TXD_L);

		/* DWORD 2 */
        if (info->IsOffloadPkt == TRUE)
        {
            txd_2->sub_type = info->SubType;
            txd_2->frm_type = info->Type;
        }

		txd_2->ndp = 0;
		txd_2->ndpa = 0;
		txd_2->sounding = 0;
		txd_2->rts = 0;
		txd_2->bc_mc_pkt = info->BM;
		txd_2->bip = 0;
		txd_2->fix_rate = 1;
		txd_2->max_tx_time = 0;
		txd_2->duration = 0;
		txd_2->htc_vld = 0;
		txd_2->frag = info->FRAG; // 0: no frag, 1: 1st frag, 2: mid frag, 3: last frag
		txd_2->max_tx_time = 0;
		txd_2->pwr_offset = 0;
		txd_2->ba_disable = 1;
		txd_2->timing_measure = 0;
		if (info->IsAutoRate)
			txd_2->fix_rate = 0;
		else
			txd_2->fix_rate = 1;
		if ((pAd->pTmrCtrlStruct != NULL)
            && (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR)) {
			if ((info->Ack == 1) && (txd_2->bc_mc_pkt == 0)) {
				txd_2->timing_measure = 1;
			}
		}

	if ((pAd->pTmrCtrlStruct != NULL)
        && (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR))
    {
        if (info->IsTmr) {
            txd_2->timing_measure = 1;
        }
    }

	/* DWORD 3 */
	if (txd_0->q_idx == TxQ_IDX_BCN0 || txd_0->q_idx == TxQ_IDX_BCN1)
        {
			txd_3->remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
        }
		else
		{
				txd_3->remain_tx_cnt = MT_TX_SHORT_RETRY;
		}

		txd_3->no_ack = (info->Ack ? 0 : 1);
		if (0 /* bar_sn_ctrl */)
			txd_3->sn_vld = 1;

		/* DWORD 4 */

		/* DWORD 5 */
#if defined(MT7615) || defined(MT7622)


#ifdef WH_EZ_SETUP	
		if (mac_entry && IS_EZ_SETUP_ENABLED(mac_entry->wdev) && info->PID == PID_P2P_ACTION)
		{
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Action Frame TXD initialized\n"));
		
			txd_5->pid = info->PID;
			txd_5->tx_status_fmt = TXS_FORMAT0;
			txd_5->tx_status_2_mcu = 0;
			txd_5->tx_status_2_host = 1;
		}
#endif

#else
		txd_5->pid = info->PID;

		if (TxSCtl->TxSFormat & (1 << info->PID))
			txd_5->tx_status_fmt = TXS_FORMAT1;
		else
			txd_5->tx_status_fmt = TXS_FORMAT0;

		if (TxSCtl->TxS2McUStatus & (1 << info->PID))
			txd_5->tx_status_2_mcu = 1;
		else
			txd_5->tx_status_2_mcu = 0;

		if (TxSCtl->TxS2HostStatus	& (1 << info->PID))
			txd_5->tx_status_2_host = 1;
		else
			txd_5->tx_status_2_host = 0;
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef HDR_TRANS_TX_SUPPORT
		//txd_5->da_select = TMI_DAS_FROM_MPDU;
#endif /* HDR_TRANS_TX_SUPPORT */
		// TODO: shiang-MT7615, fix me! bar_sn_ctrl = Write SSN to SN field and set sn_vld bit to 1
		//txd_5->bar_sn_ctrl = 1;

		if (info->PsmBySw)
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
		else
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
		//txd_5->pn_high = 0;

		/* DWORD 6 */
		if (txd_2->fix_rate == 1)
		{
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			txd_6->bw = ((1 << 2) |bw);
			txd_6->dyn_bw = 0;
			txd_6->ant_id = 0; /* Smart Antenna indicator */
			txd_6->TxBF = 0;
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD)
			{
				if (phy_mode == MODE_CCK)
					preamble = info->Preamble;

				txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}


		if (info->prot == 1) {
			txd_3->protect_frm = 1;
		}
		else if (info->prot == 2)
		{
#ifdef CONFIG_AP_SUPPORT
            if (mac_entry)
            {
                wdev = mac_entry->wdev;
                GET_GroupKey_WCID(wdev, info->WCID);
            }
#ifdef DOT11W_PMF_SUPPORT
			txd_2->bip = 1;
#endif /* DOT11W_PMF_SUPPORT */
#else
            {
                MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[info->WCID];

                info->WCID = pEntry->wdev->bss_info_argument.ucBcMcWlanIdx;
            }
#endif
			txd_1->wlan_idx = info->WCID;

			if (txd_1->wlan_idx > 127) {
				pAd->wrong_wlan_idx_num++;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
				dump_stack();
				goto error;
			}
		}
		else
		{
			txd_3->protect_frm = 0;
		}
	}

	/* DWORD 7 */
	txd_7->spe_idx = info->AntPri;
    if (info->IsOffloadPkt == TRUE)
    {
        ;//Offload pkt doesn't need to fill pp_ref_type/subtype.
    }
    else
    {
        txd_7->pp_ref_subtype= info->SubType;
        txd_7->pp_ref_type= info->Type;
    }

	txd_0->TxByteCount = txd_size + info->Length; // TODO: shiang-7603, need to adjust
	//txwi_n->MPDUtotalByteCnt = Length;

	// TODO: shiang-7603, any mapping to following parameters?
	NdisMoveMemory(tmac_info, &txd, sizeof(TMAC_TXD_L));

	if (0 /*(DebugSubCategory[9] & CATTX_TMAC) == CATTX_TMAC*/) {
		dump_tmac_info(pAd, tmac_info);
	}

	return NDIS_STATUS_SUCCESS;

error:
	dump_tmac_info(pAd, tmac_info);
	return NDIS_STATUS_FAILURE;
}


VOID write_tmac_info_ct(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_l->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_l->TxD1;

	txd_0->p_idx = P_IDX_LMAC;

	txd_0->q_idx = 0;

	txd_0->TxByteCount = sizeof(TMAC_TXD_L) +
						pTxBlk->MpduHeaderLen +
						pTxBlk->HdrPadLen +
						pTxBlk->SrcBufLen;

	txd_1->ft = TMI_FT_LONG;

	txd_1->txd_len = 0;

	txd_1->pkt_ft = TMI_PKT_FT_HIF_CT;
	txd_1->hdr_format = TMI_HDR_FT_NON_80211;
	TMI_HDR_INFO_VAL(TMI_HDR_FT_NON_80211, 0, 0, 0, 0, 0, 0, 0, txd_1->hdr_info);

	if (pTxBlk->HdrPadLen)
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
}


INT write_tmac_info_Data(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	UCHAR wcid;
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	TMAC_TXD_7 *txd_7 = &txd_s->TxD7;
	UCHAR txd_size;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
    STA_TR_ENTRY *tr_entry = NULL;
	TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
	TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
	TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
	TMAC_TXD_6 *txd_6 = &txd_l->TxD6;
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;

#if !defined(MT7615) && !defined(MT7622)
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
	struct wtbl_entry tb_entry;
#endif /* !defined(MT7615) || !defined(MT7622) */
    struct wifi_dev *wdev = NULL;
#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    UCHAR cloned_wcid;
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */


		wcid = pTxBlk->Wcid;

	if (pMacEntry)
	{
		//printk("##### MacEntry Aid is %d ##### \n", pMacEntry->Aid);
		wcid = pMacEntry->wcid;
        wdev = pMacEntry->wdev;
	}

#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    if (pAd->CommonCfg.dbdc_mode)
        cloned_wcid = 2;
    else
        cloned_wcid = 1;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, pTxBlk->Wcid %d\n", __FUNCTION__, wcid, pTxBlk->Wcid));

    if ((pTxBlk->Wcid > cloned_wcid) && (pTxBlk->Wcid <= pAd->vow_cloned_wtbl_max))
        wcid = pTxBlk->Wcid;
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */


	NdisZeroMemory(txd_l, sizeof(TMAC_TXD_L));

	txd_0 = &txd_s->TxD0;
	txd_1 = &txd_s->TxD1;

	/* DWORD 0 */
	txd_0->p_idx = P_IDX_LMAC;
#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    //if ((pTxBlk->Wcid > 2) && (pTxBlk->Wcid <= pAd->vow_cloned_wtbl_max))
    if ((pTxBlk->Wcid > 0) && (pTxBlk->Wcid <= pAd->vow_cloned_wtbl_max))
    {
        pTxBlk->wmm_set = pAd->vow_sta_wmm[wcid];
        pTxBlk->QueIdx = pAd->vow_sta_ac[wcid];
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wmm set %d, QueIdx %d\n", __FUNCTION__, pTxBlk->wmm_set, pTxBlk->QueIdx));
    }
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA) )*/

	if (pTxBlk->QueIdx < 4) {
		txd_0->q_idx = dmac_wmm_aci_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];
	} else {
		// TODO: shiang-usw, consider about MCC case!
		txd_0->q_idx = pTxBlk->QueIdx;
	}

#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    if (pTxBlk->wmm_set == 0) //the same band
    {
        if (pAd->vow_bcmc_en == 1)
            txd_0->q_idx = TxQ_IDX_BMC0;
        else if (pAd->vow_bcmc_en == 2)
            txd_0->q_idx = TxQ_IDX_BMC1;
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): TxBlk->wmm_set= %d, QID = %d\n", __FUNCTION__, __LINE__,pTxBlk->wmm_set,txd_0->q_idx));
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */

	txd_1->wlan_idx = wcid;
	
	if (txd_1->wlan_idx > 127) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
		goto error;
	}



	txd_1->pkt_ft = TMI_PKT_FT_HIF_CT; // cut-through
	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);
		if (pTxBlk->HdrPadLen)  // TODO: depends on QoS to decide if need to padding
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;

		txd_1->tid = pTxBlk->UserPriority;
	} else {
		txd_1->hdr_format = TMI_HDR_FT_NON_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format,
							TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData),
							TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP),
							1,
							RTMP_GET_PACKET_VLAN(pTxBlk->pPacket),
							(RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket) <= 1500) ? 0 : 1,
							pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);
		if (pTxBlk->HdrPadLen)
		{
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
		}
	}


    /*
      repeater entry doesn't have real wdev could bind.
      so reference the OmacIdx which is stored in tr_entry.

      other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
    */
	if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry)) {
        if (pTxBlk->tr_entry) {
            tr_entry = pTxBlk->tr_entry;
            txd_1->OwnMacAddr = tr_entry->OmacIdx;
        }
	}
    else if (pMacEntry ) {
        txd_1->OwnMacAddr = wdev->OmacIdx;

#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    if (pAd->vow_sta_mbss[wcid])
    {
        txd_1->OwnMacAddr = pAd->vow_sta_mbss[wcid]+16;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __FUNCTION__, wcid, txd_1->OwnMacAddr));
    }
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __FUNCTION__, wcid, txd_1->OwnMacAddr));
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */

    }
	else
	{
#ifdef USE_BMC
		if (pTxBlk->wdev_idx == 0)
			txd_1->OwnMacAddr = 0x0;
		else if (pTxBlk->wdev_idx >= 1 && pTxBlk->wdev_idx <= 15)
			txd_1->OwnMacAddr = 0x10 | pTxBlk->wdev_idx;
#else
		wdev = pAd->wdev_list[pTxBlk->wdev_idx];
		txd_1->OwnMacAddr = wdev->OmacIdx;
#endif /* USE_BMC */
	}


	txd_7 = &txd_l->TxD7;

	txd_1->ft = TMI_FT_LONG;
	txd_size = sizeof(TMAC_TXD_L);

	if (pTransmit) {
		ldpc = pTransmit->field.ldpc;
		mcs = pTransmit->field.MCS;
		mcs = phy_mode == MODE_VHT ? pTransmit->field.MCS & 0xf : pTransmit->field.MCS;
		sgi = pTransmit->field.ShortGI;
		stbc = pTransmit->field.STBC;
		phy_mode = pTransmit->field.MODE;
		bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
		nss = get_nsts_by_mcs(phy_mode, mcs, stbc,
				phy_mode == MODE_VHT ? ((pTransmit->field.MCS & (0x3 << 4)) >> 4) + 1 : 0);
	}

	txd_l->TxD2.max_tx_time = 0;
#if defined(VOW_SUPPORT) && defined(_FPGA)
	txd_l->TxD2.max_tx_time = pAd->vow_life_time;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, lifetime %d\n", __FUNCTION__, wcid, txd_l->TxD2.max_tx_time));
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */
	txd_l->TxD2.bc_mc_pkt = (pTxBlk->TxFrameType == TX_MCAST_FRAME ? 1 : 0);

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate))
		txd_l->TxD2.fix_rate = 1;

	txd_l->TxD2.frag = pTxBlk->FragIdx;

	txd_3->no_ack = (TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired) ? 0 : 1);

#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    txd_3->no_ack = (pAd->vow_sta_ack[wcid] ? 1 : 0);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, no ack %d\n", __FUNCTION__, wcid, txd_3->no_ack));
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */

	if (pAd->chipCap.TmrEnable == 1) {
		if (txd_3->no_ack == 0)
			txd_l->TxD2.timing_measure = 1;
	}

	if (IS_CIPHER_NONE(pTxBlk->CipherAlg))
		txd_3->protect_frm = 0;
	else
		txd_3->protect_frm = 1;
	txd_l->TxD3.remain_tx_cnt = MT_TX_SHORT_RETRY;

	//Carter, this shall apply to all unify mac. 2014-04-11
#if !defined(MT7615) && !defined(MT7622)
	// TODO: shiang-MT7615, why??
	if (txd_0->q_idx == QID_BMC)
		txd_l->TxD3.remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
#endif /* !defined(MT7615) && !defined(MT7622) */

	txd_l->TxD5.pid = pTxBlk->Pid;

	if (pTxBlk->Pid)
	{
		if (TxSCtl->TxSFormatPerPkt & (1 << pTxBlk->Pid))
			txd_l->TxD5.tx_status_fmt = TXS_FORMAT1;
		else
			txd_l->TxD5.tx_status_fmt = TXS_FORMAT0;

		if (TxSCtl->TxS2McUStatusPerPkt & (1 << pTxBlk->Pid))
			txd_l->TxD5.tx_status_2_mcu = 1;
		else
			txd_l->TxD5.tx_status_2_mcu = 0;

		if (TxSCtl->TxS2HostStatusPerPkt & (1 << pTxBlk->Pid)) {
			txd_l->TxD5.tx_status_2_host = 1;
			txd_l->TxD5.pid = AddTxSStatus(pAd, TXS_TYPE0, pTxBlk->Pid, 0, 0,
								0, pTxBlk->TxSPriv);
		}
		else
			txd_l->TxD5.tx_status_2_host = 0;
	}
	else
	{
		ULONG TxSStatusPerWlanIdx;

		// TODO: shiang-MT7615, why 64??
		if (wcid < 64)
			TxSStatusPerWlanIdx = TxSCtl->TxSStatusPerWlanIdx[0];
		else
			TxSStatusPerWlanIdx = TxSCtl->TxSStatusPerWlanIdx[1];

		if (TxSStatusPerWlanIdx & (1 << wcid))
		{
            if (TxSCtl->TxSFormatPerPktType[pTxBlk->dot11_type] & (1 << pTxBlk->dot11_subtype))
                txd_l->TxD5.tx_status_fmt = TXS_FORMAT1;
            else
                txd_l->TxD5.tx_status_fmt = TXS_FORMAT0;

			if (TxSCtl->TxS2McUStatusPerPktType[pTxBlk->dot11_type] & (1 << pTxBlk->dot11_subtype))
                txd_l->TxD5.tx_status_2_mcu = 1;
            else
                txd_l->TxD5.tx_status_2_mcu = 0;

			if (TxSCtl->TxS2HostStatusPerPktType[pTxBlk->dot11_type] & (1 << pTxBlk->dot11_subtype))
			{
				txd_l->TxD5.pid = AddTxSStatus(pAd, TXS_TYPE1, 0,
								pTxBlk->dot11_type, pTxBlk->dot11_subtype, 0, pTxBlk->TxSPriv);
                txd_l->TxD5.tx_status_2_host = 1;
			}
            else
			{
                txd_l->TxD5.tx_status_2_host = 0;
			}
		}
	}

	if(TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
	{
		txd_5->da_select = TMI_DAS_FROM_MPDU;
	}

	// TODO: shiang-MT7615, fix me! bar_sn_ctrl = Write SSN to SN field and set sn_vld bit to 1
	if (0 /* bar_sn_ctrl */)
		txd_3->sn_vld = 1;
	txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;

	/* DWORD 6 */
	if (txd_2->fix_rate == 1) {
		txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
		txd_6->bw = ((1 << 2) |bw);
		txd_6->dyn_bw = 0;
		txd_6->TxBF = 0;
		txd_6->ldpc = ldpc;
		txd_6->gi = sgi;

		if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD)
		{
			if (phy_mode == MODE_CCK)
			{
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
					preamble = SHORT_PREAMBLE;
				else
					preamble = LONG_PREAMBLE;
			}

			txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
		}
	}

	/* DWORD 7 */
	txd_7->pp_ref_subtype= pTxBlk->dot11_subtype;
	txd_7->pp_ref_type= pTxBlk->dot11_type;
#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
    txd_7->sw_tx_time = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: sw txtime %d\x1b[m\n", __FUNCTION__, txd_7->sw_tx_time));
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))*/


	txd_0->TxByteCount = txd_size +
						pTxBlk->MpduHeaderLen +
						pTxBlk->HdrPadLen +
						pTxBlk->SrcBufLen; // TODO: shiang-7603, need to adjust

//printk("%s(): txd_size=%d, MpduHeaderLen=%d, HdrPadLen=%d, SrcBufLen=%d, TxByteCount=0x%x\n",
//		__FUNCTION__, txd_size, pTxBlk->MpduHeaderLen, pTxBlk->HdrPadLen, pTxBlk->SrcBufLen, txd_0->TxByteCount);

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
		UCHAR mcs = pTransmit->field.MCS;

		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		if (pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) {
			if (mcs < MAX_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
		}
#ifdef DOT11_VHT_AC
		else if (pTransmit->field.MODE == MODE_VHT) {
			INT mcs_idx = ((mcs >> 4) * 10) +  (mcs & 0xf);
			if (mcs_idx < MAX_VHT_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_VHT[mcs_idx]++;
		}
#endif /* DOT11_VHT_AC */
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

	return NDIS_STATUS_SUCCESS;

error:
	dump_tmac_info(pAd, buf);
	return NDIS_STATUS_FAILURE;
}


VOID write_tmac_info_Cache(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	// TODO: shiang-7603, for now, go following function first;
	write_tmac_info_Data(pAd, buf, pTxBlk);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not Finish yet for HT_MT!!!!\n", __FUNCTION__, __LINE__));
	return;
}


VOID mt_write_tmac_info_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	mac_info.Type = FC_TYPE_MGMT;
	mac_info.SubType = SUBTYPE_BEACON;
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = 0;
	mac_info.Length = frmLen;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = wdev->func_idx;
	mac_info.SpeEn = 1;
	mac_info.q_idx = HcGetBcnQueueIdx(pAd,wdev);
#ifdef MT_MAC
	mac_info.TxSPriv = wdev->func_idx;
	mac_info.OmacIdx = wdev->OmacIdx;
    if (wdev->bcn_buf.BcnUpdateMethod == BCN_GEN_BY_FW)
    {
        mac_info.IsOffloadPkt = TRUE;
    }
    else
    {
        mac_info.IsOffloadPkt = FALSE;
    }
#endif
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
	write_tmac_info(pAd, tmac_buf, &mac_info, BeaconTransmit);
#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			MTMacInfoEndianChange(pAd, tmac_buf, TYPE_TXWI, sizeof(TMAC_TXD_L));
		}
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		{
			RTMPWIEndianChange(pAd, tmac_buf, TYPE_TXWI);
		}
#endif
#endif
}


INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
						__FUNCTION__, __LINE__));
	return FALSE;
}


INT mt_mac_set_ctrlch(RTMP_ADAPTER *pAd, UINT8 extch)
{
	// TODO: shiang-7603
	return FALSE;
}


#ifdef GREENAP_SUPPORT
INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
					__FUNCTION__, __LINE__));
	return FALSE;
}
#endif /* GREENAP_SUPPORT */


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
					__FUNCTION__, __LINE__));
}


#ifdef RTMP_MAC_PCI
static INT mt_asic_cfg_hif_tx_ring(RTMP_ADAPTER *pAd, RTMP_TX_RING *ring, UINT32 offset, UINT32 phy_addr, UINT32 cnt)
{
	ring->TxSwFreeIdx = 0;
	ring->TxCpuIdx = 0;
	ring->TxDMADoneCnt = 0;
	ring->TxDMADoneCntResetMark = 1;

	ring->hw_desc_base = MT_TX_RING_BASE + offset;
	ring->hw_cnt_addr = ring->hw_desc_base + 0x04;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x08;
	ring->hw_didx_addr = ring->hw_desc_base + 0x0c;

	HIF_IO_WRITE32(pAd, ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd, ring->hw_cidx_addr, ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd, ring->hw_cnt_addr, TX_RING_SIZE);

	return TRUE;
}


VOID mt_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	UINT32 phy_addr, offset;
	INT i, TxHwRingNum;
	struct rx_delay_control *rx_delay_ctl = &pAd->tr_ctl.rx_delay_ctl;

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);

	AsicWaitPDMAIdle(pAd, 100, 1000);

	{
		// TODO: shiang-usw, move this to other place!
		HIF_IO_WRITE32(pAd, MT_DELAY_INT_CFG, RX_DLY_INT_CFG);
		rx_delay_ctl->en = TRUE;
	}
	/* Reset DMA Index */
	HIF_IO_WRITE32(pAd, WPDMA_RST_PTR, 0xFFFFFFFF);

	/*
		Write Tx Ring base address registers

		The Tx Ring arrangement:
		RingIdx	SwRingIdx	AsicPriority	WMM QID		LMAC QID	MCU QID
		0 		band0_TxSw0		L			-			-
		1		band1_TxSw0		L			-			-
		2		CMD		L		H			-
		3		FW_DOWNLOAD	H			-			-
	*/
	TxHwRingNum = NUM_OF_TX_RING;
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		TxHwRingNum = 2;
#endif /* defined(MT7615) || defined(MT7622) */

	for (i = 0; i < TxHwRingNum; i++)
	{
#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			offset = i * 0x10;
		} else
#endif /* defined(MT7615) || defined(MT7622) */
		if (i == QID_AC_BE)
			offset = 0x10;
		else if (i == QID_AC_BK)
			offset = 0;
		else
			offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(pAd->PciHif.TxRing[i].Cell[0].AllocPa);
		mt_asic_cfg_hif_tx_ring(pAd, &pAd->PciHif.TxRing[i], offset, phy_addr, TX_RING_SIZE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
					i, pAd->PciHif.TxRing[i].hw_desc_base, phy_addr, TX_RING_SIZE));
	}

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		// TODO: shiang-MT7615, we don't need bcn ring now!
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	{
		RTMP_MGMT_RING *mgmt_ring;
		RTMP_TX_RING *bmc_ring;
		RTMP_BCN_RING *bcn_ring;

		/* init BMC ring */
		bmc_ring = &pAd->TxBmcRing;
		offset = QID_BMC * MT_RINGREG_DIFF;
		phy_addr = RTMP_GetPhysicalAddressLow(bmc_ring->Cell[0].AllocPa);
		mt_asic_cfg_hif_tx_ring(pAd, bmc_ring, offset, phy_addr, TX_RING_SIZE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("-->TX_BMC_RING [0x%x]: Base=0x%x, Cnt=%d!\n",
					bmc_ring->hw_desc_base, phy_addr, TX_RING_SIZE));

		/* init MGMT ring Base/Size/Index pointer CSR */
		mgmt_ring = &pAd->MgmtRing;
		phy_addr = RTMP_GetPhysicalAddressLow(mgmt_ring->Cell[0].AllocPa);
		offset = MT_RINGREG_DIFF * 4;
		mgmt_ring->TxSwFreeIdx = 0;
		mgmt_ring->TxCpuIdx = 0;
		mgmt_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
		mgmt_ring->hw_cnt_addr = (mgmt_ring->hw_desc_base + 0x04);
		mgmt_ring->hw_cidx_addr = (mgmt_ring->hw_desc_base + 0x08);
		mgmt_ring->hw_didx_addr = (mgmt_ring->hw_desc_base + 0x0c);
		HIF_IO_WRITE32(pAd, mgmt_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, mgmt_ring->hw_cidx_addr, mgmt_ring->TxCpuIdx);
		HIF_IO_WRITE32(pAd, mgmt_ring->hw_cnt_addr, MGMT_RING_SIZE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("-->TX_RING_MGMT[0x%x]: Base=0x%x, Cnt=%d!\n",
					mgmt_ring->hw_desc_base, phy_addr, MGMT_RING_SIZE));

		/* init BCN ring index pointer */
		bcn_ring = &pAd->BcnRing;
		phy_addr = RTMP_GetPhysicalAddressLow(bcn_ring->Cell[0].AllocPa);
		bcn_ring->TxSwFreeIdx = 0;
		bcn_ring->TxCpuIdx = 0;

		bcn_ring->hw_desc_base = (MT_TX_RING_BASE  + MT_RINGREG_DIFF * MT_TX_RING_BCN_IDX);
		bcn_ring->hw_cnt_addr = (bcn_ring->hw_desc_base + 0x4);
		bcn_ring->hw_cidx_addr = (bcn_ring->hw_desc_base + 0x8);
		bcn_ring->hw_didx_addr = (bcn_ring->hw_desc_base + 0xc);
		HIF_IO_WRITE32(pAd, bcn_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, bcn_ring->hw_cidx_addr,  bcn_ring->TxCpuIdx);
		HIF_IO_WRITE32(pAd, bcn_ring->hw_cnt_addr, BCN_RING_SIZE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_BCN: Base=0x%x, Cnt=%d!\n",
					phy_addr, BCN_RING_SIZE));
	}

#ifdef CONFIG_ANDES_SUPPORT
	{
		RTMP_CTRL_RING *ctrl_ring;

		/* init CTRL ring index pointer */
		ctrl_ring = &pAd->CtrlRing;
		phy_addr = RTMP_GetPhysicalAddressLow(ctrl_ring->Cell[0].AllocPa);
		offset = MT_RINGREG_DIFF * 5;
#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
			offset = MT_RINGREG_DIFF * 2;
#endif /* defined(MT7615) || defined(MT7622) */

		ctrl_ring->TxSwFreeIdx = 0;
		ctrl_ring->TxCpuIdx = 0;
		ctrl_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
		ctrl_ring->hw_cnt_addr = (ctrl_ring->hw_desc_base + 0x04);
		ctrl_ring->hw_cidx_addr = (ctrl_ring->hw_desc_base + 0x08);
		ctrl_ring->hw_didx_addr = (ctrl_ring->hw_desc_base + 0x0c);

		HIF_IO_WRITE32(pAd, ctrl_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, ctrl_ring->hw_cidx_addr, ctrl_ring->TxCpuIdx);
		HIF_IO_WRITE32(pAd, ctrl_ring->hw_cnt_addr, MGMT_RING_SIZE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_CTRL: Base=0x%x, Cnt=%d!\n",
					phy_addr, MGMT_RING_SIZE));
	}

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		/* Firmware download only ring */
		// TODO: shiang-MT7615
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): TODO: TxRing for firmware download is not init yet!\n", __FUNCTION__));
		RTMP_FWDWLO_RING *FwDwlo_ring;

		/* init Firmware download ring index pointer */
		FwDwlo_ring = &pAd->FwDwloRing;
		phy_addr = RTMP_GetPhysicalAddressLow(FwDwlo_ring->Cell[0].AllocPa);
		offset = MT_RINGREG_DIFF * 3;

		FwDwlo_ring->TxSwFreeIdx = 0;
		FwDwlo_ring->TxCpuIdx = 0;
		FwDwlo_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
		FwDwlo_ring->hw_cnt_addr = (FwDwlo_ring->hw_desc_base + 0x04);
		FwDwlo_ring->hw_cidx_addr = (FwDwlo_ring->hw_desc_base + 0x08);
		FwDwlo_ring->hw_didx_addr = (FwDwlo_ring->hw_desc_base + 0x0c);

		HIF_IO_WRITE32(pAd, FwDwlo_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, FwDwlo_ring->hw_cidx_addr, FwDwlo_ring->TxCpuIdx);
		HIF_IO_WRITE32(pAd, FwDwlo_ring->hw_cnt_addr, MGMT_RING_SIZE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("-->TX_RING_FwDwlo: Base=0x%x, Cnt=%d!\n",
					phy_addr, MGMT_RING_SIZE));
	}
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	for (i = 0; i < NUM_OF_RX_RING; i++)
	{
		UINT16 RxRingSize = (i == 0) ? RX_RING_SIZE : RX1_RING_SIZE;

		RTMP_RX_RING *rx_ring;

		rx_ring = &pAd->PciHif.RxRing[i];
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(rx_ring->Cell[0].AllocPa);
		rx_ring->RxSwReadIdx = 0;
		rx_ring->RxCpuIdx = RxRingSize - 1;
		rx_ring->hw_desc_base = MT_RX_RING_BASE + offset;
		rx_ring->hw_cidx_addr = MT_RX_RING_CIDX + offset;
		rx_ring->hw_didx_addr = MT_RX_RING_DIDX + offset;
		rx_ring->hw_cnt_addr = MT_RX_RING_CNT + offset;
		HIF_IO_WRITE32(pAd, rx_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		HIF_IO_WRITE32(pAd, rx_ring->hw_cnt_addr, RxRingSize);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->RX_RING%d[0x%x]: Base=0x%x, Cnt=%d\n",
					i, rx_ring->hw_desc_base, phy_addr, RxRingSize));
	}
}
#endif /* RTMP_MAC_PCI */

INT mt_mac_pse_init(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x82000000);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Don't Support this now!\n", __FUNCTION__, __LINE__));
		return FALSE;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	return TRUE;
}


#define HW_TX_RATE_TO_MODE(_x)			(((_x) & (0x7 << 6)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)				(((_x) & (0x3 << 9)) >> 9)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & (0x1 << 11)) >> 11)

#define MAX_TX_MODE 5
static char *HW_TX_MODE_STR[]={"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2M", "5.5M", "11M", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(UINT16 ofdm_idx)
{
	switch (ofdm_idx)
	{
		case 11: // 6M
			return HW_TX_RATE_OFDM_STR[0];
		case 15: // 9M
			return HW_TX_RATE_OFDM_STR[1];
		case 10: // 12M
			return HW_TX_RATE_OFDM_STR[2];
		case 14: // 18M
			return HW_TX_RATE_OFDM_STR[3];
		case 9: // 24M
			return HW_TX_RATE_OFDM_STR[4];
		case 13: // 36M
			return HW_TX_RATE_OFDM_STR[5];
		case 8: // 48M
			return HW_TX_RATE_OFDM_STR[6];
		case 12: // 54M
			return HW_TX_RATE_OFDM_STR[7];
		default:
			return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(UINT8 mode, UINT16 rate_idx)
{
	if (mode == 0)
		return rate_idx < 4 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[4];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}


VOID dump_wtbl_basic_info(RTMP_ADAPTER *pAd, struct wtbl_struc *tb)
{
	struct wtbl_basic_info *basic_info = &tb->peer_basic_info;
	struct wtbl_tx_rx_cap *trx_cap = &tb->trx_cap;
	struct wtbl_rate_tb *rate_info = &tb->auto_rate_tb;
	UCHAR addr[MAC_ADDR_LEN];
	UINT16 txrate[8], rate_idx, txmode, mcs, nss, stbc;

	NdisMoveMemory(&addr[0], (UCHAR *)basic_info, 4);
	addr[0] = basic_info->wtbl_d1.field.addr_0 & 0xff;
	addr[1] = ((basic_info->wtbl_d1.field.addr_0 & 0xff00) >> 8);
	addr[2] = ((basic_info->wtbl_d1.field.addr_0 & 0xff0000) >> 16);
	addr[3] = ((basic_info->wtbl_d1.field.addr_0 & 0xff000000) >> 24);
	addr[4] = basic_info->wtbl_d0.field.addr_4 & 0xff;
	addr[5] = basic_info->wtbl_d0.field.addr_5 & 0xff;

	hex_dump("WTBL Raw Data", (UCHAR *)tb, sizeof(struct wtbl_struc));

	// Basic Info (DW0~DW1)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Basic Info(DW0~DW1):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
				PRINT_MAC(addr)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tMUAR_Idx(D0[B16~21]):%d\n",
				basic_info->wtbl_d0.field.muar_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\trc_a1/rc_a2:%d/%d(D0[B22]/D0[B29])\n",
				basic_info->wtbl_d0.field.rc_a1, basic_info->wtbl_d0.field.rc_a2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tKID:%d/RCID:%d/RKV:%d/RV:%d/IKV:%d/WPI_FLAG:%d(D0[B23~24], D0[B25], D0[B26], D0[B28], D0[B30])\n",
				basic_info->wtbl_d0.field.kid, basic_info->wtbl_d0.field.rc_id,
				basic_info->wtbl_d0.field.rkv, basic_info->wtbl_d0.field.rv,
				basic_info->wtbl_d0.field.ikv, basic_info->wtbl_d0.field.wpi_flg));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tGID_SU:%d(D0[B31])\n", basic_info->wtbl_d0.field.gid_su));

	// TRX Cap(DW2~5)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("TRX Cap(DW2~5):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tsw/DIS_RHTR:%d/%d\n",
                            trx_cap->wtbl_d2.field.SW, trx_cap->wtbl_d2.field.dis_rhtr));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tHT/VHT/HT-LDPC/VHT-LDPC/DYN_BW/MMSS:%d/%d/%d/%d/%d/%d\n",
                            trx_cap->wtbl_d2.field.ht, trx_cap->wtbl_d2.field.vht,
                            trx_cap->wtbl_d5.field.ldpc, trx_cap->wtbl_d5.field.ldpc_vht,
                            trx_cap->wtbl_d5.field.dyn_bw, trx_cap->wtbl_d5.field.mm));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tTx Power Offset:0x%x(%d)\n",
                            trx_cap->wtbl_d5.field.txpwr_offset, ((trx_cap->wtbl_d5.field.txpwr_offset == 0x0) ? 0:(trx_cap->wtbl_d5.field.txpwr_offset - 0x20))));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tFCAP/G2/G4/G8/G16/CBRN:%d/%d/%d/%d/%d/%d\n",
                            trx_cap->wtbl_d5.field.fcap, trx_cap->wtbl_d5.field.g2,
                            trx_cap->wtbl_d5.field.g4, trx_cap->wtbl_d5.field.g8,
                            trx_cap->wtbl_d5.field.g16, trx_cap->wtbl_d5.field.cbrn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tHT-TxBF(tibf/tebf):%d/%d, VHT-TxBF(tibf/tebf):%d/%d, PFMU_IDX=%d\n",
                            trx_cap->wtbl_d2.field.tibf, trx_cap->wtbl_d2.field.tebf,
                            trx_cap->wtbl_d2.field.tibf_vht, trx_cap->wtbl_d2.field.tebf_vht,
                            trx_cap->wtbl_d2.field.pfmu_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tSPE_IDX=%d\n", trx_cap->wtbl_d3.field.spe_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tBA Enable:0x%x, BAFail Enable:%d\n",
                            trx_cap->wtbl_d4.field.ba_en, trx_cap->wtbl_d3.field.baf_en));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("\tQoS Enable:%d\n",
                            trx_cap->wtbl_d5.field.qos));

	if (trx_cap->wtbl_d4.field.ba_en)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\tBA WinSize: TID 0 - %d, TID 1 - %d\n",
					trx_cap->wtbl_d4.field.ba_win_size_tid_0,
					trx_cap->wtbl_d4.field.ba_win_size_tid_1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\tBA WinSize: TID 2 - %d, TID 3 - %d\n",
					trx_cap->wtbl_d4.field.ba_win_size_tid_2,
					trx_cap->wtbl_d4.field.ba_win_size_tid_3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\tBA WinSize: TID 4 - %d, TID 5 - %d\n",
					trx_cap->wtbl_d4.field.ba_win_size_tid_4,
					trx_cap->wtbl_d4.field.ba_win_size_tid_5));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\tBA WinSize: TID 6 - %d, TID 7 - %d\n",
					trx_cap->wtbl_d4.field.ba_win_size_tid_6,
					trx_cap->wtbl_d4.field.ba_win_size_tid_7));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpartial_aid:%d\n", trx_cap->wtbl_d2.field.partial_aid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twpi_even:%d\n", trx_cap->wtbl_d2.field.wpi_even));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAAD_OM/CipherSuit:%d/%d\n",
						trx_cap->wtbl_d2.field.AAD_OM, trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\taf:%d\n", trx_cap->wtbl_d3.field.af));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trdg_ba:%d/rdg capability:%d\n", trx_cap->wtbl_d3.field.rdg_ba, trx_cap->wtbl_d3.field.r));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tcipher_suit:%d\n", trx_cap->wtbl_d2.field.cipher_suit));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFromDS:%d\n", trx_cap->wtbl_d5.field.fd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tToDS:%d\n", trx_cap->wtbl_d5.field.td));

	// Rate Info (DW6~8)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rate Info (DW6~8):\n"));
	txrate[0] = (rate_info->wtbl_d6.word & 0xfff);
	txrate[1] = (rate_info->wtbl_d6.word & (0xfff << 12)) >> 12;
	txrate[2] = ((rate_info->wtbl_d6.word & (0xff<<24))>>24) | ((rate_info->wtbl_d7.word & 0xf) << 8);
	txrate[3] = ((rate_info->wtbl_d7.word & (0xfff << 4)) >>4);
	txrate[4] = ((rate_info->wtbl_d7.word & (0xfff << 16)) >> 16);
	txrate[5] = ((rate_info->wtbl_d7.word & (0xf <<28)) >> 28) | ((rate_info->wtbl_d8.word & (0xff)) << 4);
	txrate[6] = ((rate_info->wtbl_d8.word & (0xfff << 8)) >> 8);
	txrate[7] = ((rate_info->wtbl_d8.word & (0xfff << 20)) >> 20);
	for (rate_idx = 0; rate_idx < 8; rate_idx++) {
		txmode = HW_TX_RATE_TO_MODE(txrate[rate_idx]);
		mcs = HW_TX_RATE_TO_MCS(txrate[rate_idx], txmode);
		nss = HW_TX_RATE_TO_NSS(txrate[rate_idx]);
		stbc = HW_TX_RATE_TO_STBC(txrate[rate_idx]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
				rate_idx+1, txrate[rate_idx],
				txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
					mcs, hw_rate_str(txmode, mcs), nss, stbc));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpsm:%d\n", trx_cap->wtbl_d3.field.psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tskip_tx:%d\n", trx_cap->wtbl_d3.field.skip_tx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdu_i_psm:%d\n", trx_cap->wtbl_d3.field.du_i_psm));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ti_psm:%d\n", trx_cap->wtbl_d3.field.i_psm));

}



VOID dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL Basic Info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMemBaseAddr:0x%x\n",
				pAd->mac_ctrl.wtbl_base_addr[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEntrySize/Cnt:%d/%d\n",
				pAd->mac_ctrl.wtbl_entry_size[0],
				pAd->mac_ctrl.wtbl_entry_cnt[0]));
}


VOID dump_wtbl_info(RTMP_ADAPTER *pAd, UINT wtbl_idx)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

	if (wtbl_idx == RESERVED_WCID)
	{
		start_idx = 0;
		end_idx = (pAd->chipCap.WtblHwNum - 1);
	}
	else if (wtbl_idx < pAd->chipCap.WtblHwNum)
	{
		start_idx = end_idx = wtbl_idx;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():Invalid WTBL index(%d)!\n",
					__FUNCTION__, wtbl_idx));
		return;
	}

	wtbl_len = sizeof(WTBL_STRUC);
	os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);
	if (!wtbl_raw_dw)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():AllocMem fail(%d)!\n",
					__FUNCTION__, wtbl_idx));
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++)
	{
		wtbl_ent.wtbl_idx = idx;
		wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];
		/* Read WTBL Entries */
		for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
			addr = wtbl_ent.wtbl_addr + wtbl_offset;
			HW_IO_READ32(pAd, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
		}
		NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));
		dump_wtbl_entry(pAd, &wtbl_ent);
	}
	os_free_mem(wtbl_raw_dw);

}


VOID dump_wtbl_entry(RTMP_ADAPTER *pAd, struct wtbl_entry *ent)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL SW Entry[%d] info\n", ent->wtbl_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tAddr=0x%x\n", ent->wtbl_addr));

	dump_wtbl_basic_info(pAd, &ent->wtbl);
}


INT mt_wtbl_get_entry234(RTMP_ADAPTER *pAd, UCHAR widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UINT8 wtbl_idx;

	wtbl_ctrl = &pAd->mac_ctrl;
	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
	{
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():pAd->mac_ctrl not init yet!\n", __FUNCTION__));
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;
	ent->wtbl_addr = wtbl_ctrl->wtbl_base_addr[0] +
						wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];

	return TRUE;
}

INT mt_wtbl_init_ByFw(struct _RTMP_ADAPTER *pAd)
{
	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = (UINT8)pAd->chipCap.WtblHwNum;

	return TRUE;
}

INT mt_wtbl_init_ByDriver(RTMP_ADAPTER *pAd)
{
	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = (UINT8)pAd->chipCap.WtblHwNum;

	return TRUE;
}

INT mt_wtbl_init(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_WTBL_TLV_MODE
	/* We can use mt_wtbl_init_ByWtblTlv() to replace mt_wtbl_init_ByDriver() when there are no */
	/* function using mt_wtbl_get_entry234 and pAd->mac_ctrl */
	mt_wtbl_init_ByFw(pAd);
#else
	mt_wtbl_init_ByDriver(pAd);
#endif /* CONFIG_WTBL_TLV_MODE */
	return TRUE;
}
#define MCAST_WCID_TO_REMOVE 0
INT mt_hw_tb_init(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	// TODO: shiang-7603
	mt_wtbl_init(pAd);

	/* Create default entry for rx packets which A2 is not in our table */
    pAd->MgmtWlanIdx = 0;
	RTMP_STA_ENTRY_ADD(pAd, pAd->MgmtWlanIdx, BROADCAST_ADDR, TRUE, TRUE);


#if defined(RLT_MAC) || defined(RTMP_MAC)
	/*
		ASIC will keep garbage value after boot
		Clear all shared key table when initial
		This routine can be ignored in radio-ON/OFF operation.
	*/
	if (bHardReset)
	{
		UINT16 KeyIdx;
		UINT32 wcid_attr_base = 0, wcid_attr_size = 0, share_key_mode_base = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
			wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
			share_key_mode_base = RLT_SHARED_KEY_MODE_BASE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
		{
			wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = HW_WCID_ATTRI_SIZE;
			share_key_mode_base = SHARED_KEY_MODE_BASE;
		}
#endif /* RTMP_MAC */

		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			HW_IO_WRITE32(pAd, share_key_mode_base + 4*KeyIdx, 0);
		}

		/* Clear all pairwise key table when initial*/
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			HW_IO_WRITE32(pAd, wcid_attr_base + (KeyIdx * wcid_attr_size), 1);
		}
	}
#endif /* defined(RLT_MAC) || defined(RTMP_MAC) */

	return TRUE;
}


/*
	ASIC register initialization sets
*/
INT mt_mac_init(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	mt_mac_pse_init(pAd);

	MtAsicInitMac(pAd);

	/* re-set specific MAC registers for individual chip */
	// TODO: Shiang-usw-win, here we need call "mt7603_init_mac_cr" for windows!
	if (pAd->chipOps.AsicMacInit != NULL)
		pAd->chipOps.AsicMacInit(pAd);

	/* auto-fall back settings */
	MtAsicAutoFallbackInit(pAd);

	MtAsicSetMacMaxLen(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __FUNCTION__));
	return TRUE;
}


VOID mt_chip_info_show(RTMP_ADAPTER *pAd)
{
#if defined(MT7615_FPGA) || defined(MT7622_FPGA)
	if (pAd->chipCap.hif_type == HIF_MT) {
		UINT32 ver, date_code, rev;
		UINT32 mac_val;

		RTMP_IO_READ32(pAd, 0x2700, &ver);
		RTMP_IO_READ32(pAd, 0x2704, &rev);
		RTMP_IO_READ32(pAd, 0x2708, &date_code);
		RTMP_IO_READ32(pAd, 0x201f8, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("##########################################\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(%d): MT Series FPGA Version:\n", __FUNCTION__, __LINE__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tFGPA1: Code[0x700]:0x%08x, [0x704]:0x%08x, [0x708]:0x%08x\n",
					ver, rev, date_code));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\tFPGA2: Version[0x201f8]:0x%08x\n", mac_val));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("##########################################\n"));
	}
#endif /* defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7615_FPGA) */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("MAC[Ver:Rev/ID=0x%08x : 0x%08x]\n",
				pAd->MACVersion, pAd->ChipID));
}

INT mt_nic_asic_init(RTMP_ADAPTER *pAd)
{
    INT ret = NDIS_STATUS_SUCCESS;
    MT_DMASCH_CTRL_T DmaSchCtrl;

    if(MtAsicWaitPDMAIdle(pAd, 100, 1000) != TRUE)
    {
        if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        {
            ret =  NDIS_STATUS_FAILURE;
            return ret;
        }
    }
#if  defined(COMPOS_WIN)  || defined (COMPOS_TESTMODE_WIN)
    DmaSchCtrl.bBeaconSpecificGroup = FALSE;
#else
    if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2))
    {
        DmaSchCtrl.bBeaconSpecificGroup = FALSE;
    }
    else
    {
        DmaSchCtrl.bBeaconSpecificGroup = TRUE;
    }
#endif
    DmaSchCtrl.mode = DMA_SCH_LMAC;

#ifdef DMA_SCH_SUPPORT
    MtAsicDMASchedulerInit(pAd, DmaSchCtrl);
#endif

#ifdef RTMP_PCI_SUPPORT
	pAd->CommonCfg.bPCIeBus = FALSE;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():device act as PCI%s driver\n",
					__FUNCTION__, (pAd->CommonCfg.bPCIeBus ? "-E" : "")));
#endif /* RTMP_PCI_SUPPORT */

	 // TODO: shiang-7603, init MAC setting
	 // TODO: shiang-7603, init beacon buffer
	 mt_mac_init(pAd);

	 mt_hw_tb_init(pAd, TRUE);

#ifdef HDR_TRANS_RX_SUPPORT
	AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, TRUE, FALSE);
	AsicRxHeaderTaranBLCtl(pAd, 0, TRUE, ETH_TYPE_EAPOL);
	AsicRxHeaderTaranBLCtl(pAd, 1, TRUE, ETH_TYPE_WAI);
	AsicRxHeaderTaranBLCtl(pAd, 2, TRUE, ETH_TYPE_FASTROAMING);
#endif

#ifdef BA_TRIGGER_OFFLOAD
	MtAsicAutoBATrigger(pAd, TRUE, BA_TRIGGER_OFFLOAD_TIMEOUT);
#endif

	return ret;
 }


