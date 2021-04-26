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
#include "rt_config.h"
#include "lp_dvt.h"
#include "framework_dvt.h"

#define DVT_MODNAME "LP"


static INT lp_dvt_1_basic_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request req;

	DVT_LOG("arg,%s", arg);

	req.feature_id = ENUM_SYSDVT_LP;
	req.type = os_str_tol(arg, 0, 10);
	req.len = 0;
	req.payload = NULL;
	req.resp_handle = NULL;
	req.resp_len = 0;

	dvt_ut_wmcu_send(ad, &req);

	return DVT_STATUS_OK;
}


static dvt_fun lp_dvt_table[] = {
	lp_dvt_1_basic_test,
};

static struct dvt_feature_entry lp_dvt = {
	.feature_name = "lp",
	.dvt_cnt = sizeof(lp_dvt_table)/sizeof(dvt_fun),
	.dvt_table = lp_dvt_table,
};

VOID lp_dvt_init(struct dvt_framework *dvt_ctrl)
{
	dvt_feature_register(dvt_ctrl, &lp_dvt);
	DVT_LOG("lp init,ok");
}
