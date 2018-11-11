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
    protection.c

    Abstract:
    For general 802.11 Legacy/HT/nonHT protection mechanism

    Who             When            What
    --------------  ----------      ----------------------------------------------
    Hugo            2016-0505       created
*/

#include "rt_config.h"

UINT16 nonerp_sta_num(struct _MAC_TABLE_ENTRY *peer, UCHAR peer_state)
{
	if ((peer->MaxHTPhyMode.field.MODE == MODE_CCK) && (peer->Sst == SST_ASSOC)) {
		if ((peer_state == PEER_JOIN) && (!peer->nonerp_sta)) {
			peer->wdev->conn_sta.nonerp_sta_cnt++;
			peer->nonerp_sta = 1;
		}
		if ((peer_state == PEER_LEAVE) && (peer->nonerp_sta)) {
			peer->wdev->conn_sta.nonerp_sta_cnt--;
			peer->nonerp_sta = 0;
		}
	}

	return peer->wdev->conn_sta.nonerp_sta_cnt;
}

UCHAR nonerp_protection(struct wifi_dev *wdev)
{
	UCHAR use_prot = 0;

	if (wdev && (wdev->conn_sta.nonerp_sta_cnt > 0)) {
		use_prot = 1;
	}

	return use_prot;
}

VOID wdev_protect_init(struct wifi_dev *wdev)
{
	if (wdev) {
		os_zero_mem(&wdev->prot_cfg, sizeof(wdev->prot_cfg));
	}
}
