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
	rt2883.c

	Abstract:
	Specific funcitons and variables for RT2883

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"


#ifdef RT2883
REG_PAIR RT2883_BBPRegTable[] =
{
	{BBP_R4,	0x50}, /* 2883 need to */
	{BBP_R65,	0x6C},		/* fix rssi issue and add Fine AGC */
	{BBP_R103,	0xC0},
	{BBP_R105,	0x04 /*0xbc ?*/},	/* RT2883 default is 0x4. Initialized R105 to enable saving of Explicit and Implicit profiles */
	{BBP_R137,	0x0F},  /* julian suggest make the RF output more stable */
	{BBP_R179,	0x02},	/* Set ITxBF timeout to 0x9C40=1000msec*/
	{BBP_R180,	0x01},
	{BBP_R182,	0x9C},
};


UCHAR RT2883_EeBuffer[EEPROM_SIZE] = {
	0x83, 0x28, 0x01, 0x00, 0x00, 0x0c, 0x43, 0x28, 0x83, 0x00, 0x83, 0x28, 0x14, 0x18, 0xff, 0xff,
	0xff, 0xff, 0x83, 0x28, 0x14, 0x18, 0x00, 0x00, 0x01, 0x00, 0x6a, 0xff, 0x00, 0x02, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 0x43, 0x28, 0x83, 0x01, 0x00, 0x0c,
	0x43, 0x28, 0x83, 0x02, 0x33, 0x0a, 0xec, 0x00, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x20, 0x01, 0x55, 0x77, 0xa8, 0xaa, 0x8c, 0x88, 0xff, 0xff, 0x0a, 0x08, 0x08, 0x06,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x66, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa,
	0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa,
	0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa,
	0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa,
	0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	} ;

UCHAR RT2883_NUM_BBP_REG_PARMS = (sizeof(RT2883_BBPRegTable) / sizeof(REG_PAIR));

RTMP_REG_PAIR	RT2883_MACRegTable[] =	{
	{TX_SW_CFG0,		0x0},     /* Gary,2008-12-15 for Intel 5300 (FIXME)*/
	{TX_SW_CFG1,		0x0}, 	  /* Gary,2008-12-15 for Intel 5300 (FIXME)*/
	{TX_SW_CFG2,		0x40000}, 	  /* Gary,2008-12-15 for Intel 5300 (FIXME)*/
	{TX_TXBF_CFG_0,		0x8000FC21},	/* Force MCS0 for sounding response*/
	{TX_TXBF_CFG_3,		0x00009c40},	/* ETxBF Timeout = 1 sec = 0x9c40*(25 usec)*/
	{TX_FBK_CFG_3S_0,	0x12111008},	/* default value*/
};

UCHAR RT2883_NUM_MAC_REG_PARMS = (sizeof(RT2883_MACRegTable) / sizeof(RTMP_REG_PAIR));

#ifdef CONFIG_AP_SUPPORT
RTMP_REG_PAIR	RT2883_AP_MACRegTable[] =	{
	{TX_CHAIN_ADDR0_L,	0xFFFFFFFF},	/* Broadcast frames are in stream mode*/
	{TX_CHAIN_ADDR0_H,	0xFFFFF},
};

UCHAR RT2883_NUM_AP_MAC_REG_PARMS = (sizeof(RT2883_AP_MACRegTable) / sizeof(RTMP_REG_PAIR));
#endif /* CONFIG_AP_SUPPORT */	

/*
========================================================================
Routine Description:
	Initialize specific MAC registers.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT2883MacRegisters(
	IN	PRTMP_ADAPTER		pAd)
{
	UINT32 IdReg;


	for (IdReg = 0; IdReg < RT2883_NUM_MAC_REG_PARMS; IdReg++)
	{
		/* if we want to change the full regiter content */
		RTMP_IO_WRITE32(pAd, (USHORT)RT2883_MACRegTable[IdReg].Register,
								RT2883_MACRegTable[IdReg].Value);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (IdReg = 0; IdReg < RT2883_NUM_AP_MAC_REG_PARMS; IdReg++)
		{
			RTMP_IO_WRITE32(pAd, (USHORT)RT2883_AP_MACRegTable[IdReg].Register, 
								RT2883_AP_MACRegTable[IdReg].Value);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

}


/*
========================================================================
Routine Description:
	Initialize specific BBP registers.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT2883BbpRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT32 IdReg;

	DBGPRINT(RT_DEBUG_TRACE, ("%s --->\n", __FUNCTION__));

	for (IdReg = 0; IdReg < RT2883_NUM_BBP_REG_PARMS; IdReg++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, RT2883_BBPRegTable[IdReg].Register, 
									RT2883_BBPRegTable[IdReg].Value);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP_R%d=%d\n", RT2883_BBPRegTable[IdReg].Register, 
									RT2883_BBPRegTable[IdReg].Value));
	}

	/* Set ITxBF timeout to 0x9C40=1000msec*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, 0x02);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, 0x40);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, 0x01);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, 0x9C);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, 0x00);

	DBGPRINT(RT_DEBUG_TRACE, ("%s <---\n", __FUNCTION__));
	
}


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
VOID	RTMPRT2883ReadTxPwrPerRate(
	IN	PRTMP_ADAPTER	pAd)
{
	/*ULONG		data; */
	USHORT		/*i,*/ value, value2;
	
	DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));


	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_CCK_OFDM, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_CCK_OFDM + 2, value2);
	pAd->Tx20MPwrCfgGBand[0] = (value2 << 16) | value;
	pAd->Tx40MPwrCfgGBand[0] = pAd->Tx20MPwrCfgGBand[0];
	pAd->Tx20MPwrCfgABand[0] = pAd->Tx20MPwrCfgGBand[0];
	pAd->Tx40MPwrCfgABand[0] = pAd->Tx20MPwrCfgGBand[0];

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_CCK_OFDM + 4, value);
	pAd->Tx20MPwrCfgGBand[1] = value;
	pAd->Tx40MPwrCfgGBand[1] = value;
	pAd->Tx20MPwrCfgABand[1] = value;
	pAd->Tx40MPwrCfgABand[1] = value;
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G, value);
	pAd->Tx20MPwrCfgGBand[1] |= (value << 16);
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band-%lx \n", pAd->Tx20MPwrCfgGBand[1]));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G, value);
	pAd->Tx40MPwrCfgGBand[1] |= (value << 16);
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 2.4G band-%lx \n", pAd->Tx40MPwrCfgGBand[1]));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G, value);
	pAd->Tx20MPwrCfgABand[1] |= (value << 16);
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 5GHz band-%lx \n", pAd->Tx20MPwrCfgABand[1]));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G, value);
	pAd->Tx40MPwrCfgABand[1] |= (value << 16);
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 5GHz band-%lx \n", pAd->Tx40MPwrCfgABand[1]));
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 2, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value2);
	pAd->Tx20MPwrCfgGBand[2] = (value2 << 16) | value;
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band-%lx \n", pAd->Tx20MPwrCfgGBand[2]));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 2, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 4, value2);
	pAd->Tx40MPwrCfgGBand[2] = (value2 << 16) | value;
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 2.4G band-%lx \n", pAd->Tx40MPwrCfgGBand[2]));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 2, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 4, value2);
	pAd->Tx20MPwrCfgABand[2] = (value2 << 16) | value;
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 5GHz band-%lx \n", pAd->Tx20MPwrCfgABand[2]));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 2, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 4, value2);
	pAd->Tx40MPwrCfgABand[2] = (value2 << 16) | value;
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 5GHz band-%lx \n", pAd->Tx40MPwrCfgABand[2]));

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 6, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 12, value2);
	pAd->Tx20MPwrCfgGBand[3] = (value2 << 16) | value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 6, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 12, value2);
	pAd->Tx40MPwrCfgGBand[3] = (value2 << 16) | value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 6, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 12, value2);
	pAd->Tx20MPwrCfgABand[3] = (value2 << 16) | value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 6, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 12, value2);
	pAd->Tx40MPwrCfgABand[3] = (value2 << 16) | value;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 14, value);		
	pAd->Tx20MPwrCfgGBand[4] = value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 14, value);		
	pAd->Tx40MPwrCfgGBand[4] = value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 14, value);		
	pAd->Tx20MPwrCfgABand[4] = value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 14, value);		
	pAd->Tx40MPwrCfgABand[4] = value;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 8, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 10, value2);
	pAd->Tx20MPwrCfgGBand[5] = (value & 0xff) | ((value & 0xf0) << 4) | ((value & 0xff00) << 8) | ((value & 0xf000) << 12);
	pAd->Tx20MPwrCfgGBand[6] = (value2 & 0xff) | ((value2 & 0xf0) << 4) | ((value2 & 0xff00) << 8) | ((value2 & 0xf000) << 12);
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 8, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 10, value2);
	pAd->Tx40MPwrCfgGBand[5] = (value & 0xff) | ((value & 0xf0) << 4) | ((value & 0xff00) << 8) | ((value & 0xf000) << 12);
	pAd->Tx40MPwrCfgGBand[6] = (value2 & 0xff) | ((value2 & 0xf0) << 4) | ((value2 & 0xff00) << 8) | ((value2 & 0xf000) << 12);
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 8, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 10, value2);
	pAd->Tx20MPwrCfgABand[5] = (value & 0xff) | ((value & 0xf0) << 4) | ((value & 0xff00) << 8) | ((value & 0xf000) << 12);
	pAd->Tx20MPwrCfgABand[6] = (value2 & 0xff) | ((value2 & 0xf0) << 4) | ((value2 & 0xff00) << 8) | ((value2 & 0xf000) << 12);
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 8, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 10, value2);
	pAd->Tx40MPwrCfgABand[5] = (value & 0xff) | ((value & 0xf0) << 4) | ((value & 0xff00) << 8) | ((value & 0xf000) << 12);
	pAd->Tx40MPwrCfgABand[6] = (value2 & 0xff) | ((value2 & 0xf0) << 4) | ((value2 & 0xff00) << 8) | ((value2 & 0xf000) << 12);


	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0, pAd->Tx20MPwrCfgGBand[0]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1, pAd->Tx20MPwrCfgGBand[1]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2, pAd->Tx20MPwrCfgGBand[2]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3, pAd->Tx20MPwrCfgGBand[3]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4, pAd->Tx20MPwrCfgGBand[4]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx20MPwrCfgGBand[5]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx20MPwrCfgGBand[6]);
	
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT, (pAd->Tx20MPwrCfgGBand[0] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1_EXT, (pAd->Tx20MPwrCfgGBand[1] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2_EXT, (pAd->Tx20MPwrCfgGBand[2] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3_EXT, (pAd->Tx20MPwrCfgGBand[3] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4_EXT, (pAd->Tx20MPwrCfgGBand[4] & 0xf0f0f0f0) >> 4);
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
VOID	RTMPRT2883ReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR				i, choffset;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_TX_PWR_STRUC	    Power2;
	EEPROM_TX_PWR_STRUC	    Power3;
	
	/*
		Read Tx power value for all channels
			Value from 1 - 0x7f. Default value is 24.
			Power value : 2.4G 0x00 (0) ~ 0x1F (31)
						: 5.5G 0xF9 (-7) ~ 0x0F (15)
	*/
	
	/* 0. 11b/g, ch1 - ch 14 */
	for (i = 0; i < 7; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX3_PWR_OFFSET + i * 2, Power3.word);
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

		if ((Power3.field.Byte0 > 31) || (Power3.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power3 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 > 31) || (Power3.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power3 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power3 = Power3.field.Byte1;
	}
	
	/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz) */
	/* 1.1 Fill up channel */
	choffset = 14;
	for (i = 0; i < 4; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
	}

	/* 1.2 Fill up power */
	for (i = 0; i < 6; i++)
	{
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word); */
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word); */
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX3_PWR_OFFSET + i * 2, Power3.word);

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		if ((Power3.field.Byte0 < 16) && (Power3.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 < 16) && (Power3.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;			
	}
	
	/* 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz) */
	/* 2.1 Fill up channel */
	choffset = 14 + 12;
	for (i = 0; i < 5; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
	pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 5 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;

	/* 2.2 Fill up power */
	for (i = 0; i < 8; i++)
	{
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word); */
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word); */
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX3_PWR_OFFSET + (choffset - 14) + i * 2, Power3.word);

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		if ((Power3.field.Byte0 < 16) && (Power3.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 < 16) && (Power3.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;			
	}

	/* 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165 (including central frequency in BW 40MHz) */
	/* 3.1 Fill up channel */
	choffset = 14 + 12 + 16;
	for (i = 0; i < 2; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 2 + choffset + 0].Channel		= 165;
	pAd->TxPower[3 * 2 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 2 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 2 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;

	/* 3.2 Fill up power */
	for (i = 0; i < 4; i++)
	{
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word); */
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word); */
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX3_PWR_OFFSET + (choffset - 14) + i * 2, Power3.word);

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		if ((Power3.field.Byte0 < 16) && (Power3.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 < 16) && (Power3.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;			
	}

	/* 4. Print and Debug */
	choffset = 14 + 12 + 16 + 7;
	
}


#endif /* RT2883 */

