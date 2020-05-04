/*
 * lcnalloc.h - Exports for cluster (de)allocation.
 * Originated from the Linux-NTFS
 *		project.
 *
 * Copyright (c) 2002 Anton Altaparmakov
 * Copyright (c) 2004 Yura Pakhuchiy
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _NTFS_LCNALLOC_H
#define _NTFS_LCNALLOC_H

#include "types.h"
#include "runlist.h"
#include "volume.h"

/**
 * enum NTFS_CLUSTER_ALLOCATION_ZONES -
 */
enum NTFS_CLUSTER_ALLOCATION_ZONES {
	FIRST_ZONE = 0,		/* For sanity checking. */
	MFT_ZONE = 0,		/* Allocate from $MFT zone. */
	DATA_ZONE = 1,		/* Allocate from $DATA zone. */
	LAST_ZONE = 1,		/* For sanity checking. */
};

extern void update_full_status(struct ntfs_volume *vol, LCN lcn);
extern struct runlist_element *ntfs_cluster_alloc(struct ntfs_volume *vol,
						  VCN start_vcn, s64 count,
						  LCN start_lcn,
						  const enum
						  NTFS_CLUSTER_ALLOCATION_ZONES
						  zone,
						  s64 file_size);

extern int ntfs_cluster_free_from_rl(struct ntfs_volume *vol,
				     struct runlist_element *rl);
extern int ntfs_cluster_free_basic(struct ntfs_volume *vol, s64 lcn, s64 count);

extern int ntfs_cluster_free(struct ntfs_volume *vol, struct ntfs_attr *na,
			     VCN start_vcn, s64 count);

extern struct buffer_head *ntfs_load_bitmap_attr(struct ntfs_volume *vol,
						 struct ntfs_attr *na, u64 bit);
int ntfs_lcn_bitmap_clear_run(struct ntfs_volume *vol, LCN lcn, int count);
#endif /* defined _NTFS_LCNALLOC_H */
