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
	trace.h
*/


#undef TRACE_SYSTEM
#define TRACE_SYSTEM mtk_wifi

#if !defined(__TRACE_H__) || defined(TRACE_HEADER_MULTI_READ)
#define __TRACE_H__

#include <linux/tracepoint.h>

#define TRACE_MCU_CMD_INFO trace_mcu_cmd_info
TRACE_EVENT(mcu_cmd_info,
	TP_PROTO(u32 Length,
			 u32 PQID,
			 u32 CID,
			 u32 PktTypeID,
			 u32 SetQuery,
			 u32 SeqNum,
			 u32 ExtCID,
			 u32 ExtCIDOption,
			 void *PayLoad,
			 u32 PayLoadLen
	),

	TP_ARGS(Length, PQID, CID, PktTypeID, SetQuery, SeqNum, ExtCID, ExtCIDOption, 
			PayLoad, PayLoadLen
	),

	TP_STRUCT__entry(
		__field(u32, Length)
		__field(u32, PQID)
		__field(u32, CID)
		__field(u32, PktTypeID)
		__field(u32, SetQuery)
		__field(u32, SeqNum)
		__field(u32, ExtCID)
		__field(u32, ExtCIDOption)
		__array(u8, PayLoad, 128)
	),

	TP_fast_assign(
		u8 *ptr;
		u32 ofs, len, pos = 0;
		__entry->Length = Length;
		__entry->PQID = PQID;
		__entry->CID = CID;
		__entry->PktTypeID = PktTypeID;
		__entry->SeqNum = SeqNum;
		__entry->ExtCID = ExtCID;
		__entry->ExtCIDOption = ExtCIDOption;
		ptr = (u8 *)PayLoad;

		for (ofs = 0; ofs < PayLoadLen; ofs += 16) 
		{
			hex_dump_to_buffer(ptr + ofs, 16, 16, 1, __entry->PayLoad + pos, 128 - pos, 0);
			pos += strlen(__entry->PayLoad + pos);
			if (128 - pos > 0)
				__entry->PayLoad[pos++] = '\n';
		}
	),

	TP_printk("length = 0x%x, pq_id = 0x%x, cid = 0x%x, pkt_type_id = 0x%x, set_query = 0x%x\
				seq_num = 0x%x, ext_cid = 0x%x, ext_cid_option = 0x%x\
				, cmd payload = %s",
			__entry->Length,
			__entry->PQID,
			__entry->CID,
			__entry->PktTypeID,
			__entry->SeqNum,
			__entry->ExtCID,
			__entry->ExtCIDOption,
			__entry->PayLoad
	)
);


#define TRACE_MCU_EVENT_INFO trace_mcu_event_info
TRACE_EVENT(mcu_event_info,
	TP_PROTO(u32 Length,
			 u32 PktTypeID,
			 u32 EID,
			 u32 SeqNum,
			 u32 ExtEID,
			 void *PayLoad,
			 u32 PayLoadLen
	),
	
	TP_ARGS(Length, PktTypeID, EID, SeqNum, ExtEID, 
			PayLoad, PayLoadLen
	),
	
	TP_STRUCT__entry(
		__field(u32, Length)
		__field(u32, PktTypeID)
		__field(u32, EID)
		__field(u32, SeqNum)
		__field(u32, ExtEID)
		__array(u8, PayLoad, 128)
	),
	
	TP_fast_assign(
		u8 *ptr;
		u32 ofs, len, pos = 0;
		__entry->Length = Length;
		__entry->PktTypeID = PktTypeID;
		__entry->EID = EID;
		__entry->SeqNum = SeqNum;
		__entry->ExtEID = ExtEID;
		ptr = (u8 *)PayLoad;
		
		for (ofs = 0; ofs < PayLoadLen; ofs += 16) 
		{
			hex_dump_to_buffer(ptr + ofs, 16, 16, 1, __entry->PayLoad + pos, 128 - pos, 0);
			pos += strlen(__entry->PayLoad + pos);
			if (128 - pos > 0)
				__entry->PayLoad[pos++] = '\n';
		}
	),

	TP_printk("length = 0x%x, pkt_type_id = 0x%x, eid = 0x%x\
				seq_num = 0x%x, ext_eid = 0x%x\
				, event payload = %s",
			__entry->Length,
			__entry->PktTypeID,
			__entry->EID,
			__entry->SeqNum,
			__entry->ExtEID,
			__entry->PayLoad
	)
);

#define TRACE_TX_MAC_SINFO trace_tx_mac_sinfo
TRACE_EVENT(tx_mac_sinfo,
	TP_PROTO(struct _TMAC_TXD_S *txd
	),

	TP_ARGS(txd
	),
	
	TP_STRUCT__entry(
		__field(u32, tx_byte_cnt)
		__field(u32, eth_type_offset)
		__field(u8, ip_sum)
		__field(u8, ut_sum)
		__field(u8, UNxV)
		__field(u8, UTxB)
		__field(u8, q_idx)
		__field(u8, p_idx)
		__field(u16, wlan_idx)
		__field(u8, hdr_info)
		__field(u8, hdr_format)
		__field(u8, ft)
		__field(u8, hdr_pad)
		__field(u8, no_ack)
		__field(u8, tid)
		__field(u8, protect_frm)
		__field(u8, own_mac)
		__field(u16, sch_tx_time)
		__field(u16, sw_field)
	),

	TP_fast_assign(
		__entry->tx_byte_cnt = txd->txd_0.tx_byte_cnt;
		__entry->eth_type_offset = txd->txd_0.eth_type_offset;
		__entry->ip_sum = txd->txd_0.ip_sum;
		__entry->ut_sum = txd->txd_0.ut_sum;
		__entry->UNxV = txd->txd_0.UNxV;
		__entry->UTxB = txd->txd_0.UTxB;
		__entry->q_idx = txd->txd_0.q_idx;
		__entry->p_idx = txd->txd_0.p_idx;
		__entry->wlan_idx = txd->txd_1.wlan_idx;
		__entry->hdr_info = txd->txd_1.hdr_info;
		__entry->hdr_format = txd->txd_1.hdr_format;
		__entry->ft = txd->txd_1.ft;
		__entry->hdr_pad = txd->txd_1.hdr_pad;
		__entry->no_ack = txd->txd_1.no_ack;
		__entry->tid = txd->txd_1.tid;
		__entry->protect_frm = txd->txd_1.protect_frm;
		__entry->own_mac = txd->txd_1.own_mac;
		__entry->sch_tx_time = txd->txd_7.sch_tx_time;
		__entry->sw_field = txd->txd_7.sw_field;
	),
	
	TP_printk("TMAC_TXD Fields:\n\
				TMAC_TXD_0:\n\
				\t\tPortID=%d\n\
				\t\tQueID=%d\n\
				\t\tTxByteCnt=%d\n\
				\t\tEtherTypeOffset=%d\n\
				\t\tIP checksum=%d\n\
				\t\tUDP/TCP checksum=%d\n\
				\t\tUNxV=%d\n\
				\t\tUTxB=%d\n\
				\tTMAC_TXD_1:\n\
				\t\tWlan_idx=%d\n\
				\t\tHdrInfo=0x%x\n\
				\t\tHdrFormat=0x%x\n\
				\t\tTxD format=%d\n\
				\t\tHdrPadding=%d\n\
				\t\tNo ack=%d\n\
				\t\tTID=%d\n\
				\t\tProtected frame=%d\n\
				\t\town mac=0x%x\n\
				\tTMAC_TXD_7:\n\
				\t\tscheduler tx time=0x%x\n\
				\t\tsw field=0x%x\n\
			  ",
				__entry->p_idx,
				__entry->q_idx,
			 	__entry->tx_byte_cnt,
				__entry->eth_type_offset,
				__entry->ip_sum,
				__entry->ut_sum,
				__entry->UNxV,
				__entry->UTxB,
			  	__entry->wlan_idx,
			  	__entry->hdr_info,
				__entry->hdr_format,
				__entry->ft,
				__entry->hdr_pad,
				__entry->tid,
				__entry->no_ack,
				__entry->protect_frm,
				__entry->own_mac,
				__entry->sch_tx_time,
				__entry->sw_field
	)

);


#define TRACE_TX_MAC_LINFO trace_tx_mac_linfo
TRACE_EVENT(tx_mac_linfo,
	TP_PROTO(struct _TMAC_TXD_L *txd
	),
	
	TP_ARGS(txd
	),
	
	TP_STRUCT__entry(
		__field(u32, tx_byte_cnt)
		__field(u32, eth_type_offset)
		__field(u8, ip_sum)
		__field(u8, ut_sum)
		__field(u8, UNxV)
		__field(u8, UTxB)
		__field(u8, q_idx)
		__field(u8, p_idx)
		__field(u16, wlan_idx)
		__field(u8, hdr_info)
		__field(u8, hdr_format)
		__field(u8, ft)
		__field(u8, hdr_pad)
		__field(u8, no_ack)
		__field(u8, tid)
		__field(u8, protect_frm)
		__field(u8, own_mac)
		__field(u8, sub_type)
		__field(u8, frm_type)
		__field(u8, ndp)
		__field(u8, ndpa)
		__field(u8, sounding)
		__field(u8, rts)
		__field(u8, bc_mc_pkt)
		__field(u8, bip)
		__field(u8, duration)
		__field(u8, htc_vld)
		__field(u8, frag)
		__field(u8, max_tx_time)
		__field(u8, pwr_offset)
		__field(u8, ba_disable)
		__field(u8, timing_measure)
		__field(u8, fix_rate)
		__field(u8, tx_cnt)
		__field(u8, remain_tx_cnt)
		__field(u16, sn)
		__field(u8, pn_vld)
		__field(u8, sn_vld)
		__field(u32, pn_low)
		__field(u8, pid)
		__field(u8, tx_status_fmt)
		__field(u8, tx_status_2_mcu)
		__field(u8, tx_status_2_host)
		__field(u8, da_select)
		__field(u8, bar_sn_ctrl)
		__field(u8, pwr_mgmt)
		__field(u16, pn_high)
		__field(u8, fix_rate_mode)
		__field(u8, ant_id)
		__field(u8, bw)
		__field(u8, spe_en)
		__field(u8, ant_pri)
		__field(u8, dyn_bw)
		__field(u8, ETxBF)
		__field(u8, ITxBF)
		__field(u16, tx_rate)
		__field(u8, ldpc)
		__field(u8, gi)
		__field(u16, sch_tx_time)
		__field(u16, sw_field)
	),

	TP_fast_assign(
		__entry->tx_byte_cnt = txd->txd_0.tx_byte_cnt;
		__entry->eth_type_offset = txd->txd_0.eth_type_offset;
		__entry->ip_sum = txd->txd_0.ip_sum;
		__entry->ut_sum = txd->txd_0.ut_sum;
		__entry->UNxV = txd->txd_0.UNxV;
		__entry->UTxB = txd->txd_0.UTxB;
		__entry->q_idx = txd->txd_0.q_idx;
		__entry->p_idx = txd->txd_0.p_idx;
		__entry->wlan_idx = txd->txd_1.wlan_idx;
		__entry->hdr_info = txd->txd_1.hdr_info;
		__entry->hdr_format = txd->txd_1.hdr_format;
		__entry->ft = txd->txd_1.ft;
		__entry->hdr_pad = txd->txd_1.hdr_pad;
		__entry->no_ack = txd->txd_1.no_ack;
		__entry->tid = txd->txd_1.tid;
		__entry->protect_frm = txd->txd_1.protect_frm;
		__entry->own_mac = txd->txd_1.own_mac;

		__entry->sub_type = txd->txd_2.sub_type;
		__entry->frm_type = txd->txd_2.frm_type;
		__entry->ndp = txd->txd_2.ndp;
		__entry->ndpa = txd->txd_2.ndpa;
		__entry->sounding = txd->txd_2.sounding;
		__entry->rts = txd->txd_2.rts;
		__entry->bc_mc_pkt = txd->txd_2.bc_mc_pkt;
		__entry->bip = txd->txd_2.bip;
		__entry->duration = txd->txd_2.duration;
		__entry->htc_vld = txd->txd_2.htc_vld;
		__entry->frag = txd->txd_2.frag;
		__entry->max_tx_time = txd->txd_2.max_tx_time;
		__entry->pwr_offset = txd->txd_2.pwr_offset;
		__entry->ba_disable = txd->txd_2.ba_disable;
		__entry->timing_measure = txd->txd_2.timing_measure;
		__entry->fix_rate = txd->txd_2.fix_rate;

		__entry->tx_cnt = txd->txd_3.tx_cnt;
		__entry->remain_tx_cnt = txd->txd_3.remain_tx_cnt;
		__entry->sn = txd->txd_3.sn;
		__entry->pn_vld = txd->txd_3.pn_vld;
		__entry->sn_vld = txd->txd_3.sn_vld;

		__entry->pn_low = txd->txd_4.pn_low;

		__entry->pid = txd->txd_5.pid;
		__entry->tx_status_fmt = txd->txd_5.tx_status_fmt;
		__entry->tx_status_2_mcu = txd->txd_5.tx_status_2_mcu;
		__entry->tx_status_2_host = txd->txd_5.tx_status_2_host;
		__entry->da_select = txd->txd_5.da_select;
		__entry->bar_sn_ctrl = txd->txd_5.bar_sn_ctrl;
		__entry->pwr_mgmt = txd->txd_5.pwr_mgmt;
		__entry->pn_high = txd->txd_5.pn_high;

		__entry->fix_rate_mode = txd->txd_6.fix_rate_mode;
		__entry->ant_id = txd->txd_6.ant_id;
		__entry->bw = txd->txd_6.bw;
		__entry->spe_en = txd->txd_6.spe_en;
		__entry->ant_pri = txd->txd_6.ant_pri;
		__entry->dyn_bw = txd->txd_6.dyn_bw;
		__entry->ETxBF = txd->txd_6.ETxBF;
		__entry->ITxBF = txd->txd_6.ITxBF;
		__entry->tx_rate = txd->txd_6.tx_rate;
		__entry->ldpc = txd->txd_6.ldpc;
		__entry->gi = txd->txd_6.gi;
		__entry->sch_tx_time = txd->txd_7.sch_tx_time;
		__entry->sw_field = txd->txd_7.sw_field;
	),
	
	TP_printk("TMAC_TXD Fields:\n\
				TMAC_TXD_0:\n\
				\t\tPortID=%d\n\
				\t\tQueID=%d\n\
				\t\tTxByteCnt=%d\n\
				\t\tEtherTypeOffset=%d\n\
				\t\tIP checksum=%d\n\
				\t\tUDP/TCP checksum=%d\n\
				\t\tUNxV=%d\n\
				\t\tUTxB=%d\n\
				\tTMAC_TXD_1:\n\
				\t\tWlan_idx=%d\n\
				\t\tHdrInfo=0x%x\n\
				\t\tHdrFormat=0x%x\n\
				\t\tTxD format=%d\n\
				\t\tHdrPadding=%d\n\
				\t\tNo ack=%d\n\
				\t\tTID=%d\n\
				\t\tProtected frame=%d\n\
				\t\town mac=0x%x\n\
				\tTMAC_TXD_2:\n\
				\t\tSubtype=%d\n\
				\t\tType=%d\n\
				\t\tNDP=%d\n\
				\t\tNDPA=%d\n\
				\t\tSounding=%d\n\
				\t\tRTS=%d\n\
				\t\tBM=%d\n\
				\t\tBIP=%d\n\
				\t\tDU=%d\n\
				\t\tHE=%d\n\
				\t\tFrag=%d\n\
				\t\tRemaining Life time=0x%x\n\
				\t\tPower offset=0x%x\n\
				\t\tBA disable=%d\n\
				\t\tTimeing measurement=%d\n\
				\t\tFix rate=%d\n\
				\tTMAC_TXD_3:\n\
				\t\tTx count=0x%x\n\
				\t\tRemaining tx count=0x%x\n\
				\t\tSN=0x%x\n\
				\t\tPN VLD=%d\n\
				\t\tSN VLD=%d\n\
				\tTMAC_TXD_4:\n\
				\t\tPN Low=0x%x\n\
				\tTMAC_TXD_5:\n\
				\t\tPID=0x%x\n\
				\t\tTX status format=%d\n\
				\t\tTX ststus return to MCU=%d\n\
				\t\tTX status return to Host=%d\n\
				\t\tDA selection=%d\n\
				\t\tBAR SSN control=%d\n\
				\t\tPower management=%d\n\
				\tTMAC_TXD_6:\n\
				\t\tFix rate mode=%d\n\
				\t\tAnt ID=%d\n\
				\t\tFix BW mode=%d\n\
				\t\tSpatial extension enable=%d\n\
				\t\tAnt priority=%d\n\
				\t\tDyn BW=%d\n\
				\t\tExt BF=%d\n\
				\t\tImplicit BF=%d\n\
				\t\tRate to fix=%d\n\
				\t\tLDPC=%d\n\
				\t\tGI=%d\n\
				\tTMAC_TXD_7:\n\
				\t\tscheduler tx time=0x%x\n\
				\t\tsw field=0x%x\n\
			  ",
				__entry->p_idx,
				__entry->q_idx,
			 	__entry->tx_byte_cnt,
				__entry->eth_type_offset,
				__entry->ip_sum,
				__entry->ut_sum,
				__entry->UNxV,
				__entry->UTxB,
			  	__entry->wlan_idx,
			  	__entry->hdr_info,
				__entry->hdr_format,
				__entry->ft,
				__entry->hdr_pad,
				__entry->tid,
				__entry->no_ack,
				__entry->protect_frm,
				__entry->own_mac,
				__entry->sub_type,
				__entry->frm_type,
				__entry->ndp,
				__entry->ndpa,
				__entry->sounding,
				__entry->rts,
				__entry->bc_mc_pkt,
				__entry->bip,
				__entry->duration,
				__entry->htc_vld,
				__entry->frag,
				__entry->max_tx_time,
				__entry->pwr_offset,
				__entry->ba_disable,
				__entry->timing_measure,
				__entry->fix_rate,
				__entry->tx_cnt,
				__entry->remain_tx_cnt,
				__entry->sn,
				__entry->pn_vld,
				__entry->sn_vld,
				__entry->pn_low,
				__entry->pid,
				__entry->tx_status_fmt,
				__entry->tx_status_2_mcu,
				__entry->tx_status_2_host,
				__entry->da_select,
				__entry->bar_sn_ctrl,
				__entry->pwr_mgmt,
				__entry->pn_high,
				__entry->fix_rate_mode,
				__entry->ant_id,
				__entry->bw,
				__entry->spe_en,
				__entry->ant_pri,
				__entry->dyn_bw,
				__entry->ETxBF,
				__entry->ITxBF,
				__entry->tx_rate,
				__entry->ldpc,
				__entry->gi,
				__entry->sch_tx_time,
				__entry->sw_field
	)
);


#define TRACE_RX_MAC_TXS_FM1 trace_rx_mac_txs_fm1
TRACE_EVENT(rx_mac_txs_fm1,
	TP_PROTO(UCHAR *txs_info
	),

	TP_ARGS(txs_info
	),

	TP_STRUCT__entry(
		__field(u16, tx_rate)
		__field(u8, fr)
		__field(u8, txsfm)
		__field(u8, txs2m)
		__field(u8, txs2h)
		__field(u8, ME)
		__field(u8, RE)
		__field(u8, LE)
		__field(u8, BE)
		__field(u8, txop)
		__field(u8, ps)
		__field(u8, baf)
		__field(u8, tid)
		__field(u8, ant_id)
		__field(u32, timestamp)
		__field(u32, front_time)
		__field(u8, tx_pwr_dBm)
		__field(u16, transmission_delay)
		__field(u8, rxv_sn)
		__field(u8, wlan_idx)
		__field(u16, sn)
		__field(u8, tbw)
		__field(u8, pid)
		__field(u8, fm)
		__field(u8, am)
		__field(u8, mpdu_tx_cnt)
		__field(u8, last_tx_rate_idx)

	),

	TP_fast_assign(
		TXS_STRUC *txs_entry = (TXS_STRUC *)txs_info;
		TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
		TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
		TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
		TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
		TXS_D_4 *txs_d4 = &txs_entry->txs_d4;

		__entry->tx_rate = txs_d0->tx_rate;
		__entry->fr = txs_d0->fr;
		__entry->txsfm = txs_d0->txsfm;
		__entry->txs2m = txs_d0->txs2m;
		__entry->txs2h = txs_d0->txs2h;
		__entry->ME = txs_d0->ME;
		__entry->RE = txs_d0->RE;
		__entry->LE = txs_d0->LE;
		__entry->BE = txs_d0->BE;
		__entry->txop = txs_d0->txop;
		__entry->ps = txs_d0->ps;
		__entry->baf = txs_d0->baf;
		__entry->tid = txs_d0->tid;
		__entry->ant_id = txs_d0->ant_id;
		__entry->timestamp = txs_d1->timestamp;
		__entry->front_time = txs_d2->field_ft.front_time;
		__entry->tx_pwr_dBm = txs_d2->field_ft.tx_pwr_dBm;
		__entry->transmission_delay = txs_d3->transmission_delay;
		__entry->rxv_sn = txs_d3->rxv_sn;
		__entry->wlan_idx = txs_d3->wlan_idx;
		__entry->sn = txs_d4->sn;
		__entry->tbw = txs_d4->tbw;
		__entry->pid = txs_d4->pid;
		__entry->fm = txs_d4->fm;
		__entry->am = txs_d4->fm;
		__entry->mpdu_tx_cnt =  txs_d4->mpdu_tx_cnt;
		__entry->last_tx_rate_idx = txs_d4->last_tx_rate_idx;
	),

	TP_printk("tx_rate = %d\n\
				\t\tfr = %d\n\
				\t\ttxsfm = %d\n\
				\t\ttxs2m = %d\n\
				\t\ttxs2h = %d\n\
				\t\tME = %d\n\
				\t\tRE = %d\n\
				\t\tLE = %d\n\
				\t\tBE = %d\n\
				\t\ttxop = %d\n\
				\t\tps = %d\n\
				\t\tbaf = %d\n\
				\t\ttid = %d\n\
				\t\tant_id = %d\n\
				\t\ttimestamp = %d\n\
				\t\tfront_time = %d\n\
				\t\ttx_pwr_dBm = %d\n\
				\t\ttransmission_delay = %d\n\
				\t\trxv_sn = %d\n\
				\t\twlan_idx = %d\n\
				\t\tsn = %d\n\
				\t\ttbw = %d\n\
				\t\tpid = %d\n\
				\t\tfm = %d\n\
				\t\tam =%d\n\
				\t\tmpdu_tx_cnt =  %d\n\
				\t\tlast_tx_rate_idx = %d\n",
				__entry->tx_rate,
				__entry->fr,
				__entry->txsfm,
				__entry->txs2m,
				__entry->txs2h,
				__entry->ME,
				__entry->RE,
				__entry->LE,
				__entry->BE,
				__entry->txop,
				__entry->ps,
				__entry->baf,
				__entry->tid,
				__entry->ant_id,
				__entry->timestamp,
				__entry->front_time,
				__entry->tx_pwr_dBm,
				__entry->transmission_delay,
				__entry->rxv_sn,
				__entry->wlan_idx,
				__entry->sn,
				__entry->tbw,
				__entry->pid,
				__entry->fm,
				__entry->am,
				__entry->mpdu_tx_cnt,
				__entry->last_tx_rate_idx
	)

);


#define TRACE_RX_MAC_TXS_FM2 trace_rx_mac_txs_fm2
TRACE_EVENT(rx_mac_txs_fm2,
	TP_PROTO(UCHAR *txs_info
	),

	TP_ARGS(txs_info
	),

	TP_STRUCT__entry(
		__field(u16, tx_rate)
		__field(u8, fr)
		__field(u8, txsfm)
		__field(u8, txs2m)
		__field(u8, txs2h)
		__field(u8, ME)
		__field(u8, RE)
		__field(u8, LE)
		__field(u8, BE)
		__field(u8, txop)
		__field(u8, ps)
		__field(u8, baf)
		__field(u8, tid)
		__field(u8, ant_id)
		__field(u8, noise_0)
		__field(u8, noise_1)
		__field(u8, noise_2)
		__field(u8, rsv1)
		__field(u8, rcpi_0)
		__field(u8, rcpi_1)
		__field(u8, rcpi_2)
		__field(u8, rsv2)
		__field(u8, tx_pwr_dBm)
		__field(u16, transmission_delay)
		__field(u8, rxv_sn)
		__field(u8, wlan_idx)
		__field(u16, sn)
		__field(u8, tbw)
		__field(u8, pid)
		__field(u8, fm)
		__field(u8, am)
		__field(u8, mpdu_tx_cnt)
		__field(u8, last_tx_rate_idx)

	),

	TP_fast_assign(
		TXS_STRUC *txs_entry = (TXS_STRUC *)txs_info;
		TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
		TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
		TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
		TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
		TXS_D_4 *txs_d4 = &txs_entry->txs_d4;

		__entry->tx_rate = txs_d0->tx_rate;
		__entry->fr = txs_d0->fr;
		__entry->txsfm = txs_d0->txsfm;
		__entry->txs2m = txs_d0->txs2m;
		__entry->txs2h = txs_d0->txs2h;
		__entry->ME = txs_d0->ME;
		__entry->RE = txs_d0->RE;
		__entry->LE = txs_d0->LE;
		__entry->BE = txs_d0->BE;
		__entry->txop = txs_d0->txop;
		__entry->ps = txs_d0->ps;
		__entry->baf = txs_d0->baf;
		__entry->tid = txs_d0->tid;
		__entry->ant_id = txs_d0->ant_id;
		__entry->noise_0 = txs_d1->field_noise.noise_0;
		__entry->noise_1 = txs_d1->field_noise.noise_1;
		__entry->noise_2 = txs_d1->field_noise.noise_2;
		__entry->rsv1 = txs_d1->field_noise.rsv;
		__entry->rcpi_0 = txs_d2->field_rcpi.rcpi_0; 
		__entry->rcpi_1 = txs_d2->field_rcpi.rcpi_1;
		__entry->rcpi_2 = txs_d2->field_rcpi.rcpi_2;
		__entry->rsv2 = txs_d2->field_rcpi.rsv;
		__entry->tx_pwr_dBm = txs_d2->field_rcpi.tx_pwr_dBm;
		__entry->transmission_delay = txs_d3->transmission_delay;
		__entry->rxv_sn = txs_d3->rxv_sn;
		__entry->wlan_idx = txs_d3->wlan_idx;
		__entry->sn = txs_d4->sn;
		__entry->tbw = txs_d4->tbw;
		__entry->pid = txs_d4->pid;
		__entry->fm = txs_d4->fm;
		__entry->am = txs_d4->fm;
		__entry->mpdu_tx_cnt =  txs_d4->mpdu_tx_cnt;
		__entry->last_tx_rate_idx = txs_d4->last_tx_rate_idx;
	),

	TP_printk("tx_rate = %d\n\
				\t\tfr = %d\n\
				\t\ttxsfm = %d\n\
				\t\ttxs2m = %d\n\
				\t\ttxs2h = %d\n\
				\t\tME = %d\n\
				\t\tRE = %d\n\
				\t\tLE = %d\n\
				\t\tBE = %d\n\
				\t\ttxop = %d\n\
				\t\tps = %d\n\
				\t\tbaf = %d\n\
				\t\ttid = %d\n\
				\t\tant_id = %d\n\
				\t\tnoise_0 = %d\n\
				\t\tnoise_1 = %d\n\
				\t\tnoise_2 = %d\n\
				\t\trsv1 = %d\n\
				\t\trcpi_0 = %d\n\
				\t\trcpi_1 = %d\n\
				\t\trcpi_2 = %d\n\
				\t\trsv2 = %d\n\
				\t\ttx_pwr_dBm = %d\n\
				\t\ttransmission_delay = %d\n\
				\t\trxv_sn = %d\n\
				\t\twlan_idx = %d\n\
				\t\tsn = %d\n\
				\t\ttbw = %d\n\
				\t\tpid = %d\n\
				\t\tfm = %d\n\
				\t\tam =%d\n\
				\t\tmpdu_tx_cnt =  %d\n\
				\t\tlast_tx_rate_idx = %d\n",
				__entry->tx_rate,
				__entry->fr,
				__entry->txsfm,
				__entry->txs2m,
				__entry->txs2h,
				__entry->ME,
				__entry->RE,
				__entry->LE,
				__entry->BE,
				__entry->txop,
				__entry->ps,
				__entry->baf,
				__entry->tid,
				__entry->ant_id,
				__entry->noise_0,
				__entry->noise_1,
				__entry->noise_2,
				__entry->rsv1,
				__entry->rcpi_0,
				__entry->rcpi_1,
				__entry->rcpi_2,
				__entry->rsv2,
				__entry->tx_pwr_dBm,
				__entry->transmission_delay,
				__entry->rxv_sn,
				__entry->wlan_idx,
				__entry->sn,
				__entry->tbw,
				__entry->pid,
				__entry->fm,
				__entry->am,
				__entry->mpdu_tx_cnt,
				__entry->last_tx_rate_idx
	)

);


#define TRACE_RX_MAC_TXRXV trace_rx_mac_txrxv
TRACE_EVENT(rx_mac_txrxv,
	TP_PROTO(UCHAR *txrxv_info
	),

	TP_ARGS(txrxv_info
	),

	TP_STRUCT__entry(
		__field(u32, TA_0_31)
		__field(u16, TA_32_47)
		__field(u8, RxvSn)
		__field(u8, TR)
		__field(u8, Reserved)
		__field(u8, HtStbc)
		__field(u8, HtAdCode)
		__field(u8, HtExtltf)
		__field(u8, TxMode)
		__field(u8, FrMode)
		__field(u8, VHTA1_B22)
		__field(u8, HtAggregation)
		__field(u8, HtShortGi)
		__field(u8, HtSmooth)
		__field(u8, HtNoSound)
		__field(u8, VHTA2_B8_B1)
		__field(u8, VHTA1_B5_B4)
		__field(u32, Length)
		__field(u16, VHTA1_B16_B6)
		__field(u8, VHTA1_B21_B17)
		__field(u8, OFDMFreqTransDet)
		__field(u8, ACI_DETx)
		__field(u8, SelAnt)
		__field(u8, Rcpi0)
		__field(u8, Fagc0EqCal)
		__field(u8, Fgac0CalGain)
		__field(u8, Rcpi1)
		__field(u8, Fagc1EqCal)
		__field(u8, Fgac1CalGain)
		__field(u8, IBRssix)
		__field(u8, WBRssix)
		__field(u8, FagcLpfGainx)
		__field(u8, Rcpi2)
		__field(u8, Fagc2EqCal)
		__field(u8, FgacCalGain)
		__field(u8, FagcLnaGain0)
	    __field(u8, FagcLnaGain1)
		__field(u8, CagcState_1)
	    __field(u16, FoE)
	    __field(u8, LTF_ProcTime)
	    __field(u8, LTF_SNR0)
		__field(u8, Nf0)
		__field(u8, Nf1)
		__field(u8, Nf2)
		__field(u8, RxValidIndicator)
		__field(u8, NsTsField)
		__field(u8, CagcState_2)
		__field(u8, Reserved_1)
		__field(u8, PrimItfrEnv)
		__field(u8, SecItfrEnv)
		__field(u8, Sec40ItfrEnv)
		__field(u8, BtEnv)
		__field(u32, Ofdm_1)
		__field(u16, Ofdm_2)
		__field(u8, BtdNoTchLoc)
		__field(u8, OfdmLtfSNR1)
	    __field(u8, Reserved2)
		__field(u8, RxScramblingSeed)
		__field(u8, HtStfDet)
		__field(u8, BfAgcLnaGainx)
		__field(u8, Reserved0)
		__field(u8, BfAgcLpfGain)
		__field(u8, BfAgcIbRssix)
		__field(u8, BgAgcWbRssix)
		__field(u8, Reserved1)
		__field(u8, OfdmCeRmsdId)
		__field(u8, OfdmCeGicEnb)
		__field(u8, OfdmCeLtfComb)
		__field(u8, OfdmDewModeDet)
		__field(u8, FcsErr)
	),

	TP_fast_assign(
		RXV_DWORD0 *DW0 = NULL;
		RXV_DWORD1 *DW1 = NULL;
		RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
		RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
		RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
		RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
		RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
		RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;
		RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
		RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
		RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;

		DW0 = (RXV_DWORD0 *)txrxv_info;
		DW1 = (RXV_DWORD1 *)(txrxv_info + 4);
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(txrxv_info + 8);
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(txrxv_info + 12);
		RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(txrxv_info + 16);
		RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(txrxv_info + 20);
		RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(txrxv_info + 24);
		RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(txrxv_info + 28);
		RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(txrxv_info + 32);
		RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(txrxv_info + 36);
		RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(txrxv_info + 40);

		__entry->TA_0_31 = DW0->TA_0_31;
		__entry->TA_32_47 = DW1->TA_32_47;
		__entry->RxvSn = DW1->RxvSn;
		__entry->TR = DW1->TR;
		__entry->Reserved = DW1->Reserved;
		__entry->HtStbc = RXV1_1ST_CYCLE->HtStbc;
		__entry->HtAdCode = RXV1_1ST_CYCLE->HtAdCode;
		__entry->HtExtltf =RXV1_1ST_CYCLE->HtExtltf;
		__entry->TxMode = RXV1_1ST_CYCLE->TxMode;
		__entry->FrMode = RXV1_1ST_CYCLE->FrMode;
		__entry->VHTA1_B22 = RXV1_1ST_CYCLE->VHTA1_B22;
		__entry->HtAggregation = RXV1_1ST_CYCLE->HtAggregation;
		__entry->HtShortGi = RXV1_1ST_CYCLE->HtShortGi;
		__entry->HtSmooth = RXV1_1ST_CYCLE->HtSmooth;
		__entry->HtNoSound = RXV1_1ST_CYCLE->HtNoSound;
		__entry->VHTA2_B8_B1 = RXV1_1ST_CYCLE->VHTA2_B8_B1;
		__entry->VHTA1_B5_B4 = RXV1_1ST_CYCLE->VHTA1_B5_B4;
		__entry->Length = RXV1_2ND_CYCLE->Length;
		__entry->VHTA1_B16_B6 = RXV1_2ND_CYCLE->VHTA1_B16_B6;
		__entry->VHTA1_B21_B17 = RXV1_3TH_CYCLE->VHTA1_B21_B17;
		__entry->OFDMFreqTransDet = RXV1_3TH_CYCLE->OFDMFreqTransDet;
		__entry->ACI_DETx = RXV1_3TH_CYCLE->ACI_DETx;
		__entry->SelAnt = RXV1_3TH_CYCLE->SelAnt;
		__entry->Rcpi0 = RXV1_3TH_CYCLE->Rcpi0;
		__entry->Fagc0EqCal = RXV1_3TH_CYCLE->Fagc0EqCal;
		__entry->Fgac0CalGain = RXV1_3TH_CYCLE->Fgac0CalGain;
		__entry->Rcpi1 = RXV1_3TH_CYCLE->Rcpi1;
		__entry->Fagc1EqCal = RXV1_3TH_CYCLE->Fagc1EqCal;
		__entry->Fgac1CalGain = RXV1_3TH_CYCLE->Fgac1CalGain;
		__entry->IBRssix = RXV1_4TH_CYCLE->IBRssix;
		__entry->WBRssix = RXV1_4TH_CYCLE->WBRssix;
		__entry->FagcLpfGainx = RXV1_4TH_CYCLE->FagcLpfGainx;
		__entry->Rcpi2 = RXV1_4TH_CYCLE->Rcpi2;
		__entry->Fagc2EqCal = RXV1_4TH_CYCLE->Fagc2EqCal;
		__entry->FgacCalGain = RXV1_4TH_CYCLE->FgacCalGain;
		__entry->FagcLnaGain0 = RXV1_5TH_CYCLE->FagcLnaGain0;
    	__entry->FagcLnaGain1 = RXV1_5TH_CYCLE->FagcLnaGain1;
		__entry->CagcState_1 = RXV1_5TH_CYCLE->CagcState;
	    __entry->FoE = RXV1_5TH_CYCLE->FoE;
		__entry->LTF_ProcTime = RXV1_5TH_CYCLE->LTF_ProcTime;
 		__entry->LTF_SNR0 = RXV1_5TH_CYCLE->LTF_SNR0;
		__entry->Nf0 = RXV1_6TH_CYCLE->Nf0;
		__entry->Nf1 = RXV1_6TH_CYCLE->Nf1;
		__entry->Nf2 = RXV1_6TH_CYCLE->Nf2;
		__entry->RxValidIndicator = RXV1_6TH_CYCLE->RxValidIndicator;
		__entry->NsTsField = RXV1_6TH_CYCLE->NsTsField;
		__entry->CagcState_2 = RXV1_6TH_CYCLE->CagcState;
		__entry->Reserved_1 = RXV1_6TH_CYCLE->Reserved;
		__entry->PrimItfrEnv = RXV2_1ST_CYCLE->PrimItfrEnv;
		__entry->SecItfrEnv = RXV2_1ST_CYCLE->SecItfrEnv;
		__entry->Sec40ItfrEnv = RXV2_1ST_CYCLE->Sec40ItfrEnv;
		__entry->BtEnv = RXV2_1ST_CYCLE->BtEnv;
		__entry->Ofdm_1 = RXV2_1ST_CYCLE->Ofdm;
		__entry->Ofdm_2 = RXV2_2ND_CYCLE->Ofdm;
		__entry->BtdNoTchLoc = RXV2_2ND_CYCLE->BtdNoTchLoc;
		__entry->OfdmLtfSNR1 = RXV2_2ND_CYCLE->OfdmLtfSNR1;
    	__entry->Reserved2 = RXV2_2ND_CYCLE->Reserved2;
		__entry->RxScramblingSeed = RXV2_2ND_CYCLE->RxScramblingSeed;
		__entry->HtStfDet = RXV2_3TH_CYCLE->HtStfDet;
		__entry->BfAgcLnaGainx = RXV2_3TH_CYCLE->BfAgcLnaGainx;
		__entry->Reserved0 = RXV2_3TH_CYCLE->Reserved0;
		__entry->BfAgcLpfGain = RXV2_3TH_CYCLE->BfAgcLpfGain;
		__entry->BfAgcIbRssix = RXV2_3TH_CYCLE->BfAgcIbRssix;
		__entry->BgAgcWbRssix = RXV2_3TH_CYCLE->BgAgcWbRssix;
		__entry->Reserved1 = RXV2_3TH_CYCLE->Reserved1;
		__entry->OfdmCeRmsdId = RXV2_3TH_CYCLE->OfdmCeRmsdId;
		__entry->OfdmCeGicEnb = RXV2_3TH_CYCLE->OfdmCeGicEnb;
		__entry->OfdmCeLtfComb = RXV2_3TH_CYCLE->OfdmCeLtfComb;
		__entry->OfdmDewModeDet = RXV2_3TH_CYCLE->OfdmDewModeDet;
		__entry->FcsErr = RXV2_3TH_CYCLE->FcsErr;
	),

	TP_printk("\t\tTA_0_31 = 0x%x\n\
				\t\tTA_32_47 = 0x%x\n\
				\t\tRxvSn = %d\n\
				\t\tTR = %d\n\
				\t\tReserved= %d\n\
				\t\tHtStbc= %d\n\
				\t\tHtAdCode = %d\n\
				\t\tHtExtltf = %d\n\
				\t\tTxMode = %d\n\
				\t\tFrMode = %d\n\
				\t\tVHTA1_B22 = %d\n\
				\t\tHtAggregation =% d\n\
				\t\tHtShortGi = %d\n\
				\t\tHtSmooth = %d\n\
				\t\tHtNoSound = %d\n\
				\t\tVHTA2_B8_B1 = %d\n\
				\t\tVHTA1_B5_B4 = %d\n\
				\t\tLength = %d\n\
				\t\tVHTA1_B16_B6 = %d\n\
				\t\tVHTA1_B21_B17 = %d\n\
				\t\tOFDMFreqTransDet = %d\n\
				\t\tACI_DETx = %d\n\
				\t\tSelAnt = %d\n\
				\t\tRcpi0 = %d\n\
				\t\tFagc0EqCal = %d\n\
				\t\tFgac0CalGain = %d\n\
				\t\tRcpi1 = %d\n\
				\t\tFagc1EqCal = %d\n\
				\t\tFgac1CalGain = %d\n\
				\t\tIBRssix = %d\n\
				\t\tWBRssix = %d\n\
				\t\tFagcLpfGainx = %d\n\
				\t\tRcpi2 = %d\n\
				\t\tFagc2EqCal = %d\n\
				\t\tFgacCalGain = %d\n\
				\t\tFagcLnaGain0 = %d\n\
    			\t\tFagcLnaGain1 = %d\n\
				\t\tCagcState_1 = %d\n\
    			\t\tFoE = %d\n\
    			\t\tLTF_ProcTime = %d\n\
    			\t\tLTF_SNR0 = %d\n\
				\t\tNf0 = %d\n\
				\t\tNf1 = %d\n\
				\t\tNf2 = %d\n\
				\t\tRxValidIndicator = %d\n\
				\t\tNsTsField = %d\n\
				\t\tCagcState_2 = %d\n\
				\t\tReserved = %d\n\
				\t\tPrimItfrEnv = %d\n\
				\t\tSecItfrEnv = %d\n\
				\t\tSec40ItfrEnv = %d\n\
				\t\tBtEnv = %d\n\
				\t\tOfdm_1 = %d\n\
				\t\tOfdm_2 = %d\n\
				\t\tBtdNoTchLoc = %d\n\
				\t\tOfdmLtfSNR1 = %d\n\
    			\t\tReserved2 = %d\n\
				\t\tRxScramblingSeed = %d\n\
				\t\tHtStfDet = %d\n\
				\t\tBfAgcLnaGainx = %d\n\
				\t\tReserved0 = %d\n\
				\t\tBfAgcLpfGain = %d\n\
				\t\tBfAgcIbRssix = %d\n\
				\t\tBgAgcWbRssi = %d\n\
				\t\tReserved1 = %d\n\
				\t\tOfdmCeRmsdId = %d\n\
				\t\tOfdmCeGicEnb = %d\n\
				\t\tOfdmCeLtfComb = %d\n\
				\t\tOfdmDewModeDet = %d\n\
				\t\tFcsErr = %d\n",
				__entry->TA_0_31,
				__entry->TA_32_47,
				__entry->RxvSn,
				__entry->TR,
				__entry->Reserved,
				__entry->HtStbc,
				__entry->HtAdCode,
				__entry->HtExtltf,
				__entry->TxMode,
				__entry->FrMode,
				__entry->VHTA1_B22,
				__entry->HtAggregation,
				__entry->HtShortGi,
				__entry->HtSmooth,
				__entry->HtNoSound,
				__entry->VHTA2_B8_B1,
				__entry->VHTA1_B5_B4,
				__entry->Length,
				__entry->VHTA1_B16_B6,
				__entry->VHTA1_B21_B17,
				__entry->OFDMFreqTransDet,
				__entry->ACI_DETx,
				__entry->SelAnt,
				__entry->Rcpi0,
				__entry->Fagc0EqCal,
				__entry->Fgac0CalGain,
				__entry->Rcpi1,
				__entry->Fagc1EqCal,
				__entry->Fgac1CalGain,
				__entry->IBRssix,
				__entry->WBRssix,
				__entry->FagcLpfGainx,
				__entry->Rcpi2,
				__entry->Fagc2EqCal,
				__entry->FgacCalGain,
				__entry->FagcLnaGain0,
    			__entry->FagcLnaGain1,
				__entry->CagcState_1,
	    		__entry->FoE,
				__entry->LTF_ProcTime,
 				__entry->LTF_SNR0,
				__entry->Nf0,
				__entry->Nf1,
				__entry->Nf2,
				__entry->RxValidIndicator,
				__entry->NsTsField,
				__entry->CagcState_2,
				__entry->Reserved,
				__entry->PrimItfrEnv,
				__entry->SecItfrEnv,
				__entry->Sec40ItfrEnv,
				__entry->BtEnv,
				__entry->Ofdm_1,
				__entry->Ofdm_2,
				__entry->BtdNoTchLoc,
				__entry->OfdmLtfSNR1,
    			__entry->Reserved2,
				__entry->RxScramblingSeed,
				__entry->HtStfDet,
				__entry->BfAgcLnaGainx,
				__entry->Reserved0,
				__entry->BfAgcLpfGain,
				__entry->BfAgcIbRssix,
				__entry->BgAgcWbRssix,
				__entry->Reserved1,
				__entry->OfdmCeRmsdId,
				__entry->OfdmCeGicEnb,
				__entry->OfdmCeLtfComb,
				__entry->OfdmDewModeDet,
				__entry->FcsErr
	)
);

#define TRACE_RX_MAC_NORMAL trace_rx_mac_normal
TRACE_EVENT(rx_mac_normal,
	TP_PROTO(u8 *rmac_info
	),

	TP_ARGS(rmac_info
	),

	TP_STRUCT__entry(
		__field(u32, rx_byte_cnt)
		__field(u16, eth_type_offset)
		__field(u8, ip_sum)
		__field(u8, ut_sum)
		__field(u8, grp_vld)
		__field(u8, pkt_type)
		__field(u8, htc_vld)
		__field(u8, u2m)
		__field(u8, mcast)
		__field(u8, bcast)
		__field(u8, beacon_mc)
		__field(u8, beacon_uc)
		__field(u8, key_id)
		__field(u16, ch_freq)
		__field(u8, mac_hdr_len)
		__field(u8, hdr_offset)
		__field(u8, hdr_trans)
		__field(u8, payload_format)
		__field(u8, bssid)
		__field(u16, wlan_idx)
		__field(u8, tid)
		__field(u8, sec_mode)
		__field(u8, sw_bit)
		__field(u8, fcs_err)
		__field(u8, cm)
		__field(u8, clm)
		__field(u8, icv_err)
		__field(u8, tkip_mic_err)
		__field(u8, len_mismatch)
		__field(u8, de_amsdu_err)
		__field(u8, exceed_max_rx_len)
		__field(u8, llc_mis)
		__field(u8, UDF_VLT)
		__field(u8, frag)
		__field(u8, null_frm)
		__field(u8, ndata)
		__field(u8, non_ampdu_sub_frm)
		__field(u8, non_ampdu)
		__field(u16, rx_vector_seq)
		__field(u8, tsf_compare_loss)
		__field(u8, pattern_drop_bit)
		__field(u8, cls)
		__field(u8, OFLD)
		__field(u8, magic_pkt)
		__field(u8, wol)
		__field(u16, cls_bitmap)
		__field(u8, pf_mode)
		__field(u8, pf_sts)
		__field(u32, ta_low)
		__field(u32, frm_ctrl)
		__field(u32, ta_high)
		__field(u32, qos_ctrl)
		__field(u16, seq_num)
		__field(u8, frag_num)
		__field(u32, htc_ctrl)
		__field(u32, sec_pn_32)
		__field(u16, wpi_pn_64)
		__field(u16, sec_pn_48)
		__field(u32, wpi_pn_96)
		__field(u32, wpi_pn_128)
		__field(u32, timestamp)
		__field(u32, crc)
		__field(u32, rxv_1)
		__field(u32, rxv_2)
		__field(u32, rxv_3)
		__field(u32, rxv_4)
		__field(u32, rxv_5)
		__field(u32, rxv_6)
		__field(u32, rxv_7)
	),

	TP_fast_assign(
		struct rxd_base_struc *rx_base;
    	UCHAR *Pos;
		RXD_GRP4_STRUCT *RxdGrp4 = NULL;
		RXD_GRP1_STRUCT *RxdGrp1 = NULL;
		RXD_GRP2_STRUCT *RxdGrp2 = NULL;
		RXD_GRP3_STRUCT *RxdGrp3 = NULL;

		rx_base = (struct rxd_base_struc *)rmac_info;
		Pos = rmac_info;
		
		Pos += RMAC_INFO_BASE_SIZE;

		if (rx_base->rxd_0.grp_vld & RXS_GROUP4)
		{
			RxdGrp4 = (RXD_GRP4_STRUCT *)Pos;
			Pos += RMAC_INFO_GRP_4_SIZE;
		}

		if (rx_base->rxd_0.grp_vld & RXS_GROUP1)
		{
			RxdGrp1 = (RXD_GRP1_STRUCT *)Pos;
			Pos += RMAC_INFO_GRP_1_SIZE;
		}

		if (rx_base->rxd_0.grp_vld & RXS_GROUP2)
		{
			RxdGrp2 = (RXD_GRP2_STRUCT *)Pos;
			Pos += RMAC_INFO_GRP_2_SIZE;
		}

		if (rx_base->rxd_0.grp_vld & RXS_GROUP3)
		{
			RxdGrp3 = (RXD_GRP3_STRUCT *)Pos;
			Pos += RMAC_INFO_GRP_3_SIZE;
		}

		__entry->rx_byte_cnt = rx_base->rxd_0.rx_byte_cnt;
		__entry->eth_type_offset = rx_base->rxd_0.eth_type_offset;
		__entry->ip_sum = rx_base->rxd_0.ip_sum;
		__entry->ut_sum = rx_base->rxd_0.ut_sum;
		__entry->grp_vld = rx_base->rxd_0.grp_vld;
		__entry->pkt_type = rx_base->rxd_0.pkt_type;
		__entry->htc_vld =  rx_base->rxd_1.htc_vld;
		__entry->u2m = rx_base->rxd_1.u2m;
		__entry->mcast = rx_base->rxd_1.mcast;
		__entry->bcast = rx_base->rxd_1.bcast;
		__entry->beacon_mc = rx_base->rxd_1.beacon_mc;
		__entry->beacon_uc = rx_base->rxd_1.beacon_uc;
		__entry->key_id = rx_base->rxd_1.key_id;
		__entry->ch_freq = rx_base->rxd_1.ch_freq;
		__entry->mac_hdr_len = rx_base->rxd_1.mac_hdr_len;
		__entry->hdr_offset = rx_base->rxd_1.hdr_offset;
		__entry->hdr_trans = rx_base->rxd_1.hdr_trans;
		__entry->payload_format = rx_base->rxd_1.payload_format;
		__entry->bssid = rx_base->rxd_1.bssid;
		__entry->wlan_idx = rx_base->rxd_2.wlan_idx;
		__entry->tid = rx_base->rxd_2.tid;
		__entry->sec_mode = rx_base->rxd_2.sec_mode;
		__entry->sw_bit = rx_base->rxd_2.sw_bit;
		__entry->fcs_err = rx_base->rxd_2.fcs_err;
		__entry->cm = rx_base->rxd_2.cm;
		__entry->clm = rx_base->rxd_2.clm;
		__entry->icv_err = rx_base->rxd_2.icv_err;
		__entry->tkip_mic_err = rx_base->rxd_2.tkip_mic_err;
		__entry->len_mismatch = rx_base->rxd_2.len_mismatch;
		__entry->de_amsdu_err = rx_base->rxd_2.de_amsdu_err;
		__entry->exceed_max_rx_len = rx_base->rxd_2.exceed_max_rx_len;
		__entry->llc_mis = rx_base->rxd_2.llc_mis;
		__entry->UDF_VLT = rx_base->rxd_2.UDF_VLT;
		__entry->frag = rx_base->rxd_2.frag;
		__entry->null_frm = rx_base->rxd_2.null_frm;
		__entry->ndata = rx_base->rxd_2.ndata;
		__entry->non_ampdu_sub_frm = rx_base->rxd_2.non_ampdu_sub_frm;
		__entry->non_ampdu = rx_base->rxd_2.non_ampdu;
		__entry->rx_vector_seq = rx_base->rxd_3.rx_vector_seq;
		__entry->tsf_compare_loss = rx_base->rxd_3.tsf_compare_loss;
		__entry->pattern_drop_bit = rx_base->rxd_3.pattern_drop_bit;
		__entry->cls = rx_base->rxd_3.cls;
		__entry->OFLD = rx_base->rxd_3.OFLD;
		__entry->magic_pkt = rx_base->rxd_3.magic_pkt;
		__entry->wol = rx_base->rxd_3.wol;
		__entry->cls_bitmap = rx_base->rxd_3.cls_bitmap;
		__entry->pf_mode = rx_base->rxd_3.pf_mode;
		__entry->pf_sts = rx_base->rxd_3.pf_sts;
		
		if (rx_base->rxd_0.grp_vld & RXS_GROUP4)
		{	
			__entry->ta_low = RxdGrp4->ta_low; 
			__entry->frm_ctrl = RxdGrp4->frm_ctrl;
			__entry->ta_high = RxdGrp4->ta_high;
			__entry->qos_ctrl = RxdGrp4->qos_ctrl;
			__entry->seq_num = RxdGrp4->seq_num;
			__entry->frag_num = RxdGrp4->frag_num;
			__entry->htc_ctrl = RxdGrp4->htc_ctrl;
		}
		

		if (rx_base->rxd_0.grp_vld & RXS_GROUP1)
		{
			__entry->sec_pn_32 = RxdGrp1->sec_pn_32;
			__entry->wpi_pn_64 = RxdGrp1->wpi_pn_64;
			__entry->sec_pn_48 = RxdGrp1->sec_pn_48;
			__entry->wpi_pn_96 = RxdGrp1->wpi_pn_96;
			__entry->wpi_pn_128 = RxdGrp1->wpi_pn_128;
		}
		

		if (rx_base->rxd_0.grp_vld & RXS_GROUP2)
		{
			__entry->timestamp = RxdGrp2->timestamp;
			__entry->crc = RxdGrp2->crc;
		}

		if (rx_base->rxd_0.grp_vld & RXS_GROUP3)
		{
			__entry->rxv_1 = RxdGrp3->rxv_1;
			__entry->rxv_2 = RxdGrp3->rxv_2;
			__entry->rxv_3 = RxdGrp3->rxv_3;
			__entry->rxv_4 = RxdGrp3->rxv_4;
			__entry->rxv_5 = RxdGrp3->rxv_5;
			__entry->rxv_6 = RxdGrp3->rxv_6;
			__entry->rxv_7 = RxdGrp3->rxv_7;

		}
	),

	TP_printk("\t\trx_byte_cnt = 0x%x\n\
		\t\teth_type_offset = %d\n\
		\t\tip_sum = %d\n\
		\t\tut_sum = %d\n\
		\t\tgrp_vld =%d\n\
		\t\tpkt_type = %d\n\
		\t\thtc_vld =  %d\n\
		\t\tu2m = %d\n\
		\t\tmcast = %d\n\
		\t\tbcast = %d\n\
		\t\tbeacon_mc = %d\n\
		\t\tbeacon_uc = %d\n\
		\t\tkey_id = %d\n\
		\t\tch_freq = %d\n\
		\t\tmac_hdr_len = %d\n\
		\t\thdr_offset = %d\n\
		\t\thdr_trans = %d\n\
		\t\tpayload_format = %d\n\
		\t\tbssid = %d\n\
		\t\twlan_idx = %d\n\
		\t\ttid = %d\n\
		\t\tsec_mode = %d\n\
		\t\tsw_bit = %d\n\
		\t\tfcs_err = %d\n\
		\t\tcm = %d\n\
		\t\tclm = %d\n\
		\t\ticv_err = %d\n\
		\t\ttkip_mic_err = %d\n\
		\t\tlen_mismatch = %d\n\
		\t\tde_amsdu_err = %d\n\
		\t\texceed_max_rx_len = %d\n\
		\t\tllc_mis = %d\n\
		\t\tUDF_VLT = %d\n\
		\t\tfrag = %d\n\
		\t\tnull_frm = %d\n\
		\t\tndata = %d\n\
		\t\tnon_ampdu_sub_frm = %d\n\
		\t\tnon_ampdu = %d\n\
		\t\trx_vector_seq = %d\n\
		\t\ttsf_compare_loss = %d\n\
		\t\tpattern_drop_bit = %d\n\
		\t\tcls = %d\n\
		\t\tOFLD = %d\n\
		\t\tmagic_pkt = %d\n\
		\t\twol = %d\n\
		\t\tcls_bitmap = %d\n\
		\t\tpf_mode = %d\n\
		\t\tpf_sts = %d\n\
		\t\tta_low =  %d\n\
		\t\tfrm_ctrl = %d\n\
		\t\tta_high = %d\n\
		\t\tqos_ctrl = %d\n\
		\t\tseq_num = %d\n\
		\t\tfrag_num = %d\n\
		\t\thtc_ctrl = %d\n\
		\t\tsec_pn_32 = 0x%x\n\
		\t\twpi_pn_64 = 0x%x\n\
		\t\tsec_pn_48 = 0x%x\n\
		\t\twpi_pn_96 = 0x%x\n\
		\t\twpi_pn_128 = 0x%x\n\
		\t\ttimestamp = %d\n\
		\t\tcrc = %d\n\
		\t\trxv_1 = 0x%x\n\
		\t\trxv_2 = 0x%x\n\
		\t\trxv_3 = 0x%x\n\
		\t\trxv_4 = 0x%x\n\
		\t\trxv_5 = 0x%x\n\
		\t\trxv_6 = 0x%x\n\
		\t\trxv_7 = 0x%x\n\
		",
		__entry->rx_byte_cnt,
		__entry->eth_type_offset,
		__entry->ip_sum,
		__entry->ut_sum,
		__entry->grp_vld,
		__entry->pkt_type,
		__entry->htc_vld,
		__entry->u2m,
		__entry->mcast,
		__entry->bcast,
		__entry->beacon_mc,
		__entry->beacon_uc,
		__entry->key_id,
		__entry->ch_freq,
		__entry->mac_hdr_len,
		__entry->hdr_offset,
		__entry->hdr_trans,
		__entry->payload_format,
		__entry->bssid,
		__entry->wlan_idx,
		__entry->tid,
		__entry->sec_mode,
		__entry->sw_bit,
		__entry->fcs_err,
		__entry->cm,
		__entry->clm,
		__entry->icv_err,
		__entry->tkip_mic_err,
		__entry->len_mismatch,
		__entry->de_amsdu_err,
		__entry->exceed_max_rx_len,
		__entry->llc_mis,
		__entry->UDF_VLT,
		__entry->frag,
		__entry->null_frm,
		__entry->ndata,
		__entry->non_ampdu_sub_frm,
		__entry->non_ampdu,
		__entry->rx_vector_seq,
		__entry->tsf_compare_loss,
		__entry->pattern_drop_bit,
		__entry->cls,
		__entry->OFLD,
		__entry->magic_pkt,
		__entry->wol,
		__entry->cls_bitmap,
		__entry->pf_mode,
		__entry->pf_sts,
		__entry->ta_low,
		__entry->frm_ctrl,
		__entry->ta_high,
		__entry->qos_ctrl,
		__entry->seq_num,
		__entry->frag_num,
		__entry->htc_ctrl,
		__entry->sec_pn_32,
		__entry->wpi_pn_64,
		__entry->sec_pn_48,
		__entry->wpi_pn_96,
		__entry->wpi_pn_128,
		__entry->timestamp,
		__entry->crc,
		__entry->rxv_1,
		__entry->rxv_2,
		__entry->rxv_3,
		__entry->rxv_4,
		__entry->rxv_5,
		__entry->rxv_6,
		__entry->rxv_7
	)
);

#define TRACE_CR_TX_MAC_INFO trace_cr_tx_mac_info
TRACE_EVENT(cr_tx_mac_info,
	TP_PROTO(RTMP_ADAPTER *pAd
	),

	TP_ARGS(pAd
	),

	TP_STRUCT__entry(
		__field(u8, tx_stream)
		__field(u8, tx_rifs_en)
		__field(u8, rx_rifs_mode)
		__field(u8, txop_tbtt_ctl)
		__field(u8, txop_tbtt_stop_ctl)
		__field(u8, txop_burst_stop)
		__field(u8, rdg_mode)
		__field(u8, rdg_res_en)
		__field(u8, smoothing)
		__field(u32, ap_pwr_rxpe_off_time)
		__field(u32, ap_pwr_rxpe_on_time)
		__field(u32, ap_pwr_hat_time)
		__field(u8, ap_pwr_en)
		__field(u32, ac0_txop)
		__field(u32, ac1_txop)
		__field(u32, ac2_txop)
		__field(u32, ac3_txop)
		__field(u32, ac10_txop)
		__field(u32, ac11_txop)
		__field(u32, ac12_txop)
		__field(u32, ac13_txop)
		__field(u32, eifs_time)
		__field(u32, rifs_time)
		__field(u32, sifs_time)
		__field(u32, slot_time)
		__field(u32, agg_timeout)
	),

	TP_fast_assign(
		UINT32 Value;

		RTMP_IO_READ32(pAd, TMAC_TCR, &Value); 
		__entry->tx_stream = GET_TMAC_TCR_TX_STREAM_NUM(Value) + 1;
		__entry->tx_rifs_en = GET_TX_RIFS_EN(Value);
		__entry->rx_rifs_mode = GET_RX_RIFS_MODE(Value);
		__entry->txop_tbtt_ctl = GET_TXOP_TBTT_CONTROL(Value);
		__entry->txop_tbtt_stop_ctl = GET_TBTT_TX_STOP_CONTROL(Value);
		__entry->txop_burst_stop = GET_TXOP_BURST_STOP(Value);
		__entry->rdg_mode = GET_RDG_RA_MODE(Value);
		__entry->rdg_res_en = GET_RDG_RESP_EN(Value);
		__entry->smoothing = GET_SMOOTHING(Value); 

		RTMP_IO_READ32(pAd, TMAC_PSCR, &Value);
		__entry->ap_pwr_rxpe_off_time = GET_APS_OFF_TIME(Value);
		__entry->ap_pwr_rxpe_on_time = APS_ON_TIME(Value);
		__entry->ap_pwr_hat_time = GET_APS_HALT_TIME(Value);
		__entry->ap_pwr_en = GET_APS_EN(Value);    

		RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value);
		__entry->ac0_txop = GET_AC0LIMIT(Value);
		__entry->ac1_txop = GET_AC1LIMIT(Value);
			
		RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value);
		__entry->ac2_txop = GET_AC2LIMIT(Value);
		__entry->ac3_txop = GET_AC3LIMIT(Value);
			
		RTMP_IO_READ32(pAd, TMAC_ACTXOPLR3, &Value);
		__entry->ac10_txop = GET_AC10LIMIT(Value);
		__entry->ac11_txop = GET_AC11LIMIT(Value);
			
		RTMP_IO_READ32(pAd, TMAC_ACTXOPLR2, &Value);
		__entry->ac12_txop = GET_AC12LIMIT(Value);
		__entry->ac13_txop = GET_AC13LIMIT(Value);

		RTMP_IO_READ32(pAd, TMAC_ICR, &Value);
		__entry->eifs_time = GET_ICR_EIFS_TIME(Value);
		__entry->rifs_time = GET_ICR_RIFS_TIME(Value);
		__entry->sifs_time = GET_ICR_SIFS_TIME(Value);
		__entry->slot_time = GET_ICR_SLOT_TIME(Value); 
	
		RTMP_IO_READ32(pAd, ATCR, &Value);
		__entry->agg_timeout = GET_AGG_TOUT(Value);
	),

	TP_printk("TX Stream = %d\n\
			\t\tTX RIFS Enable = %d\n\
			\t\tRX RIFS Mode = %d\n\
			\t\tTXOP TBTT Control = %d\n\
			\t\tTXOP TBTT Stop Control = %d\n\
			\t\tTXOP Burst Stop = %d\n\
			\t\tRDG Mode = %d\n\
			\t\tRDG Responser Enable = %d\n\
			\t\tSmoothing = %d\n\
			\t\tAP Power Save RXPE Off Time(unit 2us) = %d\n\
			\t\tAP Power Save RXPE On Time(unit 2us) = %d\n\
			\t\tAP Power Save Halt Time (unit 32us) = %d\n\
			\t\tAP Power Enable = %d\n\
			\t\tAC0 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC1 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC2 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC3 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC10 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC11 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC12 TXOP = 0x%x (unit: 32us)\n\
			\t\tAC13 TXOP = 0x%x (unit: 32us)\n\
			\t\tEIFS Time (unit: 1us) = %d\n\
			\t\tRIFS Time (unit: 1us) = %d\n\
			\t\tSIFS Time (unit: 1us) = %d\n\
			\t\tSLOT Time (unit: 1us) = %d\n\
			\t\tAggregation Timeout (unit: 50ns) = 0x%x",
			__entry->tx_stream,
			__entry->tx_rifs_en,
			__entry->rx_rifs_mode,
			__entry->txop_tbtt_ctl,
			__entry->txop_tbtt_stop_ctl,
			__entry->txop_burst_stop,
			__entry->rdg_mode,
			__entry->rdg_res_en,
			__entry->smoothing,
			__entry->ap_pwr_rxpe_off_time,
			__entry->ap_pwr_rxpe_on_time,
			__entry->ap_pwr_hat_time,
			__entry->ap_pwr_en,
			__entry->ac0_txop,
			__entry->ac1_txop,
			__entry->ac2_txop,
			__entry->ac3_txop,
			__entry->ac10_txop,
			__entry->ac11_txop,
			__entry->ac12_txop,
			__entry->ac13_txop,
			__entry->eifs_time,
			__entry->rifs_time,
			__entry->sifs_time,
			__entry->slot_time,
			__entry->agg_timeout
	)
);

#define TRACE_CR_TX_AGG_INFO trace_cr_tx_agg_info
TRACE_EVENT(cr_tx_agg_info,
	TP_PROTO(RTMP_ADAPTER *pAd
	),

	TP_ARGS(pAd
	),

	TP_STRUCT__entry(
		__field(u8, mm_protection)
		__field(u8, gf_protection)
		__field(u8, protection_mode)
		__field(u8, bw40_protection)
		__field(u8, rifs_protection)
		__field(u8, bw80_protection)
		__field(u8, bw160_protection)
		__field(u8, erp_protection)
		__field(u32, rts_pkt_len_thd)
		__field(u32, rts_pkt_num_thd)
		__field(u32, rts_retry_count_limit)
		__field(u32, bar_tx_count_limit)
		__field(u8, ampdu_no_ba_rule)
		__field(u8, ampdu_no_ba_ar_rule)
		__field(u32, bar_tx_rate)
		__field(u32, bar_tx_mode)
		__field(u32, bar_nsts)
		__field(u8, bar_stbc)
		__field(u32, ac0_agg_limit)
		__field(u32, ac1_agg_limit)
		__field(u32, ac2_agg_limit)
		__field(u32, ac3_agg_limit)
		__field(u32, ac10_agg_limit)
		__field(u32, ac11_agg_limit)
		__field(u32, ac12_agg_limit)
		__field(u32, ac13_agg_limit)
		__field(u32, winsize0_limit)
		__field(u32, winsize1_limit)
		__field(u32, winsize2_limit)
		__field(u32, winsize3_limit)
		__field(u32, winsize4_limit)
		__field(u32, winsize5_limit)
		__field(u32, winsize6_limit)
		__field(u32, winsize7_limit)
	),

	TP_fast_assign(
		UINT32 Value;

		RTMP_IO_READ32(pAd, AGG_PCR, &Value);
		__entry->mm_protection = GET_MM_PROTECTION(Value);
		__entry->gf_protection = GET_GF_PROTECTION(Value);
		__entry->protection_mode = GET_PROTECTION_MODE(Value);
		__entry->bw40_protection = GET_BW40_PROTECTION(Value);
		__entry->rifs_protection = GET_RIFS_PROTECTION(Value);
		__entry->bw80_protection = GET_BW80_PROTECTION(Value);
		__entry->bw160_protection = GET_BW160_PROTECTION(Value);
		__entry->erp_protection = GET_ERP_PROTECTION(Value);   

		RTMP_IO_READ32(pAd, AGG_PCR1, &Value);
		__entry->rts_pkt_len_thd = GET_RTS_THRESHOLD(Value);  
		__entry->rts_pkt_num_thd = GET_RTS_PKT_NUM_THRESHOLD(Value);
		RTMP_IO_READ32(pAd, AGG_MRCR, &Value);
		__entry->rts_retry_count_limit = GET_RTS_RTY_CNT_LIMIT(Value); 
		__entry->bar_tx_count_limit = GET_BAR_TX_CNT_LIMIT(Value); 
		RTMP_IO_READ32(pAd, AGG_ACR, &Value);
		__entry->ampdu_no_ba_rule = GET_AMPDU_NO_BA_RULE(Value);
		__entry->ampdu_no_ba_ar_rule = GET_AGG_ACR_AMPDU_NO_BA_AR_RULE(Value);
		__entry->bar_tx_rate = GET_BAR_RATE_TX_RATE(Value);
		__entry->bar_tx_mode = GET_BAR_RATE_TX_MODE(Value);
		__entry->bar_nsts = GET_BAR_RATE_NSTS(Value);
		__entry->bar_stbc = GET_BAR_RATE_STBC(Value);

		RTMP_IO_READ32(pAd, AGG_AALCR, &Value); 
		__entry->ac0_agg_limit = GET_AC0_AGG_LIMIT(Value); 
		__entry->ac1_agg_limit = GET_AC1_AGG_LIMIT(Value); 
		__entry->ac2_agg_limit = GET_AC2_AGG_LIMIT(Value); 
		__entry->ac3_agg_limit = GET_AC3_AGG_LIMIT(Value);

		RTMP_IO_READ32(pAd, AGG_AALCR1, &Value); 
		__entry->ac10_agg_limit = GET_AC10_AGG_LIMIT(Value); 
		__entry->ac11_agg_limit = GET_AC11_AGG_LIMIT(Value); 
		__entry->ac12_agg_limit = GET_AC12_AGG_LIMIT(Value); 
		__entry->ac13_agg_limit = GET_AC13_AGG_LIMIT(Value);

		RTMP_IO_READ32(pAd, AGG_AWSCR, &Value);
		__entry->winsize0_limit = GET_WINSIZE0(Value); 
		__entry->winsize1_limit = GET_WINSIZE1(Value); 
		__entry->winsize2_limit = GET_WINSIZE2(Value); 
		__entry->winsize3_limit = GET_WINSIZE3(Value); 

		RTMP_IO_READ32(pAd, AGG_AWSCR1, &Value);
		__entry->winsize4_limit = GET_WINSIZE4(Value); 
		__entry->winsize5_limit = GET_WINSIZE5(Value); 
		__entry->winsize6_limit = GET_WINSIZE6(Value); 
		__entry->winsize7_limit = GET_WINSIZE7(Value); 
	),

	TP_printk("MM Protection = %d\n\
				GF Protection = %d\n\
				Protection Mode = %d\n\
				BW40 Protection = %d\n\
				RIFS Protection = %d\n\
				BW80 Protection = %d\n\
				BW160 Protection = %d\n\
				ERP Protection = 0x%x\n\
				RTS Threshold(packet length) = 0x%x\n\
				RTS PKT Nums Threshold = %d\n\
				RTS Retry Count Limit = %d\n\
				BAR Frame Tx Count Limit = %d\n\
				AMPDU No BA Rule = %d\n\
				AMPDU No BA AR Rule = %d\n\
				BAR Tx Rate = 0x%x\n\
				BAR Tx Mode = 0x%x\n\
				BAR Nsts = %d\n\
				BAR STBC = %d\n\
				AC0 Agg limit = %d\n\
				AC1 Agg limit = %d\n\
				AC2 Agg limit = %d\n\
				AC3 Agg limit = %d\n\
				AC10 Agg limit = %d\n\
				AC11 Agg limit = %d\n\
				AC12 Agg limit = %d\n\
				AC13 Agg limit = %d\n\
				Winsize0 limit = %d\n\
				Winsize1 limit = %d\n\
				Winsize2 limit = %d\n\
				Winsize3 limit = %d\n\
				Winsize4 limit = %d\n\
				Winsize5 limit = %d\n\
				Winsize6 limit = %d\n\
				Winsize7 limit = %d", 
				__entry->mm_protection,
				__entry->gf_protection,
				__entry->protection_mode,
				__entry->bw40_protection,
				__entry->rifs_protection,
				__entry->bw80_protection,
				__entry->bw160_protection,
				__entry->erp_protection,
				__entry->rts_pkt_len_thd,  
				__entry->rts_pkt_num_thd,
				__entry->rts_retry_count_limit, 
				__entry->bar_tx_count_limit, 
				__entry->ampdu_no_ba_rule,
				__entry->ampdu_no_ba_ar_rule,
				__entry->bar_tx_rate,
				__entry->bar_tx_mode,
				__entry->bar_nsts,
				__entry->bar_stbc,
				__entry->ac0_agg_limit, 
				__entry->ac1_agg_limit,
				__entry->ac2_agg_limit,
				__entry->ac3_agg_limit,
				__entry->ac10_agg_limit, 
				__entry->ac11_agg_limit,
				__entry->ac12_agg_limit, 
				__entry->ac13_agg_limit,
				__entry->winsize0_limit, 
				__entry->winsize1_limit, 
				__entry->winsize2_limit, 
				__entry->winsize3_limit, 
				__entry->winsize4_limit, 
				__entry->winsize5_limit, 
				__entry->winsize6_limit, 
				__entry->winsize7_limit 
	)
);

#define TRACE_CR_PSE_INFO trace_cr_pse_info
TRACE_EVENT(cr_pse_info,
	TP_PROTO(RTMP_ADAPTER *pAd
	),

	TP_ARGS(pAd
	),

	TP_STRUCT__entry(
		__field(u32, total_page_num)
		__field(u32, page_size_cfg)
		__field(u32, min_rsrv_p0)
		__field(u32, max_quota_p0)
		__field(u32, min_rsrv_p1)
		__field(u32, max_quota_p1)
		__field(u32, min_rsrv_p2_rq0)
		__field(u32, max_quota_p2_rq0)
		__field(u32, min_rsrv_p2_rq1)
		__field(u32, max_quota_p2_rq1)
		__field(u32, min_rsrv_p2_rq2)
		__field(u32, max_quota_p2_rq2)
		__field(u32, free_page_cnt)
		__field(u32, ffa_cnt)
		__field(u32, rsrv_pri_p0)
		__field(u32, rsrv_pri_p1)
		__field(u32, rsrv_pri_p2_rq0)
		__field(u32, rsrv_pri_p2_rq1)
		__field(u32, rsrv_pri_p2_rq2)
		__field(u32, rsrv_cnt_p0)
		__field(u32, rsrv_cnt_p1)
		__field(u32, rsrv_cnt_p2_rq0)
		__field(u32, rsrv_cnt_p2_rq1)
		__field(u32, rsrv_cnt_p2_rq2)
		__field(u32, src_cnt_p0)
		__field(u32, src_cnt_p1)
		__field(u32, src_cnt_p2_rq0)
		__field(u32, src_cnt_p2_rq1)
		__field(u32, src_cnt_p2_rq2)
	),
	
	TP_fast_assign(
		UINT32 Value;

		RTMP_IO_READ32(pAd, PSE_BC, &Value);
		__entry->total_page_num = GET_TOTAL_PAGE_NUM(Value);
		__entry->page_size_cfg = GET_PAGE_SIZE_CFG(Value);

		RTMP_IO_READ32(pAd, FC_P0, &Value);
		__entry->min_rsrv_p0 = GET_MIN_RSRV_P0(Value);
		__entry->max_quota_p0 = GET_MAX_QUOTA_P0(Value);

		RTMP_IO_READ32(pAd, FC_P1, &Value);
		__entry->min_rsrv_p1 = GET_MIN_RSRV_P1(Value);
		__entry->max_quota_p1 = GET_MAX_QUOTA_P1(Value);

		RTMP_IO_READ32(pAd, FC_P2Q0, &Value);
		__entry->min_rsrv_p2_rq0 = GET_MIN_RSRV_P2_RQ0(Value);
		__entry->max_quota_p2_rq0 = GET_MAX_QUOTA_P2_RQ0(Value);

		RTMP_IO_READ32(pAd, FC_P2Q1, &Value);
		__entry->min_rsrv_p2_rq1 = GET_MIN_RSRV_P2_RQ1(Value);
		__entry->max_quota_p2_rq1 = GET_MAX_QUOTA_P2_RQ1(Value);

		RTMP_IO_READ32(pAd, FC_P2Q2, &Value);
		__entry->min_rsrv_p2_rq2 = GET_MIN_RSRV_P2_RQ2(Value);
		__entry->max_quota_p2_rq2 = GET_MAX_QUOTA_P2_RQ2(Value);

		RTMP_IO_READ32(pAd, FC_FFC, &Value);
		__entry->free_page_cnt = GET_FREE_PAGE_CNT(Value);
		__entry->ffa_cnt = GET_FFA_CNT(Value);

		RTMP_IO_READ32(pAd, FC_FRP, &Value);
		__entry->rsrv_pri_p0 = GET_RSRV_PRI_P0(Value);
		__entry->rsrv_pri_p1 = GET_RSRV_PRI_P1(Value);
		__entry->rsrv_pri_p2_rq0 = GET_RSRV_PRI_P2_RQ0(Value);
		__entry->rsrv_pri_p2_rq1 = GET_RSRV_PRI_P2_RQ1(Value);
		__entry->rsrv_pri_p2_rq2 = GET_RSRV_PRI_P2_RQ2(Value);

		RTMP_IO_READ32(pAd, FC_RP0P1, &Value);
		__entry->rsrv_cnt_p0 = GET_RSRV_CNT_P0(Value);
		__entry->rsrv_cnt_p1 = GET_RSRV_CNT_P1(Value);

		RTMP_IO_READ32(pAd, FC_RP2Q0Q1, &Value);
		__entry->rsrv_cnt_p2_rq0 = GET_RSRV_CNT_P2_RQ0(Value);
		__entry->rsrv_cnt_p2_rq1 = GET_RSRV_CNT_P2_RQ1(Value);

		RTMP_IO_READ32(pAd, FC_RP2Q2, &Value);
		__entry->rsrv_cnt_p2_rq2 = GET_RSRV_CNT_P2_RQ2(Value);

		RTMP_IO_READ32(pAd, FC_SP0P1, &Value);
		__entry->src_cnt_p0 = GET_SRC_CNT_P0(Value);
		__entry->src_cnt_p1 = GET_SRC_CNT_P1(Value);

		RTMP_IO_READ32(pAd, FC_SP2Q0Q1, &Value);
		__entry->src_cnt_p2_rq0 = GET_SRC_CNT_P2_RQ0(Value);
		__entry->src_cnt_p2_rq1 = GET_SRC_CNT_P2_RQ1(Value);

		RTMP_IO_READ32(pAd, FC_SP2Q2, &Value);
		__entry->src_cnt_p2_rq2 = GET_SRC_CNT_P2_RQ2(Value);
	),

	TP_printk("Total Page Nums = %d\n\
			Page Size = %d\n\
			P0(HIF) min reserve setting = %d\n\
			P0(HIF) max quota setting = %d\n\
			P1(MCU) min reserve setting = %d\n\
			P1(MCU) max quota setting = %d\n\
			P2 RQ0(Rx Data) min reserve setting = %d\n\
			P2 RQ0(Rx Data) max quota setting = %d\n\
			P2 RQ1(RxV) min reserve setting = %d\n\
			P2 RQ1(RxV) max quota setting = %d\n\
			P2 RQ2(TxS) min reserve setting = %d\n\
			P2 RQ2(TxS) max quota setting = %d\n\
			Free page counter status = %d\n\
			Free for all(FFA) counter status = %d\n\
			P0(HIF) reserve priority = 0x%x\n\
			P1(MCU) reserve priority = 0x%x\n\
			P2 RQ0(Rx Data) reserve priority = 0x%x\n\
			P2 RQ1(RxV) reserve priority = 0x%x\n\
			P2 RQ2(TxS) reserve priority = 0x%x\n\
			P0(HIF) reserve counter status = %d\n\
			P1(MCU) reserve counter status = %d\n\
			P2 RQ0(Rx Data) reserve counter status = %d\n\
			P2 RQ1(RxV) reserve counter status = %d\n\
			P2 RQ2(TxS) reserve counter status = %d\n\
			P0(HIF) source counter status = %d\n\
			P1(MCU) source counter status = %d\n\
			P2 RQ0(Rx Data) source counter status = %d\n\
			P2 RQ1(RxV) source counter status = %d\n\
			P2 RQ2(TxS) source counter status = %d\n",
			__entry->total_page_num,
			__entry->page_size_cfg,
			__entry->min_rsrv_p0,
			__entry->max_quota_p0,
			__entry->min_rsrv_p1,
			__entry->max_quota_p1,
			__entry->min_rsrv_p2_rq0,
			__entry->max_quota_p2_rq0,
			__entry->min_rsrv_p2_rq1,
			__entry->max_quota_p2_rq1,
			__entry->min_rsrv_p2_rq2,
			__entry->max_quota_p2_rq2,
			__entry->free_page_cnt,
			__entry->ffa_cnt,
			__entry->rsrv_pri_p0,
			__entry->rsrv_pri_p1,
			__entry->rsrv_pri_p2_rq0,
			__entry->rsrv_pri_p2_rq1,
			__entry->rsrv_pri_p2_rq2,
			__entry->rsrv_cnt_p0,
			__entry->rsrv_cnt_p1,
			__entry->rsrv_cnt_p2_rq0,
			__entry->rsrv_cnt_p2_rq1,
			__entry->rsrv_cnt_p2_rq2,
			__entry->src_cnt_p0,
			__entry->src_cnt_p1,
			__entry->src_cnt_p2_rq0,
			__entry->src_cnt_p2_rq1,
			__entry->src_cnt_p2_rq2
		)
);



#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace
#include <trace/define_trace.h>
