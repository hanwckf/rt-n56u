/**
 * inode.c - Inode handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2002-2005 Anton Altaparmakov
 * Copyright (c) 2002-2008 Szabolcs Szakacsits
 * Copyright (c) 2004-2007 Yura Pakhuchiy
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2009-2010 Jean-Pierre Andre
 * Copyright (c) 2016      Martin Pommerenke, Jens Krieg, Arwed Meyer,
 *		           Christian René Sechting
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
 */

#include <linux/bitops.h>

#include "antfs.h"
#include "param.h"
#include "types.h"
#include "volume.h"
#include "inode.h"
#include "attrib.h"
#include "debug.h"
#include "mft.h"
#include "attrlist.h"
#include "runlist.h"
#include "lcnalloc.h"
#include "index.h"
#include "dir.h"
#include "ntfstime.h"
#include "misc.h"

struct ntfs_inode *ntfs_inode_base(struct ntfs_inode *ni)
{
	if (ni->nr_extents == -1)
		return ni->base_ni;
	return ni;
}

/**
 * ntfs_inode_mark_dirty - set the inode (and its base inode if it exists) dirty
 * @ni:		ntfs inode to set dirty
 *
 * Set the inode @ni dirty so it is written out later (at the latest at
 * ntfs_inode_close() time). If @ni is an extent inode, set the base inode
 * dirty, too.
 *
 * This function cannot fail.
 */
void ntfs_inode_mark_dirty(struct ntfs_inode *ni)
{
	NInoSetDirty(ni);
	if (ni->nr_extents == -1)
		NInoSetDirty(ni->base_ni);
}

/**
 * __ntfs_inode_allocate - Create and initialise an NTFS inode object
 * @vol:
 *
 * Description...
 *
 * Returns:
 */
static struct ntfs_inode *__ntfs_inode_allocate(struct ntfs_volume *vol)
{
	struct inode *inode = new_inode(vol->dev->d_sb);
	struct ntfs_inode *ni;

	if (!inode) {
		antfs_log_error("failed to allocate memory for inode");
		ni = ERR_PTR(-ENOMEM);
		goto out;
	}

	ni = ANTFS_NI(inode);
	memset(ni, 0, ALIGN(sizeof(struct ntfs_inode), sizeof(void *)) +
	       ALIGN(sizeof(struct ntfs_attr), sizeof(void *)));
	ni->vol = vol;
	mutex_init(&ni->ni_lock);
	inode->i_private = ni;
	/* Make sure nlink is 1 at this early stage so we don't delete mrecs
	 * when inode open fails.
	 */
	set_nlink(inode, 1);
out:
	return ni;
}

/**
 * ntfs_inode_allocate - Create an NTFS inode object
 * @vol:
 *
 * Description...
 *
 * Returns:
 */
struct ntfs_inode *ntfs_inode_allocate(struct ntfs_volume *vol)
{
	return __ntfs_inode_allocate(vol);
}

/**
 * __ntfs_inode_release - Destroy an NTFS inode object
 * @ni:
 *
 * Description...
 *
 * Returns:
 */
static void __ntfs_inode_release(struct ntfs_inode *ni)
{
	if (!ni) {
		antfs_log_error("Tried to release an inode without ni");
		return;
	}
	if (NInoDirty(ni))
		antfs_log_error_ext("Releasing dirty inode %lld!",
				(long long)ni->mft_no);
	/* FIXME: setting pointers null that are free'd, since we experienced
	 * double free's in the past. This might help finding bugs. We remove
	 * these If we won't receive more NULL-Pointer deref's in the future */
	if (NAttrNonResident(ANTFS_NA(ni)) && ANTFS_NA(ni)->rl)
		ntfs_free(ANTFS_NA(ni)->rl);
	if (NInoAttrList(ni) && ni->attr_list) {
		ntfs_free(ni->attr_list);
		ni->attr_list = NULL;
	}
	if (ni->mrec) {
		ntfs_free(ni->mrec);
		ni->mrec = NULL;
	}
	return;
}

int ntfs_inode_na_open(struct ntfs_inode *ni)
{
	int err = 0;

	/* open index root for directories data for other files */
	if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
		err = ntfs_attr_sah_open(ni, AT_INDEX_ROOT,
				NTFS_INDEX_I30, 4);
	else
		err = ntfs_attr_sah_open(ni, AT_DATA, AT_UNNAMED, 0);

	return err;
}

/**
 * @brief Open an inode ready for access
 *
 * @param[in] vol    volume to get the inode from
 * @param[in] mref   inode number / mft record number to open
 * @param[in] fn     Search for this FILE_NAME_ATTR in the inode's attributes
 *
 *
 * Allocate an ntfs_inode structure and initialize it for the given inode
 * specified by @p mref. @p mref specifies the inode number / mft record to
 * read, including the sequence number, which can be 0 if no sequence number
 * checking is to be performed.
 *
 * Then, allocate a buffer for the mft record, read the mft record from the
 * volume @p vol, and attach it to the ntfs_inode structure (->mrec). The
 * mft record is mst deprotected and sanity checked for validity and we abort
 * if deprotection or checks fail.
 *
 * Finally, search for an attribute list attribute in the mft record and if one
 * is found, load the attribute list attribute value and attach it to the
 * ntfs_inode structure (->attr_list). Also set the NI_AttrList bit to indicate
 * this.
 *
 * If a FILE_NAME_ATTR is supplied in @p fn all FILE_NAME_ATTRs are compared in
 * the mft record are compared to to the FILE_NAME_ATTR in @p fn. The parent
 * mft number must be equal and names must match. The code for that is the same
 * as in @ref ntfs_unlink.
 *
 * @return Pointer to the ntfs_inode structure on success or error code on
 * error.
 */
static struct ntfs_inode *ntfs_inode_real_open(struct ntfs_volume *vol,
					       const MFT_REF mref,
					       const struct FILE_NAME_ATTR *fn)
{
	s64 l;
	struct ntfs_inode *ni = NULL;
	struct ntfs_attr_search_ctx *ctx;
	struct STANDARD_INFORMATION *std_info;
	le32 lthle;
	int err = 0;

	antfs_log_enter("Entering for inode %lld", (long long)MREF(mref));
	if (!vol) {
		err = -EINVAL;
		goto out;
	}
	ni = __ntfs_inode_allocate(vol);
	if (IS_ERR(ni)) {
		err = PTR_ERR(ni);
		goto out;
	}
	err = ntfs_file_record_read(vol, mref, &ni->mrec, NULL);
	if (err)
		goto err_out;
	if (!(ni->mrec->flags & MFT_RECORD_IN_USE)) {
		err = -ENOENT;
		goto err_out;
	}
	ni->mft_no = MREF(mref);
	ANTFS_I(ni)->i_ino = (long) MREF(mref);
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto err_out;
	}
	/* Receive some basic information about inode. */
	err = ntfs_attr_lookup(AT_STANDARD_INFORMATION, AT_UNNAMED,
			       0, CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (err) {
		if (!ni->mrec->base_mft_record)
			antfs_log_error("No STANDARD_INFORMATION in base record"
					" %lld", (long long)MREF(mref));
		goto put_err_out;
	}
	std_info = (struct STANDARD_INFORMATION *) ((u8 *) ctx->attr +
					     le16_to_cpu(ctx->attr->
							 value_offset));
	ni->flags = std_info->file_attributes;
	ni->creation_time = std_info->creation_time;
	ni->last_data_change_time = std_info->last_data_change_time;
	ni->last_mft_change_time = std_info->last_mft_change_time;
	ni->last_access_time = std_info->last_access_time;
	/* JPA insert v3 extensions if present */
	/* length may be seen as 72 (v1.x) or 96 (v3.x) */
	lthle = ctx->attr->length;
	if (le32_to_cpu(lthle) > sizeof(struct STANDARD_INFORMATION)) {
		set_nino_flag(ni, v3_Extensions);
		ni->owner_id = std_info->owner_id;
		ni->security_id = std_info->security_id;
		ni->quota_charged = std_info->quota_charged;
		ni->usn = std_info->usn;
	} else {
		clear_nino_flag(ni, v3_Extensions);
		ni->owner_id = const_cpu_to_le32(0);
		ni->security_id = const_cpu_to_le32(0);
	}
	/* Set attribute list information. */
	err = ntfs_attr_lookup(AT_ATTRIBUTE_LIST, AT_UNNAMED, 0,
			       CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (err) {
		if (err != -ENOENT)
			goto put_err_out;
		/* Attribute list attribute does not present. */
		err = 0;
		goto get_file_name;
	}
	NInoSetAttrList(ni);
	l = ntfs_get_attribute_value_length(ctx->attr);
	if (l < 0) {
		err = (int)l;
		goto put_err_out;
	}
	if (l > 0x40000) {
		err = -EIO;
		antfs_log_error("Too large attrlist attribute (%lld), inode "
				"%lld", (long long)l, (long long)MREF(mref));
		goto put_err_out;
	}
	ni->attr_list_size = l;
	ni->attr_list = ntfs_malloc(ni->attr_list_size);
	if (!ni->attr_list) {
		err = -ENOMEM;
		goto put_err_out;
	}
	l = ntfs_get_attribute_value(vol, ctx->attr, ni->attr_list);
	if (l < 0) {
		err = (int)l;
		goto put_err_out;
	}
	if (l != ni->attr_list_size) {
		err = -EIO;
		antfs_log_error("Unexpected attrlist size (%lld <> %u), inode "
				"%lld", (long long)l, ni->attr_list_size,
				(long long)MREF(mref));
		goto put_err_out;
	}

get_file_name:
	if (fn) {
		/* TODO: This code is from ntfs_unlink. Make this a new
		 * function? */
		bool looking_for_dos_name = FALSE;
		bool looking_for_win32_name = FALSE;
		bool case_sensitive_match = TRUE;

		/* If we got a FILE_NAME, compare file name and parent
		 * inode number. */
search:
		do {
			struct FILE_NAME_ATTR *fn_tmp;
			enum IGNORE_CASE_BOOL case_sensitive = IGNORE_CASE;

			err = ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0,
					CASE_SENSITIVE, 0, NULL, 0, ctx);
			if (err)
				break;

			fn_tmp = (struct FILE_NAME_ATTR *) ((u8 *) ctx->attr +
					le16_to_cpu(ctx->attr->value_offset));

			if (looking_for_dos_name) {
				if (fn_tmp->file_name_type == FILE_NAME_DOS)
					break;
				else
					continue;
			}
			if (looking_for_win32_name) {
				if (fn_tmp->file_name_type == FILE_NAME_WIN32)
					break;
				else
					continue;
			}

			/* Ignore hard links from other directories */
			if (fn->parent_directory != fn_tmp->parent_directory) {
				antfs_log_debug("MFT record numbers don't "
						"match (%lld != %lld)",
						MREF_LE(fn_tmp->
							parent_directory),
						MREF_LE(fn->parent_directory));
				continue;
			}

			if (case_sensitive_match
					|| ((fn_tmp->file_name_type ==
							FILE_NAME_POSIX)
						&& NVolCaseSensitive(vol)))
				case_sensitive = CASE_SENSITIVE;

			if (ntfs_names_are_equal(fn->file_name,
						fn->file_name_length,
						fn_tmp->file_name,
						fn_tmp->file_name_length,
						case_sensitive,
						vol->upcase, vol->upcase_len)) {

				if (fn_tmp->file_name_type == FILE_NAME_WIN32) {
					looking_for_dos_name = TRUE;
					ntfs_attr_reinit_search_ctx(ctx);
					continue;
				}
				if (fn_tmp->file_name_type == FILE_NAME_DOS)
					looking_for_dos_name = TRUE;
				break;
			}
		} while (1);

		if (err) {
			if (err == -ENOENT && case_sensitive_match) {
				case_sensitive_match = FALSE;
				ntfs_attr_reinit_search_ctx(ctx);
				goto search;
			}
			/* Well, no. Don't even try any further. */
			antfs_log_error("Inode without matching AT_FILE_NAME: "
					"%d", err);
			goto put_err_out;
		}
		ntfs_attr_reinit_search_ctx(ctx);
	}

	err = ntfs_inode_na_open(ni);
	if (err)
		goto put_err_out;
	ntfs_attr_put_search_ctx(ctx);
out:
	if (err)
		ni = ERR_PTR(err);

	return ni;

put_err_out:
	ntfs_attr_put_search_ctx(ctx);
err_out:
	iput(ANTFS_I(ni));
	ni = NULL;
	antfs_log_error("err=%d", err);
	goto out;
}

/**
 * ntfs_inode_close - close an ntfs inode and free all associated memory
 * @ni:		ntfs inode to close
 *
 * Make sure the ntfs inode @ni is clean.
 *
 * If the ntfs inode @ni is a base inode, close all associated extent inodes,
 * then deallocate all memory attached to it, and finally free the ntfs inode
 * structure itself.
 *
 * If it is an extent inode, we disconnect it from its base inode before we
 * destroy it.
 *
 * It is OK to pass NULL to this function, it is just noop in this case.
 *
 * Return 0 on success or on error the error code. On
 * error, @ni has not been freed. The user should attempt to handle the error
 * and call ntfs_inode_close() again. The following error codes are defined:
 *
 *	EBUSY	@ni and/or its attribute list runlist is/are dirty and the
 *		attempt to write it/them to disk failed.
 *	EINVAL	@ni is invalid (probably it is an extent inode).
 *	EIO	I/O error while trying to write inode to disk.
 */
int ntfs_inode_real_close(struct ntfs_inode *ni)
{
	int ret;

	if (unlikely(IS_ERR_OR_NULL(ni)))
		return 0;

	antfs_log_enter("Entering for inode %lld", (long long)ni->mft_no);

	__ntfs_inode_release(ni);
	ret = 0;
	antfs_log_leave("err: %d", ret);
	return ret;
}

/*
 *		Open an inode
 *
 *	When possible, an entry recorded in the cache is reused
 *
 *	**NEVER REOPEN** an inode, this can lead to a duplicated
 *	cache entry (hard to detect), and to an obsolete one being
 *	reused. System files are however protected from being cached.
 */
struct ntfs_inode *ntfs_inode_open(struct ntfs_volume *vol, const MFT_REF mref,
		const struct FILE_NAME_ATTR *fn)
{
	struct super_block *sb = vol->dev->d_sb;
	struct ntfs_inode *ni = NULL;
	struct inode *inode;
	int err;

	antfs_log_enter("mref: %lld", (long long)MREF(mref));

	/* check if we already opened an instance of this mref */
	inode = ilookup(sb, (unsigned long)MREF(mref));
	if (inode) {
		/* We found a match! Just return the ntfs inode belonging to the
		 * vfs inode we just found. Never reopen a ntfs inode!
		 */
		ni = ANTFS_NI(inode);
		goto out;
	}

	/* we never opened an inode with that mref before, do that now */
	ni = ntfs_inode_real_open(vol, mref, fn);
	/* must check here if ni is error code or correct */
	if (IS_ERR(ni))
		goto out;

	/* initialize regular inodes and the root directory and insert
	 * them into the hashlist (our cache for inodes)
	 * Even the system inodes have to put into the hash list. This
	 * acts as a cache for files like 'FILE_LOGFILE' etc.
	 */
	inode = ANTFS_I(ni);
	err = antfs_inode_init(&inode, ANTFS_INODE_INIT_REPLACE);
	/* Inode may have changed here. */
	if (err)
		ni = ERR_PTR(err);
	else
		ni = ANTFS_NI(inode);
out:
	antfs_log_leave("ret: %d", IS_ERR(ni) ? (int)PTR_ERR(ni) : 0);
	return ni;
}

/*
 *		Close an inode entry
 *
 *	If cacheing is in use, the entry is synced and kept available
 *	in cache for further use.
 *
 *	System files (inode < 16 or having the IS_4 flag) are protected
 *	against being cached.
 */
void ntfs_inode_close(struct ntfs_inode *ni)
{
	if (!IS_ERR_OR_NULL(ni))
		iput(ANTFS_I(ni));
}

/**
 * ntfs_extent_inode_open - load an extent inode and attach it to its base
 * @base_ni:	base ntfs inode
 * @mref:	mft reference of the extent inode to load (in little endian)
 *
 * First check if the extent inode @mref is already attached to the base ntfs
 * inode @base_ni, and if so, return a pointer to the attached extent inode.
 *
 * If the extent inode is not already attached to the base inode, allocate an
 * ntfs_inode structure and initialize it for the given inode @mref. @mref
 * specifies the inode number / mft record to read, including the sequence
 * number, which can be 0 if no sequence number checking is to be performed.
 *
 * Then, allocate a buffer for the mft record, read the mft record from the
 * volume @base_ni->vol, and attach it to the ntfs_inode structure (->mrec).
 * The mft record is mst deprotected and sanity checked for validity and we
 * abort if deprotection or checks fail.
 *
 * Finally attach the ntfs inode to its base inode @base_ni and return a
 * pointer to the ntfs_inode structure on success or the error code on error.
 *
 * Note, extent inodes are never closed directly. They are automatically
 * disposed off by the closing of the base inode.
 */
struct ntfs_inode *ntfs_extent_inode_open(struct ntfs_inode *base_ni,
					  const leMFT_REF mref)
{
	u64 mft_no = MREF_LE(mref);
	VCN extent_vcn;
	struct runlist_element *rl;
	struct ntfs_volume *vol;
	struct ntfs_inode *ni = NULL;
	struct inode *inode;
	struct ntfs_inode **extent_nis;
	int i, err = 0;

	if (IS_ERR_OR_NULL(base_ni)) {
		err = -EINVAL;
		antfs_log_error("EINVAL");
		goto out;
	}

	antfs_log_enter("Opening extent inode %lld (base mft record %lld).",
			(unsigned long long)mft_no,
			(unsigned long long)base_ni->mft_no);

	vol = base_ni->vol;
	/* check if we already opened an instance of this mref */
	inode = ilookup(vol->dev->d_sb, (unsigned long)mft_no);
	if (inode) {
		/* We found a match! Just return the ntfs inode belonging to the
		 * vfs inode we just found. Never reopen a ntfs inode!
		 */
		/* Keep the icount at 1 for extent inodes */
		if (atomic_read(&inode->i_count) > 1)
			iput(inode);

		ni = ANTFS_NI(inode);
		if (!ni->base_ni)
			ni->base_ni = base_ni;
		ni = ANTFS_NI(inode);

		/* only attach an extent if it is not already attached */
		for (i = 0; i < base_ni->nr_extents; i++)
			if (base_ni->extent_nis[i] == ni)
				goto out;
		goto attach;
	}

	if (!base_ni->mft_no) {
		/*
		 * When getting extents of MFT, we must be sure
		 * they are in the MFT part which has already
		 * been mapped, otherwise we fall into an endless
		 * recursion.
		 * Situations have been met where extents locations
		 * are described in themselves.
		 * This is a severe error which chkdsk cannot fix.
		 */
		extent_vcn = mft_no << vol->mft_record_size_bits
		    >> vol->cluster_size_bits;
		rl = vol->mft_na->rl;
		if (rl) {
			while (rl->length
			       && ((rl->vcn + rl->length) <= extent_vcn))
				rl++;
		}
		if (!rl || (rl->lcn < 0)) {
			antfs_log_error("MFT is corrupt, cannot read"
					" its unmapped extent record %lld",
					(long long)mft_no);
			antfs_log_error("Note : chkdsk cannot fix this,"
					" try ntfsfix");
			err = -EIO;
			ni = (struct ntfs_inode *)NULL;
			goto out;
		}
	}

	/* Is the extent inode already open and attached to the base inode? */
	if (base_ni->nr_extents > 0) {
		extent_nis = base_ni->extent_nis;
		for (i = 0; i < base_ni->nr_extents; i++) {
			u16 seq_no;

			ni = extent_nis[i];
			if (mft_no != ni->mft_no)
				continue;
			/* Verify the sequence number if given. */
			seq_no = MSEQNO_LE(mref);
			if (seq_no
			    && seq_no !=
			    le16_to_cpu(ni->mrec->sequence_number)) {
				err = -EIO;
				antfs_log_error("Found stale extent mft "
						"reference mft=%lld",
						(long long)ni->mft_no);
				goto out;
			}
			goto out;
		}
	}
	/* Wasn't there, we need to load the extent inode. */
	ni = __ntfs_inode_allocate(base_ni->vol);
	if (IS_ERR(ni)) {
		err = PTR_ERR(ni);
		goto out;
	}
	err = ntfs_file_record_read(base_ni->vol, le64_to_cpu(mref), &ni->mrec,
				  NULL);
	inode = ANTFS_I(ni);
	if (err) {
		/* Shut up unlock_new_inode in iget_failed. */
		inode->i_state |= I_NEW;
		iget_failed(inode);
		goto out;
	}

	ni->mft_no = mft_no;
	ni->nr_extents = -1;
	ni->base_ni = base_ni;

	/* We really should not get a colliding inode here. Go berserk on
	 * errors.
	 */
	err = antfs_inode_init(&inode, ANTFS_INODE_INIT_DISCARD);
	if (err)
		goto out;
	/* Inode should not have changed here. */

attach:
	/* Attach extent inode to base inode, reallocating memory if needed. */
	if (!(base_ni->nr_extents & 3)) {
		i = (base_ni->nr_extents + 4) * sizeof(struct ntfs_inode *);

		extent_nis = ntfs_malloc(i);
		if (!extent_nis) {
			/* If we cannot attach the extent to base, close the
			 * extent and error out.
			 */
			err = -ENOMEM;
			iput(inode);
			goto out;
		}
		if (base_ni->nr_extents) {
			memcpy(extent_nis, base_ni->extent_nis,
			       i - 4 * sizeof(struct ntfs_inode *));
			ntfs_free(base_ni->extent_nis);
		}
		base_ni->extent_nis = extent_nis;
	}
	base_ni->extent_nis[base_ni->nr_extents++] = ni;
out:
	if (err)
		ni = ERR_PTR(err);
	antfs_log_leave("Exit: %d", err);
	return ni;
}

/**
 * ntfs_inode_attach_all_extents - attach all extents for target inode
 * @ni:		opened ntfs inode for which perform attach
 *
 * Return 0 on success and the error code on error.
 */
int ntfs_inode_attach_all_extents(struct ntfs_inode *ni)
{
	struct ntfs_inode *dummy;
	struct ATTR_LIST_ENTRY *ale;
	u64 prev_attached = 0;
	int err = 0;

	if (IS_ERR_OR_NULL(ni)) {
		antfs_log_error("Invalid arguments.");
		err = -EINVAL;
		goto out;
	}

	if (ni->nr_extents == -1)
		ni = ni->base_ni;

	antfs_log_enter("inode 0x%llx", (long long)ni->mft_no);

	/* Inode haven't got attribute list, thus nothing to attach. */
	if (!NInoAttrList(ni))
		goto out;

	if (!ni->attr_list) {
		antfs_log_error("Corrupt in-memory struct.");
		err = -EINVAL;
		goto out;
	}

	/* Walk through attribute list and attach all extents. */
	err = 0;
	ale = (struct ATTR_LIST_ENTRY *) ni->attr_list;
	while ((u8 *) ale < ni->attr_list + ni->attr_list_size) {
		if (ni->mft_no != MREF_LE(ale->mft_reference) &&
		    prev_attached != MREF_LE(ale->mft_reference)) {
			dummy = ntfs_extent_inode_open(ni, ale->mft_reference);
			if (IS_ERR(dummy)) {
				antfs_log_debug
				    ("Couldn't attach extent inode.");
				err = PTR_ERR(dummy);
				goto out;
			}
			prev_attached = MREF_LE(ale->mft_reference);
		}
		ale = (struct ATTR_LIST_ENTRY *) ((u8 *) ale +
			le16_to_cpu(ale->length));
	}
out:
	antfs_log_leave("%d", err);
	return err;
}

/**
 * ntfs_inode_sync_standard_information - update standard information attribute
 * @ni:		ntfs inode to update standard information
 *
 * Return 0 on success or error code on error.
 */
static int ntfs_inode_sync_standard_information(struct ntfs_inode *ni)
{
	struct ntfs_attr_search_ctx *ctx;
	struct STANDARD_INFORMATION *std_info;
	u32 lth;
	le32 lthle;
	int err;

	antfs_log_debug("Entering for inode %lld", (long long)ni->mft_no);

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
	err = ntfs_attr_lookup(AT_STANDARD_INFORMATION, AT_UNNAMED,
			       0, CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Failed to sync standard info (inode %lld)",
				(long long)ni->mft_no);
		ntfs_attr_put_search_ctx(ctx);
		return err;
	}
	std_info = (struct STANDARD_INFORMATION *) ((u8 *) ctx->attr +
					     le16_to_cpu(ctx->attr->
							 value_offset));
	std_info->file_attributes = ni->flags;
	std_info->creation_time = ni->creation_time;
	std_info->last_data_change_time = ni->last_data_change_time;
	std_info->last_mft_change_time = ni->last_mft_change_time;
	std_info->last_access_time = ni->last_access_time;

	/* JPA update v3.x extensions, ensuring consistency */

	lthle = ctx->attr->length;
	lth = le32_to_cpu(lthle);
	if (test_nino_flag(ni, v3_Extensions)
	    && (lth <= sizeof(struct STANDARD_INFORMATION)))
		/* log? did we corrupt the inode now? */
		antfs_log_error("bad sync of standard information");

	if (lth > sizeof(struct STANDARD_INFORMATION)) {
		std_info->owner_id = ni->owner_id;
		std_info->security_id = ni->security_id;
		std_info->quota_charged = ni->quota_charged;
		std_info->usn = ni->usn;
	}
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	return 0;
}

/**
 * ntfs_inode_sync_file_name - update FILE_NAME attributes
 * @ni:		ntfs inode to update FILE_NAME attributes
 *
 * Update all FILE_NAME attributes for inode @ni in the index.
 *
 * Return 0 on success or error code on error.
 */
static int ntfs_inode_sync_file_name(struct ntfs_inode *ni,
				     struct ntfs_inode *dir_ni)
{
	struct ntfs_attr_search_ctx *ctx = NULL;
	struct ntfs_index_context *ictx;
	struct ntfs_inode *index_ni;
	struct FILE_NAME_ATTR *fn;
	struct FILE_NAME_ATTR *fnx;
	struct REPARSE_POINT *rpp;
	le32 reparse_tag;
	int err = 0, tmp_err;

	antfs_log_enter("inode %lld", (long long)ni->mft_no);

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto err_out;
	}
	/* Collect the reparse tag, if any */
	reparse_tag = const_cpu_to_le32(0);
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
		if (!ntfs_attr_lookup(AT_REPARSE_POINT, NULL,
				      0, CASE_SENSITIVE, 0, NULL, 0, ctx)) {
			rpp = (struct REPARSE_POINT *) ((u8 *) ctx->attr +
						 le16_to_cpu(ctx->attr->
							     value_offset));
			reparse_tag = rpp->reparse_tag;
		}
		ntfs_attr_reinit_search_ctx(ctx);
	}
	/* Walk through all FILE_NAME attributes and update them. */
	while (!
	       (tmp_err =
		ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, 0, 0, NULL, 0, ctx))) {
		fn = (struct FILE_NAME_ATTR *) ((u8 *) ctx->attr +
					 le16_to_cpu(ctx->attr->value_offset));
		if (MREF_LE(fn->parent_directory) == ni->mft_no) {
			/* AT_FILE_NAME for inode itself: */
			/*
			 * WARNING: We cheat here and obtain 2 attribute
			 * search contexts for one inode (first we obtained
			 * above, second will be obtained inside
			 * ntfs_index_lookup), it's acceptable for library,
			 * but will deadlock in the kernel.
			 *
			 * FIXME: What will deadlock and how? Did not see
			 *        anything obviously dangerous on first look.
			 *        This whole thing looks strange. Does this
			 *        ever happen?
			 */
			index_ni = ni;
		} else if (dir_ni) {
			index_ni = dir_ni;
		} else {
			index_ni = ntfs_inode_open(ni->vol,
						   le64_to_cpu
						   (fn->parent_directory),
						   NULL);
		}
		if (IS_ERR(index_ni)) {
			if (!err)
				err = PTR_ERR(index_ni);
			antfs_log_error("Failed to open inode %lld with index",
					(long long)
					MREF_LE(fn->parent_directory));
			continue;
		}
		if (ni != index_ni &&  !dir_ni) {
			if (mutex_lock_interruptible_nested(&index_ni->ni_lock,
						NI_MUTEX_PARENT)) {
				err = -ERESTARTSYS;
				goto err_out;
			}
		}
		ictx = ntfs_index_ctx_get(index_ni, NTFS_INDEX_I30, 4);
		if (IS_ERR(ictx)) {
			if (!err)
				err = -ENOMEM;
			/* Could also be EINVAL here, but whatever... */
			antfs_log_error("Failed to get index ctx, inode %lld",
					(long long)index_ni->mft_no);
			if ((ni != index_ni) && !dir_ni) {
				mutex_unlock(&index_ni->ni_lock);
				ntfs_inode_close(index_ni);
			}
			continue;
		}
		tmp_err = ntfs_index_lookup(fn, sizeof(struct FILE_NAME_ATTR),
					    ictx);
		if (tmp_err) {
			if (!err) {
				if (tmp_err == -ENOENT)
					err = -EIO;
				else
					err = tmp_err;
			}
			antfs_log_warning
			    ("Index lookup failed, inode %lld (%d)",
			     (long long)index_ni->mft_no, tmp_err);
			ntfs_index_ctx_put(ictx);
			if (ni != index_ni && !dir_ni) {
				mutex_unlock(&index_ni->ni_lock);
				ntfs_inode_close(index_ni);
			}
			continue;
		}
		/* Update flags and file size. */
		fnx = (struct FILE_NAME_ATTR *) ictx->data;
		fnx->file_attributes =
		    (fnx->file_attributes & ~FILE_ATTR_VALID_FLAGS) |
		    (ni->flags & FILE_ATTR_VALID_FLAGS);
		if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY) {
			fnx->data_size = fnx->allocated_size
			    = const_cpu_to_sle64(0);
		} else {
			/* The allocated_size field of the file_name_attr holds
			 * either the allocated_size of a file, or the
			 * compressed_size.
			 * NOTE: only nonresident files can be compressed or
			 *       sparse.
			 */
			if (NAttrNonResident(ANTFS_NA(ni)) &&
			    ((ANTFS_NA(ni)->data_flags &
				ATTR_COMPRESSION_MASK)
				|| NAttrSparse(ANTFS_NA(ni))))
				fnx->allocated_size =
				    cpu_to_sle64(ANTFS_NA(ni)->compressed_size);
			else
				fnx->allocated_size =
				    cpu_to_sle64(ANTFS_NA(ni)->allocated_size);
			fnx->data_size = cpu_to_sle64(ANTFS_NA(ni)->data_size);

			/*
			 * The file name record has also to be fixed if some
			 * attribute update implied the unnamed data to be
			 * made non-resident
			 */
			fn->data_size = fnx->data_size;
			fn->allocated_size = fnx->allocated_size;
		}
		/* update or clear the reparse tag in the index */
		fnx->reparse_point_tag = reparse_tag;
		fnx->creation_time = fn->creation_time = ni->creation_time;
		fnx->last_data_change_time = fn->last_data_change_time =
		    ni->last_data_change_time;
		fnx->last_mft_change_time = fn->last_mft_change_time =
		    ni->last_mft_change_time;
		fnx->last_access_time = fn->last_access_time =
		    ni->last_access_time;

		ntfs_index_entry_mark_dirty(ictx);
		ntfs_index_ctx_put(ictx);
		if ((ni != index_ni) && !dir_ni) {
			mutex_unlock(&index_ni->ni_lock);
			ntfs_inode_close(index_ni);
		}
	}
	/* Check for real error occurred. */
	if (tmp_err != -ENOENT) {
		err = tmp_err;
		antfs_log_error("Attribute lookup failed, inode %lld",
				(long long)ni->mft_no);
		goto err_out;
	}
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave();
	return err;
err_out:
	if (!IS_ERR_OR_NULL(ctx))
		ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("err= %d", err);
	return err;
}

/**
 * ntfs_inode_sync - write the inode (and its dirty extents) to disk
 * @ni:		ntfs inode to write
 *
 * Write the inode @ni to disk as well as its dirty extent inodes if such
 * exist and @ni is a base inode. If @ni is an extent inode, only @ni is
 * written completely disregarding its base inode and any other extent inodes.
 *
 * For a base inode with dirty extent inodes if any writes fail for whatever
 * reason, the failing inode is skipped and the sync process is continued. At
 * the end the error condition that brought about the failure is returned. Thus
 * the smallest amount of data loss possible occurs.
 *
 * Return 0 on success or error code on error.
 * The following error codes are defined:
 *	EINVAL	- Invalid arguments were passed to the function.
 *	EBUSY	- Inode and/or one of its extents is busy, try again later.
 *	EIO	- I/O error while writing the inode (or one of its extents).
 */
static int ntfs_inode_sync_in_dir(struct ntfs_inode *ni,
				  struct ntfs_inode *dir_ni)
{
	struct inode *inode = NULL;
	int ret = 0;
	int err = 0;

	antfs_log_enter("inode (%lld)", (long long)ni->mft_no);

	if (IS_ERR_OR_NULL(ni)) {
		antfs_log_error("Failed to sync NULL inode");
		ret = -EINVAL;
		goto out;
	}

	inode = ANTFS_I(ni);
	/* Sync in time from VFS here -- Should be up to date. */
	ni->last_access_time = timespec2ntfs(inode->i_atime);
	ni->last_data_change_time = timespec2ntfs(inode->i_mtime);
	ni->last_mft_change_time = timespec2ntfs(inode->i_ctime);

#if 0
	{
		struct tm atime, ctime, mtime;

		time_to_tm(inode->i_atime.tv_sec, 0, &atime);
		time_to_tm(inode->i_ctime.tv_sec, 0, &ctime);
		time_to_tm(inode->i_mtime.tv_sec, 0, &mtime);

		antfs_log_error
			("(%pS) Times on sync (yyyy-mm-dd hh:mm:ss); "
			 "inode (%lld)\n"
			 "atime: %04lu-%02u-%02u %02u:%02u:%02u\n"
			 "ctime: %04lu-%02u-%02u %02u:%02u:%02u\n"
			 "mtime: %04lu-%02u-%02u %02u:%02u:%02u",
			 __builtin_return_address(0), (long long)ni->mft_no,
			 atime.tm_year + 1900, atime.tm_mon + 1,
			 atime.tm_mday, atime.tm_hour, atime.tm_min,
			 atime.tm_sec, ctime.tm_year + 1900,
			 ctime.tm_mon + 1, ctime.tm_mday, ctime.tm_hour,
			 ctime.tm_min, ctime.tm_sec, mtime.tm_year + 1900,
			 mtime.tm_mon + 1, mtime.tm_mday, mtime.tm_hour,
			 mtime.tm_min, mtime.tm_sec);
	}
#endif

	/* Update STANDARD_INFORMATION. */
	if ((ni->mrec->flags & MFT_RECORD_IN_USE) && ni->nr_extents != -1) {
		ret = ntfs_inode_sync_standard_information(ni);
		if (ret && ret != -EIO)
			ret = -EBUSY;
	}

	/* Update FILE_NAME's in the index. */
	if ((ni->mrec->flags & MFT_RECORD_IN_USE) && ni->nr_extents != -1 &&
	    NInoFileNameTestAndClearDirty(ni)) {
		err = ntfs_inode_sync_file_name(ni, dir_ni);
		if (err) {
			if (err != -EIO)
				err = -EBUSY;
			if (!ret || err == -EIO)
				ret = err;
			/* log? did we corrupt the inode? */
			antfs_log_warning("Failed to sync FILE_NAME (ino %lld)",
					  (long long)ni->mft_no);
			NInoFileNameSetDirty(ni);
		}
	}

	/* Write out attribute list from cache to disk. */
	if ((ni->mrec->flags & MFT_RECORD_IN_USE) && ni->nr_extents != -1 &&
	    NInoAttrList(ni) && NInoAttrListTestAndClearDirty(ni)) {
		struct ntfs_attr *na;

		na = ntfs_attr_open(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
		if (IS_ERR(na)) {
			err = PTR_ERR(na);
			if (err != -EIO)
				err = -EBUSY;
			if (!ret || err == -EIO) {
				antfs_log_error("Attribute list sync failed "
						"(open, inode %lld)",
						(long long)ni->mft_no);
				ret = err;
			}
			NInoAttrListSetDirty(ni);
			goto sync_inode;
		}

		if (na->data_size == ni->attr_list_size) {
			s64 tmp_br = ntfs_attr_pwrite(na, 0, ni->attr_list_size,
						      ni->attr_list);
			if (tmp_br != ni->attr_list_size) {
				if (tmp_br >= 0)
					err = -EIO;
				else
					err = (int)tmp_br;
				if (err != -EIO)
					err = -EBUSY;
				if (!ret || err == -EIO) {
					antfs_log_error("Attribute list sync "
							"failed (write, inode %lld)",
							(long long)ni->mft_no);
					ret = err;
				}
				NInoAttrListSetDirty(ni);
			}
		} else {
			err = -EIO;
			antfs_log_error("Attribute list sync failed (bad size, "
					"inode %lld)", (long long)ni->mft_no);
			NInoAttrListSetDirty(ni);
		}
		ntfs_attr_close(na);
	}

sync_inode:
	/* Write this inode out to the $MFT (and $MFTMirr if applicable). */
	if (NInoTestAndClearDirty(ni)) {
		err = ntfs_mft_record_write(ni->vol, ni->mft_no, ni->mrec);
		if (err) {
			if (err != -EIO)
				err = -EBUSY;
			if (!ret || err == -EIO)
				ret = err;
			NInoSetDirty(ni);
			/* log? is this critical? */
			antfs_log_error("MFT record sync failed, inode %lld",
					(long long)ni->mft_no);
		}
	}

	/* If this is a base inode with extents write all dirty extents, too. */
	if (ni->nr_extents > 0) {
		s32 i;

		for (i = 0; i < ni->nr_extents; ++i) {
			struct ntfs_inode *eni;

			eni = ni->extent_nis[i];
			if (!NInoTestAndClearDirty(eni))
				continue;

			err = ntfs_mft_record_write(eni->vol, eni->mft_no,
						    eni->mrec);
			if (err) {
				if (err != -EIO) {
					err = -EBUSY;
				}
				if (!ret || err == -EIO) {
					ret = err;
				}
				NInoSetDirty(eni);
				antfs_log_error("Extent MFT record sync failed,"
						" inode %lld/%lld",
						(long long)ni->mft_no,
						(long long)eni->mft_no);
			}
		}
	}

out:
	antfs_log_leave();
	return ret;
}

int ntfs_inode_sync(struct ntfs_inode *ni)
{
	return ntfs_inode_sync_in_dir(ni, (struct ntfs_inode *)NULL);
}

/**
 * ntfs_inode_add_attrlist - add attribute list to inode and fill it
 * @ni: opened ntfs inode to which add attribute list
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 * The following error codes are defined:
 *	EINVAL	- Invalid arguments were passed to the function.
 *	EEXIST	- Attribute list already exist.
 *	EIO	- Input/Ouput error occurred.
 *	ENOMEM	- Not enough memory to perform add.
 */
int ntfs_inode_add_attrlist(struct ntfs_inode *ni)
{
	int err;
	struct ntfs_attr_search_ctx *ctx;
	u8 *al = NULL, *aln;
	int al_len = 0;
	struct ATTR_LIST_ENTRY *ale = NULL;
	struct ntfs_attr *na;

	if (IS_ERR_OR_NULL(ni)) {
		antfs_log_error("EINVAL");
		return -EINVAL;
	}

	antfs_log_enter("inode %llu", (unsigned long long)ni->mft_no);

	if (NInoAttrList(ni) || ni->nr_extents) {
		antfs_log_error("Inode already has attribute list");
		return -EEXIST;
	}

	/* Form attribute list. */
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto err_out;
	}
	/* Walk through all attributes. */
	while (!
	       (err =
		ntfs_attr_lookup(AT_UNUSED, NULL, 0, 0, 0, NULL, 0, ctx))) {

		int ale_size;

		if (ctx->attr->type == AT_ATTRIBUTE_LIST) {
			err = -EIO;
			antfs_log_error("Attribute list already present");
			goto put_err_out;
		}

		ale_size = (sizeof(struct ATTR_LIST_ENTRY) + sizeof(ntfschar) *
			    ctx->attr->name_length + 7) & ~7;
		al_len += ale_size;

		aln = ntfs_realloc(al, al_len);
		if (!aln) {
			err = -ENOMEM;
			/* log? is a failed realloc critical in this case? */
			antfs_log_error("Failed to realloc %d bytes", al_len);
			goto put_err_out;
		}
		ale = (struct ATTR_LIST_ENTRY *) (aln + ((u8 *) ale - al));
		al = aln;

		memset(ale, 0, ale_size);

		/* Add attribute to attribute list. */
		ale->type = ctx->attr->type;
		ale->length = cpu_to_le16((sizeof(struct ATTR_LIST_ENTRY) +
					   sizeof(ntfschar) *
					   ctx->attr->name_length + 7) & ~7);
		ale->name_length = ctx->attr->name_length;
		ale->name_offset = (u8 *) ale->name - (u8 *) ale;
		if (ctx->attr->non_resident)
			ale->lowest_vcn = ctx->attr->lowest_vcn;
		else
			ale->lowest_vcn = const_cpu_to_sle64(0);
		ale->mft_reference = MK_LE_MREF(ni->mft_no,
						le16_to_cpu(ni->mrec->
							    sequence_number));
		ale->instance = ctx->attr->instance;
		memcpy(ale->name, (u8 *) ctx->attr +
		       le16_to_cpu(ctx->attr->name_offset),
		       ctx->attr->name_length * sizeof(ntfschar));
		ale = (struct ATTR_LIST_ENTRY *) (al + al_len);
	}
	/* Check for real error occurred. */
	if (err != -ENOENT) {
		antfs_log_error("Attribute lookup failed, inode %lld",
				(long long)ni->mft_no);
		goto put_err_out;
	}

	/* Set in-memory attribute list. */
	ni->attr_list = al;
	ni->attr_list_size = al_len;
	NInoSetAttrList(ni);
	NInoAttrListSetDirty(ni);

	/* Free space if there is not enough it for $ATTRIBUTE_LIST. */
	if (le32_to_cpu(ni->mrec->bytes_allocated) -
	    le32_to_cpu(ni->mrec->bytes_in_use) <
	    offsetof(struct ATTR_RECORD, resident_end)) {
		err = ntfs_inode_free_space(ni,
					    offsetof(struct ATTR_RECORD,
						     resident_end));
		if (err) {
			/* Failed to free space. */
			antfs_log_error("Failed to free space for attrlist");
			goto rollback;
		}
	}

	/* Add $ATTRIBUTE_LIST to mft record. */
	err = ntfs_resident_attr_record_add(ni,
					    AT_ATTRIBUTE_LIST, NULL, 0, NULL, 0,
					    const_cpu_to_le16(0));
	if (err < 0) {
		antfs_log_error("Couldn't add $ATTRIBUTE_LIST to MFT");
		goto rollback;
	}

	/* Resize it. */
	na = ntfs_attr_open(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
	if (IS_ERR(na)) {
		err = PTR_ERR(na);
		antfs_log_error("Failed to open just added $ATTRIBUTE_LIST");
		goto remove_attrlist_record;
	}
	err = ntfs_attr_truncate(na, al_len);
	if (err) {
		antfs_log_error("Failed to resize just added $ATTRIBUTE_LIST");
		ntfs_attr_close(na);
		/* Always return error */
		if (err > 0)
			err = -ENOSPC;
		goto remove_attrlist_record;
	}

	ntfs_attr_put_search_ctx(ctx);
	ntfs_attr_close(na);
	antfs_log_leave("ok");
	return 0;

remove_attrlist_record:
	/* Prevent ntfs_attr_recorm_rm from freeing attribute list. */
	ni->attr_list = NULL;
	NInoClearAttrList(ni);
	/* Remove $ATTRIBUTE_LIST record. */
	ntfs_attr_reinit_search_ctx(ctx);
	if (!ntfs_attr_lookup(AT_ATTRIBUTE_LIST, NULL, 0,
			      CASE_SENSITIVE, 0, NULL, 0, ctx)) {
		if (ntfs_attr_record_rm(ctx)) {
			antfs_logger(ANTFS_I(ni)->i_sb->s_id,
				"Rollback failed to remove attrlist");
		}
	} else
		antfs_log_error("Rollback failed to find attrlist");
	/* Setup back in-memory runlist. */
	ni->attr_list = al;
	ni->attr_list_size = al_len;
	NInoSetAttrList(ni);
rollback:
	/*
	 * Scan attribute list for attributes that placed not in the base MFT
	 * record and move them to it.
	 */
	ntfs_attr_reinit_search_ctx(ctx);
	ale = (struct ATTR_LIST_ENTRY *) al;
	while ((u8 *) ale < al + al_len) {
		if (MREF_LE(ale->mft_reference) != ni->mft_no) {
			if (!ntfs_attr_lookup(ale->type, ale->name,
					      ale->name_length,
					      CASE_SENSITIVE,
					      sle64_to_cpu(ale->lowest_vcn),
					      NULL, 0, ctx)) {
				if (ntfs_attr_record_move_to(ctx, ni)) {
					antfs_logger(ANTFS_I(ni)->i_sb->s_id,
					"Rollback failed to move attribute");
				}
			} else
				antfs_log_error("Rollback failed to find attr");
			ntfs_attr_reinit_search_ctx(ctx);
		}
		ale = (struct ATTR_LIST_ENTRY *) ((u8 *) ale +
			le16_to_cpu(ale->length));
	}
	/* Remove in-memory attribute list. */
	ni->attr_list = NULL;
	ni->attr_list_size = 0;
	NInoClearAttrList(ni);
	NInoAttrListClearDirty(ni);
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
err_out:
	ntfs_free(al);
	return err;
}

/**
 * ntfs_inode_free_space - free space in the MFT record of an inode
 * @ni:		ntfs inode in which MFT record needs more free space
 * @size:	amount of space needed to free
 *
 * Return 0 on success or the error code on error.
 */
int ntfs_inode_free_space(struct ntfs_inode *ni, int size)
{
	struct ntfs_attr_search_ctx *ctx;
	int freed, err;

	if (IS_ERR_OR_NULL(ni) || size < 0) {
		antfs_log_error("ni=%p size=%d", ni, size);
		return -EINVAL;
	}

	antfs_log_debug("Entering for inode %lld, size %d",
			(unsigned long long)ni->mft_no, size);

	freed = (le32_to_cpu(ni->mrec->bytes_allocated) -
		 le32_to_cpu(ni->mrec->bytes_in_use));

	if (size <= freed)
		return 0;

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
	/*
	 * $STANDARD_INFORMATION and $ATTRIBUTE_LIST must stay in the base MFT
	 * record, so position search context on the first attribute after them.
	 */
	err = ntfs_attr_position(AT_FILE_NAME, ctx);
	if (err)
		goto put_err_out;

	while (1) {
		int record_size;
		/*
		 * Check whether attribute is from different MFT record. If so,
		 * find next, because we don't need such.
		 */
		while (ctx->ntfs_ino->mft_no != ni->mft_no) {
retry:			err = ntfs_attr_position(AT_UNUSED, ctx);
			if (err)
				goto put_err_out;
		}

		if (ntfs_inode_base(ctx->ntfs_ino)->mft_no == FILE_MFT &&
		    ctx->attr->type == AT_DATA)
			goto retry;

		if (ctx->attr->type == AT_INDEX_ROOT)
			goto retry;

		record_size = le32_to_cpu(ctx->attr->length);

		err = ntfs_attr_record_move_away(ctx, 0);
		if (err) {
			antfs_log_error("Failed to move out attribute #2");
			break;
		}
		freed += record_size;

		/* Check whether we are done. */
		if (size <= freed) {
			ntfs_attr_put_search_ctx(ctx);
			return 0;
		}
		/*
		 * Reposition to first attribute after $STANDARD_INFORMATION
		 * and $ATTRIBUTE_LIST instead of simply skipping this attribute
		 * because in the case when we have got only in-memory attribute
		 * list then ntfs_attr_lookup will fail when it tries to find
		 * $ATTRIBUTE_LIST.
		 */
		ntfs_attr_reinit_search_ctx(ctx);
		err = ntfs_attr_position(AT_FILE_NAME, ctx);
		if (err)
			break;
	}
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	if (err == -ENOSPC) {
		antfs_log_debug("No attributes left that could be moved out.");
	}
	return err;
}
