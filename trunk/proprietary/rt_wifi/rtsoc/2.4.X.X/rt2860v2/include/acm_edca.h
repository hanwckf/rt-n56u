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

	All EDCA Related Structure & Definition.

***************************************************************************/

#ifndef __ACM_EDCA_H__
#define __ACM_EDCA_H__


/* ========================== Definition ==================================== */

#define ACM_IS_EDCA_STREAM(__TID)		((__TID) <= 7)
#define ACM_MR_EDCA_AC(__TID)			(gEDCA_UP_AC[(__TID)&0x07])
#define ACM_MR_EDCA_UP(__TID)			((__TID) & 0x0f)
#define ACM_EDCA_MAX_UP					7 /* 0 ~ 7 */

/* get User Priority for the DSCP value */
#define ACM_MR_DSCP_MAPPING(__DSCP)		(gEDCA_UP_DSCP[(__DSCP)])

#define ACM_EDCA_BE_AC_QUE_ID			0 /* UP = 0 or 3 */
#define ACM_EDCA_BK_AC_QUE_ID			1 /* UP = 1 or 2 */
#define ACM_EDCA_VI_AC_QUE_ID			2 /* UP = 4 or 5 */
#define ACM_EDCA_VO_AC_QUE_ID			3 /* UP = 6 or 7 */

#define ACM_EDCA_MAX_QSTA_DN_LINK		4 /* WMM requirement, can not change */
#define ACM_EDCA_MAX_QSTA_UP_LINK		4 /* WMM requirement, can not change */
#define ACM_EDCA_MAX_QSTA_TS_FOR_A_AC	1 /* WMM requirement, can not change */


/* ========================== Structure ===================================== */


typedef struct _ACM_ACI_AIFSN {

	UCHAR AIFSN:4; /* bit 0 */
	UCHAR ACM:1;
	UCHAR ACI:2;
	UCHAR Reserved:1;
} GNU_PACKED ACM_ACI_AIFSN;

typedef struct _ACM_AC_PARAM_RECORD {

	ACM_ACI_AIFSN ACI_AIFSN; /* 1B */

	/* CWmax = 2^ECW_Max - 1 */
	/* CWmin = 2^ECW_Min - 1 */
	UCHAR ECW_Min:4;
	UCHAR ECW_Max:4;

	UINT16 TXOP_Limit; /* in units of 32 microseconds */
} GNU_PACKED ACM_AC_PARAM_RECORD;

typedef struct _ACM_ELM_EDCA_PARAM {

	UCHAR ElementId;
	UCHAR Length;

	ACM_QOS_INFO QOS_Info; /* 1B */
	UCHAR Reserved;

	ACM_AC_PARAM_RECORD AC_BE;	/* best effort */
	ACM_AC_PARAM_RECORD AC_BK;	/* background */
	ACM_AC_PARAM_RECORD AC_VI;	/* video */
	ACM_AC_PARAM_RECORD AC_VO;	/* voice */
} GNU_PACKED ACM_ELM_EDCA_PARAM;


/* ----- WMM v1.1 ----- */
#ifdef ACM_CC_FUNC_WMM
typedef struct _ACM_WME_AC_INFO {

#ifndef APPS_INCLUDE_WLAN_11E_FINAL_EDCA_WMM_UAPSD
	UCHAR ParamSetCount:4;
	UCHAR Reserved:4;
#else

	union {
		UCHAR Info;

		struct _AP {
			UCHAR ParamSetCount:4;
			UCHAR Reserved:3;
			UCHAR FlgUapsd:1;     	/* 1: support U-APSD */
		} AP; /* QAP */

		struct _STA {
#define WME_UAPSD_STA_SUP_MASK	0x0F
			UCHAR FlgUapsdVO:1;
			UCHAR FlgUapsdVI:1;
			UCHAR FlgUapsdBK:1;
			UCHAR FlgUapsdBE:1;
			UCHAR FlgQACK:1;
			UCHAR MaxSpLen:2;
			UCHAR FlgMoreDataACK:1;
		} STA; /* QSTA */
	} OP;

#define WME_UAPSD_OP_AP		OP.AP
#define WME_UAPSD_OP_STA	OP.STA
#endif
} GNU_PACKED ACM_WME_AC_INFO;

#define ACM_WME_OUI_HDR_LEN		6
#define ACM_WME_OUI_0			0x00
#define ACM_WME_OUI_1			0x50
#define ACM_WME_OUI_2			0xf2

#define ACM_WME_OUI_TYPE		0x02
#define ACM_WME_OUI_VERSION		0x01

#define ACM_WME_OUI_ID_OFFSET	6 /* sub ID field offset from element ID */

typedef struct _ACM_ELM_WME_INFO {

	UCHAR ElementId;			/* ACM_ELM_WME_ID */
	UCHAR Length;				/* ACM_ELM_WME_INFO_LEN */

	UCHAR OUI[3];				/* 00:50:f2 (hex) */
	UCHAR OUI_Type;				/* ACM_WME_OUI_TYPE */
	UCHAR OUI_SubType;			/* WS_WME_OUI_SUBTYPE_INFO */
	UCHAR Version;				/* ACM_WME_OUI_VERSION */

	ACM_WME_AC_INFO AcInfo;
} GNU_PACKED ACM_ELM_WME_INFO;

typedef struct _ACM_WME_ACI_AIFSN {

	UCHAR AIFSN:4;			/* bit 0 ~ 3 */
	UCHAR ACM:1;			/* bit 4 */
	/* -------------------------------------	*/
	/* ACI	AC		Access Category 			*/
	/* -------------------------------------	*/
	/* 00	AC_BE   Best Effort					*/
	/* 01	AC_BK   Background					*/
	/* 10	AC_VI   Video						*/
	/* 11	AC_VO   Voice						*/
	/* -------------------------------------	*/
	UCHAR ACI:2;			/* bit 5 ~ 6 */
	UCHAR Reserved:1;		/* bit 7 */
} GNU_PACKED ACM_WME_ACI_AIFSN;

typedef struct _ACM_WME_AC_PARAM {

	ACM_WME_ACI_AIFSN ACI_AIFSN;

	UCHAR ECW_Min:4;
	UCHAR ECW_Max:4;

	UINT16 TXOP_Limit;
} GNU_PACKED ACM_WME_AC_PARAM;

typedef struct _ACM_ELM_WME_PARAM {

	UCHAR ElementId;		/* ACM_ELM_WME_ID */
	UCHAR Length;			/* ACM_ELM_WME_PARAM_LEN */

	UCHAR OUI[3];			/* 00:50:f2 (hex) */
	UCHAR OUI_Type;			/* ACM_WME_OUI_TYPE */
	UCHAR OUI_SubType;		/* WS_WME_OUI_SUBTYPE_PARAM */
	UCHAR Version;			/* ACM_WME_OUI_VERSION */

	ACM_WME_AC_INFO AcInfo;
	UCHAR Reserved;
	ACM_WME_AC_PARAM AC_BE;
	ACM_WME_AC_PARAM AC_BK;
	ACM_WME_AC_PARAM AC_VI;
	ACM_WME_AC_PARAM AC_VO;
} GNU_PACKED ACM_ELM_WME_PARAM;

typedef struct _ACM_WME_TS_INFO {

	UCHAR Reserved4:1;	/* traffic type */
	UCHAR TID:4;
	/* -------------------------------	*/
	/* Direction		Meaning			*/
	/* -------------------------------	*/
	/* 00			Uplink				*/
	/* 01			Downlink			*/
	/* 10			Reserved			*/
	/* 11			Bi-directional		*/
	/* -------------------------------	*/
	/*	ACM_DIRECTION_UP_LINK
		ACM_DIRECTION_DOWN_LINK
		ACM_DIRECTION_BIDIREC_LINK		*/
	UCHAR Direction:2;
	UCHAR One:1;		/* always 1 */

	UCHAR Zero1:1;		/* always 0, aggregation */
	UCHAR Reserved3:1;
	UCHAR PSB:1;
	UCHAR UP:3;
	UCHAR Reserved2:2;	/* ack policy */

	UCHAR Reserved1;	/* bit16: 802.11e S-APSD */
} GNU_PACKED ACM_WME_TS_INFO;

typedef struct _ACM_WME_TSPEC {

	ACM_WME_TS_INFO TsInfo;		/* 3B */

	UINT16  NominalMsduSize;
	UINT16  MaxMsduSize;		/* dont care */

	UINT32  MinServInt;			/* dont care */
	UINT32  MaxServInt;			/* dont care */
	UINT32  InactivityInt;		/* must be assigned, if no, default 1 minute */
	UINT32  SuspensionInt;		/* dont care */
	UINT32  ServiceStartTime;	/* dont care */
	UINT32  MinDataRate;		/* dont care */
	UINT32  MeanDataRate;
	UINT32  PeakDataRate;		/* dont care */
	UINT32  MaxBurstSize;		/* dont care */
	UINT32  DelayBound;			/* dont care */
	UINT32  MinPhyRate;

	UINT16  SurplusBandwidthAllowance;
	UINT16  MediumTime;
} GNU_PACKED ACM_WME_TSPEC;

typedef struct _ACM_ELM_WME_TSPEC {

	UCHAR ElementId;			/* ACM_ELM_WME_ID */
	UCHAR Length;				/* ACM_ELM_WME_TSPEC_LEN */

	UCHAR OUI[3];				/* 00:50:f2 (hex) */
	UCHAR OUI_Type;				/* ACM_WME_OUI_TYPE */
	UCHAR OUI_SubType;			/* ACM_WME_OUI_SUBTYPE_TSPEC */
	UCHAR Version;				/* ACM_WME_OUI_VERSION */

	ACM_WME_TSPEC Tspec;
} GNU_PACKED ACM_ELM_WME_TSPEC;

typedef struct _ACM_ELM_WME_TCLAS {

	UCHAR ElementId;			/* ACM_ELM_WME_ID */
	UCHAR Length;				/* ACM_ELM_WME_TSPEC_LEN */

	UCHAR OUI[3];				/* 00:50:f2 (hex) */
	UCHAR OUI_Type;				/* ACM_WME_OUI_TYPE */
	UCHAR OUI_SubType;			/* ACM_WSM_OUI_SUBTYPE_TCLAS */
	UCHAR Version;				/* ACM_WME_OUI_VERSION */

	ACM_TCLAS Tclas;
} GNU_PACKED ACM_ELM_WME_TCLAS;

typedef struct _ACM_ELM_WME_TCLAS_PROCESSING {

	UCHAR ElementId;			/* ACM_ELM_WME_ID */
	UCHAR Length;				/* ACM_ELM_WME_TSPEC_LEN */

	UCHAR OUI[3];				/* 00:50:f2 (hex) */
	UCHAR OUI_Type;				/* ACM_WME_OUI_TYPE */
	UCHAR OUI_SubType;			/* ACM_WSM_OUI_SUBTYPE_TCLAS_PROCESSING */
	UCHAR Version;				/* ACM_WME_OUI_VERSION */

#define ACM_WME_TCLAS_PROCESSING_ALL		0
#define ACM_WME_TCLAS_PROCESSING_ONE		1
	UCHAR Processing;
} GNU_PACKED ACM_ELM_WME_TCLAS_PROCESSING;

/* WME notification Frame */
typedef struct _ACM_WME_NOT_FRAME {

	/* 17: WME notification frame (ACM_CATEGORY_WME) */
	UCHAR Category;

	/* 0: setup request (ACM_ACTION_WME_SETUP_REQ);
	   1: setup response (WS_ACTION_WME_SETUP_RSP);
	   2: teardown (ACM_ACTION_WME_TEAR_DOWN);
	   255: bandwidth announce */
	UCHAR Action;

	/* For teardown frame, the field must be 0 */
	UCHAR DialogToken;

#define WLAN_STATUS_CODE_WME_ACM_ACCEPTED		0x00
#define WLAN_STATUS_CODE_WME_INVALID_PARAM		0x01
#define WLAN_STATUS_CODE_WME_REFUSED			0x03
	/* For setup request or teardown frame, the field must be 0 */
	UCHAR StatusCode;

	ACM_ELM_WME_TSPEC ElmTspec;

	/* max 5 TCLASS & 1 TCLASS Processing or none */
	UCHAR Tclas[sizeof(ACM_ELM_WME_TCLAS) * ACM_TSPEC_TCLAS_MAX_NUM +
				sizeof(ACM_ELM_WME_TCLAS_PROCESSING)];

} GNU_PACKED ACM_WME_NOT_FRAME;
#endif // ACM_CC_FUNC_WMM //


/* ======================= EDCA Function definition ======================= */

/* return EDCA used time */
STATIC void ACM_EDCA_AllocatedTimeReturn(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream);

#ifdef CONFIG_AP_SUPPORT
/* delete a lower ACM AC stream */
STATIC ACM_FUNC_STATUS ACM_EDCA_Lower_UP_Del(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				TimeOff);

/* handle a EDCA stream TSPEC */
STATIC UCHAR ACM_EDCA_ReqHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	ACM_STREAM			*pNewStream,
	ACM_PARAM_IN	ACM_STREAM			*pOldStreamIn,
	ACM_PARAM_IN	ACM_STREAM			*pOldStreamOut,
	ACM_PARAM_IN	ACM_STREAM			*pOldStreamDiffAc);
#endif // CONFIG_AP_SUPPORT //

/* update ACM used time of EDCA */
STATIC void ACM_EDCA_Param_ACM_Update(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	UINT32				AcmAcId,
	ACM_PARAM_IN	UCHAR				Direction,
	ACM_PARAM_IN	UCHAR				UP,
	ACM_PARAM_IN	UINT32				AcmTimeNew,
	ACM_PARAM_IN	UINT32				AcmTimeOld,
	ACM_PARAM_IN	UINT32				DatlAcId,
	ACM_PARAM_IN	UINT32				DatlBandwidth);


/* ======================= WMM  Function definition ======================= */
#ifdef ACM_CC_FUNC_WMM

/* ----- COMMON ----- */
/* translate 11e status code to WME status code */
STATIC UCHAR ACM_11E_WMM_StatusTranslate(
	ACM_PARAM_IN	UCHAR				StatusCode);

/* translate WME TSPEC & TCLAS to 11e TSPEC & TCLAS */
STATIC ACM_FUNC_STATUS ACM_WME_11E_TSPEC_TCLAS_Translate(
	ACM_PARAM_IN	UCHAR				*pPktElm,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	ACM_TSPEC			*pETspec,
	ACM_PARAM_IN	ACM_TCLAS			**ppTclas,
	ACM_PARAM_IN	UINT32				*pTclasNum,
	ACM_PARAM_IN	UCHAR				*pTclasProcessing);

/* translate WME TSPEC to 11e TSPEC */
STATIC ACM_FUNC_STATUS ACM_WME_11E_TSPEC_Translate(
	ACM_PARAM_IN	ACM_WME_TSPEC		*pWTspec,
	ACM_PARAM_IN	ACM_TSPEC			*pETspec);

/* make a WME action frame body */
STATIC UINT32 ACM_WME_ActionFrameBodyMake(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACM_STREAM			*pStream,
	ACM_PARAM_IN	UCHAR				*pPkt,
	ACM_PARAM_IN	UCHAR				Action,
	ACM_PARAM_IN	UCHAR				StatusCode);

/* handle a WME action frame */
STATIC void ACM_WME_ActionHandle(
	ACM_PARAM_IN	ACMR_PWLAN_STRUC	pAd,
	ACM_PARAM_IN	ACMR_STA_DB			*pCdb,
	ACM_PARAM_IN	UCHAR				*pFrameBody,
	ACM_PARAM_IN	UINT32				BodyLen,
	ACM_PARAM_IN	UINT32				PhyRate,
	ACM_PARAM_IN	UCHAR				Action,
	ACM_PARAM_OUT	UCHAR				*pStatusCode,
	ACM_PARAM_OUT	UINT16				*pMediumTime);

/* ----- QAP ----- */
#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

#endif /* ACM_CC_FUNC_WMM */

#endif // __ACM_EDCA_H__

/* End of acm_edca.h */

