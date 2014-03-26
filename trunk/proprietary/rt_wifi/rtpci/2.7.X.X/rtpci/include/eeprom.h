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
	eeprom.h

	Abstract:
	Miniport header file for eeprom related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

/* For ioctl check usage */
#define EEPROM_IS_PROGRAMMED		0x80

#define E2P_NONE			0x00
#define E2P_EFUSE_MODE			0x01
#define E2P_FLASH_MODE			0x02
#define E2P_EEPROM_MODE			0x03
#define E2P_BIN_MODE			0x04
#define NUM_OF_E2P_MODE			0x05

#ifdef RTMP_MAC_PCI
#define MAX_EEPROM_BIN_FILE_SIZE	512
#else
#define MAX_EEPROM_BIN_FILE_SIZE	1024
#endif /* !RTMP_MAC_PCI */

#define EEPROM_SIZE			0x200

#ifdef BB_SOC
#define BIN_FILE_PATH			"/etc/RT30xxEEPROM.bin"
#else
#define BIN_FILE_PATH			"/tmp/RT30xxEEPROM.bin"
#endif /* BB_SOC */

#define EEPROM_1ST_FILE_DIR		"/etc_ro/Wireless/iNIC/"
#define EEPROM_2ND_FILE_DIR		"/etc_ro/Wireless/iNIC/"

#ifdef RTMP_PCI_SUPPORT
/*************************************************************************
  *	Public function declarations for prom-based chipset
  ************************************************************************/
int rtmp_ee_prom_read16(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			Offset,
	OUT USHORT			*pValue);

int rtmp_ee_prom_write16(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			Offset,
	IN USHORT			value);

#endif /* RTMP_PCI_SUPPORT */




#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
/*************************************************************************
  *	Public function declarations for flash-based chipset
  ************************************************************************/
NDIS_STATUS rtmp_nv_init(
	IN PRTMP_ADAPTER 	pAd);

int rtmp_ee_flash_read(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			Offset,
	OUT USHORT 			*pValue);

int rtmp_ee_flash_write(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			Data);

VOID rtmp_ee_flash_read_all(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			*Data);

VOID rtmp_ee_flash_write_all(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			*Data);

#endif /* defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT) */


#ifdef RTMP_EFUSE_SUPPORT
/*************************************************************************
  *	Public function declarations for efuse-based chipset
  ************************************************************************/
int rtmp_ee_efuse_read16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset,
	OUT USHORT 			*pValue);

int rtmp_ee_efuse_write16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			data);

INT rtmp_ee_write_to_efuse(
	IN PRTMP_ADAPTER 	pAd);
#endif /* RTMP_EFUSE_SUPPORT */


/*************************************************************************
  *	Public function declarations for using BIN buffer mode to access eeprom contents
  ************************************************************************/
INT rtmp_ee_bin_read16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset,
	OUT USHORT 			*pValue);

INT rtmp_ee_bin_write16(
	IN RTMP_ADAPTER 	*pAd, 
	IN USHORT 			Offset, 
	IN USHORT 			data);

INT rtmp_ee_load_from_bin(
	IN PRTMP_ADAPTER 	pAd);

INT rtmp_ee_write_to_bin(
	IN PRTMP_ADAPTER 	pAd);

INT rtmp_ee_write_to_prom(
	IN PRTMP_ADAPTER 	pAd);
	
INT Set_LoadEepromBufferFromBin_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg);

INT Set_EepromBufferWriteBack_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg);

/*************************************************************************
  *	Public function declarations for prom operation callback functions setting
  ************************************************************************/
UCHAR RtmpEepromGetDefault(
	IN RTMP_ADAPTER 	*pAd);

INT RtmpChipOpsEepromHook(
	IN RTMP_ADAPTER 	*pAd,
	IN INT				infType);

BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER 	pAd,
	IN UINT32 			ChipVersion,
	OUT PSTRING 			*pBinFileName);

#endif /* __EEPROM_H__ */
