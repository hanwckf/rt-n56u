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

	All ACM Related Structure & Definition, only used in ACM module

***************************************************************************/

#ifndef __ACM_COMM_H__
#define __ACM_COMM_H__


/* -------------------------------------------------------------------------

	ACM			Adimission Control Mechanism
	ACM_TC		ACM TSPEC
	ACM_TS		ACM TCLASS
	ACM_SM		ACM STREAM
	AMR			ACM MACRO

   ------------------------------------------------------------------------- */




/* ========================================================================== */
/* Definition */

/* ----- General ----- */
	/*
		1. maximum number of kept fail TSPECs;
		2. maximum retry number of delts frame;
		3. maximum request timeout of ADDTS Request frame;
		4. general timer timeout;
		5. default timeout when inactivity of a request TSPEC is 0;
		6. broadcast time interval of used ACM time;
		7. maximum backup ACM MBSS number;
		8. Other BSS ACM information timeout;
		9. CUmax;
		10.How long to do Channel monitor function;
		11.When to increase AIFSN of BE;
		12.Maximum increased/decreased AIFSN;
		13.When to decrease AIFSN of BE;
	*/
#define ACM_MAX_NUM_OF_FAIL_RSV_TSPEC		5
#define ACM_MAX_NUM_OF_DELTS_RETRY			5
#define ACM_ADDTS_REQUEST_TIMEOUT			5	/* unit: 1 second */

//#define ACM_TIMER_GENERAL_PERIOD_TIMEOUT	(1000000/ACM_JIFFIES_BASE) /* 1s */
#define ACM_TIMER_GENERAL_PERIOD_TIMEOUT	(1000) /* 1000ms */

#define ACM_WME_TSPEC_INACTIVITY_DEFAULT	((UINT32)120000000) /* 120s */

#define ACM_MBSS_BW_ANNONCE_TIMEOUT_NUM		60	/* 60s */
#define ACM_MBSS_BK_NUM						20	/* max 20 other BSS records */
#define ACM_MBSS_ENTRY_TIMEOUT				2	/* 2 * 60 = 120 second */

#define ACM_CH_MON_CUMAX					80	/* % */
#define ACM_CH_MON_TIMEOUT_NUM				1	/* 1s */
#define ACM_CH_MON_ADJUST_NUM				10	/* adjust AIFSN when CU >= 95%
												during continued 10 checks */
#define ACM_CH_MON_MAX_ADJUST				5	/* 5*20 = 100us */
#define ACM_CH_MON_RECOVER_NUM				20	/* adjust AIFSN when CU < 95%
												during continued 20 checks */

	/* others */
#define ACM_STREAM_CHECK_BASE				100000	/* unit: 100 mini second */
//#define ACM_STREAM_CHECK_OFFSET				(ACM_STREAM_CHECK_BASE/ACM_JIFFIES_BASE)
#define ACM_STREAM_CHECK_OFFSET				(ACM_STREAM_CHECK_BASE/1000)

#define ACM_SECOND_BASE						((UINT32)1000000) /* unit: micro sec */
#define ACM_TIME_BASE						((UINT32)1000000) /* unit: micro sec */

#define ACM_FLG_FUNC_SUPPORT				1
#define ACM_FLG_FUNC_NOT_SUPPORT			0

#define ACM_FLG_FUNC_ENABLED				1
#define ACM_FLG_FUNC_DISABLED				0

#define ACM_ETH_DA_ADDR_LEN					6
#define ACM_ETH_SA_ADDR_LEN					6
#define ACM_ETH_TYPE_LEN					2

#define ACM_ELM_ID_LEN_SIZE					2 /* size of ID + LENGTH field */

#define ACM_MAC_ADDR_LEN					6

#define ACM_UP_MAX							8 /* 0 ~ 7 */


/* ----- Rate & Time ----- */
/* for PLCP duration table access, Rate_Legacy[] & Rate_G[] */
#define ACM_RATE_ID_54M						0
#define ACM_RATE_ID_48M						1
#define ACM_RATE_ID_36M						2
#define	ACM_RATE_ID_24M						3
#define ACM_RATE_ID_18M						4
#define ACM_RATE_ID_12M						5
#define ACM_RATE_ID_9M						6
#define	ACM_RATE_ID_6M						7

#define ACM_RATE_ID_11M						0
#define ACM_RATE_ID_5_5M					1
#define ACM_RATE_ID_2M						2
#define	ACM_RATE_ID_1M						3

/* rate value (unit: 2) */
#define ACM_RATE_54M						0x6c	/* 108 */
#define ACM_RATE_48M						0x60	/* 96 */
#define ACM_RATE_36M						0x48	/* 72 */
#define ACM_RATE_24M						0x30	/* 48 */
#define ACM_RATE_18M						0x24	/* 36 */
#define ACM_RATE_12M						0x18	/* 24 */
#define ACM_RATE_9M							0x12	/* 18 */
#define ACM_RATE_6M							0x0c	/* 12 */
#define ACM_RATE_11M						0x16	/* 22 */
#define ACM_RATE_5_5M						0x0b	/* 11 */
#define ACM_RATE_2M							0x04	/* 4 */
#define ACM_RATE_1M							0x02	/* 2 */

#define ACM_RATE_B_NUM						4
#define ACM_RATE_G_NUM						8
#define ACM_RATE_MAXIMUM_BPS				((UINT32)(54000000))

/* for Pre-Time calculation use, ACM_TX_TimeCalPre() */
#define ACM_PRE_TIME_ID_1M					0
#define ACM_PRE_TIME_ID_2M					1
#define ACM_PRE_TIME_ID_5_5M				2
#define ACM_PRE_TIME_ID_11M					3
#define ACM_PRE_TIME_ID_6M					4
#define ACM_PRE_TIME_ID_9M					5
#define ACM_PRE_TIME_ID_12M					6
#define ACM_PRE_TIME_ID_18M					7
#define ACM_PRE_TIME_ID_24M					8
#define ACM_PRE_TIME_ID_36M					9
#define ACM_PRE_TIME_ID_48M					10
#define ACM_PRE_TIME_ID_54M					11

#define ACM_PRE_TIME_SPREAMBLE				0
#define ACM_PRE_TIME_LPREAMBLE				1

#define ACM_PRE_TIME_CTS_SELF				1
#define ACM_PRE_TIME_RTS_CTS				2
#define ACM_PRE_TIME_HEADER					3
#define ACM_PRE_TIME_ACK					4

/*
	In software ACM, we partition max packet size to
	0 ~ 31B, 32 ~ 63B, .....
	i.e. the tx time of 0B is same as the tx time of 31B.
*/
#define ACM_PRE_TIME_DATA_SIZE_OFFSET	5
#define ACM_PRE_TIME_DATA_SIZE_INTERVAL	(1<<ACM_PRE_TIME_DATA_SIZE_OFFSET)
#define ACM_PRE_TIME_DATA_SIZE_NUM		(ACMR_MAX_BUF_SIZE/ACM_PRE_TIME_DATA_SIZE_INTERVAL+1)

/* Time value, unit: micro second */
#define TIME_UNIT					1024
#define TIME_LONG_PREAMBLE			0xC0
#define TIME_SHORT_PREAMBLE			0x60
#define TIME_LONG_PREAMBLEx2		0x180
#define TIME_SHORT_PREAMBLEx2		0xC0
#define TIME_LONG_PREAMBLEx3		0x240
#define TIME_SHORT_PREAMBLEx3		0x120
#define TIME_PREAMBLE_11G			0x14
#define TIME_PREAMBLE_11Gx2			0x28
#define TIME_PREAMBLE_11Gx3			0x3C /* for 11g */
#define TIME_PREAMBLE_DIFF			(TIME_LONG_PREAMBLEx3 - TIME_SHORT_PREAMBLEx3)

/*
	Clause 17 (OFDM PHY for 5 GHz, defined in the 802.11a amendment)
		SIFS = 16us
	Clause 18 (HR/DSSS PHY for 2.4 GHz, defined in the 802.11b amendment)
		SIFS = 10us
	Clause 19 (Extended Rate PHY (ERP) specification, the 802.11g PHY layer)
		SIFS = 10us (no any Clause 18 station exists)
		SIFS = 16us (any Clause 18 station exists)
*/
#define TIME_SIFS					0x0A
#define TIME_SIFSx2					0x14
#define TIME_SIFSx3					0x1E
#define TIME_SIFSG					0x10 /* support Clause 18 STA exists */
#define TIME_SIFSGx2				0x20 /* support Clause 18 STA exists */
#define TIME_SIFSGx3				0x30 /* support Clause 18 STA exists */
#define TIME_SIFSA					0x10
#define TIME_SIFSAx2				0x20
#define TIME_SIFSAx3				0x30
#define TIME_ACK_1Mbps				0x70
#define TIME_ACK_1Mbpsx2			0xE0
#define TIME_ACK_2Mbpsx2			0x70

#define FRM_LENGTH_ACK				0x0E
#define FRM_LENGTH_ACKx2			0x1C
#define FRM_LENGTH_RTS				0x14

/* 11n related */
#define FRM_LENGTH_BLOCK_ACK		32		/* compressed BLOCK ACK */

/* aggregation related */
#define FRM_LENGTH_AGG_AMSDU_HDR	17
#define FRM_LENGTH_AGG_RAILNK_HDR	14


/* ----- QoS ----- */
#define ACM_DELTS_TIMEOUT			1000	/* unit: mini seconds */
#define ACM_TIMEOUT_CHECK_BASE		100		/* unit: mini seconds */
#define ACM_BANDWIDTH_CHECK_BASE	900000	/* unit: 900000 us */

#define ACM_QOS_SUBTYPE_DATA		0x8		/* 1000 */

#define ACM_QOS_CTRL_EOSP			0x0010	/* bit 4 */


/* ----- Reason Code ----- */
#define ACM_REASON_CODE_DISASSOC_UNSPECIFIED_FAILURE		32
#define ACM_REASON_CODE_DISASSOC_LACK_SUFFICIENT_BANDWIDTH	33
#define ACM_REASON_CODE_DISASSOC_POOR_CHANNEL_CONDITION		34
#define ACM_REASON_CODE_DISASSOC_OUTSIDE_TXOP_LIMIT			35


/* ----- Status Code ----- */
#define ACM_STATUS_CODE_SUCCESS								0
#define ACM_STATUS_CODE_UNSPECIFIED_FAILURE					32
#define ACM_STATUS_CODE_ASSOC_DENIED_INSUFFICIENT_BANDWIDTH	33
#define ACM_STATUS_CODE_ASSOC_DENIED_POOR_CONDITIONS		34
#define ACM_STATUS_CODE_ASSOC_DENIED_NOT_QOS_STATION		35
#define ACM_STATUS_CODE_DECLINED							37
#define ACM_STATUS_CODE_INVALID_PARAMETERS					38
#define ACM_STATUS_CODE_SUGGESTED_TSPEC						39
#define ACM_STATUS_CODE_MAYBE_AFTER_TS_DELAY				40
#define ACM_STATUS_CODE_DIRECT_LINK_IS_NOT_ALLOWED			41
#define ACM_STATUS_CODE_DEST_STA_IS_NOT_PRESENT				42
#define ACM_STATUS_CODE_DEST_STA_IS_NOT_A_QSTA				43

#define ACM_STATUS_CODE_WMM_SUCCESS							0
#define ACM_STATUS_CODE_WMM_INVALID_PARAMETERS				1
#define ACM_STATUS_CODE_WMM_REFUSED							3

/* these status are defined by RALINK */
#define ACM_STATUS_CODE_PRIVATE_ACM_DISABLED				255




/* ========================================================================== */
/* MACRO */

/* ----- General ----- */
#ifndef GNU_PACKED
#define GNU_PACKED	__attribute__ ((packed))
#endif // GNU_PACKED //

#ifdef ACM_MEMORY_TEST
#define ACM_MEM_DEBUG(__ID)												\
	printk("11e_msg> (%d) Alloc_Num = %d\n", __ID, gAcmMemAllocNum);	\
	printk("11e_msg> (%d) Free_Num  = %d\n", __ID, gAcmMemFreeNum);
#else
#define ACM_MEM_DEBUG()
#endif // ACM_MEMORY_TEST //


	/*	copy a TSPEC; */
	/*	check if the MAC is same */
#define AMR_IS_SAME_MAC(__SRC, __DST)	((*(__SRC) == *(__DST)) &&		\
									(*((__SRC)+1) == *((__DST)+1)) &&	\
									(*((__SRC)+2) == *((__DST)+2)) &&	\
									(*((__SRC)+3) == *((__DST)+3)) &&	\
									(*((__SRC)+4) == *((__DST)+4)) &&	\
									(*((__SRC)+5) == *((__DST)+5)))

	/*	check if the two CDB is the same */
#define AMR_IS_SAME_CDB(__pCdbSrc, __pCdbDst)							\
	AMR_IS_SAME_MAC((ACMR_CLIENT_MAC((__pCdbSrc))),						\
					(ACMR_CLIENT_MAC((__pCdbDst))))

	/*	check if the pointer address is same */
#define AMR_IS_SAME_POINTER(__SRC, __DST)	((ULONG)(__SRC) == (ULONG)(__DST))

	/*	get VLAN priority field */
#define ACM_VLAN_UP_GET(__FieldTCI)		(((__FieldTCI)>>ACM_VLAN_OFFSET) & 0x07)

	/* Note: 'return' will be put in these MACRO */
	/*	1. lock a TSPEC semaphore;
		2. unlock a TSPEC semaphore; */
#define ACM_TSPEC_SEM_LOCK_CHK_RTN(__pAd, __SplFlags, __LabelSemErr, __RtnValue)\
        do { if (ACMR_ADAPTER_DB == NULL) 								\
				return (__RtnValue);									\
			 ACMR_OS_SPIN_LOCK_BH(&((__pAd)->AcmTspecSemLock));			\
			 (__SplFlags) = 0;											\
			 if (0) goto __LabelSemErr; } while(0);

#define ACM_TSPEC_SEM_LOCK_CHK(__pAd, __SplFlags, __LabelSemErr)		\
        do { if (ACMR_ADAPTER_DB == NULL) 								\
				return;													\
			 ACMR_OS_SPIN_LOCK_BH(&((__pAd)->AcmTspecSemLock));			\
			 (__SplFlags) = 0;											\
             if (0) goto __LabelSemErr; } while(0);

#define ACM_TSPEC_SEM_LOCK(__pAd, __SplFlags, __LabelSemErr) 			\
        do { ACMR_OS_SPIN_LOCK_BH(&((__pAd)->AcmTspecSemLock));			\
			 (__SplFlags) = 0;											\
             if (0) goto __LabelSemErr; } while(0);

#define ACM_TSPEC_SEM_UNLOCK(__pAd, __LabelSemErr)						\
        do { ACMR_OS_SPIN_UNLOCK_BH(&((__pAd)->AcmTspecSemLock)); } while(0);

	/*	1. irq lock;
		2. irq unlock; */

#define ACM_TSPEC_IRQ_LOCK_CHK_RTN(__pAd, __SplFlags, __LabelSemErr, __RtnValue)\
        do { if (ACMR_ADAPTER_DB == NULL)									\
				return (__RtnValue);										\
			 ACMR_OS_SPIN_LOCK_BH(&((__pAd)->AcmTspecIrqLock));				\
			 (__SplFlags) = 0;												\
             if (0) goto __LabelSemErr; } while(0);

#define ACM_TSPEC_IRQ_LOCK_CHK(__pAd, __SplFlags, __LabelSemErr)			\
        do { if (ACMR_ADAPTER_DB == NULL)									\
				return;														\
			 ACMR_OS_SPIN_LOCK_BH(&((__pAd)->AcmTspecIrqLock));				\
			 (__SplFlags) = 0;												\
             if (0) goto __LabelSemErr; } while(0);

#define ACM_TSPEC_IRQ_LOCK(__pAd, __SplFlags, __LabelSemErr)				\
        do { ACMR_OS_SPIN_LOCK_BH(&((__pAd)->AcmTspecIrqLock));				\
			 (__SplFlags) = 0;												\
             if (0) goto __LabelSemErr; } while(0);

#define ACM_TSPEC_IRQ_UNLOCK(__pAd, __SplFlags, __LabelSemErr)				\
        do { ACMR_OS_SPIN_UNLOCK_BH(&((__pAd)->AcmTspecIrqLock)); } while(0);


/* ----- QoS ----- */
#define ACM_TID_GET(__QosCtrl)		((__QosCtrl) & 0x000F)
#define ACM_TCLAS_LEN_GET(__Type)	gTCLAS_Elm_Len[(__Type)]; /* length of TCLAS */


/* ----- TSPEC ------ */
/* send a DELTS frame or WME teardown frame */
#ifdef CONFIG_AP_SUPPORT
#define ACM_DELTS_SEND(__pAd, __pCdb, __pReq, __LabelSemErr)				\
	{																		\
		UCHAR *__pPktBuf; UINT32 __Len;										\
		ACM_TS_INFO __TsInfo;												\
		if (MlmeAllocateMemory((__pAd), &__pPktBuf) == NDIS_STATUS_SUCCESS)	\
		{																	\
			__Len = ACM_FrameDeltsToStaMakeUp((__pAd), (__pCdb), __pPktBuf, (__pReq));\
			ACMR_MEM_COPY(&__TsInfo, &(__pReq)->pTspec->TsInfo, sizeof(ACM_TS_INFO));\
			MiniportMMRequest((__pAd), 0, __pPktBuf, __Len);				\
			MlmeFreeMemory((__pAd), __pPktBuf);								\
			if (__pCdb->PsMode != PWR_SAVE)									\
			{																\
				ACMP_DeltsFrameACK((__pAd), ACMR_CLIENT_MAC((__pReq)->pCdb),\
									(UCHAR *)&(__pReq)->pTspec->TsInfo, 0);	\
				ACM_FrameBwAnnSend((__pAd), FALSE);							\
			}																\
		}																	\
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
#define ACM_DELTS_SEND(__pAd, __pCdb, __pReq, __LabelSemErr)				\
	{																		\
		UCHAR *__pPktBuf; UINT32 __Len;										\
		ACM_TS_INFO __TsInfo;												\
		if (MlmeAllocateMemory((__pAd), &__pPktBuf) == NDIS_STATUS_SUCCESS)	\
		{																	\
			__Len = ACM_FrameDeltsToApMakeUp((__pAd), (__pCdb), __pPktBuf, (__pReq));\
			ACMR_MEM_COPY(&__TsInfo, &(__pReq)->pTspec->TsInfo, sizeof(ACM_TS_INFO));\
			MiniportMMRequest((__pAd), 0, __pPktBuf, __Len);				\
			MlmeFreeMemory((__pAd), __pPktBuf);								\
			ACMP_DeltsFrameACK((__pAd), ACMR_AP_ADDR_GET(__pAd),			\
							(UCHAR *)&(__pReq)->pTspec->TsInfo, 0);			\
			ACM_FrameBwAnnSend((__pAd), FALSE);								\
		}																	\
	}
#endif // CONFIG_STA_SUPPORT //

/* free a Traffic Stream, include TSPEC, TCLAS, itself */
#define ACM_FREE_TS(__pStream)												\
	{																		\
		if ((__pStream) != NULL)											\
		{																	\
			UINT32 _ts_id;													\
			if ((__pStream)->pTspec != NULL) ACMR_MEM_FREE((__pStream)->pTspec);\
			for(_ts_id=0; _ts_id<ACM_TSPEC_TCLAS_MAX_NUM; _ts_id++)			\
			{																\
				if ((__pStream)->pTclas[_ts_id] != NULL)					\
					ACMR_MEM_FREE((__pStream)->pTclas[_ts_id]);				\
			}																\
			ACMR_MEM_FREE((__pStream));										\
		}																	\
	}


/*
	A TS is identified uniquely by its TID value within the context of
	the RA and TA, dont care about direction.
*/
/* In 11e spec. a TS is idnetified uniquely by its TID and direction */
#define ACM_IS_SAME_TSPEC(__pTspecSrc, __pTspecDst)							\
	((__pTspecSrc)->TsInfo.TSID == (__pTspecDst)->TsInfo.TSID)

#define ACM_IS_SAME_TS_INFO(__TsInfoSrc, __TsInfoDst)						\
	((__TsInfoSrc).TSID == (__TsInfoDst).TSID)

#define ACM_IS_SAME_TS_INFOP(__pTsInfoSrc, __pTsInfoDst)					\
	((__pTsInfoSrc)->TSID == (__pTsInfoDst)->TSID)

/*
	Same TS condition:
	1. if the direction of any TS is bidirectional link; or
		2.1 if the TSID is the same; or
		2.2 if the AC ID is the same;
	2. if the direction is the same; or
		2.1 if the TSID is the same; or
		2.2 if the AC ID is the same;
	3. if the TSID is the same;
*/
#define ACM_IS_SAME_TS(__TsidSrc, __TsidDst, __UpSrc, __UpDst, __DirSrc, __DirDst)\
	(((((__DirSrc) == ACM_DIRECTION_BIDIREC_LINK) ||						\
	   ((__DirDst) == ACM_DIRECTION_BIDIREC_LINK) ||						\
	   ((__DirSrc) == __DirDst)) &&											\
	  (((__TsidSrc) == (__TsidDst)) || 										\
	   (ACM_MR_EDCA_AC((__UpSrc)) == ACM_MR_EDCA_AC((__UpDst))))) ||		\
	(((__TsidSrc) == (__TsidDst))))

#define ACM_TSPEC_COPY(__pTspecDst, __pTspecSrc)							\
    (ACMR_MEM_COPY((UCHAR *)(__pTspecDst), (UCHAR *)(__pTspecSrc), sizeof(ACM_TSPEC)))

#define ACM_TCLAS_COPY(__pTclasDst, __pTclasSrc)							\
    (ACMR_MEM_COPY((UCHAR *)(__pTclasDst), (UCHAR *)(__pTclasSrc), sizeof(ACM_TCLAS)))

/* free all TSPECs in the TSPEC list */
#define ACM_LIST_ALL_FREE(__pAd, __pStreamList)								\
	{																		\
		ACM_STREAM *__pStream, *__pStreamNext;								\
		__pStream = (__pStreamList)->pHead;									\
		while(__pStream) {													\
			__pStreamNext = __pStream->pNext;								\
			ACM_TC_Free((__pAd), __pStream);								\
			__pStream = __pStreamNext; }									\
		(__pStreamList)->pHead = NULL;										\
		(__pStreamList)->pTail = NULL;										\
		(__pStreamList)->TspecNum = 0;										\
	}

#define ACM_LINK_NUM_INCREASE(__pAd, __AccessPolicy, __Dir)					\
	ACM_LinkNumCtrl((__pAd), (__AccessPolicy), (__Dir), 1);

#define ACM_LINK_NUM_DECREASE(__pAd, __AccessPolicy, __Dir)					\
	ACM_LinkNumCtrl((__pAd), (__AccessPolicy), (__Dir), 0);

/* count the number of TSPEC of the station */
#ifdef CONFIG_AP_SUPPORT
#define ACM_NUM_OF_TSPEC_RECOUNT(__pAd, __pCdb)								\
	{																		\
		if ((__pCdb) != NULL)												\
		{																	\
			ACM_ENTRY_INFO *__pStaAcmInfo;									\
			ACM_STREAM *__pStream;											\
			UINT32 __IdTidNum;												\
			__pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(__pCdb);				\
			__pCdb->ACM_NumOfTspecIn = 0;									\
			__pCdb->ACM_NumOfTspecOut = 0;									\
			ACMR_MEM_ZERO(__pCdb->ACM_NumOfInTspecInAc,						\
						sizeof(__pCdb->ACM_NumOfOutTspecInAc));				\
			ACMR_MEM_ZERO(__pCdb->ACM_NumOfOutTspecInAc,					\
						sizeof(__pCdb->ACM_NumOfOutTspecInAc));				\
			for(__IdTidNum=0; __IdTidNum<ACM_STA_TID_MAX_NUM; __IdTidNum++)	\
			{																\
				__pStream = (ACM_STREAM *)__pStaAcmInfo->pAcStmIn[__IdTidNum];	\
				if (__pStream != NULL)										\
				{															\
					__pCdb->ACM_NumOfTspecIn ++;							\
					__pCdb->ACM_NumOfInTspecInAc[__pStream->AcmAcId] ++;	\
				}															\
				__pStream = (ACM_STREAM *)__pStaAcmInfo->pAcStmOut[__IdTidNum];	\
				if (__pStream != NULL)										\
				{															\
					__pCdb->ACM_NumOfTspecOut ++;							\
					__pCdb->ACM_NumOfOutTspecInAc[__pStream->AcmAcId] ++;	\
				}															\
			}																\
		}																	\
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
#define ACM_NUM_OF_TSPEC_RECOUNT(__pAd, __pCdb)								\
	{																		\
		if ((__pCdb) != NULL)												\
		{																	\
			ACM_ENTRY_INFO *__pStaAcmInfo;									\
			ACM_STREAM *__pStream;											\
			UINT32 __IdTidNum;												\
			__pStaAcmInfo = ACMR_STA_ACM_PARAM_INFO(__pCdb);				\
			__pCdb->ACM_NumOfTspecIn = 0;									\
			__pCdb->ACM_NumOfTspecOut = 0;									\
			ACMR_MEM_ZERO(__pCdb->ACM_NumOfInTspecInAc,						\
						sizeof(__pCdb->ACM_NumOfOutTspecInAc));				\
			ACMR_MEM_ZERO(__pCdb->ACM_NumOfOutTspecInAc,					\
						sizeof(__pCdb->ACM_NumOfOutTspecInAc));				\
			for(__IdTidNum=0; __IdTidNum<ACM_STA_TID_MAX_NUM; __IdTidNum++)	\
			{																\
				__pStream = (ACM_STREAM *)__pStaAcmInfo->pAcStmIn[__IdTidNum];	\
				if (__pStream != NULL)										\
				{															\
					__pCdb->ACM_NumOfTspecIn ++;							\
					__pCdb->ACM_NumOfInTspecInAc[__pStream->AcmAcId] ++;	\
				}															\
				__pStream = (ACM_STREAM *)__pStaAcmInfo->pAcStmOut[__IdTidNum];	\
				if (__pStream != NULL)										\
				{															\
					__pCdb->ACM_NumOfTspecOut ++;							\
					__pCdb->ACM_NumOfOutTspecInAc[__pStream->AcmAcId] ++;	\
				}															\
			}																\
			ACMP_RetryCountCtrl(__pAd);										\
		}																	\
	}
#endif // CONFIG_STA_SUPPORT //


/* ----- ACTION ------ */
//#if defined(CONFIG_STA_SUPPORT_SIM) || defined(CONFIG_STA_SUPPORT)
/* send a ADDTS request frame to a peer */
#define ACM_ADDREQ_MAKEUP(__pAd, __pCdb, __pPktBuf, __Len, __pReq, __LabelSemErr)\
	{																		\
		if (MlmeAllocateMemory((__pAd), &(__pPktBuf)) == NDIS_STATUS_SUCCESS)\
		{																	\
			(__Len) = ACM_FrameAddtsReqMakeUp((__pAd), (__pCdb), (__pPktBuf), (__pReq));\
		}																	\
		else																\
			(__Len) = 0;													\
	}

#define ACM_ADDREQ_SEND(__pAd, __pPktBuf, __Len)							\
	{																		\
		MiniportMMRequest((__pAd), 0, (__pPktBuf), (__Len));				\
		MlmeFreeMemory((__pAd), (__pPktBuf));								\
	}

#ifdef CONFIG_AP_SUPPORT
/* send a ADDTS response frame frame to a peer */
#define ACM_ADDRSP_SEND(__pAd, __pPkt, __PktLen)							\
	{																		\
		MiniportMMRequest((__pAd), 0, (__pPkt), (__PktLen));				\
	}
#endif // CONFIG_AP_SUPPORT //




/* ========================================================================== */
/* Structure */

/* ----- IP ----- */
typedef struct _ACM_IPHDR {

#if defined(ACM_LITTLE_ENDIAN)
	UCHAR	IHL:4,
			Version:4;
#elif defined (ACM_BIG_ENDIAN)
	UCHAR	Version:4,
			IHL:4;
#else
#error "Please fix endian mode in acm_comm.h"
#endif

	UCHAR	TOS;
	UINT16	TOT_Len;
	UINT16	ID;
	UINT16	FragOff;
	UCHAR	TTL;
	UCHAR	Protocol;
	UCHAR	Check;
	UINT32	AddrSrc;
	UINT32	AddrDst;

	/* the options start here */
} ACM_IPHDR;


/* ----- Stream ----- */
typedef struct _ACM_STREAM {

	struct _ACM_STREAM *pPrev;	/* prev TSPEC */
	struct _ACM_STREAM *pNext;	/* next TSPEC */

	ACM_TSPEC	*pTspec;
	ACM_TCLAS	*pTclas[ACM_TCLAS_MAX_NUM];

#define ACM_TS_PROCESSING_NOT_EXIST	0xFF
	UCHAR  	TclasProcessing;

#define ACM_SM_TYPE_11E			0x00
#define ACM_SM_TYPE_WIFI		0x01
	UCHAR	StreamType;			/* only support WIFI WMM ACM currently */

	/*
		The up will be the user priority of TCLAS if TCLAS exists;
		or it will be the user priority of TS INFO
	*/
	UCHAR	UP;					/* user priority */
	UCHAR	Reserved1;

    /*
		For QAP mode, *pCdb is the associated client database;
		For QSTA mode, *pCdb is the client database.
	*/
#define ACM_STREAM_CDB_COPY(__pStream, __pCdb)								\
	(__pStream)->pCdb = (__pCdb);											\
	ACMR_MEM_MAC_COPY((__pStream)->StaMac, ACMR_CLIENT_MAC((__pCdb)));

	ACMR_STA_DB	*pCdb;			/* peer device database pointer */
	UCHAR	StaMac[6];			/* peer device MAC address */

#define ACM_TX_QUEUE_TYPE_NOT_EXIST			0xFFFFFFFF
	UINT32	TxQueueType;

	UINT32	Timeout;			/* request timeout */
	UINT16	DialogToken;		/* TSPEC ID */
	UINT16	Retry;				/* retry count */

#define ACM_TC_STATUS_REQUEST				0x0000 /* requesting */
#define ACM_TC_STATUS_ACTIVE				0x0001 /* active */
#define ACM_TC_STATUS_ACTIVE_SUSPENSION		0x0002 /* active but suspended */
#define ACM_TC_STATUS_REQ_DELETING			0x0003 /* deleting the request */
#define ACM_TC_STATUS_ACT_DELETING			0x0004 /* deleting the active */
#define ACM_TC_STATUS_RENEGOTIATING			0x0005 /* renegotiating */

#define ACM_TC_STATUS_HANDLING				0x0006 /* handle request */

#define ACM_TC_STATUS_FAIL					0x0007 /* active or request fail */
	UINT16	Status;				/* current status */

#define ACM_TC_CAUSE_UNKNOWN				0x0000 /* unknown cause */
#define ACM_TC_CAUSE_REQ_TIMEOUT			0x0001 /* fail due to request timeout */
#define ACM_TC_CAUSE_SUGGESTED_TSPEC		0x0002 /* fail due to suggested TSPEC */
#define ACM_TC_CAUSE_REJECTED				0x0003 /* rejected by QAP */
#define ACM_TC_CAUSE_UNKNOWN_STATUS			0x0004 /* unknown rsp status code */
#define ACM_TC_CAUSE_INACTIVITY_TIMEOUT		0x0005 /* inactivity timeout */
#define ACM_TC_CAUSE_DELETED_BY_QAP			0x0006 /* deleted by QAP */
#define ACM_TC_CAUSE_DELETED_BY_QSTA		0x0007 /* deleted by QSTA */
#define ACM_TC_CAUSE_BANDWIDTH				0x0008 /* lower priority deletion */
	UINT16	Cause;				/* cause that makes the stream to be stopped */
								/* valid only when status == TSPEC_STATUS_FAIL */

#define ACM_TC_PHY_TSID_DISABLE				0xFF
	UCHAR	AcmAcId;			/* physical TS ID or AC ID (ACM_MR_EDCA_AC(UP)) */

#define ACM_TC_TIMEOUT_ACTION_DELTS			0x00
#define ACM_TC_TIMEOUT_ACTION_ADDTS_REQ		0x01
	UCHAR	TimeoutAction;		/* action when timeout */

	UCHAR	ReNegotiation;		/* used in renegotiation TSPEC */
	UCHAR	FlgOutLink;			/* 1: in active table; 0: in client data base */
	UCHAR	Reserved2[3];		/* reserved field */

	UINT32	TimeoutAddts;		/* timeout of addts */
	UINT32	TimeoutDelts;		/* tiemout of delts */
	UINT32	InactivityCur, SuspensionCur;  /* variable, miscroseconds */

	/* software ACM use, unit: micro second */
	UINT32	AcmUsedTimeEnqueue;		/* used in packet enqueue */
	UINT64	TxTimestampMarkEnqueue;	/* used in packet enqueue */
	UINT32	AcmUsedTimeTransmit;	/* used in packet transmit */
	UINT64	TxTimestampMarkTransmit;/* used in packet transmit */

	/* minimum physical mode & mcs */
	UCHAR	PhyModeMin;
	UCHAR	McsMin;

	/* UAPSD */
	UCHAR	FlgUapsdHandleNeed;		/* 1: means we need to update UAPSD after
									ADDTS Response frame is sent out */

#ifdef ACM_CC_FUNC_11N
	/* Note: non-AMPDU is software level concept, not hardware level */
	UINT32	TxTimestampEnqueueHT;	/* timestamp for first non-AMPDU packet */
	UINT32	TxTimeEnqueueHT;		/* the tx time of first non-AMPDU packet */
	UINT32	TxAmpduNumEnqueueHT;	/* packet number in a AMPDU */

	UINT8	HT_BaWinSize;			/* the BA Window Size */
#endif // ACM_CC_FUNC_11N //

} ACM_STREAM;

/* peer station links */
typedef struct _ACM_PEER_DEV_LIST {

    struct _ACM_PEER_DEV_LIST	*pPrev;
    struct _ACM_PEER_DEV_LIST	*pNext;

	UCHAR	MAC[6]; 			/* the MAC of the connected device */

} ACM_PEER_DEV_LIST;

/* mbss used ACM time record */
#ifdef ACM_CC_FUNC_MBSS
typedef struct _ACM_MBSS_BW {

	UCHAR Identifier;						/* unique ID */
	UCHAR Channel;							/* BSS channel, 0: invalid */
	UCHAR BSSID[6];							/* BSSID, must not be all-zero */
	UCHAR Timeout;							/* used in timeout mechanism */
	UCHAR Reserved2;
	UINT32 UsedTime[ACM_DEV_NUM_OF_AC];		/* Used ACM time, unit: us */

} ACM_MBSS_BW;
#endif // ACM_CC_FUNC_MBSS //

#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_ACL
typedef struct _ACM_ACL_ENTRY {

	struct _ACM_ACL *pNext;

	UCHAR STA_MAC[ETH_ALEN];

} ACM_ACL_ENTRY;
#endif // ACM_CC_FUNC_ACL //
#endif // CONFIG_AP_SUPPORT //



/* ----- TSPEC request & active list ----- */
typedef struct _ACM_TSPEC_REQ_LIST {

	ACM_STREAM	*pHead;		/* points to head of requested TSPEC list */
	ACM_STREAM	*pTail;		/* points to tail of requested TSPEC list */

	UINT32	TspecNum;		/* the number of requested TSPEC */

} ACM_TSPEC_REQ_LIST;

typedef struct _ACM_TSPEC_ACT_LIST {

	ACM_STREAM	*pHead;		/* points to head of activated TSPEC list */
	ACM_STREAM	*pTail;		/* points to tail of activated TSPEC list */

#define ACM_NUM_OF_ACTIVE_LINK_FOR_A_AC		65535	/* for EDCA */
#define ACM_NUM_OF_ACTIVE_LINK_FOR_A_TS		1		/* for HCCA */
	UINT16 TspecNum;		/* number of TSPEC, 1 for TS, 65535 for AC */

    /* valid only for EDCA streams */
#define ACM_DIR_UP_OR_DL	0 /* uplink or direct link */
#define ACM_DIR_DL_OR_BI	1 /* dnlink or bidirectional link */
#define ACM_DIR_DL_OR_UP	2 /* direct link or uplink */
#define ACM_DIR_BI_OR_DN	3 /* bidirectional link or dnlink */
	UCHAR  Direction;

} ACM_TSPEC_ACT_LIST;


/* ----- QoS Element ----- */
typedef struct _ACM_ELM_TSPEC {

	UCHAR ElementId;
	UCHAR Length;

	ACM_TSPEC Tspec;

} GNU_PACKED ACM_ELM_TSPEC;

typedef struct _ACM_ELM_TCLAS {

	UCHAR ElementId;
	UCHAR Length;

	ACM_TCLAS Tclas;
} __attribute__ ((packed)) ACM_ELM_TCLAS;

typedef struct _ACM_ELM_TCLAS_PROCESSING {

	UCHAR ElementId;
	UCHAR Length;

#define ACM_TCLAS_PROCESSING_ALL		0
#define ACM_TCLAS_PROCESSING_ONE		1
	UCHAR Processing;
} __attribute__ ((packed)) ACM_ELM_TCLAS_PROCESSING;

typedef struct _ACM_ELM_TS_DELAY {

	UCHAR ElementId;
	UCHAR Length;

#define ACM_TS_DELAY_DISABLE	0xFFFFFFFF
#define ACM_TS_DELAY_NORMAL		1000
#define ACM_TS_DELAY_NEVER		0	/*	never to attempt setting up of a
										traffic stream */
	UINT32 Delay; /* unit: TUs, >= 0 */

} GNU_PACKED ACM_ELM_TS_DELAY;

typedef struct _ACM_ELM_QBSS_LOAD {

	UCHAR ElementId;
	UCHAR Length;

	/* the total number of STAs currently associated with this QBSS */
	UINT16 StationCount;

	/*	defined as the percentage of time, nomalized to 255, the QAP sensed the
		medium busy, as indicated by either the physical or virtual carrier sense
		mechanism.
		This percentage is computed using the formula:
			((channel busy time / (dot11ChannelUtilizationBeaconIntervals *
			dot11BeaconPeriod * 1024)) * 255) */
	UCHAR ChanUtil;

	/*	specifies the remaining amount of medium time available via explicit
		admission control, in units of 32 microsecond periods per 1 second. */
	UINT16 AvalAdmCap;

} GNU_PACKED ACM_ELM_QBSS_LOAD;


/* ----- ACM Frame ----- */
typedef struct _ACM_ADDTS_REQ_FRAME {

	/* 1: representing QoS */
	UCHAR Category;

	/*	0: ADDTS request;
		1: ADDTS response;
		2: DELTS;
		3: Schedule */
#define ACM_ACTION_REQUEST			0
#define ACM_ACTION_RESPONSE			1
#define ACM_ACTION_DELTS			2
#define ACM_ACTION_SCHEDULE			3
	UCHAR Action;

	/* TSPEC ID */
	UCHAR DialogToken;

	/* TSPEC */
	ACM_ELM_TSPEC ElmTspec;

	/* max 5 TCLASS & 1 TCLASS Processing or none */
    UCHAR pTclas[0];

} GNU_PACKED ACM_ADDTS_REQ_FRAME;

typedef struct _ACM_ADDTS_RSP_FRAME {

	/* 1: representing QoS */
	UCHAR Category;

	/* 1: ADDTS response */
	UCHAR Action;

	/* TSPEC ID */
	UCHAR DialogToken;

	/* status */
	UCHAR StatusCode;

	/* TS Delay, TSPEC, TCLASS, TCLASS Processing, Schedule elements */
	UCHAR pElm[0];
} GNU_PACKED ACM_ADDTS_RSP_FRAME;

typedef struct _ACM_DELTS_FRAME {

	/* 1: representing QoS */
	UCHAR Category;

	/* 2: DELTS */
	UCHAR Action;

	ACM_TS_INFO TsInfo;
} GNU_PACKED ACM_DELTS_FRAME;

typedef struct _ACM_QOS_INFO { /* 1B */

	UCHAR QACK:1;
	UCHAR FlgQueueReq:1;
	UCHAR FlgTxopReq:1;
	UCHAR MoreDataAck:1;
	UCHAR EdcaUpdateCount:4;
} GNU_PACKED ACM_QOS_INFO;

#ifdef ACM_CC_FUNC_MBSS
typedef struct _ACM_BW_ANN_FRAME {

	/* 1: representing QoS */
	UCHAR Category;

	/* 255: BANDWIDTH ANNOUNCE, ACM_ACTION_WME_BW_ANN */
	UCHAR Action;

	ACM_MBSS_BW MBSS; /* my bss related ACM information */
} GNU_PACKED ACM_BW_ANN_FRAME;
#endif // ACM_CC_FUNC_MBSS //


/* ----- ACM Control Block ----- */
/* ACM control parameters */
typedef struct _ACM_CTRL_PARAM {

	UINT16	StationCount;		/* station count of the BSS */
	UINT16	AvalAdmCap;			/* available admission capcability of the BSS */
	UINT16  ChanUtil;          	/* QBSS Load, 0 ~ 255 */
	UINT32  ChanBusyTime;  		/* unit: 1 micro second */

	/* current EDCA Parameter Set for AC0 ~ AC3 */
	/* must use UCHAR; or you need to modify ACMP_IsAnyACEnabled() */
	UCHAR   FlgAcmStatus[ACM_DEV_NUM_OF_AC];

	/* Downgrade function, shall not downgrade to ACM AC */
#define ACM_DOWNGRADE_DISABLE		0xFF
	UCHAR   DowngradeAcNum[ACM_DEV_NUM_OF_AC]; /* 0 ~ 3 */

	/* minimum contention period Tcp (content period) in a service interval */
	/* set these values by minimum service interval, dont change on the fly */
#define ACM_MIN_CP_NU_DEFAULT		90
#define ACM_MIN_CP_DE_DEFAULT		100
	UINT32  CP_MinNu, CP_MinDe;	/* % = nu/de */

	/* minimum AC0/AC1 bandwidth in a second */
	/* set these values by minimum service interval, dont change on the fly */
#define ACM_MIN_BEK_NU_DEFAULT		20
#define ACM_MIN_BEK_DE_DEFAULT		100
	UINT32  BEK_MinNu, BEK_MinDe;	/* % = nu/de */

	UINT32	AC10Time;			/* the time reserved for BE/BK traffic */

	/* total used ACM time, unit: microsecond */
	UINT32  AcmTotalTime;		/* includes dnlink + uplink medium time */

	/* for each AC, used to set ACM CSR register */
	/* uplink medium time is NOT included. unit: microsecond */
	UINT32  AcmOutTime[ACM_DEV_NUM_OF_AC];

	/* for each AC, include dnlink + uplink. unit: microsecond */
	UINT32  AcmAcTime[ACM_DEV_NUM_OF_AC];

	/* current link number */
	UINT32  LinkNumUp, LinkNumDn, LinkNumBi, LinkNumDi;

	/* channel utilization */
	UCHAR	FlgIsChanUtilEnable;		/* 1: enable channel utilization */

	/* tspec timeout handle */
	UCHAR	FlgIsTspecTimeoutEnable;	/* 1: enable TSPEC timeout */

#if defined(CONFIG_STA_SUPPORT_SIM) || defined(CONFIG_STA_SUPPORT)
	/* tspec uapsd handle */
	UCHAR	FlgIsTspecUpasdEnable;		/* 1: enable TSPEC UAPSD */
#endif // CONFIG_STA_SUPPORT //

	/* dynamic ATL */
	/* 1. the sum of all MAX BW must be equal to 100 */
	/* 2. MIN must <= MAX */
	UCHAR	FlgDatl; 			/* 1: enable DATL */

	/* Note: all MAX sum must be 100, unit: 1/100 */
#define ACM_DATL_BW_MAX_SUM			100
#define ACM_DATL_NO_BORROW			0xFF
#define ACM_DATL_BW_MIN_VO			30
#define ACM_DATL_BW_MAX_VO			40
#define ACM_DATL_BW_MIN_VI			30
#define ACM_DATL_BW_MAX_VI			40
#define ACM_DATL_BW_MIN_BE			10
#define ACM_DATL_BW_MAX_BE			10
#define ACM_DATL_BW_MIN_BK			10
#define ACM_DATL_BW_MAX_BK			10
	UCHAR	DatlBwMin[ACM_DEV_NUM_OF_AC];	/* unit: 1/100 */
	UCHAR	DatlBwMax[ACM_DEV_NUM_OF_AC];	/* unit: 1/100 */

	/* record which AC borrows bandwidth from which AC */
	/* ex: [0][1] + [0][2] + [0][3] = all borrowed bandwidth for AC0 */
	UINT32	DatlBorAcBw[ACM_DEV_NUM_OF_AC][ACM_DEV_NUM_OF_AC];

	/* Statistic count */
#ifdef ACM_CC_FUNC_STATS
#define ACM_STATS_COUNT_INC(__Cnt)		__Cnt ++;
#else
#define ACM_STATS_COUNT_INC(__Cnt)
#endif // ACM_CC_FUNC_STATS //
	ACM_STATISTICS	Stats;

} ACM_CTRL_PARAM;


/* ACM Control block */
typedef struct _ACM_CTRL_BLOCK {

/* EDCA parameters */
	ACM_CTRL_PARAM  EdcaCtrlParam; 		/* EDCA control parameters */

/* Global Variable (used in acm_edca.c & acm_comm.c) */
	/* controlled by other module to enable/disable TSPEC requests */
#define ACM_MR_TSPEC_ALLOW(pAd)			{ ACMR_CB->FlgTspecAllowed = 1; }
#define ACM_MR_TSPEC_DISALLOW(pAd)		{ ACMR_CB->FlgTspecAllowed = 0; }
#define ACM_MR_TSPEC_IS_ALLOWED(pAd)	((ACMR_CB->FlgTspecAllowed) == 1)
	BOOLEAN FlgTspecAllowed; 			/* 1: allow TSPEC request */

/* TSPEC request/deletion management */
	ACMR_OS_TIMER_STRUCT TimerTspecReqCheck;
#ifndef ACMR_HANDLE_IN_TIMER
	ACMR_OS_TASK_STRUCT TaskletTspecReqCheck;
#endif // ACMR_HANDLE_IN_TIMER //
	UCHAR FlgTspecReqCheckEnable;

/* STREAM activity/suspend management */
	ACMR_OS_TIMER_STRUCT TimerStreamAliveCheck;
#ifndef ACMR_HANDLE_IN_TIMER
	ACMR_OS_TASK_STRUCT TaskletStreamAliveCheck;
#endif // ACMR_HANDLE_IN_TIMER //
	UCHAR FlgStreamAliveCheckEnable;

/* WMM ACM Test Plan says DialogToken can not be 0 */
#define TSPEC_DIALOG_TOKEN_GET(__pAd, __Token)	\
{												\
	__Token = (ACMR_CB->DialogToken++);			\
	if (__Token == 0)							\
		__Token = (ACMR_CB->DialogToken++);		\
}

	UINT16 DialogToken;					/* unique TSPEC dialog token */

	ACM_TSPEC_REQ_LIST  TspecListReq;	/* all req TSPECs */
	ACM_TSPEC_REQ_LIST  TspecListFail;	/* all fail TSPECs, max 5 */

	/* we use the parameter to get device list to check all input links */
	/* it is a single linked list */
	ACM_PEER_DEV_LIST  *pDevListPeer;

/* Power save control */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN FlgPsIsAddtsReqSent;		/* 1: waiting for ADDTS response */
	UINT32	PsModeBackup;				/* backup old PS mode */
#endif // CONFIG_STA_SUPPORT //

/* General Timer */
	/* we have a general timer to do all time-related works */
	UCHAR FlgTimerGeneralEnable;
	ACMR_OS_TIMER_STRUCT TimerGeneral;
#ifndef ACMR_HANDLE_IN_TIMER
	ACMR_OS_TASK_STRUCT TaskletGeneral;
#endif // ACMR_HANDLE_IN_TIMER //

/* Multiple BSS */
#ifdef ACM_CC_FUNC_MBSS
	/* Note: both of QAP and QSTA must learn */
	UCHAR TimeoutMbssAcm;		/* used to check if timeout */

	/* Note: we will not send bw ann frame if dialog token is different; */
	UINT32 AcmTotalTimeOld;		/* my last backup total acm used time */

	UCHAR MbssIdentifier;		/* only used in QAP */
	UINT32 MbssTotalUsedTime;	/* total used time for all AC */
	UINT32 MbssAcUsedTime[ACM_DEV_NUM_OF_AC]; /* total used time for each AC */
	ACM_MBSS_BW	MBSS[ACM_MBSS_BK_NUM];
#endif // ACM_CC_FUNC_MBSS //

/* Channel Utilization Monitor */
#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_CHAN_UTIL_MONITOR
	UCHAR CU_MON_Timeout;		/* used to check if timeout */

#define ACM_CU_MON_MODE_RECOVER		0
#define ACM_CU_MON_MODE_ADJUST		1
	UCHAR CU_MON_FlgLastMode;

	UCHAR CU_MON_FlgChangeNeed;	/* 1: adjust or receover */

	UCHAR CU_MON_AifsnAp[ACM_DEV_NUM_OF_AC];	/* current adjusted AIFSN */
	UCHAR CU_MON_AifsnBss[ACM_DEV_NUM_OF_AC];	/* current adjusted AIFSN */

	/* if CU >= 95%, adjust_count ++; or adjust_count = 0 */
	/* adjust AIFNS (adjust_num ++) when adjust_count >= ACM_CH_MON_ADJUST_NUM */
	UCHAR CU_MON_AdjustCount;
	UCHAR CU_MON_AdjustNum;

	/* if CU < 95%, recover_count ++; or recover_count = 0 */
	/* recover AIFNS when recover_count >= ACM_CH_MON_RECOVER_NUM */
	UCHAR CU_MON_RecoverCount;
#endif // ACM_CC_FUNC_CHAN_UTIL_MONITOR //
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_ACL
	/* ACL list */
#define ACMR_ACL_ENABLE(__pAd, __FlgIsEnabled)							\
	((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_IsEnabled = __FlgIsEnabled;
#define ACM_MR_ACL_IS_ENABLED(pAd)										\
	((ACMR_CB->ACL_IsEnabled) == 1)

	BOOLEAN			ACL_IsEnabled;
	ACMR_LIST		ACL_List;
#endif // ACM_CC_FUNC_ACL //
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	/* Retry setting backup */
	/*
		No need in AP mode because AP is the BSS owner, it can send packets
		even total transmission time is larger than the medium time.
	*/
	UINT32	RetryCountOldSettings;	/* old setting for retry count */
#endif // CONFIG_STA_SUPPORT //
} ACM_CTRL_BLOCK;




/* ========================================================================== */
/* Function Prototype */

/* ----- ASIC settings Function ----- */
/* reset ACM setting in CSR */
STATIC VOID ACM_ASIC_ACM_Reset(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				AcId,
	ACM_PARAM_IN	UINT32				MediumTime);

/* enable channel busy time calculation */
STATIC VOID ACM_ASIC_ChanBusyEnable(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				FlgIsEnable);

/* get the channel busy time in last TBTT */
STATIC UINT32 ACM_ASIC_ChanBusyGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

/* reset EDCA Parameters in CSR */
STATIC VOID ACM_ASIC_EDCA_Reset(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				FlgIsDeltsAll);

/* get true time value, unit: microseconds */
STATIC UINT32 ACM_ASIC_TU_Translate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				TU);


/* ----- Other Function ----- */
/* reset UAPSD state */
STATIC VOID ACM_APSD_Ctrl(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				AcId,
	ACM_PARAM_IN	UCHAR				Direction,
	ACM_PARAM_IN	UCHAR				FlgTsAdd,
	ACM_PARAM_IN	UCHAR				FlgIsApsdEnable);

/* check enough bandwidth */
STATIC ACM_FUNC_STATUS ACM_BandwidthCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				AcId,
	ACM_PARAM_IN	UINT32				SI,
	ACM_PARAM_IN	UINT32				Policy,
	ACM_PARAM_IN	UINT32				Direction,
	ACM_PARAM_IN	UINT32				AcmTimeOld,
	ACM_PARAM_IN	UINT32				AcmTimeNew,
	ACM_PARAM_OUT	UINT32				*pTimeOffset,
	ACM_PARAM_OUT	UINT32				*pDatlAc,
	ACM_PARAM_OUT	UINT32				*pDatlBw);

/* check DATL bandwidth */
STATIC ACM_FUNC_STATUS ACM_DATL_Handle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				DatlAcId,
	ACM_PARAM_IN	UINT32				AcmTimeOld,
	ACM_PARAM_IN	UINT32				AcmTimeNew,
	ACM_PARAM_OUT	UINT32				*pDatlAc,
	ACM_PARAM_OUT	UINT32				*pDatlBw);

/* update DATL information */
STATIC VOID ACM_DATL_Update(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				StmAcId,
	ACM_PARAM_IN	UINT32				AcmTimeOld,
	ACM_PARAM_IN	UINT32				AcmTimeNew,
	ACM_PARAM_OUT	UINT32				DatlAcId,
	ACM_PARAM_OUT	UINT32				DatlBandwidth);

/* get extra data length for different entrypt mode */
STATIC UINT32 ACM_EncryptExtraLenGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb);

#ifdef CONFIG_STA_SUPPORT_SIM
/* make up a WME Setup Request frame to the QAP */
STATIC UINT32 ACM_FrameAddtsReqMakeUp(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pBufFrame,
	ACM_PARAM_IN	ACM_STREAM			*pReqNew);
#endif // CONFIG_STA_SUPPORT_SIM //

#ifdef CONFIG_STA_SUPPORT
/* make up a WME Teardown frame to the QAP */
STATIC UINT32 ACM_FrameDeltsToApMakeUp(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pBufFrame,
	ACM_PARAM_IN	ACM_STREAM			*pStream);
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
/* make up a WME Teardown frame to the QSTA */
STATIC UINT32 ACM_FrameDeltsToStaMakeUp(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pBufFrame,
	ACM_PARAM_IN	ACM_STREAM			*pStream);
#endif // CONFIG_AP_SUPPORT //

/* increase or decrease the link number counter for any stream */
STATIC VOID ACM_LinkNumCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				AccessPolicy,
	ACM_PARAM_IN	UINT32				Dir,
	ACM_PARAM_IN	UINT32				FlgIsAdd);

/* set the minimum PHY Mode and MCS to the packet */
STATIC VOID ACM_PacketPhyModeMCSSet(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* get the MAC address of next client data base */
STATIC ACM_FUNC_STATUS ACM_PeerDeviceMacGetNext(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN_OUT	UINT32				*pDevIndex,
	ACM_PARAM_IN		UCHAR				*pDevMac);

#ifdef CONFIG_STA_SUPPORT
/* change PS mode to ACTIVE mode */
STATIC VOID ACM_PS_ActiveOn(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);
#endif // CONFIG_STA_SUPPORT //

/* mapping current station rate to ACM rate */
STATIC UCHAR ACM_Rate_Mapping(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb);

/* get the output or input TSPEC array list of the device */
STATIC UCHAR **ACM_StationTspecListGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	BOOLEAN				FlgIsOutputLink);

/* get TSID from a packet */
STATIC UCHAR ACM_TSID_Get(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_MBUF			*pMbuf);

/* get physical transmit queue */
STATIC UINT32 ACM_TxQueueTypeGet(
	ACM_PARAM_IN	UCHAR				AcmAcId);


/* ----- Peer device management function ----- */
/* insert the peer device to the backup link list */
STATIC ACM_FUNC_STATUS ACM_PeerDeviceAdd(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac);

/* delete the peer device from the backup link list */
STATIC VOID ACM_PeerDeviceDel(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac);

/* get next the peer device from the backup link list */
STATIC ACM_FUNC_STATUS ACM_PeerDeviceGetNext(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_PEER_DEV_LIST	**ppDevicePeer,
	ACM_PARAM_IN	UCHAR				*pDevMac);

/* free all the peer device backup link list */
STATIC VOID ACM_PeerDeviceAllFree(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

/* maintain the peer device backup link list */
STATIC VOID ACM_PeerDeviceMaintain(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac);


/* ----- Stream Management Function ----- */
/* check whether a stream is timeout due to inactivity or suspendsion */
STATIC BOOLEAN ACM_STM_IdleCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* copy the stream information */
STATIC VOID ACM_STM_InfoCopy(
	ACM_PARAM_OUT	ACM_STREAM_INFO		*pStreamInfoDst,
	ACM_PARAM_IN	ACM_STREAM			*pStreamSrc);

/* STREAM inactivity & suspension timeout check task */
STATIC VOID ACM_TASK_STM_Check(
	ACM_PARAM_IN	ULONG				Data);


/* ----- 11e TSPEC Function ----- */
/* get the EDCA TSPECs in the peer database */
#define ACM_PEER_TSPEC_OUTPUT_GET		TRUE
#define ACM_PEER_TSPEC_INPUT_GET		FALSE

/* translate factor decimal part decimal to binary */
STATIC UINT32 ACM_SurplusFactorDecimalDec2Bin(
	ACM_PARAM_IN	UINT32				DEC);

/* active a requested TSPEC */
STATIC ACM_FUNC_STATUS ACM_TC_Active(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq);

/* remove a TSPEC from the active table */
STATIC VOID ACM_TC_ActRemove(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* delete a actived TSPEC and send out a DELTS frame */
STATIC BOOLEAN ACM_TC_Delete(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* move TSPEC active the the fail list */
STATIC VOID ACM_TC_Destroy(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq,
	ACM_PARAM_IN	BOOLEAN				FlgIsActiveExcluded);

/* move the failed TSPEC to the fail list or free it */
STATIC ACM_FUNC_STATUS ACM_TC_DestroyBy_TS_Info(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo,
	ACM_PARAM_IN	UCHAR				FlgIsFromSta);

/* free a stream and dont move the failed TSPEC to the fail list */
STATIC VOID ACM_TC_Discard(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* duplicate a TSPEC */
STATIC ACM_STREAM *ACM_TC_Duplicate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* find a stream */
STATIC ACM_STREAM *ACM_TC_Find(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo);

/* find a stream in the active table */
STATIC ACM_STREAM *ACM_TC_FindInAct(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo);

/* find a stream in the peer record */
STATIC ACM_STREAM *ACM_TC_FindInPeer(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo);

/* find a stream in the requested list */
STATIC ACM_STREAM *ACM_TC_FindInReq(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo);

/* free a TSPEC */
STATIC ACM_FUNC_STATUS ACM_TC_Free(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* get the user priority */
STATIC UCHAR ACM_TC_UP_Get(
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo,
	ACM_PARAM_IN	UINT32				TclasNum,
	ACM_PARAM_IN	ACM_TCLAS			*pTclas);

/* rearrange the requested TSPEC in the requested list */
STATIC VOID ACM_TC_Rearrange(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pReqNew,
	ACM_PARAM_IN	UINT16				Retry);

/* release all activated TSPECs without DELTS */
STATIC VOID ACM_TC_ReleaseAll(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

/* check same TSPEC */
STATIC ACM_FUNC_STATUS ACM_TC_RenegotiationCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pDevMac,
	ACM_PARAM_IN	UCHAR				UP,
	ACM_PARAM_IN	ACM_TS_INFO			*pTsInfo,
	ACM_PARAM_OUT	ACM_STREAM			**ppStreamIn,
	ACM_PARAM_OUT	ACM_STREAM			**ppStreamOut,
	ACM_PARAM_OUT	ACM_STREAM			**ppStreamDifAc);

/* change ADDTS state to DELTS state */
STATIC VOID ACM_TC_Req_ADDTS2DELTS(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq);

/* check whether another link or same link exists in the requested list */
STATIC ACM_FUNC_STATUS ACM_TC_ReqCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

/* free requested TSPEC for the device entry */
STATIC VOID ACM_TC_ReqDeviceFree(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
 	ACM_PARAM_IN	ACMR_STA_DB			*pCdb);

/* free all requested TSPEC */
STATIC VOID ACM_TC_ReqAllFree(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

/* insert a outgoing requested TSPEC to the requested list */
STATIC ACM_FUNC_STATUS ACM_TC_ReqInsert(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pReqNew);

/* remove a outgoing requested TSPEC from the requested list */
STATIC VOID ACM_TC_ReqRemove(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStreamReq);

/* activate periodically */
STATIC VOID ACM_TASK_General(
	ACM_PARAM_IN	ULONG				Data);

/* TSPEC request check task */
STATIC VOID ACM_TASK_TC_ReqCheck(
	ACM_PARAM_IN	ULONG				Data);

/* get IP information from the frame */
STATIC ACM_FUNC_STATUS ACM_TCLAS_IP_INFO_Get(
	ACM_PARAM_IN	UCHAR				*pPkt,
    ACM_PARAM_OUT	ACM_TCLAS			*pTclas);

/* get VLAN information from the frame */
STATIC ACM_FUNC_STATUS ACM_TCLAS_VLAN_INFO_Get(
	ACM_PARAM_IN	UCHAR				*pPkt,
    ACM_PARAM_OUT	UINT16				*pVlanTag);

/* ----- Channel Util Monitor ----- */
#ifdef CONFIG_AP_SUPPORT
#ifdef ACM_CC_FUNC_CHAN_UTIL_MONITOR
/* adjust AIFSN of non-ACM AC when channel utilization is too high */
STATIC VOID ACM_TC_TASK_CU_Mon(
	ACM_PARAM_IN	ULONG				Data);
#endif // ACM_CC_FUNC_CHAN_UTIL_MONITOR //
#endif // CONFIG_AP_SUPPORT //


/* ----- MBSS function ----- */
/* send a broadcast Bandwidth Annonce frame */
STATIC VOID ACM_FrameBwAnnSend(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				FlgIsForceToSent);

#ifdef ACM_CC_FUNC_MBSS
/* forward the bandwidth announce action frame */
STATIC VOID ACM_MBSS_BwAnnForward(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pMblk,
	ACM_PARAM_IN	UINT32				PktLen);

/* handle the bandwidth announce action frame from other BSS */
STATIC ACM_FUNC_STATUS ACM_MBSS_BwAnnHandle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UCHAR				*pActFrame,
	ACM_PARAM_IN	UINT32				PktLen);

/* re-calculate the used time for other BSS */
STATIC VOID ACM_MBSS_BwReCalculate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

/* broadcast our used bandwidth */
STATIC VOID ACM_TC_TASK_BwAnn(
	ACM_PARAM_IN	ULONG				Data);
#endif // ACM_CC_FUNC_MBSS //


/* ----- TX time function ----- */
/* calculate the QoS packet transmission time */
UINT32 ACM_TX_TimeCal(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UCHAR				RateIndex,
	ACM_PARAM_IN	UCHAR				FlgIsGmode,
	ACM_PARAM_IN	UCHAR				FlgIsCtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsRtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsSpreambleUsed,
	ACM_PARAM_IN	UCHAR				FlgIsNoAckUsed,
	ACM_PARAM_IN	UINT32				TxopLimit,
	ACM_PARAM_OUT	UINT32				*pTimeNoData,
	ACM_PARAM_OUT	UINT32				*pTimeHeader,
	ACM_PARAM_OUT	UINT32				*pTimeCtsSelf,
	ACM_PARAM_OUT	UINT32				*pTimeRtsCts,
	ACM_PARAM_OUT	UINT32				*pTimeAck);

/* calculate the QoS packet transmission time for HT rate */
UINT32 ACM_TX_TimeCalHT(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				McsId,
	ACM_PARAM_IN	UINT32				BwId,
	ACM_PARAM_IN	UINT32				GIId,
	ACM_PARAM_IN	BOOLEAN				FlgIsRtsEnable,
	ACM_PARAM_IN	BOOLEAN				FlgIsNoAckUsed,
	ACM_PARAM_IN	BOOLEAN				FlgIsAmpdu,
	ACM_PARAM_IN	UINT32				TxopLimit,
	ACM_PARAM_OUT	UINT32				*pTimeNoData,
	ACM_PARAM_OUT	UINT32				*pTimeHeader,
	ACM_PARAM_OUT	UINT32				*pTimeAck,
	ACM_PARAM_OUT	UINT32				*pTimeDataHdrOnly,
	ACM_PARAM_OUT	UINT32				*pTimeDataOnly);

/* calculate the QoS packet transmission time on the fly */
UINT32 ACM_TX_TimeCalOnFly(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UCHAR				RateIndex,
	ACM_PARAM_IN	UCHAR				FlgIsCtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsRtsEnable,
	ACM_PARAM_IN	UCHAR				FlgIsSpreambleUsed,
	ACM_PARAM_IN	UCHAR				FlgIsNoAckUsed);

#ifdef ACM_CC_FUNC_11N
/* calculate the QoS packet transmission time on the fly for HT rate */
UINT32 ACM_TX_TimeCalOnFlyHT(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB         *pCdb,
	ACM_PARAM_IN	ACM_STREAM			*pStream,
	ACM_PARAM_IN	UINT64				Timestamp,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				McsId,
	ACM_PARAM_IN	UCHAR				FlgIsNoAckUsed);
#endif // ACM_CC_FUNC_11N //

/* pre-Calculate the QoS packet transmission time */
VOID ACM_TX_TimeCalPre(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

/* calculate the frame body transmission time */
STATIC UINT16 ACM_TX_TimePlcpCal(
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				RateId,
	ACM_PARAM_IN	UCHAR				FlgIsGmode);

/* calculate the frame body transmission time for HT rate */
STATIC UINT16 ACM_TX_TimePlcpCalHT(
	ACM_PARAM_IN	UINT32	BodyLen,
	ACM_PARAM_IN	UINT32	McsId,
	ACM_PARAM_IN	UINT32	Nss,
	ACM_PARAM_IN	UINT32	Ness,
	ACM_PARAM_IN	BOOLEAN FlgIsGF,
	ACM_PARAM_IN	BOOLEAN	FlgIs2040,
	ACM_PARAM_IN	BOOLEAN	FlgIsSGI,
	ACM_PARAM_IN	BOOLEAN	FlgIsOnlyData);


#ifdef CONFIG_AP_SUPPORT
/* ======================= AP Function definition ======================= */

/* handle QoS action frame by QAP */
STATIC VOID ACM_ActionHandleByQAP(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pMblk,
	ACM_PARAM_IN	UINT32				PktLen,
	ACM_PARAM_IN	UINT32				PhyRate);

/* handle a EDCA or HCCA stream request from a QSTA */
STATIC ACM_FUNC_STATUS ACM_TC_ReqHandle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				StreamType,
	ACM_PARAM_IN	UINT16				DialogToken,
	ACM_PARAM_IN	ACM_TSPEC			*pTspec,
	ACM_PARAM_IN	UINT32				TclasNum,
	ACM_PARAM_IN	ACM_TCLAS			*pTclas[],
	ACM_PARAM_IN	UCHAR				TclasProcessing,
	ACM_PARAM_IN	UINT32				PhyRate,
	ACM_PARAM_OUT	UCHAR				*pStatusCode,
	ACM_PARAM_OUT	UINT16				*pMediumTime);

#endif // CONFIG_AP_SUPPORT //


#ifdef CONFIG_STA_SUPPORT
/* ======================= STA Function definition ======================= */

/* handle QoS action frame by QSTA */
STATIC VOID ACM_ActionHandleByQSTA(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pMblk,
	ACM_PARAM_IN	UINT32				PktLen);

/* handle a EDCA or HCCA stream response from the QAP */
STATIC ACM_FUNC_STATUS ACM_TC_RspHandle(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				DialogToken,
	ACM_PARAM_IN	UCHAR				StatusCode,
	ACM_PARAM_IN	ACM_TSPEC			*pTspec,
	ACM_PARAM_IN	ACM_ELM_TS_DELAY	*pTsDelay,
	ACM_PARAM_OUT	UCHAR				*pStatusCode);

#endif // CONFIG_STA_SUPPORT //


/* ======================= CMD Function definition ======================= */
VOID ACM_CMD_Init(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

VOID ACM_CMD_Release(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd);

#endif // __ACM_COMM_H__

/* End of acm_comm.h */

