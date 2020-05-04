/*
 * inode.h - Defines for NTFS inode handling.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2001-2004 Anton Altaparmakov
 * Copyright (c) 2004-2007 Yura Pakhuchiy
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2006-2008 Szabolcs Szakacsits
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

#ifndef _NTFS_INODE_H
#define _NTFS_INODE_H

#include "types.h"
#include "layout.h"
#include "support.h"
#include "volume.h"
#include "ntfstime.h"

/**
 * enum ntfs_inode_state_bits -
 *
 * Defined bits for the state field in the ntfs_inode structure.
 * (f) = files only, (d) = directories only
 */
enum ntfs_inode_state_bits {
	NI_Dirty,		/* 1: Mft record needs to be written to disk. */

	/* The NI_AttrList* tests only make sense for base inodes. */
	NI_AttrList,		/* 1: Mft record contains an attribute list. */
	NI_AttrListDirty,	/* 1: Attribute list needs to be written to the
				   mft record and then to disk. */
	NI_FileNameDirty,	/* 1: FILE_NAME attributes need to be updated
				   in the index. */
	NI_v3_Extensions,	/* 1: JPA v3.x extensions present. */
	NI_TimesSet,		/* 1: Use times which were set */
	NI_index_modified,	/* 1: Set if index was modified by readdir */
	NI_Collided,		/* 1: We collided with another ino. Use extra
				 *    care with cleanup.
				 */
	NI_WritePending,        /* 1: We are currently writing to the inode
				 * set in antfs_write_begin
				 * unset in antfs_write_end
				 * antfs_write_inode will look at this flag
				 * to see if it can write back the inode
				 */
};

#define  test_nino_flag(ni, flag)	   test_bit(NI_##flag, (&(ni)->state))
#define   set_nino_flag(ni, flag)	    set_bit(NI_##flag, (&(ni)->state))
#define clear_nino_flag(ni, flag)	  clear_bit(NI_##flag, (&(ni)->state))

#define test_and_set_nino_flag(ni, flag)	\
				   test_and_set_bit(NI_##flag, (&(ni)->state))
#define test_and_clear_nino_flag(ni, flag)	\
				 test_and_clear_bit(NI_##flag, (&(ni)->state))

#define NInoDirty(ni)				  test_nino_flag(ni, Dirty)
#define NInoSetDirty(ni)			   set_nino_flag(ni, Dirty)
#define NInoClearDirty(ni)			 clear_nino_flag(ni, Dirty)
#define NInoTestAndSetDirty(ni)		  test_and_set_nino_flag(ni, Dirty)
#define NInoTestAndClearDirty(ni)	test_and_clear_nino_flag(ni, Dirty)

#define NInoAttrList(ni)			  test_nino_flag(ni, AttrList)
#define NInoSetAttrList(ni)			   set_nino_flag(ni, AttrList)
#define NInoClearAttrList(ni)			 clear_nino_flag(ni, AttrList)

#define  test_nino_al_flag(ni, flag)	 test_nino_flag(ni, AttrList##flag)
#define   set_nino_al_flag(ni, flag)	  set_nino_flag(ni, AttrList##flag)
#define clear_nino_al_flag(ni, flag)	clear_nino_flag(ni, AttrList##flag)

#define test_and_set_nino_al_flag(ni, flag)	\
			 test_and_set_nino_flag(ni, AttrList##flag)
#define test_and_clear_nino_al_flag(ni, flag)	\
		       test_and_clear_nino_flag(ni, AttrList##flag)

#define NInoAttrListDirty(ni)			    test_nino_al_flag(ni, Dirty)
#define NInoAttrListSetDirty(ni)		     set_nino_al_flag(ni, Dirty)
#define NInoAttrListClearDirty(ni)		   clear_nino_al_flag(ni, Dirty)
#define NInoAttrListTestAndSetDirty(ni)	    test_and_set_nino_al_flag(ni, Dirty)
#define NInoAttrListTestAndClearDirty(ni) test_and_clear_nino_al_flag(ni, Dirty)

#define NInoFileNameDirty(ni)                 test_nino_flag(ni, FileNameDirty)
#define NInoFileNameSetDirty(ni)               set_nino_flag(ni, FileNameDirty)
#define NInoFileNameClearDirty(ni)           clear_nino_flag(ni, FileNameDirty)
#define NInoFileNameTestAndSetDirty(ni)		\
			      test_and_set_nino_flag(ni, FileNameDirty)
#define NInoFileNameTestAndClearDirty(ni)	\
			    test_and_clear_nino_flag(ni, FileNameDirty)

#define NInoAttrIndexModified(ni)	test_nino_flag(ni, index_modified)
#define NInoAttrSetIndexModified(ni)	set_nino_flag(ni, index_modified)
#define NInoAttrClearIndexModified(ni)	clear_nino_flag(ni, index_modified)
#define NInoAttrTestAndSetIndexModified(ni) \
test_and_set_nino_flag(ni, index_modified)
#define NInoAttrTestAndClearIndexModified(ni) \
test_and_clear_nino_flag(ni, index_modified)

#define NInoCollided(ni)                      test_nino_flag(ni, Collided)
#define NInoSetCollided(ni)                    set_nino_flag(ni, Collided)
#define NInoClearCollided(ni)                clear_nino_flag(ni, Collided)
#define NInoTestAndClearCollided(ni) \
test_and_clear_nino_flag(ni, Collided)

#define NInoWritePending(ni)                    test_nino_flag(ni, WritePending)
#define NInoSetWritePending(ni)                  set_nino_flag(ni, WritePending)
#define NInoClearWritePending(ni)              clear_nino_flag(ni, WritePending)
#define NInoTestAndSetWritePending(ni) \
	test_and_set_nino_flag(ni, WritePending)
#define NInoTestAndClearWritePending(ni) \
	test_and_clear_nino_flag(ni, WritePending)
/**
* struct _ntfs_inode - The NTFS in-memory inode structure.
*
* It is just used as an extension to the fields already provided in the VFS
* inode.
*/
struct ntfs_inode {
u64 mft_no;		/* Inode / mft record number. */
struct MFT_RECORD *mrec;    /* The actual mft record of the inode. */
struct ntfs_volume *vol;/* Pointer to the ntfs volume of this inode. */
unsigned long state;	/* NTFS specific flags describing this inode.
			 * See ntfs_inode_state_bits above.
			 */
enum FILE_ATTR_FLAGS flags;	/* Flags describing the file.
				 * (Copy from STANDARD_INFORMATION)
				 */
/*
 * Attribute list support (for use by the attribute lookup functions).
 * Setup during ntfs_open_inode() for all inodes with attribute lists.
 * Only valid if NI_AttrList is set in state.
 */
	u32 attr_list_size;	/* Length of attribute list value in bytes. */
	u8 *attr_list;		/* Attribute list value itself. */
	/* Below fields are always valid. */
	s32 nr_extents;		/* For a base mft record, the number of
				   attached extent inodes (0 if none), for
				   extent records this is -1. */
	union {			/* This union is only used if nr_extents != 0.*/
		struct ntfs_inode **extent_nis;
		/* For nr_extents > 0, array of the
		   ntfs inodes of the extent mft
		   records belonging to this base
		   inode which have been loaded. */
		struct ntfs_inode *base_ni;
		/* For nr_extents == -1, the ntfs
		   inode of the base mft record. */
	};

	/* Below fields are valid only for base inode. */

	/*
	 * These four fields are copy of relevant fields from
	 * STANDARD_INFORMATION attribute and used to sync it and FILE_NAME
	 * attribute in the index.
	 */
	sle64 creation_time;
	sle64 last_data_change_time;
	sle64 last_mft_change_time;
	sle64 last_access_time;
	/* NTFS 3.x extensions added by JPA */
	/* only if NI_v3_Extensions is set in state */
	le32 owner_id;
	le32 security_id;
	le64 quota_charged;
	le64 usn;

	/* ---- */
	struct mutex ni_lock;
};

enum ntfs_time_update_flags {
	NTFS_UPDATE_ATIME = 1 << 0,
	NTFS_UPDATE_MTIME = 1 << 1,
	NTFS_UPDATE_CTIME = 1 << 2,
};

#define NTFS_UPDATE_MCTIME  (NTFS_UPDATE_MTIME | NTFS_UPDATE_CTIME)
#define NTFS_UPDATE_AMCTIME (NTFS_UPDATE_ATIME | NTFS_UPDATE_MCTIME)

enum antfs_inode_init_mode {
	ANTFS_INODE_INIT_DELETE = -1,
	ANTFS_INODE_INIT_REPLACE = 0,
	ANTFS_INODE_INIT_DISCARD = 1,
};

struct ntfs_inode *ntfs_inode_base(struct ntfs_inode *ni);

struct ntfs_inode *ntfs_inode_allocate(struct ntfs_volume *vol);

struct ntfs_inode *ntfs_inode_open(struct ntfs_volume *vol,
					  const MFT_REF mref,
					  const struct FILE_NAME_ATTR *fn);

void ntfs_inode_close(struct ntfs_inode *ni);
int ntfs_inode_close_in_dir(struct ntfs_inode *ni,
				   struct ntfs_inode *dir_ni);
int ntfs_inode_real_close(struct ntfs_inode *ni);
struct ntfs_inode *ntfs_extent_inode_open(struct ntfs_inode *base_ni,
						 const leMFT_REF mref);

int ntfs_inode_attach_all_extents(struct ntfs_inode *ni);

void ntfs_inode_mark_dirty(struct ntfs_inode *ni);

int ntfs_inode_sync(struct ntfs_inode *ni);

int ntfs_inode_sync_in_dir_open(struct ntfs_inode *ni,
				       struct ntfs_inode *dir_ni);

int ntfs_inode_add_attrlist(struct ntfs_inode *ni);

int ntfs_inode_free_space(struct ntfs_inode *ni, int size);

int ntfs_inode_badclus_bad(u64 mft_no, struct ATTR_RECORD *a);

int ntfs_inode_get_times(struct ntfs_inode *ni, char *value,
				size_t size);

int ntfs_inode_set_times(struct ntfs_inode *ni, const char *value,
				size_t size, int flags);

int ntfs_inode_na_open(struct ntfs_inode *ni);

void antfs_put_super(struct super_block *sb);

/* debugging */
#define debug_double_inode(num, type)
#define debug_cached_inode(ni)

#endif /* defined _NTFS_INODE_H */
