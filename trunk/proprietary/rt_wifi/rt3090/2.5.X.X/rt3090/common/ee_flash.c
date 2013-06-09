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
	ee_flash.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

/* The definition of the EeBuffer[] located in chips/rtxxxx.c */
extern UCHAR EeBuffer[EEPROM_SIZE];

static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start);


static UCHAR init_flag = 0;
static PUCHAR nv_ee_start = 0;

static USHORT EE_FLASH_ID_LIST[]={
	0x3090,
	0x3091,
	0x3092,
};

#define EE_FLASH_ID_NUM  (sizeof(EE_FLASH_ID_LIST) / sizeof(USHORT))



/*******************************************************************************
  *
  *	Flash-based EEPROM read/write procedures.
  *		some chips use the flash memory instead of internal EEPROM to save the 
  *		calibration info, we need these functions to do the read/write.
  *
  ******************************************************************************/
int rtmp_ee_flash_read(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT Offset,
	OUT USHORT *pValue)
{	
	if (!init_flag)
	{
		*pValue = 0xffff;
	}
	else
	{
		memcpy(pValue, nv_ee_start+ Offset, 2);
	}
	return (*pValue);
}


int rtmp_ee_flash_write(PRTMP_ADAPTER pAd, USHORT Offset, USHORT Data)
{
	if (init_flag)
	{
		memcpy(nv_ee_start+ Offset, &Data, 2);
		//rt_nv_commit();
		//rt_cfg_commit();
		RtmpFlashWrite(EeBuffer, RF_OFFSET, EEPROM_SIZE);
	}
	return 0;
}


VOID rtmp_ee_flash_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{	
	if (!init_flag)
		return;
		
	memcpy(Data, nv_ee_start, EEPROM_SIZE);
}


VOID rtmp_ee_flash_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	if (!init_flag)
		return;
	memcpy(nv_ee_start, Data, EEPROM_SIZE);
	RtmpFlashWrite(EeBuffer, RF_OFFSET, EEPROM_SIZE);
}


static NDIS_STATUS rtmp_ee_flash_reset(PUCHAR start)
{
	PUCHAR				src;
	RTMP_OS_FS_INFO		osFsInfo;
	RTMP_OS_FD			srcf;
	INT 					retval;

	src = EEPROM_DEFAULT_FILE_PATH;

	RtmpOSFSInfoChange(&osFsInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> Error opening file %s\n", src));
			return NDIS_STATUS_FAILURE;
		}
		else 
		{
			// The object must have a read method
			NdisZeroMemory(start, EEPROM_SIZE);
			
			retval = RtmpOSFileRead(srcf, start, EEPROM_SIZE);
			if (retval < 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> Read %s error %d\n", src, -retval));
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> rtmp_ee_flash_reset copy %s to eeprom buffer\n", src));
			}

			retval = RtmpOSFileClose(srcf);
			if (retval)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
			}
		}
	}

	RtmpOSFSInfoChange(&osFsInfo, FALSE);

	return NDIS_STATUS_SUCCESS;
}

#ifdef LINUX
/* 0 -- Show ee buffer */
/* 1 -- force reset to default */
/* 2 -- Change ee settings */
int	Set_EECMD_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg)
{
	USHORT i;
	
	i = simple_strtol(arg, 0, 10);
	switch(i)
	{
		case 0:
			{
				USHORT value, k;
				for (k = 0; k < EEPROM_SIZE; k+=2)
				{
					RT28xx_EEPROM_READ16(pAd, k, value);
					DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", value));
					if (((k+2) % 0x20) == 0)
						DBGPRINT(RT_DEBUG_OFF,("\n"));
				}
				
			}
			break;
		case 1:
			if (pAd->infType == RTMP_DEV_INF_RBUS)
			{
				DBGPRINT(RT_DEBUG_OFF, ("EEPROM reset to default......\n"));
				DBGPRINT(RT_DEBUG_OFF, ("The last byte of MAC address will be re-generated...\n"));
				if (rtmp_ee_flash_reset(nv_ee_start) != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Set_EECMD_Proc: rtmp_ee_flash_reset() failed\n"));
					return FALSE;
				}
			
				// Random number for the last bytes of MAC address
				{
					USHORT  Addr45;

					rtmp_ee_flash_read(pAd, 0x08, &Addr45);
					Addr45 = Addr45 & 0xff;
					Addr45 = Addr45 | (RandomByte(pAd)&0xf8) << 8;
					DBGPRINT(RT_DEBUG_OFF, ("Addr45 = %4x\n", Addr45));
					rtmp_ee_flash_write(pAd, 0x08, Addr45);
				}
			
				if ((rtmp_ee_flash_read(pAd, 0, &i) != 0x2880) && (rtmp_ee_flash_read(pAd, 0, &i) != 0x2860))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Set_EECMD_Proc: invalid eeprom\n"));
					return FALSE;
				}
			}
			break;
		case 2:
			{
				USHORT offset, value = 0;
				PUCHAR p;
				
				p = arg+2;
				offset = simple_strtol(p, 0, 10);
				p+=2;
				while (*p != '\0')
				{
					if (*p >= '0' && *p <= '9')
						value = (value << 4) + (*p - 0x30);
					else if (*p >= 'a' && *p <= 'f')
						value = (value << 4) + (*p - 0x57);
					else if (*p >= 'A' && *p <= 'F')
						value = (value << 4) + (*p - 0x37);
					p++;
				}
				RT28xx_EEPROM_WRITE16(pAd, offset, value);
			}
			break;
		default:
			break;
	}

	return TRUE;
}
#endif // LINUX //


static BOOLEAN  validFlashEepromID(RTMP_ADAPTER *pAd)
{
	USHORT eeFlashId;
	int listIdx;
	
	rtmp_ee_flash_read(pAd, 0, &eeFlashId);
	for(listIdx =0 ; listIdx < EE_FLASH_ID_NUM; listIdx++)
	{
		if (eeFlashId == EE_FLASH_ID_LIST[listIdx])
			return TRUE;
	}
	return FALSE;
}


static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start)
{
	init_flag = 1;
	nv_ee_start = start;

	if (validFlashEepromID(pAd) == FALSE)
	{
		if (rtmp_ee_flash_reset(start) != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rtmp_ee_init(): rtmp_ee_flash_init() failed\n"));
			return NDIS_STATUS_FAILURE;
		}

		// Random number for the last bytes of MAC address
		{
			USHORT  Addr45;
			
			rtmp_ee_flash_read(pAd, 0x08, &Addr45);
			Addr45 = Addr45 & 0xff;
			Addr45 = Addr45 | (RandomByte(pAd)&0xf8) << 8;
			
			rtmp_ee_flash_write(pAd, 0x08, Addr45);
			DBGPRINT(RT_DEBUG_ERROR, ("The EEPROM in Flash is wrong, use default\n"));
		}

		if (validFlashEepromID(pAd) == FALSE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rtmp_ee_flash_init(): invalid eeprom\n"));
			return NDIS_STATUS_FAILURE;
		}
	}
	
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_nv_init(PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("--> rtmp_nv_init\n"));

	RtmpFlashRead(EeBuffer, RF_OFFSET, EEPROM_SIZE);

	return rtmp_ee_flash_init(pAd, EeBuffer);

}
