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

	All ACM Related Structure & Definition, used by other modules.

***************************************************************************/

#ifndef __ACM_EXTR_H__
#define __ACM_EXTR_H__

#define ACMR_HANDLE_IN_TIMER

#ifndef ACMR_HANDLE_IN_TIMER
//#define ACMR_OS_LINUX
//#define ACMR_OS_VXWORKS
#endif // ACMR_HANDLE_IN_TIMER //

/* Porting Guide:
	.	ACMR_xx definitions

	.	ACM_APSD_Ctrl()

	.	ACM_PacketPhyModeMCSSet()
	.	ACM_PeerDeviceMacGetNext()

	.	ACM_ADDREQ_MAKEUP
	.	ACM_ADDREQ_SEND
	.	ACM_ADDRSP_SEND
	.	ACM_DELTS_SEND

	.	ACM_FrameBwAnnSend()
	.	ACM_MBSS_BwAnnForward()
	.	ACMP_NonAcmAdjustParamUpdate()
	.	ACMP_UAPSD_StateUpdate()
	.	ACMP_BE_QueueFullHandle()
 */

/* -------------------------------------------------------------------------

	ACM			Adimission Control Mechanism
	ACM_CC		ACM Condition Compile

   ------------------------------------------------------------------------- */




/* ========================================================================== */
/* Just Comment (0815) */


#define ACM_TG_CMT_UAPSD_CHANGED_BY_TSPEC

#define ACM_TG_CMT_B0_TS_INFO

#define ACM_TG_CMT_MIN_PHY_RATE

#define ACM_TG_CMT_MAX_SP_LENGTH

#define ACM_TG_CMT_USED_TIME_PASS_CRITERIA

#define ACM_TG_CMT_UAPSD_SETTING_ON_NON_ACM_AC

#define ACM_TG_CMT_ACTION_FRAME_IN_PS_MODE

#define ACM_TG_CMT_WMMAC_SUPPORT_SIGNALLING

#define ACM_TG_CMT_SPEC_UNCLEAR_ON_RESERVED_FIELD

#define ACM_TG_CMT_NO_ACK_POLICY




/* ========================================================================== */
/* Definition */

#ifdef MODULE_WMM_ACM
	#define ACM_EXTERN
#else
	#define ACM_EXTERN extern
#endif // MODULE_WMM_ACM //


/* change it based on your requirements */
#define ACM_CC_OS_LINUX					/* LINUX OS */
#define ACM_CC_FUNC_WMM					/* WMM ACM function */
#define ACM_CC_FUNC_REPLACE_RULE_TG		/* Replacement rule from Task Group */
#define ACM_CC_FUNC_SPEC_CHANGE_TG		/* WMM Spec. Change from Task Group */
#define ACM_CC_FUNC_SOFT_ACM			/* WMM software ACM function */
#define ACM_CC_FUNC_BE_BW_CTRL			/* Protect BE with TSPEC (ACM of BE = 1) */
#define ACM_CC_FUNC_QUE_TX_CTRL			/* ACM for data queue and transmit */
#define ACM_CC_FUNC_AUX_ADMIT_TIME		/* Use timestamp to calculate admit time */

#ifdef DOT11_N_SUPPORT
#define ACM_CC_FUNC_11N					/* 11N support */
#endif // DOT11_N_SUPPORT //

//#define ACM_CC_FUNC_ACL				/* ACL function in AP mode */
//#define ACM_CC_FUNC_AUX_TX_TIME		/* Accurate tx time */
//#define ACM_CC_FUNC_STATS				/* ACM statistics count */
//#define ACM_CC_FUNC_MBSS				/* ACM in multiple BSS */
//#define ACM_CC_FUNC_CHAN_UTIL_MONITOR	/* Adjust non-ACM AC when CHAN busy */
//#define ACM_CC_FUNC_TCLAS				/* EDCA TCLAS function */
//#define ACM_CC_FUNC_HCCA				/* HCCA function (not yet finish) */

//#define ACM_CC_FUNC_PS_MGMT_FME		/* Send MGMT frame with PM=1 */

#ifdef CONFIG_AP_SUPPORT
#define CONFIG_STA_SUPPORT_SIM			/* Simulation (only test use) */
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
#define CONFIG_STA_SUPPORT_SIM			/* must define in station mode */
#endif // CONFIG_STA_SUPPORT //

#ifndef RT_BIG_ENDIAN
#define ACM_LITTLE_ENDIAN				/* CPU little endian mode */
#else
#define ACM_BIG_ENDIAN					/* CPU bit endian mode */
#endif // RT_BIG_ENDIAN //

#define ACM_MEMORY_TEST					/* Memory alloc/free debug */

/* ex: OS_HZ = 100 means 100 ticks in a second, 1 jiffies = 1000000/100us */
#ifdef LINUX
#define ACM_OS_TIME_BASE				(1000000/OS_HZ)
#endif // LINUX //




/* --------------------------- Type Redefinition --------------------------- */

//#define STATIC	static
#define STATIC


/* --------------------------- Function Call --------------------------- */
/* public QOS Return Status Code */
#define ACM_PARAM_IN
#define ACM_PARAM_OUT
#define ACM_PARAM_IN_OUT

#define ACM_FUNC_STATUS							INT32

#define ACM_RTN_OK								(0)
#define ACM_RTN_FAIL							(-1)
#define ACM_RTN_FATAL_ERR						(-2)
#define ACM_RTN_NO_FREE_TS						(-3)
#define ACM_RTN_ALLOC_ERR						(-4)
#define ACM_RTN_INSUFFICIENT_BD					(-5)
#define ACM_RTN_CAP_LIMIT_EXCEED				(-6)
#define ACM_RTN_NULL_POINTER					(-7)
#define ACM_RTN_INVALID_PARAM					(-8)
#define ACM_RTN_SEM_GET_ERR						(-9)
#define ACM_RTN_DISALLOW						(-10)
#define ACM_RTN_RENO_IN_REQ_LIST				(-11)
#define ACM_RTN_EXIST							(-12)
#define ACM_RTN_NOT_EXIST						(-13)
#define ACM_RTN_INSUFFICIENT_BD_BUT_DEL_AC		(-14)
#define ACM_RTN_TCLAS_IS_NEEDED					(-15)
#define ACM_RTN_NO_ACM							(-16)


/* --------------------------- Action Frame --------------------------- */
/* In WMMv1.1 these names are changed to ADDTS Req/Rsp DELTS */
#define ACM_CATEGORY_WME						17 /* WME Action */
#define ACM_ACTION_WME_SETUP_REQ				0
#define ACM_ACTION_WME_SETUP_RSP				1
#define ACM_ACTION_WME_TEAR_DOWN				2
#define ACM_ACTION_WME_BW_ANN					255

/* public qos element */
#define ACM_ELM_TOTAL_LEN	(ACM_ELM_QBSS_LOAD_LEN+2+ACM_ELM_WME_PARAM_LEN+2)

#define ACM_ELM_QBSS_LOAD_ID					11
#define ACM_ELM_QBSS_LOAD_LEN					5
#define ACM_ELM_TSPEC_ID						13
#define ACM_ELM_TSPEC_LEN						63
#define ACM_ELM_TCLAS_ID						14
/* TCLASS element length is variable */
#define ACM_ELM_TCLAS_PROCESSING_ID				44
#define ACM_ELM_TCLAS_PROCESSING_LEN			1
#define ACM_ELM_QOS_ACTION_ID					45
/* QOS ACTION element length is variable */
#define ACM_ELM_QOS_CAP_ID						46
#define ACM_ELM_QOS_CAP_LEN						1

/* public wme element */
#define ACM_ELM_WME_ID							221
#define ACM_ELM_WME_INFO_LEN					7
#define ACM_ELM_WME_PARAM_LEN					24
#define ACM_ELM_WME_TSPEC_LEN					61
#define ACM_ELM_WME_TCLAS_LEN					61
#define ACM_ELM_WME_TCLAS_PROCESSING_LEN		7

#define ACM_NOT_FRAME_BODY_LEN					(4 + 2 + ACM_ELM_WME_TSPEC_LEN)

#define ACM_WME_OUI_SUBTYPE_INFO				0x00
#define ACM_WME_OUI_SUBTYPE_PARAM				0x01
#define ACM_WME_OUI_SUBTYPE_TSPEC				0x02
#define ACM_WSM_OUI_SUBTYPE_QOS_CAP				0x05
#define ACM_WSM_OUI_SUBTYPE_TCLAS				0x06
#define ACM_WSM_OUI_SUBTYPE_TCLAS_PROCESSING	0x07
#define ACM_WSM_OUI_SUBTYPE_TS_DELAY			0x08
#define ACM_WSM_OUI_SUBTYPE_SCHEDULE			0x09

/* element ID, length, OUI, OUI Type, OUI Subtype, Version = 8B */
#define ACM_WME_ELM_OFFSET						8


/* --------------------------- Others --------------------------- */
/* STREAM Type (max 16) */
#define ACM_QOS_TYPE_LEGACY				0x01	/* no use */
#define ACM_QOS_TYPE_DELTS				0x02	/* use */
#define ACM_QOS_TYPE_ADDTS_REQ			0x03	/* no use */
#define ACM_QOS_TYPE_ADDTS_RSP			0x04	/* use */
#define ACM_QOS_TYPE_NULL				0x05	/* use */

/* STREAM Category */
#define ACM_SM_CATEGORY_REQ				0	/* requesting TSPEC */
#define ACM_SM_CATEGORY_ACT				1	/* active TSPEC */
#define ACM_SM_CATEGORY_PEER			2	/* a peer TSPEC */
#define ACM_SM_CATEGORY_ERR				3	/* failed TSPEC */

/* WMM Related */
#define ACM_STA_TSPEC_MAX_NUM			4	/* for UP or DN, total 4 + 4 = 8 */
											/* must >= ACM_STA_NUM_OF_AC */
#define ACM_TSPEC_TCLAS_MAX_NUM			5	/* max 5 TCLAS maps to a TSPEC */
#define ACM_DEV_NUM_OF_AC				4	/* AC0 ~ AC3 */
#define ACM_VO_ID						3	/* VO AC ID */
#define ACM_VI_ID						2	/* VI AC ID */
#define ACM_BK_ID						1	/* BK AC ID */
#define ACM_BE_ID						0	/* BE AC ID */
#define ACM_STA_TID_MAX_NUM				8	/* 0 ~ 7 for EDCA, dont change it */

/* Traffic Classify */
#define ACM_CLSFY_NOT_ALLOW				0xFFFFFFFF




/* ========================================================================== */
/* MACRO */

/* ------------------ MACRO: only for QAP or only for QSTA ------------------ */

#ifdef CONFIG_AP_SUPPORT

	/* get station count */
#define ACMR_STATION_COUNT_GET(__pAd)	MacTableAssocStaNumGet(__pAd)

	/* get current channel utilization */
#define ACMR_CHAN_UTIL_GET(__pAd)		pAd->QloadChanUtil

	/* get TXOP of the AC */
#define ACMR_TXOP_BSS_GET(__pAd, __AcId)	\
		((__pAd)->ApCfg.BssEdcaParm.Txop[(__AcId)])

	/* check if WMM is enabled */
#define ACMR_WMM_IS_ENABLED(__pAd)		(__pAd)->ApCfg.MBSSID[BSS0].bWmmCapable

	/* get my MAC */
#define ACMR_AP_ADDR_GET(__pAd)			(__pAd)->CurrentAddress

	/* get the device entry of MAC */
#define ACMR_STA_ENTRY_GET(__pAd, __pDevMac)	\
		MacTableLookup((__pAd), (__pDevMac))

	/* check if current mode is AP */
#define ACMR_IS_AP_MODE					1

	/* always 1 */
#define ACMR_IS_ASSOCIATED(__pAd)		1

	/* reset ACM flag of AC */
#define ACMR_AC_ACM_CTRL(__pAd, __Flg0, __Flg1, __Flg2, __Flg3)		\
		(__pAd)->ApCfg.BssEdcaParm.bACM[QID_AC_BE] = (__Flg0);		\
		(__pAd)->ApCfg.BssEdcaParm.bACM[QID_AC_BK] = (__Flg1);		\
		(__pAd)->ApCfg.BssEdcaParm.bACM[QID_AC_VI] = (__Flg2);		\
		(__pAd)->ApCfg.BssEdcaParm.bACM[QID_AC_VO] = (__Flg3);		\
		(__pAd)->ApCfg.BssEdcaParm.EdcaUpdateCount ++;

	/* check if preamble is short */
#define ACMR_STA_IS_SPREAMBLE(__pAd, __pCdb)	\
		(CAP_IS_SHORT_PREAMBLE_ON((__pCdb)->CapabilityInfo))

	/* nothing to do */
#define ACMR_UAPSD_BACKUP(__pAd)

	/* get current channel number */
#define ACMR_CHAN_GET(__pAd)						(__pAd)->CommonCfg.Channel

	/* get current channel utilization */
#define ACMR_CHAN_UTILIZATION_GET(__pAd)			(__pAd)->QloadChanUtil

	/* update new available ACM time */
#define ACMR_AVAIL_ACM_TIME_UPDATE(__pAd, __Time)		        		\
		(__pAd)->AcmAvalCap = (UINT16)(ACM_TIME_BASE >> 5);				\
		(__pAd)->AcmAvalCap -= (UINT16)((__Time)>>5);

	/* get original settings for AIFSN */
#define ACMR_AIFSN_DEFAULT_GET(__pAd, __AifsnAp, __AifsnBss)				\
		ACMR_MEM_COPY((__AifsnAp), (__pAd)->CommonCfg.APEdcaParm.Aifsn, 4);	\
		ACMR_MEM_COPY((__AifsnBss), (__pAd)->ApCfg.BssEdcaParm.Aifsn, 4);

	/* get maximum supported rate */
#ifdef ACM_CC_FUNC_11N
#define ACMR_SUP_RATE_MAX_GET(__pAd, __pCdb, __MaxRate)					\
		{																\
			RT28XX_IOCTL_MaxRateGet(									\
							__pAd, &__pCdb->MaxHTPhyMode, &__MaxRate);	\
		};
#else

#define ACMR_SUP_RATE_MAX_GET(__pAd, __pCdb, __MaxRate)					\
		{																\
			RT28XX_IOCTL_MaxRateGet(									\
							__pAd, &__pCdb->MaxHTPhyMode, &__MaxRate);	\
			if (__MaxRate > 54000000)									\
				__MaxRate = 54000000;									\
		};
#endif // ACM_CC_FUNC_11N //

	/* check if the rate is a supported rate for non-11n */
#define ACMR_SUP_RATE_CHECK(__pAd, __Rate, __FlgIsSupRate)				\
		{																\
			UCHAR __RateIndex = ((__Rate) / 1000000) << 1;				\
			UCHAR __IdRate, __RateLen;									\
			UCHAR *__pRateSup, *__pRateExt;								\
			__FlgIsSupRate = 0;											\
			__RateLen = (__pAd)->CommonCfg.SupRateLen;					\
			__pRateSup = (__pAd)->CommonCfg.SupRate;					\
			__pRateExt = (__pAd)->CommonCfg.ExtRate;					\
			for(__IdRate=0; __IdRate<__RateLen; __IdRate++)				\
			{															\
				if (__RateIndex == (__pRateSup[__IdRate]&0x7f))			\
				{														\
					__FlgIsSupRate = 1; break;							\
				}														\
			}															\
			if ((__FlgIsSupRate) == 0)									\
			{															\
				__RateLen = (__pAd)->CommonCfg.ExtRateLen;				\
				for(__IdRate=0; __IdRate<__RateLen; __IdRate++)			\
				{														\
					if (__RateIndex == (__pRateExt[__IdRate]&0x7f))		\
					{													\
						__FlgIsSupRate = 1; break;						\
					}													\
				}														\
			}															\
		}

	/* get station power save, active or power save */
#define ACMR_STA_IS_IN_ACTIVE_MODE(__pCdb)		\
		((__pCdb)->PsMode == PWR_ACTIVE)

#ifdef ACM_CC_FUNC_11N
	/* check if only 11n is supported */
#define ACMR_IS_11N_ONLY_SUPPORT(__pAd, __pCdb)							\
		(((__pAd)->CommonCfg.PhyMode == PHY_11N_2_4G) ||				\
		((__pAd)->CommonCfg.PhyMode == PHY_11N_5G) ||					\
		((__pCdb)->HTPhyMode.field.MODE == MODE_HTGREENFIELD))

	/* check if the rate is a supported rate for 11n */
	/* if the rate is smaller than maximum rate, it should be ok */
	/* 1 RX Antennas: 0xFF; 2 RX: 0xFF FF; 3 RX: 0xFF FF FF */
#define ACMR_SUP_RATE_CHECK_11N(__pAd, __pCdb, __Rate, __FlgIsSupRate)	\
		{																\
			UINT16 __RateId = __Rate / 100000;							\
			BOOLEAN __FlgIs2040, __FlgIsSGI;							\
			UINT32 __IdMcs;												\
			__FlgIsSupRate = 0;											\
			__FlgIs2040 = ((__pCdb)->MaxHTPhyMode.field.BW == BW_40);	\
			__FlgIsSGI = (__pCdb)->MaxHTPhyMode.field.ShortGI;			\
			for(__IdMcs=0; __IdMcs<32; __IdMcs++)						\
			{															\
				if (__RateId ==											\
						gAcmMCS_HT[__FlgIs2040][__FlgIsSGI][__IdMcs])	\
				{														\
					__FlgIsSupRate = 1; break;							\
				}														\
			}															\
		}

	/* check if we supports 11N */
#define ACMR_IS_11N_SUPPORT(__pAd)				\
		((__pAd)->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)

	/* check if the station supports 11N */
#define ACMR_STA_IS_11N_SUPPORT(__pCdb)			\
		((__pCdb)->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#endif // ACM_CC_FUNC_11N //

	/* get channel busy time in a TBT */
#define ACMR_CHAN_BUSY_GET(__pAd, __Time)			\
	{	__Time = pAd->QloadLatestChannelBusyTimePri; }

	/* get UAPSD Capability */
#define ACMR_APSD_CAPABLE_GET(__pAd)				\
		pAd->CommonCfg.bAPSDCapable

#endif // CONFIG_AP_SUPPORT //


#ifdef CONFIG_STA_SUPPORT

	/* get TXOP of the AC */
#define ACMR_TXOP_BSS_GET(__pAd, __AcId)	ACMR_TXOP_AP_GET((__pAd), (__AcId))

	/* check if WMM is enabled */
#define ACMR_WMM_IS_ENABLED(__pAd)			(__pAd)->MlmeAux.APEdcaParm.bValid

	/* get MAC of associated AP */
#define ACMR_AP_ADDR_GET(__pAd)				(__pAd)->MlmeAux.Bssid

	/* only one entry in QSTA, i.e. associated QAP */
#define ACMR_STA_ENTRY_GET(__pAd, __pDevMac)				\
		IS_ENTRY_CLIENT(&((__pAd)->MacTab.Content[BSSID_WCID])) ?	\
			(&(__pAd)->MacTab.Content[BSSID_WCID]) : NULL

	/* check if current mode is STA */
#define ACMR_IS_AP_MODE							0

	/* check if we have associated to a AP */
#define ACMR_IS_ASSOCIATED(__pAd)				\
		((__pAd)->IndicateMediaState == NdisMediaStateConnected)

	/* check if preamble is short */
#define ACMR_STA_IS_SPREAMBLE(__pAd, __pCdb)	\
		((__pAd)->CommonCfg.TxPreamble == Rt802_11PreambleShort)

	/*	temporarily active our station to send ADDTS req */
#define ACMR_STA_PS_MODE_ACTIVE(__pAd)										\
	{																		\
		ACMR_CB->FlgPsIsAddtsReqSent = TRUE;								\
		if ((__pAd)->StaCfg.Psm == PWR_SAVE)								\
		{																	\
			ACMR_CB->PsModeBackup = (__pAd)->StaCfg.WindowsBatteryPowerMode;\
			RTMP_SET_PSM_BIT((__pAd), PWR_ACTIVE);								\
			OPSTATUS_SET_FLAG((__pAd), fOP_STATUS_RECEIVE_DTIM);			\
			if ((__pAd)->StaCfg.bWindowsACCAMEnable == FALSE)				\
				(__pAd)->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;	\
			(__pAd)->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;\
			AsicForceWakeup((__pAd), FALSE);								\
		}																	\
		else																\
			ACMR_CB->PsModeBackup = Ndis802_11PowerModeCAM;					\
	}

	/* recover to the old PS mode */
#define ACMR_STA_PS_MODE_RECOVER(__pAd)										\
	{																		\
		if ((ACMR_CB->FlgPsIsAddtsReqSent == TRUE) &&						\
			(ACMR_CB->PsModeBackup != Ndis802_11PowerModeCAM))				\
		{																	\
			if (ACMR_CB->PsModeBackup == Ndis802_11PowerModeFast_PSP)		\
			{																\
				OPSTATUS_SET_FLAG((__pAd), fOP_STATUS_RECEIVE_DTIM);		\
				if ((__pAd)->StaCfg.bWindowsACCAMEnable == FALSE)			\
					(__pAd)->StaCfg.WindowsPowerMode = Ndis802_11PowerModeFast_PSP;	\
				(__pAd)->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP; \
				(__pAd)->StaCfg.DefaultListenCount = 3;						\
			}																\
			else if (ACMR_CB->PsModeBackup == Ndis802_11PowerModeMAX_PSP)	\
			{																\
				OPSTATUS_SET_FLAG((__pAd), fOP_STATUS_RECEIVE_DTIM);		\
				if ((__pAd)->StaCfg.bWindowsACCAMEnable == FALSE)			\
					(__pAd)->StaCfg.WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;	\
				(__pAd)->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;\
				(__pAd)->StaCfg.DefaultListenCount = 5;						\
            }																\
			else if (ACMR_CB->PsModeBackup == Ndis802_11PowerModeLegacy_PSP)\
			{																\
				OPSTATUS_SET_FLAG((__pAd), fOP_STATUS_RECEIVE_DTIM);		\
				if ((__pAd)->StaCfg.bWindowsACCAMEnable == FALSE)			\
					(__pAd)->StaCfg.WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;	\
				(__pAd)->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;\
				(__pAd)->StaCfg.DefaultListenCount = 3;						\
			}																\
			RTMP_SET_PSM_BIT((__pAd), PWR_SAVE);								\
			if (!((__pAd)->CommonCfg.bAPSDCapable &&						\
				(__pAd)->CommonCfg.APEdcaParm.bAPSDCapable))				\
				RTMPSendNullFrame((__pAd), (__pAd)->CommonCfg.TxRate, FALSE);\
			else RTMPSendNullFrame((__pAd), (__pAd)->CommonCfg.TxRate, TRUE);\
			ACMR_CB->FlgPsIsAddtsReqSent = FALSE;							\
			ACMR_DEBUG(ACMR_DEBUG_TRACE,									\
				("11e_msg> Return power right! ACMP_StaPsCtrlRightReturn()\n"));\
		}																	\
	}

	/* backup default UAPSD state */
#define ACMR_UAPSD_BACKUP(__pAd)												\
	(__pAd)->CommonCfg.bACMAPSDBackup[ACM_EDCA_VO_AC_QUE_ID] =				\
											(__pAd)->CommonCfg.bAPSDAC_VO;	\
	(__pAd)->CommonCfg.bACMAPSDBackup[ACM_EDCA_VI_AC_QUE_ID] =				\
											(__pAd)->CommonCfg.bAPSDAC_VI;	\
	(__pAd)->CommonCfg.bACMAPSDBackup[ACM_EDCA_BK_AC_QUE_ID] =				\
											(__pAd)->CommonCfg.bAPSDAC_BK;	\
	(__pAd)->CommonCfg.bACMAPSDBackup[ACM_EDCA_BE_AC_QUE_ID] =				\
											(__pAd)->CommonCfg.bAPSDAC_BE;	\
	ACMR_DEBUG(ACMR_DEBUG_TRACE,											\
				("11e_msg> Station Default UAPSD %d %d %d %d\n",			\
				(__pAd)->CommonCfg.bAPSDAC_VO,								\
				(__pAd)->CommonCfg.bAPSDAC_VI,								\
				(__pAd)->CommonCfg.bAPSDAC_BK,								\
				(__pAd)->CommonCfg.bAPSDAC_BE));

	/* get station power save, active or power save */
#define ACMR_IS_IN_ACTIVE_MODE(__pAd, __pCdb)		\
		(__pAd->StaCfg.Psm == PWR_ACTIVE)

	/* get channel busy time in a TBT (not supported) */
#define ACMR_CHAN_BUSY_GET(__pAd, __Time)			\
	{	__Time = 0; }

	/* get WMM & UAPSD Capability */
#define ACMR_WMM_CAPABLE_GET(__pAd)					\
		pAd->CommonCfg.bWmmCapable

#define ACMR_APSD_CAPABLE_GET(__pAd)				\
		pAd->CommonCfg.bAPSDCapable

	/* Retry count change */
#define ACMR_RETRY_DISABLE(__pAd)											\
	{																		\
		UINT32 __CSR=0;														\
		RTMP_IO_READ32((__pAd), TX_RTY_CFG, &__CSR);						\
		if ((__CSR & 0x0000FFFF) != 0x0000)									\
			((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->RetryCountOldSettings =\
				(__CSR & 0x0000FFFF);										\
		__CSR = __CSR & 0xFFFF0000;											\
		RTMP_IO_WRITE32((__pAd), TX_RTY_CFG, __CSR);						\
	}

#define ACMR_RETRY_ENABLE(__pAd)											\
	{																		\
		UINT32 __CSR=0;														\
		RTMP_IO_READ32((__pAd), TX_RTY_CFG, &__CSR);						\
		__CSR = __CSR | 													\
			((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->RetryCountOldSettings;	\
		RTMP_IO_WRITE32((__pAd), TX_RTY_CFG, __CSR);						\
	}

#endif // CONFIG_STA_SUPPORT //


/* ------------------ MACRO: others related ------------------ */

	/* 	for debug information */
#define ACMR_DEBUG					DBGPRINT
#define ACMR_DEBUG_TRACE			RT_DEBUG_TRACE
#define ACMR_DEBUG_ERR				RT_DEBUG_ERROR

	/* check is the ACM function is enabled */
#define ACMR_IS_ENABLED(__pAd)		(((__pAd) != NULL) &&					\
									((__pAd)->pACM_Ctrl_BK != NULL) &&		\
									(ACMP_IsAnyACEnabled((__pAd)) == ACM_RTN_OK))

	/* sanity check */
#define ACMR_SANITY_CHECK(__pAd)	(((__pAd) != NULL) &&					\
									(ACMR_WMM_IS_ENABLED((__pAd))) &&		\
									((__pAd)->pACM_Ctrl_BK != NULL))

	/* 	for Device Adapter Control Block & ACM Control Block of pAd */
#define ACMR_PWLAN_STRUC			PRTMP_ADAPTER
#define ACMR_CB						((ACM_CTRL_BLOCK *)pAd->pACM_Ctrl_BK)
#define ACMR_ADAPTER_DB				pAd->pACM_Ctrl_BK

	/* 	for protection spin lock/unlock */
#define ACMR_LOCK_INIT				NdisAllocateSpinLock
#define ACMR_LOCK_FREE				NdisFreeSpinLock

	/* 	get self's MAC address */
#define ACMR_SELF_MAC_GET(__pAd)	(__pAd)->CurrentAddress

	/*	get current timestamp low 32-bit value */
#ifdef RTMP_MAC_PCI

#ifdef ACM_CC_FUNC_AUX_ADMIT_TIME
#define ACMR_TIMESTAMP_GET(__pAd, __TimeStamp)				\
	{														\
		UINT32 __CSR=0;	UINT64 __Value64;					\
		RTMP_IO_READ32((__pAd), TSF_TIMER_DW0, &__CSR);		\
		__TimeStamp = (UINT64)__CSR;						\
		RTMP_IO_READ32((__pAd), TSF_TIMER_DW1, &__CSR);		\
		__Value64 = (UINT64)__CSR;							\
		__TimeStamp |= (__Value64 << 32);					\
	}

#define ACMR_ALLOWED_TIME_GET(__pStream, __MediumTime)		\
	{														\
		__MediumTime = __pStream->pTspec->MediumTime << 5;	\
	}
#else

/* use jiffies to replace timestamp register get */
/* we can not get register value in tasklet or ISR in USB */
#define ACMR_TIMESTAMP_GET(__pAd, __TimeStamp)				\
	{														\
		ULONG __Time_MS;									\
		NdisGetSystemUpTime(&__Time_MS);					\
		__TimeStamp = __Time_MS * (ACM_OS_TIME_BASE);		\
	}

/*
	In USB/LINUX, we use jiffies to replace timestamp get.
	But in PC, 1 tick is 4ms = 4000us.
	Sometimes we will see extra 8000us will be counted to test result and the
	test will fail. So we need to minus the real medium time with 2 tick time.

	But in test plan, it will check minium & maximum used time.
	So it still may not pass the WiFi ACM test plan, carefully for the method!
*/
#define ACMR_ALLOWED_TIME_GET(__pStream, __MediumTime)		\
	{														\
		__MediumTime = __pStream->pTspec->MediumTime << 5;	\
		if (__MediumTime > (2*ACM_OS_TIME_BASE))			\
			__MediumTime -= (2*ACM_OS_TIME_BASE);			\
	}
#endif // ACM_CC_FUNC_AUX_ADMIT_TIME //

#else // RTMP_MAC_USB //

/* use jiffies to replace timestamp register get */
/* we can not get register value in tasklet or ISR in USB */
#define ACMR_TIMESTAMP_GET(__pAd, __TimeStamp)				\
	{														\
		ULONG __Time_MS;									\
		NdisGetSystemUpTime(&__Time_MS);					\
		__TimeStamp = __Time_MS * (ACM_OS_TIME_BASE);		\
	}

/*
	In USB/LINUX, we use jiffies to replace timestamp get.
	But in PC, 1 tick is 4ms = 4000us.
	Sometimes we will see 8000us will be added to test result and the
	test will fail.
	So we need to minus the real medium time with 2 tick time.
*/
#define ACMR_ALLOWED_TIME_GET(__pStream, __MediumTime)		\
	{														\
		__MediumTime = __pStream->pTspec->MediumTime << 5;	\
		if (__MediumTime > (2*ACM_OS_TIME_BASE))			\
			__MediumTime -= (2*ACM_OS_TIME_BASE);			\
	}
#endif // RTMP_MAC_USB //

	/* channel busy time, bit 0 must be set to 1 to enable */
#define ACMR_CHAN_BUSY_DETECT_ENABLE(__pAd)				\
	{	RTMP_IO_WRITE32((__pAd), CH_TIME_CFG, 0x0000001F); }

#define ACMR_CHAN_BUSY_DETECT_DISABLE(__pAd)			\
	{	RTMP_IO_WRITE32((__pAd), CH_TIME_CFG, 0x00000000); }

	/* get number of TSPEC for the station */
#define ACMP_NUM_OF_TSPEC_IN_GET(__pCdb)				\
	(__pCdb)->ACM_NumOfTspecIn

#define ACMP_NUM_OF_TSPEC_OUT_GET(__pCdb)				\
	(__pCdb)->ACM_NumOfTspecOut


/* ------------------ MARCO: LIST -------------------------- */

#define ACMR_LIST					LIST_HEADER

/* init the ACL list */
#define ACMR_LIST_INIT				initList

/* insert a ACL station */
#define ACMR_LIST_INSERT_TAIL(__pAd, __pEntry)							\
{																		\
	ACMR_LIST *__pList = &((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_List;\
	if ((__pList != NULL) && (__pEntry != NULL))						\
		insertTailList((__pList), (LIST_ENTRY *)(__pEntry));			\
}

/* get a ACL station from the head of list */
#define ACMR_LIST_REMOVE_HEAD(__pAd, __pEntry)							\
{																		\
	ACMR_LIST *__pList = &((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_List;\
	if ((__pList != NULL) && (__pEntry != NULL))						\
		__pEntry = (ACM_ACL_ENTRY *)removeHeadList((__pList));			\
}

/* get number of ACL station */
#define ACM_LIST_SIZE_GET(__pAd, __Size)								\
{																		\
	ACMR_LIST *__pList = &((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_List;\
	if (__pList != NULL)												\
		__Size = getListSize((__pList));								\
}

/* get a ACL station by MAC */
#define ACM_LIST_ENTRY_GET(__pAd, __pMac, __FlgIsFound, __pEntry)		\
{																		\
	ACMR_LIST *__pList = &((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_List;\
	UINT32 __Size, __IdEntry;											\
	ACM_ACL_ENTRY *__pAclEntry;											\
	__Size = getListSize((__pList));									\
	__FlgIsFound = FALSE;												\
	for(__IdEntry=0; __IdEntry<__Size; __IdEntry++)						\
	{																	\
		__pAclEntry = (ACM_ACL_ENTRY *)removeHeadList((__pList));		\
		if (ACMR_MAC_CMP(__pAclEntry->STA_MAC, __pMac) == 0)			\
		{																\
			if (__pEntry != NULL)										\
			ACMR_MEM_COPY(__pEntry, __pAclEntry, sizeof(ACM_ACL_ENTRY));\
			insertTailList((__pList), (LIST_ENTRY *)(__pAclEntry));		\
			__FlgIsFound = TRUE;										\
			break;														\
		}																\
		insertTailList((__pList), (LIST_ENTRY *)(__pAclEntry));			\
	}																	\
}

/* delete a ACL station by MAC */
#define ACM_LIST_ENTRY_DEL(__pAd, __pMac)								\
{																		\
	ACMR_LIST *__pList = &((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_List;\
	UINT32 __Size, __IdEntry;											\
	ACM_ACL_ENTRY *__pAclEntry;											\
	__Size = getListSize((__pList));									\
	for(__IdEntry=0; __IdEntry<__Size; __IdEntry++)						\
	{																	\
		__pAclEntry = (ACM_ACL_ENTRY *)removeHeadList((__pList));		\
		if (ACMR_MAC_CMP(__pAclEntry->STA_MAC, __pMac) == 0)			\
		{																\
			ACMR_MEM_FREE(__pAclEntry);									\
			break;														\
		}																\
		insertTailList((__pList), (LIST_ENTRY *)(__pAclEntry));			\
	}																	\
}

/* clear all ACL station records */
#define ACM_LIST_EMPTY(__pAd)											\
{																		\
	ACMR_LIST *__pList = &((ACM_CTRL_BLOCK *)__pAd->pACM_Ctrl_BK)->ACL_List;\
	ACM_ACL_ENTRY *__pAclEntry;											\
	__pAclEntry = (ACM_ACL_ENTRY *)removeHeadList((__pList));			\
	while(__pAclEntry != NULL)											\
	{																	\
		ACMR_MEM_FREE(__pAclEntry);										\
		__pAclEntry = (ACM_ACL_ENTRY *)removeHeadList((__pList));		\
	}																	\
}


/* ------------------ MARCO: TASK & TIMER ------------------ */

#define ACMR_OS_SPIN_LOCK_BH		RTMP_SEM_LOCK
#define ACMR_OS_SPIN_UNLOCK_BH		RTMP_SEM_UNLOCK

#define ACMR_OS_TIMER_STRUCT		RALINK_TIMER_STRUCT
#define ACMR_OS_SPIN_LOCK			NDIS_SPIN_LOCK

#define ACM_TIMER_API_PARAM							\
	ACM_PARAM_IN	PVOID		SystemSpecific1,	\
	ACM_PARAM_IN	PVOID		FunctionContext,	\
	ACM_PARAM_IN	PVOID		SystemSpecific2,	\
	ACM_PARAM_IN	PVOID		SystemSpecific3
#define ACM_TIMER_API_DATA			(ULONG)FunctionContext

	/* init a timer */
#define ACMR_TIMER_INIT(__pAd, __TimerBlock, __TimerFunc, __TimerData)	\
	RTMPInitTimer(__pAd, &(__TimerBlock), GET_TIMER_FUNCTION(__TimerFunc), (PVOID)__TimerData, FALSE);

	/* enable a timer */
#ifndef ACMR_HANDLE_IN_TIMER
#define ACMR_TIMER_ENABLE(__FlgIsEnable, __Timer, __Timeout)			\
	if (__FlgIsEnable == 0) {											\
		RTMPSetTimer(&(__Timer), __Timeout);							\
		__FlgIsEnable = 1; }
#else

#define ACMR_TIMER_ENABLE(__FlgIsEnable, __Timer, __Timeout)			\
	RTMPSetTimer(&(__Timer), __Timeout);
#endif // ACMR_HANDLE_IN_TIMER //

	/* disable a timer */
#define ACMR_TIMER_DISABLE(__FlgIsEnable, __Timer)						\
	{																	\
		BOOLEAN __Status;												\
		RTMPCancelTimer(&(__Timer), &__Status);							\
		__FlgIsEnable = 0;												\
	}


#ifndef ACMR_HANDLE_IN_TIMER
	/* activate a tasklet and schedule next timer */
#define ACMR_TASK_ACTIVATE(__Task, __Timer, __Timeout)					\
	tasklet_hi_schedule(&__Task);										\
	RTMPSetTimer(&__Timer, __Timeout);

#ifdef ACMR_OS_LINUX

#define ACMR_OS_TASK_STRUCT			struct tasklet_struct

	/* init a tasklet */
#define ACMR_TASK_INIT(__pAd, __TaskletBlock, __TaskletFunc, __TaskletData, __TkName)	\
	tasklet_init(&(__TaskletBlock), __TaskletFunc, __TaskletData);
#endif // ACMR_OS_LINUX //


#ifdef ACMR_OS_VXWORKS

#define ACMR_OS_TASK_STRUCT			struct _RTMP_NET_TASK_STRUCT_

	/* init a tasklet */
#define ACMR_TASK_INIT(__pAd, __TaskletBlock, __TaskletFunc, __TaskletData, __TkName)	\
	RTMP_NET_TASK_INIT(&(__TaskletBlock), __TaskletFunc, __TaskletData); \
	strcpy(&((__TaskletBlock).taskName[0]), __TkName);
#endif // ACMR_OS_VXWORKS //

#else

#define ACMR_OS_TASK_STRUCT			UINT32
#define ACMR_TASK_INIT(__pAd, __TaskletBlock, __TaskletFunc, __TaskletData, __TkName)
#endif // ACMR_HANDLE_IN_TIMER //



/* ------------------ MACRO: WMM Related ------------------ */

	/* 	get the WMM TXOP parameters of QAP, not BSS (used for QAP mode) */
#define ACMR_TXOP_AP_GET(__pAd, AcId)	((__pAd)->CommonCfg.APEdcaParm.Txop[AcId])

	/*	QID for AC0 ~ AC1 */
#define ACMR_QID_AC_BE					QID_AC_BE
#define ACMR_QID_AC_BK					QID_AC_BK
#define ACMR_QID_AC_VI					QID_AC_VI
#define ACMR_QID_AC_VO					QID_AC_VO


/* MACRO: device (ap & station) */

	/* 	for Device Entry & ACM Control Block of Device */
#define ACMR_STA_DB						MAC_TABLE_ENTRY
#define ACMR_STA_ACM_PARAM_INFO(__pCdb)	((ACM_ENTRY_INFO *)(&(__pCdb)->ACM_Info))

	/*
		1. check if the STA is QSTA;
		2. get Maximum number of device entry;
		3. check if the device entry is valid;
	*/
#define ACMR_IS_QSTA(__pCdb)			ACMR_IS_WMM_STA((__pCdb))
#define ACMR_STA_MAX_NUM				MAX_LEN_OF_MAC_TABLE
#define ACMR_ENTRY_ID_VALID(__pCdb)		(IS_ENTRY_CLIENT((__pCdb)))

	/*
		1. get a device entry based on array index;
		2. check if the device entry is valid;
	*/
#define ACMR_STA_GET(__pAd, __pCdb, i)	(&(__pAd)->MacTab.Content[i])
#define ACMR_STA_IS_VALID(__pCdb)		\
	(((__pCdb) != NULL) &&				\
	(IS_ENTRY_CLIENT((__pCdb))) && 			\
	((*(UINT32 *)ACMR_CLIENT_MAC((__pCdb)) != 0)))

	/*
		1. get MAC address of a device entry;
		2. check if the HT rate is used currently;
		3. get current MCS of a device entry;
		4. get current tx rate of a device entry;
	*/
#define ACMR_CLIENT_MAC(__pCdb)			(__pCdb)->Addr

#define ACMR_IS_HT_RATE_USED(__pCdb)								\
	((__pCdb)->HTPhyMode.field.MODE >= MODE_HTMIX)

#define ACMR_CLIENT_MCS_GET(__pCdb)									\
	((__pCdb)->HTPhyMode.field.MCS)

#define ACMR_CLIENT_PHY_MODE_MCS_GET(__pCdb, __PhyMode, __MCS)		\
	{																\
		HTTRANSMIT_SETTING *__pTxPhyMode = &(__pCdb)->HTPhyMode;	\
		__PhyMode = __pTxPhyMode->field.MODE;						\
		__MCS = __pTxPhyMode->field.MCS;							\
	}
#define ACMR_CLIENT_CCK_RATE_INDEX_GET(__MCS, __RateIndex)			\
	{																\
		if ((__MCS) <= 3) __RateIndex = __MCS;						\
		else if (((__MCS) >= 8) && ((__MCS) <= 11))					\
			__RateIndex = (__MCS) - 8;								\
	}
#define ACMR_CLIENT_OFDM_RATE_INDEX_GET(__MCS, __RateIndex)			\
	{	if (__MCS <= 7) __RateIndex = __MCS+4; }

	/*
		1. check if the station associates to us (QAP mode);
		2. check if the station is a WMM device;
		3. check if the preamble of stations is short preamble;
	*/
#define ACMR_STA_IS_ASSOC(__pCdb)	((__pCdb)->Sst == SST_ASSOC)
#define ACMR_IS_WMM_STA(__pCdb)		\
		CLIENT_STATUS_TEST_FLAG((__pCdb), fCLIENT_STATUS_WMM_CAPABLE)

	/*	get current associated station count */
#define ACMR_STA_CUR_COUNT(__pAd)		1	/* yet implement */


/* ------------------ MACRO: Packet Related ------------------ */

	/*
		1. packet structure pointer type;
		2. wlan header structure;
		3. get the type field of a frame;
		4. get the subtype field of a frame;
		5. get the qos field of a frame;
		6. the sub type definition of QoS Data;
		7. get legacy wlan header size;
		8. get QoS wlan header size;
		9. packet queue header;
		10. packet queue entry;
		11. translate packet queue entry to MBUF;
	*/
#define ACMR_MBUF						NDIS_PACKET
#define ACMR_WLAN_HEADER				HEADER_802_11
#define ACMR_FME_TYPE_GET(__pHdr)		((__pHdr)->FC.Type)
#define ACMR_FME_SUBTYPE_GET(__pHdr)	((__pHdr)->FC.SubType)
#define ACMR_FME_QOSCTRL_GET(__pHdr)	(*((UINT16 *)(__pHdr)->Octet))
#define ACMR_FME_SUB_TYPE_QOS_DATA		SUBTYPE_QDATA
#define ACMR_FME_SUB_TYPE_QOS_NULL		SUBTYPE_QOS_NULL
#define ACMR_FME_LEG_HEADER_SIZE		(sizeof(HEADER_802_11))
#define ACMR_FME_QOS_HEADER_SIZE		(sizeof(HEADER_802_11)+2)
#define ACMR_FME_QOS_N_HEADER_SIZE		(sizeof(HEADER_802_11)+6)
#define ACMR_QUEUE_HEADER				QUEUE_HEADER
#define ACMR_QUEUE_ENTRY				QUEUE_ENTRY
#define ACMR_QUEUE_ENTRY_TO_MBUF		QUEUE_ENTRY_TO_PACKET

	/*
		1. allocate a wlan packet;
		2. copy data to the wlan packet;
		3. free a wlan packet;
	*/
#define ACMR_PKT_ALLOCATE(__pAd, __pMblk)							\
	{																\
		__pMblk = (ACMR_MBUF *)RtmpOSNetPktAlloc(__pAd, MGMT_DMA_BUFFER_SIZE);\
	}

#define ACMR_PKT_COPY(__pMblk, __BufFrame, __Len)	\
		ACMR_MEM_COPY(skb_put(RTPKT_TO_OSPKT(__pMblk), __Len), __BufFrame, __Len)
#define ACMR_PKT_FREE(__pAd, __pMblk)				\
		RELEASE_NDIS_PACKET((__pAd), (__pMblk), NDIS_STATUS_SUCCESS)

	/*
		1. get the acm type of a packet;
		2. get the user priority of a packet;
		3. set the acm type of a packet;
		4. set the user priority of a packet;
	*/
#define ACMR_PKT_QOS_TYPE_GET(__pPkt)			\
	(((ACM_PACKET_INFO *)(RTMP_GET_PACKET_ACM_PARAM_INFO(__pPkt)))->QosType)
#define ACMR_PKT_UP_GET(__pPkt)					\
	(((ACM_PACKET_INFO *)(RTMP_GET_PACKET_ACM_PARAM_INFO(__pPkt)))->UP)

#define ACMR_PKT_QOS_TYPE_SET(__pPkt, __Type)	\
	(((ACM_PACKET_INFO *)(RTMP_GET_PACKET_ACM_PARAM_INFO(__pPkt)))->QosType) = __Type;
#define ACMR_PKT_UP_SET(__pPkt, __UP)			\
	(((ACM_PACKET_INFO *)(RTMP_GET_PACKET_ACM_PARAM_INFO(__pPkt)))->UP) = __UP;

	/*
		1. get the encrypt mode of a device entry;
		2. the WEP type;
		3. the TKIP type;
		4. the AES type;
	*/
#define ACMR_STA_ENCRYPT_MODE_GET(__pCdb)	((__pCdb)->WepStatus)
#define ACMR_ENCRYPT_WEP					Ndis802_11WEPEnabled
#define ACMR_ENCRYPT_TKIP					Ndis802_11Encryption2Enabled
#define ACMR_ENCRYPT_AES					Ndis802_11Encryption3Enabled

	/*
		1. the RALINK AGGREGATION type;
		2. the AMSDU type;
	*/
#define ACMR_AGG_RALINK						TX_RALINK_FRAME
#define ACMR_AGG_AMSDU						TX_AMSDU_FRAME

	/*
		1. the ACTION sub type;
		2. get the data pointer of a packet;
		3. get the data length of a packet;
		4. get the receiver address (Addr1) of a packet;
		5. get the transmitter address (Addr2) of a packet;
		6. get the type/length field of a packet;
		7. get packet data pointer
		8. set the receiver address (Addr1) of a packet;
		9. set the transmitter address (Addr2) of a packet;
		10. set the BSSID (Addr3) of a packet;
	*/
#define ACMR_SUBTYPE_ACTION					SUBTYPE_ACTION
#define ACMR_WLAN_PKT_GET(__pMbuf)			GET_OS_PKT_DATAPTR(__pMbuf)
#define ACMR_WLAN_LEN_GET(__pMbuf)			GET_OS_PKT_LEN(__pMbuf)

#define ACMR_WLAN_PKT_RA_GET(__pMbuf, __Addr)							\
	{																	\
		ACMR_WLAN_HEADER *__pHdr = (ACMR_WLAN_HEADER *)__pMbuf;			\
		ACMR_MEM_COPY(__Addr, __pHdr->Addr1, 6);						\
	}

#define ACMR_WLAN_PKT_TA_GET(__pMbuf, __Addr)							\
	{																	\
		ACMR_WLAN_HEADER *__pHdr= (ACMR_WLAN_HEADER *)__pMbuf;			\
		ACMR_MEM_COPY(__Addr, __pHdr->Addr2, 6);						\
	}

#define ACMR_WLAN_PKT_TYPE_GET(__pMbuf, __Type)							\
	{																	\
		ACMR_WLAN_HEADER *__pHdr = (ACMR_WLAN_HEADER *)__pMbuf;			\
		if (__pHdr->FC.Type & SUBTYPE_QDATA) 							\
			__Type = *(UINT16 *)&__pHdr->Octet[2];						\
		else __Type = *(UINT16 *)&__pHdr->Octet[0];						\
	}

#define ACMR_PKT_DATA_GET(__pMbuf, __pSrcBufVa)							\
	{																	\
		PACKET_INFO __Info;												\
		UINT32 __Len;													\
		RTMP_QueryPacketInfo(__pMbuf, &__Info, &__pSrcBufVa, &__Len);	\
	}

#define ACMR_WLAN_PKT_RA_SET(__pMbuf, __Addr)							\
	{																	\
		ACMR_WLAN_HEADER *__pHdr = (ACMR_WLAN_HEADER *)__pMbuf;			\
		ACMR_MEM_COPY(__pHdr->Addr1, __Addr, 6);						\
	}

#define ACMR_WLAN_PKT_TA_SET(__pMbuf, __Addr)							\
	{																	\
		ACMR_WLAN_HEADER *__pHdr = (ACMR_WLAN_HEADER *)__pMbuf;			\
		ACMR_MEM_COPY(__pHdr->Addr2, __Addr, 6);						\
	}

#define ACMR_WLAN_PKT_BSSID_SET(__pMbuf, __Addr)						\
	{																	\
		ACMR_WLAN_HEADER *__pHdr = (ACMR_WLAN_HEADER *)__pMbuf;			\
		ACMR_MEM_COPY(__pHdr->Addr3, __Addr, 6);						\
	}

	/*
		1. get RTS threshold;
		2. get Fragment threshold;
		3. get maximum packet size;
		4. get RTS/CTS flag;
		5. get CTS-self flag;
	*/
#define ACMR_RTS_THRESH(__pAd)				(__pAd)->CommonCfg.RtsThreshold
#define ACMR_FRG_THRESH(__pAd)				(__pAd)->CommonCfg.FragmentThreshold
#define ACMR_MAX_BUF_SIZE					MGMT_DMA_BUFFER_SIZE
#define ACMR_RTS_FLAG_GET(__pAd, __pPkt)	RTMP_GET_PACKET_RTS(__pPkt)
#define ACMR_CTS_FLAG_GET(__pAd, __pPkt)	(__pAd)->FlgCtsEnabled

	/*
		1. mark packet user priority;
		2. mark packet minimum PHY mode;
		3. mark packet minimum MCS;
		4. mark packet tx time;
		5. get packet tx time;
		6. get packet minimum PHY mode;
		7. get packet minimum MCS;
	*/
#define ACMR_PKT_MARK_UP(__pPkt, __UP)					\
		RTMP_SET_PACKET_UP(__pPkt, __UP)
#define ACMR_PKT_MARK_MIN_PHY_MODE(__pPkt, __PhyMode)	\
		RTMP_SET_PACKET_ACM_MIN_PHY_MODE(__pPkt, __PhyMode)
#define ACMR_PKT_MARK_MIN_PHY_MCS(__pPkt, __MCS)		\
		RTMP_SET_PACKET_ACM_MIN_PHY_MCS(__pPkt, __MCS)
#define ACMR_PKT_MARK_TX_TIME(__pPkt, __TxTime)			\
		RTMP_SET_PACKET_TX_TIME(__pPkt, __TxTime)
#define ACMR_PKT_TX_TIME_GET(__pPkt)					\
		RTMP_GET_PACKET_TX_TIME(__pPkt)
#define ACMR_PKT_MIN_PHY_MODE_GET(__pPkt, __PhyMode)	\
		__PhyMode = RTMP_GET_PACKET_ACM_MIN_PHY_MODE(__pPkt)
#define ACMR_PKT_MIN_PHY_MCS_GET(__pPkt, __MCS)			\
		__MCS = RTMP_GET_PACKET_ACM_MIN_PHY_MCS(__pPkt)

	/*	1. transmit a management frame; */
#define ACMR_MGMT_PKT_TX(__pAd, __pPkt, __Len)			\
		MiniportMMRequest((__pAd), 0, __pPkt, __Len)

	/*	1. Handle QoS Data or Null frame when TX; */
#ifdef ACM_CC_FUNC_TCLAS
	/*
		For TCLAS mechanism, if we want to know the UP of the packet,
		we need to compare all TCLAS to find, not get from QoS Control.
	*/
#define ACMP_DATA_NULL_HANDLE(__pAd, __pCdb, __pHeader, __TSID)			\
{																		\
		if (ACMP_NUM_OF_TSPEC_IN_GET(__pCdb) > 0)						\
			ACMP_DataNullHandle(__pAd, __pCdb, __pHeader);				\
}
#else

#define ACMP_DATA_NULL_HANDLE(__pAd, __pCdb, __pHeader, __TSID)			\
{																		\
		if (ACMP_NUM_OF_TSPEC_IN_GET(__pCdb) > 0)						\
		{																\
			if (ACMP_NumOfAcTspecInGet(__pCdb, __TSID) > 0)				\
				ACMP_DataNullHandle(__pAd, __pCdb, __pHeader);			\
		}																\
}
#endif // ACM_CC_FUNC_TCLAS //


/* ------------------ MACRO: Memory Related ------------------ */

	/*
		1. allocate a memory block;
		2. free a memory block;
		3. copy a memory block;
		4. zero a memory block;
		5. copy MAC address;
		6. set value to a memory block;
		7. MAC compare;
	*/
#ifndef ACM_MEMORY_TEST
#define ACMR_MEM_ALLOC(__pMem, __Size, __Type)					\
		os_alloc_mem(NULL, (UCHAR **)&(__pMem), __Size)

#define ACMR_MEM_FREE(__pMem)									\
		os_free_mem(NULL, __pMem)
#else

#define ACMR_MEM_ALLOC(__pMem, __Size, __Type)					\
		os_alloc_mem(NULL, (UCHAR **)&(__pMem), __Size);		\
		gAcmMemAllocNum ++;

#define ACMR_MEM_FREE(__pMem)									\
		{ os_free_mem(NULL, __pMem);							\
		gAcmMemFreeNum ++; }
#endif // ACM_MEMORY_TEST //

#define ACMR_MEM_COPY(__Dst, __Src, __Len)	memcpy(__Dst, __Src, __Len)
#define ACMR_MEM_ZERO(__Mem, __Len)			memset(__Mem, 0, __Len)
#define ACMR_MEM_MAC_COPY(__Dst, __Src)		memcpy(__Dst, __Src, 6)
#define ACMR_MEM_SET(__Dst, __Char, __Len)	memset(__Dst, __Char, __Len)
#define ACMR_MAC_CMP(__Src, __Dst)			memcmp(__Src, __Dst, 6)

#define ACMR_MGMT_FME_ALLOCATE				MlmeAllocateMemory


/* ------------------ MACRO: Utility Related ------------------ */

	/*	atoi & atoh */
#define ACMR_ARG_ATOI(__pArgv)				simple_strtol((PSTRING) __pArgv, 0, 10)
#define ACMR_ARG_ATOH(__Buf, __Hex)			AtoH((PSTRING) __Buf, __Hex, 1)

	/* htons */
#define ACMR_HTONS(__Byte)					htons(__Byte)


/* ------------------ MACRO: HT Related ------------------ */

	/* check if the STA support 20/40 MHz */
#define ACMR_IS_2040_STA(__pCdb)						\
		((__pCdb)->MaxHTPhyMode.field.BW == BW_40)

	/* check if the STA uses short GI */
#define ACMR_IS_SHORT_GI_STA(__pCdb)					\
		(__pCdb)->MaxHTPhyMode.field.ShortGI

	/* get maximum BLOCK ACK NUM */
#define ACMR_STA_MAX_BACK_NUM_GET(__pAd, __pCdb, __Num)	\
		BA_MaxWinSizeReasign(__pAd, __pCdb, &(__Num))

	/* get Nss, Number of Spatial Streams */
#define ACMR_NSS_GET(__pCdb, __Nss)									\
	do {															\
		if ((__pCdb)->MaxHTPhyMode.field.MCS < 8) __Nss = 1;		\
		else if ((__pCdb)->MaxHTPhyMode.field.MCS < 16) __Nss = 2;	\
		else if ((__pCdb)->MaxHTPhyMode.field.MCS < 24) __Nss = 3;	\
		else if ((__pCdb)->MaxHTPhyMode.field.MCS < 32) __Nss = 4;	\
		else __Nss = 1;												\
	} while(0);

	/* check if GreenField mode */
#define ACMR_IS_GREENFIELD_MODE(__pCdb)								\
		((__pCdb)->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)

	/* check if the BA is built */
#define ACMR_HT_IS_BA_BUILT(__pCdb, __UP)							\
		((__pCdb)->TXBAbitmap & (1<<(__UP)))


/* ------------------ MACRO: Rate Related ------------------ */

	/* phy mode */
#define ACMR_PHY_CCK						MODE_CCK
#define ACMR_PHY_OFDM						MODE_OFDM
#define ACMR_PHY_HT							MODE_HTMIX
#define ACMR_PHY_NONE						0x7

/* mapping to ACM_PRE_TIME_ID_1M ~ ACM_PRE_TIME_ID_54M */
#define ACMR_PHY_RATE_ID_RTS_CTS_GET(__pAd)	(__pAd)->CommonCfg.RtsRate


#define ACMR_PHY_MODE_COMPARE(__Src, __Dst)	((__Src) < (__Dst))

	/*
		If same PHY Mode, we need to check if MCS < McsMin;
		if cur phy = OFDM and min phy = CCK, MCS = 0 (6M) and MCS = 1 (9M) <
		McsMin = 3 or 11 (11M)
	*/
#define ACMR_MCS_COMPARE(__PhyMode, __MinPhy, __MCS, __McsMin, __FlgFail)	\
	{																		\
		if (__PhyMode == __MinPhy)											\
		{																	\
			if (__MCS < __McsMin)											\
				__FlgFail = 1;												\
		}																	\
		else																\
		{																	\
			if ((__PhyMode == MODE_OFDM) && (__MinPhy == MODE_CCK))			\
			{																\
				if ((__MCS <= 1) && ((__McsMin == 3) || (__McsMin == 11)))	\
					__FlgFail = 1;											\
			}																\
		}																	\
	}

	/* rate 54 ~ 1M ID, reference to other modules */
#define ACMR_RATE_DRV_54M					RATE_54
#define ACMR_RATE_DRV_48M					RATE_48
#define ACMR_RATE_DRV_36M					RATE_36
#define ACMR_RATE_DRV_24M					RATE_24
#define ACMR_RATE_DRV_18M					RATE_18
#define ACMR_RATE_DRV_12M					RATE_12
#define ACMR_RATE_DRV_9M					RATE_9
#define ACMR_RATE_DRV_6M					RATE_6
#define ACMR_RATE_DRV_11M					RATE_11
#define ACMR_RATE_DRV_5_5M					RATE_5_5
#define ACMR_RATE_DRV_2M					RATE_2
#define ACMR_RATE_DRV_1M					RATE_1

#define ACM_RATE_MAX_NUM					12
#define ACM_RATE_MAX_NUM_HT					32	/* 32 MCS */




/* ========================================================================== */
/* Structure */

/* ----- ACM Information ----- */
/* STA Entry ACM Information, put the entry to your client entry structure */
typedef struct _ACM_ENTRY_INFO {

	/*
		Important: If you add any extra parameters or change declaration type
		here, you need to change the WMM_STA_ACM_INFO_SIZE in rtmp.h
	*/

	/*
		for QAP, these TSPECs are down link TSPEC;
		for QSTA, these TSPECs are up link TSPEC
	*/
	UCHAR	*pAcStmOut [ACM_STA_TID_MAX_NUM]; /* output AC streams */

	/*
		for QAP, these TSPECs are non-down link TSPEC;
		for QSTA, these TSPECs are non-up link TSPEC
	*/
	UCHAR	*pAcStmIn [ACM_STA_TID_MAX_NUM]; /* input AC streams */

} ACM_ENTRY_INFO;


/* Packet ACM Information, put the entry to your packet structure */
/* Used in ACMR_PKT_QOS_TYPE_SET() */
typedef struct _ACM_PACKET_INFO {

	UCHAR	QosType:4;	/* ACM_QOS_TYPE_LEGACY ~ ACM_QOS_TYPE_UAPSD */
	UCHAR	UP:4;		/* used for QoS Null frame, 0 ~ 15 (EDCA+HCCA) */

} ACM_PACKET_INFO;


/* TCLASS element */
#define ACM_TCLAS_PROCESSING_MAX		1
#define ACM_TCLAS_MAX_NUM				5

typedef struct _ACM_TCLAS {

    UCHAR  UserPriority;				/* user priority */

#define ACM_TCLAS_TYPE_ETHERNET			0
#define ACM_TCLAS_TYPE_IP_V4			1
#define ACM_TCLAS_TYPE_8021DQ			2
#define ACM_TCLAS_TYPE_MAX				ACM_TCLAS_TYPE_8021DQ

#define ACM_TCLAS_TYPE_ETHERNET_LEN 	17
#define ACM_TCLAS_TYPE_IP_V4_LEN		18
#define ACM_TCLAS_TYPE_8021DQ_LEN		5

#define ACM_TCLAS_TYPE_WME_ETHERNET_LEN	(17+6) /* 6 is OUI header length */
#define ACM_TCLAS_TYPE_WME_IP_V4_LEN	(18+6) /* 6 is OUI header length */
#define ACM_TCLAS_TYPE_WME_8021DQ_LEN	(5+6)  /* 6 is OUI header length */
    UCHAR  ClassifierType;

    UCHAR  ClassifierMask;				/* maximum 8 fields mapping */

    union
    {
        struct
        {
            UCHAR   AddrSrc[6];		/* source MAC address */
            UCHAR   AddrDst[6];		/* destination MAC address */
            UINT16  Type;				/* type/length */
            UCHAR   Reserved[28];
        } __attribute__ ((packed)) Ethernet;

        struct
        {
            UCHAR   Version;
            UINT32  IpSource;
            UINT32  IpDest;
            UINT16  PortSource;
            UINT16  PortDest;
            UCHAR   DSCP;
            UCHAR   Protocol;
            UCHAR   Reserved[27];
        } __attribute__ ((packed)) IPv4;

        struct
        {
            UINT16  TagType;			/* VLAN tag (2B) */
            UCHAR   Reserved[40];
        } __attribute__ ((packed)) IEEE8021Q;
    } Clasifier;
} __attribute__ ((packed)) ACM_TCLAS;


/* ----- TSPEC ----- */
/* provided to other modules */


/* TS Info field */
typedef struct _ACM_TS_INFO {

	/*
		1 for a periodic traffic pattern (e.g. isochronous traffic stream of
		MSDUs, with constant or variable sizes, that are originated at fixed
		rate), or is set to 0 for an aperiodic, or unspecified traffic pattern
		(e.g. asynchronous traffic stream of low-duty cycles.
	*/
#define ACM_TRAFFIC_TYPE_PERIODIC		1
#define ACM_TRAFFIC_TYPE_APERIODIC		0
	UCHAR  TrafficType;

	/* 0 ~ 7, UP for prioritized QoS (TC) */
	/*
		The same TSID may be used for multiple traffic streams at different
		non-AP QSTAs.  A non-AP QSTA may use the TSID value for a downlink
		TSPEC and either an uplink or a direct link TSPEC at the same time.
		A non-AP QSTA shall not use the same TSID for both uplink and direct
		link TS.
	*/
	UCHAR  TSID;

	/* 00: uplink; 01: downlink; 10: direct link; 11: bidirectional link */
#define ACM_DIRECTION_MAX				4
#define ACM_DIRECTION_UP_LINK			0x00
#define ACM_DIRECTION_DOWN_LINK			0x01
#define ACM_DIRECTION_DIRECT_LINK		0x02
#define ACM_DIRECTION_BIDIREC_LINK		0x03
    UCHAR  Direction;

	/* 00: reserved; 01: EDCA; 10: HCCA; 11: EDCA+HCCA */
#define ACM_ACCESS_POLICY_EDCA			0x01
#define ACM_ACCESS_POLICY_HCCA			0x02
#define ACM_ACCESS_POLICY_MIX			0x03
	UCHAR  AccessPolicy;

	/*
		Valid only when access method is HCCA or when the access method is EDCA
		and the schedule subfiled is set to 1.  It is set to 1 by a non-AP QSTA
		to indicate that an aggregate schedule is required.
	*/
#define ACM_AGGREGATION_ENABLE			1
#define ACM_AGGREGATION_DISABLE			0
    UCHAR  Aggregation;

	/*
		1 indicate that APSD is to be used for the traffic associated with the
		TSPEC.
	*/
#define ACM_APSD_ENABLE					1
#define ACM_APSD_DISABLE				0
	UCHAR  APSD;

	/*
		Indicates the actual value of the UP to be used for the transport of
		MSDUs belonging to this traffic stream in cases where relative
		prioritization is required.  When the TCLAS element is present in the
		request, the user priority subfield in TSInfo field of the TSPEC
		element is reserved.
	*/
#define ACM_UP_UNKNOWN				   	0xFF
	UCHAR  UP;

	/* 00: normal ACK; 01: No ACK; 10: reserved; 11: Block ACK */
#define ACM_ACK_POLICY_NORMAL			0x00
#define ACM_ACK_POLICY_NO				0x01
#define ACM_ACK_POLICY_BLOCK			0x03
	UCHAR  AckPolicy;

	/*
		APSD vs. schedule ==> 00: no schedule; 01: unscheduled APSD;
		10: reserved; 11: scheduled APSD
	*/
	/*
		When the access policy is set to any value other than EDCA, the schedule
		subfiled is reserved.
	*/
#define ACM_SCHEDULE_NO				   	0x00
#define ACM_SCHEDULE_UN_APSD			0x01
#define ACM_SCHEDULE_APSD				0x03
	UCHAR  Schedule;

} GNU_PACKED ACM_TS_INFO;

/*
	Unspecified parameter sin these fields as indicated by a zero value indicate
	dont care.
*/
typedef struct _ACM_TSPEC {

	ACM_TS_INFO  TsInfo; /* 9B */

	/*
		Specify the nominal size, if bit15 == 1 means the size, in octets, of
		the MSDU is fixed and is indicated by bit0 ~ bit14; If bit15 == 0 means
		the size of the MSDU might not be fixed and the size indicates the
		nominal MSDU size.
	*/
#define ACM_NOM_MSDU_SIZE_CHECK_BIT			((UINT16)0x8000)
	UINT16  NominalMsduSize;

	/*
		Specify the maximum size, in octets, of MSDUs belonging to the TS under
		this traffic specification.
	*/
	UINT16  MaxMsduSize;

	/*
		Specify the minimum interval, in units of microseconds, between the
		start of two successive SPs.
	*/
#define ACM_TSPEC_MIN_SERV_INT_LIMIT		((UINT32)10000)
	UINT32  MinServInt;

	/*
		Specify the maximum interval, in units of microseconds, between the
		start of two successive SPs.
	*/
	UINT32  MaxServInt;

	/*
		Specify the maximum amount of time in units of microseconds that may
		elapse without arrival or transfer of an MSDU belonging to the TS
		before this TS is deleted by the MAC entity at the HC.
	*/
#define ACM_TSPEC_INACTIVITY_DISABLE		((UINT32)0)
#define ACM_TSPEC_INACTIVITY_MIN			((UINT32)1000000)
	UINT32  InactivityInt;

	/*
		Specify the maximum amount of time in units of microseconds that may
		elapse without arrival or transfer of an MSDU belonging to the TS before
		the generation of successive QoS (+)CF-Poll is stopped for this TS.
	*/
	/*
		A value of 4294967295 (=2^32 - 1) disables the suspension interval,
		indicating that polling for ths TS is not to be interrupted based on
		inactivity.
	*/
#define ACM_TSPEC_SUSPENSION_DISABLE		((UINT32)0xFFFFFFFF)
#define ACM_TSPEC_SUSPENSION_MIN			((UINT32)500000)
	UINT32  SuspensionInt;

	/*
		Indicates the time, expressed in microseconds, when the SP starts.
		If APSD subfield is set to 0, this field is also set to 0
		(unspecified).
	*/
	UINT32  ServiceStartTime;

	/*
		Specify the lowest data rate specified at the MAC_SAP, in units of
		bits per second.
	*/
	UINT32  MinDataRate;

	/*
		Specify the average data rate specified at the MAC_SAP, in units of
		bits per second.
	*/
	UINT32  MeanDataRate;

	/*
		Specify the maximum data rate specified at the MAC_SAP, in units of
		bits per second.
	*/
	UINT32  PeakDataRate;

	/*
		Specify the maximum burst, in units of octets, of the MSDUs belonging
		to this TS that arrive at the MAC SAP at the peak data rate.
	*/
	UINT32  MaxBurstSize;

	/*
		Specify the maximum amount of time, in units of microseconds, allowed
		to transport an MSDU belonging to the TS in this TSPEC, measured between
		the time marking the arrival of the MSDU at the local MAC sublayer from
		the local MAC SAP and the time of completion of the successful
		transmission or retransmission of the MSDU to the destination.
	*/
	UINT32  DelayBound;

	/*
		Specify the desired minimum PHY rate to use for this TS, in units of
		bits per second (bps).
	*/
	UINT32  MinPhyRate;

	/*
		Specify the excess allocation of time (and bandwidth) over and above the
		stated application rates required to transport an MSDU belonging to the
		TS in this TSPEC.
	*/
	/*
		The thirteen least significant bits indicate the decimal part while the
		three most significant bits indicate the integer part of the number.
	*/
	/*
		The field takes into account retransmissions, as the rate information
		does not include retransmissions.
	*/
	/*  range: 1.0 ~ 8.0 */
#define ACM_SURPLUS_INT_BIT_NUM			3
#define ACM_SURPLUS_INT_MAX				8	/* 1.0 ~ 8.0 */
#define ACM_SURPLUS_DEC_BIT_NUM			13
#define ACM_SURPLUS_DEC_BITMAP			0x1FFF
#define ACM_SURPLUS_DEC_VALID_NUM		2
#define ACM_SURPLUS_DEC_BASE			100
	UINT16  SurplusBandwidthAllowance;

	/*
		Contains the amount of time admitted to access the medium, in units of
		32 microsecond periods per second.
	*/
	UINT32  MediumTime;

} GNU_PACKED ACM_TSPEC;

/* Stream information */
typedef struct _ACM_STREAM_INFO {

	ACM_TSPEC	Tspec;
	ACM_TCLAS	Tclas[ACM_TCLAS_MAX_NUM];

#define ACM_TCLAS_PROCESSING_NOT_EXIST	0xFF
	UCHAR	TclasProcessing;

#define ACM_STREAM_TYPE_11E				0x00	/* 11e D8.0 */
#define ACM_STREAM_TYPE_WIFI			0x01	/* WME/WSM */
	UCHAR	StreamType;
	UCHAR	UP;				/* user priority */
	UCHAR	Reserved1;

	UCHAR	DevMac[6];		/* QSTA MAC */

#define TSPEC_STATUS_REQUEST			0x0000	/* requesting */
#define TSPEC_STATUS_ACTIVE				0x0001	/* active */
#define TSPEC_STATUS_ACTIVE_SUSPENSION	0x0002	/* active but suspended */
#define TSPEC_STATUS_REQ_DELETING		0x0003	/* deleting the request */
#define TSPEC_STATUS_ACT_DELETING		0x0004	/* deleting the active */
#define TSPEC_STATUS_RENEGOTIATING		0x0005	/* renegotiating */

#define TSPEC_STATUS_HANDLING			0x0006	/* handle request */

#define TSPEC_STATUS_FAIL				0x0007	/* active or request fail */
	UINT16  Status;			/* current status */

#define TSPEC_CAUSE_UNKNOWN				0x0000	/* unknown cause */
#define TSPEC_CAUSE_REQ_TIMEOUT			0x0001	/* fail due to request timeout */
#define TSPEC_CAUSE_SUGGESTED_TSPEC		0x0002	/* fail due to suggested TSPEC */
#define TSPEC_CAUSE_REJECTED			0x0003	/* rejected by QAP */
#define TSPEC_CAUSE_UNKNOWN_STATUS		0x0004	/* unknown rsp status code */
#define TSPEC_CAUSE_INACTIVITY_TIMEOUT	0x0005	/* inactivity timeout */
#define TSPEC_CAUSE_DELETED_BY_QAP		0x0006	/* deleted by QAP */
#define TSPEC_CAUSE_DELETED_BY_QSTA		0x0007	/* deleted by QSTA */
#define TSPEC_CAUSE_BANDWIDTH			0x0008	/* lower priority deletion */
#define TSPEC_CAUSE_REJ_MANY_TS			0x0009	/* only one TS for a AC */
#define TSPEC_CASUE_REJ_INVALID_PARAM	0x000a	/* invaild parameteres */
#define TSPEC_CAUSE_REJ_INVALID_TOKEN	0x000b	/* invalid Dialog Token */
	UINT16  Cause;			/* cause that makes the stream to be stopped */
							/* valid only when status == TSPEC_STATUS_FAIL */

#define TSPEC_PHY_TSID_DISABLE			0xFF
	UCHAR  AcmAcId;			/* physical TS ID for EDCA or HCCA */

	UCHAR  FlgOutLink;		/* 1: in active table; 0: in client data base */
	UCHAR  Reserved2[3];

	UINT32  InactivityCur, SuspensionCur;	/* variable, miscroseconds */

	/* minimum physical mode & mcs */
	UCHAR	PhyModeMin;
	UCHAR	McsMin;

} ACM_STREAM_INFO;

/* Bandwidth information */
typedef struct _ACM_BANDWIDTH_INFO {

#ifdef ACM_CC_FUNC_MBSS
	UINT32	MbssTotalUsedTime;
#endif // ACM_CC_FUNC_MBSS //

	UINT32 AcmUsedTime;	/* current total used time for ACM streams */

	UINT32 AcUsedTime;	/* the allocated EDCA time */

	UINT32 NumReqLink;	/* requested link number */

	UINT32 NumAcLinkUp;	/* the number of EDCA uplink streams */
	UINT32 NumAcLinkDn;	/* the number of EDCA dnlink streams */
	UINT32 NumAcLinkDi;	/* the number of EDCA direct streams */
	UINT32 NumAcLinkBi;	/* the number of EDCA bidirectional streams */

	UINT16 StationCount;/* station count of the BSS */
	UINT16 AvalAdmCap;	/* available admission capcability of the BSS, 32us */
	UINT16 ChanUtil;	/* channel utilization */
	UINT32 ChanBusyTime;/* channel busy time */
} ACM_BANDWIDTH_INFO;

/* ACM control parameters information */
typedef struct _ACM_CTRL_INFO {

	UCHAR  FlgIsAcmEnable[ACM_DEV_NUM_OF_AC];	/* ACM is enabled or disabled */
	UCHAR  DowngradeAcNum[ACM_DEV_NUM_OF_AC];	/* not-zero: downgrade AC ID */

	/* currently, CP % = BEK % */
	UINT32 CP_MinNu, CP_MinDe;					/* Contention Period % */
	UINT32 BEK_MinNu, BEK_MinDe;				/* BE/BK % */

	/* total time = AC0~3 up + AC0~3 dn */
	/* out time = AC0~3 dn for AP mode; AC0~3 up for STA mode */
	/* ac time = AC0 up + AC0 dn or AC1 up + AC1 dn or ... */
	UINT32 AcmTotalTime;						/* total ACM used time */
	UINT32 AcmOutTime[ACM_DEV_NUM_OF_AC];		/* Out time for each AC */
	UINT32 AcmAcTime[ACM_DEV_NUM_OF_AC];		/* Used time for each AC */

	/* the number for uplink, dnlink, bidirectional link, or direct link */
	UINT32 LinkNumUp, LinkNumDn, LinkNumBi, LinkNumDi;

	/* dynamic ATL */
	UCHAR  FlgDatl; 							/* 1: enable DATL */
	UCHAR  DatlBwMin[ACM_DEV_NUM_OF_AC];		/* unit: 1/100 */
	UCHAR  DatlBwMax[ACM_DEV_NUM_OF_AC];		/* unit: 1/100 */

	UINT32	DatlBorAcBw[ACM_DEV_NUM_OF_AC][ACM_DEV_NUM_OF_AC];

#ifdef CONFIG_AP_SUPPORT
	UINT16 AvalAdmCapAc[ACM_DEV_NUM_OF_AC]; /* for each AC, unit: 32us */
#endif // CONFIG_AP_SUPPORT //

} ACM_CTRL_INFO;

/* ACM statistics */
typedef struct _ACM_STATISTICS {
	/* drop packets due to all ACM of AC is set */
	UINT32 DropByACM;

	/* drop packets due to insufficient medium time */
	UINT32 DropByAdmittedTime;

	/* change the priority due to no TSPEC is found */
	UINT32 PriorityChange[ACM_DEV_NUM_OF_AC];

	/* downgrade over-medium time packet to other priority */
	UINT32 Downgrade[ACM_DEV_NUM_OF_AC];

#ifdef ACM_CC_FUNC_11N
	/* software predicted AMPDU number */
	/* [0],[1]:no use, [2] ~ [63] */
	UINT16 AMPDU[64]; /* maximum 64 packets in a AMPDU */
#endif // ACM_CC_FUNC_11N //
} ACM_STATISTICS;



/* general public function (QAP) */
/*
========================================================================
Routine Description:
	Initialize the ACM Module.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsAcm0Enable	- the ACM flag for AC0
	FlgIsAcm1Enable	- the ACM flag for AC1
	FlgIsAcm2Enable	- the ACM flag for AC2
	FlgIsAcm3Enable	- the ACM flag for AC3
	FlgDatl			- the Dynamic ATL flag

Return Value:
	ACM_RTN_OK		- init OK
	ACM_RTN_FAIL	- init fail

Note:
	FlgIsAcm0Enable ~ FlgIsAcm3Enable and FlgDatl are valid only for QAP mode.
========================================================================
*/
#ifdef CONFIG_AP_SUPPORT
ACM_FUNC_STATUS ACMP_Init(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsAcm0Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm1Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm2Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm3Enable,
	ACM_PARAM_IN	UCHAR					FlgDatl);
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
ACM_FUNC_STATUS ACMP_Init(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);
#endif // CONFIG_STA_SUPPORT //

/*
========================================================================
Routine Description:
	Release the ACM Resource.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- release OK
	ACM_RTN_FAIL	- release fail

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_Release(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Initialize the ACM ASIC setting.

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	None

Note:
	Must be called after ACMP_Init() is called.
========================================================================
*/
ACM_EXTERN VOID ACMP_Asic_Init(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Get bandwidth information.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- the bandwidth information

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_BandwidthInfoGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_OUT	ACM_BANDWIDTH_INFO		*pInfo);

/*
========================================================================
Routine Description:
	Set bandwidth information.

Arguments:
	pAd				- WLAN control block pointer
	StationCount	- station count of the associated BSS
	ChanUtil		- channel utilization of the associated BSS
	AvalAdmCap		- available admission capability of the associated BSS

Return Value:
	ACM_RTN_OK		- set ok
	ACM_RTN_FAIL	- set fail

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_BandwidthInfoSet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT16					StationCount,
	ACM_PARAM_IN	UINT8					ChanUtil,
	ACM_PARAM_IN	UINT16					AvalAdmCap);

/*
========================================================================
Routine Description:
	Check if the BE packet is needed to release.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the destination QSTA
	QueIdx			- 0 ~ 3 (AC0 ~ AC3)
	*pQueueHeader	- the software queue header
	pMbuf			- the packet expected to send out

Return Value:
	ACM_RTN_OK		- release it
	ACM_RTN_FAIL	- do not release it

Note:
	If we can find a TSPEC for the BE packet, we will search a non-TSPEC
	packet in the BE software queue and release it.
========================================================================
*/
ACM_FUNC_STATUS ACMP_BE_IsReallyToReleaseWhenQueFull(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					QueIdx,
	ACM_PARAM_IN	ACMR_QUEUE_HEADER		*pQueueHeader,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf);

/*
========================================================================
Routine Description:
	Handle the event when BE software queue is full.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the destination QSTA
	pMbuf			- the packet expected to send out

Return Value:
	ACM_RTN_OK		- handle ok, a non-TSPEC packet is released
	ACM_RTN_FAIL	- handle fail, no non-TSPEC packet is released

Note:
	If the ACM of BE is enabled and the bandwidth is saturated, we will
	protect the bandwidth for BE traffic with TSPEC.

	Two cases:
	1. (only for AP) Origin priority is BE but no any BE TSPEC is built
		for the station, we do NOT need to protect it.
	2. (for AP & STA) Origin priority is not BE but no any non-BE TSPEC
		is built for the station, we will translate it to BE traffic,
		but we do NOT need to protect it.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_BE_QueueFullHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf);

/*
========================================================================
Routine Description:
	Get current EDCA ACM Information.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- the EDCA ACM information

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_ControlInfomationGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACM_CTRL_INFO			*pInfo);

/*
========================================================================
Routine Description:
	Handle something when a QoS data or null frame is received.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	pHeader			- the WLAN MAC header

Return Value:
	None

Note:
	1. Only for QAP.
	2. The frame shall be uplink.
	3. For EDCA, we shall reset activity timeout for QoS data frames.
	4. In LINUX, the function must be called in a tasklet.
	5. If PktTsid is 0xFF, we will get TSID from pHeader.
========================================================================
*/
ACM_EXTERN VOID ACMP_DataNullHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_WLAN_HEADER		*pHeader);

/*
========================================================================
Routine Description:
	Check if the packet can be queued to the packet queue of the AC.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the peer device
	*pMbuf				- the packet
	FlgIsForceToHighAc	- 1: force the packet to AC3
	*pQueueType			- the new packet queue type
							(ACMR_QID_AC_BE ~ ACMR_QID_AC_VO)

Return Value:
	ACM_RTN_OK			- classify successfully
	ACM_RTN_FAIL		- do not allow to send the packet
	ACM_RTN_NO_ACM		- the ACM of the AC is disabled

Note:
	Used the function to enqueue your WLAN packets.

	WMM ACM Test Plan 4.1.3 APUT Operation with Legacy STAs

	1. When one WMM ACM station and one WMM legacy station connects to
		our AP, and WMM ACM STA creates a TSPEC for dnlink.

	2. Send traffics from AP to two stations with priority VO.

	3. We need to enqueue VO packets for WMM ACM station to VO queue
		and enqueue the VO packets for WMM legacy station to BE queue.

	4. Or if we enqueue all VO packets to the VO queue and partition packets
		to different queue when Dequeue packets, a problem will occur as below:

		a. When the traffic from AP to WMM legacy station saturates the
			bandwidth;
		b. We will drop VO packets because the VO queue is full;
		c. And we will drop VO packets for WMM ACM station but it is not
			correct because WMM ACM station has created a TSPEC for the
			VO traffic.  So We need to protect the VO traffic of WMM ACM STA.
========================================================================
*/
ACM_FUNC_STATUS ACMP_DataPacketQueue(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf,
	ACM_PARAM_IN	UCHAR					FlgIsForceToHighAc,
	ACM_PARAM_OUT	UINT32					*pQueueType);

/*
========================================================================
Routine Description:
	Enable or disable Dynamic ATL function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable
	*pDatlBwMin		- new minimum bandwidth threshold
	*pDatlBwMax		- new maximum bandwidth threshold

Return Value:
	None

Note:
	if you dont want to change bandwidth threshold, you can input NULL.
	pDatlBwMin = NULL or pDatlBwMax = NULL
========================================================================
*/
ACM_EXTERN VOID ACMP_DatlCtrl(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable,
	ACM_PARAM_IN	UCHAR					*pDatlBwMin,
	ACM_PARAM_IN	UCHAR					*pDatlBwMax);

/*
========================================================================
Routine Description:
	Inform us that tx compleletion interrupt occurred.

Arguments:
	pAd				- WLAN control block pointer
	*pDevMac		- the destination MAC
	*pTsInfo		- the TS Info of the packet
	FlgIsErr		- if the frame tx is error

Return Value:
	None

Note:
	1. Responsible for DELTS ACK frame check.
========================================================================
*/
ACM_EXTERN VOID ACMP_DeltsFrameACK(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pDevMac,
	ACM_PARAM_IN	UCHAR					*pTsInfo,
	ACM_PARAM_IN	UCHAR					FlgIsErr);

#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Append the QBSS Load element to the beacon frame.

Arguments:
	pAd				- WLAN control block pointer
	*pPkt			- the beacon frame

Return Value:
	the element total length

Note:
========================================================================
*/
ACM_EXTERN UINT32 ACMP_Element_QBSS_LoadAppend(
 	ACM_PARAM_IN		ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN_OUT	UCHAR				*pPkt);
#endif // CONFIG_AP_SUPPORT //

/*
========================================================================
Routine Description:
	Reset current ACM Flag for each AC.

Arguments:
	pAd					- WLAN control block pointer
	FlgIsAcm0Enable		- the ACM flag for AC0
	FlgIsAcm1Enable		- the ACM flag for AC1
	FlgIsAcm2Enable		- the ACM flag for AC2
	FlgIsAcm3Enable		- the ACM flag for AC3

Return Value:
	None

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_EnableFlagReset(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsAcm0Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm1Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm2Enable,
	ACM_PARAM_IN	UCHAR					FlgIsAcm3Enable);

/*
========================================================================
Routine Description:
	Resume the ACM.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_FSM_Resume(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Suspend the ACM.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
	QSTA: No any TSPEC request can be issued.
	QAP: No any TSPEC request can be handled.
========================================================================
*/
ACM_EXTERN VOID ACMP_FSM_Suspend(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Return TRUE if the ACM of all AC are enabled.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- the ACM of all AC is enabled
	ACM_RTN_FAIL	- the ACM of one AC is disabled

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_IsAllACEnabled(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Return TRUE if the ACM of any AC is enabled.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	ACM_RTN_OK		- the ACM of any AC is enabled
	ACM_RTN_FAIL	- the ACM of all AC is disabled

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_IsAnyACEnabled(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Return TRUE if the frame is Bandwidth Announce Action Frame.

Arguments:
	pAd				- WLAN control block pointer
	*pMbuf			- the frame

Return Value:
	ACM_RTN_OK		- Yes
	ACM_RTN_FAIL	- No

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_IsBwAnnounceActionFrame(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	VOID					*pMbuf);

/*
========================================================================
Routine Description:
	Check if the action frame is the DELTS frame.

Arguments:
	*pMblk			- the action frame

Return Value:
	TRUE			- Yes
	FALSE			- No

Note:
========================================================================
*/
ACM_EXTERN BOOLEAN ACMP_IsDeltsFrame(
	ACM_PARAM_IN	UCHAR					*pMblk);

/*
========================================================================
Routine Description:
	Check if the packet needs to do ACM.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the destination QSTA
	UP				- user priority

Return Value:
	TRUE			- Yes
	FALSE			- No

Note:
	When the ACM of AC is set, the packet is needed to do ACM.

	Only used in transmission path.
========================================================================
*/
BOOLEAN ACMP_IsNeedToDoAcm(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UCHAR					UP);

/*
========================================================================
Routine Description:
	Check if the action frame is the ADDTS Response frame.

Arguments:
	*pMblk			- the action frame

Return Value:
	TRUE			- Yes
	FALSE			- No

Note:
========================================================================
*/
ACM_EXTERN BOOLEAN ACMP_IsResponseFrame(
	ACM_PARAM_IN	UCHAR					*pMblk);

/*
========================================================================
Routine Description:
	Handle the management action frame.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	SubType			- the subtype of the frame
	pMblk			- the received frame
	PhyRate			- the physical tx rate for the frame

Return Value:
	ACM_RTN_OK		- pMblk is released or forwarded
	ACM_RTN_FAIL	- handle ok and pMblk is not released

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_ManagementHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	UINT32					SubType,
	ACM_PARAM_IN	UCHAR					*pMblk,
	ACM_PARAM_IN	UINT32					PktLen,
	ACM_PARAM_IN	UINT32					PhyRate);

/*
========================================================================
Routine Description:
	Classify the QoS frame to a AC queue.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- the source QSTA
	pMbuf				- the received frame
	QueueTypeCur		- the current used queue type
	FlgIsForceToHighAc	- 1: force the packet to AC3
	AggType				- ACMR_AGG_RALINK or ACMR_AGG_AMSDU
	AggId				- the aggregation number, base 1

Return Value:
	Queue Type: AC0 ~ AC3
	not AC0 ~ AC3: can not transmit

Note:
	Suppose the Tx Rate is not changed between ACMP_DataPacketQueue()
	and ACMP_MsduClassify().
========================================================================
*/
ACM_EXTERN UINT32 ACMP_MsduClassify(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf,
	ACM_PARAM_IN	UINT32					QueueTypeCur,
	ACM_PARAM_IN	BOOLEAN					FlgIsForceToHighAc,
	ACM_PARAM_IN	UCHAR					AggType,
	ACM_PARAM_IN	UCHAR					AggId);

/*
========================================================================
Routine Description:
	Get new adjust parameters for non-ACM AC.

Arguments:
	pAd				- WLAN control block pointer
	*pParam			- the parameters

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- get fail

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_NonAcmAdjustParamUpdate(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pParam);

/*
========================================================================
Routine Description:
	Get current number of input TSPEC for the UP.

Arguments:
	*pCdb			- the QSTA
	UP				- the UP

Return Value:
	Number of TSPEC

Note:
========================================================================
*/
ACM_EXTERN UINT32 ACMP_NumOfAcTspecInGet(
	ACM_PARAM_IN		ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN		UCHAR				UP);

/*
========================================================================
Routine Description:
	Get current number of output TSPEC for the UP.

Arguments:
	*pCdb			- the QSTA
	UP				- the UP

Return Value:
	Number of TSPEC

Note:
========================================================================
*/
ACM_EXTERN UINT32 ACMP_NumOfAcTspecOutGet(
	ACM_PARAM_IN		ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN		UCHAR				UP);

/*
========================================================================
Routine Description:
	Check if the current tx PHY Mode and MCS > minimum PHY Mode and MCS.

Arguments:
	pAd				- WLAN control block pointer
	*pMbuf			- the frame expected to transmit
	FlgIs2040		- 1: the packet uses 40MHz
	FlgIsShortGI	- 1: the packet uses Short GI
	PhyMode			- the PHY Mode expected to use
	Mcs				- the MCS expected to use

Return Value:
	ACM_RTN_OK		- current Mode & MCS is allowed
	ACM_RTN_FAIL	- current Mode & MCS is not allowed

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_PacketPhyModeMCSCheck(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_MBUF				*pMbuf,
	ACM_PARAM_IN	UCHAR					FlgIs2040,
	ACM_PARAM_IN	UCHAR					FlgIsShortGI,
	ACM_PARAM_IN	UCHAR					PhyMode,
	ACM_PARAM_IN	UCHAR					Mcs);

/*
========================================================================
Routine Description:
	Return power save right to system.

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	None

Note:
	1. Only for STATION mode.
	2. We will return PS right when no any pending ADDTS request frame.
========================================================================
*/
ACM_EXTERN VOID ACMP_StaPsCtrlRightReturn(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Used to signal WMM ACM support in the AP element.

Arguments:
	pAd				- WLAN control block pointer
	*pWmeElement	- the WMM Parameter IE

Return Value:
	None

Note:
	Used in QAP.
========================================================================
*/
ACM_EXTERN VOID ACMP_NullTspecSupportSignal(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN_OUT	UCHAR				*pWmeElement);

/*
========================================================================
Routine Description:
	Update UAPSD states after ADDTS Response or DELTS is sent out.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA
	*pActFrameBody	- the action frame

Return Value:
	None

Note:
	Used in QAP.
========================================================================
*/
ACM_EXTERN VOID ACMP_PsRspDeltsSentOutHandle(
	ACM_PARAM_IN		ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN		ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN_OUT	UCHAR				*pActFrameBody);

/*
========================================================================
Routine Description:
	Handle the resource allocation.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	*pBufRscReq		- the buffer which includes the TSPEC request
	*pBufRscRsp		- the buffer where we can put TSPEC response
	*pBufRspLen		- the respone frame length

Return Value:
	ACM_RTN_OK		- handle ok
	ACM_RTN_FAIL	- handle fail

Note:
	1. Used in QAP.
	2. Currently only TSPEC element for request/response, no TCLAS.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_ResourceAllocate(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pBufRscReq,
	ACM_PARAM_OUT	UCHAR				*pBufRscRsp,
	ACM_PARAM_OUT	UINT32				*pBufRspLen);

/*
========================================================================
Routine Description:
	Update the UAPSD state based on current all TSPECs.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA

Return Value:
	None

Note:
	Used in QAP.

	Use the function after reassociation request.

	Because TSPEC is not deleted after reassociation request, we need
	to update new UAPSD state based on these TSPEC and recover to some
	static settings in reassociation frame after any TSPEC is deleted.
========================================================================
*/
ACM_EXTERN VOID ACMP_UAPSD_StateUpdate(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb);
#endif // CONFIG_AP_SUPPORT //

/*
========================================================================
Routine Description:
	Get ACM related statistics counts.

Arguments:
	pAd				- WLAN control block pointer
	*pStats			- the statistics counts

Return Value:
	None

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_StatisticsGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_OUT	ACM_STATISTICS			*pStats);

/*
========================================================================
Routine Description:
	Delete a QSTA due to deauthentication or deassociation, etc.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the QSTA

Return Value:
	None

Note:
	Used in QAP.
========================================================================
*/
ACM_EXTERN VOID ACMP_StationDelete(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb);

/*
========================================================================
Routine Description:
	Clear failed stream information.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_StreamFailClear(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Get some streams information.

Arguments:
	pAd				- WLAN control block pointer
	Category		-	ACM_STM_CATEGORY_REQ,
						ACM_STM_CATEGORY_ACT,
						ACM_SM_CATEGORY_PEER,
						ACM_STM_CATEGORY_ERR
	Type			- ACM_ACCESS_POLICY_EDCA
	*pNumStm		- the number of streams, must > 0
	*pDevMac		- the QSTA MAC
	*pStreamBuf		- the stream information buffers

Return Value:
	ACM_RTN_OK		- get ok
	ACM_RTN_FAIL	- no more stream

Note:
	1. if stream_p->pTspec == NULL, the function will not copy
		TSPEC information.
	2. if stream_p->pTclas[i] == NULL, the function will not
		copy TCLAS information.
	3. If you want to get all stream information, you shall call
		ACMP_StreamNumGet() first.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_StreamsGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					Category,
	ACM_PARAM_IN	UINT32					Type,
	ACM_PARAM_IN	UINT32					*pNumStm,
	ACM_PARAM_IN	UCHAR					*pDevMac,
	ACM_PARAM_OUT	ACM_STREAM_INFO			*pStreamBuf);

/*
========================================================================
Routine Description:
	Get the number of streams.

Arguments:
	pAd				- WLAN control block pointer
	Category		-	ACM_STM_CATEGORY_REQ,
						ACM_STM_CATEGORY_ACT,
						ACM_SM_CATEGORY_PEER,
						ACM_STM_CATEGORY_ERR
	Type			- ACM_ACCESS_POLICY_EDCA
	*pDevMac		- the QSTA MAC

Return Value:
	current number of streams

Note:
========================================================================
*/
ACM_EXTERN UINT32 ACMP_StreamNumGet(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UINT32					Category,
	ACM_PARAM_IN	UINT32					Type,
	ACM_PARAM_IN	UCHAR					*pDevMac);

/*
========================================================================
Routine Description:
	Translate factor decimal part binary to decimal. (unit: 1/100)

Arguments:
	BIN				- the binary of decimal part

Return Value:
	the decimal

Note:
	Ex: 0b0001 1000 0000 0000 ==> 0.75
========================================================================
*/
UINT32 ACM_SurplusFactorDecimalBin2Dec(
	ACM_PARAM_IN	UINT32				BIN);

/*
========================================================================
Routine Description:
	Delete all activated TSPECs.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	1. Send a DELTS to the QSTA or QAP.
	2. Insert the activated TSPEC to the requested list.
	3. The TSPEC will be moved to the failed list when DELTS ACK
		is received or retry count is reached.
========================================================================
*/
ACM_EXTERN VOID ACMP_TC_DeleteAll(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Delete a TSPEC silently.

Arguments:
	pAd				- WLAN control block pointer
	*pMacPeer		- the MAC of peer
	TID				- the TID of the TSPEC

Return Value:
	TRUE			- find it and delete it ok
	FALSE			- do not find it or delete it fail

Note:
	For QAP, the pMacPeer means the MAC of a station;
	For QSTA, the pMacPeer means the MAC of associated AP;

	No DELTS frame is sent.
========================================================================
*/
BOOLEAN ACMP_TC_DeleteOneSilently(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pMacPeer,
	ACM_PARAM_IN	UCHAR					TID);

#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Enable or disable all TSPEC rejection function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable

Return Value:
	None

Note:
	Only for QAP.
========================================================================
*/
ACM_EXTERN VOID ACMP_TC_RejectCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable);
#endif // CONFIG_AP_SUPPORT //

/*
========================================================================
Routine Description:
	Enable or disable TSPEC timeout function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable

Return Value:
	None

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_TC_TimeoutCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable);

#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
	Send a renegotiated TSPEC request to the QAP.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- our STATION entry
	*pTspecSrc			- the requested TSPEC pointer
	TclasNum			- the number of TCLASS, max 5
	*pTclasSrc			- the requested TCLASS array pointer
	TclasProcessing		- 1: must match all TCLAS
	StreamType			- the stream type: WME stream

Return Value:
	ACM_RTN_OK				- request is accepted
	ACM_RTN_FAIL			- semaphore lock fail or others
	ACM_RTN_NULL_POINTER	- null pointer
	ACM_RTN_NOT_EXIST		- the old TSPEC does not exist
	ACM_RTN_INVALID_PARAM	- invalid input parameters
	ACM_RTN_SEM_GET_ERR		- get semaphore fail
	ACM_RTN_FATAL_ERR		- can not call the func in error mode
	ACM_RTN_NO_FREE_TS		- no free TS ID or same TSID & direction
	ACM_RTN_ALLOC_ERR		- TSPEC request structure allocation fail

Note:
	1. Only for non-IBSS Station Mode.
	2. DLP is not allowed.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_TC_Renegotiate(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACM_TSPEC				*pTspecSrc,
	ACM_PARAM_IN	UINT32					TclasNum,
	ACM_PARAM_IN	ACM_TCLAS				*pTclasSrc,
	ACM_PARAM_IN	UCHAR					TclasProcessing,
	ACM_PARAM_IN	UCHAR					StreamType);

/*
========================================================================
Routine Description:
	Adjust retry count limit automatically.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	Only for QSTA.

	No retry count for 'active' mode station.
========================================================================
*/
VOID ACMP_RetryCountCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd);

/*
========================================================================
Routine Description:
	Enable or disable TSPEC UAPSD function.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsEnable		- 1: enable; 0: disable

Return Value:
	None

Note:
	Only for QSTA.

	If TSPEC UAPSD function is disabled, the UAPSD field of TSPEC will
	be same as the value in station association request frame.
========================================================================
*/
VOID ACMP_TC_UapsdCtrl(
 	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					FlgIsEnable);

/*
========================================================================
Routine Description:
	Check a WME TSPEC element in a buffer.

Arguments:
	*pBuffer		- the buffer
	*pTid			- the TID of the TSPEC element
	*pMediumTime	- the medium time of the TSPEC element

Return Value:
	TRUE			- TSPEC element
	FALSE			- not TSPEC element

Note:
========================================================================
*/
BOOLEAN ACMP_WME_TSPEC_ElementCheck(
	ACM_PARAM_IN	UCHAR					*pBuffer,
	ACM_PARAM_OUT	UINT32					*pTid,
	ACM_PARAM_OUT	UINT32					*pMediumTime);
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_STA_SUPPORT_SIM
/*
========================================================================
Routine Description:
	Fill a TSPEC element to a buffer.

Arguments:
	pAd				- WLAN control block pointer
	*pBuffer		- the buffer
	*pTspec11e		- the current TSPEC

Return Value:
	filled element length

Note:
========================================================================
*/
UINT32 ACMP_WME_TSPEC_ElementFill(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	UCHAR					*pBuffer,
	ACM_PARAM_IN	ACM_TSPEC				*pTspec11e);
#endif // CONFIG_STA_SUPPORT_SIM //

/*
========================================================================
Routine Description:
	Timer APIs are provided for WLAN module use.

Arguments:
	ACM_TIMER_API_PARAM

Return Value:
	None

Note:
========================================================================
*/
ACM_EXTERN VOID ACMP_TR_TC_ReqCheck(ACM_TIMER_API_PARAM);
ACM_EXTERN VOID ACMP_TR_STM_Check(ACM_TIMER_API_PARAM);
ACM_EXTERN VOID ACMP_TR_TC_General(ACM_TIMER_API_PARAM);
ACM_EXTERN VOID ACMP_CMD_Timer_Data_Simulation(ACM_TIMER_API_PARAM);

/* general public function (QSTA) */
#ifdef CONFIG_STA_SUPPORT_SIM
/*
========================================================================
Routine Description:
	Send a TSPEC request to the QAP.

Arguments:
	pAd					- WLAN control block pointer
	*pCdb				- our STATION entry
	*pTspecSrc			- the requested TSPEC pointer
	TclasNum			- the number of TCLASS, max 5
	*pTclasSrc			- the requested TCLASS array pointer
	TclasProcessing		- 1: must match all TCLAS
	StreamType			- the stream type: WME stream

Return Value:
	ACM_RTN_OK				- request is accepted
	ACM_RTN_FAIL			- semaphore lock fail or others
	ACM_RTN_NULL_POINTER	- null pointer
	ACM_RTN_INVALID_PARAM	- invalid input parameters
	ACM_RTN_SEM_GET_ERR		- get semaphore fail
	ACM_RTN_FATAL_ERR		- can not call the func in error mode
	ACM_RTN_NO_FREE_TS		- no free TS ID or same TSID & direction
	ACM_RTN_ALLOC_ERR		- TSPEC request structure allocation fail

Note:
	1. Only for non-IBSS Station Mode.
	2. pTclasSrc is limited by ACM_TCLAS_MAX_NUM.
	3. DLP TSPEC is not allowed but DLP is allowed.
	4. *pTspecSrc & *pTclasSrc[ ] can not be freed in calling function.
	5. For WMM STA, the used TSPEC is the same.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACMP_WME_TC_Request(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC		pAd,
	ACM_PARAM_IN	ACMR_STA_DB				*pCdb,
	ACM_PARAM_IN	ACM_TSPEC				*pTspecSrc,
	ACM_PARAM_IN	UINT32					TclasNum,
	ACM_PARAM_IN	ACM_TCLAS				*pTclasSrc,
	ACM_PARAM_IN	UCHAR					TclasProcessing,
	ACM_PARAM_IN	UCHAR					StreamType,
	ACM_PARAM_IN	UINT16              	DialogToken); //snowpin
#endif // CONFIG_STA_SUPPORT_SIM //


/* EDCA public function (QAP) */
/*
========================================================================
Routine Description:
	Change current EDCA Information.

Arguments:
	pAd				- WLAN control block pointer
	CPnu			- the numerator of Contention Period,
						if 0, no any update for CPnu
	CPde			- the denominator of Contention Period
						if 0, no any update for CPde
	BEnu			- the numerator of Best Effort percentage,
						if 0, no any update for BEnu
	BEde			- the denominator of Best Effort percentage
						if 0, no any update for BEde

Return Value:
	ACM_RTN_OK		- change ok
	ACM_RTN_FAIL	- change fail

Note:
	1. CPnu/CPde is the percentage of EDCA in Service Interval,
		only valid when HCCA is enabled.
	2. BEnu/BEde is the percentage of Best Effort streams in 1 second.
	3. The function will not delete any stream if residunt
		bandwidth is not enough for (CPnu/CPde)*SI or (BEnu/BEde).
	4. New (CPnu/CPde) or (BEnu/BEde) will be valid after bandwidth is enough.
	5. If the old ACM is enabled and the new ACM is disabled,
		the function will not deleted these streams use the AC.
	6. If flg_acm = 0xFF, it means old ACM flag is kept.
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACM_EDCA_InfomationChange(
	ACM_PARAM_IN ACMR_PWLAN_STRUC			pAd,
	ACM_PARAM_IN UINT16   					CPnu,
	ACM_PARAM_IN UINT16   					CPde,
	ACM_PARAM_IN UINT16   					BEnu,
	ACM_PARAM_IN UINT16   					BEde);

/*
========================================================================
Routine Description:
	Check whether the element is WME Information.

Arguments:
	pAd					- WLAN control block pointer
	*pElm				- the element
	SubType				- the sub type

Return Value:
	ACM_RTN_OK			- check ok
	ACM_RTN_FAIL		- check fail

Note:
========================================================================
*/
ACM_EXTERN ACM_FUNC_STATUS ACM_WME_ELM_Check(
	ACM_PARAM_IN	UCHAR					*pElm,
	ACM_PARAM_IN	UCHAR					SubType);

/*
========================================================================
Routine Description:
	Test command.

Arguments:
	pAd					- WLAN control block pointer
	pArgv				- input parameters

Return Value:
	TRUE				- check ok
	FALSE				- check fail

Note:
========================================================================
*/
ACM_EXTERN INT ACM_Ioctl(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC 		pAd,
	ACM_PARAM_IN	PSTRING					pArgv);

/* Timer Handler */
/* (timer) activate periodically */
ACM_EXTERN VOID ACM_TR_TC_General(
	ACM_PARAM_IN	ULONG				Data);

/* (timer) periodical check for outgoing TSPECs in the requested list */
ACM_EXTERN VOID ACM_TR_TC_ReqCheck(
	ACM_PARAM_IN	ULONG				Data);

/* (timer) periodical check inactivity and suspension for activated streams */
ACM_EXTERN VOID ACM_TR_STM_Check(
	ACM_PARAM_IN	ULONG				Data);

/* (timer) simulation cmd tx/rx */
ACM_EXTERN VOID ACM_CMD_Timer_Data_Simulation(
	ACM_PARAM_IN	ULONG				Data);

#endif // __ACM_EXTR_H__ //

/* End of acm_extr.h */

