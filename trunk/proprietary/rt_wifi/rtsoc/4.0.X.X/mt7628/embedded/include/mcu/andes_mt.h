/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	andes_mt.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_MT_H__
#define __ANDES_MT_H__

#include "mcu.h"
#include "mcu/mt_cmd.h"

#ifdef LINUX
#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif
#endif /* LINUX */

struct _RTMP_ADAPTER;
struct cmd_msg;


VOID AndesMTFillCmdHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt);
VOID AndesMTRxEventHandler(struct _RTMP_ADAPTER *pAd, UCHAR *data);
INT32 AndesMTLoadFw(struct _RTMP_ADAPTER *pAd);
INT32 AndesMTEraseFw(struct _RTMP_ADAPTER *pAd);

#ifdef RTMP_PCI_SUPPORT
INT32 AndesMTPciKickOutCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
#ifdef MT7615
INT32 AndesMTPciKickOutCmdMsg2(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
#endif /* MT7615 */
VOID AndesMTPciFwInit(struct _RTMP_ADAPTER *pAd);
VOID AndesMTPciFwExit(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_PCI_SUPPORT */



INT32 CmdInitAccessRegWrite(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 data);
INT32 CmdInitAccessRegRead(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data);
INT32 CmdChPrivilege(struct _RTMP_ADAPTER *pAd, UINT8 Action, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream);
INT32 CmdAccessRegWrite(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 data);
INT32 CmdAccessRegRead(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data);
INT32 CmdRFRegAccessWrite(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value);
INT32 CmdRFRegAccessRead(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value);
INT32 CmdRadioOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 On);
INT32 CmdWiFiRxDisable(struct _RTMP_ADAPTER *pAd, UINT RxDisable);
INT32 CmdPmStateCtrl(struct _RTMP_ADAPTER *pAd, UCHAR State, UCHAR Mode);
INT32 CmdChannelSwitch(struct _RTMP_ADAPTER *pAd, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream, BOOLEAN scan);
INT32 CmdNicCapability(struct _RTMP_ADAPTER *pAd);
INT32 CmdPsRetrieveReq(struct _RTMP_ADAPTER *pAd, UINT32 enable);

#ifdef MT_PS
INT32 CmdPsRetrieveStartReq(struct _RTMP_ADAPTER *pAd, UINT32 WlanIdx);
INT32 CmdPsClearReq(struct _RTMP_ADAPTER *pAd, UINT32 wlanidx, BOOLEAN p_wait);
#endif /* MT_PS */
INT32 CmdSecKeyReq(struct _RTMP_ADAPTER *pAd, UINT8 AddRemove, UINT8 Keytype, UINT8 *pAddr, UINT8 Alg, UINT8 KeyID, UINT8 KeyLen, UINT8 WlanIdx, UINT8 *KeyMaterial);
INT32 CmdRfTest(struct _RTMP_ADAPTER *pAd, UINT8 Action, UINT8 Mode, UINT8 CalItem);
INT32 CmdIcapOverLap(struct _RTMP_ADAPTER *pAd, UINT32 IcapLen);
NDIS_STATUS AndesMTLoadRomPatch(struct _RTMP_ADAPTER *ad);
INT32 AndesMTEraseRomPatch(struct _RTMP_ADAPTER *ad);
INT32 CmdMultipleMacRegAccessWrite(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdMultiPleMacRegAccessRead(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdMultipleRfRegAccessWrite(struct _RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdMultiPleRfRegAccessRead(struct _RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdFwLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 FWLog2HostCtrl);
VOID CmdIOWrite32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value);
VOID CmdIORead32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value);
VOID CmdEfusBufferModeSet(struct _RTMP_ADAPTER *pAd);
NTSTATUS CmdPowerOnWiFiSys(struct _RTMP_ADAPTER *pAd);
VOID CmdSetTxPowerCtrl(struct _RTMP_ADAPTER *pAd, UINT8 central_chl);
VOID CmdGetThermalSensorResult(struct _RTMP_ADAPTER *pAd, UINT8 ActionIdx,UINT32 *Result);
#ifdef LED_CONTROL_SUPPORT
INT AndesLedEnhanceOP(struct _RTMP_ADAPTER *pAd,UCHAR LedIdx,UCHAR on_time, UCHAR off_time, UCHAR led_parameter);
#endif

#ifdef MT7636_BTCOEX
INT AndesCoexOP(struct _RTMP_ADAPTER *pAd,  UCHAR Status);
INT AndesCoexProtectionFrameOP( struct _RTMP_ADAPTER *pAd, UCHAR Mode, UCHAR Rate);
INT AndesCoexBSSInfo( struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR bQoS);
#endif

#ifdef RTMP_EFUSE_SUPPORT
VOID CmdEfuseAccessRead(struct _RTMP_ADAPTER *pAd, USHORT offset,PUCHAR pData,PUINT isVaild);
VOID CmdEfuseAccessWrite(struct _RTMP_ADAPTER *pAd, USHORT offset,PUCHAR pData);
#endif /* RTMP_EFUSE_SUPPORT */
INT32 CmdThermalProtect(struct _RTMP_ADAPTER *ad, UINT8 HighEn, CHAR HighTempTh, UINT8 LowEn, CHAR LowTempTh);
INT32 CmdExtStaRecUpdate(struct _RTMP_ADAPTER *pAd, UINT8 ucBssIndex, UINT8 ucWlanIdx, UINT32 u4EnableFeature);

INT32 CmdObtwDelta(struct _RTMP_ADAPTER *ad, BOOLEAN anyEnable, UINT8 *ObtwDeltaArray);
INT32 CmdTmrCal(struct _RTMP_ADAPTER *pAd, UINT8 enable, UINT8 band, UINT8 bw, UINT8 ant, UINT8 role);

INT32 CmdEdcaParameterSet(struct _RTMP_ADAPTER *pAd, CMD_EDCA_SET_T EdcaParam);
INT32 CmdSlotTimeSet(struct _RTMP_ADAPTER *pAd, UINT8 SlotTime,UINT8 SifsTime,UINT8 RifsTime,UINT16 EifsTime);

#ifdef CONFIG_MULTI_CHANNEL
INT CmdMccStart(struct _RTMP_ADAPTER *ad,
                    u8 channel_1st,
                    u8 channel_2nd,
                    unsigned int bw_1st,
                    unsigned int bw_2nd,
                    u8 central_1st_seg0,
                    u8 central_1st_seg1,
                    u8 central_2nd_seg0,
                    u8 central_2nd_seg1,
                    u8 role_1st,
                    u8 role_2nd,
                    u16 stay_time_1st,
                    u16 stay_time_2nd,
                    u16 idle_time,
                    u16 null_repeat_cnt,
                    u32 start_tsf);

INT CmdMccStop(struct _RTMP_ADAPTER *ad, u8 parking_idx, u8 auto_resume_mode, u16 auto_resume_interval, u32 auto_resume_tsf);


#endif /* CONFIG_MULTI_CHANNEL */

#ifdef RTMP_FLASH_SUPPORT
INT32 CmdLoadDPDDataFromFlash(struct _RTMP_ADAPTER *pAd, UINT8 channel, UINT16 doReload);
#endif /*RTMP_FLASH_SUPPORT*/




#endif /* __ANDES_MT_H__ */

