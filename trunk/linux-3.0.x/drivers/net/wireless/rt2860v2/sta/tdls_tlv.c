/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#include "rt_config.h"

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertActField(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	Category,
	IN	UINT8	ActCode)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&Category,
						1,				&ActCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertStatusCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	StatusCode)
{
	ULONG TempLen;

	StatusCode = cpu2le16(StatusCode);
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						2,				&StatusCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertReasonCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	ReasonCode)
{
	ULONG TempLen;

	ReasonCode = cpu2le16(ReasonCode);
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						2,				&ReasonCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertDialogToken(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	DialogToken)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&DialogToken,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertLinkIdentifierIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	PUCHAR	pInitAddr,
	IN	PUCHAR	pRespAddr)
{
	ULONG TempLen;
	UCHAR TDLS_IE = IE_TDLS_LINK_IDENTIFIER;
	UCHAR TDLS_IE_LEN = TDLS_ELM_LEN_LINK_IDENTIFIER - 2;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&TDLS_IE,
						1,				&TDLS_IE_LEN,
						6,				pAd->CommonCfg.Bssid,
						6,				pInitAddr,
						6,				pRespAddr,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertCapIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						2,				&pAd->StaActive.CapabilityInfo,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertSSIDIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&SsidIe,
						1,							&pAd->CommonCfg.SsidLen, 
						pAd->CommonCfg.SsidLen,		pAd->CommonCfg.Ssid,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertSupportRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&SupRateIe,
						1,							&pAd->CommonCfg.SupRateLen,
						pAd->StaActive.SupRateLen,	pAd->CommonCfg.SupRate,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertExtRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	if (pAd->StaActive.ExtRateLen != 0)
	{
		MakeOutgoingFrame(pFrameBuf,					&TempLen,
							1,							&ExtRateIe,
							1,							&pAd->CommonCfg.ExtRateLen,
							pAd->StaActive.ExtRateLen,	pAd->CommonCfg.ExtRate,							
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertQosCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
	{
		UCHAR QOS_CAP_IE = 46;
		UCHAR QOS_CAP_IE_LEN = 1;
		QBSS_STA_INFO_PARM QosInfo;

		NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));

		if (pAd->CommonCfg.bAPSDCapable)
		{
			QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
			QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
			QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
			QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
			QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
		}

		MakeOutgoingFrame(pFrameBuf,	&TempLen,
						1,				&QOS_CAP_IE,
						1,				&QOS_CAP_IE_LEN,
						1,				&QosInfo,
						END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertEDCAParameterSetIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS)
{
	ULONG TempLen;

	if (((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable)) && (pTDLS->bWmmCapable))
	{
		USHORT	idx;

		/* When the BSS is QoS capable, then the BSS QoS parameters shall be
		 * used by the TDLS peer STAs on the AP's channel, and the values 
		 * indicated inside the TDLS Setup Confirm frame apply only for the 
		 * off-channel. The EDCA parameters for the off-channel should be 
		 * the same as those on the AP's channel when QoS is supported by the BSS, 
		 * because this may optimize the channel switching process.
		 */

		UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 

		//  Reset EdcaParam
		NdisZeroMemory(&pTDLS->EdcaParm, sizeof(EDCA_PARM));
		// Enable EdcaParm used in non-QBSS.
		pTDLS->EdcaParm.bValid = TRUE;

		pTDLS->EdcaParm.bQAck		   = FALSE;
		pTDLS->EdcaParm.bQueueRequest   = FALSE;
		pTDLS->EdcaParm.bTxopRequest    = FALSE;

		WmeParmIe[2] =  ((UCHAR)pTDLS->EdcaParm.bQAck << 4) + 
						((UCHAR)pTDLS->EdcaParm.bQueueRequest << 5) + 
						((UCHAR)pTDLS->EdcaParm.bTxopRequest << 6);

		pTDLS->EdcaParm.EdcaUpdateCount = 1;
		WmeParmIe[8] = pTDLS->EdcaParm.EdcaUpdateCount & 0x0f;

		if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.ExtCapIE.TdlsUAPSD && pTDLS->TdlsExtCap.TdlsUAPSD)
			pTDLS->EdcaParm.bAPSDCapable = TRUE;
		else
			pTDLS->EdcaParm.bAPSDCapable = FALSE;
		WmeParmIe[8] |= pTDLS->EdcaParm.bAPSDCapable << 7;

		// By hardcoded 
		pTDLS->EdcaParm.Aifsn[0] = 3;
		pTDLS->EdcaParm.Aifsn[1] = 7;
		pTDLS->EdcaParm.Aifsn[2] = 2;
		pTDLS->EdcaParm.Aifsn[3] = 2;

		pTDLS->EdcaParm.Cwmin[0] = 4;
		pTDLS->EdcaParm.Cwmin[1] = 4;
		pTDLS->EdcaParm.Cwmin[2] = 3;
		pTDLS->EdcaParm.Cwmin[3] = 2;

		pTDLS->EdcaParm.Cwmax[0] = 10;
		pTDLS->EdcaParm.Cwmax[1] = 10;
		pTDLS->EdcaParm.Cwmax[2] = 4;
		pTDLS->EdcaParm.Cwmax[3] = 3;

		pTDLS->EdcaParm.Txop[0]  = 0;
		pTDLS->EdcaParm.Txop[1]  = 0;
		pTDLS->EdcaParm.Txop[2]  = 96;
		pTDLS->EdcaParm.Txop[3]  = 48;

		for (idx=QID_AC_BE; idx<=QID_AC_VO; idx++)
		{
			WmeParmIe[10+ (idx*4)] = (idx << 5)								+	  // b5-6 is ACI
								   ((UCHAR)pTDLS->EdcaParm.bACM[idx] << 4) 	+	  // b4 is ACM
								   (pTDLS->EdcaParm.Aifsn[idx] & 0x0f);			  // b0-3 is AIFSN
			WmeParmIe[11+ (idx*4)] = (pTDLS->EdcaParm.Cwmax[idx] << 4)		+	  // b5-8 is CWMAX
								   (pTDLS->EdcaParm.Cwmin[idx] & 0x0f);			  // b0-3 is CWMIN
			WmeParmIe[12+ (idx*4)] = (UCHAR)(pTDLS->EdcaParm.Txop[idx] & 0xff);	  // low byte of TXOP
			WmeParmIe[13+ (idx*4)] = (UCHAR)(pTDLS->EdcaParm.Txop[idx] >> 8);	  // high byte of TXOP
		}

		MakeOutgoingFrame(pFrameBuf,		&TempLen,
						  26,				WmeParmIe,
						  END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

#ifdef DOT11_N_SUPPORT
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertHtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
	{
		UCHAR HtLen;

#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
#endif	

		HtLen = sizeof(HT_CAPABILITY_IE);
#ifndef RT_BIG_ENDIAN
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
							1,			&HtCapIe,
							1,			&HtLen,
							HtLen,		&pAd->CommonCfg.HtCapability, 
							END_OF_ARGS);
#else
		NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
		NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
		*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
		*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));

		MakeOutgoingFrame(pFrameBuf,	&TempLen,
							1,			&HtCapIe,
							1,			&HtLen,
							HtLen,		&HtCapabilityTmp, 
							END_OF_ARGS);
#endif

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

#ifdef DOT11N_DRAFT3
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
// 20/40 BSS Coexistence (7.3.2.61)
VOID
TDLS_InsertBSSCoexistenceIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	if (pAd->CommonCfg.BACapability.field.b2040CoexistScanSup == 1)
	{
		UCHAR Length = 1;
		BSS_2040_COEXIST_IE BssCoexistence;

		MakeOutgoingFrame(pFrameBuf,		&TempLen,
							1,				&BssCoexistIe,
							1,				&Length,
							1,				&pAd->CommonCfg.BSSCoexist2040.word,							
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertExtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;
	UCHAR Length = 1;

	Length = sizeof(EXT_CAP_INFO_ELEMENT);

	if ((pAd->CommonCfg.ExtCapIE.TdlsChannelSwitching == 1) ||
		(pAd->CommonCfg.ExtCapIE.TdlsPSM== 1) ||
		(pAd->CommonCfg.ExtCapIE.TdlsUAPSD== 1))
	{
		MakeOutgoingFrame(pFrameBuf,		&TempLen,
							1,				&ExtCapIe,
							1,				&Length,
							sizeof(EXT_CAP_INFO_ELEMENT),			&pAd->CommonCfg.ExtCapIE,
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_InsertFTIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Length,
	IN FT_MIC_CTR_FIELD MICCtr,
	IN PUINT8 pMic,
	IN PUINT8 pANonce,
	IN PUINT8 pSNonce)
{
	ULONG TempLen;
	UINT16 MICCtrBuf;
	UCHAR FTIE = IE_FT_FTIE;

	MICCtrBuf = cpu2le16(MICCtr.word);
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&FTIE,
						1,				&Length,
						2,				(PUCHAR)&MICCtrBuf,
						16,				(PUCHAR)pMic,
						32,				(PUCHAR)pANonce,
						32,				(PUCHAR)pSNonce,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_InsertTimeoutIntervalIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_TIMEOUT_INTERVAL_TYPE Type,
	IN UINT32 TimeOutValue)
{
	ULONG TempLen;
	UINT8 Length;
	UINT8 TimeOutIntervalIE;
	UINT8 TimeoutType;
	UINT32 TimeoutValueBuf;

	Length = 5;
	TimeOutIntervalIE = IE_FT_TIMEOUT_INTERVAL;
	TimeoutType = Type;
	TimeoutValueBuf = cpu2le32(TimeOutValue);

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&TimeOutIntervalIE,
						1,				&Length,
						1,				(PUCHAR)&TimeoutType,
						4,				(PUCHAR)&TimeoutValueBuf,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

#endif // DOT11Z_TDLS_SUPPORT //

