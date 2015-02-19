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
#include <linux/time.h>
#include <linux/rtc.h>

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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		UINT32 Value;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				length = 0x%x, pq_id = 0x%x, cid = 0x%x, pkt_type_id = 0x%x, set_query = 0x%x\
				seq_num = 0x%x, ext_cid = 0x%x, ext_cid_option = 0x%x\
				, cmd payload = %s",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				length = 0x%x, pkt_type_id = 0x%x, eid = 0x%x\
				seq_num = 0x%x, ext_eid = 0x%x\
				, event payload = %s",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->Length,
			__entry->PktTypeID,
			__entry->EID,
			__entry->SeqNum,
			__entry->ExtEID,
			__entry->PayLoad
	)
);

#define	TRACE_MCU_PS_RETRIEVE_START_REQ trace_mcu_ps_retrieve_start_req
TRACE_EVENT(mcu_ps_retrieve_start_req,
	TP_PROTO(UINT16 WlanIdx
	),

	TP_ARGS(WlanIdx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u16, WlanIdx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->WlanIdx = WlanIdx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   WlanIdx = %d\n",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->WlanIdx
	)
);


#define	TRACE_MCU_PS_RETRIEVE_START_RSP trace_mcu_ps_retrieve_start_rsp
TRACE_EVENT(mcu_ps_retrieve_start_rsp,
	TP_PROTO(UINT8 Stage, UINT32 wlan_idx, UINT32 ps_state, UINT32 ps_qbitmap
	),

	TP_ARGS(Stage, wlan_idx, ps_state, ps_qbitmap
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u32, wlan_idx)
		__field(u32, ps_state)
		__field(u32, ps_qbitmap)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->Stage = Stage;
		__entry->wlan_idx = wlan_idx;
		__entry->ps_state = ps_state;
		__entry->ps_qbitmap = ps_qbitmap
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   wlan_idx = %d\n\
			   ps_state = %d\n\
			   ps_qbitmap = %d",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->Stage,
			__entry->wlan_idx,
			__entry->ps_state,
			__entry->ps_qbitmap
	)
);

#define	TRACE_MCU_PS_RETRIEVE_START_RSP_FROM_CR trace_mcu_ps_retrieve_start_rsp_from_cr
TRACE_EVENT(mcu_ps_retrieve_start_rsp_from_cr,
	TP_PROTO(UINT8 Stage, UINT32 wlan_idx, UINT32 ps_state, UINT32 ps_qbitmap
	),

	TP_ARGS(Stage, wlan_idx, ps_state, ps_qbitmap
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u32, wlan_idx)
		__field(u32, ps_state)
		__field(u32, ps_qbitmap)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->Stage = Stage;
		__entry->wlan_idx = wlan_idx;
		__entry->ps_state = ps_state;
		__entry->ps_qbitmap = ps_qbitmap
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   wlan_idx = %d\n\
			   ps_state = %d\n\
			   ps_qbitmap = %d",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->Stage,
			__entry->wlan_idx,
			__entry->ps_state,
			__entry->ps_qbitmap
	)
);


#define TRACE_MCU_PS_CLEAR trace_mcu_ps_clear
TRACE_EVENT(mcu_ps_clear,
	TP_PROTO(UINT16 WlanIdx
	),

	TP_ARGS(WlanIdx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u16, WlanIdx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->WlanIdx = WlanIdx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   WlanIdx = %d\n",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->WlanIdx
	)
);

#define TRACE_MCU_PS_CLEAR_RSP trace_mcu_ps_clear_rsp
TRACE_EVENT(mcu_ps_clear_rsp,
	TP_PROTO(UINT8 Stage, UINT16 WlanIdx, UINT32 ps_state
	),

	TP_ARGS(Stage, WlanIdx, ps_state
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u16, WlanIdx)
		__field(u32, ps_state)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->Stage = Stage;
		__entry->WlanIdx = WlanIdx;
		__entry->ps_state = ps_state;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   WlanIdx = %d\n\
			   ps_state = %d",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->Stage,
			__entry->WlanIdx,
			__entry->ps_state
	)
);



#define TRACE_TX_MAC_SINFO trace_tx_mac_sinfo
TRACE_EVENT(tx_mac_sinfo,
	TP_PROTO(struct _TMAC_TXD_S *txd
	),

	TP_ARGS(txd
	),
	
	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		UINT32 Value;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

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
	
	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				TMAC_TXD Fields:\n\
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
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		UINT32 Value;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

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
	
	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				TMAC_TXD Fields:\n\
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
				\t\tsw field=0x%x\n",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				tx_rate = %d\n\
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
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				tx_rate = %d\n\
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
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
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


#define TRACE_RX_MAC_NORMAL trace_rx_mac_normal
TRACE_EVENT(rx_mac_normal,
	TP_PROTO(u8 *rmac_info
	),

	TP_ARGS(rmac_info
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
		\t\trx_byte_cnt = 0x%x\n\
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
		__entry->year,
		__entry->mon,
		__entry->day,
		__entry->hour,
		__entry->min,
		__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			TX Stream = %d\n\
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
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				MM Protection = %d\n\
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
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
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
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

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

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			Total Page Nums = %d\n\
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
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
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


#define TRACE_CR_PSE_FRAME_INFO trace_cr_pse_frame_info
TRACE_EVENT(cr_pse_frame_info,
	TP_PROTO(UINT8 PID, UINT8 QID, UINT32 FID
	),

	TP_ARGS(PID, QID, FID
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, PID)
		__field(u8, QID)
		__field(u32, FID)
	),

	TP_fast_assign(
		UINT32 Value;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->PID = PID;
		__entry->QID = QID;
		__entry->FID = FID;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   PID = %d\n\
			   QID = %d\n\
			   FID = %d",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->PID,
			__entry->QID,
			__entry->FID
	)
);


#define TRACE_CR_WTBL1_INFO trace_cr_wtbl1_info
TRACE_EVENT(cr_wtbl1_info,
	TP_PROTO(RTMP_ADAPTER *pAd, struct wtbl_1_struc *tb
	),

	TP_ARGS(pAd, tb
	),

	TP_STRUCT__entry(
		__field(u32, addr0)
		__field(u32, addr1)
		__field(u32, addr2)
		__field(u32, addr3)
		__field(u32, addr4)
		__field(u32, addr5)
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, muar_idx)
		__field(u32, rc_a1)
		__field(u32, rc_a2)
		__field(u32, kid)
		__field(u32, rkv)
		__field(u32, rv)
		__field(u8, sw)
		__field(u8, wm) 
		__field(u8, mm)
		__field(u8, cipher_suit)
		__field(u8, td)
		__field(u8, fd)
		__field(u8, dis_rhtr)
		__field(u8, af)
		__field(u8, rx_ps)
		__field(u8, r)
		__field(u8, rts)
		__field(u8, cf_ack)
		__field(u8, rdg_ba)
		__field(u8, smps)
		__field(u8, baf_en)
		__field(u8, ht) 
		__field(u8, vht) 
		__field(u8, ldpc)
		__field(u8, dyn_bw)
		__field(u8, tibf)
		__field(u8, tebf)
		__field(u8, txop_ps_cap)
		__field(u8, mesh)
		__field(u8, qos)
		__field(u8, adm)
		__field(u32, gid)
		__field(u32, wtbl2_fid)
		__field(u32, wtbl2_eid)
		__field(u32, wtbl3_fid)
		__field(u32, wtbl3_eid)
		__field(u32, wtbl4_fid)
		__field(u32, wtbl4_eid)
		__field(u8, chk_per)
		__field(u8, du_i_psm)
		__field(u8, i_psm)
		__field(u8, psm)
		__field(u8, skip_tx)
		__field(u8, partial_aid)
	),

	TP_fast_assign(
		union WTBL_1_DW0 *wtbl_1_d0 = (union WTBL_1_DW0 *)&tb->wtbl_1_d0.word;
		union WTBL_1_DW1 *wtbl_1_d1 = (union WTBL_1_DW1 *)&tb->wtbl_1_d1.word;
		union WTBL_1_DW2 *wtbl_1_d2 = (union WTBL_1_DW2 *)&tb->wtbl_1_d2.word;
		union WTBL_1_DW3 *wtbl_1_d3 = (union WTBL_1_DW3 *)&tb->wtbl_1_d3.word;
		union WTBL_1_DW4 *wtbl_1_d4 = (union WTBL_1_DW4 *)&tb->wtbl_1_d4.word;
		UINT32 Value;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->addr0 = wtbl_1_d1->field.addr_0 & 0xff;
		__entry->addr1 = ((wtbl_1_d1->field.addr_0 & 0xff00) >> 8);
		__entry->addr2 = ((wtbl_1_d1->field.addr_0 & 0xff0000) >> 16);
		__entry->addr3 = ((wtbl_1_d1->field.addr_0 & 0xff000000) >> 24);
		__entry->addr4 = wtbl_1_d0->field.addr_4 & 0xff;
		__entry->addr5 = wtbl_1_d0->field.addr_5 & 0xff;

		__entry->muar_idx = wtbl_1_d0->field.muar_idx;
		__entry->rc_a1 = wtbl_1_d0->field.rc_a1;
		__entry->rc_a2 = wtbl_1_d0->field.rc_a2;
		__entry->kid = wtbl_1_d0->field.kid;
		__entry->rkv = wtbl_1_d0->field.rkv;
		__entry->rv = wtbl_1_d0->field.rv;
		__entry->sw = wtbl_1_d0->field.sw;
		__entry->wm = wtbl_1_d0->field.wm;
 		__entry->mm = wtbl_1_d2->field.mm;
		__entry->cipher_suit = wtbl_1_d2->field.cipher_suit;
		__entry->td = wtbl_1_d2->field.td; 
		__entry->fd = wtbl_1_d2->field.fd;
		__entry->dis_rhtr = wtbl_1_d2->field.dis_rhtr;
		__entry->af = wtbl_1_d2->field.af;
		__entry->rx_ps = wtbl_1_d2->field.rx_ps;
		__entry->r = wtbl_1_d2->field.r;
		__entry->rts = wtbl_1_d2->field.rts;
		__entry->cf_ack = wtbl_1_d2->field.cf_ack;
		__entry->rdg_ba = wtbl_1_d2->field.rdg_ba;
		__entry->smps = wtbl_1_d2->field.smps;
		__entry->baf_en = wtbl_1_d2->field.baf_en;
		__entry->ht = wtbl_1_d2->field.ht;
		__entry->vht = wtbl_1_d2->field.vht;
		__entry->ldpc = wtbl_1_d2->field.ldpc; 
		__entry->dyn_bw = wtbl_1_d2->field.dyn_bw;
		__entry->tibf = wtbl_1_d2->field.tibf; 
		__entry->tebf = wtbl_1_d2->field.tebf;
		__entry->txop_ps_cap = wtbl_1_d2->field.txop_ps_cap;
		__entry->mesh = wtbl_1_d2->field.mesh;
		__entry->qos = wtbl_1_d2->field.qos;
		__entry->adm = wtbl_1_d2->field.adm;
		__entry->gid = wtbl_1_d2->field.gid;
		__entry->wtbl2_fid = wtbl_1_d3->field.wtbl2_fid;
		__entry->wtbl2_eid = wtbl_1_d3->field.wtbl2_eid;
		__entry->wtbl3_fid = wtbl_1_d4->field.wtbl3_fid;
		__entry->wtbl3_eid = wtbl_1_d4->field.wtbl3_eid;
		__entry->wtbl4_fid = wtbl_1_d3->field.wtbl4_fid;
		__entry->wtbl4_eid = wtbl_1_d4->field.wtbl4_eid;
		__entry->chk_per = wtbl_1_d3->field.chk_per;
		__entry->du_i_psm = wtbl_1_d3->field.du_i_psm;
		__entry->i_psm = wtbl_1_d3->field.i_psm;
		__entry->psm = wtbl_1_d3->field.psm;
		__entry->skip_tx = wtbl_1_d3->field.skip_tx;
		__entry->partial_aid = wtbl_1_d4->field.partial_aid;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   WTBL Segment 1 Fields:\n\
			   mac addr: %02X:%02X:%02X:%02X:%02X:%02X\n\
			   MUAR_Idx:%d\n\
			   rc_a1/rc_a2:%d/%d\n\
			   kid:%d\n\
			   rkv/rv:%d/%d\n\
			   sw:%d\n\
			   wm/mm:%d/%d\n\
			   cipher_suit:%d\n\
			   td/fd:%d/%d\n\
			   dis_rhtr:%d\n\
			   af:%d\n\
			   rx_ps:%d\n\
			   r:%d\n\
			   rts:%d\n\
			   cf_ack:%d\n\
			   rdg_ba:%d\n\
			   smps:%d\n\
			   baf_en:%d\n\
			   ht/vht/ldpc/dyn_bw:%d/%d/%d/%d\n\
			   TxBF(tibf/tebf):%d / %d\n\
			   txop_ps_cap:%d\n\
			   mesh:%d\n\
			   qos:%d\n\
			   adm:%d\n\
			   gid:%d\n\
			   wtbl2_fid:%d\n\
			   wtbl2_eid:%d\n\
			   wtbl3_fid:%d\n\
			   wtbl3_eid:%d\n\
			   wtbl4_fid:%d\n\
			   wtbl4_eid:%d\n\
			   chk_per:%d\n\
			   du_i_psm:%d\n\
			   i_psm:%d\n\
			   psm:%d\n\
			   skip_tx:%d\n\
			   partial_aid:%d\n",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->addr0,
				__entry->addr1,
				__entry->addr2,
				__entry->addr3,
				__entry->addr4,
				__entry->addr5,
				__entry->muar_idx,
				__entry->rc_a1, __entry->rc_a2,
				__entry->kid,
				__entry->rkv, __entry->rv,
				__entry->sw,
				__entry->wm, __entry->mm,
				__entry->cipher_suit,
				__entry->td, __entry->fd,
				__entry->dis_rhtr,
				__entry->af,
				__entry->rx_ps,
				__entry->r,
				__entry->rts,
				__entry->cf_ack,
				__entry->rdg_ba,
				__entry->smps,
				__entry->baf_en,
				__entry->ht, __entry->vht, __entry->ldpc, __entry->dyn_bw,
				__entry->tibf, __entry->tebf,
				__entry->txop_ps_cap,
				__entry->mesh,
				__entry->qos,
				__entry->adm,
				__entry->gid,
				__entry->wtbl2_fid,
				__entry->wtbl2_eid,
				__entry->wtbl3_fid,
				__entry->wtbl3_eid,
				__entry->wtbl4_fid,
				__entry->wtbl4_eid,
				__entry->chk_per,
				__entry->du_i_psm,
				__entry->i_psm,
				__entry->psm,
				__entry->skip_tx,
				__entry->partial_aid
	)
);


#define TRACE_CR_WTBL2_INFO trace_cr_wtbl2_info
TRACE_EVENT(cr_wtbl2_info,
	TP_PROTO(RTMP_ADAPTER *pAd, struct wtbl_2_struc *tb
	),

	TP_ARGS(pAd, tb
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, pn_0)
		__field(u32, pn_32)
		__field(u32, com_sn)
		__field(u32, tid_ac_0_sn)
		__field(u32, tid_ac_1_sn)
		__field(u32, tid_ac_2_sn)
		__field(u32, tid_ac_3_sn)
		__field(u32, tid_ac_4_sn)
		__field(u32, tid_ac_5_sn)
		__field(u32, tid_ac_6_sn)
		__field(u32, tid_ac_7_sn)
		__field(u32, rate_1_tx_cnt)
		__field(u32, rate_1_fail_cnt)
		__field(u32, rate_2_tx_cnt)
		__field(u32, rate_3_tx_cnt)
		__field(u32, rate_4_tx_cnt)
		__field(u32, rate_5_tx_cnt)
		__field(u32, current_bw_tx_cnt)
		__field(u32, current_bw_fail_cnt)
		__field(u32, other_bw_tx_cnt)
		__field(u32, other_bw_fail_cnt)
		__field(u32, fcap)
		__field(u32, rate_idx)
		__field(u32, cbrn)
		__field(u32, ccbw_sel)
		__field(u32, spe_en)
		__field(u32, mpdu_fail_cnt)
		__field(u32, mpdu_ok_cnt)
		__field(u32, g2)
		__field(u32, g4)
		__field(u32, g8)
		__field(u32, g16)
		__field(u32, rate_info_0)
		__field(u32, rate_info_1)
		__field(u32, rate_info_2)
		__field(u32, resp_rcpi_0)
		__field(u32, resp_rcpi_1)
		__field(u32, resp_rcpi_2)
		__field(u32, sts_1_ch_cap_noise)
		__field(u32, sts_2_ch_cap_noise)
		__field(u32, sts_3_ch_cap_noise)
		__field(u32, ce_rmsd)
		__field(u32, cc_noise_sel)
		__field(u32, ant_sel)
		__field(u32, ba_en)
		__field(u32, ba_size)
	),

	TP_fast_assign(
		union WTBL_2_DW0 *dw_0 = &tb->wtbl_2_d0;
		union WTBL_2_DW1 *dw_1 = &tb->wtbl_2_d1;
		union WTBL_2_DW2 *dw_2 = &tb->wtbl_2_d2;
		union WTBL_2_DW3 *dw_3 = &tb->wtbl_2_d3;
		union WTBL_2_DW4 *dw_4 = &tb->wtbl_2_d4;
		union WTBL_2_DW5 *dw_5 = &tb->wtbl_2_d5;
		union WTBL_2_DW6 *dw_6 = &tb->wtbl_2_d6;
		union WTBL_2_DW7 *dw_7 = &tb->wtbl_2_d7;
		union WTBL_2_DW8 *dw_8 = &tb->wtbl_2_d8;
		union WTBL_2_DW9 *dw_9 = &tb->wtbl_2_d9;
		union WTBL_2_DW10 *dw_10 = &tb->wtbl_2_d10;
		union WTBL_2_DW11 *dw_11 = &tb->wtbl_2_d11;
		union WTBL_2_DW12 *dw_12 = &tb->wtbl_2_d12;
		union WTBL_2_DW13 *dw_13 = &tb->wtbl_2_d13;
		union WTBL_2_DW14 *dw_14 = &tb->wtbl_2_d14;
		union WTBL_2_DW15 *dw_15 = &tb->wtbl_2_d15;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->pn_0 = dw_0->pn_0; 
		__entry->pn_32 = dw_1->field.pn_32;
		__entry->com_sn = dw_1->field.com_sn;
		__entry->tid_ac_0_sn = dw_2->field.tid_ac_0_sn;
		__entry->tid_ac_1_sn = dw_2->field.tid_ac_1_sn; 
		__entry->tid_ac_2_sn = dw_2->field.tid_ac_2_sn_0 | 
								(dw_3->field.tid_ac_2_sn_9 << 9);
		__entry->tid_ac_3_sn = dw_3->field.tid_ac_3_sn;
		__entry->tid_ac_4_sn = dw_3->field.tid_4_sn;
		__entry->tid_ac_5_sn = dw_3->field.tid_5_sn_0 | (dw_4->field.tid_5_sn_5 << 5);
		__entry->tid_ac_6_sn = dw_4->field.tid_6_sn;
		__entry->tid_ac_7_sn = dw_4->field.tid_7_sn;
		__entry->rate_1_tx_cnt = dw_5->field.rate_1_tx_cnt;
		__entry->rate_1_fail_cnt = dw_5->field.rate_1_fail_cnt;
		__entry->rate_2_tx_cnt = dw_6->field.rate_2_tx_cnt;
		__entry->rate_3_tx_cnt = dw_6->field.rate_3_tx_cnt;
		__entry->rate_4_tx_cnt = dw_6->field.rate_4_tx_cnt;
		__entry->rate_5_tx_cnt = dw_6->field.rate_5_tx_cnt;
		__entry->current_bw_tx_cnt = dw_7->field.current_bw_tx_cnt;
		__entry->current_bw_fail_cnt = dw_7->field.current_bw_fail_cnt;
		__entry->other_bw_tx_cnt = dw_8->field.other_bw_tx_cnt;
		__entry->other_bw_fail_cnt = dw_8->field.other_bw_fail_cnt;
		__entry->fcap = dw_9->field.fcap;
		__entry->rate_idx = dw_9->field.rate_idx;
		__entry->cbrn = dw_9->field.cbrn;
		__entry->ccbw_sel = dw_9->field.ccbw_sel;
		__entry->spe_en = dw_9->field.spe_en;
		__entry->mpdu_fail_cnt = dw_9->field.mpdu_fail_cnt; 
		__entry->mpdu_ok_cnt = dw_9->field.mpdu_ok_cnt;
		__entry->g2 = dw_9->field.g2;
		__entry->g4 = dw_9->field.g4;
		__entry->g8 = dw_9->field.g8;
		__entry->g16 = dw_9->field.g16;
		__entry->rate_info_0 = dw_10->word;
		__entry->rate_info_1 = dw_11->word;
		__entry->rate_info_2 = dw_12->word;
		__entry->resp_rcpi_0 = dw_13->field.resp_rcpi_0; 
		__entry->resp_rcpi_1 = dw_13->field.resp_rcpi_1;
		__entry->resp_rcpi_2 = dw_13->field.resp_rcpi_2;
		__entry->sts_1_ch_cap_noise = dw_14->field.sts_1_ch_cap_noise;
		__entry->sts_2_ch_cap_noise = dw_14->field.sts_2_ch_cap_noise;
		__entry->sts_3_ch_cap_noise = dw_14->field.sts_3_ch_cap_noise;
		__entry->ce_rmsd = dw_14->field.ce_rmsd;
		__entry->cc_noise_sel = dw_14->field.cc_noise_sel;
		__entry->ant_sel = dw_14->field.ant_sel;
		__entry->ba_en = dw_15->field.ba_en;
		__entry->ba_size = dw_15->field.ba_win_size_tid;			
	),
	
	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   WTBL Segment 2 Fields:\n\
			   PN_0-31:0x%x\n\
			   PN_32-48:0x%x\n\
			   SN(NonQos/Mgmt Frame):%d\n\
			   SN(TID0~7 QoS Frame):%d - %d - %d - %d - %d - %d - %d - %d\n\
			   TxRateCnt(1-5):%d(%d) - %d - %d - %d - %d\n\
			   TxBwCnt(Current-Other):%d(%d) - %d(%d)\n\
			   FreqCap:%d\n\
			   RateIdx/CBRN/CCBW_SEL/SPE_EN: %d/%d/%d/%d\n\
			   MpduCnt(Fail/OK):%d-%d\n\
			   TxRate Info: G2/G4/G8/G16=%d/%d/%d/%d\n\
			   TxRate Info: %d/%d/%d\n\
			   Resp_RCPI0/Resp_RCPI1/Resp_RCPI2=0x%x/0x%x/0x%x\n\
			   1CC(Noise)/2CC(Noise)/3CC(Noise)/CE_RMSD/CC_Sel/Ant_Sel=0x%x/0x%x/0x%x/0x%x/%d/%d\n\
			   BA Info: BA_En/BAWinSizeIdx(Range)\n\
			   %d/%d\n",
			  __entry->year,
			  __entry->mon,
			  __entry->day,
			  __entry->hour,
			  __entry->min,
		      __entry->sec,
			  __entry->pn_0,
			  __entry->pn_32,
			  __entry->com_sn,
			  __entry->tid_ac_0_sn,
			  __entry->tid_ac_1_sn,
			  __entry->tid_ac_2_sn,
			  __entry->tid_ac_3_sn,
			  __entry->tid_ac_4_sn,
			  __entry->tid_ac_5_sn,
			  __entry->tid_ac_6_sn,
			  __entry->tid_ac_7_sn,
			  __entry->rate_1_tx_cnt,
			  __entry->rate_1_fail_cnt,
			  __entry->rate_2_tx_cnt,
			  __entry->rate_3_tx_cnt,
			  __entry->rate_4_tx_cnt,
			  __entry->rate_5_tx_cnt,
			  __entry->current_bw_tx_cnt,
			  __entry->current_bw_fail_cnt,
			  __entry->other_bw_tx_cnt,
			  __entry->other_bw_fail_cnt,
			  __entry->fcap,
			  __entry->rate_idx,
			  __entry->cbrn,
			  __entry->ccbw_sel,
			  __entry->spe_en,
			  __entry->mpdu_fail_cnt,
			  __entry->mpdu_ok_cnt,
			  __entry->g2,
			  __entry->g4,
			  __entry->g8,
			  __entry->g16,
			  __entry->rate_info_0,
			  __entry->rate_info_1,
			  __entry->rate_info_2,
			  __entry->resp_rcpi_0,
			  __entry->resp_rcpi_1,
			  __entry->resp_rcpi_2,
			  __entry->sts_1_ch_cap_noise,
			  __entry->sts_2_ch_cap_noise,
			  __entry->sts_3_ch_cap_noise,
			  __entry->ce_rmsd,
			  __entry->cc_noise_sel,
			  __entry->ant_sel,
			  __entry->ba_en,
			  __entry->ba_size	  						
	)
);


#define TRACE_CR_TR_INFO trace_cr_tr_info
TRACE_EVENT(cr_tr_info,
	TP_PROTO(CHAR *function, UINT32 ring_reg, UINT32 base, UINT32 cnt, UINT32 cidx, UINT32 didx,
			 UINT32 swidx
	),

	TP_ARGS(function, ring_reg, base, cnt, cidx, didx, swidx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
		__field(u32, ring_reg)
		__field(u32, base)
		__field(u32, cnt)
		__field(u32, cidx)
		__field(u32, didx)
		__field(u32, swidx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__assign_str(func, function);
		__entry->ring_reg = ring_reg;
		__entry->base = base;
		__entry->cnt = cnt;
		__entry->cidx = cidx;
		__entry->didx = didx;
		__entry->swidx = swidx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Ring Name = %s\n\
			   Ring Reg = 0x%04x\n\
			   Base = 0x%08x\n\
			   Cnt = 0x%x\n\
			   CIDX = 0x%x\n\
			   DIDX = 0x%x\n\
			   SWIdx = 0x%x",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
				__get_str(func),
				__entry->ring_reg,
				__entry->base,
				__entry->cnt,
				__entry->cidx,
				__entry->didx,
				__entry->swidx
	)
);


#define TRACE_CR_INTERRUPT_INFO trace_cr_interrupt_info
TRACE_EVENT(cr_interrupt_info,
	TP_PROTO(UINT32 int_csr, UINT32 int_mask, UINT32 delay_int, UINT32 dma_conf, UINT32 tx_dma_en, 
				UINT32 rx_dma_en, UINT32 tx_dma_busy, UINT32 rx_dma_busy
	),

	TP_ARGS(int_csr, int_mask, delay_int, dma_conf, tx_dma_en, rx_dma_en, tx_dma_busy, rx_dma_busy
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, int_csr)
		__field(u32, int_mask)
		__field(u32, delay_int)
		__field(u32, dma_conf)
		__field(u32, tx_dma_en)
		__field(u32, rx_dma_en)
		__field(u32, tx_dma_busy)
		__field(u32, rx_dma_busy)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->int_csr = int_csr;
		__entry->int_mask = int_mask;
		__entry->delay_int = delay_int;
		__entry->dma_conf = dma_conf;
		__entry->tx_dma_en = tx_dma_en;
		__entry->rx_dma_en = rx_dma_en;
		__entry->tx_dma_busy = tx_dma_busy;
		__entry->rx_dma_busy = rx_dma_busy;
	),
	
	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   IntCSR = 0x%x\n\
			   IntMask = 0x%x\n\
			   DelayINT = 0x%x\n\
			   DMA Configuration = 0x%x\n\
			   Tx/RxDMAEn = %d %d\n\
			   Tx/RxDMABusy = %d %d",
			  	__entry->year,
			  	__entry->mon,
			  	__entry->day,
			  	__entry->hour,
			  	__entry->min,
		      	__entry->sec,
				__entry->int_csr,
				__entry->int_mask,
				__entry->delay_int,
				__entry->dma_conf,
				__entry->tx_dma_en,
				__entry->rx_dma_en,
				__entry->tx_dma_busy,
				__entry->rx_dma_busy
	)
);






#ifdef RTMP_PCI_SUPPORT
#define TRACE_PCI_TX_RING_IDX trace_pci_tx_ring_idx
TRACE_EVENT(pci_tx_ring_idx,
	TP_PROTO(CHAR *function, UINT32 queue_idx, UINT32 reg, UINT32 cpu_idx, UINT32 dma_idx, 
				UINT32 sw_free_idx
	),

	TP_ARGS(function, queue_idx, reg, cpu_idx, dma_idx, sw_free_idx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
		__field(u32, queue_idx)
		__field(u32, reg)
		__field(u32, cpu_idx)
		__field(u32, dma_idx)
		__field(u32, sw_free_idx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__assign_str(func, function);
		__entry->queue_idx = queue_idx;
		__entry->reg = reg;
		__entry->cpu_idx = cpu_idx;
		__entry->dma_idx = dma_idx;
		__entry->sw_free_idx = sw_free_idx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s\n\
			   queue_idx = 0x%x\n\
			   reg = 0x%x\n\
			   cpu_idx = 0x%x\n\
			   dma_idx = 0x%x\n\
			   sw_free_idx = 0x%x",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
			   __get_str(func),
			   __entry->queue_idx,
			   __entry->reg,
			   __entry->cpu_idx,
			   __entry->dma_idx,
			   __entry->sw_free_idx
	)
);
#endif


#define TRACE_PS_RETRIEVE_PACKET trace_ps_retrieve_packet
TRACE_EVENT(ps_retrieve_packet,
	TP_PROTO(UINT8 stage, UINT32 wlan_idx, UINT32 hdr_info, UINT16 padding, UINT32 entry_type, UINT16 ps_state, UINT8 qos_0, 
			UINT8 qos_1, INT32 ps_qbitmap, UINT32 ps_queue_number, UINT16 token_count
	),

	TP_ARGS(stage, wlan_idx, hdr_info, padding, entry_type, ps_state, qos_0, qos_1, ps_qbitmap, ps_queue_number, token_count
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, stage)
		__field(u32, wlan_idx)
		__field(u32, hdr_info)
		__field(u16, padding)
		__field(u32, entry_type)
		__field(u16, ps_state)
		__field(u8, qos_0)
		__field(u8, qos_1)
		__field(s32, ps_qbitmap)
		__field(u32, ps_queue_number)
		__field(u32, token_count)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->stage = stage;
		__entry->wlan_idx = wlan_idx;
		__entry->hdr_info = hdr_info;
		__entry->padding = padding;
		__entry->entry_type = entry_type;
		__entry->ps_state = ps_state;
		__entry->qos_0 = qos_0;
		__entry->qos_1 = qos_1;
		__entry->ps_qbitmap = ps_qbitmap;
		__entry->ps_queue_number = ps_queue_number;
		__entry->token_count = token_count;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   stage = %d\n\
			   wlan_idx = %d\n\
			   hdr_info = %d\n\
			   padding = %d\n\
			   entry_type = %d\n\
			   ps_state = %d\n\
			   qos_0 = %d\n\
			   qos_1 = %d\n\
			   ps_qbitmap = %d\n\
			   ps_queue_number = %d\n\
			   token_count = %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
		 	   __entry->stage,
			   __entry->wlan_idx,
			   __entry->hdr_info,
			   __entry->padding,
			   __entry->entry_type,
			   __entry->ps_state,
			   __entry->qos_0,
			   __entry->qos_1,
			   __entry->ps_qbitmap,
			   __entry->ps_queue_number,
			   __entry->token_count
	)
);

#define TRACE_PS_INDICATE trace_ps_indicate
TRACE_EVENT(ps_indicate,
	TP_PROTO(UINT8 Stage, UINT32 wcid, UINT8 old_psm, UINT8 new_psm, UINT8 tr_entry_ps_state
	),

	TP_ARGS(Stage, wcid, old_psm, new_psm, tr_entry_ps_state
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u32, wcid)
		__field(u8, old_psm)
		__field(u8, new_psm)
		__field(u8, tr_entry_ps_state)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->Stage = Stage;
		__entry->wcid = wcid;
		__entry->old_psm = old_psm;
		__entry->new_psm = new_psm;
		__entry->tr_entry_ps_state = tr_entry_ps_state;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   wcid = %d\n\
			   old_psm = %d\n\
			   new_state = %d\n\
			   tr_entry_ps_state = %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
				__entry->Stage,
				__entry->wcid,
				__entry->old_psm,
				__entry->new_psm,
				__entry->tr_entry_ps_state
	)
)

#define TRACE_PS_HANDLE_RX_PS_POLL trace_ps_handle_rx_ps_poll
TRACE_EVENT(ps_handle_rx_ps_poll,
	TP_PROTO(UINT8 Stage, UINT32 wcid, UINT8 is_active, UINT8 ps_state
	),

	TP_ARGS(Stage, wcid, is_active, ps_state

	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u32, wcid)
		__field(u8, is_active)
		__field(u8, ps_state)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->Stage = Stage;
		__entry->wcid = wcid;
		__entry->is_active;
		__entry->ps_state = ps_state;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   wcid = %d\n\
			   is_active = %d\n\
			   ps_state = %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
				__entry->Stage,
				__entry->wcid,
				__entry->is_active,
				__entry->ps_state
	)
)


#define TRACE_PS_INFO trace_ps_info
TRACE_EVENT(ps_info,
	TP_PROTO(UINT32 addr0,
			 UINT32 addr1,
			 UINT32 addr2,
			 UINT32 addr3,
			 UINT32 addr4,
			 UINT32 addr5,
			 UINT32 entry_type,
			 UINT32 aid,
			 UINT32 func_tb_idx,
			 UINT32 ps_mode,
			 UINT32 ps_state,
			 UINT32 i_psm,
			 UINT32 du_i_psm,
			 UINT32 skip_tx,
			 UINT32 pfg_force,
			 UINT32 pucport,
			 UINT32 pucqueue,
			 UINT32 total_pkt_number,
			 UINT32 ps_queue_number
	),

	TP_ARGS(addr0,
			addr1,
			addr2,
			addr3,
			addr4,
			addr5,
			entry_type,
			aid,
			func_tb_idx,
			ps_mode,
			ps_state,
			i_psm,
			du_i_psm,
			skip_tx,
			pfg_force,
			pucport,
			pucqueue,
			total_pkt_number,
			ps_queue_number
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, addr0)
		__field(u32, addr1)
		__field(u32, addr2)
		__field(u32, addr3)
		__field(u32, addr4)
		__field(u32, addr5)
		__field(u32, entry_type)
		__field(u32, aid)
		__field(u32, func_tb_idx)
		__field(u32, ps_mode)
		__field(u32, ps_state)
		__field(u32, i_psm)
		__field(u32, du_i_psm)
		__field(u32, skip_tx)
		__field(u32, pfg_force)
		__field(u32, pucport)
		__field(u32, pucqueue)
		__field(u32, total_pkt_number)
		__field(u32, ps_queue_number)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->addr0 = addr0;
		__entry->addr1 = addr1;
		__entry->addr2 = addr2;
		__entry->addr3 = addr3;
		__entry->addr4 = addr4;
		__entry->addr5 = addr5;
		__entry->entry_type = entry_type;
		__entry->aid = aid;
		__entry->func_tb_idx = func_tb_idx;
		__entry->ps_mode = ps_mode;
		__entry->ps_state = ps_state;
		__entry->i_psm = i_psm;
		__entry->du_i_psm = du_i_psm;
		__entry->skip_tx = skip_tx;
		__entry->pfg_force = pfg_force;
		__entry->pucport = pucport;
		__entry->pucqueue = pucqueue;
		__entry->total_pkt_number = total_pkt_number;
		__entry->ps_queue_number = ps_queue_number
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   	%02X:%02X:%02X:%02X:%02X:%02X\n\
			 	EntryType = %10x\n\
				Aid = %5d\n\
				func_tb_idx = %5d\n\
				PsMode = %5d\n\
				ps_state = %5d\n\
				i_psm = %5d\n\
				du_i_psm = %5d\n\
				skip_tx = %5d\n\
				pfgForce = %5d\n\
				pucPort = %5d\n\
				pucQueue = %5d\n\
				Total_Packet_Number = %6d\n\
				ps_queue.Number = %6d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
				__entry->addr0,
				__entry->addr1,
				__entry->addr2,
				__entry->addr3,
				__entry->addr4,
				__entry->addr5,
				__entry->entry_type,
				__entry->aid,
				__entry->func_tb_idx,
				__entry->ps_mode,
				__entry->ps_state,
				__entry->i_psm,
				__entry->du_i_psm,
				__entry->skip_tx,
				__entry->pfg_force,
				__entry->pucport,
				__entry->pucqueue,
				__entry->total_pkt_number,
				__entry->ps_queue_number
	)
);



INT32 TraceTRInfo(RTMP_ADAPTER *pAd);
VOID TraceCrPseInfo(RTMP_ADAPTER *pAd);
VOID TraceWtblInfo(RTMP_ADAPTER *pAd, UINT32 wtbl_idx);
INT32 TracePSTable(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli);

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace
#include <trace/define_trace.h>
