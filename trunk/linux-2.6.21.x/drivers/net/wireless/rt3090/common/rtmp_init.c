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
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include	"ft.h"
#endif // DOT11R_FT_SUPPORT //
#define RT3090A_DEFAULT_INTERNAL_LNA_GAIN	0x0A
UCHAR    BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char*   CipherName[] = {"none","wep64","wep128","TKIP","AES","CKIP64","CKIP128","CKIP152","SMS4"};
#endif

//
// BBP register initialization set
//
REG_PAIR   BBPRegTable[] = {
	{BBP_R65,		0x2C},		// fix rssi issue
	{BBP_R66,		0x38},	// Also set this default value to pAd->BbpTuning.R66CurrentValue at initial
	{BBP_R69,		0x12},
	{BBP_R70,		0xa},	// BBP_R70 will change to 0x8 in ApStartUp and LinkUp for rt2860C, otherwise value is 0xa
	{BBP_R73,		0x10},
	{BBP_R81,		0x37},
	{BBP_R82,		0x62},
	{BBP_R83,		0x6A},
	{BBP_R84,		0x99},	// 0x19 is for rt2860E and after. This is for extension channel overlapping IOT. 0x99 is for rt2860D and before
	{BBP_R86,		0x00},	// middle range issue, Rory @2008-01-28 	
	{BBP_R91,		0x04},	// middle range issue, Rory @2008-01-28
	{BBP_R92,		0x00},	// middle range issue, Rory @2008-01-28
	{BBP_R103,		0x00}, 	// near range high-power issue, requested from Gary @2008-0528
	{BBP_R105,		0x05},	// 0x05 is for rt2860E to turn on FEQ control. It is safe for rt2860D and before, because Bit 7:2 are reserved in rt2860D and before.
#ifdef DOT11_N_SUPPORT
	{BBP_R106,		0x35},	// Optimizing the Short GI sampling request from Gray @2009-0409
#endif // DOT11_N_SUPPORT //
};
#define	NUM_BBP_REG_PARMS	(sizeof(BBPRegTable) / sizeof(REG_PAIR))


//
// ASIC register initialization sets
//

RTMP_REG_PAIR	MACRegTable[] =	{
#if defined(RT3352) || defined(RT3883) && defined(CONFIG_16MBSSID_MODE)
	{BCN_OFFSET0,			0x18100800}, /* if (BCNx_OFFSET > 0x7F) SHR_MSEL = 0 else SHR_MSEL = 1 */
	{BCN_OFFSET1,			0x38302820}, /* if (BCNx_OFFSET > 0x7F) SHR_MSEL = 0 else SHR_MSEL = 1 */
	{BCN_OFFSET2,			0x58504840}, /* if (BCNx_OFFSET > 0x7F) SHR_MSEL = 0 else SHR_MSEL = 1 */
	{BCN_OFFSET3,			0x78706860}, /* if (BCNx_OFFSET > 0x7F) SHR_MSEL = 0 else SHR_MSEL = 1 */
#elif defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x200)
	{BCN_OFFSET0,			0xf8f0e8e0}, /* 0x3800(e0), 0x3A00(e8), 0x3C00(f0), 0x3E00(f8), 512B for each beacon */
	{BCN_OFFSET1,			0x6f77d0c8}, /* 0x3200(c8), 0x3400(d0), 0x1DC0(77), 0x1BC0(6f), 512B for each beacon */
#elif defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x100)
	{BCN_OFFSET0,			0xece8e4e0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
	{BCN_OFFSET1,			0xfcf8f4f0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
#else
    #error You must re-calculate new value for BCN_OFFSET0 & BCN_OFFSET1 in MACRegTable[]!!!
#endif // HW_BEACON_OFFSET //

	{LEGACY_BASIC_RATE,		0x0000013f}, //  Basic rate set bitmap
	{HT_BASIC_RATE,		0x00008003}, // Basic HT rate set , 20M, MCS=3, MM. Format is the same as in TXWI.
	{MAC_SYS_CTRL,		0x00}, // 0x1004, , default Disable RX
	{RX_FILTR_CFG,		0x17f97}, //0x1400  , RX filter control,  
	{BKOFF_SLOT_CFG,	0x209}, // default set short slot time, CC_DELAY_TIME should be 2	 
	//{TX_SW_CFG0,		0x40a06}, // Gary,2006-08-23 
	{TX_SW_CFG0,		0x0}, 		// Gary,2008-05-21 for CWC test 
	{TX_SW_CFG1,		0x80606}, // Gary,2006-08-23 
	{TX_LINK_CFG,		0x1020},		// Gary,2006-08-23 
	//{TX_TIMEOUT_CFG,	0x00182090},	// CCK has some problem. So increase timieout value. 2006-10-09// MArvek RT
	{TX_TIMEOUT_CFG,	0x000a2090},	// CCK has some problem. So increase timieout value. 2006-10-09// MArvek RT , Modify for 2860E ,2007-08-01
	{MAX_LEN_CFG,		MAX_AGGREGATION_SIZE | 0x00001000},	// 0x3018, MAX frame length. Max PSDU = 16kbytes.
	{LED_CFG,		0x7f031e46}, // Gary, 2006-08-23

//#ifdef CONFIG_AP_SUPPORT
//	{WMM_AIFSN_CFG,		0x00001173},
//	{WMM_CWMIN_CFG,		0x00002344},
//	{WMM_CWMAX_CFG,		0x000034a6},
//	{WMM_TXOP0_CFG,		0x00100020},
//	{WMM_TXOP1_CFG,		0x002F0038},
//#endif // CONFIG_AP_SUPPORT //

//#ifdef CONFIG_STA_SUPPORT
//	{WMM_AIFSN_CFG,		0x00002273},
//	{WMM_CWMIN_CFG,		0x00002344},
//	{WMM_CWMAX_CFG,		0x000034aa},
//#endif // CONFIG_STA_SUPPORT //
#ifdef INF_AMAZON_SE
	{PBF_MAX_PCNT,			0x1F3F6F6F}, 	//iverson modify for usb issue, 2008/09/19
											// 6F + 6F < total page count FE
											// so that RX doesn't occupy TX's buffer space when WMM congestion.
#else
	{PBF_MAX_PCNT,			0x1F3FBF9F}, 	//0x1F3f7f9f},		//Jan, 2006/04/20
#endif // INF_AMAZON_SE //
	//{TX_RTY_CFG,			0x6bb80408},	// Jan, 2006/11/16
// WMM_ACM_SUPPORT
//	{TX_RTY_CFG,			0x6bb80101},	// sample
	{TX_RTY_CFG,			0x47d01f0f},	// Jan, 2006/11/16, Set TxWI->ACK =0 in Probe Rsp Modify for 2860E ,2007-08-03
	
	{AUTO_RSP_CFG,			0x00000013},	// Initial Auto_Responder, because QA will turn off Auto-Responder
	{CCK_PROT_CFG,			0x05740003 /*0x01740003*/},	// Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. 
	{OFDM_PROT_CFG,			0x05740003 /*0x01740003*/},	// Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. 
	{GF20_PROT_CFG,			0x01744004},    // set 19:18 --> Short NAV for MIMO PS
	{GF40_PROT_CFG,			0x03F44084},    
	{MM20_PROT_CFG,			0x01744004},    
#ifdef RTMP_MAC_PCI
	{MM40_PROT_CFG,			0x03F54084},	
#endif // RTMP_MAC_PCI //
	{TXOP_CTRL_CFG,			0x0000583f, /*0x0000243f*/ /*0x000024bf*/},	//Extension channel backoff.
	{TX_RTS_CFG,			0x00092b20},	
//#ifdef WIFI_TEST
	{EXP_ACK_TIME,			0x002400ca},	// default value
//#else
//	{EXP_ACK_TIME,			0x005400ca},	// suggested by Gray @ 20070323 for 11n intel-sta throughput
//#endif // end - WIFI_TEST //
//#ifdef CONFIG_AP_SUPPORT
//	{TBTT_SYNC_CFG,			0x00422000},	// TBTT_ADJUST(7:0) == 0	
//	{TBTT_SYNC_CFG,			0x00012000},	// TBTT_ADJUST(7:0) == 0	
//#endif // CONFIG_AP_SUPPORT //
#if defined(RT2883) || defined(RT3883)
	{TX_FBK_CFG_3S_0,		0x12111008},	// default value
	{TX_FBK_CFG_3S_1,		0x16151413},	// default value
#endif // defined(RT2883) || defined(RT3883) //
	{TXOP_HLDR_ET, 			0x00000002},

	/* Jerry comments 2008/01/16: we use SIFS = 10us in CCK defaultly, but it seems that 10us
		is too small for INTEL 2200bg card, so in MBSS mode, the delta time between beacon0
		and beacon1 is SIFS (10us), so if INTEL 2200bg card connects to BSS0, the ping
		will always lost. So we change the SIFS of CCK from 10us to 16us. */
	{XIFS_TIME_CFG,			0x33a41010},
	{PWR_PIN_CFG,			0x00000003},	// patch for 2880-E
};

#ifdef CONFIG_AP_SUPPORT
RTMP_REG_PAIR	APMACRegTable[] =	{
	{WMM_AIFSN_CFG,		0x00001173},
	{WMM_CWMIN_CFG,	0x00002344},
	{WMM_CWMAX_CFG,	0x000034a6},
	{WMM_TXOP0_CFG,		0x00100020},
	{WMM_TXOP1_CFG,		0x002F0038},
	{TBTT_SYNC_CFG,		0x00012000},
#if defined(RT2883) || defined(RT3883)
	{TX_CHAIN_ADDR0_L,	0xFFFFFFFF},	// Broadcast frames are in stream mode
	{TX_CHAIN_ADDR0_H,	0xFFFFF},
#endif // defined(RT2883) || defined(RT3883) //
};
#endif // CONFIG_AP_SUPPORT //


#define	NUM_MAC_REG_PARMS		(sizeof(MACRegTable) / sizeof(RTMP_REG_PAIR))
#ifdef CONFIG_AP_SUPPORT
#define	NUM_AP_MAC_REG_PARMS	(sizeof(APMACRegTable) / sizeof(RTMP_REG_PAIR))
#endif // CONFIG_AP_SUPPORT //


/*
	========================================================================
	
	Routine Description:
		Allocate RTMP_ADAPTER data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:
	
	========================================================================
*/
NDIS_STATUS	RTMPAllocAdapterBlock(
	IN  PVOID	handle,
	OUT	PRTMP_ADAPTER	*ppAdapter)
{
	PRTMP_ADAPTER	pAd = NULL;
	NDIS_STATUS		Status;
	INT 			index;
	UCHAR			*pBeaconBuf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocAdapterBlock\n"));

	*ppAdapter = NULL;

	do
	{
		// Allocate RTMP_ADAPTER memory block
		pBeaconBuf = kmalloc(MAX_BEACON_SIZE, MEM_ALLOC_FLAG);
		if (pBeaconBuf == NULL)
		{
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT_ERR(("Failed to allocate memory - BeaconBuf!\n"));
			break;
		}
		NdisZeroMemory(pBeaconBuf, MAX_BEACON_SIZE);

		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("Failed to allocate memory - ADAPTER\n"));
			break;
		}
		pAd->BeaconBuf = pBeaconBuf;
		DBGPRINT(RT_DEBUG_OFF, ("\n\n=== pAd = %p, size = %d ===\n\n", pAd, (UINT32)sizeof(RTMP_ADAPTER)));


		// Init spin locks
		NdisAllocateSpinLock(&pAd->MgmtRingLock);
#ifdef RTMP_MAC_PCI
		NdisAllocateSpinLock(&pAd->RxRingLock);
#if defined(RT3090) || defined(RT3592) || defined(RT3390) || defined(RT3593)
#endif // defined(RT3090) || defined(RT3592) || defined(RT3390) || defined(RT3593) //
#endif // RTMP_MAC_PCI //

		for (index =0 ; index < NUM_OF_TX_RING; index++)
		{
			NdisAllocateSpinLock(&pAd->TxSwQueueLock[index]);
			NdisAllocateSpinLock(&pAd->DeQueueLock[index]);
			pAd->DeQueueRunning[index] = FALSE;
		}

		NdisAllocateSpinLock(&pAd->irq_lock);

#ifdef WMM_ACM_SUPPORT
		NdisAllocateSpinLock(&pAd->AcmTspecSemLock);
		NdisAllocateSpinLock(&pAd->AcmTspecIrqLock);
#endif // WMM_ACM_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
        UAPSD_Init(pAd);
#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

		// assign function pointers
#ifdef MAT_SUPPORT
		/* init function pointers, used in OS_ABL */
		RTMP_MATOpsInit(pAd);
#endif // MAT_SUPPORT //
	} while (FALSE);

	if ((Status != NDIS_STATUS_SUCCESS) && (pBeaconBuf))
		kfree(pBeaconBuf);
	
	*ppAdapter = pAd;


	/*
		Init ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) 
	{
		if (os_alloc_mem(pAd,&pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN) == NDIS_STATUS_SUCCESS)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}	

	DBGPRINT_S(Status, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));
	return Status;
}


#ifdef RT3593
//
// The verification of the Tx power per rate
//
// Parameters
//	pAd: The adapter data structure
//	pTxPwr: Point to a Tx power per rate
//
// Return Value
//	None
//
VOID RTMPVerifyTxPwrPerRateExt(
	IN PRTMP_ADAPTER pAd, 
	IN OUT PUCHAR pTxPwr)
{
	DBGPRINT(RT_DEBUG_INFO, ("--> %s\n", __FUNCTION__));

	if (*pTxPwr > 0xC)
	{
		*pTxPwr = 0xC;
	}
	
	DBGPRINT(RT_DEBUG_INFO, ("<-- %s\n", __FUNCTION__));
}

//
// Read the Tx power (per data rate) based on the extended EEPROM format
//
// Parameters
//	pAd: The adapter data structure
//
// Return Value
//	None
//
VOID RTMPReadTxPwrPerRateExt(
	IN PRTMP_ADAPTER pAd)
{
	USHORT value = 0;
	ULONG TxPwrOverMAC = 0;
	UCHAR t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0, t7 = 0, t8 = 0;
	UCHAR t9 = 0, t10 = 0, t11 = 0, t12 = 0, t13 = 0, t14 = 0, t15 = 0, t16 = 0;
	UCHAR t17 = 0, t18 = 0, t19 = 0, t20 = 0, t21 = 0, t22 = 0, t23 = 0, t24 = 0;
	UCHAR t25 = 0, t26 = 0, t27 = 0, t28 = 0, t29 = 0, t30 = 0, t31 = 0, t32 = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	//
	// (a) Default Tx power for BW20 over 2.4G 
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (t2 << 12) | (t2 << 8) | (t1 << 4) | (t1));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (t2 << 8) | (t1));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9));

	//
	// (b) Default Tx power for BW40 over 2.4G 
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (DEFAULT_TX_POWER << 12) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER << 4) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("\n%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9));

	//
	// (c) Default Tx power for BW20 over 5G 
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (DEFAULT_TX_POWER << 12) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER << 4) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("\n%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9));

	//
	// (d) Default Tx power for BW40 over 5G 
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (DEFAULT_TX_POWER << 12) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER << 4) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("\n%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9));


	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}
#endif // RT3593 //


/*
	========================================================================
	
	Routine Description:
		Read initial Tx power per MCS and BW from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID	RTMPReadTxPwrPerRate(
	IN	PRTMP_ADAPTER	pAd)
{
	ULONG		data, Adata, Gdata;
	USHORT		i, value, value2;
	USHORT		value_1, value_2, value_3, value_4;
	INT			Apwrdelta, Gpwrdelta;
	UCHAR		t1,t2,t3,t4;
	BOOLEAN		bApwrdeltaMinus = TRUE, bGpwrdeltaMinus = TRUE;
	
#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RTMPReadTxPwrPerRateExt(pAd);
	}
	else
#endif // RT3593 //
	{	
		//
		// Get power delta for 20MHz and 40MHz.
		//
		DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value2);
		Apwrdelta = 0;
		Gpwrdelta = 0;

		if ((value2 & 0xff) != 0xff)
		{
			if ((value2 & 0x80))
				Gpwrdelta = (value2&0xf);
			
			if ((value2 & 0x40))
				bGpwrdeltaMinus = FALSE;
			else
				bGpwrdeltaMinus = TRUE;
		}
		if ((value2 & 0xff00) != 0xff00)
		{
			if ((value2 & 0x8000))
				Apwrdelta = ((value2&0xf00)>>8);

			if ((value2 & 0x4000))
				bApwrdeltaMinus = FALSE;
			else
				bApwrdeltaMinus = TRUE;
		}	
		DBGPRINT(RT_DEBUG_TRACE, ("Gpwrdelta = %x, Apwrdelta = %x .\n", Gpwrdelta, Apwrdelta));

		//
		// Get Txpower per MCS for 20MHz in 2.4G.
		//
		for (i=0; i<5; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, value);
			data = value;

			/* use value_1 ~ value_4 for code size reduce */
			value_1 = value&0xf;
			value_2 = (value&0xf0)>>4;
			value_3 = (value&0xf00)>>8;
			value_4 = (value&0xf000)>>12;

			if (bApwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Apwrdelta)
					t1 = value_1-(Apwrdelta);
				else
					t1 = 0;
				if (value_2 > Apwrdelta)
					t2 = value_2-(Apwrdelta);
				else
					t2 = 0;
				if (value_3 > Apwrdelta)
					t3 = value_3-(Apwrdelta);
				else
					t3 = 0;
				if (value_4 > Apwrdelta)
					t4 = value_4-(Apwrdelta);
				else
					t4 = 0;
			}				
			Adata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Gpwrdelta)
					t1 = value_1-(Gpwrdelta);
				else
					t1 = 0;
				if (value_2 > Gpwrdelta)
					t2 = value_2-(Gpwrdelta);
				else
					t2 = 0;
				if (value_3 > Gpwrdelta)
					t3 = value_3-(Gpwrdelta);
				else
					t3 = 0;
				if (value_4 > Gpwrdelta)
					t4 = value_4-(Gpwrdelta);
				else
					t4 = 0;
			}				
			Gdata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
			
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, value);

			/* use value_1 ~ value_4 for code size reduce */
			value_1 = value&0xf;
			value_2 = (value&0xf0)>>4;
			value_3 = (value&0xf00)>>8;
			value_4 = (value&0xf000)>>12;

			if (bApwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Apwrdelta)
					t1 = value_1-(Apwrdelta);
				else
					t1 = 0;
				if (value_2 > Apwrdelta)
					t2 = value_2-(Apwrdelta);
				else
					t2 = 0;
				if (value_3 > Apwrdelta)
					t3 = value_3-(Apwrdelta);
				else
					t3 = 0;
				if (value_4 > Apwrdelta)
					t4 = value_4-(Apwrdelta);
				else
					t4 = 0;
			}				
			Adata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Gpwrdelta)
					t1 = value_1-(Gpwrdelta);
				else
					t1 = 0;
				if (value_2 > Gpwrdelta)
					t2 = value_2-(Gpwrdelta);
				else
					t2 = 0;
				if (value_3 > Gpwrdelta)
					t3 = value_3-(Gpwrdelta);
				else
					t3 = 0;
				if (value_4 > Gpwrdelta)
					t4 = value_4-(Gpwrdelta);
				else
					t4 = 0;
			}				
			Gdata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
			data |= (value<<16);

			/* For 20M/40M Power Delta issue */		
			pAd->Tx20MPwrCfgABand[i] = data;
			pAd->Tx20MPwrCfgGBand[i] = data;
			pAd->Tx40MPwrCfgABand[i] = Adata;
			pAd->Tx40MPwrCfgGBand[i] = Gdata;
			
			if (data != 0xffffffff)
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);
			DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band-%lx,  Adata = %lx,  Gdata = %lx \n", data, Adata, Gdata));
		}
	}
}


/*
	========================================================================
	
	Routine Description:
		Read initial channel power parameters from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID	RTMPReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32					i, choffset;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_TX_PWR_STRUC	    Power2;
#ifdef RT3593
	EEPROM_TX_PWR_STRUC	    Power3;
	UCHAR Tx0ALC = 0, Tx1ALC = 0, Tx2ALC = 0, Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0, Tx2FinePowerCtrl = 0;
	EEPROM_ANTENNA_STRUC NICConfig0 = {{ 0 }};

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, NICConfig0.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: NICConfig0.field.TxPath = %d, NICConfig0.field.RxPath = %d\n", 
		__FUNCTION__, 
		NICConfig0.field.TxPath, 
		NICConfig0.field.RxPath));
#endif // RT3593 //

	// Read Tx power value for all channels
	// Value from 1 - 0x7f. Default value is 24.
	// Power value : 2.4G 0x00 (0) ~ 0x1F (31)
	//             : 5.5G 0xF9 (-7) ~ 0x0F (15)

	// 0. 11b/g, ch1 - ch 14
	for (i = 0; i < 7; i++)
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX0_OVER_2DOT4G + (i * 2)), Power.word);
				RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX1_OVER_2DOT4G + (i * 2)), Power2.word);
				RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX2_OVER_2DOT4G + (i * 2)), Power3.word);
			}
			else
			{
				RT28xx_EEPROM_READ16(pAd, (EEPROM_G_TX_PWR_OFFSET + i * 2), Power.word);
				RT28xx_EEPROM_READ16(pAd, (EEPROM_G_TX2_PWR_OFFSET + i * 2), Power2.word);
			}
			
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			//
			// Tx0 power control
			//
			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte0;
			Tx0FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2].Power = Tx0ALC;
			}

			if (Tx0FinePowerCtrl > 4)
			{
				pAd->TxPower[i * 2].Tx0FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
			}
			else
			{
				pAd->TxPower[i * 2].Tx0FinePowerCtrl = Tx0FinePowerCtrl;
			}

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte1;
			Tx0FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Power = Tx0ALC;
			}

			if (Tx0FinePowerCtrl > 4)
			{
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl = Tx0FinePowerCtrl;
			}

			//
			// Tx1 power control
			//
			if (NICConfig0.field.TxPath >= 2)
			{
				Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte0;
				Tx1FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power2.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx1ALC > 31)
				{
					pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power2 = Tx1ALC;
				}

				if (Tx1FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2].Tx1FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2].Tx1FinePowerCtrl = Tx1FinePowerCtrl;
				}

				Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte1;
				Tx1FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power2.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx1ALC > 31)
				{
					pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power2 = Tx1ALC;
				}

				if (Tx1FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl = Tx1FinePowerCtrl;
				}
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPower[%d].Power = 0x%02X, pAd->TxPower[%d].Tx0FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power = 0x%02X, pAd->TxPower[%d].Tx0FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power2 = 0x%02X, pAd->TxPower[%d].Tx1FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power2 = 0x%02X, pAd->TxPower[%d].Tx1FinePowerCtrl = 0x%02X, \n", 
				__FUNCTION__, 
				i * 2, 
				pAd->TxPower[i * 2].Power, 
				i * 2, 
				pAd->TxPower[i * 2].Tx0FinePowerCtrl, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl, 
				i * 2, 
				pAd->TxPower[i * 2].Power2, 
				i * 2, 
				pAd->TxPower[i * 2].Tx1FinePowerCtrl, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power2, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl));

			//
			// Tx2 power control
			//
			if (NICConfig0.field.TxPath == 3)
			{
				Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte0;
				Tx2FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power3.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx2ALC > 31)
				{
					pAd->TxPower[i * 2].Power3 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power3 = Tx2ALC;
				}

				if (Tx2FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2].Tx2FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2].Tx2FinePowerCtrl = Tx2FinePowerCtrl;
				}

				Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte1;
				Tx2FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power3.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx2ALC > 31)
				{
					pAd->TxPower[i * 2 + 1].Power3 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power3 = Tx2ALC;
				}

				if (Tx2FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2 + 1].Tx2FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Tx2FinePowerCtrl = Tx2FinePowerCtrl;
				}

				DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPower[%d].Power3 = 0x%02X, pAd->TxPower[%d].Tx2FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power3 = 0x%02X, pAd->TxPower[%d].Tx2FinePowerCtrl = 0x%02X\n", 
					__FUNCTION__, 
					i * 2, 
					pAd->TxPower[i * 2].Power3, 
					i * 2, 
					pAd->TxPower[i * 2].Tx2FinePowerCtrl, 
					i * 2 + 1, 
					pAd->TxPower[i * 2 + 1].Power3, 
					i * 2 + 1, 
					pAd->TxPower[i * 2 + 1].Tx2FinePowerCtrl));
			}
		}
		else // Tx power control over RF R12 and RF R13
#endif // RT3593 //
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2].Power = Power.field.Byte0;

			if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;

			if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
		}
	}
	
	// 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)
	// 1.1 Fill up channel
	choffset = 14;
	for (i = 0; i < 4; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
	}

	// 1.2 Fill up power
	for (i = 0; i < 6; i++)
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX0_OVER_5G + i * 2, Power.word);
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX1_OVER_5G + i * 2, Power2.word);
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX2_OVER_5G + i * 2, Power3.word);
			}
		}
		else
#endif // RT3593 //
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);
		}

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				if ((Power3.field.Byte0 < 16) && (Power3.field.Byte0 >= -7))
				{
					pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;
				}

				if ((Power3.field.Byte1 < 16) && (Power3.field.Byte1 >= -7))
				{
					pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;
				}
			}
		}
#endif // RT3593 //
	}
	
	// 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)
	// 2.1 Fill up channel
	choffset = 14 + 12;
	for (i = 0; i < 5; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
	pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
#ifdef RT3593
	pAd->TxPower[3 * 5 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;
#endif // RT3593 //

	// 2.2 Fill up power
	for (i = 0; i < 8; i++)
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX0_OVER_5G + (choffset - 14) + i * 2, Power.word);
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX1_OVER_5G + (choffset - 14) + i * 2, Power2.word);
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX2_OVER_5G + (choffset - 14) + i * 2, Power3.word);
			}
		}
		else
#endif // RT3593 //
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);
		}

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				if ((Power3.field.Byte0 < 16) && (Power3.field.Byte0 >= -7))
				{
					pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;
				}

				if ((Power3.field.Byte1 < 16) && (Power3.field.Byte1 >= -7))
				{
					pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;
				}
			}
		}
#endif // RT3593 //
	}

	// 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165, 167, 169; 171, 173 (including central frequency in BW 40MHz)
	// 3.1 Fill up channel
	choffset = 14 + 12 + 16;
	/*for (i = 0; i < 2; i++)*/
	for (i = 0; i < 3; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 3 + choffset + 0].Channel		= 171;
	pAd->TxPower[3 * 3 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 3 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
#ifdef RT3593
	pAd->TxPower[3 * 3 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;
#endif // RT3593 //

	pAd->TxPower[3 * 3 + choffset + 1].Channel		= 173;
	pAd->TxPower[3 * 3 + choffset + 1].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 3 + choffset + 1].Power2		= DEFAULT_RF_TX_POWER;
#ifdef RT3593
	pAd->TxPower[3 * 3 + choffset + 1].Power3		= DEFAULT_RF_TX_POWER;
#endif // RT3593 //

	// 3.2 Fill up power
	/*for (i = 0; i < 4; i++)*/
	for (i = 0; i < 6; i++)
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX0_OVER_5G + (choffset - 14) + i * 2, Power.word);
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX1_OVER_5G + (choffset - 14) + i * 2, Power2.word);
				RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX2_OVER_5G + (choffset - 14) + i * 2, Power3.word);
			}
		}
		else
#endif // RT3593 //
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);
		}

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				if ((Power3.field.Byte0 < 16) && (Power3.field.Byte0 >= -7))
				{
					pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;
				}

				if ((Power3.field.Byte1 < 16) && (Power3.field.Byte1 >= -7))
				{
					pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;
				}
			}
		}
#endif // RT3593 //
	}

	// 4. Print and Debug
	/*choffset = 14 + 12 + 16 + 7;*/
	choffset = 14 + 12 + 16 + 11;
	

}


/*
	========================================================================
	
	Routine Description:
		Read initial parameters from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID	NICReadEEPROMParameters(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			mac_addr)
{
	UINT32			data = 0;
	USHORT			i, value, value2;
	UCHAR			TmpPhy;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_VERSION_STRUC    Version;
	EEPROM_ANTENNA_STRUC	Antenna;
	EEPROM_NIC_CONFIG2_STRUC    NicConfig2;
	USHORT  Addr01,Addr23,Addr45 ;
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;


	DBGPRINT(RT_DEBUG_TRACE, ("--> NICReadEEPROMParameters\n"));	

	if (pAd->chipOps.eeinit)
	{
		pAd->chipOps.eeinit(pAd);
#ifdef RTMP_EFUSE_SUPPORT
#ifdef RT30xx
#ifdef RALINK_ATE
		if(!pAd->bFroceEEPROMBuffer && pAd->bEEPROMFile)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> NICReadEEPROMParameters::(Efuse)Load to EEPROM Buffer Mode\n"));	
			eFuseLoadEEPROM(pAd);
		}
#endif // RALINK_ATE //
#endif // RT30xx //
#endif // RTMP_EFUSE_SUPPORT //
	}

	// Init EEPROM Address Number, before access EEPROM; if 93c46, EEPROMAddressNum=6, else if 93c66, EEPROMAddressNum=8
	RTMP_IO_READ32(pAd, E2PROM_CSR, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("--> E2PROM_CSR = 0x%x\n", data));

	if((data & 0x30) == 0)
		pAd->EEPROMAddressNum = 6;		// 93C46
	else if((data & 0x30) == 0x10)
		pAd->EEPROMAddressNum = 8;     // 93C66
	else
		pAd->EEPROMAddressNum = 8;     // 93C86
	DBGPRINT(RT_DEBUG_TRACE, ("--> EEPROMAddressNum = %d\n", pAd->EEPROMAddressNum ));

	/* Read MAC setting from EEPROM and record as permanent MAC address */
	DBGPRINT(RT_DEBUG_TRACE, ("Initialize MAC Address from E2PROM \n"));

	RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
	RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
	RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);

	pAd->PermanentAddress[0] = (UCHAR)(Addr01 & 0xff);
	pAd->PermanentAddress[1] = (UCHAR)(Addr01 >> 8);
	pAd->PermanentAddress[2] = (UCHAR)(Addr23 & 0xff);
	pAd->PermanentAddress[3] = (UCHAR)(Addr23 >> 8);
	pAd->PermanentAddress[4] = (UCHAR)(Addr45 & 0xff);
	pAd->PermanentAddress[5] = (UCHAR)(Addr45 >> 8);

	//more conveninet to test mbssid, so ap's bssid &0xf1
	if (pAd->PermanentAddress[0] == 0xff)
		pAd->PermanentAddress[0] = RandomByte(pAd)&0xf8;
			
	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
								PRINT_MAC(pAd->PermanentAddress)));

	/* Assign the actually working MAC Address */
	if (pAd->bLocalAdminMAC)
	{		
		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from Configuration file(.dat). \n"));
	}
	else if (mac_addr && 
			 strlen((PSTRING)mac_addr) == 17 &&
			 (strcmp(mac_addr, "00:00:00:00:00:00") != 0))
	{
		INT		j;
		PSTRING	macptr;

		macptr = (PSTRING) mac_addr;

		for (j=0; j<MAC_ADDR_LEN; j++)
		{
			AtoH(macptr, &pAd->CurrentAddress[j], 1);
			macptr=macptr+3;
		}	
		
		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from Moudle Parameter. \n"));
	}
	else
	{
		COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from EEPROM. \n"));
	}

	/* Set the current MAC to ASIC */	
	csr2.field.Byte0 = pAd->CurrentAddress[0];
	csr2.field.Byte1 = pAd->CurrentAddress[1];
	csr2.field.Byte2 = pAd->CurrentAddress[2];
	csr2.field.Byte3 = pAd->CurrentAddress[3];
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW0, csr2.word);
	csr3.word = 0;
	csr3.field.Byte4 = pAd->CurrentAddress[4];
	csr3.field.Byte5 = pAd->CurrentAddress[5];
	csr3.field.U2MeMask = 0xff;
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW1, csr3.word);
	DBGPRINT_RAW(RT_DEBUG_TRACE,("Current MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(pAd->CurrentAddress)));

	// if not return early. cause fail at emulation.
	// Init the channel number for TX channel power
#if defined(RT2883) || defined(RT3883)
#ifdef RT3883
	if (IS_RT3883(pAd))
		RTMPRT3883ReadChannelPwr(pAd);
	else
#endif // RT3883 //
#endif // defined(RT2883) || defined(RT3883) //
		RTMPReadChannelPwr(pAd);

	// if E2PROM version mismatch with driver's expectation, then skip
	// all subsequent E2RPOM retieval and set a system error bit to notify GUI
	RT28xx_EEPROM_READ16(pAd, EEPROM_VERSION_OFFSET, Version.word);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: Version = %d, FAE release #%d\n", Version.field.Version, Version.field.FaeReleaseNumber));

	if (Version.field.Version > VALID_EEPROM_VERSION)
	{
		DBGPRINT_ERR(("E2PROM: WRONG VERSION 0x%x, should be %d\n",Version.field.Version, VALID_EEPROM_VERSION));
		/*pAd->SystemErrorBitmap |= 0x00000001;

		// hard-code default value when no proper E2PROM installed
		pAd->bAutoTxAgcA = FALSE;
		pAd->bAutoTxAgcG = FALSE;

		// Default the channel power
		for (i = 0; i < MAX_NUM_OF_CHANNELS; i++)
			pAd->TxPower[i].Power = DEFAULT_RF_TX_POWER;

		// Default the channel power
		for (i = 0; i < MAX_NUM_OF_11JCHANNELS; i++)
			pAd->TxPower11J[i].Power = DEFAULT_RF_TX_POWER;
		
		for(i = 0; i < NUM_EEPROM_BBP_PARMS; i++)
			pAd->EEPROMDefaultValue[i] = 0xffff;
		return;  */
	}

	// Read BBP default value from EEPROM and store to array(EEPROMDefaultValue) in pAd
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] = value;

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] = value;

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		UCHAR CountryRegion5G = 0, CountryRegion2Dot4G = 0;

		RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_COUNTRY_REGION_CODE_FOR_5G_BAND - 1), value);
				//
		// Swap
		//
		CountryRegion5G = ((value & 0xFF00) >> 8);
		CountryRegion2Dot4G = (value & 0x00FF);

		value = ((CountryRegion2Dot4G << 8) | CountryRegion5G);
				pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_COUNTRY_REGION, value);	// Country Region
		pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
	}

#ifdef BT_COEXISTENCE_SUPPORT
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC3_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG3_OFFSET] = value;
	pAd->NicConfig3.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG3_OFFSET];
#endif // BT_COEXISTENCE_SUPPORT //

	for(i = 0; i < 8; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_BBP_BASE_OFFSET + i*2, value);
		pAd->EEPROMDefaultValue[i+EEPROM_BBP_ARRAY_OFFSET] = value;
	}

	// We have to parse NIC configuration 0 at here.
	// If TSSI did not have preloaded value, it should reset the TxAutoAgc to false
	// Therefore, we have to read TxAutoAgc control beforehand.
	// Read Tx AGC control bit
	Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];
	if (Antenna.word == 0xFFFF)
	{
#ifdef RT30xx
		if(IS_RT3090(pAd))
		{
			Antenna.word = 0;
			Antenna.field.RfIcType = RFIC_3020;
			Antenna.field.TxPath = 1;
			Antenna.field.RxPath = 1;		
		}
		else
#endif // RT30xx //
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
		if (IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			Antenna.word = 0;
#ifdef RT3883
			if (IS_RT2883(pAd))
				Antenna.field.RfIcType = RFIC_3853;
#endif // RT3883 //
#ifdef RT3593
		if (IS_RT3593(pAd))
			Antenna.field.RfIcType = RFIC_3053;
#endif // RT3593 //
			Antenna.field.TxPath = 3;
			Antenna.field.RxPath = 3;
		}
		else
#endif // defined(RT2883) || defined(RT3883) || defined(RT3593) //
		{

			Antenna.word = 0;
			Antenna.field.RfIcType = RFIC_2820;
			Antenna.field.TxPath = 1;
			Antenna.field.RxPath = 2;
		}
		DBGPRINT(RT_DEBUG_WARN, ("E2PROM error, hard code as 0x%04x\n", Antenna.word));	
	}

	// Choose the desired Tx&Rx stream.
	if ((pAd->CommonCfg.TxStream == 0) || (pAd->CommonCfg.TxStream > Antenna.field.TxPath))
		pAd->CommonCfg.TxStream = Antenna.field.TxPath;

	if ((pAd->CommonCfg.RxStream == 0) || (pAd->CommonCfg.RxStream > Antenna.field.RxPath))
	{
		pAd->CommonCfg.RxStream = Antenna.field.RxPath;
	
		if ((pAd->MACVersion != RALINK_3883_VERSION) &&
			(pAd->MACVersion != RALINK_2883_VERSION) &&
			(pAd->CommonCfg.RxStream > 2))
		{
			// only 2 Rx streams for RT2860 series
			pAd->CommonCfg.RxStream = 2;
		}
	}


	/* EEPROM offset 0x36 - NIC Configuration 1 */
	NicConfig2.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];

#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	if ((pAd->MACVersion == 0x28600100) || (pAd->MACVersion == 0x28700100))
	WSC_HDR_BTN_MR_HDR_SUPPORT_SET(pAd, NicConfig2.field.EnableWPSPBC);
#endif // WSC_INCLUDED //

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (NicConfig2.word == 0xffff)
		{
			NicConfig2.word = 0;
		}
	}
#endif // CONFIG_AP_SUPPORT //	


	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;
	
	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;
	
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("NICReadEEPROMParameters: RxPath = %d, TxPath = %d\n", Antenna.field.RxPath, Antenna.field.TxPath));

	// Save the antenna for future use
	pAd->Antenna.word = Antenna.word;

	// Set the RfICType here, then we can initialize RFIC related operation callbacks
	pAd->Mlme.RealRxPath = (UCHAR) Antenna.field.RxPath;

	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;


#ifdef RTMP_RF_RW_SUPPORT
	RtmpChipOpsRFHook(pAd);
#endif // RTMP_RF_RW_SUPPORT //

	//
	// Reset PhyMode if we don't support 802.11a
	// Only RFIC_2850 & RFIC_2750 support 802.11a
	//
	if ((Antenna.field.RfIcType != RFIC_2850)
		&& (Antenna.field.RfIcType != RFIC_2750)
		&& (Antenna.field.RfIcType != RFIC_3052)
#ifdef RT3593
		&& (Antenna.field.RfIcType != RFIC_3053)
#endif // RT3593
#ifdef RT3883
		&& (Antenna.field.RfIcType != RFIC_3853)
#endif // RT3883 //
		)
	{
		if ((pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED) || 
			(pAd->CommonCfg.PhyMode == PHY_11A))
			pAd->CommonCfg.PhyMode = PHY_11BG_MIXED;
#ifdef DOT11_N_SUPPORT
		else if ((pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED)	|| 
				 (pAd->CommonCfg.PhyMode == PHY_11AN_MIXED) 	|| 
				 (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED) 	||
				 (pAd->CommonCfg.PhyMode == PHY_11N_5G))
			pAd->CommonCfg.PhyMode = PHY_11BGN_MIXED;
#endif // DOT11_N_SUPPORT //

		pAd->RFICType = RFIC_24GHZ; // CRDA
	}
	else
	{
		pAd->RFICType = RFIC_24GHZ | RFIC_5GHZ; // CRDA
	}
	
	// Read TSSI reference and TSSI boundary for temperature compensation. This is ugly
	// 0. 11b/g
	{
		/* these are tempature reference value (0x00 ~ 0xFE)
		   ex: 0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
		   TssiPlusBoundaryG [4] [3] [2] [1] [0] (smaller) +
		   TssiMinusBoundaryG[0] [1] [2] [3] [4] (larger) */
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G, Power.word);
			pAd->TssiMinusBoundaryG[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 2), Power.word);
			pAd->TssiMinusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 4), Power.word);
			pAd->TssiRefG = Power.field.Byte0;
			pAd->TssiPlusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 6), Power.word);
			pAd->TssiPlusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 8), Power.word);
			pAd->TssiPlusBoundaryG[4] = Power.field.Byte0;

			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_2DOT4G - 1), value);
			pAd->TxAgcStepG = ((value & 0xFF00) >> 8);

			pAd->TxAgcCompensateG = 0;
			pAd->TssiMinusBoundaryG[0] = pAd->TssiRefG;
			pAd->TssiPlusBoundaryG[0]  = pAd->TssiRefG;

			// Disable TxAgc if the based value is not right
			if (pAd->TssiRefG == 0xFF)
			{
				pAd->bAutoTxAgcG = FALSE;
			}
		}
		else
#endif // RT3593 //
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND1, Power.word);
			pAd->TssiMinusBoundaryG[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND2, Power.word);
			pAd->TssiMinusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND3, Power.word);
			pAd->TssiRefG   = Power.field.Byte0; /* reference value [0] */
			pAd->TssiPlusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND4, Power.word);
			pAd->TssiPlusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND5, Power.word);
			pAd->TssiPlusBoundaryG[4] = Power.field.Byte0;
			pAd->TxAgcStepG = Power.field.Byte1;    
			pAd->TxAgcCompensateG = 0;
			pAd->TssiMinusBoundaryG[0] = pAd->TssiRefG;
			pAd->TssiPlusBoundaryG[0]  = pAd->TssiRefG;

			// Disable TxAgc if the based value is not right
			if (pAd->TssiRefG == 0xff)
				pAd->bAutoTxAgcG = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE,("E2PROM: G Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
			pAd->TssiRefG,
			pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
			pAd->TxAgcStepG, pAd->bAutoTxAgcG));
	}	
	// 1. 11a
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G, Power.word);
			pAd->TssiMinusBoundaryA[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 2), Power.word);
			pAd->TssiMinusBoundaryA[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 4), Power.word);
			pAd->TssiRefA = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 6), Power.word);
			pAd->TssiPlusBoundaryA[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 8), Power.word);
			pAd->TssiPlusBoundaryA[4] = Power.field.Byte0;

			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_5G - 1), value);
			pAd->TxAgcStepA = ((value & 0xFF00) >> 8);

			pAd->TxAgcCompensateA = 0;
			pAd->TssiMinusBoundaryA[0] = pAd->TssiRefA;
			pAd->TssiPlusBoundaryA[0]  = pAd->TssiRefA;

			// Disable TxAgc if the based value is not right
			if (pAd->TssiRefA == 0xFF)
			{
				pAd->bAutoTxAgcA = FALSE;
			}
		}
		else
#endif // RT3593 //
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND1, Power.word);
			pAd->TssiMinusBoundaryA[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND2, Power.word);
			pAd->TssiMinusBoundaryA[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND3, Power.word);
			pAd->TssiRefA = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND4, Power.word);
			pAd->TssiPlusBoundaryA[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND5, Power.word);
			pAd->TssiPlusBoundaryA[4] = Power.field.Byte0;
			pAd->TxAgcStepA = Power.field.Byte1;    
			pAd->TxAgcCompensateA = 0;
			pAd->TssiMinusBoundaryA[0] = pAd->TssiRefA;
			pAd->TssiPlusBoundaryA[0]  = pAd->TssiRefA;

			// Disable TxAgc if the based value is not right
			if (pAd->TssiRefA == 0xff)
				pAd->bAutoTxAgcA = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE,("E2PROM: A Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryA[4], pAd->TssiMinusBoundaryA[3], pAd->TssiMinusBoundaryA[2], pAd->TssiMinusBoundaryA[1],
			pAd->TssiRefA,
			pAd->TssiPlusBoundaryA[1], pAd->TssiPlusBoundaryA[2], pAd->TssiPlusBoundaryA[3], pAd->TssiPlusBoundaryA[4],
			pAd->TxAgcStepA, pAd->bAutoTxAgcA));
	}	
	pAd->BbpRssiToDbmDelta = 0x0;
	
	// Read frequency offset setting for RF
#ifdef RT3593
	if (IS_RT3593(pAd))
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_FREQUENCY_OFFSET, value);
	else
#endif // RT3593 //
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);

	if ((value & 0x00FF) != 0x00FF)
		pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
	else
		pAd->RfFreqOffset = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: RF FreqOffset=0x%lx \n", pAd->RfFreqOffset));

	//CountryRegion byte offset (38h)
	value = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] >> 8;		// 2.4G band
	value2 = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] & 0x00FF;	// 5G band
	
	if ((value <= REGION_MAXIMUM_BG_BAND) && (value2 <= REGION_MAXIMUM_A_BAND))
	{
		pAd->CommonCfg.CountryRegion = ((UCHAR) value) | 0x80;
		pAd->CommonCfg.CountryRegionForABand = ((UCHAR) value2) | 0x80;
		TmpPhy = pAd->CommonCfg.PhyMode;
		pAd->CommonCfg.PhyMode = 0xff;
		RTMPSetPhyMode(pAd, TmpPhy);
#ifdef DOT11_N_SUPPORT
		SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
	}

	//
	// Get RSSI Offset on EEPROM 0x9Ah & 0x9Ch.
	// The valid value are (-10 ~ 10) 
	// 
#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_RSSI0_OVER_2DOT4G, value);
		pAd->BGRssiOffset0 = (value & 0x00FF);
		pAd->BGRssiOffset1 = ((value >> 8) & 0x00FF);
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, value);
		pAd->BGRssiOffset0 = value & 0x00ff;
		pAd->BGRssiOffset1 = (value >> 8);
	}

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_RSSI2_OVER_2DOT4G, value);
		pAd->BGRssiOffset2 = (value & 0x00FF);

		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH100_TO_CH128, value);
		pAd->ALNAGain1 = (value & 0x00FF);
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET+2, value);
//		if (IS_RT2860(pAd)) // RT2860 supports 3 Rx and the 2.4 GHz RSSI #2 offset is in the EEPROM 0x48
			pAd->BGRssiOffset2 = value & 0x00ff;
		pAd->ALNAGain1 = (value >> 8);
	}

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_2DOT4G, value);
		pAd->BLNAGain = (value & 0x00FF);
		pAd->ALNAGain0 = (value >> 8);
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, value);
		pAd->BLNAGain = value & 0x00ff;
		pAd->ALNAGain0 = (value >> 8);
	}
	
#ifdef RT3090
	// RT3090A's internal LNA gain is 0x0A// RT3090A's internal LNA gain is 0x0A
	if (IS_RT3090A(pAd))
	{
		pAd->BLNAGain = RT3090A_DEFAULT_INTERNAL_LNA_GAIN;
	}
#endif // RT3090 //
	// Validate 11b/g RSSI_0 offset.
	if ((pAd->BGRssiOffset0 < -10) || (pAd->BGRssiOffset0 > 10))
		pAd->BGRssiOffset0 = 0;

	// Validate 11b/g RSSI_1 offset.
	if ((pAd->BGRssiOffset1 < -10) || (pAd->BGRssiOffset1 > 10))
		pAd->BGRssiOffset1 = 0;

	// Validate 11b/g RSSI_2 offset.
	if ((pAd->BGRssiOffset2 < -10) || (pAd->BGRssiOffset2 > 10))
		pAd->BGRssiOffset2 = 0;

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_RSSI0_OVER_5G, value);
		pAd->ARssiOffset0 = (value & 0x00FF);
		pAd->ARssiOffset1 = ((value >> 8) & 0x00FF);
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);
		pAd->ARssiOffset0 = value & 0x00ff;
		pAd->ARssiOffset1 = (value >> 8);
	}

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_RSSI2_OVER_5G, value);
		pAd->ARssiOffset2 = (value & 0x00FF);

		RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH132_TO_CH165 - 1), value);
		pAd->ALNAGain2 = ((value >> 8) & 0x00FF);
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET+2), value);
		pAd->ARssiOffset2 = value & 0x00ff;
		pAd->ALNAGain2 = (value >> 8);
	}

#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
	RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET2, value);
	pAd->ALNAGain1 = value & 0x00ff;
	pAd->ALNAGain2 = (value >> 8);
	}
#endif // defined(RT2883) || defined(RT3883) //

	if (((UCHAR)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;
	if (((UCHAR)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	// Validate 11a RSSI_0 offset.
	if ((pAd->ARssiOffset0 < -10) || (pAd->ARssiOffset0 > 10))
		pAd->ARssiOffset0 = 0;

	// Validate 11a RSSI_1 offset.
	if ((pAd->ARssiOffset1 < -10) || (pAd->ARssiOffset1 > 10))
		pAd->ARssiOffset1 = 0;

	//Validate 11a RSSI_2 offset.
	if ((pAd->ARssiOffset2 < -10) || (pAd->ARssiOffset2 > 10))
		pAd->ARssiOffset2 = 0;

#ifdef RT30xx
	//
	// Get TX mixer gain setting
	// 0xff are invalid value
	// Note: RT30xX default value is 0x00 and will program to RF_R17 only when this value is not zero.
	//       RT359X default value is 0x02
	//
	if (IS_RT30xx(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXMIXER_GAIN_2_4G, value);
		pAd->TxMixerGain24G = 0;
		value &= 0x00ff;
		if (value != 0xff)
		{
			value &= 0x07;
			pAd->TxMixerGain24G = (UCHAR)value;
		}

#ifdef RT3593
		//2 TODO: test only!!!
		//2 TODO: Where are the Tx mixer gain for 2.4 GHz and 5 GHz?
		pAd->TxMixerGain24G = 0;
		pAd->TxMixerGain5G = 0;
#endif // RT3593 //
	}
#endif // RT30xx //
	
	//
	// Get LED Setting.
	//
#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_LED_MODE_SETTING - 1, value);
		pAd->LedCntl.word = ((value & 0xFF00) >> 8);

		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_LED_AG_CONFIGURATION, value);
		pAd->Led1 = value;

		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_LED_ACT_CONFIGURATION, value);
		pAd->Led2 = value;

		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_LED_AG_ACT_POLARITY, value);
		pAd->Led3 = value;
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);
		pAd->LedCntl.word = (value>>8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED1_OFFSET, value);
		pAd->Led1 = value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED2_OFFSET, value);
		pAd->Led2 = value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED3_OFFSET, value);
		pAd->Led3 = value;
	}
		
#if defined(RT2883) || defined(RT3883)
#ifdef RT3883
	if (IS_RT3883(pAd))
		RTMPRT3883ReadTxPwrPerRate(pAd);
	else
#endif // RT3883 //
#endif // #endif // defined(RT2883) || defined(RT3883) //
		RTMPReadTxPwrPerRate(pAd);

#ifdef SINGLE_SKU
#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_MAX_TX_POWER_OVER_2DOT4G_AND_5G, pAd->CommonCfg.DefineMaxTxPwr);
	}
	else
#endif // RT3593 //
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_DEFINE_MAX_TXPWR, pAd->CommonCfg.DefineMaxTxPwr);
	}

	if ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF) <= 0x50 && pAd->CommonCfg.AntGain > 0 && pAd->CommonCfg.BandedgeDelta >= 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Single SKU Mode is enabled\n"));
		pAd->CommonCfg.bSKUMode = TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Single SKU Mode is disabled\n"));
		pAd->CommonCfg.bSKUMode = FALSE;
	}
#endif // SINGLE_SKU //

#if defined(RT2883) || defined(RT3883)
	if(IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		UCHAR BbpValue;
		
		// select Rx chain 0
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &BbpValue);
		BbpValue &= ~0x60;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, BbpValue);

		RT28xx_EEPROM_READ16(pAd, 0x1a0, value);
		BbpValue = (UCHAR) (value & 0xff);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, BbpValue);
		BbpValue = (UCHAR) (value >> 8);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R175, BbpValue);

		RT28xx_EEPROM_READ16(pAd, 0x1a2, value);
		BbpValue = (UCHAR) (value & 0xff);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, BbpValue);
		BbpValue = (UCHAR) (value >> 8);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R177, BbpValue);


		// select Rx chain 1
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &BbpValue);
		BbpValue &= ~0x60;
		BbpValue |= 0x20;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, BbpValue);

		RT28xx_EEPROM_READ16(pAd, 0x1a4, value);
		BbpValue = (UCHAR) (value >> 8);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R175, BbpValue);

		RT28xx_EEPROM_READ16(pAd, 0x1a6, value);
		BbpValue = (UCHAR) (value >> 8);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R177, BbpValue);


		// select Rx chain 2
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &BbpValue);
		BbpValue &= ~0x60;
		BbpValue |= 0x40;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, BbpValue);

		RT28xx_EEPROM_READ16(pAd, 0x1a8, value);
		BbpValue = (UCHAR) (value & 0xff);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, BbpValue);
		BbpValue = (UCHAR) (value >> 8);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R175, BbpValue);

		RT28xx_EEPROM_READ16(pAd, 0x1aa, value);
		BbpValue = (UCHAR) (value & 0xff);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, BbpValue);
		BbpValue = (UCHAR) (value >> 8);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R177, BbpValue);

	}
#ifdef TXBF_SUPPORT
	// Read ITxBF compensation parameters from EEPROM
	for (i=0; i<6; i++) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_ITXBF_CAL_RX0+2*i, value);
		pAd->EEPROMITxBFCalParams[i] = value;
	}
#endif // TXBF_SUPPORT //

#endif // defined(RT2883) || defined(RT3883) //

#ifdef RT30xx
#ifdef RTMP_EFUSE_SUPPORT
	RtmpEfuseSupportCheck(pAd);
#endif // RTMP_EFUSE_SUPPORT //
#endif // RT30xx //

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICReadEEPROMParameters\n"));
}


/*
	========================================================================
	
	Routine Description:
		Set default value from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	NICInitAsicFromEEPROM(
	IN	PRTMP_ADAPTER	pAd)
{
	USHORT					i;
	EEPROM_NIC_CONFIG2_STRUC    NicConfig2;
	UCHAR	BBPR3 = 0;
#ifdef RT30xx
	UCHAR			bbpreg = 0;
	UCHAR			RFValue = 0;
#endif // RT30xx //
#if defined(RT2883) || defined(RT3883)
	UCHAR	bbpValue;
#endif // defined(RT2883) || defined(RT3883) //
	
	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitAsicFromEEPROM\n"));
	for(i = EEPROM_BBP_ARRAY_OFFSET; i < NUM_EEPROM_BBP_PARMS; i++)
	{
		UCHAR BbpRegIdx, BbpValue;
	
		if ((pAd->EEPROMDefaultValue[i] != 0xFFFF) && (pAd->EEPROMDefaultValue[i] != 0))
		{
			BbpRegIdx = (UCHAR)(pAd->EEPROMDefaultValue[i] >> 8);
			BbpValue  = (UCHAR)(pAd->EEPROMDefaultValue[i] & 0xff);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BbpRegIdx, BbpValue);
		}
	}

	NicConfig2.word = pAd->NicConfig2.word;

#ifdef RT30xx
	// set default antenna as main
	if ((pAd->RfIcType == RFIC_2020) || (pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_3320))
		AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
#endif // RT30xx //

#ifdef LED_CONTROL_SUPPORT
	//
	// Send LED Setting to MCU.
	//
	if (pAd->LedCntl.word == 0xFF)
	{
		pAd->LedCntl.word = 0x01;
		pAd->Led1 = 0x5555;
		pAd->Led2 = 0x2221;

#ifdef RTMP_MAC_PCI
		pAd->Led3 = 0xA9F8;
#endif // RTMP_MAC_PCI //
	}

	AsicSendCommandToMcu(pAd, 0x52, 0xff, (UCHAR)pAd->Led1, (UCHAR)(pAd->Led1 >> 8));
	AsicSendCommandToMcu(pAd, 0x53, 0xff, (UCHAR)pAd->Led2, (UCHAR)(pAd->Led2 >> 8));
	AsicSendCommandToMcu(pAd, 0x54, 0xff, (UCHAR)pAd->Led3, (UCHAR)(pAd->Led3 >> 8));
	AsicSendCommandToMcu(pAd, 0x51, 0xff, 0, pAd->LedCntl.field.Polarity);
	
	pAd->LedIndicatorStrength = 0xFF;
	RTMPSetSignalLED(pAd, -100);	// Force signal strength Led to be turned off, before link up
#endif // LED_CONTROL_SUPPORT //

#ifdef RTMP_RF_RW_SUPPORT
	//Init RT30xx RFRegisters after read RFIC type from EEPROM
	NICInitRFRegisters(pAd);
#endif // RTMP_RF_RW_SUPPORT //

#ifdef RTMP_PCI_SUPPORT
                if (IS_RT3090(pAd)|| IS_RT3572(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd))
                {
                        RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
                        if (pChipOps->AsicReverseRfFromSleepMode)
                                pChipOps->AsicReverseRfFromSleepMode(pAd);
                }

#endif // RTMP_PCI_SUPPORT //
	// Turn off patching for cardbus controller
	if (NicConfig2.field.CardbusAcceleration == 1)
	{
//		pAd->bTest1 = TRUE;
	}

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

#ifdef RTMP_INTERNAL_TX_ALC
/*
    Internal Tx ALC support is starting from RT3370 / RT3390, which combine PA / LNA in single chip.
    The old chipset don't have this, add new feature flag RTMP_INTERNAL_ALC.
 */
	// Internal Tx ALC
	if (((NicConfig2.field.DynamicTxAgcControl == 1) && 
            (NicConfig2.field.bInternalTxALC == 1)) || (!IS_RT3390(pAd)))
	{
		pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}
	else
	{
		if (NicConfig2.field.bInternalTxALC == 1)
		{
			pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		}
		else
		{
			pAd->TxPowerCtrl.bInternalTxALC = FALSE;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPowerCtrl.bInternalTxALC = %d\n", 
		__FUNCTION__, 
		pAd->TxPowerCtrl.bInternalTxALC));
#endif // RTMP_INTERNAL_TX_ALC //

	//
	// Since BBP has been progamed, to make sure BBP setting will be 
	// upate inside of AsicAntennaSelect, so reset to UNKNOWN_BAND!!
	//
	pAd->CommonCfg.BandState = UNKNOWN_BAND;
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);
	if(pAd->Antenna.field.RxPath == 3)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		BBPR3 |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
	

#ifdef RT30xx
	// Initialize RT3070 serial MAC registers which is different from RT2870 serial
	if (IS_RT3090(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd))
	{
		// enable DC filter
		if ((pAd->MACVersion & 0xffff) >= 0x0211)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xc0);
		}

		// improve power consumption in RT3071 Ver.E 
		if ((pAd->MACVersion & 0xffff) >= 0x0211)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R31, &bbpreg);
			bbpreg &= (~0x3);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, bbpreg);
		}


		RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);
		
		// RT3071 version E has fixed this issue
		if ((pAd->MACVersion & 0xffff) < 0x0211)
		{
			if (pAd->NicConfig2.field.DACTestBit == 1)
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x2C);	// To fix throughput drop drastically
			}
			else
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0F);	// To fix throughput drop drastically
			}
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);
		}
	}
	else if (IS_RT3070(pAd))
	{
		if ((pAd->MACVersion & 0xffff) >= 0x0201)
		{
			// enable DC filter
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xc0);
			
			// improve power consumption in RT3070 Ver.F
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R31, &bbpreg);
			bbpreg &= (~0x3);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, bbpreg);
		}
		/*
		     RT3070(E) Version[0200]
		     RT3070(F) Version[0201]
		 */
		if (((pAd->MACVersion & 0xffff) < 0x0201))
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x2C);	// To fix throughput drop drastically
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0);
		}
	}
	else if (IS_RT3071(pAd) || IS_RT3572(pAd))
	{
		RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);
		if (((pAd->MACVersion & 0xffff) < 0x0211))
		{
			if (pAd->NicConfig2.field.DACTestBit == 1)
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x1F); // To fix throughput drop drastically
			}
			else
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0F); // To fix throughput drop drastically
			}
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);
		}
	}

	// update registers from EEPROM for RT3071 or later(3572/3562/3592).
	if (IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
	{
		UCHAR RegIdx, RegValue;
		USHORT value;

		// after RT3071, write BBP from EEPROM 0xF0 to 0x102
		for (i = 0xF0; i <= 0x102; i = i+2)
		{
			value = 0xFFFF;
			RT28xx_EEPROM_READ16(pAd, i, value);
			if ((value != 0xFFFF) && (value != 0))
			{
				RegIdx = (UCHAR)(value >> 8);
				RegValue  = (UCHAR)(value & 0xff);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, RegIdx, RegValue);
				DBGPRINT(RT_DEBUG_TRACE, ("Update BBP Registers from EEPROM(0x%0x), BBP(0x%x) = 0x%x\n", i, RegIdx, RegValue));
			}
		}

		// after RT3071, write RF from EEPROM 0x104 to 0x116
		for (i = 0x104; i <= 0x116; i = i+2)
		{
			value = 0xFFFF;
			RT28xx_EEPROM_READ16(pAd, i, value);
			if ((value != 0xFFFF) && (value != 0))
			{
				RegIdx = (UCHAR)(value >> 8);
				RegValue  = (UCHAR)(value & 0xff);
				RT30xxWriteRFRegister(pAd, RegIdx, RegValue);
				DBGPRINT(RT_DEBUG_TRACE, ("Update RF Registers from EEPROM0x%x), BBP(0x%x) = 0x%x\n", i, RegIdx, RegValue));
			}
		}
	}
#endif // RT30xx //

#if defined(RT2883) || defined(RT3883)
	// Set ITxBF compensation parameters if they are not all the uninitialized value 0xFFFF
#ifdef TXBF_SUPPORT	
	if ((pAd->EEPROMITxBFCalParams[0] & pAd->EEPROMITxBFCalParams[1] & pAd->EEPROMITxBFCalParams[2] &
		pAd->EEPROMITxBFCalParams[3] & pAd->EEPROMITxBFCalParams[4] & pAd->EEPROMITxBFCalParams[5]) != 0xFFFF) {
		
		// Select Ant0 
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &bbpValue);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (bbpValue & ~0x60));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, (pAd->EEPROMITxBFCalParams[0] & 0xFF));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R175, (pAd->EEPROMITxBFCalParams[0] >> 8));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, (pAd->EEPROMITxBFCalParams[1] & 0xFF));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R177, (pAd->EEPROMITxBFCalParams[1] >> 8));
		
		// Select Ant1 
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (bbpValue & ~0x60) | 0x20);
		//BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, (pAd->EEPROMITxBFCalParams[2] & 0xFF));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R175, (pAd->EEPROMITxBFCalParams[2] >> 8));
		//BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, (pAd->EEPROMITxBFCalParams[3] & 0xFF));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R177, (pAd->EEPROMITxBFCalParams[3] >> 8));
		
		// Select Ant2
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (bbpValue & ~0x60) | 0x40);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, (pAd->EEPROMITxBFCalParams[4] & 0xFF));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R175, (pAd->EEPROMITxBFCalParams[4] >> 8));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, (pAd->EEPROMITxBFCalParams[5] & 0xFF));
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R177, (pAd->EEPROMITxBFCalParams[5] >> 8));
		
		// Enable Phase Compensation
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, 0x3C);

		// Restore R27
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpValue);
	}
	else
		printk("EEPROM TxBF is not set\n");
#endif // TXBF_SUPPORT //
#endif // defined(RT2883) || defined(RT3883) //

	DBGPRINT(RT_DEBUG_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d, Polar+LED mode=%x\n", 
				pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, 
				pAd->RfIcType, pAd->LedCntl.word));
	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}

/*
	========================================================================
	
	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
NDIS_STATUS	NICInitializeAdapter(
	IN	PRTMP_ADAPTER	pAd,
	IN   BOOLEAN    bHardReset)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
	WPDMA_GLO_CFG_STRUC	GloCfg;
#ifdef RTMP_MAC_PCI
	UINT32			Value;
	DELAY_INT_CFG_STRUC	IntCfg;
#endif // RTMP_MAC_PCI //
//	INT_MASK_CSR_STRUC		IntMask;
	ULONG	i =0;
	ULONG	j=0;
	//AC_TXOP_CSR0_STRUC	csr0;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAdapter\n"));
	
	// 3. Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits:
retry:
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;
		
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return NDIS_STATUS_FAILURE;
		RTMPusecDelay(1000);
		i++;
	}while ( i<100);
	DBGPRINT(RT_DEBUG_TRACE, ("<== DMA offset 0x208 = 0x%x\n", GloCfg.word));	
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
	
	// Record HW Beacon offset
#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		DBGPRINT(RT_DEBUG_INFO, ("%s: Put the beacon contents in the higher shared memory location\n", __FUNCTION__));

		pAd->BeaconOffset[0] = HW_BEACON_BASE0_REDIRECTION;
		pAd->BeaconOffset[1] = HW_BEACON_BASE1_REDIRECTION;
		pAd->BeaconOffset[2] = HW_BEACON_BASE2_REDIRECTION;
		pAd->BeaconOffset[3] = HW_BEACON_BASE3_REDIRECTION;
		pAd->BeaconOffset[4] = HW_BEACON_BASE4_REDIRECTION;
		pAd->BeaconOffset[5] = HW_BEACON_BASE5_REDIRECTION;
		pAd->BeaconOffset[6] = HW_BEACON_BASE6_REDIRECTION;
		pAd->BeaconOffset[7] = HW_BEACON_BASE7_REDIRECTION;
	}
	else
#endif // RT3593 //
	{
		pAd->BeaconOffset[0] = HW_BEACON_BASE0;
		pAd->BeaconOffset[1] = HW_BEACON_BASE1;
		pAd->BeaconOffset[2] = HW_BEACON_BASE2;
		pAd->BeaconOffset[3] = HW_BEACON_BASE3;
		pAd->BeaconOffset[4] = HW_BEACON_BASE4;
		pAd->BeaconOffset[5] = HW_BEACON_BASE5;
		pAd->BeaconOffset[6] = HW_BEACON_BASE6;
		pAd->BeaconOffset[7] = HW_BEACON_BASE7;
	}
	
#if defined (RT3352) || defined (RT3883) && defined (CONFIG_16MBSSID_MODE)
	pAd->BeaconOffset[8] = HW_BEACON_BASE8;
	pAd->BeaconOffset[9] = HW_BEACON_BASE9;
	pAd->BeaconOffset[10] = HW_BEACON_BASE10;
	pAd->BeaconOffset[11] = HW_BEACON_BASE11;
	pAd->BeaconOffset[12] = HW_BEACON_BASE12;
	pAd->BeaconOffset[13] = HW_BEACON_BASE13;
	pAd->BeaconOffset[14] = HW_BEACON_BASE14;
	pAd->BeaconOffset[15] = HW_BEACON_BASE15;
#endif

	//
	// write all shared Ring's base address into ASIC
	//

	// asic simulation sequence put this ahead before loading firmware.
	// pbf hardware reset
#ifdef RTMP_MAC_PCI
	RTMP_IO_WRITE32(pAd, WPDMA_RST_IDX, 0x1003f);	// 0x10000 for reset rx, 0x3f resets all 6 tx rings.
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe1f);
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe00);
#endif // RTMP_MAC_PCI //

	// Initialze ASIC for TX & Rx operation
	if (NICInitializeAsic(pAd , bHardReset) != NDIS_STATUS_SUCCESS)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return NDIS_STATUS_FAILURE;
		if (j++ == 0)
		{
			NICLoadFirmware(pAd);
			goto retry;
		}
		return NDIS_STATUS_FAILURE;
	}


#ifdef RTMP_MAC_PCI
	// Write AC_BK base address register
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_BK].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR1, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR1 : 0x%x\n", Value));

	// Write AC_BE base address register
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_BE].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR0, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR0 : 0x%x\n", Value));

	// Write AC_VI base address register
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_VI].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR2, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR2 : 0x%x\n", Value));

	// Write AC_VO base address register
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_VO].Cell[0].AllocPa);	
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR3, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR3 : 0x%x\n", Value));

	// Write HCCA base address register
	  Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_HCCA].Cell[0].AllocPa);
	  RTMP_IO_WRITE32(pAd, TX_BASE_PTR4, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR4 : 0x%x\n", Value));

	// Write MGMT_BASE_CSR register
	Value = RTMP_GetPhysicalAddressLow(pAd->MgmtRing.Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR5, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR5 : 0x%x\n", Value));

	// Write RX_BASE_CSR register
	Value = RTMP_GetPhysicalAddressLow(pAd->RxRing.Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, RX_BASE_PTR, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> RX_BASE_PTR : 0x%x\n", Value));

	// Init RX Ring index pointer
	pAd->RxRing.RxSwReadIdx = 0;
	pAd->RxRing.RxCpuIdx = RX_RING_SIZE-1;
	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);
	
	// Init TX rings index pointer
	{
		for (i=0; i<NUM_OF_TX_RING; i++)
		{
			pAd->TxRing[i].TxSwFreeIdx = 0;
			pAd->TxRing[i].TxCpuIdx = 0;
			RTMP_IO_WRITE32(pAd, (TX_CTX_IDX0 + i * 0x10) ,  pAd->TxRing[i].TxCpuIdx);
		}
	}

	// init MGMT ring index pointer
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;
	RTMP_IO_WRITE32(pAd, TX_MGMTCTX_IDX,  pAd->MgmtRing.TxCpuIdx);

	//
	// set each Ring's SIZE  into ASIC. Descriptor Size is fixed by design.
	//

	// Write TX_RING_CSR0 register
	Value = TX_RING_SIZE;
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT0, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT1, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT2, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT3, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT4, Value);
	Value = MGMT_RING_SIZE;
	RTMP_IO_WRITE32(pAd, TX_MGMTMAX_CNT, Value);

	// Write RX_RING_CSR register
	Value = RX_RING_SIZE;
	RTMP_IO_WRITE32(pAd, RX_MAX_CNT, Value);
#endif // RTMP_MAC_PCI //



#ifdef RTMP_MAC_PCI
	// 3. Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits:
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;
		
		RTMPusecDelay(1000);
		i++;
	}while ( i < 100);

	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
	
	IntCfg.word = 0;
	RTMP_IO_WRITE32(pAd, DELAY_INT_CFG, IntCfg.word);
#endif // RTMP_MAC_PCI //


	// reset action
	// Load firmware
	//  Status = NICLoadFirmware(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAdapter\n"));
	return Status;
}

/*
	========================================================================
	
	Routine Description:
		Initialize ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
NDIS_STATUS	NICInitializeAsic(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN		bHardReset)
{
	ULONG			Index = 0;
	UCHAR			R0 = 0xff;
	UINT32			MacCsr12 = 0, Counter = 0;
#ifdef RTMP_MAC_PCI
	UINT32			Value;
#endif // RTMP_MAC_PCI //
	USHORT			KeyIdx;
	INT				i,apidx;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAsic\n"));

#ifdef RTMP_MAC_PCI
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x3);	// To fix driver disable/enable hang issue when radio off
	if (bHardReset == TRUE)
	{
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x3);
	}
	else
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
	// Initialize MAC register to default value
	for (Index = 0; Index < NUM_MAC_REG_PARMS; Index++)
	{
#ifdef RT30xx
		if ((MACRegTable[Index].Register == TX_SW_CFG0) && (IS_RT3090(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd)))
		{
			MACRegTable[Index].Value = 0x00000400;
		}
#endif // RT30xx //
		RTMP_IO_WRITE32(pAd, MACRegTable[Index].Register, MACRegTable[Index].Value);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (Index = 0; Index < NUM_AP_MAC_REG_PARMS; Index++)
		{
			RTMP_IO_WRITE32(pAd, APMACRegTable[Index].Register, APMACRegTable[Index].Value);
		}
	}
#endif // CONFIG_AP_SUPPORT //

#endif // RTMP_MAC_PCI //

#ifdef RTMP_MAC_PCI
	pAd->CommonCfg.bPCIeBus = FALSE; // PCI bus

	// PCI and PCIe have different value in US_CYC_CNT
	RTMP_IO_READ32(pAd, PCI_CFG, &Value);
	if ((Value & 0x10000) == 0)
	{
		US_CYC_CNT_STRUC	USCycCnt;

		RTMP_IO_READ32(pAd, US_CYC_CNT, &USCycCnt.word);
		USCycCnt.field.UsCycCnt = 0x7D;
		RTMP_IO_WRITE32(pAd, US_CYC_CNT, USCycCnt.word);
		
		pAd->CommonCfg.bPCIeBus = TRUE;
		
		DBGPRINT(RT_DEBUG_TRACE, ("NICInitializeAsic::act as PCIe driver \n"));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("NICInitializeAsic::act as PCI driver \n"));
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->CommonCfg.bPCIeBus = %d\n", 
		__FUNCTION__, 
		pAd->CommonCfg.bPCIeBus));
#endif // RTMP_MAC_PCI //


	//
	// Before program BBP, we need to wait BBP/RF get wake up.
	//
	Index = 0;
	do
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacCsr12);

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return NDIS_STATUS_FAILURE;

		if ((MacCsr12 & 0x03) == 0)	// if BB.RF is stable
			break;
		
		DBGPRINT(RT_DEBUG_TRACE, ("Check MAC_STATUS_CFG  = Busy = %x\n", MacCsr12));
		RTMPusecDelay(1000);
	} while (Index++ < 100);

#ifdef RTMP_MAC_PCI
	// The commands to firmware should be after these commands, these commands will init firmware
	// PCI and USB are not the same because PCI driver needs to wait for PCI bus ready
	RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, 0);	// initialize BBP R/W access agent
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CSR, 0);
	AsicSendCommandToMcu(pAd, 0x72, 0, 0, 0);
#endif // RTMP_MAC_PCI //

	// Wait to be stable.
	RTMPusecDelay(1000);
	pAd->LastMCUCmd = 0x72;

	// Read BBP register, make sure BBP is up and running before write new data
	Index = 0;
	do 
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R0, &R0);
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return NDIS_STATUS_FAILURE;
		DBGPRINT(RT_DEBUG_TRACE, ("BBP version = %x\n", R0));
	} while ((++Index < 20) && ((R0 == 0xff) || (R0 == 0x00)));
	//ASSERT(Index < 20); //this will cause BSOD on Check-build driver

	if ((R0 == 0xff) || (R0 == 0x00))
		return NDIS_STATUS_FAILURE;

	// Initialize BBP register to default value
	for (Index = 0; Index < NUM_BBP_REG_PARMS; Index++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBPRegTable[Index].Register, BBPRegTable[Index].Value);
	}

	if (pAd->chipOps.pBBPRegTable)
	{
		REG_PAIR *pbbpRegTb = pAd->chipOps.pBBPRegTable;
		
		for (Index = 0; Index < pAd->chipOps.bbpRegTbSize; Index++)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, pbbpRegTb[Index].Register, pbbpRegTb[Index].Value);
			DBGPRINT(RT_DEBUG_TRACE, ("BBP_R%d=%d\n", pbbpRegTb[Index].Register, pbbpRegTb[Index].Value));
		}
	}
	
	
#ifdef RTMP_MAC_PCI
	// TODO: shiang, check MACVersion, currently, rbus-based chip use this.
	if (pAd->MACVersion == 0x28720200)
	{
		//UCHAR value;
		ULONG value2;

		//disable MLD by Bruce 20080704
		//BBP_IO_READ8_BY_REG_ID(pAd, BBP_R105, &value);
		//BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, value | 4);
	
		//Maximum PSDU length from 16K to 32K bytes	
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &value2);
		value2 &= ~(0x3<<12);
		value2 |= (0x2<<12);
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, value2);
	}
#endif // RTMP_MAC_PCI //

	// for rt2860E and after, init BBP_R84 with 0x19. This is for extension channel overlapping IOT.
	// RT3090 should not program BBP R84 to 0x19, otherwise TX will block.
	//3070/71/72,3090,3090A( are included in RT30xx),3572,3390
	if (((pAd->MACVersion & 0xffff) != 0x0101) &&
		!(IS_RT30xx(pAd)|| IS_RT3572(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd)))
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x19);

#ifdef RT30xx
	// RF power sequence setup
	if (IS_RT30xx(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd))
	{	//update for RT3070/71/72/90/91/92,3572,3390.
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R79, 0x13);		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R80, 0x05);	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R81, 0x33);	
	}
#endif // RT30xx //

	if (pAd->MACVersion == 0x28600100)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x12);
	}
	
	if ((pAd->MACVersion == RALINK_3883_VERSION) || ((pAd->MACVersion >= RALINK_2880E_VERSION) && (pAd->MACVersion < RALINK_3070_VERSION))) // 3*3
	{
		// enlarge MAX_LEN_CFG
		UINT32 csr;
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &csr);
#if defined(RT2883) || defined(RT3883)
		if (IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			csr |= 0x3fff; // fix me
		}
		else
#endif // defined(RT2883) || defined(RT3883) //
		{
		csr &= 0xFFF;
		csr |= 0x2000;
		}
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, csr);
	}



	// Clear raw counters
	RTMP_IO_READ32(pAd, RX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &Counter);
	
	// ASIC will keep garbage value after boot
	// Clear all shared key table when initial
	// This routine can be ignored in radio-ON/OFF operation. 
	if (bHardReset)
	{
		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE + 4*KeyIdx, 0);
		}

		// Clear all pairwise key table when initial
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, MAC_WCID_ATTRIBUTE_BASE + (KeyIdx * HW_WCID_ATTRI_SIZE), 1);
		}
	}
	
	// assert HOST ready bit
//  RTMP_IO_WRITE32(pAd, MAC_CSR1, 0x0); // 2004-09-14 asked by Mark
//  RTMP_IO_WRITE32(pAd, MAC_CSR1, 0x4);

	// It isn't necessary to clear this space when not hard reset. 	
	if (bHardReset == TRUE)
	{
		// clear all on-chip BEACON frame space			

		for (apidx = 0; apidx < HW_BEACON_MAX_COUNT; apidx++)
		{
			for (i = 0; i < HW_BEACON_OFFSET>>2; i+=4)
				RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[apidx] + i, 0x00); 
#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
		}
	}
	


	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAsic\n"));
	return NDIS_STATUS_SUCCESS;
}




#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
/*
	========================================================================
	
	Routine Description:
		In the case, Client may be silent left without sending DeAuth or DeAssoc.
		AP'll continue retry packets for the client since AP doesn't know the STA
		is gone. To Minimum affection of exist traffic is disable retransmition for
		all those packet relative to the STA.
		So decide to change ack required setting of all packet in TX ring
		to "no ACK" requirement for specific Client.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
static VOID ClearTxRingClientAck(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry)

{
	INT index;
	USHORT TxIdx;
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC pTxD;
	PTXWI_STRUC pTxWI;

	if (!pAd || !pEntry)
		return;

	for (index = 3; index >= 0; index--)
	{
		pTxRing = &pAd->TxRing[index];
		for (TxIdx = 0; TxIdx < TX_RING_SIZE; TxIdx++)
		{
			pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
			if (!pTxD->DMADONE)
			{
				 pTxWI = (PTXWI_STRUC) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
				 if (pTxWI->WirelessCliID == pEntry->Aid)
					pTxWI->ACK = FALSE;
			}
		}
	}
}
#endif // RTMP_MAC_PCI //
#endif // CONFIG_AP_SUPPORT //

VOID NICUpdateFifoStaCounters(
	IN PRTMP_ADAPTER pAd)
{
	TX_STA_FIFO_STRUC	StaFifo;
	MAC_TABLE_ENTRY		*pEntry;
	UINT32				i = 0;
	UCHAR				pid = 0, wcid = 0;
	INT32				reTry;
	UCHAR				succMCS;

#ifdef RALINK_ATE		
	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return;
#endif // RALINK_ATE //

		do
		{
			RTMP_IO_READ32(pAd, TX_STA_FIFO, &StaFifo.word);

			if (StaFifo.field.bValid == 0)
				break;
		
			wcid = (UCHAR)StaFifo.field.wcid;

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef DFS_SUPPORT
				/* the CTS frame for DFS and Carrier-Sense. */
				if ((pAd->CommonCfg.Channel > 14)
					&& (pAd->CommonCfg.bIEEE80211H == TRUE)
					&& (pAd->CommonCfg.RadarDetect.RDMode == RD_NORMAL_MODE)
					&& (wcid == DFS_CTS_WCID))
				{
					i++;
					DFSStartTrigger(pAd);
					continue;
				}
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
				if ((pAd->CommonCfg.CarrierDetect.Enable == TRUE)
					&& (wcid == CS_CTS_WCID))
				{
					i++;
					CarrierDetectStartTrigger(pAd);
					continue;
				}
#endif // CARRIER_DETECTION_SUPPORT //
			}
#endif // CONFIG_AP_SUPPORT //

		/* ignore NoACK and MGMT frame use 0xFF as WCID */
			if ((StaFifo.field.TxAckRequired == 0) || (wcid >= MAX_LEN_OF_MAC_TABLE))
			{
				i++;
				continue;
			}

			/* PID store Tx MCS Rate */
			pid = (UCHAR)StaFifo.field.PidType;

			pEntry = &pAd->MacTab.Content[wcid];

			pEntry->DebugFIFOCount++;

#ifdef DOT11_N_SUPPORT
#endif // DOT11_N_SUPPORT //

#ifdef UAPSD_AP_SUPPORT
			UAPSD_SP_AUE_Handle(pAd, pEntry, StaFifo.field.TxSuccess);
#endif // UAPSD_AP_SUPPORT //


			if (!StaFifo.field.TxSuccess)
			{
				pEntry->FIFOCount++;
				pEntry->OneSecTxFailCount++;
									
				if (pEntry->FIFOCount >= 1)
				{			
					DBGPRINT(RT_DEBUG_TRACE, ("#"));
#ifdef DOT11_N_SUPPORT
					pEntry->NoBADataCountDown = 64;
#endif // DOT11_N_SUPPORT //

//#ifdef CONFIG_STA_SUPPORT
//#ifdef DOT11Z_TDLS_SUPPORT
//					IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
//					{
//						if(IS_ENTRY_TDLS(pEntry))
//							TDLS_LinkTearDown(pAd);
//					}
//#endif // DOT11Z_TDLS_SUPPORT //
//#endif // CONFIG_STA_SUPPORT //

					// Update the continuous transmission counter.
					pEntry->ContinueTxFailCnt++;

					if(pEntry->PsMode == PWR_ACTIVE)
					{
#ifdef DOT11_N_SUPPORT					
						int tid;
						for (tid=0; tid<NUM_OF_TID; tid++)
						{
							BAOriSessionTearDown(pAd, pEntry->Aid,  tid, FALSE, FALSE);
						}
#endif // DOT11_N_SUPPORT //


#ifdef WDS_SUPPORT
						// fix WDS Jam issue
						if(IS_ENTRY_WDS(pEntry)
							&& (pEntry->LockEntryTx == FALSE)
							&& (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
						{ 
							DBGPRINT(RT_DEBUG_TRACE, ("Entry %02x:%02x:%02x:%02x:%02x:%02x Blocked!! (Fail Cnt = %d)\n",
								pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
								pEntry->Addr[4],pEntry->Addr[5],pEntry->ContinueTxFailCnt ));

							pEntry->LockEntryTx = TRUE;
						}
#endif // WDS_SUPPORT //
					}

					//pEntry->FIFOCount = 0;
				}
				//pEntry->bSendBAR = TRUE;
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
			// if Tx fail >= 20, then clear TXWI ack in Tx Ring
			if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
				ClearTxRingClientAck(pAd, pEntry);	
#endif // RTMP_MAC_PCI //				
#endif // CONFIG_AP_SUPPORT //
			}
			else
			{
#ifdef DOT11_N_SUPPORT
				if ((pEntry->PsMode != PWR_SAVE) && (pEntry->NoBADataCountDown > 0))
				{
					pEntry->NoBADataCountDown--;
					if (pEntry->NoBADataCountDown==0)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("@\n"));
					}
				}
#endif // DOT11_N_SUPPORT //
				pEntry->FIFOCount = 0;
				pEntry->OneSecTxNoRetryOkCount++;
				// update NoDataIdleCount when sucessful send packet to STA.
				pEntry->NoDataIdleCount = 0;
				pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
				pEntry->LockEntryTx = FALSE;
#endif // WDS_SUPPORT //
			}

			succMCS = StaFifo.field.SuccessRate & 0x7F;

#if defined(RT2883) || defined(RT3883)
			if (pEntry->HTCapability.MCSSet[2] == 0xff)
			{
				if (succMCS > pid)
					pid = pid + 16;
			}
#endif // defined(RT2883) || defined(RT3883) //

			reTry = pid - succMCS;

			if (StaFifo.field.TxSuccess)
			{
				pEntry->TXMCSExpected[pid]++;
				if (pid == succMCS)
				{
					pEntry->TXMCSSuccessful[pid]++;
				}
				else 
				{
					pEntry->TXMCSAutoFallBack[pid][succMCS]++;
				}
			}
			else
			{
				pEntry->TXMCSFailed[pid]++;
			}


#if defined(RT2883) || defined(RT3883)
			if (pEntry->HTCapability.MCSSet[2] == 0xff)
				reTry = (pid % 3) - (succMCS % 3) + (pid >> 3) - (succMCS >> 3);
			else
			{
#endif // defined(RT2883) || defined(RT3883) //
			if (reTry > 0)
			{
#ifndef NEW_RATE_ADAPT_SUPPORT
				if ((pid >= 12) && succMCS <=7)
				{
					reTry -= 4;
				} 
#endif
				pEntry->OneSecTxRetryOkCount += reTry;
			}
#if defined(RT2883) || defined(RT3883)
			}
#endif // defined(RT2883) || defined(RT3883) //

			i++;
			// ASIC store 16 stack
		} while ( i < (TX_RING_SIZE<<1) );

}

/*
	========================================================================
	
	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID NICUpdateRawCounters(
	IN PRTMP_ADAPTER pAd)
{
	UINT32	OldValue;//, Value2;
	//ULONG	PageSum, OneSecTransmitCount;
	//ULONG	TxErrorRatio, Retry, Fail;
	RX_STA_CNT0_STRUC	 RxStaCnt0;
	RX_STA_CNT1_STRUC   RxStaCnt1;
	RX_STA_CNT2_STRUC   RxStaCnt2;
	TX_STA_CNT0_STRUC 	 TxStaCnt0;
	TX_STA_CNT1_STRUC	 StaTx1;
	TX_STA_CNT2_STRUC	 StaTx2;
#ifdef STATS_COUNT_SUPPORT
	TX_AGG_CNT_STRUC	TxAggCnt;
	TX_AGG_CNT0_STRUC	TxAggCnt0;
	TX_AGG_CNT1_STRUC	TxAggCnt1;
	TX_AGG_CNT2_STRUC	TxAggCnt2;
	TX_AGG_CNT3_STRUC	TxAggCnt3;
	TX_AGG_CNT4_STRUC	TxAggCnt4;
	TX_AGG_CNT5_STRUC	TxAggCnt5;
	TX_AGG_CNT6_STRUC	TxAggCnt6;
	TX_AGG_CNT7_STRUC	TxAggCnt7;
#endif // STATS_COUNT_SUPPORT //
	COUNTER_RALINK		*pRalinkCounters;


	pRalinkCounters = &pAd->RalinkCounters;

	RTMP_IO_READ32(pAd, RX_STA_CNT0, &RxStaCnt0.word);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &RxStaCnt2.word);

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	if ((pAd->CommonCfg.CarrierDetect.Enable == FALSE) || (pAd->OpMode == OPMODE_STA))
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
	{
		RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);
	    // Update RX PLCP error counter
	    pAd->PrivateInfo.PhyRxErrCnt += RxStaCnt1.field.PlcpErr;
		// Update False CCA counter
		pAd->RalinkCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
	}

#ifdef STATS_COUNT_SUPPORT
	// Update FCS counters
	OldValue= pAd->WlanCounters.FCSErrorCount.u.LowPart;
	pAd->WlanCounters.FCSErrorCount.u.LowPart += (RxStaCnt0.field.CrcErr); // >> 7);
	if (pAd->WlanCounters.FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters.FCSErrorCount.u.HighPart++;
#endif // STATS_COUNT_SUPPORT //

	// Add FCS error count to private counters
	pRalinkCounters->OneSecRxFcsErrCnt += RxStaCnt0.field.CrcErr;
	OldValue = pRalinkCounters->RealFcsErrCount.u.LowPart;
	pRalinkCounters->RealFcsErrCount.u.LowPart += RxStaCnt0.field.CrcErr;
	if (pRalinkCounters->RealFcsErrCount.u.LowPart < OldValue)
		pRalinkCounters->RealFcsErrCount.u.HighPart++;

	// Update Duplicate Rcv check
	pRalinkCounters->DuplicateRcv += RxStaCnt2.field.RxDupliCount;
#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart += RxStaCnt2.field.RxDupliCount;
#endif // STATS_COUNT_SUPPORT //
	// Update RX Overflow counter
	pAd->Counters8023.RxNoBuffer += (RxStaCnt2.field.RxFifoOverflowCount);
	
	//pAd->RalinkCounters.RxCount = 0;

	
	//if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) || 
	//	(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) && (pAd->MacTab.Size != 1)))
	if (!pAd->bUpdateBcnCntDone)
	{
	// Update BEACON sent count
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &TxStaCnt0.word);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &StaTx1.word);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &StaTx2.word);
	pRalinkCounters->OneSecBeaconSentCnt += TxStaCnt0.field.TxBeaconCount;
	pRalinkCounters->OneSecTxRetryOkCount += StaTx1.field.TxRetransmit;
	pRalinkCounters->OneSecTxNoRetryOkCount += StaTx1.field.TxSuccess;
	pRalinkCounters->OneSecTxFailCount += TxStaCnt0.field.TxFailCount;

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += StaTx1.field.TxSuccess;
	pAd->WlanCounters.RetryCount.u.LowPart += StaTx1.field.TxRetransmit;
	pAd->WlanCounters.FailedCount.u.LowPart += TxStaCnt0.field.TxFailCount;
#endif // STATS_COUNT_SUPPORT //
	}


	//if (pAd->bStaFifoTest == TRUE)
#ifdef STATS_COUNT_SUPPORT
	{
		RTMP_IO_READ32(pAd, TX_AGG_CNT, &TxAggCnt.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT0, &TxAggCnt0.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT1, &TxAggCnt1.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT2, &TxAggCnt2.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT3, &TxAggCnt3.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT4, &TxAggCnt4.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT5, &TxAggCnt5.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT6, &TxAggCnt6.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT7, &TxAggCnt7.word);
		pRalinkCounters->TxAggCount += TxAggCnt.field.AggTxCount;
		pRalinkCounters->TxNonAggCount += TxAggCnt.field.NonAggTxCount;
		pRalinkCounters->TxAgg1MPDUCount += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TxAgg2MPDUCount += TxAggCnt0.field.AggSize2Count;
		
		pRalinkCounters->TxAgg3MPDUCount += TxAggCnt1.field.AggSize3Count;
		pRalinkCounters->TxAgg4MPDUCount += TxAggCnt1.field.AggSize4Count;
		pRalinkCounters->TxAgg5MPDUCount += TxAggCnt2.field.AggSize5Count;
		pRalinkCounters->TxAgg6MPDUCount += TxAggCnt2.field.AggSize6Count;
	
		pRalinkCounters->TxAgg7MPDUCount += TxAggCnt3.field.AggSize7Count;
		pRalinkCounters->TxAgg8MPDUCount += TxAggCnt3.field.AggSize8Count;
		pRalinkCounters->TxAgg9MPDUCount += TxAggCnt4.field.AggSize9Count;
		pRalinkCounters->TxAgg10MPDUCount += TxAggCnt4.field.AggSize10Count;

		pRalinkCounters->TxAgg11MPDUCount += TxAggCnt5.field.AggSize11Count;
		pRalinkCounters->TxAgg12MPDUCount += TxAggCnt5.field.AggSize12Count;
		pRalinkCounters->TxAgg13MPDUCount += TxAggCnt6.field.AggSize13Count;
		pRalinkCounters->TxAgg14MPDUCount += TxAggCnt6.field.AggSize14Count;

		pRalinkCounters->TxAgg15MPDUCount += TxAggCnt7.field.AggSize15Count;
		pRalinkCounters->TxAgg16MPDUCount += TxAggCnt7.field.AggSize16Count;

		// Calculate the transmitted A-MPDU count
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt0.field.AggSize2Count >> 1);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize3Count / 3);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize4Count >> 2);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize5Count / 5);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize6Count / 6);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize7Count / 7);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize8Count >> 3);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize9Count / 9);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize10Count / 10);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize11Count / 11);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize12Count / 12);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize13Count / 13);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize14Count / 14);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize15Count / 15);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize16Count >> 4);	
	}
#endif // STATS_COUNT_SUPPORT //			

#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct	*pDiag;
		UCHAR			ArrayCurIdx, i;
		
		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;
		
		if (pDiag->inited == 0)
		{
			NdisZeroMemory(pDiag, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->inited = 1;
		}
		else
		{
			// Tx
			pDiag->TxFailCnt[ArrayCurIdx] = TxStaCnt0.field.TxFailCount;
			pDiag->TxAggCnt[ArrayCurIdx] = TxAggCnt.field.AggTxCount;
			pDiag->TxNonAggCnt[ArrayCurIdx] = TxAggCnt.field.NonAggTxCount;

			pDiag->TxAMPDUCnt[ArrayCurIdx][0] = TxAggCnt0.field.AggSize1Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][1] = TxAggCnt0.field.AggSize2Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][2] = TxAggCnt1.field.AggSize3Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][3] = TxAggCnt1.field.AggSize4Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][4] = TxAggCnt2.field.AggSize5Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][5] = TxAggCnt2.field.AggSize6Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][6] = TxAggCnt3.field.AggSize7Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][7] = TxAggCnt3.field.AggSize8Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][8] = TxAggCnt4.field.AggSize9Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][9] = TxAggCnt4.field.AggSize10Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][10] = TxAggCnt5.field.AggSize11Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][11] = TxAggCnt5.field.AggSize12Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][12] = TxAggCnt6.field.AggSize13Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][13] = TxAggCnt6.field.AggSize14Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][14] = TxAggCnt7.field.AggSize15Count;
			pDiag->TxAMPDUCnt[ArrayCurIdx][15] = TxAggCnt7.field.AggSize16Count;

			pDiag->RxCrcErrCnt[ArrayCurIdx] = RxStaCnt0.field.CrcErr;
			
			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;
			for (i =0; i < 9; i++)
			{
				pDiag->TxDescCnt[ArrayCurIdx][i]= 0;
				pDiag->TxSWQueCnt[ArrayCurIdx][i] =0;
				pDiag->TxMcsCnt[ArrayCurIdx][i] = 0;
				pDiag->RxMcsCnt[ArrayCurIdx][i] = 0;
			}
			pDiag->TxDataCnt[ArrayCurIdx] = 0;
			pDiag->TxFailCnt[ArrayCurIdx] = 0;
			pDiag->RxDataCnt[ArrayCurIdx] = 0;
			pDiag->RxCrcErrCnt[ArrayCurIdx]  = 0;
//			for (i = 9; i < 16; i++)
			for (i = 9; i < 24; i++) // 3*3
			{
				pDiag->TxDescCnt[ArrayCurIdx][i] = 0;
				pDiag->TxMcsCnt[ArrayCurIdx][i] = 0;
				pDiag->RxMcsCnt[ArrayCurIdx][i] = 0;
}

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
		}
		
	}
#endif // DBG_DIAGNOSE //


}

NDIS_STATUS NICLoadFirmware(
	IN PRTMP_ADAPTER pAd)
{
	/*
	1. For PCI:
        (1) Write SYS_CTRL bit16(HST_PM_SEL) to 1
        (2) Write 8051 firmware to RAM.
        (3) Write SYS_CTRL to 0.
        (4) Write SYS_CTRL bit0(MCU_RESET) to 1 to do MCU HW reset.

	2. For USB:
        (1)Get current firmware operation mode via VendorRequest(0x1, 0x11) command.
        (2) Write SYS_CTRL bit7(MCU_READY) to 0.
        (3) Write SYS_CTRL bit0(MCU_RESET) to 1 to do MCU reset to run 8051 on ROM.
        (4) Check MCU ready via SYS_CTRL bit7(MCU_READY).
        (5) Write 8051 firmware to RAM.
        (6) Write MAC 0x7014 to 0xffffffff.
        (7) Write MAC 0x701c to 0xffffffff.
        (8) Change 8051 from ROM to RAM site via VendorRequest(0x01, 0x8) command.
	*/

	NDIS_STATUS	 status = NDIS_STATUS_SUCCESS;
	if (pAd->chipOps.loadFirmware)
		status = pAd->chipOps.loadFirmware(pAd);

	return status;
}


/*
	========================================================================
	
	Routine Description:
		erase 8051 firmware image in MAC ASIC

	Arguments:
		Adapter						Pointer to our adapter

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/
VOID NICEraseFirmware(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->chipOps.eraseFirmware)
		pAd->chipOps.eraseFirmware(pAd);
	
}/* End of NICEraseFirmware */


/*
	========================================================================
	
	Routine Description:
		Load Tx rate switching parameters

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS         firmware image load ok
		NDIS_STATUS_FAILURE         image not found

	IRQL = PASSIVE_LEVEL

	Rate Table Format:
		1. (B0: Valid Item number) (B1:Initial item from zero)
		2. Item Number(Dec)      Mode(Hex)     Current MCS(Dec)    TrainUp(Dec)    TrainDown(Dec)
		
	========================================================================
*/
NDIS_STATUS NICLoadRateSwitchingParams(
	IN PRTMP_ADAPTER pAd)
{
	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================
	
	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address
		
	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
ULONG	RTMPCompareMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	// Equal
	return (0);
}


/*
	========================================================================
	
	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None
		
	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPZeroMemory(
	IN	PVOID	pSrc,
	IN	ULONG	Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = 0x00;
	}
}


/*
	========================================================================
	
	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size
		
	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID RTMPMoveMemory(
	OUT	PVOID	pDest,
	IN	PVOID	pSrc,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length==0) || (pDest && pSrc));

	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem1[Index] = pMem2[Index];
	}
}

VOID UserCfgExit(
	IN RTMP_ADAPTER *pAd)
{
#ifdef DOT11_N_SUPPORT
	BATableExit(pAd);
#endif // DOT11_N_SUPPORT //

	NdisFreeSpinLock(&pAd->MacTabLock);
}

/*
	========================================================================
	
	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID	UserCfgInit(
	IN	PRTMP_ADAPTER pAd)
{
	UINT i;
#ifdef CONFIG_AP_SUPPORT
	UINT j;
#endif // CONFIG_AP_SUPPORT //
//	EDCA_PARM DefaultEdcaParm;
    UINT key_index, bss_index;


	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit\n"));    
	 
	//
	//  part I. intialize common configuration
	//

	for(key_index=0; key_index<SHARE_KEY_NUM; key_index++)
	{
		for(bss_index = 0; bss_index < MAX_MBSSID_NUM; bss_index++)
		{
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->bLocalAdminMAC = FALSE;
	pAd->EepromAccess = FALSE;
	
	pAd->Antenna.word = 0; 
	pAd->CommonCfg.BBPCurrentBW = BW_20;

	pAd->LedCntl.word = 0;
#ifdef RTMP_MAC_PCI
	pAd->LedIndicatorStrength = 0;
#endif // RTMP_MAC_PCI //

	pAd->bAutoTxAgcA = FALSE;			// Default is OFF
	pAd->bAutoTxAgcG = FALSE;			// Default is OFF
#ifdef RTMP_INTERNAL_TX_ALC
	pAd->TxPowerCtrl.bInternalTxALC = FALSE; // Off by default
#endif // RTMP_INTERNAL_TX_ALC //
	pAd->RfIcType = RFIC_2820;

	// Init timer for reset complete event
	pAd->CommonCfg.CentralChannel = 1;
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      // in units of usec 
	pAd->CommonCfg.TxPower = 100; //mW
	pAd->CommonCfg.TxPowerPercentage = 0xffffffff; // AUTO
	pAd->CommonCfg.TxPowerDefault = 0xffffffff; // AUTO
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; // use Long preamble on TX by defaut
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->CommonCfg.RtsThreshold = 2347;
	pAd->CommonCfg.FragmentThreshold = 2346;
	pAd->CommonCfg.UseBGProtection = 0;    // 0: AUTO
	pAd->CommonCfg.bEnableTxBurst = TRUE; //0;    	
	pAd->CommonCfg.PhyMode = 0xff;     // unknown
	pAd->CommonCfg.BandState = UNKNOWN_BAND;
	pAd->CommonCfg.RadarDetect.CSPeriod = 10;
	pAd->CommonCfg.RadarDetect.CSCount = 0;
	pAd->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
	
#ifdef RT3052
	RTMP_SYS_IO_READ32(0xb000000c, &pAd->CommonCfg.CID);
	RTMP_SYS_IO_READ32(0xb0000000, &pAd->CommonCfg.CN);
#endif // RT3052 //

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
	pAd->CommonCfg.carrier_func = DISABLE_TONE_RADAR;

#ifdef TONE_RADAR_DETECT_SUPPORT

#ifdef TONE_RADAR_DETECT_V1
	pAd->CommonCfg.carrier_func = TONE_RADAR_V1;
#endif //TONE_RADAR_DETECT_V1 //

#ifdef TONE_RADAR_DETECT_V2
	pAd->CommonCfg.carrier_func = TONE_RADAR_V2;
#endif // TONE_RADAR_DETECT_V2 //

	pAd->CommonCfg.CarrierDetect.delta = CARRIER_DETECT_DELTA;
	pAd->CommonCfg.CarrierDetect.div_flag = CARRIER_DETECT_DIV_FLAG;
	pAd->CommonCfg.CarrierDetect.criteria = CARRIER_DETECT_CRITIRIA;
	pAd->CommonCfg.CarrierDetect.threshold = CARRIER_DETECT_THRESHOLD;
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //

	pAd->CommonCfg.RadarDetect.ChMovingTime = 65;
	pAd->CommonCfg.RadarDetect.LongPulseRadarTh = 3;
	pAd->CommonCfg.bAPSDCapable = FALSE;
	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	pAd->CommonCfg.bCountryFlag = FALSE;
	pAd->CommonCfg.TxStream = 0;
	pAd->CommonCfg.RxStream = 0;

	NdisZeroMemory(&pAd->BeaconTxWI, sizeof(pAd->BeaconTxWI));

#ifdef DOT11_N_SUPPORT
	NdisZeroMemory(&pAd->CommonCfg.HtCapability, sizeof(pAd->CommonCfg.HtCapability));
	pAd->HTCEnable = FALSE;
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;
	
#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	// Unit : TU. 5~1000
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	// Unit : TU. 10~1000
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	// Unit : Second	
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	// Unit : TU. 200~10000
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	// Unit : TU. 20~10000
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	// Unit : percentage
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);

	pAd->CommonCfg.bBssCoexEnable = TRUE; // by default, we enable this feature, you can disable it via the profile or ioctl command
	pAd->CommonCfg.BssCoexApCntThr = 0; 
#endif  // DOT11N_DRAFT3 //

	NdisZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_ENABLE;
	pAd->CommonCfg.BACapability.field.MpduDensity = 0;
	pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64; //32;
	pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64; //32;
	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit. BACapability = 0x%x\n", pAd->CommonCfg.BACapability.word));    

	pAd->CommonCfg.BACapability.field.AutoBA = FALSE;	
	BATableInit(pAd, &pAd->BATable);

	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bHTProtect = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
#ifdef GREENAP_SUPPORT
	pAd->ApCfg.bGreenAPEnable=FALSE;
	pAd->ApCfg.bGreenAPActive = FALSE;
	pAd->ApCfg.GreenAPLevel= GREENAP_WITHOUT_ANY_STAS_CONNECT;
#endif // GREENAP_SUPPORT //
	pAd->CommonCfg.bBADecline = FALSE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	if (pAd->MACVersion == 0x28720200)
	{
		pAd->CommonCfg.TxBASize = 13; //by Jerry recommend
	}else{
		pAd->CommonCfg.TxBASize = 7;
	}

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
#endif // DOT11_N_SUPPORT //

	//pAd->CommonCfg.HTPhyMode.field.BW = BW_20;
	//pAd->CommonCfg.HTPhyMode.field.MCS = MCS_AUTO;
	//pAd->CommonCfg.HTPhyMode.field.ShortGI = GI_800;
	//pAd->CommonCfg.HTPhyMode.field.STBC = STBC_NONE;
	pAd->CommonCfg.TxRate = RATE_6;
	
	pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
	pAd->CommonCfg.MlmeTransmit.field.BW = BW_20;
	pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;

	pAd->CommonCfg.BeaconPeriod = 100;     // in mSec


#ifdef WAPI_SUPPORT
	pAd->CommonCfg.wapi_usk_rekey_method = REKEY_METHOD_DISABLE;
	pAd->CommonCfg.wapi_msk_rekey_method = REKEY_METHOD_DISABLE;
	pAd->CommonCfg.wapi_msk_rekey_cnt = 0;
#endif // WAPI_SUPPORT //

#ifdef RT3883
	pAd->CommonCfg.VCORecalibrationThreshold = DEFAULT_VCO_RECALIBRATION_THRESHOLD;
	pAd->RefreshTssi = 1;
#endif // RT3883 //

#ifdef MCAST_RATE_SPECIFIC
	pAd->CommonCfg.MCastPhyMode.word
	= pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word;
#endif // MCAST_RATE_SPECIFIC //

	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = TRUE;

	//
	// part II. intialize STA specific configuration
	//

	// global variables mXXXX used in MAC protocol state machines
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);

	// PHY specification
	pAd->CommonCfg.PhyMode = PHY_11BG_MIXED;		// default PHY mode
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  // CCK use LONG preamble


	// Default for extra information is not valid
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;
	
	// Default Config change flag
	pAd->bConfigChanged = FALSE;

	// 
	// part III. AP configurations
	//
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		// Set MBSS Default Configurations
		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM;
		for(j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		{
			pAd->ApCfg.MBSSID[j].AuthMode = Ndis802_11AuthModeOpen;
			pAd->ApCfg.MBSSID[j].WepStatus = Ndis802_11EncryptionDisabled;
			pAd->ApCfg.MBSSID[j].GroupKeyWepStatus = Ndis802_11EncryptionDisabled;
			pAd->ApCfg.MBSSID[j].DefaultKeyId = 0;
			pAd->ApCfg.MBSSID[j].WpaMixPairCipher = MIX_CIPHER_NOTUSE;

#ifdef DOT1X_SUPPORT
			pAd->ApCfg.MBSSID[j].IEEE8021X = FALSE;
			pAd->ApCfg.MBSSID[j].PreAuth = FALSE;

			// PMK cache setting
			pAd->ApCfg.MBSSID[j].PMKCachePeriod = (10 * 60 * OS_HZ); // unit : tick(default: 10 minute)
			NdisZeroMemory(&pAd->ApCfg.MBSSID[j].PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));

			/* dot1x related per BSS */
			pAd->ApCfg.MBSSID[j].radius_srv_num = 0;
			pAd->ApCfg.MBSSID[j].NasIdLen = 0;
#endif // DOT1X_SUPPORT //

			/* VLAN related */
        		pAd->ApCfg.MBSSID[j].VLAN_VID = 0;

			// Default MCS as AUTO
			pAd->ApCfg.MBSSID[j].bAutoTxRateSwitch = TRUE;
			pAd->ApCfg.MBSSID[j].DesiredTransmitSetting.field.MCS = MCS_AUTO;

			// Default is zero. It means no limit.
			pAd->ApCfg.MBSSID[j].MaxStaNum = 0;
			pAd->ApCfg.MBSSID[j].StaCount = 0;
			
#ifdef WSC_AP_SUPPORT
			{
				PWSC_CTRL pWscControl;
				INT idx;
				/*
					WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize 
					and MLME Initialize is called before UserCfgInit.

				*/
				pWscControl = &pAd->ApCfg.MBSSID[j].WscControl;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				pWscControl->WscMode = 1;
				pWscControl->WscConfStatus = 1;
				pWscControl->WscConfigMethods= 0x0084;
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				pWscControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | j);
				pWscControl->pAd = pAd;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pWscControl->WscConfMode = 0;
				pWscControl->WscStatus = 0;
				pWscControl->WscState = 0;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				pWscControl->WscUseUPnP = 0;
				pWscControl->bWCNTest = FALSE;
				pWscControl->WscKeyASCII = 0; /* default, 0 (64 Hex) */
				
				/*
					Enrollee 192 random bytes for DH key generation
				*/
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

				// Enrollee Nonce, first generate and save to Wsc Control Block
				for (idx = 0; idx < 16; idx++)
				{
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);
				}
				NdisZeroMemory(&pWscControl->WscDefaultSsid, sizeof(NDIS_802_11_SSID));
				NdisZeroMemory(&pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
				NdisZeroMemory(&pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);

			}
#endif // WSC_AP_SUPPORT //        

			for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
	        		pAd->ApCfg.MBSSID[j].TimBitmaps[i] = 0;
		}
		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;

		pAd->ApCfg.ErpIeContent = 0;

		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;

#ifdef IDS_SUPPORT
		// Default disable IDS threshold and reset all IDS counters
		pAd->ApCfg.IdsEnable = FALSE;
		pAd->ApCfg.AuthFloodThreshold = 0;
		pAd->ApCfg.AssocReqFloodThreshold = 0;
		pAd->ApCfg.ReassocReqFloodThreshold = 0;
		pAd->ApCfg.ProbeReqFloodThreshold = 0;
		pAd->ApCfg.DisassocFloodThreshold = 0;
		pAd->ApCfg.DeauthFloodThreshold = 0;
		pAd->ApCfg.EapReqFloodThreshold = 0;
		RTMPClearAllIdsCounter(pAd);
#endif // IDS_SUPPORT //

#ifdef WDS_SUPPORT
		APWdsInitialize(pAd);
#endif // WDS_SUPPORT

#ifdef WSC_AP_SUPPORT
		pAd->WriteWscCfgToDatFile = 0xFF;
		pAd->WriteWscCfgToAr9DatFile = FALSE;
#endif // WSC_AP_SUPPORT //

#ifdef APCLI_SUPPORT
		for(j = 0; j < MAX_APCLI_NUM; j++) 
		{
			pAd->ApCfg.ApCliTab[j].bAutoTxRateSwitch = TRUE;
			pAd->ApCfg.ApCliTab[j].DesiredTransmitSetting.field.MCS = MCS_AUTO;
		}
#endif // APCLI_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //


	//
	// part IV. others
	//
	// dynamic BBP R66:sensibity tuning to overcome background noise
	pAd->BbpTuning.bEnable                = TRUE;  
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta               = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;
	
	//
	// Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.
	// if not initial this value, the default value will be 0.
	//
	pAd->BbpTuning.R66CurrentValue = 0x38;

	pAd->Bbp94 = BBPR94_DEFAULT;
	pAd->BbpForCCK = FALSE;
	
	// Default is FALSE for test bit 1
	//pAd->bTest1 = FALSE;
	
	// initialize MAC table and allocate spin lock
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	NdisAllocateSpinLock(&pAd->MacTabLock);

	//RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, TRUE);
	//RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);

#ifdef RALINK_ATE
	NdisZeroMemory(&pAd->ate, sizeof(ATE_INFO));
	pAd->ate.Mode = ATE_STOP;
#ifdef RT3350
	pAd->ate.PABias = 0;
#endif // RT3350  //
	pAd->ate.TxCount = 200;/* to exceed TX_RING_SIZE ... */
	pAd->ate.TxDoneCount = 0;
	pAd->ate.RFFreqOffset = 0;
	pAd->ate.Payload = 0xA5;/* to be backward compatible */	
	pAd->ate.IPG = 0;/* 0 : no IPG by default */	
	pAd->ate.TxLength = 1024;
	pAd->ate.TxWI.ShortGI = 0;// LONG GI : 800 ns
	pAd->ate.TxWI.PHYMODE = MODE_CCK;
	pAd->ate.TxWI.MCS = 3;
	pAd->ate.TxWI.BW = BW_20;
	/* please do not change this default channel value */
	pAd->ate.Channel = 1;

#ifdef RTMP_MAC_PCI 
#endif // RTMP_MAC_PCI //
	pAd->ate.QID = QID_AC_BE;

#if defined (RT2883) || defined (RT3883)
	/* For stream mode in 3T/3R ++ */
	/* use broadcast address as default value */
	pAd->ate.Addr1[0] = 0xFF;
	pAd->ate.Addr1[1] = 0xFF;
	pAd->ate.Addr1[2] = 0xFF;
	pAd->ate.Addr1[3] = 0xFF;
	pAd->ate.Addr1[4] = 0xFF;
	pAd->ate.Addr1[5] = 0xFF;

	pAd->ate.Addr2[0] = 0x00;
	pAd->ate.Addr2[1] = 0x11;
	pAd->ate.Addr2[2] = 0x22;
	pAd->ate.Addr2[3] = 0xAA;
	pAd->ate.Addr2[4] = 0xBB;
	pAd->ate.Addr2[5] = 0xCC;

	NdisMoveMemory(pAd->ate.Addr3, pAd->ate.Addr2, ETH_LENGTH_OF_ADDRESS);

	{		
		UINT32 data;

		data = 0xFFFFFFFF;
    	RTMP_IO_WRITE32(pAd, 0x1044, data); 
    	RTMP_IO_READ32(pAd, 0x1048, &data); 

    	data = data | 0x0000FFFF;
    	RTMP_IO_WRITE32(pAd, 0x1048, data); 
	}
	/* For stream mode in 3T/3R -- */
#else
	pAd->ate.Addr1[0] = 0x00;
	pAd->ate.Addr1[1] = 0x11;
	pAd->ate.Addr1[2] = 0x22;
	pAd->ate.Addr1[3] = 0xAA;
	pAd->ate.Addr1[4] = 0xBB;
	pAd->ate.Addr1[5] = 0xCC;

	NdisMoveMemory(pAd->ate.Addr2, pAd->ate.Addr1, ETH_LENGTH_OF_ADDRESS);
	NdisMoveMemory(pAd->ate.Addr3, pAd->ate.Addr1, ETH_LENGTH_OF_ADDRESS);
#endif // defined (RT2883) || defined (RT3883) //

	pAd->ate.bRxFER = 0;
	pAd->ate.bQATxStart = FALSE;
	pAd->ate.bQARxStart = FALSE;
	pAd->ate.bAutoTxAlc = FALSE;
#ifdef RTMP_MAC_PCI 
	pAd->ate.bFWLoading = FALSE;
#endif // RTMP_MAC_PCI //


#ifdef RALINK_QA
	pAd->ate.TxStatus = 0;
	pAd->ate.AtePid = THREAD_PID_INIT_VALUE;
#endif // RALINK_QA //
#endif // RALINK_ATE //


	pAd->CommonCfg.bWiFiTest = FALSE;
#ifdef RTMP_MAC_PCI
    pAd->bPCIclkOff = FALSE;
#endif // RTMP_MAC_PCI //

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

#ifdef DOT11R_FT_SUPPORT
	FT_CfgInitial(pAd);
#endif // DOT11R_FT_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef ANT_DIVERSITY_SUPPORT
		if ( pAd->CommonCfg.bRxAntDiversity == ANT_FIX_ANT2)
		{
			pAd->RxAnt.Pair1PrimaryRxAnt = 1;
			pAd->RxAnt.Pair1SecondaryRxAnt = 0;
		}
		else // Default
		{
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
		pAd->RxAnt.EvaluatePeriod = 0;
		pAd->RxAnt.RcvPktNumWhenEvaluate = 0;
#ifdef CONFIG_AP_SUPPORT
		pAd->RxAnt.Pair1AvgRssiGroup1[0] = pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
		pAd->RxAnt.Pair1AvgRssiGroup2[0] = pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
#endif // CONFIG_AP_SUPPORT //
#endif // ANT_DIVERSITY_SUPPORT //


#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
	{
		PBSS_ENTRY	pBssEntry = &pAd->ScanTab.BssEntry[i];
		
		if (pAd->ProbeRespIE[i].pIe)
			pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
		else
			pBssEntry->pVarIeFromProbRsp = NULL;
	}
#endif // defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) //

#ifdef RT3593
	//
	// NIC uses the maximum Tx capability (3 Tx streams) to transmit any broadcast packets.
	//
	if (IS_RT3593(pAd))
	{
		TX_CHAIN_ADDR0_L_STRUC TxChainAddr0L = { { 0 } };
		TX_CHAIN_ADDR0_H_STRUC TxChainAddr0H = { { 0 } };

		TxChainAddr0L.field.TxChainAddr0L = 0xFFFFFFFF;
		RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_L, TxChainAddr0L.word);
		
		RTMP_IO_READ32(pAd, TX_CHAIN_ADDR0_H, &TxChainAddr0H.word);
		TxChainAddr0H.field.TxChainAddr0H = 0xFFFF;
		TxChainAddr0H.field.TxChainSel0 = 0xF;
		RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_H, TxChainAddr0H.word);
	}
#endif // RT3593 //

#ifdef HW_COEXISTENCE_SUPPORT
	pAd->bHWCoexistenceInit = FALSE;
	pAd->bWiMaxCoexistenceOn = FALSE;
#endif // HW_COEXISTENCE_SUPPORT //
#ifdef BT_COEXISTENCE_SUPPORT
	BtCoexistUserCfgInit(pAd);
#endif // BT_COEXISTENCE_SUPPORT //

#ifdef WSC_INCLUDED
	NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
	pAd->CommonCfg.WscPBCOverlap = FALSE;
#endif // WSC_INCLUDED //

	DBGPRINT(RT_DEBUG_TRACE, ("<-- UserCfgInit\n"));
}

// IRQL = PASSIVE_LEVEL
UCHAR BtoH(STRING ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        // Handle numerals
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  // Handle capitol hex digits
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  // Handle small hex digits
	return(255);
}

//
//  FUNCTION: AtoH(char *, UCHAR *, int)
//
//  PURPOSE:  Converts ascii string to network order hex
//
//  PARAMETERS:
//    src    - pointer to input ascii string
//    dest   - pointer to output hex
//    destlen - size of dest
//
//  COMMENTS:
//
//    2 ascii bytes make a hex byte so must put 1st ascii byte of pair
//    into upper nibble and 2nd ascii byte of pair into lower nibble.
//
// IRQL = PASSIVE_LEVEL

void AtoH(PSTRING src, PUCHAR dest, int destlen)
{
	PSTRING srcptr;
	PUCHAR destTemp;

	srcptr = src;	
	destTemp = (PUCHAR) dest; 

	while(destlen--)
	{
		*destTemp = BtoH(*srcptr++) << 4;    // Put 1st ascii byte in upper nibble.
		*destTemp += BtoH(*srcptr++);      // Add 2nd ascii byte to above.
		destTemp++;
	}
}


//+++Mark by shiang, not use now, need to remove after confirm
//---Mark by shiang, not use now, need to remove after confirm


/*
	========================================================================
	
	Routine Description:
		Init timer objects

	Arguments:
		pAd			Pointer to our adapter
		pTimer				Timer structure
		pTimerFunc			Function to execute when timer expired
		Repeat				Ture for period timer

	Return Value:
		None

	Note:
		
	========================================================================
*/
VOID	RTMPInitTimer(
	IN	PRTMP_ADAPTER			pAd,
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	PVOID					pTimerFunc,
	IN	PVOID					pData,
	IN	BOOLEAN					Repeat)
{
	//
	// Set Valid to TRUE for later used.
	// It will crash if we cancel a timer or set a timer 
	// that we haven't initialize before.
	// 
	pTimer->Valid      = TRUE;
	
	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = pAd;

	RTMP_OS_Init_Timer(pAd, &pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer);	
}


/*
	========================================================================
	
	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.
		
	========================================================================
*/
VOID	RTMPSetTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value)
{
	if (pTimer->Valid)
	{
		RTMP_ADAPTER *pAd;
		
		pAd = (RTMP_ADAPTER *)pTimer->pAd;
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			DBGPRINT_ERR(("RTMPSetTimer failed, Halt in Progress!\n"));
			return;
		}
		
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		}
		else
		{
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}
	}
	else
	{
		DBGPRINT_ERR(("RTMPSetTimer failed, Timer hasn't been initialize!\n"));
	}
}


/*
	========================================================================
	
	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.
		
	========================================================================
*/
VOID	RTMPModTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value)
{
	BOOLEAN	Cancel;

	if (pTimer->Valid)
	{
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		}
		else
		{
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
		}
	}
	else
	{
		DBGPRINT_ERR(("RTMPModTimer failed, Timer hasn't been initialize!\n"));
	}
}


/*
	========================================================================
	
	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.
		
	========================================================================
*/
VOID	RTMPCancelTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	OUT	BOOLEAN					*pCancelled)
{
	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;
		
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		
		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		// We need to go-through the TimerQ to findout this timer handler and remove it if 
		//		it's still waiting for execution.
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif // RTMP_TIMER_TASK_SUPPORT //
	}
	else
	{
		DBGPRINT_ERR(("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
	}
}

#ifdef LED_CONTROL_SUPPORT
/*
	========================================================================
	
	Routine Description:
		Set LED Status

	Arguments:
		pAd						Pointer to our adapter
		Status					LED Status

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID RTMPSetLED(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR			Status)
{
	//ULONG			data;
	UCHAR			HighByte = 0;
	UCHAR			LowByte;
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	PWSC_CTRL		pWscControl = NULL;

#ifdef CONFIG_AP_SUPPORT
	pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl;
#endif // CONFIG_AP_SUPPORT //
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //
	BOOLEAN 		bIgnored = FALSE;
	BOOLEAN			FlgCmdSend = FALSE;

#ifdef RALINK_ATE
	/*
		In ATE mode of RT2860 AP/STA, we have erased 8051 firmware.
		So LED mode is not supported when ATE is running.
	*/
	if (!IS_RT3572(pAd))
	{
		if (ATE_ON(pAd))
			return;
	}
#endif // RALINK_ATE //

	LowByte = pAd->LedCntl.field.LedMode&0x7f;
	switch (Status)
	{
		case LED_LINK_DOWN:
			HighByte = 0x20;
			pAd->LedIndicatorStrength = 0; 
			FlgCmdSend = TRUE;
			break;
		case LED_LINK_UP:
			if (pAd->CommonCfg.Channel > 14)
				HighByte = 0xa0;
			else
				HighByte = 0x60;
			FlgCmdSend = TRUE;
			break;
		case LED_RADIO_ON:
			HighByte = 0x20;
			FlgCmdSend = TRUE;
			break;
		case LED_HALT: 
			LowByte = 0; // Driver sets MAC register and MAC controls LED
		case LED_RADIO_OFF:
			HighByte = 0;
			FlgCmdSend = TRUE;
			break;
		case LED_WPS:
			HighByte = 0x10;
			FlgCmdSend = TRUE;
			break;
		case LED_ON_SITE_SURVEY:
			HighByte = 0x08;
			FlgCmdSend = TRUE;
			break;
		case LED_POWER_UP:
			HighByte = 0x04;
			FlgCmdSend = TRUE;
			break;
#ifdef RALINK_ATE
#endif // RALINK_ATE //
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
		case LED_WPS_IN_PROCESS:
			if (WscSupportWPSLEDMode(pAd))
			{
				HighByte = LINK_STATUS_WPS_IN_PROCESS;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				pWscControl->WscLEDMode = LED_WPS_IN_PROCESS;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_IN_PROCESS\n", __FUNCTION__));				
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_ERROR:
			if (WscSupportWPSLEDMode(pAd))
			{
				// In the case of LED mode 9, the error LED should be turned on only after WPS walk time expiration.
				if ((pWscControl->bWPSWalkTimeExpiration == FALSE) && 
					 ((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9))
				{
					// do nothing.
				}
				else
				{
					HighByte = LINK_STATUS_WPS_ERROR;
					AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				}
		
				pWscControl->WscLEDMode = LED_WPS_ERROR;
				pWscControl->WscLastWarningLEDMode = LED_WPS_ERROR;
				
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_ERROR\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_SESSION_OVERLAP_DETECTED:
			if (WscSupportWPSLEDMode(pAd))
			{
				HighByte = LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				pWscControl->WscLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
				pWscControl->WscLastWarningLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SESSION_OVERLAP_DETECTED\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_SUCCESS:
			if (WscSupportWPSLEDMode(pAd))
			{
				if (((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_7) || 
					((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_11) || 
					((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_12))
				{
					// In the WPS LED mode 7, 11 and 12, the blue LED would last 300 seconds regardless of the AP's security settings.
					HighByte = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
					AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		
					// Turn off the WPS successful LED pattern after 300 seconds.
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
					
					DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS_WITH_SECURITY)\n", __FUNCTION__));
				}
				else if ((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_8) // The WPS LED mode 8
				{
					if (WscAPHasSecuritySetting(pAd, pWscControl)) // The WPS AP has the security setting.
					{
						HighByte = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
						AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
						pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		
						// Turn off the WPS successful LED pattern after 300 seconds.
						RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
						
						DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS_WITH_SECURITY)\n", __FUNCTION__));
					}
					else // The WPS AP does not have the secuirty setting.
					{
						HighByte = LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY;
						AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
						pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		
						// Turn off the WPS successful LED pattern after 300 seconds.
						RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
						
						DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY)\n", __FUNCTION__));
					}
				}
				else if ((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // The WPS LED mode 9.
				{
					// Always turn on the WPS blue LED for 300 seconds.
					HighByte = LINK_STATUS_WPS_BLUE_LED;
					AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		
					// Turn off the WPS successful LED pattern after 300 seconds.
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
					
					DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_BLUE_LED)\n", __FUNCTION__));
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (Incorrect LED mode = %d)\n", 
						__FUNCTION__, (pAd->LedCntl.field.LedMode & LED_MODE)));
					ASSERT(FALSE);
				}
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_TURN_LED_OFF:
			if (WscSupportWPSLEDMode(pAd))
			{
				HighByte = LINK_STATUS_WPS_TURN_LED_OFF;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				pWscControl->WscLEDMode = LED_WPS_TURN_LED_OFF;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_TURN_ON_BLUE_LED:
			if (WscSupportWPSLEDMode(pAd))
			{
				HighByte = LINK_STATUS_WPS_BLUE_LED;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_ON_BLUE_LED\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_NORMAL_CONNECTION_WITHOUT_SECURITY:
			if (WscSupportWPSLEDMode(pAd))
			{
				HighByte = LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_NORMAL_CONNECTION_WITH_SECURITY:
			if (WscSupportWPSLEDMode(pAd))
			{
				HighByte = LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		//WPS LED Mode 10
		case LED_WPS_MODE10_TURN_ON:
			if(WscSupportWPSLEDMode10(pAd))
			{
				HighByte = LINK_STATUS_WPS_MODE10_TURN_ON;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_MODE10_TURN_ON\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_MODE10_FLASH:
			if(WscSupportWPSLEDMode10(pAd))
			{	
				HighByte = LINK_STATUS_WPS_MODE10_FLASH;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_MODE10_FLASH\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
		case LED_WPS_MODE10_TURN_OFF:
			if(WscSupportWPSLEDMode10(pAd))
			{
				HighByte = LINK_STATUS_WPS_MODE10_TURN_OFF;
				AsicSendCommandToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_MODE10_TURN_OFF\n", __FUNCTION__));
			}
			else
				bIgnored = TRUE;
			break;
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //
		default:
			DBGPRINT(RT_DEBUG_WARN, ("RTMPSetLED::Unknown Status %d\n", Status));
			break;
	}

	if (FlgCmdSend == TRUE)
	{
		/* command here so we can reduce code size */
		AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
	}

    //
	// Keep LED status for LED SiteSurvey mode.
	// After SiteSurvey, we will set the LED mode to previous status.
	//
	if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP) && (bIgnored == FALSE))
		pAd->LedStatus = Status;
    
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetLED::Mode=%d,HighByte=0x%02x,LowByte=0x%02x\n", pAd->LedCntl.field.LedMode, HighByte, LowByte));
}

/*
	========================================================================
	
	Routine Description:
		Set LED Signal Stregth 

	Arguments:
		pAd						Pointer to our adapter
		Dbm						Signal Stregth

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	
	Note:
		Can be run on any IRQL level. 

		According to Microsoft Zero Config Wireless Signal Stregth definition as belows.
		<= -90  No Signal
		<= -81  Very Low
		<= -71  Low
		<= -67  Good
		<= -57  Very Good
		 > -57  Excellent		
	========================================================================
*/
VOID RTMPSetSignalLED(
	IN PRTMP_ADAPTER 	pAd, 
	IN NDIS_802_11_RSSI Dbm)
{
	UCHAR		nLed = 0;

	if (pAd->LedCntl.field.LedMode == LED_MODE_SIGNAL_STREGTH)
	{
	if (Dbm <= -90)
		nLed = 0;
	else if (Dbm <= -81)
		nLed = 1;
	else if (Dbm <= -71)
		nLed = 3;
	else if (Dbm <= -67)
		nLed = 7;
	else if (Dbm <= -57)
		nLed = 15;
	else 
		nLed = 31;

	//
	// Update Signal Stregth to firmware if changed.
	//
	if (pAd->LedIndicatorStrength != nLed)
	{
		AsicSendCommandToMcu(pAd, 0x51, 0xff, nLed, pAd->LedCntl.field.Polarity);
		pAd->LedIndicatorStrength = nLed;
	}
}
}

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
//
// LED indication for normal connection start.
//
VOID
LEDConnectionStart(
	IN PRTMP_ADAPTER pAd)
{
	// LED indication.
	//if (pAd->StaCfg.WscControl.bWPSSession == FALSE)
	//if (pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE && pAd->StaCfg.WscControl.bWscTrigger)
	if (pAd->StaCfg.WscControl.WscConfMode == WSC_DISABLE)
	{
		if ((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
		{
			UCHAR WPSLEDStatus = 0;
			
			// The AP uses OPEN-NONE.
			if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeOpen) && (pAd->StaCfg.WepStatus == Ndis802_11WEPDisabled))
			{
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
#ifdef RTMP_MAC_PCI
				RTMPSetLED(pAd, WPSLEDStatus);
#endif // RTMP_MAC_PCI //
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
			}
			else // The AP uses an encryption algorithm.
			{
				WPSLEDStatus = LED_WPS_IN_PROCESS;
#ifdef RTMP_MAC_PCI
				RTMPSetLED(pAd, WPSLEDStatus);
#endif // RTMP_MAC_PCI //
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_IN_PROCESS\n", __FUNCTION__));
			}
		}
	}
}


//
// LED indication for normal connection completion.
//
VOID
LEDConnectionCompletion(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN bSuccess)
{
	// LED indication.
	//if (pAd->StaCfg.WscControl.bWPSSession == FALSE)
	if (pAd->StaCfg.WscControl.WscConfMode == WSC_DISABLE)
	{
		if ((pAd->LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
		{
			UCHAR WPSLEDStatus = 0;
			
			if (bSuccess == TRUE) // Successful connenction.
			{				
				// The AP uses OPEN-NONE.
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeOpen) && (pAd->StaCfg.WepStatus == Ndis802_11WEPDisabled))
				{
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITHOUT_SECURITY;
#ifdef RTMP_MAC_PCI
					RTMPSetLED(pAd, WPSLEDStatus);
#endif // RTMP_MAC_PCI //
					DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITHOUT_SECURITY\n", __FUNCTION__));
				}
				else // The AP uses an encryption algorithm.
				{
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITH_SECURITY;
#ifdef RTMP_MAC_PCI
					RTMPSetLED(pAd, WPSLEDStatus);
#endif // RTMP_MAC_PCI //
					DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITH_SECURITY\n", __FUNCTION__));
				}
			}
			else // Connection failure.
			{
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
#ifdef RTMP_MAC_PCI
				RTMPSetLED(pAd, WPSLEDStatus);
#endif // RTMP_MAC_PCI //
				DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
			}
		}
	}
}
#endif // WSC_LED_SUPPORT //
#endif // WSC_STA_SUPPORT //
#endif // LED_CONTROL_SUPPORT //

/*
	========================================================================
	
	Routine Description:
		Enable RX 

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL
	
	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd)
{
//	WPDMA_GLO_CFG_STRUC	GloCfg;
//	ULONG	i = 0;
	UINT32 rx_filter_flag;

	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPEnableRxTx\n"));

	// Enable Rx DMA.
	RT28XXDMAEnable(pAd);

	// enable RX of MAC block
	if (pAd->OpMode == OPMODE_AP)
	{
		rx_filter_flag = APNORMAL;

#ifdef CONFIG_AP_SUPPORT
#ifdef IDS_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.IdsEnable)
				rx_filter_flag &= (~0x4);	// Don't drop those not-U2M frames
		}
#endif // IDS_SUPPORT //			
#endif // CONFIG_AP_SUPPORT //

		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);     // enable RX of DMA block
	}
	
#ifdef HW_COEXISTENCE_SUPPORT
	if (pAd->bHWCoexistenceInit)
	{
#ifdef BT_COEXISTENCE_SUPPORT
		if (IS_ENABLE_BT_WIFI_ACTIVE_PULL_LOW_BY_FORCE(pAd))
		{
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, WLANBTCOEXISTENCE_WIFI_ACT_PULL_LOW);
		}
		else
#endif // BT_COEXISTENCE_SUPPORT //
		{
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, WLANBTCOEXISTENCE);
		}
	}
	else
#endif // HW_COEXISTENCE_SUPPORT //
	{
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPEnableRxTx\n"));	
}


//+++Add by shiang, move from os/linux/rt_main_dev.c
void CfgInitHook(PRTMP_ADAPTER pAd)
{
	//pAd->bBroadComHT = TRUE;
}

//---Add by shiang, move from os/linux/rt_main_dev.c


static INT RtmpChipOpsRegister(
	IN RTMP_ADAPTER *pAd,
	IN INT			infType)
{
	RTMP_CHIP_OP	*pChipOps = &pAd->chipOps;
	int status;
	
	memset(pChipOps, 0, sizeof(RTMP_CHIP_OP));

	/* set eeprom related hook functions */
	status = RtmpChipOpsEepromHook(pAd, infType);

	/* set mcu related hook functions */
	switch(infType)
	{
#ifdef RTMP_PCI_SUPPORT
		case RTMP_DEV_INF_PCI:
		case RTMP_DEV_INF_PCIE:
			pChipOps->loadFirmware = RtmpAsicLoadFirmware;
			pChipOps->eraseFirmware = RtmpAsicEraseFirmware;
			pChipOps->sendCommandToMcu = RtmpAsicSendCommandToMcu;
			break;
#endif // RTMP_PCI_SUPPORT //


		default:
			break;
	}

	return status;
}


INT RtmpRaDevCtrlInit(
	IN RTMP_ADAPTER *pAd, 
	IN RTMP_INF_TYPE infType)
{
	//VOID	*handle;

	// Assign the interface type. We need use it when do register/EEPROM access.
	pAd->infType = infType;

	

#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	DBGPRINT(RT_DEBUG_TRACE, ("AP Driver version-%s\n", AP_DRIVER_VERSION));
#endif // CONFIG_AP_SUPPORT //


	RtmpChipOpsRegister(pAd, infType);

#ifdef MULTIPLE_CARD_SUPPORT
{
	extern BOOLEAN RTMP_CardInfoRead(PRTMP_ADAPTER pAd);

	// find its profile path
	pAd->MC_RowID = -1; // use default profile path
	RTMP_CardInfoRead(pAd);

	if (pAd->MC_RowID == -1)
#ifdef CONFIG_AP_SUPPORT
		strcpy(pAd->MC_FileName, AP_PROFILE_PATH);
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("MC> ROW = %d, PATH = %s\n", pAd->MC_RowID, pAd->MC_FileName));
}
#endif // MULTIPLE_CARD_SUPPORT //

	return 0;
}


BOOLEAN RtmpRaDevCtrlExit(IN RTMP_ADAPTER *pAd)
{
	INT index;
	
#ifdef MULTIPLE_CARD_SUPPORT
extern UINT8  MC_CardUsed[MAX_NUM_OF_MULTIPLE_CARD];

	if ((pAd->MC_RowID >= 0) && (pAd->MC_RowID <= MAX_NUM_OF_MULTIPLE_CARD))
		MC_CardUsed[pAd->MC_RowID] = 0; // not clear MAC address
#endif // MULTIPLE_CARD_SUPPORT //


	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) 
	{
		if (pAd->ProbeRespIE[index].pIe)
			os_free_mem(pAd, pAd->ProbeRespIE[index].pIe);
	}

	RTMPFreeAdapter(pAd);

	return TRUE;
}




#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(
	IN PRTMP_ADAPTER pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
}
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#ifdef VENDOR_FEATURE3_SUPPORT
VOID RTMP_IO_WRITE32(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT32 Value)
{
	_RTMP_IO_WRITE32(pAd, Offset, Value);
}

VOID RTMP_BBP_IO_READ8_BY_REG_ID(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT8 *pValue)
{
	_RTMP_BBP_IO_READ8_BY_REG_ID(pAd, Offset, pValue);
}

VOID RTMP_BBP_IO_READ8(
	PRTMP_ADAPTER pAd,
	UCHAR Offset,
	UINT8 *pValue,
	BOOLEAN FlgValidMCR)
{
	_RTMP_BBP_IO_READ8(pAd, Offset, pValue, FlgValidMCR);
}

VOID RTMP_BBP_IO_WRITE8_BY_REG_ID(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT8 Value)
{
	_RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, Offset, Value);
}

VOID RTMP_BBP_IO_WRITE8(
	PRTMP_ADAPTER pAd,
	UCHAR Offset,
	UINT8 Value,
	BOOLEAN FlgValidMCR)
{
	_RTMP_BBP_IO_WRITE8(pAd, Offset, Value, FlgValidMCR);
}
#endif // VENDOR_FEATURE3_SUPPORT //


#ifdef RTMP_MAC_PCI
VOID CMDHandler(                                                                                                                                                
    IN PRTMP_ADAPTER pAd)                                                                                                                                       
{                                                                                                                                                               
	PCmdQElmt		cmdqelmt;                                                                                                                                       
	PUCHAR			pData;                                                                                                                                          
	NDIS_STATUS		NdisStatus = NDIS_STATUS_SUCCESS;                                                                                                               
//	ULONG			Now = 0;
//	NTSTATUS		ntStatus;
//	unsigned long	IrqFlags;
	
	while (pAd && pAd->CmdQ.size > 0)	
	{                                                                                                                                                           
		NdisStatus = NDIS_STATUS_SUCCESS;
		                                                                                                                      
		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		NdisReleaseSpinLock(&pAd->CmdQLock);
		                                                                                                        
		if (cmdqelmt == NULL)                                                                                                                                   
			break; 
			                                                                                                                                             
		pData = cmdqelmt->buffer;                                      
		                                                                                         
		if(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			switch (cmdqelmt->command)
			{
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_PCI_SUPPORT
				case CMDTHREAD_CHAN_RESCAN:
					DBGPRINT(RT_DEBUG_TRACE, ("cmd> Re-scan channel! \n"));

					pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, TRUE);

#ifdef DOT11_N_SUPPORT
					// If phymode > PHY_11ABGN_MIXED and BW=40 check extension channel, after select channel  
					N_ChannelCheck(pAd);
#endif // DOT11_N_SUPPORT //

					DBGPRINT(RT_DEBUG_TRACE, ("cmd> Switch to %d! \n", pAd->CommonCfg.Channel));
					APStop(pAd);
					APStartUp(pAd);

#ifdef AP_QLOAD_SUPPORT
					QBSS_LoadAlarmResume(pAd);
#endif // AP_QLOAD_SUPPORT //
					break;
#endif // RTMP_PCI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

				case CMDTHREAD_REG_HINT:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_CRDA_REG_HINT(pAd, pData, cmdqelmt->bufferlength);
#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //
					break;

				case CMDTHREAD_REG_HINT_11D:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_CRDA_REG_HINT11D(pAd, pData, cmdqelmt->bufferlength);
#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //
					break;

				case CMDTHREAD_SCAN_END:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_SCAN_END(pAd, FALSE);
#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //
					break;

				case CMDTHREAD_CONNECT_RESULT_INFORM:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_CONN_RESULT_INFORM(pAd,
												pAd->MlmeAux.Bssid,
												pData, cmdqelmt->bufferlength,
												pData, cmdqelmt->bufferlength,
												1);
#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //
					break;

				default:
					DBGPRINT(RT_DEBUG_ERROR, ("--> Control Thread !! ERROR !! Unknown(cmdqelmt->command=0x%x) !! \n", cmdqelmt->command));
					break;
			}
		}

		if (cmdqelmt->CmdFromNdis == TRUE)
		{
			if (cmdqelmt->buffer != NULL)
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
		else
		{
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
	}	/* end of while */
}
#endif // RTMP_MAC_PCI //



/* export wlan function symbol list */
#ifdef OS_ABL_SUPPORT


EXPORT_SYMBOL(NICInitializeAdapter);
EXPORT_SYMBOL(NICInitAsicFromEEPROM);
EXPORT_SYMBOL(NICReadEEPROMParameters);
#ifdef RTMP_RF_RW_SUPPORT
EXPORT_SYMBOL(NICInitRFRegisters);
#endif // RTMP_RF_RW_SUPPORT //
EXPORT_SYMBOL(UserCfgExit);
EXPORT_SYMBOL(UserCfgInit);
EXPORT_SYMBOL(CfgInitHook);
EXPORT_SYMBOL(RTMPEnableRxTx);
EXPORT_SYMBOL(RTMPSetPhyMode);
EXPORT_SYMBOL(RTMPMaxRssi);
EXPORT_SYMBOL(RTMPAllocAdapterBlock);
EXPORT_SYMBOL(RTMPAllocTxRxRingMemory);
EXPORT_SYMBOL(RTMPFreeTxRxRingMemory);
EXPORT_SYMBOL(NICLoadRateSwitchingParams);
EXPORT_SYMBOL(RtmpRaDevCtrlInit);
EXPORT_SYMBOL(RtmpRaDevCtrlExit);
EXPORT_SYMBOL(NICLoadFirmware);
EXPORT_SYMBOL(RTMPSetProfileParameters);
EXPORT_SYMBOL(RT28XXDMADisable);

#ifdef DOT11_N_SUPPORT
EXPORT_SYMBOL(SetCommonHT);
EXPORT_SYMBOL(N_ChannelCheck);
#endif // DOT11_N_SUPPORT //

EXPORT_SYMBOL(AsicSwitchChannel);
EXPORT_SYMBOL(AsicLockChannel);

EXPORT_SYMBOL(ba_reordering_resource_init);
EXPORT_SYMBOL(ba_reordering_resource_release);

EXPORT_SYMBOL(RTMPDeQueuePacket);

EXPORT_SYMBOL(CMDHandler);
EXPORT_SYMBOL(RTEnqueueInternalCmd);
EXPORT_SYMBOL(RTThreadDequeueCmd);

EXPORT_SYMBOL(TpcReqTabInit);
EXPORT_SYMBOL(TpcReqTabExit);
EXPORT_SYMBOL(MeasureReqTabInit);
EXPORT_SYMBOL(MeasureReqTabExit);

EXPORT_SYMBOL(MlmeInit);
EXPORT_SYMBOL(MlmeHandler);
EXPORT_SYMBOL(MlmeHalt);
EXPORT_SYMBOL(MlmeEnqueue);

EXPORT_SYMBOL(RTMPCancelTimer);
EXPORT_SYMBOL(AsicSendCommandToMcu);
EXPORT_SYMBOL(MacTableLookup);
EXPORT_SYMBOL(RTMPZeroMemory);
EXPORT_SYMBOL(RTMPSoftEncryptWEP);
EXPORT_SYMBOL(getRate);

/* command */
EXPORT_SYMBOL(RTMPIoctlGetSiteSurvey);

EXPORT_SYMBOL(RTMPIoctlGetSTAINFO);	// ASUS EXT by Jiahao
EXPORT_SYMBOL(RTMPIoctlGetSTAT);	// ASUS EXT by Jiahao
EXPORT_SYMBOL(RTMPIoctlGetRSSI);	// ASUS EXT by Jiahao

#ifdef CONFIG_AP_SUPPORT
EXPORT_SYMBOL(APInitialize);
EXPORT_SYMBOL(APStartUp);
EXPORT_SYMBOL(APShutdown);
EXPORT_SYMBOL(APAutoSelectChannel);
EXPORT_SYMBOL(APMakeAllBssBeacon);
EXPORT_SYMBOL(APUpdateAllBeaconFrame);
EXPORT_SYMBOL(MbssKickOutStas);

#ifdef UAPSD_AP_SUPPORT
EXPORT_SYMBOL(UAPSD_SP_Close);

#endif // UAPSD_AP_SUPPORT //

#ifdef AP_QLOAD_SUPPORT
EXPORT_SYMBOL(QBSS_LoadInit);
#endif // AP_QLOAD_SUPPORT //

EXPORT_SYMBOL(APSendPackets);
EXPORT_SYMBOL(RTMPIoctlGetMacTable);
EXPORT_SYMBOL(RTMPAPIoctlE2PROM);
EXPORT_SYMBOL(RTMPIoctlStatistics);
EXPORT_SYMBOL(RT28XX_IOCTL_MaxRateGet);
#ifdef DOT11_N_SUPPORT
EXPORT_SYMBOL(RTMPIoctlQueryBaTable);
#endif // DOT11_N_SUPPORT //
EXPORT_SYMBOL(RTMPAPQueryInformation);
EXPORT_SYMBOL(RTMPAPPrivIoctlShow);
EXPORT_SYMBOL(RTMPAPPrivIoctlSet);
EXPORT_SYMBOL(RTMPAPSetInformation);

#ifdef RTMP_MAC_PCI
EXPORT_SYMBOL(RTMPHandleMcuInterrupt);
EXPORT_SYMBOL(APRxDoneInterruptHandle);
EXPORT_SYMBOL(APStop);

#ifdef AP_QLOAD_SUPPORT
EXPORT_SYMBOL(QBSS_LoadAlarmResume);
EXPORT_SYMBOL(QBSS_LoadUpdate);
#endif // AP_QLOAD_SUPPORT //
#endif // RTMP_MAC_PCI //

#ifdef DBG
EXPORT_SYMBOL(RTMPAPIoctlBBP);
EXPORT_SYMBOL(RTMPAPIoctlMAC);
#ifdef RTMP_RF_RW_SUPPORT
EXPORT_SYMBOL(RTMPAPIoctlRF);
#endif // RTMP_RF_RW_SUPPORT //
#endif // DBG //
#endif // CONFIG_AP_SUPPORT //




#ifdef RTMP_MAC_PCI
EXPORT_SYMBOL(RT28xxPciAsicRadioOff);
EXPORT_SYMBOL(RTMPHandleTxRingDmaDoneInterrupt);
EXPORT_SYMBOL(RTMPHandlePreTBTTInterrupt);
EXPORT_SYMBOL(RTMPHandleTBTTInterrupt);
EXPORT_SYMBOL(NICUpdateFifoStaCounters);
EXPORT_SYMBOL(RTMPHandleMgmtRingDmaDoneInterrupt);
EXPORT_SYMBOL(RTMPHandleRxCoherentInterrupt);
#endif // RTMP_MAC_PCI //



#ifdef MAT_SUPPORT
EXPORT_SYMBOL(MATEngineInit);
EXPORT_SYMBOL(MATEngineExit);
EXPORT_SYMBOL(getIPv6MacTbInfo);
EXPORT_SYMBOL(getIPMacTbInfo);
#endif // MAT_SUPPORT //

#ifdef WMM_ACM_SUPPORT
EXPORT_SYMBOL(ACMP_Init);
EXPORT_SYMBOL(ACMP_Release);

#ifdef CONFIG_AP_SUPPORT
EXPORT_SYMBOL(ACMP_PsRspDeltsSentOutHandle);
#endif // CONFIG_AP_SUPPORT //

#endif // WMM_ACM_SUPPORT //


#ifdef APCLI_SUPPORT
EXPORT_SYMBOL(ApCliIfUp);
EXPORT_SYMBOL(RT28xx_ApCli_Remove);
EXPORT_SYMBOL(ApCliIfLookUp);
#endif // APCLI_SUPPORT //

#ifdef WDS_SUPPORT
EXPORT_SYMBOL(WdsDown);
EXPORT_SYMBOL(RT28xx_WDS_Remove);
#endif // WDS_SUPPORT //

EXPORT_SYMBOL(GenerateWpsPinCode);

#ifdef WSC_INCLUDED
EXPORT_SYMBOL(WscInit);
EXPORT_SYMBOL(WscThreadExit);
EXPORT_SYMBOL(WSC_HDR_BTN_Init);
EXPORT_SYMBOL(WscGenerateUUID);
EXPORT_SYMBOL(WscThreadInit);
EXPORT_SYMBOL(WSC_HDR_BTN_Stop);
EXPORT_SYMBOL(WscStop);

#ifdef WSC_AP_SUPPORT
EXPORT_SYMBOL(RTMPIoctlWscProfile);
EXPORT_SYMBOL(RTMPIoctlSetWSCOOB);
#endif // WSC_AP_SUPPORT //

#ifdef WSC_STA_SUPPORT
EXPORT_SYMBOL(Set_WscMode_Proc);
EXPORT_SYMBOL(Set_WscGenPinCode_Proc);
EXPORT_SYMBOL(Set_WscPinCode_Proc);
EXPORT_SYMBOL(WscCreateProfileFromCfg);
EXPORT_SYMBOL(Set_WscGetConf_Proc);
EXPORT_SYMBOL(WscGetAuthTypeStr);
EXPORT_SYMBOL(Set_WscConfMode_Proc);
EXPORT_SYMBOL(Set_WscSsid_Proc);
EXPORT_SYMBOL(Set_WscBssid_Proc);
EXPORT_SYMBOL(WscWriteConfToPortCfg);
EXPORT_SYMBOL(WscInitRegistrarPair);
EXPORT_SYMBOL(WscGetAuthTypeFromStr);
EXPORT_SYMBOL(WscGetEncryTypeStr);
EXPORT_SYMBOL(WscGetEncrypTypeFromStr);
EXPORT_SYMBOL(MlmeEnqueueForWsc);
#endif // WSC_STA_SUPPORT //
#endif // WSC_INCLUDED //

#ifdef CONFIG_AP_SUPPORT
//#ifdef AUTO_CH_SELECT_ENHANCE
EXPORT_SYMBOL(AutoChBssTableDestroy);
EXPORT_SYMBOL(ChannelInfoDestroy);
EXPORT_SYMBOL(AutoChBssTableInit);
EXPORT_SYMBOL(ChannelInfoInit);
//#endif // AUTO_CH_SELECT_ENHANCE //
#endif // CONFIG_AP_SUPPORT //

#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11N_DRAFT3
EXPORT_SYMBOL(APOverlappingBSSScan);
EXPORT_SYMBOL(RTMP_11N_D3_TimerInit);
#endif // DOT11N_DRAFT3 //
#endif // CONFIG_AP_SUPPORT //

#endif // DOT11_N_SUPPORT //

#ifdef BLOCK_NET_IF
EXPORT_SYMBOL(initblockQueueTab);
#endif // BLOCK_NET_IF //

#ifdef DOT11K_RRM_SUPPORT
EXPORT_SYMBOL(RRM_CfgInit);
EXPORT_SYMBOL(RRM_QuietUpdata);
#endif // DOT11K_RRM_SUPPORT //

#ifdef RALINK_ATE
EXPORT_SYMBOL(RtmpDoAte);

#endif // RALINK_ATE //

#ifdef DOT11R_FT_SUPPORT
#endif // DOT11R_FT_SUPPORT //


#ifdef QOS_DLS_SUPPORT
#endif // QOS_DLS_SUPPORT //

#ifdef DFS_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
EXPORT_SYMBOL(pulse_radar_detect_tasklet);
EXPORT_SYMBOL(width_radar_detect_tasklet);
EXPORT_SYMBOL(carrier_sense_tasklet);
#endif // RTMP_MAC_PCI //
#endif // CONFIG_AP_SUPPORT //
#endif // DFS_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef TONE_RADAR_DETECT_SUPPORT
EXPORT_SYMBOL(RTMPHandleRadarInterrupt);
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef WAPI_SUPPORT
EXPORT_SYMBOL(WAPIInstallSharedKey);
EXPORT_SYMBOL(RTMPStartWapiRekeyTimerAction);
EXPORT_SYMBOL(RTMPDeriveWapiGTK);
EXPORT_SYMBOL(RTMPIoctlQueryWapiConf);
EXPORT_SYMBOL(RTMPCancelWapiRekeyTimerAction);
EXPORT_SYMBOL(WAPIInstallPairwiseKey);
#endif // WAPI_SUPPORT //

#ifdef BT_COEXISTENCE_SUPPORT
EXPORT_SYMBOL(BtCoexistInit);
#endif // BT_COEXISTENCE_SUPPORT //

#ifdef LED_CONTROL_SUPPORT
EXPORT_SYMBOL(RTMPSetLED);
#endif // LED_CONTROL_SUPPORT //

#endif // OS_ABL_SUPPORT //
