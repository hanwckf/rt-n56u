/***************************************************************************
* MediaTek Inc.
* 4F, No. 2 Technology 5th Rd.
* Science-based Industrial Park
* Hsin-chu, Taiwan, R.O.C.
*
* (c) Copyright 1997-2019, MediaTek, Inc.
*
* All rights reserved. MediaTek source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of MediaTek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of MediaTek Technology, Inc. is obtained.
***************************************************************************

*/

#ifndef __DBG_TXCMD_EXPORT_H__
#define __DBG_TXCMD_EXPORT_H__
struct dbg_txcmd_framework;

INT dbg_txcmd_feature_search(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID dbg_txcmd_framework_exit(struct _RTMP_ADAPTER *ad, struct dbg_txcmd_framework *dvt_ctrl);

#endif

