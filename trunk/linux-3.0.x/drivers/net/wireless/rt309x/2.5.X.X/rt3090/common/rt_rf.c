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
	rt_rf.c

	Abstract:
	Ralink Wireless driver RF related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"


#ifdef RTMP_RF_RW_SUPPORT
/*
	========================================================================
	
	Routine Description: Write RT30xx RF register through MAC

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value)
{
	RF_CSR_CFG_STRUC	rfcsr = { { 0 } };
	UINT				i = 0;
#if defined(RT3593) || defined(RT53xx)
	RF_CSR_CFG_EXT_STRUC RfCsrCfgExt = { { 0 } };
#endif // RT3593 || defined(RT53xx) //


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT30xxWriteRFRegister. Not allow to write RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif // RTMP_MAC_PCI //

#if defined(RT3593) || defined(RT53xx)
	if (IS_RT3593(pAd) || IS_RT5390(pAd))
	{
		//ASSERT((regID <= 63)); // R0~R63
		ASSERT((regID >= 0) && (regID <= 63)); // R0~R63

		do
		{
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &RfCsrCfgExt.word);

			if (!RfCsrCfgExt.field.RF_CSR_KICK)
			{
				break;
			}
			
			i++;
		}
		
		while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
			; // Do nothing

		if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		RfCsrCfgExt.field.RF_CSR_WR = 1;
		RfCsrCfgExt.field.RF_CSR_KICK = 1;
		RfCsrCfgExt.field.TESTCSR_RFACC_REGNUM = regID; // R0~R63
		RfCsrCfgExt.field.RF_CSR_DATA = value;
		
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, RfCsrCfgExt.word);
		
		/* zero patch based on windosw driver */
		//2 REMOVE?? Jason
		//2 For 5390A only (Not FIB)

		//2 USE IT??? Jason
		if (regID == RF_R17)
		{
			RTMPusecDelay(15000);
		}
		
	}
	else
#endif // RT3593  || defined(RT53xx)//
	{
		if (IS_RT3883(pAd))
		{
			ASSERT((regID <= 63)); // R0~R63
		}
		else
		{
		ASSERT((regID <= 31)); // R0~R31
		}

		do
		{
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (!rfcsr.field.RF_CSR_KICK)
				break;
			i++;
		}
		while ((i < RETRY_LIMIT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

		if ((i == RETRY_LIMIT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		rfcsr.field.RF_CSR_WR = 1;
		rfcsr.field.RF_CSR_KICK = 1;
		rfcsr.field.TESTCSR_RFACC_REGNUM = regID; // R0~R31
		rfcsr.field.RF_CSR_DATA = value;
		
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
	}

	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================
	
	Routine Description: Read RT30xx RF register through MAC

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue)
{
	RF_CSR_CFG_STRUC	rfcsr = { { 0 } };
	UINT				i=0, k=0;

#if defined(RT3593) || defined(RT53xx)
	RF_CSR_CFG_EXT_STRUC RfCsrCfgExt = { { 0 } };
#endif // RT3593 || defined(RT53xx) //


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT30xxReadRFRegister. Not allow to read RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif // RTMP_MAC_PCI //

#if defined(RT3593) || defined(RT53xx)
	if (IS_RT3593(pAd) || IS_RT5390(pAd))
	{
		//ASSERT((regID <= 63)); // R0~R63
		ASSERT((regID >= 0) && (regID <= 63)); // R0~R63
		
		for (i = 0; i < MAX_BUSY_COUNT; i++)
		{
			if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
				return STATUS_UNSUCCESSFUL;
			
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &RfCsrCfgExt.word);

			if (RfCsrCfgExt.field.RF_CSR_KICK == BUSY)
			{
				continue;
			}
			
			RfCsrCfgExt.word = 0;
			RfCsrCfgExt.field.RF_CSR_WR = 0;
			RfCsrCfgExt.field.RF_CSR_KICK = 1;
			RfCsrCfgExt.field.TESTCSR_RFACC_REGNUM = regID; // R0~R63
			
			RTMP_IO_WRITE32(pAd, RF_CSR_CFG, RfCsrCfgExt.word);
			
			for (k = 0; k < MAX_BUSY_COUNT; k++)
			{
				if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
					return STATUS_UNSUCCESSFUL;
				
				RTMP_IO_READ32(pAd, RF_CSR_CFG, &RfCsrCfgExt.word);

				if (RfCsrCfgExt.field.RF_CSR_KICK == IDLE)
				{
					break;
				}
			}
			
			if ((RfCsrCfgExt.field.RF_CSR_KICK == IDLE) && 
			     (RfCsrCfgExt.field.TESTCSR_RFACC_REGNUM == regID))
			{
				*pValue = (UCHAR)(RfCsrCfgExt.field.RF_CSR_DATA);
				break;
			}
		}

		if (RfCsrCfgExt.field.RF_CSR_KICK == BUSY)
		{																	
			DBGPRINT_ERR(("RF read R%d = 0x%X fail, i[%d], k[%d]\n", regID, (UINT32)RfCsrCfgExt.word, i, k));
			return STATUS_UNSUCCESSFUL;
		}
	}
	else
#endif // defined(RT3593) || defined(RT53xx) //
	{
		if (IS_RT3883(pAd))
		{
			ASSERT((regID <= 63)); // R0~R63
		}
		else
		{
		ASSERT((regID <= 31)); // R0~R31
		}

		for (i=0; i<MAX_BUSY_COUNT; i++)
		{
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (rfcsr.field.RF_CSR_KICK == BUSY)									
			{																
				continue;													
			}																
			rfcsr.word = 0;
			rfcsr.field.RF_CSR_WR = 0;
			rfcsr.field.RF_CSR_KICK = 1;
			rfcsr.field.TESTCSR_RFACC_REGNUM = regID;
			RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
			for (k=0; k<MAX_BUSY_COUNT; k++)
			{
				RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

				if (rfcsr.field.RF_CSR_KICK == IDLE)
					break;
			}
			if ((rfcsr.field.RF_CSR_KICK == IDLE) &&
				(rfcsr.field.TESTCSR_RFACC_REGNUM == regID))
			{
				*pValue = (UCHAR)(rfcsr.field.RF_CSR_DATA);
				break;
			}
		}
		if (rfcsr.field.RF_CSR_KICK == BUSY)
		{																	
			DBGPRINT_ERR(("RF read R%d=0x%X fail, i[%d], k[%d]\n", regID, rfcsr.word,i,k));
			return STATUS_UNSUCCESSFUL;
		}
	}

	return STATUS_SUCCESS;
}


VOID NICInitRFRegisters(
	IN RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.AsicRfInit)
		pAd->chipOps.AsicRfInit(pAd);
}


VOID RtmpChipOpsRFHook(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

	pChipOps->pRFRegTable = NULL;
	pChipOps->pBBPRegTable = NULL;
	pChipOps->bbpRegTbSize = 0;
	pChipOps->AsicRfInit = NULL;
	pChipOps->AsicRfTurnOn = NULL;
	pChipOps->AsicRfTurnOff = NULL;
	pChipOps->AsicReverseRfFromSleepMode = NULL;
	pChipOps->AsicHaltAction = NULL;
	
	/* We depends on RfICType and MACVersion to assign the corresponding operation callbacks. */




#ifdef RT30xx


#ifdef RT53xx
	if (IS_RT5390(pAd))
	{
		pChipOps->AsicRfTurnOff = RT30xxLoadRFSleepModeSetup;
		pChipOps->pRFRegTable = RF5390RegTable;
		pChipOps->AsicRfInit = NICInitRT5390RFRegisters;
		pChipOps->AsicReverseRfFromSleepMode = RT30xxReverseRFSleepModeSetup;
		pChipOps->AsicHaltAction = RT30xxHaltAction;
	}
#endif // RT3593 //

	if (IS_RT30xx(pAd))
	{
		/* 
			WARNING: 
				Currently following table are shared by all RT30xx based IC, change it carefully when you add a new IC here.
		*/
#ifdef RT33xx
		if (IS_RT3390(pAd))
		{
#ifdef RT3390
			if (pAd->infType == RTMP_DEV_INF_PCIE)
			{
				pChipOps->pRFRegTable = RF3320_RFRegTable;
				pChipOps->AsicRfInit = NICInitRT3390RFRegisters;
			}
#endif // RT3390 //
			pChipOps->AsicHaltAction = RT33xxHaltAction;
			pChipOps->AsicRfTurnOff = RT33xxLoadRFSleepModeSetup;		
			pChipOps->AsicReverseRfFromSleepMode = RT33xxReverseRFSleepModeSetup;
		}
		else
#endif // RT33xx //
		{
		pChipOps->pRFRegTable = RT3020_RFRegTable;
		pChipOps->AsicHaltAction = RT30xxHaltAction;
		pChipOps->AsicRfTurnOff = RT30xxLoadRFSleepModeSetup;
		pChipOps->AsicReverseRfFromSleepMode = RT30xxReverseRFSleepModeSetup;
		
#ifdef RT3090
		if (IS_RT3090(pAd) && (pAd->infType == RTMP_DEV_INF_PCIE))
		{
			pChipOps->AsicRfInit = NICInitRT3090RFRegisters;
		}
#endif // RT3090 //
	}
	}
#endif // RT30xx //

	DBGPRINT(RT_DEBUG_TRACE, ("Chip specific bbpRegTbSize=%d!\n", pChipOps->bbpRegTbSize));
	
}
//
//XO output may have glitch during unstable time and depend on cap value. RF_R17
//0x40:25us
//0x3F:20us
//0x20:8us
//0x10:1-2us
//0x08:0us
//0x04:0us
//0x02:0us
//0x01:0us
//
VOID RFMultiStepXoCode(
	IN	PRTMP_ADAPTER pAd,
	IN	UCHAR	rfRegID,
	IN	UCHAR	rfRegValue,
	IN	UCHAR	rfRegValuePre)
{
	UINT i = 0, count = 0, RFValue = 0;
	BOOLEAN bit7IsTrue = (rfRegValue & (0x80));

	rfRegValue &= (0x7F);
	rfRegValuePre &= (0x7F);

	if (rfRegValuePre == rfRegValue)
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("RFMultiStepXoCode--> Write Value 0x%02x, previous Value 0x%02x, bit7IsTrue = %d\n",
		rfRegValue, rfRegValuePre, bit7IsTrue));

	if (rfRegValue > rfRegValuePre)
	{
		// Sequentially
		for (i = rfRegValuePre; i <= rfRegValue; i++)		
		{
			count++;
			
			if (bit7IsTrue)
			{
				RFValue = (i |0x80);		
				RT30xxWriteRFRegister(pAd, rfRegID, RFValue);
			}
			else
				RT30xxWriteRFRegister(pAd, rfRegID, i);
		}
	}
	else
	{
		// one step
		if (bit7IsTrue)
			rfRegValue |= 0x80;	
		RT30xxWriteRFRegister(pAd, rfRegID, rfRegValue);
		count++;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("RFMultiStepXoCode<-- Write Value 0x%02x, previous Value 0x%02x, running step count = %d\n",
		rfRegValue, rfRegValuePre, count));
}
#endif // RTMP_RF_RW_SUPPORT //

