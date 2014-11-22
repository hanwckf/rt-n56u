/****************************************************************************

    Module Name:
    rt_os_util.c
 
    Abstract:
	All functions provided from UTIL module are put here (OS independent).
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

/*#include "rt_config.h"
*/
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rtmp_osabl.h"



VOID RtmpDrvRateGet(
	IN	VOID					*pReserved,
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN      UINT8                                   Antenna,
	OUT	UINT32					*pRate)
{
	UINT32 MCS_1NSS = (UINT32) MCS;
	*pRate = 0;
	
	DBGPRINT(RT_DEBUG_TRACE,("<==== %s \nMODE: %x shortGI: %x BW: %x MCS: %x Antenna: %x \n"
		,__FUNCTION__,MODE,ShortGI,BW,MCS,Antenna));
	if((BW >= Rate_BW_MAX) || (ShortGI >= Rate_GI_MAX) || (BW >= Rate_BW_MAX))
	{		
		DBGPRINT(RT_DEBUG_ERROR,("<==== %s MODE: %x shortGI: %x BW: %x MCS: %x Antenna: %x , param error\n",__FUNCTION__,MODE,ShortGI,BW,MCS,Antenna));
		return;
	}
	
#ifdef DOT11_VHT_AC
    if (MODE >= MODE_VHT)
    {
		if(MCS_1NSS > 9)
		{
			Antenna = (MCS / 10)+1;
			MCS_1NSS %= 10;
		}
        *pRate = RalinkRate_VHT_1NSS[BW][ShortGI][MCS_1NSS];
    }
    else
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
	if ((MODE >= MODE_HTMIX) && (MODE < MODE_VHT))
	{
		if(MCS_1NSS > 7)
		{			
			Antenna = (MCS / 8)+1;
			MCS_1NSS %= 8;
		}
		*pRate = RalinkRate_HT_1NSS[BW][ShortGI][MCS_1NSS];
	}
	else 
#endif /* DOT11_N_SUPPORT */
	if (MODE == MODE_OFDM)
		*pRate = RalinkRate_Legacy[MCS_1NSS+4];
	else 
		*pRate = RalinkRate_Legacy[MCS_1NSS];


	
	*pRate *= 500000;
#if defined(DOT11_VHT_AC) || defined(DOT11_N_SUPPORT)
    if (MODE >= MODE_HTMIX)
		*pRate *= Antenna;
#endif /* DOT11_VHT_AC */

	DBGPRINT(RT_DEBUG_TRACE,("=====> %s \nMODE: %x shortGI: %x BW: %x MCS: %x Antenna: %x  Rate = %d\n"
		,__FUNCTION__,MODE,ShortGI,BW,MCS_1NSS,Antenna, (*pRate)/1000000));
	

}


char *rtstrchr(const char * s, int c)
{
    for(; *s != (char) c; ++s)
        if (*s == '\0')
            return NULL;
    return (char *) s;
}


VOID RtmpMeshDown(
	IN VOID *pDrvCtrlBK,
	IN BOOLEAN WaitFlag,
	IN BOOLEAN (*RtmpMeshLinkCheck)(IN VOID *pAd))
{
}



	
BOOLEAN RtmpOsCmdDisplayLenCheck(
	IN UINT32 LenSrc,
	IN UINT32 Offset)
{
	if (LenSrc > (IW_PRIV_SIZE_MASK - Offset))
		return FALSE;

	return TRUE;
}






INT32  RtPrivIoctlSetVal(VOID)
{
    return (INT32)RTPRIV_IOCTL_SET;
}


#ifndef RTMP_ANDES_JAY
#endif /* RTMP_ANDES_JAY */

