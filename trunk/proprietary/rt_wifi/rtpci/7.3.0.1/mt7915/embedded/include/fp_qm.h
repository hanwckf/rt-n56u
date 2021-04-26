/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#ifndef __FP_QM_H__

enum {
	FP_QUE0,
	FP_QUE1,
};

struct fp_qm {
	UINT16 max_tx_process_cnt;
	UINT16 max_mgmt_que_num;
	UINT16 max_data_que_num;
	UINT16 extra_reserved_que_num;
};

struct fp_tx_flow_control {
	ULONG flag;
	ULONG *TxFlowBlockState;
	DL_LIST *TxBlockDevList;
};

VOID fp_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd, UINT8 idx);
VOID fp_rx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);

#endif


