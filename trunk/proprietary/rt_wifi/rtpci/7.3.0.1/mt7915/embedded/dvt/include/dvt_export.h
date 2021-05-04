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

#ifndef __DVT_EXPORT_H__
#define __DVT_EXPORT_H__

struct dvt_framework;

INT dvt_feature_search(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID dvt_framework_exit(struct _RTMP_ADAPTER *ad, struct dvt_framework *dvt_ctrl);
#ifdef DOT11_HE_AX
INT dvt_enable_sta_he_test(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
#endif /* DOT11_HE_AX */

#endif
