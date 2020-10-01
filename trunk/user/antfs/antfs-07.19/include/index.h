/*
 * index.h - Defines for NTFS index handling.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2004 Anton Altaparmakov
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2005 Yura Pakhuchiy
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

#ifndef _NTFS_INDEX_H
#define _NTFS_INDEX_H

/* Convenience macros to test the versions of gcc.
 *  Use them like this:
 *  #if __GNUC_PREREQ (2,8)
 *  ... code requiring gcc 2.8 or later ...
 *  #endif
 *  Note - they won't work for gcc1 or glibc1, since the _MINOR macros
 *  were not defined then.
 */

#ifndef __GNUC_PREREQ
#if defined __GNUC__ && defined __GNUC_MINOR__
#define __GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ(maj, min) 0
#endif
#endif

/* allows us to warn about unused results of certain function calls */
#ifndef __attribute_warn_unused_result__
#if __GNUC_PREREQ(3, 4)
#define __attribute_warn_unused_result__ \
	__attribute__ ((__warn_unused_result__))
#else
#define __attribute_warn_unused_result__	/* empty */
#endif
#endif

#include "attrib.h"
#include "types.h"
#include "layout.h"
#include "inode.h"
#include "mft.h"

#define  VCN_INDEX_ROOT_PARENT  ((VCN)-2)

#define  MAX_PARENT_VCN		32

typedef int (*COLLATE) (struct ntfs_volume *vol, const void *data1, int len1,
			const void *data2, int len2);

/**
 * struct ntfs_index_context -
 * @ni:			inode containing the @entry described by this context
 * @name:		name of the index described by this context
 * @name_len:		length of the index name
 * @entry:		index entry (points into @ir or @ia)
 * @data:		index entry data (points into @entry)
 * @data_len:		length in bytes of @data
 * @is_in_root:		TRUE if @entry is in @ir or FALSE if it is in @ia
 * @ir:			index root if @is_in_root or NULL otherwise
 * @actx:		attribute search context if in root or NULL otherwise
 * @ia:			index block if @is_in_root is FALSE or NULL otherwise
 * @ia_na:		opened INDEX_ALLOCATION attribute
 * @parent_pos:		parent entries' positions in the index block
 * @parent_vcn:		entry's parent node or VCN_INDEX_ROOT_PARENT
 * @new_vcn:            new VCN if we need to create a new index block
 * @median:		move to the parent if splitting index blocks
 * @ib_dirty:		TRUE if index block was changed
 * @block_size:		index block size
 * @vcn_size_bits:	VCN size bits for this index block
 *
 * @ni is the inode this context belongs to.
 *
 * @entry is the index entry described by this context.  @data and @data_len
 * are the index entry data and its length in bytes, respectively.  @data
 * simply points into @entry.  This is probably what the user is interested in.
 *
 * If @is_in_root is TRUE, @entry is in the index root attribute @ir described
 * by the attribute search context @actx and inode @ni.  @ia and
 * @ib_dirty are undefined in this case.
 *
 * If @is_in_root is FALSE, @entry is in the index allocation attribute and @ia
 * point to the index allocation block and VCN where it's placed,
 * respectively. @ir and @actx are NULL in this case. @ia_na is opened
 * INDEX_ALLOCATION attribute. @ib_dirty is TRUE if index block was changed and
 * FALSE otherwise.
 *
 * To obtain a context call ntfs_index_ctx_get().
 *
 * When finished with the @entry and its @data, call ntfs_index_ctx_put() to
 * free the context and other associated resources.
 *
 * If the index entry was modified, call ntfs_index_entry_mark_dirty() before
 * the call to ntfs_index_ctx_put() to ensure that the changes are written
 * to disk.
 */
struct ntfs_index_context {
	struct ntfs_inode *ni;
	ntfschar *name;
	u32 name_len;
	struct INDEX_ENTRY *entry;
	void *data;
	u16 data_len;
	COLLATE collate;
	bool is_in_root;
	struct INDEX_ROOT *ir;
	struct ntfs_attr_search_ctx *actx;
	struct INDEX_BLOCK *ib;
	struct ntfs_attr *ia_na;
	int parent_pos[MAX_PARENT_VCN];	/* parent entries' positions */
	VCN parent_vcn[MAX_PARENT_VCN];	/* entry's parent nodes */
	int pindex;	/* maximum it's the number of the parent nodes  */
	bool ib_dirty;
	bool bad_index;
	u8 block_size_bits;
	u8 vcn_size_bits;
};

struct ntfs_index_context *ntfs_index_ctx_get(struct ntfs_inode *ni,
					      ntfschar *name, u32 name_len);
void ntfs_index_ctx_put(struct ntfs_index_context *ictx);
void ntfs_index_ctx_reinit(struct ntfs_index_context *ictx);

int ntfs_index_lookup(const void *key, const int key_len,
	    struct ntfs_index_context *ictx) __attribute_warn_unused_result__;

struct INDEX_ENTRY *ntfs_index_next(struct INDEX_ENTRY *ie,
				    struct ntfs_index_context *ictx);

int ntfs_index_add_filename(struct ntfs_inode *ni, struct FILE_NAME_ATTR *fn,
				   MFT_REF mref);
int ntfs_index_remove(struct ntfs_inode *dir_ni, struct ntfs_inode *ni,
			     const void *key, const int keylen);

struct INDEX_ROOT *ntfs_index_root_get(struct ntfs_inode *ni,
				struct ATTR_RECORD *attr);

VCN ntfs_ie_get_vcn(struct INDEX_ENTRY *ie);

void ntfs_index_entry_mark_dirty(struct ntfs_index_context *ictx);

char *ntfs_ie_filename_get(struct INDEX_ENTRY *ie);
void ntfs_ie_filename_dump(struct INDEX_ENTRY *ie);
void ntfs_ih_filename_dump(struct INDEX_HEADER *ih);

/* the following was added by JPA for use in security.c */
int ntfs_ie_add(struct ntfs_index_context *icx, struct INDEX_ENTRY *ie);
int ntfs_index_rm(struct ntfs_index_context *icx);

#endif /* _NTFS_INDEX_H */
