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


static UCHAR *phy_bw_str[] = {"20M", "40M", "80M", "10M"};
char* get_bw_str(int bandwidth)
{
	if (bandwidth >= BW_20 && bandwidth <= BW_10)
		return phy_bw_str[bandwidth];
	else
		return "N/A";
}



UINT32 RtAsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
	RX_STA_CNT0_STRUC RxStaCnt;

	RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt.word);

	return RxStaCnt.field.CrcErr;
}


UINT32 RtAsicGetCCACnt(RTMP_ADAPTER *pAd)
{
	RX_STA_CNT1_STRUC RxStaCnt1;

	RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);

	return RxStaCnt1.field.FalseCca;
}


UINT32 RtAsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{
	UINT32 cnt = 0, reg = 0;

	if (ch_idx == 0)
		reg = CH_BUSY_STA;
	else if (ch_idx == 1)
		reg = CH_BUSY_STA_SEC;

	if (reg)
		RTMP_IO_READ32(pAd, reg, &cnt);

	return cnt;
}


#ifdef FIFO_EXT_SUPPORT
BOOLEAN RtAsicGetFifoTxCnt(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->wcid >= 1 && pEntry->wcid <= 8)
	{
		WCID_TX_CNT_STRUC wcidTxCnt;
		UINT32 regAddr;

		regAddr = WCID_TX_CNT_0 + (pEntry->wcid - 1) * 4;
		RTMP_IO_READ32(pAd, regAddr, &wcidTxCnt.word);

		pEntry->fifoTxSucCnt += wcidTxCnt.field.succCnt;
		pEntry->fifoTxRtyCnt += wcidTxCnt.field.reTryCnt;
	}

	return TRUE;
}


VOID RtAsicFifoExtSet(IN RTMP_ADAPTER *pAd)
{
	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		RTMP_IO_WRITE32(pAd, WCID_MAPPING_0, 0x04030201);
		RTMP_IO_WRITE32(pAd, WCID_MAPPING_1, 0x08070605);
	}
}


VOID RtAsicFifoExtEntryClean(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	WCID_TX_CNT_STRUC wcidTxCnt;
	UINT32 regAddr;

	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		/* We clean the fifo info when MCS is 0 and Aid is from 1~8 */
		if (pEntry->wcid >=1  && pEntry->wcid <= 8)
		{
			regAddr = WCID_TX_CNT_0 + (pEntry->wcid - 1) * 4;
			RTMP_IO_READ32(pAd, regAddr, &wcidTxCnt.word);
		}
	}
}
#endif /* FIFO_EXT_SUPPORT */




INT RtAsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	TX_RTY_CFG_STRUC tx_rty_cfg = {.word = 0};

	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	tx_rty_cfg.field.TxautoFBEnable = ((enable == TRUE) ? 1 : 0);
	RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);

	return TRUE;
}


INT RtAsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
#ifdef RANGE_EXTEND
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xedcba980);
#endif // RANGE_EXTEND //
#ifdef DOT11N_SS3_SUPPORT
	if (pAd->Antenna.field.TxPath >= 3)
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, 0x12111008);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, 0x16151413);
	}
#endif /* DOT11N_SS3_SUPPORT */

	return TRUE;
}


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


VOID RtAsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6], PhyMode = 0x4000;
	USHORT offset;
	UCHAR i;
	UINT32 MacReg = 0;
	UCHAR Channel = HcGetRadioChannel(pAd);
#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */

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
#endif /* DOT11_VHT_AC */

	/* update PHY mode and rate*/
	if (pAd->OpMode == OPMODE_AP)
	{
		/* update PHY mode and rate*/
		if (Channel> 14)
			ProtCfg.field.ProtectRate = PhyMode;
		ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		// Decide Protect Rate for Legacy packet
		if (Channel > 14)
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

#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */

#ifdef CONFIG_FPGA_MODE
//+++Add by shiang for debug
	if (pAd->fpga_ctl.fpga_on & 0x8)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():RtsRate=%d\n",
					__FUNCTION__, pAd->CommonCfg.RtsRate));
	}
//---Add by shiang for debug
#endif /* CONFIG_FPGA_MODE */

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
		if (Channel <= 14) {
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
					Protect[REG_IDX_GF20] = 0x01754004;
					Protect[REG_IDX_GF40] = 0x03f54084;
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */
				break;

 			case 1:
				/* This is "HT non-member protection mode." */
				/* If there may be non-HT STAs my BSS*/
				ProtCfg.word = 0x01744004;	/* PROT_CTRL(17:16) : 0 (None) */
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1 */
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				}
				/* Assign Protection method for 20&40 MHz packets */
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
#endif /* DOT11_VHT_AC */

				break;

			case 2:
				/* If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets */
				ProtCfg.word = 0x01744004;  /* PROT_CTRL(17:16) : 0 (None) */
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1 */
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/* ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				}
				/* Assign Protection method for 40MHz packets */
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
#endif /* DOT11_VHT_AC */
				break;

			case 3:
				/* HT mixed mode. PROTECT ALL!*/
				/* Assign Rate */
				ProtCfg.word = 0x01744004;	/* Duplicaet legacy 24M. BW set 1 */
				ProtCfg4.word = 0x03f44084;
				/* both 20MHz and 40MHz are protected. Whether use RTS or CTS-to-self depends on the */
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/* ERP use Protection bit is set, use protection rate at Clause 18 */
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083 */
				}
				/* Assign Protection method for 20&40 MHz packets */
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
#endif /* DOT11_VHT_AC */
				break;

			case 8:
				/* Special on for Atheros problem n chip. */
				ProtCfg.word = 0x01754004;	/* Duplicaet legacy 24M. BW set 1. */
				ProtCfg4.word = 0x03f54084;
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01750003;	/* ERP use Protection bit is set, use protection rate at Clause 18 */
					ProtCfg4.word = 0x03f50003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083 */
				}

#ifdef ENHANCE_ULTP
				if (pAd->rts_option == 1) {
				        // disable RTS/CTS only when one STA with High MCS rate
				        if (pAd->MacTab.Content.N)

				} else if (pAd->rts_option == 2) {
				        ProtCfg.word = 0x01744004;      /*duplicaet legacy 24M. BW set 1.*/
				        ProtCfg4.word = 0x03f44084;
				        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Enhance RTS/CTS ------\n"));
				}
#endif /* ENHANCE_ULTP */

				Protect[REG_IDX_MM20] = ProtCfg.word; 	/* 0x01754004; */
				Protect[REG_IDX_MM40] = ProtCfg4.word; /* 0x03f54084; */
				Protect[REG_IDX_GF20] = ProtCfg.word; 	/* 0x01754004; */
				Protect[REG_IDX_GF40] = ProtCfg4.word; /* 0x03f54084; */
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
		}
		RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);
	}

#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID RtAsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan)
{
	UCHAR bw;

	if (pAd->chipOps.ChipSwitchChannel)
		pAd->chipOps.ChipSwitchChannel(pAd, Channel, bScan);
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("For this chip, no specified channel switch function!\n"));
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#ifdef ANT_DIVERSITY_SUPPORT
VOID RtAsicAntennaSelect(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		if (pAd->Mlme.OneSecPeriodicRound % 5 == 0)
#endif /* CONFIG_AP_SUPPORT */

	{
		/* patch for AsicSetRxAnt failed*/
		pAd->RxAnt.EvaluatePeriod = 0;

		/* check every 2 second. If rcv-beacon less than 5 in the past 2 second, then AvgRSSI is no longer a */
		/* valid indication of the distance between this AP and its clients.*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)  ||
			OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
		{
			SHORT	realavgrssi1;

			/* if no traffic then reset average rssi to trigger evaluation*/
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
		    	INT	recvPktNum = pAd->RxAnt.RcvPktNum[pAd->RxAnt.Pair1PrimaryRxAnt];

				if (!recvPktNum)
					return;
				APAsicAntennaAvg(pAd, pAd->RxAnt.Pair1PrimaryRxAnt, &realavgrssi1);
			}
#endif /* CONFIG_AP_SUPPORT */

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Ant-realrssi0(%d), Lastrssi0(%d), EvaluateStableCnt=%d\n", realavgrssi1, pAd->RxAnt.Pair1LastAvgRssi, pAd->RxAnt.EvaluateStableCnt));

			/* if the difference between two rssi is larger or less than 5, then evaluate the other antenna*/
			if ((pAd->RxAnt.EvaluateStableCnt < 2) || (realavgrssi1 > (pAd->RxAnt.Pair1LastAvgRssi + 5)) || (realavgrssi1 < (pAd->RxAnt.Pair1LastAvgRssi - 5)))
				AsicEvaluateRxAnt(pAd);

				pAd->RxAnt.Pair1LastAvgRssi = realavgrssi1;
		}
		else
		{
			/* if not connected, always switch antenna to try to connect*/
			UCHAR	temp;

			temp = pAd->RxAnt.Pair1PrimaryRxAnt;
			pAd->RxAnt.Pair1PrimaryRxAnt = pAd->RxAnt.Pair1SecondaryRxAnt;
			pAd->RxAnt.Pair1SecondaryRxAnt = temp;

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmePeriodicExecTimer: no connect, switch to another one to try connection\n"));

			AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
		}
	}
}
#endif /* ANT_DIVERSITY_SUPPORT */


VOID RtAsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
	BBP_CSR_CFG_STRUC	BbpCsr;

	/* Still need to find why BBP agent keeps busy, but in fact, 
	        hardware still function ok. Now clear busy first.	*/
	/* IF chipOps.AsicResetBbpAgent == NULL, run "else" part */
	if (pAd->chipOps.AsicResetBbpAgent != NULL)
	{	
	    pAd->chipOps.AsicResetBbpAgent(pAd);
	}
    else if (pAd->chipCap.MCUType & (SWMCU | M8051 | CR4))
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
                        ("Reset BBP Agent busy bit.!! \n"));
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
 /* CFG_TODO */
VOID RtAsicSetBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR omac_idx)
{
	UINT32 Addr4;
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
	UINT32 regValue;
#endif /* P2P_SUPPORT */

	Addr4 = (UINT32)(pBssid[0]) |
			(UINT32)(pBssid[1] << 8)  |
			(UINT32)(pBssid[2] << 16) |
			(UINT32)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW0, Addr4);
#endif /* HDR_TRANS_SUPPORT */

	Addr4 = 0;
	/* always one BSSID in STA mode*/
	Addr4 = (UINT32)(pBssid[4]) | (ULONG)(pBssid[5] << 8);


	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

#ifdef HDR_TRANS_SUPPORT
	Addr4 |= 0x18000000;
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, Addr4);
#endif /* HDR_TRANS_SUPPORT */

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
	{
		PUCHAR pP2PBssid = &pAd->CurrentAddress[0];

		Addr4 = (UINT32)(pP2PBssid[0]) |
				(UINT32)(pP2PBssid[1] << 8)  |
				(UINT32)(pP2PBssid[2] << 16) |
				(UINT32)(pP2PBssid[3] << 24);
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

#ifdef HDR_TRANS_SUPPORT
		RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW0, Addr4);
#endif /* HDR_TRANS_SUPPORT */

		Addr4 = 0;

		/* always one BSSID in STA mode */
		Addr4 = (UINT32)(pP2PBssid[4]) | (ULONG)(pP2PBssid[5] << 8);

		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

		RTMP_IO_READ32(pAd, MAC_BSSID_DW1, &regValue);
		regValue &= 0x0000FFFF;
		regValue |= (1 << 16);

		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE0)
		{
			if ((pAd->CurrentAddress[5] % 2 != 0)
			)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));

		}
		else
		{
			/*set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014)
			to disable/enable the new MAC address assignment.  */
		    regValue |= (1 << 21);
		}

		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, regValue);
#ifdef HDR_TRANS_SUPPORT
		/*
			point WCID MAC table to 0x1800
			This is for debug.
			But HDR_TRANS doesn't work if you remove it.
			Check after IC formal release.
		*/
		regValue |= 0x18000000;
		RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, regValue);
#endif /* HDR_TRANS_SUPPORT */
	}
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_CONCURRENT_DEVICE */
}


INT RtAsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx)
{
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;

	csr2.field.Byte0 = addr[0];
	csr2.field.Byte1 = addr[1];
	csr2.field.Byte2 = addr[2];
	csr2.field.Byte3 = addr[3];
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW0, csr2.word);

	csr3.word = 0;
	csr3.field.Byte4 = addr[4];
	{
		csr3.field.Byte5 = addr[5];
		csr3.field.U2MeMask = 0xff;
	}
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW1, csr3.word);

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW0, csr2.word);
	csr3.word &= 0xff00ffff;
	csr3.word |= 0x00410000;				// HW test code
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW1, csr3.word);
#endif /* HDR_TRANS_SUPPORT */

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
VOID RtAsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfBcns)
{
	UCHAR NumOfMacs;
	UINT32 regValue;

	RTMP_IO_READ32(pAd, MAC_BSSID_DW1, &regValue);
	regValue &= 0x0000FFFF;

	/*
		Note:
			1.The MAC address of Mesh and AP-Client link are different from Main BSSID.
			2.If the Mesh link is included, its MAC address shall follow the last MBSSID's MAC by increasing 1.
			3.If the AP-Client link is included, its MAC address shall follow the Mesh interface MAC by increasing 1.
	*/
	NumOfMacs = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAX_APCLI_NUM;


	/* set Multiple BSSID mode */
	if (NumOfMacs <= 1)
	{
		pAd->ApCfg.MacMask = ~(1-1);
		/*regValue |= 0x0; */
	}
	else if (NumOfMacs <= 2)
	{
		if ((pAd->CurrentAddress[5] % 2 != 0)
		)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));

		regValue |= (1<<16);
		pAd->ApCfg.MacMask = ~(2-1);
	}
	else if (NumOfMacs <= 4)
	{
		if (pAd->CurrentAddress[5] % 4 != 0)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The 4-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 4\n"));

		regValue |= (2<<16);
		pAd->ApCfg.MacMask = ~(4-1);
	}
	else if (NumOfMacs <= 8)
	{
		if (pAd->CurrentAddress[5] % 8 != 0)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The 8-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 8\n"));

		regValue |= (3<<16);
		pAd->ApCfg.MacMask = ~(8-1);
	}
	else if (NumOfMacs <= 16)
	{
		/* Set MULTI_BSSID_MODE_BIT4 in MAC register 0x1014 */
		regValue |= (1<<22);
		pAd->ApCfg.MacMask = ~(16-1);
	}

	/* set Multiple BSSID Beacon number */
	if (NumOfBcns > 1)
	{
		if (NumOfBcns > 8)
			regValue |= (((NumOfBcns - 1) >> 3) << 23);
		regValue |= (((NumOfBcns - 1) & 0x7)  << 18);
	}

	/* 	set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014)
		to disable/enable the new MAC address assignment.  */
	if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
	{
		regValue |= (1 << 21);
#ifdef ENHANCE_NEW_MBSSID_MODE
		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE2)
			regValue |=  (1 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE3)
			regValue |=  (2 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE4)
			regValue |=  (3 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE5)
			regValue |=  (4 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE6)
			regValue |=  (5 << 24);
#endif /* ENHANCE_NEW_MBSSID_MODE */
	}

	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, regValue);

#ifdef HDR_TRANS_SUPPORT
	/*
		point WCID MAC table to 0x1800
		This is for debug.
		But HDR_TRANS doesn't work if you remove it.
		Check after IC formal release.
	*/
	regValue |= 0x18000000;
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, regValue);
#endif /* HDR_TRANS_SUPPORT */
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT RtAsicSetReptFuncEnable(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 mac_val;

	RTMP_IO_READ32(pAd, MAC_ADDR_EXT_EN, &mac_val);
	if (enable)
		mac_val |= 0x1;
	else
		mac_val &= (~0x1);
	RTMP_IO_WRITE32(pAd, MAC_ADDR_EXT_EN, mac_val);

	return TRUE;
}


VOID RtAsicInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	ULONG offset, Addr;
	UCHAR tempMAC[MAC_ADDR_LEN];

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" %s.\n", __FUNCTION__));

	COPY_MAC_ADDR(tempMAC, pAddr);

	offset = 0x1480 + (HW_WCID_ENTRY_SIZE * CliIdx);
	Addr = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = tempMAC[4] + (tempMAC[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
							PRINT_MAC(tempMAC), CliIdx));

}


VOID RtAsicRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx)
{
	ULONG offset, Addr;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" %s.\n", __FUNCTION__));

	offset = 0x1480 + (HW_WCID_ENTRY_SIZE * CliIdx);
	Addr = 0;
	RTMP_IO_WRITE32(pAd, offset, Addr);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);
}
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT RtAsicSetRxFilter(RTMP_ADAPTER *pAd)
{
	UINT32 rx_filter_flag;

	/* enable RX of MAC block*/
	if ((pAd->OpMode == OPMODE_AP)
	)
	{
		rx_filter_flag = APNORMAL;

#ifdef CONFIG_AP_SUPPORT
#ifdef IDS_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.IdsEnable)
				rx_filter_flag &= (~0x4);	/* Don't drop those not-U2M frames*/
		}
#endif /* IDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	}

	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);

	return TRUE;
}


#ifdef DOT11_N_SUPPORT
INT RtAsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	TX_LINK_CFG_STRUC TxLinkCfg;
	UINT32 Data = 0;

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn =  (bEnable ? 1 : 0);
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	Data &= 0xFFFFFF00;
	if (bEnable) {
		Data |= 0x80;
	} else {
		/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)
			&& (pAd->CommonCfg.bEnableTxBurst == TRUE)
#ifdef DOT11_N_SUPPORT
			&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif /* DOT11_N_SUPPORT */
		)
			Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);


	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter

    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None

    ========================================================================
*/
VOID RtAsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
	TX_LINK_CFG_STRUC  TxLinkCfg;

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);

	TxLinkCfg.field.TxCFAckEn = bPiggyBack;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
}


static INT RtAsicSetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
	UINT32 val, mask, time_mask;

	if (type == INT_TIMER_EN_PRE_TBTT) {
		mask = 0x1;
		timeout = (timeout & 0xffff);
		time_mask = 0xffff;
	}
	else if (type == INT_TIMER_EN_GP_TIMER) {
		mask = 0x2;
		timeout = (timeout & 0xffff) << 16;
		time_mask = (0xffff << 16);
	}
	else
		mask = 0x3;

	RTMP_IO_READ32(pAd, INT_TIMER_EN, &val);
	if (enable == FALSE)
		val &= (~mask);
	else
		val |= mask;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, val);

	if (enable) {
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &val);
		val = (val & (~time_mask)) | timeout;
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, val);
	}

	return TRUE;
}


INT RtAsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx)
{
	UINT32 timeout = 0;

	if (enable == TRUE)
		timeout = 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */

	return RtAsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_PRE_TBTT, timeout);
}


INT RtAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return RtAsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}


INT RtAsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 mac_val;

	/* Note: if bit 0 == 0, the function will be disabled */
	if (enable) {
		/*
			Count EIFS, NAV, RX busy, TX busy as channel busy and
			enable Channel statistic timer (bit 0)
		*/
		mac_val = 0x0000001F;
	}
	else
		mac_val = 0x0;
	RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);

	return TRUE;
}


INT RtAsicGetTsfTime(RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part)
{
	RTMP_IO_READ32(pAd, TSF_TIMER_DW1, high_part);
	RTMP_IO_READ32(pAd, TSF_TIMER_DW0, low_part);

	return TRUE;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID RtAsicDisableSync(RTMP_ADAPTER *pAd)
{
	BCN_TIME_CFG_STRUC csr;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->Disable TSF synchronization\n"));

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
VOID RtAsicEnableBssSync(PRTMP_ADAPTER pAd, USHORT BeaconPeriod)
{
	BCN_TIME_CFG_STRUC csr;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s()\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
/*	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, 0x00000000);*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		csr.field.BeaconInterval = BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
		csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_AP_SUPPORT */
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}


/*CFG_TODO*/
VOID RtAsicEnableApBssSync(RTMP_ADAPTER *pAd, USHORT BeaconPeriod)
{
	BCN_TIME_CFG_STRUC csr;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s()\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);

	csr.field.BeaconInterval = BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr.field.bTsfTicking = 1;
	csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
	csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
	csr.field.bTBTTEnable = 1;

	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}




static UINT32 wmm_cr_addr[] = {
	EDCA_AC0_CFG, /* WMM_PARAM_AC_0 */
	EDCA_AC1_CFG, /* WMM_PARAM_AC_1 */
	EDCA_AC2_CFG, /* WMM_PARAM_AC_2 */
	EDCA_AC3_CFG, /* WMM_PARAM_AC_3 */
};

static RTMP_REG_PAIR wmm_cr_mask[] = {
	{0x000ff, 0}, /* WMM_PARAM_TXOP */
	{0x00f00, 8}, /* WMM_PARAM_AIFSN */
	{0x0f000, 12}, /* WMM_PARAM_CWMIN */
	{0xf0000, 16}, /* WMM_PARAM_CWMAX */
	{0xfffff, 0}, /* WMM_PARAM_ALL */
};


INT RtAsicSetWmmParam(RTMP_ADAPTER *pAd, UINT ac, UINT type, UINT val)
{
	UINT32 addr = 0, cr_val, mask = 0, shift;

	if (ac <= WMM_PARAM_AC_3)
		addr = wmm_cr_addr[ac];

	if (type <= WMM_PARAM_ALL) {
		mask = wmm_cr_mask[type].Register;
		shift = wmm_cr_mask[type].Value;
	}

	if (addr && mask) {
		RTMP_IO_READ32(pAd, addr, &cr_val);
		cr_val &= (~mask);
		cr_val |= (val << shift);
		RTMP_IO_WRITE32(pAd, addr, cr_val);
		return TRUE;
	}

	return FALSE;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID RtAsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm)
{
	EDCA_AC_CFG_STRUC Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC AifsnCsr;
	CWMIN_CSR_STRUC CwminCsr;
	CWMAX_CSR_STRUC CwmaxCsr;
	int i;
	UCHAR PhyMode = HcGetRadioPhyMode(pAd);

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		/*========================================================*/
		/*      MAC Register has a copy .*/
		/*========================================================*/
		if( pAd->CommonCfg.bEnableTxBurst )
		{
			/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
			Ac0Cfg.field.AcTxop = 0x20; /* Suggest by John for TxBurst in HT Mode*/
		}
		else
			Ac0Cfg.field.AcTxop = 0;	/* QID_AC_BE*/
		Ac0Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac0Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac0Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);

		Ac1Cfg.field.AcTxop = 0;	/* QID_AC_BK*/
		Ac1Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac1Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac1Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);

		if (WMODE_EQUAL(PhyMode, WMODE_B))
		{
			Ac2Cfg.field.AcTxop = 192;	/* AC_VI: 192*32us ~= 6ms*/
			Ac3Cfg.field.AcTxop = 96;	/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			Ac2Cfg.field.AcTxop = 94;	/* AC_VI: 94*32us ~= 3ms*/
			Ac3Cfg.field.AcTxop = 47;	/* AC_VO: 47*32us ~= 1.5ms*/
		}
		Ac2Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac2Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac2Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		Ac3Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac3Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac3Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);

		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = 0;		/* QID_AC_BE*/
		csr0.field.Ac1Txop = 0;		/* QID_AC_BK*/
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
		if (WMODE_EQUAL(PhyMode, WMODE_B))
		{
			csr1.field.Ac2Txop = 192;		/* AC_VI: 192*32us ~= 6ms*/
			csr1.field.Ac3Txop = 96;		/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			csr1.field.Ac2Txop = 94;		/* AC_VI: 94*32us ~= 3ms*/
			csr1.field.Ac3Txop = 47;		/* AC_VO: 47*32us ~= 1.5ms*/
		}
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin1 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin2 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin3 = pAd->wmm_cw_min;
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax1 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax2 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax3 = pAd->wmm_cw_max;
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, 0x00002222);
	}
	else
	{
		/*========================================================*/
		/*      MAC Register has a copy.*/
		/*========================================================*/

		/* Modify Cwmin/Cwmax/Txop on queue[QID_AC_VI], Recommend by Jerry 2005/07/27*/
		/* To degrade our VIDEO Queue's throughput for WiFi WMM S3T07 Issue.*/

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


		Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
		Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
		Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
		Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];

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

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
#endif /* CONFIG_AP_SUPPORT */
		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);
	}

	pAd->CommonCfg.RestoreBurstMode = Ac0Cfg.word;
}



#ifdef CONFIG_AP_SUPPORT
/*
	Wirte non-zero value to AC0 TXOP to boost performace
	To pass WMM, AC0 TXOP must be zero.
	It is necessary to turn AC0 TX_OP dynamically.
*/
VOID dynamic_tune_be_tx_op(RTMP_ADAPTER *pAd, ULONG nonBEpackets)
{
	UINT32 RegValue;
	AC_TXOP_CSR0_STRUC csr0;

	if (pAd->CommonCfg.bEnableTxBurst
#ifdef DOT11_N_SUPPORT
		|| pAd->CommonCfg.bRdg
		|| pAd->CommonCfg.bRalinkBurstMode
#endif /* DOT11_N_SUPPORT */
	)
	{
		if (
#ifdef DOT11_N_SUPPORT
			(pAd->WIFItestbed.bGreenField && pAd->MacTab.fAnyStationNonGF[0] == TRUE) ||
			((pAd->OneSecondnonBEpackets > nonBEpackets) || pAd->MacTab.fAnyStationMIMOPSDynamic) ||
#endif /* DOT11_N_SUPPORT */
			(pAd->MacTab.fAnyTxOPForceDisable))
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE))
			{
				RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &RegValue);

				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
				{
					RegValue = pAd->CommonCfg.RestoreBurstMode;
					RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);
				}

				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
				{
					TX_LINK_CFG_STRUC   TxLinkCfg;

					RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
					TxLinkCfg.field.TxRDGEn = 0;
					RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

					RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
				}
				/* disable AC0(BE) TX_OP */
				RegValue  &= 0xFFFFFF00; /* for WMM test */
				/*if ((RegValue & 0x0000FF00) == 0x00004300) */
				/*	RegValue += 0x00001100; */
				RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, RegValue);
				if (pAd->CommonCfg.APEdcaParm[0].Txop[QID_AC_VO] != 102)
				{
					csr0.field.Ac0Txop = 0;		/* QID_AC_BE */
				}
				else
				{
					/* for legacy b mode STA */
					csr0.field.Ac0Txop = 10;		/* QID_AC_BE */
				}
				csr0.field.Ac1Txop = 0;		/* QID_AC_BK */
				RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
			}
		}
		else
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)==0)
			{
				/* enable AC0(BE) TX_OP */
				UCHAR	txop_value_burst = 0x20;	/* default txop for Tx-Burst */
				UCHAR   txop_value;

#ifdef LINUX
#endif /* LINUX */

				RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &RegValue);

				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
					txop_value = 0x80;
				else if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
					txop_value = 0x80;
				else if (pAd->CommonCfg.bEnableTxBurst)
					txop_value = txop_value_burst;
				else
					txop_value = 0;

				RegValue  &= 0xFFFFFF00;
				/*if ((RegValue & 0x0000FF00) == 0x00005400)
					RegValue -= 0x00001100; */
				/*txop_value = 0; */
				RegValue  |= txop_value;  /* for performance, set the TXOP to non-zero */
				RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, RegValue);
				csr0.field.Ac0Txop = txop_value;	/* QID_AC_BE */
				csr0.field.Ac1Txop = 0;				/* QID_AC_BK */
				RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
			}
		}
	}
	pAd->OneSecondnonBEpackets = 0;
}
#endif /* CONFIG_AP_SUPPORT */


INT RtAsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
	TX_RTY_CFG_STRUC tx_rty_cfg;

	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	if (type == TX_RTY_CFG_RTY_LIMIT_SHORT)
		tx_rty_cfg.field.ShortRtyLimit = limit;
	else if (type == TX_RTY_CFG_RTY_LIMIT_LONG)
		tx_rty_cfg.field.LongRtyLimit = limit;
	RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);

	return TRUE;
}


UINT32 RtAsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
	TX_RTY_CFG_STRUC tx_rty_cfg = {.word = 0};

	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	if (type == TX_RTY_CFG_RTY_LIMIT_SHORT)
		return tx_rty_cfg.field.ShortRtyLimit;
	else if (type == TX_RTY_CFG_RTY_LIMIT_LONG)
		return tx_rty_cfg.field.LongRtyLimit;

	return 0;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID RtAsicSetSlotTime(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime)
{
	UINT32	RegValue = 0;

	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	RegValue = RegValue & 0xFFFFFF00;

	RegValue |= SlotTime;

	RTMP_IO_WRITE32(pAd, BKOFF_SLOT_CFG, RegValue);
}




INT RtAsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

#ifdef RTMP_MAC_PCI
	/* TODO: check MACVersion, currently, rbus-based chip use this.*/
	if (pAd->MACVersion == 0x28720200)
	{
		/*Maximum PSDU length from 16K to 32K bytes	*/
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &mac_val);
		mac_val &= ~(0x3<<12);
		mac_val |= (0x2<<12);
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, mac_val);
	}
#endif /* RTMP_MAC_PCI */

	if ((IS_RT3883(pAd)) || IS_RT65XX(pAd) ||
		((pAd->MACVersion >= RALINK_2880E_VERSION) &&
		(pAd->MACVersion < RALINK_3070_VERSION))) /* 3*3*/
	{
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &mac_val);
		{
			mac_val &= 0xFFF;
			mac_val |= 0x2000;
		}
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, mac_val);
	}

#ifdef DOT11_N_SUPPORT



#endif /* DOT11_N_SUPPORT */

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
VOID RtAsicGetTxTsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
	USHORT Wcid;
	USHORT offset;
	UCHAR IvEiv[8];
	INT i;

	/* Initial value */
	os_zero_mem(IvEiv, 8);
	os_zero_mem(pTxTsc, 6);

	/* Get apidx for this BSSID */
	GET_GroupKey_WCID(wdev, Wcid);

	/* When the group rekey action is triggered, a count-down(3 seconds) is started.
	   During the count-down, use the initial PN as TSC.
	   Otherwise, get the IVEIV from ASIC. */
	if (pAd->ApCfg.MBSSID[wdev->func_idx].RekeyCountDown > 0)
	{
		/*
		In IEEE 802.11-2007 8.3.3.4.3 described :
		The PN shall be implemented as a 48-bit monotonically incrementing
		non-negative integer, initialized to 1 when the corresponding
		temporal key is initialized or refreshed. */
		IvEiv[0] = 1;
	}
	else
	{
		UINT32 temp1, temp2;
		UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */
		/* Read IVEIV from Asic */
		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);

		/* Use Read32 to avoid endian problem */
		RTMP_IO_READ32(pAd, offset, &temp1);
		RTMP_IO_READ32(pAd, offset+4, &temp2);
		for ( i=0; i<4; i++)
		{
			IvEiv[i] = (UCHAR)(temp1 >> (i*8));
			IvEiv[i+4] = (UCHAR)(temp2 >> (i*8));
		}
	}

	/* Record current TxTsc */
	if (wdev->GroupKeyWepStatus == Ndis802_11AESEnable)
	{	/* AES */
		*pTxTsc 	= IvEiv[0];
		*(pTxTsc+1) = IvEiv[1];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];
	}
	else
	{	/* TKIP */
		*pTxTsc 	= IvEiv[2];
		*(pTxTsc+1) = IvEiv[0];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n",
						__FUNCTION__, Wcid,
						*pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));


}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================
	Description:
		Add Shared key information into ASIC.
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.

    Return:
	========================================================================
*/
VOID RtAsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	ULONG offset; /*, csr0;*/
	SHAREDKEY_MODE_STRUC csr1;
	UCHAR org_bssindex;
#ifdef RTMP_MAC_PCI
	INT   i;
#endif /* RTMP_MAC_PCI */

	PUCHAR		pKey = pCipherKey->Key;
	PUCHAR		pTxMic = pCipherKey->TxMic;
	PUCHAR		pRxMic = pCipherKey->RxMic;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): BssIndex=%d, KeyIdx=%d\n",
				__FUNCTION__, BssIndex,KeyIdx));
/*============================================================================================*/

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): %s key #%d\n",
				__FUNCTION__, CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
/*============================================================================================*/

	org_bssindex = BssIndex;
	if (BssIndex >= 8)
		BssIndex -= 8;

	{
		/* fill key material - key + TX MIC + RX MIC*/
		UINT32 share_key_base = 0, share_key_size = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			if (org_bssindex >= 8)
				share_key_base = RLT_SHARED_KEY_TABLE_BASE_EXT;
			else
				share_key_base = RLT_SHARED_KEY_TABLE_BASE;
			share_key_size = RLT_HW_KEY_ENTRY_SIZE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			if (org_bssindex >= 8)
				share_key_base = SHARED_KEY_TABLE_BASE_EXT;
			else
				share_key_base = SHARED_KEY_TABLE_BASE;
			share_key_size = HW_KEY_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		offset = share_key_base + (4*BssIndex + KeyIdx)*share_key_size;
#ifdef RTMP_MAC_PCI
		{
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
		}
#endif /* RTMP_MAC_PCI */

	}

	{
		UINT32 share_key_mode_base = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			if (org_bssindex >= 8)
				share_key_mode_base = RLT_SHARED_KEY_MODE_BASE_EXT;
			else
				share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			if (org_bssindex >= 8)
				share_key_mode_base = SHARED_KEY_MODE_BASE_EXT;
			else
				share_key_mode_base = SHARED_KEY_MODE_BASE;
		}
#endif /* RTMP_MAC */

		/* Update cipher algorithm. WSTA always use BSS0*/
		RTMP_IO_READ32(pAd, share_key_mode_base + 4 * (BssIndex/2), &csr1.word);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Read: SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", BssIndex,KeyIdx, csr1.word));
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
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
		RTMP_IO_WRITE32(pAd, share_key_mode_base+ 4 * (BssIndex/2), csr1.word);
	}
}


/*	IRQL = DISPATCH_LEVEL*/
VOID RtAsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	/*ULONG SecCsr0;*/
	SHAREDKEY_MODE_STRUC csr1;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():#%d \n",
					__FUNCTION__, BssIndex*4 + KeyIdx));

	{
		UINT32 share_key_mode_base = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			share_key_mode_base = SHARED_KEY_MODE_BASE;
#endif /* RTMP_MAC */

		RTMP_IO_READ32(pAd, share_key_mode_base+4*(BssIndex/2), &csr1.word);
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
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
		RTMP_IO_WRITE32(pAd, share_key_mode_base+4*(BssIndex/2), csr1.word);
	}
	ASSERT(BssIndex < 4);
	ASSERT(KeyIdx < 4);

}


VOID RtAsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	ULONG	offset;
	UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
		iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
		iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
	}
#endif /* RTMP_MAC */

	offset = iveiv_tb_base + (WCID * iveiv_tb_size);

	RTMP_IO_WRITE32(pAd, offset, uIV);
	RTMP_IO_WRITE32(pAd, offset + 4, uEIV);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid(%d) 0x%08lx, 0x%08lx \n",
									__FUNCTION__, WCID, uIV, uEIV));
}


#ifdef MCS_LUT_SUPPORT
VOID RtAsicMcsLutUpdate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef PEER_DELBA_TX_ADAPT
	if (pEntry->bPeerDelBaTxAdaptEn)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s(): Rate not update due to bPeerDelBaTxAdaptEn is 0x%x.\n",
				__FUNCTION__, pEntry->bPeerDelBaTxAdaptEn));
		return;
	}
#endif /* PEER_DELBA_TX_ADAPT */

	if(RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) && (pEntry->wcid < 128))
	{
		UINT32 wcid_offset;
		HW_RATE_CTRL_STRUCT rate_ctrl;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
			rate_ctrl.RATE_CTRL_N.PHYMODE = pEntry->HTPhyMode.field.MODE;
			rate_ctrl.RATE_CTRL_N.STBC = pEntry->HTPhyMode.field.STBC;
			rate_ctrl.RATE_CTRL_N.ShortGI = pEntry->HTPhyMode.field.ShortGI;
			rate_ctrl.RATE_CTRL_N.BW = pEntry->HTPhyMode.field.BW;

			if (pAd->chipCap.phy_caps & fPHY_CAP_LDPC)
				rate_ctrl.RATE_CTRL_N.ldpc = pEntry->HTPhyMode.field.ldpc;

			rate_ctrl.RATE_CTRL_N.MCS = pEntry->HTPhyMode.field.MCS;
#ifdef TXBF_SUPPORT
			rate_ctrl.RATE_CTRL_N.eTxBF = pEntry->HTPhyMode.field.eTxBF;
			rate_ctrl.RATE_CTRL_N.iTxBF = pEntry->HTPhyMode.field.iTxBF;

			printk("pEntry->HTPhyMode.field.eTxBF = %d, pEntry->HTPhyMode.field.iTxBF = %d\n",
				       pEntry->HTPhyMode.field.eTxBF, pEntry->HTPhyMode.field.iTxBF);

			if (pEntry->HTPhyMode.field.eTxBF || pEntry->HTPhyMode.field.iTxBF)
				rate_ctrl.RATE_CTRL_N.STBC = FALSE;
#endif
		}
		else
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
		{
			rate_ctrl.RATE_CTRL_O.PHYMODE = pEntry->HTPhyMode.field.MODE;
			rate_ctrl.RATE_CTRL_O.STBC = pEntry->HTPhyMode.field.STBC;
			rate_ctrl.RATE_CTRL_O.ShortGI = pEntry->HTPhyMode.field.ShortGI;
			rate_ctrl.RATE_CTRL_O.BW = pEntry->HTPhyMode.field.BW;
			rate_ctrl.RATE_CTRL_O.MCS = pEntry->HTPhyMode.field.MCS;
#ifdef TXBF_SUPPORT
			rate_ctrl.RATE_CTRL_O.eTxBF = pEntry->HTPhyMode.field.eTxBF;
			rate_ctrl.RATE_CTRL_O.iTxBF = pEntry->HTPhyMode.field.iTxBF;

			if (pEntry->HTPhyMode.field.eTxBF || pEntry->HTPhyMode.field.iTxBF)
				rate_ctrl.RATE_CTRL_O.STBC = FALSE;
#endif
		}
		else
#endif /* RTMP_MAC */
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): HIF Type Error !!!\n", __FUNCTION__));
			return;
		}

		wcid_offset = MAC_MCS_LUT_BASE + (pEntry->wcid * 8);

		RTMP_IO_WRITE32(pAd, wcid_offset, rate_ctrl.word);
		RTMP_IO_WRITE32(pAd, wcid_offset + 4, 0x00);

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():MCS_LUT update, write to MAC=0x%08x, Value=0x%04x, WCID=%d\n",
					__FUNCTION__, wcid_offset, pEntry->HTPhyMode.word, pEntry->wcid));

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\tWcid=%d, APMlmeSetTxRate - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d \n"
			                                     "\                            ETxBf=%d, ITxBf=%d\n\n",
			pEntry->wcid,
			pEntry->CurrTxRateIndex,
			pEntry->HTPhyMode.field.MCS,
			pEntry->HTPhyMode.field.STBC,
			pEntry->HTPhyMode.field.ShortGI,
			pEntry->HTPhyMode.field.MODE,
			pEntry->HTPhyMode.field.BW,
			pEntry->HTPhyMode.field.eTxBF,
			pEntry->HTPhyMode.field.iTxBF));
	}
}
#endif /* MCS_LUT_SUPPORT */


VOID RtAsicUpdateBASession(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UCHAR basize, BOOLEAN isAdd, INT ses_type)
{
	UINT32 Value = 0, Offset;

	if (ses_type == BA_SESSION_RECP) {
		Offset = MAC_WCID_BASE + wcid * HW_WCID_ENTRY_SIZE + 4;
		RTMP_IO_READ32(pAd, Offset, &Value);
		if (isAdd)
			Value |= (0x10000 << tid);
		else
			Value &= (~(0x10000 << tid));
		RTMP_IO_WRITE32(pAd, Offset, Value);
	}
}


VOID RtAsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr)
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
VOID RtAsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
	WCID_ATTRIBUTE_STRUC WCIDAttri;
	USHORT offset;
	UINT32 wcid_attr_base = 0, wcid_attr_size = 0;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
		wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
		wcid_attr_size = HW_WCID_ATTRI_SIZE;
	}
#endif /* RTMP_MAC */

	/* Initialize the content of WCID Attribue  */
	WCIDAttri.word = 0;

	/* The limitation of HW WCID table */
	if (Wcid > 254)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:Invalid wcid(%d)\n", __FUNCTION__, Wcid));
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
	offset = wcid_attr_base + (Wcid * wcid_attr_size);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri.word);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:WCID #%d, KeyIdx #%d, WCIDAttri=0x%x, Alg=%s\n",
					__FUNCTION__, Wcid, KeyIdx, WCIDAttri.word, CipherName[CipherAlg]));
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID RtAsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
	UINT32 offset;
	UCHAR cnt, cnt_s, cnt_e;
#ifdef MCS_LUT_SUPPORT
	UINT32 mcs_tb_offset = 0;
#endif /* MCS_LUT_SUPPORT */


	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():wcid_idx=0x%x\n",
				__FUNCTION__, wcid_idx));
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
VOID RtAsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
	INT i;
	ULONG offset;
	UINT32 pairwise_key_base = 0, pairwise_key_len = 0;
	UCHAR *pKey = pCipherKey->Key;
	UCHAR *pTxMic = pCipherKey->TxMic;
	UCHAR *pRxMic = pCipherKey->RxMic;
	UCHAR CipherAlg = pCipherKey->CipherAlg;
#ifdef RTMP_MAC
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
#endif /* RTMP_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		pairwise_key_base = RLT_PAIRWISE_KEY_TABLE_BASE;
		pairwise_key_len = RLT_HW_KEY_ENTRY_SIZE;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		pairwise_key_base = PAIRWISE_KEY_TABLE_BASE;
		pairwise_key_len = HW_KEY_ENTRY_SIZE;
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
	}
#endif /* RTMP_MAC */

	/* EKEY */
	offset = pairwise_key_base + (WCID * pairwise_key_len);
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

	/*  MIC KEY */
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
#ifdef RTMP_MAC
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
#endif /* RTMP_MAC */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WCID #%d Alg=%s\n",
				__FUNCTION__, WCID, CipherName[CipherAlg]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
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
VOID RtAsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	RtAsicUpdateWcidAttributeEntry(pAd,
							  BSS0,
							  0,
							  CIPHER_NONE,
							  Wcid,
							  PAIRWISEKEYTABLE);
}


INT RtAsicSendCommandToMcu(
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


BOOLEAN RtAsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	BOOLEAN cmd_done = TRUE;

	RtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);

#ifdef RTMP_MAC_PCI
	cmd_done = AsicCheckCommanOk(pAd, Token);
#endif /* RTMP_MAC_PCI */

	return cmd_done;
}


BOOLEAN RtAsicSendCommandToMcuBBP(
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


VOID RtAsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
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
						/*if (pAd->StaCfg[0].bRadio == FALSE)*/
						if (1)
						{
							RTMP_RF_IO_WRITE32(pAd, R3);

							MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():#%d(RF=%d, ) , R2=0x%08x,  R3 = 0x%08x \n",
								__FUNCTION__, Channel, pAd->RfIcType, R2, R3));
						}
						else
							MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():#%d(RF=%d, ) , R2=0x%08x \n",
								__FUNCTION__, Channel, pAd->RfIcType, R2));
						break;
					}
				}
				break;
#endif /* defined(RT28xx) || defined(RT2880) || defined(RT2883) */
			default:
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():#%d : Unkonwn RFIC=%d\n",
							__FUNCTION__, Channel, pAd->RfIcType));
				break;
		}
	}
}


#ifdef WAPI_SUPPORT
VOID RtAsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high)
{
	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		UINT32 offset;
		UINT32 wapi_pn_base = 0, wapi_pn_size = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			wapi_pn_base = RLT_WAPI_PN_TABLE_BASE;
			wapi_pn_size = RLT_WAPI_PN_ENTRY_SIZE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			wapi_pn_base = WAPI_PN_TABLE_BASE;
			wapi_pn_size = WAPI_PN_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		offset = wapi_pn_base + (WCID * wapi_pn_size);

		RTMP_IO_WRITE32(pAd, offset, pn_low);
		RTMP_IO_WRITE32(pAd, offset + 4, pn_high);
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s : Not support HW_WAPI_PN_TABLE\n",
								__FUNCTION__));
	}

}
#endif /* WAPI_SUPPORT */





#ifdef STREAM_MODE_SUPPORT
UINT32 RtStreamModeRegVal(RTMP_ADAPTER *pAd)
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
VOID RtAsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
	UINT32 streamWord;
	UINT32 regAddr, regVal;


	if (!pAd->chipCap.FlgHwStreamMode)
		return;

	streamWord = RtStreamModeRegVal(pAd);
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


VOID RtAsicStreamModeInit(RTMP_ADAPTER *pAd)
{
	int chainIdx;
	UCHAR *pMacAddr;

	if (pAd->chipCap.FlgHwStreamMode == FALSE)
		return;

	for (chainIdx = 0; chainIdx < STREAM_MODE_STA_NUM; chainIdx++)
	{
		pMacAddr = &pAd->CommonCfg.StreamModeMac[chainIdx][0];
		RtAsicSetStreamMode(pAd, pMacAddr, chainIdx, TRUE);
	}
}
#endif // STREAM_MODE_SUPPORT //


VOID RtAsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	AUTO_RSP_CFG_STRUC csr4;

	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &csr4.word);
	if (TxPreamble == Rt802_11PreambleLong)
		csr4.field.AutoResponderPreamble = 0;
	else
		csr4.field.AutoResponderPreamble = 1;
	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, csr4.word);
}


#ifdef DOT11_N_SUPPORT
#define MAX_AGG_CNT	8
INT RtAsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
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
#if MAX_AGG_CNT > 32
						TX_AGG_CNT16, TX_AGG_CNT23,
#endif
	};


	os_zero_mem(aggCnt, cnt_len * sizeof(ULONG));
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
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Get AggSize at Reg(0x%x) with val(0x%08x) [AGG_%d=>%ld, AGG_%d=>%ld]\n",
						__FUNCTION__, reg_addr, reg_val.word, cnt, aggCnt[cnt], cnt+1, aggCnt[cnt+1]));
				cnt += 2;
			} else {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Get AggSize at Reg(0x%x) failed, no enough buffer(cnt_len=%d, cnt=%d)\n",
							__FUNCTION__, reg_addr, cnt_len, cnt));
			}
		}
	}

	return TRUE;
}


INT RtAsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 Data = 0;

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	if (enable)
	{
		pAd->CommonCfg.RestoreBurstMode = Data;
		Data  &= 0xFFF00000;
		Data  |= 0x86380;
	} else {
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
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

	if (enable)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);
	else
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);

	return TRUE;
}
#endif // DOT11_N_SUPPORT //




INT RtAsicWaitMacTxRxIdle(RTMP_ADAPTER *pAd)
{
	UINT32 Index = 0, val;

	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;

		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &val);
		if ((val & 0x03) == 0)	/* if BB.RF is stable*/
			return TRUE;

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Check if MAC_STATUS_CFG is busy(=%x)\n", val));
		RtmpusecDelay(1000);
	} while (Index++ < 100);

	return FALSE;
}


INT RtAsicSetMacTxRx(RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable)
{
	UINT32 mac_val, val_field;

	switch (txrx)
	{
		case ASIC_MAC_TX:
			val_field = MAC_SYS_CTRL_TXEN;
			break;
		case ASIC_MAC_RX:
			val_field = MAC_SYS_CTRL_RXEN;
			break;
		case ASIC_MAC_TXRX:
			val_field = MAC_SYS_CTRL_RXEN | MAC_SYS_CTRL_TXEN;
			break;
		default:
			val_field = 0;
			break;
	}

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &mac_val);
	if (enable)
		mac_val |= val_field;
	else
		mac_val &= (~val_field);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, mac_val);

	return TRUE;
}


INT RtAsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
#ifdef RTMP_MAC_PCI
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG , &GloCfg.word);
	if (enable == TRUE)
	{
		GloCfg.field.EnableTxDMA = 1;
		GloCfg.field.EnableRxDMA = 1;
		GloCfg.field.EnTXWriteBackDDONE = 1;
		GloCfg.field.WPDMABurstSIZE = pAd->chipCap.WPDMABurstSIZE;
	} else {
		GloCfg.field.EnableRxDMA = 0;
		GloCfg.field.EnableTxDMA = 0;
		GloCfg.field.EnTXWriteBackDDONE = 0;
	}
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
#endif /* RTMP_MAC_PCI */
	return TRUE;
}


BOOLEAN RtAsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
#ifdef RTMP_MAC_PCI
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;


	do {
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word));
			return TRUE;
		}
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;
		RtmpusecDelay(wait_us);
	}while ((i++) < round);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMABusy, GloCfg=0x%x\n", GloCfg.word));
#endif /* RTMP_MAC_PCI */
	return FALSE;
}


INT RtAsicSetMacWD(RTMP_ADAPTER *pAd)
{
	int count = 0;
	BOOLEAN MAC_ready = FALSE;
	UINT32	MacCsr12 = 0;

	/* Disable MAC*/
	RtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, FALSE);

	/* polling MAC status*/
	while (count < 10)
	{
		RtmpusecDelay(1000);
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacCsr12);

		/* if MAC is idle*/
		if ((MacCsr12 & 0x03) == 0)
		{
			MAC_ready = TRUE;
			break;
		}
		count ++;
	}

	if (MAC_ready)
	{
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
		RtmpusecDelay(1);
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Warning, MAC isn't ready \n"));
	}

	RtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	return TRUE;
}


INT RtAsicTOPInit(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

#ifdef RLT_MAC

	if (IS_MT76x0(pAd) || IS_MT76x2(pAd) || IS_MT7601(pAd)) {
		if (pAd->WlanFunCtrl.field.WLAN_EN == 0)
			rlt_wlan_chip_onoff(pAd, TRUE, FALSE);
	}
#endif /* RLT_MAC */



	/* Make sure MAC gets ready.*/
	if (WaitForAsicReady(pAd) != TRUE)
		return FALSE;

#ifdef RTMP_MAC_PCI

		mac_val = 0x2;	/* To fix driver disable/enable hang issue when radio off*/
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, mac_val);
#endif /* RTMP_MAC_PCI */

	return TRUE;
}


INT RtStopDmaRx(RTMP_ADAPTER *pAd, UCHAR Level)
{
	PNDIS_PACKET pRxPacket;
	RX_BLK RxBlk, *pRxBlk;
	UINT32 RxPending = 0, MacReg = 0, MTxCycle = 0;
	BOOLEAN bReschedule = FALSE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> %s\n", __FUNCTION__));

	/*
		process whole rx ring
	*/
	while (1)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return 0;
		pRxBlk = &RxBlk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, 0);
		if ((RxPending == 0) && (bReschedule == FALSE))
			break;
		else
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
	}

	/*
		Check DMA Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI

		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		if (MacReg & 0x8)
		{
			RtmpusecDelay(50);
		}
		else
			break;

#endif


		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:RX DMA busy!! DMA_CFG = 0x%08x\n", __FUNCTION__, MacReg));
	}

	if (Level == RTMP_HALT)
	{
		/* Disable DMA RX */
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x4);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, MacReg);
#endif
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


INT RtStopDmaTx(RTMP_ADAPTER *pAd, UCHAR Level)
{
	UINT32 MacReg = 0, MTxCycle = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> %s\n", __FUNCTION__));

	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		if ((MacReg & 0x2) == 0)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMA Tx Idle, MacReg=0x%x\n", MacReg));
			break;
		}
#endif


		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TX DMA busy!! DMA_CFG(%x)\n", MacReg));
	}

	if (Level == RTMP_HALT)
	{
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x00000001);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, MacReg);
#endif

	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


INT RtAsicSetTxStream(RTMP_ADAPTER *pAd, UCHAR opmode, BOOLEAN up)
{
	/*
		Select DAC according to HT or Legacy, write to BBP R1(bit4:3)
		In HT mode and two stream mode, both DACs are selected.
		In legacy mode or one stream mode, DAC-0 is selected.
	*/
	UCHAR PhyMode = HcGetRadioPhyMode(pAd);
#ifdef CONFIG_AP_SUPPORT
	if (opmode == OPMODE_AP)
	{

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) && (pAd->Antenna.field.TxPath == 2))
			bbp_set_txdac(pAd, 2);
		else
#endif /* DOT11_N_SUPPORT */
			bbp_set_txdac(pAd, 0);
	}
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}


INT RtAsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 rx_path)
{
	/* Receiver Antenna selection */
	bbp_set_rxpath(pAd, rx_path);

	return TRUE;
}


INT RtAsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
	return bbp_set_bw(pAd, bw, BandIdx);
}


#ifdef MAC_APCLI_SUPPORT
/*
	==========================================================================
	Description:
		Set BSSID of Root AP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID RtAsicSetApCliBssid(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN UCHAR index)
{
	UINT32		  Addr4 = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%x:%x:%x:%x:%x:%x\n",
				__FUNCTION__, PRINT_MAC(pBssid)));

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

#ifdef NEW_WOW_SUPPORT
VOID RT28xxAndesWOWEnable(RTMP_ADAPTER *pAd)
{
	NEW_WOW_MASK_CFG_STRUCT mask_cfg;
	NEW_WOW_SEC_CFG_STRUCT sec_cfg;
	NEW_WOW_INFRA_CFG_STRUCT infra_cfg;
	NEW_WOW_P2P_CFG_STRUCT p2p_cfg;
	NEW_WOW_PARAM_STRUCT wow_param;
	struct CMD_UNIT CmdUnit;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	INT32 Ret;
	MAC_TABLE_ENTRY *pEntry = NULL;


	os_zero_mem(&CmdUnit, sizeof(CmdUnit));

	/* WOW enable */
	os_zero_mem(&wow_param, sizeof(wow_param));

	wow_param.Parameter = WOW_ENABLE; /* WOW enable */
	wow_param.Value = TRUE;

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&wow_param;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	RtmpOsMsDelay(1);
	/* mask configuration */
	os_zero_mem(&mask_cfg, sizeof(mask_cfg));

	mask_cfg.Config_Type = WOW_MASK_CFG; 	/* detect mask config */
	mask_cfg.Function_Enable = TRUE;
	mask_cfg.Detect_Mask = 1UL << WOW_MAGIC_PKT;	/* magic packet */
	mask_cfg.Event_Mask = 0;

	CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_MASK_CFG_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&mask_cfg;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed!!(%d/%d)\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, mask_cfg.Config_Type);
		return;
	}

	RtmpOsMsDelay(1);

	/* security configuration */
	if (pAd->StaCfg[0].AuthMode >= Ndis802_11AuthModeWPAPSK)
	{
		os_zero_mem(&sec_cfg, sizeof(sec_cfg));

		sec_cfg.Config_Type = WOW_SEC_CFG; 	/* security config */

		if (pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeWPAPSK)
			sec_cfg.WPA_Ver = 0;
		else if (pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeWPA2PSK)
			sec_cfg.WPA_Ver = 1;

		pEntry = &pAd->MacTab.Content[BSSID_WCID];

		NdisCopyMemory(sec_cfg.PTK, pEntry->PTK, 64);
		NdisCopyMemory(sec_cfg.R_COUNTER, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);

		sec_cfg.Key_Id = pAd->StaCfg[0].DefaultKeyId;
		sec_cfg.Cipher_Alg = pEntry->WepStatus;
		printk("\x1b[31m%s: wep status %d\x1b[m\n", __FUNCTION__, pEntry->WepStatus);
		sec_cfg.Group_Cipher = pAd->StaCfg[0].GroupCipher;
		printk("\x1b[31m%s: group status %d\x1b[m\n", __FUNCTION__, sec_cfg.Group_Cipher);
		printk("\x1b[31m%s: aid %d\x1b[m\n", __FUNCTION__, pEntry->Aid);
		sec_cfg.WCID = BSSID_WCID;

		CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
		CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_SEC_CFG_STRUCT);
		CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&sec_cfg;

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

		if (Ret != NDIS_STATUS_SUCCESS)
		{
			printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, sec_cfg.Config_Type);
			return;
		}

		RtmpOsMsDelay(1);
	}

	/* Infra configuration */

	os_zero_mem(&infra_cfg, sizeof(infra_cfg));

	infra_cfg.Config_Type = WOW_INFRA_CFG; 	/* infra config */

	COPY_MAC_ADDR(infra_cfg.STA_MAC, pAd->CurrentAddress);
	COPY_MAC_ADDR(infra_cfg.AP_MAC, pAd->CommonCfg.Bssid);

	CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_INFRA_CFG_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&infra_cfg;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		infra_cfg.AP_Status = TRUE;
	else
		infra_cfg.AP_Status = FALSE;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, infra_cfg.Config_Type);
		return;
	}

	RtmpOsMsDelay(1);


	/* P2P configuration */

	/* Wakeup Option */
	os_zero_mem(&wow_param, sizeof(wow_param));

	wow_param.Parameter = WOW_WAKEUP; /* Wakeup Option */
	if (pAd->WOW_Cfg.bInBand)
	{
#ifdef RTMP_MAC_PCI
		wow_param.Value = WOW_WAKEUP_BY_PCIE;
#else
		wow_param.Value = WOW_WAKEUP_BY_USB;
#endif /* RTMP_MAC_PCI */
	}
	else
	{
		INT32 Value;

		wow_param.Value = WOW_WAKEUP_BY_GPIO;

		RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &Value);
		printk("\x1b[31m%s: 0x80 = %x\x1b[m\n", __FUNCTION__, Value);
		Value &= ~0x01010000; /* GPIO0(ouput) --> 0(data) */
		RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, Value);
	}

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&wow_param;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	RtmpOsMsDelay(1);


	/* traffic to Andes */
	os_zero_mem(&wow_param, sizeof(wow_param));
	wow_param.Parameter = WOW_TRAFFIC; /* Traffic switch */
	wow_param.Value = WOW_PKT_TO_ANDES;	/* incoming packet to FW */

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&wow_param.Parameter;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	RtmpOsMsDelay(1);

    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
}


VOID RT28xxAndesWOWDisable(RTMP_ADAPTER *pAd)
{
    NEW_WOW_PARAM_STRUCT param;
    struct CMD_UNIT CmdUnit;
    RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
    INT32 Ret;
    UINT32 Value;
    MAC_TABLE_ENTRY *pEntry = NULL;

    printk("\x1b[31m%s: ...\x1b[m", __FUNCTION__);

    /* clean BulkIn Reset flag */
    //pAd->Flags &= ~0x80000;
    RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

    os_zero_mem(&CmdUnit, sizeof(CmdUnit));

    /* WOW disable */
    os_zero_mem(&param, sizeof(param));
    param.Parameter = WOW_ENABLE;
    param.Value = FALSE;

    CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* WOW enable */
    CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
    CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&param;

    Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

    if (Ret != NDIS_STATUS_SUCCESS)
    {
        printk("\x1b[31m%s: send WOW config command failed!!\x1b[m\n", __FUNCTION__);
        return;
    }

    RtmpOsMsDelay(1);


    /* traffic to Host */
    os_zero_mem(&param, sizeof(param));
    param.Parameter = WOW_TRAFFIC;
    param.Value = WOW_PKT_TO_HOST;

    CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE;
    CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
    CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&param;

    Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

    if (Ret != NDIS_STATUS_SUCCESS)
    {
        printk("\x1b[31m%s: send WOW config command failed!!\x1b[m\n", __FUNCTION__);
        return;
    }

    RtmpOsMsDelay(1);


    /* Restore MAC TX/RX */
    RtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

    RTUSBBulkReceive(pAd);
    RTUSBBulkCmdRspEventReceive(pAd);

    /* restore hardware remote wakeup flag */
    RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &Value);
    printk("\x1b[31m%s: 0x80 %08x\x1b[m\n", __FUNCTION__, Value);
    Value &= ~0x80;
    RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, Value);

	if (pAd->WOW_Cfg.bInBand == FALSE)
	{
		INT32 Value;

		RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &Value);
		printk("\x1b[31m%s: 0x80 = %x\x1b[m\n", __FUNCTION__, Value);
		Value &= ~0x01010000; /* GPIO0(ouput) --> 0(data) */
		RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, Value);
	}
}

#endif /* NEW_WOW_SUPPORT */


VOID RtmpPrepareHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR OpMode,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN CHAR Index)
{
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR *pNullFrame, *tmac_info = (UCHAR *)&pAd->NullTxWI;
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	ULONG Length;
	UCHAR *ptr;
	UINT i;
	UINT32 longValue;
	MAC_TX_INFO mac_info;

	// TODO: shiang-7603!! fix me
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MT7603 Not support yet!\n", __FUNCTION__));
		return;
	}

	NState = MlmeAllocateMemory(pAd, (PUCHAR *)&pNullFrame);

	os_zero_mem(pNullFrame, 48);
	os_zero_mem(tmac_info, TXWISize);

	if (NState == NDIS_STATUS_SUCCESS)
	{
		pNullFr = (PHEADER_802_11) pNullFrame;
		Length = sizeof(HEADER_802_11);

		pNullFr->FC.Type = FC_TYPE_DATA;
		pNullFr->FC.SubType = SUBTYPE_DATA_NULL;
		if (Index == 1)
			pNullFr->FC.ToDs = 0;
		else
		pNullFr->FC.ToDs = 1;
		pNullFr->FC.FrDs = 0;

		COPY_MAC_ADDR(pNullFr->Addr1, pEntry->Addr);
		{
			COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pNullFr->Addr3, pAd->CommonCfg.Bssid);
		}

		pNullFr->FC.PwrMgmt = PwrMgmt;

		pNullFr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, pAd->CommonCfg.TxRate, 14);

		/* sequence is increased in MlmeHardTx */
		pNullFr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; /* next sequence  */

		if (bQosNull)
		{
			UCHAR *qos_p = ((UCHAR *)pNullFr) + Length;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			Length += 2;
		}

	os_zero_mem((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.FRAG = FALSE;

	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = FALSE;
	mac_info.AMPDU = FALSE;

	mac_info.BM = IS_BM_MAC_ADDR(pNullFr->Addr1);
	mac_info.Ack = TRUE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;

	mac_info.WCID = pEntry->Aid;
	mac_info.Length = Length;

	mac_info.TID = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	mac_info.SpeEn = 1;

		if (Index == 1)
		{
			HTTRANSMIT_SETTING Transmit;

			Transmit.word = pEntry->MaxHTPhyMode.word;
			Transmit.field.BW = 0;
			if (Transmit.field.MCS > 7)
				Transmit.field.MCS = 7;

			mac_info.PID = (UCHAR)Transmit.field.MCS;
			mac_info.TxRate = (UCHAR)Transmit.field.MCS;
			write_tmac_info(pAd, tmac_info, &mac_info, &Transmit);
		}
		else
		{
			struct dev_rate_info *rate = &pEntry->wdev.rate;
			mac_info.PID = (UCHAR)rate->MlmeTransmit.field.MCS;
			mac_info.TxRate = (UCHAR)rate->MlmeTransmit.field.MCS;
			write_tmac_info(pAd, tmac_info, &mac_info,
							&rate->MlmeTransmit);
		}

		if (bWaitACK) {
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP) {
				TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;

				pTxWI->TXWI_O.TXRPT = 1;
			}
#endif /* RTMP_MAC */
#ifdef RLT_MAC
			// TODO: shiang, how about RT65xx series??
#endif /* RLT_MAC */
		}

		ptr = (PUCHAR)&pAd->NullTxWI;
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */
		for (i=0; i < TXWISize; i+=4)
		{
			longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
			if (Index == 0)
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[0] + i, longValue);
			else if (Index == 1)
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[1] + i, longValue);

			ptr += 4;
		}

		ptr = pNullFrame;
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif /* RT_BIG_ENDIAN */
		for (i= 0; i< Length; i+=4)
		{
			longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
			if (Index == 0) //for ra0
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[0] + TXWISize+ i, longValue);
			else if (Index == 1) //for p2p0
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[1] + TXWISize+ i, longValue);

			ptr += 4;
		}
	}

	if (pNullFrame)
		MlmeFreeMemory( pNullFrame);
}

#ifdef CONFIG_AP_SUPPORT
VOID RtAsicSetWdevIfAddr(RTMP_ADAPTER *pAd, VOID *wdev_void, INT opmode)
{
    INT idx = wdev->func_idx;
    UCHAR MacMask = 0;

    struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;

    if (opmode == OPMODE_AP)
    {
        COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
        if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
        {
            if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
            {
                if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
                    MacMask = 0xFE;
                else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
                    MacMask = 0xFC;
                else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
                    MacMask = 0xF8;
                else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 16)
                    MacMask = 0xF0;

                if (idx > 0)
                {
                    wdev->if_addr[0] |= 0x2;
                    if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
                    {
                        /*
                            Refer to HW definition -
                            Bit1 of MAC address Byte0 is local administration bit
                            and should be set to 1 in extended multiple BSSIDs'
                            Bit3~ of MAC address Byte0 is extended multiple BSSID index.
                         */
#ifdef ENHANCE_NEW_MBSSID_MODE
                        wdev->if_addr[0] &= ((MacMask << 2) + 3);
#endif /* ENHANCE_NEW_MBSSID_MODE */
                        wdev->if_addr[0] += ((wdev->func_idx - 1) << 2);
                    }
#ifdef ENHANCE_NEW_MBSSID_MODE
                    else
                    {
                        wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
                        wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += (idx - 1);
                    }
#endif /* ENHANCE_NEW_MBSSID_MODE */
                }
            }
            else
                wdev->if_addr[5] += idx;
        }
    }
    return;
}
#endif /* CONFIG_AP_SUPPORT */

INT RtAsicEnableBeacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR NumOfBcns)
{
    AsicSetMbssMode(pAd, NumOfBcns);

    return TRUE;
}

INT RtAsicDisableBeacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
    UCHAR NumOfBcns;
    NumOfBcns = clearHwBcnTxMem(pAd);

    return NumOfBcns;
}
