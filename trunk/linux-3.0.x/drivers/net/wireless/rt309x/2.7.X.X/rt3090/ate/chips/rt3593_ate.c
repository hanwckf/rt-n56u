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
	rt3593_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT305x(i.e., RT3050/RT3051/RT3052)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3593

#include "rt_config.h"

extern FREQUENCY_ITEM FreqItems3053[];
extern UCHAR NUM_OF_3053_CHNL;


/*
========================================================================
	
	Routine Description: 3593 R66 writing must select BBP_R27

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
NTSTATUS ATE_RT3593WriteBBPR66(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Value)
{
	NTSTATUS NStatus = STATUS_UNSUCCESSFUL;
	UCHAR	bbpData = 0;

	if (!IS_RT3593(pAd))
	{
		DBGPRINT_ERR(("%s: Incorrect MAC version, pAd->MACVersion = 0x%X\n", 
			__FUNCTION__, 
			pAd->MACVersion));

		return NStatus;
	}

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &bbpData);

	/* R66 controls the gain of Rx0 */
	bbpData &= ~(0x60);	/* clear bit 5,6 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpData);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Value);

	/* R66 controls the gain of Rx1 */
	bbpData |= 0x20; /* set bit 5 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpData);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Value);

	if (IS_RT3593(pAd)) /* Rx chain 2 */
	{
		/* R66 controls the gain of Rx chain 2 */
		/* clear bit 5 and 6 (index to Rx chain for RX I/Q imbalance) */
		bbpData &= ~(0x60); 
		bbpData |= 0x40; /* Rx chain 2 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpData);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Value);
	}

	NStatus = STATUS_SUCCESS;

	return NStatus;
} 


VOID RT3593ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 Value = 0;
/*	RTMP_RF_REGS *RFRegTable = NULL; */
	UCHAR index = 0, BbpValue = 0, R66 = 0x30, Channel = 0;
	CHAR TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER;
	UCHAR RefFreqOffset;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue = 0;
	/*UCHAR PreRFValue = 0;*/
#endif /* RTMP_RF_RW_SUPPORT */
#ifdef DOT11N_SS3_SUPPORT
	CHAR TxPwer3 = DEFAULT_RF_TX_POWER;
#endif /* DOT11N_SS3_SUPPORT */
	UCHAR Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0, Tx2FinePowerCtrl = 0;
	BBP_R109_STRUC BbpR109 = { { 0 } };
	BBP_R110_STRUC BbpR110 = { { 0 } };
	INTERNAL_1_STRUCT TxAttenuationCtrl;
	
#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;
#ifdef DOT11N_SS3_SUPPORT
	TxPwer3 = pAd->ate.TxPower2;
#endif /* DOT11N_SS3_SUPPORT */

	for (index = 0; index < NUM_OF_3053_CHNL; index++)
	{
		if (pAd->RfIcType != RFIC_3053)
		{
			DBGPRINT_ERR(("%s: Incorrect RF IC type, pAd->RfIcType = 0x%X", 
				__FUNCTION__, pAd->RfIcType));
			
			break;
		}

		if (Channel == FreqItems3053[index].Channel)
		{
			/* Set the BBP Tx fine power control in 0.1dB step */
			BbpR109.field.Tx0PowerCtrl = Tx0FinePowerCtrl;

			if (pAd->Antenna.field.TxPath >= 2)
			{
				BbpR109.field.Tx1PowerCtrl = Tx1FinePowerCtrl;
			}
			else
			{
				BbpR109.field.Tx1PowerCtrl = 0;
			}

			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R109, BbpR109.byte);

			DBGPRINT(RT_DEBUG_TRACE, ("%s: Channel = %d, BBP_R109 = 0x%X\n",
				__FUNCTION__,
				Channel,
				BbpR109.byte));

			if (pAd->Antenna.field.TxPath == 3)
			{
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R110, &BbpR110.byte);
				/* Tx2 power control in 0.1dB step */
				BbpR110.field.Tx2PowerCtrl = Tx2FinePowerCtrl; 
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R110, BbpR110.byte);
			}

			/* for 2.4G, restore BBP25, BBP26 */
			if (Channel <= 14)
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, pAd->Bbp25);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, pAd->Bbp26);
			}
			/* hard code for 5GHz, Gary 2008-12-10 ??? */
			/* set BBP_R25 and BBP_R26 as 0x00 to improve EVM */
			else
			{
				/* IQ Phase Correction */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x00);
				/* IQ Phase correction value */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, 0x00);
			}

			/* Programming channel parameters */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, FreqItems3053[index].N); /* N */
			/* K, N<11:8> is set to zero */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, (FreqItems3053[index].K & 0x0F)); 

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R11, (PUCHAR)&RFValue);
			RFValue = ((RFValue & ~0x03) | (FreqItems3053[index].R & 0x03)); /* R */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R11, (PUCHAR)&RFValue);

			if (Channel <= 14)
			{
				/* 
					pll_idoh (charge pump current control, 1: x2)
					and pll_mod (choose fractional divide, 01: mod10)
				*/
				RFValue = ((RFValue & ~0x4C) | 0x44); 
			}
			else
			{
				/* 
					pll_idoh (charge pump current control, 1: x2)
					and pll_mod (choose fractional divide, 10: mod2)
				*/
				RFValue = ((RFValue & ~0x4C) | 0x48); 
			}

			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);

			if (Channel <= 14)
			{
				RFValue = 0x00; /* 000xxxxx */
				/* tx0_alc */
				RFValue = (RFValue | (TxPwer & 0x1F)); 
			}
			else
			{
				RFValue = 0x48; /* 01xx1xxx */
				/* tx0_alc */
				RFValue = (RFValue | ((TxPwer & 0x18) << 1) | (TxPwer & 0x07)); 
#ifdef RTMP_MAC_PCI
				if (IS_PCI_INF(pAd))
				{
					/* disable bit 3 from CH36 to CH140 (low ALC CHs) */
				 	if ((Channel >= 36) && (Channel <= 140))
				 	{
						RFValue &= ~0x8;
				 	}
				}
#endif /* RTMP_MAC_PCI */
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, (UCHAR)RFValue);

			if (Channel <= 14)
			{
				RFValue = 0x00; /* 000xxxxx */
				/* tx1_alc */
				RFValue = (RFValue | (TxPwer2 & 0x1F)); 
			}
			else
			{
				RFValue = 0x48; /* 01xx1xxx */
				/* tx1_alc */
				RFValue = (RFValue | ((TxPwer2 & 0x18) << 1) | (TxPwer2 & 0x07)); 
#ifdef RTMP_MAC_PCI
				if (IS_PCI_INF(pAd))
				{
					/* disable bit 3 from CH36 to CH140 (low ALC CHs) */
				 	if ((Channel >= 36) && (Channel <= 140))
				 	{
						RFValue &= ~0x8;
				 	}
				}
#endif /* RTMP_MAC_PCI */
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, (UCHAR)RFValue);

			if (Channel <= 14)
			{
				RFValue = 0x00; /* 000xxxxx */
				/* tx2_alc */
				RFValue = (RFValue | (TxPwer3 & 0x1F)); 
			}
			else
			{
				RFValue = 0x48; /* 01xx1xxx */
				/* tx2_alc */
				RFValue = (RFValue | ((TxPwer3 & 0x18) << 1) | (TxPwer3 & 0x07)); 
#ifdef RTMP_MAC_PCI
				if (IS_PCI_INF(pAd))
				{
					/* disable bit 3 from CH36 to CH140 (low ALC CHs) */
				 	if ((Channel >= 36) && (Channel <= 140))
				 	{
						RFValue &= ~0x8;
				 	}
				}
#endif /* RTMP_MAC_PCI */
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, (UCHAR)RFValue);

			ATEAsicSetTxRxPath(pAd);

			ATE_CHIP_RX_VGA_GAIN_INIT(pAd);
			
			RefFreqOffset = pAd->ate.RFFreqOffset;
			RTMPAdjustFrequencyOffset(pAd, &RefFreqOffset);

			/* tx_agc_fc (capacitor control in Tx baseband filter) */
			RFValue = 0x78; 
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, (UCHAR)RFValue);

			if (Channel <= 14) /* 2.4GHz */
			{
				/* rx_agc_fc (capacitor control in Rx baseband filter) */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0xA0);
			}
			else /* 5GHz */
			{
				/* rx_agc_fc (capacitor control in Rx baseband filter) */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);

			if (pAd->ate.TxWI.BW == BW_40) /* BW 40 */
			{
				/* 40MBW Bit[2:1]=1,1 */
				RFValue = ((RFValue & ~0x06) | (0x01 << 1) | (0x01 << 2)); 
			}
			else /* BW 20 */
			{
				/* 20MBW Bit[2:1]=0,0 */
				RFValue = ((RFValue & ~0x06) | (0x00 << 1) | (0x00 << 2)); 
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R36, (PUCHAR)&RFValue);

			if (Channel <= 14)
			{
				/* RF_BS (RF band select, 1: g-band operation) */
				RFValue = ((RFValue & ~0x80) | 0x80); 
			}
			else
			{
				/* RF_BS (RF band select, 0: a-band operation) */
				RFValue = ((RFValue & ~0x80) | 0x00); 
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R36, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R34, (PUCHAR)&RFValue);

			if (Channel <= 14)
			{
				/* 
					vcolo_bs (VCO buffer ch1, ch2 and ch0 band tuning, 111: g-band) 
					and vcolobuf_ien (VCO buffer current control, 1: high current)
				*/
				RFValue = 0x3C; 
			}
			else
			{
				/* 
					vcolo_bs (VCO buffer ch1, ch2 and ch0 band tuning, 000: a-band) 
					and vcolobuf_ien (VCO buffer current control, 1: high current)
				*/
				RFValue = 0x20; 
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R34, (UCHAR)RFValue);

			//2 TODO: Where is the tssi_bs?
/*
			RT30xxReadRFRegister(pAd, RF_R34, (PUCHAR)&Value);
			if  (Channel <= 14)
				Value = 0xC3;
			else
				Value = 0xC0;
			RT30xxWriteRFRegister(pAd, RF_R09, (UCHAR)Value);
*/

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);\

			if (Channel <= 14)
			{
				/* pfd_delay (10: 0.9 ns), pll_r2 (011: 4K) and pll_c1 (010: 17 pF) */
				RFValue = 0x1A; 
			}
			else
			{
				/* pfd_delay (10: 0.9 ns), pll_r2 (100: 5.5 K) and pll_c1 (01: 17 pF) */
				RFValue = 0x12; 
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

			/* 2010/09/20 */
			if (Channel <= 128)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				/* vco_ic (VCO bias current control, 01: low) */
				RFValue = ((RFValue & ~0xC0) | 0x80); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);
			}
			else /* High channel in 5GHz */
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				/* vco_ic (VCO bias current control, 01: low) */
				RFValue = ((RFValue & ~0xC0) | 0x40); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
			/* rxvcm (Rx BB filter VCM) */
			RFValue = ((RFValue & ~0x18) | 0x10); 
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

			if (Channel <= 14)
			{
				/* pll_comp_ic, pll_pre_ic */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R10, 0xD3);	
				/* pll_R, pll_mod, pll_idoh */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, 0x12);	
			}
			else
			{
				/* pll_comp_ic, pll_pre_ic */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R10, 0xD8);	
				/* pll_R, pll_mod, pll_idoh */	
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, 0x23);	
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R51, (PUCHAR)&RFValue);
			/* transmit IF mixer current control (both bands) */
			RFValue = ((RFValue & ~0x03) | 0x01); 
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R51, (UCHAR)RFValue);

			if (Channel <= 14)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R51, (PUCHAR)&RFValue);
				/* tx_mx1_cc (RF mixer output tank tuning (both bands), 101: g-band) */
				RFValue = ((RFValue & ~0x1C) | (0x05 << 2)); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R51, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R51, (PUCHAR)&RFValue);
				/* tx_mx1_ic (transmit RF mixer current control (both bands), 011: g-band) */
				RFValue = ((RFValue & ~0xE0) | 0x60); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R51, (UCHAR)RFValue);
			}
			else
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R51, (PUCHAR)&RFValue);
				/* tx_mx1_cc (RF mixer output tank tuning (both bands), 100: a-band) */
				RFValue = ((RFValue & ~0x1C) | (0x04 << 2)); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R51, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R51, (PUCHAR)&RFValue);
				/* tx_mx1_ic (transmit RF mixer current control (both bands), 010: a-band) */
				RFValue = ((RFValue & ~0xE0) | 0x40); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R51, (UCHAR)RFValue);
			}

			if (Channel <= 14)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
				/* tx_lo1_ic (transmit LO1 current control, 011: g-band) */
				RFValue = ((RFValue & ~0x1C) | 0x0C); 

				/* 3593 TXBF TODO */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R50, (PUCHAR)&RFValue);
				/* tx_lo1_en (0:LO1 follows TR switch) */
				RFValue = ((RFValue & ~0x20) | 0x00); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R50, (UCHAR)RFValue);
			}
			else
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
				/* tx_lo1_ic (transmit LO1 current control, 010: g-band) */
				RFValue = ((RFValue & ~0x1C) | 0x08); 

				/* 3593 TXBF TODO */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);

#ifdef TXBF_SUPPORT
			if (pAd->ate.bTxBF == TRUE)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
				RFValue |= 0x20;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);
			}
#endif /* TXBF_SUPPORT */

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R50, (PUCHAR)&RFValue);
				/* tx_lo1_en (0:LO1 follows TR switch) */
				RFValue = ((RFValue & ~0x20) | 0x00); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R50, (UCHAR)RFValue);
			}

			if (Channel <= 14)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R57, (PUCHAR)&RFValue);
				/* drv_cc (balun capacitor calbration, 011011: g-band) */
				RFValue = ((RFValue & ~0xFC) | 0x6C); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R57, (UCHAR)RFValue);
			}
			else
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R57, (PUCHAR)&RFValue);
				/* drv_cc (balun capacitor calbration, 001111: a-band) */
				RFValue = ((RFValue & ~0xFC) | 0x3C); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R57, (UCHAR)RFValue);
			}

			if (Channel <= 14)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R44, (PUCHAR)&RFValue);
				/* rx_mix1_ic, rxa_lnactr, lna_vc, lna_inbias_en and lna_en */
				RFValue = 0x93; 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R44, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R52, (PUCHAR)&RFValue);
				/* drv_gnd_a, tx_vga_cc_a and tx_mx2_gain */
				RFValue = 0x45; 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R52, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);

				/* 
					vcocal_en (initiate VCO calibration
					(reset after completion))
					- It should be at the end of RF configuration. 
				*/
				RFValue = ((RFValue & ~0x80) | 0x80); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);					
			}
			else
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R44, (PUCHAR)&RFValue);
				/* rx_mix1_ic, rxa_lnactr, lna_vc, lna_inbias_en and lna_en */
				RFValue = 0x9B; 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R44, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R52, (PUCHAR)&RFValue);
				/* drv_gnd_a, tx_vga_cc_a and tx_mx2_gain */
				RFValue = 0x05; 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R52, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);

				/* 
					vcocal_en (initiate VCO calibration 
					(reset after completion))
					- It should be at the end of RF configuration.
				*/
				RFValue = ((RFValue & ~0x80) | 0xBE); 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);
			}

			if ((Channel >= 1) && (Channel <= 14)) /* 2.4GHz */
			{
				RFValue = 0x23;

				/* 3593 TXBF TODO */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);

				RFValue = 0xBB;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R45, (UCHAR)RFValue);
			}
			else if ((Channel >= 36) && (Channel <= 64)) /* Low channels in 5GHz */
			{
				RFValue = 0x36;

				/* 3593 TXBF TODO */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);

				RFValue = 0xEB;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R45, (UCHAR)RFValue);
			}
			else if ((Channel >= 100) && (Channel <= 128)) /* Middle channels in 5GHz */
			{
				RFValue = 0x32;

				/* 3593 TXBF TODO */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);

				RFValue = 0xB3;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R45, (UCHAR)RFValue);
			}
			else /* High channel in 5GHz */
			{
				RFValue = 0x30;

				/* 3593 TXBF TODO */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);

				RFValue = 0x9B;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R45, (UCHAR)RFValue);
			}

#ifdef TXBF_SUPPORT
			if (pAd->ate.bTxBF == TRUE)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R39, (PUCHAR)&RFValue);
				RFValue |= 0x40;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);
			}
#endif /* TXBF_SUPPORT */

			pAd->LatchRfRegs.Channel = Channel; /* Channel latch */

#ifdef TXBF_SUPPORT
			/* Do a Divider Calibration and update BBP registers */
			if (pAd->ate.bTxBF)
			{
				ITxBFLoadLNAComp(pAd);
				ITxBFDividerCalibration(pAd, 2, 0, NULL);
			}
#endif /* TXBF_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("%s: RT3053: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
				__FUNCTION__, 
				Channel, 
				pAd->RfIcType, 
				TxPwer, 
				TxPwer2, 
				pAd->Antenna.field.TxPath, 
				FreqItems3053[index].N, 
				FreqItems3053[index].K, 
				FreqItems3053[index].R));

			break;
		}
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x30050F02;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
/*		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0); */	

		/* Rx High power VGA offset for LNA select */
		if (IS_RT3593(pAd))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R77, 0x98);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62); 
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x8A);
			if (pAd->NicConfig2.field.ExternalLNAForG)
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
			}
			else
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
			}
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R92, 0x02);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R104, 0x92);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x25);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R120, 0x50);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R137, 0x0F);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R148, 0xC8);
		}
		else
		{
			/* For 1T/2R chip only... */
			if (pAd->NicConfig2.field.ExternalLNAForG)
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
			}
			else
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
			}
		}

		/* 5G band selection PIN, bit1 and bit2 are complement */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Disable unused PA_PE */
		if (pAd->ate.TxAntennaSel == 1)
		{
			/* TX 0 */
			TxPinCfg = 0x30050F02; 
		}
		else if (pAd->ate.TxAntennaSel == 2)
		{
			/* TX 1 */
			TxPinCfg = 0x30050F08; 
		}
		else if (pAd->ate.TxAntennaSel == 3)
		{
			/* TX 2 */
			TxPinCfg = 0x32050F00; 
		}
		else
		{
			/* All TX */
			TxPinCfg = 0x32050F0A; 
		}

		/* Disable unused LNA_PE */
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg = TxPinCfg & ~0x30000C00; 
		}
		else if (pAd->Antenna.field.RxPath == 2)
		{
			TxPinCfg = TxPinCfg & ~0x30000000;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		/* PCIe PHY Transmit attenuation adjustment */
		RTMP_IO_READ32(pAd, PCIE_PHY_TX_ATTENUATION_CTRL, &TxAttenuationCtrl.word);

		if (Channel == 14) /* Channel #14 */
		{
			/* Enable PCIe PHY Tx attenuation */
			TxAttenuationCtrl.field.PCIE_PHY_TX_ATTEN_EN = 1; 
			/* 9/16 full drive level */
			TxAttenuationCtrl.field.PCIE_PHY_TX_ATTEN_VALUE = 4; 
		}
		else /* Channel #1 ~ #13 */
		{
			/* Disable PCIe PHY Tx attenuation */
			TxAttenuationCtrl.field.PCIE_PHY_TX_ATTEN_EN = 0; 
			/* n/a */
			TxAttenuationCtrl.field.PCIE_PHY_TX_ATTEN_VALUE = 0; 
		}

		RTMP_IO_WRITE32(pAd, PCIE_PHY_TX_ATTENUATION_CTRL, TxAttenuationCtrl.word);
	}
	/* channel > 14 */
	else
	{
		UINT32	TxPinCfg = 0x30050F01;
		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R77, 0x98);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82); 
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x9A);

		/* Rx High power VGA offset for LNA select */
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R91, &BbpValue);
		ASSERT((BbpValue == 0x04));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R92, 0x02);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R104, 0x92);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x25);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R120, 0x50);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R137, 0x0F);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R148, 0xC8);
		
		/* 5 G band selection PIN, bit1 and bit2 are complement */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Disable unused PA_PE */
		if (pAd->ate.TxAntennaSel == 1)
		{
			/* TX 0 */
			TxPinCfg = 0x30050F01; 
		}
		else if (pAd->ate.TxAntennaSel == 2)
		{
			/* TX 1 */
			TxPinCfg = 0x30050F04; 
		}
		else if (pAd->ate.TxAntennaSel == 3)
		{
			/* TX 2 */
			TxPinCfg = 0x31050F00; 
		}
		else
		{
			/* All TX */
			TxPinCfg = 0x31050F05; 
		}

		/* Disable unused LNA_PE */
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg = TxPinCfg & ~0x30000C00; 
		}
		else if (pAd->Antenna.field.RxPath == 2)
		{
			TxPinCfg = TxPinCfg & ~0x30000000;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}

	/* GPIO control */
	if (IS_RT3593(pAd))
	{
		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);

#ifdef RTMP_MAC_PCI
		if (RT3593OverPCI(pAd)) /* PCI */
		{
			/* Band selection (one GPIO pin controls three paths): GPIO #7 (output) */
			if (Channel <= 14)
			{
				 Value = ((Value & ~0x00008080) | 0x00000080);
			}
			else
			{
				Value = ((Value & ~0x00008080) | 0x00000000);
			}

			/* LNA PE control (one GPIO pin controls three LNA PEs): GPIO #4 (output) */
			Value = ((Value & ~0x00001010) | 0x00000010);
		}
		else /* PCIe */
		{
			/* Band selection (one GPIO pin controls three paths): GPIO #8 (output) */
			if (Channel <= 14)
			{
				Value = ((Value & ~0x01010000) | 0x00010000);
			}
			else
			{
				Value = ((Value & ~0x01010000) | 0x00000000);
			}

			/* LNA PE control (one GPIO pin controls three LNA PEs): GPIO #4 (output) */
			Value = ((Value & ~0x00001010) | 0x00000010);
		}
#endif /* RTMP_MAC_PCI */


		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);

	}

	if (Channel > 14)
	{
		R66 = 0x20 + (GET_LNA_GAIN(pAd)*5)/3;
	}
	else
	{
		R66 = 0x2E + GET_LNA_GAIN(pAd);
	}				
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);		

	/*
		On 11A, We should delay and wait RF/BBP to be stable
		and the appropriate time should be 1000 micro seconds. 

		2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.
	*/
	RTMPusecDelay(1000);  
}


INT RT3593ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	CHAR TxPower = 0;
	UCHAR RFValue = 0, Channel = pAd->ate.Channel;
	
#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
#ifdef DOT11N_SS3_SUPPORT
	else if (index == 2)
	{
		TxPower = pAd->ate.TxPower2;
	}
#endif /* DOT11N_SS3_SUPPORT */
	else
	{
#ifdef DOT11N_SS3_SUPPORT
		DBGPRINT_ERR(("Only TxPower0, TxPower1, and TxPower2 are adjustable !\n"));
#else
		DBGPRINT_ERR(("Only TxPower0 and TxPower1 are adjustable !\n"));
#endif /* DOT11N_SS3_SUPPORT */
		DBGPRINT_ERR(("TxPower%d is out of range !\n", index));
	}

	if (Channel <= 14) /* G band */
	{
		RFValue = 0x00; /* 000xxxxx */
		RFValue = (RFValue | (TxPower & 0x1F)); 
	}
	else /* A band */ 
	{
		if ((Channel >= 36) && (Channel <= 140)) 
		{
			/* a(ch36~ch140): 01xx0xxx */
			RFValue = 0x40 | (((TxPower & 0x18) << 1) | (TxPower & 0x7));
#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd))
			{
				/* disable bit 3 from CH36 to CH140 (low ALC CHs) */
				RFValue &= ~0x8;
			}
#endif /* RTMP_MAC_PCI */
		}
		else if ((Channel >= 149) && (Channel <= 165)) 
		{
			/* a(ch149~ch165): 01xx1xxx */
			RFValue = 0x48 | (((TxPower & 0x18) << 1) | (TxPower & 0x7));
		}
	}

	if (index == 0)
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, RFValue);
	else if (index == 2)
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, RFValue);
	else if (index == 1)		
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, RFValue);

	return 0;
}


/*
==========================================================================
	Description:
		This routine sets initial value of VGA in the RX chain.
		AGC is the abbreviation of "Automatic Gain Controller",
		while VGA (Variable Gain Amplifier) is a part of AGC loop.
		(i.e., VGA + level detector + feedback loop = AGC)

    Return:
        VOID
==========================================================================
*/
VOID RT3593ATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band */
		R66 = (UCHAR)(0x20 + (LNAGain * 5)/3);
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}


VOID RT3593ATE_SET_TX_RATE_POWER(
	IN PRTMP_ADAPTER pAd, 
	IN UINT8 bandwidth,
	IN UCHAR channel)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;
	INT32 i, j;
	CHAR Value;
	unsigned long flags;

	CfgOfTxPwrCtrlOverMAC.NumOfEntries = MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS;
	
	if (bandwidth == BW_40)
	{
		if (channel > 14)
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9;
		}
		else
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9;
		}
	}
	else
	{
		if (channel > 14)
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9;
		}
		else
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9;
		}
	}


	for (i=0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

				{
					if (Value < 0)
					{
						Value = 0; /* min */
					}
					else if (Value  > 0xC)
					{
						Value = 0xC; /* max */
					}
					
				}
				/* fill new value to CSR offset */
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value << j*4);
			}
			RTMP_INT_LOCK(&pAd->irq_lock, flags); 
			RTMP_IO_WRITE32(pAd, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue);
			RTMP_INT_UNLOCK(&pAd->irq_lock, flags); 

		}
	}

}


INT	RT3593_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	/* Fix the error spectrum of CCK-40MHz. */
	/* Turn on BBP 20MHz mode by request here. */
	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT_ERR(("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\nBandwidth switch to 20\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
				}
				RTMPusecDelay(5000);				
			}
		}
		else
		{
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				}
 				RTMPusecDelay(5000);				
 			}
		}
 
		RT3593ATE_SET_TX_RATE_POWER(pAd, BW_20, pAd->ate.Channel);

		/* Set BW = 20 MHz */
		/* Set BBP R4 = 0x40 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, 0x40);
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x12 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x08 */
		value = 0x08;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x10 */
		value = 0x10;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
		/* enable DC filter */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R103, &value);
		value |= 0xC0;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, value);

		/* If Channel=14 and Bandwidth=20M and Mode=CCK, set BBP R4 bit5=1 */
		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

			if (TxMode == MODE_CCK)
			{
				/* When Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
				value |= 0x00000020; /* set bit5=1 */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);				
			}
		}   		
	}
	/* If bandwidth = 40M, set RF Reg4 bit 21 = 0. */
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
				}
				RTMPusecDelay(5000);				
			}
		}
		else
		{
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				{
					 RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				}
				RTMPusecDelay(5000);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
			{
    			value = 0x28;
    			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
		}

		RT3593ATE_SET_TX_RATE_POWER(pAd, BW_40, pAd->ate.Channel);

		/* Set BW = 40 MHz */
		/* Set BBP R4 = 0x50 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, 0x50);
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x12 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x0A */
		value = 0x0A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x10 */
		value = 0x10;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		/* disable DC filter before E version chip, other enable DC filter */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R103, &value);
		if ((pAd->MACVersion & 0xffff) < 0x0211)
			value &= ~(0xC0);
		else
			value |= 0xC0;
		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, value);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


INT	RT3593_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR RFFreqOffset = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	/* RT35xx ATE will reuse this code segment. */
	if (RFFreqOffset >= 96)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_FREQOFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pAd->ate.RFFreqOffset = RFFreqOffset;

	ATEAsicSwitchChannel(pAd);

	return TRUE;
}


VOID RT3593ATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd) 
{
	UCHAR Value;
	UCHAR BbpValue = 0;
	UCHAR ATEMode = pAd->ate.Mode;

	TX_CHAIN_ADDR0_H_STRUC TxChainAddr0H;	
	TX_CHAIN_ADDR0_L_STRUC TxChainAddr0L;

	RTMP_IO_READ32(pAd, TX_CHAIN_ADDR0_H, &TxChainAddr0H.word);

	/* set TX path, pAd->ate.TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1 */
	if ((ATEMode == ATE_TXFRAME) || (ATEMode == ATE_TXCONT) || (ATEMode == ATE_TXCARR)
		|| (ATEMode == ATE_TXCARRSUPP))
	{
		switch (pAd->Antenna.field.TxPath)
		{
			case 3:
				switch (pAd->ate.TxAntennaSel)
				{
					case ANT_ALL:		
					/* set MAC register */
					TxChainAddr0H.field.TxChainSel0 = 3;
#ifdef TXBF_SUPPORT
					/* enable rx0, rx1,and rx2 */
					Value = 0xFF;
#else
					Value = 0xAB;
#endif /* TXBF_SUPPORT */
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
					break;
					
					case ANT_0:
					/* set MAC register */
					TxChainAddr0H.field.TxChainSel0 = 0;
#ifdef TXBF_SUPPORT
					/* enable rx0, rx1,and rx2 */
					Value = 0x5F;
#else
					Value = 0x0B;
#endif /* TXBF_SUPPORT */
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
					break;

					case ANT_1:	
					/* set MAC register */
					TxChainAddr0H.field.TxChainSel0 = 1;
#ifdef TXBF_SUPPORT
					/* enable rx0, rx1,and rx2 */
					Value = 0x77;
#else
					Value = 0x23;
#endif /* TXBF_SUPPORT */
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
					break;

					case ANT_2:	
					/* set MAC register */
					TxChainAddr0H.field.TxChainSel0 = 2;
#ifdef TXBF_SUPPORT
					/* enable rx0, rx1,and rx2 */
					Value = 0xD7;
#else
					Value = 0x83;
#endif /* TXBF_SUPPORT */
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
					break;					

					default: /* TX0 */		
					/* set MAC register */
					TxChainAddr0H.field.TxChainSel0 = 0;
#ifdef TXBF_SUPPORT
					/* enable rx0, rx1,and rx2 */
					Value = 0x5F;
#else
					Value = 0x0B;
#endif /* TXBF_SUPPORT */
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
					break;

				}

				break;

			default:

			break;
		}

		/* Write higher bytes of DA */
		TxChainAddr0H.field.TxChainAddr0H_Byte5 = pAd->ate.Addr1[5];
		TxChainAddr0H.field.TxChainAddr0H_Byte4 = pAd->ate.Addr1[4];		
		RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_H, TxChainAddr0H.word);

		/* Write lower bytes of DA */
		TxChainAddr0L.field.TxChainAddr0L_Byte3 =pAd->ate.Addr1[3];
		TxChainAddr0L.field.TxChainAddr0L_Byte2 =pAd->ate.Addr1[2];
		TxChainAddr0L.field.TxChainAddr0L_Byte1 =pAd->ate.Addr1[1];
		TxChainAddr0L.field.TxChainAddr0L_Byte0 =pAd->ate.Addr1[0];
		RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_L, TxChainAddr0L.word);
	}
	else if(ATEMode == ATE_RXFRAME)
	{
		/* Set RX path, pAd->RxAntennaSel : 0 -> All, 1 -> RX0, 2 -> RX1, 3 -> RX2 */
		switch (pAd->Antenna.field.RxPath)
		{
			case 3:
				switch (pAd->ate.RxAntennaSel)
				{
					case 1:
						/* set BBP R3, bit 4:3:1:0 = 0000 */							
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x00;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
						/* set RF R1, bit 6:4:2 = 001 */
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &Value);
						Value &= ~RXPowerEnMask;
						Value |=  0x04;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
					break;
					case 2:
						/* set BBP R3, bit 4:3:1:0 = 0001 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x01;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
						/* set RF R1, bit 6:4:2 = 010 */
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &Value);
						Value &= ~RXPowerEnMask;
						Value |= 0x10;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
						break;
					case 3:	
						/* set BBP R3, bit 4:3:1:0 = 0002 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x02;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
						/* set RF R1, bit 6:4:2 = 100 */
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &Value);
						Value &= ~RXPowerEnMask;
						Value |= 0x40;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
						break;								

					default:	
						/* set BBP R3, bit 4:3:1:0 = 1000 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);							
						/* set RF R1, bit 6:4:2 = 111 */
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &Value);
						Value |= RXPowerEnMask;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, Value);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x46);
						break;
				}
				break;
			default:				
				break;		
		}
	}		
}

struct _ATE_CHIP_STRUCT RALINK3593 =
{
	/* functions */
	.ChannelSwitch = RT3593ATEAsicSwitchChannel,
	.TxPwrHandler = RT3593ATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT3593ATERxVGAInit,
	.AsicSetTxRxPath = RT3593ATEAsicSetTxRxPath,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT3593_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT3593_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 9,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,
	.bBBPStoreTXCONT = TRUE,
	.bBBPLoadATESTOP = TRUE,
};
#endif /* RT3593 */

