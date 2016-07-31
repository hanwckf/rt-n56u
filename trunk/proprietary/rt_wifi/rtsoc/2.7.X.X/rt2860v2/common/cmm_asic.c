/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC
	
	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"

#ifdef RT2880
/* Reset the RFIC setting to new series    */
RTMP_RF_REGS RF2850RegTable[] = {
/*		ch	 R1 		 R2 		 R3(TX0~4=0) R4*/
		{1,  0x98402ecc, 0x984c0786, 0x981ab455, 0x9800510b},
		{2,  0x98402ecc, 0x984c0786, 0x981a8a55, 0x9800519f},
		{3,  0x98402ecc, 0x984c078a, 0x981a8a55, 0x9800518b},
		{4,  0x98402ecc, 0x984c078a, 0x981a8a55, 0x9800519f},
		{5,  0x98402ecc, 0x984c078e, 0x981a8a55, 0x9800518b},
		{6,  0x98402ecc, 0x984c078e, 0x981a8a55, 0x9800519f},
		{7,  0x98402ecc, 0x984c0792, 0x981a8a55, 0x9800518b},
		{8,  0x98402ecc, 0x984c0792, 0x981a8a55, 0x9800519f},
		{9,  0x98402ecc, 0x984c0796, 0x981a8a55, 0x9800518b},
		{10, 0x98402ecc, 0x984c0796, 0x981a8a55, 0x9800519f},
		{11, 0x98402ecc, 0x984c079a, 0x981a8a55, 0x9800518b},
		{12, 0x98402ecc, 0x984c079a, 0x981a8a55, 0x9800519f},
		{13, 0x98402ecc, 0x984c079e, 0x981a8a55, 0x9800518b},
		{14, 0x98402ecc, 0x984c07a2, 0x981a8a55, 0x98005193},

		/* 802.11 UNI / HyperLan 2*/
		{36, 0x98402ecc, 0x984c099a, 0x98198a55, 0x980ed1a3},
		{38, 0x98402ecc, 0x984c099e, 0x98198a55, 0x980ed193},
		{40, 0x98402ec8, 0x984c0682, 0x98198a55, 0x980ed183},
		{44, 0x98402ec8, 0x984c0682, 0x98198a55, 0x980ed1a3},
		{46, 0x98402ec8, 0x984c0686, 0x98198a55, 0x980ed18b},
		{48, 0x98402ec8, 0x984c0686, 0x98198a55, 0x980ed19b},
		{52, 0x98402ec8, 0x984c068a, 0x98198a55, 0x980ed193},
		{54, 0x98402ec8, 0x984c068a, 0x98198a55, 0x980ed1a3},
		{56, 0x98402ec8, 0x984c068e, 0x98198a55, 0x980ed18b},
		{60, 0x98402ec8, 0x984c0692, 0x98198a55, 0x980ed183},
		{62, 0x98402ec8, 0x984c0692, 0x98198a55, 0x980ed193},
		{64, 0x98402ec8, 0x984c0692, 0x98198a55, 0x980ed1a3}, /* Plugfest#4, Day4, change RFR3 left4th 9->5.*/

		/* 802.11 HyperLan 2*/
		{100, 0x98402ec8, 0x984c06b2, 0x981b8a55, 0x980ed783},
		
		/* 2008.04.30 modified */
		/* The system team has AN to improve the EVM value */
		/* for channel 102 to 108 for the RT2850/RT2750 dual band solution.*/
		{102, 0x98402ec8, 0x985c06b2, 0x985b8a55, 0x980ed793},
		{104, 0x98402ec8, 0x985c06b2, 0x985b8a55, 0x980ed1a3},
		{108, 0x98402ecc, 0x985c0a32, 0x985b8a55, 0x980ed193},

		{110, 0x98402ecc, 0x984c0a36, 0x981b8a55, 0x980ed183},
		{112, 0x98402ecc, 0x984c0a36, 0x981b8a55, 0x980ed19b},
		{116, 0x98402ecc, 0x984c0a3a, 0x981b8a55, 0x980ed1a3},
		{118, 0x98402ecc, 0x984c0a3e, 0x981b8a55, 0x980ed193},
		{120, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed183},
		{124, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed193},
		{126, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed15b}, /* 0x980ed1bb->0x980ed15b required by Rory 20070927*/
		{128, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed1a3},
		{132, 0x98402ec4, 0x984c0386, 0x981b8a55, 0x980ed18b},
		{134, 0x98402ec4, 0x984c0386, 0x981b8a55, 0x980ed193},
		{136, 0x98402ec4, 0x984c0386, 0x981b8a55, 0x980ed19b},
		{140, 0x98402ec4, 0x984c038a, 0x981b8a55, 0x980ed183},

		/* 802.11 UNII*/
		{149, 0x98402ec4, 0x984c038a, 0x981b8a55, 0x980ed1a7},
		{151, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed187},
		{153, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed18f},
		{157, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed19f},
		{159, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed1a7},
		{161, 0x98402ec4, 0x984c0392, 0x981b8a55, 0x980ed187},
		{165, 0x98402ec4, 0x984c0392, 0x981b8a55, 0x980ed197},
		{167, 0x98402ec4, 0x984c03d2, 0x981b9855, 0x9815531f},
		{169, 0x98402ec4, 0x984c03d2, 0x981b9855, 0x98155327},
		{171, 0x98402ec4, 0x984c03d6, 0x981b9855, 0x98155307},
		{173, 0x98402ec4, 0x984c03d6, 0x981b9855, 0x9815530f},

		/* Japan*/
		{184, 0x95002ccc, 0x9500491e, 0x9509be55, 0x950c0a0b},
		{188, 0x95002ccc, 0x95004922, 0x9509be55, 0x950c0a13},
		{192, 0x95002ccc, 0x95004926, 0x9509be55, 0x950c0a1b},
		{196, 0x95002ccc, 0x9500492a, 0x9509be55, 0x950c0a23},
		{208, 0x95002ccc, 0x9500493a, 0x9509be55, 0x950c0a13},
		{212, 0x95002ccc, 0x9500493e, 0x9509be55, 0x950c0a1b},
		{216, 0x95002ccc, 0x95004982, 0x9509be55, 0x950c0a23},

		/* still lack of MMAC(Japan) ch 34,38,42,46*/
};
UCHAR	NUM_OF_2850_CHNL = (sizeof(RF2850RegTable) / sizeof(RTMP_RF_REGS));
#endif /* RT2880 */

#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm						0x01
#define MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK		0x03

#ifdef CONFIG_STA_SUPPORT
VOID AsicUpdateAutoFallBackTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pRateTable)
{
	UCHAR					i;
	HT_FBK_CFG0_STRUC		HtCfg0;
	HT_FBK_CFG1_STRUC		HtCfg1;
	LG_FBK_CFG0_STRUC		LgCfg0;
	LG_FBK_CFG1_STRUC		LgCfg1;
#ifdef DOT11N_SS3_SUPPORT
	TX_FBK_CFG_3S_0_STRUC	Ht3SSCfg0;
	TX_FBK_CFG_3S_1_STRUC	Ht3SSCfg1;
#endif /* DOT11N_SS3_SUPPORT */
	PRTMP_TX_RATE_SWITCH	pCurrTxRate, pNextTxRate;

#ifdef AGS_SUPPORT
	PRTMP_TX_RATE_SWITCH_AGS	pCurrTxRate_AGS, pNextTxRate_AGS;	
	BOOLEAN					bUseAGS = FALSE;

	if (AGS_IS_USING(pAd, pRateTable))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Use AGS\n", __FUNCTION__));
		
		bUseAGS = TRUE;

		Ht3SSCfg0.word = 0x1211100f;
		Ht3SSCfg1.word = 0x16151413;	
	}
#endif /* AGS_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT3883(pAd))
	{
		Ht3SSCfg0.word = 0x12111008;
		Ht3SSCfg1.word = 0x16151413;
	}
#endif /* DOT11N_SS3_SUPPORT */

	/* set to initial value*/
	HtCfg0.word = 0x65432100;
	HtCfg1.word = 0xedcba980;
	LgCfg0.word = 0xedcba988;
	LgCfg1.word = 0x00002100;

#ifdef NEW_RATE_ADAPT_SUPPORT
	/* Use standard fallback if using new rate table */
	if (ADAPT_RATE_TABLE(pRateTable))
		goto skipUpdate;
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	if (bUseAGS)
	{
		pNextTxRate_AGS = (PRTMP_TX_RATE_SWITCH_AGS)pRateTable+1;
		pNextTxRate = (PRTMP_TX_RATE_SWITCH)pNextTxRate_AGS;
	}
	else
#endif /* AGS_SUPPORT */
		pNextTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1;

	for (i = 1; i < *((PUCHAR) pRateTable); i++)
	{
#ifdef AGS_SUPPORT
		if (bUseAGS)
		{
			pCurrTxRate_AGS = (PRTMP_TX_RATE_SWITCH_AGS)pRateTable+1+i;
			pCurrTxRate = (PRTMP_TX_RATE_SWITCH)pCurrTxRate_AGS;
		}
		else
#endif /* AGS_SUPPORT */
			pCurrTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1+i;

		switch (pCurrTxRate->Mode)
		{
			case 0:		/* CCK */
				break;
			case 1:		/* OFDM */
				{
					switch(pCurrTxRate->CurrMCS)
					{
						case 0:
							LgCfg0.field.OFDMMCS0FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 1:
							LgCfg0.field.OFDMMCS1FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 2:
							LgCfg0.field.OFDMMCS2FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 3:
							LgCfg0.field.OFDMMCS3FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 4:
							LgCfg0.field.OFDMMCS4FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 5:
							LgCfg0.field.OFDMMCS5FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 6:
							LgCfg0.field.OFDMMCS6FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 7:
							LgCfg0.field.OFDMMCS7FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
					}
				}
				break;
#ifdef DOT11_N_SUPPORT
			case 2:		/* HT-MIX */
			case 3:		/* HT-GF */
				{
					if ((pNextTxRate->Mode >= MODE_HTMIX) && (pCurrTxRate->CurrMCS != pNextTxRate->CurrMCS))
					{
						if (pCurrTxRate->CurrMCS <= 15)
						{
							switch(pCurrTxRate->CurrMCS)
							{
								case 0:
									HtCfg0.field.HTMCS0FBK = pNextTxRate->CurrMCS;
									break;
								case 1:
									HtCfg0.field.HTMCS1FBK = pNextTxRate->CurrMCS;
									break;
								case 2:
									HtCfg0.field.HTMCS2FBK = pNextTxRate->CurrMCS;
									break;
								case 3:
									HtCfg0.field.HTMCS3FBK = pNextTxRate->CurrMCS;
									break;
								case 4:
									HtCfg0.field.HTMCS4FBK = pNextTxRate->CurrMCS;
									break;
								case 5:
									HtCfg0.field.HTMCS5FBK = pNextTxRate->CurrMCS;
									break;
								case 6:
									HtCfg0.field.HTMCS6FBK = pNextTxRate->CurrMCS;
									break;
								case 7:
									HtCfg0.field.HTMCS7FBK = pNextTxRate->CurrMCS;
									break;
								case 8:
									HtCfg1.field.HTMCS8FBK = pNextTxRate->CurrMCS;
									break;
								case 9:
									HtCfg1.field.HTMCS9FBK = pNextTxRate->CurrMCS;
									break;
								case 10:
									HtCfg1.field.HTMCS10FBK = pNextTxRate->CurrMCS;
									break;
								case 11:
									HtCfg1.field.HTMCS11FBK = pNextTxRate->CurrMCS;
									break;
								case 12:
									HtCfg1.field.HTMCS12FBK = pNextTxRate->CurrMCS;
									break;
								case 13:
									HtCfg1.field.HTMCS13FBK = pNextTxRate->CurrMCS;
									break;
								case 14:
									HtCfg1.field.HTMCS14FBK = pNextTxRate->CurrMCS;
									break;
								case 15:
									HtCfg1.field.HTMCS15FBK = pNextTxRate->CurrMCS;
									break;
							}
						}
						else 
#ifdef AGS_SUPPORT
						if ((bUseAGS == TRUE) && 
							(pCurrTxRate->CurrMCS >= 16) && (pCurrTxRate->CurrMCS <= 23))
						{
							switch(pCurrTxRate->CurrMCS)
							{
								case 16:
									Ht3SSCfg0.field.HTMCS16FBK = pNextTxRate->CurrMCS;
									break;
								case 17:
									Ht3SSCfg0.field.HTMCS17FBK = pNextTxRate->CurrMCS;
									break;
								case 18:
									Ht3SSCfg0.field.HTMCS18FBK = pNextTxRate->CurrMCS;
									break;
								case 19:
									Ht3SSCfg0.field.HTMCS19FBK = pNextTxRate->CurrMCS;
									break;
								case 20:
									Ht3SSCfg1.field.HTMCS20FBK = pNextTxRate->CurrMCS;
									break;
								case 21:
									Ht3SSCfg1.field.HTMCS21FBK = pNextTxRate->CurrMCS;
									break;
								case 22:
									Ht3SSCfg1.field.HTMCS22FBK = pNextTxRate->CurrMCS;
									break;
								case 23:
									Ht3SSCfg1.field.HTMCS23FBK = pNextTxRate->CurrMCS;
									break;
							}
						}
						else
#endif /* AGS_SUPPORT */
							DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support CurrMCS=%d\n", pCurrTxRate->CurrMCS));
					}
				}
				break;
#endif /* DOT11_N_SUPPORT */
		}

		pNextTxRate = pCurrTxRate;
	}

#ifdef AGS_SUPPORT
	if (bUseAGS == TRUE)
	{
		Ht3SSCfg0.field.HTMCS16FBK = 0x8; // MCS 16 -> MCS 8
		HtCfg1.field.HTMCS8FBK = 0x0; // MCS 8 -> MCS 0

		LgCfg0.field.OFDMMCS2FBK = 0x3; // OFDM 12 -> CCK 11
		LgCfg0.field.OFDMMCS1FBK = 0x2; // OFDM 9 -> CCK 5.5
		LgCfg0.field.OFDMMCS0FBK = 0x2; // OFDM 6 -> CCK 5.5
	}
#endif /* AGS_SUPPORT */

#ifdef NEW_RATE_ADAPT_SUPPORT
skipUpdate:
#endif /* NEW_RATE_ADAPT_SUPPORT */

	RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, HtCfg0.word);
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, HtCfg1.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG0, LgCfg0.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG1, LgCfg1.word);

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd)
#ifdef AGS_SUPPORT
		|| (bUseAGS == TRUE)
#endif /* AGS_SUPPORT */ 
	)
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, Ht3SSCfg0.word);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, Ht3SSCfg1.word);
		DBGPRINT(RT_DEBUG_TRACE, ("AsicUpdateAutoFallBackTable: Ht3SSCfg0=0x%x, Ht3SSCfg1=0x%x\n", Ht3SSCfg0.word, Ht3SSCfg1.word));
	}
#endif /* DOT11N_SS3_SUPPORT */

}
#endif /* CONFIG_STA_SUPPORT */

/*
	========================================================================

	Routine Description:
		Set MAC register value according operation mode.
		OperationMode AND bNonGFExist are for MM and GF Proteciton.
		If MM or GF mask is not set, those passing argument doesn't not take effect.
		
		Operation mode meaning:
		= 0 : Pure HT, no preotection.
		= 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
		= 0x10: No Transmission in 40M is protected.
		= 0x11: Transmission in both 40M and 20M shall be protected
		if (bNonGFExist)
			we should choose not to use GF. But still set correct ASIC registers.
	========================================================================
*/
VOID 	AsicUpdateProtect(
	IN		PRTMP_ADAPTER	pAd,
	IN 		USHORT			OperationMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist)	
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT			offset;
	UCHAR			i;
	UINT32 MacReg = 0;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

#ifdef DOT11_N_SUPPORT
	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
	{
		return;
	}

#ifdef RT3883
	if (pAd->FlgCWC)
		RT3883_CWC_ProtectAdjust(pAd, &SetMask, &OperationMode);
	else
#endif /* RT3883 */
	if (pAd->BATable.numDoneOriginator)
	{
		/* */
		/* enable the RTS/CTS to avoid channel collision*/
		/* */
		SetMask |= ALLN_SETPROTECT;
		OperationMode = 8;
	}
#endif /* DOT11_N_SUPPORT */

	/* Config ASIC RTS threshold register*/
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	/* If the user want disable RtsThreshold and enbale Amsdu/Ralink-Aggregation, set the RtsThreshold as 4096*/
        if ((
#ifdef DOT11_N_SUPPORT
			(pAd->CommonCfg.BACapability.field.AmsduEnable) || 
#endif /* DOT11_N_SUPPORT */
			(pAd->CommonCfg.bAggregationCapable == TRUE))
            && pAd->CommonCfg.RtsThreshold == MAX_RTS_THRESHOLD)
        {
			MacReg |= (0x1000 << 8);
        }
        else
        {
			MacReg |= (pAd->CommonCfg.RtsThreshold << 8);
        }

	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	/* Initial common protection settings*/
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	/* update PHY mode and rate*/
	if (pAd->OpMode == OPMODE_AP)
	{
		/* update PHY mode and rate*/
		if (pAd->CommonCfg.Channel > 14)
			ProtCfg.field.ProtectRate = 0x4000;
		ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;	
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		// Decide Protect Rate for Legacy packet
		if (pAd->CommonCfg.Channel > 14)
		{
			ProtCfg.field.ProtectRate = 0x4000; // OFDM 6Mbps
		}
		else 
		{
			ProtCfg.field.ProtectRate = 0x0000; // CCK 1Mbps
			if (pAd->CommonCfg.MinTxRate > RATE_11)
				ProtCfg.field.ProtectRate |= 0x4000; // OFDM 6Mbps
		}
	}

	/* Handle legacy(B/G) protection*/
	if (bDisableBGProtect)
	{
		/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
		ProtCfg.field.ProtectCtrl = 0;
		Protect[0] = ProtCfg.word;
		Protect[1] = ProtCfg.word;
		pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
	}
	else
	{
		/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
		ProtCfg.field.ProtectCtrl = 0;			/* CCK do not need to be protected*/
		Protect[0] = ProtCfg.word;
		ProtCfg.field.ProtectCtrl = ASIC_CTS;	/* OFDM needs using CCK to protect*/
		Protect[1] = ProtCfg.word;
		pAd->FlgCtsEnabled = 1; /* CTS-self is used */
	}

#ifdef DOT11_N_SUPPORT
	/* Decide HT frame protection.*/
	if ((SetMask & ALLN_SETPROTECT) != 0)
	{
		switch(OperationMode)
		{
			case 0x0:
				/* NO PROTECT */
				/* 1.All STAs in the BSS are 20/40 MHz HT*/
				/* 2. in ai 20/40MHz BSS*/
				/* 3. all STAs are 20MHz in a 20MHz BSS*/
				/* Pure HT. no protection.*/

				/* MM20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[2] = 0x01744004;	

				/* MM40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None) */
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[3] = 0x03f44084;

				/* CF20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[4] = 0x01744004;

				/* CF40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[5] = 0x03f44084;

				if (bNonGFExist)
				{
					/* PROT_NAV(19:18)  -- 01 (Short NAV protectiion)*/
					/* PROT_CTRL(17:16) -- 01 (RTS/CTS)*/
					Protect[4] = 0x01754004;
					Protect[5] = 0x03f54084;
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
				break;
				
 			case 1:
				/* This is "HT non-member protection mode."*/
				/* If there may be non-HT STAs my BSS*/
				ProtCfg.word = 0x01744004;	/* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				}
				/*Assign Protection method for 20&40 MHz packets*/
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;
				
			case 2:
				/* If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets*/
				ProtCfg.word = 0x01744004;  /* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				} 
				/*Assign Protection method for 40MHz packets*/
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				if (bNonGFExist)
				{
					ProtCfg.field.ProtectCtrl = ASIC_RTS;
					ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				}
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;

				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
				break;
				
			case 3:
				/* HT mixed mode.	 PROTECT ALL!*/
				/* Assign Rate*/
				ProtCfg.word = 0x01744004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f44084;
				/* both 20MHz and 40MHz are protected. Whether use RTS or CTS-to-self depends on the*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}
				/*Assign Protection method for 20&40 MHz packets*/
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;	
				
			case 8:
				/* Special on for Atheros problem n chip.*/
				ProtCfg.word = 0x01754004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f54084;
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01750003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f50003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}
				
				Protect[2] = ProtCfg.word; 	/*0x01754004;*/
				Protect[3] = ProtCfg4.word; /*0x03f54084;*/
				Protect[4] = ProtCfg.word; 	/*0x01754004;*/
				Protect[5] = ProtCfg4.word; /*0x03f54084;*/
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;		
		}
	}
#endif /* DOT11_N_SUPPORT */
	
	offset = CCK_PROT_CFG;
	for (i = 0;i < 6;i++)
	{
		if ((SetMask & (1<< i)))
		{
			RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);
		}
	}
}


VOID AsicBBPAdjust(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_ASIC_BBP_ADJUST(pAd);

}

	
/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSwitchChannel(
					  IN PRTMP_ADAPTER pAd, 
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan) 
{
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

			if( (RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == 1)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("AsicSwitchChannel: autopm_resume success\n"));
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
			}
			else if ((RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == (-1))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel autopm_resume fail ------\n"));
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
				return;
			}
			else
				DBGPRINT(RT_DEBUG_TRACE, ("AsicSwitchChannel: autopm_resume do nothing \n"));

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	RTMP_CHIP_ASIC_SWITCH_CHANNEL(pAd, Channel, bScan);

}

/*
	==========================================================================
	Description:
		This function is required for 2421 only, and should not be used during
		site survey. It's only required after NIC decided to stay at a channel
		for a longer period.
		When this function is called, it's always after AsicSwitchChannel().

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Channel) 
{
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */

VOID InitRfPaModeTable(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 mac_value;
	UCHAR bit, pa_value;

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG0, &mac_value);
	
	for (bit = 0; bit < 8; bit += 2) /* CCK */
	{
		pa_value = (UCHAR)((mac_value >> bit) & (0x03));
		pAd->rf_pa_mode_over_cck[bit/2] = pa_value;
	}

	for (bit = 8; bit < 24; bit += 2) /* OFDM */
	{
		pa_value = (UCHAR)((mac_value >> bit) & (0x03));
		pAd->rf_pa_mode_over_ofdm[(bit - 8)/2] = pa_value;
	}
		
	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG1, &mac_value);
	
	for (bit = 0; bit < 32; bit += 2) /* HT */
	{
		pa_value = (UCHAR)((mac_value >> bit) & (0x03));
		pAd->rf_pa_mode_over_ht[bit/2] = pa_value;
	}
}

VOID AsicGetTxPowerOffset(
	IN PRTMP_ADAPTER 			pAd,
	IN PULONG 					TxPwr)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;
	DBGPRINT(RT_DEBUG_INFO, ("-->AsicGetTxPowerOffset\n"));

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	CfgOfTxPwrCtrlOverMAC.NumOfEntries = 5; /* MAC 0x1314, 0x1318, 0x131C, 0x1320 and 1324 */

	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		if (pAd->CommonCfg.CentralChannel > 14)
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgABand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgABand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgABand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgABand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgABand[4];
		}
		else
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgGBand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[4];
		}
	}
	else
	{
		if (pAd->CommonCfg.CentralChannel > 14)
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgABand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgABand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgABand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgABand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgABand[4];
		}
		else
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgGBand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgGBand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgGBand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgGBand[4];
		}
	}

	NdisCopyMemory(TxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	DBGPRINT(RT_DEBUG_INFO, ("<--AsicGetTxPowerOffset\n"));
}

VOID AsicGetAutoAgcOffsetForExternalTxAlc(
	IN PRTMP_ADAPTER 			pAd,
	IN PCHAR 					pDeltaPwr,
	IN PCHAR 					pTotalDeltaPwr,
	IN PCHAR 					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1)
{
	BBP_R49_STRUC	BbpR49;
	BOOLEAN			bAutoTxAgc = FALSE;
	UCHAR			TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep, idx;
	PCHAR			pTxAgcCompensate = NULL;
	CHAR    			DeltaPwr = 0;

	DBGPRINT(RT_DEBUG_INFO, ("-->%s\n", __FUNCTION__));

	BbpR49.byte = 0;

	/* TX power compensation for temperature variation based on TSSI. Try every 4 second */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			/* bg channel */
			bAutoTxAgc = pAd->bAutoTxAgcG;
			TssiRef = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep = pAd->TxAgcStepG;
			pTxAgcCompensate = &pAd->TxAgcCompensateG;
		}
		else
		{
			/* a channel */
			bAutoTxAgc = pAd->bAutoTxAgcA;
			TssiRef = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary = &pAd->TssiPlusBoundaryA[0];
			TxAgcStep = pAd->TxAgcStepA;
			pTxAgcCompensate = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);

			/* TSSI representation */
			if (IS_RT3071(pAd) || IS_RT3390(pAd) || IS_RT3090A(pAd) || IS_RT3572(pAd)) /* 5-bits */
			{
				BbpR49.byte = (BbpR49.byte & 0x1F);
			}
				
			/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
			/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
			/* step value is defined in pAd->TxAgcStepG for tx power value */

			/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
			/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
			    above value are examined in mass factory production */
			/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

			/* plus (+) is 0x00 ~ 0x45, minus (-) is 0xa0 ~ 0xf0 */
			/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
			/* if value is 0xa5, tx power will be -= TxAgcStep*(2-1) */

			if (BbpR49.byte > pTssiMinusBoundary[1])
			{
				/* Reading is larger than the reference value */
				/* Check for how large we need to decrease the Tx power */
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49.byte <= pTssiMinusBoundary[idx])  /* Found the range */
						break;
				}
				/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
#ifdef RT3883
				if (IS_RT3883(pAd))
				{
					if ((idx == 5) && ((BbpR49.byte) >  pTssiMinusBoundary[4] + 8))
						idx += 1;
				}
#endif /* RT3883 */

				*pTxAgcCompensate = -(TxAgcStep * (idx-1));			
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					                BbpR49.byte, TssiRef, TxAgcStep, idx-1));                    
			}
			else if (BbpR49.byte < pTssiPlusBoundary[1])
			{
				/* Reading is smaller than the reference value */
				/* Check for how large we need to increase the Tx power */
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49.byte >= pTssiPlusBoundary[idx])   /* Found the range*/
						break;
				}
#ifdef RT3883
				if (IS_RT3883(pAd) && (idx == 5))
				{
					if ((BbpR49.byte) <  (pTssiPlusBoundary[4] - 16))
						idx += 2;
					else if ((BbpR49.byte) <  (pTssiPlusBoundary[4] - 8))
						idx += 1;
				}
#endif /* RT3883 */

				/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
				                	BbpR49.byte, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
				                	BbpR49.byte, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			bAutoTxAgc = pAd->bAutoTxAgcG;
			pTxAgcCompensate = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc = pAd->bAutoTxAgcA;
			pTxAgcCompensate = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

#ifdef RT3883
	if (IS_RT3883(pAd) && (pAd->bTxPwrRangeExt))
		DeltaPwr -= 2;
#endif /* RT3883 */

	*pDeltaPwr = DeltaPwr;
	*pAgcCompensate = *pTxAgcCompensate;

	DBGPRINT(RT_DEBUG_INFO, ("<--%s\n", __FUNCTION__));
}

#ifdef RTMP_TEMPERATURE_COMPENSATION
VOID InitLookupTable(
	IN PRTMP_ADAPTER pAd)
{
	int Idx, IdxTmp;
	int i;
	enum IEEE80211_BAND band;
	int band_nums = 1;
	const int Offset = 7;
	EEPROM_WORD_STRUC WordStruct = {{0}};
	UCHAR PlusStepNum[IEEE80211_BAND_NUMS][8] = {{0, 1, 3, 2, 3, 3, 3, 2}, {0, 1, 3, 2, 3, 3, 3, 2}};
	UCHAR MinusStepNum[IEEE80211_BAND_NUMS][8] = {{1, 1, 1, 1, 1, 1, 0, 1}, {1, 1, 1, 1, 1, 1, 0, 1}};
	UCHAR Step[IEEE80211_BAND_NUMS] = {10, 10};
	UCHAR RFValue = 0, BbpValue = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("==> InitLookupTable\n"));
	
	/* Read from EEPROM, as parameters for lookup table for G band */
	RT28xx_EEPROM_READ16(pAd, 0x6e, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] EEPROM 6e = %x\n", WordStruct.word));
	PlusStepNum[IEEE80211_BAND_2G][0] = (WordStruct.field.Byte0 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][2] = (WordStruct.field.Byte1 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x70, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] EEPROM 70 = %x\n", WordStruct.word));
	PlusStepNum[IEEE80211_BAND_2G][4] = (WordStruct.field.Byte0 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][6] = (WordStruct.field.Byte1 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x72, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] EEPROM 72 = %x\n", WordStruct.word));
	MinusStepNum[IEEE80211_BAND_2G][0] = (WordStruct.field.Byte0 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][2] = (WordStruct.field.Byte1 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x74, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4] EEPROM 74 = %x\n", WordStruct.word));
	MinusStepNum[IEEE80211_BAND_2G][4] = (WordStruct.field.Byte0 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][6] = (WordStruct.field.Byte1 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x76, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] EEPROM 76 = %x\n", WordStruct.word));
	pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_2G] = (WordStruct.field.Byte0 & 0x0F);
	Step[IEEE80211_BAND_2G] = (WordStruct.field.Byte0 >> 4);
	pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G] = (CHAR)WordStruct.field.Byte1;

	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] Plus = %u %u %u %u %u %u %u %u\n",
		PlusStepNum[IEEE80211_BAND_2G][0],
		PlusStepNum[IEEE80211_BAND_2G][1],
		PlusStepNum[IEEE80211_BAND_2G][2],
		PlusStepNum[IEEE80211_BAND_2G][3],
		PlusStepNum[IEEE80211_BAND_2G][4],
		PlusStepNum[IEEE80211_BAND_2G][5],
		PlusStepNum[IEEE80211_BAND_2G][6],
		PlusStepNum[IEEE80211_BAND_2G][7]
		));
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] Minus = %u %u %u %u %u %u %u %u\n",
		MinusStepNum[IEEE80211_BAND_2G][0],
		MinusStepNum[IEEE80211_BAND_2G][1],
		MinusStepNum[IEEE80211_BAND_2G][2],
		MinusStepNum[IEEE80211_BAND_2G][3],
		MinusStepNum[IEEE80211_BAND_2G][4],
		MinusStepNum[IEEE80211_BAND_2G][5],
		MinusStepNum[IEEE80211_BAND_2G][6],
		MinusStepNum[IEEE80211_BAND_2G][7]
		));
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] tssi gain/step = %u\n", pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_2G]));
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4] Step = %u\n", Step[IEEE80211_BAND_2G]));
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4] RefTemp_2G = %d\n", pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G]));

#ifdef A_BAND_SUPPORT
	if (RFIC_IS_5G_BAND(pAd))
	{
		/* Read from EEPROM, as parameters for lookup table for A band */
		RT28xx_EEPROM_READ16(pAd, 0xd4, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] EEPROM d4 = %x\n", WordStruct.word));
		PlusStepNum[IEEE80211_BAND_5G][0] = (WordStruct.field.Byte0 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][2] = (WordStruct.field.Byte1 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xd6, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] EEPROM d6 = %x\n", WordStruct.word));
		PlusStepNum[IEEE80211_BAND_5G][4] = (WordStruct.field.Byte0 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][6] = (WordStruct.field.Byte1 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xd8, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] EEPROM d8 = %x\n", WordStruct.word));
		MinusStepNum[IEEE80211_BAND_5G][0] = (WordStruct.field.Byte0 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][2] = (WordStruct.field.Byte1 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xda, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] EEPROM da = %x\n", WordStruct.word));
		MinusStepNum[IEEE80211_BAND_5G][4] = (WordStruct.field.Byte0 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][6] = (WordStruct.field.Byte1 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xdc, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] EEPROM dc = %x\n", WordStruct.word));
		pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_5G] = (WordStruct.field.Byte0 & 0x0F);
		Step[IEEE80211_BAND_5G] = (WordStruct.field.Byte0 >> 4);
		pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G] = (CHAR)WordStruct.field.Byte1;

		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] Plus = %u %u %u %u %u %u %u %u\n",
			PlusStepNum[IEEE80211_BAND_5G][0],
			PlusStepNum[IEEE80211_BAND_5G][1],
			PlusStepNum[IEEE80211_BAND_5G][2],
			PlusStepNum[IEEE80211_BAND_5G][3],
			PlusStepNum[IEEE80211_BAND_5G][4],
			PlusStepNum[IEEE80211_BAND_5G][5],
			PlusStepNum[IEEE80211_BAND_5G][6],
			PlusStepNum[IEEE80211_BAND_5G][7]
			));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] Minus = %u %u %u %u %u %u %u %u\n",
			MinusStepNum[IEEE80211_BAND_5G][0],
			MinusStepNum[IEEE80211_BAND_5G][1],
			MinusStepNum[IEEE80211_BAND_5G][2],
			MinusStepNum[IEEE80211_BAND_5G][3],
			MinusStepNum[IEEE80211_BAND_5G][4],
			MinusStepNum[IEEE80211_BAND_5G][5],
			MinusStepNum[IEEE80211_BAND_5G][6],
			MinusStepNum[IEEE80211_BAND_5G][7]
			));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] tssi gain/step = %u\n", pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_5G]));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] Step = %u\n", Step[IEEE80211_BAND_5G]));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] RefTemp_2G = %d\n", pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G]));

		band_nums = IEEE80211_BAND_NUMS;
	}
#endif /* A_BAND_SUPPORT */


	for (band = IEEE80211_BAND_2G; band < band_nums; band++)
	{
		/* positive */
		i = 0;
		IdxTmp = 1;

		pAd->TxPowerCtrl.LookupTable[band][1 + Offset] = Step[band] / 2;
		pAd->TxPowerCtrl.LookupTable[band][0 + Offset] = pAd->TxPowerCtrl.LookupTable[band][1 + Offset] - Step[band];
		for (Idx = 2; Idx < 26;)/* Idx++ )*/
		{
			if (PlusStepNum[band][i] != 0 || i >= 8)
			{
				if (Idx >= IdxTmp + PlusStepNum[band][i] && i < 8)
				{
					pAd->TxPowerCtrl.LookupTable[band][Idx + Offset] = pAd->TxPowerCtrl.LookupTable[band][Idx - 1 + Offset] + (Step[band] - (i+1) + 1);
					IdxTmp = IdxTmp + PlusStepNum[band][i];
					i += 1;
				}
				else
				{
					pAd->TxPowerCtrl.LookupTable[band][Idx + Offset] = pAd->TxPowerCtrl.LookupTable[band][Idx - 1 + Offset] + (Step[band] - (i+1) + 1);
				}
				Idx++;
			}
			else
			{
				i += 1;
			}
		}

		/* negative */
		i = 0;
		IdxTmp = 1;
		for (Idx = 1; Idx < 8;)/* Idx++ )*/
		{
			if (MinusStepNum[band][i] != 0 || i >= 8)
			{
				if ((Idx + 1) >= IdxTmp + MinusStepNum[band][i] && i < 8)
				{
					pAd->TxPowerCtrl.LookupTable[band][-Idx + Offset] = pAd->TxPowerCtrl.LookupTable[band][-Idx + 1 + Offset] - (Step[band] + (i+1) - 1);
					IdxTmp = IdxTmp + MinusStepNum[band][i];
					i += 1;
				}
				else
				{
					pAd->TxPowerCtrl.LookupTable[band][-Idx + Offset] = pAd->TxPowerCtrl.LookupTable[band][-Idx + 1 + Offset] - (Step[band] + (i+1) - 1);
				}
				Idx++;
			}
			else
			{
				i += 1;
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Lookup table as below:\n"));
		for (Idx = 0; Idx < 33; Idx++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation band(%d)] %d, %d\n", band, Idx - Offset, pAd->TxPowerCtrl.LookupTable[band][Idx]));
		}
	}
	
	/* Set BBP_R47 */
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpValue);

	/* bit3 = 0 */
	BbpValue = (BbpValue & 0xf7);

	/* bit7 = 1, bit4 = 0 */
	BbpValue = (BbpValue | 0x80);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpValue);
	
	/*  Set RF_R27 */
	RT30xxReadRFRegister(pAd, RF_R27, &RFValue);

	/* Set [7:6] to 01. For method 2, it is set at initialization. */
	RFValue = (RFValue & 0x7f);
	RFValue = (RFValue | 0x40);

	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Set RF_R27 to 0x%x\n", RFValue));
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
}

VOID AsicGetAutoAgcOffsetForTemperatureSensor(
	IN PRTMP_ADAPTER 		pAd,
	IN PCHAR				pDeltaPwr,
	IN PCHAR				pTotalDeltaPwr,
	IN PCHAR				pAgcCompensate,
	IN PCHAR 				pDeltaPowerByBbpR1)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable;
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry0 = NULL; /* Ant0 */
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry1 = NULL; /* Ant1 */
	BBP_R49_STRUC	BbpR49;
	BOOLEAN			bAutoTxAgc = FALSE;
	PCHAR			pTxAgcCompensate = NULL;
	UCHAR 			RFValue = 0;
	CHAR			TuningTableUpperBound = 0, TuningTableIndex0 = 0, TuningTableIndex1 = 0;
	INT 				CurrentTemp = 0;
	INT RefTemp;
	INT *LookupTable;
	INT	LookupTableIndex = pAd->TxPowerCtrl.LookupTableIndex + TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET;

	DBGPRINT(RT_DEBUG_INFO, ("-->%s\n", __FUNCTION__));
	
	BbpR49.byte = 0;
	*pTotalDeltaPwr = 0;

#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
	{
		/* a band channel */
		bAutoTxAgc = pAd->bAutoTxAgcA;
		pTxAgcCompensate = &pAd->TxAgcCompensateA;
		TxPowerTuningTable = pChipCap->TxPowerTuningTable_5G;
		RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G];
		LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_5G][0];
		TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_5G;
	}
	else
#endif /* A_BAND_SUPPORT */
	{
		/* bg band channel */
		bAutoTxAgc = pAd->bAutoTxAgcG;
		pTxAgcCompensate = &pAd->TxAgcCompensateG;
		TxPowerTuningTable = pChipCap->TxPowerTuningTable_2G;
		RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G];
		LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_2G][0];
		TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_2G;
	}

	/* AutoTxAgc in EEPROM means temperature compensation enabled/diablded. */
	if (bAutoTxAgc)
	{ 
		/* Current temperature */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
		CurrentTemp = (CHAR)BbpR49.byte;
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] BBP_R49 = %02x, current temp = %d\n", BbpR49.byte, CurrentTemp));
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] RefTemp = %d\n", RefTemp));
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] index = %d\n", pAd->TxPowerCtrl.LookupTableIndex));
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex - 1, LookupTable[LookupTableIndex - 1]));
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex, LookupTable[LookupTableIndex]));
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex + 1, LookupTable[LookupTableIndex + 1]));
		if (CurrentTemp > RefTemp + LookupTable[LookupTableIndex + 1] + ((LookupTable[LookupTableIndex + 1] - LookupTable[LookupTableIndex]) >> 2) &&
			LookupTableIndex < 32)
		{
			DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] ++\n"));
			LookupTableIndex++;
			pAd->TxPowerCtrl.LookupTableIndex++;
		}
		else if (CurrentTemp < RefTemp + LookupTable[LookupTableIndex] - ((LookupTable[LookupTableIndex] - LookupTable[LookupTableIndex - 1]) >> 2) &&
			LookupTableIndex > 0)
		{
			DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] --\n"));
			LookupTableIndex--;
			pAd->TxPowerCtrl.LookupTableIndex--;
		}
		else
		{
			DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] ==\n"));
		}

		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] idxTxPowerTable=%d, idxTxPowerTable2=%d, TuningTableUpperBound=%d\n",
			pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPowerCtrl.LookupTableIndex,
			pAd->TxPowerCtrl.idxTxPowerTable2 + pAd->TxPowerCtrl.LookupTableIndex,
			TuningTableUpperBound));

		TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable 
									+ pAd->TxPowerCtrl.LookupTableIndex 
#ifdef DOT11_N_SUPPORT
									+ pAd->TxPower[pAd->CommonCfg.CentralChannel-1].Power;
#else
									+ pAd->TxPower[pAd->CommonCfg.Channel-1].Power;
#endif /* DOT11_N_SUPPORT */
		/* The boundary verification */ 
		TuningTableIndex0 = (TuningTableIndex0 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex0;
		TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex0;
		TxPowerTuningTableEntry0 = (TX_POWER_TUNING_ENTRY_STRUCT *)&TxPowerTuningTable[TuningTableIndex0 + TX_POWER_TUNING_ENTRY_OFFSET];
		
		TuningTableIndex1 = pAd->TxPowerCtrl.idxTxPowerTable2 
									+ pAd->TxPowerCtrl.LookupTableIndex 
#ifdef DOT11_N_SUPPORT				
									+ pAd->TxPower[pAd->CommonCfg.CentralChannel-1].Power2;
#else
									+ pAd->TxPower[pAd->CommonCfg.Channel-1].Power2;
#endif /* DOT11_N_SUPPORT */
		/* The boundary verification */
		TuningTableIndex1 = (TuningTableIndex1 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex1;
		TuningTableIndex1 = (TuningTableIndex1 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex1;
		TxPowerTuningTableEntry1 = (TX_POWER_TUNING_ENTRY_STRUCT *)&TxPowerTuningTable[TuningTableIndex1 + TX_POWER_TUNING_ENTRY_OFFSET];
			
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] (tx0)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry0->RF_TX_ALC, TxPowerTuningTableEntry0->MAC_PowerDelta, TuningTableIndex0));
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] (tx1)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry1->RF_TX_ALC, TxPowerTuningTableEntry1->MAC_PowerDelta, TuningTableIndex1));

		/* Update RF_R49 [0:5] */
		RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
		RFValue = ((RFValue & ~0x3F) | TxPowerTuningTableEntry0->RF_TX_ALC);
		if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
		{
			RFValue = ((RFValue & ~0x3F) | 0x27);
		}
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] Update RF_R49[0:5] to 0x%x\n", TxPowerTuningTableEntry0->RF_TX_ALC));
		RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

		/* Update RF_R50 [0:5] */
		RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
		RFValue = ((RFValue & ~0x3F) | TxPowerTuningTableEntry1->RF_TX_ALC);
		if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
		{
			RFValue = ((RFValue & ~0x3F) | 0x27);
		}
		DBGPRINT(RT_DEBUG_INFO, ("[temp. compensation] Update RF_R50[0:5] to 0x%x\n", TxPowerTuningTableEntry1->RF_TX_ALC));
		RT30xxWriteRFRegister(pAd, RF_R50, RFValue);
		
		*pTotalDeltaPwr = TxPowerTuningTableEntry0->MAC_PowerDelta;
		
	}

	*pAgcCompensate = *pTxAgcCompensate;
	DBGPRINT(RT_DEBUG_INFO, ("<--%s\n", __FUNCTION__));
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN 		PRTMP_ADAPTER 	pAd,
	IN 		PCHAR 			pTotalDeltaPower,
	INOUT 	PULONG			pSingleSKUTotalDeltaPwr,
	INOUT  	PUCHAR              	pSingleSKUBbpR1Offset) 
{
	INT		i, j;
	CHAR	Value;
	CHAR 	MinValue = 127;
	UCHAR	BbpR1 = 0;
	UCHAR  	TxPwrInEEPROM = 0xFF, CountryTxPwr = 0xFF, criterion;
	UCHAR   	AdjustMaxTxPwr[(MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS * 8)]; 
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};
	
	/* Get TX rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);
		
	/* Handle regulatory max. TX power constraint */
	if (pAd->CommonCfg.Channel > 14) 
	{
		TxPwrInEEPROM = ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF00) >> 8); /* 5G band */
	}
	else 
	{
		TxPwrInEEPROM = (pAd->CommonCfg.DefineMaxTxPwr & 0x00FF); /* 2.4G band */
	}

	CountryTxPwr = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel); 

	/* Use OFDM 6M as the criterion */
	criterion = (UCHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue & 0x000F0000) >> 16);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: criterion=%d, TxPwrInEEPROM=%d, CountryTxPwr=%d\n", 
		__FUNCTION__, criterion, TxPwrInEEPROM, CountryTxPwr));

	/* Adjust max. TX power according to the relationship of TX power in EEPROM */
	for (i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (i == 0)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); 

				if (j < 4)
				{
					AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion) + 4; /* CCK has 4dBm larger than OFDM */
				}
				else
				{
					AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion);
				}

				DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, (Default)Value=%d, %d\n", 
					__FUNCTION__,
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset,
					i, 
					j, 
					Value, 
					AdjustMaxTxPwr[i*8+j]));
			}
		}
		else
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

				AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion);

				DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, (Default)Value=%d, %d\n", 
					__FUNCTION__,
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
					i, 
					j, 
					Value, 
					AdjustMaxTxPwr[i*8+j]));
			}
		}
	}

	/* Adjust TX power according to the relationship */
	for (i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

				/* The TX power is larger than the regulatory, the power should be restrained */
				if (AdjustMaxTxPwr[i*8+j] > CountryTxPwr)
				{
					Value = (AdjustMaxTxPwr[i*8+j] - CountryTxPwr);
					
					if (Value > 0xF)
					{
						/* The output power is larger than Country Regulatory over 15dBm, the origianl design has overflow case */
						DBGPRINT(RT_DEBUG_ERROR,("%s: Value overflow - %d\n", __FUNCTION__, Value));
					}

						*(pSingleSKUTotalDeltaPwr+i) = (*(pSingleSKUTotalDeltaPwr+i) & ~(0x0000000F << j*4)) | (Value << j*4);

					DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, (Exceed)Value=%d, %d\n", 
						__FUNCTION__,
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
						i, 
						j, 
						Value, 
						AdjustMaxTxPwr[i*8+j]));
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, Value=%d, %d, no change\n",
						__FUNCTION__,
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
						i, 
						j, 
						Value, 
						AdjustMaxTxPwr[i*8+j]));
				}
			}
		}
	}

	/* Calculate the min. TX power */
	for(i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				CHAR PwrChange;
				/* 
				   After Single SKU, each data rate offset power value is saved in TotalDeltaPwr[].
				   PwrChange will add SingleSKUDeltaPwr and TotalDeltaPwr[] for each data rate to calculate
				   the final adjust output power value which is saved in MAC Reg. and BBP_R1.
				*/
				
				/*   
				   Value / TxPwr[] is get from eeprom 0xDEh ~ 0xEFh and increase or decrease the  
				   20/40 Bandwidth Delta Value in eeprom 0x50h. 
				*/
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); /* 0 ~ 15 */

				/* Fix the corner case of Single SKU read eeprom offset 0xF0h ~ 0xFEh which for BBP Instruction configuration */
					if (Value == 0xF)
						continue;

				/* Value_offset is current Pwr comapre with Country Regulation and need adjust delta value */
					PwrChange = (CHAR)((*(pSingleSKUTotalDeltaPwr+i) >> j*4) & 0x0F); /* 0 ~ 15 */
				PwrChange -= *pTotalDeltaPower;

				Value -= PwrChange;
				
				if (MinValue > Value)
					MinValue = Value;				
			}
		}
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	/* Depend on the min. TX power to adjust and prevent the value of MAC_TX_PWR_CFG less than 0 */
	if ((MinValue < 0) && (MinValue >= -6))
	{
		BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;
		*pSingleSKUBbpR1Offset = 6;
	}
	else if ((MinValue < -6)&&(MinValue >= -12))
	{
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;
		*pSingleSKUBbpR1Offset = 12;
	}
	else if (MinValue < -12)
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s: ASIC limit..\n", __FUNCTION__));
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;
		*pSingleSKUBbpR1Offset = 12;
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: <After BBP R1> TotalDeltaPower = %d dBm, BbpR1 = 0x%02X \n", __FUNCTION__, *pTotalDeltaPower, BbpR1));
}
#endif /* SINGLE_SKU */

VOID AsicPercentageDeltaPower(
	IN 		PRTMP_ADAPTER 		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1) 
{
	/* 
		Calculate delta power based on the percentage specified from UI.
		E2PROM setting is calibrated for maximum TX power (i.e. 100%).
		We lower TX power here according to the percentage specified from UI.
	*/
	
	if (pAd->CommonCfg.TxPowerPercentage >= 100) /* AUTO TX POWER control */
	{
#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA)
#ifdef P2P_SUPPORT
			&& (!P2P_GO_ON(pAd))
#endif /* P2P_SUPPORT */
		)
		{
			/* To patch high power issue with some APs, like Belkin N1.*/
			if (Rssi > -35)
			{
				*pDeltaPwr -= 12;
			}
			else if (Rssi > -40)
			{
				*pDeltaPwr -= 6;
			}
			else
				;
		}
#endif /* CONFIG_STA_SUPPORT */
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 90) /* 91 ~ 100% & AUTO, treat as 100% in terms of mW */
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60) /* 61 ~ 90%, treat as 75% in terms of mW		 DeltaPwr -= 1; */
	{
		*pDeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30) /* 31 ~ 60%, treat as 50% in terms of mW		 DeltaPwr -= 3; */
	{
		*pDeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15) /* 16 ~ 30%, treat as 25% in terms of mW		 DeltaPwr -= 6; */
	{
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9) /* 10 ~ 15%, treat as 12.5% in terms of mW		 DeltaPwr -= 9; */
	{
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
		*pDeltaPwr -= 3;
	}
	else /* 0 ~ 9 %, treat as MIN(~3%) in terms of mW		 DeltaPwr -= 12; */
	{
		*pDeltaPowerByBbpR1 -= 12; /* -12 dBm */
	}
}

VOID AsicCompensatePowerViaBBP(
	IN 		PRTMP_ADAPTER 		pAd,
	INOUT	PCHAR				pTotalDeltaPower) 
{
	UCHAR	BbpR1 = 0;
	
	DBGPRINT(RT_DEBUG_INFO, ("%s: <Before BBP R1> TotalDeltaPower = %d dBm\n", __FUNCTION__, *pTotalDeltaPower));

	/* The BBP R1 controls the transmit power for all rates */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;

	if (*pTotalDeltaPower <= -12)
	{
		*pTotalDeltaPower += 12;
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

		DBGPRINT(RT_DEBUG_INFO, ("%s: Drop the transmit power by 12 dBm (BBP R1)\n", __FUNCTION__));
	}
	else if ((*pTotalDeltaPower <= -6) && (*pTotalDeltaPower > -12))
	{
		*pTotalDeltaPower += 6;
		BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;		

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

		DBGPRINT(RT_DEBUG_INFO, ("%s: Drop the transmit power by 6 dBm (BBP R1)\n", __FUNCTION__));
	}
	else
	{
		/* Control the the transmit power by using the MAC only */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: <After BBP R1> TotalDeltaPower = %d dBm, BbpR1 = 0x%02X \n", __FUNCTION__, *pTotalDeltaPower, BbpR1));
}

/*
	==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in LINK UP in INFRASTRUCTURE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRatSwitching()
	==========================================================================
 */

VOID AsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	INT			i, j;
	CHAR 		Value;
	CHAR		Rssi = -127;
	CHAR		DeltaPwr = 0;
	CHAR		TxAgcCompensate = 0;
	CHAR		DeltaPowerByBbpR1 = 0; 
	CHAR		TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */
#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT3352
	CHAR		TotalDeltaPower2 = 0, Value2 = 0;
	BOOLEAN bTX1 = FALSE;
#endif /* RT3352 */
#endif /* RTMP_INTERNAL_TX_ALC */
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};	
#ifdef SINGLE_SKU
	CHAR		TotalDeltaPowerOri = 0;
	UCHAR		SingleSKUBbpR1Offset = 0;
	ULONG		SingleSKUTotalDeltaPwr[MAX_TXPOWER_ARRAY_SIZE] = {0};
#endif /* SINGLE_SKU */


#ifdef CONFIG_STA_SUPPORT
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) || 
#ifdef RTMP_MAC_PCI
		(pAd->bPCIclkOff == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) ||
#endif /* RTMP_MAC_PCI */
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		return;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if(INFRA_ON(pAd))
		{
			Rssi = RTMPMaxRssi(pAd, 
						   pAd->StaCfg.RssiSample.AvgRssi0, 
						   pAd->StaCfg.RssiSample.AvgRssi1, 
						   pAd->StaCfg.RssiSample.AvgRssi2);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Get Tx rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);
	/* Get temperature compensation delta power value */
	RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(
		pAd, &DeltaPwr, &TotalDeltaPower, &TxAgcCompensate, &DeltaPowerByBbpR1);

	DBGPRINT(RT_DEBUG_INFO, ("%s: DeltaPwr=%d, TotalDeltaPower=%d, TxAgcCompensate=%d, DeltaPowerByBbpR1=%d\n",
			__FUNCTION__,
			DeltaPwr,
			TotalDeltaPower,
			TxAgcCompensate,
			DeltaPowerByBbpR1));
		
#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT3352
	if (IS_RT3352(pAd) && (pAd->TxPowerCtrl.bInternalTxALC == TRUE))
	{
		TotalDeltaPower2 = pAd->TxPowerCtrl.TotalDeltaPower2;
	}

	if (IS_RT3352(pAd) && (pAd->TxPowerCtrl.bInternalTxALC == FALSE))
	{
	/* Get delta power based on the percentage specified from UI */
	AsicPercentageDeltaPower(pAd, Rssi, &DeltaPwr,&DeltaPowerByBbpR1);
	}
#endif /* RT3352 */
#endif /* RTMP_INTERNAL_TX_ALC */
#ifndef RT3352
	/* Get delta power based on the percentage specified from UI */
	AsicPercentageDeltaPower(pAd, Rssi, &DeltaPwr,&DeltaPowerByBbpR1);
#endif /* RT3352 */

	/* The transmit power controlled by the BBP */
	TotalDeltaPower += DeltaPowerByBbpR1; 
	/* The transmit power controlled by the MAC */
	TotalDeltaPower += DeltaPwr; 	

#ifdef SINGLE_SKU
	if (pAd->CommonCfg.bSKUMode == TRUE)
	{
		/* Re calculate delta power while enabling Single SKU */
		GetSingleSkuDeltaPower(pAd, &TotalDeltaPower, (PULONG)&SingleSKUTotalDeltaPwr, &SingleSKUBbpR1Offset);
	
		TotalDeltaPowerOri = TotalDeltaPower;
	}
	else
#endif /* SINGLE_SKU */
	{
#ifndef RT3352
		AsicCompensatePowerViaBBP(pAd, &TotalDeltaPower);
#endif /* RT3352 */
	}			

	/* Power will be updated each 4 sec. */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
#ifndef RT3352
/*****************************************************************************/
		/* Set new Tx power for different Tx rates */
		for (i=0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
		{
			TX_POWER_CONTROL_OVER_MAC_ENTRY *pTxPwrEntry;
			ULONG reg_val;

			pTxPwrEntry = &CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i];
			reg_val = pTxPwrEntry->RegisterValue;
			if (reg_val != 0xffffffff)
			{	
				for (j=0; j<8; j++)
				{
					CHAR _upbound, _lowbound, t_pwr;
					BOOLEAN _bValid;

					_lowbound = 0;
					_bValid = TRUE;

						Value = (CHAR)((reg_val >> j*4) & 0x0F);
#ifdef SINGLE_SKU
					if (pAd->CommonCfg.bSKUMode == TRUE)
					{
						TotalDeltaPower = SingleSKUBbpR1Offset + TotalDeltaPowerOri - (CHAR)((SingleSKUTotalDeltaPwr[i] >> j*4) & 0x0F);	

						DBGPRINT(RT_DEBUG_INFO, ("%s: BbpR1Offset(%d) + TX ALC(%d) - SingleSKU[%d/%d](%d) = TotalDeltaPower(%d)\n",
							__FUNCTION__, SingleSKUBbpR1Offset,
							TotalDeltaPowerOri, i, j,
							(CHAR)((SingleSKUTotalDeltaPwr[i] >> j*4) & 0x0F),
							TotalDeltaPower));
					}
#endif /* SINGLE_SKU */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
					/* The upper bounds of MAC 0x1314 ~ 0x1324 are variable */
					if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE)^(pAd->chipCap.bTempCompTxALC == TRUE))
					{
						switch (0x1314 + (i * 4))
						{
							case 0x1314: 
								_upbound = 0xe;
							break;

							case 0x1318: 
								_upbound = (j <= 3) ? 0xc : 0xe;
							break;

							case 0x131C: 
								_upbound = ((j == 0) || (j == 2) || (j == 3)) ? 0xc : 0xe;
							break;

							case 0x1320: 
								_upbound = (j == 1) ? 0xe : 0xc;
							break;

							case 0x1324: 
								_upbound = 0xc;
							break;

							default: 
							{
								/* do nothing */
								_bValid = FALSE;
								DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknown register = 0x%x\n", __FUNCTION__, (0x1314 + (i * 4))));
							}
							break;
						}
					}
					else
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */
#ifdef RT3883
					if (IS_RT3883(pAd))
						_upbound =  (pAd->NicConfig2.field.DynamicTxAgcControl) ? 0xf : 0xc;
					else
#endif /* RT3883 */
						_upbound = 0xc;

					if (_bValid)
					{
							t_pwr = Value + TotalDeltaPower;
							if (t_pwr < _lowbound)
								Value = _lowbound;
							else if (t_pwr > _upbound)
								Value = _upbound;
							else
								Value = t_pwr;
						}

					/* Fill new value into the corresponding MAC offset */
						pTxPwrEntry->RegisterValue = (reg_val & ~(0x0000000F << j*4)) | (Value << j*4);
				}

				RTMP_IO_WRITE32(pAd, pTxPwrEntry->MACRegisterOffset, pTxPwrEntry->RegisterValue);

			}
		}
#else /* specific for RT3352 */
/*****************************************************************************/
		/* Set new Tx power for different Tx rates */
		for (i=0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
		{
			if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
			{
				for (j=0; j<8; j++)
				{
					Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT3352
					/* Tx power adjustment over MAC */
					if (IS_RT3352(pAd) && (pAd->TxPowerCtrl.bInternalTxALC == TRUE))
					{
						if (j & 0x00000001) /* j=1, 3, 5, 7 */
						{
							/* TX1 ALC */
							Value2 = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); /* 0 ~ 15 */
							bTX1 = TRUE;
						}
						else /* j=0, 2, 4, 6 */
						{
							/* TX0 ALC */
							Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); /* 0 ~ 15 */
							bTX1 = FALSE;
						}
					}
#endif /* RT3352 */
					/* The upper bounds of the MAC 0x1314~0x1324 are variable when the STA uses the internal Tx ALC.*/
					if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE) && (pAd->Mlme.OneSecPeriodicRound % 4 == 0))
					{
						switch (TX_PWR_CFG_0 + (i * 4))
						{
							case TX_PWR_CFG_0: 
							{
								if (bTX1 == FALSE)
								{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
#ifdef RT3352
								/* Tx power adjustment over MAC */
								if (IS_RT3352(pAd) && (bTX1 == TRUE))
								{
									/* TX1 ALC */
									if ((Value2 + TotalDeltaPower2) < 0)
									{
										Value2 = 0;
									}
									else if ((Value2 + TotalDeltaPower2) > 0xE)
									{
										Value2 = 0xE;
									}
									else
									{
										Value2 += TotalDeltaPower2;
									}
								}
#endif /* RT3352 */
							}
							break;

							case TX_PWR_CFG_1: 
							{
								if (bTX1 == FALSE)
								{
								if ((j >= 0) && (j <= 3))
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)/* by HK 2011.04.06 */
									{
										Value = 0xE;/* by HK 2011.04.06 */
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
								else
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)
									{
										Value = 0xE;
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
							}
#ifdef RT3352
								/* Tx power adjustment over MAC */
								if (IS_RT3352(pAd) && (bTX1 == TRUE))
								{
									/* TX1 ALC */
									if ((j >= 0) && (j <= 3))
									{
										if ((Value2 + TotalDeltaPower2) < 0)
										{
											Value2 = 0;
										}
										else if ((Value2 + TotalDeltaPower2) > 0xE)/* by HK 2011.04.06 */
										{
											Value2 = 0xE;/* by HK 2011.04.06 */
										}
										else
										{
											Value2 += TotalDeltaPower2;
										}
									}
									else
									{
										if ((Value2 + TotalDeltaPower2) < 0)
										{
											Value2 = 0;
										}
										else if ((Value2 + TotalDeltaPower2) > 0xE)
										{
											Value2 = 0xE;
										}
										else
										{
											Value2 += TotalDeltaPower2;
										}
									}
								}
#endif /* RT3352 */
							}
							break;

							case TX_PWR_CFG_2: 
							{
								if (bTX1 == FALSE)
								{
								if ((j == 0) || (j == 2) || (j == 3))
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)/* by HK 2011.04.06 */
									{
										Value = 0xE;/* by HK 2011.04.06 */
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
								else
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)
									{
										Value = 0xE;
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
							}
#ifdef RT3352
								/* Tx power adjustment over MAC */
								if (IS_RT3352(pAd) && (bTX1 == TRUE))
								{
									/* TX1 ALC */
									if ((j == 0) || (j == 2) || (j == 3))
									{
										if ((Value2 + TotalDeltaPower2) < 0)
										{
											Value2 = 0;
										}
										else if ((Value2 + TotalDeltaPower2) > 0xE)/* by HK 2011.04.06 */
										{
											Value2 = 0xE;/* by HK 2011.04.06 */
										}
										else
										{
											Value2 += TotalDeltaPower2;
										}
									}
									else
									{
										if ((Value2 + TotalDeltaPower2) < 0)
										{
											Value2 = 0;
										}
										else if ((Value2 + TotalDeltaPower2) > 0xE)
										{
											Value2 = 0xE;
										}
										else
										{
											Value2 += TotalDeltaPower2;
										}
									}
								}
#endif /* RT3352 */
							}
							break;

							case TX_PWR_CFG_3: 
							{
								if (bTX1 == FALSE)
								{
								if ((j == 0) || (j == 2) || (j == 3) || 
								    ((j >= 4) && (j <= 7)))
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)/* by HK 2011.04.06 */
									{
										Value = 0xE;/* by HK 2011.04.06 */
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
								else
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)
									{
										Value = 0xE;
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
							}
#ifdef RT3352
								/* Tx power adjustment over MAC */
								if (IS_RT3352(pAd) && (bTX1 == TRUE))
								{
									/* TX1 ALC */
									if ((j == 0) || (j == 2) || (j == 3) || 
									((j >= 4) && (j <= 7)))
									{
										if ((Value2 + TotalDeltaPower2) < 0)
										{
											Value2 = 0;
										}
										else if ((Value2 + TotalDeltaPower2) > 0xE)/* by HK 2011.04.06 */
										{
											Value2 = 0xE;/* by HK 2011.04.06 */
										}
										else
										{
											Value2 += TotalDeltaPower2;
										}
									}
									else
									{
										if ((Value2 + TotalDeltaPower2) < 0)
										{
											Value2 = 0;
										}
										else if ((Value2 + TotalDeltaPower2) > 0xE)
										{
											Value2 = 0xE;
										}
										else
										{
											Value2 += TotalDeltaPower2;
										}
									}		
								}	
#endif /* RT3352 */
							}
							break;

							case TX_PWR_CFG_4: 
							{
								if (bTX1 == FALSE)
								{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)/* by HK 2011.04.06 */
								{
									Value = 0xE;/* by HK 2011.04.06 */
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
#ifdef RT3352
								/* Tx power adjustment over MAC */
								if (IS_RT3352(pAd) && (bTX1 == TRUE))
								{
									/* TX1 ALC */
									if ((Value2 + TotalDeltaPower2) < 0)
									{
										Value2 = 0;
									}
									else if ((Value2 + TotalDeltaPower2) > 0xE)/* by HK 2011.04.06 */
									{
										Value2 = 0xE;/* by HK 2011.04.06 */
									}
									else
									{
										Value2 += TotalDeltaPower2;
									}
								}
#endif /* RT3352 */
							}
							break;

							default: 
							{							
								/* do nothing*/
								DBGPRINT(RT_DEBUG_ERROR, ("%s: unknown register = 0x%X\n", 
									__FUNCTION__, 
									(TX_PWR_CFG_0 + (i * 4))));
							}
							break;
						}
					}
					else
#endif /* RTMP_INTERNAL_TX_ALC */
					{
						if ((Value + TotalDeltaPower) < 0)
						{
							Value = 0; /* min */
						}
						else if ((Value + TotalDeltaPower) > 0xC)
						{
							Value = 0xC; /* max */
						}
						else
						{
							Value += TotalDeltaPower; /* temperature compensation */
						}
					}

#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT3352
					/* fill new value to CSR offset */
					/* Tx power adjustment over MAC */
					if (IS_RT3352(pAd) && (pAd->TxPowerCtrl.bInternalTxALC == TRUE) && (pAd->Mlme.OneSecPeriodicRound % 4 == 0))
					{
						if (bTX1 == TRUE) /* j=1, 3, 5, 7 */
						{
							/* TX1 ALC */
							CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = 
								(CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value2 << j*4);
						}
						else /* j=0, 2, 4, 6 */
						{
							/* TX0 ALC */
							CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = 
								(CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value << j*4);
						}
					}
#else
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value << j*4);
#endif /* RT3352 */
					else
#endif /* RTMP_INTERNAL_TX_ALC */
					{
						/* TX0 ALC only */
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value << j*4);
					}
				}

				/* write tx power value to CSR */
				/* TX_PWR_CFG_0 (8 tx rate) for	TX power for OFDM 12M/18M
												TX power for OFDM 6M/9M
												TX power for CCK5.5M/11M
												TX power for CCK1M/2M */
				/* TX_PWR_CFG_1 ~ TX_PWR_CFG_4 */

				{
	/*				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, TxPwr[i]);*/
#ifdef RTMP_INTERNAL_TX_ALC
					if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE) && (pAd->Mlme.OneSecPeriodicRound % 4 == 0))
#endif /* RTMP_INTERNAL_TX_ALC */
					RTMP_IO_WRITE32(pAd, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue);
				}
			}
		}
/*****************************************************************************/
#endif /* !RT3352 */
		/* Extra set MAC registers to compensate Tx power if any */
		RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
	}

}

#ifdef THERMAL_PROTECT_SUPPORT
VOID thermal_protection(
	IN RTMP_ADAPTER 	*pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	INT32 temp_diff = 0, current_temp = 0;	

	current_temp = pChipOps->ChipGetCurrentTemp(pAd);
	temp_diff = current_temp - pAd->last_thermal_pro_temp;
	pAd->last_thermal_pro_temp = current_temp;

	DBGPRINT(RT_DEBUG_INFO, ("%s - current temp=%d, temp diff=%d\n", 
					__FUNCTION__, current_temp, temp_diff));
	
	if (temp_diff > 0) {
		if (current_temp > (pAd->thermal_pro_criteria + 10) /* 90 */)
			pChipOps->ThermalPro2ndCond(pAd);
		else if (current_temp > pAd->thermal_pro_criteria /* 80 */)
			pChipOps->ThermalPro1stCond(pAd);
		else
			pChipOps->ThermalProDefaultCond(pAd);
	} else if (temp_diff < 0) {
		if (current_temp < (pAd->thermal_pro_criteria - 5) /* 75 */)
			pChipOps->ThermalProDefaultCond(pAd);
		else if (current_temp < (pAd->thermal_pro_criteria + 5) /* 85 */)
			pChipOps->ThermalPro1stCond(pAd);
		else
			pChipOps->ThermalPro2ndCond(pAd);
	}
}
#endif /* THERMAL_PROTECT_SUPPORT */

VOID AsicResetBBPAgent(
IN PRTMP_ADAPTER pAd)
{
	BBP_CSR_CFG_STRUC	BbpCsr;

	/* Still need to find why BBP agent keeps busy, but in fact, hardware still function ok. Now clear busy first.	*/
	/* IF chipOps.AsicResetBbpAgent == NULL, run "else" part */
	RTMP_CHIP_ASIC_RESET_BBP_AGENT(pAd);
		else
		{
		DBGPRINT(RT_DEBUG_INFO, ("Reset BBP Agent busy bit.!! \n"));
	RTMP_IO_READ32(pAd, H2M_BBP_AGENT, &BbpCsr.word);
	BbpCsr.field.Busy = 0;
	RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
}
	
}
#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
		put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup 
		automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
		the wakeup timer timeout. Driver has to issue a separate command to wake
		PHY up.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp) 
{
	RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp);
}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever manual wakeup is required
		AsicForceSleep() should only be used when not in INFRA BSS. When
		in INFRA BSS, we should use AsicSleepThenAutoWakeup() instead.
	==========================================================================
 */
VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd)
{

}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever Twakeup timer (set via AsicSleepThenAutoWakeup)
		expired.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN    bFromTx)
{
    DBGPRINT(RT_DEBUG_INFO, ("--> AsicForceWakeup \n"));
    RTMP_STA_FORCE_WAKEUP(pAd, bFromTx);	
}
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetBssid(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pBssid) 
{
	ULONG		  Addr4;
#ifdef P2P_SUPPORT
	UINT32	regValue;
#endif /* P2P_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("==============> AsicSetBssid %x:%x:%x:%x:%x:%x\n",
		pBssid[0],pBssid[1],pBssid[2],pBssid[3], pBssid[4],pBssid[5]));
	
	Addr4 = (ULONG)(pBssid[0])		 | 
			(ULONG)(pBssid[1] << 8)  | 
			(ULONG)(pBssid[2] << 16) |
			(ULONG)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

	Addr4 = 0;
	/* always one BSSID in STA mode*/
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);

#ifdef P2P_SUPPORT
#ifdef P2P_ODD_MAC_ADJUST
	if ( (pAd->CurrentAddress[5] & 0x01 ) == 0x01 )
	{
		Addr4 |= (1 << 16 );
	}
#endif /* P2P_ODD_MAC_ADJUST */
#endif /* P2P_SUPPORT */

	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);
#ifdef P2P_SUPPORT
	if (P2P_INF_ON(pAd))
	{
		PUCHAR pP2PBssid = &pAd->CurrentAddress[0];

		Addr4 = (ULONG)(pP2PBssid[0])		 | 
				(ULONG)(pP2PBssid[1] << 8)  | 
				(ULONG)(pP2PBssid[2] << 16) |
				(ULONG)(pP2PBssid[3] << 24);
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

		Addr4 = 0;

		/* always one BSSID in STA mode */
		Addr4 = (ULONG)(pP2PBssid[4]) | (ULONG)(pP2PBssid[5] << 8);

		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

		RTMP_IO_READ32(pAd, MAC_BSSID_DW1, &regValue);
		regValue &= 0x0000FFFF;
		regValue |= (1 << 16);		

		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE0)
		{
			if ((pAd->CurrentAddress[5] % 2 != 0)
#ifdef P2P_SUPPORT
#ifdef P2P_ODD_MAC_ADJUST
				&& FALSE
#endif /* P2P_ODD_MAC_ADJUST */
#endif /* P2P_SUPPORT */
			)
				DBGPRINT(RT_DEBUG_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));
		
		}
		else
		{
			/*set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014) 
			to disable/enable the new MAC address assignment.  */
			regValue |= (1 << 21);
		}
		
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, regValue);
	}
#endif /* P2P_SUPPORT */
}

VOID AsicSetMcastWC(
	IN PRTMP_ADAPTER pAd) 
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];
	USHORT		offset;
	
	pEntry->Sst        = SST_ASSOC;
	pEntry->Aid        = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index*/
	pEntry->PsMode     = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate; 
	offset = MAC_WCID_BASE + BSS0Mcast_WCID * HW_WCID_ENTRY_SIZE;
}

/*
	==========================================================================
	Description:   

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDelWcidTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	Wcid) 
{
	ULONG		  Addr0 = 0x0, Addr1 = 0x0;
	ULONG 		offset;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicDelWcidTab==>Wcid = 0x%x\n",Wcid));
	offset = MAC_WCID_BASE + Wcid * HW_WCID_ENTRY_SIZE;
	RTMP_IO_WRITE32(pAd, offset, Addr0);
	offset += 4;
	RTMP_IO_WRITE32(pAd, offset, Addr1);
}

#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableRDG(
	IN PRTMP_ADAPTER pAd) 
{
	TX_LINK_CFG_STRUC	TxLinkCfg;
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 1;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	Data  &= 0xFFFFFF00;
	Data  |= 0x80;
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

	/*OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);*/
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableRDG(
	IN PRTMP_ADAPTER pAd) 
{
	TX_LINK_CFG_STRUC	TxLinkCfg;
	UINT32				Data = 0;



	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 0;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	
	Data  &= 0xFFFFFF00;
	/*Data  |= 0x20;*/
#ifndef WIFI_TEST
	/*if ( pAd->CommonCfg.bEnableTxBurst )	*/
	/*	Data |= 0x60;  for performance issue not set the TXOP to 0*/
#endif
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif /* DOT11_N_SUPPORT */
	)
	{
		/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
		if (pAd->CommonCfg.bEnableTxBurst)
		Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

}
#endif /* DOT11_N_SUPPORT */

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;
	
	DBGPRINT(RT_DEBUG_TRACE, ("--->Disable TSF synchronization\n"));

	/* 2003-12-20 disable TSF and TBTT while NIC in power-saving have side effect*/
	/*			  that NIC will never wakes up because TSF stops and no more */
	/*			  TBTT interrupts*/
	pAd->TbttTickCount = 0;
	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
	csr.field.bBeaconGen = 0;
	csr.field.bTBTTEnable = 0;
	csr.field.TsfSyncMode = 0;
	csr.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);

}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableBssSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
/*	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, 0x00000000);*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
		csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 1; /* sync TSF in INFRASTRUCTURE mode*/
		csr.field.bBeaconGen  = 0; /* do NOT generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_STA_SUPPORT */	
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}

/*
	==========================================================================
	Description:
	Note: 
		BEACON frame in shared memory should be built ok before this routine
		can be called. Otherwise, a garbage frame maybe transmitted out every
		Beacon period.

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableIbssSync(
	IN PRTMP_ADAPTER pAd)
{
	BCN_TIME_CFG_STRUC csr9;
	PUCHAR			ptr;
	UINT i;
	ULONG beaconBaseLocation = 0;
	USHORT			beaconLen = (USHORT) pAd->BeaconTxWI.MPDUtotalByteCount;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT32 longptr;
#ifdef RT_BIG_ENDIAN
	TXWI_STRUC		localTxWI;
	
	NdisMoveMemory((PUCHAR)&localTxWI, (PUCHAR)&pAd->BeaconTxWI, TXWISize);
	RTMPWIEndianChange(pAd, (PUCHAR)&localTxWI, TYPE_TXWI);
	beaconLen = (USHORT) localTxWI.MPDUtotalByteCount;
#endif /* RT_BIG_ENDIAN */

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(MPDUtotalByteCount=%d, beaconLen=%d)\n", pAd->BeaconTxWI.MPDUtotalByteCount, beaconLen));


	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(ADHOC mode. MPDUtotalByteCount = %d)\n", pAd->BeaconTxWI.MPDUtotalByteCount));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word);
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
	beaconBaseLocation = HW_BEACON_BASE0(pAd);

#ifdef RTMP_MAC_PCI
	/* move BEACON TXD and frame content to on-chip memory*/
	ptr = (PUCHAR)&pAd->BeaconTxWI;
	for (i=0; i < TXWISize; i+=4)
	{
		longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_CHIP_UPDATE_BEACON(pAd, HW_BEACON_BASE0(pAd) + i, longptr, 4);
		ptr += 4;
	}

	/* start right after the 16-byte TXWI field*/
	ptr = pAd->BeaconBuf;
	for (i=0; i< beaconLen; i+=4)
	{
		longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_CHIP_UPDATE_BEACON(pAd, HW_BEACON_BASE0(pAd) + TXWISize + i, longptr, 4);
		ptr +=4;
	}
#endif /* RTMP_MAC_PCI */



	
	/* For Wi-Fi faily generated beacons between participating stations. */
	/* Set TBTT phase adaptive adjustment step to 8us (default 16us)*/
	/* don't change settings 2006-5- by Jerry*/
	/*RTMP_IO_WRITE32(pAd, TBTT_SYNC_CFG, 0x00001010);*/
	
	/* start sending BEACON*/
	csr9.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr9.field.bTsfTicking = 1;
#ifdef IWSC_SUPPORT
	/*
		 SYNC with nobody
		 If Canon loses our Beacon over 5 seconds, Canon will delete us silently.
	*/
	csr9.field.TsfSyncMode = 3; // sync TSF in IBSS mode
#else /* IWSC_SUPPORT */
	/*
		(STA ad-hoc mode) Upon the reception of BEACON frame from associated BSS, 
		local TSF is updated with remote TSF only if the remote TSF is greater than local TSF
	*/
	csr9.field.TsfSyncMode = 2; /* sync TSF in IBSS mode*/
#endif /* !IWSC_SUPPORT */
	csr9.field.bTBTTEnable = 1;
	csr9.field.bBeaconGen = 1;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER pAd,
	IN PEDCA_PARM	 pEdcaParm)
{
	EDCA_AC_CFG_STRUC   Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC    AifsnCsr;
	CWMIN_CSR_STRUC    CwminCsr;
	CWMAX_CSR_STRUC    CwmaxCsr;
	int i;
	UINT32 MaxWcidNum = MAX_LEN_OF_MAC_TABLE;
	
#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
		MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
#endif /* MAC_REPEATER_SUPPORT */

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("AsicSetEdcaParm\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; i < MaxWcidNum; i++)
		{
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) || IS_ENTRY_APCLI(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		/*========================================================*/
		/*      MAC Register has a copy .*/
		/*========================================================*/
/*#ifndef WIFI_TEST*/
		if( pAd->CommonCfg.bEnableTxBurst )		
		{
			/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
			Ac0Cfg.field.AcTxop = 0x20; /* Suggest by John for TxBurst in HT Mode*/
		}
		else
			Ac0Cfg.field.AcTxop = 0;	/* QID_AC_BE*/
/*#else*/
/*		Ac0Cfg.field.AcTxop = 0;	 QID_AC_BE*/
/*#endif					*/
		Ac0Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac0Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac0Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);

		Ac1Cfg.field.AcTxop = 0;	/* QID_AC_BK*/
		Ac1Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac1Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac1Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);

		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			Ac2Cfg.field.AcTxop = 192;	/* AC_VI: 192*32us ~= 6ms*/
			Ac3Cfg.field.AcTxop = 96;	/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			Ac2Cfg.field.AcTxop = 96;	/* AC_VI: 96*32us ~= 3ms*/
			Ac3Cfg.field.AcTxop = 48;	/* AC_VO: 48*32us ~= 1.5ms*/
		}
		Ac2Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac2Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac2Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		Ac3Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac3Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac3Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);

		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = 0;		/* QID_AC_BE*/
		csr0.field.Ac1Txop = 0;		/* QID_AC_BK*/
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			csr1.field.Ac2Txop = 192;		/* AC_VI: 192*32us ~= 6ms*/
			csr1.field.Ac3Txop = 96;		/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			csr1.field.Ac2Txop = 96;		/* AC_VI: 96*32us ~= 3ms*/
			csr1.field.Ac3Txop = 48;		/* AC_VO: 48*32us ~= 1.5ms*/
		}
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin1 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin2 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin3 = CW_MIN_IN_BITS;
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax1 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax2 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax3 = CW_MAX_IN_BITS;
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, 0x00002222);

		NdisZeroMemory(&pAd->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));

	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		/*========================================================*/
		/*      MAC Register has a copy.*/
		/*========================================================*/
		
		/* Modify Cwmin/Cwmax/Txop on queue[QID_AC_VI], Recommend by Jerry 2005/07/27*/
		/* To degrade our VIDO Queue's throughput for WiFi WMM S3T07 Issue.*/
		
		/*pEdcaParm->Txop[QID_AC_VI] = pEdcaParm->Txop[QID_AC_VI] * 7 / 10;  rt2860c need this		*/

		Ac0Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BE];
		Ac0Cfg.field.Cwmin= pEdcaParm->Cwmin[QID_AC_BE];
		Ac0Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BE];
		Ac0Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BE]; /*+1;*/

		Ac1Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BK];
		Ac1Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_BK]; /*+2; */
		Ac1Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BK];
		Ac1Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BK]; /*+1;*/


		Ac2Cfg.field.AcTxop = (pEdcaParm->Txop[QID_AC_VI] * 6) / 10;
#ifdef RTMP_RBUS_SUPPORT
		if(pAd->Antenna.field.TxPath == 1)
		{
			Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI] + 1;
			Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI] + 1;			
		}
		else
#endif 			
		{
		Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI];
		Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI];
		}
		/*sync with window 20110524*/
		Ac2Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VI] + 1; /* 5.2.27 T6 Pass Tx VI+BE, but will impack 5.2.27/28 T7. Tx VI*/
		
#ifdef INF_AMAZON_SE
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			Ac2Cfg.field.Aifsn = 0x3; /*for WiFi WMM A1-T07.*/
#endif /* CONFIG_AP_SUPPORT */
#endif /* INF_AMAZON_SE */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				Ac2Cfg.field.Aifsn -= 1; 

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: conexant legacy sta ==> broadcom 11n sta*/
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				Ac0Cfg.field.Aifsn = 3;
				Ac2Cfg.field.AcTxop = 5;
			}
			
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		if (APCLI_IF_UP_CHECK(pAd, 0) && pAd->bApCliCertTest == TRUE)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10) 
				Ac2Cfg.field.Aifsn -= 1; 

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: conexant legacy sta ==> broadcom 11n sta*/
			/*
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				Ac0Cfg.field.Aifsn = 3;
				Ac2Cfg.field.AcTxop = 5;
			}*/
			
		}
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */

		Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
		Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
		Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
		Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];

/*#ifdef WIFI_TEST*/
		if (pAd->CommonCfg.bWiFiTest)
		{
			if (Ac3Cfg.field.AcTxop == 102)
			{
			Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE] ? pEdcaParm->Txop[QID_AC_BE] : 10;
				Ac0Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BE]-1; /* AIFSN must >= 1 */
			Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
				Ac1Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BK];
			Ac2Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VI];
			} /* End of if */
		}
/*#endif  WIFI_TEST */
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
		/* STA TestBed changes in this item: for sta wifitest 5.2.32, 2011/04/11 */
		/* just for 5390 5392 pci, 5370 5372 not need this patch */
		if((IS_RT5390(pAd) || IS_RT5392(pAd)) && pEdcaParm->Aifsn[QID_AC_VI] == 10)
		{                   
			Ac0Cfg.field.AcTxop = 38;
		}
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
#ifdef RTMP_MAC_PCI
		/* STA TestBed changes in this item: for sta wifitest 5.2.32, 2011/04/11 */
		/* just for 5390 5392 pci, 5370 5372 not need this patch */
		if (APCLI_IF_UP_CHECK(pAd, 0) && pAd->bApCliCertTest == TRUE)
		{
			if(pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{                   
				Ac0Cfg.field.AcTxop = 38;
			}
		}
#endif /* RTMP_MAC_PCI */
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */

		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);


		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = Ac0Cfg.field.AcTxop;
		csr0.field.Ac1Txop = Ac1Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);

		csr1.field.Ac2Txop = Ac2Cfg.field.AcTxop;
		csr1.field.Ac3Txop = Ac3Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pEdcaParm->Cwmin[QID_AC_BE];
		CwminCsr.field.Cwmin1 = pEdcaParm->Cwmin[QID_AC_BK];
		CwminCsr.field.Cwmin2 = pEdcaParm->Cwmin[QID_AC_VI];
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO];
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; /*for TGn wifi test*/
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		if(APCLI_IF_UP_CHECK(pAd, 0) && pAd->bApCliCertTest == TRUE)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; /*for TGn wifi test*/
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */

		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pEdcaParm->Cwmax[QID_AC_BE];
		CwmaxCsr.field.Cwmax1 = pEdcaParm->Cwmax[QID_AC_BK];
		CwmaxCsr.field.Cwmax2 = pEdcaParm->Cwmax[QID_AC_VI];
		CwmaxCsr.field.Cwmax3 = pEdcaParm->Cwmax[QID_AC_VO];
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		AifsnCsr.word = 0;
		AifsnCsr.field.Aifsn0 = Ac0Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BE];*/
		AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BK];*/
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
		AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VI];*/
#ifdef INF_AMAZON_SE
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
			AifsnCsr.field.Aifsn2 = 0x2; /*pEdcaParm->Aifsn[QID_AC_VI]; for WiFi WMM A1-T07.*/
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* INF_AMAZON_SE */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn - 4;

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: connexant legacy sta ==> broadcom 11n sta*/
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				AifsnCsr.field.Aifsn0 = 3;
				AifsnCsr.field.Aifsn2 = 7;
			}

			if (INFRA_ON(pAd))
				CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_WMM_CAPABLE);
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; /*pEdcaParm->Aifsn[QID_AC_VO]; for TGn wifi test*/
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		if (APCLI_IF_UP_CHECK(pAd, 0) && pAd->bApCliCertTest == TRUE)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; /*pEdcaParm->Aifsn[QID_AC_VO]; for TGn wifi test*/

			/* TODO: Is this modification also suitable for RT3052/RT3050 ???*/
			{
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
					AifsnCsr.field.Aifsn2 = 0x2; /*pEdcaParm->Aifsn[QID_AC_VI]; for WiFi WMM S4-T04.*/
			}

		}
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);

		NdisMoveMemory(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		if (!ADHOC_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}

	}

	pAd->CommonCfg.RestoreBurstMode = Ac0Cfg.word;
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID 	AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime) 
{
	ULONG	SlotTime;
	UINT32	RegValue = 0;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
		bUseShortSlotTime = TRUE;
#endif /* CONFIG_STA_SUPPORT */

	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* force using short SLOT time for FAE to demo performance when TxBurst is ON*/
		if (((pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
#ifdef DOT11_N_SUPPORT
			|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
#endif /* DOT11_N_SUPPORT */
			)
		{
			/* In this case, we will think it is doing Wi-Fi test*/
			/* And we will not set to short slot when bEnableTxBurst is TRUE.*/
		}
		else if (pAd->CommonCfg.bEnableTxBurst)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	
	/* For some reasons, always set it to short slot time.*/
	/* ToDo: Should consider capability with 11B*/
#ifdef CONFIG_STA_SUPPORT 
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.BssType == BSS_ADHOC)	
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	RegValue = RegValue & 0xFFFFFF00;

	RegValue |= SlotTime;

	RTMP_IO_WRITE32(pAd, BKOFF_SLOT_CFG, RegValue);
}

/*
	========================================================================
	Description:
		Add Shared key information into ASIC. 
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.
		
    Return:
	========================================================================
*/
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	ULONG offset; /*, csr0;*/
	SHAREDKEY_MODE_STRUC csr1;
	UINT16 SharedKeyTableBase, SharedKeyModeBase;
#ifdef RTMP_MAC_PCI
	INT   i;
#endif /* RTMP_MAC_PCI */

	PUCHAR		pKey = pCipherKey->Key;
	PUCHAR		pTxMic = pCipherKey->TxMic;
	PUCHAR		pRxMic = pCipherKey->RxMic;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicAddSharedKeyEntry BssIndex=%d, KeyIdx=%d\n", BssIndex,KeyIdx));
/*============================================================================================*/

	DBGPRINT(RT_DEBUG_TRACE,("AsicAddSharedKeyEntry: %s key #%d\n", CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
/*============================================================================================*/
	
	/* fill key material - key + TX MIC + RX MIC*/
	if (BssIndex >= 8)
	{
		SharedKeyTableBase = SHARED_KEY_TABLE_BASE_EXT;
		SharedKeyModeBase = SHARED_KEY_MODE_BASE_EXT;
		BssIndex -= 8;
	}
	else
	{
		SharedKeyTableBase = SHARED_KEY_TABLE_BASE;
		SharedKeyModeBase = SHARED_KEY_MODE_BASE;
	}

#ifdef RTMP_MAC_PCI
	offset = SharedKeyTableBase + (4*BssIndex + KeyIdx)*HW_KEY_ENTRY_SIZE;
	for (i=0; i<MAX_LEN_OF_SHARE_KEY; i++) 
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}

	offset += MAX_LEN_OF_SHARE_KEY;
	if (pTxMic)
	{
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pTxMic[i]);
		}
	}

	offset += 8;
	if (pRxMic)
	{
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pRxMic[i]);
		}
	}
#endif /* RTMP_MAC_PCI */


	
	/* Update cipher algorithm. WSTA always use BSS0*/
	RTMP_IO_READ32(pAd, SharedKeyModeBase + 4*(BssIndex/2), &csr1.word);
	DBGPRINT(RT_DEBUG_TRACE,("Read: SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", BssIndex,KeyIdx, csr1.word));
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss0Key3CipherAlg = CipherAlg;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss1Key3CipherAlg = CipherAlg;
	}
	DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SharedKeyModeBase+4*(BssIndex/2), csr1.word);
		
}

/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	/*ULONG SecCsr0;*/
	SHAREDKEY_MODE_STRUC csr1;
	UINT16 SharedKeyModeBase;

	DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

	if (BssIndex >= 8)
	{
		SharedKeyModeBase = SHARED_KEY_MODE_BASE_EXT;
		BssIndex -= 8;
	}
	else
	{
		SharedKeyModeBase = SHARED_KEY_MODE_BASE;
	}

	RTMP_IO_READ32(pAd, SharedKeyModeBase+4*(BssIndex/2), &csr1.word);
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = 0;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = 0;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = 0;
		else
			csr1.field.Bss0Key3CipherAlg = 0;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = 0;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = 0;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = 0;
		else
			csr1.field.Bss1Key3CipherAlg = 0;
	}
	DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SharedKeyModeBase+4*(BssIndex/2), csr1.word);
	ASSERT(BssIndex < 8);
	ASSERT(KeyIdx < 4);

}

VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	ULONG	offset;

	offset = MAC_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);

	RTMP_IO_WRITE32(pAd, offset, uIV);
	RTMP_IO_WRITE32(pAd, offset + 4, uEIV);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: wcid(%d) 0x%08lx, 0x%08lx \n", 
									__FUNCTION__, WCID, uIV, uEIV));	
}

VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN PUCHAR        pAddr)
{
	ULONG offset;
	ULONG Addr;
	
	offset = MAC_WCID_BASE + (WCID * HW_WCID_ENTRY_SIZE);	
	Addr = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = pAddr[4] + (pAddr[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);	
}
	
/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:

    Note :
		The key table selection rule :
    	1.	Wds-links and Mesh-links always use Pair-wise key table. 	
		2. 	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled, 
			it needs to set key into Pair-wise Key Table.
		3.	The pair-wise key security mode is set NONE, it means as no security.
		4.	In STA Adhoc mode, it always use shared key table.
		5.	Otherwise, use shared key table

	========================================================================
*/
VOID	AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
	WCID_ATTRIBUTE_STRUC WCIDAttri;	
	USHORT		offset;

	/* Initialize the content of WCID Attribue  */
	WCIDAttri.word = 0;

	/* The limitation of HW WCID table */
	if (/*Wcid < 1 ||*/ Wcid > 254)
	{		
		DBGPRINT(RT_DEBUG_WARN, ("%s: Wcid is invalid (%d). \n", 
										__FUNCTION__, Wcid));	
		return;
	}

	/* Update the pairwise key security mode.
	   Use bit10 and bit3~1 to indicate the pairwise cipher mode */	
	WCIDAttri.field.PairKeyModeExt = ((CipherAlg & 0x08) >> 3);
	WCIDAttri.field.PairKeyMode = (CipherAlg & 0x07);

	/* Update the MBSS index.
	   Use bit11 and bit6~4 to indicate the BSS index */	
	WCIDAttri.field.BSSIdxExt = ((BssIdx & 0x08) >> 3);
	WCIDAttri.field.BSSIdx = (BssIdx & 0x07);

#ifdef WAPI_SUPPORT
	/* Update WAPI related information */
	if (CipherAlg == CIPHER_SMS4)
	{
		if (KeyTabFlag == SHAREDKEYTABLE)
			WCIDAttri.field.WAPI_MCBC = 1;
		WCIDAttri.field.WAPIKeyIdx = ((KeyIdx == 0) ? 0 : 1); 
	}
#endif /* WAPI_SUPPORT */
	
	/* Assign Key Table selection */		
	WCIDAttri.field.KeyTab = KeyTabFlag;

	/* Update related information to ASIC */
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid * HW_WCID_ATTRI_SIZE);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : WCID #%d, KeyIndex #%d, Alg=%s\n", __FUNCTION__, Wcid, KeyIdx, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE, ("		WCIDAttri = 0x%x \n", WCIDAttri.word));	
	
}
	

/*
	========================================================================
	Description:
		Add Pair-wise key material into ASIC. 
		Update pairwise key, TxMic and RxMic to Asic Pair-wise key table
				
    Return:
	========================================================================
*/
VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
	INT i;
	ULONG 		offset;
	PUCHAR		 pKey = pCipherKey->Key;
	PUCHAR		 pTxMic = pCipherKey->TxMic;
	PUCHAR		 pRxMic = pCipherKey->RxMic;
#ifdef DBG
	UCHAR		CipherAlg = pCipherKey->CipherAlg;
#endif
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	unsigned long irqFlag = 0;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_LOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
#endif /* RTMP_MAC_PCI */
	/* EKEY*/
	offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
#ifdef RTMP_MAC_PCI
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i++)
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}
#endif /* RTMP_MAC_PCI */
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i+=4)
	{
		UINT32 Value;
		RTMP_IO_READ32(pAd, offset + i, &Value);
	}

	offset += MAX_LEN_OF_PEER_KEY;
	
	/*  MIC KEY*/
	if (pTxMic)
	{
#ifdef RTMP_MAC_PCI
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pTxMic[i]);
		}
#endif /* RTMP_MAC_PCI */
	}
	offset += 8;
	if (pRxMic)
	{
#ifdef RTMP_MAC_PCI
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pRxMic[i]);
		}
#endif /* RTMP_MAC_PCI */
	}
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_UNLOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif /* SPECIFIC_BCN_BUF_SUPPORT*/
#endif /* RTMP_MAC_PCI */
	DBGPRINT(RT_DEBUG_TRACE,("AsicAddPairwiseKeyEntry: WCID #%d Alg=%s\n",WCID, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
}

/*
	========================================================================
	Description:
		Remove Pair-wise key material from ASIC. 

    Return:
	========================================================================
*/	
VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	AsicUpdateWcidAttributeEntry(pAd, 
							  BSS0,
							  0,
							  CIPHER_NONE, 
							  Wcid,
							  PAIRWISEKEYTABLE);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : Wcid #%d \n", __FUNCTION__, Wcid));
}

BOOLEAN AsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#ifdef RTMP_PCI_SUPPORT
	if (IS_PCI_INF(pAd))
		in_atomic = TRUE;
#endif /* RTMP_USB_SUPPORT */
	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
	else
		return FALSE;
}


BOOLEAN AsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked)
{
	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1, FlgIsNeedLocked);
	else
		return FALSE;
}

/*
	========================================================================
	Description:
		For 1x1 chipset : 2070 / 3070 / 3090 / 3370 / 3390 / 5370 / 5390 
		Usage :	1. Set Default Antenna as initialize
				2. Antenna Diversity switching used
				3. iwpriv command switch Antenna

    Return:
	========================================================================
 */
VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	if (pAd->chipOps.SetRxAnt)
		pAd->chipOps.SetRxAnt(pAd, Ant);

}


VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER pAd, 
	IN	UCHAR		Channel) 
{
	if (pAd->chipOps.AsicRfTurnOff)
	{
		pAd->chipOps.AsicRfTurnOff(pAd);
	}
	else
	{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
		/* RF R2 bit 18 = 0*/
		UINT32			R1 = 0, R2 = 0, R3 = 0;
		UCHAR			index;
		RTMP_RF_REGS	*RFRegTable;
	
		RFRegTable = RF2850RegTable;
#endif /* defined(RT28xx) || defined(RT2880) || defined(RT2883) */

		switch (pAd->RfIcType)
		{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
#if defined(RT28xx) || defined(RT2880)
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:
#endif /* defined(RT28xx) || defined(RT2880) */
#ifdef RT2883
			case RFIC_2853:
#endif /* RT2883 */
				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R1 = RFRegTable[index].R1 & 0xffffdfff;
						R2 = RFRegTable[index].R2 & 0xfffbffff;
						R3 = RFRegTable[index].R3 & 0xfff3ffff;

						RTMP_RF_IO_WRITE32(pAd, R1);
						RTMP_RF_IO_WRITE32(pAd, R2);

						/* Program R1b13 to 1, R3/b18,19 to 0, R2b18 to 0. */
						/* Set RF R2 bit18=0, R3 bit[18:19]=0*/
						/*if (pAd->StaCfg.bRadio == FALSE)*/
						if (1)
						{
							RTMP_RF_IO_WRITE32(pAd, R3);

							DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x,  R3 = 0x%08x \n",
								Channel, pAd->RfIcType, R2, R3));
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x \n",
								Channel, pAd->RfIcType, R2));
						break;
					}
				}
				break;
#endif /* defined(RT28xx) || defined(RT2880) || defined(RT2883) */
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d : Unkonwn RFIC=%d\n",
											Channel, pAd->RfIcType));
				break;
		}
	}
}


#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high)
{
	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		ULONG	offset;

		offset = WAPI_PN_TABLE_BASE + (WCID * WAPI_PN_ENTRY_SIZE);

		RTMP_IO_WRITE32(pAd, offset, pn_low);
		RTMP_IO_WRITE32(pAd, offset + 4, pn_high);
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : Not support HW_WAPI_PN_TABLE\n", 
								__FUNCTION__));
	}
	
}
#endif /* WAPI_SUPPORT */



#ifdef VCORECAL_SUPPORT
VOID AsicVCORecalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR RFValue = 0;
	UINT32 TxPinCfg = 0;
	UINT8	mode = pAd->chipCap.FlgIsVcoReCalMode;

	if (mode == VCO_CAL_DISABLE)
		return;

#ifdef RT6352
	if (pAd->bCalibrationDone == FALSE)
		return;
#endif /* RT6352 */

#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT5350
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		UCHAR BbpR47 = 0;

	    //TSSI_REPORT_SEL = 0
	    RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	    BbpR47 &= ~0x3;
	    RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );
	}
#endif /* RT5350 */
#endif /* RTMP_INTERNAL_TX_ALC */

	RTMP_IO_READ32(pAd, TX_PIN_CFG, &TxPinCfg);
	TxPinCfg &= 0xFCFFFFF0;
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

	switch (mode)
	{
		case VCO_CAL_MODE_1:
			RT30xxReadRFRegister(pAd, RF_R07, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x01; /* bit 0=vcocal_en*/
			RT30xxWriteRFRegister(pAd, RF_R07, (UCHAR)RFValue);
			break;

		case VCO_CAL_MODE_2:
			RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en*/
			RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);
			break;

#ifdef RT6352
		case VCO_CAL_MODE_3:
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R05, 0x40);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, 0x0C);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R04, &RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en*/
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, RFValue);
			break;
#endif /* RT6352 */

		default:
			return;
	}

	RtmpOsMsDelay(2);

	RTMP_IO_READ32(pAd, TX_PIN_CFG, &TxPinCfg);
	if (pAd->CommonCfg.Channel <= 14)
	{
		if ((pAd->Antenna.field.TxPath == 1)
#ifdef GREENAP_SUPPORT
			|| (pAd->ApCfg.bGreenAPActive == TRUE)	 /* avoid to corrupt GreenAP operation */
#endif /* GREENAP_SUPPORT */
			)
			TxPinCfg |= 0x2;
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg |= 0xA;
		else if (pAd->Antenna.field.TxPath == 3)
			TxPinCfg |= 0x0200000A;
	}
	else
	{
		if ((pAd->Antenna.field.TxPath == 1)
#ifdef GREENAP_SUPPORT
			|| (pAd->ApCfg.bGreenAPActive == TRUE)	/* avoid to corrupt GreenAP operation */
#endif /* GREENAP_SUPPORT */
			)
			TxPinCfg |= 0x1;
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg |= 0x5;
		else if (pAd->Antenna.field.TxPath == 3)
			TxPinCfg |= 0x01000005;
	}
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef TXBF_SUPPORT
		// Do a Divider Calibration and update BBP registers
		if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_CAL)==0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			ITxBFDividerCalibration(pAd, 2, 0, NULL);
		}

		if (pAd->CommonCfg.ETxBfEnCond)
		{
			INT idx;
			
			for (idx = 1; idx < MAX_LEN_OF_MAC_TABLE; idx++)
			{
				MAC_TABLE_ENTRY		*pEntry;

				pEntry = &pAd->MacTab.Content[idx];
				if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->eTxBfEnCond))
				{
					BOOLEAN Cancelled;

					RTMPCancelTimer(&pEntry->eTxBfProbeTimer, &Cancelled);

					pEntry->bfState = READY_FOR_SNDG0;
					eTxBFProbing(pAd, pEntry);
				}
			}
		}
#endif // TXBF_SUPPORT //
}
#endif /* VCORECAL_SUPPORT */


#ifdef STREAM_MODE_SUPPORT
// StreamModeRegVal - return MAC reg value for StreamMode setting
UINT32 StreamModeRegVal(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 streamWord;

	switch (pAd->CommonCfg.StreamMode)
	{
		case 1:
			streamWord = 0x030000;
			break;
		case 2:
			streamWord = 0x0c0000;
			break;
		case 3:
			streamWord = 0x0f0000;
			break;
		default:
			streamWord = 0x0;
			break;
	}

	return streamWord;
}


/*
	========================================================================
	Description:
		configure the stream mode of specific MAC or all MAC and set to ASIC. 

	Prameters:
		pAd		 --- 
		pMacAddr ---
		bClear	 --- disable the stream mode for specific macAddr when
						(pMacAddr!=NULL)
	
    Return:
	========================================================================
*/
VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
	UINT32 streamWord;
	UINT32 regAddr, regVal;

	
	if (!pAd->chipCap.FlgHwStreamMode)
		return;
		
	streamWord = StreamModeRegVal(pAd);
	if (!bEnabled)
		streamWord = 0;

	regAddr = TX_CHAIN_ADDR0_L + chainIdx * 4;
	RTMP_IO_WRITE32(pAd, regAddr,  
					(UINT32)(pMacAddr[0]) | 
					(UINT32)(pMacAddr[1] << 8)  | 
					(UINT32)(pMacAddr[2] << 16) | 
					(UINT32)(pMacAddr[3] << 24));
	
	RTMP_IO_READ32(pAd, regAddr + 4, &regVal);
	regVal &= (~0x000f0000);
	RTMP_IO_WRITE32(pAd, regAddr + 4, 
					(regVal | streamWord) | 
					(UINT32)(pMacAddr[4]) | 
					(UINT32)(pMacAddr[5] << 8));
	
}


VOID RtmpStreamModeInit(
	IN RTMP_ADAPTER *pAd)
{
	int chainIdx;
	UCHAR *pMacAddr;

	if (pAd->chipCap.FlgHwStreamMode == FALSE)
		return;	
	
	for (chainIdx = 0; chainIdx < STREAM_MODE_STA_NUM; chainIdx++)
	{
		pMacAddr = &pAd->CommonCfg.StreamModeMac[chainIdx][0];
		AsicSetStreamMode(pAd, pMacAddr, chainIdx, TRUE);
	}
}


/* Enable the stream mode*/

/* Parameters*/
/*	pAd: The adapter data structure*/

/* Return Value:*/
/*	None*/

VOID AsicEnableStreamMode(
	IN PRTMP_ADAPTER pAd)
{
	TX_CHAIN_ADDR0_L_STRUC TxChainAddr0L = {{0}};
	TX_CHAIN_ADDR0_H_STRUC TxChainAddr0H = {{0}};
	TX_CHAIN_ADDR1_H_STRUC TxChainAddr1H = {{0}};
	TX_CHAIN_ADDR2_H_STRUC TxChainAddr2H = {{0}};
	TX_CHAIN_ADDR3_H_STRUC TxChainAddr3H = {{0}};

	DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

	/* Chain #0 for broadcast*/
	TxChainAddr0L.field.TxChainAddr0L_Byte3 = 0xFF;
	TxChainAddr0L.field.TxChainAddr0L_Byte2 = 0xFF;
	TxChainAddr0L.field.TxChainAddr0L_Byte1 = 0xFF;
	TxChainAddr0L.field.TxChainAddr0L_Byte0 = 0xFF;
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_L, TxChainAddr0L.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR0_H, &TxChainAddr0H.word);
	TxChainAddr0H.field.TxChainAddr0H_Byte4 = 0xFF;
	TxChainAddr0H.field.TxChainAddr0H_Byte5 = 0xFF;
	TxChainAddr0H.field.TxChainSel0 = 0xF; /* Enable the stream mode for chain #0*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_H, TxChainAddr0H.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR1_H, &TxChainAddr1H.word);
	TxChainAddr1H.field.TxChainSel0 = 0xF; /* Enable the stream mode for chain #1*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR1_H, TxChainAddr1H.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR2_H, &TxChainAddr2H.word);
	TxChainAddr2H.field.TxChainSel0 = 0xF; /* Enable the stream mode for chain #2*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR2_H, TxChainAddr2H.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR3_H, &TxChainAddr3H.word);
	TxChainAddr3H.field.TxChainSel0 = 0xF; /* Enable the stream mode for chain #3*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR3_H, TxChainAddr3H.word);
	
	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
}


/* Disable the stream mode*/

/* Parameters*/
/*	pAd: The adapter data structure*/

/* Return Value:*/
/*	None*/

VOID AsicDisableStreamMode(
	IN PRTMP_ADAPTER pAd)
{
	TX_CHAIN_ADDR0_L_STRUC TxChainAddr0L = {{0}};
	TX_CHAIN_ADDR0_H_STRUC TxChainAddr0H = {{0}};
	TX_CHAIN_ADDR1_H_STRUC TxChainAddr1H = {{0}};
	TX_CHAIN_ADDR2_H_STRUC TxChainAddr2H = {{0}};
	TX_CHAIN_ADDR3_H_STRUC TxChainAddr3H = {{0}};

	DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

	/* Chain #0 for broadcast*/
	TxChainAddr0L.field.TxChainAddr0L_Byte3 = 0xFF;
	TxChainAddr0L.field.TxChainAddr0L_Byte2 = 0xFF;
	TxChainAddr0L.field.TxChainAddr0L_Byte1 = 0xFF;
	TxChainAddr0L.field.TxChainAddr0L_Byte0 = 0xFF;
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_L, TxChainAddr0L.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR0_H, &TxChainAddr0H.word);
	TxChainAddr0H.field.TxChainAddr0H_Byte4 = 0xFF;
	TxChainAddr0H.field.TxChainAddr0H_Byte5 = 0xFF;
	TxChainAddr0H.field.TxChainSel0 = 0x0; /* Disable the stream mode for chain #0*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_H, TxChainAddr0H.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR1_H, &TxChainAddr1H.word);
	TxChainAddr1H.field.TxChainSel0 = 0x0; /* Disable the stream mode for chain #1*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR1_H, TxChainAddr1H.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR2_H, &TxChainAddr2H.word);
	TxChainAddr2H.field.TxChainSel0 = 0x0; /* Disable the stream mode for chain #2*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR2_H, TxChainAddr2H.word);

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR3_H, &TxChainAddr3H.word);
	TxChainAddr3H.field.TxChainSel0 = 0x0; /* Disable the stream mode for chain #3*/
	RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR3_H, TxChainAddr3H.word);
	
	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
}
#endif // STREAM_MODE_SUPPORT //


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableRalinkBurstMode(
	IN PRTMP_ADAPTER pAd) 
{
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	pAd->CommonCfg.RestoreBurstMode = Data;
	Data  &= 0xFFF00000;
	Data  |= 0x86380;
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableRalinkBurstMode(
	IN PRTMP_ADAPTER pAd) 
{
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);

	Data = pAd->CommonCfg.RestoreBurstMode;
	Data &= 0xFFFFFF00;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif // DOT11_N_SUPPORT //
	)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
			Data |= 0x80;
		else if (pAd->CommonCfg.bEnableTxBurst)
			Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
}
#endif // DOT11_N_SUPPORT //


VOID RtmpUpdateFilterCoefficientControl(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Channel)
{
	UCHAR		BBPValue = 0;

	if (Channel == 14)
	{
		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			/* when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue |= 0x20; /* set bit5=1 */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
		}
		else
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
		}
	}
}

#ifdef WOW_SUPPORT
#endif /* WOW_SUPPORT */

#ifdef MAC_APCLI_SUPPORT
/*
	==========================================================================
	Description:
		Set BSSID of Root AP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetApCliBssid(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pBssid,
	IN UCHAR index) 
{
	UINT32		  Addr4 = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("===> AsicSetApCliBssid %x:%x:%x:%x:%x:%x\n",
				PRINT_MAC(pBssid)));
	
	Addr4 = (UINT32)(pBssid[0]) | 
			(UINT32)(pBssid[1] << 8)  | 
			(UINT32)(pBssid[2] << 16) |
			(UINT32)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_APCLI_BSSID_DW0, Addr4);

	Addr4 = 0;
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);
	/* Enable APCLI mode */
	Addr4 |= 0x10000;

	RTMP_IO_WRITE32(pAd, MAC_APCLI_BSSID_DW1, Addr4);
}
#endif /* MAC_APCLI_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(
	IN PRTMP_ADAPTER pAd
)
{
	if (pAd->chipOps.AsicMeasureFalseCCA)
		pAd->chipOps.AsicMeasureFalseCCA(pAd);
}

VOID AsicMitigateMicrowave(
	IN PRTMP_ADAPTER pAd
)
{
	if (pAd->chipOps.AsicMitigateMicrowave)
		pAd->chipOps.AsicMitigateMicrowave(pAd);
}
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef DROP_MASK_SUPPORT
VOID asic_set_drop_mask(
	PRTMP_ADAPTER ad,
	USHORT	wcid,
	BOOLEAN enable)
{
	UINT32 mac_reg = 0, mac_old, reg_id, group_index;
	UINT32 drop_mask = (1U << (wcid % 32));

	/* each group has 32 entries */
	group_index = (wcid - (wcid % 32)) >> 5 /* divided by 32 */;
	reg_id = (TX_WCID_DROP_MASK0 + 4*group_index);

	NdisAcquireSpinLock(&ad->drop_mask_lock);

	RTMP_IO_READ32(ad, reg_id, &mac_reg);
	mac_old = mac_reg;
	mac_reg = (enable ? (mac_reg | drop_mask) : (mac_reg & ~drop_mask));
	if (mac_reg != mac_old)
		RTMP_IO_WRITE32(ad, reg_id, mac_reg);

	NdisReleaseSpinLock(&ad->drop_mask_lock);

	DBGPRINT(RT_DEBUG_TRACE,
			("%s(%u):, wcid = %u, reg_id = 0x%08x, mac_reg = 0x%08x, group_index = %u, drop_mask = 0x%08x\n",
			__FUNCTION__, enable, wcid, reg_id, mac_reg, group_index, drop_mask));
}


VOID asic_drop_mask_reset(
	PRTMP_ADAPTER ad)
{
	UINT32 i, reg_id;

	NdisAcquireSpinLock(&ad->drop_mask_lock);

	for ( i = 0; i < 8 /* num of drop mask group */; i++)
	{
		reg_id = (TX_WCID_DROP_MASK0 + i*4);
		RTMP_IO_WRITE32(ad, reg_id, 0);
	}

	NdisReleaseSpinLock(&ad->drop_mask_lock);

	DBGPRINT(RT_DEBUG_TRACE, ("%s()\n", __FUNCTION__));
}

#endif /* DROP_MASK_SUPPORT */

