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
	rt65xx.c

	Abstract:
	Specific funcitons and configurations for RT65xx

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT65xx

#include	"rt_config.h"


/*
	========================================================================
	
	Routine Description:
		Enable Wlan function. this action will enable wlan clock so that chip can accept command. So MUST put in the 
		very beginning of Initialization. And put in the very LAST in the Halt function.

	Arguments:
		pAd		Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL
	
	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID ral_wlan_chip_onoff(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN bOn,
	IN BOOLEAN bResetWLAN)
{
	// TODO: check the functionality of the WLAN_FUN_CTRL here, now we just bring up it bu no fine tune.
	WLAN_FUN_CTRL_STRUC WlanFunCtrl = {.word=0};

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->WlanEnLock);
#endif


	RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl.word);
	DBGPRINT(RT_DEBUG_OFF, ("==>%s(): OnOff:%d, pAd->WlanFunCtrl:0x%x, Reg-WlanFunCtrl=0x%x\n",
				__FUNCTION__, bOn, pAd->WlanFunCtrl.word, WlanFunCtrl.word));

	if (bResetWLAN == TRUE)
	{
		WlanFunCtrl.field.GPIO0_OUT_OE_N = 0xFF;
		WlanFunCtrl.field.FRC_WL_ANT_SET = 0;
	}

	if (bOn == TRUE)
	{
		WlanFunCtrl.field.WLAN_CLK_EN = 1;
		WlanFunCtrl.field.WLAN_EN = 1;
	}
	else
	{
		WlanFunCtrl.field.PCIE_APP0_CLK_REQ = 0;
		WlanFunCtrl.field.WLAN_EN = 0;
		WlanFunCtrl.field.WLAN_CLK_EN = 0;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("WlanFunCtrl.word = 0x%x\n", WlanFunCtrl.word));
	RTMP_IO_FORCE_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
	pAd->WlanFunCtrl.word = WlanFunCtrl.word;
	RTMPusecDelay(2);
	
	RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl.word);
	DBGPRINT(RT_DEBUG_ERROR,
		("<== %s():  pAd->WlanFunCtrl.word = 0x%x, Reg->WlanFunCtrl=0x%x!\n",
		__FUNCTION__, pAd->WlanFunCtrl.word, WlanFunCtrl.word));
	
#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->WlanEnLock);
#endif
	
}

VOID dump_bw_info(RTMP_ADAPTER *pAd)
{
#ifdef DBG
		UINT32 core_r1, agc_r0, be_r0, band_cfg;
		static UCHAR *bw_str[]={"20", "10", "40", "80"};
		UCHAR bw, prim_ch_idx, decode_cap;
		static UCHAR *decode_str[] = {"0", "20", "40", "20/40",
									"80", "20/80", "40/80", "20/40/80"};
		UCHAR tx_prim;
		RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


		RTMP_BBP_IO_READ32(pAd, CORE_R1, &core_r1);
		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc_r0);
		RTMP_BBP_IO_READ32(pAd, TXBE_R0, &be_r0);
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
		
		/*  Tx/RX : control channel setting */
		DBGPRINT(RT_DEBUG_TRACE, ("%s():RegisterSetting: TX_BAND_CFG=0x%x, CORE_R1=0x%x, AGC1_R0=0x%x, TXBE_R0=0x%x\n",
				__FUNCTION__, band_cfg, core_r1, agc_r0, be_r0));
		bw = ((core_r1 & 0x18) >> 3) & 0xff;
		DBGPRINT(RT_DEBUG_TRACE, ("[CORE_R1]\n"));
		DBGPRINT(RT_DEBUG_TRACE, ("\tTx/Rx BandwidthCtrl(CORE_R1[4:3])=%d(%s MHz)\n", 
					bw, bw_str[bw]));

		DBGPRINT(RT_DEBUG_TRACE, ("[AGC_R0]\n"));
		prim_ch_idx = ((agc_r0 & 0x300) >> 8) & 0xff;
		DBGPRINT(RT_DEBUG_TRACE, ("\tPrimary Channel Idx(AGC_R0[9:8])=%d\n", prim_ch_idx));
		decode_cap = ((agc_r0 & 0x7000) >> 12);
		DBGPRINT(RT_DEBUG_TRACE, ("\tDecodeBWCap(AGC_R0[14:12])=%d(%s MHz Data)\n",
					decode_cap, decode_str[decode_cap]));

		DBGPRINT(RT_DEBUG_TRACE, ("[TXBE_R0]\n"));
		tx_prim = (be_r0 & 0x3);
		DBGPRINT(RT_DEBUG_TRACE, ("\tTxPrimary(TXBE_R0[1:0])=%d\n", tx_prim));
		DBGPRINT(RT_DEBUG_TRACE, ("\n%s\n", pChipCap->MACRegisterVer));
		DBGPRINT(RT_DEBUG_TRACE, ("%s\n", pChipCap->BBPRegisterVer));
		DBGPRINT(RT_DEBUG_TRACE, ("%s\n\n", pChipCap->RFRegisterVer));
#endif
}


#endif /* RT65xx */

