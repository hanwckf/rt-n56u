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
	mt_mac_ctrl.h
*/

#ifndef __MT_MAC_CTRL_H__
#define __MT_MAC_CTRL_H__

// TODO: shiang-usw,remove this
#define HW_BEACON_OFFSET		0x0200
// TODO: ---End


// TODO: shiang-usw, mark by shiang, need to remove it
typedef union GNU_PACKED _TXWI_STRUC {
	UINT32 word;
}TXWI_STRUC;

typedef union GNU_PACKED _RXWI_STRUC {
	UINT32 word;
}RXWI_STRUC;
// TODO: --End


#define TOTAL_PID_HASH_NUMS	0x20
typedef struct _TXS_CTL {
	/* TXS type hash table */
	MT_DL_LIST TxSType[TOTAL_PID_HASH_NUMS];
	NDIS_SPIN_LOCK TxSTypeLock[TOTAL_PID_HASH_NUMS];
	UINT64 TxS2McUStatus;
	UINT64 TxS2HostStatus;
	ULONG TxSFormat;
} TXS_CTL, *PTXS_CTL;

typedef struct _TXS_TYPE {
	MT_DL_LIST List;
	UINT32 Pid;
	UINT8 Format;
	INT32 (*TxSHandler)(struct _RTMP_ADAPTER *pAd, CHAR *Data);
	BOOLEAN DumpTxSReport;
} TXS_TYPE, *PTXS_TYPE;

typedef INT32 (*TXS_HANDLER)(struct _RTMP_ADAPTER *pAd, CHAR *Data);

struct _RTMP_ADAPTER;

INT32 InitTxSTypeTable(struct _RTMP_ADAPTER *pAd);
INT32 InitTxSCommonCallBack(struct _RTMP_ADAPTER *pAd);
INT32 ExitTxSTypeTable(struct _RTMP_ADAPTER *pAd);
INT32 AddTxSType(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, 
					TXS_HANDLER TxSHandler, BOOLEAN DumpTxSReport);
INT32 RemoveTxSType(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format);
INT32 TxSTypeCtl(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, 
									BOOLEAN TxS2Mcu, BOOLEAN TxS2Host);
INT32 ParseTxSPacket(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data);
INT32 BcnTxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data);
#ifdef CONFIG_STA_SUPPORT
INT32 NullFrameTxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data);
#endif //CFG_TDLS_SUPPORT


#endif /* __MT_MAC_CTRL_H__ */
