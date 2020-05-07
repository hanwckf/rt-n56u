/**
 * attrlist.c - Attribute list attribute handling code.  Originated from the Linux-NTFS
 *		project.
 *
 * Copyright (c) 2004-2005 Anton Altaparmakov
 * Copyright (c) 2004-2005 Yura Pakhuchiy
 * Copyright (c)      2006 Szabolcs Szakacsits
 * Copyright (c)      2016 Martin Pommerenke, Jens Krieg, Arwed Meyer,
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

#include "antfs.h"
#include "types.h"
#include "layout.h"
#include "attrib.h"
#include "attrlist.h"
#include "debug.h"
#include "unistr.h"
#include "misc.h"

/**
 * ntfs_attrlist_need - check whether inode need attribute list
 *
 * @param ni		opened ntfs inode for which perform check
 *
 * Check whether all are attributes belong to one MFT record, in that case
 * attribute list is not needed.
 *
 * @retval 1 if inode need attribute list
 * @retval 0 if not
 * @retval -EINVAL - Invalid arguments passed to function or attribute haven't
 *                   got attribute list.
 */
int ntfs_attrlist_need(struct ntfs_inode *ni)
{
	struct ATTR_LIST_ENTRY *ale;

	if (IS_ERR_OR_NULL(ni)) {
		antfs_log_error("Invalid arguments.");
		return -EINVAL;
	}

	antfs_log_enter("inode 0x%llx.", (long long)ni->mft_no);

	if (!NInoAttrList(ni)) {
		antfs_log_leave("Inode haven't got attribute list.");
		return -EINVAL;
	}

	if (!ni->attr_list) {
		antfs_log_error("Corrupt in-memory struct.");
		return -EINVAL;
	}

	ale = (struct ATTR_LIST_ENTRY *) ni->attr_list;
	while ((u8 *) ale < ni->attr_list + ni->attr_list_size) {
		if (MREF_LE(ale->mft_reference) != ni->mft_no) {
			antfs_log_leave("Need attrlist");
			return 1;
		}
		ale = (struct ATTR_LIST_ENTRY *) ((u8 *) ale +
			le16_to_cpu(ale->length));
	}
	antfs_log_leave("Don't need attrlist");
	return 0;
}

/**
 * ntfs_attrlist_entry_add - add an attribute list attribute entry
 *
 * @param ni	opened ntfs inode, which contains that attribute
 * @param attr	attribute record to add to attribute list
 *
 * @retval 0 on success
 * @retval -EINVAL - Invalid arguments passed to function.
 * @retval -ENOMEM - Not enough memory to allocate necessary buffers.
 * @retval -EIO - I/O error occurred or damaged filesystem.
 * @retval -EEXIST - Such attribute already present in attribute list.
 */
int ntfs_attrlist_entry_add(struct ntfs_inode *ni, struct ATTR_RECORD *attr)
{
	struct ATTR_LIST_ENTRY *ale;
	leMFT_REF mref;
	struct ntfs_attr *na = NULL;
	struct ntfs_attr_search_ctx *ctx;
	u8 *new_al;
	int entry_len, entry_offset, err;

	antfs_log_enter("inode 0x%llx, attr 0x%x",
			(long long)ni->mft_no,
			(unsigned)le32_to_cpu(attr->type));

	if (IS_ERR_OR_NULL(ni) || IS_ERR_OR_NULL(attr)) {
		antfs_log_error("Invalid arguments.");
		return -EINVAL;
	}

	mref = MK_LE_MREF(ni->mft_no, le16_to_cpu(ni->mrec->sequence_number));

	if (ni->nr_extents == -1)
		ni = ni->base_ni;

	if (!NInoAttrList(ni)) {
		antfs_log_debug("Attribute list isn't present.");
		return -ENOENT;
	}

	/* Determine size and allocate memory for new attribute list. */
	entry_len = (sizeof(struct ATTR_LIST_ENTRY) + sizeof(ntfschar) *
		     attr->name_length + 7) & ~7;
	new_al = ntfs_calloc(ni->attr_list_size + entry_len);
	if (!new_al)
		return -ENOMEM;

	/* Find place for the new entry. */
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto err_out;
	}

	err = ntfs_attr_lookup(attr->type, (attr->name_length) ? (ntfschar *)
		    ((u8 *) attr + le16_to_cpu(attr->name_offset)) :
		    AT_UNNAMED, attr->name_length, CASE_SENSITIVE,
		    (attr->non_resident) ? sle64_to_cpu(attr->lowest_vcn) : 0,
		    (attr->non_resident) ? NULL : ((u8 *) attr +
		    le16_to_cpu(attr->value_offset)), (attr->non_resident) ? 0 :
		    le32_to_cpu(attr->value_length), ctx);
	if (!err) {
		/* Found some extent, check it to be before new extent. */
		if (ctx->al_entry->lowest_vcn == attr->lowest_vcn) {
			err = -EEXIST;
			antfs_log_debug("Such attribute already present in the "
					"attribute list.");
			ntfs_attr_put_search_ctx(ctx);
			goto err_out;
		}
		/* Add new entry after this extent. */
		ale = (struct ATTR_LIST_ENTRY *) ((u8 *) ctx->al_entry +
					   le16_to_cpu(ctx->al_entry->length));
	} else {
		/* Check for real errors. */
		if (err != -ENOENT) {
			antfs_log_debug("Attribute lookup failed.");
			ntfs_attr_put_search_ctx(ctx);
			goto err_out;
		}
		/* No previous extents found. */
		ale = ctx->al_entry;
	}
	/* Don't need it anymore, @ctx->al_entry points to @ni->attr_list. */
	ntfs_attr_put_search_ctx(ctx);

	/* Determine new entry offset. */
	entry_offset = ((u8 *) ale - ni->attr_list);
	/* Set pointer to new entry. */
	ale = (struct ATTR_LIST_ENTRY *) (new_al + entry_offset);
	/* Zero it to fix valgrind warning. */
	memset(ale, 0, entry_len);
	/* Form new entry. */
	ale->type = attr->type;
	ale->length = cpu_to_le16(entry_len);
	ale->name_length = attr->name_length;
	ale->name_offset = offsetof(struct ATTR_LIST_ENTRY, name);
	if (attr->non_resident)
		ale->lowest_vcn = attr->lowest_vcn;
	else
		ale->lowest_vcn = const_cpu_to_sle64(0);
	ale->mft_reference = mref;
	ale->instance = attr->instance;
	memcpy(ale->name, (u8 *) attr + le16_to_cpu(attr->name_offset),
	       attr->name_length * sizeof(ntfschar));

	/* Resize $ATTRIBUTE_LIST to new length. */
	na = ntfs_attr_open(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
	if (IS_ERR(na)) {
		err = PTR_ERR(na);
		na = NULL;
		antfs_log_error("Failed to open $ATTRIBUTE_LIST attribute.");
		goto err_out;
	}
	err = ntfs_attr_truncate(na, ni->attr_list_size + entry_len);
	if (err) {
		antfs_log_error("$ATTRIBUTE_LIST resize failed: %d", err);
		/* Make STATUS_ATTRIBUTE_FILLED_MFT an error */
		if (err > 0)
			err = -ENOSPC;
		goto err_out;
	}

	/* Copy entries from old attribute list to new. */
	memcpy(new_al, ni->attr_list, entry_offset);
	memcpy(new_al + entry_offset + entry_len, ni->attr_list +
	       entry_offset, ni->attr_list_size - entry_offset);

	/* Set new runlist. */
	ntfs_free(ni->attr_list);
	ni->attr_list = new_al;
	ni->attr_list_size = ni->attr_list_size + entry_len;
	NInoAttrListSetDirty(ni);
	/* Done! */
	ntfs_attr_close(na);
	return 0;
err_out:
	ntfs_attr_close(na);
	ntfs_free(new_al);
	return err;
}

/**
 * ntfs_attrlist_entry_rm - remove an attribute list attribute entry
 * @param ctx attribute search context describing the attribute list entry
 *
 * Remove the attribute list entry @ctx->al_entry from the attribute list.
 *
 * @retval 0 on success
 * @retval negative error code on failure
 */
int ntfs_attrlist_entry_rm(struct ntfs_attr_search_ctx *ctx)
{
	u8 *new_al;
	int new_al_len;
	struct ntfs_inode *base_ni;
	struct ntfs_attr *na;
	struct ATTR_LIST_ENTRY *ale;
	int err;

	if (IS_ERR_OR_NULL(ctx) || !ctx->ntfs_ino || !ctx->al_entry) {
		antfs_log_error("Invalid arguments.");
		return -EINVAL;
	}

	if (ctx->base_ntfs_ino)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;
	ale = ctx->al_entry;

	antfs_log_enter
	    ("Entering for inode 0x%llx, attr 0x%x, lowest_vcn %lld.",
	     (long long)ctx->ntfs_ino->mft_no,
	     (unsigned)le32_to_cpu(ctx->al_entry->type),
	     (long long)sle64_to_cpu(ctx->al_entry->lowest_vcn));

	if (!NInoAttrList(base_ni)) {
		antfs_log_debug("Attribute list isn't present.");
		return -ENOENT;
	}

	/* Allocate memory for new attribute list. */
	new_al_len = base_ni->attr_list_size - le16_to_cpu(ale->length);
	new_al = ntfs_calloc(new_al_len);
	if (!new_al)
		return -ENOMEM;

	/* Reisze $ATTRIBUTE_LIST to new length. */
	na = ntfs_attr_open(base_ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
	if (IS_ERR(na)) {
		err = PTR_ERR(na);
		na = NULL;
		antfs_log_error("Failed to open $ATTRIBUTE_LIST attribute: %d",
				err);
		goto err_out;
	}

	err = ntfs_attr_truncate(na, new_al_len);
	if (err) {
		antfs_log_error("$ATTRIBUTE_LIST resize failed: %d", err);
		/* Make STATUS_ATTRIBUTE_FILLED_MFT an error. */
		if (err > 0)
			err = -ENOSPC;
		goto err_out;
	}

	/* Copy entries from old attribute list to new. */
	memcpy(new_al, base_ni->attr_list, (u8 *) ale - base_ni->attr_list);
	memcpy(new_al + ((u8 *) ale - base_ni->attr_list),
	       (u8 *) ale + le16_to_cpu(ale->length),
	       new_al_len - ((u8 *) ale - base_ni->attr_list));

	/* Set new runlist. */
	ntfs_free(base_ni->attr_list);
	base_ni->attr_list = new_al;
	base_ni->attr_list_size = new_al_len;
	NInoAttrListSetDirty(base_ni);
	/* Done! */
	ntfs_attr_close(na);
	return 0;
err_out:
	ntfs_attr_close(na);
	ntfs_free(new_al);
	return err;
}
