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
typedef enum _PROT_REG_IDX_{
	REG_IDX_CCK = 0,	/* 0x1364 */
	REG_IDX_OFDM = 1,	/* 0x1368 */
	REG_IDX_MM20 = 2,  /* 0x136c */
	REG_IDX_MM40 = 3, /* 0x1370 */
	REG_IDX_GF20 = 4, /* 0x1374 */
	REG_IDX_GF40 = 5, /* 0x1378 */
	
}PROT_REG_IDX;

VOID AsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist)	
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT offset;
	UCHAR i;
	UINT16 PhyMode = 0x4000;
	UINT32 MacReg = 0;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

#ifdef DOT11_N_SUPPORT
	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
		return;

	if (pAd->BATable.numDoneOriginator)
	{
		/* enable the RTS/CTS to avoid channel collision*/
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

#ifdef DOT11_VHT_AC
	PhyMode = 0x2000; /* Bit 15:13, 0:Legacy CCK, 1: Legacy OFDM, 2: HT mix mode, 3: HT green field, 4: VHT mode, 5-7: Reserved */
#endif /* DOT11_VHT_AC */

	/* update PHY mode and rate*/
	if (pAd->OpMode == OPMODE_AP)
	{
		/* update PHY mode and rate*/
		if (pAd->CommonCfg.Channel > 14)
			ProtCfg.field.ProtectRate = PhyMode;
		ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		// Decide Protect Rate for Legacy packet
		if (pAd->CommonCfg.Channel > 14)
		{
			ProtCfg.field.ProtectRate = PhyMode; // OFDM 6Mbps
		}
		else 
		{
			ProtCfg.field.ProtectRate = 0x0000; // CCK 1Mbps
			if (pAd->CommonCfg.MinTxRate > RATE_11)
				ProtCfg.field.ProtectRate |= PhyMode; // OFDM 6Mbps
		}
	}

#ifdef FPGA_MODE
//+++Add by shiang for debug
	if (pAd->fpga_on & 0x8)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():RtsRate=%d\n",
					__FUNCTION__, pAd->CommonCfg.RtsRate));
	}
//---Add by shiang for debug
#endif

	/* Handle legacy(B/G) protection*/
	if (bDisableBGProtect)
	{
		/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
		ProtCfg.field.ProtectCtrl = 0;
		Protect[REG_IDX_CCK] = ProtCfg.word;
		Protect[REG_IDX_OFDM] = ProtCfg.word;
		pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14) {
			/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
			ProtCfg.field.ProtectCtrl = 0;			/* CCK do not need to be protected*/
			Protect[REG_IDX_CCK] = ProtCfg.word;
			ProtCfg.field.ProtectCtrl = ASIC_CTS;	/* OFDM needs using CCK to protect*/
			Protect[REG_IDX_OFDM] = ProtCfg.word;
			pAd->FlgCtsEnabled = 1; /* CTS-self is used */
		} else {
			ProtCfg.field.ProtectCtrl = 0;
			Protect[REG_IDX_CCK] = ProtCfg.word;
			Protect[REG_IDX_OFDM] = ProtCfg.word;
			pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
		}
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
#ifdef DOT11_VHT_AC
				/* 	PROT_RATE(15:0)  -- 0x2004 (OFDM 24M)*/
				Protect[2] = 0x01742004;
#else /* DOT11_VHT_AC */
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[2] = 0x01744004;
#endif /* !DOT11_VHT_AC */

				/* MM40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None) */
#ifdef DOT11_VHT_AC
				/* 	PROT_RATE(15:0)  -- 0x2084 (duplicate OFDM 24M)*/
				Protect[3] = 0x03f42084;
#else
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[3] = 0x03f44084;
#endif

				/* CF20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
#ifdef DOT11_VHT_AC
				/* 	PROT_RATE(15:0)  -- 0x2004 (OFDM 24M)*/
				Protect[4] = 0x01742004;
#else
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[4] = 0x01744004;
#endif

				/* CF40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
#ifdef DOT11_VHT_AC
				/* 	PROT_RATE(15:0)  -- 0x2084 (duplicate OFDM 24M)*/
				Protect[5] = 0x03f42084;
#else
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[5] = 0x03f44084;
#endif

				if (bNonGFExist)
				{
					/* PROT_NAV(19:18)  -- 01 (Short NAV protectiion)*/
					/* PROT_CTRL(17:16) -- 01 (RTS/CTS)*/
#ifdef DOT11_VHT_AC
					Protect[REG_IDX_GF20] = 0x01752004;
					Protect[REG_IDX_GF40] = 0x03f52084;
#else
					Protect[REG_IDX_GF20] = 0x01754004;
					Protect[REG_IDX_GF40] = 0x03f54084;
#endif
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					PROT_CFG_STRUC vht_port_cfg;

					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
				break;
				
 			case 1:
				/* This is "HT non-member protection mode."*/
				/* If there may be non-HT STAs my BSS*/
#ifdef DOT11_VHT_AC
				ProtCfg.word = 0x01742004;	/* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f42084; /* duplicaet legacy 24M. BW set 1.*/
#else
				ProtCfg.word = 0x01744004;	/* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
#endif
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
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					// Temporary tuen on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
					PROT_CFG_STRUC vht_port_cfg;

					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */

				break;
				
			case 2:
				/* If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets*/
#ifdef DOT11_VHT_AC
				ProtCfg.word = 0x01742004;  /* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f42084; /* duplicaet legacy 24M. BW set 1.*/
#else
				ProtCfg.word = 0x01744004;  /* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
#endif
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				} 
				/*Assign Protection method for 40MHz packets*/
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				if (bNonGFExist)
				{
					ProtCfg.field.ProtectCtrl = ASIC_RTS;
					ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				}
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;

				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					PROT_CFG_STRUC vht_port_cfg;

					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
				break;
				
			case 3:
				/* HT mixed mode. PROTECT ALL!*/
				/* Assign Rate*/
#ifdef DOT11_VHT_AC
				ProtCfg.word = 0x01742004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f42084;
#else
				ProtCfg.word = 0x01744004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f44084;
#endif
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
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					// Temporary tuen on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
					PROT_CFG_STRUC vht_port_cfg;

					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
				break;
				
			case 8:
				/* Special on for Atheros problem n chip.*/
#ifdef DOT11_VHT_AC
				ProtCfg.word = 0x01752004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f52084;
#else
				ProtCfg.word = 0x01754004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f54084;
#endif
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01750003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f50003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}
				
				Protect[REG_IDX_MM20] = ProtCfg.word; 	/*0x01754004;*/
				Protect[REG_IDX_MM40] = ProtCfg4.word; /*0x03f54084;*/
				Protect[REG_IDX_GF20] = ProtCfg.word; 	/*0x01754004;*/
				Protect[REG_IDX_GF40] = ProtCfg4.word; /*0x03f54084;*/
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

#ifdef DOT11_VHT_AC
#ifdef RT65xx
	if (IS_RT65XX(pAd))
	{
		RTMP_IO_READ32(pAd, TX_PROT_CFG8, &MacReg);
		MacReg &= (~0x18000000);
		if (pAd->CommonCfg.vht_bw_signal)
		{
			if (pAd->CommonCfg.vht_bw_signal == 1) /* static */
				MacReg |= 0x08000000;
			else if (pAd->CommonCfg.vht_bw_signal == 2)/* dynamic */
				MacReg |= 0x18000000;
		}
		RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, MacReg);
	}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
}


VOID AsicBBPAdjust(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-6590, now this function only used for AP mode, why we need this differentation?
	if (pAd->chipOps.ChipBBPAdjust != NULL)
		pAd->chipOps.ChipBBPAdjust(pAd);
}

	
/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSwitchChannel(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR Channel,
	IN BOOLEAN bScan) 
{
	UCHAR bw;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->chipOps.ChipSwitchChannel)
		pAd->chipOps.ChipSwitchChannel(pAd, Channel, bScan);
	else
		DBGPRINT(RT_DEBUG_ERROR, ("For this chip, no specified channel switch function!\n"));

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	if (bScan)
		bw = BW_20;
	else {
		bw = pAd->CommonCfg.BBPCurrentBW;
#ifdef IQ_CAL_SUPPORT
		RTMP_CHIP_IQ_CAL(pAd, Channel);
#endif /* IQ_CAL_SUPPORT */
	}
	RTMPSetAGCInitValue(pAd, bw);
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


#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */


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

	DBGPRINT(RT_DEBUG_TRACE, ("===> AsicSetBssid %x:%x:%x:%x:%x:%x\n",
				PRINT_MAC(pBssid)));
	
	Addr4 = (ULONG)(pBssid[0]) | 
			(ULONG)(pBssid[1] << 8)  | 
			(ULONG)(pBssid[2] << 16) |
			(ULONG)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW0, Addr4);
#endif /* HDR_TRANS_SUPPORT */

	Addr4 = 0;
	/* always one BSSID in STA mode*/
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);


	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

#ifdef HDR_TRANS_SUPPORT
	/*
		point WCID MAC table to 0x1800
		This is for debug.
		But HDR_TRANS doesn't work if you remove it.
		Check after IC formal release.
	*/
	Addr4 |= 0x18000000;
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, Addr4);
#endif /* HDR_TRANS_SUPPORT */

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
#ifdef APCLI_SUPPORT	
	UCHAR apidx;
	BOOLEAN bMaskBcn;	
#endif /* APCLI_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

#ifdef APCLI_SUPPORT	
	// for apcli DFS, if ra0 not up, don,t send bcn
	bMaskBcn = TRUE;
	
	for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		if(BeaconTransmitRequired(pAd, apidx, &pAd->ApCfg.MBSSID[apidx]))
		{
			bMaskBcn = FALSE;
			break;
		}
	}

	if (bMaskBcn &&  APCLI_IF_UP_CHECK(pAd, 0)) {
			DBGPRINT(RT_DEBUG_OFF, ("Apcli DFS need mask beacon!!!\n"));
			AsicCtrlBcnMask(pAd, 0xFF);
	}
	else {
		AsicCtrlBcnMask(pAd, 0x0);
	}
#endif /* APCLI_SUPPORT */

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
	USHORT			beaconLen = (USHORT) pAd->BeaconTxWI.TxWIMPDUByteCnt;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT32 longptr;
	
#ifdef RT_BIG_ENDIAN
	{
	TXWI_STRUC		localTxWI;
	
	NdisMoveMemory((PUCHAR)&localTxWI, (PUCHAR)&pAd->BeaconTxWI, TXWISize);
	RTMPWIEndianChange(pAd, (PUCHAR)&localTxWI, TYPE_TXWI);
	beaconLen = (USHORT) localTxWI.TxWIMPDUByteCnt;
	}
#endif /* RT_BIG_ENDIAN */

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(TxWIMPDUByteCnt=%d, beaconLen=%d)\n", pAd->BeaconTxWI.TxWIMPDUByteCnt, beaconLen));


	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(ADHOC mode. TxWIMPDUByteCnt = %d)\n", pAd->BeaconTxWI.TxWIMPDUByteCnt));

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
	/*
		(STA ad-hoc mode) Upon the reception of BEACON frame from associated BSS, 
		local TSF is updated with remote TSF only if the remote TSF is greater than local TSF
	*/
	csr9.field.TsfSyncMode = 2; /* sync TSF in IBSS mode*/
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

		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
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
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
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
			}
		}
/*#endif  WIFI_TEST */


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


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
#endif /* CONFIG_AP_SUPPORT */
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


	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;


	
	/* For some reasons, always set it to short slot time.*/
	/* ToDo: Should consider capability with 11B*/

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

	offset = SharedKeyTableBase + (4*BssIndex + KeyIdx)*HW_KEY_ENTRY_SIZE;
	
#ifdef RTMP_MAC_PCI
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
	RTMP_IO_READ32(pAd, SharedKeyModeBase+4*(BssIndex/2), &csr1.word);
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
	UINT16 SharedKeyTableBase, SharedKeyModeBase;
	
	DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

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

	RTMP_IO_READ32(pAd, SharedKeyTableBase+4*(BssIndex/2), &csr1.word);
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
	ASSERT(BssIndex < 4);
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
	==========================================================================
	Description:   

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx) 
{
	UINT32 offset;
	UCHAR cnt, cnt_s, cnt_e;
#ifdef MCS_LUT_SUPPORT
	UINT32 mcs_tb_offset = 0;
#endif /* MCS_LUT_SUPPORT */


	DBGPRINT(RT_DEBUG_TRACE, ("AsicDelWcidTab==>wcid_idx = 0x%x\n",wcid_idx));
	if (wcid_idx == WCID_ALL) {
		cnt_s = 0;
		cnt_e = (WCID_ALL - 1);
	} else {
#ifdef MCS_LUT_SUPPORT
		if (RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT))
			mcs_tb_offset = 0x400;
#endif /* MCS_LUT_SUPPORT */
		cnt_s = cnt_e = wcid_idx;
	}
	
	for (cnt = cnt_s; cnt_s <= cnt_e; cnt_s++)
	{
		offset = MAC_WCID_BASE + cnt * HW_WCID_ENTRY_SIZE;
		RTMP_IO_WRITE32(pAd, offset, 0x0);
		RTMP_IO_WRITE32(pAd, offset + 4, 0x0);
#ifdef MCS_LUT_SUPPORT
		if (mcs_tb_offset) {
			offset += mcs_tb_offset;
			RTMP_IO_WRITE32(pAd, offset, 0x0);
			RTMP_IO_WRITE32(pAd, offset + 4, 0x0);
		}
#endif /* MCS_LUT_SUPPORT */
	}
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
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_MAC_PCI
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
#ifdef RT65xx
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd))
		return TRUE;
#endif /* RT65xx */

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
#ifdef RT65xx
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd)) {
		return TRUE;
	}
#endif /* RT65xx */

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
	UINT8 mode = pAd->chipCap.FlgIsVcoReCalMode;

	if (mode == VCO_CAL_DISABLE)
		return;

#ifdef RTMP_INTERNAL_TX_ALC
#endif /* RTMP_INTERNAL_TX_ALC */

	RTMP_IO_READ32(pAd, TX_PIN_CFG, &TxPinCfg);
	TxPinCfg &= 0xFCFFFFF0;
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

	switch (mode)
	{

#ifdef RLT_RF
		case VCO_CAL_MODE_3:
			rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en*/
			rlt_rf_write(pAd, RF_BANK0, RF_R04, RFValue);
			break;
#endif /* RLT_RF */

		default:
			return;
	}

	RtmpOsMsDelay(1);

	RTMP_IO_READ32(pAd, TX_PIN_CFG, &TxPinCfg);
	if (pAd->CommonCfg.Channel <= 14)
	{
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg |= 0x2;
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg |= 0xA;
		else if (pAd->Antenna.field.TxPath == 3)
			TxPinCfg |= 0x0200000A;
	}
	else
	{
		if (pAd->Antenna.field.TxPath == 1)
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

	regAddr = TX_CHAIN_ADDR0_L + chainIdx * 8;
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


#ifdef WOW_SUPPORT
#endif /* WOW_SUPPORT */


INT AsicSetPreTbttInt(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 val;
	
	RTMP_IO_READ32(pAd, INT_TIMER_CFG, &val);
	val &= 0xffff0000;
	val |= 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */
	RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, val);
	/* Enable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &val);
	val |=0x1;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, val);

	return TRUE;
}


BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;


	do {
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0)) {
			DBGPRINT(RT_DEBUG_TRACE, ("==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word));
			return TRUE;
		}
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;
		RTMPusecDelay(wait_us);
	}while ((i++) < round);

	DBGPRINT(RT_DEBUG_TRACE, ("==>  DMABusy, GloCfg=0x%x\n", GloCfg.word));
	
	return FALSE;
}


#ifdef DOT11_N_SUPPORT
#if defined(RT65xx)
#define MAX_AGG_CNT	32
#elif defined(RT2883) || defined(RT3883)
#define MAX_AGG_CNT	16
#else
#define MAX_AGG_CNT	8
#endif
INT AsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
	UINT32 reg_addr;
	TX_AGG_CNT_STRUC reg_val;
	int i, cnt, seg;
	static USHORT aggReg[] = {
						TX_AGG_CNT, TX_AGG_CNT3,
#if MAX_AGG_CNT > 8
						TX_AGG_CNT4, TX_AGG_CNT7,
#endif
#if MAX_AGG_CNT > 16
						TX_AGG_CNT8, TX_AGG_CNT15, 
#endif
	};


	NdisZeroMemory(aggCnt, cnt_len * sizeof(ULONG));
	seg = (sizeof(aggReg) /sizeof(USHORT));

	cnt = 0;
	for (i = 0; i < seg; i += 2)
	{
		for (reg_addr = aggReg[i] ; reg_addr <= aggReg[i+1] ; reg_addr += 4)
		{
			RTMP_IO_READ32(pAd, reg_addr, &reg_val.word);
			if (cnt < (cnt_len -1)) {
				aggCnt[cnt] = reg_val.field.AggCnt_x;
				aggCnt[cnt+1] = reg_val.field.AggCnt_y;
				DBGPRINT(RT_DEBUG_TRACE, ("%s():Get AggSize at Reg(0x%x) with val(0x%08x) [AGG_%d=>%ld, AGG_%d=>%ld]\n",
						__FUNCTION__, reg_addr, reg_val.word, cnt, aggCnt[cnt], cnt+1, aggCnt[cnt+1]));
				cnt += 2;
			} else {
				DBGPRINT(RT_DEBUG_TRACE, ("%s():Get AggSize at Reg(0x%x) failed, no enough buffer(cnt_len=%d, cnt=%d)\n",
							__FUNCTION__, reg_addr, cnt_len, cnt));
			}
		}
	}

	return TRUE;
}

#endif /* DOT11_N_SUPPORT */


INT AsicSetChannel(RTMP_ADAPTER *pAd, UCHAR ch, UCHAR bw, UCHAR ext_ch, BOOLEAN bScan)
{
	rtmp_bbp_set_bw(pAd, bw);

	/*  Tx/RX : control channel setting */
	rtmp_bbp_set_ctrlch(pAd, ext_ch);
	rtmp_mac_set_ctrlch(pAd, ext_ch);

	/* Let BBP register at 20MHz to do scan */
	AsicSwitchChannel(pAd, ch, bScan);
	AsicLockChannel(pAd, ch);

#ifdef RT28xx
	RT28xx_ch_tunning(pAd, bw);
#endif /* RT28xx */

	return 0;
}

#ifdef RLT_MAC
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

#ifdef MAC_REPEATER_SUPPORT
/*
	==========================================================================
	Description:
		Set Extended MAC Address

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetExtendedMacAddr(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pMacAddr,
	IN UINT32 Idx) 
{
	UINT32		  Addr4 = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("===> AsicSetExtendedMacAddr %x:%x:%x:%x:%x:%x\n",
				PRINT_MAC(pMacAddr)));
	
	Addr4 = (UINT32)(pMacAddr[0]) | 
			(UINT32)(pMacAddr[1] << 8)  | 
			(UINT32)(pMacAddr[2] << 16) |
			(UINT32)(pMacAddr[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_ADDR_EXT0_31_0 + (Idx*8), Addr4);

	Addr4 = 0;
	Addr4 = (ULONG)(pMacAddr[4]) | (ULONG)(pMacAddr[5] << 8);
	/* Enable APCLI mode */
	Addr4 |= 0x10000;

	RTMP_IO_WRITE32(pAd, MAC_ADDR_EXT0_47_32 + (Idx*8), Addr4);
}
#endif /* MAC_REPEATER_SUPPORT */

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
#endif /* RLT_MAC */

