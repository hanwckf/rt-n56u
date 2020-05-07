/**
 * attrib.c - Attribute handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2010 Anton Altaparmakov
 * Copyright (c) 2002-2005 Richard Russon
 * Copyright (c) 2002-2008 Szabolcs Szakacsits
 * Copyright (c) 2004-2007 Yura Pakhuchiy
 * Copyright (c) 2007-2015 Jean-Pierre Andre
 * Copyright (c) 2010      Erik Larsson
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

#include "antfs.h"
#include "param.h"
#include "attrib.h"
#include "attrlist.h"
#include "device.h"
#include "mft.h"
#include "debug.h"
#include "mst.h"
#include "volume.h"
#include "types.h"
#include "layout.h"
#include "runlist.h"
#include "lcnalloc.h"
#include "dir.h"
#include "compress.h"
#include "bitmap.h"
#include "misc.h"

ntfschar AT_UNNAMED[] = { const_cpu_to_le16('\0') };

ntfschar STREAM_SDS[] = { const_cpu_to_le16('$'),
	const_cpu_to_le16('S'),
	const_cpu_to_le16('D'),
	const_cpu_to_le16('S'),
	const_cpu_to_le16('\0')
};

ntfschar TXF_DATA[] = { const_cpu_to_le16('$'),
	const_cpu_to_le16('T'),
	const_cpu_to_le16('X'),
	const_cpu_to_le16('F'),
	const_cpu_to_le16('_'),
	const_cpu_to_le16('D'),
	const_cpu_to_le16('A'),
	const_cpu_to_le16('T'),
	const_cpu_to_le16('A'),
	const_cpu_to_le16('\0')
};

static bool NAttrFlag(struct ntfs_attr *na, enum FILE_ATTR_FLAGS flag)
{
	if (na->type == AT_DATA && na->name == AT_UNNAMED)
		return na->ni->flags & flag ? true : false;
	return false;
}

static void NAttrSetFlag(struct ntfs_attr *na, enum FILE_ATTR_FLAGS flag)
{
	if (na->type == AT_DATA && na->name == AT_UNNAMED) {
		na->ni->flags |= flag;
	} else {
		antfs_log_debug("Denied setting flag %d for not unnamed data "
				"attribute", le32_to_cpu(flag));
	}
}

static void NAttrClearFlag(struct ntfs_attr *na, enum FILE_ATTR_FLAGS flag)
{
	if (na->type == AT_DATA && na->name == AT_UNNAMED)
		na->ni->flags &= ~flag;
}

#define GenNAttrIno(func_name, flag) \
int NAttr##func_name(struct ntfs_attr *na) { return NAttrFlag(na, flag); } \
void NAttrSet##func_name(struct ntfs_attr *na)	 { NAttrSetFlag(na, flag); } \
void NAttrClear##func_name(struct ntfs_attr *na) { NAttrClearFlag(na, flag); }

GenNAttrIno(Compressed, FILE_ATTR_COMPRESSED)
GenNAttrIno(Encrypted, FILE_ATTR_ENCRYPTED)
GenNAttrIno(Sparse, FILE_ATTR_SPARSE_FILE)

/**
 * ntfs_get_attribute_value_length - Find the length of an attribute
 * @a:
 *
 * Description...
 *
 * Returns:
 */
s64 ntfs_get_attribute_value_length(const struct ATTR_RECORD *a)
{
	if (!a)
		return -EINVAL;

	if (a->non_resident)
		return sle64_to_cpu(a->data_size);

	return (s64) le32_to_cpu(a->value_length);
}

/**
 * ntfs_get_attribute_value - Get a copy of an attribute
 * @vol:    ntfs volume of the partition the attribute resides in
 * @a:	    pointer to an attribute record which contains the desired value
 * @b:	    buffer to write the value into
 *
 * @return length of the attribute value in bytes, or errorcode if something
 *  went wrong or 0 if no value is given!
 *
 * ntfs_get_attribute_value writes the value of an attribute into given buffer
 * @b and returns the length of the value in bytes. The return value will be
 * used by caller to identify till where the actual value has been written into
 * the buffer @b. TODO this is really a short description -> more in depth plz!
 */
s64 ntfs_get_attribute_value(const struct ntfs_volume *vol,
			     const struct ATTR_RECORD *a, u8 *b)
{
	struct runlist_element *rl;
	s64 total, r, data_size, ret = 0;
	int i;

	/* Sanity checks. */
	if (!vol || !a || !b) {
		ret = -EINVAL;
		goto out;
	}
	/* Complex attribute? */
	/*
	 * Ignore the flags in case they are not zero for an attribute list
	 * attribute.  Windows does not complain about invalid flags and chkdsk
	 * does not detect or fix them so we need to cope with it, too.
	 */
	if (a->type != AT_ATTRIBUTE_LIST && a->flags) {
		antfs_log_error
		    ("Non-zero (%04x) attribute flags. Cannot handle "
		     "this yet.", le16_to_cpu(a->flags));
		ret = -EOPNOTSUPP;
		goto out;
	}
	if (!a->non_resident) {
		/* Attribute is resident. */

		/* Sanity check. */
		if (le32_to_cpu(a->value_length) + le16_to_cpu(a->value_offset)
		    > le32_to_cpu(a->length)) {
			/* - ret is zero - */
			goto out;
		}

		memcpy(b, (const char *)a + le16_to_cpu(a->value_offset),
		       le32_to_cpu(a->value_length));
		ret = (s64) le32_to_cpu(a->value_length);
		goto out;
	}

	/* Attribute is not resident. */

	/* If no data, return 0. */
	if (!(a->data_size)) {
		ret = 0;
		goto out;
	}
	/*
	 * FIXME: What about attribute lists?!? (AIA)
	 */
	/* Decompress the mapping pairs array into a runlist. */
	rl = ntfs_mapping_pairs_decompress(vol, a, NULL);
	if (IS_ERR(rl)) {
		ret = PTR_ERR(rl);
		goto out;
	}
	/*
	 * FIXED: We were overflowing here in a nasty fashion when we
	 * reach the last cluster in the runlist as the buffer will
	 * only be big enough to hold data_size bytes while we are
	 * reading in allocated_size bytes which is usually larger
	 * than data_size, since the actual data is unlikely to have a
	 * size equal to a multiple of the cluster size!
	 * FIXED2:  We were also overflowing here in the same fashion
	 * when the data_size was more than one run smaller than the
	 * allocated size which happens with Windows XP sometimes.
	 * FIXUP: The proposed temporary fix was rather ... rough.
	 * Duplicated a lot of code, needed an extra buffer ...
	 * FIXED3: Respect sparse runs (LCN_HOLE).
	 */
	data_size = sle64_to_cpu(a->data_size);
	/* Now load all clusters in the runlist into b. */
	for (i = 0, total = 0; rl[i].length; i++) {
		s64 to_read;

		/* Make sure we don't overflow. */
		to_read = rl[i].length << vol->cluster_size_bits;
		if (total + to_read > data_size)
			to_read = data_size - total;
		if (rl[i].lcn == LCN_HOLE) {
			memset(b + total, 0, to_read);
			r = to_read;
		} else {
			r = ntfs_pread(vol->dev,
					rl[i].lcn << vol->cluster_size_bits,
					to_read,
					b + total);
		}
		if (r != to_read) {
#define ESTR "Error reading attribute value"
			if (r < 0) {
				antfs_log_error(ESTR);
				ret = r;
			} else if (r < to_read) {
				antfs_log_debug(ESTR
						": Ran out of input data.");
				ret = -EIO;
			} else {
				antfs_log_debug(ESTR ": unknown error");
				ret = -EIO;
			}
#undef ESTR
			ntfs_free(rl);
			goto out;
		}
		total += r;
	}
	ntfs_free(rl);
	ret = total;
out:
	return ret;
}

/* Already cleaned up code below, but still look for FIXME:... */

/**
 * __ntfs_attr_init - primary initialization of an ntfs attribute structure
 * @na:		ntfs attribute to initialize
 * @ni:		ntfs inode with which to initialize the ntfs attribute
 * @type:	attribute type
 * @name:	attribute name in little endian Unicode or NULL
 * @name_len:	length of attribute @name in Unicode characters (if @name given)
 *
 * Initialize the ntfs attribute @na with @ni, @type, @name, and @name_len.
 */
static void __ntfs_attr_init(struct ntfs_attr *na, struct ntfs_inode *ni,
			     const enum ATTR_TYPES type, ntfschar *name,
			     const u32 name_len)
{
	na->rl = NULL;
	na->ni = ni;
	na->type = type;
	na->name = name;
	if (name)
		na->name_len = name_len;
	else
		na->name_len = 0;
}

/**
 * ntfs_attr_init - initialize an ntfs_attr with data sizes and status
 * @na:
 * @non_resident:
 * @compressed:
 * @encrypted:
 * @sparse:
 * @allocated_size:
 * @data_size:
 * @initialized_size:
 * @compressed_size:
 * @compression_unit:
 *
 * Final initialization for an ntfs attribute.
 */
void ntfs_attr_init(struct ntfs_attr *na, const bool non_resident,
		    const enum ATTR_FLAGS data_flags,
		    const bool encrypted, const bool sparse,
		    const s64 allocated_size, const s64 data_size,
		    const s64 initialized_size, const s64 compressed_size,
		    const u8 compression_unit)
{
	if (!NAttrInitialized(na)) {
		na->data_flags = data_flags;
		if (non_resident)
			NAttrSetNonResident(na);
		if (data_flags & ATTR_COMPRESSION_MASK)
			NAttrSetCompressed(na);
		if (encrypted)
			NAttrSetEncrypted(na);
		if (sparse)
			NAttrSetSparse(na);
		na->allocated_size = allocated_size;
		na->data_size = data_size;
		na->initialized_size = initialized_size;
		if ((data_flags & ATTR_COMPRESSION_MASK) || sparse) {
			struct ntfs_volume *vol = na->ni->vol;

			na->compressed_size = compressed_size;
			na->compression_block_clusters = 1 << compression_unit;
			na->compression_block_size = 1 << (compression_unit +
							   vol->
							   cluster_size_bits);
			na->compression_block_size_bits =
			    ffs(na->compression_block_size) - 1;
		}
		NAttrSetInitialized(na);
	}
}

/**
 * ntfs_attr_open - open an ntfs attribute for access
 * @ni:		open ntfs inode in which the ntfs attribute resides
 * @type:	attribute type
 * @name:	attribute name in little endian Unicode or AT_UNNAMED or NULL
 * @name_len:	length of attribute @name in Unicode characters (if @name given)
 *
 * Allocate a new ntfs attribute structure, initialize it with @ni, @type,
 * @name, and @name_len, then return it. Return error code on error.
 *
 * If @name is AT_UNNAMED look specifically for an unnamed attribute.  If you
 * do not care whether the attribute is named or not set @name to NULL.  In
 * both those cases @name_len is not used at all.
 */
static int ntfs_attr_fill(struct ntfs_inode *ni, struct ntfs_attr *na,
		   const enum ATTR_TYPES type, ntfschar *name, u32 name_len)
{
	struct ntfs_attr_search_ctx *ctx;
	ntfschar *newname = NULL;
	struct ATTR_RECORD *a;
	le16 cs;
	int err = 0;

	antfs_log_enter("Entering for inode %lld, attr 0x%x.",
			(unsigned long long)ni->mft_no, le32_to_cpu(type));

	if (IS_ERR_OR_NULL(ni) || !ni->vol || !ni->mrec) {
		err = -EINVAL;
		antfs_log_error("Invalid arguments.");
		goto err_out;
	}
	if (name && name != AT_UNNAMED && name != NTFS_INDEX_I30) {
		name = ntfs_ucsndup(name, name_len);
		if (IS_ERR(name)) {
			antfs_log_error("Wrong attribute name!");
			err = -ENOMEM;
			goto err_out;
		}
		newname = name;
	}

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx)) {
		antfs_log_error("Failed to get search context");
		err = PTR_ERR(ctx);
		goto err_out;
	}

	err = ntfs_attr_lookup(type, name, name_len, 0, 0, NULL, 0, ctx);
	if (err) {
		if (err != -ENOENT) {
			antfs_log_error("Failed to lookup attribute 0x%x from"
					" ni(%lld)", le32_to_cpu(type),
					ni->mft_no);
		} else {
			antfs_log_debug("Attribute 0x%x from ni(%lld) does"
					" not exist!", le32_to_cpu(type),
					ni->mft_no);
		}
		goto err_out_put;
	}

	a = ctx->attr;

	if (!name) {
		if (a->name_length) {
			name =
			    ntfs_ucsndup((ntfschar *) ((u8 *) a +
					 le16_to_cpu(a->name_offset)),
					 a->name_length);
			if (IS_ERR(name)) {
				err = -ENOMEM;
				antfs_log_error("could not conver ucs to mbs");
				goto err_out_put;
			}
			newname = name;
			name_len = a->name_length;
		} else {
			name = AT_UNNAMED;
			name_len = 0;
		}
	}

	__ntfs_attr_init(na, ni, type, name, name_len);

	/*
	 * Wipe the flags in case they are not zero for an attribute list
	 * attribute.  Windows does not complain about invalid flags and chkdsk
	 * does not detect or fix them so we need to cope with it, too.
	 */
	if (type == AT_ATTRIBUTE_LIST)
		a->flags = const_cpu_to_le16(0);

	if ((type == AT_DATA)
	    && (a->non_resident ? !a->initialized_size : !a->value_length)) {
		/*
		 * Define/redefine the compression state if stream is
		 * empty, based on the compression mark on parent
		 * directory (for unnamed data streams) or on current
		 * inode (for named data streams). The compression mark
		 * may change any time, the compression state can only
		 * change when stream is wiped out.
		 *
		 * Also prevent compression on NTFS version < 3.0
		 * or cluster size > 4K or compression is disabled
		 */
		a->flags &= ~ATTR_COMPRESSION_MASK;
		if ((ni->flags & FILE_ATTR_COMPRESSED)
		    && (ni->vol->major_ver >= 3)
		    && NVolCompression(ni->vol)
		    && (ni->vol->cluster_size <= MAX_COMPRESSION_CLUSTER_SIZE))
			a->flags |= ATTR_IS_COMPRESSED;
	}

	cs = a->flags & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE);

	/* a file may be sparse though its unnamed data is not (cf $UsnJrnl) */
	if (na->type == AT_DATA && na->name == AT_UNNAMED &&
	    (((a->flags & ATTR_IS_SPARSE) && !NAttrSparse(na)) ||
	     (!(a->flags & ATTR_IS_ENCRYPTED) != !NAttrEncrypted(na)))) {
		err = -EIO;
		antfs_log_error("Inode %lld has corrupt attribute flags "
				"(0x%x <> 0x%x)",
				(unsigned long long)ni->mft_no,
				le16_to_cpu(a->flags),
				le32_to_cpu(na->ni->flags));
		goto err_out_put;
	}

	if (a->non_resident) {
		if ((a->flags & ATTR_COMPRESSION_MASK)
		    && !a->compression_unit) {
			err = -EIO;
			antfs_log_error("Compressed inode %lld attr 0x%x has "
					"no compression unit",
					(unsigned long long)ni->mft_no,
					le32_to_cpu(type));
			goto err_out_put;
		}
		ntfs_attr_init(na, TRUE, a->flags,
			       a->flags & ATTR_IS_ENCRYPTED,
			       a->flags & ATTR_IS_SPARSE,
			       sle64_to_cpu(a->allocated_size),
			       sle64_to_cpu(a->data_size),
			       sle64_to_cpu(a->initialized_size),
			       cs ? sle64_to_cpu(a->compressed_size) : 0,
			       cs ? a->compression_unit : 0);
	} else {
		s64 l = le32_to_cpu(a->value_length);
		ntfs_attr_init(na, FALSE, a->flags,
			       a->flags & ATTR_IS_ENCRYPTED,
			       a->flags & ATTR_IS_SPARSE, (l + 7) & ~7, l, l,
			       cs ? (l + 7) & ~7 : 0, 0);
	}
	ntfs_attr_put_search_ctx(ctx);
out:
	antfs_log_leave("err: %d", err);
	return err;

err_out_put:
	ntfs_attr_put_search_ctx(ctx);
err_out:
	ntfs_free(newname);
	goto out;
}

struct ntfs_attr *ntfs_attr_open(struct ntfs_inode *ni,
				 const enum ATTR_TYPES type, ntfschar *name,
				 u32 name_len)
{
	struct ntfs_attr *na = NULL;
	int err = 0;

	na = ntfs_calloc(sizeof(struct ntfs_attr));
	if (!na) {
		na = ERR_PTR(-ENOMEM);
		goto out;
	}
	err = ntfs_attr_fill(ni, na, type, name, name_len);
	if (err) {
		ntfs_free(na);
		na = ERR_PTR(err);
	}
out:
	return na;
}

/**
 * @brief opens the AT_DATA or AT_INDEX_ROOT attribute
 *
 * @param ni	    ntfs inode to store the attribute in @ni->na
 * @param type	    type of attribute (AT_DATA or AT_INDEX_ROOT)
 * @param name	    name of the attribute (AT_UNNAMED or I$30)
 * @param name_len  length of @name (either 0 or 4)
 * @return	    0 if @ni->na is now filled with the attribute
 *		    error code otherwise
 *
 *	The AT_DATA or AT_INDEX_ROOT attribute is held inside the @ni->na
 *	because of the Standard Attribute Header (sah) which holds the
 *	data_size, allocated_size, etc. attributes of the file/directory.
 */
int ntfs_attr_sah_open(struct ntfs_inode *ni, const enum ATTR_TYPES type,
		    ntfschar *name, u32 name_len)
{
	return ntfs_attr_fill(ni, ANTFS_NA(ni), type, name, name_len);
}

/**
 * ntfs_attr_close - free an ntfs attribute structure
 * @na:		ntfs attribute structure to free
 *
 * Release all memory associated with the ntfs attribute @na and then release
 * @na itself.
 */
void ntfs_attr_close(struct ntfs_attr *na)
{
	if (IS_ERR_OR_NULL(na))
		return;
	if (NAttrNonResident(na) && na->rl)
		ntfs_free(na->rl);
	/* Don't release if using an internal constant. */
	if (na->name != AT_UNNAMED && na->name != NTFS_INDEX_I30
	    && na->name != STREAM_SDS)
		ntfs_free(na->name);
	ntfs_free(na);
}

/**
 * ntfs_attr_map_runlist - map (a part of) a runlist of an ntfs attribute
 * @na:		ntfs attribute for which to map (part of) a runlist
 * @vcn:	map runlist part containing this vcn
 *
 * Map the part of a runlist containing the @vcn of the ntfs attribute @na.
 *
 * Return 0 on success and the error code on error.
 */
int ntfs_attr_map_runlist(struct ntfs_attr *na, VCN vcn)
{
	LCN lcn;
	struct ntfs_attr_search_ctx *ctx;
	int err;

	antfs_log_debug("Entering for inode 0x%llx, attr 0x%x, vcn 0x%llx.",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)vcn);

	lcn = ntfs_rl_vcn_to_lcn(na->rl, vcn);
	if (lcn >= 0 || lcn == LCN_HOLE || lcn == LCN_ENOENT)
		return 0;

	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	/* Find the attribute in the mft record. */
	err = ntfs_attr_lookup(na->type, na->name, na->name_len, CASE_SENSITIVE,
			       vcn, NULL, 0, ctx);
	if (!err) {
		struct runlist_element *rl;

		/* Decode the runlist. */
		rl = ntfs_mapping_pairs_decompress(na->ni->vol, ctx->attr,
						   na->rl);
		if (!IS_ERR(rl)) {
			na->rl = rl;
			ntfs_attr_put_search_ctx(ctx);
			return 0;
		}
		err = PTR_ERR(rl);
	}

	antfs_log_error("Exit err=%d", err);
	ntfs_attr_put_search_ctx(ctx);
	return err;
}

#if PARTIAL_RUNLIST_UPDATING

/*
 *		Map the runlist of an attribute from some point to the end
 *
 *	Returns 0 if success,
 *		-1 if it failed (errno telling why)
 */

static int ntfs_attr_map_partial_runlist(struct ntfs_attr *na, VCN vcn)
{
	VCN last_vcn;
	VCN highest_vcn;
	VCN needed;
	struct runlist_element *rl;
	struct ATTR_RECORD *a;
	bool startseen;
	struct ntfs_attr_search_ctx *ctx;
	bool done;
	bool newrunlist;

	antfs_log_debug("Entered vcn: %lld", vcn);

	if (NAttrFullyMapped(na)) {
		if (na->rl) {
			antfs_log_leave("Fully mapped");
			return 0;
		}
		NAttrClearFullyMapped(na);
	}

	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (IS_ERR(ctx)) {
		antfs_log_leave("Failed to get ctx");
		return PTR_ERR(ctx);
	}

	/* Get the last vcn in the attribute. */
	last_vcn = na->allocated_size >> na->ni->vol->cluster_size_bits;

	needed = vcn;
	highest_vcn = 0;
	startseen = FALSE;
	done = FALSE;
	rl = (struct runlist_element *)NULL;
	do {
		newrunlist = FALSE;
		/* Find the attribute in the mft record. */
		if (!ntfs_attr_lookup
		    (na->type, na->name, na->name_len, CASE_SENSITIVE, needed,
		     NULL, 0, ctx)) {

			a = ctx->attr;
			/* Decode and merge the runlist. */
			if (ntfs_rl_vcn_to_lcn(na->rl, needed)
			    == LCN_RL_NOT_MAPPED) {
				rl = ntfs_mapping_pairs_decompress(na->ni->vol,
								   a, na->rl);
				newrunlist = TRUE;
			} else {
				rl = na->rl;
				if (IS_ERR_OR_NULL(rl)) {
					antfs_log_error("na->rl err: %d",
							(int)PTR_ERR(rl));
				}
			}
			if (!IS_ERR(rl)) {
				na->rl = rl;
				highest_vcn = sle64_to_cpu(a->highest_vcn);
				if (highest_vcn < needed) {
					/* corruption detection on unchanged
					 * runlists */
					if (newrunlist
					    && ((highest_vcn + 1) < last_vcn)) {
						antfs_log_error("Corrupt "
								 "attribute"
								 " list");
						ntfs_attr_put_search_ctx(ctx);
						return -EIO;
					}
					done = TRUE;
				}
				needed = highest_vcn + 1;
				if (!a->lowest_vcn)
					startseen = TRUE;
			}
		} else {
			done = TRUE;
		}
	} while (!IS_ERR_OR_NULL(rl) && !done && (needed < last_vcn));

	if (IS_ERR(rl)) {
		antfs_log_error("rl error: %d", (int)PTR_ERR(rl));
		rl = NULL;
	}

	ntfs_attr_put_search_ctx(ctx);
	/*
	 * Make sure we reached the end, unless the last
	 * runlist was modified earlier (using HOLES_DELAY
	 * leads to have a visibility over attributes which
	 * have not yet been fully updated)
	 */
	if (done && newrunlist && (needed < last_vcn)) {
		antfs_log_error("End of runlist not reached");
		return -EIO;
	}
	/* mark fully mapped if we did so */
	if (rl && startseen)
		NAttrSetFullyMapped(na);

	antfs_log_leave("Exit: %d", rl ? 0 : -EIO);
	return rl ? 0 : -EIO;
}

#endif

/**
 * ntfs_attr_map_whole_runlist - map the whole runlist of an ntfs attribute
 * @na:		ntfs attribute for which to map the runlist
 *
 * Map the whole runlist of the ntfs attribute @na.  For an attribute made up
 * of only one attribute extent this is the same as calling
 * ntfs_attr_map_runlist(na, 0) but for an attribute with multiple extents this
 * will map the runlist fragments from each of the extents thus giving access
 * to the entirety of the disk allocation of an attribute.
 *
 * Return 0 on success and the error code on error.
 */
int ntfs_attr_map_whole_runlist(struct ntfs_attr *na)
{
	VCN next_vcn, last_vcn, highest_vcn;
	struct ntfs_attr_search_ctx *ctx;
	struct ntfs_volume *vol = na->ni->vol;
	struct ATTR_RECORD *a;
	int ret = -EIO;
	int not_mapped;

	antfs_log_enter("Entering for inode %llu, attr 0x%x.",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type));

	/* avoid multiple full runlist mappings */
	if (NAttrFullyMapped(na)) {
		if (na->rl) {
			ret = 0;
			goto out;
		}
		NAttrClearFullyMapped(na);
	}
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (IS_ERR(ctx)) {
		ret = PTR_ERR(ctx);
		goto out;
	}

	/* Map all attribute extents one by one. */
	next_vcn = last_vcn = highest_vcn = 0;
	a = NULL;
	while (1) {
		struct runlist_element *rl;

		not_mapped = 0;
		if (ntfs_rl_vcn_to_lcn(na->rl, next_vcn) == LCN_RL_NOT_MAPPED)
			not_mapped = 1;

		ret = ntfs_attr_lookup(na->type, na->name, na->name_len,
				       CASE_SENSITIVE, next_vcn, NULL, 0, ctx);
		if (ret)
			break;

		a = ctx->attr;

		if (not_mapped) {
			/* Decode the runlist. */
			rl = ntfs_mapping_pairs_decompress(na->ni->vol,
							   a, na->rl);
			if (IS_ERR(rl)) {
				ret = PTR_ERR(rl);
				goto err_out;
			}
			na->rl = rl;
		}

		/* Are we in the first extent? */
		if (!next_vcn) {
			if (a->lowest_vcn) {
				antfs_log_error("First extent of inode %llu "
						"attribute has non-zero lowest_vcn",
						(unsigned long long)na->ni->
						mft_no);
				goto err_out;
			}
			/* Get the last vcn in the attribute. */
			last_vcn = sle64_to_cpu(a->allocated_size) >>
			    vol->cluster_size_bits;
		}

		/* Get the lowest vcn for the next extent. */
		highest_vcn = sle64_to_cpu(a->highest_vcn);
		next_vcn = highest_vcn + 1;

		/* Only one extent or error, which we catch below. */
		if (highest_vcn <= 0) {
			ret = -ENOENT;
			break;
		}

		/* Avoid endless loops due to corruption. */
		if (next_vcn < sle64_to_cpu(a->lowest_vcn)) {
			/* log? we are corrupt already */
			antfs_log_error("Inode %llu has corrupt attribute list",
					(unsigned long long)na->ni->mft_no);
			goto err_out;
		}
	}
	if (!a) {
		antfs_log_error("Couldn't find attribute for runlist mapping");
		goto err_out;
	}
	/*
	 * Cannot check highest_vcn when the last runlist has
	 * been modified earlier, as runlists and sizes may be
	 * updated without highest_vcn being in sync, when
	 * HOLES_DELAY is used
	 */
	if (not_mapped && highest_vcn && highest_vcn != last_vcn - 1) {
		ret = -EIO;
		antfs_log_error("Failed to load full runlist: inode: %llu "
				"highest_vcn: 0x%llx last_vcn: 0x%llx",
				(unsigned long long)na->ni->mft_no,
				(long long)highest_vcn, (long long)last_vcn);
		goto err_out;
	}
	/* This is not an error if we have a single extent or zero length file
	 * or if all extents were mapped and attr_lookup failed. Also check
	 * na->rl to make sure everything got mapped.
	 */
	if (ret == -ENOENT && na->rl) {
		NAttrSetFullyMapped(na);
		ret = 0;
	}
err_out:
	ntfs_attr_put_search_ctx(ctx);
out:
	antfs_log_leave();
	return ret;
}

/**
 * ntfs_attr_find_vcn - find a vcn in the runlist of an ntfs attribute
 * @na:		ntfs attribute whose runlist to search
 * @vcn:	vcn to find
 *
 * Find the virtual cluster number @vcn in the runlist of the ntfs attribute
 * @na and return the the address of the runlist element containing the @vcn.
 *
 * Note you need to distinguish between the lcn of the returned runlist
 * element being >= 0 and LCN_HOLE. In the later case you have to return zeroes
 * on read and allocate clusters on write. You need to update the runlist, the
 * attribute itself as well as write the modified mft record to disk.
 *
 * If there is an error return the error code. The
 * following error codes are defined:
 *	EINVAL		Input parameter error.
 *	ENOENT		There is no such vcn in the runlist.
 *	ENOMEM		Not enough memory.
 *	EIO		I/O error or corrupt metadata.
 */
struct runlist_element *ntfs_attr_find_vcn(struct ntfs_attr *na, const VCN vcn)
{
	struct runlist_element *rl;
	bool is_retry = FALSE;
	int err = 0;

	if (!na || !NAttrNonResident(na) || vcn < 0)
		return ERR_PTR(-EINVAL);

	antfs_log_enter("inode 0x%llx, attr 0x%x, vcn %llx",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)vcn);
retry:
	rl = na->rl;
	if (!rl) {
		antfs_log_debug("!na->rl");
		goto map_rl;
	}
	if (vcn < rl[0].vcn) {
		antfs_log_debug("vcn (%lld) < rl[0].vcn (%lld)", vcn,
				rl[0].vcn);
		goto map_rl;
	}
	while (rl->length) {
		if (vcn < rl[1].vcn) {
			if (rl->lcn >= (LCN) LCN_HOLE) {
				antfs_log_leave("Found @lcn %lld",
						(long long)rl->lcn);
				return rl;
			}
			break;
		}
		rl++;
	}
	switch (rl->lcn) {
	case (LCN) LCN_RL_NOT_MAPPED:
		antfs_log_debug("LCN_RL_NOT_MAPPED");
		goto map_rl;
	case (LCN) LCN_ENOENT:
		antfs_log_leave("LCN: -ENOENT");
		return ERR_PTR(-ENOENT);
	case (LCN) LCN_EINVAL:
		antfs_log_leave("LCN: -EINVAL");
		return ERR_PTR(-EINVAL);
	default:
		antfs_log_leave("LCN %lld: -EIO", (long long)rl->lcn);
		return ERR_PTR(-EIO);
	}
map_rl:
	/* The @vcn is in an unmapped region, map the runlist and retry. */
	err = ntfs_attr_map_runlist(na, vcn);
	antfs_log_debug("ntfs_attr_map_runlist err=%d", err);
	if (!is_retry && !err) {
		is_retry = TRUE;
		goto retry;
	}
	/*
	 * If we already retried or the mapping attempt failed something has
	 * gone badly wrong. EINVAL and ENOENT coming from a failed mapping
	 * attempt are equivalent to errors for us as they should not happen
	 * in our code paths.
	 */
	antfs_log_error("Exit with err=%d (changed to -EIO)", err);
	antfs_log_leave();
	return ERR_PTR(-EIO);
}

/**
 * ntfs_attr_pread_i - see description at ntfs_attr_pread()
 */
static s64 ntfs_attr_pread_i(struct ntfs_attr *na, const s64 pos, s64 count,
			     void *b)
{
	s64 br, to_read, ofs, total, total2, max_read, max_init;
	struct ntfs_volume *vol;
	struct runlist_element *rl;
	u16 efs_padding_length;
	int err;

	/* Sanity checking arguments is done in ntfs_attr_pread(). */
	if ((na->data_flags & ATTR_COMPRESSION_MASK) && NAttrNonResident(na)) {
		if ((na->data_flags & ATTR_COMPRESSION_MASK)
		    == ATTR_IS_COMPRESSED)
			return ntfs_compressed_attr_pread(na, pos, count, b);
		else {
			/* compression mode not supported */
			return -EOPNOTSUPP;
		}
	}
	/*
	 * Encrypted non-resident attributes are not supported.  We return
	 * access denied, which is what Windows NT4 does, too.
	 * However, allow if mounted with efs_raw option
	 */
	vol = na->ni->vol;
	if (!vol->efs_raw && NAttrEncrypted(na) && NAttrNonResident(na))
		return -EACCES;

	if (!count)
		return 0;
	/*
	 * Truncate reads beyond end of attribute,
	 * but round to next 512 byte boundary for encrypted
	 * attributes with efs_raw mount option
	 */
	max_read = na->data_size;
	max_init = na->initialized_size;
	if (na->ni->vol->efs_raw && (na->data_flags & ATTR_IS_ENCRYPTED)
	    && NAttrNonResident(na)) {
		if (na->data_size != na->initialized_size) {
			antfs_log_error
			    ("uninitialized encrypted file not supported");
			return -EINVAL;
		}
		max_init = max_read = ((na->data_size + 511) & ~511) + 2;
	}
	if (pos + count > max_read) {
		if (pos >= max_read)
			return 0;
		count = max_read - pos;
	}
	/* If it is a resident attribute, get the value from the mft record. */
	if (!NAttrNonResident(na)) {
		struct ntfs_attr_search_ctx *ctx;
		char *val;

		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (IS_ERR(ctx))
			return PTR_ERR(ctx);

		err = ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				       0, NULL, 0, ctx);
		if (err) {
res_err_out:
			ntfs_attr_put_search_ctx(ctx);
			return err;
		}
		val = (char *)ctx->attr + le16_to_cpu(ctx->attr->value_offset);
		if (val < (char *)ctx->attr || val +
		    le32_to_cpu(ctx->attr->value_length) >
		    (char *)ctx->mrec + vol->mft_record_size) {
			err = -EIO;
			/* log? sanity check failed! */
			antfs_log_error("Sanity check failed");
			goto res_err_out;
		}
		memcpy(b, val + pos, count);
		ntfs_attr_put_search_ctx(ctx);
		return count;
	}
	total = total2 = 0;
	/* Zero out reads beyond initialized size. */
	if (pos + count > max_init) {
		if (pos >= max_init) {
			memset(b, 0, count);
			return count;
		}
		total2 = pos + count - max_init;
		count -= total2;
		memset((u8 *) b + count, 0, total2);
	}
	/*
	 * for encrypted non-resident attributes with efs_raw set
	 * the last two bytes aren't read from disk but contain
	 * the number of padding bytes so original size can be
	 * restored
	 */
	if (na->ni->vol->efs_raw &&
	    (na->data_flags & ATTR_IS_ENCRYPTED) &&
	    ((pos + count) > max_init - 2)) {
		efs_padding_length = 511 - ((na->data_size - 1) & 511);
		if (pos + count == max_init) {
			if (count == 1) {
				*((u8 *) b + count - 1) =
				    (u8) (efs_padding_length >> 8);
				count--;
				total2++;
			} else {
				*(le16 *) ((u8 *) b + count - 2) =
				    cpu_to_le16(efs_padding_length);
				count -= 2;
				total2 += 2;
			}
		} else {
			*((u8 *) b + count - 1) =
			    (u8) (efs_padding_length & 0xff);
			count--;
			total2++;
		}
	}

	/* Find the runlist element containing the vcn. */
	rl = ntfs_attr_find_vcn(na, pos >> vol->cluster_size_bits);
	if (IS_ERR(rl)) {
		/*
		 * If the vcn is not present it is an out of bounds read.
		 * However, we already truncated the read to the data_size,
		 * so getting this here is an error.
		 */
		err = PTR_ERR(rl);
		if (err == -ENOENT) {
			err = -EIO;
			antfs_log_error("Failed to find VCN #1");
		}
		return err;
	}
	/*
	 * Gather the requested data into the linear destination buffer. Note,
	 * a partial final vcn is taken care of by the @count capping of read
	 * length.
	 */
	ofs = pos - (rl->vcn << vol->cluster_size_bits);
	for (; count; rl++, ofs = 0) {
		if (rl->lcn == LCN_RL_NOT_MAPPED) {
			rl = ntfs_attr_find_vcn(na, rl->vcn);
			if (IS_ERR(rl)) {
				err = PTR_ERR(rl);
				if (err == -ENOENT) {
					err = -EIO;
					antfs_log_error("Failed to find VCN"
							"#2");
				}
				goto rl_err_out;
			}
			/* Needed for case when runs merged. */
			ofs = pos + total - (rl->vcn << vol->cluster_size_bits);
		}
		if (!rl->length) {
			total = -EIO;
			/* log? rl is corrupt */
			antfs_log_error("Zero run length");
			goto rl_err_out;
		}
		if (rl->lcn < (LCN) 0) {
			if (rl->lcn != (LCN) LCN_HOLE) {
				/* log? rl is corrupt */
				antfs_log_error("Bad run (%lld)",
						(long long)rl->lcn);
				goto rl_err_out;
			}
			/* It is a hole, just zero the matching @b range. */
			to_read = min(count, (rl->length <<
					      vol->cluster_size_bits) - ofs);
			memset(b, 0, to_read);
			/* Update progress counters. */
			total += to_read;
			count -= to_read;
			b = (u8 *) b + to_read;
			continue;
		}
		/* It is a real lcn, read it into @dst. */
		to_read = min(count, (rl->length << vol->cluster_size_bits) -
			      ofs);
retry:
		antfs_log_debug("Reading %lld bytes from vcn %lld, lcn %lld,"
				"ofs %lld.", (long long)to_read,
				(long long)rl->vcn, (long long)rl->lcn,
				(long long)ofs);
		br = ntfs_pread(vol->dev,
				(rl->lcn << vol->cluster_size_bits) + ofs,
				to_read, b);
		/* If everything ok, update progress counters and continue. */
		if (br > 0) {
			total += br;
			count -= br;
			b = (u8 *) b + br;
		}
		if (br == to_read)
			continue;
		/* If the syscall was interrupted, try again. */
		if (br < 0 && (int)br == -EINTR)
			goto retry;
		if (total)
			return total;
		if (!br)
			err = -EIO;
		else
			err = (int)br;
		antfs_log_error("ntfs_pread failed");
		return err;
	}
	/* Finally, return the number of bytes read. */
	return total + total2;
rl_err_out:
	if (total)
		return total;
	return -EIO;
}

/**
 * ntfs_attr_pread - read from an attribute specified by an ntfs_attr structure
 * @na:		ntfs attribute to read from
 * @pos:	byte position in the attribute to begin reading from
 * @count:	number of bytes to read
 * @b:		output data buffer
 *
 * This function will read @count bytes starting at offset @pos from the ntfs
 * attribute @na into the data buffer @b.
 *
 * On success, return the number of successfully read bytes. If this number is
 * lower than @count this means that the read reached end of file or that an
 * error was encountered during the read so that the read is partial. 0 means
 * end of file or nothing was read (also return 0 when @count is 0).
 *
 * On error and nothing has been read, return the error code of ntfs_pread(), or
 *  EINVAL in case of invalid arguments.
 */
s64 ntfs_attr_pread(struct ntfs_attr *na, const s64 pos, s64 count, void *b)
{
	s64 ret;

	if (!na || !na->ni || !na->ni->vol || !b || pos < 0 || count < 0) {
		antfs_log_error("na=%p  b=%p  pos=%lld  count=%lld",
				na, b, (long long)pos, (long long)count);
		return -EINVAL;
	}

	antfs_log_enter("inode %lld attr 0x%x pos %lld count %lld",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)pos,
			(long long)count);

	ret = ntfs_attr_pread_i(na, pos, count, b);

	antfs_log_leave();
	return ret;
}

static int ntfs_attr_fill_zero(struct ntfs_attr *na, s64 pos, s64 count)
{
	char *buf;
	s64 written, size, end = pos + count;
	s64 ofsi;
	const struct runlist_element *rli;
	struct ntfs_volume *vol;
	int ret = 0;

	antfs_log_debug("pos %lld, count %lld", (long long)pos,
			(long long)count);

	if (!na || pos < 0 || count < 0) {
		ret = -EINVAL;
		goto err_out;
	}

	buf = ntfs_calloc(NTFS_BUF_SIZE);
	if (!buf) {
		ret = -ENOMEM;
		goto err_out;
	}

	rli = na->rl;
	ofsi = 0;
	vol = na->ni->vol;
	while (pos < end) {
		while (rli->length && (ofsi + (rli->length <<
					       vol->cluster_size_bits) <=
				       pos)) {
			ofsi += (rli->length << vol->cluster_size_bits);
			rli++;
		}
		size = min(end - pos, (s64) NTFS_BUF_SIZE);
		/*
		 * If the zeroed block is fully within a hole,
		 * we need not write anything, so advance as far
		 * as possible within the hole.
		 */
		if ((rli->lcn == (LCN) LCN_HOLE)
		    && (ofsi <= pos)
		    && (ofsi + (rli->length << vol->cluster_size_bits)
			>= (pos + size))) {
			size = min(end - pos, ofsi - pos
				   + (rli->length << vol->cluster_size_bits));
			pos += size;
		} else {
			written = ntfs_rl_pwrite(vol, rli, ofsi, pos,
						 size, buf);
			if (written <= 0) {
				antfs_log_error("Failed to zero space");
				ret = written;
				goto err_free;
			}
			pos += written;
		}
	}
err_free:
	ntfs_free(buf);
err_out:
	return ret;
}

static int ntfs_attr_fill_hole(struct ntfs_attr *na, s64 count, s64 *ofs,
			       struct runlist_element **rl, VCN *update_from)
{
	s64 to_write;
	s64 need;
	struct ntfs_volume *vol = na->ni->vol;
	int ret = 0;
	struct runlist_element *rlc;
	LCN lcn_seek_from = -1;
	VCN cur_vcn, from_vcn;

	to_write = min(count, ((*rl)->length << vol->cluster_size_bits) - *ofs);

	cur_vcn = (*rl)->vcn;
	from_vcn = (*rl)->vcn + (*ofs >> vol->cluster_size_bits);

	antfs_log_debug
	    ("count: %lld, cur_vcn: %lld, from: %lld, to: %lld, ofs: %lld",
	     (long long)count, (long long)cur_vcn, (long long)from_vcn,
	     (long long)to_write, (long long)*ofs);

	/* Map the runlist to be able to update mapping pairs later. */
#if PARTIAL_RUNLIST_UPDATING
	if (!na->rl) {
		ret = ntfs_attr_map_whole_runlist(na);
		if (ret)
			goto err_out;
	} else {
		/* make sure the run ahead of hole is mapped */
		if ((*rl)->lcn == LCN_HOLE) {
			ret = ntfs_attr_map_partial_runlist(na,
							    (cur_vcn ? cur_vcn -
							     1 : cur_vcn));
			if (!ret)
				goto err_out;
		}
	}
#else
	ret = ntfs_attr_map_whole_runlist(na);
	if (ret)
		goto err_out;
#endif

	/* Restore @*rl, it probably get lost during runlist mapping. */
	*rl = ntfs_attr_find_vcn(na, cur_vcn);
	if (IS_ERR(rl)) {
		antfs_log_error("Failed to find run after mapping runlist. "
				"Please report to %s.", NTFS_DEV_LIST);
		ret = PTR_ERR(rl);
		goto err_out;
	}

	/* Search backwards to find the best lcn to start seek from. */
	rlc = *rl;
	while (rlc->vcn) {
		rlc--;
		if (rlc->lcn >= 0) {
			/*
			 * avoid fragmenting a compressed file
			 * Windows does not do that, and that may
			 * not be desirable for files which can
			 * be updated
			 */
			if (na->data_flags & ATTR_COMPRESSION_MASK)
				lcn_seek_from = rlc->lcn + rlc->length;
			else
				lcn_seek_from =
				    rlc->lcn + (from_vcn - rlc->vcn);
			break;
		}
	}
	if (lcn_seek_from == -1) {
		/* Backwards search failed, search forwards. */
		rlc = *rl;
		while (rlc->length) {
			rlc++;
			if (rlc->lcn >= 0) {
				lcn_seek_from =
				    rlc->lcn - (rlc->vcn - from_vcn);
				if (lcn_seek_from < -1)
					lcn_seek_from = -1;
				break;
			}
		}
	}

	need = ((*ofs + to_write - 1) >> vol->cluster_size_bits)
	    + 1 + (*rl)->vcn - from_vcn;
	if ((na->data_flags & ATTR_COMPRESSION_MASK)
	    && (need < na->compression_block_clusters)) {
		/*
		 * for a compressed file, be sure to allocate the full
		 * compression block, as we may need space to decompress
		 * existing compressed data.
		 * So allocate the space common to compression block
		 * and existing hole.
		 */
		VCN alloc_vcn;

		if ((from_vcn & -na->compression_block_clusters) <= (*rl)->vcn)
			alloc_vcn = (*rl)->vcn;
		else
			alloc_vcn = from_vcn & -na->compression_block_clusters;
		need = (alloc_vcn | (na->compression_block_clusters - 1))
		    + 1 - alloc_vcn;
		if (need > (*rl)->length) {
			antfs_log_error("Cannot allocate %lld clusters"
					" within a hole of %lld",
					(long long)need,
					(long long)(*rl)->length);
			ret = -EIO;
			goto err_out;
		}
		rlc = ntfs_cluster_alloc(vol, alloc_vcn, need,
					 lcn_seek_from, DATA_ZONE,
					 na->data_size);
	} else
		rlc = ntfs_cluster_alloc(vol, from_vcn, need,
					 lcn_seek_from, DATA_ZONE,
					 na->data_size);
	if (IS_ERR_OR_NULL(rlc)) {
		ret = -EIO;
		goto err_out;
	}

	*rl = ntfs_runlists_merge(na->rl, rlc);
	NAttrSetRunlistDirty(na);
	/*
	 * For a compressed attribute, we must be sure there are two
	 * available entries, so reserve them before it gets too late.
	 */
	if (!IS_ERR(*rl) && (na->data_flags & ATTR_COMPRESSION_MASK)) {
		struct runlist_element *oldrl = na->rl;

		na->rl = *rl;
		*rl = ntfs_rl_extend(na, *rl, 2);
		if (IS_ERR(*rl))
			na->rl = oldrl;	/* restore to original if failed */
	}
	/* This extra test here is important: *rl is changed above! */
	if (IS_ERR(*rl)) {
		ret = PTR_ERR(*rl);
		antfs_log_error("Failed to merge runlists");
		if (ntfs_cluster_free_from_rl(vol, rlc)) {
			antfs_log_error("Failed to free hot clusters. "
					"Please run chkdsk /f");
		}
		goto err_out;
	}
	na->unused_runs = 2;
	na->rl = *rl;
	if ((*update_from == -1) || (from_vcn < *update_from))
		*update_from = from_vcn;
	*rl = ntfs_attr_find_vcn(na, cur_vcn);
	if (IS_ERR(*rl)) {
		/*
		 * It's definitely a BUG, if we failed to find @cur_vcn, because
		 * we missed it during instantiating of the hole.
		 */
		antfs_log_error("Failed to find run after hole instantiation. "
				"Please report to %s.", NTFS_DEV_LIST);
		ret = -EIO;
		goto err_out;
	}
	/* If leaved part of the hole go to the next run. */
	if ((*rl)->lcn < 0)
		(*rl)++;
	/* Now LCN shoudn't be less than 0. */
	if ((*rl)->lcn < 0) {
		antfs_log_error("BUG! LCN is lesser than 0. "
				"Please report to the %s.", NTFS_DEV_LIST);
		ret = -EIO;
		goto err_out;
	}
	if (*ofs) {
		/* Clear non-sparse region from @cur_vcn to @*ofs. */
		if (ntfs_attr_fill_zero(na, cur_vcn << vol->cluster_size_bits,
					*ofs)) {
			ret = -EIO;
			goto err_out;
		}
	}
	if ((*rl)->vcn < cur_vcn) {
		/*
		 * Clusters that replaced hole are merged with
		 * previous run, so we need to update offset.
		 */
		*ofs += (cur_vcn - (*rl)->vcn) << vol->cluster_size_bits;
	}
	if ((*rl)->vcn > cur_vcn) {
		/*
		 * We left part of the hole, so we need to update offset
		 */
		*ofs -= ((*rl)->vcn - cur_vcn) << vol->cluster_size_bits;
	}
	if (na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_SPARSE))
		na->compressed_size += need << vol->cluster_size_bits;
err_out:
	return ret;
}

static int stuff_hole(struct ntfs_attr *na, const s64 pos);

/*
 *		Split an existing hole for overwriting with data
 *	The hole may have to be split into two or three parts, so
 *	that the overwritten part fits within a single compression block
 *
 *	No cluster allocation is needed, this will be done later in
 *	standard hole filling, hence no need to reserve runs for
 *	future needs.
 *
 *	Returns the number of clusters with existing compressed data
 *		in the compression block to be written to
 *		(or the full block, if it was a full hole)
 *		-1 if there were an error
 */

static int split_compressed_hole(struct ntfs_attr *na,
				 struct runlist_element **prl,
				 s64 pos, s64 count, VCN *update_from)
{
	int compressed_part;
	int cluster_size_bits = na->ni->vol->cluster_size_bits;
	struct runlist_element *rl = *prl;

	compressed_part = na->compression_block_clusters;
	/* reserve entries in runlist if we have to split */
	if (rl->length > na->compression_block_clusters) {
		*prl = ntfs_rl_extend(na, *prl, 2);
		if (!*prl) {
			compressed_part = -1;
		} else {
			rl = *prl;
			na->unused_runs = 2;
		}
	}
	if (*prl && (rl->length > na->compression_block_clusters)) {
		/*
		 * Locate the update part relative to beginning of
		 * current run
		 */
		int beginwrite = (pos >> cluster_size_bits) - rl->vcn;
		s32 endblock = (((pos + count - 1) >> cluster_size_bits)
				| (na->compression_block_clusters - 1)) + 1 -
		    rl->vcn;

		compressed_part = na->compression_block_clusters
		    - (rl->length & (na->compression_block_clusters - 1));
		if ((beginwrite + compressed_part) >=
		    na->compression_block_clusters)
			compressed_part = na->compression_block_clusters;
		/*
		 * if the run ends beyond end of needed block
		 * we have to split the run
		 */
		if (endblock < rl[0].length) {
			struct runlist_element *xrl;
			int n;

			/*
			 * we have to split into three parts if the run
			 * does not end within the first compression block.
			 * This means the hole begins before the
			 * compression block.
			 */
			if (endblock > na->compression_block_clusters) {
				if (na->unused_runs < 2) {
					antfs_log_error
					    ("No free run, case 1");
				}
				na->unused_runs -= 2;
				xrl = rl;
				n = 0;
				while (xrl->length) {
					xrl++;
					n++;
				}
				do {
					xrl[2] = *xrl;
					xrl--;
				} while (xrl != rl);
				rl[1].length = na->compression_block_clusters;
				rl[2].length = rl[0].length - endblock;
				rl[0].length = endblock
				    - na->compression_block_clusters;
				rl[1].lcn = LCN_HOLE;
				rl[2].lcn = LCN_HOLE;
				rl[1].vcn = rl[0].vcn + rl[0].length;
				rl[2].vcn = rl[1].vcn
				    + na->compression_block_clusters;
				rl = ++(*prl);
			} else {
				/*
				 * split into two parts and use the
				 * first one
				 */
				if (!na->unused_runs) {
					antfs_log_error
					    ("No free run, case 2");
				}
				na->unused_runs--;
				xrl = rl;
				n = 0;
				while (xrl->length) {
					xrl++;
					n++;
				}
				do {
					xrl[1] = *xrl;
					xrl--;
				} while (xrl != rl);
				if (beginwrite < endblock) {
					/* we will write into the first part of
					 * hole */
					rl[1].length = rl[0].length - endblock;
					rl[0].length = endblock;
					rl[1].vcn = rl[0].vcn + rl[0].length;
					rl[1].lcn = LCN_HOLE;
				} else {
					/* we will write into the second part of
					 * hole. impossible ? */
					rl[1].length = rl[0].length - endblock;
					rl[0].length = endblock;
					rl[1].vcn = rl[0].vcn + rl[0].length;
					rl[1].lcn = LCN_HOLE;
					rl = ++(*prl);
				}
			}
		} else {
			if (rl[1].length) {
				struct runlist_element *xrl;
				int n;

				/*
				 * split into two parts and use the
				 * last one
				 */
				if (!na->unused_runs) {
					antfs_log_error
					    ("No free run, case 4");
				}
				na->unused_runs--;
				xrl = rl;
				n = 0;
				while (xrl->length) {
					xrl++;
					n++;
				}
				do {
					xrl[1] = *xrl;
					xrl--;
				} while (xrl != rl);
			} else {
				rl[2].lcn = rl[1].lcn;
				rl[2].vcn = rl[1].vcn;
				rl[2].length = rl[1].length;
			}
			rl[1].vcn -= na->compression_block_clusters;
			rl[1].lcn = LCN_HOLE;
			rl[1].length = na->compression_block_clusters;
			rl[0].length -= na->compression_block_clusters;
			if (pos >= (rl[1].vcn << cluster_size_bits))
				rl = ++(*prl);
		}
		NAttrSetRunlistDirty(na);
		if ((*update_from == -1) || ((*prl)->vcn < *update_from))
			*update_from = (*prl)->vcn;
	}
	return compressed_part;
}

/*
 *		Borrow space from adjacent hole for appending data
 *	The hole may have to be split so that the end of hole is not
 *	affected by cluster allocation and overwriting
 *	Cluster allocation is needed for the overwritten compression block
 *
 *	Must always leave two unused entries in the runlist
 *
 *	Returns the number of clusters with existing compressed data
 *		in the compression block to be written to
 *		-1 if there were an error
 */
static int borrow_from_hole(struct ntfs_attr *na, struct runlist_element **prl,
			    s64 pos, s64 count, VCN *update_from,
			    bool wasnonresident)
{
	int compressed_part = 0;
	int cluster_size_bits = na->ni->vol->cluster_size_bits;
	struct runlist_element *rl = *prl;
	s32 endblock;
	long long allocated;
	struct runlist_element *zrl;
	int irl;
	bool undecided;
	bool nothole;
	int err = -EIO;

	/* check whether the compression block is fully allocated */
	endblock =
	    (((pos + count -
	       1) >> cluster_size_bits) | (na->compression_block_clusters -
					   1)) + 1 - rl->vcn;
	allocated = 0;
	zrl = rl;
	irl = 0;
	while (zrl->length && (zrl->lcn >= 0) && (allocated < endblock)) {
		allocated += zrl->length;
		zrl++;
		irl++;
	}

	undecided = (allocated < endblock) && (zrl->lcn == LCN_RL_NOT_MAPPED);
	nothole = (allocated >= endblock) || (zrl->lcn != LCN_HOLE);

	if (undecided || nothole) {
		struct runlist_element *orl = na->rl;
		s64 olcn = (*prl)->lcn;
#if PARTIAL_RUNLIST_UPDATING
		VCN prevblock;
#endif
		/*
		 * Map the runlist, unless it has not been created.
		 * If appending data, a partial mapping from the
		 * end of previous block will do.
		 */
		irl = *prl - na->rl;
#if PARTIAL_RUNLIST_UPDATING
		prevblock = pos >> cluster_size_bits;
		if (prevblock)
			prevblock--;

		err = NAttrDataAppending(na)
		    ? ntfs_attr_map_partial_runlist(na, prevblock)
		    : ntfs_attr_map_whole_runlist(na);
		if (!NAttrBeingNonResident(na) && err) {
#else
		err = ntfs_attr_map_whole_runlist(na);
		if (!NAttrBeingNonResident(na) && err) {
#endif
			rl = (struct runlist_element *)NULL;
		} else {
			/*
			 * Mapping the runlist may cause its relocation,
			 * and relocation may be at the same place with
			 * relocated contents.
			 * Have to find the current run again when this
			 * happens.
			 */
			if ((na->rl != orl) || ((*prl)->lcn != olcn)) {
				zrl = &na->rl[irl];
				while (zrl->length && (zrl->lcn != olcn))
					zrl++;
				*prl = zrl;
			}
			if (!(*prl)->length) {
				antfs_log_error("Mapped run not found,"
						" inode %lld lcn 0x%llx",
						(long long)na->ni->mft_no,
						(long long)olcn);
				rl = (struct runlist_element *)NULL;
			} else {
				rl = ntfs_rl_extend(na, *prl, 2);
				na->unused_runs = 2;
			}
		}
		*prl = rl;
		if (rl && undecided) {
			allocated = 0;
			zrl = rl;
			irl = 0;
			while (zrl->length && (zrl->lcn >= 0)
			       && (allocated < endblock)) {
				allocated += zrl->length;
				zrl++;
				irl++;
			}
		}
	}
	/*
	 * compression block not fully allocated and followed
	 * by a hole : we must allocate in the hole.
	 */
	if (rl && (allocated < endblock) && (zrl->lcn == LCN_HOLE)) {
		s64 xofs;

		/*
		 * split the hole if not fully needed
		 */
		if ((allocated + zrl->length) > endblock) {
			struct runlist_element *xrl;

			*prl = ntfs_rl_extend(na, *prl, 1);
			if (*prl) {
				/* beware : rl was reallocated */
				rl = *prl;
				zrl = &rl[irl];
				na->unused_runs = 0;
				xrl = zrl;
				while (xrl->length)
					xrl++;
				do {
					xrl[1] = *xrl;
				} while (xrl-- != zrl);
				zrl->length = endblock - allocated;
				zrl[1].length -= zrl->length;
				zrl[1].vcn = zrl->vcn + zrl->length;
				NAttrSetRunlistDirty(na);
			}
		}
		if (*prl) {
			if (wasnonresident)
				compressed_part = na->compression_block_clusters
				    - zrl->length;
			xofs = 0;
			err = ntfs_attr_fill_hole(na,
						  zrl->
						  length << cluster_size_bits,
						  &xofs, &zrl, update_from);
			if (!err)
				compressed_part = err;
			else {
			/* go back to initial cluster, now reallocated */
				while (zrl->vcn > (pos >> cluster_size_bits))
					zrl--;
				*prl = zrl;
			}
		}
	}
	if (!*prl) {
		antfs_log_error("No elements to borrow from a hole");
		compressed_part = err;
	} else if ((*update_from == -1) || ((*prl)->vcn < *update_from))
		*update_from = (*prl)->vcn;
	return compressed_part;
}


/**
 * ntfs_attr_pwrite - positioned write to an ntfs attribute
 * @na:		ntfs attribute to write to
 * @pos:	position in the attribute to write to
 * @count:	number of bytes to write
 * @b:		data buffer to write to disk
 *
 * This function will write @count bytes from data buffer @b to ntfs attribute
 * @na at position @pos.
 *
 * On success, return the number of successfully written bytes. If this number
 * is lower than @count this means that an error was encountered during the
 * write so that the write is partial. 0 means nothing was written (also return
 * 0 when @count is 0).
 *
 * On error and nothing has been written, the return code of ntfs_pwrite(), or
 * -EINVAL in case of invalid arguments is returned.
 */
static s64 ntfs_attr_pwrite_i(struct ntfs_attr *na, const s64 pos, s64 count,
			      const void *b)
{
	s64 written, to_write, ofs, old_initialized_size, old_data_size;
	s64 total = 0;
	VCN update_from = -1;
	struct ntfs_volume *vol;
	s64 fullcount;
	struct ntfs_attr_search_ctx *ctx = NULL;
	struct runlist_element *rl;
	s64 hole_end;
	int eo, err = -EIO;
	int compressed_part;
	struct {
		unsigned int undo_initialized_size:1;
		unsigned int undo_data_size:1;
	} need_to = {
	0, 0};
	bool wasnonresident = FALSE;
	bool compressed;

	antfs_log_enter("pos=%lld; count=%lld", pos, count);
	vol = na->ni->vol;
	compressed = (na->data_flags & ATTR_COMPRESSION_MASK)
	    != const_cpu_to_le16(0);
	na->unused_runs = 0;	/* prepare overflow checks */
	/*
	 * Encrypted attributes are only supported in raw mode.  We return
	 * access denied, which is what Windows NT4 does, too.
	 * Moreover a file cannot be both encrypted and compressed.
	 */
	if ((na->data_flags & ATTR_IS_ENCRYPTED)
	    && (compressed || !vol->efs_raw)) {
		total = -EACCES;
		antfs_log_error("Cannot write encrypted+compressed.");
		goto out;
	}
	/*
	 * Fill the gap, when writing beyond the end of a compressed
	 * file. This will make recursive calls
	 */
	if (compressed && (na->type == AT_DATA)
	    && (pos > na->initialized_size)) {
		eo = stuff_hole(na, pos);
		if (eo != 0) {
			/* Always error here */
			if (eo > 0)
				eo = -ENOSPC;
			total = eo;
			goto out;
	    }
	}
	/* If this is a compressed attribute it needs special treatment. */
	wasnonresident = NAttrNonResident(na) != 0;
	/*
	 * Compression is restricted to data streams and
	 * only ATTR_IS_COMPRESSED compression mode is supported.
	 */
	if (compressed && ((na->type != AT_DATA)
			   || ((na->data_flags & ATTR_COMPRESSION_MASK)
			       != ATTR_IS_COMPRESSED))) {
		total = -EOPNOTSUPP;
		antfs_log_error("Cannot write compressed NOT data attrib or "
				"unsupported compression method.");
		goto out;
	}

	if (!count)
		goto out;
	/* for a compressed file, get prepared to reserve a full block */
	fullcount = count;
	/* If the write reaches beyond the end, extend the attribute. */
	old_data_size = na->data_size;
	/* identify whether this is appending to a nonresident data attribute */
	if ((na->type == AT_DATA) && (pos >= old_data_size)
	    && NAttrNonResident(na))
		NAttrSetDataAppending(na);
	if (pos + count > na->data_size) {
#if PARTIAL_RUNLIST_UPDATING
		/*
		 * When appending data, the attribute is first extended
		 * before being filled with data. This may cause the
		 * attribute to be made temporarily sparse, which
		 * implies reformating the inode and reorganizing the
		 * full runlist. To avoid unnecessary reorganization,
		 * we avoid sparse testing until the data is filled in.
		 */
		eo = ntfs_attr_truncate_i(na, pos + count,
					  (NAttrDataAppending(na) ?
					   HOLES_DELAY : HOLES_OK));
		if (eo) {
			antfs_log_error("Failed to enlarge attribute");
			/* Make STATUS_ATTRIBUTE_FILLED_MFT an error here */
			if (eo > 0)
				eo = -ENOSPC;
			total = eo;
			goto out;
		}
		/*
		 * If we avoided updating the runlist, we must be sure
		 * to cancel the enlargement and put back the runlist to
		 * a clean state if we get into some error.
		 */
		if (NAttrDataAppending(na))
			need_to.undo_data_size = 1;
#else
		eo = ntfs_attr_truncate_i(na, pos + count, HOLES_OK);
		if (eo) {
			antfs_log_error("Failed to enlarge attribute");
			/* Make STATUS_ATTRIBUTE_FILLED_MFT an error here */
			if (eo > 0)
				eo = -ENOSPC;
			total = eo;
			goto out;
		}
#endif
		/* resizing may change the compression mode */
		compressed = (na->data_flags & ATTR_COMPRESSION_MASK)
		    != const_cpu_to_le16(0);
		need_to.undo_data_size = 1;
	}
	/*
	 * For compressed data, a single full block was allocated
	 * to deal with compression, possibly in a previous call.
	 * We are not able to process several blocks because
	 * some clusters are freed after compression and
	 * new allocations have to be done before proceeding,
	 * so truncate the requested count if needed (big buffers).
	 */
	if (compressed) {
		fullcount = (pos | (na->compression_block_size - 1)) + 1 - pos;
		if (count > fullcount)
			count = fullcount;
	}
	old_initialized_size = na->initialized_size;
	/* If it is a resident attribute, write the data to the mft record. */
	if (!NAttrNonResident(na)) {
		char *val;

		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (IS_ERR(ctx)) {
			eo = PTR_ERR(ctx);
			goto err_out;
		}
		eo = ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				      0, NULL, 0, ctx);
		if (eo) {
			antfs_log_error("lookup failed");
			goto err_out;
		}
		val = (char *)ctx->attr + le16_to_cpu(ctx->attr->value_offset);
		if (val < (char *)ctx->attr || val +
		    le32_to_cpu(ctx->attr->value_length) >
		    (char *)ctx->mrec + vol->mft_record_size) {
			eo = -EIO;
			/* log? sanity check failed */
			antfs_log_error("Sanity check failed");
			goto err_out;
		}
		memcpy(val + pos, b, count);
		eo = ntfs_mft_record_write(vol, ctx->ntfs_ino->mft_no,
					   ctx->mrec);
		if (eo) {
			/*
			 * NOTE: We are in a bad state at this moment. We have
			 * dirtied the mft record but we failed to commit it to
			 * disk. Since we have read the mft record ok before,
			 * it is unlikely to fail writing it, so is ok to just
			 * return error here... (AIA)
			 */
			antfs_log_error("failed to write mft record");
			goto err_out;
		}
		ntfs_attr_put_search_ctx(ctx);
		total = count;
		goto out;
	}

	/* Handle writes beyond initialized_size. */

	if (pos + count > na->initialized_size) {
#if PARTIAL_RUNLIST_UPDATING
		/*
		 * When appending, we only need to map the end of the runlist,
		 * starting at the last previously allocated run, so that
		 * we are able a new one to it.
		 * However, for compressed file, we need the full compression
		 * block, which may be split in several extents.
		 */

		if (compressed && !NAttrDataAppending(na)) {
			eo = ntfs_attr_map_whole_runlist(na);
			if (eo)
				goto err_out;
		} else {
			VCN block_begin;

			if (NAttrDataAppending(na)
					|| (pos < na->initialized_size))
				block_begin = pos >> vol->cluster_size_bits;
			else
				block_begin =
					na->initialized_size >> vol->
					cluster_size_bits;

			if (compressed)
				block_begin &= -na->compression_block_clusters;
			if (block_begin)
				block_begin--;
			eo = ntfs_attr_map_partial_runlist(na, block_begin);
			if (eo)
				goto err_out;
			if ((update_from == -1) || (block_begin < update_from))
				update_from = block_begin;
		}
#else
		eo = ntfs_attr_map_whole_runlist(na);
		if (eo)
			goto err_out;
#endif
		/*
		 * For a compressed attribute, we must be sure there is an
		 * available entry, and, when reopening a compressed file,
		 * we may need to split a hole. So reserve the entries
		 * before it gets too late.
		 */
		if (compressed) {
			na->rl = ntfs_rl_extend(na, na->rl, 2);
			if (IS_ERR(na->rl)) {
				eo = PTR_ERR(na->rl);
				na->rl = NULL;
				goto err_out;
			}
			na->unused_runs = 2;
		}
		/* Set initialized_size to @pos + @count. */
		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (IS_ERR(ctx)) {
			eo = PTR_ERR(ctx);
			goto err_out;
		}
		eo = ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				0, NULL, 0, ctx);
		if (eo)
			goto err_out;

		/* If write starts beyond initialized_size, zero the gap. */
		if (pos > na->initialized_size) {
			eo = ntfs_attr_fill_zero(na, na->initialized_size,
					pos - na->initialized_size);
			if (eo)
				goto err_out;
		}

		ctx->attr->initialized_size = cpu_to_sle64(pos + count);
		/* fix data_size for compressed files */
		if (compressed) {
			na->data_size = pos + count;
			ctx->attr->data_size = ctx->attr->initialized_size;
		}
		eo = ntfs_mft_record_write(vol, ctx->ntfs_ino->mft_no,
				ctx->mrec);
		if (eo) {
			/*
			 * Undo the change in the in-memory copy and send it
			 * back for writing.
			 */
			ctx->attr->initialized_size =
				cpu_to_sle64(old_initialized_size);
			ntfs_mft_record_write(vol, ctx->ntfs_ino->mft_no,
					ctx->mrec);
			goto err_out;
		}
		na->initialized_size = pos + count;
		ntfs_attr_put_search_ctx(ctx);
		ctx = NULL;
		/*
		 * NOTE: At this point the initialized_size in the mft record
		 * has been updated BUT there is random data on disk thus if
		 * we decide to abort, we MUST change the initialized_size
		 * again.
		 */
		need_to.undo_initialized_size = 1;
	}
	/* Find the runlist element containing the vcn. */
	rl = ntfs_attr_find_vcn(na, pos >> vol->cluster_size_bits);
	if (IS_ERR(rl)) {
		eo = PTR_ERR(rl);
		/*
		 * If the vcn is not present it is an out of bounds write.
		 * However, we already extended the size of the attribute,
		 * so getting this here must be an error of some kind.
		 */
		if (eo == -ENOENT) {
			eo = -EIO;
			antfs_log_error("Failed to find VCN #3");
		}
		goto err_out;
	}
	/*
	 * Determine if there is compressed data in the current
	 * compression block (when appending to an existing file).
	 * If so, decompression will be needed, and the full block
	 * must be allocated to be identified as uncompressed.
	 * This comes in two variants, depending on whether
	 * compression has saved at least one cluster.
	 * The compressed size can never be over full size by
	 * more than 485 (maximum for 15 compression blocks
	 * compressed to 4098 and the last 3640 bytes compressed
	 * to 3640 + 3640/8 = 4095, with 15*2 + 4095 - 3640 = 485)
	 * This is less than the smallest cluster, so the hole is
	 * is never beyond the cluster next to the position of
	 * the first uncompressed byte to write.
	 */
	compressed_part = 0;
	if (compressed) {
		if ((rl->lcn == (LCN) LCN_HOLE)
		    && wasnonresident) {
			if (rl->length < na->compression_block_clusters)
				/*
				 * the needed block is in a hole smaller
				 * than the compression block : we can use
				 * it fully
				 */
				compressed_part
				    = na->compression_block_clusters
				    - rl->length;
			else {
				/*
				 * the needed block is in a hole bigger
				 * than the compression block : we must
				 * split the hole and use it partially
				 */
				compressed_part = split_compressed_hole(na,
						&rl, pos, count, &update_from);
			}
		} else {
			if (rl->lcn >= 0) {
				/*
				 * the needed block contains data, make
				 * sure the full compression block is
				 * allocated. Borrow from hole if needed
				 */
				compressed_part = borrow_from_hole(na,
						&rl, pos, count, &update_from,
						wasnonresident);
			}
		}

		if (compressed_part < 0) {
			eo = compressed_part;
			goto err_out;
		}

		/* just making non-resident, so not yet compressed */
		if (NAttrBeingNonResident(na)
		    && (compressed_part < na->compression_block_clusters))
			compressed_part = 0;
	}
	ofs = pos - (rl->vcn << vol->cluster_size_bits);
	/*
	 * Scatter the data from the linear data buffer to the volume. Note, a
	 * partial final vcn is taken care of by the @count capping of write
	 * length.
	 */
	for (hole_end = 0; count; rl++, ofs = 0) {
		if (rl->lcn == LCN_RL_NOT_MAPPED) {
			rl = ntfs_attr_find_vcn(na, rl->vcn);
			if (IS_ERR(rl)) {
				eo = PTR_ERR(rl);
				if (eo == -ENOENT) {
					eo = -EIO;
					antfs_log_error("Failed to find VCN#4");
				}
				goto rl_err_out;
			}
			/* Needed for case when runs merged. */
			ofs = pos + total - (rl->vcn << vol->cluster_size_bits);
		}
		if (!rl->length) {
			eo = -EIO;
			antfs_log_error("Zero run length");
			goto rl_err_out;
		}
		if (rl->lcn < (LCN) 0) {
			hole_end = rl->vcn + rl->length;

			if (rl->lcn != (LCN) LCN_HOLE) {
				eo = -EIO;
				/* log? rl is corrupted? */
				antfs_log_error("Unexpected LCN (%lld)",
						(long long)rl->lcn);
				goto rl_err_out;
			}
			eo = ntfs_attr_fill_hole(na, fullcount, &ofs, &rl,
						 &update_from);
			if (eo)
				goto err_out;
		}
		if (compressed) {
			while (rl->length
			       && (ofs >=
				   (rl->length << vol->cluster_size_bits))) {
				ofs -= rl->length << vol->cluster_size_bits;
				rl++;
			}
		}

		/* It is a real lcn, write it to the volume. */
		to_write =
		    min(count, (rl->length << vol->cluster_size_bits) - ofs);
retry:
		antfs_log_debug("Writing %lld bytes to vcn %lld, lcn %lld, ofs "
				"%lld --> wpos=%lld.", (long long)to_write,
				(long long)rl->vcn, (long long)rl->lcn,
				(long long)ofs,
				(long long)((rl->lcn << vol->cluster_size_bits)
					+ ofs));
		if (!NVolReadOnly(vol)) {

			s64 wpos = (rl->lcn << vol->cluster_size_bits) + ofs;
			s64 wend =
			    (rl->vcn << vol->cluster_size_bits) + ofs +
			    to_write;
			u32 bsize = vol->cluster_size;
			/* Byte size needed to zero fill a cluster */
			s64 rounding =
			    ((wend + bsize - 1) & ~(s64) (bsize - 1)) - wend;
			/**
			 * Zero fill to cluster boundary if we're writing at the
			 * end of the attribute or into an ex-sparse cluster.
			 * This will cause the kernel not to seek and read disk
			 * blocks during write(2) to fill the end of the buffer
			 * which increases write speed by 2-10 fold typically.
			 *
			 * This is done even for compressed files, because
			 * data is generally first written uncompressed.
			 */
			if (rounding && ((wend == na->initialized_size) ||
					 (wend <
					  (hole_end << vol->
					   cluster_size_bits)))) {

				char *cb;

				rounding += to_write;
				antfs_log_debug("Zero fill up: wend=%lld "
						"rounding=%lld to_write=%lld",
						(long long)wend,
						(long long)rounding,
						(long long)to_write);

				cb = ntfs_malloc(rounding);
				if (!cb) {
					eo = -ENOMEM;
					goto err_out;
				}

				memcpy(cb, b, to_write);
				memset(cb + to_write, 0, rounding - to_write);

				if (compressed) {
					written = ntfs_compressed_pwrite(na,
						 rl, wpos, ofs, to_write,
						 rounding, cb, compressed_part,
						 &update_from);
				} else {
					written = ntfs_pwrite(vol->dev, wpos,
							      rounding, cb);
					if (written == rounding)
						written = to_write;
				}

				ntfs_free(cb);
			} else {
				if (compressed) {
					written = ntfs_compressed_pwrite(na,
						 rl, wpos, ofs, to_write,
						 to_write, b, compressed_part,
						 &update_from);
				} else
					written = ntfs_pwrite(vol->dev, wpos,
							      to_write, b);
			}
		} else
			written = to_write;
		/* If everything ok, update progress counters and continue. */
		if (written > 0) {
			total += written;
			count -= written;
			fullcount -= written;
			b = (const u8 *)b + written;
		}
		if (written != to_write) {
			/* Partial write cannot be dealt with, stop there */
			/* If the syscall was interrupted, try again. */
			if (written == -EINTR)
				goto retry;
			eo = !written ? -EIO : written;
			goto rl_err_out;
		}
		compressed_part = 0;
	}
done:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	/*
	 *       Update mapping pairs if needed.
	 * For a compressed file, we try to make a partial update
	 * of the mapping list. This makes a difference only if
	 * inode extents were needed.
	 */
	if (NAttrRunlistDirty(na)) {
		eo = ntfs_attr_update_mapping_pairs(na,
						    (update_from <
						     0 ? 0 : update_from));
		if (!eo) {
			/*
			 * FIXME: trying to recover by goto rl_err_out;
			 * could cause driver hang by infinite looping.
			 */
			total = eo;
			goto out;
		}
		if (!wasnonresident)
			NAttrClearBeingNonResident(na);
		NAttrClearDataAppending(na);
	}
out:
	antfs_log_leave("Exit: %lld", total);
	return total;
rl_err_out:
	if (total) {
		if (need_to.undo_initialized_size) {
			if (pos + total > na->initialized_size)
				goto done;
			/*
			 * TODO: Need to try to change initialized_size. If it
			 * succeeds goto done, otherwise goto err_out. (AIA)
			 */
			goto err_out;
		}
		goto done;
	}
err_out:
	if (need_to.undo_initialized_size) {
		if (!ctx) {
			ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
			if (IS_ERR(ctx))
				err = PTR_ERR(ctx);
		} else {
			ntfs_attr_reinit_search_ctx(ctx);
		}
		if (!err) {
			err = ntfs_attr_lookup(na->type, na->name,
					       na->name_len, 0, 0, NULL, 0,
					       ctx);
			if (!err) {
				na->initialized_size = old_initialized_size;
				ctx->attr->initialized_size =
				    cpu_to_sle64(old_initialized_size);
				err =
				    ntfs_mft_record_write(vol,
							  ctx->ntfs_ino->mft_no,
							  ctx->mrec);
			}
		}
		if (err) {
			/*
			 * FIXME: At this stage could try to recover by filling
			 * old_initialized_size -> new_initialized_size with
			 * data or at least zeroes. (AIA)
			 */
			antfs_logger(vol->dev->d_sb->s_id,
					"Eeek! Failed to recover from "
					"error. Leaving metadata in "
					"inconsistent state! Run chkdsk!");
		}
	}
	if (!IS_ERR_OR_NULL(ctx))
		ntfs_attr_put_search_ctx(ctx);
	/* Update mapping pairs if needed. */
	if (NAttrRunlistDirty(na))
		ntfs_attr_update_mapping_pairs(na, 0);
	/* Restore original data_size if needed. */
	if (need_to.undo_data_size
	    && ntfs_attr_truncate_i(na, old_data_size, HOLES_OK))
		antfs_log_error("Failed to restore data_size");

	total = eo;
	goto out;
}

s64 ntfs_attr_pwrite(struct ntfs_attr *na, const s64 pos, s64 count,
		     const void *b)
{
	s64 total = 0;
	s64 written = 0;

	antfs_log_enter("Entering for inode %lld, attr 0x%x, pos 0x%llx, count "
			"0x%llx.", (long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)pos,
			(long long)count);

	if (IS_ERR_OR_NULL(na) || IS_ERR_OR_NULL(na->ni) ||
	    !na->ni->vol || !b || pos < 0 || count < 0) {
		written = -EINVAL;
		antfs_log_error("EINVAL");
		goto out;
	}

	/*
	 * Compressed attributes may be written partially, so
	 * we may have to iterate.
	 */
	do {
		written = ntfs_attr_pwrite_i(na, pos + total,
					     count - total,
					     (const u8 *)b + total);
		if (written > 0)
			total += written;
	} while ((written > 0) && (total < count));
out:

	total = written < 0 ? written : total;
	antfs_log_leave("Exit: %lld", total);
	/* TODO: does that work? or return written if it is error code? */
	/*---return (total > 0 ? total : written); ---*/
	/*--- return (written < 0 ? written : total); ---*/
	return total;
}

int ntfs_attr_pclose(struct ntfs_attr *na)
{
	s64 ofs;
	int failed;
	int ok = 1;
	VCN update_from = -1;
	struct ntfs_volume *vol;
	struct ntfs_attr_search_ctx *ctx = NULL;
	struct runlist_element *rl;
	int err = 0;
	int compressed_part;
	int compressed = 0;

	if (!na || !na->ni || !na->ni->vol) {
		err = -EINVAL;
		antfs_log_error("Wrong arguments");
		goto err_set;
	}

	antfs_log_enter("Entering for inode 0x%llx, attr 0x%x.",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type));

	vol = na->ni->vol;
	na->unused_runs = 0;
	compressed = (na->data_flags & ATTR_COMPRESSION_MASK)
	    != const_cpu_to_le16(0);
	/*
	 * Encrypted non-resident attributes are not supported.  We return
	 * access denied, which is what Windows NT4 does, too.
	 */
	if (NAttrEncrypted(na) && NAttrNonResident(na)) {
		err = -EACCES;
		goto err_set;
	}
	/* If this is not a compressed attribute get out */
	/* same if it is resident */
	if (!compressed || !NAttrNonResident(na))
		goto out;

	/* safety check : no recursion on close */
	if (NAttrComprClosing(na)) {
		err = -EIO;
		antfs_log_error("Bad ntfs_attr_pclose"
				" recursion on inode %lld",
				(long long)na->ni->mft_no);
		goto out;
	}
	NAttrSetComprClosing(na);
	/*
	 * For a compressed attribute, we must be sure there are two
	 * available entries, so reserve them before it gets too late.
	 */
	err = ntfs_attr_map_whole_runlist(na);
	if (err)
		goto err_out;
	rl = ntfs_rl_extend(na, na->rl, 2);
	if (IS_ERR(rl)) {
		err = PTR_ERR(rl);
		goto err_out;
	}
	na->rl = rl;
	na->unused_runs = 2;
	/* Find the runlist element containing the terminal vcn. */
	rl = ntfs_attr_find_vcn(na,
				(na->initialized_size -
				 1) >> vol->cluster_size_bits);
	if (IS_ERR(rl)) {
		/*
		 * If the vcn is not present it is an out of bounds write.
		 * However, we have already written the last byte uncompressed,
		 * so getting this here must be an error of some kind.
		 */
		err = PTR_ERR(rl);
		if (err == -ENOENT) {
			err = -EIO;
			antfs_log_error("Failed to find VCN #5");
		}
		goto err_out;
	}
	/*
	 * Scatter the data from the linear data buffer to the volume. Note, a
	 * partial final vcn is taken care of by the @count capping of write
	 * length.
	 */
	compressed_part = 0;
	if (rl->lcn >= 0) {
		struct runlist_element *xrl;

		xrl = rl;
		do {
			xrl++;
		} while (xrl->lcn >= 0);
		compressed_part = (-xrl->length)
		    & (na->compression_block_clusters - 1);
	} else if (rl->lcn == (LCN) LCN_HOLE) {
		if (rl->length < na->compression_block_clusters)
			compressed_part
			    = na->compression_block_clusters - rl->length;
		else
			compressed_part = na->compression_block_clusters;
	}
	/* done, if the last block set was compressed */
	if (compressed_part)
		goto out;

	ofs = na->initialized_size - (rl->vcn << vol->cluster_size_bits);

	if (rl->lcn == LCN_RL_NOT_MAPPED) {
		rl = ntfs_attr_find_vcn(na, rl->vcn);
		if (IS_ERR(rl)) {
			err = PTR_ERR(rl);
			if (err == -ENOENT) {
				err = -EIO;
				antfs_log_error("Failed to find VCN#6");
			}
			goto rl_err_out;
		}
		/* Needed for case when runs merged. */
		ofs =
		    na->initialized_size - (rl->vcn << vol->cluster_size_bits);
	}
	if (!rl->length) {
		err = -EIO;
		antfs_log_error("Zero run length");
		goto rl_err_out;
	}
	if (rl->lcn < (LCN) 0) {
		if (rl->lcn != (LCN) LCN_HOLE) {
			err = -EIO;
			antfs_log_error("Unexpected LCN (%lld)",
					(long long)rl->lcn);
			goto rl_err_out;
		}
		err = ntfs_attr_fill_hole(na, (s64) 0, &ofs, &rl, &update_from);
		if (!err)
			goto err_out;
	}
	while (rl->length && (ofs >= (rl->length << vol->cluster_size_bits))) {
		ofs -= rl->length << vol->cluster_size_bits;
		rl++;
	}

/* retry: */
	failed = 0;
	if (update_from < 0)
		update_from = 0;
	if (!NVolReadOnly(vol))
		failed = ntfs_compressed_close(na, rl, ofs, &update_from);
	if (failed) {
		/* If the syscall was interrupted, try again. */
		/* TODO: Can this ever happen? */
		goto rl_err_out;
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	/* Update mapping pairs if needed. */
	if (NAttrFullyMapped(na))
		err = ntfs_attr_update_mapping_pairs(na, update_from);
	if (!err) {
		/*
		 * FIXME: trying to recover by goto rl_err_out;
		 * could cause driver hang by infinite looping.
		 */
		ok = 0;
		goto out;
	}
out:
	NAttrClearComprClosing(na);
	antfs_log_leave();
	return ok ? 0 : err;
rl_err_out:
	/*
	 * need not restore old sizes, only compressed_size
	 * can have changed. It has been set according to
	 * the current runlist while updating the mapping pairs,
	 * and must be kept consistent with the runlists.
	 */
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	/* Update mapping pairs if needed. */
	if (NAttrFullyMapped(na))
		ntfs_attr_update_mapping_pairs(na, 0);
err_set:
	ok = 0;
	goto out;
}

/**
 * ntfs_attr_mst_pread - multi sector transfer protected ntfs attribute read
 * @na:           multi sector transfer protected ntfs attribute to read from
 * @pos:          byte position in the attribute to begin reading from
 * @bk_cnt:       number of mst protected blocks to read
 * @bk_size_bits: bit shift size of each mst protected block in bytes
 * @dst:          output data buffer
 *
 * This function will read @bk_cnt blocks of size @bk_size bytes each starting
 * at offset @pos from the ntfs attribute @na into the data buffer @b.
 *
 * On success, the multi sector transfer fixups are applied and the number of
 * read blocks is returned. If this number is lower than @bk_cnt this means
 * that the read has either reached end of attribute or that an error was
 * encountered during the read so that the read is partial. 0 means end of
 * attribute or nothing to read (also return 0 when @bk_cnt or @bk_size are 0).
 *
 * On error and nothing has been read, the return code of ntfs_attr_pread() or
 * -EINVAL in case of invalid arguments is returned.
 *
 * NOTE: If an incomplete multi sector transfer is detected the magic is
 * changed to BAAD but no error is returned, i.e. it is possible that any of
 * the returned blocks have multi sector transfer errors. This should be
 * detected by the caller by checking each block with is_baad_recordp(&block).
 * The reasoning is that we want to fixup as many blocks as possible and we
 * want to return even bad ones to the caller so, e.g. in case of ntfsck, the
 * errors can be repaired.
 */
s64 ntfs_attr_mst_pread(struct ntfs_attr *na, const s64 pos, const s64 bk_cnt,
			const u8 bk_size_bits, void *dst, bool warn_ov)
{
	s64 br;
	u8 *end;
	bool warn;

	antfs_log_enter("inode 0x%llx, attr type 0x%x, pos 0x%llx.",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)pos);
	if (bk_cnt < 0 || bk_size_bits < NTFS_BLOCK_SIZE_BITS) {
		antfs_log_error("Wrong arguments");
		return -EINVAL;
	}
	/* Just use this as a global lock for everything not user. */
	br = (s64)ntfs_mftbmp_lock(na->ni->vol);
	if (br)
		goto out_nolock;

	br = ntfs_attr_pread(na, pos, bk_cnt << bk_size_bits, dst);
	if (br <= 0)
		goto out;

	br >>= bk_size_bits;
	/* log errors unless silenced */
	if (warn_ov)
		warn = !na->ni || !na->ni->vol || !NVolNoFixupWarn(na->ni->vol);
	else
		warn = false;

	/* ntfs-3g warns about but otherwise IGNOREs mst errors here.
	 * This is important to prevent endless error loops when allocating mft
	 * records. We also ignore errors in this case, but not on other
	 * occasions!
	 */
	for (end = (u8 *) dst + (br << bk_size_bits); (u8 *) dst < end;
	     dst = (u8 *) dst + (1 << bk_size_bits)) {
		int err = ntfs_mst_post_read_fixup_warn((struct NTFS_RECORD *)
				dst, 1 << bk_size_bits, warn);

		if (err && warn_ov) {
			br = (s64)err;
			goto out;
		}
	}

	/* Finally, return the number of blocks read. */
out:
	ntfs_mftbmp_unlock(na->ni->vol);
out_nolock:
	antfs_log_leave();
	return br;
}

/**
 * ntfs_attr_mst_pwrite - multi sector transfer protected ntfs attribute write
 * @na:           multi sector transfer protected ntfs attribute to write to
 * @pos:          position in the attribute to write to
 * @bk_cnt:       number of mst protected blocks to write
 * @bk_size_bits: bit shift size of each mst protected block in bytes
 * @src:          data buffer to write to disk
 *
 * This function will write @bk_cnt blocks of size @bk_size bytes each from
 * data buffer @b to multi sector transfer (mst) protected ntfs attribute @na
 * at position @pos.
 *
 * On success, return the number of successfully written blocks. If this number
 * is lower than @bk_cnt this means that an error was encountered during the
 * write so that the write is partial. 0 means nothing was written (also
 * return 0 when @bk_cnt or @bk_size are 0).
 *
 * On error and nothing has been written, the return code of ntfs_attr_pwrite(),
 * or -EINVAL in case of invalid arguments is returned.
 *
 * NOTE: We mst protect the data, write it, then mst deprotect it using a quick
 * deprotect algorithm (no checking). This saves us from making a copy before
 * the write and at the same time causes the usn to be incremented in the
 * buffer. This conceptually fits in better with the idea that cached data is
 * always deprotected and protection is performed when the data is actually
 * going to hit the disk and the cache is immediately deprotected again
 * simulating an mst read on the written data. This way cache coherency is
 * achieved.
 */
s64 ntfs_attr_mst_pwrite(struct ntfs_attr *na, const s64 pos, s64 bk_cnt,
			 const u8 bk_size_bits, void *src)
{
	s64 i, ret = 0;

	/* Note na->ni->mft_no is 0 here when we come from
	 * ntfs_mft_record_init. */
	antfs_log_enter
	    ("Entering for inode 0x%llx, attr type 0x%x, pos 0x%llx "
	     "bk_cnt 0x%llx, bk_size_bits 0x%x",
	     (unsigned long long)na->ni->mft_no, le32_to_cpu(na->type),
	     (long long)pos, (long long)bk_cnt, (unsigned int)bk_size_bits);
	if (bk_cnt < 0 || bk_size_bits < NTFS_BLOCK_SIZE_BITS || !src) {
		antfs_log_error("EINVAL");
		ret = -EINVAL;
		goto out_nolock;
	}
	if (!bk_cnt)
		goto out_nolock;
	/* Just use this as a global lock for everything not user. */
	ret = (s64)ntfs_mftbmp_lock(na->ni->vol);
	if (ret)
		goto out_nolock;

	/* Prepare data for writing. */
	for (i = 0; i < bk_cnt; ++i) {
		int err;

		err = ntfs_mst_pre_write_fixup((struct NTFS_RECORD *)
					       ((u8 *) src +
						(i << bk_size_bits)),
					       1 << bk_size_bits);
		if (err < 0) {
			/* Abort write at this position. */
			antfs_log_error("#1");
			if (!i) {
				ret = (s64)err;
				goto out;
			}
			bk_cnt = i;
			break;
		}
	}
	/* Write the prepared data. */
	ret = ntfs_attr_pwrite(na, pos, bk_cnt << bk_size_bits, src);
	if (ret <= 0)
		antfs_log_error("written=%lld", (long long)ret);
	/* Quickly deprotect the data again. */
	for (i = 0; i < bk_cnt; ++i)
		ntfs_mst_post_write_fixup((struct NTFS_RECORD *) ((u8 *) src +
						(i << bk_size_bits)));
	if (ret <= 0)
		goto out;
	/* Finally, return the number of complete blocks written. */
	ret >>= bk_size_bits;
out:
	ntfs_mftbmp_unlock(na->ni->vol);
out_nolock:
	antfs_log_leave("written=%lld", (long long)ret);
	return ret;
}

/**
 * ntfs_attr_find - find (next) attribute in mft record
 * @type:	attribute type to find
 * @name:	attribute name to find (optional, i.e. NULL means don't care)
 * @name_len:	attribute name length (only needed if @name present)
 * @ic:		IGNORE_CASE or CASE_SENSITIVE (ignored if @name not present)
 * @val:	attribute value to find (optional, resident attributes only)
 * @val_len:	attribute value length
 * @ctx:	search context with mft record and attribute to search from
 *
 * You shouldn't need to call this function directly. Use lookup_attr() instead.
 *
 * ntfs_attr_find() takes a search context @ctx as parameter and searches the
 * mft record specified by @ctx->mrec, beginning at @ctx->attr, for an
 * attribute of @type, optionally @name and @val. If found, ntfs_attr_find()
 * returns 0 and @ctx->attr will point to the found attribute.
 *
 * If not found, ntfs_attr_find() returns -ENOENT and @ctx->attr will point to
 * the attribute before which the attribute being searched for would need to be
 * inserted if such an action were to be desired.
 *
 * On actual error, ntfs_attr_find() returns the error code but not -ENOENT. In
 * this case @ctx->attr is undefined and in particular do not rely on it not
 * changing.
 *
 * If @ctx->is_first is TRUE, the search begins with @ctx->attr itself. If it
 * is FALSE, the search begins after @ctx->attr.
 *
 * If @type is AT_UNUSED, return the first found attribute, i.e. one can
 * enumerate all attributes by setting @type to AT_UNUSED and then calling
 * ntfs_attr_find() repeatedly until it returns -ENOENT to indicate that there
 * are no more entries. During the enumeration, each successful call of
 * ntfs_attr_find() will return the next attribute in the mft record @ctx->mrec.
 *
 * If @type is AT_END, seek to the end and return -ENOENT. AT_END is not a valid
 * attribute, its length is zero for example, thus it is safer to return error
 * instead of success in this case. This also allows us to interoperate cleanly
 * with ntfs_external_attr_find().
 *
 * If @name is AT_UNNAMED search for an unnamed attribute. If @name is present
 * but not AT_UNNAMED search for a named attribute matching @name. Otherwise,
 * match both named and unnamed attributes.
 *
 * If @ic is IGNORE_CASE, the @name comparison is not case sensitive and
 * @ctx->ntfs_ino must be set to the ntfs inode to which the mft record
 * @ctx->mrec belongs. This is so we can get at the ntfs volume and hence at
 * the upcase table. If @ic is CASE_SENSITIVE, the comparison is case
 * sensitive. When @name is present, @name_len is the @name length in Unicode
 * characters.
 *
 * If @name is not present (NULL), we assume that the unnamed attribute is
 * being searched for.
 *
 * Finally, the resident attribute value @val is looked for, if present.
 * If @val is not present (NULL), @val_len is ignored.
 *
 * ntfs_attr_find() only searches the specified mft record and it ignores the
 * presence of an attribute list attribute (unless it is the one being searched
 * for, obviously). If you need to take attribute lists into consideration, use
 * ntfs_attr_lookup() instead (see below). This also means that you cannot use
 * ntfs_attr_find() to search for extent records of non-resident attributes, as
 * extents with lowest_vcn != 0 are usually described by the attribute list
 * attribute only. - Note that it is possible that the first extent is only in
 * the attribute list while the last extent is in the base mft record, so don't
 * rely on being able to find the first extent in the base mft record.
 *
 * Warning: Never use @val when looking for attribute types which can be
 *	    non-resident as this most likely will result in a crash!
 */
static int ntfs_attr_find(const enum ATTR_TYPES type, const ntfschar *name,
			  const u32 name_len, const enum IGNORE_CASE_BOOL ic,
			  const u8 *val, const u32 val_len,
			  struct ntfs_attr_search_ctx *ctx)
{
	struct ATTR_RECORD *a;
	struct ntfs_volume *vol;
	ntfschar *upcase;
	u32 upcase_len;

	antfs_log_enter("attribute type 0x%x", le32_to_cpu(type));

	if (ctx->ntfs_ino) {
		vol = ctx->ntfs_ino->vol;
		upcase = vol->upcase;
		upcase_len = vol->upcase_len;
	} else {
		if (name && name != AT_UNNAMED) {
			antfs_log_error("Invalid argument");
			return -EINVAL;
		}
		vol = NULL;
		upcase = NULL;
		upcase_len = 0;
	}
	/*
	 * Iterate over attributes in mft record starting at @ctx->attr, or the
	 * attribute following that, if @ctx->is_first is TRUE.
	 */
	if (ctx->is_first) {
		a = ctx->attr;
		ctx->is_first = FALSE;
	} else
		a = (struct ATTR_RECORD *) ((char *)ctx->attr +
				     le32_to_cpu(ctx->attr->length));
	for (;; a = (struct ATTR_RECORD *) ((char *)a +
		le32_to_cpu(a->length))) {
		if (p2n(a) < p2n(ctx->mrec) || (char *)a > (char *)ctx->mrec +
		    le32_to_cpu(ctx->mrec->bytes_allocated))
			break;
		ctx->attr = a;
		if (((type != AT_UNUSED) && (le32_to_cpu(a->type) >
					     le32_to_cpu(type))) ||
		    (a->type == AT_END)) {
			antfs_log_leave("ENOENT");
			return -ENOENT;
		}
		if (!a->length)
			break;
		/* If this is an enumeration return this attribute. */
		if (type == AT_UNUSED) {
			antfs_log_leave("AT_UNUSED");
			return 0;
		}
		if (a->type != type)
			continue;
		/*
		 * If @name is AT_UNNAMED we want an unnamed attribute.
		 * If @name is present, compare the two names.
		 * Otherwise, match any attribute.
		 */
		if (name == AT_UNNAMED) {
			/* The search failed if the found attribute is named. */
			if (a->name_length) {
				antfs_log_leave
				    ("AT_UNNAMED but a->name_length");
				return -ENOENT;
			}
		} else if (name) {
			register int rc = ntfs_names_full_collate(name,
				name_len, (ntfschar *) ((char *) a +
				le16_to_cpu(a->name_offset)), a->name_length,
				ic, upcase, upcase_len);
			if (rc > 0) {
				/* If the strings are not equal,
				 * continue search. */
				continue;
			} else if (rc < 0) {
				/*
				 * If @name collates before a->name,
				 * there is no matching attribute.
				 */
				antfs_log_leave("Not found");
				return -ENOENT;
			}
		}
		/*
		 * The names match or @name not present and attribute is
		 * unnamed. If no @val specified, we have found the attribute
		 * and are done.
		 */
		if (!val) {
			antfs_log_leave("Found");
			return 0;
		}
		/* @val is present; compare values. */
		else {
			register int rc;

			rc = memcmp(val,
				    (char *)a + le16_to_cpu(a->value_offset),
				    min(val_len, le32_to_cpu(a->value_length)));
			/*
			 * If @val collates before the current attribute's
			 * value, there is no matching attribute.
			 */
			if (!rc) {
				register u32 avl;
				avl = le32_to_cpu(a->value_length);
				if (val_len == avl) {
					antfs_log_leave("Found");
					return 0;
				}
				if (val_len < avl) {
					antfs_log_leave
					    ("val_len < avl (mem corrupt?)");
					return -ENOENT;
				}
			} else if (rc < 0) {
				antfs_log_leave("Not found (memcmp)");
				return -ENOENT;
			}
		}
	}
	antfs_log_error("Corrupt inode (%lld)",
			ctx->ntfs_ino ? (long long)ctx->ntfs_ino->mft_no : -1);
	antfs_log_leave("-EIO");
	return -EIO;
}

void ntfs_attr_name_free(char **name)
{
	if (*name) {
		ntfs_free(*name);
		*name = NULL;
	}
}

char *ntfs_attr_name_get(const ntfschar *uname, const int uname_len)
{
	char *name = NULL;
	int name_len;

	name_len = ntfs_ucstombs(uname, uname_len, &name, 0);
	if (name_len < 0) {
		antfs_log_error("ntfs_ucstombs");
		return NULL;

	} else if (name_len > 0)
		return name;

	ntfs_attr_name_free(&name);
	return NULL;
}

/**
 * ntfs_external_attr_find - find an attribute in the attribute list of an inode
 * @type:	attribute type to find
 * @name:	attribute name to find (optional, i.e. NULL means don't care)
 * @name_len:	attribute name length (only needed if @name present)
 * @ic:		IGNORE_CASE or CASE_SENSITIVE (ignored if @name not present)
 * @lowest_vcn:	lowest vcn to find (optional, non-resident attributes only)
 * @val:	attribute value to find (optional, resident attributes only)
 * @val_len:	attribute value length
 * @ctx:	search context with mft record and attribute to search from
 *
 * You shouldn't need to call this function directly. Use ntfs_attr_lookup()
 * instead.
 *
 * Find an attribute by searching the attribute list for the corresponding
 * attribute list entry. Having found the entry, map the mft record for read
 * if the attribute is in a different mft record/inode, find the attribute in
 * there and return it.
 *
 * If @type is AT_UNUSED, return the first found attribute, i.e. one can
 * enumerate all attributes by setting @type to AT_UNUSED and then calling
 * ntfs_external_attr_find() repeatedly until it returns -ENOENT to indicate
 * that there are no more entries. During the enumeration, each successful call
 * of ntfs_external_attr_find() will return the next attribute described by the
 * attribute list of the base mft record described by the search context @ctx.
 *
 * If @type is AT_END, seek to the end of the base mft record ignoring the
 * attribute list completely and return -ENOENT. AT_END is not a valid
 * attribute, its length is zero for example, thus it is safer to return error
 * instead of success in this case.
 *
 * If @name is AT_UNNAMED search for an unnamed attribute. If @name is present
 * but not AT_UNNAMED search for a named attribute matching @name. Otherwise,
 * match both named and unnamed attributes.
 *
 * On first search @ctx->ntfs_ino must be the inode of the base mft record and
 * @ctx must have been obtained from a call to ntfs_attr_get_search_ctx().
 * On subsequent calls, @ctx->ntfs_ino can be any extent inode, too
 * (@ctx->base_ntfs_ino is then the base inode).
 *
 * After finishing with the attribute/mft record you need to call
 * ntfs_attr_put_search_ctx() to cleanup the search context (unmapping any
 * mapped extent inodes, etc).
 *
 * Return 0 if the search was successful and error code if not.
 *
 * On success, @ctx->attr is the found attribute, it is in mft record
 * @ctx->mrec, and @ctx->al_entry is the attribute list entry for this
 * attribute with @ctx->base_* being the base mft record to which @ctx->attr
 * belongs.
 *
 * On error ENOENT, i.e. attribute not found, @ctx->attr is set to the
 * attribute which collates just after the attribute being searched for in the
 * base ntfs inode, i.e. if one wants to add the attribute to the mft record
 * this is the correct place to insert it into, and if there is not enough
 * space, the attribute should be placed in an extent mft record.
 * @ctx->al_entry points to the position within @ctx->base_ntfs_ino->attr_list
 * at which the new attribute's attribute list entry should be inserted.  The
 * other @ctx fields, base_ntfs_ino, base_mrec, and base_attr are set to NULL.
 * The only exception to this is when @type is AT_END, in which case
 * @ctx->al_entry is set to NULL also (see above).
 *
 * The following error codes are defined:
 *	ENOENT	Attribute not found, not an error as such.
 *	EINVAL	Invalid arguments.
 *	EIO	I/O error or corrupt data structures found.
 *	ENOMEM	Not enough memory to allocate necessary buffers.
 */
static int ntfs_external_attr_find(enum ATTR_TYPES type, const ntfschar *name,
				   const u32 name_len,
				   const enum IGNORE_CASE_BOOL ic,
				   const VCN lowest_vcn, const u8 *val,
				   const u32 val_len,
				   struct ntfs_attr_search_ctx *ctx)
{
	struct ntfs_inode *base_ni, *ni;
	struct ntfs_volume *vol;
	struct ATTR_LIST_ENTRY *al_entry, *next_al_entry;
	u8 *al_start, *al_end;
	struct ATTR_RECORD *a;
	ntfschar *al_name;
	u32 al_name_len;
	bool is_first_search = FALSE;

	ni = ctx->ntfs_ino;
	base_ni = ctx->base_ntfs_ino;
	antfs_log_enter("inode %lld, attribute type 0x%x.",
			(unsigned long long)ni->mft_no, le32_to_cpu(type));
	if (!base_ni) {
		/* First call happens with the base mft record. */
		base_ni = ctx->base_ntfs_ino = ctx->ntfs_ino;
		ctx->base_mrec = ctx->mrec;
	}
	if (ni == base_ni)
		ctx->base_attr = ctx->attr;
	if (type == AT_END)
		goto not_found;
	vol = base_ni->vol;
	al_start = base_ni->attr_list;
	al_end = al_start + base_ni->attr_list_size;
	if (!ctx->al_entry) {
		ctx->al_entry = (struct ATTR_LIST_ENTRY *) al_start;
		is_first_search = TRUE;
	}
	/*
	 * Iterate over entries in attribute list starting at @ctx->al_entry,
	 * or the entry following that, if @ctx->is_first is TRUE.
	 */
	if (ctx->is_first) {
		al_entry = ctx->al_entry;
		ctx->is_first = FALSE;
		/*
		 * If an enumeration and the first attribute is higher than
		 * the attribute list itself, need to return the attribute list
		 * attribute.
		 */
		if ((type == AT_UNUSED) && is_first_search &&
		    le32_to_cpu(al_entry->type) >
		    le32_to_cpu(AT_ATTRIBUTE_LIST))
			goto find_attr_list_attr;
	} else {
		al_entry = (struct ATTR_LIST_ENTRY *) ((char *)ctx->al_entry +
						le16_to_cpu(ctx->al_entry->
							    length));
		/*
		 * If this is an enumeration and the attribute list attribute
		 * is the next one in the enumeration sequence, just return the
		 * attribute list attribute from the base mft record as it is
		 * not listed in the attribute list itself.
		 */
		if ((type == AT_UNUSED) && le32_to_cpu(ctx->al_entry->type) <
		    le32_to_cpu(AT_ATTRIBUTE_LIST) &&
		    le32_to_cpu(al_entry->type) >
		    le32_to_cpu(AT_ATTRIBUTE_LIST)) {
			int rc;
find_attr_list_attr:

			/* Check for bogus calls. */
			if (name || name_len || val || val_len || lowest_vcn) {
				antfs_log_error("Wrong arguments");
				return -EINVAL;
			}

			/* We want the base record. */
			ctx->ntfs_ino = base_ni;
			ctx->mrec = ctx->base_mrec;
			ctx->is_first = TRUE;
			/* Sanity checks are performed elsewhere. */
			ctx->attr = (struct ATTR_RECORD *) ((u8 *) ctx->mrec +
						     le16_to_cpu(ctx->mrec->
								 attrs_offset));

			/* Find the attribute list attribute. */
			rc = ntfs_attr_find(AT_ATTRIBUTE_LIST, NULL, 0,
					    IGNORE_CASE, NULL, 0, ctx);

			/*
			 * Setup the search context so the correct
			 * attribute is returned next time round.
			 */
			ctx->al_entry = al_entry;
			ctx->is_first = TRUE;

			/* Got it. Done. */
			if (!rc)
				return 0;

			/* Error! If other than not found return it. */
			if (rc != -ENOENT)
				return rc;

			/* log? Not found?!? Absurd! */
			antfs_log_error("Attribute list wasn't found");
			return -EIO;
		}
	}
	for (;; al_entry = next_al_entry) {
		/* Out of bounds check. */
		if ((u8 *) al_entry < base_ni->attr_list ||
		    (u8 *) al_entry > al_end)
			break;	/* Inode is corrupt. */
		ctx->al_entry = al_entry;
		/* Catch the end of the attribute list. */
		if ((u8 *) al_entry == al_end)
			goto not_found;
		if (!al_entry->length)
			break;
		if ((u8 *) al_entry + 6 > al_end || (u8 *) al_entry +
		    le16_to_cpu(al_entry->length) > al_end)
			break;
		next_al_entry = (struct ATTR_LIST_ENTRY *) ((u8 *) al_entry +
						     le16_to_cpu(al_entry->
								 length));
		if (type != AT_UNUSED) {
			if (le32_to_cpu(al_entry->type) > le32_to_cpu(type))
				goto not_found;
			if (type != al_entry->type)
				continue;
		}
		al_name_len = al_entry->name_length;
		al_name =
		    (ntfschar *) ((u8 *) al_entry + al_entry->name_offset);
		/*
		 * If !@type we want the attribute represented by this
		 * attribute list entry.
		 */
		if (type == AT_UNUSED)
			goto is_enumeration;
		/*
		 * If @name is AT_UNNAMED we want an unnamed attribute.
		 * If @name is present, compare the two names.
		 * Otherwise, match any attribute.
		 */
		if (name == AT_UNNAMED) {
			if (al_name_len)
				goto not_found;
		} else if (name) {
			int rc = ntfs_names_full_collate(name, name_len,
				    al_name, al_name_len, ic, vol->upcase,
				    vol->upcase_len);
			/* If the strings are not equal, continue search.
			 * If @name collates before al_name,
			 * there is no matching attribute.
			 */
			if (rc > 0)
				continue;
			else if (rc < 0)
				goto not_found;
		}
		/*
		 * The names match or @name not present and attribute is
		 * unnamed. Now check @lowest_vcn. Continue search if the
		 * next attribute list entry still fits @lowest_vcn. Otherwise
		 * we have reached the right one or the search has failed.
		 */
		if (lowest_vcn && (u8 *) next_al_entry >= al_start &&
		    (u8 *) next_al_entry + 6 < al_end &&
		    (u8 *) next_al_entry + le16_to_cpu(next_al_entry->length) <=
		    al_end
		    && sle64_to_cpu(next_al_entry->lowest_vcn) <= lowest_vcn
		    && next_al_entry->type == al_entry->type
		    && next_al_entry->name_length == al_name_len
		    && ntfs_names_are_equal((ntfschar *) ((char *)
							  next_al_entry +
							  next_al_entry->
							  name_offset),
					    next_al_entry->name_length, al_name,
					    al_name_len, CASE_SENSITIVE,
					    vol->upcase, vol->upcase_len))
			continue;
is_enumeration:
		if (MREF_LE(al_entry->mft_reference) == ni->mft_no) {
			if (MSEQNO_LE(al_entry->mft_reference) !=
			    le16_to_cpu(ni->mrec->sequence_number)) {
				antfs_log_error("Found stale mft reference in "
						"attribute list!");
				break;
			}
		} else {	/* Mft references do not match. */
			/* Do we want the base record back? */
			if (MREF_LE(al_entry->mft_reference) ==
			    base_ni->mft_no) {
				ni = ctx->ntfs_ino = base_ni;
				ctx->mrec = ctx->base_mrec;
			} else {
				/* We want an extent record. */
				ni = ntfs_extent_inode_open(base_ni,
							    al_entry->
							    mft_reference);
				if (IS_ERR(ni))
					break;
				ctx->ntfs_ino = ni;
				ctx->mrec = ni->mrec;
			}
		}
		a = ctx->attr = (struct ATTR_RECORD *) ((char *)ctx->mrec +
						 le16_to_cpu(ctx->mrec->
							     attrs_offset));
		/*
		 * ctx->ntfs_ino, ctx->mrec, and ctx->attr now point to the
		 * mft record containing the attribute represented by the
		 * current al_entry.
		 *
		 * We could call into ntfs_attr_find() to find the right
		 * attribute in this mft record but this would be less
		 * efficient and not quite accurate as ntfs_attr_find() ignores
		 * the attribute instance numbers for example which become
		 * important when one plays with attribute lists. Also, because
		 * a proper match has been found in the attribute list entry
		 * above, the comparison can now be optimized. So it is worth
		 * re-implementing a simplified ntfs_attr_find() here.
		 *
		 * Use a manual loop so we can still use break and continue
		 * with the same meanings as above.
		 */
do_next_attr_loop:
		if ((char *)a < (char *)ctx->mrec
		    || (char *)a >
		    (char *)ctx->mrec + le32_to_cpu(ctx->mrec->bytes_allocated))
			break;
		if (a->type == AT_END)
			continue;
		if (!a->length)
			break;
		if (al_entry->instance != a->instance)
			goto do_next_attr;
		/*
		 * If the type and/or the name are/is mismatched between the
		 * attribute list entry and the attribute record, there is
		 * corruption so we break and return error EIO.
		 */
		if (al_entry->type != a->type)
			break;
		if (!ntfs_names_are_equal((ntfschar *) ((char *)a +
		    le16_to_cpu(a->name_offset)), a->name_length, al_name,
		    al_name_len, CASE_SENSITIVE, vol->upcase, vol->upcase_len))
			break;
		ctx->attr = a;
		/*
		 * If no @val specified or @val specified and it matches, we
		 * have found it! Also, if !@type, it is an enumeration, so we
		 * want the current attribute.
		 */
		if ((type == AT_UNUSED) || !val || (!a->non_resident &&
		    le32_to_cpu(a->value_length) == val_len &&
		    !memcmp((char *)a + le16_to_cpu(a->value_offset), val,
		    val_len))) {
			return 0;
		}
do_next_attr:
		/* Proceed to the next attribute in the current mft record. */
		a = (struct ATTR_RECORD *) ((char *)a + le32_to_cpu(a->length));
		goto do_next_attr_loop;
	}
	if (ni != base_ni) {
		ctx->ntfs_ino = base_ni;
		ctx->mrec = ctx->base_mrec;
		ctx->attr = ctx->base_attr;
	}
	/* log? have a corrupt inode */
	antfs_log_error("Inode is corrupt (%lld)", (long long)base_ni->mft_no);
	return -EIO;
not_found:
	/*
	 * If we were looking for AT_END or we were enumerating and reached the
	 * end, we reset the search context @ctx and use ntfs_attr_find() to
	 * seek to the end of the base mft record.
	 */
	if (type == AT_UNUSED || type == AT_END) {
		ntfs_attr_reinit_search_ctx(ctx);
		return ntfs_attr_find(AT_END, name, name_len, ic, val, val_len,
				      ctx);
	}
	/*
	 * The attribute wasn't found.  Before we return, we want to ensure
	 * @ctx->mrec and @ctx->attr indicate the position at which the
	 * attribute should be inserted in the base mft record.  Since we also
	 * want to preserve @ctx->al_entry we cannot reinitialize the search
	 * context using ntfs_attr_reinit_search_ctx() as this would set
	 * @ctx->al_entry to NULL.  Thus we do the necessary bits manually (see
	 * ntfs_attr_init_search_ctx() below).  Note, we _only_ preserve
	 * @ctx->al_entry as the remaining fields (base_*) are identical to
	 * their non base_ counterparts and we cannot set @ctx->base_attr
	 * correctly yet as we do not know what @ctx->attr will be set to by
	 * the call to ntfs_attr_find() below.
	 */
	ctx->mrec = ctx->base_mrec;
	ctx->attr = (struct ATTR_RECORD *) ((u8 *) ctx->mrec +
				     le16_to_cpu(ctx->mrec->attrs_offset));
	ctx->is_first = TRUE;
	ctx->ntfs_ino = ctx->base_ntfs_ino;
	ctx->base_ntfs_ino = NULL;
	ctx->base_mrec = NULL;
	ctx->base_attr = NULL;
	/*
	 * In case there are multiple matches in the base mft record, need to
	 * keep enumerating until we get an attribute not found response (or
	 * another error), otherwise we would keep returning the same attribute
	 * over and over again and all programs using us for enumeration would
	 * lock up in a tight loop.
	 */
	{
		int ret;

		do {
			ret = ntfs_attr_find(type, name, name_len, ic, val,
					     val_len, ctx);
		} while (!ret);
		return ret;
	}
}

/**
 * ntfs_attr_lookup - find an attribute in an ntfs inode
 * @type:	attribute type to find
 * @name:	attribute name to find (optional, i.e. NULL means don't care)
 * @name_len:	attribute name length (only needed if @name present)
 * @ic:		IGNORE_CASE or CASE_SENSITIVE (ignored if @name not present)
 * @lowest_vcn:	lowest vcn to find (optional, non-resident attributes only)
 * @val:	attribute value to find (optional, resident attributes only)
 * @val_len:	attribute value length
 * @ctx:	search context with mft record and attribute to search from
 *
 * Find an attribute in an ntfs inode. On first search @ctx->ntfs_ino must
 * be the base mft record and @ctx must have been obtained from a call to
 * ntfs_attr_get_search_ctx().
 *
 * This function transparently handles attribute lists and @ctx is used to
 * continue searches where they were left off at.
 *
 * If @type is AT_UNUSED, return the first found attribute, i.e. one can
 * enumerate all attributes by setting @type to AT_UNUSED and then calling
 * ntfs_attr_lookup() repeatedly until it returns -ENOENT to indicate that there
 * are no more entries. During the enumeration, each successful call of
 * ntfs_attr_lookup() will return the next attribute, with the current attribute
 * being described by the search context @ctx.
 *
 * If @type is AT_END, seek to the end of the base mft record ignoring the
 * attribute list completely and return -ENOENT. AT_END is not a valid
 * attribute, its length is zero for example, thus it is safer to return error
 * instead of success in this case.  It should never be needed to do this, but
 * we implement the functionality because it allows for simpler code inside
 * ntfs_external_attr_find().
 *
 * If @name is AT_UNNAMED search for an unnamed attribute. If @name is present
 * but not AT_UNNAMED search for a named attribute matching @name. Otherwise,
 * match both named and unnamed attributes.
 *
 * After finishing with the attribute/mft record you need to call
 * ntfs_attr_put_search_ctx() to cleanup the search context (unmapping any
 * mapped extent inodes, etc).
 *
 * Return 0 if the search was successful and error code if not.
 *
 * On success, @ctx->attr is the found attribute, it is in mft record
 * @ctx->mrec, and @ctx->al_entry is the attribute list entry for this
 * attribute with @ctx->base_* being the base mft record to which @ctx->attr
 * belongs.  If no attribute list attribute is present @ctx->al_entry and
 * @ctx->base_* are NULL.
 *
 * On error ENOENT, i.e. attribute not found, @ctx->attr is set to the
 * attribute which collates just after the attribute being searched for in the
 * base ntfs inode, i.e. if one wants to add the attribute to the mft record
 * this is the correct place to insert it into, and if there is not enough
 * space, the attribute should be placed in an extent mft record.
 * @ctx->al_entry points to the position within @ctx->base_ntfs_ino->attr_list
 * at which the new attribute's attribute list entry should be inserted.  The
 * other @ctx fields, base_ntfs_ino, base_mrec, and base_attr are set to NULL.
 * The only exception to this is when @type is AT_END, in which case
 * @ctx->al_entry is set to NULL also (see above).
 *
 *
 * The following error codes are defined:
 *	ENOENT	Attribute not found, not an error as such.
 *	EINVAL	Invalid arguments.
 *	EIO	I/O error or corrupt data structures found.
 *	ENOMEM	Not enough memory to allocate necessary buffers.
 */
int ntfs_attr_lookup(const enum ATTR_TYPES type, const ntfschar *name,
		     const u32 name_len, const enum IGNORE_CASE_BOOL ic,
		     const VCN lowest_vcn, const u8 *val, const u32 val_len,
		     struct ntfs_attr_search_ctx *ctx)
{
	struct ntfs_inode *base_ni;
	int ret = -EINVAL;

	antfs_log_enter("attribute type 0x%x", le32_to_cpu(type));

	if (!ctx || !ctx->mrec || !ctx->attr || (name && name != AT_UNNAMED &&
		(!ctx->ntfs_ino || !ctx->ntfs_ino->vol ||
		 !ctx->ntfs_ino->vol->upcase ||
		 !ctx->ntfs_ino->vol->upcase_len))) {
		antfs_log_error("EINVAL");
		goto out;
	}

	if (ctx->base_ntfs_ino)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;
	if (!base_ni || !NInoAttrList(base_ni) || type == AT_ATTRIBUTE_LIST)
		ret =
		    ntfs_attr_find(type, name, name_len, ic, val, val_len, ctx);
	else
		ret = ntfs_external_attr_find(type, name, name_len, ic,
					      lowest_vcn, val, val_len, ctx);
out:
	antfs_log_leave("ret=%d", ret);
	return ret;
}

/**
 * ntfs_attr_position - find given or next attribute type in an ntfs inode
 * @type:	attribute type to start lookup
 * @ctx:	search context with mft record and attribute to search from
 *
 * Find an attribute type in an ntfs inode or the next attribute which is not
 * the AT_END attribute. Please see more details at ntfs_attr_lookup.
 *
 * Return 0 if the search was successful and error code if not.
 *
 * The following error codes are defined:
 *	EINVAL	Invalid arguments.
 *	EIO	I/O error or corrupt data structures found.
 *	ENOMEM	Not enough memory to allocate necessary buffers.
 *	ENOSPC	No attribute was found after 'type', only AT_END.
 */
int ntfs_attr_position(const enum ATTR_TYPES type,
		       struct ntfs_attr_search_ctx *ctx)
{
	int err;

	err = ntfs_attr_lookup(type, NULL, 0, CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (err) {
		if (err != -ENOENT)
			return err;
		if (ctx->attr->type == AT_END)
			return -ENOSPC;
	}
	return 0;
}

/**
 * ntfs_attr_init_search_ctx - initialize an attribute search context
 * @ctx:	attribute search context to initialize
 * @ni:		ntfs inode with which to initialize the search context
 * @mrec:	mft record with which to initialize the search context
 *
 * Initialize the attribute search context @ctx with @ni and @mrec.
 */
static void ntfs_attr_init_search_ctx(struct ntfs_attr_search_ctx *ctx,
				      struct ntfs_inode *ni,
				      struct MFT_RECORD *mrec)
{
	if (!mrec && ni)
		mrec = ni->mrec;

	ctx->mrec = mrec;
	/* Sanity checks are performed elsewhere. */
	if (!mrec) {
		antfs_log_error("mrec NULL!");
		ctx->attr = NULL;
	} else {
		ctx->attr =
		    (struct ATTR_RECORD *) ((u8 *) mrec +
				     le16_to_cpu(mrec->attrs_offset));
	}
	ctx->is_first = TRUE;
	ctx->ntfs_ino = ni;
	ctx->al_entry = NULL;
	ctx->base_ntfs_ino = NULL;
	ctx->base_mrec = NULL;
	ctx->base_attr = NULL;
}

/**
 * ntfs_attr_reinit_search_ctx - reinitialize an attribute search context
 * @ctx:	attribute search context to reinitialize
 *
 * Reinitialize the attribute search context @ctx.
 *
 * This is used when a search for a new attribute is being started to reset
 * the search context to the beginning.
 */
void ntfs_attr_reinit_search_ctx(struct ntfs_attr_search_ctx *ctx)
{
	if (!ctx->base_ntfs_ino) {
		/* No attribute list. */
		ctx->is_first = TRUE;
		/* Sanity checks are performed elsewhere. */
		ctx->attr = (struct ATTR_RECORD *) ((u8 *) ctx->mrec +
					     le16_to_cpu(ctx->mrec->
							 attrs_offset));
		/*
		 * This needs resetting due to ntfs_external_attr_find() which
		 * can leave it set despite having zeroed ctx->base_ntfs_ino.
		 */
		ctx->al_entry = NULL;
		return;
	}			/* Attribute list. */
	ntfs_attr_init_search_ctx(ctx, ctx->base_ntfs_ino, ctx->base_mrec);
	return;
}

/**
 * ntfs_attr_get_search_ctx - allocate/initialize a new attribute search context
 * @ni:		ntfs inode with which to initialize the search context
 * @mrec:	mft record with which to initialize the search context
 *
 * Allocate a new attribute search context, initialize it with @ni and @mrec,
 * and return it. Return error code if failed.
 *
 * @mrec can be NULL, in which case the mft record is taken from @ni.
 *
 * Note: For low level utilities which know what they are doing we allow @ni to
 * be NULL and @mrec to be set.  Do NOT do this unless you understand the
 * implications!!!  For example it is no longer safe to call ntfs_attr_lookup().
 */
struct ntfs_attr_search_ctx *ntfs_attr_get_search_ctx(struct ntfs_inode *ni,
					       struct MFT_RECORD *mrec)
{
	struct ntfs_attr_search_ctx *ctx;

	if (IS_ERR_OR_NULL(ni) && !mrec) {
		antfs_log_error("NULL arguments");
		ctx = ERR_PTR(-EINVAL);
		goto out;
	}
	ctx = ntfs_malloc(sizeof(struct ntfs_attr_search_ctx));
	if (ctx)
		ntfs_attr_init_search_ctx(ctx, ni, mrec);
	else
		ctx = ERR_PTR(-ENOMEM);
out:
	return ctx;
}

/**
 * ntfs_attr_put_search_ctx - release an attribute search context
 * @ctx:	attribute search context to free
 *
 * Release the attribute search context @ctx.
 */
void ntfs_attr_put_search_ctx(struct ntfs_attr_search_ctx *ctx)
{
	/* NOTE: save errno if it could change and function stays void! */
	ntfs_free(ctx);
}

/**
 * ntfs_attr_find_in_attrdef - find an attribute in the $AttrDef system file
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to find
 *
 * Search for the attribute definition record corresponding to the attribute
 * @type in the $AttrDef system file.
 *
 * Return the attribute type definition record if found and NULL if not found
 * or an error occurred. On error the error code is returned. The
 * following error codes are defined:
 *	ENOENT	- The attribute @type is not specified in $AttrDef.
 *	EINVAL	- Invalid parameters (e.g. @vol is not valid).
 */
struct ATTR_DEF *ntfs_attr_find_in_attrdef(const struct ntfs_volume *vol,
				    const enum ATTR_TYPES type)
{
	struct ATTR_DEF *ad;

	if (!vol || !vol->attrdef || !type) {
		antfs_log_error("type=0x%02x", le32_to_cpu(type));
		return ERR_PTR(-EINVAL);
	}
	for (ad = vol->attrdef; (u8 *) ad - (u8 *) vol->attrdef <
	     vol->attrdef_len && ad->type; ++ad) {
		/* We haven't found it yet, carry on searching. */
		if (le32_to_cpu(ad->type) < le32_to_cpu(type))
			continue;
		/* We found the attribute; return it. */
		if (ad->type == type)
			return ad;
		/* We have gone too far already. No point in continuing. */
		break;
	}
	antfs_log_error("type=0x%02x", le32_to_cpu(type));
	return ERR_PTR(-ENOENT);
}

/**
 * ntfs_attr_size_bounds_check - check a size of an attribute type for validity
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to check
 * @size:	size which to check
 *
 * Check whether the @size in bytes is valid for an attribute of @type on the
 * ntfs volume @vol. This information is obtained from $AttrDef system file.
 *
 * Return 0 if valid and error code if not valid or an error occurred. The
 * following error codes are defined:
 *	ERANGE	- @size is not valid for the attribute @type.
 *	ENOENT	- The attribute @type is not specified in $AttrDef.
 *	EINVAL	- Invalid parameters (e.g. @size is < 0 or @vol is not valid).
 */
int ntfs_attr_size_bounds_check(const struct ntfs_volume *vol,
				const enum ATTR_TYPES type, const s64 size)
{
	struct ATTR_DEF *ad;
	s64 min_size, max_size;

	if (size < 0) {
		antfs_log_error("size=%lld", (long long)size);
		return -EINVAL;
	}

	/*
	 * $ATTRIBUTE_LIST shouldn't be greater than 0x40000, otherwise
	 * Windows would crash. This is not listed in the AttrDef.
	 */
	if (type == AT_ATTRIBUTE_LIST && size > 0x40000) {
		antfs_log_error("Too large attrlist (%lld)", (long long)size);
		return -ERANGE;
	}

	ad = ntfs_attr_find_in_attrdef(vol, type);
	if (IS_ERR(ad))
		return PTR_ERR(ad);

	min_size = sle64_to_cpu(ad->min_size);
	max_size = sle64_to_cpu(ad->max_size);

	/* The $AttrDef generated by Windows specifies 2 as min_size for the
	 * volume name attribute, but in reality Windows sets it to 0 when
	 * clearing the volume name. If we want to be able to clear the volume
	 * name we must also accept 0 as min_size, despite the $AttrDef
	 * definition. */
	if (type == AT_VOLUME_NAME)
		min_size = 0;

	if ((min_size && (size < min_size)) ||
	    ((max_size > 0) && (size > max_size))) {
		antfs_log_error
		    ("Attr type 0x%02x size check failed (min,size,max="
		     "%lld,%lld,%lld)", le32_to_cpu(type), (long long)min_size,
		     (long long)size, (long long)max_size);
		return -ERANGE;
	}
	return 0;
}

/**
 * ntfs_attr_can_be_non_resident - check if an attribute can be non-resident
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type to check
 * @name:	attribute name to check
 * @name_len:	attribute name length
 *
 * Check whether the attribute of @type and @name with name length @name_len on
 * the ntfs volume @vol is allowed to be non-resident.  This information is
 * obtained from $AttrDef system file and is augmented by rules imposed by
 * Microsoft (e.g. see http://support.microsoft.com/kb/974729/).
 *
 * Return 0 if the attribute is allowed to be non-resident and error code is
 * returned. The following error codes are defined:
 *	EPERM	- The attribute is not allowed to be non-resident.
 *	ENOENT	- The attribute @type is not specified in $AttrDef.
 *	EINVAL	- Invalid parameters (e.g. @vol is not valid).
 */
static int ntfs_attr_can_be_non_resident(const struct ntfs_volume *vol,
					 const enum ATTR_TYPES type,
					 const ntfschar *name, int name_len)
{
	struct ATTR_DEF *ad;
	bool allowed;

	/*
	 * Microsoft has decreed that $LOGGED_UTILITY_STREAM attributes with a
	 * name of $TXF_DATA must be resident despite the entry for
	 * $LOGGED_UTILITY_STREAM in $AttrDef allowing them to be non-resident.
	 * Failure to obey this on the root directory mft record of a volume
	 * causes Windows Vista and later to see the volume as a RAW volume and
	 * thus cannot mount it at all.
	 */
	if ((type == AT_LOGGED_UTILITY_STREAM)
	    && name
	    && ntfs_names_are_equal(TXF_DATA, 9, name, name_len,
				    CASE_SENSITIVE, vol->upcase,
				    vol->upcase_len))
		allowed = FALSE;
	else {
		/* Find the attribute definition record in $AttrDef. */
		ad = ntfs_attr_find_in_attrdef(vol, type);
		if (IS_ERR(ad)) {
			antfs_log_error("ntfs_attr_find_in_attrdef failed: %d",
					(int)PTR_ERR(ad));
			return PTR_ERR(ad);
		}
		/* Check the flags and return the result. */
		allowed = !(ad->flags & ATTR_DEF_RESIDENT);
	}
	if (!allowed) {
		antfs_log_debug("Attribute can't be non-resident");
		return -EPERM;
	}
	return 0;
}

/**
 * ntfs_attr_can_be_resident - check if an attribute can be resident
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to check
 *
 * Check whether the attribute of @type on the ntfs volume @vol is allowed to
 * be resident. This information is derived from our ntfs knowledge and may
 * not be completely accurate, especially when user defined attributes are
 * present. Basically we allow everything to be resident except for index
 * allocation and extended attribute attributes.
 *
 * Return 0 if the attribute is allowed to be resident and error code if an
 * error occurred. The following error codes are defined:
 *	EPERM	- The attribute is not allowed to be resident.
 *	EINVAL	- Invalid parameters (e.g. @vol is not valid).
 *
 * Warning: In the system file $MFT the attribute $Bitmap must be non-resident
 *	    otherwise windows will not boot (blue screen of death)!  We cannot
 *	    check for this here as we don't know which inode's $Bitmap is being
 *	    asked about so the caller needs to special case this.
 */
int ntfs_attr_can_be_resident(const struct ntfs_volume *vol,
			      const enum ATTR_TYPES type)
{
	if (!vol || !vol->attrdef || !type)
		return -EINVAL;
	if (type != AT_INDEX_ALLOCATION)
		return 0;

	antfs_log_debug("Attribute can't be resident");
	return -EPERM;
}

/**
 * ntfs_make_room_for_attr - make room for an attribute inside an mft record
 * @m:		mft record
 * @pos:	position at which to make space
 * @size:	byte size to make available at this position
 *
 * @pos points to the attribute in front of which we want to make space.
 *
 * Return 0 on success or the error code on error. Possible error codes are:
 *	ENOSPC	- There is not enough space available to complete operation. The
 *		  caller has to make space before calling this.
 *	EINVAL	- Input parameters were faulty.
 */
int ntfs_make_room_for_attr(struct MFT_RECORD *m, u8 *pos, u32 size)
{
	u32 biu;

	antfs_log_enter("pos 0x%d, size %u.",
			(int)(pos - (u8 *) m), (unsigned)size);

	/* Make size 8-byte alignment. */
	size = (size + 7) & ~7;

	/* Rigorous consistency checks. */
	if (IS_ERR_OR_NULL(m) || !pos || pos < (u8 *) m) {
		antfs_log_error("pos=%p  m=%p", pos, m);
		return -EINVAL;
	}
	/* The -8 is for the attribute terminator. */
	if (pos - (u8 *) m > (int)le32_to_cpu(m->bytes_in_use) - 8) {
		antfs_log_error("EINVAL: attribute terminator?");
		return -EINVAL;
	}
	/* Nothing to do. */
	if (!size) {
		antfs_log_leave("!size");
		return 0;
	}

	biu = le32_to_cpu(m->bytes_in_use);
	/* Do we have enough space? */
	if (biu + size > le32_to_cpu(m->bytes_allocated) ||
	    pos + size > (u8 *) m + le32_to_cpu(m->bytes_allocated)) {
		antfs_log_debug("Not enough space in the MFT record");
		return -ENOSPC;
	}
	/* Move everything after pos to pos + size. */
	memmove(pos + size, pos, biu - (pos - (u8 *) m));
	/* Update mft record. */
	m->bytes_in_use = cpu_to_le32(biu + size);
	antfs_log_leave("ok");
	return 0;
}

/**
 * ntfs_resident_attr_record_add - add resident attribute to inode
 * @ni:		opened ntfs inode to which MFT record add attribute
 * @type:	type of the new attribute
 * @name:	name of the new attribute
 * @name_len:	name length of the new attribute
 * @val:	value of the new attribute
 * @size:	size of new attribute (length of @val, if @val != NULL)
 * @flags:	flags of the new attribute
 *
 * Return offset to attribute from the beginning of the mft record on success
 * and error code on error. Possible error codes are:
 *	EINVAL	- Invalid arguments passed to function.
 *	EEXIST	- Attribute of such type and with same name already exists.
 *	EIO	- I/O error occurred or damaged filesystem.
 */
int ntfs_resident_attr_record_add(struct ntfs_inode *ni, enum ATTR_TYPES type,
				  const ntfschar *name, u8 name_len,
				  const u8 *val, u32 size,
				  enum ATTR_FLAGS data_flags)
{
	struct ntfs_attr_search_ctx *ctx;
	u32 length;
	struct ATTR_RECORD *a;
	struct MFT_RECORD *m;
	int err, offset;
	struct ntfs_inode *base_ni;

	antfs_log_enter("inode 0x%llx, attr 0x%x, flags 0x%x.",
			(long long)ni->mft_no, (unsigned)le32_to_cpu(type),
			(unsigned)le16_to_cpu(data_flags));

	if (IS_ERR_OR_NULL(ni) || (!name && name_len)) {
		antfs_log_error("Invalid arguments");
		return -EINVAL;
	}

	err = ntfs_attr_can_be_resident(ni->vol, type);
	if (err) {
		if (err == -EPERM)
			antfs_log_error("Attribute can't be resident.");
		else
			antfs_log_error("ntfs_attr_can_be_resident failed.");
		return err;
	}

	/* Locate place where record should be. */
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
	/*
	 * Use ntfs_attr_find instead of ntfs_attr_lookup to find place for
	 * attribute in @ni->mrec, not any extent inode in case if @ni is base
	 * file record.
	 */
	err = ntfs_attr_find(type, name, name_len, CASE_SENSITIVE, val, size,
			     ctx);
	if (!err) {
		err = -EEXIST;
		antfs_log_debug("Attribute already present.");
		goto put_err_out;
	} else if (err != -ENOENT) {
		err = -EIO;
		goto put_err_out;
	}
	a = ctx->attr;
	m = ctx->mrec;

	/* Make room for attribute. */
	length = offsetof(struct ATTR_RECORD, resident_end) +
	    ((name_len * sizeof(ntfschar) + 7) & ~7) + ((size + 7) & ~7);
	err = ntfs_make_room_for_attr(ctx->mrec, (u8 *) ctx->attr, length);
	if (err) {
		antfs_log_error("Failed to make room for attribute.");
		goto put_err_out;
	}

	/* Setup record fields. */
	offset = ((u8 *) a - (u8 *) m);
	a->type = type;
	a->length = cpu_to_le32(length);
	a->non_resident = 0;
	a->name_length = name_len;
	a->name_offset = (name_len
			  ?
			  const_cpu_to_le16(offsetof(struct ATTR_RECORD,
						     resident_end))
			  : const_cpu_to_le16(0));
	a->flags = data_flags;
	a->instance = m->next_attr_instance;
	a->value_length = cpu_to_le32(size);
	a->value_offset = cpu_to_le16(length - ((size + 7) & ~7));
	if (val)
		memcpy((u8 *) a + le16_to_cpu(a->value_offset), val, size);
	else
		memset((u8 *) a + le16_to_cpu(a->value_offset), 0, size);
	if (type == AT_FILE_NAME)
		a->resident_flags = RESIDENT_ATTR_IS_INDEXED;
	else
		a->resident_flags = 0;
	if (name_len)
		memcpy((u8 *) a + le16_to_cpu(a->name_offset),
		       name, sizeof(ntfschar) * name_len);
	m->next_attr_instance =
	    cpu_to_le16((le16_to_cpu(m->next_attr_instance) + 1) & 0xffff);
	if (ni->nr_extents == -1)
		base_ni = ni->base_ni;
	else
		base_ni = ni;
	if (type != AT_ATTRIBUTE_LIST && NInoAttrList(base_ni)) {
		err = ntfs_attrlist_entry_add(ni, a);
		if (err) {
			ntfs_attr_record_resize(m, a, 0);
			antfs_log_error("Failed add attribute entry to "
					"ATTRIBUTE_LIST.");
			goto put_err_out;
		}
	}
	ntfs_inode_mark_dirty(ni);
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("offset: %d", offset);
	return offset;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_non_resident_attr_record_add - add extent of non-resident attribute
 * @ni:			opened ntfs inode to which MFT record add attribute
 * @type:		type of the new attribute extent
 * @name:		name of the new attribute extent
 * @name_len:		name length of the new attribute extent
 * @lowest_vcn:		lowest vcn of the new attribute extent
 * @dataruns_size:	dataruns size of the new attribute extent
 * @flags:		flags of the new attribute extent
 *
 * Return offset to attribute from the beginning of the mft record on success
 * and error code on error. Possible error codes are:
 *	EINVAL	- Invalid arguments passed to function.
 *	EEXIST	- Attribute of such type, with same lowest vcn and with same
 *		  name already exists.
 *	EIO	- I/O error occurred or damaged filesystem.
 */
int ntfs_non_resident_attr_record_add(struct ntfs_inode *ni,
				      enum ATTR_TYPES type,
				      const ntfschar *name, u8 name_len,
				      VCN lowest_vcn, int dataruns_size,
				      enum ATTR_FLAGS flags)
{
	struct ntfs_attr_search_ctx *ctx;
	u32 length;
	struct ATTR_RECORD *a;
	struct MFT_RECORD *m;
	struct ntfs_inode *base_ni;
	int err, offset;

	if (!ni || dataruns_size <= 0 || (!name && name_len)) {
		antfs_log_error("Invalid arguments");
		return -EINVAL;
	}

	antfs_log_enter("inode 0x%llx, attr 0x%x, lowest_vcn %lld, "
			"dataruns_size %d, flags 0x%x.",
			(long long)ni->mft_no, (unsigned)le32_to_cpu(type),
			(long long)lowest_vcn, dataruns_size,
			(unsigned)le16_to_cpu(flags));

	err = ntfs_attr_can_be_non_resident(ni->vol, type, name, name_len);
	if (err) {
		if (err == -EPERM)
			antfs_log_error("Attribute can't be non resident");
		else
			antfs_log_error("ntfs_attr_can_be_non_resident failed");
		return err;
	}

	/* Locate place where record should be. */
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
	/*
	 * Use ntfs_attr_find instead of ntfs_attr_lookup to find place for
	 * attribute in @ni->mrec, not any extent inode in case if @ni is base
	 * file record.
	 */
	err = ntfs_attr_find(type, name, name_len, CASE_SENSITIVE, NULL, 0,
			     ctx);
	if (!err) {
		err = -EEXIST;
		antfs_log_error("Attribute 0x%x already present",
				le32_to_cpu(type));
		goto put_err_out;
	}
	if (err != -ENOENT) {
		antfs_log_error("ntfs_attr_find failed");
		err = -EIO;
		goto put_err_out;
	}
	a = ctx->attr;
	m = ctx->mrec;

	/* Make room for attribute. */
	dataruns_size = (dataruns_size + 7) & ~7;
	length = offsetof(struct ATTR_RECORD, compressed_size) +
		((sizeof(ntfschar) * name_len + 7) & ~7) +
	    dataruns_size +
	    ((flags & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE)) ?
	     sizeof(a->compressed_size) : 0);
	err = ntfs_make_room_for_attr(ctx->mrec, (u8 *) ctx->attr, length);
	if (err) {
		antfs_log_error("Failed to make room for attribute");
		goto put_err_out;
	}

	/* Setup record fields. */
	a->type = type;
	a->length = cpu_to_le32(length);
	a->non_resident = 1;
	a->name_length = name_len;
	a->name_offset = cpu_to_le16(offsetof(struct ATTR_RECORD,
				compressed_size) + ((flags &
				       (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE)) ?
				      sizeof(a->compressed_size) : 0));
	a->flags = flags;
	a->instance = m->next_attr_instance;
	a->lowest_vcn = cpu_to_sle64(lowest_vcn);
	a->mapping_pairs_offset = cpu_to_le16(length - dataruns_size);
	a->compression_unit = (flags & ATTR_IS_COMPRESSED)
	    ? STANDARD_COMPRESSION_UNIT : 0;
	/* If @lowest_vcn == 0, than setup empty attribute. */
	if (!lowest_vcn) {
		a->highest_vcn = const_cpu_to_sle64(-1);
		a->allocated_size = const_cpu_to_sle64(0);
		a->data_size = const_cpu_to_sle64(0);
		a->initialized_size = const_cpu_to_sle64(0);
		/* Set empty mapping pairs. */
		*((u8 *) a + le16_to_cpu(a->mapping_pairs_offset)) = 0;
	}
	if (name_len)
		memcpy((u8 *) a + le16_to_cpu(a->name_offset),
		       name, sizeof(ntfschar) * name_len);
	m->next_attr_instance =
	    cpu_to_le16((le16_to_cpu(m->next_attr_instance) + 1) & 0xffff);
	if (ni->nr_extents == -1)
		base_ni = ni->base_ni;
	else
		base_ni = ni;
	if (type != AT_ATTRIBUTE_LIST && NInoAttrList(base_ni)) {
		err = ntfs_attrlist_entry_add(ni, a);
		if (err) {
			antfs_log_error("Failed add attr entry to attrlist");
			ntfs_attr_record_resize(m, a, 0);
			goto put_err_out;
		}
	}
	ntfs_inode_mark_dirty(ni);
	/*
	 * Locate offset from start of the MFT record where new attribute is
	 * placed. We need relookup it, because record maybe moved during
	 * update of attribute list.
	 */
	ntfs_attr_reinit_search_ctx(ctx);
	err = ntfs_attr_lookup(type, name, name_len, CASE_SENSITIVE,
			       lowest_vcn, NULL, 0, ctx);
	if (err) {
		antfs_log_error("attribute lookup failed");
		ntfs_attr_put_search_ctx(ctx);
		return err;

	}
	offset = (u8 *) ctx->attr - (u8 *) ctx->mrec;
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("offset: %d", offset);
	return offset;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_attr_record_rm - remove attribute extent
 * @ctx:	search context describing the attribute which should be removed
 *
 * If this function succeed, user should reinit search context if he/she wants
 * use it anymore.
 *
 * Return 0 on success and error code on error.
 * Possible error codes are:
 *	EINVAL	- Invalid arguments passed to function.
 *	EIO	- I/O error occurred or damaged filesystem.
 */
int ntfs_attr_record_rm(struct ntfs_attr_search_ctx *ctx)
{
	int err;
	struct ntfs_inode *base_ni, *ni;
	enum ATTR_TYPES type;

	if (!ctx || !ctx->ntfs_ino || !ctx->mrec || !ctx->attr)
		return -EINVAL;

	antfs_log_enter("Entering for inode 0x%llx, attr 0x%x.",
			(long long)ctx->ntfs_ino->mft_no,
			(unsigned)le32_to_cpu(ctx->attr->type));
	type = ctx->attr->type;
	ni = ctx->ntfs_ino;
	if (ctx->base_ntfs_ino)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;

	/* Remove attribute itself. */
	if (ntfs_attr_record_resize(ctx->mrec, ctx->attr, 0)) {
		antfs_log_debug
		    ("Couldn't remove attribute record. Bug or damaged MFT "
		     "record.");
		if (NInoAttrList(base_ni) && type != AT_ATTRIBUTE_LIST)
			if (ntfs_attrlist_entry_add(ni, ctx->attr)) {
				antfs_logger(ANTFS_I(ni)->i_sb->s_id,
					"Rollback failed. Leaving "
					"inconsistent metadata.");
			}
		return -EIO;
	}
	ntfs_inode_mark_dirty(ni);

	/*
	 * Remove record from $ATTRIBUTE_LIST if present and we don't want
	 * delete $ATTRIBUTE_LIST itself.
	 */
	if (NInoAttrList(base_ni) && type != AT_ATTRIBUTE_LIST) {
		err = ntfs_attrlist_entry_rm(ctx);
		if (err) {
			antfs_log_debug("Couldn't delete record from "
					"$ATTRIBUTE_LIST.");
			return err;
		}
	}

	/* Post $ATTRIBUTE_LIST delete setup. */
	if (type == AT_ATTRIBUTE_LIST) {
		if (NInoAttrList(base_ni) && base_ni->attr_list)
			ntfs_free(base_ni->attr_list);
		base_ni->attr_list = NULL;
		NInoClearAttrList(base_ni);
		NInoAttrListClearDirty(base_ni);
	}

	/* Free MFT record, if it doesn't contain attributes. */
	if (le32_to_cpu(ctx->mrec->bytes_in_use) -
	    le16_to_cpu(ctx->mrec->attrs_offset) == 8) {
		bool is_base_ni = (ni == base_ni);

		clear_nlink(ANTFS_I(ni));
		ntfs_inode_close(ni);
		/* Remove done if we freed base inode. */
		if (is_base_ni)
			return 0;
	}

	if (type == AT_ATTRIBUTE_LIST || !NInoAttrList(base_ni))
		return 0;

	/* Remove attribute list if we don't need it any more. */
	if (!ntfs_attrlist_need(base_ni)) {
		ntfs_attr_reinit_search_ctx(ctx);
		if (ntfs_attr_lookup(AT_ATTRIBUTE_LIST, NULL, 0, CASE_SENSITIVE,
				     0, NULL, 0, ctx)) {
			/*
			 * FIXME: Should we succeed here? Definitely something
			 * goes wrong because NInoAttrList(base_ni) returned
			 * that we have got attribute list.
			 */
			antfs_log_debug("Couldn't find attribute list. Succeed "
					"anyway.");
			return 0;
		}
		/* Deallocate clusters. */
		if (ctx->attr->non_resident) {
			struct runlist_element *al_rl;

			al_rl = ntfs_mapping_pairs_decompress(base_ni->vol,
							      ctx->attr, NULL);
			if (IS_ERR(al_rl)) {
				antfs_log_debug
				    ("Couldn't decompress attribute list "
				     "runlist. Succeed anyway.");
				return 0;
			}
			if (ntfs_cluster_free_from_rl(base_ni->vol, al_rl)) {
				antfs_logger(ANTFS_I(base_ni)->i_sb->s_id,
					    "Leaking clusters! Run chkdsk."
					    " couldn't free clusters from "
					    "attribute list runlist.");
			}
			ntfs_free(al_rl);
		}
		/* Remove attribute record itself. */
		if (ntfs_attr_record_rm(ctx)) {
			/*
			 * FIXME: Should we succeed here? BTW, chkdsk doesn't
			 * complain if it find MFT record with attribute list,
			 * but without extents.
			 */
			antfs_log_debug
			    ("Couldn't remove attribute list. Succeed "
			     "anyway.");
			return 0;
		}
	}
	return 0;
}

/**
 * ntfs_attr_add - add attribute to inode
 * @ni:		opened ntfs inode to which add attribute
 * @type:	type of the new attribute
 * @name:	name in unicode of the new attribute
 * @name_len:	name length in unicode characters of the new attribute
 * @val:	value of new attribute
 * @size:	size of the new attribute / length of @val (if specified)
 *
 * @val should always be specified for always resident attributes (eg. FILE_NAME
 * attribute), for attributes that can become non-resident @val can be NULL
 * (eg. DATA attribute). @size can be specified even if @val is NULL, in this
 * case data size will be equal to @size and initialized size will be equal
 * to 0.
 *
 * If inode haven't got enough space to add attribute, add attribute to one of
 * it extents, if no extents present or no one of them have enough space, than
 * allocate new extent and add attribute to it.
 *
 * If on one of this steps attribute list is needed but not present, than it is
 * added transparently to caller. So, this function should not be called with
 * @type == AT_ATTRIBUTE_LIST, if you really need to add attribute list call
 * ntfs_inode_add_attrlist instead.
 *
 * On success return 0. On error return the error code.
 */
int ntfs_attr_add(struct ntfs_inode *ni, enum ATTR_TYPES type,
		  ntfschar *name, u8 name_len, const u8 *val, s64 size)
{
	u32 attr_rec_size;
	int err, eo, i, offset;
	bool is_resident;
	bool can_be_non_resident = FALSE;
	struct ntfs_inode *attr_ni;
	struct ntfs_attr *na;
	enum ATTR_FLAGS data_flags;

	if (IS_ERR_OR_NULL(ni) || size < 0 || type == AT_ATTRIBUTE_LIST) {
		antfs_log_error("ni=%p  size=%lld", ni, (long long)size);
		return -EINVAL;
	}

	antfs_log_enter("inode %lld, attr %x, size %lld.",
			(long long)ni->mft_no, le32_to_cpu(type),
			(long long)size);

	if (ni->nr_extents == -1)
		ni = ni->base_ni;

	/* Check the attribute type and the size. */
	err = ntfs_attr_size_bounds_check(ni->vol, type, size);
	if (err) {
		antfs_log_error("size_bounds_check: %d", err);
		return err == -ENOENT ? -EIO : err;
	}

	/* Sanity checks for always resident attributes. */
	err = ntfs_attr_can_be_non_resident(ni->vol, type, name, name_len);
	if (err) {
		if (err != -EPERM) {
			antfs_log_error("ntfs_attr_can_be_non_resident failed");
			goto err_out;
		}
		/* @val is mandatory. */
		if (!val) {
			antfs_log_error("val is mandatory for always resident "
					"attributes");
			return -EINVAL;
		}
		if (size > ni->vol->mft_record_size) {
			antfs_log_error("Attribute is too big");
			return -ERANGE;
		}
	} else
		can_be_non_resident = TRUE;

	/*
	 * Determine resident or not will be new attribute. We add 8 to size in
	 * non resident case for mapping pairs.
	 */
	err = ntfs_attr_can_be_resident(ni->vol, type);
	if (!err) {
		is_resident = TRUE;
	} else {
		if (err != -EPERM) {
			antfs_log_error("ntfs_attr_can_be_resident failed");
			goto err_out;
		}
		is_resident = FALSE;
	}
	/* Calculate attribute record size. */
	if (is_resident)
		attr_rec_size = offsetof(struct ATTR_RECORD, resident_end) +
		    ((name_len * sizeof(ntfschar) + 7) & ~7) +
		    ((size + 7) & ~7);
	else
		attr_rec_size = offsetof(struct ATTR_RECORD, non_resident_end) +
		    ((name_len * sizeof(ntfschar) + 7) & ~7) + 8;

	/*
	 * If we have enough free space for the new attribute in the base MFT
	 * record, then add attribute to it.
	 */
	if (le32_to_cpu(ni->mrec->bytes_allocated) -
	    le32_to_cpu(ni->mrec->bytes_in_use) >= attr_rec_size) {
		attr_ni = ni;
		antfs_log_debug("Keep attribute resident");
		goto add_attr_record;
	}

	/* Try to add to extent inodes. */
	antfs_log_debug("Try add extent inode");
	err = ntfs_inode_attach_all_extents(ni);
	if (err != 0) {
		antfs_log_error("Failed to attach all extents to inode");
		goto err_out;
	}
	for (i = 0; i < ni->nr_extents; i++) {
		attr_ni = ni->extent_nis[i];
		if (le32_to_cpu(attr_ni->mrec->bytes_allocated) -
		    le32_to_cpu(attr_ni->mrec->bytes_in_use) >= attr_rec_size)
			goto add_attr_record;
	}

	antfs_log_debug("No space in extent inode");
	/* There is no extent that contain enough space for new attribute. */
	if (!NInoAttrList(ni)) {
		/* Add attribute list not present, add it and retry. */
		err = ntfs_inode_add_attrlist(ni);
		if (err != 0) {
			antfs_log_error("Failed to add attribute list");
			goto err_out;
		}
		return ntfs_attr_add(ni, type, name, name_len, val, size);
	}
	/* Allocate new extent. */
	antfs_log_debug("Alloc new extent");
	attr_ni = ntfs_mft_record_alloc(ni->vol, ni);
	if (IS_ERR(attr_ni)) {
		err = PTR_ERR(attr_ni);
		antfs_log_error("Failed to allocate extent record");
		goto err_out;
	}

add_attr_record:
	if ((ni->flags & FILE_ATTR_COMPRESSED)
	    && (ni->vol->major_ver >= 3)
	    && NVolCompression(ni->vol)
	    && (ni->vol->cluster_size <= MAX_COMPRESSION_CLUSTER_SIZE)
	    && ((type == AT_DATA)
		|| ((type == AT_INDEX_ROOT) && (name == NTFS_INDEX_I30))))
		data_flags = ATTR_IS_COMPRESSED;
	else
		data_flags = const_cpu_to_le16(0);
	if (is_resident) {
		/* Add resident attribute. */
		offset = ntfs_resident_attr_record_add(attr_ni, type, name,
						       name_len, val, size,
						       data_flags);
		if (offset < 0) {
			if (offset == -ENOSPC && can_be_non_resident)
				goto add_non_resident;
			err = offset;
			antfs_log_error("Failed to add resident attribute");
			goto free_err_out;
		}
		antfs_log_leave("is resident");
		return 0;
	}

add_non_resident:
	/* Add non resident attribute. */
	offset = ntfs_non_resident_attr_record_add(attr_ni, type, name,
						   name_len, 0, 8, data_flags);
	if (offset < 0) {
		err = offset;
		antfs_log_error("Failed to add non resident attribute");
		goto free_err_out;
	}

	/* If @size == 0, we are done. */
	if (!size) {
		antfs_log_leave("!size: ok");
		return 0;
	}

	/* Open new attribute and resize it. */
	na = ntfs_attr_open(ni, type, name, name_len);
	if (IS_ERR(na)) {
		err = PTR_ERR(na);
		antfs_log_error("Failed to open just added attribute");
		goto rm_attr_err_out;
	}
	/* Resize and set attribute value. */
	err = ntfs_attr_truncate_i(na, size, HOLES_OK);
	if (err) {
		/* Make STATUS_ATTRIBUTE_FILLED_MFT an error here */
		if (err > 0)
			err = -ENOSPC;
		ntfs_attr_close(na);
		antfs_log_error("truncate failed");
		goto err_out;
	}
	if (val) {
		err = ntfs_attr_pwrite(na, 0, size, val);
		if (err != size) {
			antfs_log_error("Failed to initialize just added "
					"attribute");
			eo = ntfs_attr_rm(na);
			if (eo) {
				antfs_log_error("Failed to remove just added "
						"attribute");
				err = eo;
			}
			if (err >= 0)
				err = -EIO;
			ntfs_attr_close(na);
			goto err_out;
		}
	}
	ntfs_attr_close(na);
	antfs_log_leave("ok");
	return 0;

rm_attr_err_out:
	/* Remove just added attribute. */
	if (ntfs_attr_record_resize(attr_ni->mrec, (struct ATTR_RECORD *)
				    ((u8 *) attr_ni->mrec + offset), 0)) {
		antfs_logger(ANTFS_I(attr_ni)->i_sb->s_id,
				"Failed to remove just added attribute #2");
	}
free_err_out:
	/* Free MFT record, if it doesn't contain attributes. */
	if (le32_to_cpu(attr_ni->mrec->bytes_in_use) -
	    le16_to_cpu(attr_ni->mrec->attrs_offset) == 8) {
		clear_nlink(ANTFS_I(attr_ni));
		ntfs_inode_close(attr_ni);
	}
err_out:
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_attr_rm - remove attribute from ntfs inode
 * @na:		opened ntfs attribute to delete
 *
 * Remove attribute and all it's extents from ntfs inode. If attribute was non
 * resident also free all clusters allocated by attribute.
 *
 * Return 0 on success or error code on error.
 */
int ntfs_attr_rm(struct ntfs_attr *na)
{
	struct ntfs_attr_search_ctx *ctx;
	int ret = 0;

	if (!na) {
		antfs_log_error("Invalid arguments passed.");
		ret = -EINVAL;
		goto out;
	}

	antfs_log_enter("Entering for inode 0x%llx, attr 0x%x.",
			(long long)na->ni->mft_no, le32_to_cpu(na->type));

	/* Free cluster allocation. */
	if (NAttrNonResident(na)) {
		ret = ntfs_attr_map_whole_runlist(na);
		if (ret != 0)
			return ret;
		ret = ntfs_cluster_free(na->ni->vol, na, 0, -1);
		if (ret < 0) {
			antfs_logger(ANTFS_I(na->ni)->i_sb->s_id,
				    "Failed to free cluster allocation. "
				    "Leaving inconsistent metadata.");
		}
	}

	/* Search for attribute extents and remove them all. */
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (IS_ERR(ctx)) {
		ret = PTR_ERR(ctx);
		goto out;
	}
	while (!(ret = ntfs_attr_lookup(na->type, na->name, na->name_len,
					CASE_SENSITIVE, 0, NULL, 0, ctx))) {
		ret = ntfs_attr_record_rm(ctx);
		if (ret) {
			antfs_logger(ANTFS_I(na->ni)->i_sb->s_id,
				    "Failed to remove attribute extent. "
				    "Leaving inconsistent metadata");
		}
		ntfs_attr_reinit_search_ctx(ctx);
	}
	ntfs_attr_put_search_ctx(ctx);
	if (ret != -ENOENT) {
		antfs_logger(ANTFS_I(na->ni)->i_sb->s_id,
				"Attribute lookup failed. Probably leaving"
				" inconsistent metadata");
	} else
		ret = 0;
out:
	antfs_log_leave("ret: %d", ret);
	return ret;
}

/**
 * ntfs_attr_record_resize - resize an attribute record
 * @m:		mft record containing attribute record
 * @a:		attribute record to resize
 * @new_size:	new size in bytes to which to resize the attribute record @a
 *
 * Resize the attribute record @a, i.e. the resident part of the attribute, in
 * the mft record @m to @new_size bytes.
 *
 * Return 0 on success and the error code on error.
 * The following error codes are defined:
 *	ENOSPC	- Not enough space in the mft record @m to perform the resize.
 * Note that on error no modifications have been performed whatsoever.
 *
 * Warning: If you make a record smaller without having copied all the data you
 *	    are interested in the data may be overwritten!
 */
int ntfs_attr_record_resize(struct MFT_RECORD *m, struct ATTR_RECORD *a,
			    u32 new_size)
{
	u32 old_size, alloc_size, attr_size;
	int err = 0;
	old_size = le32_to_cpu(m->bytes_in_use);
	alloc_size = le32_to_cpu(m->bytes_allocated);
	attr_size = le32_to_cpu(a->length);

	antfs_log_enter("Sizes: old=0x%x alloc=0x%x attr=0x%x new=0x%x",
			(unsigned)old_size, (unsigned)alloc_size,
			(unsigned)attr_size, (unsigned)new_size);

	/* Align to 8 bytes, just in case the caller hasn't. */
	new_size = (new_size + 7) & ~7;

	/* If the actual attribute length has changed, move things around. */
	if (new_size != attr_size) {

		u32 new_muse = old_size - attr_size + new_size;

		/* STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT will cause
		 * ntfs_ir_make_space() to create an INDEX_ALLOCATION attribute
		 * instead of moving INDEX_ROOT to an extent.
		 */
		if (a->type == AT_INDEX_ROOT && new_size > attr_size &&
		    new_muse + 120 > alloc_size) {
			err = STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT;
			if (old_size + 120 > alloc_size) {
				antfs_log_debug("Too big INDEX_ROOT (%u > %u)",
						new_muse + 120, alloc_size);
			}
			goto out;
		}

		/* Not enough space in this mft record. */
		if (new_muse > alloc_size) {
			err = -ENOSPC;
			/* Nothing to be THAT much worried about here. */
			antfs_log_debug("Not enough space in the MFT record "
					"(%u > %u) type %x", new_muse, alloc_size, a->type);
			goto out;
		}

		/* Move attributes following @a to their new location. */
		memmove((u8 *) a + new_size, (u8 *) a + attr_size,
			old_size - ((u8 *) a - (u8 *) m) - attr_size);

		/* Adjust @m to reflect the change in used space. */
		m->bytes_in_use = cpu_to_le32(new_muse);
		antfs_log_debug("new_muse: 0x%x", new_muse);

		/* Adjust @a to reflect the new size. */
		if (new_size >= offsetof(struct ATTR_RECORD, length) +
		    sizeof(a->length))
			a->length = cpu_to_le32(new_size);
	}
out:
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_resident_attr_value_resize - resize the value of a resident attribute
 * @m:		mft record containing attribute record
 * @a:		attribute record whose value to resize
 * @new_size:	new size in bytes to which to resize the attribute value of @a
 *
 * Resize the value of the attribute @a in the mft record @m to @new_size bytes.
 * If the value is made bigger, the newly "allocated" space is cleared.
 *
 * Return 0 or STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT on success and the negative
 * error code.
 *
 * The following error codes are defined:
 *	ENOSPC	- Not enough space in the mft record @m to perform the resize.
 * Note that on error no modifications have been performed whatsoever.
 */
int ntfs_resident_attr_value_resize(struct MFT_RECORD *m, struct ATTR_RECORD *a,
				    const u32 new_size)
{
	int ret = 0;

	antfs_log_enter("new size %u", (unsigned)new_size);

	/* Resize the resident part of the attribute record. */
	ret = ntfs_attr_record_resize(m, a, (le16_to_cpu(a->value_offset) +
					new_size + 7) & ~7);
	if (ret != 0)
		goto out;
	/*
	 * If we made the attribute value bigger, clear the area between the
	 * old size and @new_size.
	 */
	if (new_size > le32_to_cpu(a->value_length))
		memset((u8 *) a + le16_to_cpu(a->value_offset) +
		       le32_to_cpu(a->value_length), 0, new_size -
		       le32_to_cpu(a->value_length));
	/* Finally update the length of the attribute value. */
	a->value_length = cpu_to_le32(new_size);
out:

	return ret;
}

/**
 * ntfs_attr_record_move_to - move attribute record to target inode
 * @ctx:	attribute search context describing the attribute record
 * @ni:		opened ntfs inode to which move attribute record
 *
 * If this function succeed, user should reinit search context if he/she wants
 * use it anymore.
 *
 * Return 0 on success and the error code on error.
 */
int ntfs_attr_record_move_to(struct ntfs_attr_search_ctx *ctx,
			     struct ntfs_inode *ni)
{
	struct ntfs_attr_search_ctx *nctx;
	struct ATTR_RECORD *a;
	int err = 0;

	if (!ctx || !ctx->attr || !ctx->ntfs_ino || IS_ERR_OR_NULL(ni)) {
		antfs_log_error("Invalid arguments passed.");
		err = -EINVAL;
		goto out;
	}

	antfs_log_enter
	    ("ctx->attr->type 0x%x, ctx->ntfs_ino->mft_no "
	     "0x%llx, ni->mft_no 0x%llx.",
	     (unsigned)le32_to_cpu(ctx->attr->type),
	     (long long)ctx->ntfs_ino->mft_no, (long long)ni->mft_no);

	if (ctx->ntfs_ino == ni)
		goto out;

	if (!ctx->al_entry) {
		antfs_log_debug
		    ("Inode should contain attribute list to use this "
		     "function.");
		err = -EINVAL;
		goto out;
	}

	/* Find place in MFT record where attribute will be moved. */
	a = ctx->attr;
	nctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(nctx)) {
		err = PTR_ERR(nctx);
		goto out;
	}

	/*
	 * Use ntfs_attr_find instead of ntfs_attr_lookup to find place for
	 * attribute in @ni->mrec, not any extent inode in case if @ni is base
	 * file record.
	 */
	err =
	    ntfs_attr_find(a->type,
			   (ntfschar *) ((u8 *) a +
					 le16_to_cpu(a->name_offset)),
			   a->name_length, CASE_SENSITIVE, NULL, 0, nctx);
	if (!err) {
		antfs_log_debug
		    ("Attribute of such type, with same name already "
		     "present in this MFT record.");
		err = -EEXIST;
		goto put_out;
	}
	if (err != -ENOENT) {
		antfs_log_debug("Attribute lookup failed.");
		goto put_out;
	}

	/* Make space and move attribute. */
	err = ntfs_make_room_for_attr(ni->mrec, (u8 *) nctx->attr,
				      le32_to_cpu(a->length));
	if (err) {
		antfs_log_debug("Couldn't make space for attribute.");
		goto put_out;
	}
	memcpy(nctx->attr, a, le32_to_cpu(a->length));
	nctx->attr->instance = nctx->mrec->next_attr_instance;
	nctx->mrec->next_attr_instance = cpu_to_le16((le16_to_cpu
						      (nctx->mrec->
						       next_attr_instance) +
						      1) & 0xffff);
	ntfs_attr_record_resize(ctx->mrec, a, 0);
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_inode_mark_dirty(ni);

	/* Update attribute list. */
	ctx->al_entry->mft_reference =
	    MK_LE_MREF(ni->mft_no, le16_to_cpu(ni->mrec->sequence_number));
	ctx->al_entry->instance = nctx->attr->instance;
	ntfs_attrlist_mark_dirty(ni);

put_out:
	ntfs_attr_put_search_ctx(nctx);
out:
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_attr_record_move_away - move away attribute record from it's mft record
 * @ctx:	attribute search context describing the attribute record
 * @extra:	minimum amount of free space in the new holder of record
 *
 * New attribute record holder must have free @extra bytes after moving
 * attribute record to it.
 *
 * If this function succeed, user should reinit search context if he/she wants
 * use it anymore.
 *
 * Return 0 on success and the error code on error.
 */
int ntfs_attr_record_move_away(struct ntfs_attr_search_ctx *ctx, int extra)
{
	struct ntfs_inode *base_ni, *ni;
	struct MFT_RECORD *m;
	int i, err = 0;

	if (!ctx || !ctx->attr || !ctx->ntfs_ino || extra < 0) {
		antfs_log_error("ctx=%p ctx->attr=%p extra=%d",
				ctx, ctx ? ctx->attr : NULL, extra);
		err = -EINVAL;
		goto out;
	}

	antfs_log_enter("attr 0x%x, inode %llu",
			(unsigned)le32_to_cpu(ctx->attr->type),
			(unsigned long long)ctx->ntfs_ino->mft_no);

	if (ctx->ntfs_ino->nr_extents == -1)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;

	if (!NInoAttrList(base_ni)) {
		antfs_log_error("Inode %llu has no attrlist",
				(unsigned long long)base_ni->mft_no);
		err = -EINVAL;
		goto out;
	}

	err = ntfs_inode_attach_all_extents(ctx->ntfs_ino);
	if (err) {
		antfs_log_error("Couldn't attach extents, inode=%llu",
				(unsigned long long)base_ni->mft_no);
		goto out;
	}

	/* Walk through all extents and try to move attribute to them. */
	for (i = 0; i < base_ni->nr_extents; i++) {
		ni = base_ni->extent_nis[i];
		m = ni->mrec;

		if (ctx->ntfs_ino->mft_no == ni->mft_no)
			continue;

		if (le32_to_cpu(m->bytes_allocated) -
		    le32_to_cpu(m->bytes_in_use) <
		    le32_to_cpu(ctx->attr->length) + extra)
			continue;

		/*
		 * ntfs_attr_record_move_to can fail if extent with other lowest
		 * VCN already present in inode we trying move record to. So,
		 * do not return error.
		 */
		err = ntfs_attr_record_move_to(ctx, ni);
		if (!err)
			goto out;
	}

	/*
	 * Failed to move attribute to one of the current extents, so allocate
	 * new extent and move attribute to it.
	 */
	ni = ntfs_mft_record_alloc(base_ni->vol, base_ni);
	if (IS_ERR(ni)) {
		antfs_log_error("Couldn't allocate MFT record");
		err = PTR_ERR(ni);
		goto out;
	}
	err = ntfs_attr_record_move_to(ctx, ni);
	if (err)
		antfs_log_error("Couldn't move attribute to MFT record");
out:
	antfs_log_leave("err: %d", err);
	return err;
}

static int ntfs_data_attr_copy_non_resident(struct ntfs_attr *na,
					    struct ntfs_attr_search_ctx *ctx)
{
	struct ATTR_RECORD *a = ctx->attr;
	struct page *page;
	u8 *kaddr;
	u32 attr_size = le32_to_cpu(a->value_length);

	page = grab_cache_page(ANTFS_I(na->ni)->i_mapping, 0);
	if (unlikely(!page))
		return -ENOMEM;

	kaddr = kmap_atomic(page);
	memcpy(kaddr, (u8*)a + le16_to_cpu(a->value_offset), attr_size);
	memset(kaddr + attr_size, 0, PAGE_SIZE - attr_size);
	kunmap_atomic(kaddr);
	flush_dcache_page(page);
	SetPageUptodate(page);
	unlock_page(page);
	put_page(page);

	return attr_size;
}

/**
 * ntfs_attr_make_non_resident - convert a resident to a non-resident attribute
 * @na:		open ntfs attribute to make non-resident
 * @ctx:	ntfs search context describing the attribute
 *
 * Convert a resident ntfs attribute to a non-resident one.
 *
 * Return 0 on success and the error code on failure.
 * The following error codes are defined:
 *	EPERM	- The attribute is not allowed to be non-resident.
 *	TODO: others...
 *
 * NOTE to self: No changes in the attribute list are required to move from
 *		 a resident to a non-resident attribute.
 *
 * Warning: We do not set the inode dirty and we do not write out anything!
 *	    We expect the caller to do this as this is a fairly low level
 *	    function and it is likely there will be further changes made.
 */
int ntfs_attr_make_non_resident(struct ntfs_attr *na,
				struct ntfs_attr_search_ctx *ctx)
{
	s64 new_allocated_size, bw;
	struct ntfs_volume *vol = na->ni->vol;
	struct ATTR_RECORD *a = ctx->attr;
	struct runlist_element *rl;
	int mp_size, mp_ofs, name_ofs, arec_size, err = 0;

	antfs_log_enter("inode 0x%llx, attr 0x%x.", (unsigned long long)
			na->ni->mft_no, le32_to_cpu(na->type));

	/* Some preliminary sanity checking. */
	if (NAttrNonResident(na)) {
		antfs_log_error("Eeek!  Trying to make non-resident attribute "
				"non-resident.  Aborting...");
		err = -EINVAL;
		goto out;
	}

	/* Check that the attribute is allowed to be non-resident. */
	err =
	    ntfs_attr_can_be_non_resident(vol, na->type, na->name,
					  na->name_len);
	if (err)
		goto out;

	new_allocated_size = (le32_to_cpu(a->value_length) + vol->cluster_size
			      - 1) & ~(vol->cluster_size - 1);

	if (new_allocated_size > 0) {
		if ((a->flags & ATTR_COMPRESSION_MASK)
		    == ATTR_IS_COMPRESSED) {
			/* must allocate full compression blocks */
			new_allocated_size = ((new_allocated_size - 1)
					      |
					      ((1L <<
						(STANDARD_COMPRESSION_UNIT +
						 vol->cluster_size_bits)) -
					       1)) + 1;
		}
		/* Start by allocating clusters to hold the attribute value. */
		rl = ntfs_cluster_alloc(vol, 0, new_allocated_size >>
					vol->cluster_size_bits, -1, DATA_ZONE,
					0);
		if (IS_ERR_OR_NULL(rl)) {
			err = PTR_ERR(rl);
			goto out;
		}
	} else {
		rl = NULL;
		NAttrClearFullyMapped(na);
	}
	/*
	 * Setup the in-memory attribute structure to be non-resident so that
	 * we can use ntfs_attr_pwrite().
	 */
	NAttrSetNonResident(na);
	NAttrSetBeingNonResident(na);
	na->rl = rl;
	na->allocated_size = new_allocated_size;
	na->data_size = na->initialized_size = le32_to_cpu(a->value_length);
	/*
	 * FIXME: For now just clear all of these as we don't support them when
	 * writing.
	 */
	NAttrClearSparse(na);
	NAttrClearEncrypted(na);
	if ((a->flags & ATTR_COMPRESSION_MASK) == ATTR_IS_COMPRESSED) {
		/* set compression writing parameters */
		na->compression_block_size
		    = 1 << (STANDARD_COMPRESSION_UNIT + vol->cluster_size_bits);
		na->compression_block_clusters = 1 << STANDARD_COMPRESSION_UNIT;
	}

	if (rl) {
		/* Now copy the attribute value to the allocated cluster(s). */
		if (na->type == AT_DATA &&
		    !(a->flags & ATTR_COMPRESSION_MASK))
			bw = ntfs_data_attr_copy_non_resident(na, ctx);
		else
			bw = ntfs_attr_pwrite(na, 0, le32_to_cpu(a->value_length),
					      (u8 *) a + le16_to_cpu(a->value_offset));

		if (bw != le32_to_cpu(a->value_length)) {
			if (bw < 0)
				err = bw;
			else
				err = -EIO;
			antfs_log_debug
				("Eeek!  Failed to write out attribute value "
				 "(bw = %lli, errno = %i). Aborting...",
				 (long long)bw, err);
			goto cluster_free_err_out;
		}
	}
	/* Determine the size of the mapping pairs array. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl, 0, INT_MAX);
	if (mp_size < 0) {
		err = mp_size;
		antfs_log_debug
		    ("Eeek!  Failed to get size for mapping pairs array.  "
		     "Aborting...");
		goto cluster_free_err_out;
	}
	/* Calculate new offsets for the name and the mapping pairs array. */
	if (na->ni->flags & FILE_ATTR_COMPRESSED)
		name_ofs = (sizeof(struct ATTR_RECORD) + 7) & ~7;
	else
		name_ofs = (sizeof(struct ATTR_RECORD) -
			    sizeof(a->compressed_size) + 7) & ~7;
	mp_ofs = (name_ofs + a->name_length * sizeof(ntfschar) + 7) & ~7;
	/*
	 * Determine the size of the resident part of the non-resident
	 * attribute record. (Not compressed thus no compressed_size element
	 * present.)
	 */
	arec_size = (mp_ofs + mp_size + 7) & ~7;

	/* Resize the resident part of the attribute record. */
	err = ntfs_attr_record_resize(ctx->mrec, a, arec_size);
	if (err) {
		/* We want to return an error from this function even for
		 * STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT */
		if (err > 0)
			err = -ENOSPC;
		goto cluster_free_err_out;
	}

	/*
	 * Convert the resident part of the attribute record to describe a
	 * non-resident attribute.
	 */
	a->non_resident = 1;

	/* Move the attribute name if it exists and update the offset. */
	if (a->name_length)
		memmove((u8 *) a + name_ofs,
			(u8 *) a + le16_to_cpu(a->name_offset),
			a->name_length * sizeof(ntfschar));
	a->name_offset = cpu_to_le16(name_ofs);

	/* Setup the fields specific to non-resident attributes. */
	a->lowest_vcn = const_cpu_to_sle64(0);
	a->highest_vcn = cpu_to_sle64((new_allocated_size - 1) >>
				      vol->cluster_size_bits);

	a->mapping_pairs_offset = cpu_to_le16(mp_ofs);

	/*
	 * Update the flags to match the in-memory ones.
	 * However cannot change the compression state if we had
	 * a fuse_file_info open with a mark for release.
	 * The decisions about compression can only be made when
	 * creating/recreating the stream, not when making non resident.
	 */
	a->flags &= ~(ATTR_IS_SPARSE | ATTR_IS_ENCRYPTED);
	if ((a->flags & ATTR_COMPRESSION_MASK) == ATTR_IS_COMPRESSED) {
		/* support only ATTR_IS_COMPRESSED compression mode */
		a->compression_unit = STANDARD_COMPRESSION_UNIT;
		a->compressed_size = const_cpu_to_sle64(0);
	} else {
		a->compression_unit = 0;
		a->flags &= ~ATTR_COMPRESSION_MASK;
		na->data_flags = a->flags;
	}

	memset(&a->reserved1, 0, sizeof(a->reserved1));

	a->allocated_size = cpu_to_sle64(new_allocated_size);
	a->data_size = a->initialized_size = cpu_to_sle64(na->data_size);

	/* Generate the mapping pairs array in the attribute record. */
	err =
	    ntfs_mapping_pairs_build(vol, (u8 *) a + mp_ofs, arec_size - mp_ofs,
				     rl, 0, NULL) < 0;
	if (err) {
		/* FIXME: Eeek! We need rollback! (AIA) */
		antfs_logger(vol->dev->d_sb->s_id,
			    "Eeek! Failed to build mapping pairs. Leaving "
			    "corrupt attribute record on disk. In memory "
			    "runlist is still intact! Error code is %i. FIXME: "
			    "Need to rollback instead!", err);
	}

	/* Done! */
out:
	antfs_log_leave("err: %d", err);
	return err;

cluster_free_err_out:
	if (rl && ntfs_cluster_free(vol, na, 0, -1) < 0) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Eeek! Failed to release allocated "
				"clusters in error code path. Leaving "
				"inconsistent metadata...");
	}
	NAttrClearNonResident(na);
	NAttrClearFullyMapped(na);
	na->allocated_size = na->data_size;
	na->rl = NULL;
	ntfs_free(rl);
	goto out;
}

static int ntfs_resident_attr_resize(struct ntfs_attr *na, const s64 newsize);

/**
 * ntfs_resident_attr_resize - resize a resident, open ntfs attribute
 * @na:		resident ntfs attribute to resize
 * @newsize:	new size (in bytes) to which to resize the attribute
 *
 * Change the size of a resident, open ntfs attribute @na to @newsize bytes.
 * Can also be used to force an attribute non-resident. In this case, the
 * size cannot be changed.
 *
 * On success return 0
 * On error return values are:
 *	STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT
 *	Negative error code otherwise
 * The following error codes are defined:
 *	-ENOMEM - Not enough memory to complete operation.
 *	-ERANGE - @newsize is not valid for the attribute type of @na.
 *	-ENOSPC - There is no enough space in base mft to resize $ATTRIBUTE_LIST
 */
static int ntfs_resident_attr_resize_i(struct ntfs_attr *na, const s64 newsize,
				       enum hole_type holes)
{
	struct ntfs_attr_search_ctx *ctx;
	struct ntfs_volume *vol;
	struct ntfs_inode *ni;
	int err;

	antfs_log_enter("Inode 0x%llx attr 0x%x new size %lld",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)newsize);

	/* Get the attribute record that needs modification. */
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
	err = ntfs_attr_lookup(na->type, na->name, na->name_len, 0, 0, NULL, 0,
			       ctx);
	if (err) {
		antfs_log_error("ntfs_attr_lookup failed");
		goto put_err_out;
	}
	vol = na->ni->vol;
	/*
	 * Check the attribute type and the corresponding minimum and maximum
	 * sizes against @newsize and fail if @newsize is out of bounds.
	 */
	err = ntfs_attr_size_bounds_check(vol, na->type, newsize);
	if (err) {
		if (err == -ENOENT)
			err = -EIO;
		antfs_log_error("bounds check failed");
		goto put_err_out;
	}
	/*
	 * If @newsize is bigger than the mft record we need to make the
	 * attribute non-resident if the attribute type supports it. If it is
	 * smaller we can go ahead and attempt the resize.
	 */
	if ((newsize < vol->mft_record_size) && (holes != HOLES_NONRES)) {
		/* Perform the resize of the attribute record. */
		err = ntfs_resident_attr_value_resize(ctx->mrec, ctx->attr,
						      newsize);
		if (!err) {
			/* Update attribute size everywhere. */
			na->data_size = na->initialized_size = newsize;
			na->allocated_size = (newsize + 7) & ~7;
			if ((na->data_flags & ATTR_COMPRESSION_MASK)
			    || NAttrSparse(na))
				na->compressed_size = na->allocated_size;
			if (na->type == AT_DATA && na->name == AT_UNNAMED)
				NInoFileNameSetDirty(na->ni);
			goto resize_done;
		}
		/* Prefer AT_INDEX_ALLOCATION instead of AT_ATTRIBUTE_LIST */
		if (err == STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT)
			goto put_err_out;
	}
	/* There is not enough space in the mft record to perform the resize. */

	/* Make the attribute non-resident if possible. */
	err = ntfs_attr_make_non_resident(na, ctx);
	if (!err) {
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_put_search_ctx(ctx);
		/*
		 * do not truncate when forcing non-resident, this
		 * could cause the attribute to be made resident again,
		 * so size changes are not allowed.
		 */
		if (holes == HOLES_NONRES) {
			err = 0;
			if (newsize != na->data_size) {
				antfs_log_error("Cannot change size when"
						" forcing non-resident");
				err = -EIO;
			}
			return err;
		}
		/* Resize non-resident attribute */
		return ntfs_attr_truncate_i(na, newsize, holes);
	} else if (err != -ENOSPC && err != -EPERM) {
		antfs_log_error("Failed to make attribute non-resident");
		goto put_err_out;
	}

	/* Try to make other attributes non-resident and retry each time. */
	ntfs_attr_init_search_ctx(ctx, NULL, na->ni->mrec);
	while (!
	       (err =
		ntfs_attr_lookup(AT_UNUSED, NULL, 0, 0, 0, NULL, 0, ctx))) {
		struct ntfs_attr *tna;
		struct ATTR_RECORD *a;

		a = ctx->attr;
		if (a->non_resident)
			continue;

		/*
		 * Check out whether convert is reasonable. Assume that mapping
		 * pairs will take 8 bytes.
		 */
		if (le32_to_cpu(a->length) <= offsetof(struct ATTR_RECORD,
						       compressed_size) +
		    ((a->name_length * sizeof(ntfschar) + 7) & ~7) + 8)
			continue;

		if (a->type == AT_DATA || a->type == AT_INDEX_ROOT)
			tna = ANTFS_NA(na->ni);
		else
			tna = ntfs_attr_open(na->ni, a->type, (ntfschar *)
				    ((u8 *) a + le16_to_cpu(a->name_offset)),
				    a->name_length);
		if (IS_ERR(tna)) {
			err = PTR_ERR(tna);
			antfs_log_error("Couldn't open attribute");
			goto put_err_out;
		}
		err = ntfs_attr_make_non_resident(tna, ctx);
		if (err) {
			if (!(a->type == AT_DATA || a->type == AT_INDEX_ROOT))
				ntfs_attr_close(tna);
			continue;
		}
		if ((tna->type == AT_DATA) && !tna->name_len) {
			/*
			 * If we had to make the unnamed data attribute
			 * non-resident, propagate its new allocated size
			 * to all name attributes and directory indexes
			 */
			NInoFileNameSetDirty(tna->ni);
		}
		if (!(a->type == AT_DATA || a->type == AT_INDEX_ROOT))
			err = ntfs_attr_pclose(tna);
		if (((tna->data_flags & ATTR_COMPRESSION_MASK)
		     == ATTR_IS_COMPRESSED)
		    && err) {
			ntfs_attr_close(tna);
			goto put_err_out;
		}
		ntfs_inode_mark_dirty(tna->ni);
		if (!(a->type == AT_DATA || a->type == AT_INDEX_ROOT))
			ntfs_attr_close(tna);
		ntfs_attr_put_search_ctx(ctx);
		return ntfs_resident_attr_resize_i(na, newsize, holes);
	}
	/* Check whether error occurred. */
	if (err != -ENOENT) {
		antfs_log_error("Attribute lookup failed 1");
		goto put_err_out;
	}

	/*
	 * The standard information and attribute list attributes can't be
	 * moved out from the base MFT record, so try to move out others.
	 */
	if (na->type == AT_STANDARD_INFORMATION
	    || na->type == AT_ATTRIBUTE_LIST) {
		ntfs_attr_put_search_ctx(ctx);
		if (!NInoAttrList(na->ni)) {
			err = ntfs_inode_add_attrlist(na->ni);
			if (err) {
				antfs_log_error("Could not add attribute list: "
						"%d", err);
				return err;
			}
		}
		err = ntfs_inode_free_space(na->ni, offsetof(struct ATTR_RECORD,
					    non_resident_end) + 8);
		if (err) {
			antfs_log_error("Could not free space in MFT record: "
					"%d", err);
			return err;
		}
		return ntfs_resident_attr_resize_i(na, newsize, holes);
	}

	/*
	 * Move the attribute to a new mft record, creating an attribute list
	 * attribute or modifying it if it is already present.
	 */

	/* Point search context back to attribute which we need resize. */
	ntfs_attr_init_search_ctx(ctx, na->ni, NULL);
	err = ntfs_attr_lookup(na->type, na->name, na->name_len, CASE_SENSITIVE,
			       0, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Attribute lookup failed 2");
		goto put_err_out;
	}

	/*
	 * Check whether attribute is already single in this MFT record.
	 * 8 added for the attribute terminator.
	 */
	if (le32_to_cpu(ctx->mrec->bytes_in_use) ==
	    le16_to_cpu(ctx->mrec->attrs_offset) +
	    le32_to_cpu(ctx->attr->length) + 8) {
		antfs_log_debug("MFT record is filled with one attribute");
		err = STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT;
		goto put_err_out;
	}

	/* Add attribute list if not present. */
	if (na->ni->nr_extents == -1)
		ni = na->ni->base_ni;
	else
		ni = na->ni;
	if (!NInoAttrList(ni)) {
		ntfs_attr_put_search_ctx(ctx);
		err = ntfs_inode_add_attrlist(ni);
		if (err)
			return err;
		return ntfs_resident_attr_resize_i(na, newsize, holes);
	}
	/* Allocate new mft record. */
	ni = ntfs_mft_record_alloc(vol, ni);
	if (IS_ERR(ni)) {
		err = PTR_ERR(ni);
		antfs_log_error("Couldn't allocate new MFT record: %d", err);
		goto put_err_out;
	}
	/* Move attribute to it. */
	err = ntfs_attr_record_move_to(ctx, ni);
	if (err) {
		antfs_log_error("Couldn't move attribute to new MFT record: %d",
				err);
		goto put_err_out;
	}
	/* Update ntfs attribute. */
	if (na->ni->nr_extents == -1)
		na->ni = ni;

	ntfs_attr_put_search_ctx(ctx);
	/* Try to perform resize once again. */
	return ntfs_resident_attr_resize_i(na, newsize, holes);

resize_done:
	/*
	 * Set the inode (and its base inode if it exists) dirty so it is
	 * written out later.
	 */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("ok");
	return 0;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_leave("err: %d", err);
	return err;
}

static int ntfs_resident_attr_resize(struct ntfs_attr *na, const s64 newsize)
{
	int ret;

	antfs_log_enter("newsize_ %lld", newsize);
	ret = ntfs_resident_attr_resize_i(na, newsize, HOLES_OK);
	antfs_log_leave("%d", ret);
	return ret;
}

/*
 * If we are in the first extent, then set/clean sparse bit,
 * update allocated and compressed size.
 *
 * On success return 0 and on error return the error code and free ctx.
 *
 * -EAGAIN means retry, other error codes are actual errors
 */
static int ntfs_attr_update_meta(struct ATTR_RECORD *a, struct ntfs_attr *na,
				 struct MFT_RECORD *m, enum hole_type holes,
				 struct ntfs_attr_search_ctx *ctx)
{
	int sparse, err = 0;
	bool sparse_changed = false;

	antfs_log_enter("inode 0x%llx, attr 0x%x",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type));

	if (a->lowest_vcn) {
		antfs_log_leave("lowest vcn");
		return 0;
	}

	a->allocated_size = cpu_to_sle64(na->allocated_size);

	/* Update sparse bit, unless this is an intermediate state */
	if (holes == HOLES_DELAY)
		sparse = (a->flags & ATTR_IS_SPARSE) != const_cpu_to_le16(0);
	else {
		sparse = ntfs_rl_sparse(na->rl);
		if (sparse < 0) {
			antfs_log_leave("sparse %d", sparse);
			err = sparse;
			goto err_put;
		}
	}

	/* Check whether attribute becomes sparse, unless check is delayed. */
	if ((holes != HOLES_DELAY)
	    && sparse && !(a->flags & (ATTR_IS_SPARSE | ATTR_IS_COMPRESSED))) {
		/*
		 * Move attribute to another mft record, if attribute is too
		 * small to add compressed_size field to it and we have no
		 * free space in the current mft record.
		 */
		if ((le32_to_cpu(a->length) -
		     le16_to_cpu(a->mapping_pairs_offset) == 8)
		    && !(le32_to_cpu(m->bytes_allocated) -
			 le32_to_cpu(m->bytes_in_use))) {

			if (!NInoAttrList(na->ni)) {
				/* put here because ntfs_inode_add_attrlist
				 * allocates a new search ctx */
				ntfs_attr_put_search_ctx(ctx);
				err = ntfs_inode_add_attrlist(na->ni);
				if (err)
					return err;
				antfs_log_leave("KEEP_SEARCHING");
				return -EAGAIN;
			}
			err = ntfs_attr_record_move_away(ctx, 8);
			if (err) {
				antfs_log_error("Failed to move attribute");
				goto err_put;
			}
			antfs_log_leave("KEEP_SEARCHING");
			err = -EAGAIN;
			goto err_put;
		}
		if (!
		    (le32_to_cpu(a->length) -
		     le16_to_cpu(a->mapping_pairs_offset))) {
			antfs_log_error("Mapping pairs space is 0");
			err = -EIO;
			goto err_put;
		}

		NAttrSetSparse(na);
		a->flags |= ATTR_IS_SPARSE;
		na->data_flags = a->flags;
		a->compression_unit = STANDARD_COMPRESSION_UNIT;
		/* Windows set it so, even if attribute is not actually
		 * compressed. */

		memmove((u8 *) a + le16_to_cpu(a->name_offset) + 8,
			(u8 *) a + le16_to_cpu(a->name_offset),
			a->name_length * sizeof(ntfschar));

		a->name_offset = cpu_to_le16(le16_to_cpu(a->name_offset) + 8);

		a->mapping_pairs_offset =
		    cpu_to_le16(le16_to_cpu(a->mapping_pairs_offset) + 8);
		sparse_changed = true;
	}

	/* Attribute no longer sparse. */
	if (!sparse && (a->flags & ATTR_IS_SPARSE) &&
	    !(a->flags & ATTR_IS_COMPRESSED)) {

		NAttrClearSparse(na);
		a->flags &= ~ATTR_IS_SPARSE;
		na->data_flags = a->flags;
		a->compression_unit = 0;

		memmove((u8 *) a + le16_to_cpu(a->name_offset) - 8,
			(u8 *) a + le16_to_cpu(a->name_offset),
			a->name_length * sizeof(ntfschar));

		if (le16_to_cpu(a->name_offset) >= 8)
			a->name_offset =
			    cpu_to_le16(le16_to_cpu(a->name_offset) - 8);

		a->mapping_pairs_offset =
		    cpu_to_le16(le16_to_cpu(a->mapping_pairs_offset) - 8);
	}

	/* Initialize compressed size if required (attribute was not sparse
	 * before but is now).
	 */
	if (sparse_changed
	    && NAttrFullyMapped(na)
	    && (sparse || (na->data_flags & ATTR_COMPRESSION_MASK))) {
		s64 new_compr_size;

		new_compr_size =
		    ntfs_rl_get_compressed_size(na->ni->vol, na->rl);
		if (new_compr_size < 0) {
			antfs_log_leave("compr fail");
			err = new_compr_size;
			goto err_put;
		}

		na->compressed_size = new_compr_size;
		a->compressed_size = cpu_to_sle64(new_compr_size);
	}
	/*
	 * Set FILE_NAME dirty flag, to update sparse bit and
	 * allocated size in the index.
	 */
	if (na->type == AT_DATA && na->name == AT_UNNAMED)
		NInoFileNameSetDirty(na->ni);

	antfs_log_leave("ok");
	return err;

err_put:
	ntfs_attr_put_search_ctx(ctx);
	return err;

}

#define NTFS_VCN_DELETE_MARK -2

/**
 * ntfs_attr_update_mapping_pairs_i - see ntfs_attr_update_mapping_pairs
 */
static int ntfs_attr_update_mapping_pairs_i(struct ntfs_attr *na, VCN from_vcn,
					    enum hole_type holes)
{
	struct ntfs_attr_search_ctx *ctx;
	struct ntfs_inode *ni, *base_ni;
	struct MFT_RECORD *m;
	struct ATTR_RECORD *a;
	VCN stop_vcn;
	const struct runlist_element *stop_rl;
	int err = 0, mp_size, cur_max_mp_size, exp_max_mp_size;
	bool finished_build;
	bool first_updated = FALSE;

retry:
	if (IS_ERR_OR_NULL(na) || !na->rl) {
		antfs_log_error("na=%p; na->rl=%p", na, na ? na->rl : NULL);
		err = -EINVAL;
		goto out;
	}

	antfs_log_enter("inode %llu, attr 0x%x, holes: %u",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), holes);

	if (!NAttrNonResident(na)) {
		err = -EINVAL;
		antfs_log_error("resident attribute");
		goto out;
	}
#if PARTIAL_RUNLIST_UPDATING
	/*
	 * For a file just been made sparse, we will have
	 * to reformat the first extent, so be sure the
	 * runlist is fully mapped and fully processed.
	 * Same if the file was sparse and is not any more.
	 * Note : not needed if the full runlist is to be processed
	 */
	if ((holes != HOLES_DELAY)
	    && (!NAttrFullyMapped(na) || from_vcn)
	    && !(na->data_flags & ATTR_IS_COMPRESSED)) {
		bool changed;

		if (!(na->data_flags & ATTR_IS_SPARSE)) {
			int sparse = 0;
			struct runlist_element *xrl;

			/*
			 * If attribute was not sparse, we only
			 * have to check whether there is a hole
			 * in the updated region.
			 */
			for (xrl = na->rl; xrl->length; xrl++) {
				if (xrl->lcn < 0) {
					if (xrl->lcn == LCN_HOLE) {
						sparse = 1;
						break;
					}
					if (xrl->lcn != LCN_RL_NOT_MAPPED) {
						sparse = -1;
						break;
					}
				}
			}
			if (sparse < 0) {
				antfs_log_error
				    ("Could not check whether sparse");
				err = -EIO;
				goto out;
			}
			changed = sparse > 0;
		} else {
			/*
			 * If attribute was sparse, the compressed
			 * size has been maintained, and it gives
			 * and easy way to check whether the
			 * attribute is still sparse.
			 */
			changed = (((na->data_size - 1)
				    | (na->ni->vol->cluster_size - 1)) + 1)
			    == na->compressed_size;
		}
		if (changed) {
			err = ntfs_attr_map_whole_runlist(na);
			if (err) {
				antfs_log_error
				    ("Could not map whole for sparse change");
				goto out;
			}
			from_vcn = 0;
		}
	}
#endif
	if (na->ni->nr_extents == -1)
		base_ni = na->ni->base_ni;
	else
		base_ni = na->ni;

	ctx = ntfs_attr_get_search_ctx(base_ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto out;
	}

	/* Fill attribute records with new mapping pairs. */
	stop_vcn = 0;
	stop_rl = na->rl;
	finished_build = FALSE;
	while (!(err = ntfs_attr_lookup(na->type, na->name, na->name_len,
					CASE_SENSITIVE, from_vcn, NULL, 0,
					ctx))) {
		a = ctx->attr;
		m = ctx->mrec;
		if (!a->lowest_vcn)
			first_updated = TRUE;
		/*
		 * If runlist is updating not from the beginning, then set
		 * @stop_vcn properly, i.e. to the lowest vcn of record that
		 * contain @from_vcn. Also we do not need @from_vcn anymore,
		 * set it to 0 to make ntfs_attr_lookup enumerate attributes.
		 */
		if (from_vcn) {
			LCN first_lcn;

			stop_vcn = sle64_to_cpu(a->lowest_vcn);
			from_vcn = 0;
			/*
			 * Check whether the first run we need to update is
			 * the last run in runlist, if so, then deallocate
			 * all attrubute extents starting this one.
			 */
			first_lcn = ntfs_rl_vcn_to_lcn(na->rl, stop_vcn);
			if (first_lcn == LCN_EINVAL) {
				antfs_log_error("Bad runlist");
				err = -EIO;
				goto put_err_out;
			}
			if (first_lcn == LCN_ENOENT ||
			    first_lcn == LCN_RL_NOT_MAPPED)
				finished_build = TRUE;
		}

		/*
		 * Check whether we finished mapping pairs build, if so mark
		 * extent as need to delete (by setting highest vcn to
		 * NTFS_VCN_DELETE_MARK (-2), we shall check it later and
		 * delete extent) and continue search.
		 */
		if (finished_build) {
			antfs_log_debug("Mark attr 0x%x for delete in inode "
					"%lld.",
					(unsigned)le32_to_cpu(a->type),
					(long long)ctx->ntfs_ino->mft_no);
			a->highest_vcn = cpu_to_sle64(NTFS_VCN_DELETE_MARK);
			ntfs_inode_mark_dirty(ctx->ntfs_ino);
			continue;
		}

		err = ntfs_attr_update_meta(a, na, m, holes, ctx);
		if (err == -EAGAIN)
			goto retry;
		else if (err)
			goto out;

		/*
		 * Determine maximum possible length of mapping pairs,
		 * if we shall *not* expand space for mapping pairs.
		 */
		cur_max_mp_size = le32_to_cpu(a->length) -
		    le16_to_cpu(a->mapping_pairs_offset);
		/*
		 * Determine maximum possible length of mapping pairs in the
		 * current mft record, if we shall expand space for mapping
		 * pairs.
		 */
		exp_max_mp_size = le32_to_cpu(m->bytes_allocated) -
		    le32_to_cpu(m->bytes_in_use) + cur_max_mp_size;
		/* Get the size for the rest of mapping pairs array. */
		mp_size = ntfs_get_size_for_mapping_pairs(na->ni->vol, stop_rl,
							  stop_vcn,
							  exp_max_mp_size);
		if (mp_size <= 0) {
			antfs_log_error("get MP size failed");
			err = mp_size ? mp_size : -EIO;
			goto put_err_out;
		}
		/* Test mapping pairs for fitting in the current mft record. */
		if (mp_size > exp_max_mp_size) {
			/*
			 * Mapping pairs of $ATTRIBUTE_LIST attribute must fit
			 * in the base mft record. Try to move out other
			 * attributes and try again.
			 */
			if (na->type == AT_ATTRIBUTE_LIST) {
				ntfs_attr_put_search_ctx(ctx);
				if (ntfs_inode_free_space(na->ni, mp_size -
							  cur_max_mp_size)) {
					antfs_log_error("Attribute list is too "
							"big. Defragment the "
							"volume");
					err = -EIO;
					goto out;
				}
				goto retry;
			}

			/* Add attribute list if it isn't present, and retry. */
			if (!NInoAttrList(base_ni)) {
				ntfs_attr_put_search_ctx(ctx);
				err = ntfs_inode_add_attrlist(base_ni);
				if (err) {
					antfs_log_error("Can not add attrlist");
					goto out;
				}
				goto retry;
			}

			/*
			 * Set mapping pairs size to maximum possible for this
			 * mft record. We shall write the rest of mapping pairs
			 * to another MFT records.
			 */
			mp_size = exp_max_mp_size;
		}

		/* Change space for mapping pairs if we need it. */
		if (((mp_size + 7) & ~7) != cur_max_mp_size) {
			err = ntfs_attr_record_resize(m, a,
			      le16_to_cpu(a->mapping_pairs_offset) + mp_size);
			if (err) {
				antfs_log_error("Failed to resize attribute");
				/* Always make this error here */
				err = -EIO;
				goto put_err_out;
			}
		}

		/* Update lowest vcn. */
		a->lowest_vcn = cpu_to_sle64(stop_vcn);
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		if ((ctx->ntfs_ino->nr_extents == -1 ||
		     NInoAttrList(ctx->ntfs_ino)) &&
		    ctx->attr->type != AT_ATTRIBUTE_LIST) {
			ctx->al_entry->lowest_vcn = cpu_to_sle64(stop_vcn);
			ntfs_attrlist_mark_dirty(ctx->ntfs_ino);
		}

		/*
		 * Generate the new mapping pairs array directly into the
		 * correct destination, i.e. the attribute record itself.
		 */
		err =
		    ntfs_mapping_pairs_build(na->ni->vol,
					     (u8 *) a +
					     le16_to_cpu(a->
							 mapping_pairs_offset),
					     mp_size, na->rl, stop_vcn,
					     &stop_rl);
		if (!err)
			finished_build = TRUE;
		if (stop_rl)
			stop_vcn = stop_rl->vcn;
		else
			stop_vcn = 0;
		if (!finished_build && err != -ENOSPC) {
			antfs_log_error("Failed to build mapping pairs");
			goto put_err_out;
		}
		a->highest_vcn = cpu_to_sle64(stop_vcn - 1);
	}
	/* Check whether error occurred. */
	if (err != -ENOENT) {
		antfs_log_error("Attribute lookup failed");
		goto put_err_out;
	}
	/*
	 * If the base extent was skipped in the above process,
	 * we still may have to update the sizes.
	 */
	if (!first_updated) {
		le16 spcomp;

		ntfs_attr_reinit_search_ctx(ctx);
		err = ntfs_attr_lookup(na->type, na->name, na->name_len,
				       CASE_SENSITIVE, 0, NULL, 0, ctx);
		if (!err) {
			a = ctx->attr;
			a->allocated_size = cpu_to_sle64(na->allocated_size);
			spcomp = na->data_flags
			    & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE);
			if (spcomp)
				a->compressed_size =
					cpu_to_sle64(na->compressed_size);

			if ((na->type == AT_DATA) && (na->name == AT_UNNAMED))
				NInoFileNameSetDirty(na->ni);
		} else {
			antfs_log_error
			    ("Failed to update sizes in base extent");
			goto put_err_out;
		}
	}

	/* Deallocate not used attribute extents and return with success. */
	if (finished_build) {
		ntfs_attr_reinit_search_ctx(ctx);
		antfs_log_debug("Deallocate marked extents.");
		while (!
		       (err =
			ntfs_attr_lookup(na->type, na->name, na->name_len,
					 CASE_SENSITIVE, 0, NULL, 0, ctx))) {
			if (sle64_to_cpu(ctx->attr->highest_vcn) !=
			    NTFS_VCN_DELETE_MARK)
				continue;
			/* Remove unused attribute record. */
			err = ntfs_attr_record_rm(ctx);
			if (err) {
				antfs_log_error("Could not remove unused attr");
				goto put_err_out;
			}
			ntfs_attr_reinit_search_ctx(ctx);
		}
		if (err != -ENOENT) {
			antfs_log_error("Attr lookup failed");
			goto put_err_out;
		}
		antfs_log_debug("Deallocate done.");
		ntfs_attr_put_search_ctx(ctx);
		goto ok;
	}
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;

	/* Allocate new MFT records for the rest of mapping pairs. */
	while (1) {
		/* Calculate size of rest mapping pairs. */
		mp_size = ntfs_get_size_for_mapping_pairs(na->ni->vol,
							  na->rl, stop_vcn,
							  INT_MAX);
		if (mp_size <= 0) {
			antfs_log_error("get mp size failed");
			err = mp_size ? mp_size : -EIO;
			goto put_err_out;
		}
		ni = ntfs_mft_record_alloc(na->ni->vol, base_ni);
		if (IS_ERR(ni)) {
			err = PTR_ERR(ni);
			antfs_log_warning("Could not allocate new MFT record: "
					"err=%d", err);
			goto put_err_out;
		}
		m = ni->mrec;
		/*
		 * If mapping size exceed available space, set them to
		 * possible maximum.
		 */
		cur_max_mp_size = le32_to_cpu(m->bytes_allocated) -
		    le32_to_cpu(m->bytes_in_use) -
		    (offsetof(struct ATTR_RECORD, compressed_size) +
		     (((na->data_flags & ATTR_COMPRESSION_MASK)
		       || NAttrSparse(na)) ?
		      sizeof(a->compressed_size) : 0)) -
		    ((sizeof(ntfschar) * na->name_len + 7) & ~7);
		if (mp_size > cur_max_mp_size)
			mp_size = cur_max_mp_size;
		/* Add attribute extent to new record. */
		err = ntfs_non_resident_attr_record_add(ni, na->type,
							na->name, na->name_len,
							stop_vcn, mp_size,
							na->data_flags);
		if (err < 0) {
			antfs_log_error("Could not add attribute extent");
			clear_nlink(ANTFS_I(ni));
			ntfs_inode_close(ni);
			goto put_err_out;
		}
		a = (struct ATTR_RECORD *) ((u8 *) m + err);

		err = ntfs_mapping_pairs_build(na->ni->vol, (u8 *) a +
		       le16_to_cpu(a->mapping_pairs_offset), mp_size, na->rl,
		       stop_vcn, &stop_rl);
		if (stop_rl)
			stop_vcn = stop_rl->vcn;
		else
			stop_vcn = 0;
		if (err < 0 && err != -ENOSPC) {
			antfs_log_error("Failed to build MP");
			clear_nlink(ANTFS_I(ni));
			ntfs_inode_close(ni);
			goto put_err_out;
		}
		a->highest_vcn = cpu_to_sle64(stop_vcn - 1);
		ntfs_inode_mark_dirty(ni);
		/* All mapping pairs has been written. */
		if (!err)
			break;
	}
ok:
	NAttrClearRunlistDirty(na);
	err = 0;
out:
	antfs_log_leave("Exit: %d", err);
	return err;
put_err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	goto out;
}

#undef NTFS_VCN_DELETE_MARK

/**
 * ntfs_attr_update_mapping_pairs - update mapping pairs for ntfs attribute
 * @na:		non-resident ntfs open attribute for which we need update
 * @from_vcn:	update runlist starting this VCN
 *
 * Build mapping pairs from @na->rl and write them to the disk. Also, this
 * function updates sparse bit, allocated and compressed size (allocates/frees
 * space for this field if required).
 *
 * @na->allocated_size should be set to correct value for the new runlist before
 * call to this function. Vice-versa @na->compressed_size will be calculated and
 * set to correct value during this function.
 *
 * FIXME: This function does not update sparse bit and compressed size correctly
 * if called with @from_vcn != 0.
 *
 * FIXME: Rewrite without using NTFS_VCN_DELETE_MARK define.
 *
 * On success return 0 and on error return the error code.
 * The following error codes are defined:
 *	EINVAL - Invalid arguments passed.
 *	ENOMEM - Not enough memory to complete operation.
 *	ENOSPC - There is no enough space in base mft to resize $ATTRIBUTE_LIST
 *		 or there is no free MFT records left to allocate.
 */
int ntfs_attr_update_mapping_pairs(struct ntfs_attr *na, VCN from_vcn)
{
	int ret;

	antfs_log_enter();
	ret = ntfs_attr_update_mapping_pairs_i(na, from_vcn, HOLES_OK);
	antfs_log_leave();
	return ret;
}

/**
 * ntfs_non_resident_attr_shrink - shrink a non-resident, open ntfs attribute
 * @na:		non-resident ntfs attribute to shrink
 * @newsize:	new size (in bytes) to which to shrink the attribute
 *
 * Reduce the size of a non-resident, open ntfs attribute @na to @newsize bytes.
 *
 * On success return 0 and on error return the error code.
 * The following error codes are defined:
 *	ENOMEM	- Not enough memory to complete operation.
 *	ERANGE	- @newsize is not valid for the attribute type of @na.
 */
static int ntfs_non_resident_attr_shrink(struct ntfs_attr *na,
					 const s64 newsize)
{
	struct ntfs_volume *vol;
	struct ntfs_attr_search_ctx *ctx;
	VCN first_free_vcn;
	s64 nr_freed_clusters;
	int err = 0;

	antfs_log_enter("Inode 0x%llx attr 0x%02x new size %lld",
			(unsigned long long)
			na->ni->mft_no, le32_to_cpu(na->type),
			(long long)newsize);

	vol = na->ni->vol;

	/*
	 * Check the attribute type and the corresponding minimum size
	 * against @newsize and fail if @newsize is too small.
	 */
	err = ntfs_attr_size_bounds_check(vol, na->type, newsize);
	if (err) {
		if (err == -ERANGE) {
			antfs_log_error("Eeek! Size bounds check failed. "
					"Aborting...");
		} else if (err == -ENOENT)
			err = -EIO;
		goto out;
	}

	/* The first cluster outside the new allocation. */
	if (na->data_flags & ATTR_COMPRESSION_MASK)
		/*
		 * For compressed files we must keep full compressions blocks,
		 * but currently we do not decompress/recompress the last
		 * block to truncate the data, so we may leave more allocated
		 * clusters than really needed.
		 */
		first_free_vcn = (((newsize - 1)
				   | (na->compression_block_size - 1)) + 1)
		    >> vol->cluster_size_bits;
	else
		first_free_vcn = (newsize + vol->cluster_size - 1) >>
		    vol->cluster_size_bits;
	/*
	 * Compare the new allocation with the old one and only deallocate
	 * clusters if there is a change.
	 */
	antfs_log_debug("Allocated clusters (old; new): %lld; %lld",
			na->allocated_size >> vol->cluster_size_bits,
			first_free_vcn);
	if ((na->allocated_size >> vol->cluster_size_bits) != first_free_vcn) {
		err = ntfs_attr_map_whole_runlist(na);
		if (err) {
			antfs_log_error("Eeek! ntfs_attr_map_whole_runlist "
					"failed.");
			goto out;
		}
		/* Deallocate all clusters starting with the first free one. */
		nr_freed_clusters = ntfs_cluster_free(vol, na, first_free_vcn,
						      -1);
		if (nr_freed_clusters < 0) {
			antfs_logger(vol->dev->d_sb->s_id,
				    "Eeek! Freeing of clusters failed. "
				    "Aborting...");
			err = nr_freed_clusters;
			goto out;
		}

		if (na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_SPARSE))
			na->compressed_size -= nr_freed_clusters <<
				vol->cluster_size_bits;
		/* Truncate the runlist itself. */
		err = ntfs_rl_truncate(&na->rl, first_free_vcn);
		if (err) {
			/*
			 * Failed to truncate the runlist, so just throw it
			 * away, it will be mapped afresh on next use.
			 */
			ntfs_free(na->rl);
			na->rl = NULL;
			NAttrClearFullyMapped(na);
			antfs_log_error("Eeek! Run list truncation failed.");
			goto out;
		}
		NAttrSetRunlistDirty(na);

		/* Prepare to mapping pairs update. */
		na->allocated_size = first_free_vcn << vol->cluster_size_bits;
		/* Write mapping pairs for new runlist. */
		err = ntfs_attr_update_mapping_pairs(na, 0/*first_free_vcn */);
		if (err) {
			antfs_logger(vol->dev->d_sb->s_id,
				    "Eeek! Mapping pairs update failed. "
				    "Leaving inconsistent metadata. Run "
				    "chkdsk");
			goto out;
		}
	}

	/* Get the first attribute record. */
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (!ctx) {
		err = -ENOMEM;
		goto out;
	}

	err = ntfs_attr_lookup(na->type, na->name, na->name_len, CASE_SENSITIVE,
			       0, NULL, 0, ctx);
	if (err) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Eeek! Lookup of first attribute extent "
				"failed. Leaving inconsistent metadata.");
		if (err == -ENOENT)
			err = -EIO;
		goto put_err_out;
	}

	/* Update data and initialized size. */
	na->data_size = newsize;
	ctx->attr->data_size = cpu_to_sle64(newsize);
	if (newsize < na->initialized_size) {
		na->initialized_size = newsize;
		ctx->attr->initialized_size = cpu_to_sle64(newsize);
	}
	/* Update data size in the index. */
	if (na->type == AT_DATA && na->name == AT_UNNAMED)
		NInoFileNameSetDirty(na->ni);

	/* Set the inode dirty so it is written out later. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	/* Done! */
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
out:
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * ntfs_non_resident_attr_expand_i - expand a non-resident, open ntfs attribute
 * @na:		non-resident ntfs attribute to expand
 * @newsize:	new size (in bytes) to which to expand the attribute
 *
 * Expand the size of a non-resident, open ntfs attribute @na to @newsize bytes,
 * by allocating new clusters.
 *
 * On success return 0 and on error return the error code.
 * The following error codes are defined:
 *	ENOMEM - Not enough memory to complete operation.
 *	ERANGE - @newsize is not valid for the attribute type of @na.
 *	ENOSPC - There is no enough space in base mft to resize $ATTRIBUTE_LIST.
 */
static int ntfs_non_resident_attr_expand_i(struct ntfs_attr *na,
				    const s64 newsize, enum hole_type holes)
{
	LCN lcn_seek_from;
	VCN first_free_vcn;
	struct ntfs_volume *vol;
	struct ntfs_attr_search_ctx *ctx;
	struct runlist_element *rl, *rln;
	s64 org_alloc_size, org_compr_size;
	int err = 0;

	antfs_log_enter("Inode %lld, attr 0x%x, new size %lld old size %lld",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)newsize,
			(long long)na->data_size);

	vol = na->ni->vol;

	/*
	 * Check the attribute type and the corresponding maximum size
	 * against @newsize and fail if @newsize is too big.
	 */
	err = ntfs_attr_size_bounds_check(vol, na->type, newsize);
	if (err) {
		if (err == -ENOENT)
			err = -EIO;
		antfs_log_error("bounds check failed");
		goto out;
	}

	if (na->type == AT_DATA)
		NAttrSetDataAppending(na);
	/* Save for future use. */
	org_alloc_size = na->allocated_size;
	org_compr_size = na->compressed_size;
	/* The first cluster outside the new allocation. */
	first_free_vcn = (newsize + vol->cluster_size - 1) >>
	    vol->cluster_size_bits;
	/*
	 * Compare the new allocation with the old one and only allocate
	 * clusters if there is a change.
	 */
	if ((na->allocated_size >> vol->cluster_size_bits) < first_free_vcn) {
#if PARTIAL_RUNLIST_UPDATING
		s64 start_update;

		/*
		 * Update from the last previously allocated run,
		 * as we may have to expand an existing hole.
		 */
		start_update = na->allocated_size >> vol->cluster_size_bits;
		if (start_update)
			start_update--;
		err = ntfs_attr_map_partial_runlist(na, start_update);
		if (err) {
			antfs_log_error("failed to map partial runlist");
			goto out;
		}
#else
		err = ntfs_attr_map_whole_runlist(na);
		if (err) {
			antfs_log_error("ntfs_attr_map_whole_runlist failed");
			goto out;
		}
#endif

		/*
		 * If we extend $DATA attribute on NTFS 3+ volume, we can add
		 * sparse runs instead of real allocation of clusters.
		 */
		if ((na->type == AT_DATA) && (vol->major_ver >= 3)
		    && (holes != HOLES_NO)) {
			rl = ntfs_malloc(ANTFS_RL_BUF_SIZE);
			if (!rl) {
				err = -ENOMEM;
				goto out;
			}

			rl[0].vcn = (na->allocated_size >>
				     vol->cluster_size_bits);
			rl[0].lcn = LCN_HOLE;
			rl[0].length = first_free_vcn -
			    (na->allocated_size >> vol->cluster_size_bits);
			rl[1].vcn = first_free_vcn;
			rl[1].lcn = LCN_ENOENT;
			rl[1].length = 0;
		} else {
			s64 count;
			/*
			 * Determine first after last LCN of attribute.
			 * We will start seek clusters from this LCN to avoid
			 * fragmentation.  If there are no valid LCNs in the
			 * attribute let the cluster allocator choose the
			 * starting LCN.
			 */
			lcn_seek_from = -1;
			if (na->rl->length) {
				/* Seek to the last run list element. */
				for (rl = na->rl; (rl + 1)->length; rl++)
					/*just run through rl*/;
				/*
				 * If the last LCN is a hole or similar seek
				 * back to last valid LCN.
				 */
				while (rl->lcn < 0 && rl != na->rl)
					rl--;
				/*
				 * Only set lcn_seek_from it the LCN is valid.
				 */
				if (rl->lcn >= 0)
					lcn_seek_from = rl->lcn + rl->length;
			}

			count = first_free_vcn - (na->allocated_size >>
						  vol->cluster_size_bits);
			rl = ntfs_cluster_alloc(vol, na->allocated_size >>
						vol->cluster_size_bits,
						count,
						lcn_seek_from, DATA_ZONE,
						na->data_size);
			if (IS_ERR(rl)) {
				antfs_log_error("Cluster allocation failed "
						"(%lld clusters, err=%d)",
						(long long)count,
						(int)PTR_ERR(rl));
				err = PTR_ERR(rl);
				goto out;
			}

			if (na->data_flags & (ATTR_COMPRESSION_MASK |
					      ATTR_IS_SPARSE))
				na->compressed_size += count <<
					vol->cluster_size_bits;
		}

		/* Append new clusters to attribute runlist. */
		rln = ntfs_runlists_merge(na->rl, rl);
		if (IS_ERR(rln)) {
			/* Failed, free just allocated clusters. */
			err = PTR_ERR(rln);
			antfs_log_error("Run list merge failed");
			ntfs_cluster_free_from_rl(vol, rl);
			ntfs_free(rl);
			goto out;
		}
		na->rl = rln;
		NAttrSetRunlistDirty(na);

		/* Prepare to mapping pairs update. */
		na->allocated_size = first_free_vcn << vol->cluster_size_bits;
#if PARTIAL_RUNLIST_UPDATING
		/*
		 * Write mapping pairs for new runlist, unless this is
		 * a temporary state before appending data.
		 * If the update is not done, we must be sure to do
		 * it later, and to get to a clean state even on errors.
		 */
		err = ntfs_attr_update_mapping_pairs_i(na, start_update, holes);
		if ((holes != HOLES_DELAY) && err) {
#else
		/* Write mapping pairs for new runlist. */
		err = ntfs_attr_update_mapping_pairs(na, 0);
		if (err) {
#endif
			antfs_log_error("Mapping pairs update failed");
			goto rollback;
		}
	}

	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		if (na->allocated_size == org_alloc_size)
			goto out;
		else
			goto rollback;
	}

	err = ntfs_attr_lookup(na->type, na->name, na->name_len, CASE_SENSITIVE,
			       0, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Lookup of first attribute extent failed");
		if (err == -ENOENT)
			err = -EIO;
		if (na->allocated_size != org_alloc_size) {
			ntfs_attr_put_search_ctx(ctx);
			goto rollback;
		} else
			goto put_err_out;
	}

	/* Update data size. */
	na->data_size = newsize;
	ctx->attr->data_size = cpu_to_sle64(newsize);
	/* Update data size in the index. */
	if (na->type == AT_DATA && na->name == AT_UNNAMED)
		NInoFileNameSetDirty(na->ni);
	/* Set the inode dirty so it is written out later. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	/* Done! */
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
out:
	return err;
rollback:
	/* Free allocated clusters. */
	if (ntfs_cluster_free(vol, na, org_alloc_size >>
			      vol->cluster_size_bits, -1) < 0) {
		err = -EIO;
		antfs_log_error("Leaking clusters");
	}
	/* Now, truncate the runlist itself. */
	if (ntfs_rl_truncate(&na->rl, org_alloc_size >>
				vol->cluster_size_bits)) {
		/*
		 * Failed to truncate the runlist, so just throw it away, it
		 * will be mapped afresh on next use.
		 */
		ntfs_free(na->rl);
		na->rl = NULL;
		NAttrClearFullyMapped(na);
		antfs_log_error("Couldn't truncate runlist. Rollback failed");
	} else {
		NAttrSetRunlistDirty(na);
		/* Prepare to mapping pairs update. */
		na->allocated_size = org_alloc_size;
		if (na->data_flags & (ATTR_COMPRESSION_MASK |
				      ATTR_IS_SPARSE))
			na->compressed_size = org_compr_size;
		/* Restore mapping pairs. */
		if (ntfs_attr_update_mapping_pairs(na, 0/*na->allocated_size >>
			vol->cluster_size_bits*/)) {
			antfs_log_error("Failed to restore old mapping pairs");
		}
	}
	goto out;
}

static int ntfs_non_resident_attr_expand(struct ntfs_attr *na,
				const s64 newsize, enum hole_type holes)
{
	int ret;

	antfs_log_enter();
	ret = ntfs_non_resident_attr_expand_i(na, newsize, holes);
	antfs_log_leave();
	return ret;
}

/**
 * ntfs_attr_truncate - resize an ntfs attribute
 * @na:		open ntfs attribute to resize
 * @newsize:	new size (in bytes) to which to resize the attribute
 * @holes:	how to create a hole if expanding
 *
 * Change the size of an open ntfs attribute @na to @newsize bytes. If the
 * attribute is made bigger and the attribute is resident the newly
 * "allocated" space is cleared and if the attribute is non-resident the
 * newly allocated space is marked as not initialised and no real allocation
 * on disk is performed.
 *
 * On success return 0.
 * On error return values are:
 *	STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT
 *	STATUS_ERROR - otherwise
 * The following error codes are defined:
 *	EINVAL	   - Invalid arguments were passed to the function.
 *	EOPNOTSUPP - The desired resize is not implemented yet.
 *	EACCES	   - Encrypted attribute.
 */
int ntfs_attr_truncate_i(struct ntfs_attr *na, const s64 newsize,
			 enum hole_type holes)
{
	int ret = STATUS_ERROR;
	s64 fullsize;
	bool compressed;

	if (IS_ERR_OR_NULL(na) || newsize < 0 ||
	    (na->ni->mft_no == FILE_MFT && na->type == AT_DATA)) {
		antfs_log_error("Invalid arguments passed.");
		return -EINVAL;
	}

	antfs_log_enter("inode %lld, attr 0x%02x, newsize %lld",
			(unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)newsize);

	if (na->data_size == newsize &&
	    !(na->allocated_size > 0 && newsize == 0)) {
		antfs_log_leave("Size is already ok");
		ret = STATUS_OK;
		goto out;
	}
	/*
	 * Encrypted attributes are not supported. We return access denied,
	 * which is what Windows NT4 does, too.
	 */
	if ((na->data_flags & ATTR_IS_ENCRYPTED) && !na->ni->vol->efs_raw) {
		ret = -EACCES;
		antfs_log_info("Cannot truncate encrypted attribute");
		goto out;
	}
	/*
	 * TODO: Implement making handling of compressed attributes.
	 * Currently we can only expand the attribute or delete it,
	 * and only for ATTR_IS_COMPRESSED. This is however possible
	 * for resident attributes when there is no open fuse context
	 * (important case : $INDEX_ROOT:$I30)
	 */
	compressed = (na->data_flags & ATTR_COMPRESSION_MASK)
	    != const_cpu_to_le16(0);
	if (compressed && NAttrNonResident(na)
	    && ((na->data_flags & ATTR_COMPRESSION_MASK) !=
		ATTR_IS_COMPRESSED)) {
		ret = -EOPNOTSUPP;
		antfs_log_error("Failed to truncate compressed attribute");
		goto out;
	}

	if (NAttrNonResident(na)) {
		/*
		 * For compressed data, the last block must be fully
		 * allocated, and we do not know the size of compression
		 * block until the attribute has been made non-resident.
		 * Moreover we can only process a single compression
		 * block at a time (from where we are about to write),
		 * so we silently do not allocate more.
		 *
		 * Note : do not request upsizing of compressed files
		 * unless being able to face the consequences !
		 */
		if (compressed && newsize && (newsize > na->data_size))
			fullsize = (na->initialized_size
				    | (na->compression_block_size - 1)) + 1;
		else
			fullsize = newsize;
		if (fullsize > na->data_size)
			ret = ntfs_non_resident_attr_expand(na, fullsize,
							    holes);
		else
			ret = ntfs_non_resident_attr_shrink(na, fullsize);
	} else
		ret = ntfs_resident_attr_resize_i(na, newsize, holes);

out:
	antfs_log_leave("Return status %d", ret);
	return ret;
}

/*
 *		Resize an attribute, creating a hole if relevant
 */

int ntfs_attr_truncate(struct ntfs_attr *na, const s64 newsize)
{
	int err;

	err = ntfs_attr_truncate_i(na, newsize, HOLES_OK);
	if (unlikely(!err)) {
		NAttrClearDataAppending(na);
		NAttrClearBeingNonResident(na);
	}
	return err;
}

/**
 * @brief Get greatest LCN from end of runlist
 */
static LCN rl_get_next_lcn(struct runlist_element *rl_start)
{
	struct runlist_element *rl = rl_start;

	/* goto end of rl */
	if (!rl || !rl[0].length)
		return (LCN)-1;
	for (; rl[1].length; rl++)
		/*just go through rl*/;
	/* Get last "real" run */
	for (; rl->lcn < 0 && rl != rl_start; rl--)
		/*go through rl*/;
	/* Get last cluster number from it and continue there */
	if (rl->lcn > (LCN) 0)
		return rl->lcn + rl->length;
	return (LCN)-1;
}

/**
 * @brief Do cluster allocation for AT_DATA attribute
 *
 * Allocates @count clusters at @VCN for an open attribute.
 * Runlist is updated with allocated clusters.
 * The code here is in parts from @ref ntfs_non_resident_attr_expand_i.
 *
 * @param ni Pointer to ntfs inode
 * @param na Pointer to open AT_DATA attribute for @ni
 * @param vcn Start allocation at this vcn
 * @param count Number of clusters to allocate
 * @param res_lcn First allocated LCN (can be used for get_block)
 *
 * @return 0 on success or negative error code
 */
int antfs_do_cluster_alloc(struct ntfs_inode *ni, struct ntfs_attr *na, VCN vcn,
			   size_t count, LCN *res_lcn)
{
	VCN start_update;
	LCN first_lcn, next_lcn;
	s64 org_alloc_size = na->allocated_size;
	struct runlist_element *rl, *tmp_rl;
	struct ntfs_volume *vol = ni->vol;
	int err;

	/* FIXME: Either add more attribute type checks here if this is ever
	 * used for something else than AT_DATA or think about modifying
	 * ntfs_non_resident_attr_expand etc. and use these instead... */
	NAttrSetDataAppending(na);

	antfs_log_debug("Allocate new block; VCN: %lld; count: %zu",
			(long long)vcn, count);
#if PARTIAL_RUNLIST_UPDATING
	/*
	 * Update from the last previously allocated run,
	 * as we may have to expand an existing hole.
	 */
	if (vcn) {
		start_update = vcn - 1;
		if (start_update)
			start_update--;
	} else {
		start_update = 0;
	}
	err = ntfs_attr_map_partial_runlist(na, start_update);
	if (err) {
		antfs_log_error("failed to map partial runlist");
		goto err_out;
	}
#else
	err = ntfs_attr_map_whole_runlist(na);
	if (err) {
		antfs_log_error("ntfs_attr_map_whole_runlist failed");
		goto err_out;
	}
#endif

	next_lcn = ntfs_rl_vcn_to_lcn(na->rl, vcn - 1) + 1;
	if (next_lcn <= 0)
		next_lcn = rl_get_next_lcn(na->rl);

	rl = ntfs_cluster_alloc(vol, vcn, count, next_lcn,
				DATA_ZONE, na->data_size);
	if (IS_ERR_OR_NULL(rl)) {
		err = PTR_ERR(rl);
		goto err_out;
	}
	/* New allocated cluster */
	first_lcn = rl->lcn;

	tmp_rl = ntfs_runlists_merge(na->rl, rl);
	if (IS_ERR(tmp_rl)) {
		err = PTR_ERR(tmp_rl);
		antfs_log_error("Merging runlist failed: %d", err);
		if (ntfs_cluster_free_from_rl(vol, rl))
			antfs_logger(vol->dev->d_sb->s_id,
				     "ntfs_cluster_free_from_rl() failed");
		ntfs_free(rl);
		goto err_out;
	}

	na->rl = tmp_rl;
	NAttrSetRunlistDirty(na);

	if ((vcn + count) << vol->cluster_size_bits > na->allocated_size)
		na->allocated_size = (vcn + count) << vol->cluster_size_bits;
	antfs_log_debug("na->allocated_size (%lld)", na->allocated_size);

#if PARTIAL_RUNLIST_UPDATING
	/*
	 * Write mapping pairs for new runlist, unless this is
	 * a temporary state before appending data.
	 * If the update is not done, we must be sure to do
	 * it later, and to get to a clean state even on errors.
	 */
	err = ntfs_attr_update_mapping_pairs_i(na, start_update, HOLES_NO);
#else
	err = ntfs_attr_update_mapping_pairs(na, 0);
#endif
	if (err) {
		antfs_log_error("Mapping pairs update failed");
		goto rollback;
	}

	if (na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_SPARSE))
		na->compressed_size += count << vol->cluster_size_bits;

	/* TODO: Is this correct (also see below)? */
	NAttrClearDataAppending(na);
	/* Data size is updated later (write_end?) */
	*res_lcn = first_lcn;
	antfs_log_leave("Exit: 0");
	return 0;

rollback:
	/* Free allocated clusters. */
	if (ntfs_cluster_free(vol, na, org_alloc_size >> vol->cluster_size_bits,
				-1) < 0) {
		antfs_log_error("Cluster free failed. rl in RAM damaged.");
		antfs_logger(vol->dev->d_sb->s_id,
			     "Leaking clusters.");
		err = -EIO;
		/* Stuff in rl is garbage. Get rid of it. */
		goto err_na;
	}
	/* Now, truncate the runlist itself. */
	if (ntfs_rl_truncate(&na->rl, org_alloc_size >>
		vol->cluster_size_bits)) {
		/*
		 * Failed to truncate the runlist, so just throw it away, it
		 * will be mapped afresh on next use.
		 */
err_na:
		ntfs_free(na->rl);
		na->rl = NULL;
		NAttrClearFullyMapped(na);
		antfs_logger(vol->dev->d_sb->s_id, "Couldn't truncate runlist. "
				"Have to re-build it.");
	} else {
		NAttrSetRunlistDirty(na);
		/* Prepare to mapping pairs update. */
		na->allocated_size = org_alloc_size;
		/* Restore mapping pairs. */
		if (ntfs_attr_update_mapping_pairs(na, 0/*na->allocated_size >>
			vol->cluster_size_bits */)) {
			antfs_logger(vol->dev->d_sb->s_id,
				    "Failed to restore old mapping pairs");
		}
	}
err_out:
	NAttrClearDataAppending(na);
	return err;
}

/**
 * @note may return STATUS_ATTRIBUTE_FILLED_MFT
 */
static int stuff_hole(struct ntfs_attr *na, const s64 pos)
{
	s64 size;
	s64 begin_size;
	s64 end_size;
	s64 tmp_size;
	char *buf;
	int ret;

	ret = 0;
	/*
	 * If the attribute is resident, the compression block size
	 * is not defined yet and we can make no decision.
	 * So we first try resizing to the target and if the
	 * attribute is still resident, we're done
	 */

	antfs_log_debug("--> pos=%lld", pos);

	if (!NAttrNonResident(na)) {
		ret = ntfs_resident_attr_resize(na, pos);
		if (!ret && !NAttrNonResident(na))
			na->initialized_size = na->data_size = pos;
	}
	if (!ret && NAttrNonResident(na)) {
		/* does the hole span over several compression block ? */
		if ((pos ^ na->initialized_size)
		    & ~(na->compression_block_size - 1)) {
			begin_size = ((na->initialized_size - 1)
				      | (na->compression_block_size - 1))
			    + 1 - na->initialized_size;
			end_size = pos & (na->compression_block_size - 1);
			size = (begin_size > end_size ? begin_size : end_size);
		} else {
			/* short stuffing in a single compression block */
			begin_size = size = pos - na->initialized_size;
			end_size = 0;
		}
		if (size) {
			antfs_log_debug
			    ("malloc -- size: %lld; na->initialized_size=%lld; "
			     "begin_size: %lld; end_size: %lld; "
			     "na->compression_bock_size: %u", size,
			     na->initialized_size, begin_size, end_size,
			     na->compression_block_size);
			buf = (char *)ntfs_malloc(size);
		} else
			buf = (char *)NULL;
		if (buf || !size) {
			memset(buf, 0, size);
			/* stuff into current block */
			if (begin_size) {
				tmp_size = ntfs_attr_pwrite(na,
							    na->
							    initialized_size,
							    begin_size, buf);
				if (tmp_size != begin_size)
					ret =
					    (tmp_size <
					     0) ? (int)tmp_size : -EIO;
			}
			/* create an unstuffed hole */
			if (!ret && ((na->initialized_size + end_size) < pos))
				ret = ntfs_non_resident_attr_expand(na,
								    pos -
								    end_size,
								    HOLES_OK);
			else
				na->initialized_size
				    = na->data_size = pos - end_size;
			/* stuff into the target block */
			if (!ret && end_size) {
				tmp_size = ntfs_attr_pwrite(na,
							    na->
							    initialized_size,
							    end_size, buf);
				if (tmp_size != end_size)
					ret =
					    (tmp_size <
					     0) ? (int)tmp_size : -EIO;
			}
			if (buf)
				ntfs_free(buf);
		} else
			ret = -EIO;
	}
	/* make absolutely sure we have reached the target */
	if (!ret && (na->initialized_size != pos)) {
		antfs_log_error("Failed to stuff a compressed file "
				"target %lld reached %lld",
				(long long)pos,
				(long long)na->initialized_size);
		ret = -EIO;
	}
	antfs_log_leave("Exit: %d", ret);
	return ret;
}

/**
 * ntfs_attr_readall - read the entire data from an ntfs attribute
 * @ni:		open ntfs inode in which the ntfs attribute resides
 * @type:	attribute type
 * @name:	attribute name in little endian Unicode or AT_UNNAMED or NULL
 * @name_len:	length of attribute @name in Unicode characters (if @name given)
 * @data_size:	if non-NULL then store here the data size
 *
 * This function will read the entire content of an ntfs attribute.
 * If @name is AT_UNNAMED then look specifically for an unnamed attribute.
 * If @name is NULL then the attribute could be either named or not.
 * In both those cases @name_len is not used at all.
 *
 * On success a buffer is allocated with the content of the attribute
 * and which needs to be freed when it's not needed anymore. If the
 * @data_size parameter is non-NULL then the data size is set there.
 *
 * On error the error code is returned.
 */
void *ntfs_attr_readall(struct ntfs_inode *ni, const enum ATTR_TYPES type,
			ntfschar *name, u32 name_len, s64 *data_size)
{
	struct ntfs_attr *na;
	void *data;
	void *ret = NULL;
	s64 size;

	antfs_log_enter();

	na = ntfs_attr_open(ni, type, name, name_len);
	if (IS_ERR(na)) {
		antfs_log_error("ntfs_attr_open failed, inode %lld attr 0x%lx",
				(long long)ni->mft_no, (long)le32_to_cpu(type));
		ret = na;
		goto err_exit;
	}
	data = ntfs_malloc(na->data_size);
	if (!data) {
		ret = ERR_PTR(-ENOMEM);
		goto out;
	}

	size = ntfs_attr_pread(na, 0, na->data_size, data);
	if (size != na->data_size) {
		antfs_log_error("ntfs_attr_pread failed");
		ntfs_free(data);
		if (size < 0)
			ret = ERR_PTR(size);
		else
			ret = ERR_PTR(-EIO);
		goto out;
	}
	ret = data;
	if (data_size)
		*data_size = size;
out:
	ntfs_attr_close(na);
err_exit:
	antfs_log_leave();
	return ret;
}

int ntfs_attr_exist(struct ntfs_inode *ni, const enum ATTR_TYPES type,
		    const ntfschar *name, u32 name_len)
{
	struct ntfs_attr_search_ctx *ctx;
	int ret;

	antfs_log_enter();

	if (type == AT_DATA || type == AT_INDEX_ROOT)
		return 1;

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx)) {
		antfs_log_error("no search_ctx");
		return 0;
	}

	ret = ntfs_attr_lookup(type, name, name_len, CASE_SENSITIVE, 0, NULL, 0,
			       ctx);

	ntfs_attr_put_search_ctx(ctx);

	antfs_log_leave("ret: %d", ret);
	return ret < 0 ? 0 : 1;
}

int ntfs_attr_remove(struct ntfs_inode *ni, const enum ATTR_TYPES type,
		     ntfschar *name, u32 name_len)
{
	struct ntfs_attr *na = NULL;
	int ret;

	antfs_log_enter();

	if (!ni) {
		antfs_log_error("NULL inode pointer");
		return -EINVAL;
	}

	if (type == AT_DATA || type == AT_INDEX_ROOT) {
		na = ANTFS_NA(ni);
		/* This ALWAYS happens on rollback in __ntfs_create as
		 * na->ni here only gets set in antfs_inode_init which
		 * is called later AFTER __ntfs_create is done.
		 * Cannot use ANTFS_NA here at all!
		 * So set na to NULL and open the attribute below.
		 */
		if (!na->ni) {
			na = NULL;
		} else if (IS_ERR(na->ni)) {
			/* This must not happen though. */
			antfs_log_error("ino 0x%llx; na->ni is error %d!\n",
					(long long)ni->mft_no,
					(int)PTR_ERR(na->ni));
			na = NULL;
		}
	}

	if (!na)
		na = ntfs_attr_open(ni, type, name, name_len);

	if (IS_ERR(na)) {
		/* do not log removal of non-existent stream */
		if (type != AT_DATA) {
			antfs_log_error
			    ("Failed to open attribute 0x%02x of inode "
			     "0x%llx", le32_to_cpu(type),
			     (unsigned long long)ni->mft_no);
		}
		return PTR_ERR(na);
	}

	ret = ntfs_attr_rm(na);
	if (ret)
		antfs_log_error("Failed to remove attribute 0x%02x of inode "
				"0x%llx", le32_to_cpu(type),
				(unsigned long long)ni->mft_no);
	if (na != ANTFS_NA(ni))
		ntfs_attr_close(na);
	else
		na->ni = NULL;

	return ret;
}

#if 0
/* Below macros are 32-bit ready. */
#define BCX(x) ((x) - (((x) >> 1) & 0x77777777) - \
		      (((x) >> 2) & 0x33333333) - \
		      (((x) >> 3) & 0x11111111))
#define BITCOUNT(x) (((BCX(x) + (BCX(x) >> 4)) & 0x0F0F0F0F) % 255)
#endif

static const int bitfree_lut256[256] = {
	/* 00..0F */
	8, 7, 7, 6, 7, 6, 6, 5,
	7, 6, 6, 5, 6, 5, 5, 4,
	/* 10..1F */
	7, 6, 6, 5, 6, 5, 5, 4,
	6, 5, 5, 4, 5, 4, 4, 3,
	/* 20..2F */
	7, 6, 6, 5, 6, 5, 5, 4,
	6, 5, 5, 4, 5, 4, 4, 3,
	/* 30..3F */
	6, 5, 5, 4, 5, 4, 4, 3,
	5, 4, 4, 3, 4, 3, 3, 2,
	/* 40..4F */
	7, 6, 6, 5, 6, 5, 5, 4,
	6, 5, 5, 4, 5, 4, 4, 3,
	/* 50..5F */
	6, 5, 5, 4, 5, 4, 4, 3,
	5, 4, 4, 3, 4, 3, 3, 2,
	/* 60..6F */
	6, 5, 5, 4, 5, 4, 4, 3,
	5, 4, 4, 3, 4, 3, 3, 2,
	/* 70..7F */
	5, 4, 4, 3, 4, 3, 3, 2,
	4, 3, 3, 2, 3, 2, 2, 1,
	/* 80..8F */
	7, 6, 6, 5, 6, 5, 5, 4,
	6, 5, 5, 4, 5, 4, 4, 3,
	/* 90..9F */
	6, 5, 5, 4, 5, 4, 4, 3,
	5, 4, 4, 3, 4, 3, 3, 2,
	/* A0..AF */
	6, 5, 5, 4, 5, 4, 4, 3,
	5, 4, 4, 3, 4, 3, 3, 2,
	/* B0..BF */
	5, 4, 4, 3, 4, 3, 3, 2,
	4, 3, 3, 2, 3, 2, 2, 1,
	/* C0..CF */
	6, 5, 5, 4, 5, 4, 4, 3,
	5, 4, 4, 3, 4, 3, 3, 2,
	/* D0..DF */
	5, 4, 4, 3, 4, 3, 3, 2,
	4, 3, 3, 2, 3, 2, 2, 1,
	/* E0..EF */
	5, 4, 4, 3, 4, 3, 3, 2,
	4, 3, 3, 2, 3, 2, 2, 1,
	/* F0..FF */
	4, 3, 3, 2, 3, 2, 2, 1,
	3, 2, 2, 1, 2, 1, 1, 0
};

/*--- #define ANTFS_BMP_DBG 1 ---*/
#ifdef ANTFS_BMP_DBG
static void print_bmp_debug(char bmp, loff_t *dbg_cluster_bit,
		loff_t *block_start, bool *in_one_block)
{
	int i;

	for (i = 0; i < 8 && bmp; ++i, bmp >>= 1) {
		loff_t next_block_start = *dbg_cluster_bit + i;

		if ((bmp & 1) && (!*in_one_block || !next_block_start)) {
			pr_info("0x%llx..0x%llx: 0 (0x%llx clusters)\n",
					*block_start, next_block_start,
					(next_block_start - *block_start));
			*in_one_block = true;
			*block_start = next_block_start;
		} else if (!(bmp & 1) && (*in_one_block || !next_block_start)) {
			pr_info("0x%llx..0x%llx: 1 (0x%llx clusters)\n",
					*block_start, next_block_start,
					(next_block_start - *block_start));
			*in_one_block = false;
			*block_start = next_block_start;
		}
	}

	*dbg_cluster_bit += 8;
}
#endif

s64 ntfs_attr_get_free_bits(struct ntfs_attr *na)
{
	s64 nr_free = 0;
	struct runlist_element *rl, *rl_start;
	int dbits;
	size_t size_left;
	struct ntfs_volume *vol;
	s64 last_nr_free;
	size_t bufcnt_orig;
#ifdef ANTFS_BMP_DBG
	bool in_one_block;
	loff_t block_start = 0;
	loff_t dbg_cluster_bit;
#endif

	antfs_log_enter();

	if (IS_ERR_OR_NULL(na) || IS_ERR_OR_NULL(na->ni) ||
	    IS_ERR_OR_NULL(na->ni->vol)) {
		antfs_log_error("invalid na");
		nr_free = -EINVAL;
		goto out;
	}

	if ((na->data_flags & ATTR_COMPRESSION_MASK) || !NAttrNonResident(na)) {
		antfs_log_error
		    ("Cannot count bits in compressed or resident attributes.");
		nr_free = -EINVAL;
		goto out;
	}

	/* Get runlist. Make sure it is mapped. */
	if (ntfs_attr_map_whole_runlist(na)) {
		antfs_log_debug("ntfs_attr_map_runlist failed");
		nr_free = -EIO;
		goto out;
	}

	vol = na->ni->vol;
	dbits = (vol->cluster_size_bits - vol->dev->d_sb->s_blocksize_bits);
	rl_start = rl = na->rl;
	size_left = na->data_size;

	/* Walk runlist, load all clusters and count bits. */
	for (; rl->length; rl++) {
		sector_t sect;
		size_t rcnt;

		antfs_log_debug("run: %d; VCN: %lld; LCN: %lld; length: %lld",
				(int)(rl - rl_start), (long long)rl->vcn,
				(long long)rl->lcn, (long long)rl->length);
		if (rl->lcn < 0) {
			/* XXX: Is this valid at all? */
			if (rl->lcn == (LCN) LCN_HOLE) {
				antfs_log_warning("Hole in bitmap?");
				if (size_left >
				    rl->length << vol->cluster_size_bits) {
					size_left -=
					    rl->length << vol->
					    cluster_size_bits;
					nr_free +=
					    rl->length << (vol->
							   cluster_size_bits +
							   3);
				} else {
					nr_free += size_left << 3;
					goto out;
				}
				continue;
			}
			antfs_log_error("Invalid lcn @run %d: %d",
					(int)(rl - rl_start), (int)rl->lcn);
			nr_free = -EIO;
			goto out;
		}
		sect = rl->lcn << dbits;
#ifdef ANTFS_BMP_DBG
		dbg_cluster_bit = rl->vcn << (vol->cluster_size_bits + 3);
#endif
		for (rcnt = rl->length << dbits; rcnt; sect++, rcnt--) {
			size_t bufcnt;
			u8 *buf;
			struct buffer_head *bh = sb_bread(vol->dev->d_sb, sect);

			if (IS_ERR_OR_NULL(bh)) {
				antfs_log_error
				    ("sb_bread failed for sect %lld; run %d, "
				     "lcn %lld", (long long)sect,
				     (int)(rl - rl_start),
				     (long long)rl->lcn);
				nr_free = -EIO;
				goto out;
			}
			antfs_log_debug("Read sect: %lld; len: %lld",
					(long long)sect, (long long)bh->b_size);
			bufcnt = bh->b_size;
			if (bufcnt > size_left)
				bufcnt = size_left;

			last_nr_free = nr_free;
			bufcnt_orig = bufcnt;
			size_left -= bufcnt;

			for (buf = bh->b_data; bufcnt; bufcnt--, buf++) {
#ifdef ANTFS_BMP_DBG
				print_bmp_debug(*buf, &dbg_cluster_bit,
						&block_start, &in_one_block);
#endif
				nr_free += bitfree_lut256[*buf];
			}

			brelse(bh);
			antfs_log_debug("bytes read: %lld (left: %lld); free "
					"bits in buffer: %lld (total: %lld)",
					(long long)bufcnt_orig,
					(long long)size_left,
					(long long)(nr_free - last_nr_free),
					nr_free);
			if (!size_left)
				goto out;
		}
	}

out:
#ifdef ANTFS_BMP_DBG
	pr_info("0x%llx..0x%llx: %d (0x%llx clusters)\n\n",
			block_start, dbg_cluster_bit,
			in_one_block?1:0,
			(dbg_cluster_bit - block_start));
#endif
	antfs_log_leave("nr_free: %lld", (long long)nr_free);
	return nr_free;
}
