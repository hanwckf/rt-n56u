/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_profile.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#include "rt_config.h"


NDIS_STATUS	RTMPReadParametersHook(
	IN	PRTMP_ADAPTER pAd)
{
	PSTRING					src = NULL;
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osFSInfo;
	INT 						retval = NDIS_STATUS_FAILURE;
	PSTRING					buffer;

	buffer = kmalloc(MAX_INI_BUFFER_SIZE, MEM_ALLOC_FLAG);
	if(buffer == NULL)
		return NDIS_STATUS_FAILURE;
	memset(buffer, 0x00, MAX_INI_BUFFER_SIZE);
			
	{	
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			src = AP_PROFILE_PATH;
		}
#endif // CONFIG_AP_SUPPORT //

#ifdef MULTIPLE_CARD_SUPPORT
		src = (PSTRING)pAd->MC_FileName;
#endif // MULTIPLE_CARD_SUPPORT //
	}

	if (src && *src)
	{
		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Open file \"%s\" failed!\n", src));
		}
		else 
		{
			retval =RtmpOSFileRead(srcf, buffer, MAX_INI_BUFFER_SIZE);
			if (retval > 0)
			{
				RTMPSetProfileParameters(pAd, buffer);
				retval = NDIS_STATUS_SUCCESS;
			}
			else
				DBGPRINT(RT_DEBUG_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));

			retval = RtmpOSFileClose(srcf);
			if ( retval != 0)
			{
				retval = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
			}
		}
		
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}
	
	kfree(buffer);
	
	return (retval);

}

