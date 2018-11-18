 /***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
    protection.h

    Abstract:
    Generic 802.11 Legacy/HT/nonHT Protection Mechanism

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Hugo        2016-0505     create

*/

#ifndef __PROTECTION_H__
#define __PROTECTION_H__

struct _MAC_TABLE_ENTRY;
struct _RTMP_ADAPTER;

struct erp_protect {
	UCHAR use_protection;
};

struct gf_protect {
	UCHAR nongf_exist;
};

struct ht_protect {
	UCHAR op_mode;
};

enum ht_protection {
	NON_PROTECT=0,
	NONMEMBER_PROTECT=1,
	BW20_PROTECT=2,
	NONHT_MM_PROTECT=3
};

struct protection_cfg {
	struct erp_protect erp;
	struct gf_protect gf;
	struct ht_protect ht;
};

enum peer_state {
	PEER_JOIN = 0,
	PEER_LEAVE
};

UINT16 nonerp_sta_num(struct _MAC_TABLE_ENTRY *peer, UCHAR peer_state);
UCHAR nonerp_protection(struct wifi_dev *wdev);
VOID wdev_protect_init(struct wifi_dev *wdev);
VOID protect_update(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *peer, UCHAR peer_state);
VOID protect_perodic_detect(struct _RTMP_ADAPTER *pAd);

#endif /* __PROTECTION_H__ */
