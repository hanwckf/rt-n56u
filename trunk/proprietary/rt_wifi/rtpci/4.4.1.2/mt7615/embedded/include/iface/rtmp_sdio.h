#ifdef MTK_LICENSE
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
	rtmp_sdio.h
*/
#endif /* MTK_LICENSE */
#ifndef __RTMP_SDIO_H__
#define __RTMP_SDIO_H__

#include "rt_config.h"
#include "mtsdio_io.h"

struct _RTMP_ADAPTER;
struct _TX_BLK;
struct wifi_dev;

USHORT MtSDIO_WriteSubTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT MtSDIO_WriteSingleTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT MtSDIO_WriteFragTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR fragNum, USHORT *freeCnt);
USHORT MtSDIO_WriteMultiTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR frmNum, USHORT *freeCnt);
VOID MtSDIO_FinalWriteTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, USHORT mpdu_len, USHORT TxIdx);

VOID MtSDIODataLastTxIdx(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, USHORT TxIdx);
UINT32 MtSDIOTxResourceRequest(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, UINT16 u2PgCnt, UINT32 req_cnt);
VOID MtSDIODataKickOut(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR QueIdx);
int MtSDIOMgmtKickOut(struct _RTMP_ADAPTER *pAd, UCHAR *pSrcBufVA, UINT SrcBufLen);
VOID MtSDIONullFrameKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, UCHAR *pNullFrm, UINT32 frmLen);

VOID MTSDIOBssBeaconExit(struct _RTMP_ADAPTER *pAd);
VOID MTSDIOBssBeaconStop(struct _RTMP_ADAPTER *pAd);
VOID MTSDIOBssBeaconStart(struct _RTMP_ADAPTER * pAd);
VOID MTSDIOBssBeaconInit(struct _RTMP_ADAPTER *pAd);


VOID
MtSdioSubmitDataToSDIOWorker(
        IN struct _RTMP_ADAPTER *prAd
);

VOID
MtSdioSubmitMgmtToSDIOWorker(
        IN struct _RTMP_ADAPTER *prAd,
        IN UCHAR ucQueIdx,
        IN PNDIS_PACKET prPacket,
        IN UCHAR *prSrcBufVA,
        IN UINT u4SrcBufLen
        );

#if CFG_SDIO_TX_AGG
VOID MtSDIOAggTXInit(struct _RTMP_ADAPTER *pAd);
VOID MtSDIOAggTXKickout(struct _RTMP_ADAPTER *pAd);
#endif


VOID SdioMlmeRadioOn(
	struct _RTMP_ADAPTER * pAd, struct wifi_dev *wdev);

VOID SdioMlmeRadioOFF(
	struct _RTMP_ADAPTER * pAd, struct wifi_dev *wdev);

/*from mtsdio_data.h*/
INT32 MTSDIOCmdTx(struct _RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length);
INT32 MTSDIODataTx(struct _RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length);
VOID MTSDIODataIsr(struct _RTMP_ADAPTER *pAd);
#if CFG_SDIO_RX_THREAD
INT MTSDIORxThread(IN ULONG ulContext);
#endif
INT MTSDIOWorkerThread(IN ULONG ulContext);

INT
MTSDIOCmdThread(
        IN ULONG ulContext
);
#if CFG_SDIO_RX_THREAD
VOID
MTSDIORxThreadEventNotify(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT32 u4EventBit
);
#endif
VOID
MTSDIOWorkerThreadEventNotify(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT32 u4EventBit
);

VOID
MTSDIOTxResetResource (
        IN struct _RTMP_ADAPTER *prAd
);

VOID MTSDIOTxInitResetResource (
        IN struct _RTMP_ADAPTER *prAd
);

UINT8
MTSDIOTxGetCmdResourceType(
        IN struct _RTMP_ADAPTER *prAd
);

INT32
MTSDIOTxAcquireResource(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT8       ucTC,
        IN UINT8       ucPageCount
);
ULONG
MTSDIOTxQueryPageResource(
        IN struct _RTMP_ADAPTER *prAd,
    IN UINT8       ucTC
);

UINT8
MTSDIOTxGetPageCount(
        IN UINT32 u4FrameLength,
        IN BOOLEAN fgIncludeDesc
);

BOOLEAN
MTSDIOTxReleaseResource(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT16       *au2TxRlsCnt
);

VOID
MTSDIOTxInitialize(
        IN struct _RTMP_ADAPTER *prAd
);
VOID
MTSDIORxInitialize(
        IN struct _RTMP_ADAPTER *prAd
);
VOID
MTSDIOTxUninitialize(
        IN struct _RTMP_ADAPTER *prAd
);
VOID
MTSDIORxUninitialize(
        IN struct _RTMP_ADAPTER *prAd
);

#if(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 0)
INT32 MTSDIORxReadBuffer(IN struct _RTMP_ADAPTER *prAd);
#elif(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 1)
INT32 MTSDIORxEnhancedReadBuffer(IN struct _RTMP_ADAPTER *prAd);
#elif(CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)
INT32 MTSDIORxEnhancedAggReadBuffer(IN struct _RTMP_ADAPTER *prAd);
#endif



/* <-------------- to be removed to another header file */
void MTSDIOProcessSoftwareInterrupt(IN struct _RTMP_ADAPTER * prAd);
void MTSDIOProcessAbnormalInterrupt(IN struct _RTMP_ADAPTER * prAd);
void MTSDIOProcessTxInterrupt(IN struct _RTMP_ADAPTER * prAd);
void MTSDIOProcessRxInterrupt(IN struct _RTMP_ADAPTER * prAd);

typedef struct _INT_EVENT_MAP_T {
    UINT32      u4Int;
    UINT32      u4Event;
} INT_EVENT_MAP_T, *P_INT_EVENT_MAP_T;

//typedef VOID (*IST_EVENT_FUNCTION)(PRTMP_ADAPTER);
typedef VOID (*IST_EVENT_FUNCTION)(struct _RTMP_ADAPTER *);

enum ENUM_INT_EVENT_T {
    INT_EVENT_ABNORMAL,
    INT_EVENT_SW_INT,
    INT_EVENT_TX,
    INT_EVENT_RX,
    INT_EVENT_NUM
};

#define PORT_INDEX_LMAC                         0
#define PORT_INDEX_MCU                          1

/* MCU quque index */
typedef enum _ENUM_MCU_Q_INDEX_T {
    MCU_Q0_INDEX = 0,
    MCU_Q1_INDEX,
    MCU_Q2_INDEX,
    MCU_Q3_INDEX,
    MCU_Q_NUM
} ENUM_MCU_Q_INDEX_T;

/* LMAC Tx queue index */
typedef enum _ENUM_MAC_TXQ_INDEX_T {
    MAC_TXQ_AC0_INDEX = 0,
    MAC_TXQ_AC1_INDEX,
    MAC_TXQ_AC2_INDEX,
    MAC_TXQ_AC3_INDEX,
    MAC_TXQ_AC4_INDEX,
    MAC_TXQ_AC5_INDEX,
    MAC_TXQ_AC6_INDEX,
    MAC_TXQ_BCN_INDEX,
    MAC_TXQ_BMC_INDEX,
    MAC_TXQ_AC10_INDEX,
    MAC_TXQ_AC11_INDEX,
    MAC_TXQ_AC12_INDEX,
    MAC_TXQ_AC13_INDEX,
    MAC_TXQ_AC14_INDEX,
    MAC_TXQ_NUM
} ENUM_MAC_TXQ_INDEX_T;


typedef struct _TX_RESOURCE_CONTROL_T {
    /* HW TX queue definition */
    UINT8      ucDestPortIndex;
    UINT8      ucDestQueueIndex;
    /* HIF Interrupt status index*/
    UINT8      ucHifTxQIndex;
} TX_RESOURCE_CONTROL_T, *PTX_RESOURCE_CONTROL_T;





#endif
