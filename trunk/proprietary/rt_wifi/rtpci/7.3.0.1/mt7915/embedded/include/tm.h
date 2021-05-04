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

#ifndef __TM_H__
#define __TM_H__

enum tm_method {
	TASKLET_METHOD,
	TASKLET_NAPI_METHOD,
};

enum task_type {
	TX_DEQ_TASK,
	RX_DEQ_TASK,
	CMD_MSG_TASK,
};

enum tx_task_state {
	TX_DEQ_RUNNING,
	TX_PKT_PROCESSING,
};

/**
 * @init: tm resource initialization
 * @exit: tm resource exit
 * @schedule_task: schedule task to coressponding tm according to task type
 * @schedule_task_on: schedule task to coressponding tm and specific cpu according to task type
 */
struct tm_ops {
	INT(*init)(struct _RTMP_ADAPTER *pAd);
	INT(*exit)(struct _RTMP_ADAPTER *pAd);
	INT(*schedule_task)(struct _RTMP_ADAPTER *pAd, enum task_type type, UINT8 idx);
	INT(*schedule_task_on)(struct _RTMP_ADAPTER *pAd, INT cpu, enum task_type type, UINT8 idx);
} ____cacheline_aligned;

INT tm_init(struct _RTMP_ADAPTER *pAd);
INT tm_exit(struct _RTMP_ADAPTER *pAd);

#endif
