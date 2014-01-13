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
	rt_config.h

	Abstract:
	Central header file to maintain all include files for all NDIS
	miniport driver routines.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Paul Lin    08-01-2002    created

*/
#ifndef	__RT_CONFIG_H__
#define	__RT_CONFIG_H__

/* #define WDS_VLAN_SUPPORT */

#include "rtmp_comm.h"
/*#include "rtmp_type.h" */
/*#include "rtmp_os.h" */

#include "rtmp_def.h"
#include "rtmp_chip.h"
#include "rtmp_timer.h"


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
#include "cfg80211extr.h"
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef CONFIG_STA_SUPPORT
#ifdef AGS_SUPPORT
#include "ags.h"
#endif /* AGS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#include "mlme.h"
#include "crypt_md5.h"
#include "crypt_sha2.h"
#include "crypt_hmac.h"
#include "crypt_aes.h"
#include "crypt_arc4.h"
/*#include "rtmp_cmd.h" */
#include "rtmp.h"
#include "ap.h"
#include "wpa.h"
#include "chlist.h"
#include "spectrum.h"
#ifdef CONFIG_AP_SUPPORT
#include "ap_autoChSel.h"
#endif /* CONFIG_AP_SUPPORT */
#include "rt_os_util.h"

#include "eeprom.h"
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
#include "rtmp_mcu.h"
#endif

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#include "rt_os_net.h"

#include "rt_cal.h"

#ifdef UAPSD_SUPPORT
#include "uapsd.h"
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#include "ap_mbss.h"
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
#include "ap_wds.h"
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
#include "ap_apcli.h"
#endif /* APCLI_SUPPORT */

#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */

#include "ap_ids.h"
#include "ap_cfg.h"

#ifdef CLIENT_WDS
#include "client_wds.h"
#endif /* CLIENT_WDS */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
#define STA_WSC_INCLUDED
#endif /* WSC_STA_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef BLOCK_NET_IF
#include "netif_block.h"
#endif /* BLOCK_NET_IF */

#ifdef IGMP_SNOOP_SUPPORT
#include "igmp_snoop.h"
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef RALINK_ATE
#include "rt_ate.h"
#endif /* RALINK_ATE */

#ifdef RALINK_QA
#include "rt_qa.h"
#endif /* RALINK_QA */

#ifdef RALINK_QA
#ifndef RALINK_ATE
#error "For supporting QA GUI, please set HAS_ATE=y and HAS_QA_SUPPORT=y."
#endif /* RALINK_ATE */
#endif /* RALINK_QA */


#ifdef RTMP_RBUS_SUPPORT
#ifdef RT3XXX_ANTENNA_DIVERSITY_SUPPORT
#include "ap_diversity.h"
#endif /* RT3XXX_ANTENNA_DIVERSITY_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef WAPI_SUPPORT
#include "wapi.h"
#endif /* WAPI_SUPPORT */


#ifdef DOT11Z_TDLS_SUPPORT
#include "tdls.h"
#include "tdls_uapsd.h"
#endif /* DOT11Z_TDLS_SUPPORT */

#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
#define RALINK_PASSPHRASE	"Ralink"
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
#ifndef APCLI_SUPPORT
#error "Build Apcli for being controlled by NetworkManager or wext, please set HAS_APCLI_SUPPORT=y and HAS_APCLI_WPA_SUPPLICANT=y"
#endif /* APCLI_SUPPORT */
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#ifndef WPA_SUPPLICANT_SUPPORT
#error "Build for being controlled by NetworkManager or wext, please set HAS_WPA_SUPPLICANT=y and HAS_NATIVE_WPA_SUPPLICANT_SUPPORT=y"
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

#endif /* CONFIG_STA_SUPPORT */

#ifdef WSC_INCLUDED
#include "crypt_biginteger.h"
#include "crypt_dh.h"
#include "wsc_tlv.h"
#endif /* WSC_INCLUDED */


#ifdef IKANOS_VX_1X0
#include "vr_ikans.h"
#endif /* IKANOS_VX_1X0 */





#ifdef WAPI_SUPPORT
#include "wapi.h"
#endif /* WAPI_SUPPORT */

#ifdef P2P_SUPPORT
/*#include "p2p_inf.h" */
#include "p2p.h"
#include "p2pcli.h"
#endif /* P2P_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#include "sta_cfg.h"
#endif /* CONFIG_STA_SUPPORT */



#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
#include "video.h"
#endif /* VIDEO_TURBINE_SUPPORT */

#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif /* WORKQUEUE_BH / */
#endif /* RTMP_RBUS_SUPPORT */


#ifdef TXBF_SUPPORT
#include "rt_txbf.h"
#endif /* TXBF_SUPPORT */


#endif	/* __RT_CONFIG_H__ */

