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
	multi_hif.h
*/

#ifndef __MULTI_HIF_H__
#define __MULTI_HIF_H__

struct multi_hif_entry {
	struct _DL_LIST list;
	struct _DL_LIST glist;
	UINT32 id;
	UINT32 gid;
	UINT32 rid;
	UINT8 hif_ctrl[0] ____cacheline_aligned;
};

VOID multi_hif_init(VOID);
VOID multi_hif_exit(VOID);
VOID multi_hif_entry_free(VOID *hif_ctrl);
VOID multi_hif_entry_gid_set(VOID *hif_ctrl, UINT32 gid);
UINT32 multi_hif_entry_gid_get(VOID *hif_ctrl);
VOID *multi_hif_entry_get_by_id(UINT32 id);
UINT32 multi_hif_entry_id_get(VOID *hif_ctrl);
NDIS_STATUS multi_hif_entry_alloc(VOID **hif_ctrl, UINT32 size);
VOID multi_hif_entry_get_by_gid(UINT32 gid, struct _DL_LIST *head);
VOID multi_hif_entry_rid_set(VOID *hif_ctrl, UINT32 rid);
UINT32 multi_hif_entry_rid_get(VOID *hif_ctrl);

#endif /*__MULTI_HIF_H__*/
