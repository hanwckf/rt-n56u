/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    sta_ioctl.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Rory Chen   01-03-2003    created
	Rory Chen   02-14-2005    modify to support RT61
*/

#include	"rt_config.h"

#ifdef DOT11Z_TDLS_SUPPORT
#include "tdls_cmm.h"
#endif // DOT11Z_TDLS_SUPPORT //
#ifdef DBG
extern ULONG    RTDebugLevel;
#endif

#define NR_WEP_KEYS 				4
#define WEP_SMALL_KEY_LEN 			(40/8)
#define WEP_LARGE_KEY_LEN 			(104/8)

#define GROUP_KEY_NO                4

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define IWE_STREAM_ADD_EVENT(_A, _B, _C, _D, _E)		iwe_stream_add_event(_A, _B, _C, _D, _E)
#define IWE_STREAM_ADD_POINT(_A, _B, _C, _D, _E)		iwe_stream_add_point(_A, _B, _C, _D, _E)
#define IWE_STREAM_ADD_VALUE(_A, _B, _C, _D, _E, _F)	iwe_stream_add_value(_A, _B, _C, _D, _E, _F)
#else
#define IWE_STREAM_ADD_EVENT(_A, _B, _C, _D, _E)		iwe_stream_add_event(_B, _C, _D, _E)
#define IWE_STREAM_ADD_POINT(_A, _B, _C, _D, _E)		iwe_stream_add_point(_B, _C, _D, _E)
#define IWE_STREAM_ADD_VALUE(_A, _B, _C, _D, _E, _F)	iwe_stream_add_value(_B, _C, _D, _E, _F)
#endif

extern UCHAR    CipherWpa2Template[];

typedef struct GNU_PACKED _RT_VERSION_INFO{
    UCHAR       DriverVersionW;
    UCHAR       DriverVersionX;
    UCHAR       DriverVersionY;
    UCHAR       DriverVersionZ;
    UINT        DriverBuildYear;
    UINT        DriverBuildMonth;
    UINT        DriverBuildDay;
} RT_VERSION_INFO, *PRT_VERSION_INFO;

struct iw_priv_args privtab[] = {
{ RTPRIV_IOCTL_SET, 
  IW_PRIV_TYPE_CHAR | 1024, 0,
  "set"},

{ RTPRIV_IOCTL_SHOW, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  ""},
/* --- sub-ioctls definitions --- */   
#ifdef MAT_SUPPORT
	{ SHOW_IPV4_MAT_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "ipv4_matinfo" },
	{ SHOW_IPV6_MAT_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "ipv6_matinfo" },
	{ SHOW_ETH_CLONE_MAC,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "cloneMAC" },
#endif // MAT_SUPPORT //
    { SHOW_CONN_STATUS,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "connStatus" },
	{ SHOW_DRVIER_VERION,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "driverVer" },
    { SHOW_BA_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "bainfo" },
	{ SHOW_DESC_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "descinfo" },
    { RAIO_OFF,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "radio_off" },
	{ RAIO_ON,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "radio_on" },
#ifdef QOS_DLS_SUPPORT
	{ SHOW_DLS_ENTRY_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "dlsentryinfo" },
#endif // QOS_DLS_SUPPORT //
	{ SHOW_CFG_VALUE,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "show" },
	{ SHOW_ADHOC_ENTRY_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "adhocEntry" },
#ifdef DOT11Z_TDLS_SUPPORT
	{ SHOW_TDLS_ENTRY_INFO,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "tdlsentryinfo" },
#endif // DOT11Z_TDLS_SUPPORT //
/* --- sub-ioctls relations --- */

#ifdef DBG
{ RTPRIV_IOCTL_BBP,
  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  "bbp"},
{ RTPRIV_IOCTL_MAC,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "mac"},  
#ifdef RTMP_RF_RW_SUPPORT
{ RTPRIV_IOCTL_RF,
  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  "rf"},
#endif // RTMP_RF_RW_SUPPORT //
{ RTPRIV_IOCTL_E2P,
  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
  "e2p"},
#endif  /* DBG */

{ RTPRIV_IOCTL_STATISTICS,
  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  "stat"}, 
{ RTPRIV_IOCTL_GSITESURVEY,
  0, IW_PRIV_TYPE_CHAR | 1024,
  "get_site_survey"},

#ifdef WSC_STA_SUPPORT
{ RTPRIV_IOCTL_SET_WSC_PROFILE_U32_ITEM,
  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "" },
{ RTPRIV_IOCTL_SET_WSC_PROFILE_U32_ITEM,
  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 0, 0, "" },
{ RTPRIV_IOCTL_SET_WSC_PROFILE_STRING_ITEM,
  IW_PRIV_TYPE_CHAR | 128, 0, "" },
/* --- sub-ioctls definitions --- */    
	{ WSC_CREDENTIAL_COUNT,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wsc_cred_count" },
	{ WSC_CREDENTIAL_SSID,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_cred_ssid" },
	{ WSC_CREDENTIAL_AUTH_MODE,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_cred_auth" },
	{ WSC_CREDENTIAL_ENCR_TYPE,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_cred_encr" },
	{ WSC_CREDENTIAL_KEY_INDEX,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_cred_keyIdx" },
	{ WSC_CREDENTIAL_KEY,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_cred_key" },
	{ WSC_CREDENTIAL_MAC,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_cred_mac" },	
	{ WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wsc_conn_by_idx" },
	{ WSC_SET_DRIVER_AUTO_CONNECT,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wsc_auto_conn" },
	{ WSC_SET_CONF_MODE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wsc_conf_mode" },
	{ WSC_SET_MODE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wsc_mode" },
	{ WSC_SET_PIN,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_pin" },
	{ WSC_SET_SSID,
	  IW_PRIV_TYPE_CHAR | 128, 0, "wsc_ssid" },
	{ WSC_START,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 0, 0, "wsc_start" },
	{ WSC_STOP,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 0, 0, "wsc_stop" },
    { WSC_GEN_PIN_CODE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 0, 0, "wsc_gen_pincode" },
	{ WSC_AP_BAND,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wsc_ap_band" },
/* --- sub-ioctls relations --- */
#endif // WSC_STA_SUPPORT //
};

extern __s32 ralinkrate[];
extern UINT32 RT_RateSize;




/*
This is required for LinEX2004/kernel2.6.7 to provide iwlist scanning function
*/

int
rt_ioctl_giwname(struct net_device *dev,
		   struct iw_request_info *info,
		   char *name, char *extra)
{
	strncpy(name, "Ralink STA", IFNAMSIZ);
	return 0;
}

int rt_ioctl_siwfreq(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_freq *freq, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	int 	chan = -1;

	GET_PAD_FROM_NET_DEV(pAd, dev);

    //check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;   
    }


	if (freq->e > 1)
		return -EINVAL;

	if((freq->e == 0) && (freq->m <= 1000))
		chan = freq->m;	// Setting by channel number 
	else
		MAP_KHZ_TO_CHANNEL_ID( (freq->m /100) , chan); // Setting by frequency - search the table , like 2.412G, 2.422G, 

    if (ChannelSanity(pAd, chan) == TRUE)
    {
	pAd->CommonCfg.Channel = chan;
	DBGPRINT(RT_DEBUG_ERROR, ("==>rt_ioctl_siwfreq::SIOCSIWFREQ[cmd=0x%x] (Channel=%d)\n", SIOCSIWFREQ, pAd->CommonCfg.Channel));
    }
    else
        return -EINVAL;
    
	return 0;
}


int rt_ioctl_giwfreq(struct net_device *dev,
		   struct iw_request_info *info,
		   struct iw_freq *freq, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	UCHAR ch;
	ULONG	m = 2412000;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;   
	}
		ch = pAd->CommonCfg.Channel;

	DBGPRINT(RT_DEBUG_TRACE,("==>rt_ioctl_giwfreq  %d\n", ch));

	MAP_CHANNEL_ID_TO_KHZ(ch, m);
	freq->m = m * 100;
	freq->e = 1;
	freq->i = 0;
	
	return 0;
}


int rt_ioctl_siwmode(struct net_device *dev,
		   struct iw_request_info *info,
		   __u32 *mode, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
    	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
       	return -ENETDOWN;   
    }

	switch (*mode)
	{
		case IW_MODE_ADHOC:
			Set_NetworkType_Proc(pAd, "Adhoc");
			break;
		case IW_MODE_INFRA:
			Set_NetworkType_Proc(pAd, "Infra");
			break;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,20))
        case IW_MODE_MONITOR:
			Set_NetworkType_Proc(pAd, "Monitor");
			break;
#endif            
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_siwmode::SIOCSIWMODE (unknown %d)\n", *mode));
			return -EINVAL;
	}
	
	// Reset Ralink supplicant to not use, it will be set to start when UI set PMK key
	pAd->StaCfg.WpaState = SS_NOTUSE;

	return 0;
}


int rt_ioctl_giwmode(struct net_device *dev,
		   struct iw_request_info *info,
		   __u32 *mode, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;   
    }

	if (ADHOC_ON(pAd))
		*mode = IW_MODE_ADHOC;
    else if (INFRA_ON(pAd))
		*mode = IW_MODE_INFRA;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,20))
    else if (MONITOR_ON(pAd))
    {
        *mode = IW_MODE_MONITOR;
    }
#endif         
    else
        *mode = IW_MODE_AUTO;

	DBGPRINT(RT_DEBUG_TRACE, ("==>rt_ioctl_giwmode(mode=%d)\n", *mode));
	return 0;
}

int rt_ioctl_siwsens(struct net_device *dev,
		   struct iw_request_info *info,
		   char *name, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
        	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;   
    	}

	return 0;
}

int rt_ioctl_giwsens(struct net_device *dev,
		   struct iw_request_info *info,
		   char *name, char *extra)
{
	return 0;
}

int rt_ioctl_giwrange(struct net_device *dev,
		   struct iw_request_info *info,
		   struct iw_point *data, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	struct iw_range *range = (struct iw_range *) extra;
	u16 val;
	int i;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
    	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;   
	}
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE ,("===>rt_ioctl_giwrange\n"));
	data->length = sizeof(struct iw_range);
	memset(range, 0, sizeof(struct iw_range));

	range->txpower_capa = IW_TXPOW_DBM;

	if (INFRA_ON(pAd)||ADHOC_ON(pAd))
	{
		range->min_pmp = 1 * 1024;
		range->max_pmp = 65535 * 1024;
		range->min_pmt = 1 * 1024;
		range->max_pmt = 1000 * 1024;
		range->pmp_flags = IW_POWER_PERIOD;
		range->pmt_flags = IW_POWER_TIMEOUT;
		range->pm_capa = IW_POWER_PERIOD | IW_POWER_TIMEOUT |
			IW_POWER_UNICAST_R | IW_POWER_ALL_R;
	}

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 14;

	range->retry_capa = IW_RETRY_LIMIT;
	range->retry_flags = IW_RETRY_LIMIT;
	range->min_retry = 0;
	range->max_retry = 255;

	range->num_channels =  pAd->ChannelListNum;

	val = 0;
	for (i = 1; i <= range->num_channels; i++) 
	{
		u32 m = 2412000;
		range->freq[val].i = pAd->ChannelList[i-1].Channel;
		MAP_CHANNEL_ID_TO_KHZ(pAd->ChannelList[i-1].Channel, m);
		range->freq[val].m = m * 100; /* OS_HZ */
		
		range->freq[val].e = 1;
		val++;
		if (val == IW_MAX_FREQUENCIES)
			break;
	}
	range->num_frequency = val;

	range->max_qual.qual = 100; /* what is correct max? This was not
					* documented exactly. At least
					* 69 has been observed. */
	range->max_qual.level = 0; /* dB */
	range->max_qual.noise = 0; /* dB */

	/* What would be suitable values for "average/typical" qual? */
	range->avg_qual.qual = 20;
	range->avg_qual.level = -60;
	range->avg_qual.noise = -95;
	range->sensitivity = 3;

	range->max_encoding_tokens = NR_WEP_KEYS;
	range->num_encoding_sizes = 2;
	range->encoding_size[0] = 5;
	range->encoding_size[1] = 13;

	range->min_rts = 0;
	range->max_rts = 2347;
	range->min_frag = 256;
	range->max_frag = 2346;

#if WIRELESS_EXT > 17
	/* IW_ENC_CAPA_* bit field */
	range->enc_capa = IW_ENC_CAPA_WPA | IW_ENC_CAPA_WPA2 | 
					IW_ENC_CAPA_CIPHER_TKIP | IW_ENC_CAPA_CIPHER_CCMP;
#endif

	return 0;
}

int rt_ioctl_siwap(struct net_device *dev,
		      struct iw_request_info *info,
		      struct sockaddr *ap_addr, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
    NDIS_802_11_MAC_ADDRESS Bssid;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
       	return -ENETDOWN;   
    }

	if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
    {
        RTMP_MLME_RESET_STATE_MACHINE(pAd);
        DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
    }

	if (NdisCmpMemory(ZERO_MAC_ADDR, ap_addr->sa_data, MAC_ADDR_LEN) == 0)
        return 0;

    // tell CNTL state machine to call NdisMSetInformationComplete() after completing
    // this request, because this request is initiated by NDIS.
    pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 
	// Prevent to connect AP again in STAMlmePeriodicExec
	pAd->MlmeAux.AutoReconnectSsidLen= 32;

    memset(Bssid, 0, MAC_ADDR_LEN);
    memcpy(Bssid, ap_addr->sa_data, MAC_ADDR_LEN);
    MlmeEnqueue(pAd, 
                MLME_CNTL_STATE_MACHINE, 
                OID_802_11_BSSID, 
                sizeof(NDIS_802_11_MAC_ADDRESS),
                (VOID *)&Bssid, 0);
    
    DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCSIWAP %02x:%02x:%02x:%02x:%02x:%02x\n",
        Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));

	return 0;
}

int rt_ioctl_giwap(struct net_device *dev,
		      struct iw_request_info *info,
		      struct sockaddr *ap_addr, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;   
    }

	if (INFRA_ON(pAd) || ADHOC_ON(pAd))
	{
		ap_addr->sa_family = ARPHRD_ETHER;
		memcpy(ap_addr->sa_data, &pAd->CommonCfg.Bssid, ETH_ALEN);
	}
#ifdef WPA_SUPPLICANT_SUPPORT    
    // Add for RT2870
    else if (pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
    {
        ap_addr->sa_family = ARPHRD_ETHER;
        memcpy(ap_addr->sa_data, &pAd->MlmeAux.Bssid, ETH_ALEN);
    }
#endif // WPA_SUPPLICANT_SUPPORT //    
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIWAP(=EMPTY)\n"));
		return -ENOTCONN;
	}

	return 0;
}

/*
 * Units are in db above the noise floor. That means the
 * rssi values reported in the tx/rx descriptors in the
 * driver are the SNR expressed in db.
 *
 * If you assume that the noise floor is -95, which is an
 * excellent assumption 99.5 % of the time, then you can
 * derive the absolute signal level (i.e. -95 + rssi). 
 * There are some other slight factors to take into account
 * depending on whether the rssi measurement is from 11b,
 * 11g, or 11a.   These differences are at most 2db and
 * can be documented.
 *
 * NB: various calculations are based on the orinoco/wavelan
 *     drivers for compatibility
 */
static void set_quality(PRTMP_ADAPTER pAd,
                        struct iw_quality *iq, 
                        PBSS_ENTRY pBssEntry)
{
	__u8 ChannelQuality;

	// Normalize Rssi
	if (pBssEntry->Rssi >= -50)
        ChannelQuality = 100;
	else if (pBssEntry->Rssi >= -80) // between -50 ~ -80dbm
		ChannelQuality = (__u8)(24 + ((pBssEntry->Rssi + 80) * 26)/10);
	else if (pBssEntry->Rssi >= -90)   // between -80 ~ -90dbm
        ChannelQuality = (__u8)((pBssEntry->Rssi + 90) * 26)/10;   
	else
		ChannelQuality = 0;
        
    iq->qual = (__u8)ChannelQuality;
    
    iq->level = (__u8)(pBssEntry->Rssi);

    if (pBssEntry->Rssi >= -70)
		iq->noise = -92;
	else
		iq->noise = pBssEntry->Rssi - pBssEntry->MinSNR;		

    iq->updated = pAd->iw_stats.qual.updated;
}

int rt_ioctl_iwaplist(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
 	PRTMP_ADAPTER pAd = NULL;	

	struct sockaddr addr[IW_MAX_AP];
	struct iw_quality qual[IW_MAX_AP];
	int i;

	GET_PAD_FROM_NET_DEV(pAd, dev);

   	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		data->length = 0;
		return 0;
        //return -ENETDOWN;   
	}

	for (i = 0; i <IW_MAX_AP ; i++)
	{
		if (i >=  pAd->ScanTab.BssNr)
			break;
		addr[i].sa_family = ARPHRD_ETHER;
			memcpy(addr[i].sa_data, &pAd->ScanTab.BssEntry[i].Bssid, MAC_ADDR_LEN);
		set_quality(pAd, &qual[i], &pAd->ScanTab.BssEntry[i]);	
	}
	data->length = i;
	memcpy(extra, &addr, i*sizeof(addr[0]));
	data->flags = 1;		/* signal quality present (sort of) */
	memcpy(extra + i*sizeof(addr[0]), &qual, i*sizeof(qual[i]));

	return 0;
}

#if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT)
int rt_ioctl_siwscan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	int Status = NDIS_STATUS_SUCCESS;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;   
	}

	if (MONITOR_ON(pAd))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
        return -EINVAL;
    }


#ifdef WPA_SUPPLICANT_SUPPORT
	if ((pAd->StaCfg.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
	{
		pAd->StaCfg.WpaSupplicantScanCount++;
	}
#endif // WPA_SUPPLICANT_SUPPORT //

    pAd->StaCfg.bScanReqIsFromWebUI = TRUE;
	do{

#ifdef WPA_SUPPLICANT_SUPPORT
		if (((pAd->StaCfg.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE) &&
			(pAd->StaCfg.WpaSupplicantScanCount > 3))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!! WpaSupplicantScanCount > 3\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		}
#endif // WPA_SUPPLICANT_SUPPORT //

		if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) &&
			((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || 
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) ||
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&	
			(pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		StaSiteSurvey(pAd, NULL, SCAN_ACTIVE);
	}while(0);
	return NDIS_STATUS_SUCCESS;
}

int rt_ioctl_giwscan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	int i=0;
	PSTRING current_ev = extra, previous_ev = extra;
	PSTRING end_buf;
	PSTRING current_val;
	STRING custom[MAX_CUSTOM_LEN] = {0};
#ifndef IWEVGENIE
	unsigned char idx;
#endif // IWEVGENIE //
	struct iw_event iwe;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    {
		/*
		 * Still scanning, indicate the caller should try again.
		 */
		pAd->StaCfg.bScanReqIsFromWebUI = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_giwscan:: Still scanning\n"));
		return -EAGAIN;
	}

	if (pAd->StaCfg.bImprovedScan)
	{
		/*
		 * Fast scanning doesn't complete yet.
		 */
		pAd->StaCfg.bScanReqIsFromWebUI = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_giwscan:: Still scanning\n"));
		return -EAGAIN;
	}


#ifdef WPA_SUPPLICANT_SUPPORT
	if ((pAd->StaCfg.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
	{
		pAd->StaCfg.WpaSupplicantScanCount = 0;
	}
#endif // WPA_SUPPLICANT_SUPPORT //

	if (pAd->ScanTab.BssNr == 0)
	{
		data->length = 0;
		return 0;
	}
	
#if WIRELESS_EXT >= 17
    if (data->length > 0)
        end_buf = extra + data->length;
    else
        end_buf = extra + IW_SCAN_MAX_DATA;
#else
    end_buf = extra + IW_SCAN_MAX_DATA;
#endif

	for (i = 0; i < pAd->ScanTab.BssNr; i++) 
	{
		if (current_ev >= end_buf)
        {
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
        }
		
		//MAC address
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
				memcpy(iwe.u.ap_addr.sa_data, &pAd->ScanTab.BssEntry[i].Bssid, ETH_ALEN);

        previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev,end_buf, &iwe, IW_EV_ADDR_LEN);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif

		/* 
		Protocol:
			it will show scanned AP's WirelessMode .
			it might be
					802.11a
					802.11a/n
					802.11g/n
					802.11b/g/n
					802.11g
					802.11b/g
		*/
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWNAME;


	{
		PBSS_ENTRY pBssEntry=&pAd->ScanTab.BssEntry[i];
		BOOLEAN isGonly=FALSE;
		int rateCnt=0;

		if (pBssEntry->Channel>14)
		{
			if (pBssEntry->HtCapabilityLen!=0)
				strcpy(iwe.u.name,"802.11a/n");
			else	
				strcpy(iwe.u.name,"802.11a");
		}
		else
		{
			/*
				if one of non B mode rate is set supported rate . it mean G only. 
			*/
			for (rateCnt=0;rateCnt<pBssEntry->SupRateLen;rateCnt++)
			{									
				/*
					6Mbps(140) 9Mbps(146) and >=12Mbps(152) are supported rate , it mean G only. 
				*/
				if (pBssEntry->SupRate[rateCnt]==140 || pBssEntry->SupRate[rateCnt]==146 || pBssEntry->SupRate[rateCnt]>=152)
					isGonly=TRUE;
			}

			for (rateCnt=0;rateCnt<pBssEntry->ExtRateLen;rateCnt++)
			{
				if (pBssEntry->ExtRate[rateCnt]==140 || pBssEntry->ExtRate[rateCnt]==146 || pBssEntry->ExtRate[rateCnt]>=152)
					isGonly=TRUE;
			}		
			
			
			if (pBssEntry->HtCapabilityLen!=0)
			{
				if (isGonly==TRUE)
					strcpy(iwe.u.name,"802.11g/n");
				else
					strcpy(iwe.u.name,"802.11b/g/n");
			}
			else
			{
				if (isGonly==TRUE)
					strcpy(iwe.u.name,"802.11g");
				else
				{
					if (pBssEntry->SupRateLen==4 && pBssEntry->ExtRateLen==0)
						strcpy(iwe.u.name,"802.11b");
					else
						strcpy(iwe.u.name,"802.11b/g");		
				}
			}
		}
	}

		previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev,end_buf, &iwe, IW_EV_ADDR_LEN);
		if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
	   		return -E2BIG;
#else
			break;
#endif

		//ESSID	
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.length = pAd->ScanTab.BssEntry[i].SsidLen;
		iwe.u.data.flags = 1;
 
        previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_POINT(info, current_ev,end_buf, &iwe, (PSTRING) pAd->ScanTab.BssEntry[i].Ssid);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
		
		//Network Type 
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWMODE;
		if (pAd->ScanTab.BssEntry[i].BssType == Ndis802_11IBSS)
		{
			iwe.u.mode = IW_MODE_ADHOC;
		}
		else if (pAd->ScanTab.BssEntry[i].BssType == Ndis802_11Infrastructure)
		{
			iwe.u.mode = IW_MODE_INFRA;
		}
		else
		{
			iwe.u.mode = IW_MODE_AUTO;
		}
		iwe.len = IW_EV_UINT_LEN;

        previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev, end_buf, &iwe,  IW_EV_UINT_LEN);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif

		//Channel and Frequency
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWFREQ;
		{
			UCHAR ch = pAd->ScanTab.BssEntry[i].Channel;
			ULONG	m = 0;
			MAP_CHANNEL_ID_TO_KHZ(ch, m);
			iwe.u.freq.m = m * 100;
			iwe.u.freq.e = 1;
		iwe.u.freq.i = 0;
		previous_ev = current_ev;
		current_ev = IWE_STREAM_ADD_EVENT(info, current_ev,end_buf, &iwe, IW_EV_FREQ_LEN);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif
		}	    

        //Add quality statistics
        //================================
        memset(&iwe, 0, sizeof(iwe));
    	iwe.cmd = IWEVQUAL;
    	iwe.u.qual.level = 0;
    	iwe.u.qual.noise = 0;
		set_quality(pAd, &iwe.u.qual, &pAd->ScanTab.BssEntry[i]);
    	current_ev = IWE_STREAM_ADD_EVENT(info, current_ev, end_buf, &iwe, IW_EV_QUAL_LEN);
	if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif

		//Encyption key
		//================================
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWENCODE;
		if (CAP_IS_PRIVACY_ON (pAd->ScanTab.BssEntry[i].CapabilityInfo ))
			iwe.u.data.flags =IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;

        previous_ev = current_ev;		
        current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf,&iwe, (char *)pAd->SharedKey[BSS0][(iwe.u.data.flags & IW_ENCODE_INDEX)-1].Key);
        if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
            return -E2BIG;
#else
			break;
#endif

		//Bit Rate
		//================================
		if (pAd->ScanTab.BssEntry[i].SupRateLen)
        {
            UCHAR tmpRate = pAd->ScanTab.BssEntry[i].SupRate[pAd->ScanTab.BssEntry[i].SupRateLen-1];
			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = SIOCGIWRATE;
    		current_val = current_ev + IW_EV_LCP_LEN;            
            if (tmpRate == 0x82)
                iwe.u.bitrate.value =  1 * 1000000;
            else if (tmpRate == 0x84)
                iwe.u.bitrate.value =  2 * 1000000;
            else if (tmpRate == 0x8B)
                iwe.u.bitrate.value =  5.5 * 1000000;
            else if (tmpRate == 0x96)
                iwe.u.bitrate.value =  11 * 1000000;
            else
    		    iwe.u.bitrate.value =  (tmpRate/2) * 1000000;
            
			if (pAd->ScanTab.BssEntry[i].ExtRateLen)
			{
				UCHAR tmpSupRate =(pAd->ScanTab.BssEntry[i].SupRate[pAd->ScanTab.BssEntry[i].SupRateLen-1]& 0x7f);
				UCHAR tmpExtRate =(pAd->ScanTab.BssEntry[i].ExtRate[pAd->ScanTab.BssEntry[i].ExtRateLen-1]& 0x7f);
				iwe.u.bitrate.value = (tmpSupRate > tmpExtRate) ? (tmpSupRate)*500000 : (tmpExtRate)*500000;	
			}

			if (tmpRate == 0x6c && pAd->ScanTab.BssEntry[i].HtCapabilityLen > 0)
			{
				int rate_count = RT_RateSize/sizeof(__s32);
				HT_CAP_INFO capInfo = pAd->ScanTab.BssEntry[i].HtCapability.HtCapInfo;
				int shortGI = capInfo.ChannelWidth ? capInfo.ShortGIfor40 : capInfo.ShortGIfor20;
				int maxMCS = pAd->ScanTab.BssEntry[i].HtCapability.MCSSet[1] ?  15 : 7;
				int rate_index = 12 + ((UCHAR)capInfo.ChannelWidth * 24) + ((UCHAR)shortGI *48) + ((UCHAR)maxMCS);
				if (rate_index < 0)
					rate_index = 0;
				if (rate_index > rate_count)
					rate_index = rate_count;
				iwe.u.bitrate.value	=  ralinkrate[rate_index] * 500000;
			}
            
			iwe.u.bitrate.disabled = 0;
			current_val = IWE_STREAM_ADD_VALUE(info, current_ev,
				current_val, end_buf, &iwe,
    			IW_EV_PARAM_LEN);            

        	if((current_val-current_ev)>IW_EV_LCP_LEN)
            	current_ev = current_val;
        	else
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
        }
            
#ifdef IWEVGENIE
        //WPA IE
		if (pAd->ScanTab.BssEntry[i].WpaIE.IELen > 0)
        {
			memset(&iwe, 0, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
			memcpy(custom, &(pAd->ScanTab.BssEntry[i].WpaIE.IE[0]), 
						   pAd->ScanTab.BssEntry[i].WpaIE.IELen);
			iwe.cmd = IWEVGENIE;
			iwe.u.data.length = pAd->ScanTab.BssEntry[i].WpaIE.IELen;
			current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe, custom);
			if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
		}
            
		//WPA2 IE
        if (pAd->ScanTab.BssEntry[i].RsnIE.IELen > 0)
        {
        	memset(&iwe, 0, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
			memcpy(custom, &(pAd->ScanTab.BssEntry[i].RsnIE.IE[0]), 
						   pAd->ScanTab.BssEntry[i].RsnIE.IELen);
			iwe.cmd = IWEVGENIE;
			iwe.u.data.length = pAd->ScanTab.BssEntry[i].RsnIE.IELen;
			current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe, custom);
			if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
        }

#ifdef WSC_INCLUDED
		//WPS IE
		if (pAd->ScanTab.BssEntry[i].WpsIE.IELen > 0)
        {
        	memset(&iwe, 0, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
			memcpy(custom, &(pAd->ScanTab.BssEntry[i].WpsIE.IE[0]), 
						   pAd->ScanTab.BssEntry[i].WpsIE.IELen);
			iwe.cmd = IWEVGENIE;
			iwe.u.data.length = pAd->ScanTab.BssEntry[i].WpsIE.IELen;
			current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe, custom);
			if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
        }
#endif // WSC_INCLUDED //

#else
        //WPA IE
		//================================
        if (pAd->ScanTab.BssEntry[i].WpaIE.IELen > 0)
        {
    		NdisZeroMemory(&iwe, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
    		iwe.cmd = IWEVCUSTOM;
            iwe.u.data.length = (pAd->ScanTab.BssEntry[i].WpaIE.IELen * 2) + 7;
            NdisMoveMemory(custom, "wpa_ie=", 7);
            for (idx = 0; idx < pAd->ScanTab.BssEntry[i].WpaIE.IELen; idx++)
                sprintf(custom, "%s%02x", custom, pAd->ScanTab.BssEntry[i].WpaIE.IE[idx]);
            previous_ev = current_ev;
    		current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe,  custom);
            if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
        }

        //WPA2 IE
        if (pAd->ScanTab.BssEntry[i].RsnIE.IELen > 0)
        {
    		NdisZeroMemory(&iwe, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
    		iwe.cmd = IWEVCUSTOM;
            iwe.u.data.length = (pAd->ScanTab.BssEntry[i].RsnIE.IELen * 2) + 7;
            NdisMoveMemory(custom, "rsn_ie=", 7);
			for (idx = 0; idx < pAd->ScanTab.BssEntry[i].RsnIE.IELen; idx++)
                sprintf(custom, "%s%02x", custom, pAd->ScanTab.BssEntry[i].RsnIE.IE[idx]);
            previous_ev = current_ev;
    		current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe,  custom);
            if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
        }

#ifdef WSC_INCLUDED
		//WPS IE
		if (pAd->ScanTab.BssEntry[i].WpsIE.IELen > 0)
        {
    		NdisZeroMemory(&iwe, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
    		iwe.cmd = IWEVCUSTOM;
            iwe.u.data.length = (pAd->ScanTab.BssEntry[i].WpsIE.IELen * 2) + 7;
            NdisMoveMemory(custom, "wps_ie=", 7);
			for (idx = 0; idx < pAd->ScanTab.BssEntry[i].WpsIE.IELen; idx++)
                sprintf(custom, "%s%02x", custom, pAd->ScanTab.BssEntry[i].WpsIE.IE[idx]);
            previous_ev = current_ev;
    		current_ev = IWE_STREAM_ADD_POINT(info, current_ev, end_buf, &iwe,  custom);
            if (current_ev == previous_ev)
#if WIRELESS_EXT >= 17
                return -E2BIG;
#else
			    break;
#endif
        }
#endif // WSC_INCLUDED //

#endif // IWEVGENIE //
	}

	data->length = current_ev - extra;
    pAd->StaCfg.bScanReqIsFromWebUI = FALSE;
	DBGPRINT(RT_DEBUG_ERROR ,("===>rt_ioctl_giwscan. %d(%d) BSS returned, data->length = %d\n",i , pAd->ScanTab.BssNr, data->length));
	return 0;
}
#endif

int rt_ioctl_siwessid(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *essid)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
       	return -ENETDOWN;   
    }

	if (data->flags)
	{
		PSTRING	pSsidString = NULL;

		// Includes null character.
		if (data->length > (IW_ESSID_MAX_SIZE + 1)) 
			return -E2BIG;

		pSsidString = kmalloc(MAX_LEN_OF_SSID+1, MEM_ALLOC_FLAG);
		if (pSsidString)
        {
			NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID+1);
			NdisMoveMemory(pSsidString, essid, data->length);
			if (Set_SSID_Proc(pAd, pSsidString) == FALSE)
				return -EINVAL;
			kfree(pSsidString);
		}
		else
			return -ENOMEM;
		}
	else
    {
		// ANY ssid
		if (Set_SSID_Proc(pAd, "") == FALSE)
			return -EINVAL;
    }
	return 0;
}

int rt_ioctl_giwessid(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *essid)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

	data->flags = 1;		
    if (MONITOR_ON(pAd))
    {
        data->length  = 0;
        return 0;
    }

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		DBGPRINT(RT_DEBUG_TRACE ,("MediaState is connected\n"));
		data->length = pAd->CommonCfg.SsidLen;
		memcpy(essid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
	}
	else
	{//the ANY ssid was specified
		data->length  = 0;
		DBGPRINT(RT_DEBUG_TRACE ,("MediaState is not connected, ess\n"));
	}

	return 0;

}

int rt_ioctl_siwnickn(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *nickname)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

    //check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE ,("INFO::Network is down!\n"));
        return -ENETDOWN;   
    }

	if (data->length > IW_ESSID_MAX_SIZE)
		return -EINVAL;

	memset(pAd->nickname, 0, IW_ESSID_MAX_SIZE + 1);
	memcpy(pAd->nickname, nickname, data->length);


	return 0;
}

int rt_ioctl_giwnickn(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *nickname)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		data->length = 0;
        return -ENETDOWN;
	}

	if (data->length > strlen((PSTRING) pAd->nickname) + 1)
		data->length = strlen((PSTRING) pAd->nickname) + 1;
	if (data->length > 0) {
		memcpy(nickname, pAd->nickname, data->length-1);
		nickname[data->length-1] = '\0';
	}
	return 0;
}

int rt_ioctl_siwrts(struct net_device *dev,
		       struct iw_request_info *info,
		       struct iw_param *rts, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	u16 val;

	GET_PAD_FROM_NET_DEV(pAd, dev);

    //check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;   
    }
	
	if (rts->disabled)
		val = MAX_RTS_THRESHOLD;
	else if (rts->value < 0 || rts->value > MAX_RTS_THRESHOLD)
		return -EINVAL;
	else if (rts->value == 0)
	    val = MAX_RTS_THRESHOLD;
	else
		val = rts->value;
	
	if (val != pAd->CommonCfg.RtsThreshold)
		pAd->CommonCfg.RtsThreshold = val;

	return 0;
}

int rt_ioctl_giwrts(struct net_device *dev,
		       struct iw_request_info *info,
		       struct iw_param *rts, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;   
    	}

	rts->value = pAd->CommonCfg.RtsThreshold;
	rts->disabled = (rts->value == MAX_RTS_THRESHOLD);
	rts->fixed = 1;

	return 0;
}

int rt_ioctl_siwfrag(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *frag, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	u16 val;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;   
    	}

	if (frag->disabled)
		val = MAX_FRAG_THRESHOLD;
	else if (frag->value >= MIN_FRAG_THRESHOLD || frag->value <= MAX_FRAG_THRESHOLD)
        val = __cpu_to_le16(frag->value & ~0x1); /* even numbers only */
	else if (frag->value == 0)
	    val = MAX_FRAG_THRESHOLD;
	else
		return -EINVAL;

	pAd->CommonCfg.FragmentThreshold = val;
	return 0;
}

int rt_ioctl_giwfrag(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *frag, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;   
    	}
		
	frag->value = pAd->CommonCfg.FragmentThreshold;
	frag->disabled = (frag->value == MAX_FRAG_THRESHOLD);
	frag->fixed = 1;

	return 0;
}

#define MAX_WEP_KEY_SIZE 13
#define MIN_WEP_KEY_SIZE 5
int rt_ioctl_siwencode(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *erq, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;   
    	}

	if ((erq->length == 0) &&
        (erq->flags & IW_ENCODE_DISABLED))
	{
		pAd->StaCfg.PairCipher = Ndis802_11WEPDisabled;
		pAd->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
		pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
        pAd->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
        goto done;
	}
	else if (erq->flags & IW_ENCODE_RESTRICTED || erq->flags & IW_ENCODE_OPEN)
	{
	    //pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
		STA_PORT_SECURED(pAd);
		pAd->StaCfg.PairCipher = Ndis802_11WEPEnabled;
		pAd->StaCfg.GroupCipher = Ndis802_11WEPEnabled;
		pAd->StaCfg.WepStatus = Ndis802_11WEPEnabled;
		if (erq->flags & IW_ENCODE_RESTRICTED)
			pAd->StaCfg.AuthMode = Ndis802_11AuthModeShared;
    	else
			pAd->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
	}
    
    if (erq->length > 0) 
	{
		int keyIdx = (erq->flags & IW_ENCODE_INDEX) - 1;
		/* Check the size of the key */
		if (erq->length > MAX_WEP_KEY_SIZE) 
		{
			return -EINVAL;
		}
		/* Check key index */
		if ((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
        {
            DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::Wrong keyIdx=%d! Using default key instead (%d)\n", 
                                        keyIdx, pAd->StaCfg.DefaultKeyId));
            
            //Using default key
			keyIdx = pAd->StaCfg.DefaultKeyId;   
        }
		else
			pAd->StaCfg.DefaultKeyId = keyIdx;

        NdisZeroMemory(pAd->SharedKey[BSS0][keyIdx].Key,  16);
		
		if (erq->length == MAX_WEP_KEY_SIZE)
        {      
			pAd->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
            pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
		}
		else if (erq->length == MIN_WEP_KEY_SIZE)
        {      
            pAd->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
            pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
		}
		else
			/* Disable the key */
			pAd->SharedKey[BSS0][keyIdx].KeyLen = 0;

		/* Check if the key is not marked as invalid */
		if(!(erq->flags & IW_ENCODE_NOKEY)) 
		{
			/* Copy the key in the driver */
			NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, extra, erq->length);
        }
	} 
    else 
			{
		/* Do we want to just set the transmit key index ? */
		int index = (erq->flags & IW_ENCODE_INDEX) - 1;
		if ((index >= 0) && (index < 4)) 
        {      
			pAd->StaCfg.DefaultKeyId = index;
            }
        else
			/* Don't complain if only change the mode */
		if (!(erq->flags & IW_ENCODE_MODE))
		{
				return -EINVAL;
		}
	}
		
done:
    DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::erq->flags=%x\n",erq->flags));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::AuthMode=%x\n",pAd->StaCfg.AuthMode));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::DefaultKeyId=%x, KeyLen = %d\n",pAd->StaCfg.DefaultKeyId , pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::WepStatus=%x\n",pAd->StaCfg.WepStatus));
	return 0;
}

int
rt_ioctl_giwencode(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *erq, char *key)
{
	int kid;
	PRTMP_ADAPTER pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;   
	}
		
	kid = erq->flags & IW_ENCODE_INDEX;
	DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_giwencode %d\n", erq->flags & IW_ENCODE_INDEX));

	if (pAd->StaCfg.WepStatus == Ndis802_11WEPDisabled)
	{
		erq->length = 0;
		erq->flags = IW_ENCODE_DISABLED;
	} 
	else if ((kid > 0) && (kid <=4))
	{
		// copy wep key
		erq->flags = kid ;			/* NB: base 1 */
		if (erq->length > pAd->SharedKey[BSS0][kid-1].KeyLen)
			erq->length = pAd->SharedKey[BSS0][kid-1].KeyLen;
		memcpy(key, pAd->SharedKey[BSS0][kid-1].Key, erq->length);
		//if ((kid == pAd->PortCfg.DefaultKeyId))
		//erq->flags |= IW_ENCODE_ENABLED;	/* XXX */
		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared)
			erq->flags |= IW_ENCODE_RESTRICTED;		/* XXX */
		else
			erq->flags |= IW_ENCODE_OPEN;		/* XXX */
		
	}
	else if (kid == 0)
	{
		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared)
			erq->flags |= IW_ENCODE_RESTRICTED;		/* XXX */
		else
			erq->flags |= IW_ENCODE_OPEN;		/* XXX */
		erq->length = pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen;
		memcpy(key, pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key, erq->length);
		// copy default key ID
		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared)
			erq->flags |= IW_ENCODE_RESTRICTED;		/* XXX */
		else
			erq->flags |= IW_ENCODE_OPEN;		/* XXX */
		erq->flags = pAd->StaCfg.DefaultKeyId + 1;			/* NB: base 1 */
		erq->flags |= IW_ENCODE_ENABLED;	/* XXX */
	}
		
	return 0;

}

int rt_ioctl_setparam(struct net_device *dev, struct iw_request_info *info,
			 void *w, char *extra)
{
	PRTMP_ADAPTER pAd;
	POS_COOKIE pObj;
	PSTRING this_char = extra;
	PSTRING value;
	int  Status=0;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	{
		pObj->ioctl_if_type = INT_MAIN;
        pObj->ioctl_if = MAIN_MBSSID;
	}
	
	//check if the interface is down
    	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
			return -ENETDOWN;
    	}

	if (!*this_char)
		return -EINVAL;
	                                                                                                                            
	if ((value = rtstrchr(this_char, '=')) != NULL)                                                                             
	    *value++ = 0;
	                                                                                                                            
	if (!value && (strcmp(this_char, "SiteSurvey") != 0))                                                                                                      
	    return -EINVAL;                                                                                                                  
	else
		goto SET_PROC;

	// reject setting nothing besides ANY ssid(ssidLen=0)
    if (!*value && (strcmp(this_char, "SSID") != 0))
        return -EINVAL; 

SET_PROC:
	Status = RTMPSTAPrivIoctlSet(pAd, this_char, value);
		
    return Status;
}


#ifdef WSC_STA_SUPPORT
BOOLEAN CheckWscAuthType(
    IN USHORT authType)
{
	switch(authType)
	{
		case WSC_AUTHTYPE_OPEN:
			break;
		case WSC_AUTHTYPE_WPAPSK:
			break;
		case WSC_AUTHTYPE_SHARED:
			break;
		case WSC_AUTHTYPE_WPA:
			break;
		case WSC_AUTHTYPE_WPA2:
			break;
		case WSC_AUTHTYPE_WPA2PSK:
			break;
        default:
            return FALSE;
	}

    return TRUE;
}

USHORT CheckWscEncryType(
    IN USHORT encryType)
{
	switch(encryType)
	{
		case WSC_ENCRTYPE_NONE:
			break;
		case WSC_ENCRTYPE_WEP:
			break;        
		case WSC_ENCRTYPE_TKIP:
			break;
		case WSC_ENCRTYPE_AES:
			break;
        default:
            return FALSE;
	}

    return TRUE;
}

static int
rt_private_set_wsc_u32_item(struct net_device *dev, struct iw_request_info *info,
			 u32 *uwrq, char *extra)
{
    PRTMP_ADAPTER pAd = NULL;
    int  Status=0;
    u32 subcmd = *uwrq;
    PWSC_PROFILE    pWscProfile = NULL;
   	u32 value = 0;

	GET_PAD_FROM_NET_DEV(pAd, dev);
	pWscProfile = &pAd->StaCfg.WscControl.WscProfile;

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

    switch(subcmd)
    {
        case WSC_CREDENTIAL_COUNT:
            value = *(uwrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_COUNT, value = %d\n", __FUNCTION__, value));
            if (value >= 0 && value <= 8)
            {
                pWscProfile->ProfileCnt = value;
            }
            else
                Status = -EINVAL;
            break;        
        case WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX:
            value = *(uwrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX, value = %d\n", __FUNCTION__, value));
            if ((value >= 0 && value <= 7) &&
                (value < pWscProfile->ProfileCnt))
            {
                WscWriteConfToPortCfg(pAd, &pAd->StaCfg.WscControl, &pAd->StaCfg.WscControl.WscProfile.Profile[value], TRUE);
                pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
                LinkDown(pAd, TRUE);
            }
            else
                Status = -EINVAL;
            break;
        case WSC_SET_DRIVER_AUTO_CONNECT:
            value = *(uwrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_DRIVER_AUTO_CONNECT, value = %d\n", __FUNCTION__, value));
            if ((value == 0x00) || 
				(value == 0x01) || 
				(value == 0x02))
            {
                pAd->StaCfg.WscControl.WscDriverAutoConnect = value;
            }
            else
                Status = -EINVAL;
            break;
        case WSC_SET_CONF_MODE:
            value = *(uwrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_CONF_MODE, value = %d\n", __FUNCTION__, value));
		if (value == 2)
			value = 4;
            switch(value)
            {
                case WSC_DISABLE:
                    Set_WscConfMode_Proc(pAd, "0");
                    break;
                case WSC_ENROLLEE:
                    Set_WscConfMode_Proc(pAd, "1");
                    break;
                case WSC_REGISTRAR:
                    Set_WscConfMode_Proc(pAd, "2");
                    break;
                default:
                    Status = -EINVAL;
                    break;
            }
            break;
        case WSC_SET_MODE:
            value = *(uwrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_MODE, value = %d\n", __FUNCTION__, value));
            switch(value)
            {
                case WSC_PIN_MODE:
                    Set_WscMode_Proc(pAd, "1");
                    break;
                case WSC_PBC_MODE:
                    Set_WscMode_Proc(pAd, "2");
                    break;
                default:
                    Status = -EINVAL;
                    break;
            }
            break;
        case WSC_START:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_START\n", __FUNCTION__));
            Set_WscGetConf_Proc(pAd, "1");
            break;
        case WSC_STOP:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_STOP\n", __FUNCTION__));
						
            // Disassociate the link if WPS is working.
        	if ( INFRA_ON(pAd) && 
                 (pAd->StaCfg.WscControl.bWscTrigger == TRUE) && 
                 (pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) )
        	{
        		MLME_DISASSOC_REQ_STRUCT	DisReq;
        										
        		// Set to immediately send the media disconnect event
        		pAd->MlmeAux.CurrReqIsFromNdis = TRUE;

        		DBGPRINT(RT_DEBUG_TRACE, ("disassociate with current AP \n"));
        		DisassocParmFill(pAd, &DisReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
        		MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
        					sizeof(MLME_DISASSOC_REQ_STRUCT), &DisReq, 0);

        		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
				RTMP_MLME_HANDLER(pAd);
        	}

        	// Turn off WSC state matchine
        	WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
        			FALSE,
#endif // CONFIG_AP_SUPPORT //
        			&pAd->StaCfg.WscControl);
            pAd->StaCfg.WscControl.WscConfMode = WSC_DISABLE;
			BssTableDeleteEntry(&pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.Bssid, pAd->MlmeAux.Channel);
            break;
        case WSC_GEN_PIN_CODE:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_GEN_PIN_CODE\n", __FUNCTION__));
            Set_WscGenPinCode_Proc(pAd, "1");
            break;

		case WSC_AP_BAND:
			value = *(uwrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_PBC_BAND, value = %d\n", __FUNCTION__, value));
			if (value < PREFERRED_WPS_AP_PHY_TYPE_MAXIMUM)
			{
				pAd->StaCfg.WscControl.WpsApBand= value;
			}
			break;
			
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d, value = %d\n", __FUNCTION__, subcmd, value));
            break;
    }
    
    return Status;
}

static int
rt_private_set_wsc_string_item(struct net_device *dev, struct iw_request_info *info,
		struct iw_point *dwrq, char *extra)
{    
    int  Status=0;
    u32 subcmd = dwrq->flags;
    u32 tmpProfileIndex = (u32)(extra[0] - 0x30);
    u32 dataLen;
    PRTMP_ADAPTER   pAd = NULL;
    PWSC_PROFILE    pWscProfile = NULL;
    USHORT  tmpAuth = 0, tmpEncr = 0;

	GET_PAD_FROM_NET_DEV(pAd, dev);
	pWscProfile = &pAd->StaCfg.WscControl.WscProfile;

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

    if ((subcmd != WSC_SET_SSID) && 
        (tmpProfileIndex > 7 || tmpProfileIndex < 0))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - subcmd = %d, tmpProfileIndex = %d\n", __FUNCTION__, subcmd, tmpProfileIndex));
        return -EINVAL;
    }

    if ((subcmd != WSC_SET_SSID) && 
		(subcmd != WSC_SET_PIN))
    // extra: "1 input_string", dwrq->length includes '\0'. 3 is size of [index, blank and '\0']
    dataLen = dwrq->length - 3;
    else
        dataLen = dwrq->length;
    
    switch(subcmd)
    {
        case WSC_CREDENTIAL_SSID:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_SSID(%s)\n", __FUNCTION__, extra+2));
            if (dataLen > 0 && dataLen <= NDIS_802_11_LENGTH_SSID)
            {
                pWscProfile->Profile[tmpProfileIndex].SSID.SsidLength = dataLen;
                NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].SSID.Ssid, NDIS_802_11_LENGTH_SSID);
                NdisMoveMemory(pWscProfile->Profile[tmpProfileIndex].SSID.Ssid, extra+2, dataLen);
            }
            else
                Status = -E2BIG;
            break;
        case WSC_CREDENTIAL_AUTH_MODE:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_AUTH_MODE(%s)\n", __FUNCTION__, extra+2));
            if ((tmpAuth = WscGetAuthTypeFromStr(extra+2)) != 0)
            {
                pWscProfile->Profile[tmpProfileIndex].AuthType = tmpAuth;
            }
            else
                Status = -EINVAL;
            break;
        case WSC_CREDENTIAL_ENCR_TYPE:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_ENCR_TYPE(%s)\n", __FUNCTION__, extra+2));
            if ((tmpEncr = WscGetEncrypTypeFromStr(extra+2)) != 0)
            {
                pWscProfile->Profile[tmpProfileIndex].EncrType = tmpEncr;
            }
            else
                Status = -EINVAL;
            break;
        case WSC_CREDENTIAL_KEY_INDEX:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_KEY_INDEX(%s)\n", __FUNCTION__, extra+2));
            if ( *(extra+2) >= 0x31 && *(extra+2) <= 0x34)
            {
                pWscProfile->Profile[tmpProfileIndex].KeyIndex = (UCHAR)*(extra+2) - 0x30;
            }
            else
                Status = -EINVAL;
            break;
        case WSC_CREDENTIAL_KEY:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_KEY(%s)\n", __FUNCTION__, extra+2));
            if ((dataLen >= 8 && dataLen <= 64) ||
                (dataLen == 5 || dataLen == 10 || dataLen == 13 || dataLen == 26))
            {
                pWscProfile->Profile[tmpProfileIndex].KeyLength = dataLen;
                NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].Key, 64);
                NdisMoveMemory(pWscProfile->Profile[tmpProfileIndex].Key, extra+2, dataLen);
            }
            else
                Status = -EINVAL;
            break;
        case WSC_CREDENTIAL_MAC:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_MAC(%s)\n", __FUNCTION__, extra+2));
            {
                INT sscanf_rv = 0;
                UINT tmp_val[6] = {0};
                sscanf_rv = sscanf(extra+2, "%02x:%02x:%02x:%02x:%02x:%02x", 
                                                          &tmp_val[0],
                                                          &tmp_val[1],
                                                          &tmp_val[2],
                                                          &tmp_val[3],
                                                          &tmp_val[4],
                                                          &tmp_val[5]);
                if ( sscanf_rv == 6)
                {
                    int ii;
                    NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].MacAddr, 6);
                    for (ii=0; ii<6; ii++)
                        pWscProfile->Profile[tmpProfileIndex].MacAddr[ii] = (UCHAR)tmp_val[ii];
                }
                else
                    Status = -EINVAL;
            }            
            break;
        case WSC_SET_SSID:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_SSID(%s)\n", __FUNCTION__, extra));
            if (dataLen > 0 && dataLen <= NDIS_802_11_LENGTH_SSID)
            {
            	Set_WscSsid_Proc(pAd, (PSTRING) extra);
            }
            else
                Status = -E2BIG;
            break;
		case WSC_SET_PIN:
        	DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_PIN, value = (%s)\n", __FUNCTION__, extra));
			if ( dataLen > 0 )
			{
				if (Set_WscPinCode_Proc(pAd, extra) == FALSE)
					Status = -EINVAL;
			}
            else
                Status = -EINVAL;
            break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d\n", __FUNCTION__, subcmd));
            break;
    }
    
    return Status;
}
#endif // WSC_STA_SUPPORT //


#ifdef RANGE_EXT_SUPPORT
#define ENHANCED_STAT_DISPLAY	// Display PER and PLR statistics
#endif // RANGE_EXT_SUPPORT //

static int
rt_private_get_statistics(struct net_device *dev, struct iw_request_info *info,
		struct iw_point *wrq, char *extra)
{
	INT				Status = 0;
    PRTMP_ADAPTER   pAd = NULL;
	ULONG txCount = 0;
#ifdef ENHANCED_STAT_DISPLAY
	ULONG per, plr;
#endif

	GET_PAD_FROM_NET_DEV(pAd, dev);

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

    if (extra == NULL)
    {
        wrq->length = 0;
        return -EIO;
    }
    
    memset(extra, 0x00, IW_PRIV_SIZE_MASK);
    sprintf(extra, "\n\n");

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		txCount = pAd->ate.TxDoneCount;
	else
#endif // RALINK_ATE //
		txCount = (ULONG)pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;
	sprintf(extra+strlen(extra), "Tx success                      = %lu\n", txCount);
#ifdef ENHANCED_STAT_DISPLAY
	per = txCount==0? 0: 1000*(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart)/(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart+txCount);
    sprintf(extra+strlen(extra), "Tx retry count                  = %lu, PER=%ld.%1ld%%\n",
									(ULONG)pAd->WlanCounters.RetryCount.u.LowPart,
									per/10, per % 10);
	plr = txCount==0? 0: 10000*pAd->WlanCounters.FailedCount.u.LowPart/(pAd->WlanCounters.FailedCount.u.LowPart+txCount);
    sprintf(extra+strlen(extra), "Tx fail to Rcv ACK after retry  = %lu, PLR=%ld.%02ld%%\n",
									(ULONG)pAd->WlanCounters.FailedCount.u.LowPart, plr/100, plr%100);
#else
    sprintf(extra+strlen(extra), "Tx retry count          		  = %lu\n", (ULONG)pAd->WlanCounters.RetryCount.u.LowPart);
    sprintf(extra+strlen(extra), "Tx fail to Rcv ACK after retry  = %lu\n", (ULONG)pAd->WlanCounters.FailedCount.u.LowPart);
    sprintf(extra+strlen(extra), "RTS Success Rcv CTS             = %lu\n", (ULONG)pAd->WlanCounters.RTSSuccessCount.u.LowPart);
    sprintf(extra+strlen(extra), "RTS Fail Rcv CTS                = %lu\n", (ULONG)pAd->WlanCounters.RTSFailureCount.u.LowPart);
#endif // ENHANCED_STAT_DISPLAY //

    sprintf(extra+strlen(extra), "Rx success                      = %lu\n", (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart);
#ifdef ENHANCED_STAT_DISPLAY
	per = pAd->WlanCounters.ReceivedFragmentCount.u.LowPart==0? 0: 1000*(pAd->WlanCounters.FCSErrorCount.u.LowPart)/(pAd->WlanCounters.FCSErrorCount.u.LowPart+pAd->WlanCounters.ReceivedFragmentCount.u.LowPart);
    sprintf(extra+strlen(extra), "Rx with CRC                     = %ld, PER=%ld.%1ld%%\n",
										(ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart, per/10, per % 10);
    sprintf(extra+strlen(extra), "Rx drop due to out of resource  = %lu\n", (ULONG)pAd->Counters8023.RxNoBuffer);
    sprintf(extra+strlen(extra), "Rx duplicate frame              = %lu\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);

    sprintf(extra+strlen(extra), "False CCA                       = %lu\n", (ULONG)pAd->RalinkCounters.FalseCCACnt);
#else
    sprintf(extra+strlen(extra), "Rx with CRC                     = %ld\n", (ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart);
    sprintf(extra+strlen(extra), "Rx drop due to out of resource  = %lu\n", (ULONG)pAd->Counters8023.RxNoBuffer);
    sprintf(extra+strlen(extra), "Rx duplicate frame              = %lu\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);

    sprintf(extra+strlen(extra), "False CCA (one second)          = %lu\n", (ULONG)pAd->RalinkCounters.OneSecFalseCCACnt);
#endif // ENHANCED_STAT_DISPLAY //

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		if (pAd->ate.RxAntennaSel == 0)
		{
    		sprintf(extra+strlen(extra), "RSSI-A                          = %ld\n", (LONG)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta));
			sprintf(extra+strlen(extra), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->ate.LastRssi1 - pAd->BbpRssiToDbmDelta));
			sprintf(extra+strlen(extra), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->ate.LastRssi2 - pAd->BbpRssiToDbmDelta));
		}
		else
		{
    		sprintf(extra+strlen(extra), "RSSI                            = %ld\n", (LONG)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta));
		}
	}
	else
#endif // RALINK_ATE //
	{
#ifdef ENHANCED_STAT_DISPLAY
    	sprintf(extra+strlen(extra), "RSSI                            = %ld %ld %ld\n",
    			(LONG)(pAd->StaCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta),
    			(LONG)(pAd->StaCfg.RssiSample.LastRssi1 - pAd->BbpRssiToDbmDelta),
    			(LONG)(pAd->StaCfg.RssiSample.LastRssi2 - pAd->BbpRssiToDbmDelta));

    	// Display Last Rx Rate and BF SNR of first Associated entry in MAC table
    	if (pAd->MacTab.Size > 0)
    	{
    		static char *phyMode[4] = {"CCK", "OFDM", "MM", "GF"};
    		int i;

    		for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
			{
    			PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);
    			if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst==SST_ASSOC)
				{
					UINT32 lastRxRate = pEntry->LastRxRate;

					sprintf(extra+strlen(extra), "Last RX Rate                    = MCS %d, %2dM, %cGI, %s%s\n",
							lastRxRate & 0x7F,  ((lastRxRate>>7) & 0x1)? 40: 20,
							((lastRxRate>>8) & 0x1)? 'S': 'L',
							phyMode[(lastRxRate>>14) & 0x3],
							((lastRxRate>>9) & 0x3)? ", STBC": " ");

#if defined(RT2883) || defined(RT3883)
					sprintf(extra+strlen(extra), "BF SNR                          = %d.%02d, %d.%02d, %d.%02d  FO:%02X\n",
							pEntry->BF_SNR[0]/4, (pEntry->BF_SNR[0] % 4)*25,
							pEntry->BF_SNR[1]/4, (pEntry->BF_SNR[1] % 4)*25,
							pEntry->BF_SNR[2]/4, (pEntry->BF_SNR[2] % 4)*25,
							pEntry->freqOffset & 0xFF);
#endif // defined(RT2883) || defined(RT3883) //
					break;

				}
			}
    	}
#else
    	sprintf(extra+strlen(extra), "RSSI-A                          = %ld\n", (LONG)(pAd->StaCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta));
        sprintf(extra+strlen(extra), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->StaCfg.RssiSample.LastRssi1 - pAd->BbpRssiToDbmDelta));
        sprintf(extra+strlen(extra), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->StaCfg.RssiSample.LastRssi2 - pAd->BbpRssiToDbmDelta));
#endif // ENHANCED_STAT_DISPLAY //
	}   
#ifdef WPA_SUPPLICANT_SUPPORT
    sprintf(extra+strlen(extra), "WpaSupplicantUP                 = %d\n\n", pAd->StaCfg.WpaSupplicantUP);
#endif // WPA_SUPPLICANT_SUPPORT //


	// display pin code
	sprintf(extra+strlen(extra), "RT2860 Linux STA PinCode\t%08u\n", GenerateWpsPinCode(pAd, BSS0));
#ifdef WSC_STA_SUPPORT
{
	char	mode_str[16]={0};
	ULONG	wps_status, wps_state;
    int     idx = 0;

	wps_state = pAd->StaCfg.WscControl.WscState;
	wps_status = pAd->StaCfg.WscControl.WscStatus;
	
	if (pAd->StaCfg.WscControl.WscMode == WSC_PIN_MODE)
		sprintf(mode_str, "PIN -");
	else
		sprintf(mode_str, "PBC -");
	
		sprintf(extra+strlen(extra), "WPS Information(Driver Auto-Connect is %s - %d):\n",
	                                                  pAd->StaCfg.WscControl.WscDriverAutoConnect ? "Enabled":"Disabled",
	                                                  pAd->StaCfg.WscControl.WscDriverAutoConnect);
	// display pin code
	//sprintf(extra+strlen(extra), "RT2860 Linux STA PinCode\t%08u\n", pAd->StaCfg.WscControl.WscEnrolleePinCode);
	// display status
	if ((wps_state == WSC_STATE_OFF) || (wps_status & 0xff00))
	{
		if (wps_status == STATUS_WSC_CONFIGURED)
		{
			sprintf(extra+strlen(extra), "WPS messages exchange successfully !!!\n");
		}
		else if ((wps_status == STATUS_WSC_NOTUSED))
		{
			sprintf(extra+strlen(extra), "WPS not used.\n");
		}
		else if(wps_status & 0xff00)	// error message
		{
			if (wps_status == STATUS_WSC_PBC_TOO_MANY_AP)
				sprintf(extra+strlen(extra), "%s Too many PBC AP. Please wait... \n", mode_str);
			else if (wps_status == STATUS_WSC_PBC_NO_AP)
				sprintf(extra+strlen(extra), "%s No available PBC AP. Please wait... \n", mode_str);
			else if (wps_status & 0x0100)
				sprintf(extra+strlen(extra), "%s Proceed to get the Registrar profile. Please wait... \n", mode_str);
			else	// status of eap failed
				sprintf(extra+strlen(extra), "WPS didn't complete !!!\n");
		}
		else
		{
			// wrong state
		}
	}
	else
	{
		sprintf(extra+strlen(extra), "%s WPS Proceed. Please wait... \n", mode_str);
	}
	sprintf(extra+strlen(extra), "\n");
    sprintf(extra+strlen(extra), "WPS Profile Count               = %d\n", pAd->StaCfg.WscControl.WscProfile.ProfileCnt);
    for (idx = 0; idx < pAd->StaCfg.WscControl.WscProfile.ProfileCnt ; idx++)
    {
        PWSC_CREDENTIAL pCredential = &pAd->StaCfg.WscControl.WscProfile.Profile[idx];

        if (strlen(extra) + sizeof(WSC_CREDENTIAL) >= IW_PRIV_SIZE_MASK)
        {
            break;
        }
        
        sprintf(extra+strlen(extra), "Profile[%d]:\n", idx);        
        sprintf(extra+strlen(extra), "SSID                            = %s\n", pCredential->SSID.Ssid);
        sprintf(extra+strlen(extra), "MAC                             = %02X:%02X:%02X:%02X:%02X:%02X\n", 
                                                                           pCredential->MacAddr[0],
                                                                           pCredential->MacAddr[1],
                                                                           pCredential->MacAddr[2],
                                                                           pCredential->MacAddr[3],
                                                                           pCredential->MacAddr[4],
                                                                           pCredential->MacAddr[5]);
        sprintf(extra+strlen(extra), "AuthType                        = %s\n", WscGetAuthTypeStr(pCredential->AuthType));
        sprintf(extra+strlen(extra), "EncrypType                      = %s\n", WscGetEncryTypeStr(pCredential->EncrType)); 
        sprintf(extra+strlen(extra), "KeyIndex                        = %d\n", pCredential->KeyIndex);
        if (pCredential->KeyLength != 0)
        {
            if (pCredential->AuthType & (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_WPANONE))
            {
            if (pCredential->KeyLength < 64)
                sprintf(extra+strlen(extra), "Key                             = %s\n", pCredential->Key);
            else
            {
                char key_print[65] = {0};
                NdisMoveMemory(key_print, pCredential->Key, 64);
                sprintf(extra+strlen(extra), "Key                             = %s\n", key_print);
            }
        }
            else if ((pCredential->AuthType == WSC_AUTHTYPE_OPEN) ||
                     (pCredential->AuthType == WSC_AUTHTYPE_SHARED))
            {
                //check key string is ASCII or not
                if (RTMPCheckStrPrintAble((PCHAR)pCredential->Key, (UCHAR)pCredential->KeyLength))
                    sprintf(extra+strlen(extra), "Key                             = %s\n", pCredential->Key);
                else
                {
                    int idx;
                    sprintf(extra+strlen(extra), "Key                             = ");
                    for (idx = 0; idx < pCredential->KeyLength; idx++)
                        sprintf(extra+strlen(extra), "%02X", pCredential->Key[idx]);
                    sprintf(extra+strlen(extra), "\n");
                }
            }
        }
    }
    sprintf(extra+strlen(extra), "\n");
}
#endif // WSC_STA_SUPPORT //
        
    wrq->length = strlen(extra) + 1; // 1: size of '\0'
    DBGPRINT(RT_DEBUG_TRACE, ("<== rt_private_get_statistics, wrq->length = %d\n", wrq->length));

    return Status;
}

#ifdef DOT11_N_SUPPORT
void	getBaInfo(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			pOutBuf)
{
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_TDLS(pEntry)) && (pEntry->Sst == SST_ASSOC))
			|| IS_ENTRY_WDS(pEntry) || IS_ENTRY_MESH(pEntry))
		{		
			sprintf(pOutBuf, "%s\n%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (AP) -\n",
                pOutBuf,
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], pEntry->Aid);
			
			sprintf(pOutBuf, "%s[Recipient]\n", pOutBuf);
			for (j=0; j < NUM_OF_TID; j++)
			{
				if (pEntry->BARecWcidArray[j] != 0)
				{
					pRecBAEntry =&pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];
					sprintf(pOutBuf, "%sTID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d\n", pOutBuf, j, pRecBAEntry->BAWinSize, pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen);
				}
			}
			sprintf(pOutBuf, "%s\n", pOutBuf);

			sprintf(pOutBuf, "%s[Originator]\n", pOutBuf);
			for (j=0; j < NUM_OF_TID; j++)
			{
				if (pEntry->BAOriWcidArray[j] != 0)
				{
					pOriBAEntry =&pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]];
					sprintf(pOutBuf, "%sTID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n", pOutBuf, j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence, pEntry->TxSeq[j]);
				}
			}
			sprintf(pOutBuf, "%s\n\n", pOutBuf);
		}
        if (strlen(pOutBuf) > (IW_PRIV_SIZE_MASK - 30))
                break;
	}

	return;
}
#endif // DOT11_N_SUPPORT //

static int
rt_private_show(struct net_device *dev, struct iw_request_info *info,
		struct iw_point *wrq, PSTRING extra)
{
	INT				Status = 0;
	PRTMP_ADAPTER   pAd;
	POS_COOKIE		pObj;
	u32             subcmd = wrq->flags;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (extra == NULL)
	{
		wrq->length = 0;
		return -EIO;
	}
	memset(extra, 0x00, IW_PRIV_SIZE_MASK);
    
	{
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
	}
    
    switch(subcmd)
    {

        case SHOW_CONN_STATUS:
            if (MONITOR_ON(pAd))
            {
#ifdef DOT11_N_SUPPORT
                if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED &&
                    pAd->CommonCfg.RegTransmitSetting.field.BW)
                    sprintf(extra, "Monitor Mode(CentralChannel %d)\n", pAd->CommonCfg.CentralChannel);
                else
#endif // DOT11_N_SUPPORT //
                    sprintf(extra, "Monitor Mode(Channel %d)\n", pAd->CommonCfg.Channel);
            }
            else
            {
                if (pAd->IndicateMediaState == NdisMediaStateConnected)
            	{        	    
            	    if (INFRA_ON(pAd))
                    {   
                    sprintf(extra, "Connected(AP: %s[%02X:%02X:%02X:%02X:%02X:%02X])\n", 
                                    pAd->CommonCfg.Ssid, 
                                    pAd->CommonCfg.Bssid[0],
                                    pAd->CommonCfg.Bssid[1],
                                    pAd->CommonCfg.Bssid[2],
                                    pAd->CommonCfg.Bssid[3],
                                    pAd->CommonCfg.Bssid[4],
                                    pAd->CommonCfg.Bssid[5]);
            		DBGPRINT(RT_DEBUG_TRACE ,("Ssid=%s ,Ssidlen = %d\n",pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen));
            	}
                    else if (ADHOC_ON(pAd))
                        sprintf(extra, "Connected\n");
            	}
            	else
            	{
            	    sprintf(extra, "Disconnected\n");
            		DBGPRINT(RT_DEBUG_TRACE ,("ConnStatus is not connected\n"));
            	}
            }
            wrq->length = strlen(extra) + 1; // 1: size of '\0'
            break;
        case SHOW_DRVIER_VERION:
            sprintf(extra, "Driver version-%s, %s %s\n", STA_DRIVER_VERSION, __DATE__, __TIME__ );
            wrq->length = strlen(extra) + 1; // 1: size of '\0'
            break;
#ifdef DOT11_N_SUPPORT
        case SHOW_BA_INFO:
            getBaInfo(pAd, extra);
            wrq->length = strlen(extra) + 1; // 1: size of '\0'
            break;
#endif // DOT11_N_SUPPORT //
		case SHOW_DESC_INFO:
			{
				Show_DescInfo_Proc(pAd, NULL);
				wrq->length = 0; // 1: size of '\0'				
			}
			break;
        case RAIO_OFF:
            if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
                if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
		        {
		            RTMP_MLME_RESET_STATE_MACHINE(pAd);
		            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
		        }
            }
            pAd->StaCfg.bSwRadio = FALSE;
            if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
            {
                pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                if (pAd->StaCfg.bRadio == FALSE)
                {
                    MlmeRadioOff(pAd);
                    // Update extra information
					pAd->ExtraInfo = SW_RADIO_OFF;
                } 
            }
            sprintf(extra, "Radio Off\n");
            wrq->length = strlen(extra) + 1; // 1: size of '\0'
            break;
        case RAIO_ON:
            pAd->StaCfg.bSwRadio = TRUE;
            //if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
            {
                pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                if (pAd->StaCfg.bRadio == TRUE)
                {
                    MlmeRadioOn(pAd);
                    // Update extra information
					pAd->ExtraInfo = EXTRA_INFO_CLEAR;
                }
            }
            sprintf(extra, "Radio On\n");
            wrq->length = strlen(extra) + 1; // 1: size of '\0'
            break;


#ifdef QOS_DLS_SUPPORT
		case SHOW_DLS_ENTRY_INFO:
			{
				Set_DlsEntryInfo_Display_Proc(pAd, NULL);
				wrq->length = 0; // 1: size of '\0'
			}
			break;
#endif // QOS_DLS_SUPPORT //

#ifdef DOT11Z_TDLS_SUPPORT
		case SHOW_TDLS_ENTRY_INFO:
			{
				Set_TdlsEntryInfo_Display_Proc(pAd, NULL);
				wrq->length = 0; // 1: size of '\0'
			}
			break;
#endif // DOT11Z_TDLS_SUPPORT //

		case SHOW_CFG_VALUE:
			{
				Status = RTMPShowCfgValue(pAd, (PSTRING) wrq->pointer, extra);
				if (Status == 0)
					wrq->length = strlen(extra) + 1; // 1: size of '\0'
			}
			break;
		case SHOW_ADHOC_ENTRY_INFO:
			Show_Adhoc_MacTable_Proc(pAd, extra);
			wrq->length = strlen(extra) + 1; // 1: size of '\0'
			break;

#ifdef WMM_ACM_SUPPORT

       /* case SHOW_ACM_BADNWIDTH:
            AcmCmdBandwidthGuiDisplay(pAd, extra);
            wrq->length = strlen(extra) + 1; // 1: size of '\0'            
            break;*/
        case SHOW_ACM_STREAM:
            //AcmCmdStreamGuiDisplay(pAd, extra);
            wrq->length = strlen(extra) + 1; // 1: size of '\0'
            printk("SHOW_ACM_STREAM - wrq->length = %d\n", wrq->length);
            break;

#endif
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d\n", __FUNCTION__, subcmd));
            break;
    }
    
    return Status;
}

#ifdef SIOCSIWMLME
int rt_ioctl_siwmlme(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
{
	PRTMP_ADAPTER   pAd = NULL;
	struct iw_mlme *pMlme = (struct iw_mlme *)wrqu->data.pointer;
	MLME_QUEUE_ELEM				MsgElem;
	MLME_DISASSOC_REQ_STRUCT	DisAssocReq;
	MLME_DEAUTH_REQ_STRUCT      DeAuthReq;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

	if (pMlme == NULL)
		return -EINVAL;

	switch(pMlme->cmd)
	{
#ifdef IW_MLME_DEAUTH	
		case IW_MLME_DEAUTH:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - IW_MLME_DEAUTH\n", __FUNCTION__));			                
			COPY_MAC_ADDR(DeAuthReq.Addr, pAd->CommonCfg.Bssid);
			DeAuthReq.Reason = pMlme->reason_code;
			MsgElem.MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
			NdisMoveMemory(MsgElem.Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
			MlmeDeauthReqAction(pAd, &MsgElem);
			if (INFRA_ON(pAd))
			{
			    LinkDown(pAd, FALSE);
			    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
			}
			break;
#endif // IW_MLME_DEAUTH //
#ifdef IW_MLME_DISASSOC
		case IW_MLME_DISASSOC:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - IW_MLME_DISASSOC\n", __FUNCTION__));
			COPY_MAC_ADDR(DisAssocReq.Addr, pAd->CommonCfg.Bssid);
			DisAssocReq.Reason =  pMlme->reason_code;

			MsgElem.Machine = ASSOC_STATE_MACHINE;
			MsgElem.MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem.MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem.Msg, &DisAssocReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, &MsgElem);
			break;
#endif // IW_MLME_DISASSOC //
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - Unknow Command\n", __FUNCTION__));
			break;
	}
	
	return 0;
}
#endif // SIOCSIWMLME //

#if WIRELESS_EXT > 17


int rt_ioctl_siwauth(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAd = NULL;
	struct iw_param *param = &wrqu->param;

	GET_PAD_FROM_NET_DEV(pAd, dev);

    //check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;   
	}
	switch (param->flags & IW_AUTH_INDEX) {
    	case IW_AUTH_WPA_VERSION:
            if (param->value == IW_AUTH_WPA_VERSION_WPA)
            {            
                pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPAPSK;
				if (pAd->StaCfg.BssType == BSS_ADHOC)
					pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPANone;
            }
            else if (param->value == IW_AUTH_WPA_VERSION_WPA2)
                pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPA2PSK;
			
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_VERSION - param->value = %d!\n", __FUNCTION__, param->value));
            break;
    	case IW_AUTH_CIPHER_PAIRWISE:
            if (param->value == IW_AUTH_CIPHER_NONE)
            {
                pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
                pAd->StaCfg.PairCipher = Ndis802_11WEPDisabled;
            }
            else if (param->value == IW_AUTH_CIPHER_WEP40 ||
                     param->value == IW_AUTH_CIPHER_WEP104)
            {
                pAd->StaCfg.WepStatus = Ndis802_11WEPEnabled;
                pAd->StaCfg.PairCipher = Ndis802_11WEPEnabled;
#ifdef WPA_SUPPLICANT_SUPPORT                
                pAd->StaCfg.IEEE8021X = FALSE;
#endif // WPA_SUPPLICANT_SUPPORT //
            }
            else if (param->value == IW_AUTH_CIPHER_TKIP)
            {
                pAd->StaCfg.WepStatus = Ndis802_11Encryption2Enabled;
                pAd->StaCfg.PairCipher = Ndis802_11Encryption2Enabled;
            }
            else if (param->value == IW_AUTH_CIPHER_CCMP)
            {
                pAd->StaCfg.WepStatus = Ndis802_11Encryption3Enabled;
                pAd->StaCfg.PairCipher = Ndis802_11Encryption3Enabled;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_CIPHER_PAIRWISE - param->value = %d!\n", __FUNCTION__, param->value));
            break;
    	case IW_AUTH_CIPHER_GROUP:
            if (param->value == IW_AUTH_CIPHER_NONE)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
            }
            else if (param->value == IW_AUTH_CIPHER_WEP40)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11GroupWEP40Enabled;
            }
			else if (param->value == IW_AUTH_CIPHER_WEP104)
            {
				pAd->StaCfg.GroupCipher = Ndis802_11GroupWEP104Enabled;
            }
            else if (param->value == IW_AUTH_CIPHER_TKIP)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11Encryption2Enabled;
            }
            else if (param->value == IW_AUTH_CIPHER_CCMP)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11Encryption3Enabled;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_CIPHER_GROUP - param->value = %d!\n", __FUNCTION__, param->value));
            break;
    	case IW_AUTH_KEY_MGMT:
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
			pAd->StaCfg.WpaSupplicantUP &= 0x7F;
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
            if (param->value == IW_AUTH_KEY_MGMT_802_1X)
            { 
                if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)
                {
                    pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPA;
#ifdef WPA_SUPPLICANT_SUPPORT                    
                    pAd->StaCfg.IEEE8021X = FALSE;
#endif // WPA_SUPPLICANT_SUPPORT //
                }
                else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
                {
                    pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPA2;
#ifdef WPA_SUPPLICANT_SUPPORT
                    pAd->StaCfg.IEEE8021X = FALSE;
#endif // WPA_SUPPLICANT_SUPPORT //
                }
#ifdef WPA_SUPPLICANT_SUPPORT                
                else
                    // WEP 1x
                    pAd->StaCfg.IEEE8021X = TRUE;
#endif // WPA_SUPPLICANT_SUPPORT //                
            }
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
            else if (param->value == 0)
            {
                //pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
				STA_PORT_SECURED(pAd);
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_KEY_MGMT - param->value = %d!\n", __FUNCTION__, param->value));
            break;
    	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
            break;
    	case IW_AUTH_PRIVACY_INVOKED:
            /*if (param->value == 0)
			{
                pAd->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
                pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
                pAd->StaCfg.PairCipher = Ndis802_11WEPDisabled;
        	    pAd->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
            }*/            
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_PRIVACY_INVOKED - param->value = %d!\n", __FUNCTION__, param->value));
    		break;
    	case IW_AUTH_DROP_UNENCRYPTED:
            if (param->value != 0)
                pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{
                //pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
				STA_PORT_SECURED(pAd);
			}
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_VERSION - param->value = %d!\n", __FUNCTION__, param->value));
    		break;
    	case IW_AUTH_80211_AUTH_ALG: 
			if (param->value & IW_AUTH_ALG_SHARED_KEY) 
            {
				pAd->StaCfg.AuthMode = Ndis802_11AuthModeShared;
			} 
            else if ((param->value & (IW_AUTH_ALG_OPEN_SYSTEM | IW_AUTH_ALG_LEAP)))
            {
				pAd->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
			} 
            else
				pAd->StaCfg.AuthMode = Ndis802_11AuthModeAutoSwitch;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_80211_AUTH_ALG - param->value = %d!\n", __FUNCTION__, param->value));
			break;
    	case IW_AUTH_WPA_ENABLED:
    		DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_ENABLED - Driver supports WPA!(param->value = %d)\n", __FUNCTION__, param->value));
    		break;
    	default:
    		return -EOPNOTSUPP;
}

	return 0;
}

int rt_ioctl_giwauth(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAd = NULL;
	struct iw_param *param = &wrqu->param;

	GET_PAD_FROM_NET_DEV(pAd, dev);

    //check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;   
    }

	switch (param->flags & IW_AUTH_INDEX) {
	case IW_AUTH_DROP_UNENCRYPTED:
        param->value = (pAd->StaCfg.WepStatus == Ndis802_11WEPDisabled) ? 0 : 1;
		break;

	case IW_AUTH_80211_AUTH_ALG:
        param->value = (pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared) ? IW_AUTH_ALG_SHARED_KEY : IW_AUTH_ALG_OPEN_SYSTEM;
		break;

	case IW_AUTH_WPA_ENABLED:
		param->value = (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA) ? 1 : 0;
		break;

	default:
		return -EOPNOTSUPP;
	}
    DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_giwauth::param->value = %d!\n", param->value));
	return 0;
}

void fnSetCipherKey(
    IN  PRTMP_ADAPTER   pAd,
    IN  INT             keyIdx,    
    IN  UCHAR           CipherAlg,
    IN  BOOLEAN         bGTK,
    IN  struct iw_encode_ext *ext)
{
    NdisZeroMemory(&pAd->SharedKey[BSS0][keyIdx], sizeof(CIPHER_KEY));
    pAd->SharedKey[BSS0][keyIdx].KeyLen = LEN_TK;
    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, ext->key, LEN_TK);
    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].TxMic, ext->key + LEN_TK, LEN_TKIP_MIC);
    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].RxMic, ext->key + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);
    pAd->SharedKey[BSS0][keyIdx].CipherAlg = CipherAlg;

    // Update group key information to ASIC Shared Key Table	   
	AsicAddSharedKeyEntry(pAd, 
						  BSS0, 
						  keyIdx, 
						  &pAd->SharedKey[BSS0][keyIdx]);
			
	// Update ASIC WCID attribute table and IVEIV table
	if (!bGTK)
		RTMPSetWcidSecurityInfo(pAd, 
	    						BSS0, 
	    						keyIdx, 
	    						pAd->SharedKey[BSS0][keyIdx].CipherAlg, 
	       						BSSID_WCID, 
	       						SHAREDKEYTABLE);
}

int rt_ioctl_siwencodeext(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
			{
	PRTMP_ADAPTER   pAd = NULL;
	struct iw_point *encoding = &wrqu->encoding;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
    int keyIdx, alg = ext->alg;
	
	GET_PAD_FROM_NET_DEV(pAd, dev);
	
    //check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;   
	}

    if (encoding->flags & IW_ENCODE_DISABLED)
	{
        keyIdx = (encoding->flags & IW_ENCODE_INDEX) - 1;
        // set BSSID wcid entry of the Pair-wise Key table as no-security mode
	    AsicRemovePairwiseKeyEntry(pAd, BSSID_WCID);
        pAd->SharedKey[BSS0][keyIdx].KeyLen = 0;
		pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)keyIdx);
        NdisZeroMemory(&pAd->SharedKey[BSS0][keyIdx], sizeof(CIPHER_KEY));
        DBGPRINT(RT_DEBUG_TRACE, ("%s::Remove all keys!(encoding->flags = %x)\n", __FUNCTION__, encoding->flags));
    }
					else
    {
        // Get Key Index and convet to our own defined key index
    	keyIdx = (encoding->flags & IW_ENCODE_INDEX) - 1;
    	if((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
    		return -EINVAL;               
					
        if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
        {
            pAd->StaCfg.DefaultKeyId = keyIdx;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::DefaultKeyId = %d\n", __FUNCTION__, pAd->StaCfg.DefaultKeyId));
        }

        switch (alg) {
    		case IW_ENCODE_ALG_NONE:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_NONE\n", __FUNCTION__));
    			break;
    		case IW_ENCODE_ALG_WEP:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_WEP - ext->key_len = %d, keyIdx = %d\n", __FUNCTION__, ext->key_len, keyIdx));
    			if (ext->key_len == MAX_WEP_KEY_SIZE)
                {      
        			pAd->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
                    pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
				}
        		else if (ext->key_len == MIN_WEP_KEY_SIZE)
                {      
                    pAd->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
                    pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
			}
        		else
                    return -EINVAL;
                                
                NdisZeroMemory(pAd->SharedKey[BSS0][keyIdx].Key,  16);
			    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, ext->key, ext->key_len);

				if (pAd->StaCfg.GroupCipher == Ndis802_11GroupWEP40Enabled ||					
					pAd->StaCfg.GroupCipher == Ndis802_11GroupWEP104Enabled)				
				{										
					// Set Group key material to Asic					
					AsicAddSharedKeyEntry(pAd, BSS0, keyIdx, &pAd->SharedKey[BSS0][keyIdx]);										

					// Assign pairwise key info
					RTMPSetWcidSecurityInfo(pAd,
										 	BSS0, 
										 	keyIdx, 
										 	pAd->SharedKey[BSS0][keyIdx].CipherAlg, 												 
										 	BSSID_WCID, 
										 	SHAREDKEYTABLE);
					STA_PORT_SECURED(pAd);					    				
					// Indicate Connected for GUI    				
					pAd->IndicateMediaState = NdisMediaStateConnected;				
				}
    			break;
            case IW_ENCODE_ALG_TKIP:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_TKIP - keyIdx = %d, ext->key_len = %d\n", __FUNCTION__, keyIdx, ext->key_len));
                if (ext->key_len == 32)
                {
                    if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
                    {
                        fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, FALSE, ext);
                        if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA2)
                        {
                            //pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
                            STA_PORT_SECURED(pAd);
                            pAd->IndicateMediaState = NdisMediaStateConnected;	
                        }
		}
                    else if (ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
                    {
                        fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, TRUE, ext);
		
                        // set 802.1x port control
            	        //pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
            	        STA_PORT_SECURED(pAd);
            	        pAd->IndicateMediaState = NdisMediaStateConnected;	
                    }
                }
                else
                    return -EINVAL;
                break;
            case IW_ENCODE_ALG_CCMP:
                if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
		{
                    fnSetCipherKey(pAd, keyIdx, CIPHER_AES, FALSE, ext);
                    if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA2)
                    	//pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
                    	STA_PORT_SECURED(pAd);
                    	pAd->IndicateMediaState = NdisMediaStateConnected;	
                }
                else if (ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
                {
                    fnSetCipherKey(pAd, keyIdx, CIPHER_AES, TRUE, ext);
                    
                    // set 802.1x port control
        	        //pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
        	        STA_PORT_SECURED(pAd);
        	        pAd->IndicateMediaState = NdisMediaStateConnected;	
                }
                break;
    		default:
    			return -EINVAL;
		}
    }
			
    return 0;
}

int
rt_ioctl_giwencodeext(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER pAd = NULL;
	PCHAR pKey = NULL;
	struct iw_point *encoding = &wrqu->encoding;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
	int idx, max_key_len;

	GET_PAD_FROM_NET_DEV(pAd, dev);

	DBGPRINT(RT_DEBUG_TRACE ,("===> rt_ioctl_giwencodeext\n"));

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

	max_key_len = encoding->length - sizeof(*ext);
	if (max_key_len < 0)
		return -EINVAL;

	idx = encoding->flags & IW_ENCODE_INDEX;
	if (idx) 
	{
		if (idx < 1 || idx > 4)
			return -EINVAL;
		idx--;

		if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) ||
			(pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled))
		{
			if (idx != pAd->StaCfg.DefaultKeyId)
			{
				ext->key_len = 0;
				return 0;
			}
		}
	} 
	else
		idx = pAd->StaCfg.DefaultKeyId;

	encoding->flags = idx + 1;
	memset(ext, 0, sizeof(*ext));

	ext->key_len = 0;
	switch(pAd->StaCfg.WepStatus) {
		case Ndis802_11WEPDisabled:
			ext->alg = IW_ENCODE_ALG_NONE;
			encoding->flags |= IW_ENCODE_DISABLED;		
			break;
		case Ndis802_11WEPEnabled:
			ext->alg = IW_ENCODE_ALG_WEP;
			if (pAd->SharedKey[BSS0][idx].KeyLen > max_key_len)
				return -E2BIG;
			else
			{
				ext->key_len = pAd->SharedKey[BSS0][idx].KeyLen;				
				pKey = (PCHAR)&(pAd->SharedKey[BSS0][idx].Key[0]);
			}
			break;
		case Ndis802_11Encryption2Enabled:
		case Ndis802_11Encryption3Enabled:
			if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
				ext->alg = IW_ENCODE_ALG_TKIP;
			else
				ext->alg = IW_ENCODE_ALG_CCMP;
			
			if (max_key_len < 32)
				return -E2BIG;
			else
			{
				ext->key_len = 32;
				pKey = (PCHAR)&pAd->StaCfg.PMK[0];
			}
			break;
		default:
			return -EINVAL;
	}

	if (ext->key_len && pKey)
	{
		encoding->flags |= IW_ENCODE_ENABLED;
		memcpy(ext->key, pKey, ext->key_len);
	}
	
	return 0;
}

#ifdef SIOCSIWGENIE
int rt_ioctl_siwgenie(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);	
	
	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}	
#ifdef WPA_SUPPLICANT_SUPPORT
	if (pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
	{
		DBGPRINT(RT_DEBUG_TRACE ,("===> rt_ioctl_siwgenie\n"));
		pAd->StaCfg.bRSN_IE_FromWpaSupplicant = FALSE;
		if ((wrqu->data.length == 0) ||
		    (extra == NULL))
		{
			return -EINVAL;
		}
		else if (wrqu->data.length) 
		{
			if (pAd->StaCfg.pWpaAssocIe)
			{
				kfree(pAd->StaCfg.pWpaAssocIe);
				pAd->StaCfg.pWpaAssocIe = NULL;
			}
			pAd->StaCfg.pWpaAssocIe = kmalloc(wrqu->data.length, MEM_ALLOC_FLAG);
			if (pAd->StaCfg.pWpaAssocIe)
			{
				pAd->StaCfg.WpaAssocIeLen = wrqu->data.length;
				NdisMoveMemory(pAd->StaCfg.pWpaAssocIe, extra, pAd->StaCfg.WpaAssocIeLen);
				pAd->StaCfg.bRSN_IE_FromWpaSupplicant = TRUE;
			}
			else
				pAd->StaCfg.WpaAssocIeLen = 0;
		}
		return 0;
	}
	else
#endif // WPA_SUPPLICANT_SUPPORT //
	return -EOPNOTSUPP;
}
#endif // SIOCSIWGENIE //

int rt_ioctl_giwgenie(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);	
	
	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}
	
	if ((pAd->StaCfg.RSNIE_Len == 0) ||
		(pAd->StaCfg.AuthMode < Ndis802_11AuthModeWPA))
	{
		wrqu->data.length = 0;
		return 0;
	}

#ifdef WPA_SUPPLICANT_SUPPORT
#ifdef SIOCSIWGENIE
	if ((pAd->StaCfg.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE &&
		(pAd->StaCfg.WpaAssocIeLen > 0))
	{
		if (wrqu->data.length < pAd->StaCfg.WpaAssocIeLen)
			return -E2BIG;

		wrqu->data.length = pAd->StaCfg.WpaAssocIeLen;
		memcpy(extra, pAd->StaCfg.pWpaAssocIe, pAd->StaCfg.WpaAssocIeLen);
	}
	else
#endif // SIOCSIWGENIE //
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
	{
		UCHAR RSNIe = IE_WPA;
		
		if (wrqu->data.length < (pAd->StaCfg.RSNIE_Len + 2)) // ID, Len
			return -E2BIG;
		wrqu->data.length = pAd->StaCfg.RSNIE_Len + 2;
		
		if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
            (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2))
			RSNIe = IE_RSN;
		
		extra[0] = (char)RSNIe;
		extra[1] = pAd->StaCfg.RSNIE_Len;
		memcpy(extra+2, &pAd->StaCfg.RSN_IE[0], pAd->StaCfg.RSNIE_Len);
	}
	
	return 0;
}

int rt_ioctl_siwpmksa(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
{
	PRTMP_ADAPTER   pAd = NULL;
	struct iw_pmksa *pPmksa = (struct iw_pmksa *)wrqu->data.pointer;
	INT	CachedIdx = 0, idx = 0;

	GET_PAD_FROM_NET_DEV(pAd, dev);	

	//check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
	}

	if (pPmksa == NULL)
		return -EINVAL;

	DBGPRINT(RT_DEBUG_TRACE ,("===> rt_ioctl_siwpmksa\n"));
	switch(pPmksa->cmd)
	{
		case IW_PMKSA_FLUSH:
			NdisZeroMemory(pAd->StaCfg.SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_FLUSH\n"));
			break;
		case IW_PMKSA_REMOVE:
			for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			{
		        // compare the BSSID
		        if (NdisEqualMemory(pPmksa->bssid.sa_data, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
		        {
		        	NdisZeroMemory(pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN);
					NdisZeroMemory(pAd->StaCfg.SavedPMK[CachedIdx].PMKID, 16);
					for (idx = CachedIdx; idx < (pAd->StaCfg.SavedPMKNum - 1); idx++)
					{
						NdisMoveMemory(&pAd->StaCfg.SavedPMK[idx].BSSID[0], &pAd->StaCfg.SavedPMK[idx+1].BSSID[0], MAC_ADDR_LEN);
						NdisMoveMemory(&pAd->StaCfg.SavedPMK[idx].PMKID[0], &pAd->StaCfg.SavedPMK[idx+1].PMKID[0], 16);
					}
					pAd->StaCfg.SavedPMKNum--;
			        break;
		        }
	        }
			
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_REMOVE\n"));
			break;
		case IW_PMKSA_ADD:
			for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			{
		        // compare the BSSID
		        if (NdisEqualMemory(pPmksa->bssid.sa_data, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
			        break;			
	        }

	        // Found, replace it
	        if (CachedIdx < PMKID_NO)
	        {
		        DBGPRINT(RT_DEBUG_OFF, ("Update PMKID, idx = %d\n", CachedIdx));
		        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].BSSID[0], pPmksa->bssid.sa_data, MAC_ADDR_LEN);
				NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].PMKID[0], pPmksa->pmkid, 16);
		        pAd->StaCfg.SavedPMKNum++;
	        }
	        // Not found, replace the last one
	        else
	        {
		        // Randomly replace one
		        CachedIdx = (pPmksa->bssid.sa_data[5] % PMKID_NO);
		        DBGPRINT(RT_DEBUG_OFF, ("Update PMKID, idx = %d\n", CachedIdx));
		        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].BSSID[0], pPmksa->bssid.sa_data, MAC_ADDR_LEN);
				NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].PMKID[0], pPmksa->pmkid, 16);
	        }
			
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_ADD\n"));
			break;
		default:
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - Unknow Command!!\n"));
			break;
	}

	return 0;
}
#endif // #if WIRELESS_EXT > 17

#ifdef DBG
static int
rt_private_ioctl_bbp(struct net_device *dev, struct iw_request_info *info,
		struct iw_point *wrq, char *extra)
			{
	PSTRING				this_char;
	PSTRING				value = NULL;
	UCHAR				regBBP = 0;
//	CHAR				arg[255]={0};
	UINT32				bbpId;
	UINT32				bbpValue;
	BOOLEAN				bIsPrintAllBBP = FALSE;
	INT					Status = 0;
    PRTMP_ADAPTER       pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, dev);	


	memset(extra, 0x00, IW_PRIV_SIZE_MASK);

	if (wrq->length > 1) //No parameters.
				{
		sprintf(extra, "\n");
					
		//Parsing Read or Write
		this_char = wrq->pointer;
		DBGPRINT(RT_DEBUG_TRACE, ("this_char=%s\n", this_char));
		if (!*this_char)                                                                            
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;		
		
		if (!value || !*value)
		{ //Read                                                                                    
			DBGPRINT(RT_DEBUG_TRACE, ("this_char=%s, value=%s\n", this_char, value));
			if (sscanf(this_char, "%d", &(bbpId)) == 1)                                             
			{  
				if (bbpId <= MAX_BBP_ID)
				{                                                                                   
#ifdef RALINK_ATE
					if (ATE_ON(pAd))
					{
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);
					}
					else
#endif // RALINK_ATE //
					{
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);                          
					}
					sprintf(extra+strlen(extra), "R%02d[0x%02X]:%02X\n", bbpId, bbpId, regBBP);
                    wrq->length = strlen(extra) + 1; // 1: size of '\0'
					DBGPRINT(RT_DEBUG_TRACE, ("msg=%s\n", extra));
				}                                                                                   
				else                                                                                
				{//Invalid parametes, so default printk all bbp                                     
					bIsPrintAllBBP = TRUE;
					goto next;                                                                          
				}                                                                                   
			}                                                                                       
			else                                                                                    
			{ //Invalid parametes, so default printk all bbp                                        
				bIsPrintAllBBP = TRUE;
				goto next;                                                                              
			}                                                                                       
		}                                                                                           
		else                                                                                        
		{ //Write                                
			if ((sscanf(this_char, "%d", &(bbpId)) == 1) && (sscanf(value, "%x", &(bbpValue)) == 1))
			{
				if (bbpId <= MAX_BBP_ID)
				{                                                                                   
#ifdef RALINK_ATE
					if (ATE_ON(pAd))
					{
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpValue);          
						/* read it back for showing */                                                      
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);  
					}
					else
#endif // RALINK_ATE //
					{
					    RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpValue);          
    					/* read it back for showing */                                                      
    					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);                          
			}
					sprintf(extra+strlen(extra), "R%02d[0x%02X]:%02X\n", bbpId, bbpId, regBBP);
                    wrq->length = strlen(extra) + 1; // 1: size of '\0'
					DBGPRINT(RT_DEBUG_TRACE, ("msg=%s\n", extra));                                       
				}                                                                                   
				else                                                                                
				{//Invalid parametes, so default printk all bbp                                     
					bIsPrintAllBBP = TRUE;                                                          
					goto next;                                                                          
				}                                                                                   
			}                                                                                       
			else                                                                                    
			{ //Invalid parametes, so default printk all bbp                                        
				bIsPrintAllBBP = TRUE;                                                              
				goto next;                                                                              
			}                                                                                       
		}                             
		}
	else
		bIsPrintAllBBP = TRUE;

next:    
	if (bIsPrintAllBBP)
	{   
		memset(extra, 0x00, IW_PRIV_SIZE_MASK);
		sprintf(extra, "\n");
		for (bbpId = 0; bbpId <= MAX_BBP_ID; bbpId++)
		{
		    if (strlen(extra) >= (IW_PRIV_SIZE_MASK - 20))
                break;
#ifdef RALINK_ATE
			if (ATE_ON(pAd))
			{
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP); 
			}
			else
#endif // RALINK_ATE //
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);
			sprintf(extra+strlen(extra), "R%02d[0x%02X]:%02X    ", bbpId, bbpId, regBBP);
			if (bbpId%5 == 4)
				sprintf(extra+strlen(extra), "\n");
			//sprintf(extra+strlen(extra), "%03d = %02X\n", bbpId, regBBP);  // edit by johnli, change display format
		}
		
        wrq->length = strlen(extra) + 1; // 1: size of '\0'
        DBGPRINT(RT_DEBUG_TRACE, ("wrq->length = %d\n", wrq->length));
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==rt_private_ioctl_bbp\n\n"));	
    
    return Status;
}
#endif // DBG //

int rt_ioctl_siwrate(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
    PRTMP_ADAPTER   pAd = NULL;
    UINT32          rate = wrqu->bitrate.value, fixed = wrqu->bitrate.fixed;

	GET_PAD_FROM_NET_DEV(pAd, dev);

    //check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwrate::Network is down!\n"));
    	return -ENETDOWN;   
	}    

    DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwrate::(rate = %d, fixed = %d)\n", rate, fixed));
    /* rate = -1 => auto rate
       rate = X, fixed = 1 => (fixed rate X)       
    */
    if (rate == -1)
    {
        //Auto Rate
        pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;	
		pAd->StaCfg.bAutoTxRateSwitch = TRUE;
		if ((pAd->CommonCfg.PhyMode <= PHY_11G) ||
		    (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
            RTMPSetDesiredRates(pAd, -1);
			
#ifdef DOT11_N_SUPPORT
            SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
    }
    else
    {        
        if (fixed)
        {
        	pAd->StaCfg.bAutoTxRateSwitch = FALSE;
            if ((pAd->CommonCfg.PhyMode <= PHY_11G) ||
                (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
                RTMPSetDesiredRates(pAd, rate);
            else
            {
                pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
#ifdef DOT11_N_SUPPORT
                SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
            }
            DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwrate::(HtMcs=%d)\n",pAd->StaCfg.DesiredTransmitSetting.field.MCS));
        }
        else
        {
            // TODO: rate = X, fixed = 0 => (rates <= X)
            return -EOPNOTSUPP;
        }
    }

    return 0;
}

int rt_ioctl_giwrate(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
    PRTMP_ADAPTER   pAd = NULL;
    int rate_index = 0, rate_count = 0;
    HTTRANSMIT_SETTING ht_setting; 

	GET_PAD_FROM_NET_DEV(pAd, dev);

    rate_count = RT_RateSize/sizeof(__s32);
    //check if the interface is down
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;   
	}

    if ((pAd->StaCfg.bAutoTxRateSwitch == FALSE) &&
        (INFRA_ON(pAd)) &&
        ((pAd->CommonCfg.PhyMode <= PHY_11G) || (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM)))
        ht_setting.word = pAd->StaCfg.HTPhyMode.word;
    else
        ht_setting.word = pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;
    
#ifdef DOT11_N_SUPPORT
    if (ht_setting.field.MODE >= MODE_HTMIX)
    {
//    	rate_index = 12 + ((UCHAR)ht_setting.field.BW *16) + ((UCHAR)ht_setting.field.ShortGI *32) + ((UCHAR)ht_setting.field.MCS);
    	rate_index = 12 + ((UCHAR)ht_setting.field.BW *24) + ((UCHAR)ht_setting.field.ShortGI *48) + ((UCHAR)ht_setting.field.MCS);
    }
    else 
#endif // DOT11_N_SUPPORT //
    if (ht_setting.field.MODE == MODE_OFDM)                
    	rate_index = (UCHAR)(ht_setting.field.MCS) + 4;
    else if (ht_setting.field.MODE == MODE_CCK)   
    	rate_index = (UCHAR)(ht_setting.field.MCS);

    if (rate_index < 0)
        rate_index = 0;
    
    if (rate_index > rate_count)
        rate_index = rate_count;

    wrqu->bitrate.value = ralinkrate[rate_index] * 500000;
    wrqu->bitrate.disabled = 0;

    return 0;
}

static const iw_handler rt_handler[] =
{
	(iw_handler) NULL,			            /* SIOCSIWCOMMIT */
	(iw_handler) rt_ioctl_giwname,			/* SIOCGIWNAME   */
	(iw_handler) NULL,			            /* SIOCSIWNWID   */
	(iw_handler) NULL,			            /* SIOCGIWNWID   */
	(iw_handler) rt_ioctl_siwfreq,		    /* SIOCSIWFREQ   */
	(iw_handler) rt_ioctl_giwfreq,		    /* SIOCGIWFREQ   */
	(iw_handler) rt_ioctl_siwmode,		    /* SIOCSIWMODE   */
	(iw_handler) rt_ioctl_giwmode,		    /* SIOCGIWMODE   */
	(iw_handler) NULL,		                /* SIOCSIWSENS   */
	(iw_handler) NULL,		                /* SIOCGIWSENS   */
	(iw_handler) NULL /* not used */,		/* SIOCSIWRANGE  */
	(iw_handler) rt_ioctl_giwrange,		    /* SIOCGIWRANGE  */
	(iw_handler) NULL /* not used */,		/* SIOCSIWPRIV   */
	(iw_handler) NULL /* kernel code */,    /* SIOCGIWPRIV   */
	(iw_handler) NULL /* not used */,		/* SIOCSIWSTATS  */
	(iw_handler) rt28xx_get_wireless_stats /* kernel code */,    /* SIOCGIWSTATS  */
	(iw_handler) NULL,		                /* SIOCSIWSPY    */
	(iw_handler) NULL,		                /* SIOCGIWSPY    */
	(iw_handler) NULL,				        /* SIOCSIWTHRSPY */
	(iw_handler) NULL,				        /* SIOCGIWTHRSPY */
	(iw_handler) rt_ioctl_siwap,            /* SIOCSIWAP     */
	(iw_handler) rt_ioctl_giwap,		    /* SIOCGIWAP     */
#ifdef SIOCSIWMLME
	(iw_handler) rt_ioctl_siwmlme,	        /* SIOCSIWMLME   */
#else
	(iw_handler) NULL,				        /* SIOCSIWMLME */
#endif // SIOCSIWMLME //
	(iw_handler) rt_ioctl_iwaplist,		    /* SIOCGIWAPLIST */
#ifdef SIOCGIWSCAN
	(iw_handler) rt_ioctl_siwscan,		    /* SIOCSIWSCAN   */
	(iw_handler) rt_ioctl_giwscan,		    /* SIOCGIWSCAN   */
#else
	(iw_handler) NULL,				        /* SIOCSIWSCAN   */
	(iw_handler) NULL,				        /* SIOCGIWSCAN   */
#endif /* SIOCGIWSCAN */
	(iw_handler) rt_ioctl_siwessid,		    /* SIOCSIWESSID  */
	(iw_handler) rt_ioctl_giwessid,		    /* SIOCGIWESSID  */
	(iw_handler) rt_ioctl_siwnickn,		    /* SIOCSIWNICKN  */
	(iw_handler) rt_ioctl_giwnickn,		    /* SIOCGIWNICKN  */
	(iw_handler) NULL,				        /* -- hole --    */
	(iw_handler) NULL,				        /* -- hole --    */
	(iw_handler) rt_ioctl_siwrate,          /* SIOCSIWRATE   */
	(iw_handler) rt_ioctl_giwrate,          /* SIOCGIWRATE   */
	(iw_handler) rt_ioctl_siwrts,		    /* SIOCSIWRTS    */
	(iw_handler) rt_ioctl_giwrts,		    /* SIOCGIWRTS    */
	(iw_handler) rt_ioctl_siwfrag,		    /* SIOCSIWFRAG   */
	(iw_handler) rt_ioctl_giwfrag,		    /* SIOCGIWFRAG   */
	(iw_handler) NULL,		                /* SIOCSIWTXPOW  */
	(iw_handler) NULL,		                /* SIOCGIWTXPOW  */
	(iw_handler) NULL,		                /* SIOCSIWRETRY  */
	(iw_handler) NULL,		                /* SIOCGIWRETRY  */
	(iw_handler) rt_ioctl_siwencode,		/* SIOCSIWENCODE */
	(iw_handler) rt_ioctl_giwencode,		/* SIOCGIWENCODE */
	(iw_handler) NULL,		                /* SIOCSIWPOWER  */
	(iw_handler) NULL,		                /* SIOCGIWPOWER  */
	(iw_handler) NULL,						/* -- hole -- */	
	(iw_handler) NULL,						/* -- hole -- */
#if WIRELESS_EXT > 17	
    (iw_handler) rt_ioctl_siwgenie,         /* SIOCSIWGENIE  */
	(iw_handler) rt_ioctl_giwgenie,         /* SIOCGIWGENIE  */
	(iw_handler) rt_ioctl_siwauth,		    /* SIOCSIWAUTH   */
	(iw_handler) rt_ioctl_giwauth,		    /* SIOCGIWAUTH   */
	(iw_handler) rt_ioctl_siwencodeext,	    /* SIOCSIWENCODEEXT */
	(iw_handler) rt_ioctl_giwencodeext,		/* SIOCGIWENCODEEXT */
	(iw_handler) rt_ioctl_siwpmksa,         /* SIOCSIWPMKSA  */
#endif
};

static const iw_handler rt_priv_handlers[] = {
	(iw_handler) NULL, /* + 0x00 */
	(iw_handler) NULL, /* + 0x01 */
#ifndef CONFIG_AP_SUPPORT
	(iw_handler) rt_ioctl_setparam, /* + 0x02 */
#else
	(iw_handler) NULL, /* + 0x02 */
#endif // CONFIG_AP_SUPPORT //
#ifdef DBG	
	(iw_handler) rt_private_ioctl_bbp, /* + 0x03 */	
#else
	(iw_handler) NULL, /* + 0x03 */
#endif
	(iw_handler) NULL, /* + 0x04 */
	(iw_handler) NULL, /* + 0x05 */
	(iw_handler) NULL, /* + 0x06 */
	(iw_handler) NULL, /* + 0x07 */
	(iw_handler) NULL, /* + 0x08 */
	(iw_handler) rt_private_get_statistics, /* + 0x09 */
	(iw_handler) NULL, /* + 0x0A */
	(iw_handler) NULL, /* + 0x0B */
	(iw_handler) NULL, /* + 0x0C */
	(iw_handler) NULL, /* + 0x0D */
	(iw_handler) NULL, /* + 0x0E */
	(iw_handler) NULL, /* + 0x0F */
	(iw_handler) NULL, /* + 0x10 */
	(iw_handler) rt_private_show, /* + 0x11 */
    (iw_handler) NULL, /* + 0x12 */
	(iw_handler) NULL, /* + 0x13 */
#ifdef WSC_STA_SUPPORT	
	(iw_handler) rt_private_set_wsc_u32_item, /* + 0x14 */
#else
    (iw_handler) NULL, /* + 0x14 */
#endif // WSC_STA_SUPPORT //
	(iw_handler) NULL, /* + 0x15 */
#ifdef WSC_STA_SUPPORT	
	(iw_handler) rt_private_set_wsc_string_item, /* + 0x16 */
#else
    (iw_handler) NULL, /* + 0x16 */
#endif // WSC_STA_SUPPORT //
	(iw_handler) NULL, /* + 0x17 */
	(iw_handler) NULL, /* + 0x18 */
};

const struct iw_handler_def rt28xx_iw_handler_def =
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
	.standard	= (iw_handler *) rt_handler,
	.num_standard	= sizeof(rt_handler) / sizeof(iw_handler),
	.private	= (iw_handler *) rt_priv_handlers,
	.num_private		= N(rt_priv_handlers),
	.private_args	= (struct iw_priv_args *) privtab,
	.num_private_args	= N(privtab),
#if IW_HANDLER_VERSION >= 7
    .get_wireless_stats = rt28xx_get_wireless_stats,
#endif 
};

INT RTMPSetInformation(
    IN  PRTMP_ADAPTER pAd,
    IN  OUT struct ifreq    *rq,
    IN  INT                 cmd)
{
    struct iwreq                        *wrq = (struct iwreq *) rq;
    NDIS_802_11_SSID                    Ssid;
    NDIS_802_11_MAC_ADDRESS             Bssid;
    RT_802_11_PHY_MODE                  PhyMode;
    RT_802_11_STA_CONFIG                StaConfig;
    NDIS_802_11_RATES                   aryRates;
    RT_802_11_PREAMBLE                  Preamble;
    NDIS_802_11_WEP_STATUS              WepStatus;
    NDIS_802_11_AUTHENTICATION_MODE     AuthMode = Ndis802_11AuthModeMax;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  BssType;
    NDIS_802_11_RTS_THRESHOLD           RtsThresh;
    NDIS_802_11_FRAGMENTATION_THRESHOLD FragThresh;
    NDIS_802_11_POWER_MODE              PowerMode;
    PNDIS_802_11_KEY                    pKey = NULL;
    PNDIS_802_11_WEP			        pWepKey =NULL;
    PNDIS_802_11_REMOVE_KEY             pRemoveKey = NULL;
    NDIS_802_11_CONFIGURATION           Config, *pConfig = NULL;
    NDIS_802_11_NETWORK_TYPE            NetType;
    ULONG                               Now;
    UINT                                KeyIdx = 0;
    INT                                 Status = NDIS_STATUS_SUCCESS, MaxPhyMode = PHY_11G;
    ULONG                               PowerTemp;
    BOOLEAN                             RadioState;
    BOOLEAN                             StateMachineTouched = FALSE;
     PNDIS_802_11_PASSPHRASE                    ppassphrase = NULL;     
#ifdef DOT11_N_SUPPORT
	OID_SET_HT_PHYMODE					HT_PhyMode;	//11n ,kathy
#endif // DOT11_N_SUPPORT //
#ifdef WPA_SUPPLICANT_SUPPORT    
    PNDIS_802_11_PMKID                  pPmkId = NULL;
    BOOLEAN				                IEEE8021xState = FALSE;
    BOOLEAN				                IEEE8021x_required_keys = FALSE;
    UCHAR                               wpa_supplicant_enable = 0;
#endif // WPA_SUPPLICANT_SUPPORT //

#ifdef SNMP_SUPPORT	
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						ctmp;
#endif // SNMP_SUPPORT //


#ifdef WAPI_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif // WAPI_SUPPORT //

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	UINT	WPSLedMode10 = 0;
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //

#ifdef DOT11_N_SUPPORT
	MaxPhyMode = PHY_11N_5G;
#endif // DOT11_N_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("-->RTMPSetInformation(),	0x%08x\n", cmd&0x7FFF));
	switch(cmd & 0x7FFF) {
		case RT_OID_802_11_COUNTRY_REGION:
			if (wrq->u.data.length < sizeof(UCHAR))
				Status = -EINVAL;
			// Only avaliable when EEPROM not programming
            else if (!(pAd->CommonCfg.CountryRegion & 0x80) && !(pAd->CommonCfg.CountryRegionForABand & 0x80))
			{
				ULONG   Country;
				UCHAR	TmpPhy;

				Status = copy_from_user(&Country, wrq->u.data.pointer, wrq->u.data.length);
				pAd->CommonCfg.CountryRegion = (UCHAR)(Country & 0x000000FF);
				pAd->CommonCfg.CountryRegionForABand = (UCHAR)((Country >> 8) & 0x000000FF);
                TmpPhy = pAd->CommonCfg.PhyMode;
				pAd->CommonCfg.PhyMode = 0xff;
				// Build all corresponding channel information
				RTMPSetPhyMode(pAd, TmpPhy);
#ifdef DOT11_N_SUPPORT
				SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_COUNTRY_REGION (A:%d  B/G:%d)\n", pAd->CommonCfg.CountryRegionForABand,
				    pAd->CommonCfg.CountryRegion));
            }
            break;
        case OID_802_11_BSSID_LIST_SCAN:
            Now = jiffies;
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_BSSID_LIST_SCAN, TxCnt = %d \n", pAd->RalinkCounters.LastOneSecTotalTxCount));

            if (MONITOR_ON(pAd))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
                break;
            }

			//Benson add 20080527, when radio off, sta don't need to scan
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
				break;
				
			if (pAd->RalinkCounters.LastOneSecTotalTxCount > 100)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
				Status = NDIS_STATUS_SUCCESS;
				break;
            }
            
            if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) &&
				((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || 
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) ||
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
                (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
				Status = NDIS_STATUS_SUCCESS;
				break;
            }

#ifdef WSC_STA_SUPPORT
			if ((pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
				(pAd->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP))
			{				
				DBGPRINT(RT_DEBUG_TRACE, ("!!! WPS processing now! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
				Status = NDIS_STATUS_SUCCESS;
				break;			
			}
#endif // WSC_STA_SUPPORT //

			StaSiteSurvey(pAd, NULL, SCAN_ACTIVE);
            break;
        case OID_802_11_SSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
                Status = -EINVAL;
            else
            {
            	PSTRING pSsidString = NULL;
                Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);

				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SSID (Len=%d,Ssid=%s)\n", Ssid.SsidLength, Ssid.Ssid));
                if (Ssid.SsidLength > MAX_LEN_OF_SSID)
                    Status = -EINVAL;
                else
                {
                	if (Ssid.SsidLength == 0)
		    		{
                		Set_SSID_Proc(pAd, "");
		    		}
					else
                    {
	                	pSsidString = (PSTRING)kmalloc(MAX_LEN_OF_SSID+1, MEM_ALLOC_FLAG);
						if (pSsidString)
						{
							NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID+1);
							NdisMoveMemory(pSsidString, Ssid.Ssid, Ssid.SsidLength);
							Set_SSID_Proc(pAd, pSsidString);
							kfree(pSsidString);
						}
						else
							Status = -ENOMEM;
                    }
                }
            }
            break;
		case OID_802_11_SET_PASSPHRASE:
    	    ppassphrase= kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);

    	    if(ppassphrase== NULL)
            {
               	Status = -ENOMEM;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_PASSPHRASE, Failed!!\n"));
               	break;
            }
			else
           	{
               	Status = copy_from_user(ppassphrase, wrq->u.data.pointer, wrq->u.data.length);

				if (Status)
            	{
                	Status  = -EINVAL;
                	DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_PASSPHRASE, Failed (length mismatch)!!\n"));
           		}
           		else
           		{
					if(ppassphrase->KeyLength < 8 || ppassphrase->KeyLength > 64)
					{
						Status  = -EINVAL;
                    	DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_PASSPHRASE, Failed (len less than 8 or greater than 64)!!\n"));
					}
					else
					{
	                    			// set key passphrase and length
	 					NdisZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
	    				NdisMoveMemory(pAd->StaCfg.WpaPassPhrase, &ppassphrase->KeyMaterial, ppassphrase->KeyLength);
	   					pAd->StaCfg.WpaPassPhraseLen = ppassphrase->KeyLength;
						hex_dump("pAd->StaCfg.WpaPassPhrase", pAd->StaCfg.WpaPassPhrase, 64);
						printk("WpaPassPhrase=%s\n",pAd->StaCfg.WpaPassPhrase);
					}
                }
            }
         	kfree(ppassphrase);
	   		break;
		
        case OID_802_11_BSSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_MAC_ADDRESS))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&Bssid, wrq->u.data.pointer, wrq->u.data.length);

                // tell CNTL state machine to call NdisMSetInformationComplete() after completing
                // this request, because this request is initiated by NDIS.
                pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 

				// Prevent to connect AP again in STAMlmePeriodicExec
				pAd->MlmeAux.AutoReconnectSsidLen= 32;

                if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
                {
                    RTMP_MLME_RESET_STATE_MACHINE(pAd);
                    DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
                }
                MlmeEnqueue(pAd, 
                            MLME_CNTL_STATE_MACHINE, 
                            OID_802_11_BSSID, 
                            sizeof(NDIS_802_11_MAC_ADDRESS),
                            (VOID *)&Bssid, 0);
                Status = NDIS_STATUS_SUCCESS;
                StateMachineTouched = TRUE;

                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
            }
            break;
        case RT_OID_802_11_RADIO:
            if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&RadioState, wrq->u.data.pointer, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_RADIO (=%d)\n", RadioState));
                if (pAd->StaCfg.bSwRadio != RadioState)
                {
                    pAd->StaCfg.bSwRadio = RadioState;
                    if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
                    {
                        pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                        if (pAd->StaCfg.bRadio == TRUE)
                        {
                            MlmeRadioOn(pAd);
                            // Update extra information
							pAd->ExtraInfo = EXTRA_INFO_CLEAR;
                        }
                        else
                        {
                        	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				            {
				                if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
						        {
						            RTMP_MLME_RESET_STATE_MACHINE(pAd);
						            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
						        }
				            }
							
                            MlmeRadioOff(pAd);
                            // Update extra information
							pAd->ExtraInfo = SW_RADIO_OFF;
                        }
                    }
                }
            }
            break;
        case RT_OID_802_11_PHY_MODE:
            if (wrq->u.data.length != sizeof(RT_802_11_PHY_MODE))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&PhyMode, wrq->u.data.pointer, wrq->u.data.length);
				if (PhyMode <= MaxPhyMode)
				{
					pAd->CommonCfg.DesiredPhyMode = PhyMode;
                	RTMPSetPhyMode(pAd, PhyMode);
#ifdef DOT11_N_SUPPORT
					SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
				}
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_PHY_MODE (=%d)\n", PhyMode));
            }
            break;
        case RT_OID_802_11_STA_CONFIG:
            if (wrq->u.data.length != sizeof(RT_802_11_STA_CONFIG))
                Status  = -EINVAL;
            else
            {
            	UINT32	Value;
				
                Status = copy_from_user(&StaConfig, wrq->u.data.pointer, wrq->u.data.length);
                pAd->CommonCfg.bEnableTxBurst = StaConfig.EnableTxBurst;
                pAd->CommonCfg.UseBGProtection = StaConfig.UseBGProtection;
                pAd->CommonCfg.bUseShortSlotTime = 1; // 2003-10-30 always SHORT SLOT capable
                if ((pAd->CommonCfg.PhyMode != StaConfig.AdhocMode) &&
					(StaConfig.AdhocMode <= MaxPhyMode))
                {
                    // allow dynamic change of "USE OFDM rate or not" in ADHOC mode
                    // if setting changed, need to reset current TX rate as well as BEACON frame format
                    if (pAd->StaCfg.BssType == BSS_ADHOC)
                    {
                    	pAd->CommonCfg.PhyMode = StaConfig.AdhocMode;
                    	RTMPSetPhyMode(pAd, PhyMode);
                        MlmeUpdateTxRates(pAd, FALSE, 0);
                        MakeIbssBeacon(pAd);           // re-build BEACON frame
                        AsicEnableIbssSync(pAd);   // copy to on-chip memory
                    }
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_STA_CONFIG (Burst=%d, Protection=%ld,ShortSlot=%d\n",
                                        pAd->CommonCfg.bEnableTxBurst,
                                        pAd->CommonCfg.UseBGProtection,
                                        pAd->CommonCfg.bUseShortSlotTime));

#ifdef XLINK_SUPPORT
				if (pAd->StaCfg.PSPXlink)
					Value = PSPXLINK;
				else
#endif // XLINK_SUPPORT //
					Value = STANORMAL;
				RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, Value);
            }
            break;
        case OID_802_11_DESIRED_RATES:
            if (wrq->u.data.length != sizeof(NDIS_802_11_RATES))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&aryRates, wrq->u.data.pointer, wrq->u.data.length);
                NdisZeroMemory(pAd->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
                NdisMoveMemory(pAd->CommonCfg.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DESIRED_RATES (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
                    pAd->CommonCfg.DesireRate[0],pAd->CommonCfg.DesireRate[1],
                    pAd->CommonCfg.DesireRate[2],pAd->CommonCfg.DesireRate[3],
                    pAd->CommonCfg.DesireRate[4],pAd->CommonCfg.DesireRate[5],
                    pAd->CommonCfg.DesireRate[6],pAd->CommonCfg.DesireRate[7] ));
                // Changing DesiredRate may affect the MAX TX rate we used to TX frames out
                MlmeUpdateTxRates(pAd, FALSE, 0);
            }
            break;
        case RT_OID_802_11_PREAMBLE:
            if (wrq->u.data.length != sizeof(RT_802_11_PREAMBLE))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&Preamble, wrq->u.data.pointer, wrq->u.data.length);
                if (Preamble == Rt802_11PreambleShort)
                {
                    pAd->CommonCfg.TxPreamble = Preamble;
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
                }
                else if ((Preamble == Rt802_11PreambleLong) || (Preamble == Rt802_11PreambleAuto))
                {
                    // if user wants AUTO, initialize to LONG here, then change according to AP's
                    // capability upon association.
                    pAd->CommonCfg.TxPreamble = Preamble;
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
                }
                else
                {
                    Status = -EINVAL;
                    break;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_PREAMBLE (=%d)\n", Preamble));
            }
            break;
        case OID_802_11_WEP_STATUS:
            if (wrq->u.data.length != sizeof(NDIS_802_11_WEP_STATUS))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&WepStatus, wrq->u.data.pointer, wrq->u.data.length);
                // Since TKIP, AES, WEP are all supported. It should not have any invalid setting
                if (WepStatus <= Ndis802_11Encryption3KeyAbsent)
                {
                    if (pAd->StaCfg.WepStatus != WepStatus)
                    {
                        // Config has changed
                        pAd->bConfigChanged = TRUE;
                    }
                    pAd->StaCfg.WepStatus     = WepStatus;
                    pAd->StaCfg.PairCipher    = WepStatus;
                	pAd->StaCfg.GroupCipher   = WepStatus;

					if (pAd->StaCfg.BssType == BSS_ADHOC)
					{
						// Build all corresponding channel information
						RTMPSetPhyMode(pAd, pAd->CommonCfg.DesiredPhyMode);
#ifdef DOT11_N_SUPPORT
					SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
					}
                }
                else
                {
                    Status  = -EINVAL;
                    break;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEP_STATUS (=%d)\n",WepStatus));
            }
            break;
        case OID_802_11_AUTHENTICATION_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_AUTHENTICATION_MODE)) 
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&AuthMode, wrq->u.data.pointer, wrq->u.data.length);
                if (AuthMode > Ndis802_11AuthModeMax)
                {
                    Status  = -EINVAL;
                    break;
                }
                else
                {
                    if (pAd->StaCfg.AuthMode != AuthMode)
                    {
                        // Config has changed
                        pAd->bConfigChanged = TRUE;
                    }
                    pAd->StaCfg.AuthMode = AuthMode;
                }
                pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_AUTHENTICATION_MODE (=%d) \n",pAd->StaCfg.AuthMode));
            }
            break;
        case OID_802_11_INFRASTRUCTURE_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&BssType, wrq->u.data.pointer, wrq->u.data.length);
							
				if (BssType == Ndis802_11IBSS)
					Set_NetworkType_Proc(pAd, "Adhoc");
				else if (BssType == Ndis802_11Infrastructure)
					Set_NetworkType_Proc(pAd, "Infra");
                else if (BssType == Ndis802_11Monitor)
					Set_NetworkType_Proc(pAd, "Monitor");
                else
                {
                    Status  = -EINVAL;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_INFRASTRUCTURE_MODE (unknown)\n"));
                }
            }
            break;
	 case OID_802_11_REMOVE_WEP:
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_WEP\n"));
            if (wrq->u.data.length != sizeof(NDIS_802_11_KEY_INDEX))
            {
				Status = -EINVAL;
            }
            else 
            {
		KeyIdx = *(NDIS_802_11_KEY_INDEX *) wrq->u.data.pointer;

		if (KeyIdx & 0x80000000)
		{
			// Should never set default bit when remove key
			Status = -EINVAL;
		}
		else
		{
			KeyIdx = KeyIdx & 0x0fffffff;
			if (KeyIdx >= 4){
				Status = -EINVAL;
			}
			else
			{
						pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
						pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
						AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)KeyIdx);
			}
		}
            }
            break;
        case RT_OID_802_11_RESET_COUNTERS:
            NdisZeroMemory(&pAd->WlanCounters, sizeof(COUNTER_802_11));
            NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
            NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));
            pAd->Counters8023.RxNoBuffer   = 0;
			pAd->Counters8023.GoodReceives = 0;
			pAd->Counters8023.RxNoBuffer   = 0;
#ifdef TXBF_SUPPORT
		{
			int i;
			for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
				NdisZeroMemory(&pAd->MacTab.Content[i].TxBFCounters, sizeof(pAd->MacTab.Content[i].TxBFCounters));
		}
#endif // TXBF_SUPPORT //

            DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_RESET_COUNTERS \n"));
            break;
        case OID_802_11_RTS_THRESHOLD:
            if (wrq->u.data.length != sizeof(NDIS_802_11_RTS_THRESHOLD))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&RtsThresh, wrq->u.data.pointer, wrq->u.data.length);
                if (RtsThresh > MAX_RTS_THRESHOLD)
                    Status  = -EINVAL;
                else
                    pAd->CommonCfg.RtsThreshold = (USHORT)RtsThresh;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_RTS_THRESHOLD (=%ld)\n",RtsThresh));
            break;
        case OID_802_11_FRAGMENTATION_THRESHOLD:
            if (wrq->u.data.length != sizeof(NDIS_802_11_FRAGMENTATION_THRESHOLD))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&FragThresh, wrq->u.data.pointer, wrq->u.data.length);
                pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
                if (FragThresh > MAX_FRAG_THRESHOLD || FragThresh < MIN_FRAG_THRESHOLD)
                {
                    if (FragThresh == 0)
                    {
                        pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
                        pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
                    }
                    else
                        Status  = -EINVAL;
                }
                else
                    pAd->CommonCfg.FragmentThreshold = (USHORT)FragThresh;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_FRAGMENTATION_THRESHOLD (=%ld) \n",FragThresh));
            break;
        case OID_802_11_POWER_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_POWER_MODE))
                Status = -EINVAL;
            else
            {
                Status = copy_from_user(&PowerMode, wrq->u.data.pointer, wrq->u.data.length);
                if (PowerMode == Ndis802_11PowerModeCAM) 
                	Set_PSMode_Proc(pAd, "CAM");
                else if (PowerMode == Ndis802_11PowerModeMAX_PSP) 
                	Set_PSMode_Proc(pAd, "Max_PSP");
                else if (PowerMode == Ndis802_11PowerModeFast_PSP) 
					Set_PSMode_Proc(pAd, "Fast_PSP");
                else if (PowerMode == Ndis802_11PowerModeLegacy_PSP) 
					Set_PSMode_Proc(pAd, "Legacy_PSP");
                else
                    Status = -EINVAL;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_POWER_MODE (=%d)\n",PowerMode));
            break;
         case RT_OID_802_11_TX_POWER_LEVEL_1:
			if (wrq->u.data.length  < sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&PowerTemp, wrq->u.data.pointer, wrq->u.data.length);
				if (PowerTemp > 100)
					PowerTemp = 0xffffffff;  // AUTO
				pAd->CommonCfg.TxPowerDefault = PowerTemp; //keep current setting.
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;			
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_TX_POWER_LEVEL_1 (=%ld)\n", pAd->CommonCfg.TxPowerPercentage));
			}	
	        break;
		case OID_802_11_NETWORK_TYPE_IN_USE: 
			if (wrq->u.data.length != sizeof(NDIS_802_11_NETWORK_TYPE))
				Status = -EINVAL;
			else 
			{
				Status = copy_from_user(&NetType, wrq->u.data.pointer, wrq->u.data.length);

				if (NetType == Ndis802_11DS)
					RTMPSetPhyMode(pAd, PHY_11B);
				else if (NetType == Ndis802_11OFDM24)
					RTMPSetPhyMode(pAd, PHY_11BG_MIXED);
				else if (NetType == Ndis802_11OFDM5)
					RTMPSetPhyMode(pAd, PHY_11A);
				else 
					Status = -EINVAL;
#ifdef DOT11_N_SUPPORT
				if (Status == NDIS_STATUS_SUCCESS)
					SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_NETWORK_TYPE_IN_USE (=%d)\n",NetType));
		    }	    
			break;
        // For WPA PSK PMK key
        case RT_OID_802_11_ADD_WPA:
            pKey = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);
            if(pKey == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            
            Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
            if (pKey->Length != wrq->u.data.length)
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA, Failed!!\n"));
            }
            else
            {
                if ((pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPAPSK) &&
				    (pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPA2PSK) &&
				    (pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPANone) )
                {
                    Status = -EOPNOTSUPP;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA, Failed!! [AuthMode != WPAPSK/WPA2PSK/WPANONE]\n"));
                }
                else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
						 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
						 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone) )     // Only for WPA PSK mode
				{		
                    NdisMoveMemory(pAd->StaCfg.PMK, &pKey->KeyMaterial, pKey->KeyLength);
                    // Use RaConfig as PSK agent.
                    // Start STA supplicant state machine
                    if (pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPANone)
                        pAd->StaCfg.WpaState = SS_START;

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA (id=0x%x, Len=%d-byte)\n", pKey->KeyIndex, pKey->KeyLength));
                }
                else
                {   
                    pAd->StaCfg.WpaState = SS_NOTUSE;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA (id=0x%x, Len=%d-byte)\n", pKey->KeyIndex, pKey->KeyLength));
                }
            }
            kfree(pKey);
            break;
        case OID_802_11_REMOVE_KEY:
            pRemoveKey = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);
            if(pRemoveKey == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            
            Status = copy_from_user(pRemoveKey, wrq->u.data.pointer, wrq->u.data.length);
            if (pRemoveKey->Length != wrq->u.data.length)
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Failed!!\n"));
            }
            else
            {
                if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
                {
                    RTMPWPARemoveKeyProc(pAd, pRemoveKey);
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Remove WPA Key!!\n"));
                }
                else
                {
                    KeyIdx = pRemoveKey->KeyIndex;

                    if (KeyIdx & 0x80000000)
                    {
                        // Should never set default bit when remove key
                        Status  = -EINVAL;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Failed!!(Should never set default bit when remove key)\n"));
                    }
                    else
                    {
                        KeyIdx = KeyIdx & 0x0fffffff;
                        if (KeyIdx > 3)
                        {
                            Status  = -EINVAL;
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Failed!!(KeyId[%d] out of range)\n", KeyIdx));
                        }
                        else
                        {
                            pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
                            pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
                            AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)KeyIdx); 
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY (id=0x%x, Len=%d-byte)\n", pRemoveKey->KeyIndex, pRemoveKey->Length));
                        }
                    }
                }
            }
            kfree(pRemoveKey);
            break;        
        // New for WPA
        case OID_802_11_ADD_KEY:
            pKey = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);
            if(pKey == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
            if (pKey->Length != wrq->u.data.length)
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_KEY, Failed!!\n"));
            }
            else
            {
                RTMPAddKey(pAd, pKey);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_KEY (id=0x%x, Len=%d-byte)\n", pKey->KeyIndex, pKey->KeyLength));
            }
            kfree(pKey);
            break;
        case OID_802_11_CONFIGURATION:
            if (wrq->u.data.length != sizeof(NDIS_802_11_CONFIGURATION))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&Config, wrq->u.data.pointer, wrq->u.data.length);
                pConfig = &Config;

                if ((pConfig->BeaconPeriod >= 20) && (pConfig->BeaconPeriod <=400))
                     pAd->CommonCfg.BeaconPeriod = (USHORT) pConfig->BeaconPeriod;
                
                pAd->StaActive.AtimWin = (USHORT) pConfig->ATIMWindow;
                MAP_KHZ_TO_CHANNEL_ID(pConfig->DSConfig, pAd->CommonCfg.Channel);
                //
				// Save the channel on MlmeAux for CntlOidRTBssidProc used.
				//
				pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
				
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_CONFIGURATION (BeacnPeriod=%ld,AtimW=%ld,Ch=%d)\n",
                    pConfig->BeaconPeriod, pConfig->ATIMWindow, pAd->CommonCfg.Channel));
                // Config has changed
                pAd->bConfigChanged = TRUE;
            }
            break;
#ifdef DOT11_N_SUPPORT
		case RT_OID_802_11_SET_HT_PHYMODE:
			if (wrq->u.data.length	!= sizeof(OID_SET_HT_PHYMODE))
				Status = -EINVAL;
			else 
			{
			    POID_SET_HT_PHYMODE	pHTPhyMode = &HT_PhyMode;
                
				Status = copy_from_user(&HT_PhyMode, wrq->u.data.pointer, wrq->u.data.length);				
				DBGPRINT(RT_DEBUG_TRACE, ("Set::pHTPhyMode	(PhyMode = %d,TransmitNo = %d, HtMode =	%d,	ExtOffset =	%d , MCS = %d, BW =	%d,	STBC = %d, SHORTGI = %d) \n", 
				pHTPhyMode->PhyMode, pHTPhyMode->TransmitNo,pHTPhyMode->HtMode,pHTPhyMode->ExtOffset,
				pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC,	pHTPhyMode->SHORTGI));
				if (pAd->CommonCfg.PhyMode	>= PHY_11ABGN_MIXED)
					RTMPSetHT(pAd,	pHTPhyMode);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_HT_PHYMODE(MCS=%d,BW=%d,SGI=%d,STBC=%d)\n", 
				pAd->StaCfg.HTPhyMode.field.MCS, pAd->StaCfg.HTPhyMode.field.BW, pAd->StaCfg.HTPhyMode.field.ShortGI,
				pAd->StaCfg.HTPhyMode.field.STBC));
			break;
#endif // DOT11_N_SUPPORT //
		case RT_OID_802_11_SET_APSD_SETTING:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				ULONG apsd ;
				Status = copy_from_user(&apsd, wrq->u.data.pointer,	wrq->u.data.length);

				/*-------------------------------------------------------------------
				|B31~B7	|	B6~B5	 |	 B4	 |	 B3	 |	B2	 |	B1	 |	   B0		|
				---------------------------------------------------------------------
				| Rsvd	| Max SP Len | AC_VO | AC_VI | AC_BK | AC_BE | APSD	Capable	|
				---------------------------------------------------------------------*/
				pAd->CommonCfg.bAPSDCapable = (apsd & 0x00000001) ? TRUE :	FALSE;
				pAd->CommonCfg.bAPSDAC_BE = ((apsd	& 0x00000002) >> 1)	? TRUE : FALSE;
				pAd->CommonCfg.bAPSDAC_BK = ((apsd	& 0x00000004) >> 2)	? TRUE : FALSE;
				pAd->CommonCfg.bAPSDAC_VI = ((apsd	& 0x00000008) >> 3)	? TRUE : FALSE;
				pAd->CommonCfg.bAPSDAC_VO = ((apsd	& 0x00000010) >> 4)	? TRUE : FALSE;
				pAd->CommonCfg.MaxSPLength	= (UCHAR)((apsd	& 0x00000060) >> 5);

				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_APSD_SETTING (apsd=0x%lx, APSDCap=%d, [BE,BK,VI,VO]=[%d/%d/%d/%d],	MaxSPLen=%d)\n", apsd, pAd->CommonCfg.bAPSDCapable,
					pAd->CommonCfg.bAPSDAC_BE,	pAd->CommonCfg.bAPSDAC_BK,	pAd->CommonCfg.bAPSDAC_VI,	pAd->CommonCfg.bAPSDAC_VO,	pAd->CommonCfg.MaxSPLength));
			}
			break;

		case RT_OID_802_11_SET_APSD_PSM:
			if (wrq->u.data.length	!= sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				// Driver needs to notify AP when PSM changes
				Status = copy_from_user(&pAd->CommonCfg.bAPSDForcePowerSave, wrq->u.data.pointer, wrq->u.data.length);
				if (pAd->CommonCfg.bAPSDForcePowerSave	!= pAd->StaCfg.Psm)
				{
					RTMP_SET_PSM_BIT(pAd,	pAd->CommonCfg.bAPSDForcePowerSave);
					RTMPSendNullFrame(pAd,	pAd->CommonCfg.TxRate,	TRUE);
				}
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_APSD_PSM (bAPSDForcePowerSave:%d)\n",	pAd->CommonCfg.bAPSDForcePowerSave));
			}
			break;
#ifdef QOS_DLS_SUPPORT
		case RT_OID_802_11_SET_DLS:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				BOOLEAN	oldvalue = pAd->CommonCfg.bDLSCapable;
				Status = copy_from_user(&pAd->CommonCfg.bDLSCapable, wrq->u.data.pointer, wrq->u.data.length);
				if (oldvalue &&	!pAd->CommonCfg.bDLSCapable)
				{
					int	i;
					// tear	down local dls table entry
					for	(i=0; i<MAX_NUM_OF_INIT_DLS_ENTRY; i++)
					{
						if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
						{
							pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
							pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
							RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
						}
					}

					// tear	down peer dls table	entry
					for	(i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
					{
						if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
						{
							pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
							pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
							RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
						}
					}
				}

				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_DLS (=%d)\n", pAd->CommonCfg.bDLSCapable));
			}
			break;

		case RT_OID_802_11_SET_DLS_PARAM:
			if (wrq->u.data.length	!= sizeof(RT_802_11_DLS_UI))
				Status = -EINVAL;
			else 
			{
				RT_802_11_DLS	Dls;

				NdisZeroMemory(&Dls, sizeof(RT_802_11_DLS));
				RTMPMoveMemory(&Dls, wrq->u.data.pointer, sizeof(RT_802_11_DLS_UI));
				MlmeEnqueue(pAd, 
							MLME_CNTL_STATE_MACHINE, 
							RT_OID_802_11_SET_DLS_PARAM, 
							sizeof(RT_802_11_DLS), 
							&Dls, 0);
				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_DLS_PARAM \n"));
			}
			break;
#endif // QOS_DLS_SUPPORT //

#ifdef DOT11Z_TDLS_SUPPORT
		case RT_OID_802_11_SET_TDLS:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				BOOLEAN	oldvalue = pAd->StaCfg.bTDLSCapable;

				Status = copy_from_user(&pAd->StaCfg.bTDLSCapable, wrq->u.data.pointer, wrq->u.data.length);
				if (oldvalue &&	!pAd->StaCfg.bTDLSCapable)
				{
					// tear	down local dls table entry
					TDLS_LinkTearDown(pAd);
					TDLS_SearchTabReset(pAd);
				}

				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_DLS (=%d)\n", pAd->CommonCfg.bDLSCapable));
			}
			break;

		case RT_OID_802_11_SET_TDLS_PARAM:
			if (wrq->u.data.length	!= sizeof(RT_802_11_TDLS_UI))
				Status = -EINVAL;
			else 
			{
				RT_802_11_TDLS		TDLS;

				// Initialized mlme request
				RTMPZeroMemory(&TDLS, sizeof(RT_802_11_TDLS));
				RTMPMoveMemory(&TDLS, wrq->u.data.pointer, sizeof(RT_802_11_TDLS_UI));				

				MlmeEnqueue(pAd, 
							MLME_CNTL_STATE_MACHINE, 
							RT_OID_802_11_SET_TDLS_PARAM, 
							sizeof(RT_802_11_TDLS), 
							&TDLS, 0);

				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_TDLS_PARAM \n"));
			}
			break;
#endif // DOT11Z_TDLS_SUPPORT //

		case RT_OID_802_11_SET_WMM:
			if (wrq->u.data.length	!= sizeof(BOOLEAN))
				Status = -EINVAL;
			else 
			{
				Status = copy_from_user(&pAd->CommonCfg.bWmmCapable, wrq->u.data.pointer, wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_WMM (=%d)	\n", pAd->CommonCfg.bWmmCapable));
			}
			break;

		case OID_802_11_DISASSOCIATE:
			//
			// Set NdisRadioStateOff to	TRUE, instead of called	MlmeRadioOff.
			// Later on, NDIS_802_11_BSSID_LIST_EX->NumberOfItems should be	0 
			// when	query OID_802_11_BSSID_LIST.
			//
			// TRUE:  NumberOfItems	will set to	0.
			// FALSE: NumberOfItems	no change.
			//			
			pAd->CommonCfg.NdisRadioStateOff =	TRUE;
			// Set to immediately send the media disconnect	event
			pAd->MlmeAux.CurrReqIsFromNdis	= TRUE;			
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DISASSOCIATE	\n"));

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
			if (pAd->StaCfg.WscControl.bSkipWPSTurnOffLED == FALSE)
			{
				UCHAR WPSLEDStatus = LED_WPS_TURN_LED_OFF;
				BOOLEAN Cancelled;

#ifdef RTMP_MAC_PCI
				RTMPSetLED(pAd, WPSLEDStatus);
#endif // RTMP_MAC_PCI //

				// Cancel the WPS LED timer.
				RTMPCancelTimer(&pAd->StaCfg.WscControl.WscLEDTimer, &Cancelled);
			}
#endif // WSC_LED_SUPPORT //
#endif // WSC_STA_SUPPORT //

			if (INFRA_ON(pAd))
			{
				if (pAd->Mlme.CntlMachine.CurrState !=	CNTL_IDLE)
				{
					RTMP_MLME_RESET_STATE_MACHINE(pAd);
					DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME	busy, reset	MLME state machine !!!\n"));
				}

				MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE,
					OID_802_11_DISASSOCIATE,
					0,
					NULL, 0);

				StateMachineTouched	= TRUE;
			}
			break;

#ifdef DOT11_N_SUPPORT
		case RT_OID_802_11_SET_IMME_BA_CAP:
				if (wrq->u.data.length != sizeof(OID_BACAP_STRUC))
					Status = -EINVAL;
				else
				{
					OID_BACAP_STRUC Orde ;
					Status = copy_from_user(&Orde, wrq->u.data.pointer, wrq->u.data.length);
					if (Orde.Policy > BA_NOTUSE)
					{
						Status = NDIS_STATUS_INVALID_DATA;
					}
					else if (Orde.Policy == BA_NOTUSE)
					{
						pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
						pAd->CommonCfg.BACapability.field.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.AmsduEnable = Orde.AmsduEnable;
						pAd->CommonCfg.DesiredHtPhy.AmsduSize= Orde.AmsduSize;
						pAd->CommonCfg.DesiredHtPhy.MimoPs= Orde.MMPSmode;
						pAd->CommonCfg.BACapability.field.MMPSmode = Orde.MMPSmode;
						// UPdata to HT IE
						pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = Orde.MMPSmode;
						pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = Orde.AmsduSize;
						pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = Orde.MpduDensity;
					}
					else
					{
                        pAd->CommonCfg.BACapability.field.AutoBA = Orde.AutoBA;
						pAd->CommonCfg.BACapability.field.Policy = IMMED_BA; // we only support immediate BA.
						pAd->CommonCfg.BACapability.field.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.AmsduEnable = Orde.AmsduEnable;
						pAd->CommonCfg.DesiredHtPhy.AmsduSize= Orde.AmsduSize;
						pAd->CommonCfg.DesiredHtPhy.MimoPs = Orde.MMPSmode;
						pAd->CommonCfg.BACapability.field.MMPSmode = Orde.MMPSmode;
							
						// UPdata to HT IE
						pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = Orde.MMPSmode;
						pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = Orde.AmsduSize;
						pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = Orde.MpduDensity;
						
						if (pAd->CommonCfg.BACapability.field.RxBAWinLimit > MAX_RX_REORDERBUF)
							pAd->CommonCfg.BACapability.field.RxBAWinLimit = MAX_RX_REORDERBUF;

					}

					pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
					DBGPRINT(RT_DEBUG_TRACE, ("Set::(Orde.AutoBA = %d) (Policy=%d)(ReBAWinLimit=%d)(TxBAWinLimit=%d)(AutoMode=%d)\n",Orde.AutoBA, pAd->CommonCfg.BACapability.field.Policy,
						pAd->CommonCfg.BACapability.field.RxBAWinLimit,pAd->CommonCfg.BACapability.field.TxBAWinLimit, pAd->CommonCfg.BACapability.field.AutoBA));
					DBGPRINT(RT_DEBUG_TRACE, ("Set::(MimoPs = %d)(AmsduEnable = %d) (AmsduSize=%d)(MpduDensity=%d)\n",pAd->CommonCfg.DesiredHtPhy.MimoPs, pAd->CommonCfg.DesiredHtPhy.AmsduEnable,
						pAd->CommonCfg.DesiredHtPhy.AmsduSize, pAd->CommonCfg.DesiredHtPhy.MpduDensity));
				}

				break;
		case RT_OID_802_11_ADD_IMME_BA:
			DBGPRINT(RT_DEBUG_TRACE, (" Set :: RT_OID_802_11_ADD_IMME_BA \n"));
			if (wrq->u.data.length != sizeof(OID_ADD_BA_ENTRY))
					Status = -EINVAL;
			else 
			{
				UCHAR		        index;
				OID_ADD_BA_ENTRY    BA;
				MAC_TABLE_ENTRY     *pEntry;

				Status = copy_from_user(&BA, wrq->u.data.pointer, wrq->u.data.length);
				if (BA.TID > 15)
				{
					Status = NDIS_STATUS_INVALID_DATA;
					break;
				}
				else
				{
					//BATableInsertEntry
					//As ad-hoc mode, BA pair is not limited to only BSSID. so add via OID. 
					index = BA.TID;
					// in ad hoc mode, when adding BA pair, we should insert this entry into MACEntry too
					pEntry = MacTableLookup(pAd, BA.MACAddr);
					if (!pEntry)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_802_11_ADD_IMME_BA. break on no connection.----:%x:%x\n", BA.MACAddr[4], BA.MACAddr[5]));
						break;
					}
					if (BA.IsRecipient == FALSE)
					{
					    if (pEntry->bIAmBadAtheros == TRUE)
							pAd->CommonCfg.BACapability.field.RxBAWinLimit = 0x10;

						BAOriSessionSetUp(pAd, pEntry, index, 0, 100, TRUE);
					}
					else 
					{
						//BATableInsertEntry(pAd, pEntry->Aid, BA.MACAddr, 0, 0xffff, BA.TID, BA.nMSDU, BA.IsRecipient);
					}

					DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_IMME_BA. Rec = %d. Mac = %x:%x:%x:%x:%x:%x . \n",
						BA.IsRecipient, BA.MACAddr[0], BA.MACAddr[1], BA.MACAddr[2], BA.MACAddr[2]
						, BA.MACAddr[4], BA.MACAddr[5]));
				}
			}
			break;

		case RT_OID_802_11_TEAR_IMME_BA:
			DBGPRINT(RT_DEBUG_TRACE, ("Set :: RT_OID_802_11_TEAR_IMME_BA \n"));
			if (wrq->u.data.length != sizeof(OID_ADD_BA_ENTRY))
					Status = -EINVAL;
			else 
			{
				POID_ADD_BA_ENTRY	pBA;
				MAC_TABLE_ENTRY *pEntry;
				
				pBA = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);

				if (pBA == NULL)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Set :: RT_OID_802_11_TEAR_IMME_BA kmalloc() can't allocate enough memory\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				else
				{
					Status = copy_from_user(pBA, wrq->u.data.pointer, wrq->u.data.length);
					DBGPRINT(RT_DEBUG_TRACE, ("Set :: RT_OID_802_11_TEAR_IMME_BA(TID=%d, bAllTid=%d)\n", pBA->TID, pBA->bAllTid));
					
					if (!pBA->bAllTid && (pBA->TID > NUM_OF_TID))
					{
						Status = NDIS_STATUS_INVALID_DATA;
						break;
					}
					
					if (pBA->IsRecipient == FALSE)
					{
						pEntry = MacTableLookup(pAd, pBA->MACAddr);
						DBGPRINT(RT_DEBUG_TRACE, (" pBA->IsRecipient == FALSE\n"));
						if (pEntry)
						{
							DBGPRINT(RT_DEBUG_TRACE, (" pBA->pEntry\n"));
							BAOriSessionTearDown(pAd, pEntry->Aid, pBA->TID, FALSE, TRUE);
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("Set :: Not found pEntry \n"));
					}
					else
					{
						pEntry = MacTableLookup(pAd, pBA->MACAddr);
						if (pEntry)
						{
							BARecSessionTearDown( pAd, (UCHAR)pEntry->Aid, pBA->TID, TRUE);
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("Set :: Not found pEntry \n"));
					}
					kfree(pBA);
				}
            }
            break;
#endif // DOT11_N_SUPPORT //

        // For WPA_SUPPLICANT to set static wep key	  
    	case OID_802_11_ADD_WEP:
    	    pWepKey = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);

    	    if(pWepKey == NULL)
            {
                Status = -ENOMEM;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, Failed!!\n"));
                break;
            }
            Status = copy_from_user(pWepKey, wrq->u.data.pointer, wrq->u.data.length);
            if (Status)
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, Failed (length mismatch)!!\n"));
            }
            else
            {
		        KeyIdx = pWepKey->KeyIndex & 0x0fffffff;
                // KeyIdx must be 0 ~ 3
                if (KeyIdx > 4)
    			{
                    Status  = -EINVAL;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, Failed (KeyIdx must be smaller than 4)!!\n"));
                }
                else
                {
                    UCHAR CipherAlg = 0;
                    PUCHAR Key;

					// Zero the specific shared key
					NdisZeroMemory(&pAd->SharedKey[BSS0][KeyIdx], sizeof(CIPHER_KEY));

                    // set key material and key length
                    pAd->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pWepKey->KeyLength;
                    NdisMoveMemory(pAd->SharedKey[BSS0][KeyIdx].Key, &pWepKey->KeyMaterial, pWepKey->KeyLength);

                    switch(pWepKey->KeyLength)
                    {
                        case 5:
                            CipherAlg = CIPHER_WEP64;
                            break;
                        case 13:
                            CipherAlg = CIPHER_WEP128;
                            break;
                        default:
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, only support CIPHER_WEP64(len:5) & CIPHER_WEP128(len:13)!!\n"));
                            Status = -EINVAL;
                            break;
                    }
                    pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CipherAlg;

                    // Default key for tx (shared key)
                    if (pWepKey->KeyIndex & 0x80000000)
                    {
#ifdef WPA_SUPPLICANT_SUPPORT
                        NdisZeroMemory(&pAd->StaCfg.DesireSharedKey[KeyIdx], sizeof(CIPHER_KEY));
								
                        // set key material and key length
                        pAd->StaCfg.DesireSharedKey[KeyIdx].KeyLen = (UCHAR) pWepKey->KeyLength;
                        NdisMoveMemory(pAd->StaCfg.DesireSharedKey[KeyIdx].Key, &pWepKey->KeyMaterial, pWepKey->KeyLength);
                        pAd->StaCfg.DesireSharedKeyId = KeyIdx;
                        pAd->StaCfg.DesireSharedKey[KeyIdx].CipherAlg = CipherAlg;
#endif // WPA_SUPPLICANT_SUPPORT //                    
                        pAd->StaCfg.DefaultKeyId = (UCHAR) KeyIdx;
                    }
                    
#ifdef WPA_SUPPLICANT_SUPPORT
					if ((pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
						(pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA))
					{
						Key = pWepKey->KeyMaterial;
						
						// Set Group key material to Asic
    					AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);
						
						/* STA doesn't need to set WCID attribute for group key */
						STA_PORT_SECURED(pAd);
						
        				// Indicate Connected for GUI
        				pAd->IndicateMediaState = NdisMediaStateConnected;
					}
                    else if (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_SECURED)
#endif // WPA_SUPPLICANT_SUPPORT
                    {
                        Key = pAd->SharedKey[BSS0][KeyIdx].Key;

                        // Set key material and cipherAlg to Asic
        				AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);	
                        
                        if (pWepKey->KeyIndex & 0x80000000)
                        {
							/* STA doesn't need to set WCID attribute for group key */
    						// Assign pairwise key info
    						RTMPSetWcidSecurityInfo(pAd, 
												 BSS0, 
												 KeyIdx, 
												 CipherAlg, 												 
												 BSSID_WCID, 
												 SHAREDKEYTABLE);
                        }
                    }
					DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP (id=0x%x, Len=%d-byte), %s\n", pWepKey->KeyIndex, pWepKey->KeyLength, (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_SECURED) ? "Port Secured":"Port NOT Secured"));
				}
            }
            kfree(pWepKey);
            break;
#ifdef WPA_SUPPLICANT_SUPPORT
	    case OID_SET_COUNTERMEASURES:
            if (wrq->u.data.length != sizeof(int))
                Status  = -EINVAL;
            else
            {
                int enabled = 0;
                Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);
                if (enabled == 1)
                    pAd->StaCfg.bBlockAssoc = TRUE;
                else
                    // WPA MIC error should block association attempt for 60 seconds
                    pAd->StaCfg.bBlockAssoc = FALSE;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_SET_COUNTERMEASURES bBlockAssoc=%s\n", pAd->StaCfg.bBlockAssoc ? "TRUE":"FALSE"));
            }
	        break;
        case RT_OID_WPA_SUPPLICANT_SUPPORT:
			if (wrq->u.data.length != sizeof(UCHAR))
                Status  = -EINVAL;
            else
            {
				Status = copy_from_user(&wpa_supplicant_enable, wrq->u.data.pointer, wrq->u.data.length);
				if (wpa_supplicant_enable & WPA_SUPPLICANT_ENABLE_WPS)
					pAd->StaCfg.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
				else
				{
					pAd->StaCfg.WpaSupplicantUP = wpa_supplicant_enable;
					pAd->StaCfg.WpaSupplicantUP &= 0x7F;
				}
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_WPA_SUPPLICANT_SUPPORT (=0x%02X)\n", pAd->StaCfg.WpaSupplicantUP));
			}
            break;	   
        case OID_802_11_DEAUTHENTICATION:
            if (wrq->u.data.length != sizeof(MLME_DEAUTH_REQ_STRUCT))
                Status  = -EINVAL;
            else
            {
                MLME_DEAUTH_REQ_STRUCT      *pInfo;
				MLME_QUEUE_ELEM *MsgElem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);                
                if (MsgElem == NULL)
                {
                	DBGPRINT(RT_DEBUG_ERROR, ("%s():alloc memory failed!\n", __FUNCTION__));
                        return -EINVAL;
                }

                pInfo = (MLME_DEAUTH_REQ_STRUCT *) MsgElem->Msg;
                Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);
                MlmeDeauthReqAction(pAd, MsgElem);
				kfree(MsgElem);
				
                if (INFRA_ON(pAd))
                {
                    LinkDown(pAd, FALSE);
                    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->Reason));
            }
            break;
        case OID_802_11_DROP_UNENCRYPTED:
            if (wrq->u.data.length != sizeof(int))
                Status  = -EINVAL;
            else
            {
                int enabled = 0;
                Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);
                if (enabled == 1)
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                else
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
				NdisAcquireSpinLock(&pAd->MacTabLock);
				pAd->MacTab.Content[BSSID_WCID].PortSecured = pAd->StaCfg.PortSecured;
				NdisReleaseSpinLock(&pAd->MacTabLock);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DROP_UNENCRYPTED (=%d)\n", enabled));
            }
            break;
        case OID_802_11_SET_IEEE8021X:
            if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&IEEE8021xState, wrq->u.data.pointer, wrq->u.data.length);                				
		        pAd->StaCfg.IEEE8021X = IEEE8021xState;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_IEEE8021X (=%d)\n", IEEE8021xState));
            }
            break;
        case OID_802_11_SET_IEEE8021X_REQUIRE_KEY:	
			if (wrq->u.data.length != sizeof(BOOLEAN))
				 Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&IEEE8021x_required_keys, wrq->u.data.pointer, wrq->u.data.length);                				
				pAd->StaCfg.IEEE8021x_required_keys = IEEE8021x_required_keys;				
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_IEEE8021X_REQUIRE_KEY (%d)\n", IEEE8021x_required_keys));
			}	
			break;
        case OID_802_11_PMKID:
	        pPmkId = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);

	        if(pPmkId == NULL) {
                Status = -ENOMEM;
                break;
            }
            Status = copy_from_user(pPmkId, wrq->u.data.pointer, wrq->u.data.length);
	  	  
	        // check the PMKID information
	        if (pPmkId->BSSIDInfoCount == 0)
                NdisZeroMemory(pAd->StaCfg.SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
	        else
	        {
		        PBSSID_INFO	pBssIdInfo;
		        UINT		BssIdx;
		        UINT		CachedIdx;

		        for (BssIdx = 0; BssIdx < pPmkId->BSSIDInfoCount; BssIdx++)
		        {
			        // point to the indexed BSSID_INFO structure
			        pBssIdInfo = (PBSSID_INFO) ((PUCHAR) pPmkId + 2 * sizeof(UINT) + BssIdx * sizeof(BSSID_INFO));
			        // Find the entry in the saved data base.
			        for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			        {
				        // compare the BSSID
				        if (NdisEqualMemory(pBssIdInfo->BSSID, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, sizeof(NDIS_802_11_MAC_ADDRESS)))
					        break;			
			        }

			        // Found, replace it
			        if (CachedIdx < PMKID_NO)
			        {
				        DBGPRINT(RT_DEBUG_OFF, ("Update OID_802_11_PMKID, idx = %d\n", CachedIdx));
				        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
				        pAd->StaCfg.SavedPMKNum++;
			        }
			        // Not found, replace the last one
			        else
			        {
				        // Randomly replace one
				        CachedIdx = (pBssIdInfo->BSSID[5] % PMKID_NO);
				        DBGPRINT(RT_DEBUG_OFF, ("Update OID_802_11_PMKID, idx = %d\n", CachedIdx));
				        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
			        }				
		        }
			}
			if(pPmkId) 
				kfree(pPmkId);
	        break;

		case RT_OID_WPS_PROBE_REQ_IE:
			if (pAd->StaCfg.pWpsProbeReqIe)
			{
				kfree(pAd->StaCfg.pWpsProbeReqIe);
				pAd->StaCfg.pWpsProbeReqIe = NULL;
			}
			pAd->StaCfg.WpsProbeReqIeLen = 0;
			pAd->StaCfg.pWpsProbeReqIe = kmalloc(wrq->u.data.length, MEM_ALLOC_FLAG);
			if (pAd->StaCfg.pWpsProbeReqIe)
			{
				Status = copy_from_user(pAd->StaCfg.pWpsProbeReqIe, wrq->u.data.pointer, wrq->u.data.length);

				if (Status)
            	{
                	Status  = -EINVAL;
					if (pAd->StaCfg.pWpsProbeReqIe)
					{
						kfree(pAd->StaCfg.pWpsProbeReqIe);
						pAd->StaCfg.pWpsProbeReqIe = NULL;
					}
					pAd->StaCfg.WpsProbeReqIeLen = 0;
                	DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_WPS_PROBE_REQ_IE, Failed (copy_from_user failed)!!\n"));
           		}
           		else
           		{
					pAd->StaCfg.WpsProbeReqIeLen = wrq->u.data.length;
					hex_dump("WpsProbeReqIe", pAd->StaCfg.pWpsProbeReqIe, pAd->StaCfg.WpsProbeReqIeLen);
					DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_WPS_PROBE_REQ_IE, WpsProbeReqIeLen = %d!!\n",
								pAd->StaCfg.WpsProbeReqIeLen));
           		}
			}
			else
				Status = -ENOMEM;
			break;
#endif // WPA_SUPPLICANT_SUPPORT //

#ifdef WSC_STA_SUPPORT
		case RT_OID_WSC_EAPMSG:
			{
				RTMP_WSC_U2KMSG_HDR *msgHdr = NULL;
				PUCHAR pUPnPMsg = NULL;
				UINT msgLen = 0, Machine = 0, msgType = 0;
				int retVal, senderID = 0;

				DBGPRINT(RT_DEBUG_TRACE, ("WSC::RT_OID_WSC_EAPMSG, wrq->u.data.length=%d!\n", wrq->u.data.length));
			
				msgLen = wrq->u.data.length;				
				if((pUPnPMsg = kmalloc(msgLen, GFP_KERNEL)) == NULL)
					Status = -EINVAL;
				else
				{
					memset(pUPnPMsg, 0, msgLen);
					retVal = copy_from_user(pUPnPMsg, wrq->u.data.pointer, msgLen);
					
					msgHdr = (RTMP_WSC_U2KMSG_HDR *)pUPnPMsg;
					senderID = *((int *)&msgHdr->Addr2);
					//assign the STATE_MACHINE type
					{
                        Machine = WSC_STATE_MACHINE;
						msgType = WSC_EAPOL_UPNP_MSG;
												
						retVal = MlmeEnqueueForWsc(pAd, msgHdr->envID, senderID, Machine, msgType, msgLen, pUPnPMsg);
						if((retVal == FALSE) && (msgHdr->envID != 0))
						{
							DBGPRINT(RT_DEBUG_TRACE, ("MlmeEnqueuForWsc return False and envID=0x%x!\n", msgHdr->envID));
							Status = -EINVAL;
						}
					}

					kfree(pUPnPMsg);
				}
				DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_EAPMSG finished!\n"));
			}
			break;
        case RT_OID_WSC_SET_PROFILE:
            if (wrq->u.data.length != sizeof(WSC_PROFILE))
                Status = -EINVAL;
            else
            {
                PWSC_PROFILE pWscProfile = &pAd->StaCfg.WscControl.WscProfile;
                NdisZeroMemory(pWscProfile, sizeof(WSC_PROFILE));
                Status = copy_from_user(pWscProfile, wrq->u.data.pointer, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_PROFILE:: ProfileCnt = %d\n", pWscProfile->ProfileCnt));
            }
            break;
            
        case RT_OID_WSC_SET_CONF_MODE: // WPS disable, Enrollee or Registrar
            if (wrq->u.data.length != sizeof(INT))
                Status = -EINVAL;
            else
            {
                INT WscConfMode = 0;
                Status = copy_from_user(&WscConfMode, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == 0)
                {
					if (WscConfMode == 2)
						WscConfMode = 4;

                    switch(WscConfMode)
                    {
                        case WSC_ENROLLEE:
                            Set_WscConfMode_Proc(pAd, "1");
                            break;
                        case WSC_REGISTRAR:
                            Set_WscConfMode_Proc(pAd, "2");
							WscConfMode = 2;
                            break;
                        case WSC_DISABLE:
                        default:
                            Set_WscConfMode_Proc(pAd, "0");
                            break;
                    }                    
                }
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_CONF_MODE:: WscConfMode = %d\n", WscConfMode));
            }
            break;
        case RT_OID_WSC_SET_MODE:
            if (wrq->u.data.length != sizeof(INT))
                Status = -EINVAL;
            else
            {
                INT WscMode = 0; // PIN or PBC
                Status = copy_from_user(&WscMode, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == 0)
                {
                    if (WscMode == 1)
                        Set_WscMode_Proc(pAd, "1"); // PIN
                    else if (WscMode == 2)
                        Set_WscMode_Proc(pAd, "2"); // PBC
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_MODE::unknown WscMode = %d\n", WscMode));
                        Status = -EINVAL;
                    }
                }
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_MODE::WscMode = %d\n", WscMode));
            }
            break;
        case RT_OID_WSC_SET_PIN_CODE:
            if (wrq->u.data.length != 8) // PIN Code Length is 8
                Status = -EINVAL;
            else
            {
                CHAR PinCode[9] = {0};
                Status = copy_from_user(&PinCode[0], wrq->u.data.pointer, wrq->u.data.length);
                if (Status == 0)
                {
                    if (Set_WscPinCode_Proc(pAd, (PSTRING) &PinCode[0]) == FALSE)
                        Status = -EINVAL;
                }
            }
            break;
        case RT_OID_WSC_SET_SSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
                Status = -EINVAL;
            else
            {
                NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
                Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);
                Set_WscSsid_Proc(pAd, (PSTRING) Ssid.Ssid);
            }
            break;
        case RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX:
            if (wrq->u.data.length != sizeof(UINT))
                Status = -EINVAL;
            else
            {
                UINT wsc_profile_index = 0; // PIN or PBC
                PWSC_CTRL   pWscControl = &pAd->StaCfg.WscControl;
                unsigned long	IrqFlags;
                
                Status = copy_from_user(&wsc_profile_index, wrq->u.data.pointer, wrq->u.data.length);
                if (wsc_profile_index < pWscControl->WscProfile.ProfileCnt)
                {                    
                    RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
                    WscWriteConfToPortCfg(pAd, pWscControl, &pWscControl->WscProfile.Profile[wsc_profile_index], TRUE);
                    RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
                    pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
                    LinkDown(pAd, TRUE);                    
                }
                else
                    DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX:: wrong wsc_profile_index(%d)\n", wsc_profile_index));                
            }
            break;
        case RT_OID_WSC_DRIVER_AUTO_CONNECT:
            if (wrq->u.data.length != sizeof(UCHAR))
                Status = -EINVAL;
            else
            {
                Status = copy_from_user(&pAd->StaCfg.WscControl.WscDriverAutoConnect, wrq->u.data.pointer, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_DRIVER_AUTO_CONNECT::WscDriverAutoConnect is %d\n", 
                                            pAd->StaCfg.WscControl.WscDriverAutoConnect));
            }
            break;
        case RT_OID_WSC_SET_PASSPHRASE:
            if (wrq->u.data.length > 64 || wrq->u.data.length < 8)
                Status = -EINVAL;
            else
            {
                Status = copy_from_user(pAd->StaCfg.WscControl.WpaPsk, wrq->u.data.pointer, wrq->u.data.length);
                NdisZeroMemory(pAd->StaCfg.WscControl.WpaPsk, 64);
                pAd->StaCfg.WscControl.WpaPskLen = wrq->u.data.length;
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_PASSPHRASE::KeyLen(%d)\n", pAd->StaCfg.WscControl.WpaPskLen));
            }
            break;
#endif // WSC_STA_SUPPORT //

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
		case RT_OID_LED_WPS_MODE10:
			if(!(pAd->StaCfg.bRadio) ||
				(wrq->u.data.length != sizeof(UINT)))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&WPSLedMode10, wrq->u.data.pointer, wrq->u.data.length);
				if((WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_ON) && 
					(WPSLedMode10 != LINK_STATUS_WPS_MODE10_FLASH) && 
					(WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_OFF))
				{
					Status = NDIS_STATUS_INVALID_DATA;
					DBGPRINT(RT_DEBUG_INFO, ("WPS LED Mode 10::Parameter of LED Mode 10 must be 0x00, or 0x01, or 0x02\n"));
				}
				else
				{ 
#ifdef RTMP_MAC_PCI
					RTMPSetLED(pAd, WPSLedMode10);
#endif // RTMP_MAC_PCI //
				}
			}
			break;
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //

#ifdef SNMP_SUPPORT
		case OID_802_11_SHORTRETRYLIMIT:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&ShortRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
				RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
				tx_rty_cfg.field.ShortRtyLimit = ShortRetryLimit;
				RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SHORTRETRYLIMIT (tx_rty_cfg.field.ShortRetryLimit=%d, ShortRetryLimit=%ld)\n", tx_rty_cfg.field.ShortRtyLimit, ShortRetryLimit));
			}
			break;

		case OID_802_11_LONGRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT \n"));
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&LongRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
				RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
				tx_rty_cfg.field.LongRtyLimit = LongRetryLimit;
				RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT (tx_rty_cfg.field.LongRetryLimit= %d,LongRetryLimit=%ld)\n", tx_rty_cfg.field.LongRtyLimit, LongRetryLimit));
			}
			break;

		case OID_802_11_WEPDEFAULTKEYVALUE:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE\n"));
			pKey = kmalloc(wrq->u.data.length, GFP_KERNEL);
			Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
			//pKey = &WepKey;
			
			if ( pKey->Length != wrq->u.data.length)
			{
				Status = -EINVAL;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n"));
			}
			KeyIdx = pKey->KeyIndex & 0x0fffffff;
			DBGPRINT(RT_DEBUG_TRACE,("pKey->KeyIndex =%d, pKey->KeyLength=%d\n", pKey->KeyIndex, pKey->KeyLength));

			// it is a shared key
			if (KeyIdx > 4)
				Status = -EINVAL;
			else
			{
				pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(&pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key, &pKey->KeyMaterial, pKey->KeyLength);
				if (pKey->KeyIndex & 0x80000000)
				{
					// Default key for tx (shared key)
					pAd->StaCfg.DefaultKeyId = (UCHAR) KeyIdx;
				}
				//RestartAPIsRequired = TRUE;
			}
			break;


		case OID_802_11_WEPDEFAULTKEYID:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYID \n"));

			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
				Status = copy_from_user(&pAd->StaCfg.DefaultKeyId, wrq->u.data.pointer, wrq->u.data.length);

			break;


		case OID_802_11_CURRENTCHANNEL:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_CURRENTCHANNEL \n"));
			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&ctmp, wrq->u.data.pointer, wrq->u.data.length);
				sprintf((PSTRING)&ctmp,"%d", ctmp);
				Set_Channel_Proc(pAd, (PSTRING)&ctmp);
			}
			break;
#endif


#ifdef WAPI_SUPPORT
		case OID_802_11_WAPI_PID:
			{
				unsigned long wapi_pid;
    			if (copy_from_user(&pObj->wapi_pid, wrq->u.data.pointer, wrq->u.data.length))
				{
					Status = -EFAULT; 	
				}
    			else
    			{
					RTMP_GET_OS_PID(pObj->wapi_pid, wapi_pid);
					pObj->wapi_pid_nr = wapi_pid;
					DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_WAPI_PID::(WapiPid=%lu(0x%x))\n", wapi_pid, pObj->wapi_pid));
				}
    		}
			break;
		case OID_802_11_PORT_SECURE_STATE:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_PORT_SECURE_STATE, len=%d/%d\n", wrq->u.data.length, sizeof(WAPI_PORT_SECURE_STRUCT)));
			if (wrq->u.data.length != sizeof(WAPI_PORT_SECURE_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_PORT_SECURE_STRUCT  wapi_port;

				Status = copy_from_user(&wapi_port, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {
					if (INFRA_ON(pAd))
					{						
						if (NdisEqualMemory(pAd->MlmeAux.Bssid, wapi_port.Addr, MAC_ADDR_LEN))
						{
							switch (wapi_port.state)
							{
								case WAPI_PORT_SECURED:
									//pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
									STA_PORT_SECURED(pAd);
									pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
									// Indicate Connected for GUI
									pAd->IndicateMediaState = NdisMediaStateConnected;
									break;
								
								default:
									pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
									pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;

									pAd->IndicateMediaState = NdisMediaStateDisconnected;
									break;								
							}	
							DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_PORT_SECURE_STATE (state=%d)\n", wapi_port.state));
						}
					}
				}				
            }
			break;

		case OID_802_11_UCAST_KEY_INFO:
			if (wrq->u.data.length != sizeof(WAPI_UCAST_KEY_STRUCT))
                Status  = -EINVAL;
            else
            {                								
				MAC_TABLE_ENTRY 		*pEntry = NULL;
				WAPI_UCAST_KEY_STRUCT   wapi_ukey;

				Status = copy_from_user(&wapi_ukey, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {
					if (INFRA_ON(pAd))
					{						
						if (NdisEqualMemory(pAd->MlmeAux.Bssid, wapi_ukey.Addr, MAC_ADDR_LEN))
						{
							pEntry = &pAd->MacTab.Content[BSSID_WCID];
							pEntry->usk_id = wapi_ukey.key_id;
							NdisMoveMemory(pAd->StaCfg.PTK, wapi_ukey.PTK, 64);	
							NdisMoveMemory(pEntry->PTK, wapi_ukey.PTK, 64);
							
							/* Install pairwise key */
							WAPIInstallPairwiseKey(pAd, pEntry, FALSE);

							// Start or re-start USK rekey mechanism, if necessary.
							RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
							RTMPStartWapiRekeyTimerAction(pAd, pEntry);

							DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_UCAST_KEY_INFO complete\n"));
							hex_dump("WAPI UCAST KEY", pAd->StaCfg.PTK, 64);
						}
					}	
				}				
            }
			break;	

		case OID_802_11_MCAST_KEY_INFO:
			if (wrq->u.data.length != sizeof(WAPI_MCAST_KEY_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_MCAST_KEY_STRUCT   wapi_mkey;

				Status = copy_from_user(&wapi_mkey, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {                	
                	// Obtain the NMK and tx_iv of AE
                	pAd->StaCfg.DefaultKeyId = wapi_mkey.key_id;
                	//NdisMoveMemory(pAd->StaCfg.rx_iv, wapi_mkey.m_tx_iv, LEN_WAPI_TSC);
                	NdisMoveMemory(pAd->StaCfg.NMK, wapi_mkey.NMK, 16);

					// Calculate GTK
					RTMPDeriveWapiGTK(pAd->StaCfg.NMK, pAd->StaCfg.GTK);
                														
					/* Install Shared key */
					WAPIInstallSharedKey(pAd, 
										 pAd->StaCfg.GroupCipher, 
										 BSS0, 
										 pAd->StaCfg.DefaultKeyId, 
										 MCAST_WCID,
										 pAd->StaCfg.GTK);
																		
					DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_MCAST_KEY_INFO complete\n"));
				}				
            }
			break;								
#endif // WAPI_SUPPORT //

#ifdef XLINK_SUPPORT
		case RT_OID_802_11_SET_PSPXLINK_MODE:
			if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&pAd->StaCfg.PSPXlink, wrq->u.data.pointer, wrq->u.data.length);
				/*if (pAd->StaCfg.PSPXlink)
					RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_PROMISCUOUS)*/
				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_PSPXLINK_MODE(=%d) \n", pAd->StaCfg.PSPXlink));
            }
			break;
#endif // XLINK_SUPPORT //


        default:
            DBGPRINT(RT_DEBUG_TRACE, ("Set::unknown IOCTL's subcmd = 0x%08x\n", cmd));
            Status = -EOPNOTSUPP;
            break;
    }


    return Status;
}

INT RTMPQueryInformation(
    IN  PRTMP_ADAPTER pAd,
    IN  OUT struct ifreq    *rq,
    IN  INT                 cmd)
{
    struct iwreq                        *wrq = (struct iwreq *) rq;
    NDIS_802_11_BSSID_LIST_EX           *pBssidList = NULL;
    PNDIS_WLAN_BSSID_EX                 pBss;
    NDIS_802_11_SSID                    Ssid;
    NDIS_802_11_CONFIGURATION           *pConfiguration = NULL;
    RT_802_11_LINK_STATUS               *pLinkStatus = NULL;
    RT_802_11_STA_CONFIG                *pStaConfig = NULL;
    NDIS_802_11_STATISTICS              *pStatistics = NULL;
    NDIS_802_11_RTS_THRESHOLD           RtsThresh;
    NDIS_802_11_FRAGMENTATION_THRESHOLD FragThresh;
    NDIS_802_11_POWER_MODE              PowerMode;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  BssType;
    RT_802_11_PREAMBLE                  PreamType;
    NDIS_802_11_AUTHENTICATION_MODE     AuthMode;
    NDIS_802_11_WEP_STATUS              WepStatus;
    NDIS_MEDIA_STATE                    MediaState;
    ULONG                               BssBufSize, ulInfo=0, NetworkTypeList[4], apsd = 0, RateValue=0;
    USHORT                              BssLen = 0;
    PUCHAR                              pBuf = NULL, pPtr;
    INT                                 Status = NDIS_STATUS_SUCCESS;
    UINT                                we_version_compiled;
    UCHAR                               i, Padding = 0;
    BOOLEAN                             RadioState;
    STRING								driverVersion[8];
    OID_SET_HT_PHYMODE			        *pHTPhyMode = NULL;
    HTTRANSMIT_SETTING	HTPhyMode;
	
#ifdef WSC_STA_SUPPORT
	UINT	                            WscPinCode = 0;
	PWSC_PROFILE						pProfile;
#endif // WSC_STA_SUPPORT //

#ifdef SNMP_SUPPORT	
	//for snmp, kathy
	DefaultKeyIdxValue			*pKeyIdxValue;
	INT							valueLen;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						tmp[64];
#endif //SNMP

    switch(cmd) 
    {
        case RT_OID_DEVICE_NAME:
            wrq->u.data.length = sizeof(pAd->nickname);
            Status = copy_to_user(wrq->u.data.pointer, pAd->nickname, wrq->u.data.length);
            break;        
        case RT_OID_VERSION_INFO:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_VERSION_INFO \n"));
			wrq->u.data.length = 8*sizeof(CHAR);
			sprintf(&driverVersion[0], "%s", STA_DRIVER_VERSION);
			driverVersion[7] = '\0';
			if (copy_to_user(wrq->u.data.pointer, &driverVersion[0], wrq->u.data.length))
            {
				Status = -EFAULT;
            }
            break;

        case OID_802_11_BSSID_LIST:
            if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
            	/*
            	 * Still scanning, indicate the caller should try again.
            	 */
            	pAd->StaCfg.bScanReqIsFromWebUI = TRUE;
            	DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BSSID_LIST (Still scanning)\n"));
				return -EAGAIN;
            }

			if (pAd->StaCfg.bImprovedScan)
			{
				/*
				 * Fast scanning doesn't complete yet.
				 */
				pAd->StaCfg.bScanReqIsFromWebUI = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BSSID_LIST (Still scanning)\n"));
				return -EAGAIN;
			}
			
#ifdef WPA_SUPPLICANT_SUPPORT
			if ((pAd->StaCfg.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
			{
				pAd->StaCfg.WpaSupplicantScanCount = 0;
			}
#endif // WPA_SUPPLICANT_SUPPORT //
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BSSID_LIST (%d BSS returned)\n",pAd->ScanTab.BssNr));
			pAd->StaCfg.bScanReqIsFromWebUI = FALSE;
            // Claculate total buffer size required
            BssBufSize = sizeof(ULONG);
            
            for (i = 0; i < pAd->ScanTab.BssNr; i++) 
            {
                // Align pointer to 4 bytes boundary.
                //Padding = 4 - (pAd->ScanTab.BssEntry[i].VarIELen & 0x0003);
                //if (Padding == 4)
                //    Padding = 0;
                BssBufSize += (sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + pAd->ScanTab.BssEntry[i].VarIELen + Padding);
            }

            // For safety issue, we add 256 bytes just in case
            BssBufSize += 256;
            // Allocate the same size as passed from higher layer
            pBuf = kmalloc(BssBufSize, MEM_ALLOC_FLAG);
            if(pBuf == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            // Init 802_11_BSSID_LIST_EX structure
            NdisZeroMemory(pBuf, BssBufSize);
            pBssidList = (PNDIS_802_11_BSSID_LIST_EX) pBuf;
            pBssidList->NumberOfItems = pAd->ScanTab.BssNr;
            
            // Calculate total buffer length
            BssLen = 4; // Consist of NumberOfItems
            // Point to start of NDIS_WLAN_BSSID_EX
            // pPtr = pBuf + sizeof(ULONG);
            pPtr = (PUCHAR) &pBssidList->Bssid[0];
            for (i = 0; i < pAd->ScanTab.BssNr; i++) 
            {
                pBss = (PNDIS_WLAN_BSSID_EX) pPtr;
                NdisMoveMemory(&pBss->MacAddress, &pAd->ScanTab.BssEntry[i].Bssid, MAC_ADDR_LEN);
                if ((pAd->ScanTab.BssEntry[i].Hidden == 1) && (pAd->StaCfg.bShowHiddenSSID == FALSE))
                {
                    //
					// We must return this SSID during 4way handshaking, otherwise Aegis will failed to parse WPA infomation
					// and then failed to send EAPOl farame.
					//
					if ((pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA) && (pAd->StaCfg.PortSecured != WPA_802_1X_PORT_SECURED))
					{
						pBss->Ssid.SsidLength = pAd->ScanTab.BssEntry[i].SsidLen;
						NdisMoveMemory(pBss->Ssid.Ssid, pAd->ScanTab.BssEntry[i].Ssid, pAd->ScanTab.BssEntry[i].SsidLen);
					}
					else
                    	pBss->Ssid.SsidLength = 0;
                }
                else
                {
                    pBss->Ssid.SsidLength = pAd->ScanTab.BssEntry[i].SsidLen;
                    NdisMoveMemory(pBss->Ssid.Ssid, pAd->ScanTab.BssEntry[i].Ssid, pAd->ScanTab.BssEntry[i].SsidLen);
                }
                pBss->Privacy = pAd->ScanTab.BssEntry[i].Privacy;
                pBss->Rssi = pAd->ScanTab.BssEntry[i].Rssi - pAd->BbpRssiToDbmDelta;
                pBss->NetworkTypeInUse = NetworkTypeInUseSanity(&pAd->ScanTab.BssEntry[i]);
                pBss->Configuration.Length = sizeof(NDIS_802_11_CONFIGURATION);
                pBss->Configuration.BeaconPeriod = pAd->ScanTab.BssEntry[i].BeaconPeriod;  
                pBss->Configuration.ATIMWindow = pAd->ScanTab.BssEntry[i].AtimWin;
				//NdisMoveMemory(&pBss->QBssLoad, &pAd->ScanTab.BssEntry[i].QbssLoad, sizeof(QBSS_LOAD_UI));

                MAP_CHANNEL_ID_TO_KHZ(pAd->ScanTab.BssEntry[i].Channel, pBss->Configuration.DSConfig);

                if (pAd->ScanTab.BssEntry[i].BssType == BSS_INFRA) 
                    pBss->InfrastructureMode = Ndis802_11Infrastructure;
                else
                    pBss->InfrastructureMode = Ndis802_11IBSS;

                NdisMoveMemory(pBss->SupportedRates, pAd->ScanTab.BssEntry[i].SupRate, pAd->ScanTab.BssEntry[i].SupRateLen);
                NdisMoveMemory(pBss->SupportedRates + pAd->ScanTab.BssEntry[i].SupRateLen,
                               pAd->ScanTab.BssEntry[i].ExtRate,
                               pAd->ScanTab.BssEntry[i].ExtRateLen);

                if (pAd->ScanTab.BssEntry[i].VarIELen == 0)
                {
                    pBss->IELength = sizeof(NDIS_802_11_FIXED_IEs);
                    NdisMoveMemory(pBss->IEs, &pAd->ScanTab.BssEntry[i].FixIEs, sizeof(NDIS_802_11_FIXED_IEs));
                    pPtr = pPtr + sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs);
                }
                else
                {
                    pBss->IELength = (ULONG)(sizeof(NDIS_802_11_FIXED_IEs) + pAd->ScanTab.BssEntry[i].VarIELen);
                    pPtr = pPtr + sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs);
                    NdisMoveMemory(pBss->IEs, &pAd->ScanTab.BssEntry[i].FixIEs, sizeof(NDIS_802_11_FIXED_IEs));
                    NdisMoveMemory(pBss->IEs + sizeof(NDIS_802_11_FIXED_IEs), pAd->ScanTab.BssEntry[i].VarIEs, pAd->ScanTab.BssEntry[i].VarIELen);
                    pPtr += pAd->ScanTab.BssEntry[i].VarIELen;
                }
                pBss->Length = (ULONG)(sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + pAd->ScanTab.BssEntry[i].VarIELen + Padding);

#if WIRELESS_EXT < 17                
                if ((BssLen + pBss->Length) < wrq->u.data.length)
                BssLen += pBss->Length;
                else
                {
                    pBssidList->NumberOfItems = i;
                    break;
                }
#else
                BssLen += pBss->Length;
#endif
            }

#if WIRELESS_EXT < 17            
            wrq->u.data.length = BssLen;
#else
            if (BssLen > wrq->u.data.length)
            {
                kfree(pBssidList);
                return -E2BIG;
            }
            else
                wrq->u.data.length = BssLen;
#endif
            Status = copy_to_user(wrq->u.data.pointer, pBssidList, BssLen);
            kfree(pBssidList);
            break;
        case OID_802_3_CURRENT_ADDRESS:
            wrq->u.data.length = MAC_ADDR_LEN;
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
            break;
        case OID_GEN_MEDIA_CONNECT_STATUS:
            if (pAd->IndicateMediaState == NdisMediaStateConnected)
                MediaState = NdisMediaStateConnected;
            else
                MediaState = NdisMediaStateDisconnected;
                
            wrq->u.data.length = sizeof(NDIS_MEDIA_STATE);
            Status = copy_to_user(wrq->u.data.pointer, &MediaState, wrq->u.data.length);
            break;   
        case OID_802_11_BSSID:
            if (INFRA_ON(pAd) || ADHOC_ON(pAd))
            {
                Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Bssid, sizeof(NDIS_802_11_MAC_ADDRESS));

            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BSSID(=EMPTY)\n"));
                Status = -ENOTCONN;
            }
            break;
        case OID_802_11_SSID:
			NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
			NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
            Ssid.SsidLength = pAd->CommonCfg.SsidLen;
			memcpy(Ssid.Ssid, pAd->CommonCfg.Ssid,	Ssid.SsidLength);
            wrq->u.data.length = sizeof(NDIS_802_11_SSID);
            Status = copy_to_user(wrq->u.data.pointer, &Ssid, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SSID (Len=%d, ssid=%s)\n", Ssid.SsidLength,Ssid.Ssid));
            break;
        case RT_OID_802_11_QUERY_LINK_STATUS:
            pLinkStatus = (RT_802_11_LINK_STATUS *) kmalloc(sizeof(RT_802_11_LINK_STATUS), MEM_ALLOC_FLAG);
            if (pLinkStatus)
            {
                pLinkStatus->CurrTxRate = RateIdTo500Kbps[pAd->CommonCfg.TxRate];   // unit : 500 kbps
                pLinkStatus->ChannelQuality = pAd->Mlme.ChannelQuality;
                pLinkStatus->RxByteCount = pAd->RalinkCounters.ReceivedByteCount;
                pLinkStatus->TxByteCount = pAd->RalinkCounters.TransmittedByteCount;
        		pLinkStatus->CentralChannel = pAd->CommonCfg.CentralChannel;
                wrq->u.data.length = sizeof(RT_802_11_LINK_STATUS);
                Status = copy_to_user(wrq->u.data.pointer, pLinkStatus, wrq->u.data.length);
                kfree(pLinkStatus);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LINK_STATUS\n"));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LINK_STATUS(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case OID_802_11_CONFIGURATION:
            pConfiguration = (NDIS_802_11_CONFIGURATION *) kmalloc(sizeof(NDIS_802_11_CONFIGURATION), MEM_ALLOC_FLAG);
            if (pConfiguration)
            {
                pConfiguration->Length = sizeof(NDIS_802_11_CONFIGURATION);
                pConfiguration->BeaconPeriod = pAd->CommonCfg.BeaconPeriod;
                pConfiguration->ATIMWindow = pAd->StaActive.AtimWin;
                MAP_CHANNEL_ID_TO_KHZ(pAd->CommonCfg.Channel, pConfiguration->DSConfig);
                wrq->u.data.length = sizeof(NDIS_802_11_CONFIGURATION);
                Status = copy_to_user(wrq->u.data.pointer, pConfiguration, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CONFIGURATION(BeaconPeriod=%ld,AtimW=%ld,Channel=%d) \n", 
                                        pConfiguration->BeaconPeriod, pConfiguration->ATIMWindow, pAd->CommonCfg.Channel));
				kfree(pConfiguration);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CONFIGURATION(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
		case RT_OID_802_11_SNR_0:
			if ((pAd->StaCfg.LastSNR0 > 0))
			{
#if defined(RT2883) || defined(RT3883)
				if (IS_RT2883(pAd) || IS_RT3883(pAd))
				{
					ulInfo = (pAd->StaCfg.LastSNR0 * 3 + 8) >> 4;
				}
				else
#endif // defined(RT2883) || defined(RT3883) //
				{
				ulInfo = ((0xeb	- pAd->StaCfg.LastSNR0) * 3) /	16 ;
				}
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_SNR_0(0x=%lx)\n", ulInfo));
			}
            else
			    Status = -EFAULT;
			break;
		case RT_OID_802_11_SNR_1:
			if ((pAd->Antenna.field.RxPath	> 1) && 
                (pAd->StaCfg.LastSNR1 > 0))
			{
#if defined(RT2883) || defined(RT3883)
				if (IS_RT2883(pAd) || IS_RT3883(pAd))
				{
					ulInfo = (pAd->StaCfg.LastSNR1 * 3 + 8) >> 4;
				}
				else
#endif // defined(RT2883) || defined(RT3883) //
				{
				ulInfo = ((0xeb	- pAd->StaCfg.LastSNR1) * 3) /	16 ;
				}
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_1(0x=%lx)\n",ulInfo));
			}
			else
				Status = -EFAULT;
            DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_1(pAd->StaCfg.LastSNR1=%d)\n",pAd->StaCfg.LastSNR1));
			break;

#if defined(RT2883) || defined(RT3883)
		case RT_OID_802_11_SNR_2:
			if ((pAd->Antenna.field.RxPath	> 2) && 
                (pAd->StaCfg.LastSNR2 > 0))
			{
				ulInfo = (pAd->StaCfg.LastSNR2 * 3 + 8) >> 4;
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_2(0x=%lx)\n",ulInfo));
			}
			else
				Status = -EFAULT;
            DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_2(pAd->StaCfg.LastSNR2=%d)\n",pAd->StaCfg.LastSNR2));
			break;

		case RT_OID_802_11_STREAM_SNR:
			{
			long StreamSnr[3];
			StreamSnr[0] = pAd->StaCfg.BF_SNR[0];
			StreamSnr[1] = pAd->StaCfg.BF_SNR[1];
			StreamSnr[2] = pAd->StaCfg.BF_SNR[2];
			wrq->u.data.length = sizeof(StreamSnr);
			Status = copy_to_user(wrq->u.data.pointer, &StreamSnr,	wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_STR_SNR(0x=%ld, %ld)\n", StreamSnr[0], StreamSnr[1]));
			}
			break;

#endif // defined(RT2883) || defined(RT3883) //


        case OID_802_11_RSSI_TRIGGER:
            ulInfo = pAd->StaCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_RSSI_TRIGGER(=%ld)\n", ulInfo));
            break;
		case OID_802_11_RSSI:
        case RT_OID_802_11_RSSI:
			ulInfo = pAd->StaCfg.RssiSample.LastRssi0;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			break;
		case RT_OID_802_11_RSSI_1:
            ulInfo = pAd->StaCfg.RssiSample.LastRssi1;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			break;
        case RT_OID_802_11_RSSI_2:
            ulInfo = pAd->StaCfg.RssiSample.LastRssi2;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			break;
        case OID_802_11_STATISTICS:
            pStatistics = (NDIS_802_11_STATISTICS *) kmalloc(sizeof(NDIS_802_11_STATISTICS), MEM_ALLOC_FLAG);
            if (pStatistics)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_STATISTICS \n"));
                // add the most up-to-date h/w raw counters into software counters
			    NICUpdateRawCounters(pAd);
                
                // Sanity check for calculation of sucessful count
                if (pAd->WlanCounters.TransmittedFragmentCount.QuadPart < pAd->WlanCounters.RetryCount.QuadPart)
                    pAd->WlanCounters.TransmittedFragmentCount.QuadPart = pAd->WlanCounters.RetryCount.QuadPart;

                pStatistics->TransmittedFragmentCount.QuadPart = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;
                pStatistics->MulticastTransmittedFrameCount.QuadPart = pAd->WlanCounters.MulticastTransmittedFrameCount.QuadPart;
                pStatistics->FailedCount.QuadPart = pAd->WlanCounters.FailedCount.QuadPart;
                pStatistics->RetryCount.QuadPart = pAd->WlanCounters.RetryCount.QuadPart;
                pStatistics->MultipleRetryCount.QuadPart = pAd->WlanCounters.MultipleRetryCount.QuadPart;
                pStatistics->RTSSuccessCount.QuadPart = pAd->WlanCounters.RTSSuccessCount.QuadPart;
                pStatistics->RTSFailureCount.QuadPart = pAd->WlanCounters.RTSFailureCount.QuadPart;
                pStatistics->ACKFailureCount.QuadPart = pAd->WlanCounters.ACKFailureCount.QuadPart;
                pStatistics->FrameDuplicateCount.QuadPart = pAd->WlanCounters.FrameDuplicateCount.QuadPart;
                pStatistics->ReceivedFragmentCount.QuadPart = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
                pStatistics->MulticastReceivedFrameCount.QuadPart = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;
#ifdef DBG	
                pStatistics->FCSErrorCount = pAd->RalinkCounters.RealFcsErrCount;
#else
                pStatistics->FCSErrorCount.QuadPart = pAd->WlanCounters.FCSErrorCount.QuadPart;
                pStatistics->FrameDuplicateCount.u.LowPart = pAd->WlanCounters.FrameDuplicateCount.u.LowPart / 100;
#endif
                wrq->u.data.length = sizeof(NDIS_802_11_STATISTICS);
                Status = copy_to_user(wrq->u.data.pointer, pStatistics, wrq->u.data.length);
                kfree(pStatistics);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_STATISTICS(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;

#ifdef TXBF_SUPPORT
	case RT_OID_802_11_QUERY_TXBF_TABLE:
		{
			INT i;
			RT_802_11_TXBF_TABLE *pMacTab;

			pMacTab = (RT_802_11_TXBF_TABLE *)kmalloc(sizeof(RT_802_11_TXBF_TABLE), MEM_ALLOC_FLAG);
			if (pMacTab)
			{
				pMacTab->Num = 0;
				for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
				{
					if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC))
					{
						memcpy(&pMacTab->Entry[pMacTab->Num], &pAd->MacTab.Content[i].TxBFCounters, sizeof(RT_COUNTER_TXBF));
						pMacTab->Num++;
					}
				}

				wrq->u.data.length = sizeof(RT_802_11_TXBF_TABLE);
				Status = copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length);
				kfree(pMacTab);
			}
			else
			{
				Status = -EFAULT;
			}
		}
		break;
#endif // TXBF_SUPPORT //

        case OID_GEN_RCV_OK:
            ulInfo = pAd->Counters8023.GoodReceives;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            break;
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo = pAd->Counters8023.RxNoBuffer;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            break;
        case RT_OID_802_11_PHY_MODE:
            ulInfo = (ULONG)pAd->CommonCfg.PhyMode;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PHY_MODE (=%ld)\n", ulInfo));
            break;
        case RT_OID_802_11_STA_CONFIG:
            pStaConfig = (RT_802_11_STA_CONFIG *) kmalloc(sizeof(RT_802_11_STA_CONFIG), MEM_ALLOC_FLAG);
            if (pStaConfig)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG\n"));
                pStaConfig->EnableTxBurst = pAd->CommonCfg.bEnableTxBurst;
                pStaConfig->EnableTurboRate = 0;
                pStaConfig->UseBGProtection = pAd->CommonCfg.UseBGProtection;
                pStaConfig->UseShortSlotTime = pAd->CommonCfg.bUseShortSlotTime;
                //pStaConfig->AdhocMode = pAd->StaCfg.AdhocMode;
                pStaConfig->HwRadioStatus = (pAd->StaCfg.bHwRadio == TRUE) ? 1 : 0;
                pStaConfig->Rsv1 = 0;
                pStaConfig->SystemErrorBitmap = pAd->SystemErrorBitmap;
                wrq->u.data.length = sizeof(RT_802_11_STA_CONFIG);
                Status = copy_to_user(wrq->u.data.pointer, pStaConfig, wrq->u.data.length);
                kfree(pStaConfig);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case OID_802_11_RTS_THRESHOLD:
            RtsThresh = pAd->CommonCfg.RtsThreshold;
            wrq->u.data.length = sizeof(RtsThresh);
            Status = copy_to_user(wrq->u.data.pointer, &RtsThresh, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_RTS_THRESHOLD(=%ld)\n", RtsThresh));
            break;
        case OID_802_11_FRAGMENTATION_THRESHOLD:
            FragThresh = pAd->CommonCfg.FragmentThreshold;
            if (pAd->CommonCfg.bUseZeroToDisableFragment == TRUE)
                FragThresh = 0;
            wrq->u.data.length = sizeof(FragThresh);
            Status = copy_to_user(wrq->u.data.pointer, &FragThresh, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_FRAGMENTATION_THRESHOLD(=%ld)\n", FragThresh));
            break;
        case OID_802_11_POWER_MODE:
            PowerMode = pAd->StaCfg.WindowsPowerMode;
            wrq->u.data.length = sizeof(PowerMode);
            Status = copy_to_user(wrq->u.data.pointer, &PowerMode, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_POWER_MODE(=%d)\n", PowerMode));
            break;
        case RT_OID_802_11_RADIO:
            RadioState = (BOOLEAN) pAd->StaCfg.bSwRadio;
            wrq->u.data.length = sizeof(RadioState);
            Status = copy_to_user(wrq->u.data.pointer, &RadioState, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_RADIO (=%d)\n", RadioState));
            break;
        case OID_802_11_INFRASTRUCTURE_MODE:
            if (pAd->StaCfg.BssType == BSS_ADHOC)
                BssType = Ndis802_11IBSS;
            else if (pAd->StaCfg.BssType == BSS_INFRA)
                BssType = Ndis802_11Infrastructure;
            else if (pAd->StaCfg.BssType == BSS_MONITOR)
                BssType = Ndis802_11Monitor;
            else
                BssType = Ndis802_11AutoUnknown;

            wrq->u.data.length = sizeof(BssType);
            Status = copy_to_user(wrq->u.data.pointer, &BssType, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_INFRASTRUCTURE_MODE(=%d)\n", BssType));
            break;
        case RT_OID_802_11_PREAMBLE:
            PreamType = pAd->CommonCfg.TxPreamble;
            wrq->u.data.length = sizeof(PreamType);
            Status = copy_to_user(wrq->u.data.pointer, &PreamType, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PREAMBLE(=%d)\n", PreamType));
            break;
        case OID_802_11_AUTHENTICATION_MODE:
            AuthMode = pAd->StaCfg.AuthMode;
            wrq->u.data.length = sizeof(AuthMode);
            Status = copy_to_user(wrq->u.data.pointer, &AuthMode, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_AUTHENTICATION_MODE(=%d)\n", AuthMode));
            break;
        case OID_802_11_WEP_STATUS:
            WepStatus = pAd->StaCfg.WepStatus;
            wrq->u.data.length = sizeof(WepStatus);
            Status = copy_to_user(wrq->u.data.pointer, &WepStatus, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WEP_STATUS(=%d)\n", WepStatus));
            break;
        case OID_802_11_TX_POWER_LEVEL:
			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.TxPower, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_TX_POWER_LEVEL %x\n",pAd->CommonCfg.TxPower));
			break;
        case RT_OID_802_11_TX_POWER_LEVEL_1:
            wrq->u.data.length = sizeof(ULONG);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.TxPowerPercentage, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_TX_POWER_LEVEL_1 (=%ld)\n", pAd->CommonCfg.TxPowerPercentage));
			break;
        case OID_802_11_NETWORK_TYPES_SUPPORTED:
			if ((pAd->RfIcType	== RFIC_2850) ||
				(pAd->RfIcType ==	RFIC_2750) ||
				(pAd->RfIcType == RFIC_3052) ||
				(pAd->RfIcType == RFIC_3053) || 
				(pAd->RfIcType == RFIC_2853) || 
				(pAd->RfIcType == RFIC_3853))
			{
				NetworkTypeList[0] = 3;                 // NumberOfItems = 3
				NetworkTypeList[1] = Ndis802_11DS;      // NetworkType[1] = 11b
				NetworkTypeList[2] = Ndis802_11OFDM24;  // NetworkType[2] = 11g
				NetworkTypeList[3] = Ndis802_11OFDM5;   // NetworkType[3] = 11a
                wrq->u.data.length = 16;
				Status = copy_to_user(wrq->u.data.pointer, &NetworkTypeList[0], wrq->u.data.length);
			}
			else
			{
				NetworkTypeList[0] = 2;                 // NumberOfItems = 2
				NetworkTypeList[1] = Ndis802_11DS;      // NetworkType[1] = 11b
				NetworkTypeList[2] = Ndis802_11OFDM24;  // NetworkType[2] = 11g
			    wrq->u.data.length = 12;
				Status = copy_to_user(wrq->u.data.pointer, &NetworkTypeList[0], wrq->u.data.length);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_NETWORK_TYPES_SUPPORTED\n"));
				break;
	    case OID_802_11_NETWORK_TYPE_IN_USE:
            wrq->u.data.length = sizeof(ULONG);
			if (pAd->CommonCfg.PhyMode == PHY_11A)
				ulInfo = Ndis802_11OFDM5;
			else if ((pAd->CommonCfg.PhyMode == PHY_11BG_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11G))
				ulInfo = Ndis802_11OFDM24;
			else
				ulInfo = Ndis802_11DS;
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;
        	case RT_OID_802_11_QUERY_LAST_RX_RATE:
            		ulInfo = (ULONG)pAd->LastRxRate;
            		wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_RX_RATE (=%ld)\n", ulInfo));
			break;
		case RT_OID_802_11_QUERY_LAST_TX_RATE:
			ulInfo = (ULONG)pAd->LastTxRate;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_TX_RATE (=%lx)\n", ulInfo));
			break;
        		case RT_OID_802_11_QUERY_MAP_REAL_RX_RATE:
			RateValue=0;
			HTPhyMode.word =(USHORT) pAd->LastRxRate;
			getRate(HTPhyMode, &RateValue);
			wrq->u.data.length = sizeof(RateValue);
			Status = copy_to_user(wrq->u.data.pointer, &RateValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_RX_RATE (=%ld)\n", RateValue));
			break;
		case RT_OID_802_11_QUERY_MAP_REAL_TX_RATE:
			RateValue=0;
			HTPhyMode.word = (USHORT)pAd->LastTxRate;
			getRate(HTPhyMode, &RateValue);
			wrq->u.data.length = sizeof(RateValue);
			Status = copy_to_user(wrq->u.data.pointer, &RateValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_TX_RATE (=%ld)\n", RateValue));
			break;
		case RT_OID_802_11_QUERY_TX_PHYMODE:
			ulInfo = (ULONG)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_TX_PHYMODE (=%lx)\n", ulInfo));
			break;
        case RT_OID_802_11_QUERY_EEPROM_VERSION:
            wrq->u.data.length = sizeof(ULONG);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->EepromVersion, wrq->u.data.length);
            break;
        case RT_OID_802_11_QUERY_FIRMWARE_VERSION:
            wrq->u.data.length = sizeof(ULONG);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->FirmwareVersion, wrq->u.data.length);
			break;
	    case RT_OID_802_11_QUERY_NOISE_LEVEL:
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->BbpWriteLatch[66], wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_NOISE_LEVEL (=%d)\n", pAd->BbpWriteLatch[66]));
			break;
	    case RT_OID_802_11_EXTRA_INFO:
			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->ExtraInfo, wrq->u.data.length);
	        DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_EXTRA_INFO (=%ld)\n", pAd->ExtraInfo));
	        break;
	    case RT_OID_WE_VERSION_COMPILED:
	        wrq->u.data.length = sizeof(UINT);
	        we_version_compiled = WIRELESS_EXT;
	        Status = copy_to_user(wrq->u.data.pointer, &we_version_compiled, wrq->u.data.length);
	        break;
		case RT_OID_802_11_QUERY_APSD_SETTING:
			apsd = (pAd->CommonCfg.bAPSDCapable | (pAd->CommonCfg.bAPSDAC_BE << 1) | (pAd->CommonCfg.bAPSDAC_BK << 2)
				| (pAd->CommonCfg.bAPSDAC_VI << 3)	| (pAd->CommonCfg.bAPSDAC_VO << 4)	| (pAd->CommonCfg.MaxSPLength << 5));

			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &apsd, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_APSD_SETTING (=0x%lx,APSDCap=%d,AC_BE=%d,AC_BK=%d,AC_VI=%d,AC_VO=%d,MAXSPLen=%d)\n", 
				apsd,pAd->CommonCfg.bAPSDCapable,pAd->CommonCfg.bAPSDAC_BE,pAd->CommonCfg.bAPSDAC_BK,pAd->CommonCfg.bAPSDAC_VI,pAd->CommonCfg.bAPSDAC_VO,pAd->CommonCfg.MaxSPLength));
			break;
		case RT_OID_802_11_QUERY_APSD_PSM:
			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bAPSDForcePowerSave, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_APSD_PSM (=%d)\n", pAd->CommonCfg.bAPSDForcePowerSave));			
			break;
		case RT_OID_802_11_QUERY_WMM:
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bWmmCapable, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_WMM (=%d)\n",	pAd->CommonCfg.bWmmCapable));
			break;


#ifdef WPA_SUPPLICANT_SUPPORT
        case RT_OID_NEW_DRIVER:
            {
                UCHAR enabled = 1;
    	        wrq->u.data.length = sizeof(UCHAR);
    	        Status = copy_to_user(wrq->u.data.pointer, &enabled, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_NEW_DRIVER (=%d)\n", enabled));
            }
	        break;
        case RT_OID_WPA_SUPPLICANT_SUPPORT:
	        wrq->u.data.length = sizeof(UCHAR);
	        Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.WpaSupplicantUP, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WPA_SUPPLICANT_SUPPORT (=%d)\n", pAd->StaCfg.WpaSupplicantUP));
	        break;
#endif // WPA_SUPPLICANT_SUPPORT //

#ifdef WSC_STA_SUPPORT
		case RT_OID_WSC_QUERY_STATUS:
			wrq->u.data.length = sizeof(INT);
			if (copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.WscControl.WscStatus, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_STATUS (=%d)\n", pAd->StaCfg.WscControl.WscStatus));
			break;

		case RT_OID_WSC_PIN_CODE:
			wrq->u.data.length = sizeof(UINT);
			WscPinCode = pAd->StaCfg.WscControl.WscEnrolleePinCode;
			
			if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_PIN_CODE (=%d)\n", WscPinCode));
			break;

        case RT_OID_WSC_QUERY_DEFAULT_PROFILE:
			wrq->u.data.length = sizeof(WSC_PROFILE);
			pProfile = kmalloc(sizeof(WSC_PROFILE), MEM_ALLOC_FLAG);
			RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
            WscCreateProfileFromCfg(pAd, STA_MODE, &pAd->StaCfg.WscControl, pProfile);
			if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("WSC Profile:\n"));
				DBGPRINT(RT_DEBUG_TRACE, ("SSID = %s\n", pProfile->Profile[0].SSID.Ssid));
				DBGPRINT(RT_DEBUG_TRACE, ("AuthType = %s\n", WscGetAuthTypeStr(pProfile->Profile[0].AuthType)));
				DBGPRINT(RT_DEBUG_TRACE, ("EncrpType = %s\n", WscGetEncryTypeStr(pProfile->Profile[0].EncrType)));

				if (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_WEP)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WEP Key = %s\n", pProfile->Profile[0].Key));
					DBGPRINT(RT_DEBUG_TRACE, ("DefaultKey ID = %d\n", pProfile->Profile[0].KeyIndex));
				}
				else if ((pProfile->Profile[0].EncrType == WSC_ENCRTYPE_TKIP) || (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_AES))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PassPhrase Key = %s\n", pProfile->Profile[0].Key));
                    pProfile->Profile[0].KeyIndex = 1;
				}
				DBGPRINT(RT_DEBUG_TRACE, ("\n"));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_DEFAULT_PROFILE \n"));
			break;
		case RT_OID_802_11_WSC_QUERY_PROFILE:
			wrq->u.data.length = sizeof(WSC_PROFILE);
			pProfile = kmalloc(sizeof(WSC_PROFILE), MEM_ALLOC_FLAG);
			RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
			NdisMoveMemory(pProfile, &pAd->StaCfg.WscControl.WscProfile, sizeof(WSC_PROFILE));
            if ((pProfile->Profile[0].AuthType == WSC_AUTHTYPE_OPEN) && (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_NONE))
            {
                pProfile->Profile[0].KeyLength = 0;
                NdisZeroMemory(pProfile->Profile[0].Key, 64);
            }
			if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("WSC Profile:\n"));
				DBGPRINT(RT_DEBUG_TRACE, ("SSID = %s\n", pProfile->Profile[0].SSID.Ssid));
				DBGPRINT(RT_DEBUG_TRACE, ("AuthType = %s\n", WscGetAuthTypeStr(pProfile->Profile[0].AuthType)));
				DBGPRINT(RT_DEBUG_TRACE, ("EncrpType = %s\n", WscGetEncryTypeStr(pProfile->Profile[0].EncrType)));

				if (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_WEP)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WEP Key = %s\n", pProfile->Profile[0].Key));
					DBGPRINT(RT_DEBUG_TRACE, ("DefaultKey ID = %d\n", pProfile->Profile[0].KeyIndex));
				}
				else if ((pProfile->Profile[0].EncrType == WSC_ENCRTYPE_TKIP) || (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_AES))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PassPhrase Key = %s\n", pProfile->Profile[0].Key));
                    pProfile->Profile[0].KeyIndex = 1;
				}
				DBGPRINT(RT_DEBUG_TRACE, ("\n"));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WSC_QUERY_PROFILE \n"));
			break;
			
		case RT_OID_WSC_UUID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_UUID \n"));
			wrq->u.data.length = UUID_LEN_STR;
			if (copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.WscControl.Wsc_Uuid_Str[0], UUID_LEN_STR))
			{
				Status = -EFAULT;
			}
			break;
		case RT_OID_WSC_MAC_ADDRESS:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_MAC_ADDRESS \n"));
			wrq->u.data.length = MAC_ADDR_LEN;
			if (copy_to_user(wrq->u.data.pointer, pAd->CurrentAddress, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;
#endif // WSC_STA_SUPPORT //
        case RT_OID_DRIVER_DEVICE_NAME:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_DRIVER_DEVICE_NAME \n"));
			wrq->u.data.length = 16;
			if (copy_to_user(wrq->u.data.pointer, pAd->StaCfg.dev_name, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
            break;
        case RT_OID_802_11_QUERY_HT_PHYMODE:
            pHTPhyMode = (OID_SET_HT_PHYMODE *) kmalloc(sizeof(OID_SET_HT_PHYMODE), MEM_ALLOC_FLAG);
            if (pHTPhyMode)
            {           
                pHTPhyMode->PhyMode = pAd->CommonCfg.PhyMode;
    			pHTPhyMode->HtMode = (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE;
    			pHTPhyMode->BW = (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.BW;
    			pHTPhyMode->MCS= (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MCS;
    			pHTPhyMode->SHORTGI= (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.ShortGI;
    			pHTPhyMode->STBC= (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.STBC;
    	
    			pHTPhyMode->ExtOffset = ((pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel) ? (EXTCHA_BELOW) : (EXTCHA_ABOVE));
                wrq->u.data.length = sizeof(OID_SET_HT_PHYMODE);
                if (copy_to_user(wrq->u.data.pointer, pHTPhyMode, wrq->u.data.length))
    			{
    				Status = -EFAULT;
    			}
    			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_HT_PHYMODE (PhyMode = %d, MCS =%d, BW = %d, STBC = %d, ExtOffset=%d)\n",
    				pHTPhyMode->HtMode, pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->ExtOffset));
    			DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (.word = %x )\n", pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case RT_OID_802_11_COUNTRY_REGION:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_COUNTRY_REGION \n"));
			wrq->u.data.length = sizeof(ulInfo);
            ulInfo = pAd->CommonCfg.CountryRegionForABand;
            ulInfo = (ulInfo << 8)|(pAd->CommonCfg.CountryRegion);
			if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
            {
				Status = -EFAULT;
            }
            break;
        case RT_OID_802_11_QUERY_DAT_HT_PHYMODE:
            pHTPhyMode = (OID_SET_HT_PHYMODE *) kmalloc(sizeof(OID_SET_HT_PHYMODE), MEM_ALLOC_FLAG);
            if (pHTPhyMode)
            {           
                pHTPhyMode->PhyMode = pAd->CommonCfg.PhyMode;
    			pHTPhyMode->HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
    			pHTPhyMode->BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
    			pHTPhyMode->MCS= (UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.MCS;
    			pHTPhyMode->SHORTGI= (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;
    			pHTPhyMode->STBC= (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
    	
                wrq->u.data.length = sizeof(OID_SET_HT_PHYMODE);
                if (copy_to_user(wrq->u.data.pointer, pHTPhyMode, wrq->u.data.length))
    			{
    				Status = -EFAULT;
    			}
    			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_HT_PHYMODE (PhyMode = %d, MCS =%d, BW = %d, STBC = %d, ExtOffset=%d)\n",
    				pHTPhyMode->HtMode, pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->ExtOffset));
    			DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (.word = %x )\n", pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case RT_OID_QUERY_MULTIPLE_CARD_SUPPORT:            
			wrq->u.data.length = sizeof(UCHAR);
            i = 0;
#ifdef MULTIPLE_CARD_SUPPORT
            i = 1;
#endif // MULTIPLE_CARD_SUPPORT //
			if (copy_to_user(wrq->u.data.pointer, &i, wrq->u.data.length))
            {
				Status = -EFAULT;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_QUERY_MULTIPLE_CARD_SUPPORT(=%d) \n", i));
            break;
#ifdef SNMP_SUPPORT
		case RT_OID_802_11_MAC_ADDRESS:
            wrq->u.data.length = MAC_ADDR_LEN;
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTUREROUI:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTUREROUI \n"));
			wrq->u.data.length = ManufacturerOUI_LEN;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTURERNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTURERNAME \n"));
			wrq->u.data.length = strlen(ManufacturerNAME);
			Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
			break;

		case RT_OID_802_11_RESOURCETYPEIDNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_RESOURCETYPEIDNAME \n"));
			wrq->u.data.length = strlen(ResourceTypeIdName);
			Status = copy_to_user(wrq->u.data.pointer, ResourceTypeIdName, wrq->u.data.length);
			break;

		case RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED \n"));
			ulInfo = 1; // 1 is support wep else 2 is not support.
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;

		case RT_OID_802_11_POWERMANAGEMENTMODE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_POWERMANAGEMENTMODE \n"));
			if (pAd->StaCfg.Psm == PSMP_ACTION)
				ulInfo = 1; // 1 is power active else 2 is power save.
			else
				ulInfo = 2;
			
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;

		case OID_802_11_WEPDEFAULTKEYVALUE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WEPDEFAULTKEYVALUE \n"));
			//KeyIdxValue.KeyIdx = pAd->PortCfg.MBSSID[pAd->IoctlIF].DefaultKeyId;
			pKeyIdxValue = wrq->u.data.pointer;
			DBGPRINT(RT_DEBUG_TRACE,("KeyIdxValue.KeyIdx = %d, \n",pKeyIdxValue->KeyIdx));
			valueLen = pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen;
			NdisMoveMemory(pKeyIdxValue->Value,
						   &pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key,
						   valueLen);
			pKeyIdxValue->Value[valueLen]='\0';

			wrq->u.data.length = sizeof(DefaultKeyIdxValue);

			Status = copy_to_user(wrq->u.data.pointer, pKeyIdxValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE,("DefaultKeyId = %d, total len = %d, str len=%d, KeyValue= %02x %02x %02x %02x \n", 
										pAd->StaCfg.DefaultKeyId, 
										wrq->u.data.length, 
										pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen,
										pAd->SharedKey[BSS0][0].Key[0],
										pAd->SharedKey[BSS0][1].Key[0],
										pAd->SharedKey[BSS0][2].Key[0],
										pAd->SharedKey[BSS0][3].Key[0]));
			break;

		case OID_802_11_WEPDEFAULTKEYID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WEPDEFAULTKEYID \n"));
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.DefaultKeyId, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("DefaultKeyId =%d \n", pAd->StaCfg.DefaultKeyId));
			break;

		case RT_OID_802_11_WEPKEYMAPPINGLENGTH:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WEPKEYMAPPINGLENGTH \n"));
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer,
									&pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen,
									wrq->u.data.length);
			break;

		case OID_802_11_SHORTRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SHORTRETRYLIMIT \n"));
			wrq->u.data.length = sizeof(ULONG);
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
			ShortRetryLimit = tx_rty_cfg.field.ShortRtyLimit;
			DBGPRINT(RT_DEBUG_TRACE, ("ShortRetryLimit =%ld,  tx_rty_cfg.field.ShortRetryLimit=%d\n", ShortRetryLimit, tx_rty_cfg.field.ShortRtyLimit));
			Status = copy_to_user(wrq->u.data.pointer, &ShortRetryLimit, wrq->u.data.length);
			break;

		case OID_802_11_LONGRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_LONGRETRYLIMIT \n"));
			wrq->u.data.length = sizeof(ULONG);
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
			LongRetryLimit = tx_rty_cfg.field.LongRtyLimit;
			DBGPRINT(RT_DEBUG_TRACE, ("LongRetryLimit =%ld,  tx_rty_cfg.field.LongRtyLimit=%d\n", LongRetryLimit, tx_rty_cfg.field.LongRtyLimit));
			Status = copy_to_user(wrq->u.data.pointer, &LongRetryLimit, wrq->u.data.length);
			break;
			
		case RT_OID_802_11_PRODUCTID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PRODUCTID \n"));
		
#ifdef RTMP_MAC_PCI
			{
			
				USHORT  device_id=0;
				if (((POS_COOKIE)pAd->OS_Cookie)->pci_dev != NULL)
			    	pci_read_config_word(((POS_COOKIE)pAd->OS_Cookie)->pci_dev, PCI_DEVICE_ID, &device_id);
				else 
					DBGPRINT(RT_DEBUG_TRACE, (" pci_dev = NULL\n"));
				sprintf((PSTRING)tmp, "%04x %04x\n", NIC_PCI_VENDOR_ID, device_id);
			}
#endif // RTMP_MAC_PCI //
			wrq->u.data.length = strlen((PSTRING)tmp);
			Status = copy_to_user(wrq->u.data.pointer, tmp, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTUREID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTUREID \n"));
			wrq->u.data.length = strlen(ManufacturerNAME);
			Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
			break;

		case OID_802_11_CURRENTCHANNEL:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CURRENTCHANNEL \n"));
			wrq->u.data.length = sizeof(UCHAR);
			DBGPRINT(RT_DEBUG_TRACE, ("sizeof UCHAR=%d, channel=%d \n", sizeof(UCHAR), pAd->CommonCfg.Channel));
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Channel, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;
#endif //SNMP_SUPPORT
		
		case OID_802_11_BUILD_CHANNEL_EX:
			{
				UCHAR value;
				DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BUILD_CHANNEL_EX \n"));
				wrq->u.data.length = sizeof(UCHAR);
#ifdef EXT_BUILD_CHANNEL_LIST
				DBGPRINT(RT_DEBUG_TRACE, ("Support EXT_BUILD_CHANNEL_LIST.\n"));
				value = 1;
#else
				DBGPRINT(RT_DEBUG_TRACE, ("Doesn't support EXT_BUILD_CHANNEL_LIST.\n"));
				value = 0;
#endif // EXT_BUILD_CHANNEL_LIST //
				Status = copy_to_user(wrq->u.data.pointer, &value, 1);
				DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			}
			break;

		case OID_802_11_GET_CH_LIST:
			{
				PRT_CHANNEL_LIST_INFO pChListBuf;

				DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_GET_CH_LIST \n"));
				if (pAd->ChannelListNum == 0)
				{
					wrq->u.data.length = 0;
					break;
				}

				pChListBuf = (RT_CHANNEL_LIST_INFO *) kmalloc(sizeof(RT_CHANNEL_LIST_INFO), MEM_ALLOC_FLAG);
				if (pChListBuf == NULL)
				{
					wrq->u.data.length = 0;
					break;
				}

				pChListBuf->ChannelListNum = pAd->ChannelListNum;
				for (i = 0; i < pChListBuf->ChannelListNum; i++)
					pChListBuf->ChannelList[i] = pAd->ChannelList[i].Channel;

				wrq->u.data.length = sizeof(RT_CHANNEL_LIST_INFO);
				Status = copy_to_user(wrq->u.data.pointer, pChListBuf, sizeof(RT_CHANNEL_LIST_INFO));
				DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));

				if (pChListBuf)
					kfree(pChListBuf);
			}
			break;

		case OID_802_11_GET_COUNTRY_CODE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_GET_COUNTRY_CODE \n"));
			wrq->u.data.length = 2;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.CountryCode, 2);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;

#ifdef EXT_BUILD_CHANNEL_LIST
		case OID_802_11_GET_CHANNEL_GEOGRAPHY:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_GET_CHANNEL_GEOGRAPHY \n"));
			wrq->u.data.length = 1;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Geography, 1);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;			
#endif // EXT_BUILD_CHANNEL_LIST //


#ifdef QOS_DLS_SUPPORT
		case RT_OID_802_11_QUERY_DLS:
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bDLSCapable, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_DLS(=%d)\n", pAd->CommonCfg.bDLSCapable));
			break;

		case RT_OID_802_11_QUERY_DLS_PARAM:
			{
				PRT_802_11_DLS_INFO	pDlsInfo = kmalloc(sizeof(RT_802_11_DLS_INFO), GFP_ATOMIC);
				if (pDlsInfo == NULL)
					break;

				for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++)
				{
					RTMPMoveMemory(&pDlsInfo->Entry[i], &pAd->StaCfg.DLSEntry[i], sizeof(RT_802_11_DLS_UI));
				}

				pDlsInfo->num = MAX_NUM_OF_DLS_ENTRY;
				wrq->u.data.length = sizeof(RT_802_11_DLS_INFO);
				Status = copy_to_user(wrq->u.data.pointer, pDlsInfo, wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_DLS_PARAM\n"));

				if (pDlsInfo)
					kfree(pDlsInfo);
			}
			break;
#endif // QOS_DLS_SUPPORT //
#ifdef WAPI_SUPPORT
		case OID_802_11_WAPI_CONFIGURATION:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::Get WAPI Configuration(%d)\n", sizeof(WAPI_CONF)));
			RTMPIoctlQueryWapiConf(pAd, wrq);	
			break;			
		case OID_802_11_WAPI_IE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WAPI_IE\n"));
			if (wrq->u.data.length != sizeof(WAPI_WIE_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_WIE_STRUCT   wapi_ie;
				MAC_TABLE_ENTRY		*pEntry;

				NdisZeroMemory(&wapi_ie, sizeof(WAPI_WIE_STRUCT));
				NdisMoveMemory(wapi_ie.addr, wrq->u.data.pointer, MAC_ADDR_LEN);

				pEntry = &pAd->MacTab.Content[BSSID_WCID];
						
				if ((NdisEqualMemory(pEntry->Addr, wapi_ie.addr, MAC_ADDR_LEN)) && 
					(pEntry->RSNIE_Len > 0))
				{										
					wapi_ie.wie_len = pEntry->RSNIE_Len;
					NdisMoveMemory(wapi_ie.wie, pEntry->RSN_IE, pEntry->RSNIE_Len);						
				}
								
				if (copy_to_user(wrq->u.data.pointer, &wapi_ie, wrq->u.data.length))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
				}								
            }
			break;		
#endif // WAPI_SUPPORT //

#ifdef XLINK_SUPPORT
		case OID_802_11_SET_PSPXLINK_MODE:
			wrq->u.data.length = sizeof(BOOLEAN);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.PSPXlink, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SET_PSPXLINK_MODE(=%d)\n", pAd->StaCfg.PSPXlink));
			break;
#endif // XLINK_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
		case OID_802_11_QUERY_WirelessMode:
			wrq->u.data.length = sizeof(UCHAR);
	    	Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.PhyMode, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_QUERY_WirelessMode(=%d)\n", pAd->CommonCfg.PhyMode));
			break;
#endif // RTMP_RBUS_SUPPORT //

    default:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::unknown IOCTL's subcmd = 0x%08x\n", cmd));
            Status = -EOPNOTSUPP;
            break;
    }
    return Status;
}

INT rt28xx_sta_ioctl(
	IN	struct net_device	*net_dev, 
	IN	OUT	struct ifreq	*rq, 
	IN	INT					cmd)
{
	POS_COOKIE			pObj;
	RTMP_ADAPTER        *pAd = NULL;
	struct iwreq        *wrq = (struct iwreq *) rq;
	BOOLEAN				StateMachineTouched = FALSE;
	INT					Status = NDIS_STATUS_SUCCESS;
	USHORT				subcmd;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
    //check if the interface is down
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	    if (wrq->u.data.pointer == NULL)
	    {
		    return Status;
	    }

	    if (strstr(wrq->u.data.pointer, "OpMode") == NULL)
#endif // CONFIG_APSTA_MIXED_SUPPORT //
		{
            DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		    return -ENETDOWN;  
        }
    }

	{	// determine this ioctl command is comming from which interface.
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
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
			DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIFHWADDR\n"));
			memcpy(wrq->u.name, pAd->CurrentAddress, ETH_ALEN);
			break;	
		case SIOCGIWNAME:
        {
        	char *name=&wrq->u.name[0];
        	rt_ioctl_giwname(net_dev, NULL, name, NULL);
            break;
		}
		case SIOCGIWESSID:  //Get ESSID
        {
        	struct iw_point *essid=&wrq->u.essid;
        	rt_ioctl_giwessid(net_dev, NULL, essid, essid->pointer);
            break;
		}
		case SIOCSIWESSID:  //Set ESSID
        	{
        	struct iw_point	*essid=&wrq->u.essid;
        	rt_ioctl_siwessid(net_dev, NULL, essid, essid->pointer);
            break;  
		}
		case SIOCSIWNWID:   // set network id (the cell)
		case SIOCGIWNWID:   // get network id
			Status = -EOPNOTSUPP;
			break;
		case SIOCSIWFREQ:   //set channel/frequency (Hz)
        	{
        	struct iw_freq *freq=&wrq->u.freq;
        	rt_ioctl_siwfreq(net_dev, NULL, freq, NULL);
			break;
		}
		case SIOCGIWFREQ:   // get channel/frequency (Hz)
        	{
        	struct iw_freq *freq=&wrq->u.freq;
        	rt_ioctl_giwfreq(net_dev, NULL, freq, NULL);
			break;
		}
		case SIOCSIWNICKN: //set node name/nickname
        	{
        	//struct iw_point *data=&wrq->u.data;
        	//rt_ioctl_siwnickn(net_dev, NULL, data, NULL);
			break;
			}
		case SIOCGIWNICKN: //get node name/nickname
        {
			struct iw_point	*erq = NULL;
        	erq = &wrq->u.data;
            erq->length = strlen((PSTRING) pAd->nickname);
            Status = copy_to_user(erq->pointer, pAd->nickname, erq->length);
			break;
		}
		case SIOCGIWRATE:   //get default bit rate (bps)
		    rt_ioctl_giwrate(net_dev, NULL, &wrq->u, NULL);
            break;
	    case SIOCSIWRATE:  //set default bit rate (bps)
	        rt_ioctl_siwrate(net_dev, NULL, &wrq->u, NULL);
            break;
        case SIOCGIWRTS:  // get RTS/CTS threshold (bytes)
        	{
        	struct iw_param *rts=&wrq->u.rts;
        	rt_ioctl_giwrts(net_dev, NULL, rts, NULL);
            break;
		}
        case SIOCSIWRTS:  //set RTS/CTS threshold (bytes)
        	{
        	struct iw_param *rts=&wrq->u.rts;
        	rt_ioctl_siwrts(net_dev, NULL, rts, NULL);
            break;
		}
        case SIOCGIWFRAG:  //get fragmentation thr (bytes)
        	{
        	struct iw_param *frag=&wrq->u.frag;
        	rt_ioctl_giwfrag(net_dev, NULL, frag, NULL);
            break;
		}
        case SIOCSIWFRAG:  //set fragmentation thr (bytes)
        	{
        	struct iw_param *frag=&wrq->u.frag;
        	rt_ioctl_siwfrag(net_dev, NULL, frag, NULL);
            break;
		}
        case SIOCGIWENCODE:  //get encoding token & mode
        	{
        	struct iw_point *erq=&wrq->u.encoding;
        	if(erq)
        		rt_ioctl_giwencode(net_dev, NULL, erq, erq->pointer);
            break;
		}
        case SIOCSIWENCODE:  //set encoding token & mode
        	{
        	struct iw_point *erq=&wrq->u.encoding;
        	if(erq)
        		rt_ioctl_siwencode(net_dev, NULL, erq, erq->pointer);
            break;
		}
		case SIOCGIWAP:     //get access point MAC addresses
        	{
        	struct sockaddr *ap_addr=&wrq->u.ap_addr;
        	rt_ioctl_giwap(net_dev, NULL, ap_addr, ap_addr->sa_data);
			break;
		}
	    case SIOCSIWAP:  //set access point MAC addresses
        	{
        	struct sockaddr *ap_addr=&wrq->u.ap_addr;
        	rt_ioctl_siwap(net_dev, NULL, ap_addr, ap_addr->sa_data);
            break;
		}
		case SIOCGIWMODE:   //get operation mode
        	{
        	__u32 *mode=&wrq->u.mode;
        	rt_ioctl_giwmode(net_dev, NULL, mode, NULL);
            break;
		}
		case SIOCSIWMODE:   //set operation mode
        	{
        	__u32 *mode=&wrq->u.mode;
        	rt_ioctl_siwmode(net_dev, NULL, mode, NULL);
            break;
		}
		case SIOCGIWSENS:   //get sensitivity (dBm)
		case SIOCSIWSENS:	//set sensitivity (dBm)
		case SIOCGIWPOWER:  //get Power Management settings
		case SIOCSIWPOWER:  //set Power Management settings
		case SIOCGIWTXPOW:  //get transmit power (dBm)
		case SIOCSIWTXPOW:  //set transmit power (dBm)
		case SIOCGIWRANGE:	//Get range of parameters
		case SIOCGIWRETRY:	//get retry limits and lifetime
		case SIOCSIWRETRY:	//set retry limits and lifetime
			Status = -EOPNOTSUPP;
			break;

		case RT_PRIV_IOCTL:
        case RT_PRIV_IOCTL_EXT:
			subcmd = wrq->u.data.flags;
			if( subcmd & OID_GET_SET_TOGGLE)
				Status = RTMPSetInformation(pAd, rq, subcmd);
			else
				Status = RTMPQueryInformation(pAd, rq, subcmd);
			break;		
		case SIOCGIWPRIV:
			if (wrq->u.data.pointer) 
			{
				if ( access_ok(VERIFY_WRITE, wrq->u.data.pointer, sizeof(privtab)) != TRUE)
					break;
				if ((sizeof(privtab) / sizeof(privtab[0])) <= wrq->u.data.length)
				{
					wrq->u.data.length = sizeof(privtab) / sizeof(privtab[0]);
					if (copy_to_user(wrq->u.data.pointer, privtab, sizeof(privtab)))
						Status = -EFAULT;
				}
				else
					Status = -E2BIG;
			}
			break;
		case RTPRIV_IOCTL_SET:
			if(access_ok(VERIFY_READ, wrq->u.data.pointer, wrq->u.data.length) != TRUE)   
					break;
			rt_ioctl_setparam(net_dev, NULL, NULL, wrq->u.data.pointer);
			break;
		case RTPRIV_IOCTL_GSITESURVEY:
			RTMPIoctlGetSiteSurvey(pAd, wrq);
		    break;			
#ifdef DBG
		case RTPRIV_IOCTL_MAC:
			RTMPIoctlMAC(pAd, wrq);
			break;
		case RTPRIV_IOCTL_E2P:
			RTMPIoctlE2PROM(pAd, wrq);
			break;
#ifdef RTMP_RF_RW_SUPPORT
		case RTPRIV_IOCTL_RF:
			RTMPIoctlRF(pAd, wrq);
			break;
#endif // RTMP_RF_RW_SUPPORT //
#endif // DBG //

        case SIOCETHTOOL:
                break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("IOCTL::unknown IOCTL's cmd = 0x%08x\n", cmd));
			Status = -EOPNOTSUPP;
			break;
	}

    if(StateMachineTouched) // Upper layer sent a MLME-related operations
    	RTMP_MLME_HANDLER(pAd);

	return Status;
}

#ifdef DBG
/* 
    ==========================================================================
    Description:
        Read / Write MAC
    Arguments:
        pAd                    Pointer to our adapter
        wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 mac 0        ==> read MAC where Addr=0x0
               2.) iwpriv ra0 mac 0=12     ==> write MAC where Addr=0x0, value=12
    ==========================================================================
*/
VOID RTMPIoctlMAC(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	INT					j = 0, k = 0;
	STRING				msg[1024];
	STRING				arg[255];
	ULONG				macAddr = 0;
	UCHAR				temp[16];
	STRING				temp2[16];
	UINT32				macValue = 0;
	INT					Status;
	BOOLEAN				bIsPrintAllMAC = FALSE;


	memset(msg, 0x00, 1024);
	if (wrq->u.data.length > 1) //No parameters.
	{   
	    Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		sprintf(msg, "\n");
		
		//Parsing Read or Write
	    this_char = arg;
		if (!*this_char)
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;

		if (!value || !*value)
		{ //Read
			// Sanity check
			if(strlen(this_char) > 4)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					return;
			}

			// Mac Addr
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}
			
			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			if(strlen(this_char) == 4)
			{
				AtoH(this_char, temp, 2);
				macAddr = *temp*256 + temp[1];					
				if (macAddr < 0xFFFF)
				{
					RTMP_IO_READ32(pAd, macAddr, &macValue);
					DBGPRINT(RT_DEBUG_TRACE, ("MacAddr=%lx, MacValue=%x\n", macAddr, macValue));
					sprintf(msg+strlen(msg), "[0x%08lX]:%08X  ", macAddr , macValue);
				}
				else
				{//Invalid parametes, so default printk all mac
					bIsPrintAllMAC = TRUE;
					goto next;
				}
			}
		}
		else
		{ //Write
			memcpy(&temp2, value, strlen(value));
			temp2[strlen(value)] = '\0';

			// Sanity check
			if((strlen(this_char) > 4) || strlen(temp2) > 8)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					return;
			}

			j = strlen(temp2);
			while(j-- > 0)
			{
				if(temp2[j] > 'f' || temp2[j] < '0')
					return;
			}

			//MAC Addr
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}

			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			//MAC value
			k = j = strlen(temp2);
			while(j-- > 0)
			{
				temp2[8-k+j] = temp2[j];
			}
			
			while(k < 8)
				temp2[7-k++]='0';
			temp2[8]='\0';

			{
				AtoH(this_char, temp, 2);
				macAddr = *temp*256 + temp[1];

				AtoH(temp2, temp, 4);
				macValue = *temp*256*256*256 + temp[1]*256*256 + temp[2]*256 + temp[3];

				// debug mode
				if (macAddr == (HW_DEBUG_SETTING_BASE + 4))
				{
					// 0x2bf4: byte0 non-zero: enable R17 tuning, 0: disable R17 tuning
                    if (macValue & 0x000000ff) 
                    {
                        pAd->BbpTuning.bEnable = TRUE;
                        DBGPRINT(RT_DEBUG_TRACE,("turn on R17 tuning\n"));
                    }
                    else
                    {
                        UCHAR R66;
                        pAd->BbpTuning.bEnable = FALSE;
                        R66 = 0x26 + GET_LNA_GAIN(pAd);
#ifdef RALINK_ATE
						if (ATE_ON(pAd))
						{
#ifdef RTMP_RBUS_SUPPORT
							// TODO: Shiang, we need to add MACVersion Check here!!!!
#if defined(RT2883) || defined(RT3883)
							if (IS_RT2883(pAd) || IS_RT3883(pAd))
							{
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x20);
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x40);
							}
#endif // defined(RT2883) || defined(RT3883) //
#endif // RTMP_RBUS_SUPPORT //
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
						}
						else
#endif // RALINK_ATE //
						{
#if defined(RT2883) || defined(RT3883)
							if (IS_RT2883(pAd) || IS_RT3883(pAd))
							{
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x20);
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x40);
							}
#endif // defined(RT2883) || defined(RT3883) //
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
						}
                        DBGPRINT(RT_DEBUG_TRACE,("turn off R17 tuning, restore to 0x%02x\n", R66));
                    }
					return;
				}

				DBGPRINT(RT_DEBUG_TRACE, ("MacAddr=%02lx, MacValue=0x%x\n", macAddr, macValue));
				
				RTMP_IO_WRITE32(pAd, macAddr, macValue);
				sprintf(msg+strlen(msg), "[0x%08lX]:%08X  ", macAddr, macValue);
			}
		}
	}
	else
		bIsPrintAllMAC = TRUE;
next:
	if (bIsPrintAllMAC)
	{
		struct file		*file_w;
		PSTRING			fileName = "MacDump.txt";
		mm_segment_t	orig_fs;

		orig_fs = get_fs();
		set_fs(KERNEL_DS); 

		// open file
		file_w = filp_open(fileName, O_WRONLY|O_CREAT, 0);
		if (IS_ERR(file_w)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("-->2) %s: Error %ld opening %s\n", __FUNCTION__, -PTR_ERR(file_w), fileName));
		}
		else 
		{
			if (file_w->f_op && file_w->f_op->write) 
			{
				file_w->f_pos = 0;
				macAddr = 0x1000;
				
				while (macAddr <= 0x1800)
				{
					RTMP_IO_READ32(pAd, macAddr, &macValue);
					sprintf(msg, "%08lx = %08X\n", macAddr, macValue);
					
					// write data to file
					file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);
					
					printk("%s", msg);
					macAddr += 4;
				}
				sprintf(msg, "\nDump all MAC values to %s\n", fileName);
			}
			filp_close(file_w, NULL);
		}
		set_fs(orig_fs); 
	}
	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");

	// Copy the information into the user buffer
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlMAC\n\n"));
}

/* 
    ==========================================================================
    Description:
        Read / Write E2PROM
    Arguments:
        pAd                    Pointer to our adapter
        wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 e2p 0     	==> read E2PROM where Addr=0x0
               2.) iwpriv ra0 e2p 0=1234    ==> write E2PROM where Addr=0x0, value=1234
    ==========================================================================
*/
VOID RTMPIoctlE2PROM(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	INT					j = 0, k = 0;
	STRING				msg[1024];
	STRING				arg[255];
	USHORT				eepAddr = 0;
	UCHAR				temp[16];
	STRING				temp2[16];
	USHORT				eepValue;
	int					Status;
	BOOLEAN				bIsPrintAllE2P = FALSE;

	
	memset(msg, 0x00, 1024);
	if (wrq->u.data.length > 1) //No parameters.
	{   
	    Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		sprintf(msg, "\n");

	    //Parsing Read or Write
		this_char = arg;
		
		
		if (!*this_char)
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;

		if (!value || !*value)
		{ //Read

			// Sanity check
			if(strlen(this_char) > 4)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					return;
			}

			// E2PROM addr
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}
			
			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			if(strlen(this_char) == 4)
			{
				AtoH(this_char, temp, 2);
				eepAddr = *temp*256 + temp[1];					
				if (eepAddr < 0xFFFF)
				{
					RT28xx_EEPROM_READ16(pAd, eepAddr, eepValue);
					sprintf(msg+strlen(msg), "[0x%04X]:0x%04X  ", eepAddr , eepValue);
				}
				else
				{//Invalid parametes, so default printk all bbp
					bIsPrintAllE2P = TRUE;
					goto next;
				}
			}
		}
		else
		{ //Write
			memcpy(&temp2, value, strlen(value));
			temp2[strlen(value)] = '\0';

			// Sanity check
			if((strlen(this_char) > 4) || strlen(temp2) > 8)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					return;
			}
			j = strlen(temp2);
			while(j-- > 0)
			{
				if(temp2[j] > 'f' || temp2[j] < '0')
					return;
			}

			//MAC Addr
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}

			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			//MAC value
			k = j = strlen(temp2);
			while(j-- > 0)
			{
				temp2[4-k+j] = temp2[j];
			}
			
			while(k < 4)
				temp2[3-k++]='0';
			temp2[4]='\0';

			AtoH(this_char, temp, 2);
			eepAddr = *temp*256 + temp[1];

			AtoH(temp2, temp, 2);
			eepValue = *temp*256 + temp[1];

			RT28xx_EEPROM_WRITE16(pAd, eepAddr, eepValue);
			sprintf(msg+strlen(msg), "[0x%02X]:%02X  ", eepAddr, eepValue);
		}
	}
	else
		bIsPrintAllE2P = TRUE;
next:
	if (bIsPrintAllE2P)
	{
		struct file		*file_w;
		PSTRING			fileName = "EEPROMDump.txt";
		mm_segment_t	orig_fs;

		orig_fs = get_fs();
		set_fs(KERNEL_DS); 

		// open file
		file_w = filp_open(fileName, O_WRONLY|O_CREAT, 0);
		if (IS_ERR(file_w)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("-->2) %s: Error %ld opening %s\n", __FUNCTION__, -PTR_ERR(file_w), fileName));
		}
		else 
		{
			if (file_w->f_op && file_w->f_op->write) 
			{
				file_w->f_pos = 0;
				eepAddr = 0x00;
				
				while (eepAddr <= 0xFE)
				{
					RT28xx_EEPROM_READ16(pAd, eepAddr, eepValue);
					sprintf(msg, "%08x = %04x\n", eepAddr , eepValue);
					
					// write data to file
					file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);
					
					printk("%s", msg);
					eepAddr += 2;
				}
				sprintf(msg, "\nDump all EEPROM values to %s\n", fileName);
			}
			filp_close(file_w, NULL);
		}
		set_fs(orig_fs); 
	}
	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");


	// Copy the information into the user buffer
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlE2PROM\n"));	
}
#endif // DBG //


