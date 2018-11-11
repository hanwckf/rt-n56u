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
		__entry->Length = Length;
		__entry->PQID = PQID;
		__entry->CID = CID;
		__entry->PktTypeID = PktTypeID;
		__entry->SetQuery = SetQuery;
		__entry->SeqNum = SeqNum;
		__entry->ExtCID = ExtCID;
		__entry->ExtCIDOption = ExtCIDOption;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				length = 0x%x, pq_id = 0x%x, cid = 0x%x, pkt_type_id = 0x%x, set_query = 0x%x\
				seq_num = 0x%x, ext_cid = 0x%x, ext_cid_option = 0x%x",
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
			__entry->SetQuery,
			__entry->SeqNum,
			__entry->ExtCID,
			__entry->ExtCIDOption
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
		u32 ofs, pos = 0;
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


#define TRACE_TR_ENTRY trace_tr_entry
TRACE_EVENT(tr_entry,
	TP_PROTO(struct _STA_TR_ENTRY *tr_entry
	),

	TP_ARGS(tr_entry
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, entry_type)
		__field(u8, wdev_idx)
		__field(u8, wcid)
		__field(u8, func_tb_idx)
		__array(u8, add, MAC_ADDR_LEN) 		 
		__field(u16, non_qos_seq)
		__field(u16, qos_seq_tid_0)
		__field(u16, qos_seq_tid_1)
		__field(u16, qos_seq_tid_2)
		__field(u16, qos_seq_tid_3)
		__field(u16, qos_seq_tid_4)
		__field(u16, qos_seq_tid_5)
		__field(u16, qos_seq_tid_6)
		__field(u16, qos_seq_tid_7)
		__array(u8, bssid, MAC_ADDR_LEN)
		__field(u8, port_secured)
		__field(u8, ps_mode)
		__field(u8, cur_txrate)
		__field(u8, mpdu_density)
		__field(u8, max_rampdufactor)
		__field(u8, amsdu_size)
		__field(u8, mmps_mode)
		__field(u8, enq_cap)
		__field(u8, deq_cap)
		__field(u8, omac_idx)
		__field(u16, rx_ba_bitmap)
		__field(u16, tx_ba_bitmap)
		__field(u16, tx_auto_ba_bitmap)
		__field(u16, ba_decline_bitmap)
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
		__entry->entry_type = tr_entry->EntryType;
		__entry->wdev_idx = tr_entry->wdev->wdev_idx;
		__entry->wcid = tr_entry->wcid;
		__entry->func_tb_idx = tr_entry->func_tb_idx;
		memcpy(__entry->add, tr_entry->Addr, MAC_ADDR_LEN);
		__entry->non_qos_seq = tr_entry->NonQosDataSeq;
		__entry->qos_seq_tid_0 = tr_entry->TxSeq[0];
		__entry->qos_seq_tid_1 = tr_entry->TxSeq[1];
		__entry->qos_seq_tid_2 = tr_entry->TxSeq[2];
		__entry->qos_seq_tid_3 = tr_entry->TxSeq[3];
		__entry->qos_seq_tid_4 = tr_entry->TxSeq[4];
		__entry->qos_seq_tid_5 = tr_entry->TxSeq[5];
		__entry->qos_seq_tid_6 = tr_entry->TxSeq[6];
		__entry->qos_seq_tid_7 = tr_entry->TxSeq[7];
		memcpy(__entry->bssid, tr_entry->bssid, MAC_ADDR_LEN);
		__entry->port_secured = tr_entry->PortSecured;
		__entry->ps_mode = tr_entry->PsMode;
		__entry->cur_txrate = tr_entry->CurrTxRate;
		__entry->mpdu_density = tr_entry->MpduDensity;
		__entry->max_rampdufactor = tr_entry->MaxRAmpduFactor;
		__entry->amsdu_size = tr_entry->AMsduSize;
		__entry->mmps_mode = tr_entry->MmpsMode;
		__entry->enq_cap = tr_entry->enq_cap;
		__entry->deq_cap = tr_entry->deq_cap;
		__entry->omac_idx = tr_entry->OmacIdx;
		__entry->rx_ba_bitmap = tr_entry->RXBAbitmap;
		__entry->tx_ba_bitmap = tr_entry->TXBAbitmap;
		__entry->tx_auto_ba_bitmap = tr_entry->TXAutoBAbitmap;
		__entry->ba_decline_bitmap = tr_entry->BADeclineBitmap;
	),

	TP_printk("tr_entry: system Time = %04d-%02d-%02d %02d:%02d:%02d)\n",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec
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

#define TRACE_FP_BH trace_fp_bh
TRACE_EVENT(fp_bh,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
			   __get_str(func)
	)
);

#define TRACE_RX1_DONE_TASKLET trace_rx1_done_tasklet
TRACE_EVENT(rx1_done_tasklet,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
			   __get_str(func)
	)
);

#define TRACE_RX_DONE_TASKLET trace_rx_done_tasklet
TRACE_EVENT(rx_done_tasklet,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
		    	__entry->sec,
			   __get_str(func)
	)
);

#define TRACE_TX0_DATA trace_tx0_data
TRACE_EVENT(tx0_data,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#define TRACE_TX1_DATA trace_tx1_data
TRACE_EVENT(tx1_data,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#define TRACE_TX0_MGMT trace_tx0_mgmt
TRACE_EVENT(tx0_mgmt,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#define TRACE_TX1_MGMT trace_tx1_mgmt
TRACE_EVENT(tx1_mgmt,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#define TRACE_RX0_DATA trace_rx0_data
TRACE_EVENT(rx0_data,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#define TRACE_RX0_MGMT trace_rx0_mgmt
TRACE_EVENT(rx0_mgmt,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#define TRACE_RX0_CTRL trace_rx0_ctrl
TRACE_EVENT(rx0_ctrl,
	TP_PROTO(const CHAR *function
	),

	TP_ARGS(function
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
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
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func)
	)
);

#endif
INT32 TraceTRInfo(RTMP_ADAPTER *pAd);

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace
#include <trace/define_trace.h>
