/*
 ***************************************************************************
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

    Module Name:
    twt_ctrl.h

    Abstract:
    Support twt hardware control

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#ifndef __TWT_CTRL_H__
#define __TWT_CTRL_H__

#define MTK_TWT_GROUP_EN			0
#define TWT_IFNO_FRAME_EN			0

/* TWT definitions for hw */
#define TWT_HW_AGRT_MAX_NUM			16
#if (MTK_TWT_GROUP_EN == 1)
#define TWT_HW_GRP_MAX_NUM			8
#else
#define TWT_HW_GRP_MAX_NUM			0
#endif
#define TWT_HW_GRP_MAX_MEMBER_CNT	8

#define TWT_TYPE_INDIVIDUAL			0
#define TWT_TYPE_GROUP				1

#define INVALID_TWT_HW_ID			0xff

/* max group grade */
#define MAX_GRP_GRADE				100

/* in unit of 256usec */
#define TWT_MAX_SP					255
#define TWT_SP_SPAN_TIME			((TWT_MAX_SP << 8) * TWT_HW_AGRT_MAX_NUM)

/* 16TU = 16*1024usec*/
#define TWT_TSF_ALIGNMNET_UINT		(16 * 1024)

#define SCH_LINK					0
#define USCH_LINK					1
#define SCH_LINK_NUM				2

#endif /* __TWT_CTRL_H__ */
