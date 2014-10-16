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
	Name		Date				Modification logs
*/
#include "rt_config.h"

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
static struct {
	UINT32 	ChipVersion;
	PSTRING 	name;
} RTMP_CHIP_E2P_FILE_TABLE[] = {
#ifdef BB_SOC
	{0x3071,	"RT3092E2P.bin"},
	{0x3090,	"RT3090E2P.bin"},
	{0x3593,	"RT3592E2P.bin"},
	{0x5390,	"RT5390E2P.bin"},
	{0x5392,	"RT5392E2P.bin"},
	{0x5592,	"RT5592E2P.bin"},
#else
	{0x3071,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3090,	"RT3090_PCIe_1T1R_ALC_V1_2.bin"},
	{0x3390,	"RT3390_PCIe_1T1R_LNA_ALC_ADT_R21_V1_2.bin"},
	{0x3593,	"RT3593_PCIe_3T3R_V1_3.bin"},
	{0x5390,	"RT5390_PCIe_1T1R_MAIN_ANT_V1_3.bin" },
	{0x5392,	"RT5392_PCIe_2T2R_ALC_V1_4.bin"},
	{0x5592,	"RT5592_PCIe_2T2R_V1_7.bin"},
#endif /* BB_SOC */
	{0,}
};
#endif
#endif

UCHAR RtmpEepromGetDefault(
	IN RTMP_ADAPTER 	*pAd)
{
	UCHAR e2p_dafault = 0;
	
	if (IS_RT2860(pAd) || IS_RT2870(pAd))
		e2p_dafault = E2P_EEPROM_MODE;
	else if (pAd->infType == RTMP_DEV_INF_RBUS)
		e2p_dafault = E2P_FLASH_MODE;
	else
		e2p_dafault = E2P_EFUSE_MODE;

	DBGPRINT(RT_DEBUG_OFF, ("%s::e2p_dafault=%d\n", __FUNCTION__, e2p_dafault));
	return e2p_dafault;
}


INT RtmpChipOpsEepromHook(
	IN RTMP_ADAPTER 	*pAd,
	IN INT				infType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	UCHAR e2p_type = pAd->E2pAccessMode;
	UINT32 val;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::e2p_type=%d, inf_Type=%d\n", __FUNCTION__, e2p_type, infType));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		return -1;
#if 0
	/* If e2p_type is out of range, get the default mode */
	e2p_type = ((e2p_type != 0) && (e2p_type < NUM_OF_E2P_MODE)) ? e2p_type : RtmpEepromGetDefault(pAd);
#else
	pAd->E2pAccessMode = E2P_FLASH_MODE;
        e2p_type = E2P_FLASH_MODE;
#endif
	if (infType == RTMP_DEV_INF_RBUS)
	{
		e2p_type = E2P_FLASH_MODE;
		pChipOps->loadFirmware = NULL;
	}

	switch (e2p_type)
	{
		case E2P_EEPROM_MODE:
			break;
		case E2P_BIN_MODE:
		{
			pChipOps->eeinit = rtmp_ee_load_from_bin;
			pChipOps->eeread = rtmp_ee_bin_read16;
			pChipOps->eewrite = rtmp_ee_bin_write16;
			DBGPRINT(RT_DEBUG_OFF, ("NVM is BIN mode\n"));
			return 0;
		}

#ifdef RTMP_FLASH_SUPPORT
		case E2P_FLASH_MODE:
		{
			pChipOps->eeinit = rtmp_nv_init;
			pChipOps->eeread = rtmp_ee_flash_read;
			pChipOps->eewrite = rtmp_ee_flash_write;
			DBGPRINT(RT_DEBUG_OFF, ("NVM is FLASH mode\n"));
			return 0;
		}
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
		default:
		{
			efuse_probe(pAd);
			if (pAd->bUseEfuse)
			{
				pChipOps->eeinit = eFuse_init;
				pChipOps->eeread = rtmp_ee_efuse_read16;
				pChipOps->eewrite = rtmp_ee_efuse_write16;
				DBGPRINT(RT_DEBUG_OFF, ("NVM is EFUSE mode\n"));	
				return 0;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::hook efuse mode failed\n", __FUNCTION__));
				break;
			}
		}
#endif /* RTMP_EFUSE_SUPPORT */
	}

	/* Hook functions based on interface types for EEPROM */
	switch (infType) 
	{
#ifdef RTMP_PCI_SUPPORT
		case RTMP_DEV_INF_PCI:
		case RTMP_DEV_INF_PCIE:
			RTMP_IO_READ32(pAd, E2PROM_CSR, &val);
			if (((val & 0x30) == 0) && (!IS_RT3290(pAd)))
				pAd->EEPROMAddressNum = 6; /* 93C46 */
			else
				pAd->EEPROMAddressNum = 8; /* 93C66 or 93C86 */

			pChipOps->eeinit = NULL;
			pChipOps->eeread = rtmp_ee_prom_read16;
			pChipOps->eewrite = rtmp_ee_prom_write16;
			break;
#endif /* RTMP_PCI_SUPPORT */


		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::hook failed\n", __FUNCTION__));
			break;
	}

	DBGPRINT(RT_DEBUG_OFF, ("NVM is EEPROM mode\n"));
	return 0;
}

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER 	pAd,
	IN UINT32 			ChipVersion,
	OUT PSTRING 			*pBinFileName)
{
	BOOLEAN found = FALSE;
	INT i = 0;
	
	for (i = 0; RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion != 0; i++)
	{
		if (RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion == ChipVersion)
		{
			*pBinFileName = RTMP_CHIP_E2P_FILE_TABLE[i].name;
			found = TRUE;
			break;
		}
	}

	if (found == TRUE)
		DBGPRINT(RT_DEBUG_OFF, ("%s::Found E2P bin file name=%s\n", __FUNCTION__, *pBinFileName));
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s::E2P bin file name not found\n", __FUNCTION__));
	
	return found;
}
#endif
#endif

INT rtmp_ee_bin_read16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset,
	OUT USHORT 			*pValue)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s::Read from EEPROM buffer\n", __FUNCTION__));
	NdisMoveMemory(pValue, &(pAd->EEPROMImage[Offset]), 2);
	*pValue = le2cpu16(*pValue);

	return (*pValue);
}


INT rtmp_ee_bin_write16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			data)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s::Write to EEPROM buffer\n", __FUNCTION__));
	data = le2cpu16(data);
	NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &data, 2);

	return 0;
}


INT rtmp_ee_load_from_bin(
	IN PRTMP_ADAPTER 	pAd)
{
	PSTRING src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	STRING bin_file_path[128];
	PSTRING bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
		src = BIN_FILE_PATH;
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else 
		{
			NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			ret_val = RtmpOSFileRead(srcf, (PSTRING)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			
			if (ret_val > 0)
			{
#ifdef BB_SOC
				INT i;
				for(i=0; i<=MAX_EEPROM_BIN_FILE_SIZE; i+=2) 
				{
					//printk("i=%d, Value=%04x \n", i, *(PUINT16)&pAd->EEPROMImage[i]);
					*(PUINT16)&pAd->EEPROMImage[i]=SWAP16(*(PUINT16)&pAd->EEPROMImage[i]);
				}
#endif /* BB_SOC */
				ret_val = NDIS_STATUS_SUCCESS;
			}	
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s::Read file \"%s\" failed(errCode=%d)!\n", __FUNCTION__, src, ret_val));
      		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);
			
	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;	
}


INT rtmp_ee_write_to_bin(
	IN PRTMP_ADAPTER 	pAd)
{
	PSTRING src = NULL;
	INT ret_val;			
	RTMP_OS_FD srcf;
#ifdef BB_SOC	
	INT ret_val2;
	RTMP_OS_FD srcf2;
#endif /* BB_SOC */
	RTMP_OS_FS_INFO osFSInfo;

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	STRING bin_file_path[128];
	PSTRING bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
		src = BIN_FILE_PATH;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);	

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_WRONLY|O_CREAT, 0);
#ifdef BB_SOC		
		srcf2 = RtmpOSFileOpen(BIN_FILE_PATH, O_WRONLY|O_CREAT, 0);
#endif /* BB_SOC */

		if (IS_FILE_OPEN_ERR(srcf)
#ifdef BB_SOC
			|| IS_FILE_OPEN_ERR(srcf2)
#endif /* BB_SOC */
			) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else 
		{
			RtmpOSFileWrite(srcf, (PSTRING)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
#ifdef BB_SOC
			RtmpOSFileWrite(srcf2, (PSTRING)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
#endif /* BB_SOC */			
		}	
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);
#ifdef BB_SOC
	ret_val2 = RtmpOSFileClose(srcf2);
#endif /* BB_SOC */
			
	if (ret_val
#ifdef BB_SOC
		&& ret_val2
#endif /* BB_SOC */		
		)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));
	
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	
	return TRUE;	
}


INT Set_LoadEepromBufferFromBin_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	
	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM buffer from BIN, and change to BIN buffer mode\n"));	
		rtmp_ee_load_from_bin(pAd);

		/* Change to BIN eeprom buffer mode */
		pAd->E2pAccessMode = E2P_BIN_MODE;
		RtmpChipOpsEepromHook(pAd, pAd->infType);
		return TRUE;
	}
}


INT Set_EepromBufferWriteBack_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg)
{
	UINT e2p_mode = simple_strtol(arg, 0, 10);

	if (e2p_mode >= NUM_OF_E2P_MODE)
		return FALSE;

	switch (e2p_mode)
	{
#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to eFuse\n"));
			rtmp_ee_write_to_efuse(pAd);
			break;
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RTMP_FLASH_SUPPORT
		case E2P_FLASH_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to Flash\n"));
			rtmp_ee_flash_write_all(pAd, (PUSHORT)pAd->EEPROMImage);
			break;
#endif /* RTMP_FLASH_SUPPORT */

		case E2P_EEPROM_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to EEPROM\n"));
			rtmp_ee_write_to_prom(pAd);
			break;

		case E2P_BIN_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to BIN\n"));	
			rtmp_ee_write_to_bin(pAd);
			break;
			
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::do not support this EEPROM access mode\n", __FUNCTION__));
			return FALSE;
	}
	
	return TRUE;
}
