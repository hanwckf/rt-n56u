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

#ifndef __LP_DVT_H__
#define __LP_DVT_H__

typedef enum _ENUM_SYSDVT_LP_TYPE_T {
	ENUM_SYSDVT_LP_TYPE_LMAC_OWN = 0,
	ENUM_SYSDVT_LP_TYPE_LMAC_OWNBACK,
} ENUM_SYSDVT_LP_TYPE_T;

/*export to features*/
VOID lp_dvt_init(struct dvt_framework *dvt_ctrl);

#endif
