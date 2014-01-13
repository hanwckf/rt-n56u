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
	ap_ioctl.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#include "rt_config.h"
#include "rtmp.h"
#include <linux/wireless.h>

struct iw_priv_args ap_privtab[] = {
{ RTPRIV_IOCTL_SET, 
// 1024 --> 1024 + 512
/* larger size specific to allow 64 ACL MAC addresses to be set up all at once. */
  IW_PRIV_TYPE_CHAR | 1536, 0,
  "set"},  
{ RTPRIV_IOCTL_SHOW,
  IW_PRIV_TYPE_CHAR | 1024, 0,
  "show"},
{ RTPRIV_IOCTL_GSITESURVEY,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 ,
  "get_site_survey"}, 
#ifdef INF_AR9
  { RTPRIV_IOCTL_GET_AR9_SHOW,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 ,
  "ar9_show"}, 
#endif
#ifdef WSC_AP_SUPPORT
  { RTPRIV_IOCTL_SET_WSCOOB,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 ,
  "set_wsc_oob"}, 
#endif
{ RTPRIV_IOCTL_GET_MAC_TABLE,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 ,
  "get_mac_table"}, 
{ RTPRIV_IOCTL_E2P,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "e2p"},
#ifdef DBG
{ RTPRIV_IOCTL_BBP,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "bbp"},
{ RTPRIV_IOCTL_MAC,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "mac"},
#ifdef RTMP_RF_RW_SUPPORT
{ RTPRIV_IOCTL_RF,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "rf"},
#endif // RTMP_RF_RW_SUPPORT //
#endif // DBG //

#ifdef WSC_AP_SUPPORT
{ RTPRIV_IOCTL_WSC_PROFILE,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 ,
  "get_wsc_profile"},
#endif // WSC_AP_SUPPORT //
{ RTPRIV_IOCTL_QUERY_BATABLE,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 ,
  "get_ba_table"},
{ RTPRIV_IOCTL_STATISTICS,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "stat"}
};

#ifdef CONFIG_APSTA_MIXED_SUPPORT
const struct iw_handler_def rt28xx_ap_iw_handler_def =
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
	.private_args	= (struct iw_priv_args *) ap_privtab,
	.num_private_args	= N(ap_privtab),
#if IW_HANDLER_VERSION >= 7
	.get_wireless_stats = rt28xx_get_wireless_stats,
#endif 
};
#endif // CONFIG_APSTA_MIXED_SUPPORT //

INT rt28xx_ap_ioctl(
	IN	struct net_device	*net_dev, 
	IN	OUT	struct ifreq	*rq, 
	IN	INT					cmd)
{
	RTMP_ADAPTER	*pAd = NULL;
    struct iwreq	*wrq = (struct iwreq *) rq;
    INT				Status = NDIS_STATUS_SUCCESS;
    USHORT			subcmd, index;
	POS_COOKIE		pObj;
	INT			apidx=0;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

    //+ patch for SnapGear Request even the interface is down
    if(cmd== SIOCGIWNAME){
	    DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIWNAME\n"));

		strcpy(wrq->u.name, "RTWIFI SoftAP");

	    return Status;
    }//- patch for SnapGear
	
    if((RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MAIN) && !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if (wrq->u.data.pointer == NULL)
		return Status;

	if (cmd == RTPRIV_IOCTL_SET)
	{
		if (strstr(wrq->u.data.pointer, "OpMode") == NULL)
			return -ENETDOWN;
	}
	else
#endif // CONFIG_APSTA_MIXED_SUPPORT //
		return -ENETDOWN;
    }

    // determine this ioctl command is comming from which interface.
    if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MAIN)
    {
		pObj->ioctl_if_type = INT_MAIN;
        pObj->ioctl_if = MAIN_MBSSID;
    }
    else if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MBSSID)
    {
		pObj->ioctl_if_type = INT_MBSSID;
//    	if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MBSSID
		if (strcmp(net_dev->name, pAd->net_dev->name) != 0) // sample
    	{
	        for (index = 1; index < pAd->ApCfg.BssidNum; index++)
	    	{
	    	    if (pAd->ApCfg.MBSSID[index].MSSIDDev == net_dev)
	    	    {
	    	        pObj->ioctl_if = index;
	    	        
	    	        break;
	    	    }
	    	}
	        // Interface not found!
	        if(index == pAd->ApCfg.BssidNum)
	        {
//	        	DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl can not find I/F\n"));
	            return -ENETDOWN;
	        }
	    }
	    else    // ioctl command from I/F(ra0)
	    {
			GET_PAD_FROM_NET_DEV(pAd, net_dev);	
    	    pObj->ioctl_if = MAIN_MBSSID;
//	        DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl can not find I/F and use default: cmd = 0x%08x\n", cmd));
	    }
        MBSS_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
        apidx = pObj->ioctl_if;
    }
#ifdef WDS_SUPPORT
	else if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_WDS)
	{
		pObj->ioctl_if_type = INT_WDS;
		for(index = 0; index < MAX_WDS_ENTRY; index++)
		{
			if (pAd->WdsTab.WdsEntry[index].dev == net_dev)
			{
				pObj->ioctl_if = index;

				break;
			}
			
			if(index == MAX_WDS_ENTRY)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl can not find wds I/F\n"));
				return -ENETDOWN;
			}
		}
	}
#endif // WDS_SUPPORT //
#ifdef APCLI_SUPPORT
	else if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_APCLI)
	{
		pObj->ioctl_if_type = INT_APCLI;
		for (index = 0; index < MAX_APCLI_NUM; index++)
		{
			if (pAd->ApCfg.ApCliTab[index].dev == net_dev)
			{
				pObj->ioctl_if = index;

				break;
			}

			if(index == MAX_APCLI_NUM)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl can not find Apcli I/F\n"));
				return -ENETDOWN;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif // APCLI_SUPPORT //
    else
    {
//    	DBGPRINT(RT_DEBUG_WARN, ("IOCTL is not supported in WDS interface\n"));
    	return -EOPNOTSUPP;
    }
		
	switch(cmd)
	{
#ifdef RALINK_ATE
#ifdef RALINK_28xx_QA
		case RTPRIV_IOCTL_ATE:
			{
				RtmpDoAte(pAd, wrq);
			}
			break;
#endif // RALINK_28xx_QA // 
#endif // RALINK_ATE //
        case SIOCGIFHWADDR:
			DBGPRINT(RT_DEBUG_TRACE, ("IOCTLIOCTLIOCTL::SIOCGIFHWADDR\n"));
            if (pObj->ioctl_if < MAX_MBSSID_NUM)
    			strcpy((PSTRING) wrq->u.name, (PSTRING) pAd->ApCfg.MBSSID[pObj->ioctl_if].Bssid);
			break;
		case SIOCGIWNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIWNAME\n"));
#ifdef RTMP_MAC_PCI
			strcpy(wrq->u.name, "RT2860 SoftAP");
#endif // RTMP_MAC_PCI //
			break;
		case SIOCSIWESSID:  //Set ESSID
			Status = -EOPNOTSUPP;
			break;
		case SIOCGIWESSID:  //Get ESSID
			{
				struct iw_point *erq = &wrq->u.essid;
				PCHAR pSsidStr = NULL;

				erq->flags=1;
              //erq->length = pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen;
              
#ifdef APCLI_SUPPORT
				if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_APCLI)
				{
					if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].Valid == TRUE)
					{
						erq->length = pAd->ApCfg.ApCliTab[pObj->ioctl_if].SsidLen;
						pSsidStr = (PCHAR)&pAd->ApCfg.ApCliTab[pObj->ioctl_if].Ssid;
					}
					else {
						erq->length = 0;
						pSsidStr = NULL;
					}
				}
				else
#endif // APCLI_SUPPORT //
				{
				erq->length = pAd->ApCfg.MBSSID[apidx].SsidLen;
					pSsidStr = (PCHAR)pAd->ApCfg.MBSSID[apidx].Ssid;
				}

				if((erq->pointer) && (pSsidStr != NULL))
				{
					//if(copy_to_user(erq->pointer, pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid, erq->length))
					if(copy_to_user(erq->pointer, pSsidStr, erq->length))
					{
						Status = -EFAULT;
						break;
					}
				}
				DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIWESSID (Len=%d, ssid=%s...)\n", erq->length, (char *)erq->pointer));
			}
			break;
		case SIOCGIWNWID: // get network id 
		case SIOCSIWNWID: // set network id (the cell)
			Status = -EOPNOTSUPP;
			break;
		case SIOCGIWFREQ: // get channel/frequency (Hz)
			wrq->u.freq.m = pAd->CommonCfg.Channel;
			wrq->u.freq.e = 0;
			wrq->u.freq.i = 0;
			break; 
		case SIOCSIWFREQ: //set channel/frequency (Hz)
		case SIOCGIWNICKN:
		case SIOCSIWNICKN: //set node name/nickname
		case SIOCGIWRATE:  //get default bit rate (bps)
            {

				PHTTRANSMIT_SETTING		pHtPhyMode;
#ifdef APCLI_SUPPORT
				if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_APCLI)
					pHtPhyMode = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].HTPhyMode;
				else
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
				if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_WDS)
					pHtPhyMode = &pAd->WdsTab.WdsEntry[pObj->ioctl_if].HTPhyMode;
				else
#endif // WDS_SUPPORT //
					pHtPhyMode = &pAd->ApCfg.MBSSID[pObj->ioctl_if].HTPhyMode;


			RT28XX_IOCTL_MaxRateGet(pAd, pHtPhyMode, (UINT32 *)&wrq->u.bitrate.value);
			wrq->u.bitrate.disabled = 0;
            }
			break;
		case SIOCSIWRATE:  //set default bit rate (bps)
		case SIOCGIWRTS:  // get RTS/CTS threshold (bytes)
		case SIOCSIWRTS:  //set RTS/CTS threshold (bytes)
		case SIOCGIWFRAG:  //get fragmentation thr (bytes)
		case SIOCSIWFRAG:  //set fragmentation thr (bytes)
		case SIOCGIWENCODE:  //get encoding token & mode
		case SIOCSIWENCODE:  //set encoding token & mode
			Status = -EOPNOTSUPP;
			break;
		case SIOCGIWAP:  //get access point MAC addresses
			{
				PCHAR pBssidStr;

				wrq->u.ap_addr.sa_family = ARPHRD_ETHER;
				//memcpy(wrq->u.ap_addr.sa_data, &pAd->ApCfg.MBSSID[pObj->ioctl_if].Bssid, ETH_ALEN);
#ifdef APCLI_SUPPORT
				if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_APCLI)
				{
					if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].Valid == TRUE)
						pBssidStr = (PCHAR)&APCLI_ROOT_BSSID_GET(pAd, pAd->ApCfg.ApCliTab[pObj->ioctl_if].MacTabWCID);
					else
						pBssidStr = NULL;
				}
				else
#endif // APCLI_SUPPORT //
				{
					pBssidStr = (PCHAR) &pAd->ApCfg.MBSSID[pObj->ioctl_if].Bssid[0];
				}

				if (pBssidStr != NULL)
				{
					memcpy(wrq->u.ap_addr.sa_data, pBssidStr, ETH_ALEN);
					DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIWAP(=%02x:%02x:%02x:%02x:%02x:%02x)\n",
						pBssidStr[0],pBssidStr[1],pBssidStr[2], pBssidStr[3],pBssidStr[4],pBssidStr[5]));
				}
				else
				{
					memset(wrq->u.ap_addr.sa_data, 0, ETH_ALEN);
				}
			}
			break;
		case SIOCGIWMODE:  //get operation mode
			wrq->u.mode = IW_MODE_INFRA;   //SoftAP always on INFRA mode.
			break;
		case SIOCSIWAP:  //set access point MAC addresses
		case SIOCSIWMODE:  //set operation mode
		case SIOCGIWSENS:   //get sensitivity (dBm)
		case SIOCSIWSENS:	//set sensitivity (dBm)
		case SIOCGIWPOWER:  //get Power Management settings
		case SIOCSIWPOWER:  //set Power Management settings
		case SIOCGIWTXPOW:  //get transmit power (dBm)
		case SIOCSIWTXPOW:  //set transmit power (dBm)
		//case SIOCGIWRANGE:	//Get range of parameters
		case SIOCGIWRETRY:	//get retry limits and lifetime
		case SIOCSIWRETRY:	//set retry limits and lifetime
			Status = -EOPNOTSUPP;
			break;
		case SIOCGIWRANGE:	//Get range of parameters
		    {
				struct iw_range range;
				UINT32 len;

				memset(&range, 0, sizeof(range));
				range.we_version_compiled = WIRELESS_EXT;
				range.we_version_source = 14;

				/*
					what is correct max? This was not
					documented exactly. At least
					69 has been observed.
				*/
				range.max_qual.qual = 100;
				range.max_qual.level = 0; /* dB */
				range.max_qual.noise = 0; /* dB */
				len = copy_to_user(wrq->u.data.pointer, &range, sizeof(range));
		    }
		    break;
		    
		case RT_PRIV_IOCTL:
			subcmd = wrq->u.data.flags;
			if (subcmd & OID_GET_SET_TOGGLE)
				Status = RTMPAPSetInformation(pAd, wrq,  (INT)subcmd);
			else
				Status = RTMPAPQueryInformation(pAd, wrq, (INT)subcmd);
			break;
		
#ifdef HOSTAPD_SUPPORT
		case SIOCSIWGENIE:

			if(wrq->u.data.length > 20 && MAX_LEN_OF_RSNIE > wrq->u.data.length && wrq->u.data.pointer)
			{
				UCHAR RSNIE_Len[2];
				UCHAR RSNIe[2];
				int offset_next_ie=0;

				DBGPRINT(RT_DEBUG_TRACE,("ioctl SIOCSIWGENIE pAd->IoctlIF=%d\n",pAd->IoctlIF));

				RSNIe[0]=*(UINT8 *)wrq->u.data.pointer;
				if(IE_WPA != RSNIe[0] && IE_RSN != RSNIe[0] )
				{
					DBGPRINT(RT_DEBUG_TRACE,("IE %02x != 0x30/0xdd\n",RSNIe[0]));
					Status = -EINVAL;
					break;
				}
				RSNIE_Len[0]=*((UINT8 *)wrq->u.data.pointer + 1);
				if(wrq->u.data.length != RSNIE_Len[0]+2)
				{
					DBGPRINT(RT_DEBUG_TRACE,("IE use WPA1 WPA2\n"));
					NdisZeroMemory(pAd->ApCfg.MBSSID[pAd->IoctlIF].RSN_IE[1], MAX_LEN_OF_RSNIE);
					RSNIe[1]=*(UINT8 *)wrq->u.data.pointer;
					RSNIE_Len[1]=*((UINT8 *)wrq->u.data.pointer + 1);
					DBGPRINT(RT_DEBUG_TRACE,( "IE1 %02x %02x\n",RSNIe[1],RSNIE_Len[1]));
					pAd->ApCfg.MBSSID[pAd->IoctlIF].RSNIE_Len[1] = RSNIE_Len[1];
					NdisMoveMemory(pAd->ApCfg.MBSSID[pAd->IoctlIF].RSN_IE[1], (UCHAR *)(wrq->u.data.pointer)+2, RSNIE_Len[1]);
					offset_next_ie=RSNIE_Len[1]+2;
				}
				else
					DBGPRINT(RT_DEBUG_TRACE,("IE use only %02x\n",RSNIe[0]));

				NdisZeroMemory(pAd->ApCfg.MBSSID[pAd->IoctlIF].RSN_IE[0], MAX_LEN_OF_RSNIE);
				RSNIe[0]=*(((UINT8 *)wrq->u.data.pointer)+offset_next_ie);
				RSNIE_Len[0]=*(((UINT8 *)wrq->u.data.pointer) + offset_next_ie + 1);
				if(IE_WPA != RSNIe[0] && IE_RSN != RSNIe[0] )
				{
					Status = -EINVAL;
					break;
				}
				pAd->ApCfg.MBSSID[pAd->IoctlIF].RSNIE_Len[0] = RSNIE_Len[0];
				NdisMoveMemory(pAd->ApCfg.MBSSID[pAd->IoctlIF].RSN_IE[0], ((UCHAR *)(wrq->u.data.pointer))+2+offset_next_ie, RSNIE_Len[0]);
				APMakeAllBssBeacon(pAd);
				APUpdateAllBeaconFrame(pAd);

			}
			break;
#endif //HOSTAPD_SUPPORT//

		case SIOCGIWPRIV:
			if (wrq->u.data.pointer) 
			{
				if ( access_ok(VERIFY_WRITE, wrq->u.data.pointer, sizeof(ap_privtab)) != TRUE)
					break;
				if ((sizeof(ap_privtab) / sizeof(ap_privtab[0])) <= wrq->u.data.length)
				{
					wrq->u.data.length = sizeof(ap_privtab) / sizeof(ap_privtab[0]);
					if (copy_to_user(wrq->u.data.pointer, ap_privtab, sizeof(ap_privtab)))
						Status = -EFAULT;
				}
				else
					Status = -E2BIG;
			}
			break;
		case RTPRIV_IOCTL_SET:
			{
				if( access_ok(VERIFY_READ, wrq->u.data.pointer, wrq->u.data.length) == TRUE)
					Status = RTMPAPPrivIoctlSet(pAd, wrq);
			}
			break;
		    
		case RTPRIV_IOCTL_SHOW:
			{
				if( access_ok(VERIFY_READ, wrq->u.data.pointer, wrq->u.data.length) == TRUE)
					Status = RTMPAPPrivIoctlShow(pAd, wrq);
			}
			break;	
			
#ifdef INF_AR9
#ifdef AR9_MAPI_SUPPORT
		case RTPRIV_IOCTL_GET_AR9_SHOW:
			{
				if( access_ok(VERIFY_READ, wrq->u.data.pointer, wrq->u.data.length) == TRUE)
					Status = RTMPAPPrivIoctlAR9Show(pAd, wrq);
			}	
		    break;
#endif //AR9_MAPI_SUPPORT//
#endif//INF_AR9//

#ifdef WSC_AP_SUPPORT
		case RTPRIV_IOCTL_SET_WSCOOB:
			RTMPIoctlSetWSCOOB(pAd);
		    break;
#endif//WSC_AP_SUPPORT//

/* modified by Red@Ralink, 2009/09/30 */
		case RTPRIV_IOCTL_GET_MAC_TABLE:
			RTMPIoctlGetMacTable(pAd,wrq);
		    break;

#ifdef RTMP_RBUS_SUPPORT
		case RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT:
			RTMPIoctlGetMacTableStaInfo(pAd,wrq);
			break;
#endif // RTMP_RBUS_SUPPORT //
/* end of modification */

#ifdef AP_SCAN_SUPPORT
		case RTPRIV_IOCTL_GSITESURVEY:
			RTMPIoctlGetSiteSurvey(pAd,wrq);
			break;
#endif // AP_SCAN_SUPPORT //

		case RTPRIV_IOCTL_STATISTICS:
			RTMPIoctlStatistics(pAd, wrq);
			break;

#ifdef WSC_AP_SUPPORT
		case RTPRIV_IOCTL_WSC_PROFILE:
		    RTMPIoctlWscProfile(pAd, wrq);
		    break;
#endif // WSC_AP_SUPPORT //
#ifdef DOT11_N_SUPPORT
		case RTPRIV_IOCTL_QUERY_BATABLE:
		    RTMPIoctlQueryBaTable(pAd, wrq);
		    break;
#endif // DOT11_N_SUPPORT //
		case RTPRIV_IOCTL_E2P:
			RTMPAPIoctlE2PROM(pAd, wrq);
			break;

#ifdef DBG
		case RTPRIV_IOCTL_BBP:
			RTMPAPIoctlBBP(pAd, wrq);
			break;

		case RTPRIV_IOCTL_MAC:
			RTMPAPIoctlMAC(pAd, wrq);
			break;

#ifdef RTMP_RF_RW_SUPPORT
		case RTPRIV_IOCTL_RF:
			RTMPAPIoctlRF(pAd, wrq);
			break;
#endif // RTMP_RF_RW_SUPPORT //
#endif // DBG //

		default:
//			DBGPRINT(RT_DEBUG_ERROR, ("IOCTL::unknown IOCTL's cmd = 0x%08x\n", cmd));
			Status = -EOPNOTSUPP;
			break;
	}

	return Status;
}
