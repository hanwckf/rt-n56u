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
	mcu.h

	Abstract:
	MCU related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __MCU_H__
#define __MCU_H__

enum MCU_TYPE {
	SWMCU,
	M8051,
	ANDES,
};


struct _RTMP_ADAPTER;

typedef void (*CMD_RSP_HANDLER)(struct _RTMP_ADAPTER *pAd, UCHAR *Data);

/*
 * CMD Unit (8051, Andes, ...,and etc)
 */
struct CMD_UNIT {
	union {
		struct {
			UCHAR Command;
			UCHAR Token;
			UCHAR Arg0;
			UCHAR Arg1;
		} MCU51;
		struct {
			UINT8 Type;
			USHORT CmdPayloadLen;
			PUCHAR CmdPayload;
			USHORT RspPayloadLen;
			PUCHAR RspPayload;
			ULONG Timeout;
			BOOLEAN NeedRsp;
			BOOLEAN NeedWait;
			CMD_RSP_HANDLER CmdRspHdler;
		} ANDES;
	} u;
};


struct MCU_CTRL {
	UCHAR CmdSeq;
	BOOLEAN IsFWReady;
	BOOLEAN bFwHasBeenLoaded;
	NDIS_SPIN_LOCK CmdRspEventListLock;
	DL_LIST CmdRspEventList;
};


struct CMD_RSP_EVENT {
	DL_LIST List;
	UCHAR CmdSeq;	
	UINT32 Timeout;
	BOOLEAN NeedWait;
#ifdef RTMP_PCI_SUPPORT
	BOOLEAN bAckDone;
#endif /* RTMP_PCI_SUPPORT */
	UCHAR **RspPayload;
	USHORT *RspPayloadLen;
};

VOID ChipOpsMCUHook(struct _RTMP_ADAPTER *pAd, enum MCU_TYPE MCUType);
VOID MCUCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID MCUCtrlExit(struct _RTMP_ADAPTER *pAd);

#endif 
