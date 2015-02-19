/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All CFG80211 Function Prototype.

***************************************************************************/


#ifndef __CFG80211CMM_H__
#define __CFG80211CMM_H__

#ifdef RT_CFG80211_SUPPORT

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE 	
#define CFG80211_GetEventDevice(__pAd) __pAd->cfg80211_ctrl.dummy_p2p_net_dev
#else
#define CFG80211_GetEventDevice(__pAd) __pAd->net_dev
#endif	/* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef CFG_TDLS_SUPPORT
#define CATEGORY_TDLS				12
#define	PROTO_NAME_TDLS				2
#define TDLS_TIMEOUT				5000	// unit: msec
#define IE_TDLS_LINK_IDENTIFIER			101
#define TDLS_ELM_LEN_LINK_IDENTIFIER		18

enum tdls_operation {
	TDLS_DISCOVERY_REQ,
	TDLS_SETUP,
	TDLS_TEARDOWN,
	TDLS_ENABLE_LINK,
	TDLS_DISABLE_LINK,
};

enum tdls_entry_op
{
	tdls_insert_entry=0,
	tdls_delete_entry
};
#endif /* CFG_TDLS_SUPPORT */


#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211CMM_H__ */


