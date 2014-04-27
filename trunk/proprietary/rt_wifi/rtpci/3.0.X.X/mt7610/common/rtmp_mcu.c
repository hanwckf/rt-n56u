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
	rtmp_mcu.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"
#include 	"firmware.h"



/* New 8k byte firmware size for RT3071/RT3072*/
#define FIRMWAREIMAGE_MAX_LENGTH	0x2000
#ifdef WOW_SUPPORT 
#define FIRMWAREIMAGE_WOW_LENGTH	0x3000 /* WOW support firmware(12KB) */
#endif/*WOW_SUPPORT*/
#define FIRMWAREIMAGE_LENGTH			(sizeof (FirmwareImage) / sizeof(UCHAR))
#define FIRMWARE_MAJOR_VERSION		0

#define FIRMWAREIMAGEV1_LENGTH		0x1000
#define FIRMWAREIMAGEV2_LENGTH		0x1000
#ifdef WOW_SUPPORT 
#define FIRMWAREIMAGEV3_LENGTH		0x2000 /* WOW support firmware */
#endif/*WOW_SUPPORT*/

#ifdef RTMP_MAC_PCI
#define FIRMWARE_MINOR_VERSION		2
#endif /* RTMP_MAC_PCI */

const unsigned short ccitt_16Table[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
#define ByteCRC16(v, crc) \
	(unsigned short)((crc << 8) ^  ccitt_16Table[((crc >> 8) ^ (v)) & 255])

unsigned char BitReverse(unsigned char x)
{
	int i;
	unsigned char Temp=0;
	for(i=0; ; i++)
	{
		if(x & 0x80)	Temp |= 0x80;
		if(i==7)		break;
		x	<<= 1;
		Temp >>= 1;
	}
	return Temp;
}


/*
	========================================================================
	
	Routine Description:
		erase 8051 firmware image in MAC ASIC

	Arguments:
		Adapter						Pointer to our adapter

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/
INT RtmpAsicEraseFirmware(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 i;

	for(i = 0; i < MAX_FIRMWARE_IMAGE_SIZE; i += 4)
		RTMP_IO_WRITE32(pAd, FIRMWARE_IMAGE_BASE + i, 0);

	return 0;
}

NDIS_STATUS isMCUNeedToLoadFIrmware(
	IN PRTMP_ADAPTER pAd)
{
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	ULONG			Index;
	UINT32			MacReg;
	
	Index = 0;

	do {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return NDIS_STATUS_FAILURE;
		
		RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &MacReg);

		if (MacReg & 0x100) /* check bit 8*/
			break;
		
		RTMPusecDelay(1000);
	} while (Index++ < 100);

	if (Index >= 100)
		Status = NDIS_STATUS_FAILURE;

	return Status;
}

NDIS_STATUS isMCUnotReady(
	IN PRTMP_ADAPTER pAd)
{
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	ULONG			Index;
	UINT32			MacReg;

#ifdef RT65xx
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Ignore for MCU status check for 6590 now!\n", __FUNCTION__));
		return Status;
	}
#endif /* RT65xx */

	
	Index = 0;

	do {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return NDIS_STATUS_FAILURE;
		
		RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &MacReg);

		if (MacReg & 0x80) /* check bit 7*/
			break;
		
		RTMPusecDelay(1000);
	} while (Index++ < 1000);

	if (Index >= 1000)
		Status = NDIS_STATUS_FAILURE;

	return Status;
}
/*
	========================================================================
	
	Routine Description:
		Load 8051 firmware file into MAC ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS         firmware image load ok
		NDIS_STATUS_FAILURE         image not found

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/
NDIS_STATUS RtmpAsicLoadFirmware(
	IN PRTMP_ADAPTER pAd)
{
#ifdef BIN_IN_FILE
#define NICLF_DEFAULT_USE()	\
	flg_default_firm_use = TRUE; \
	DBGPRINT(RT_DEBUG_OFF, ("%s - Use default firmware!\n", __FUNCTION__));

	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	PUCHAR			src;
	RTMP_OS_FD		srcf;
	INT 				retval, i;
	PUCHAR			pFirmwareImage;
	INT				FileLength = 0;
	UINT32			MacReg;
	ULONG			Index;
	ULONG			firm;
	BOOLEAN			flg_default_firm_use = FALSE;
	RTMP_OS_FS_INFO	osFSInfo;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s\n", __FUNCTION__));

	/* init */
	pFirmwareImage = NULL;
	src = RTMP_FIRMWARE_FILE_NAME;

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	pAd->FirmwareVersion = (FIRMWARE_MAJOR_VERSION << 8) + \
						   FIRMWARE_MINOR_VERSION;


	/* allocate firmware buffer */
	os_alloc_mem(pAd, (UCHAR **)&pFirmwareImage, MAX_FIRMWARE_IMAGE_SIZE);
	if (pFirmwareImage == NULL)
	{
		/* allocate fail, use default firmware array in firmware.h */
		DBGPRINT(RT_DEBUG_ERROR, ("%s - Allocate memory fail!\n", __FUNCTION__));
		NICLF_DEFAULT_USE();
	}
	else
	{
		/* allocate ok! zero the firmware buffer */
		memset(pFirmwareImage, 0x00, MAX_FIRMWARE_IMAGE_SIZE);
	}


	/* if ok, read firmware file from *.bin file */
	if (flg_default_firm_use == FALSE)
	{
		do
		{
			/* open the bin file */
			srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(srcf)) 
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s - Error opening file %s\n", __FUNCTION__, src));
				NICLF_DEFAULT_USE();
				break;
			}


			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(srcf, pFirmwareImage, MAX_FIRMWARE_IMAGE_SIZE);
			if (FileLength != MAX_FIRMWARE_IMAGE_SIZE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: error file length (=%d) in RT2860AP.BIN\n",
					   __FUNCTION__, FileLength));
				NICLF_DEFAULT_USE();
				break;
			}
			else
			{
				PUCHAR ptr = pFirmwareImage;
				USHORT crc = 0xffff;


				/* calculate firmware CRC */
				for(i=0; i<(MAX_FIRMWARE_IMAGE_SIZE-2); i++, ptr++)
					crc = ByteCRC16(BitReverse(*ptr), crc);

				if ((pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-2] != \
								(UCHAR)BitReverse((UCHAR)(crc>>8))) ||
					(pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-1] != \
								(UCHAR)BitReverse((UCHAR)crc)))
				{
					/* CRC fail */
					DBGPRINT(RT_DEBUG_ERROR, ("%s: CRC = 0x%02x 0x%02x "
						   "error, should be 0x%02x 0x%02x\n",
						   __FUNCTION__,
						   pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-2],
						   pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-1],
						   (UCHAR)(crc>>8), (UCHAR)(crc)));
					NICLF_DEFAULT_USE();
					break;
				}
				else
				{
					/* firmware is ok */
					pAd->FirmwareVersion = \
						(pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-4] << 8) +
						pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-3];

					/* check if firmware version of the file is too old */
					if ((pAd->FirmwareVersion) < \
											((FIRMWARE_MAJOR_VERSION << 8) +
									  	 	 FIRMWARE_MINOR_VERSION))
					{
						DBGPRINT(RT_DEBUG_ERROR, ("%s: firmware version too old!\n", __FUNCTION__));
						NICLF_DEFAULT_USE();
						break;
					}
				}

				DBGPRINT(RT_DEBUG_TRACE,
						 ("NICLoadFirmware: CRC ok, ver=%d.%d\n",
						  pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-4],
						  pFirmwareImage[MAX_FIRMWARE_IMAGE_SIZE-3]));
			}
			break;
		} while(TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(srcf))
			;
		else
		{
			retval = RtmpOSFileClose(srcf);
			if (retval)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("--> Error %d closing %s\n", -retval, src));
			}
		}
	}


	/* write firmware to ASIC */
	if (flg_default_firm_use == TRUE)
	{
		/* use default fimeware, free allocated buffer */
		if (pFirmwareImage != NULL)
			os_free_mem(NULL, pFirmwareImage);

		/* use default *.bin array */
		pFirmwareImage = FirmwareImage;
		FileLength = sizeof(FirmwareImage);
	}

	/* enable Host program ram write selection */
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x10000); 

	for(i=0; i<FileLength; i+=4)
	{
		firm = pFirmwareImage[i] +
			   (pFirmwareImage[i+3] << 24) +
			   (pFirmwareImage[i+2] << 16) +
			   (pFirmwareImage[i+1] << 8);

		RTMP_IO_WRITE32(pAd, FIRMWARE_IMAGE_BASE + i, firm);
	}

	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x00000);
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x00001);

	/* initialize BBP R/W access agent */
	RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, 0);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CSR, 0);

	if (flg_default_firm_use == FALSE)
	{
		/* use file firmware, free allocated buffer */
		if (pFirmwareImage != NULL)
			os_free_mem(NULL, pFirmwareImage);
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
#else

	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	PUCHAR			pFirmwareImage;
	ULONG			FileLength;
	UINT32			Version = (pAd->MACVersion >> 16);


	pFirmwareImage = FirmwareImage;
	FileLength = sizeof(FirmwareImage);


	/* New 8k byte firmware size for RT3071/RT3072*/
	/*DBGPRINT(RT_DEBUG_TRACE, ("Usb Chip\n"));*/
	if (FIRMWAREIMAGE_LENGTH == FIRMWAREIMAGE_MAX_LENGTH)
	/*The firmware image consists of two parts. One is the origianl and the other is the new.*/
	/*Use Second Part*/
	{
#ifdef RTMP_MAC_PCI
		if ((Version == 0x2860) || (Version == 0x3572) || IS_RT3090(pAd) 
			|| IS_RT3390(pAd) || IS_RT3593(pAd) || IS_RT5390(pAd) || IS_RT5392(pAd))
		{
			pFirmwareImage = FirmwareImage;
			FileLength = FIRMWAREIMAGE_LENGTH;
		}
#endif /* RTMP_MAC_PCI */
	}
	else
	{
#if defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)
		/* WOW firmware is 12KB */
		if ((Version != 0x2860) && (Version != 0x2872) && (Version != 0x3070))
		{
			if (FIRMWAREIMAGE_LENGTH == FIRMWAREIMAGE_WOW_LENGTH) /* size 0x3000 */
			{
				if (pAd->WOW_Cfg.bWOWFirmware == TRUE)
				{
					pFirmwareImage = (PUCHAR)&FirmwareImage[FIRMWAREIMAGEV3_LENGTH]; /* WOW offset: 0x2000 */
					FileLength = FIRMWAREIMAGEV1_LENGTH; /* 0x1000 */
					DBGPRINT(RT_DEBUG_OFF, ("%s: Load WOW firmware!!\n", __FUNCTION__));
				}
				else
				{
					pFirmwareImage = (PUCHAR)&FirmwareImage[FIRMWAREIMAGEV2_LENGTH]; /* normal offset: 0x1000 */
					FileLength = FIRMWAREIMAGEV1_LENGTH; /* 0x1000 */
					DBGPRINT(RT_DEBUG_OFF, ("%s: Load normal firmware!!\n", __FUNCTION__));
				}

			}
		}
		else
#endif /* defined(WOW_SUPPORT) && defined(RTMP_MAC_USB) */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("KH: bin file should be 8KB.\n"));
			Status = NDIS_STATUS_FAILURE;
		}
	}

	RTMP_WRITE_FIRMWARE(pAd, pFirmwareImage, FileLength);

#endif

	if (isMCUnotReady(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): MCU is not ready!\n\n\n", __FUNCTION__));
		Status = NDIS_STATUS_FAILURE;
	}

    DBGPRINT(RT_DEBUG_TRACE, ("<=== %s (status=%d)\n", __FUNCTION__, Status));

    return Status;
}


INT RtmpAsicSendCommandToMcu(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Command,
	IN UCHAR			Token,
	IN UCHAR			Arg0,
	IN UCHAR			Arg1,
	IN BOOLEAN			FlgIsNeedLocked)
{
	HOST_CMD_CSR_STRUC	H2MCmd;
	H2M_MAILBOX_STRUC	H2MMailbox;
	INT i = 0;
	int ret;


#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */



	{
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */


		ret = FALSE;
	do
	{
		RTMP_IO_READ32(pAd, H2M_MAILBOX_CSR, &H2MMailbox.word);
		if (H2MMailbox.field.Owner == 0)
			break;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))				
		{
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
				goto done;
		}
		RTMPusecDelay(2);
	} while(i++ < 100);

	if (i >= 100)
	{
#ifdef RTMP_MAC_PCI
#ifdef RALINK_ATE
			if (IS_PCI_INF(pAd) && (pAd->ate.bFWLoading == TRUE))
				{
			; /* wait for firmware loading */
		}
		else
#endif /* RALINK_ATE */
#endif /* RTMP_MAC_PCI */
		{
		DBGPRINT_ERR(("H2M_MAILBOX still hold by MCU. command fail\n"));
		}
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
			goto done;
	}
#ifdef RTMP_MAC_PCI
#ifdef RALINK_ATE
		else if (IS_PCI_INF(pAd) && (pAd->ate.bFWLoading == TRUE))
	{
		/* mail box is not busy anymore */
		/* reloading of firmware is completed */
		pAd->ate.bFWLoading = FALSE;
	}
#endif /* RALINK_ATE */
#endif /* RTMP_MAC_PCI */

	H2MMailbox.field.Owner	  = 1;	   /* pass ownership to MCU*/
	H2MMailbox.field.CmdToken = Token;
	H2MMailbox.field.HighByte = Arg1;
	H2MMailbox.field.LowByte  = Arg0;
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CSR, H2MMailbox.word);

	H2MCmd.word 			  = 0;
	H2MCmd.field.HostCommand  = Command;
	RTMP_IO_WRITE32(pAd, HOST_CMD_CSR, H2MCmd.word);
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
}


	if (Command == WAKE_MCU_CMD)
		pAd->LastMCUCmd = Command;

	ret = TRUE;

done:

	return ret;
}


#ifdef RTMP_PCI_SUPPORT

INT PCIAsicSendCmdToAndes(PRTMP_ADAPTER pAd, struct CMD_UNIT CmdUnit)
{

	return 0;
}

#endif

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 51)) || (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18))) 
VOID USBUploadFWComplete(purbb_t pURB)
{
	PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	RTMP_ADAPTER *pAd = pHTTXContext->pAd;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	printk("%s\n", __FUNCTION__);
	RTMP_SEM_EVENT_UP(&MCtrl->FWUploadSem);
}
#else
VOID USBUploadFWComplete(purbb_t pURB, pregs *pt_regs)
{
	PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	RTMP_ADAPTER *pAd = pHTTXContext->pAd;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	printk("%s\n", __FUNCTION__);
	RTMP_SEM_EVENT_UP(&MCtrl->FWUploadSem);
}
#endif


static NDIS_STATUS USBLoadIVB(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT32 i;
	USHORT Value;
	USHORT Index;
	USHORT Temp;
	
	Status = RTUSB_VendorRequest(pAd,
								 USBD_TRANSFER_DIRECTION_OUT,
								 DEVICE_VENDOR_REQUEST_OUT,
								 0x01,
								 0x13,
								 0x00,
								 NULL,
								 0);

	if (Status)
	{
			DBGPRINT(RT_DEBUG_ERROR, ("Step 11.1 fail\n"));
			return Status;
	}

	Index = 0xEF00;

	for (i = 0; i < 64; i += 2)
	{
		Value = FirmwareImage[i];
		Temp = FirmwareImage[i + 1];
		Value |= (Temp << 8); 
	
		Status = RTUSB_VendorRequest(pAd,
									 USBD_TRANSFER_DIRECTION_OUT,
									 DEVICE_VENDOR_REQUEST_OUT,
									 0x22,
									 Value,
									 Index,
									 NULL,
									 0);

		if (Status)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Step 11.2 fail\n"));
			return Status;
		}

		Index += 2;
	}

	Status = RTUSB_VendorRequest(pAd,
								 USBD_TRANSFER_DIRECTION_OUT,
								 DEVICE_VENDOR_REQUEST_OUT,
								 0x01,
								 0x14,
								 Index,
								 NULL,
								 0);

	if (Status)
	{
			DBGPRINT(RT_DEBUG_ERROR, ("Step 11.3 fail\n"));
			return Status;
	}

	Status = RTUSB_VendorRequest(pAd,
								 USBD_TRANSFER_DIRECTION_OUT,
								 DEVICE_VENDOR_REQUEST_OUT,
								 0x01,
								 0x12,
								 0x00,
								 FirmwareImage,
								 64);

	if (Status)
	{
			DBGPRINT(RT_DEBUG_ERROR, ("Step 12 fail\n"));
			return Status;
	}

	return Status;
}


NDIS_STATUS USBLoadFirmwareToAndes(RTMP_ADAPTER *pAd)
{
	PURB pURB;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	ra_dma_addr_t DataDMA;
	PUCHAR DataBuffer;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;
	TXINFO_NMAC_CMD *TxInfoCmd;	
	UINT32 FWLen = sizeof(FirmwareImage);
	INT32 SentLen;
	UINT32 CurLen = 0x40;
	UINT32 MACValue;
	USHORT Value;
	PHT_TX_CONTEXT pHTTXContext = &(pAd->TxContext[0]);
	INT Ret;

	RTMP_IO_WRITE32(pAd, 0x250, 0x10058);

	RTMP_IO_WRITE32(pAd, 0xa44, 0x0);

	RTMP_IO_WRITE32(pAd, 0x230, 0x84210);

	RTMP_IO_WRITE32(pAd, 0x400, 0x80c00);

	RTMP_IO_WRITE32(pAd, 0x800, 0x01);

	RTMP_IO_READ32(pAd, 0x0404, &MACValue);
	MACValue |= 0xF;
	RTMP_IO_WRITE32(pAd, 0x0404, MACValue);

	/* Enable FCE */
	RTMP_IO_WRITE32(pAd, FCE_PSE_CTRL, 0x01);

	/* Enable USB_DMA_CFG */
	RTMP_IO_WRITE32(pAd, USB_DMA_CFG, 0xC00000);

	/* FCE tx_fs_base_ptr */
	RTMP_IO_WRITE32(pAd, TX_CPU_PORT_FROM_FCE_BASE_PTR, 0x400230);

	/* FCE tx_fs_max_cnt */
	RTMP_IO_WRITE32(pAd, TX_CPU_PORT_FROM_FCE_MAX_COUNT, 0x01); 

	/* FCE pdma enable */
	RTMP_IO_WRITE32(pAd, FCE_PDMA_GLOBAL_CONF, 0x44);  

	/* FCE skip_fs_en */
	RTMP_IO_WRITE32(pAd, FCE_SKIP_FS, 0x03);

	/* Allocate URB */
	pURB = RTUSB_ALLOC_URB(0);

	if (!pURB)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not allocate URB\n"));
		Status = NDIS_STATUS_RESOURCES; 
		goto error0;
	}

	/* Allocate TransferBuffer */
	DataBuffer = RTUSB_URB_ALLOC_BUFFER(pObj->pUsb_Dev, 1200, &DataDMA);
		
	if (!DataBuffer)
	{
		Status = NDIS_STATUS_RESOURCES;
		goto error1;
	}

	while (1)
	{
		SentLen = (FWLen - CurLen) >= 1024 ? 1024 : (FWLen - CurLen);

		if (SentLen > 0)
		{
			TxInfoCmd = (TXINFO_NMAC_CMD *)DataBuffer;
			TxInfoCmd->pkt_len = SentLen;
			TxInfoCmd->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)TxInfoCmd, TYPE_TXINFO);
#endif
			NdisMoveMemory(DataBuffer + sizeof(*TxInfoCmd), &FirmwareImage[CurLen], SentLen);


			Value = CurLen & 0xFFFF;

			/* Set FCE DMA descriptor */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x230,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set FCE DMA descriptor fail\n"));
				goto error2;
			}
			
			Value = ((CurLen & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x232,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set FCE DMA descriptor fail\n"));
				goto error2;
			}

			

			CurLen += SentLen;

			while ((SentLen % 4) != 0)
				SentLen++;

			Value = ((SentLen << 16) & 0xFFFF);

			/* Set FCE DMA length */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x234,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set FCE DMA length fail\n"));
				goto error2;
			}
			
			Value = (((SentLen << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x236,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set FCE DMA length fail\n"));
				goto error2;
			}
		
			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(pURB,
									 pObj->pUsb_Dev,
									 pAd->BulkOutEpAddr[0],
									 DataBuffer,
									 SentLen + sizeof(*TxInfoCmd),
									 USBUploadFWComplete,
									 pHTTXContext,
									 DataDMA);

			printk("%s: submit URB, SentLen = %d, FWLen = %d, CurLen = %d\n", __FUNCTION__, SentLen, FWLen, CurLen);
			Status = RTUSB_SUBMIT_URB(pURB);

			if (Status)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("SUBMIT URB fail\n"));
				goto error2;
			}
		
			RTMP_SEM_EVENT_WAIT(&MCtrl->FWUploadSem, Ret);

			RTMP_IO_READ32(pAd, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, &MACValue);
			MACValue++;
			RTMP_IO_WRITE32(pAd, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, MACValue);
			mdelay(5);
		}
		else
		{
			/* Upload new 64 bytes interrupt vector */
			Status = USBLoadIVB(pAd);
			break;
		}		

	}
	
error2:
	/* Free TransferBuffer */
	RTUSB_URB_FREE_BUFFER(pObj->pUsb_Dev, 1200, 
								DataBuffer, DataDMA);

error1:
	/* Free URB */
	RTUSB_FREE_URB(pURB);

error0: 
	return Status;
}


VOID MCUCtrlInit(PRTMP_ADAPTER pAd)
{
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	NdisZeroMemory(MCtrl, sizeof(*MCtrl));
	MCtrl->CmdSeq = 1;
	RTMP_SEM_EVENT_INIT_LOCKED(&MCtrl->FWUploadSem, pSemList);
	RTMP_SEM_EVENT_INIT(&MCtrl->CmdRspEventListLock, &pAd->RscSemMemList);
	DlListInit(&MCtrl->CmdRspEventList);
}


VOID MCUCtrlExit(PRTMP_ADAPTER pAd)
{
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	RTMP_SEM_EVENT_DESTORY(&MCtrl->FWUploadSem);
	RTMP_SEM_EVENT_DESTORY(&MCtrl->CmdRspEventListLock);
}


static inline UCHAR GetCmdSeq(PRTMP_ADAPTER pAd)
{
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	(MCtrl->CmdSeq++) == 16 ? 1 : MCtrl->CmdSeq;
	return MCtrl->CmdSeq;
}


#if ((LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 51)) || (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18))) 
VOID USBKickOutCmdComplete(purbb_t pURB)
{
	PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	RTMP_ADAPTER *pAd = pHTTXContext->pAd;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	printk("%s\n", __FUNCTION__);
	RTMP_SEM_EVENT_UP(&MCtrl->FWUploadSem);
}
#else
VOID USBKickOutCmdComplete(purbb_t pURB, pregs *pt_regs)
{
	PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	RTMP_ADAPTER *pAd = pHTTXContext->pAd;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	printk("%s\n", __FUNCTION__);
	RTMP_SEM_EVENT_UP(&MCtrl->FWUploadSem);
}
#endif


INT USBKickOutCmd(PRTMP_ADAPTER pAd, PCHAR Buf, UINT32 Len)
{
	PURB pURB;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PUCHAR DataBuffer;
	ra_dma_addr_t DataDMA;
	PHT_TX_CONTEXT pHTTXContext = &(pAd->TxContext[1]);
	INT Ret;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;

	/* Allocate URB */
	pURB = RTUSB_ALLOC_URB(0);

	if (!pURB)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not allocate URB\n"));
		Status = NDIS_STATUS_RESOURCES; 
		goto error0;
	}

	/* Allocate TransferBuffer */
	DataBuffer = RTUSB_URB_ALLOC_BUFFER(pObj->pUsb_Dev, Len, &DataDMA);
		
	if (!DataBuffer)
	{
		Status = NDIS_STATUS_RESOURCES;
		goto error1;
	}
	
	NdisMoveMemory(DataBuffer, Buf, Len);

	hex_dump("CmdBuffer", (char *)Buf, Len);
			
	/* Initialize URB descriptor */
	RTUSB_FILL_HTTX_BULK_URB(pURB,
							 pObj->pUsb_Dev,
							 pAd->BulkOutEpAddr[0],
							 DataBuffer,
							 Len,
							 USBKickOutCmdComplete,
							 pHTTXContext,
							 DataDMA);

	Status = RTUSB_SUBMIT_URB(pURB);

	if (Status)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("SUBMIT URB fail\n"));
		goto error2;
	}
			
	RTMP_SEM_EVENT_WAIT(&MCtrl->FWUploadSem, Ret);

error2:
	/* Free TransferBuffer */
	RTUSB_URB_FREE_BUFFER(pObj->pUsb_Dev, Len, 
								DataBuffer, DataDMA);

error1:
	/* Free URB */
	RTUSB_FREE_URB(pURB);

error0: 
	return Status;
}

INT AsicSendCmdToAndes(PRTMP_ADAPTER pAd, struct CMD_UNIT CmdUnit)
{
	UINT32 VarLen;
	PCHAR Pos, Buf;
	TXINFO_NMAC_CMD *TxInfoCmd;
	INT32 Ret;
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;
	struct CMD_RSP_EVENT *CmdRspEvent;

	VarLen = sizeof(*TxInfoCmd) + CmdUnit.u.ANDES.CmdPayloadLen;

	os_alloc_mem(pAd, (UCHAR **)&Buf, VarLen);

	Pos = Buf;
	TxInfoCmd = (TXINFO_NMAC_CMD *)Pos;
	TxInfoCmd->info_type = CMD_PACKET;
	TxInfoCmd->d_port = CPU_TX_PORT;
	TxInfoCmd->cmd_type = CmdUnit.u.ANDES.Type;

	if (CmdUnit.u.ANDES.NeedRsp)
	{
		TxInfoCmd->cmd_seq = GetCmdSeq(pAd);

		os_alloc_mem(NULL, (UCHAR **)&CmdRspEvent, sizeof(*CmdRspEvent));

		if (!CmdRspEvent)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
			return NDIS_STATUS_RESOURCES;
		}

		CmdRspEvent->CmdSeq = TxInfoCmd->cmd_seq;
		CmdRspEvent->Timeout = CmdUnit.u.ANDES.Timeout;

		if (CmdUnit.u.ANDES.CmdRspHdler)
			CmdRspEvent->CmdRspHdler = CmdUnit.u.ANDES.CmdRspHdler;

		CmdRspEvent->RspPayload = CmdUnit.u.ANDES.RspPayload;
		CmdRspEvent->RspPayloadLen = &CmdUnit.u.ANDES.RspPayloadLen;

		if (CmdUnit.u.ANDES.NeedWait)
		{
			CmdRspEvent->NeedWait = TRUE;
			RTMP_SEM_EVENT_INIT_LOCKED(&CmdRspEvent->WaitSem, pSemList);
		}

		RTMP_SEM_EVENT_WAIT(&MCtrl->CmdRspEventListLock, Ret);
		DlListAddTail(&MCtrl->CmdRspEventList, &CmdRspEvent->List);
		RTMP_SEM_EVENT_UP(&MCtrl->CmdRspEventListLock);
	}
	else
	{	
		TxInfoCmd->cmd_seq = 0;
	}

	TxInfoCmd->pkt_len = CmdUnit.u.ANDES.CmdPayloadLen;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)TxInfoCmd, TYPE_TXINFO);
#endif

	Pos += sizeof(*TxInfoCmd);
	
	NdisMoveMemory(Pos, CmdUnit.u.ANDES.CmdPayload, CmdUnit.u.ANDES.CmdPayloadLen);

	USBKickOutCmd(pAd, Buf, VarLen);
	
	/* Wait for Command Rsp */
	if (CmdUnit.u.ANDES.NeedWait) {
		RTMP_SEM_EVENT_WAIT(&CmdRspEvent->WaitSem, Ret);		
		RTMP_SEM_EVENT_WAIT(&MCtrl->CmdRspEventListLock, Ret);
		DlListDel(&CmdRspEvent->List);
		os_free_mem(NULL, CmdRspEvent);
		RTMP_SEM_EVENT_UP(&MCtrl->CmdRspEventListLock);
	}

	os_free_mem(NULL, Buf);

	return 0;
}


INT BurstWrite(PRTMP_ADAPTER pAd, UINT32 Offset, UINT32 *Data, UINT32 Cnt)
{
	struct CMD_UNIT CmdUnit;
	PCHAR Pos, Buf;
	UINT32 VarLen = sizeof(Offset) + 4 * Cnt;
	UINT32 Value, i;

	printk("%s\n", __FUNCTION__);

	os_alloc_mem(pAd, (UCHAR **)&Buf, VarLen);

	Pos = Buf;

	Offset = cpu2le32(Offset);

	NdisMoveMemory(Pos, &Offset, 4);
	Pos += 4;

	for (i = 0; i < Cnt; i++)
	{
		Value = cpu2le32(Data[i]);
		NdisMoveMemory(Pos, &Value, 4);
		Pos += 4;

	};

	NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	CmdUnit.u.ANDES.Type = BURST_WRITE;
	CmdUnit.u.ANDES.CmdPayloadLen = VarLen;
	CmdUnit.u.ANDES.CmdPayload = Buf;
	
	AsicSendCmdToAndes(pAd, CmdUnit);

	os_free_mem(NULL, Buf);

	return NDIS_STATUS_SUCCESS;
}


static VOID CmdDoneHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{


}


static VOID CmdErrorHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{


}


static VOID CmdRetryHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{


}


static VOID PwrRspEventHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{


}


static VOID WowRspEventHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{


}


static VOID CarrierDetectRspEventHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{



}


static VOID DFSDetectRspEventHandler(PRTMP_ADAPTER pAd, UCHAR *Data)
{



}


CMD_RSP_HANDLER CmdRspHandlerTable[] =
{
	CmdDoneHandler,
	CmdErrorHandler,
	CmdRetryHandler,
	PwrRspEventHandler,
	WowRspEventHandler,
	CarrierDetectRspEventHandler,
	DFSDetectRspEventHandler,
};


INT BurstRead(PRTMP_ADAPTER pAd, UINT32 Offset, UINT32 Cnt, UINT32 *Data)
{
	struct CMD_UNIT CmdUnit;
	UINT32 VarLen = sizeof(Offset) + sizeof(Cnt);
	PCHAR Pos, Buf;

	printk("%s\n", __FUNCTION__);

	os_alloc_mem(pAd, (UCHAR **)&Buf, VarLen);

	Pos = Buf;

	Offset = cpu2le32(Offset);
	NdisMoveMemory(Pos, &Offset, 4);
	Pos += 4;
	NdisMoveMemory(Pos, &Cnt, 4);
	Pos += 4;

	NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	CmdUnit.u.ANDES.Type = BURST_READ;
	CmdUnit.u.ANDES.CmdPayloadLen = VarLen;
	CmdUnit.u.ANDES.CmdPayload = Buf;
	CmdUnit.u.ANDES.NeedRsp = TRUE;
	CmdUnit.u.ANDES.NeedWait = TRUE;
	CmdUnit.u.ANDES.Timeout = 0;

	AsicSendCmdToAndes(pAd, CmdUnit);

	NdisMoveMemory(Data, CmdUnit.u.ANDES.RspPayload, CmdUnit.u.ANDES.RspPayloadLen);

	os_free_mem(NULL, Buf);
	
	return NDIS_STATUS_SUCCESS;
}

