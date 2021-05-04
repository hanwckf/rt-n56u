/*
  * Copyright (c) 2016 MediaTek Inc.  All rights reserved.
  *
  * This software is available to you under a choice of one of two
  * licenses.  You may choose to be licensed under the terms of the GNU
  * General Public License (GPL) Version 2, available from the file
  * COPYING in the main directory of this source tree, or the
  * BSD license below:
  *
  *     Redistribution and use in source and binary forms, with or
  *     without modification, are permitted provided that the following
  *     conditions are met:
  *
  *      - Redistributions of source code must retain the above
  *        copyright notice, this list of conditions and the following
  *        disclaimer.
  *
  *      - Redistributions in binary form must reproduce the above
  *        copyright notice, this list of conditions and the following
  *        disclaimer in the documentation and/or other materials
  *        provided with the distribution.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */

#ifndef _DIAG_H_
#define _DIAG_H_

#ifdef WIFI_DIAG

#include "rtmp_comm.h"
#include "rtmp_type.h"
#include "rtmp_os.h"
#include "rtmp.h"

typedef enum _ENUM_DIAG_CONN_ERROR_CODE{
	DIAG_CONN_FRAME_LOST = 0,
	DIAG_CONN_CAP_ERROR,
	DIAG_CONN_AUTH_FAIL,
	DIAG_CONN_ACL_BLK,
	DIAG_CONN_STA_LIM,
	DIAG_CONN_DEAUTH,
	DIAG_CONN_BAND_STE,
	DIAG_CONN_ERROR_MAX,
	DIAG_CONN_DEAUTH_COM
}ENUM_DIAG_CONN_ERROR_CODE;


void diag_conn_error(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR* addr,
	ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason);
void diag_conn_error_write(PRTMP_ADAPTER pAd);
void diag_add_pid(OS_TASK *pTask);
void diag_del_pid(OS_TASK *pTask);
void diag_get_process_info(PRTMP_ADAPTER	pAdapter, RTMP_IOCTL_INPUT_STRUCT	*wrq);
void diag_miniport_mm_request(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length);
void diag_bcn_tx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame,ULONG FrameLen );
void diag_log_file_write(PRTMP_ADAPTER pAd);
void diag_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void diag_dev_rx_cntl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void diag_ap_mlme_one_sec_proc(PRTMP_ADAPTER pAd);
void diag_ctrl_alloc(PRTMP_ADAPTER pAd);
void diag_ctrl_free(PRTMP_ADAPTER pAd);
BOOLEAN diag_proc_init(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
BOOLEAN diag_proc_exit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
#if defined(MT7663) || defined(MT7915)
void diag_get_snr(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR *pData);
#endif

#endif
#endif /* #ifndef _DIAG_H_ */

