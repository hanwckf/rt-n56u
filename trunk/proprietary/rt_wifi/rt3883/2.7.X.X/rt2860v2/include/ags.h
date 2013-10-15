/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/  
    
/****************************************************************************

	Abstract:

	All AGS (Adaptive Group Switching) Related Structure & Definition

***************************************************************************/ 
    
#ifndef __AGS_H__
#define __AGS_H__


extern UCHAR AGS1x1HTRateTable[];
extern UCHAR AGS2x2HTRateTable[];
extern UCHAR AGS3x3HTRateTable[];


#define AGS_TX_QUALITY_WORST_BOUND       8
    
/* The size, in bytes, of an AGS entry in the rate switch table */
#define SIZE_OF_AGS_RATE_TABLE_ENTRY	9
    

typedef struct _RTMP_TX_RATE_SWITCH_AGS {
	UCHAR	ItemNo;
	
#ifdef RT_BIG_ENDIAN
	UCHAR	Rsv2:2;
	UCHAR	Mode:2;
	UCHAR	Rsv1:1;	
	UCHAR	BW:1;
	UCHAR	ShortGI:1;
	UCHAR	STBC:1;
#else
	UCHAR	STBC:1;
	UCHAR	ShortGI:1;
	UCHAR	BW:1;
	UCHAR	Rsv1:1;
	UCHAR	Mode:2;
	UCHAR	Rsv2:2;
#endif /* RT_BIG_ENDIAN */

	UCHAR	CurrMCS;
	UCHAR	TrainUp;
	UCHAR	TrainDown;
	UCHAR	downMcs;
	UCHAR	upMcs3;
	UCHAR	upMcs2;
	UCHAR	upMcs1;
} RTMP_TX_RATE_SWITCH_AGS, *PRTMP_TX_RATE_SWITCH_AGS;


/* AGS control */
typedef struct _AGS_CONTROL {
	UCHAR MCSGroup; /* The MCS group under testing */
	UCHAR lastRateIdx;
} AGS_CONTROL,*PAGS_CONTROL;


/* The statistics information for AGS */
typedef struct _AGS_STATISTICS_INFO {
	CHAR	RSSI;
	ULONG	TxErrorRatio;
	ULONG	AccuTxTotalCnt;
	ULONG	TxTotalCnt;
	ULONG	TxSuccess;
	ULONG	TxRetransmit;
	ULONG	TxFailCount;
} AGS_STATISTICS_INFO, *PAGS_STATISTICS_INFO;


/* Support AGS (Adaptive Group Switching) */
#define SUPPORT_AGS(__pAd)			(IS_RT3593(__pAd))
#define AGS_IS_USING(__pAd, __pRateTable)			\
    (SUPPORT_AGS(__pAd) && \
     ((__pRateTable == AGS1x1HTRateTable) || \
      (__pRateTable == AGS2x2HTRateTable) || \
      (__pRateTable == AGS3x3HTRateTable))) 

#endif /* __AGS_H__ */
    
/* End of ags.h */ 
