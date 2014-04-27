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

#ifdef RTMP_MAC_PCI
#define EEPROM_SIZE					0x200
#endif /* RTMP_MAC_PCI */



#ifdef RT_BIG_ENDIAN
typedef	union _EEPROM_WORD_STRUC {
	struct {
		UCHAR	Byte1;				// High Byte
		UCHAR	Byte0;				// Low Byte
	} field;
	USHORT	word;
} EEPROM_WORD_STRUC;
#else
typedef	union _EEPROM_WORD_STRUC {
	struct {
		UCHAR	Byte0;
		UCHAR	Byte1;
	} field;
	USHORT	word;
} EEPROM_WORD_STRUC;
#endif


/* ------------------------------------------------------------------- */
/*  E2PROM data layout */
/* ------------------------------------------------------------------- */

/* Board type */

#define BOARD_TYPE_MINI_CARD		0	/* Mini card */
#define BOARD_TYPE_USB_PEN		1	/* USB pen */

/*
	EEPROM antenna select format
*/

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		USHORT DACTestBit:1;	/* control if driver should patch the DAC issue */
		USHORT CoexBit:1;
		USHORT bInternalTxALC:1;	/* Internal Tx ALC */
		USHORT AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		USHORT AntDiversity:1;	/* Antenna diversity */
		USHORT Rsv1:1;	/* must be 0 */
		USHORT BW40MAvailForA:1;	/* 0:enable, 1:disable */
		USHORT BW40MAvailForG:1;	/* 0:enable, 1:disable */
		USHORT EnableWPSPBC:1;	/* WPS PBC Control bit */
		USHORT BW40MSidebandForA:1;
		USHORT BW40MSidebandForG:1;
		USHORT CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		USHORT ExternalLNAForA:1;	/* external LNA enable for 5G */
		USHORT ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		USHORT DynamicTxAgcControl:1;	/* */
		USHORT HardwareRadioControl:1;	/* Whether RF is controlled by driver or HW. 1:enable hw control, 0:disable */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#else
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		USHORT HardwareRadioControl:1;	/* 1:enable, 0:disable */
		USHORT DynamicTxAgcControl:1;	/* */
		USHORT ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		USHORT ExternalLNAForA:1;	/* external LNA enable for 5G */
		USHORT CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		USHORT BW40MSidebandForG:1;
		USHORT BW40MSidebandForA:1;
		USHORT EnableWPSPBC:1;	/* WPS PBC Control bit */
		USHORT BW40MAvailForG:1;	/* 0:enable, 1:disable */
		USHORT BW40MAvailForA:1;	/* 0:enable, 1:disable */
		USHORT Rsv1:1;	/* must be 0 */
		USHORT AntDiversity:1;	/* Antenna diversity */
		USHORT AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		USHORT bInternalTxALC:1;	/* Internal Tx ALC */
		USHORT CoexBit:1;
		USHORT DACTestBit:1;	/* control if driver should patch the DAC issue */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#endif


#if defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592)

#ifdef RTMP_PCI_SUPPORT
#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_NIC_CINFIG3_STRUC {
	struct {
		USHORT Rsv1:4;	/* must be 0 */
		USHORT rx_temp_comp:1;
		USHORT CrystalShared:2;
		USHORT CoexAnt:1;
		USHORT TxStream:4;	/* Number of Tx stream */
		USHORT RxStream:4;	/* Number of rx stream */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG3_STRUC, *PEEPROM_NIC_CONFIG3_STRUC;
#else
typedef union _EEPROM_NIC_CINFIG3_STRUC {
	struct {
		USHORT RxStream:4;	/* Number of rx stream */
		USHORT TxStream:4;	/* Number of Tx stream */
		USHORT CoexAnt:1;
		USHORT CrystalShared:2;
		USHORT rx_temp_comp:1;
		USHORT Rsv1:4;	/* must be 0 */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG3_STRUC, *PEEPROM_NIC_CONFIG3_STRUC;
#endif
#endif /* RTMP_PCI_SUPPORT */
#endif /* defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) */



/*
	TX_PWR Value valid range 0xFA(-6) ~ 0x24(36)
*/
#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte1;	/* High Byte */
		signed char Byte0;	/* Low Byte */
	} field;
	USHORT word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#else
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte0;	/* Low Byte */
		signed char Byte1;	/* High Byte */
	} field;
	USHORT word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_VERSION_STRUC {
	struct {
		UCHAR Version;	/* High Byte */
		UCHAR FaeReleaseNumber;	/* Low Byte */
	} field;
	USHORT word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#else
typedef union _EEPROM_VERSION_STRUC {
	struct {
		UCHAR FaeReleaseNumber;	/* Low Byte */
		UCHAR Version;	/* High Byte */
	} field;
	USHORT word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_LED_STRUC {
	struct {
		USHORT Rsvd:3;	/* Reserved */
		USHORT LedMode:5;	/* Led mode. */
		USHORT PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		USHORT PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		USHORT PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		USHORT PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		USHORT PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		USHORT PolarityACT:1;	/* Polarity ACT setting. */
		USHORT PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		USHORT PolarityRDY_G:1;	/* Polarity RDY_G setting. */
	} field;
	USHORT word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#else
typedef union _EEPROM_LED_STRUC {
	struct {
		USHORT PolarityRDY_G:1;	/* Polarity RDY_G setting. */
		USHORT PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		USHORT PolarityACT:1;	/* Polarity ACT setting. */
		USHORT PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		USHORT PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		USHORT PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		USHORT PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		USHORT PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		USHORT LedMode:5;	/* Led mode. */
		USHORT Rsvd:3;	/* Reserved */
	} field;
	USHORT word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		UCHAR TxPowerEnable:1;	/* Enable */
		UCHAR Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		UCHAR DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
	} field;
	UCHAR value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#else
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		UCHAR DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
		UCHAR Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		UCHAR TxPowerEnable:1;	/* Enable */
	} field;
	UCHAR value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		UCHAR	Byte1;	/* High Byte */
		UCHAR	Byte0;	/* Low Byte */
	} field;
	
	USHORT		word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#else
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		UCHAR	Byte0;	/* Low Byte */
		UCHAR	Byte1;	/* High Byte */
	} field;

	USHORT		word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#endif /* RT_BIG_ENDIAN */


struct _RTMP_ADAPTER;

#ifdef RTMP_PCI_SUPPORT
int rtmp_ee_prom_read16(struct _RTMP_ADAPTER *pAd, USHORT Offst, USHORT *pVal);
int rtmp_ee_prom_write16(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT val);
#endif /* RTMP_PCI_SUPPORT */




#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
NDIS_STATUS rtmp_nv_init(struct _RTMP_ADAPTER *pAd);
int rtmp_ee_flash_read(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pVal);
int rtmp_ee_flash_write(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT data);
VOID rtmp_ee_flash_read_all(struct _RTMP_ADAPTER *pAd, USHORT *Data);
VOID rtmp_ee_flash_write_all(struct _RTMP_ADAPTER *pAd, USHORT *Data);
#endif /* defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT) */


#ifdef RTMP_EFUSE_SUPPORT
INT eFuseLoadEEPROM(struct _RTMP_ADAPTER *pAd);
INT eFuseWriteEeeppromBuf(struct _RTMP_ADAPTER *pAd);
VOID eFuseGetFreeBlockCount(struct _RTMP_ADAPTER *pAd, UINT *EfuseFreeBlock);

int rtmp_ee_efuse_read16(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pVal);
int rtmp_ee_efuse_write16(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT data);


NTSTATUS eFuseRead(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pData, USHORT len);
NTSTATUS eFuseWrite(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pData, USHORT len);

INT eFuse_init(struct _RTMP_ADAPTER *pAd);
INT efuse_probe(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_EFUSE_SUPPORT */


/*************************************************************************
  *	Public function declarations for prom operation callback functions setting
  ************************************************************************/
INT RtmpChipOpsEepromHook(struct _RTMP_ADAPTER *pAd, INT infType);

#endif /* __EEPROM_H__ */
