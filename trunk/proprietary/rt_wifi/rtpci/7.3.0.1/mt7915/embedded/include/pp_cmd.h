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
	pp_cmd.h
*/

#ifndef __CMM_PP_CMD_H__
#define __CMM_PP_CMD_H__

#ifdef CFG_SUPPORT_FALCON_PP
/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include "rt_config.h"
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    MACRO
 ******************************************************************************/

/*******************************************************************************
 *    TYPES
 ******************************************************************************/
typedef struct _PP_CMD_T {
	UINT_8	cmd_sub_id;
	UINT_8	dbdc_idx;
	UINT_8	pp_ctrl;
	UINT_8	pp_auto_mode;
} PP_CMD_T, *P_PP_CMD_T;

typedef enum _ENUM_PP_CTRL_T {
    /** SET **/
    PP_CTRL_PP_DIS = 0x0,
    PP_CTRL_PP_EN,
    PP_CTRL_SU_AUTOBW_EN,
    PP_CTRL_NUM
} ENUM_PP_CTRL_T, *P_ENUM_PP_CTRL_T;

/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/
NDIS_STATUS pp_mbss_init(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
NDIS_STATUS pp_profile_pp_en(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS set_pp_cap_ctrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS pp_cmd_cap_ctrl(IN PRTMP_ADAPTER pAd, IN P_PP_CMD_T pp_cmd_cap);
#endif				/* CFG_SUPPORT_FALCON_PP */
#endif				/* __CMM_PP_CMD_H__ */
