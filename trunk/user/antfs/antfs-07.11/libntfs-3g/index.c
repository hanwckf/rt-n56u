/**
 * index.c - NTFS index handling.  Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2004-2005 Anton Altaparmakov
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2005-2006 Yura Pakhuchiy
 * Copyright (c) 2005-2008 Szabolcs Szakacsits
 * Copyright (c) 2007 Jean-Pierre Andre
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
/* TODO:
 * This may look sort of nice and clean on first look but in the end this is
 * encapsulation hell where we do things twice and threefold.
 * Surely one can throw out a good share of redundant code in here.
 */

#include <asm/div64.h>

#include "antfs.h"
#include "attrib.h"
#include "debug.h"
#include "index.h"
#include "collate.h"
#include "mst.h"
#include "dir.h"
#include "bitmap.h"
#include "reparse.h"
#include "misc.h"

/**
 * ntfs_index_entry_mark_dirty - mark an index entry dirty
 *
 * @param ictx  ntfs index context describing the index entry
 *
 * Mark the index entry described by the index entry context @ictx dirty.
 *
 * If the index entry is in the index root attribute, simply mark the inode
 * containing the index root attribute dirty.  This ensures the mftrecord, and
 * hence the index root attribute, will be written out to disk later.
 *
 * If the index entry is in an index block belonging to the index allocation
 * attribute, set ib_dirty to TRUE, thus index block will be updated during
 * ntfs_index_ctx_put.
 */
void ntfs_index_entry_mark_dirty(struct ntfs_index_context *ictx)
{
	if (ictx->is_in_root)
		ntfs_inode_mark_dirty(ictx->actx->ntfs_ino);
	else
		ictx->ib_dirty = TRUE;
}

static s64 ntfs_ib_vcn_to_pos(struct ntfs_index_context *icx, VCN vcn)
{
	return vcn << icx->vcn_size_bits;
}

static VCN ntfs_ib_pos_to_vcn(struct ntfs_index_context *icx, s64 pos)
{
	return pos >> icx->vcn_size_bits;
}

/**
 * @retval STATUS_OK if ok (this is 0)
 * @return negative error code on failure
 */
static int ntfs_ib_write(struct ntfs_index_context *icx, struct INDEX_BLOCK *ib)
{
	s64 ret, vcn = sle64_to_cpu(ib->index_block_vcn);

	antfs_log_enter("vcn: %lld", (long long)vcn);

	ret = ntfs_attr_mst_pwrite(icx->ia_na, ntfs_ib_vcn_to_pos(icx, vcn),
				   1, icx->block_size_bits, ib);
	if (ret != 1) {
		antfs_log_error("Failed to write index block %lld, inode %llu",
				(long long)vcn,
				(unsigned long long)icx->ni->mft_no);
		return (ret < 0) ? (int)ret : -EIO;
	}

	antfs_log_leave("ok");
	return STATUS_OK;
}

/**
 * @retval STATUS_OK if ok (this is 0)
 * @return negative error code on failure
 */
static int ntfs_icx_ib_write(struct ntfs_index_context *icx)
{
	int err;

	err = ntfs_ib_write(icx, icx->ib);
	if (err != 0)
		return err;

	icx->ib_dirty = FALSE;

	return STATUS_OK;
}

/**
 * ntfs_index_ctx_get - allocate and initialize a new index context
 *
 * @param ni        ntfs inode with which to initialize the context
 * @param name      name of the which context describes
 * @param name_len  length of the index name
 *
 * Allocate a new index context, initialize it with @ni and return it.
 *
 * @return ptr to new index context or error ptr
 */
struct ntfs_index_context *ntfs_index_ctx_get(struct ntfs_inode *ni,
				       ntfschar *name, u32 name_len)
{
	struct ntfs_index_context *icx;

	antfs_log_enter();

	if (IS_ERR_OR_NULL(ni)) {
		antfs_log_error("Invalid arguments.");
		return ERR_PTR(-EINVAL);
	}
	if (ni->nr_extents == -1)
		ni = ni->base_ni;
	icx = ntfs_calloc(sizeof(struct ntfs_index_context));
	if (icx) {
		*icx = (struct ntfs_index_context) {
			.ni = ni,
			.name = name,
			.name_len = name_len,
		};
	} else {
		antfs_log_error("Failed to allocate index context! OOM");
		icx = ERR_PTR(-ENOMEM);
	}
	antfs_log_leave();
	return icx;
}

static void ntfs_index_ctx_free(struct ntfs_index_context *icx)
{
	antfs_log_enter();

	if (!icx->bad_index && !icx->entry)
		return;

	if (icx->actx)
		ntfs_attr_put_search_ctx(icx->actx);

	if (!icx->is_in_root) {
		if (icx->ib_dirty) {
			/* FIXME: Error handling!!! */
			ntfs_ib_write(icx, icx->ib);
		}
		ntfs_free(icx->ib);
	}

	ntfs_attr_close(icx->ia_na);
	antfs_log_leave();
}

/**
 * ntfs_index_ctx_put - release an index context
 *
 * @param icx  index context to free
 *
 * Release the index context @icx, releasing all associated resources.
 */
void ntfs_index_ctx_put(struct ntfs_index_context *icx)
{
	ntfs_index_ctx_free(icx);
	ntfs_free(icx);
}

/**
 * ntfs_index_ctx_reinit - reinitialize an index context
 * @param icx  index context to reinitialize
 *
 * Reinitialize the index context @icx so it can be used for ntfs_index_lookup.
 */
void ntfs_index_ctx_reinit(struct ntfs_index_context *icx)
{
	antfs_log_enter();

	ntfs_index_ctx_free(icx);

	*icx = (struct ntfs_index_context) {
		.ni = icx->ni,
		.name = icx->name,
		.name_len = icx->name_len,
	};
}

static sle64 *ntfs_ie_get_vcn_addr(struct INDEX_ENTRY *ie)
{
	return (sle64 *) ((u8 *) ie + le16_to_cpu(ie->length) -
			     sizeof(sle64));
}

/**
 *  Get the subnode vcn to which the index entry refers.
 */
VCN ntfs_ie_get_vcn(struct INDEX_ENTRY *ie)
{
	return sle64_to_cpup(ntfs_ie_get_vcn_addr(ie));
}

static struct INDEX_ENTRY *ntfs_ie_get_first(struct INDEX_HEADER *ih)
{
	return (struct INDEX_ENTRY *) ((u8 *) ih +
			le32_to_cpu(ih->entries_offset));
}

static struct INDEX_ENTRY *ntfs_ie_get_next(struct INDEX_ENTRY *ie)
{
	return (struct INDEX_ENTRY *) ((char *)ie + le16_to_cpu(ie->length));
}

static u8 *ntfs_ie_get_end(struct INDEX_HEADER *ih)
{
	/* FIXME: check if it isn't overflowing the index block size */
	return (u8 *) ih + le32_to_cpu(ih->index_length);
}

static int ntfs_ie_end(struct INDEX_ENTRY *ie)
{
	return ie->ie_flags & INDEX_ENTRY_END || !ie->length;
}

/**
 *  Find the last entry in the index block
 */
static struct INDEX_ENTRY *ntfs_ie_get_last(struct INDEX_ENTRY *ie,
					    char *ies_end)
{
	antfs_log_enter();

	while ((char *)ie < ies_end && !ntfs_ie_end(ie))
		ie = ntfs_ie_get_next(ie);

	return ie;
}

static struct INDEX_ENTRY *ntfs_ie_get_by_pos(struct INDEX_HEADER *ih, int pos)
{
	struct INDEX_ENTRY *ie;

	antfs_log_enter("pos: %d", pos);

	ie = ntfs_ie_get_first(ih);

	while (pos-- > 0)
		ie = ntfs_ie_get_next(ie);

	return ie;
}

static struct INDEX_ENTRY *ntfs_ie_prev(struct INDEX_HEADER *ih,
					struct INDEX_ENTRY *ie)
{
	struct INDEX_ENTRY *ie_prev = NULL;
	struct INDEX_ENTRY *tmp;

	antfs_log_enter();

	tmp = ntfs_ie_get_first(ih);

	while (tmp != ie) {
		ie_prev = tmp;
		tmp = ntfs_ie_get_next(tmp);
	}

	return ie_prev;
}

static int ntfs_ih_numof_entries(struct INDEX_HEADER *ih)
{
	int n;
	struct INDEX_ENTRY *ie;
	u8 *end;

	antfs_log_enter();

	end = ntfs_ie_get_end(ih);
	ie = ntfs_ie_get_first(ih);
	for (n = 0; !ntfs_ie_end(ie) && (u8 *) ie < end; n++)
		ie = ntfs_ie_get_next(ie);
	antfs_log_leave();
	return n;
}

static int ntfs_ih_one_entry(struct INDEX_HEADER *ih)
{
	return (ntfs_ih_numof_entries(ih) == 1);
}

static int ntfs_ih_zero_entry(struct INDEX_HEADER *ih)
{
	return (ntfs_ih_numof_entries(ih) == 0);
}

static void ntfs_ie_delete(struct INDEX_HEADER *ih, struct INDEX_ENTRY *ie)
{
	u32 new_size;

	antfs_log_enter();

	new_size = le32_to_cpu(ih->index_length) - le16_to_cpu(ie->length);
	ih->index_length = cpu_to_le32(new_size);
	memmove(ie, (u8 *) ie + le16_to_cpu(ie->length),
		new_size - ((u8 *) ie - (u8 *) ih));
	antfs_log_leave();
}

static void ntfs_ie_set_vcn(struct INDEX_ENTRY *ie, VCN vcn)
{
	*ntfs_ie_get_vcn_addr(ie) = cpu_to_sle64(vcn);
}

/**
 *  Insert @ie index entry at @pos entry. Used @ih values should be ok already.
 */
static void ntfs_ie_insert(struct INDEX_HEADER *ih, struct INDEX_ENTRY *ie,
			   struct INDEX_ENTRY *pos)
{
	int ie_size = le16_to_cpu(ie->length);

	antfs_log_enter();

	ih->index_length = cpu_to_le32(le32_to_cpu(ih->index_length) + ie_size);
	memmove((u8 *) pos + ie_size, pos,
		le32_to_cpu(ih->index_length) - ((u8 *) pos - (u8 *) ih) -
		ie_size);
	memcpy(pos, ie, ie_size);
}

/**
 * Create copy of @INDEX_ENTRY
 *
 * @param ie  Index entry to copy
 *
 * @return Copy of @ie or NULL
 */
static struct INDEX_ENTRY *ntfs_ie_dup(struct INDEX_ENTRY *ie)
{
	struct INDEX_ENTRY *dup;

	antfs_log_enter();

	dup = ntfs_malloc(le16_to_cpu(ie->length));
	if (dup)
		memcpy(dup, ie, le16_to_cpu(ie->length));

	return dup;
}

/**
 * Creaty copy of @INDEX_ENTRY without VCN
 *
 * @param ie  Index entry to copy
 *
 * @return Copy of index entry with VCN removed or NULL
 */
static struct INDEX_ENTRY *ntfs_ie_dup_novcn(struct INDEX_ENTRY *ie)
{
	struct INDEX_ENTRY *dup;
	int size = le16_to_cpu(ie->length);

	antfs_log_enter();

	if (ie->ie_flags & INDEX_ENTRY_NODE)
		size -= sizeof(VCN);

	dup = ntfs_malloc(size);
	if (dup) {
		memcpy(dup, ie, size);
		dup->ie_flags &= ~INDEX_ENTRY_NODE;
		dup->length = cpu_to_le16(size);
	}
	return dup;
}

/**
 * @retval 0 ok
 * @retval -1 failure (no error code)
 */
static int ntfs_ia_check(struct ntfs_index_context *icx, struct INDEX_BLOCK *ib,
			 VCN vcn)
{
	u32 ib_size = (unsigned)le32_to_cpu(ib->index.allocated_size) + 0x18;

	antfs_log_enter();

	if (!ntfs_is_indx_record(ib->magic)) {

		/* log? message for already corrupt ib signature? */
		antfs_log_error("Corrupt index block signature: vcn %lld inode "
				"%llu", (long long)vcn,
				(unsigned long long)icx->ni->mft_no);
		return -1;
	}

	if (sle64_to_cpu(ib->index_block_vcn) != vcn) {

		antfs_log_error("Corrupt index block: VCN (%lld) is different "
				"from expected VCN (%lld) in inode %llu",
				(long long)sle64_to_cpu(ib->index_block_vcn),
				(long long)vcn,
				(unsigned long long)icx->ni->mft_no);
		return -1;
	}

	if (ib_size != 1 << icx->block_size_bits) {

		antfs_log_error
		    ("Corrupt index block : VCN (%lld) of inode %llu "
		     "has a size (%u) differing from the index "
		     "specified size (%u)", (long long)vcn,
		     (unsigned long long)icx->ni->mft_no, ib_size,
		     1 << icx->block_size_bits);
		return -1;
	}
	antfs_log_leave();
	return 0;
}

/**
 * @return Pointer to INDEX_ROOT or error ptr on failure
 */
static struct INDEX_ROOT *ntfs_ir_lookup(struct ntfs_inode *ni, ntfschar *name,
				  u32 name_len,
				  struct ntfs_attr_search_ctx **ctx)
{
	struct ATTR_RECORD *a;
	struct INDEX_ROOT *ir = NULL;
	int err;

	antfs_log_enter();

	*ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(*ctx)) {
		antfs_log_leave("no search_ctx");
		return (struct INDEX_ROOT *) ctx;
	}

	err = ntfs_attr_lookup(AT_INDEX_ROOT, name, name_len, CASE_SENSITIVE, 0,
			      NULL, 0, *ctx);
	if (err != 0) {
		antfs_log_error("Failed to lookup $INDEX_ROOT");
		goto err_out;
	}

	a = (*ctx)->attr;
	if (a->non_resident) {
		err = -EINVAL;
		antfs_log_error("Non-resident $INDEX_ROOT detected");
		goto err_out;
	}

	ir = (struct INDEX_ROOT *) ((char *)a + le16_to_cpu(a->value_offset));
err_out:
	if (!ir) {
		ntfs_attr_put_search_ctx(*ctx);
		ir = ERR_PTR(err);
	}
	if (IS_ERR(ir)) {
		antfs_log_leave("err: %d", (int)PTR_ERR(ir));
	} else {
		antfs_log_leave();
	}
	return ir;
}

/**
 * @return Pointer to INDEX_ROOT or error ptr on failure
 */
static struct INDEX_ROOT *ntfs_ir_lookup2(struct ntfs_inode *ni, ntfschar *name,
				   u32 len)
{
	struct ntfs_attr_search_ctx *ctx;
	struct INDEX_ROOT *ir;

	ir = ntfs_ir_lookup(ni, name, len, &ctx);
	if (!IS_ERR(ir))
		ntfs_attr_put_search_ctx(ctx);
	return ir;
}

/**
 * @brief Find a key in the index block.
 *
 * @retval STATUS_OK with if we know for sure that the
 *             entry exists and @ie_out points to this entry.
 * @retval STATUS_NOT_FOUND (this is -ENOENT) if we know for sure the
 *             entry doesn't exist and @ie_out is the insertion point.
 * @retval STATUS_KEEP_SEARCHING (this is -EAGAIN) if we can't answer the above
 *             question and @vcn will contain the node index block.
 * @return other negative error codes on unexpected error during lookup.
 *         You can use IS_STATUS_ERROR macro to check for this state.
 */
static int ntfs_ie_lookup(const void *key, const int key_len,
			  struct ntfs_index_context *icx,
			  struct INDEX_HEADER *ih, VCN *vcn,
			  struct INDEX_ENTRY **ie_out)
{
	struct INDEX_ENTRY *ie;
	u8 *index_end;
	int rc, item = 0;

	antfs_log_enter();

	index_end = ntfs_ie_get_end(ih);

	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (ie = ntfs_ie_get_first(ih);; ie = ntfs_ie_get_next(ie)) {
		/* Bounds checks. */
		if ((u8 *) ie + sizeof(struct INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *) ie + le16_to_cpu(ie->length) > index_end) {
			antfs_log_error("Index entry out of bounds in inode "
					"%llu.",
					(unsigned long long)icx->ni->mft_no);
			return -ERANGE;
		}
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ntfs_ie_end(ie))
			break;
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		if (!icx->collate) {
			antfs_log_error("Collation function not defined");
			return -EOPNOTSUPP;
		}
		rc = icx->collate(icx->ni->vol, key, key_len,
				  &ie->key, le16_to_cpu(ie->key_length));
		if (rc == NTFS_COLLATION_ERROR) {
			antfs_log_error("Collation error. Perhaps a filename "
					"contains invalid characters?");
			return -ERANGE;
		}
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;

		if (!rc) {
			*ie_out = ie;
			icx->parent_pos[icx->pindex] = item;
			return STATUS_OK;
		}

		item++;
	}
	/*
	 * We have finished with this index block without success. Check for the
	 * presence of a child node and if not present return with errno ENOENT,
	 * otherwise we will keep searching in another index block.
	 */
	if (!(ie->ie_flags & INDEX_ENTRY_NODE)) {
		antfs_log_leave("Index entry wasn't found.");
		*ie_out = ie;
		return STATUS_NOT_FOUND;
	}

	/* Get the starting vcn of the index_block holding the child node. */
	*vcn = ntfs_ie_get_vcn(ie);
	if (*vcn < 0) {
		antfs_log_error("Negative vcn in inode %llu",
				(unsigned long long)icx->ni->mft_no);
		return -EINVAL;
	}

	antfs_log_leave("Parent entry number %d", item);
	icx->parent_pos[icx->pindex] = item;

	return STATUS_KEEP_SEARCHING;
}

/**
 * @brief Open AT_INDEX_ALLOCATION attribute for inode
 *
 * @return Pointer to opened ntfs attribute or error ptr on failure
 */
static struct ntfs_attr *ntfs_ia_open(struct ntfs_index_context *icx,
				      struct ntfs_inode *ni)
{
	struct ntfs_attr *na;

	na = ntfs_attr_open(ni, AT_INDEX_ALLOCATION, icx->name, icx->name_len);
	if (IS_ERR(na)) {
		antfs_log_error("Failed to open index allocation of inode "
				"%llu: %d", (unsigned long long)ni->mft_no,
				(int)PTR_ERR(na));
	}

	return na;
}

/**
 * @brief Read from index block
 *
 * @return 0 if ok or negative error code on failure
 */
static int ntfs_ib_read(struct ntfs_index_context *icx, VCN vcn,
			struct INDEX_BLOCK *dst)
{
	s64 pos, ret;

	antfs_log_enter("vcn: %lld", (long long)vcn);

	pos = ntfs_ib_vcn_to_pos(icx, vcn);

	ret =
	    ntfs_attr_mst_pread(icx->ia_na, pos, 1, icx->block_size_bits,
				(u8 *) dst, true);
	if (ret != 1) {
		if (ret < 0) {
			antfs_log_error("Failed to read index block");
			return ret;
		} else {
			antfs_log_error("Failed to read full index block at "
					"%lld", (long long)pos);
			return -EIO;
		}
	}

	if (ntfs_ia_check(icx, dst, vcn)) {
		antfs_log_leave("-EIO");
		return -EIO;
	}

	antfs_log_leave();
	return 0;
}

/**
 * @return STATUS_OK (0) if ok or -EOPNOTSUPP on overflow
 */
static int ntfs_icx_parent_inc(struct ntfs_index_context *icx)
{
	icx->pindex++;
	if (icx->pindex >= MAX_PARENT_VCN) {
		antfs_log_error("Index is over %d level deep", MAX_PARENT_VCN);
		return -EOPNOTSUPP;
	}
	return STATUS_OK;
}

/**
 * @return STATUS_OK (0) if ok or -EINVAL on underflow
 */
static int ntfs_icx_parent_dec(struct ntfs_index_context *icx)
{
	icx->pindex--;
	if (icx->pindex < 0) {
		antfs_log_error("Corrupt index pointer (%d)", icx->pindex);
		return -EINVAL;
	}
	return STATUS_OK;
}

/**
 * ntfs_index_lookup - find a key in an index and return its index entry
 *
 * @param[IN] key key  for which to search in the index
 * @param[IN] key_len  length of @key in bytes
 * @param[INOUT] icx   context describing the index and the returned entry
 *
 * Before calling ntfs_index_lookup(), @icx must have been obtained from a
 * call to ntfs_index_ctx_get().
 *
 * Look for the @key in the index specified by the index lookup context @icx.
 * ntfs_index_lookup() walks the contents of the index looking for the @key.
 *
 * If the @key is found in the index, 0 is returned and @icx is setup to
 * describe the index entry containing the matching @key.  @icx->entry is the
 * index entry and @icx->data and @icx->data_len are the index entry data and
 * its length in bytes, respectively.
 *
 * If the @key is not found in the index, -ENOENT is returned and
 * @icx is setup to describe the index entry whose key collates immediately
 * after the search @key, i.e. this is the position in the index at which
 * an index entry with a key of @key would need to be inserted.
 *
 * If an error occurs the negative error code is returned and @icx left
 * untouched.
 *
 * When finished with the entry and its data, call ntfs_index_ctx_put() to free
 * the context and other associated resources.
 *
 * If the index entry was modified, call ntfs_index_entry_mark_dirty() before
 * the call to ntfs_index_ctx_put() to ensure that the changes are written
 * to disk.
 *
 * @return 0 if ok or negative error code
 */
int ntfs_index_lookup(const void *key, const int key_len,
		      struct ntfs_index_context *icx)
{
	VCN old_vcn, vcn;
	struct ntfs_inode *ni = icx->ni;
	struct INDEX_ROOT *ir;
	struct INDEX_ENTRY *ie;
	struct INDEX_BLOCK *ib = NULL;
	u32 block_size;
	int err;

	antfs_log_enter();

	if (!key || key_len <= 0) {
		antfs_log_error("key: %p  key_len: %d", key, key_len);
		return -EINVAL;
	}

	ir = ntfs_ir_lookup(ni, icx->name, icx->name_len, &icx->actx);
	if (IS_ERR(ir)) {
		err = PTR_ERR(ir);
		if (err == -ENOENT)
			err = -EIO;
		return err;
	}

	block_size = le32_to_cpu(ir->index_block_size);
	if (block_size < NTFS_BLOCK_SIZE) {
		err = -EINVAL;
		antfs_log_error("Index block size (0x%x) is smaller than the "
				"sector size (0x%x)", (int)block_size,
				NTFS_BLOCK_SIZE);
		goto err_out;
	}

	if (block_size & (block_size - 1)) {
		err = -EINVAL;
		antfs_log_error("Index block size (0x%x) is not power of 2.",
				(int)block_size);
		goto err_out;
	}

	icx->block_size_bits = __ffs(block_size);

	if (ni->vol->cluster_size <= block_size)
		icx->vcn_size_bits = ni->vol->cluster_size_bits;
	else
		icx->vcn_size_bits = NTFS_BLOCK_SIZE_BITS;
	/* get the appropriate collation function */
	icx->collate = ntfs_get_collate_function(ir->collation_rule);
	if (IS_ERR(icx->collate)) {
		err = PTR_ERR(icx->collate);
		antfs_log_error("Unknown collation rule 0x%x",
				(unsigned)le32_to_cpu(ir->collation_rule));
		goto err_out;
	}

	old_vcn = VCN_INDEX_ROOT_PARENT;
	/*
	 * FIXME: check for both ir and ib that the first index entry is
	 * within the index block.
	 */
	err = ntfs_ie_lookup(key, key_len, icx, &ir->index, &vcn, &ie);
	if (IS_STATUS_ERROR(err))
		goto err_lookup;

	icx->ir = ir;

	if (err != STATUS_KEEP_SEARCHING) {
		/* STATUS_OK or STATUS_NOT_FOUND */
		icx->is_in_root = TRUE;
		icx->parent_vcn[icx->pindex] = old_vcn;
		goto done;
	}

	/* Child node present, descend into it. */

	icx->ia_na = ntfs_ia_open(icx, ni);
	if (IS_ERR(icx->ia_na)) {
		err = PTR_ERR(icx->ia_na);
		if (err == -ENOENT)
			err = -EIO;
		icx->ia_na = NULL;
		goto err_out;
	}

	ib = ntfs_malloc(block_size);
	if (!ib) {
		err = -ENOMEM;
		goto err_out;
	}

descend_into_child_node:
	icx->parent_vcn[icx->pindex] = old_vcn;
	err = ntfs_icx_parent_inc(icx);
	if (err != 0)
		goto err_out;
	old_vcn = vcn;

	antfs_log_debug("Descend into node with VCN %lld", (long long)vcn);

	err = ntfs_ib_read(icx, vcn, ib);
	if (err != 0)
		goto err_out;

	err = ntfs_ie_lookup(key, key_len, icx, &ib->index, &vcn, &ie);
	if (err != STATUS_KEEP_SEARCHING) {
		if (err && err != STATUS_NOT_FOUND)
			goto err_out;

		/* STATUS_OK or STATUS_NOT_FOUND */
		icx->is_in_root = FALSE;
		icx->ib = ib;
		icx->parent_vcn[icx->pindex] = vcn;
		goto done;
	}

	if ((ib->index.ih_flags & NODE_MASK) == LEAF_NODE) {
		antfs_log_error("Index entry with child node found in a leaf "
				"node in inode 0x%llx.",
				(unsigned long long)ni->mft_no);
		goto err_out;
	}

	goto descend_into_child_node;

err_out:
	icx->bad_index = TRUE;	/* Force icx->* to be freed */
err_lookup:
	ntfs_free(ib);
	if (!err)
		err = -EIO;
	antfs_log_leave("err: %d", err);
	return err;
done:
	icx->entry = ie;
	icx->data = (u8 *) ie + offsetof(struct INDEX_ENTRY, key);
	icx->data_len = le16_to_cpu(ie->key_length);
	antfs_log_leave("err: %d", err);
	return err;

}

/**
 * @return Allocated index block or NULL
 */
static struct INDEX_BLOCK *ntfs_ib_alloc(VCN ib_vcn, u32 ib_size,
				enum INDEX_HEADER_FLAGS node_type)
{
	struct INDEX_BLOCK *ib;
	int ih_size = sizeof(struct INDEX_HEADER);

	antfs_log_enter("ib_vcn: %lld ib_size: %u", (long long)ib_vcn, ib_size);

	ib = ntfs_calloc(ib_size);
	if (!ib)
		return NULL;

	ib->magic = magic_INDX;
	ib->usa_ofs = const_cpu_to_le16(sizeof(struct INDEX_BLOCK));
	ib->usa_count = cpu_to_le16(ib_size / NTFS_BLOCK_SIZE + 1);
	/* Set USN to 1 */
	*(le16 *) ((char *)ib + le16_to_cpu(ib->usa_ofs)) =
	    const_cpu_to_le16(1);
	ib->lsn = const_cpu_to_sle64(0);

	ib->index_block_vcn = cpu_to_sle64(ib_vcn);

	ib->index.entries_offset = cpu_to_le32((ih_size +
						le16_to_cpu(ib->usa_count) * 2 +
						7) & ~7);
	ib->index.index_length = const_cpu_to_le32(0);
	ib->index.allocated_size = cpu_to_le32(ib_size -
				    (sizeof(struct INDEX_BLOCK) - ih_size));
	ib->index.ih_flags = node_type;

	return ib;
}

/**
 *  @brief Find the median by going through all the entries
 *
 *  Median means the entry in the middle of the index array
 *  for odd number of entries or the one left to the center for
 *  even number of entries.
 */
static struct INDEX_ENTRY *ntfs_ie_get_median(struct INDEX_HEADER *ih)
{
	struct INDEX_ENTRY *ie, *ie_start;
	u8 *ie_end;
	int i = 0, median;

	antfs_log_enter();

	ie = ie_start = ntfs_ie_get_first(ih);
	ie_end = (u8 *) ntfs_ie_get_end(ih);

	while ((u8 *) ie < ie_end && !ntfs_ie_end(ie)) {
		ie = ntfs_ie_get_next(ie);
		i++;
	}
	/*
	 * NOTE: this could be also the entry at the half of the index block.
	 */
	median = i / 2 - 1;

	antfs_log_debug("Entries: %d  median: %d", i, median);

	for (i = 0, ie = ie_start; i <= median; i++)
		ie = ntfs_ie_get_next(ie);

	return ie;
}

static s64 ntfs_ibm_vcn_to_pos(struct ntfs_index_context *icx, VCN vcn)
{
	return ntfs_ib_vcn_to_pos(icx, vcn) >> icx->block_size_bits;
}

static s64 ntfs_ibm_pos_to_vcn(struct ntfs_index_context *icx, s64 pos)
{
	return ntfs_ib_pos_to_vcn(icx, pos << icx->block_size_bits);
}

/**
 * @retval STATUS_OK ok (0)
 * @return negative error code on failure
 */
static int ntfs_ibm_add(struct ntfs_index_context *icx)
{
	u8 bmp[8];
	int err;

	antfs_log_enter();

	if (ntfs_attr_exist(icx->ni, AT_BITMAP, icx->name, icx->name_len)) {
		antfs_log_leave();
		return STATUS_OK;
	}
	/*
	 * AT_BITMAP must be at least 8 bytes.
	 */
	memset(bmp, 0, sizeof(bmp));
	err = ntfs_attr_add(icx->ni, AT_BITMAP, icx->name, icx->name_len,
			    bmp, sizeof(bmp));
	if (err != 0) {
		antfs_log_error("Failed to add AT_BITMAP");
		return err;
	}

	antfs_log_leave();
	return STATUS_OK;
}

/**
 * @retval STATUS_OK ok (0)
 * @return negative error code on failure
 */
static int ntfs_ibm_modify(struct ntfs_index_context *icx, VCN vcn, int set)
{
	u8 byte[8] = {0};
	s64 pos = ntfs_ibm_vcn_to_pos(icx, vcn);
	u32 bpos = pos / 8;
	u32 bit = 1 << (pos % 8);
	struct ntfs_attr *na;
	int to_write = 1;
	int err;

	antfs_log_enter("%s vcn: %lld", set ? "set" : "clear", (long long)vcn);

	na = ntfs_attr_open(icx->ni, AT_BITMAP, icx->name, icx->name_len);
	if (IS_ERR(na)) {
		antfs_log_error("Failed to open $BITMAP attribute");
		return PTR_ERR(na);
	}

	if (set && (na->data_size < bpos + 1)) {
		/* write up to 8 bytes so initialized_size is always the same as
		 * data_size. chkdsk will complain otherwise
		 */
		to_write = 8 - (na->data_size & 7);
		err = ntfs_attr_truncate(na, (na->data_size + 8) & ~7);
		if (err != 0) {
			/* Always make this an error. */
			if (err > 0)
				err = -ENOSPC;
			antfs_log_error("Failed to truncate AT_BITMAP");
			goto err_na;
		}
	}

	err = ntfs_attr_pread(na, bpos, 1, byte);
	if (err != 1) {
		antfs_log_error("Failed to read $BITMAP");
		goto err_na;
	}

	if (set)
		byte[0] |= bit;
	else
		byte[0] &= ~bit;

	err = ntfs_attr_pwrite(na, bpos, to_write, byte);
	if (err != to_write) {
		antfs_log_error("Failed to write $Bitmap");
		goto err_na;
	}

	err = STATUS_OK;
err_na:
	if (err > 0)
		err = -EIO;
	ntfs_attr_close(na);
	antfs_log_leave("err=%d", err);
	return err;
}

static int ntfs_ibm_set(struct ntfs_index_context *icx, VCN vcn)
{
	return ntfs_ibm_modify(icx, vcn, 1);
}

static int ntfs_ibm_clear(struct ntfs_index_context *icx, VCN vcn)
{
	return ntfs_ibm_modify(icx, vcn, 0);
}

/**
 * @return 0 on success or negative error code
 */
static int ntfs_ibm_get_free(struct ntfs_index_context *icx, VCN *rvcn)
{
	u8 *bm;
	int bit;
	s64 vcn, byte, size;
	int err;

	antfs_log_enter();

	bm = ntfs_attr_readall(icx->ni, AT_BITMAP, icx->name, icx->name_len,
			       &size);
	if (IS_ERR(bm))
		return PTR_ERR(bm);

	for (byte = 0; byte < size; byte++) {

		if (bm[byte] == 255)
			continue;

		for (bit = 0; bit < 8; bit++) {
			if (!(bm[byte] & (1 << bit))) {
				vcn = ntfs_ibm_pos_to_vcn(icx, byte * 8 + bit);
				goto out;
			}
		}
	}

	vcn = ntfs_ibm_pos_to_vcn(icx, size * 8);
out:
	antfs_log_debug("allocated vcn: %lld", (long long)vcn);

	err = ntfs_ibm_set(icx, vcn);
	if (err != 0)
		vcn = (VCN) -1;

	ntfs_free(bm);
	*rvcn = vcn;
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * @brief Create copy of INDEX_ROOT converted to INDEX_BLOCK
 *
 * @return Pointer tp INDEX_BLOCK or NULL
 */
static struct INDEX_BLOCK *ntfs_ir_to_ib(struct INDEX_ROOT *ir, VCN ib_vcn)
{
	struct INDEX_BLOCK *ib;
	struct INDEX_ENTRY *ie_last;
	char *ies_start, *ies_end;
	int i;

	antfs_log_enter("Entering for VCN %lld", ib_vcn);

	ib = ntfs_ib_alloc(ib_vcn, le32_to_cpu(ir->index_block_size),
			   LEAF_NODE);
	if (!ib)
		return NULL;

	ies_start = (char *)ntfs_ie_get_first(&ir->index);
	ies_end = (char *)ntfs_ie_get_end(&ir->index);
	ie_last = ntfs_ie_get_last((struct INDEX_ENTRY *) ies_start, ies_end);
	/*
	 * Copy all entries, including the termination entry
	 * as well, which can never have any data.
	 */
	i = (char *)ie_last - ies_start + le16_to_cpu(ie_last->length);
	memcpy(ntfs_ie_get_first(&ib->index), ies_start, i);

	ib->index.ih_flags = ir->index.ih_flags;
	ib->index.index_length = cpu_to_le32(i +
					     le32_to_cpu(ib->index.
							 entries_offset));
	return ib;
}

static void ntfs_ir_nill(struct INDEX_ROOT *ir)
{
	struct INDEX_ENTRY *ie_last;
	char *ies_start, *ies_end;

	antfs_log_enter();
	/*
	 * TODO: This function could be much simpler.
	 */
	ies_start = (char *)ntfs_ie_get_first(&ir->index);
	ies_end = (char *)ntfs_ie_get_end(&ir->index);
	ie_last = ntfs_ie_get_last((struct INDEX_ENTRY *) ies_start, ies_end);
	/*
	 * Move the index root termination entry forward
	 */
	if ((char *)ie_last > ies_start) {
		memmove(ies_start, (char *)ie_last,
			le16_to_cpu(ie_last->length));
		ie_last = (struct INDEX_ENTRY *) ies_start;
	}
}

/**
 * @return 0 on success or negative error code
 */
static int ntfs_ib_copy_tail(struct ntfs_index_context *icx,
			     struct INDEX_BLOCK *src,
			     struct INDEX_ENTRY *median, VCN new_vcn)
{
	u8 *ies_end;
	struct INDEX_ENTRY *ie_head;	/* first entry after the median */
	int tail_size, err;
	struct INDEX_BLOCK *dst;

	antfs_log_enter();

	dst = ntfs_ib_alloc(new_vcn, 1 << icx->block_size_bits,
			    src->index.ih_flags & NODE_MASK);
	if (!dst)
		return -ENOMEM;

	ie_head = ntfs_ie_get_next(median);

	ies_end = (u8 *) ntfs_ie_get_end(&src->index);
	/* Tail size is bytes from entry after median to index end */
	tail_size = ies_end - (u8 *) ie_head;
	/* Copy tail as defined above to start of new index and update size. */
	memcpy(ntfs_ie_get_first(&dst->index), ie_head, tail_size);

	dst->index.index_length = cpu_to_le32(tail_size +
					      le32_to_cpu(dst->index.
							  entries_offset));
	err = ntfs_ib_write(icx, dst);

	ntfs_free(dst);
	return err;
}

/**
 * @return STATUS_OK (0) if ok or negative error code
 */
static int ntfs_ib_cut_tail(struct ntfs_index_context *icx,
			    struct INDEX_BLOCK *ib, struct INDEX_ENTRY *ie)
{
	char *ies_start, *ies_end;
	struct INDEX_ENTRY *ie_last;

	antfs_log_enter();

	ies_start = (char *)ntfs_ie_get_first(&ib->index);
	ies_end = (char *)ntfs_ie_get_end(&ib->index);

	ie_last = ntfs_ie_get_last((struct INDEX_ENTRY *) ies_start, ies_end);
	if (ie_last->ie_flags & INDEX_ENTRY_NODE)
		ntfs_ie_set_vcn(ie_last, ntfs_ie_get_vcn(ie));

	memcpy(ie, ie_last, le16_to_cpu(ie_last->length));

	ib->index.index_length = cpu_to_le32(((char *)ie - ies_start) +
					     le16_to_cpu(ie->length) +
					     le32_to_cpu(ib->index.
							 entries_offset));

	return ntfs_ib_write(icx, ib);
}

/**
 * @return 0 if ok or negative error code
 */
static int ntfs_ia_add(struct ntfs_index_context *icx)
{
	int err;

	antfs_log_enter();

	err = ntfs_ibm_add(icx);
	if (err != 0) {
		antfs_log_leave("ntfs_ibm_add: %d", err);
		return err;
	}

	if (!ntfs_attr_exist
	    (icx->ni, AT_INDEX_ALLOCATION, icx->name, icx->name_len)) {
		err = ntfs_attr_add(icx->ni, AT_INDEX_ALLOCATION, icx->name,
				    icx->name_len, NULL, 0);
		if (err != 0) {
			antfs_log_error("Failed to add AT_INDEX_ALLOCATION");
			return err;
		}
	}

	icx->ia_na = ntfs_ia_open(icx, icx->ni);
	if (IS_ERR(icx->ia_na)) {
		err = PTR_ERR(icx->ia_na);
		icx->ia_na = NULL;
		antfs_log_leave("err: %d", err);
		return err;
	}

	antfs_log_leave("ok");
	return 0;
}

/**
 * @return 0 if ok or negative error code
 */
static int ntfs_ir_reparent(struct ntfs_index_context *icx)
{
	struct ntfs_attr_search_ctx *ctx = NULL;
	struct INDEX_ROOT *ir;
	struct INDEX_ENTRY *ie;
	struct INDEX_BLOCK *ib = NULL;
	VCN new_ib_vcn;
	int ix_root_size;
	int err;

	antfs_log_enter();

	ir = ntfs_ir_lookup2(icx->ni, icx->name, icx->name_len);
	if (IS_ERR(ir)) {
		err = PTR_ERR(ir);
		goto out;
	}

	if (((ir->index.ih_flags & NODE_MASK) == SMALL_INDEX)) {
		err = ntfs_ia_add(icx);
		if (err != 0)
			goto out;
	}

	err = ntfs_ibm_get_free(icx, &new_ib_vcn);
	if (err)
		goto out;

	ir = ntfs_ir_lookup2(icx->ni, icx->name, icx->name_len);
	if (IS_ERR(ir)) {
		err = PTR_ERR(ir);
		goto clear_bmp;
	}

	ib = ntfs_ir_to_ib(ir, new_ib_vcn);
	if (ib == NULL) {
		antfs_log_error("Failed to move index root to index block");
		err = -ENOMEM;
		goto clear_bmp;
	}

	err = ntfs_ib_write(icx, ib);
	if (err != 0)
		goto clear_bmp;

retry:
	ir = ntfs_ir_lookup(icx->ni, icx->name, icx->name_len, &ctx);
	if (IS_ERR(ir)) {
		err = PTR_ERR(ir);
		goto clear_bmp;
	}

	ntfs_ir_nill(ir);

	ie = ntfs_ie_get_first(&ir->index);
	ie->ie_flags |= INDEX_ENTRY_NODE;
	ie->length =
	    const_cpu_to_le16(sizeof(struct INDEX_ENTRY_HEADER) + sizeof(VCN));

	ir->index.ih_flags = LARGE_INDEX;
	ir->index.index_length =
	    cpu_to_le32(le32_to_cpu(ir->index.entries_offset)
			+ le16_to_cpu(ie->length));
	ir->index.allocated_size = ir->index.index_length;
	ix_root_size = sizeof(struct INDEX_ROOT) - sizeof(struct INDEX_HEADER)
	    + le32_to_cpu(ir->index.allocated_size);

	err = ntfs_resident_attr_value_resize(ctx->mrec, ctx->attr,
					      ix_root_size);
	/* synchronize sizes to na */
	if (!err) {
		struct ntfs_attr *na = ANTFS_NA(ctx->ntfs_ino);
		na->data_size = na->initialized_size = ix_root_size;
		na->allocated_size = (ix_root_size + 7) & ~7;
	} else {
		/*
		 * When there is no space to build a non-resident
		 * index, we may have to move the root to an extent
		 */
		/* We either error out or retry so: Always make err hold
		 * a negative error value. */
		err = -ENOSPC;
		if ((ctx->al_entry || !ntfs_inode_add_attrlist(icx->ni))) {
			ntfs_attr_put_search_ctx(ctx);
			ctx = NULL;
			ir = ntfs_ir_lookup(icx->ni, icx->name, icx->name_len,
					    &ctx);
			if (!IS_ERR(ir)) {
				err = ntfs_attr_record_move_away(ctx,
					ix_root_size -
					le32_to_cpu(ctx->attr->value_length));
				if (!err) {
					ntfs_attr_put_search_ctx(ctx);
					ctx = NULL;
					goto retry;
				}
			}
		}
		/* FIXME: revert index root */
		antfs_log_error("Failed to resize resident attr: Rollback.");
		goto clear_bmp;
	}
	/*
	 *  FIXME: do it earlier if we have enough space in IR (should always),
	 *  so in error case we wouldn't lose the IB.
	 */
	ntfs_ie_set_vcn(ie, new_ib_vcn);

	err = STATUS_OK;
err_out:
	ntfs_free(ib);
	ntfs_attr_put_search_ctx(ctx);
out:
	antfs_log_leave("err: %d", err);
	return err;
clear_bmp:
	ntfs_ibm_clear(icx, new_ib_vcn);
	goto err_out;
}

/**
 * ntfs_ir_truncate - Truncate index root attribute
 *
 * @return STATUS_OK, STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT or other
 *         negative error code (can be checked with IS_STATUS_ERROR macro)
 */
static int ntfs_ir_truncate(struct ntfs_index_context *icx, int data_size)
{
	struct ntfs_attr *na;
	int err;

	antfs_log_enter("data_size: %d", data_size);

	na = ANTFS_NA(icx->ni);
	/*
	 *  INDEX_ROOT must be resident and its entries can be moved to
	 *  INDEX_BLOCK, so ENOSPC isn't a real error.
	 */
	err = ntfs_attr_truncate(na, data_size +
				 offsetof(struct INDEX_ROOT, index));
	if (err == STATUS_OK) {
		icx->ir = ntfs_ir_lookup2(icx->ni, icx->name, icx->name_len);
		if (IS_ERR(icx->ir)) {
			err = PTR_ERR(icx->ir);
			icx->ir = NULL;
			return err;
		}

		icx->ir->index.allocated_size = cpu_to_le32(data_size);

	} else if (IS_STATUS_ERROR(err))
		antfs_log_error("Failed to truncate INDEX_ROOT: %d", err);

	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_ir_make_space - Make more space for the index root attribute
 *
 * @return STATUS_OK or STATUS_KEEP_SEARCHING on success or other negative
 *         error on failure (check with IS_STATUS_ERROR macro)
 */
static int ntfs_ir_make_space(struct ntfs_index_context *icx, int data_size)
{
	int err;

	antfs_log_enter("data_size: %d", data_size);

	err = ntfs_ir_truncate(icx, data_size);
	if (err == STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT) {
		/* Root filled up: Grow the tree... */
		err = ntfs_ir_reparent(icx);
		if (err == STATUS_OK)
			err = STATUS_KEEP_SEARCHING;
		else
			antfs_log_error("Failed to nodify INDEX_ROOT: %d", err);
	}

	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * @note 'ie' must be a copy of a real index entry.
 *
 * @return STATUS_OK (0) if ok or negative error code
 */
static int ntfs_ie_add_vcn(struct INDEX_ENTRY **ie)
{
	struct INDEX_ENTRY *p, *old = *ie;

	old->length = cpu_to_le16(le16_to_cpu(old->length) + sizeof(VCN));
	p = ntfs_realloc(old, le16_to_cpu(old->length));
	if (!p)
		return -ENOMEM;

	p->ie_flags |= INDEX_ENTRY_NODE;
	*ie = p;

	return STATUS_OK;
}

/**
 * @brief Insert Copy of index entry with new VCN at pos in ih
 *
 * Insert @orig_ie (a copy of it) at @pos and set the VCN
 * for the original index entry at @pos to @new_vcn, the new
 * inserted entry gets the VCN of the "old" index entry.
 *
 * @return STATUS_OK (0) if ok or negative error code
 */
static int ntfs_ih_insert(struct INDEX_HEADER *ih, struct INDEX_ENTRY *orig_ie,
			  VCN new_vcn, int pos)
{
	struct INDEX_ENTRY *ie_node, *ie;
	int err;
	VCN old_vcn;

	antfs_log_enter();

	ie = ntfs_ie_dup(orig_ie);
	if (!ie) {
		antfs_log_error("ntfs_io_dup failed");
		return -ENOMEM;
	}

	if (!(ie->ie_flags & INDEX_ENTRY_NODE)) {
		err = ntfs_ie_add_vcn(&ie);
		if (err != 0) {
			antfs_log_error("ntfs_ie_add_vcn failed: %d", err);
			goto out;
		}
	}

	ie_node = ntfs_ie_get_by_pos(ih, pos);
	old_vcn = ntfs_ie_get_vcn(ie_node);
	ntfs_ie_set_vcn(ie_node, new_vcn);

	ntfs_ie_insert(ih, ie, ie_node);
	ntfs_ie_set_vcn(ie_node, old_vcn);
	err = STATUS_OK;
out:
	ntfs_free(ie);

	return err;
}

static VCN ntfs_icx_parent_vcn(struct ntfs_index_context *icx)
{
	return icx->parent_vcn[icx->pindex];
}

static VCN ntfs_icx_parent_pos(struct ntfs_index_context *icx)
{
	return icx->parent_pos[icx->pindex];
}

/**
 * @brief Insert @median into index root
 *
 * Expands the index root and inserts @median entry with @new_vcn
 * into index root at parent_pos(?)
 *
 * @return STATUS_OK (0) if ok or negative error code
 */
static int ntfs_ir_insert_median(struct ntfs_index_context *icx,
				 struct INDEX_ENTRY *median, VCN new_vcn)
{
	u32 new_size;
	int err;

	antfs_log_enter("Entering for new_vcn: %lld", new_vcn);

	icx->ir = ntfs_ir_lookup2(icx->ni, icx->name, icx->name_len);
	if (IS_ERR(icx->ir)) {
		err = PTR_ERR(icx->ir);
		icx->ir = NULL;
		return err;
	}

	new_size = le32_to_cpu(icx->ir->index.index_length) +
	    le16_to_cpu(median->length);
	if (!(median->ie_flags & INDEX_ENTRY_NODE))
		new_size += sizeof(VCN);

	err = ntfs_ir_make_space(icx, new_size);
	if (err != STATUS_OK)
		return err;

	icx->ir = ntfs_ir_lookup2(icx->ni, icx->name, icx->name_len);
	if (IS_ERR(icx->ir)) {
		err = PTR_ERR(icx->ir);
		icx->ir = NULL;
		return err;
	}

	return ntfs_ih_insert(&icx->ir->index, median, new_vcn,
			      ntfs_icx_parent_pos(icx));
}

static int ntfs_ib_split(struct ntfs_index_context *icx,
			 struct INDEX_BLOCK *ib);

/**
 * @return STATUS_OK or STATUS_KEEP_SEARCHING on success or other negative
 *         error code on failure.
 */
static int ntfs_ib_insert(struct ntfs_index_context *icx,
			  struct INDEX_ENTRY *ie, VCN new_vcn)
{
	struct INDEX_BLOCK *ib;
	u32 idx_size, allocated_size;
	int err;
	VCN old_vcn;

	antfs_log_enter("Entering for new_vcn %lld", new_vcn);

	ib = ntfs_malloc(1 << icx->block_size_bits);
	if (!ib) {
		antfs_log_error("ntfs_malloc failed");
		return -ENOMEM;
	}

	old_vcn = ntfs_icx_parent_vcn(icx);

	err = ntfs_ib_read(icx, old_vcn, ib);
	if (err != 0) {
		antfs_log_error("ntfs_ib_read failed: %d", err);
		goto err_out;
	}

	idx_size = le32_to_cpu(ib->index.index_length);
	allocated_size = le32_to_cpu(ib->index.allocated_size);
	/* FIXME: sizeof(VCN) should be included only if ie has no VCN */
	if (idx_size + le16_to_cpu(ie->length) + sizeof(VCN) > allocated_size) {
		err = ntfs_ib_split(icx, ib);
		if (err == STATUS_OK) {
			err = STATUS_KEEP_SEARCHING;
		} else {
			antfs_log_debug("ntfs_ib_split failed: %d", err);
		}
		/* This is most likely no real error too. */
		goto err_out;
	}

	err = ntfs_ih_insert(&ib->index, ie, new_vcn, ntfs_icx_parent_pos(icx));
	if (err != 0) {
		antfs_log_error("ntfs_ih_insert failed: %d", err);
		goto err_out;
	}

	err = ntfs_ib_write(icx, ib);
	if (err != 0) {
		antfs_log_error("ntfs_ib_write failed: %d", err);
		goto err_out;
	}

	err = STATUS_OK;
err_out:
	ntfs_free(ib);
	return err;
}

/**
 * @brief ntfs_ib_split - Split an index block
 *
 * This splits an index block in half by allocating a new
 * index block and copying half of the source index block
 * to the new one.
 *
 * @return STATUS_OK or STATUS_KEEP_SEARCHING on success or
 *         other negative error code on failure
 */
static int ntfs_ib_split(struct ntfs_index_context *icx, struct INDEX_BLOCK *ib)
{
	struct INDEX_ENTRY *median;
	VCN new_vcn;
	int err;

	antfs_log_enter();

	err = ntfs_icx_parent_dec(icx);
	if (err != 0)
		return err;

	median = ntfs_ie_get_median(&ib->index);
	err = ntfs_ibm_get_free(icx, &new_vcn);
	if (err)
		return err;

	err = ntfs_ib_copy_tail(icx, ib, median, new_vcn);
	if (err != 0) {
		antfs_log_error("copy_tail failed");
		ntfs_ibm_clear(icx, new_vcn);
		return err;
	}

	if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT)
		err = ntfs_ir_insert_median(icx, median, new_vcn);
	else
		err = ntfs_ib_insert(icx, median, new_vcn);

	if (err != STATUS_OK) {
		/*---
		antfs_log_error("err=%d; Rollback for vcn %lld", err, new_vcn);
		---*/
		/* This is MOST LIKELY no real error. This is ok. */
		ntfs_ibm_clear(icx, new_vcn);
		return err;
	}

	return ntfs_ib_cut_tail(icx, ib, median);
}

/* JPA static */
/**
 * @return STATUS_OK (0) or negative error code
 */
int ntfs_ie_add(struct ntfs_index_context *icx, struct INDEX_ENTRY *ie)
{
	struct INDEX_HEADER *ih;
	int allocated_size, new_size;
	int err;

#ifdef DEBUG
/* removed by JPA to make function usable for security indexes
	char *fn;
	fn = ntfs_ie_filename_get(ie);
	antfs_log_debug("file: '%s'", fn);
	ntfs_attr_name_free(&fn);
*/
#endif

	antfs_log_enter();
	while (1) {

		/* This walks the index tree to some leaf and either finds
		 * our index entry ... or a spot where we can attach it. */
		err =
		    ntfs_index_lookup(&ie->key, le16_to_cpu(ie->key_length),
				      icx);
		if (!err) {
			err = -EEXIST;
			antfs_log_error("Index already have such entry");
			goto err_out;
		}
		if (err != -ENOENT) {
			antfs_log_error("Failed to find place for new entry");
			goto err_out;
		}

		if (icx->is_in_root)
			ih = &icx->ir->index;
		else
			ih = &icx->ib->index;

		allocated_size = le32_to_cpu(ih->allocated_size);
		new_size =
		    le32_to_cpu(ih->index_length) + le16_to_cpu(ie->length);

		if (new_size <= allocated_size)
			break;

		antfs_log_debug
		    ("index block sizes: allocated: %d  needed: %d",
		     allocated_size, new_size);

		if (icx->is_in_root) {
			err = ntfs_ir_make_space(icx, new_size);
			if (IS_STATUS_ERROR(err))
				goto err_out;
		} else {
			err = ntfs_ib_split(icx, icx->ib);
			if (IS_STATUS_ERROR(err))
				goto err_out;
		}

		ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
		ntfs_index_ctx_reinit(icx);
	}

	ntfs_ie_insert(ih, ie, icx->entry);
	ntfs_index_entry_mark_dirty(icx);

	err = STATUS_OK;
err_out:
	antfs_log_leave("%s (err=%d)", err ? "Failed" : "Done", err);
	return err;
}

/**
 * ntfs_index_add_filename - add filename to directory index
 *
 * @param ni    ntfs inode describing directory to which index add filename
 * @param fn    FILE_NAME attribute to add
 * @param mref  reference of the inode which @fn describes
 *
 * @return 0 on success or negative error code
 */
int ntfs_index_add_filename(struct ntfs_inode *ni, struct FILE_NAME_ATTR *fn,
			    MFT_REF mref)
{
	struct INDEX_ENTRY *ie;
	struct ntfs_index_context *icx;
	int fn_size, ie_size, err;

	antfs_log_enter("mref: %llu", (long long)MREF(mref));

	if (IS_ERR_OR_NULL(ni) || !fn) {
		antfs_log_error("Invalid arguments.");
		return -EINVAL;
	}

	fn_size = (fn->file_name_length * sizeof(ntfschar)) +
	    sizeof(struct FILE_NAME_ATTR);
	ie_size = (sizeof(struct INDEX_ENTRY_HEADER) + fn_size + 7) & ~7;

	ie = ntfs_calloc(ie_size);
	if (!ie) {
		antfs_log_error("oom");
		return -ENOMEM;
	}

	ie->indexed_file = cpu_to_le64(mref);
	ie->length = cpu_to_le16(ie_size);
	ie->key_length = cpu_to_le16(fn_size);
	memcpy(&ie->key, fn, fn_size);

	icx = ntfs_index_ctx_get(ni, NTFS_INDEX_I30, 4);
	if (IS_ERR(icx)) {
		err = PTR_ERR(icx);
		goto out;
	}

	err = ntfs_ie_add(icx, ie);
	ntfs_index_ctx_put(icx);
out:
	ntfs_free(ie);
	return err;
}

/**
 * @return STATUS_OK (0) or negative error code
 */
static int ntfs_ih_takeout(struct ntfs_index_context *icx,
			   struct INDEX_HEADER *ih, struct INDEX_ENTRY *ie,
			   struct INDEX_BLOCK *ib)
{
	struct INDEX_ENTRY *ie_roam;
	int err;

	antfs_log_enter();

	ie_roam = ntfs_ie_dup_novcn(ie);
	if (!ie_roam)
		return -ENOMEM;

	ntfs_ie_delete(ih, ie);

	if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT) {
		ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
	} else {
		err = ntfs_ib_write(icx, ib);
		if (err != 0)
			goto out;
	}

	ntfs_index_ctx_reinit(icx);

	err = ntfs_ie_add(icx, ie_roam);
out:
	ntfs_free(ie_roam);
	return err;
}

/**
 *  Used if an empty index block to be deleted has END entry as the parent
 *  in the INDEX_ROOT which is the only one there.
 */
static void ntfs_ir_leafify(struct ntfs_index_context *icx,
			    struct INDEX_HEADER *ih)
{
	struct INDEX_ENTRY *ie;

	antfs_log_enter();

	ie = ntfs_ie_get_first(ih);
	ie->ie_flags &= ~INDEX_ENTRY_NODE;
	ie->length = cpu_to_le16(le16_to_cpu(ie->length) - sizeof(VCN));

	ih->index_length =
	    cpu_to_le32(le32_to_cpu(ih->index_length) - sizeof(VCN));
	ih->ih_flags &= ~LARGE_INDEX;

	/* Not fatal error */
	ntfs_ir_truncate(icx, le32_to_cpu(ih->index_length));
}

/**
 *  Used if an empty index block to be deleted has END entry as the parent
 *  in the INDEX_ROOT which is not the only one there.
 *
 * @return STATUS_OK (0) or negative error code
 */
static int ntfs_ih_reparent_end(struct ntfs_index_context *icx,
				struct INDEX_HEADER *ih, struct INDEX_BLOCK *ib)
{
	struct INDEX_ENTRY *ie, *ie_prev;

	antfs_log_enter();

	ie = ntfs_ie_get_by_pos(ih, ntfs_icx_parent_pos(icx));
	ie_prev = ntfs_ie_prev(ih, ie);

	if (!ie_prev)
		return -EINVAL;

	ntfs_ie_set_vcn(ie, ntfs_ie_get_vcn(ie_prev));

	return ntfs_ih_takeout(icx, ih, ie_prev, ib);
}

/**
 * @return STATUS_OK (0) or negative error code
 */
static int ntfs_index_rm_leaf(struct ntfs_index_context *icx)
{
	struct INDEX_BLOCK *ib = NULL;
	struct INDEX_HEADER *parent_ih;
	struct INDEX_ENTRY *ie;
	int err;

	antfs_log_debug("pindex: %d", icx->pindex);

	err = ntfs_icx_parent_dec(icx);
	if (err != 0)
		return err;

	err = ntfs_ibm_clear(icx, icx->parent_vcn[icx->pindex + 1]);
	if (err != 0)
		return err;

	if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT)
		parent_ih = &icx->ir->index;
	else {
		ib = ntfs_malloc(1 << icx->block_size_bits);
		if (!ib)
			return -ENOMEM;

		err = ntfs_ib_read(icx, ntfs_icx_parent_vcn(icx), ib);
		if (err != 0)
			goto out;

		parent_ih = &ib->index;
	}

	ie = ntfs_ie_get_by_pos(parent_ih, ntfs_icx_parent_pos(icx));
	if (!ntfs_ie_end(ie)) {
		err = ntfs_ih_takeout(icx, parent_ih, ie, ib);
		goto out;
	}

	if (ntfs_ih_zero_entry(parent_ih)) {

		if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT) {
			ntfs_ir_leafify(icx, parent_ih);
			goto ok;
		}

		err = ntfs_index_rm_leaf(icx);
		goto out;
	}

	err = ntfs_ih_reparent_end(icx, parent_ih, ib);
	if (err != 0)
		goto out;
ok:
	err = STATUS_OK;
out:
	ntfs_free(ib);
	return err;
}

/**
 * @return STATUS_OK (0) or negative error code
 */
static int ntfs_index_rm_node(struct ntfs_index_context *icx)
{
	int entry_pos, pindex;
	VCN vcn;
	struct INDEX_BLOCK *ib = NULL;
	struct INDEX_ENTRY *ie_succ, *ie, *entry = icx->entry;
	struct INDEX_HEADER *ih;
	u32 new_size;
	int delta, err;

	antfs_log_enter();

	if (IS_ERR_OR_NULL(icx->ia_na)) {
		icx->ia_na = ntfs_ia_open(icx, icx->ni);
		if (IS_ERR(icx->ia_na)) {
			err = PTR_ERR(icx->ia_na);
			icx->ia_na = NULL;
			return err;
		}
	}

	ib = ntfs_malloc(1 << icx->block_size_bits);
	if (!ib)
		return -ENOMEM;

	ie_succ = ntfs_ie_get_next(icx->entry);
	entry_pos = icx->parent_pos[icx->pindex]++;
	pindex = icx->pindex;
descend:
	vcn = ntfs_ie_get_vcn(ie_succ);
	err = ntfs_ib_read(icx, vcn, ib);
	if (err != 0)
		goto out;

	ie_succ = ntfs_ie_get_first(&ib->index);

	err = ntfs_icx_parent_inc(icx);
	if (err != 0)
		goto out;

	icx->parent_vcn[icx->pindex] = vcn;
	icx->parent_pos[icx->pindex] = 0;

	if ((ib->index.ih_flags & NODE_MASK) == INDEX_NODE)
		goto descend;

	if (ntfs_ih_zero_entry(&ib->index)) {
		err = -EIO;
		antfs_log_error("Empty index block");
		goto out;
	}

	ie = ntfs_ie_dup(ie_succ);
	if (!ie) {
		err = -ENOMEM;
		goto out;
	}

	err = ntfs_ie_add_vcn(&ie);
	if (err != 0)
		goto out2;

	ntfs_ie_set_vcn(ie, ntfs_ie_get_vcn(icx->entry));

	if (icx->is_in_root)
		ih = &icx->ir->index;
	else
		ih = &icx->ib->index;

	delta = le16_to_cpu(ie->length) - le16_to_cpu(icx->entry->length);
	new_size = le32_to_cpu(ih->index_length) + delta;
	if (delta > 0) {
		if (icx->is_in_root) {
			err = ntfs_ir_make_space(icx, new_size);
			if (err != STATUS_OK)
				goto out2;

			ih = &icx->ir->index;
			entry = ntfs_ie_get_by_pos(ih, entry_pos);

		} else if (new_size > le32_to_cpu(ih->allocated_size)) {
			icx->pindex = pindex;
			err = ntfs_ib_split(icx, icx->ib);
			if (err == STATUS_OK)
				err = STATUS_KEEP_SEARCHING;
			goto out2;
		}
	}

	ntfs_ie_delete(ih, entry);
	ntfs_ie_insert(ih, ie, entry);

	if (icx->is_in_root) {
		err = ntfs_ir_truncate(icx, new_size);
		if (err != 0)
			goto out2;
	} else {
		err = ntfs_icx_ib_write(icx);
		if (err != 0)
			goto out2;
	}

	ntfs_ie_delete(&ib->index, ie_succ);

	if (ntfs_ih_zero_entry(&ib->index)) {
		err = ntfs_index_rm_leaf(icx);
		if (err != 0)
			goto out2;
	} else {
		err = ntfs_ib_write(icx, ib);
		if (err != 0)
			goto out2;
	}

	err = STATUS_OK;
out2:
	ntfs_free(ie);
out:
	ntfs_free(ib);
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_index_rm - remove entry from the index
 *
 * @param icx  index context describing entry to delete
 *
 * Delete entry described by @icx from the index. Index context is always
 * reinitialized after use of this function, so it can be used for index
 * lookup once again.
 *
 * @return 0 on success or negative error code
 */
/*static JPA*/
int ntfs_index_rm(struct ntfs_index_context *icx)
{
	struct INDEX_HEADER *ih;
	int err = STATUS_OK;

	antfs_log_enter();

	if (!icx || (!icx->ib && !icx->ir) || ntfs_ie_end(icx->entry)) {
		antfs_log_error("Invalid arguments.");
		err = -EINVAL;
		goto out;
	}
	if (icx->is_in_root)
		ih = &icx->ir->index;
	else
		ih = &icx->ib->index;

	if (icx->entry->ie_flags & INDEX_ENTRY_NODE) {

		err = ntfs_index_rm_node(icx);

	} else if (icx->is_in_root || !ntfs_ih_one_entry(ih)) {

		ntfs_ie_delete(ih, icx->entry);

		if (icx->is_in_root) {
			err =
			    ntfs_ir_truncate(icx,
					     le32_to_cpu(ih->index_length));
			if (err != STATUS_OK)
				goto out;
		} else {
			err = ntfs_icx_ib_write(icx);
			if (err != 0)
				goto out;
		}
	} else {
		err = ntfs_index_rm_leaf(icx);
		if (err != 0)
			goto out;
	}
out:
	antfs_log_leave();
	return err;
}

/**
 * @return 0 on success or negative error code
 */
int ntfs_index_remove(struct ntfs_inode *dir_ni,
		      struct ntfs_inode *ni __attribute__ ((unused)),
		      const void *key, const int keylen)
{
	int err;
	struct ntfs_index_context *icx;

	antfs_log_enter();
	icx = ntfs_index_ctx_get(dir_ni, NTFS_INDEX_I30, 4);
	if (IS_ERR(icx))
		return PTR_ERR(icx);

	while (1) {

		err = ntfs_index_lookup(key, keylen, icx);
		if (err != 0)
			goto err_out;
		/* TODO: Use ni to also compare mref? */

		err = ntfs_index_rm(icx);
		if (err == STATUS_OK)
			break;
		else if (err != STATUS_KEEP_SEARCHING)
			goto err_out;

		ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
		ntfs_index_ctx_reinit(icx);
	}

	ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
	/* TODO this is only a hot-fix this should be set when we definitely
	 * know that the B+tree changed!
	 */
	NInoAttrSetIndexModified(dir_ni);
out:
	ntfs_index_ctx_put(icx);
	antfs_log_leave();
	return err;
err_out:
	antfs_log_error("Delete failed (%d)", err);
	goto out;
}

/*
 *		Walk down the index tree (leaf bound)
 *	until there are no subnode in the first index entry
 *	returns the entry at the bottom left in subnode
 *
 * @return index entry at bottom left in subnode (may be NULL!)
 */
static struct INDEX_ENTRY *ntfs_index_walk_down(struct INDEX_ENTRY *ie,
					 struct ntfs_index_context *ictx)
{
	struct INDEX_ENTRY *entry;
	s64 vcn;
	int err;

	entry = ie;
	do {
		vcn = ntfs_ie_get_vcn(entry);
		if (ictx->is_in_root) {

			/* down from level zero */

			ictx->ir = NULL;
			ictx->ib = (struct INDEX_BLOCK *)
				ntfs_malloc(1 << ictx->block_size_bits);
			if (!ictx->ib)
				return NULL;
			ictx->pindex = 1;
			ictx->is_in_root = FALSE;
		} else {

			/* down from non-zero level */

			ictx->pindex++;
		}
		ictx->parent_pos[ictx->pindex] = 0;
		ictx->parent_vcn[ictx->pindex] = vcn;
		err = ntfs_ib_read(ictx, vcn, ictx->ib);
		if (!err) {
			ictx->entry = ntfs_ie_get_first(&ictx->ib->index);
			entry = ictx->entry;
		} else {
			entry = NULL;
		}
	} while (entry && (entry->ie_flags & INDEX_ENTRY_NODE));
	return entry;
}

/*
 *		Walk up the index tree (root bound)
 *	until there is a valid data entry in parent
 *	returns the parent entry or NULL if no more parent
 *
 * @return parent entry or NULL
 */
static struct INDEX_ENTRY *ntfs_index_walk_up(struct INDEX_ENTRY *ie,
				       struct ntfs_index_context *ictx)
{
	struct INDEX_ENTRY *entry;
	s64 vcn;

	entry = ie;
	if (ictx->pindex > 0) {
		do {
			ictx->pindex--;
			if (!ictx->pindex) {

				/* we have reached the root */

				ntfs_free(ictx->ib);
				ictx->ib = NULL;
				ictx->is_in_root = TRUE;
				/* a new search context is to be allocated */
				if (ictx->actx)
					ntfs_free(ictx->actx);
				ictx->ir = ntfs_ir_lookup(ictx->ni,
							  ictx->name,
							  ictx->name_len,
							  &ictx->actx);
				if (!IS_ERR(ictx->ir))
					entry =
					    ntfs_ie_get_by_pos(&ictx->ir->index,
						ictx->parent_pos[ictx->pindex]);
				else {
					entry = NULL;
				}
			} else {
				/* up into non-root node */
				vcn = ictx->parent_vcn[ictx->pindex];
				if (!ntfs_ib_read(ictx, vcn, ictx->ib)) {
					entry =
					    ntfs_ie_get_by_pos(&ictx->ib->index,
						ictx->parent_pos[ictx->pindex]);
				} else
					entry = NULL;
			}
			ictx->entry = entry;
		} while (entry && (ictx->pindex > 0)
			 && (entry->ie_flags & INDEX_ENTRY_END));
	} else
		entry = NULL;
	return entry;
}

/*
 *		Get next entry in an index according to collating sequence.
 *	Must be initialized through a ntfs_index_lookup()
 *
 *	Returns next entry or NULL if none
 *
 *	Sample layout :
 *
 *                 +---+---+---+---+---+---+---+---+    n ptrs to subnodes
 *                 |   |   | 10| 25| 33|   |   |   |    n-1 keys in between
 *                 +---+---+---+---+---+---+---+---+    no key in last entry
 *                              | A | A
 *                              | | | +-------------------------------+
 *   +--------------------------+ | +-----+                           |
 *   |                            +--+    |                           |
 *   V                               |    V                           |
 * +---+---+---+---+---+---+---+---+ |  +---+---+---+---+---+---+---+---+
 * | 11| 12| 13| 14| 15| 16| 17|   | |  | 26| 27| 28| 29| 30| 31| 32|   |
 * +---+---+---+---+---+---+---+---+ |  +---+---+---+---+---+---+---+---+
 *                               |   |
 *       +-----------------------+   |
 *       |                           |
 *     +---+---+---+---+---+---+---+---+
 *     | 18| 19| 20| 21| 22| 23| 24|   |
 *     +---+---+---+---+---+---+---+---+
 */
struct INDEX_ENTRY *ntfs_index_next(struct INDEX_ENTRY *ie,
				    struct ntfs_index_context *ictx)
{
	struct INDEX_ENTRY *next;
	le16 flags;

	/*
	 * lookup() may have returned an invalid node
	 * when searching for a partial key
	 * if this happens, walk up
	 */

	if (ie->ie_flags & INDEX_ENTRY_END)
		next = ntfs_index_walk_up(ie, ictx);
	else {
		/*
		 * get next entry in same node
		 * there is always one after any entry with data
		 */

		next = (struct INDEX_ENTRY *) ((char *)ie +
			le16_to_cpu(ie->length));
		++ictx->parent_pos[ictx->pindex];
		flags = next->ie_flags;

		/* walk down if it has a subnode */

		if (flags & INDEX_ENTRY_NODE) {
			next = ntfs_index_walk_down(next, ictx);
		} else {

			/* walk up it has no subnode, nor data */

			if (flags & INDEX_ENTRY_END)
				next = ntfs_index_walk_up(next, ictx);
		}
	}
	/* return NULL if stuck at end of a block */

	if (next && (next->ie_flags & INDEX_ENTRY_END))
		next = NULL;
	return next;
}
