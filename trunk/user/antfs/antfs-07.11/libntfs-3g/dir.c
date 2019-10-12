/**
 * dir.c - Directory handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2002-2005 Anton Altaparmakov
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2004-2008 Szabolcs Szakacsits
 * Copyright (c) 2005-2007 Yura Pakhuchiy
 * Copyright (c) 2008-2014 Jean-Pierre Andre
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

#include <linux/slab.h>
#include <linux/stat.h>

#include "antfs.h"
#include "param.h"
#include "types.h"
#include "debug.h"
#include "attrib.h"
#include "inode.h"
#include "dir.h"
#include "volume.h"
#include "mft.h"
#include "index.h"
#include "ntfstime.h"
#include "lcnalloc.h"
#include "misc.h"
#include "reparse.h"
#include "object_id.h"

/*
 * The little endian Unicode strings "$I30", "$SII", "$SDH", "$O"
 *  and "$Q" as global constants.
 */
ntfschar NTFS_INDEX_I30[5] = { const_cpu_to_le16('$'), const_cpu_to_le16('I'),
	const_cpu_to_le16('3'), const_cpu_to_le16('0'),
	const_cpu_to_le16('\0')
};

ntfschar NTFS_INDEX_SII[5] = { const_cpu_to_le16('$'), const_cpu_to_le16('S'),
	const_cpu_to_le16('I'), const_cpu_to_le16('I'),
	const_cpu_to_le16('\0')
};

ntfschar NTFS_INDEX_SDH[5] = { const_cpu_to_le16('$'), const_cpu_to_le16('S'),
	const_cpu_to_le16('D'), const_cpu_to_le16('H'),
	const_cpu_to_le16('\0')
};

ntfschar NTFS_INDEX_O[3] = { const_cpu_to_le16('$'), const_cpu_to_le16('O'),
	const_cpu_to_le16('\0')
};

ntfschar NTFS_INDEX_Q[3] = { const_cpu_to_le16('$'), const_cpu_to_le16('Q'),
	const_cpu_to_le16('\0')
};

ntfschar NTFS_INDEX_R[3] = { const_cpu_to_le16('$'), const_cpu_to_le16('R'),
	const_cpu_to_le16('\0')
};

/**
 * ntfs_inode_lookup_by_name - find an inode in a directory given its name
 *
 * @param dir_ni     ntfs inode of the directory in which to search for the name
 * @param uname      Unicode name for which to search in the directory
 * @param uname_len  length of the name @uname in Unicode characters
 * @param fne        Pointer to copy FILE_NAME_ATTR found for given name outside
 *                   this function.
 *
 * Look for an inode with name @uname in the directory with inode @dir_ni.
 * ntfs_inode_lookup_by_name() walks the contents of the directory looking for
 * the Unicode name. If the name is found in the directory, the corresponding
 * inode number (>= 0) is returned as a mft reference in cpu format, i.e. it
 * is a 64-bit number containing the sequence number.
 *
 * On error, return negative error code. If the inode is is not
 * found this is -ENOENT.
 *
 * Note, @uname_len does not include the (optional) terminating NULL character.
 *
 * Note, we look for a case sensitive match first but we also look for a case
 * insensitive match at the same time. If we find a case insensitive match, we
 * save that for the case that we don't find an exact match, where we return
 * the mft reference of the case insensitive match.
 *
 * If the volume is mounted with the case sensitive flag set, then we only
 * allow exact matches.
 */
int ntfs_inode_lookup_by_name(struct ntfs_inode *dir_ni,
			      const ntfschar *uname, const int uname_len,
			      u64 *mref, struct FILE_NAME_ATTR **fne)
{
	VCN vcn;
	s64 br;
	struct ntfs_volume *vol = dir_ni->vol;
	struct ntfs_attr_search_ctx *ctx;
	struct INDEX_ROOT *ir;
	struct INDEX_ENTRY *ie;
	struct INDEX_BLOCK *ia;
	enum IGNORE_CASE_BOOL case_sensitivity;
	u8 *index_end;
	struct ntfs_attr *ia_na;
	int err, rc;
	u32 index_block_size;
	u8 index_block_size_bits, index_vcn_size_bits;

	antfs_log_enter();

	*mref = (s64) -1;
	if (IS_ERR_OR_NULL(dir_ni) || !dir_ni->mrec || !uname ||
	    uname_len <= 0)
		return -EINVAL;

	if (fne)
		*fne = NULL;
	ctx = ntfs_attr_get_search_ctx(dir_ni, NULL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	/* Find the index root attribute in the mft record. */
	err = ntfs_attr_lookup(AT_INDEX_ROOT, NTFS_INDEX_I30, 4, CASE_SENSITIVE,
			       0, NULL, 0, ctx);
	if (err) {
		antfs_log_error
		    ("Index root attribute missing in directory inode " "%lld",
		     (unsigned long long)dir_ni->mft_no);
		goto eo_put_err_out;
	}
	case_sensitivity =
	    (NVolCaseSensitive(vol) ? CASE_SENSITIVE: IGNORE_CASE);
	/* Get to the index root value. */
	ir = (struct INDEX_ROOT *) ((u8 *) ctx->attr +
			     le16_to_cpu(ctx->attr->value_offset));
	index_block_size = le32_to_cpu(ir->index_block_size);
	/* This ensures index_block_size is a power of 2. */
	if (index_block_size < NTFS_BLOCK_SIZE ||
	    index_block_size & (index_block_size - 1)) {
		antfs_log_error("Index block size %u is invalid.",
				(unsigned)index_block_size);
		goto put_err_out;
	}
	index_block_size_bits = __ffs(index_block_size);
	index_end = (u8 *) &ir->index + le32_to_cpu(ir->index.index_length);
	/* The first index entry. */
	ie = (struct INDEX_ENTRY *) ((u8 *) &ir->index +
			      le32_to_cpu(ir->index.entries_offset));
	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (;; ie = (struct INDEX_ENTRY *) ((u8 *) ie +
		le16_to_cpu(ie->length))) {
		/* Bounds checks. */
		if ((u8 *) ie < (u8 *) ctx->mrec || (u8 *) ie +
		    sizeof(struct INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *) ie + le16_to_cpu(ie->key_length) > index_end) {
			antfs_log_error
			    ("Index entry out of bounds in inode %lld",
			     (unsigned long long)dir_ni->mft_no);
			goto put_err_out;
		}
		/*
		 * The last entry cannot contain a name. It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ie->ie_flags & INDEX_ENTRY_END)
			break;

		if (!le16_to_cpu(ie->length)) {
			antfs_log_error("Zero length index entry in inode %lld",
					(unsigned long long)dir_ni->mft_no);
			goto put_err_out;
		}
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_names_full_collate(uname, uname_len,
					     (ntfschar *)&ie->key.file_name.
					     file_name,
					     ie->key.file_name.file_name_length,
					     case_sensitivity, vol->upcase,
					     vol->upcase_len);
		/*
		 * If uname collates before the name of the current entry, there
		 * is definitely no such name in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
		/* The names are not equal, continue the search. */
		if (rc)
			continue;
		/*
		 * Perfect match, this will never happen as the
		 * ntfs_are_names_equal() call will have gotten a match but we
		 * still treat it correctly.
		 */
		*mref = le64_to_cpu(ie->indexed_file);
		if (fne) {
			struct FILE_NAME_ATTR *fn = &ie->key.file_name;
			int cplen = (fn->file_name_length * 2) + sizeof(*fn);

			*fne = ntfs_malloc(cplen);
			if (*fne)
				memcpy(*fne, fn, cplen);
		}
		ntfs_attr_put_search_ctx(ctx);
		return 0;
	}
	/*
	 * We have finished with this index without success. Check for the
	 * presence of a child node and if not present return error code
	 * ENOENT, unless we have got the mft reference of a matching name
	 * cached in mref in which case return mref.
	 */
	if (!(ie->ie_flags & INDEX_ENTRY_NODE)) {
		ntfs_attr_put_search_ctx(ctx);
		antfs_log_debug("Entry not found - between root entries.");
		return -ENOENT;
	}

	/* Child node present, descend into it. */
	/* Open the index allocation attribute. */
	ia_na = ntfs_attr_open(dir_ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
	if (IS_ERR(ia_na)) {
		antfs_log_error("Failed to open index allocation (inode %lld)",
				(unsigned long long)dir_ni->mft_no);
		err = PTR_ERR(ia_na);
		goto eo_put_err_out;
	}

	/* Allocate a buffer for the current index block. */
	ia = ntfs_malloc(index_block_size);
	if (!ia) {
		ntfs_attr_close(ia_na);
		err = -ENOMEM;
		goto eo_put_err_out;
	}

	/* Determine the size of a vcn in the directory index. */
	if (vol->cluster_size <= index_block_size)
		index_vcn_size_bits = vol->cluster_size_bits;
	else
		index_vcn_size_bits = NTFS_BLOCK_SIZE_BITS;

	/* Get the starting vcn of the index_block holding the child node. */
	vcn =
	    sle64_to_cpup((sle64 *) ((u8 *) ie + le16_to_cpu(ie->length) - 8));

descend_into_child_node:

	/* Read the index block starting at vcn. */
	br = ntfs_attr_mst_pread(ia_na, vcn << index_vcn_size_bits, 1,
				 index_block_size_bits, ia, true);
	if (br != 1) {
		if (br >= 0)
			err = -EIO;
		else
			err = (int)br;
		antfs_log_error("Failed to read vcn 0x%llx",
				(unsigned long long)vcn);
		goto close_err_out;
	}

	if (sle64_to_cpu(ia->index_block_vcn) != vcn) {
		antfs_log_error
		    ("Actual VCN (0x%llx) of index buffer is different "
		     "from expected VCN (0x%llx).",
		     (long long)sle64_to_cpu(ia->index_block_vcn),
		     (long long)vcn);
		err = -EIO;
		goto close_err_out;
	}
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 != index_block_size) {
		antfs_log_error
		    ("Index buffer (VCN 0x%llx) of directory inode 0x%llx "
		     "has a size (%u) differing from the directory "
		     "specified size (%u).", (long long)vcn,
		     (unsigned long long)dir_ni->mft_no,
		     (unsigned)le32_to_cpu(ia->index.allocated_size) + 0x18,
		     (unsigned)index_block_size);
		err = -EIO;
		goto close_err_out;
	}
	index_end = (u8 *) &ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8 *) ia + index_block_size) {
		antfs_log_error
		    ("Size of index buffer (VCN 0x%llx) of directory inode "
		     "0x%llx exceeds maximum size.", (long long)vcn,
		     (unsigned long long)dir_ni->mft_no);
		err = -EIO;
		goto close_err_out;
	}

	/* The first index entry. */
	ie = (struct INDEX_ENTRY *) ((u8 *) &ia->index +
			      le32_to_cpu(ia->index.entries_offset));
	/*
	 * Iterate similar to above big loop but applied to index buffer, thus
	 * loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (;; ie = (struct INDEX_ENTRY *) ((u8 *) ie +
		le16_to_cpu(ie->length))) {
		/* Bounds check. */
		if ((u8 *) ie < (u8 *) ia || (u8 *) ie +
		    sizeof(struct INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *) ie + le16_to_cpu(ie->key_length) > index_end) {
			antfs_log_error
			    ("Index entry out of bounds in directory "
			     "inode %lld.",
			     (unsigned long long)dir_ni->mft_no);
			err = -EIO;
			goto close_err_out;
		}
		/*
		 * The last entry cannot contain a name. It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ie->ie_flags & INDEX_ENTRY_END)
			break;

		if (!le16_to_cpu(ie->length)) {
			err = -EIO;
			antfs_log_error("Zero length index entry in inode %lld",
					(unsigned long long)dir_ni->mft_no);
			goto close_err_out;
		}
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_names_full_collate(uname, uname_len,
					     (ntfschar *) &ie->key.file_name.
					     file_name,
					     ie->key.file_name.file_name_length,
					     case_sensitivity, vol->upcase,
					     vol->upcase_len);
		/*
		 * If uname collates before the name of the current entry, there
		 * is definitely no such name in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
		/* The names are not equal, continue the search. */
		if (rc)
			continue;
		*mref = le64_to_cpu(ie->indexed_file);
		if (fne) {
			struct FILE_NAME_ATTR *fn = &ie->key.file_name;
			int cplen = (fn->file_name_length * 2) + sizeof(*fn);

			*fne = ntfs_malloc(cplen);
			if (*fne)
				memcpy(*fne, fn, cplen);
		}
		ntfs_free(ia);
		ntfs_attr_close(ia_na);
		ntfs_attr_put_search_ctx(ctx);
		return 0;
	}
	/*
	 * We have finished with this index buffer without success. Check for
	 * the presence of a child node.
	 */
	if (ie->ie_flags & INDEX_ENTRY_NODE) {
		if ((ia->index.ih_flags & NODE_MASK) == LEAF_NODE) {
			antfs_log_error
			    ("Index entry with\n child node found in a leaf "
			     "node in directory inode %lld.",
			     (unsigned long long)dir_ni->mft_no);
			err = -EIO;
			goto close_err_out;
		}
		/* Child node present, descend into it. */
		vcn =
		    sle64_to_cpup((sle64 *) ((u8 *) ie +
					     le16_to_cpu(ie->length) - 8));
		if (vcn >= 0)
			goto descend_into_child_node;
		antfs_log_error("Negative child node vcn in directory inode "
				"0x%llx.", (unsigned long long)dir_ni->mft_no);
		err = -EIO;
		goto close_err_out;
	}
	ntfs_free(ia);
	ntfs_attr_close(ia_na);
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_debug("Entry not found.");
	return -ENOENT;
put_err_out:
	err = -EIO;
	/* log? do we wanna know if a user has a corrupt dir? */
	antfs_log_debug("Corrupt directory. Aborting lookup.");
eo_put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_error("err: %d", err);
	return err;
close_err_out:
	ntfs_free(ia);
	ntfs_attr_close(ia_na);
	goto eo_put_err_out;
}

/*
 *		Lookup a file in a directory from its UTF-8 name
 *
 *	The name is first fetched from cache if one is defined
 *	Result is set in @inum. This may be set to -1 if inode not found.
 *
 *	@return 0 on success or negative error code if not possible
 */
int ntfs_inode_lookup_by_mbsname(struct ntfs_inode *dir_ni, const char *name,
				 u64 *inum)
{
	int uname_len;
	ntfschar *uname = NULL;
	char *cached_name;
	const char *const_name;
	int err = 0;

	if (!NVolCaseSensitive(dir_ni->vol)) {
		cached_name = ntfs_uppercase_mbs(name,
						 dir_ni->vol->upcase,
						 dir_ni->vol->upcase_len);
		if (IS_ERR(cached_name))
			const_name = NULL;
		else
			const_name = cached_name;
	} else {
		cached_name = (char *)NULL;
		const_name = name;
	}
	if (const_name) {
		{
			/* Generate unicode name. */
			uname_len = ntfs_mbstoucs(cached_name, &uname);
			if (uname_len >= 0)
				err = ntfs_inode_lookup_by_name(dir_ni,
								uname,
								uname_len,
								inum, NULL);
			else
				err = -ENOENT;
		}
		if (!IS_ERR(cached_name))
			ntfs_free(cached_name);
	} else
		err = -ENOENT;
	if (err)
		*inum = (u64) -1;

	return err;
}

/**
 * ntfs_pathname_to_inode - Find the inode which represents the given pathname
 *
 * @param vol       An ntfs volume obtained from ntfs_mount
 * @param parent    A directory inode to begin the search (may be NULL)
 * @param pathname  Pathname to be located
 *
 * Take an ASCII pathname and find the inode that represents it.  The function
 * splits the path and then descends the directory tree.  If @parent is NULL,
 * then the root directory '.' will be used as the base for the search.
 *
 * @retval inode  Success, the pathname was valid
 * @return error ptr on failure
 */
struct ntfs_inode *ntfs_pathname_to_inode(struct ntfs_volume *vol,
					  struct ntfs_inode *parent,
					  const char *pathname)
{
	u64 inum;
	int len, err = 0;
	char *p, *q;
	struct ntfs_inode *ni;
	struct ntfs_inode *result = NULL;
	ntfschar *unicode = NULL;
	char *ascii = NULL;

	if (IS_ERR_OR_NULL(vol) || !pathname)
		return ERR_PTR(-EINVAL);

	antfs_log_enter("path: '%s'", pathname);

	ascii = kstrdup(pathname, GFP_KERNEL);
	if (!ascii) {
		antfs_log_error("Out of memory.");
		err = -ENOMEM;
		goto out;
	}

	p = ascii;
	/* Remove leading /'s. */
	while (p && *p && *p == PATH_SEP)
		p++;
	if (parent) {
		ni = parent;
	} else {
		ni = ntfs_inode_open(vol, FILE_ROOT, NULL);
		if (IS_ERR(ni)) {
			antfs_log_debug("Couldn't open the inode of the root "
					"directory.");
			err = PTR_ERR(ni);
			goto out;
		}
	}

	while (p && *p) {
		/* Find the end of the first token. */
		q = strchr(p, PATH_SEP);
		if (q != NULL)
			*q = '\0';
		len = ntfs_mbstoucs(p, &unicode);
		if (len < 0) {
			antfs_log_error("Could not convert filename to Unicode:"
					" '%s'", p);
			err = len;
			goto close;
		} else if (len > NTFS_MAX_NAME_LEN) {
			err = -ENAMETOOLONG;
			goto close;
		}
		err = ntfs_inode_lookup_by_name(ni, unicode, len, &inum, NULL);
		if (err) {
			antfs_log_debug("Couldn't find name '%s' in pathname "
					"'%s'.", p, pathname);
			goto close;
		}

		if (ni != parent)
			ntfs_inode_close(ni);

		inum = MREF(inum);
		ni = ntfs_inode_open(vol, inum, NULL);
		if (IS_ERR(ni)) {
			antfs_log_debug("Cannot open inode %llu: %s.",
					(unsigned long long)inum, p);
			err = PTR_ERR(ni);
			goto out;
		}

		ntfs_free(unicode);
		unicode = NULL;

		if (q)
			*q++ = PATH_SEP;	/* JPA */
		p = q;
		while (p && *p && *p == PATH_SEP)
			p++;
	}

	result = ni;
	goto out;
close:
	if (ni && (ni != parent)) {
		ntfs_inode_close(ni);
	}
out:
	ntfs_free(ascii);
	ntfs_free(unicode);
	antfs_log_leave("err: %d", err);
	return result ? result : ERR_PTR(err);
}

/*
 * The little endian Unicode string ".." for ntfs_readdir().
 */
static const ntfschar dotdot[3] = { const_cpu_to_le16('.'),
	const_cpu_to_le16('.'),
	const_cpu_to_le16('\0')
};

/*
 * union index_union -
 * More helpers for ntfs_readdir().
 */
union index_union {
	struct INDEX_ROOT *ir;
	struct INDEX_BLOCK *ia;
} __attribute__ ((__transparent_union__));

/**
 * enum INDEX_TYPE -
 * More helpers for ntfs_readdir().
 */
enum INDEX_TYPE {
	INDEX_TYPE_ROOT,	/* index root */
	INDEX_TYPE_ALLOCATION,	/* index allocation */
};

/*
 *		Decode Interix file types
 *
 *	Non-Interix types are returned as plain files, because a
 *	Windows user may force patterns very similar to Interix,
 *	and most metadata files have such similar patters.
 */
static u32 ntfs_interix_types(struct ntfs_inode *ni)
{
	u32 dt_type;
	le64 magic;

	dt_type = NTFS_DT_UNKNOWN;

	/*
	 * Unrecognized patterns (eg HID + SYST for metadata)
	 * are plain files or directories
	 */
	if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
		dt_type = NTFS_DT_DIR;
	else
		dt_type = NTFS_DT_REG;
	if (ANTFS_NA(ni)->data_size <= 1) {
		if (!(ni->flags & FILE_ATTR_HIDDEN))
			dt_type = (ANTFS_NA(ni)->data_size ? NTFS_DT_SOCK :
					NTFS_DT_FIFO);
	} else {
		if ((ANTFS_NA(ni)->data_size >= (s64) sizeof(magic))
		    && (ntfs_attr_pread(ANTFS_NA(ni), 0, sizeof(magic), &magic)
			== sizeof(magic))) {
			if (magic == INTX_SYMBOLIC_LINK)
				dt_type = NTFS_DT_LNK;
			else if (magic == INTX_BLOCK_DEVICE)
				dt_type = NTFS_DT_BLK;
			else if (magic == INTX_CHARACTER_DEVICE)
				dt_type = NTFS_DT_CHR;
		}
	}

	return dt_type;
}

/*
 *		Decode file types
 *
 *	Better only use for Interix types and junctions,
 *	unneeded complexity when used for plain files or directories
 *
 *	Error cases are logged and returned as unknown.
 */
static u32 ntfs_dir_entry_type(struct ntfs_inode *dir_ni, MFT_REF mref,
			       enum FILE_ATTR_FLAGS attributes)
{
	struct ntfs_inode *ni;
	u32 dt_type;

	dt_type = NTFS_DT_UNKNOWN;
	ni = ntfs_inode_open(dir_ni->vol, mref, NULL);
	if (!IS_ERR(ni)) {
		if ((attributes & FILE_ATTR_REPARSE_POINT)
		    && ntfs_possible_symlink(ni))
			dt_type = NTFS_DT_LNK;
		else if ((attributes & FILE_ATTR_SYSTEM)
			 && !(attributes & FILE_ATTR_I30_INDEX_PRESENT))
			dt_type = ntfs_interix_types(ni);
		else
			dt_type = (attributes
				   & FILE_ATTR_I30_INDEX_PRESENT
				   ? NTFS_DT_DIR : NTFS_DT_REG);
		ntfs_inode_close(ni);
	}
	if (dt_type == NTFS_DT_UNKNOWN)
		antfs_log_error("Could not decode the type of inode %lld",
				(long long)MREF(mref));
	return dt_type;
}

/**
 * ntfs_filldir - ntfs specific filldir method
 *
 * @param dir_ni      ntfs inode of current directory
 * @param pos         current position in directory
 * @param ivcn_bits   log(2) of index vcn size
 * @param index_type  specifies whether @iu is an index root or an index allocation
 * @param iu          index root or index block to which @ie belongs
 * @param ie          current index entry
 * @param dirent      context for filldir callback supplied by the caller
 * @param filldir     filldir callback supplied by the caller
 *
 * Pass information specifying the current directory entry @ie to the @filldir
 * callback.
 *
 * @retval 0 on success
 * @return negative error code on failure
 */
static int ntfs_filldir(struct ntfs_inode *dir_ni, s64 *pos, u8 ivcn_bits,
			const enum INDEX_TYPE index_type, union index_union iu,
			struct INDEX_ENTRY *ie, void *dirent,
			ntfs_filldir_t filldir)
{
	struct FILE_NAME_ATTR *fn = &ie->key.file_name;
	unsigned dt_type;
	bool metadata;
	ntfschar *loname;
	int err;
	MFT_REF mref;

	antfs_log_enter();

	/* Advance the position even if going to skip the entry. */
	if (index_type == INDEX_TYPE_ALLOCATION)
		*pos =
		    (u8 *) ie - (u8 *) iu.ia +
		    (sle64_to_cpu(iu.ia->index_block_vcn) << ivcn_bits) +
		    dir_ni->vol->mft_record_size;
	else			/* if (index_type == INDEX_TYPE_ROOT) */
		*pos = (u8 *) ie - (u8 *) iu.ir;
	mref = le64_to_cpu(ie->indexed_file);
	metadata = (MREF(mref) != FILE_ROOT) && (MREF(mref) < FILE_FIRST_USER);
	/* Skip root directory self reference entry. */
	if (MREF_LE(ie->indexed_file) == FILE_ROOT) {
		antfs_log_leave("done (FILE_root)");
		return 0;
	}
	if ((ie->key.file_name.file_attributes
	     & (FILE_ATTR_REPARSE_POINT | FILE_ATTR_SYSTEM))
	    && !metadata)
		dt_type = ntfs_dir_entry_type(dir_ni, mref,
					      ie->key.file_name.
					      file_attributes);
	else if (ie->key.file_name.
		 file_attributes & FILE_ATTR_I30_INDEX_PRESENT)
		dt_type = NTFS_DT_DIR;
	else
		dt_type = NTFS_DT_REG;

	/* return metadata files and hidden files if requested */
	if ((!metadata && (NVolShowHidFiles(dir_ni->vol)
			   || !(fn->file_attributes & FILE_ATTR_HIDDEN)))
	    || (NVolShowSysFiles(dir_ni->vol) && (NVolShowHidFiles(dir_ni->vol)
						  || metadata))) {
		if (NVolCaseSensitive(dir_ni->vol)) {
			err = filldir(dirent, fn->file_name,
				      fn->file_name_length,
				      fn->file_name_type, *pos, mref, dt_type);
			if (err && err != -ENOMEM) {
				antfs_log_warning("filldir (%pS) failed (%d)",
						  filldir, err);
			}
		} else {
			loname =
			    (ntfschar *) ntfs_malloc(2 * fn->file_name_length);
			if (!loname) {
				antfs_log_error("ntfs_malloc failed");
				return -ENOMEM;
			}
			memcpy(loname, fn->file_name, 2 * fn->file_name_length);
			ntfs_name_locase(loname, fn->file_name_length,
					 dir_ni->vol->locase,
					 dir_ni->vol->upcase_len);
			err = filldir(dirent, loname,
				      fn->file_name_length,
				      fn->file_name_type, *pos, mref, dt_type);
			ntfs_free(loname);
		}
	} else
		err = 0;

	antfs_log_leave("exit (%d)", err);
	return err;
}

/**
 * ntfs_mft_get_parent_ref - find mft reference of parent directory of an inode
 *
 * @param ni  ntfs inode whose parent directory to find
 *
 * Find the parent directory of the ntfs inode @ni. To do this, find the first
 * file name attribute in the mft record of @ni and return the parent mft
 * reference from that.
 *
 * Note this only makes sense for directories, since files can be hard linked
 * from multiple directories and there is no way for us to tell which one is
 * being looked for.
 *
 * Technically directories can have hard links, too, but we consider that as
 * illegal as Linux/UNIX do not support directory hard links.
 *
 * Return the mft reference of the parent directory on success or negative
 * error code.
 */
static MFT_REF ntfs_mft_get_parent_ref(struct ntfs_inode *ni)
{
	MFT_REF mref;
	struct ntfs_attr_search_ctx *ctx;
	struct FILE_NAME_ATTR *fn;
	int err;

	antfs_log_enter();

	if (IS_ERR_OR_NULL(ni))
		return ERR_MREF(-EINVAL);

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(ctx))
		return ERR_MREF(PTR_ERR(ctx));
	err = ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0, 0, 0, NULL, 0, ctx);
	if (err) {
		antfs_log_error("No file name found in inode %lld",
				(unsigned long long)ni->mft_no);
		goto err_out;
	}
	if (ctx->attr->non_resident) {
		antfs_log_error("File name attribute must be resident (inode "
				"%lld)", (unsigned long long)ni->mft_no);
		goto io_err_out;
	}
	fn = (struct FILE_NAME_ATTR *) ((u8 *) ctx->attr +
				 le16_to_cpu(ctx->attr->value_offset));
	if ((u8 *) fn + le32_to_cpu(ctx->attr->value_length) >
	    (u8 *) ctx->attr + le32_to_cpu(ctx->attr->length)) {
		/* log? do we want to know that corruption was here? */
		antfs_log_error("Corrupt file name attribute in inode %lld.",
				(unsigned long long)ni->mft_no);
		goto io_err_out;
	}
	mref = le64_to_cpu(fn->parent_directory);
	ntfs_attr_put_search_ctx(ctx);
	return mref;
io_err_out:
	err = -EIO;
err_out:
	ntfs_attr_put_search_ctx(ctx);
	return ERR_MREF(err);
}

/**
 * ntfs_readdir - read the contents of an ntfs directory
 *
 * @param dir_ni   ntfs inode of current directory
 * @param pos      current position in directory
 * @param dirent   context for filldir callback supplied by the caller
 * @param filldir  filldir callback supplied by the caller
 *
 * Parse the index root and the index blocks that are marked in use in the
 * index bitmap and hand each found directory entry to the @filldir callback
 * supplied by the caller.
 *
 * @return 0 on success negative error code
 *
 * Note: Index blocks are parsed in ascending vcn order, from which follows
 * that the directory entries are not returned sorted.
 */
int ntfs_readdir(struct ntfs_inode *dir_ni, s64 *pos,
		 void *dirent, ntfs_filldir_t filldir)
{
	s64 i_size, br, ia_pos, bmp_pos, ia_start;
	struct ntfs_volume *vol;
	struct ntfs_attr *ia_na, *bmp_na = NULL;
	struct ntfs_attr_search_ctx *ctx = NULL;
	u8 *index_end, *bmp = NULL;
	struct INDEX_ROOT *ir;
	struct INDEX_ENTRY *ie;
	struct INDEX_BLOCK *ia = NULL;
	int ir_pos, bmp_buf_size, bmp_buf_pos, err;
	u32 index_block_size;
	u8 index_block_size_bits, index_vcn_size_bits;

	if (IS_ERR_OR_NULL(dir_ni) || !pos || !filldir) {
		antfs_log_error("EINVAL");
		return -EINVAL;
	}

	if (!(dir_ni->mrec->flags & MFT_RECORD_IS_DIRECTORY))
		return -ENOTDIR;

	vol = dir_ni->vol;

	antfs_log_enter("Entering for inode %lld, *pos 0x%llx.",
			(unsigned long long)dir_ni->mft_no, (long long)*pos);

	/* Open the index allocation attribute. */
	ia_na = ntfs_attr_open(dir_ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
	if (IS_ERR(ia_na)) {
		err = PTR_ERR(ia_na);
		if (err != -ENOENT) {
			/* log? previously corrupted? */
			antfs_log_error
			    ("Failed to open index allocation attribute. "
			     "Directory inode %lld is corrupt or bug",
			     (unsigned long long)dir_ni->mft_no);
			return err;
		}
		i_size = 0;
	} else {
		i_size = ia_na->data_size;
	}

	if (NInoAttrTestAndClearIndexModified(dir_ni))
		*pos = 0;

	/* Are we at end of dir yet? */
	if (*pos >= i_size + vol->mft_record_size)
		goto done;

	/* Emulate . and .. for all directories. */
	if (!*pos) {
		err = filldir(dirent, dotdot, 1, FILE_NAME_POSIX, *pos,
			      MK_MREF(dir_ni->mft_no,
				      le16_to_cpu(dir_ni->mrec->
						  sequence_number)),
			      NTFS_DT_DIR);
		if (err)
			goto err_out;
		++*pos;
	}
	if (*pos == 1) {
		MFT_REF parent_mref;

		parent_mref = ntfs_mft_get_parent_ref(dir_ni);
		if (IS_ERR_MREF(parent_mref)) {
			antfs_log_error("Parent directory not found");
			goto dir_err_out;
		}

		err = filldir(dirent, dotdot, 2, FILE_NAME_POSIX, *pos,
			      parent_mref, NTFS_DT_DIR);
		if (err)
			goto err_out;
		++*pos;
	}

	ctx = ntfs_attr_get_search_ctx(dir_ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto err_out;
	}

	/* Get the offset into the index root attribute. */
	ir_pos = (int)*pos;
	/* Find the index root attribute in the mft record. */
	if (ntfs_attr_lookup
	    (AT_INDEX_ROOT, NTFS_INDEX_I30, 4, CASE_SENSITIVE, 0, NULL, 0,
	     ctx)) {
		antfs_log_error
		    ("Index root attribute missing in directory inode " "%lld",
		     (unsigned long long)dir_ni->mft_no);
		goto dir_err_out;
	}
	/* Get to the index root value. */
	ir = (struct INDEX_ROOT *) ((u8 *) ctx->attr +
			     le16_to_cpu(ctx->attr->value_offset));

	/* Determine the size of a vcn in the directory index. */
	index_block_size = le32_to_cpu(ir->index_block_size);
	if (index_block_size < NTFS_BLOCK_SIZE ||
	    index_block_size & (index_block_size - 1)) {
		antfs_log_error("Index block size %u is invalid.",
				(unsigned)index_block_size);
		goto dir_err_out;
	}
	index_block_size_bits = __ffs(index_block_size);
	if (vol->cluster_size <= index_block_size)
		index_vcn_size_bits = vol->cluster_size_bits;
	else
		index_vcn_size_bits = NTFS_BLOCK_SIZE_BITS;

	/* Are we jumping straight into the index allocation attribute? */
	if (*pos >= vol->mft_record_size) {
		ntfs_attr_put_search_ctx(ctx);
		ctx = NULL;
		goto skip_index_root;
	}

	index_end = (u8 *) &ir->index + le32_to_cpu(ir->index.index_length);
	/* The first index entry. */
	ie = (struct INDEX_ENTRY *) ((u8 *) &ir->index +
			      le32_to_cpu(ir->index.entries_offset));
	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry or until filldir tells us it has had enough
	 * or signals an error (both covered by the rc test).
	 */
	for (;; ie = (struct INDEX_ENTRY *) ((u8 *) ie +
			le16_to_cpu(ie->length))) {
		antfs_log_debug("In index root, offset %d.",
				(int)((u8 *) ie - (u8 *) ir));
		/* Bounds checks. */
		if ((u8 *) ie < (u8 *) ctx->mrec || (u8 *) ie +
		    sizeof(struct INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *) ie + le16_to_cpu(ie->key_length) > index_end)
			goto dir_err_out;
		/* The last entry cannot contain a name. */
		if (ie->ie_flags & INDEX_ENTRY_END)
			break;

		if (!le16_to_cpu(ie->length))
			goto dir_err_out;

		/* Skip index root entry if continuing previous readdir. */
		if (ir_pos > (u8 *) ie - (u8 *) ir)
			continue;
		/*
		 * Submit the directory entry to ntfs_filldir(), which will
		 * invoke the filldir() callback as appropriate.
		 */
		err = ntfs_filldir(dir_ni, pos, index_vcn_size_bits,
				   INDEX_TYPE_ROOT, ir, ie, dirent, filldir);
		if (err) {
			ntfs_attr_put_search_ctx(ctx);
			ctx = NULL;
			goto err_out;
		}
	}
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;

	/* If there is no index allocation attribute we are finished. */
	/* Goto is taken if we have no AT_INDEX_ALLOCATION attribute. */
	if (IS_ERR(ia_na))
		goto EOD;

	/* Advance *pos to the beginning of the index allocation. */
	*pos = vol->mft_record_size;

skip_index_root:

	if (IS_ERR(ia_na))
		goto done;

	/* Allocate a buffer for the current index block. */
	ia = ntfs_malloc(index_block_size);
	if (!ia) {
		err = -ENOMEM;
		goto err_out;
	}

	bmp_na = ntfs_attr_open(dir_ni, AT_BITMAP, NTFS_INDEX_I30, 4);
	if (IS_ERR(bmp_na)) {
		antfs_log_error("Failed to open index bitmap attribute");
		goto dir_err_out;
	}

	/* Get the offset into the index allocation attribute. */
	ia_pos = *pos - vol->mft_record_size;

	bmp_pos = ia_pos >> index_block_size_bits;
	if (bmp_pos >> 3 >= bmp_na->data_size) {
		antfs_log_error("Current index position exceeds index bitmap "
				"size.");
		goto dir_err_out;
	}

	bmp_buf_size = min(bmp_na->data_size - (bmp_pos >> 3), (s64) 4096);
	bmp = ntfs_malloc(bmp_buf_size);
	if (!bmp) {
		err = -ENOMEM;
		goto err_out;
	}

	br = ntfs_attr_pread(bmp_na, bmp_pos >> 3, bmp_buf_size, bmp);
	if (br != bmp_buf_size) {
		err = (br < 0) ? (int)br : -EIO;
		antfs_log_error("Failed to read from index bitmap attribute");
		goto err_out;
	}

	bmp_buf_pos = bmp_pos & 7;
	/* If the index block is not in use find the next one that is. */
	while (!(bmp[bmp_buf_pos >> 3] & (1 << (bmp_buf_pos & 7)))) {
find_next_index_buffer:
		bmp_pos++;
		bmp_buf_pos++;
		/* If we have reached the end of the bitmap, we are done. */
		if (bmp_pos >> 3 >= bmp_na->data_size)
			goto EOD;
		ia_pos = bmp_pos << index_block_size_bits;
		if (bmp_buf_pos >> 3 < bmp_buf_size)
			continue;
		/* Read next chunk from the index bitmap. */
		bmp_buf_pos = 0;
		if ((bmp_pos >> 3) + bmp_buf_size > bmp_na->data_size)
			bmp_buf_size = bmp_na->data_size - (bmp_pos >> 3);
		br = ntfs_attr_pread(bmp_na, bmp_pos >> 3, bmp_buf_size, bmp);
		if (br != bmp_buf_size) {
			err = (br < 0) ? (int)br : -EIO;
			antfs_log_error
			    ("Failed to read from index bitmap attribute");
			goto err_out;
		}
	}

	antfs_log_debug("Handling index block 0x%llx.", (long long)bmp_pos);

	/* Read the index block starting at bmp_pos. */
	br = ntfs_attr_mst_pread(ia_na, bmp_pos << index_block_size_bits, 1,
				 index_block_size_bits, ia, true);
	if (br != 1) {
		err = (br < 0) ? (int)br : -EIO;
		antfs_log_error("Failed to read index block");
		goto err_out;
	}

	ia_start = ia_pos & ~(s64) (index_block_size - 1);
	if (sle64_to_cpu(ia->index_block_vcn) != ia_start >>
	    index_vcn_size_bits) {
		antfs_log_error
		    ("Actual VCN (0x%llx) of index buffer is different "
		     "from expected VCN (0x%llx) in inode 0x%llx.",
		     (long long)sle64_to_cpu(ia->index_block_vcn),
		     (long long)ia_start >> index_vcn_size_bits,
		     (unsigned long long)dir_ni->mft_no);
		goto dir_err_out;
	}
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 != index_block_size) {
		antfs_log_error
		    ("Index buffer (VCN 0x%llx) of directory inode %lld "
		     "has a size (%u) differing from the directory "
		     "specified size (%u).",
		     (long long)ia_start >> index_vcn_size_bits,
		     (unsigned long long)dir_ni->mft_no,
		     (unsigned)le32_to_cpu(ia->index.allocated_size)
		     + 0x18, (unsigned)index_block_size);
		goto dir_err_out;
	}
	index_end = (u8 *) &ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8 *) ia + index_block_size) {
		antfs_log_error
		    ("Size of index buffer (VCN 0x%llx) of directory inode "
		     "%lld exceeds maximum size.",
		     (long long)ia_start >> index_vcn_size_bits,
		     (unsigned long long)dir_ni->mft_no);
		goto dir_err_out;
	}
	/* The first index entry. */
	ie = (struct INDEX_ENTRY *) ((u8 *) &ia->index +
			      le32_to_cpu(ia->index.entries_offset));
	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry or until ntfs_filldir tells us it has had
	 * enough or signals an error (both covered by the rc test).
	 */
	for (;; ie = (struct INDEX_ENTRY *) ((u8 *) ie +
			le16_to_cpu(ie->length))) {
		antfs_log_debug("In index allocation, offset 0x%llx.",
				(long long)ia_start + ((u8 *) ie - (u8 *) ia));
		/* Bounds checks. */
		if ((u8 *) ie < (u8 *) ia || (u8 *) ie +
		    sizeof(struct INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *) ie + le16_to_cpu(ie->key_length) > index_end) {
			antfs_log_error
			    ("Index entry out of bounds in directory inode "
			     "%lld.", (unsigned long long)dir_ni->mft_no);
			goto dir_err_out;
		}
		/* The last entry cannot contain a name. */
		if (ie->ie_flags & INDEX_ENTRY_END)
			break;

		if (!le16_to_cpu(ie->length))
			goto dir_err_out;

		/* Skip index entry if continuing previous readdir. */
		if (ia_pos - ia_start > (u8 *) ie - (u8 *) ia)
			continue;
		/*
		 * Submit the directory entry to ntfs_filldir(), which will
		 * invoke the filldir() callback as appropriate.
		 */
		err = ntfs_filldir(dir_ni, pos, index_vcn_size_bits,
				   INDEX_TYPE_ALLOCATION, ia, ie, dirent,
				   filldir);
		if (err)
			goto err_out;
	}
	goto find_next_index_buffer;
EOD:
	/* We are finished, set *pos to EOD. */
	*pos = i_size + vol->mft_record_size;
done:
	ntfs_free(ia);
	ntfs_free(bmp);
	ntfs_attr_close(bmp_na);
	ntfs_attr_close(ia_na);
	antfs_log_debug("EOD, *pos 0x%llx, returning 0.", (long long)*pos);
	return 0;
dir_err_out:
	err = -EIO;
err_out:
	antfs_log_debug("failed. (%d)", err);
	if (!IS_ERR_OR_NULL(ctx))
		ntfs_attr_put_search_ctx(ctx);
	ntfs_free(ia);
	ntfs_free(bmp);
	ntfs_attr_close(bmp_na);
	ntfs_attr_close(ia_na);
	return err;
}

/**
 * __ntfs_create - create object on ntfs volume
 *
 * @param dir_ni      ntfs inode for directory in which create new object
 * @param securid     id of inheritable security descriptor, 0 if none
 * @param name        unicode name of new object
 * @param name_len    length of the name in unicode characters
 * @param type        type of the object to create
 * @param dev         major and minor device numbers (obtained from makedev())
 * @param target      target in unicode (only for symlinks)
 * @param target_len  length of target in unicode characters
 *
 * Internal, use ntfs_create{,_device,_symlink} wrappers instead.
 *
 * @type can be:
 *	S_IFREG		to create regular file
 *	S_IFDIR		to create directory
 *	S_IFBLK		to create block device
 *	S_IFCHR		to create character device
 *	S_IFLNK		to create symbolic link
 *	S_IFIFO		to create FIFO
 *	S_IFSOCK	to create socket
 * other values are invalid.
 *
 * @dev is used only if @type is S_IFBLK or S_IFCHR, in other cases its value
 * ignored.
 *
 * @target and @target_len are used only if @type is S_IFLNK, in other cases
 * their value ignored.
 *
 * @return opened ntfs inode that describes created object on success or error
 *         pointer
 */
static struct ntfs_inode *__ntfs_create(struct ntfs_inode *dir_ni, le32 securid,
					const ntfschar *name, u8 name_len,
					mode_t type, dev_t dev,
					const ntfschar *target, int target_len)
{
	struct ntfs_inode *ni;
	struct inode *inode;
	struct FILE_NAME_ATTR *fn = NULL;
	struct STANDARD_INFORMATION *si = NULL;
	int err, fn_len, si_len;

	antfs_log_enter();

	/* Sanity checks. */
	if (IS_ERR_OR_NULL(dir_ni) || !name || !name_len) {
		antfs_log_error("Invalid arguments.");
		return ERR_PTR(-EINVAL);
	}

	if (dir_ni->flags & FILE_ATTR_REPARSE_POINT)
		return ERR_PTR(-EOPNOTSUPP);

	ni = ntfs_mft_record_alloc(dir_ni->vol, NULL);
	if (IS_ERR(ni))
		return ni;
	/*
	 * Create STANDARD_INFORMATION attribute.
	 * JPA Depending on available inherited security descriptor,
	 * Write STANDARD_INFORMATION v1.2 (no inheritance) or v3
	 */
	if (securid)
		si_len = sizeof(struct STANDARD_INFORMATION);
	else
		si_len = offsetof(struct STANDARD_INFORMATION, v1_end);
	antfs_log_debug("si_len: %d", si_len);
	si = ntfs_calloc(si_len);
	if (!si) {
		err = -ENOMEM;
		goto err_out;
	}
	si->creation_time = ni->creation_time;
	si->last_data_change_time = ni->last_data_change_time;
	si->last_mft_change_time = ni->last_mft_change_time;
	si->last_access_time = ni->last_access_time;
	if (securid) {
		set_nino_flag(ni, v3_Extensions);
		ni->owner_id = si->owner_id = const_cpu_to_le32(0);
		ni->security_id = si->security_id = securid;
		ni->quota_charged = si->quota_charged = const_cpu_to_le64(0);
		ni->usn = si->usn = const_cpu_to_le64(0);
	} else
		clear_nino_flag(ni, v3_Extensions);
	if (!S_ISREG(type) && !S_ISDIR(type)) {
		si->file_attributes = FILE_ATTR_SYSTEM;
		ni->flags = FILE_ATTR_SYSTEM;
	}
	ni->flags |= FILE_ATTR_ARCHIVE;
	if (NVolHideDotFiles(dir_ni->vol)
	    && (name_len > 1)
	    && (name[0] == const_cpu_to_le16('.'))
	    && (name[1] != const_cpu_to_le16('.')))
		ni->flags |= FILE_ATTR_HIDDEN;
	/*
	 * Set compression flag according to parent directory
	 * unless NTFS version < 3.0 or cluster size > 4K
	 * or compression has been disabled
	 */
	if ((dir_ni->flags & FILE_ATTR_COMPRESSED)
	    && (dir_ni->vol->major_ver >= 3)
	    && NVolCompression(dir_ni->vol)
	    && (dir_ni->vol->cluster_size <= MAX_COMPRESSION_CLUSTER_SIZE)
	    && (S_ISREG(type) || S_ISDIR(type)))
		ni->flags |= FILE_ATTR_COMPRESSED;
	/* Add STANDARD_INFORMATION to inode. */
	err = ntfs_attr_add(ni, AT_STANDARD_INFORMATION, AT_UNNAMED, 0,
			    (u8 *) si, si_len);
	if (err != 0) {
		antfs_log_error("Failed to add STANDARD_INFORMATION "
				"attribute.");
		goto err_out;
	}

	if (S_ISDIR(type)) {
		struct INDEX_ROOT *ir = NULL;
		struct INDEX_ENTRY *ie;
		int ir_len, index_len;

		/* Create INDEX_ROOT attribute. */
		index_len = sizeof(struct INDEX_HEADER) +
			sizeof(struct INDEX_ENTRY_HEADER);
		ir_len = offsetof(struct INDEX_ROOT, index) + index_len;
		ir = ntfs_calloc(ir_len);
		if (!ir) {
			err = -ENOMEM;
			goto err_out;
		}
		ir->type = AT_FILE_NAME;
		ir->collation_rule = COLLATION_FILE_NAME;
		ir->index_block_size = cpu_to_le32(ni->vol->indx_record_size);
		if (ni->vol->cluster_size <= ni->vol->indx_record_size)
			ir->clusters_per_index_block =
			    ni->vol->indx_record_size >>
			    ni->vol->cluster_size_bits;
		else
			ir->clusters_per_index_block =
			    ni->vol->indx_record_size >> NTFS_BLOCK_SIZE_BITS;
		ir->index.entries_offset =
		    const_cpu_to_le32(sizeof(struct INDEX_HEADER));
		ir->index.index_length = cpu_to_le32(index_len);
		ir->index.allocated_size = cpu_to_le32(index_len);
		ie = (struct INDEX_ENTRY *) ((u8 *) ir +
				sizeof(struct INDEX_ROOT));
		ie->length =
			const_cpu_to_le16(sizeof(struct INDEX_ENTRY_HEADER));
		ie->key_length = const_cpu_to_le16(0);
		ie->ie_flags = INDEX_ENTRY_END;
		/* Add INDEX_ROOT attribute to inode. */
		err = ntfs_attr_add(ni, AT_INDEX_ROOT, NTFS_INDEX_I30, 4,
					 (u8 *) ir, ir_len);
		if (err != 0) {
			ntfs_free(ir);
			antfs_log_error
			    ("Failed to add INDEX_ROOT attribute.");
			goto err_out;
		}
		ntfs_free(ir);
		/* since we are already in a path we only enter as a directory,
		 * mark this record as a directory already here and not later */
		ni->mrec->flags |= MFT_RECORD_IS_DIRECTORY;
	} else {
		struct INTX_FILE *data;
		int data_len;

		switch (type) {
		case S_IFLNK:
			data_len = sizeof(enum INTX_FILE_TYPES) +
			    target_len * sizeof(ntfschar);
			data = ntfs_malloc(data_len);
			if (!data) {
				err = -ENOMEM;
				goto err_out;
			}
			data->magic = INTX_SYMBOLIC_LINK;
			memcpy(data->target, target,
			       target_len * sizeof(ntfschar));
			break;
		default:	/* FIFO or regular file. */
			data = NULL;
			data_len = 0;
			break;
		}
		/* Add DATA attribute to inode. */
		err = ntfs_attr_add(ni, AT_DATA, AT_UNNAMED, 0, (u8 *) data,
				    data_len);
		if (err != 0) {
			antfs_log_error("Failed to add DATA attribute.");
			ntfs_free(data);
			goto err_out;
		}
		ntfs_free(data);
	}
	/* Create FILE_NAME attribute. */
	fn_len = sizeof(struct FILE_NAME_ATTR) + name_len * sizeof(ntfschar);
	fn = ntfs_calloc(fn_len);
	if (!fn) {
		err = -ENOMEM;
		goto err_out;
	}
	fn->parent_directory = MK_LE_MREF(dir_ni->mft_no,
					  le16_to_cpu(dir_ni->mrec->
						      sequence_number));
	fn->file_name_length = name_len;
	fn->file_name_type = FILE_NAME_POSIX;
	if (S_ISDIR(type))
		fn->file_attributes = FILE_ATTR_I30_INDEX_PRESENT;
	if (!S_ISREG(type) && !S_ISDIR(type))
		fn->file_attributes = FILE_ATTR_SYSTEM;
	else
		fn->file_attributes |= ni->flags & FILE_ATTR_COMPRESSED;
	fn->file_attributes |= FILE_ATTR_ARCHIVE;
	fn->file_attributes |= ni->flags & FILE_ATTR_HIDDEN;
	fn->creation_time = ni->creation_time;
	fn->last_data_change_time = ni->last_data_change_time;
	fn->last_mft_change_time = ni->last_mft_change_time;
	fn->last_access_time = ni->last_access_time;

	/* we don't need to distinguish between file and directory since every
	 * newly created file has a size of 0! TODO: check for special files
	 * like fifo's, blkdevices, etc */
	fn->data_size = fn->allocated_size = const_cpu_to_sle64(0);
	memcpy(fn->file_name, name, name_len * sizeof(ntfschar));
	/* Add FILE_NAME attribute to inode. */
	err = ntfs_attr_add(ni, AT_FILE_NAME, AT_UNNAMED, 0, (u8 *) fn, fn_len);
	if (err != 0) {
		antfs_log_error("Failed to add FILE_NAME attribute: %d", err);
		goto err_out;
	}
	/* Add FILE_NAME attribute to index. */
	err = ntfs_index_add_filename(dir_ni, fn, MK_MREF(ni->mft_no,
			le16_to_cpu(ni->mrec->sequence_number)));
	if (err != 0) {
		antfs_log_error("Failed to add entry to the index: %d", err);
		goto err_out;
	}
	/* Set hard links count */
	ni->mrec->link_count = const_cpu_to_le16(1);
	ntfs_inode_mark_dirty(ni);
	/* Done! */
	ntfs_free(fn);
	ntfs_free(si);
	antfs_log_leave("Done");
	return ni;

err_out:
	inode = ANTFS_I(ni);
	/* Need to remove mft record and inode. Use evict inode
	 * callback to do the job properly.
	 * Make sure the mrec is removed if possible: clear nlink.
	 *
	 * ... set I_NEW to shut up iget_failed.
	 */
	clear_nlink(inode);
	inode->i_state |= I_NEW;
	iget_failed(inode);
	ntfs_free(fn);
	ntfs_free(si);
	antfs_log_error("%d", err);
	return ERR_PTR(err);
}

/**
 * Some wrappers around __ntfs_create() ...
 */
struct ntfs_inode *ntfs_create(struct ntfs_inode *dir_ni, le32 securid,
			       const ntfschar *name, u8 name_len, mode_t type)
{
	if (type != S_IFREG && type != S_IFDIR && type != S_IFIFO) {
		antfs_log_error("Invalid arguments.");
		return ERR_PTR(-EINVAL);
	}
	return __ntfs_create(dir_ni, securid, name, name_len, type, 0, NULL, 0);
}

struct ntfs_inode *ntfs_create_symlink(struct ntfs_inode *dir_ni, le32 securid,
				       const ntfschar *name, u8 name_len,
				       const ntfschar *target, int target_len)
{
	if (!target || !target_len) {
		antfs_log_error("Invalid argument (%p, %d)",
				target, target_len);
		return ERR_PTR(-EINVAL);
	}
	return __ntfs_create(dir_ni, securid, name, name_len, S_IFLNK, 0,
			     target, target_len);
}

int ntfs_check_empty_dir(struct ntfs_inode *ni)
{
	int err = 0;

	if (!(ni->mrec->flags & MFT_RECORD_IS_DIRECTORY))
		return 0;

	/* Non-empty directory? */
	if ((ANTFS_NA(ni)->data_size != sizeof(struct INDEX_ROOT) +
	     sizeof(struct INDEX_ENTRY_HEADER))) {
		/* Both ENOTEMPTY and EEXIST are ok. We use the more common. */
		err = -ENOTEMPTY;
		antfs_log_debug("Directory is not empty.");
	}

	return err;
}

static int ntfs_check_unlinkable_dir(struct ntfs_inode *ni,
				     struct FILE_NAME_ATTR *fn)
{
	int link_count = le16_to_cpu(ni->mrec->link_count);
	int err;

	err = ntfs_check_empty_dir(ni);
	if (!err || err != -ENOTEMPTY)
		return err;
	/*
	 * Directory is non-empty, so we can unlink only if there is more than
	 * one "real" hard link, i.e. links aren't different DOS and WIN32 names
	 */
	if ((link_count == 1) ||
	    (link_count == 2 && fn->file_name_type == FILE_NAME_DOS)) {
		err = -ENOTEMPTY;
		antfs_log_debug("Non-empty directory without hard links");
		goto no_hardlink;
	}

	err = 0;
no_hardlink:
	return err;
}

/**
 * @brief Unlink a NTFS inode from directory
 *
 * @param vol       ntfs volume we are working on
 * @param ni        ntfs inode for object to delte
 * @param dir_ni    ntfs inode for directory in which to delete inode
 * @param name      unicode name of the object to delete
 * @param name_len  length of the name in unicode characters
 *
 * @ni is always closed after the call to this function (even if it failed),
 * user does not need to call ntfs_inode_close himself.
 *
 * @return 0 on success or negative error code
 */
int ntfs_unlink(struct ntfs_volume *vol, struct ntfs_inode *ni,
		struct ntfs_inode *dir_ni, const ntfschar *name, u8 name_len)
{
	struct ntfs_attr_search_ctx *actx = NULL;
	struct FILE_NAME_ATTR *fn = NULL;
	bool looking_for_dos_name = FALSE, looking_for_win32_name = FALSE;
	bool case_sensitive_match = TRUE;
	int err = 0;

	antfs_log_enter("inode %lld in directory %lld", ni->mft_no,
			dir_ni->mft_no);

	if (IS_ERR_OR_NULL(ni) || IS_ERR_OR_NULL(dir_ni) ||
	    !name || !name_len) {
		antfs_log_error("Invalid arguments.");
		err = -EINVAL;
		goto out;
	}
	if (ni->nr_extents == -1)
		ni = ni->base_ni;
	if (dir_ni->nr_extents == -1)
		dir_ni = dir_ni->base_ni;
	/*
	 * Search for FILE_NAME attribute with such name. If it's in POSIX or
	 * WIN32_AND_DOS namespace, then simply remove it from index and inode.
	 * If filename in DOS or in WIN32 namespace, then remove DOS name first,
	 * only then remove WIN32 name.
	 */
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(actx)) {
		err = PTR_ERR(actx);
		goto out;
	}
search:
	while (!
	       (err =
		ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0, CASE_SENSITIVE, 0,
				 NULL, 0, actx))) {
		enum IGNORE_CASE_BOOL case_sensitive = IGNORE_CASE;

		fn = (struct FILE_NAME_ATTR *) ((u8 *) actx->attr +
					 le16_to_cpu(actx->attr->value_offset));
#if ANTFS_LOGLEVEL >= ANTFS_LOGLEVEL_DBG
		{
			char *s;
			s = ntfs_attr_name_get(fn->file_name,
					       fn->file_name_length);
			antfs_log_debug
			    ("name: '%s'  type: %d  dos: %d  win32: %d  "
			     "case: %d", s, fn->file_name_type,
			     looking_for_dos_name, looking_for_win32_name,
			     case_sensitive_match);
			ntfs_attr_name_free(&s);
		}
#endif
		if (looking_for_dos_name) {
			if (fn->file_name_type == FILE_NAME_DOS)
				break;
			else
				continue;
		}
		if (looking_for_win32_name) {
			if (fn->file_name_type == FILE_NAME_WIN32)
				break;
			else
				continue;
		}

		/* Ignore hard links from other directories */
		if (dir_ni->mft_no != MREF_LE(fn->parent_directory)) {
			antfs_log_debug("MFT record numbers don't match "
					"(%lld != %lld)", dir_ni->mft_no,
					MREF_LE(fn->parent_directory));
			continue;
		}
		if (case_sensitive_match
		    || ((fn->file_name_type == FILE_NAME_POSIX)
			&& NVolCaseSensitive(ni->vol)))
			case_sensitive = CASE_SENSITIVE;

		if (ntfs_names_are_equal(fn->file_name, fn->file_name_length,
					 name, name_len, case_sensitive,
					 ni->vol->upcase,
					 ni->vol->upcase_len)) {

			if (fn->file_name_type == FILE_NAME_WIN32) {
				looking_for_dos_name = TRUE;
				ntfs_attr_reinit_search_ctx(actx);
				continue;
			}
			if (fn->file_name_type == FILE_NAME_DOS)
				looking_for_dos_name = TRUE;
			break;
		}
	}
	if (err) {
		/*
		 * If case sensitive search failed, then try once again
		 * ignoring case.
		 */
		if (err == -ENOENT && case_sensitive_match) {
			case_sensitive_match = FALSE;
			ntfs_attr_reinit_search_ctx(actx);
			goto search;
		}
		goto out;
	}

	err = ntfs_check_unlinkable_dir(ni, fn);
	if (err != 0)
		goto out;

	err = ntfs_index_remove(dir_ni, ni, fn,
				le32_to_cpu(actx->attr->value_length));
	if (err != 0)
		goto out;

	/*
	 * Keep the last name in place, this is useful for undeletion
	 * (Windows also does so), however delete the name if it were
	 * in an extent, to avoid leaving an attribute list.
	 */
	if ((ni->mrec->link_count == const_cpu_to_le16(1)) &&
	    !actx->base_ntfs_ino) {
		/* make sure to not loop to another search */
		looking_for_dos_name = FALSE;
	} else {
		err = ntfs_attr_record_rm(actx);
		if (err != 0)
			goto out;
	}

	ni->mrec->link_count =
	    cpu_to_le16(le16_to_cpu(ni->mrec->link_count) - 1);

	ntfs_inode_mark_dirty(ni);
	if (looking_for_dos_name) {
		looking_for_dos_name = FALSE;
		looking_for_win32_name = TRUE;
		ntfs_attr_reinit_search_ctx(actx);
		goto search;
	}

out:
	if (!IS_ERR_OR_NULL(actx))
		ntfs_attr_put_search_ctx(actx);
	if (err)
		antfs_log_error("Could not unlink inode: %d", err);
	antfs_log_leave();
	return err;
}

/**
 * @brief Free a NTFS inode
 *
 * @param ni        ntfs inode for object to delte
 *
 * @ni is always closed after the call to this function (even if it failed),
 * user does not need to call ntfs_inode_close himself.
 *
 * @return 0 on success or negative error code
 */
int ntfs_inode_free(struct ntfs_inode *ni)
{
	struct ntfs_attr_search_ctx *actx = NULL;
	int err = 0, i = 0;

	antfs_log_enter("inode %lld", ni->mft_no);

	if (IS_ERR_OR_NULL(ni)) {
		antfs_log_error("Invalid arguments.");
		err = -EINVAL;
		goto out;
	}
	if (ni->mrec->link_count) {
		antfs_log_error("mrec->link_count = %d", ni->mrec->link_count);
		err = -EINVAL;
		goto out;
	}

	/* If we are an extent inode no freeing of non-resident attributes is
	 * needed. This happens when the base inode gets freed */
	if (ni->nr_extents == -1) {
		antfs_log_debug("skip extend inode %llu", ni->mft_no);
		goto skip_free;
	}
	ntfs_delete_reparse_index(ni);
	/*
	 * Failed to remove the reparse index : proceed anyway
	 * This is not a critical error, the entry is useless
	 * because of sequence_number, and stopping file deletion
	 * would be much worse as the file is not referenced now.
	 */
	ntfs_delete_object_id_index(ni);
	/*
	 * Failed to remove the object id index : proceed anyway
	 * This is not a critical error.
	 */
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (IS_ERR(actx)) {
		err = PTR_ERR(actx);
		goto out;
	}
	/* Free all clusters from non resident attributes. Since this walks the
	 * mrec in RAM this is ok even if this inode collided.
	 */
	while (!(err = ntfs_attrs_walk(actx))) {
		if (actx->attr->non_resident) {
			struct runlist_element *rl;

			rl = ntfs_mapping_pairs_decompress(ni->vol, actx->attr,
					NULL);
			if (IS_ERR(rl)) {
				/* log? we don't know if we just did that */
				antfs_log_error("Failed to decompress "
						"runlist. Leaving inconsistent metadata.");
				continue;
			}
			if (ntfs_cluster_free_from_rl(ni->vol, rl)) {
				ntfs_free(rl);
				antfs_log_error("Failed to free clusters.  "
						"Leaving inconsistent metadata.");
				continue;
			}
			ntfs_free(rl);
		}
	}
	if (err != -ENOENT) {
		/* log? do we corrupt or not? */
		antfs_log_error("Attribute enumeration failed.  "
				"Probably leaving inconsistent metadata.");
	}
	/* All extents should be attached after attribute walk. */
	for (i = 0; i < ni->nr_extents; i++)
		clear_nlink(ANTFS_I(ni->extent_nis[i]));
skip_free:
	err = ntfs_mft_record_free(ni);
	if (err)
		antfs_logger(ANTFS_I(ni)->i_sb->s_id,
				"Failed to free base MFT record. Leaving "
				"inconsistent metadata. err=%d", err);
out:
	if (!IS_ERR_OR_NULL(actx))
		ntfs_attr_put_search_ctx(actx);
	if (err)
		antfs_log_error("Error freeing inode: %d", err);
	antfs_log_leave();
	return err;
}

/**
 * ntfs_link - create hard link for file or directory
 *
 * @param ni        ntfs inode for object to create hard link
 * @param dir_ni    ntfs inode for directory in which new link should be placed
 * @param name      unicode name of the new link
 * @param name_len  length of the name in unicode characters
 *
 * NOTE: At present we allow creating hardlinks to directories, we use them
 * in a temporary state during rename. But it's defenitely bad idea to have
 * hard links to directories as a result of operation.
 *
 * FIXME: Create internal  __ntfs_link that allows hard links to a directories
 * and external ntfs_link that do not. Write ntfs_rename that uses __ntfs_link.
 *
 * @return 0 on success or negative error code
 */
static int ntfs_link_i(struct ntfs_inode *ni, struct ntfs_inode *dir_ni,
		       const ntfschar *name, u8 name_len,
		       enum FILE_NAME_TYPE_FLAGS nametype)
{
	struct FILE_NAME_ATTR *fn = NULL;
	int fn_len, err;

	antfs_log_enter();

	if (IS_ERR_OR_NULL(ni) || IS_ERR_OR_NULL(dir_ni)
	    || !name || !name_len || ni->mft_no == dir_ni->mft_no) {
		err = -EINVAL;
		antfs_log_error("ntfs_link wrong arguments");
		goto err_out;
	}

	if (NVolHideDotFiles(dir_ni->vol)) {
		/* Set hidden flag according to the latest name */
		if ((name_len > 1)
		    && (name[0] == const_cpu_to_le16('.'))
		    && (name[1] != const_cpu_to_le16('.')))
			ni->flags |= FILE_ATTR_HIDDEN;
		else
			ni->flags &= ~FILE_ATTR_HIDDEN;
	}

	/* Create FILE_NAME attribute. */
	fn_len = sizeof(struct FILE_NAME_ATTR) + name_len * sizeof(ntfschar);
	fn = ntfs_calloc(fn_len);
	if (!fn) {
		err = -ENOMEM;
		goto err_out;
	}
	fn->parent_directory = MK_LE_MREF(dir_ni->mft_no,
					  le16_to_cpu(dir_ni->mrec->
						      sequence_number));
	fn->file_name_length = name_len;
	fn->file_name_type = nametype;
	fn->file_attributes = ni->flags;
	if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY) {
		fn->file_attributes |= FILE_ATTR_I30_INDEX_PRESENT;
		fn->data_size = fn->allocated_size = const_cpu_to_sle64(0);
	} else {
		/* The allocated_size field of the file_name_attr holds either
		 * the allocated_size of a file, or the compressed_size.
		 * NOTE: only nonresident files can be compressed or sparse.
		 */
		if (NAttrNonResident(ANTFS_NA(ni)) &&
				((ANTFS_NA(ni)->data_flags &
				  ATTR_COMPRESSION_MASK)
			|| NAttrSparse(ANTFS_NA(ni))))
			fn->allocated_size =
				cpu_to_sle64(ANTFS_NA(ni)->compressed_size);
		else
			fn->allocated_size =
				cpu_to_sle64(ANTFS_NA(ni)->allocated_size);
		fn->data_size = cpu_to_sle64(ANTFS_NA(ni)->data_size);
	}
	fn->creation_time = ni->creation_time;
	fn->last_data_change_time = ni->last_data_change_time;
	fn->last_mft_change_time = ni->last_mft_change_time;
	fn->last_access_time = ni->last_access_time;
	memcpy(fn->file_name, name, name_len * sizeof(ntfschar));
	/* Add FILE_NAME attribute to index. */
	err = ntfs_index_add_filename(dir_ni, fn, MK_MREF(ni->mft_no,
				    le16_to_cpu(ni->mrec->sequence_number)));
	if (err != 0) {
		antfs_log_error("Failed to add filename to the index");
		goto err_out;
	}
	/* Add FILE_NAME attribute to inode. */
	err = ntfs_attr_add(ni, AT_FILE_NAME, AT_UNNAMED, 0, (u8 *) fn, fn_len);
	if (err != 0) {
		antfs_log_error("Failed to add FILE_NAME attribute.");
		/* Try to remove just added attribute from index. */
		if (ntfs_index_remove(dir_ni, ni, fn, fn_len))
			goto rollback_failed;
		goto err_out;
	}
	/* Increment hard links count. */
	ni->mrec->link_count =
	    cpu_to_le16(le16_to_cpu(ni->mrec->link_count) + 1);
	/* Done! */
	ntfs_inode_mark_dirty(ni);
	ntfs_free(fn);
	antfs_log_debug("Done.");
	return 0;

rollback_failed:
	antfs_logger(ANTFS_I(ni)->i_sb->s_id,
		    "Rollback failed. Leaving inconsistent metadata.");
err_out:
	ntfs_free(fn);
	return err;
}

int ntfs_link(struct ntfs_inode *ni, struct ntfs_inode *dir_ni,
	      const ntfschar *name, u8 name_len)
{
	return ntfs_link_i(ni, dir_ni, name, name_len, FILE_NAME_POSIX);
}

/*
 *		Get a parent directory from an inode entry
 *
 *	This is only used in situations where the path used to access
 *	the current file is not known for sure. The result may be different
 *	from the path when the file is linked in several parent directories.
 *
 *	Currently this is only used for translating ".." in the target
 *	of a Vista relative symbolic link
 *
 *	@return ntfs inode pointer or error pointer on failure
 */
struct ntfs_inode *ntfs_dir_parent_inode(struct ntfs_inode *ni)
{
	struct ntfs_inode *dir_ni = (struct ntfs_inode *)ERR_PTR(-ENOENT);
	u64 inum;
	struct FILE_NAME_ATTR *fn;
	struct ntfs_attr_search_ctx *ctx;
	int err;

	if (ni->mft_no != FILE_ROOT) {
		/* find the name in the attributes */
		ctx = ntfs_attr_get_search_ctx(ni, NULL);
		if (IS_ERR(ctx))
			return (struct ntfs_inode *)ctx;

		err = ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0,
				       CASE_SENSITIVE, 0, NULL, 0, ctx);
		if (!err) {
			/* We know this will always be resident. */
			fn = (struct FILE_NAME_ATTR *) ((u8 *) ctx->attr +
						 le16_to_cpu(ctx->attr->
							     value_offset));
			inum = le64_to_cpu(fn->parent_directory);
			if (inum != (u64) -1)
				dir_ni = ntfs_inode_open(ni->vol, MREF(inum),
							 NULL);
		} else {
			dir_ni = ERR_PTR(err);
		}
		ntfs_attr_put_search_ctx(ctx);
	}
	return dir_ni;
}
