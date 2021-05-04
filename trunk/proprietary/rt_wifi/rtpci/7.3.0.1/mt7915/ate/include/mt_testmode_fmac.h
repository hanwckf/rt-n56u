/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_testmode_fmac.h
*/

#ifndef __MT_TESTMODE_FMAC_H__
#define __MT_TESTMODE_FMAC_H__


INT32 mtf_ate_ipg_cr_restore(RTMP_ADAPTER *ad, UCHAR band_idx);
INT32 mtf_ate_mac_cr_restore(struct _RTMP_ADAPTER *ad);
INT32 mtf_ate_mac_cr_backup_and_set(struct _RTMP_ADAPTER *ad);
INT32 mtf_ate_ampdu_ba_limit(struct _RTMP_ADAPTER *ad, UINT8 wmm_idx, UINT8 agg_limit);
INT32 mtf_ate_set_sta_pause_cr(struct _RTMP_ADAPTER *ad, UINT8 ac_idx);
INT32 mtf_ate_set_ifs_cr(struct _RTMP_ADAPTER *pAd, UINT8 band_idx);


#endif /*  __MT_TESTMODE_FMAC_H__ */
