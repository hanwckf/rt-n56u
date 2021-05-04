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
	txpwr.c
*/

/*******************************************************************************
 *    INCLUDED COMMON FILES
 ******************************************************************************/

#include "rt_config.h"

/*******************************************************************************
 *    INCLUDED EXTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *    INCLUDED INTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *   PRIVATE DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
INT32 MtCmdPwrLimitTblUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1Type,
	UINT8 u1ChannelBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel
)
{
	struct cmd_msg *msg;
	CMD_POWER_LIMIT_TABLE_CTRL_T rPwrLimitTblCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: u1Type: %d, u1BandIdx: %d, u1ChannelBand: %d, u1ControlChannel: %d, u1CentralChannel: %d\n",
			 __FUNCTION__, u1Type, u1BandIdx, u1ChannelBand, u1ControlChannel, u1CentralChannel));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rPwrLimitTblCtrl, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V1)
		rPwrLimitTblCtrl.u1PowerCtrlFormatId = POWER_LIMIT_TABLE_CTRL;
	rPwrLimitTblCtrl.u1PwrLimitType 	 = u1Type;
	rPwrLimitTblCtrl.u1BandIdx			 = u1BandIdx;

	if (cap->txpower_type == TX_POWER_TYPE_V1)
	/* Fill Power Limit Parameters to CMD payload */
		MtPwrFillLimitParam(pAd, u1ChannelBand, u1ControlChannel,
				u1CentralChannel, &rPwrLimitTblCtrl.uPwrLimitTbl, u1Type);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rPwrLimitTblCtrl,
						sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT32 MtCmdTxPwrUpCtrl(
	RTMP_ADAPTER *pAd,
	INT8          ucBandIdx,
	CHAR          cPwrUpCat,
	CHAR          cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM])
{
	struct cmd_msg *msg;
	CMD_POWER_BOOST_TABLE_CTRL_T TxPwrUpTblCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: ucBandIdx: %d, cPwrUpCat: %d\n",
		 __func__, ucBandIdx, cPwrUpCat));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: cPwrUpValue: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
		 __func__, cPwrUpValue[0], cPwrUpValue[1], cPwrUpValue[2],
		 cPwrUpValue[3], cPwrUpValue[4], cPwrUpValue[5],
		 cPwrUpValue[6], cPwrUpValue[7], cPwrUpValue[8],
		 cPwrUpValue[9], cPwrUpValue[10], cPwrUpValue[11]));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* init buffer structure */
	os_zero_mem(&TxPwrUpTblCtrl, sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		TxPwrUpTblCtrl.ucPowerCtrlFormatId = TXPOWER_UP_TABLE_CTRL_V0;
	else
		TxPwrUpTblCtrl.ucPowerCtrlFormatId = TXPOWER_UP_TABLE_CTRL_V1;
	TxPwrUpTblCtrl.ucBandIdx           = ucBandIdx;
	TxPwrUpTblCtrl.cPwrUpCat           = cPwrUpCat;

	/* update Power Up Table value to buffer structure */
	os_move_mem(TxPwrUpTblCtrl.cPwrUpValue, cPwrUpValue,
			sizeof(CHAR) * POWER_UP_CATEGORY_RATE_NUM);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&TxPwrUpTblCtrl,
		sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s:(ret = %d)\n", __func__, ret));

	return ret;
}
#endif /* TX_POWER_CONTROL_SUPPORT */

