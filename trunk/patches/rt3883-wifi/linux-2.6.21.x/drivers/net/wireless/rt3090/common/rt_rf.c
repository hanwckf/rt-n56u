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
#ifdef RT3593
	RF_CSR_CFG_EXT_STRUC RfCsrCfgExt = { { 0 } };
#endif // RT3593 //


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT30xxWriteRFRegister. Not allow to write RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif // RTMP_MAC_PCI //

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		ASSERT((regID <= 63)); // R0~R63

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
	}
	else
#endif // RT3593 //
	{
		ASSERT((regID <= 31)); // R0~R31
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

#ifdef RT3593
	RF_CSR_CFG_EXT_STRUC RfCsrCfgExt = { { 0 } };
#endif // RT3593 //


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT30xxReadRFRegister. Not allow to read RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif // RTMP_MAC_PCI //

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		ASSERT((regID <= 63)); // R0~R63
		
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
#endif // RT3593 //
	{
		ASSERT((regID <= 31)); // R0~R31

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


#ifdef RT3883
        if (IS_RT3883(pAd) && (pAd->infType == RTMP_DEV_INF_RBUS))
        {
		pChipOps->pRFRegTable = RT3883_RFRegTable;
		pChipOps->pBBPRegTable = RT3883_BBPRegTable;
		pChipOps->bbpRegTbSize = (sizeof(RT3883_BBPRegTable) / sizeof(REG_PAIR));
		pChipOps->AsicRfInit = NICInitRT3883RFRegisters;
		pChipOps->AsicHaltAction = RT3883HaltAction;
		pChipOps->AsicRfTurnOff = RT3883LoadRFSleepModeSetup;
		pChipOps->AsicReverseRfFromSleepMode = RT3883ReverseRFSleepModeSetup;
	}
#endif // RT3883 //


#ifdef RT30xx

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		pChipOps->AsicRfTurnOff = RT30xxLoadRFSleepModeSetup;
		pChipOps->pRFRegTable = RF3053RegTable;
		pChipOps->AsicRfInit = NICInitRT3593RFRegisters;
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
#endif // RT30xx //

	DBGPRINT(RT_DEBUG_TRACE, ("Chip specific bbpRegTbSize=%d!\n", pChipOps->bbpRegTbSize));
	
}

#endif // RTMP_RF_RW_SUPPORT //

