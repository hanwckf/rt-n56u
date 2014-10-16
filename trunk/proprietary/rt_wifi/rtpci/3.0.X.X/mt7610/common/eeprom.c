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
	eeprom.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#include "rt_config.h"


INT RtmpChipOpsEepromHook(RTMP_ADAPTER *pAd, INT infType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	UINT32 e2p_csr;

#ifdef RTMP_FLASH_SUPPORT
#ifdef MT76x0
	/*
		MT7610E alrady do rtmp_nv_init in MT76x0_WLAN_ChipOnOff.
	*/
	if (IS_MT7610E(pAd))
		pChipOps->eeinit = NULL;
	else
#endif /* MT76x0 */
	pChipOps->eeinit = rtmp_nv_init;
	
	pChipOps->eeread = rtmp_ee_flash_read;
	pChipOps->eewrite = rtmp_ee_flash_write;
	return 0;
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
	efuse_probe(pAd);
	if(pAd->bUseEfuse)
	{
		pChipOps->eeinit = eFuse_init;
		pChipOps->eeread = rtmp_ee_efuse_read16;
		pChipOps->eewrite = rtmp_ee_efuse_write16;
		DBGPRINT(RT_DEBUG_OFF, ("NVM is EFUSE\n"));
		DBGPRINT(RT_DEBUG_TRACE, ("Efuse Size=0x%x [Range:%x-%x] \n",
				pAd->chipCap.EFUSE_USAGE_MAP_SIZE,
				pAd->chipCap.EFUSE_USAGE_MAP_START,
				pAd->chipCap.EFUSE_USAGE_MAP_END));
		return 0;
	}
	else
	{
		pAd->bFroceEEPROMBuffer = FALSE;
		DBGPRINT(RT_DEBUG_OFF, ("NVM is EEPROM\n"));
	}
#endif /* RTMP_EFUSE_SUPPORT */

	switch(infType)
	{
#ifdef RTMP_PCI_SUPPORT
		case RTMP_DEV_INF_PCI:
		case RTMP_DEV_INF_PCIE:
			/* Init EEPROM Address Number, before access EEPROM; if 93c46, EEPROMAddressNum=6, else if 93c66, EEPROMAddressNum=8 */
			RTMP_IO_READ32(pAd, E2PROM_CSR, &e2p_csr);
#ifdef RT3290
			if (IS_RT3290(pAd))
				pAd->EEPROMAddressNum = 8;     // 93C66
			else
#endif /* RT3290 */
#ifdef RT65xx
			if (IS_RT65XX(pAd))
				pAd->EEPROMAddressNum = 8;     // 93C66
			else
#endif /* RT65xx */
			if ((e2p_csr & 0x30) == 0)
				pAd->EEPROMAddressNum = 6;		/* 93C46*/
			else if ((e2p_csr & 0x30) == 0x10)
				pAd->EEPROMAddressNum = 8;     /* 93C66*/
			else
				pAd->EEPROMAddressNum = 8;     /* 93C86*/
			DBGPRINT(RT_DEBUG_TRACE, ("--> E2PROM_CSR=0x%x, EEPROMAddressNum=%d\n",
										e2p_csr, pAd->EEPROMAddressNum ));
			pChipOps->eeinit = NULL;
			pChipOps->eeread = rtmp_ee_prom_read16;
			pChipOps->eewrite = rtmp_ee_prom_write16;
			break;
#endif /* RTMP_PCI_SUPPORT */


		default:
			DBGPRINT(RT_DEBUG_ERROR, ("RtmpChipOpsEepromHook() failed!\n"));
			break;
	}

	return 0;
}

