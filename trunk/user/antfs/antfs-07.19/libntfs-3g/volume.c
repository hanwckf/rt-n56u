/**
 * volume.c - NTFS volume handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2006 Anton Altaparmakov
 * Copyright (c) 2002-2009 Szabolcs Szakacsits
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2010      Jean-Pierre Andre
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

#include "antfs.h"

#include "param.h"
#include "volume.h"
#include "attrib.h"
#include "mft.h"
#include "bootsect.h"
#include "device.h"
#include "debug.h"
#include "inode.h"
#include "runlist.h"
#include "logfile.h"
#include "dir.h"
#include "misc.h"

/**
 * ntfs_volume_alloc - Create an NTFS volume object and initialise it
 *
 * Description...
 *
 * Returns:
 */
struct ntfs_volume *ntfs_volume_alloc(void)
{
	return ntfs_calloc(sizeof(struct ntfs_volume));
}

static void ntfs_inode_close_free(struct ntfs_inode **ni)
{
	if (ni && *ni) {
		ntfs_inode_close(*ni);
		*ni = NULL;
	}
}

/**
 * __ntfs_volume_release - Destroy an NTFS volume object
 * @v:
 *
 * Description...
 *
 * Returns:
 */
static int __ntfs_volume_release(struct ntfs_volume *v)
{
	int tmp_err, err = 0;

	ntfs_inode_close_free(&v->vol_ni);
	if (v->mftbmp_bh) {
		brelse(v->mftbmp_bh);
		v->mftbmp_bh = NULL;
	}
	if (v->lcnbmp_bh) {
		brelse(v->lcnbmp_bh);
		v->lcnbmp_bh = NULL;
	}
	/*
	 * FIXME: Inodes must be synced before closing
	 * attributes, otherwise unmount could fail.
	 */
	if (v->lcnbmp_ni && NInoDirty(v->lcnbmp_ni))
		ntfs_inode_sync(v->lcnbmp_ni);
	v->lcnbmp_na = NULL;
	ntfs_inode_close_free(&v->lcnbmp_ni);

	if (v->mft_ni && NInoDirty(v->mft_ni))
		ntfs_inode_sync(v->mft_ni);
	ntfs_attr_close(v->mftbmp_na);
	v->mftbmp_na = NULL;

	if (!IS_ERR_OR_NULL(v->mftmirr_ni) && NInoDirty(v->mftmirr_ni))
		ntfs_inode_sync(v->mftmirr_ni);
	v->mftmirr_na = NULL;
	ntfs_inode_close_free(&v->mftmirr_ni);

	/*
	 * close mft_ni last because other inodes might access vol->mft_na->rl
	 */
	ntfs_inode_close_free(&v->mft_ni);
	v->mft_na = NULL;

	if (v->dev) {
		struct ntfs_device *dev = v->dev;

		tmp_err = dev->d_ops->sync(dev);
		if (tmp_err && !err)
			err = tmp_err;
		tmp_err = dev->d_ops->close(dev);
		if (tmp_err && !err)
			err = tmp_err;
	}

	ntfs_free(v->vol_name);
	ntfs_free(v->upcase);
	if (v->locase)
		ntfs_free(v->locase);
	ntfs_free(v->attrdef);
	ntfs_free(v);

	return err;
}

static void ntfs_attr_setup_flag(struct ntfs_inode *ni)
{
	struct STANDARD_INFORMATION *si;

	si = ntfs_attr_readall(ni, AT_STANDARD_INFORMATION, AT_UNNAMED,
			       0, NULL);
	if (!IS_ERR(si)) {
		ni->flags = si->file_attributes;
		ntfs_free(si);
	}
}

/**
 * ntfs_mft_load - load the $MFT and setup the ntfs volume with it
 * @vol:	ntfs volume whose $MFT to load
 *
 * Load $MFT from @vol and setup @vol with it. After calling this function the
 * volume @vol is ready for use by all read access functions provided by the
 * ntfs library.
 *
 * Return 0 on success and on error the error code.
 */
static int ntfs_mft_load(struct ntfs_volume *vol)
{
	VCN next_vcn, last_vcn, highest_vcn;
	s64 l;
	struct inode *inode;
	struct MFT_RECORD *mb = NULL;
	struct ntfs_attr_search_ctx *ctx = NULL;
	struct ATTR_RECORD *a;
	int err;

	antfs_log_enter("vol=%p", vol);
	/* Manually setup an ntfs_inode. */
	vol->mft_ni = ntfs_inode_allocate(vol);
	mb = ntfs_malloc(vol->mft_record_size);
	if (IS_ERR(vol->mft_ni) || !mb) {
		antfs_log_error("Error allocating memory for $MFT");
		ntfs_free(mb);
		if (IS_ERR(vol->mft_ni))
			err = PTR_ERR(vol->mft_ni);
		else
			err = -ENOMEM;
		goto error_exit;
	}
	vol->mft_ni->mft_no = 0;
	vol->mft_ni->mrec = mb;
	/* We don't actually "create" the MFT here, but if this inode_init
	 * fails, we're in trouble.
	 */
	inode = ANTFS_I(vol->mft_ni);
	err = antfs_inode_init(&inode, ANTFS_INODE_INIT_DISCARD);
	if (err) {
		antfs_log_error("Failed to initialize the MFT!");
		vol->mft_ni = NULL;
		goto error_exit;
	}
	/* Can't use any of the higher level functions yet! */
	l = ntfs_mst_pread(vol->dev, vol->mft_lcn << vol->cluster_size_bits, 1,
			vol->mft_record_size, mb);
	if (l != 1) {
		if (l >= 0)
			err = -EIO;
		else
			err = l;
		antfs_log_error("Error reading $MFT");
		goto error_exit;
	}

	err = ntfs_mft_record_check(vol, 0, mb);
	if (err)
		goto error_exit;

	ctx = ntfs_attr_get_search_ctx(vol->mft_ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto error_exit;
	}

	/* Find the $ATTRIBUTE_LIST attribute in $MFT if present. */
	err = ntfs_attr_lookup(AT_ATTRIBUTE_LIST, AT_UNNAMED, 0, 0, 0, NULL, 0,
			ctx);
	if (err) {
		if (err != -ENOENT) {
			antfs_log_error("$MFT has corrupt attribute list.");
			goto io_error_exit;
		}
		goto mft_has_no_attr_list;
	}
	NInoSetAttrList(vol->mft_ni);
	l = ntfs_get_attribute_value_length(ctx->attr);
	if (l <= 0 || l > 0x40000) {
		antfs_log_error("$MFT/$ATTR_LIST invalid length (%lld).",
			       (long long)l);
		err = l < 0 ? (int)l : -EIO;
		goto error_exit;
	}
	vol->mft_ni->attr_list_size = l;
	vol->mft_ni->attr_list = ntfs_malloc(l);
	if (!vol->mft_ni->attr_list) {
		err = -ENOMEM;
		goto error_exit;
	}

	l = ntfs_get_attribute_value(vol, ctx->attr, vol->mft_ni->attr_list);
	if (l <= 0) {
		antfs_log_error("Failed to get value of $MFT/$ATTR_LIST.");
		goto io_error_exit;
	}
	if (l != vol->mft_ni->attr_list_size) {
		antfs_log_error("Partial read of $MFT/$ATTR_LIST (%lld != "
			       "%u).", (long long)l,
			       vol->mft_ni->attr_list_size);
		goto io_error_exit;
	}

mft_has_no_attr_list:

	ntfs_attr_setup_flag(vol->mft_ni);

	/* We now have a fully setup ntfs inode for $MFT in vol->mft_ni. */

	/* Get an ntfs attribute for $MFT/$DATA and set it up, too. */
	err = ntfs_attr_sah_open(vol->mft_ni, AT_DATA, AT_UNNAMED, 0);
	if (!err) {
		vol->mft_na = ANTFS_NA(vol->mft_ni);
	} else {
		vol->mft_na = NULL;
		antfs_log_error("Failed to open ntfs attribute");
		goto error_exit;
	}
	/* Read all extents from the $DATA attribute in $MFT. */
	ntfs_attr_reinit_search_ctx(ctx);
	last_vcn = vol->mft_na->allocated_size >> vol->cluster_size_bits;
	highest_vcn = next_vcn = 0;
	a = NULL;
	while (!ntfs_attr_lookup(AT_DATA, AT_UNNAMED, 0, 0, next_vcn, NULL, 0,
			ctx)) {
		struct runlist_element *nrl;

		a = ctx->attr;
		/* $MFT must be non-resident. */
		if (!a->non_resident) {
			antfs_log_error("$MFT must be non-resident.");
			goto io_error_exit;
		}
		/* $MFT must be uncompressed and unencrypted. */
		if (a->flags & ATTR_COMPRESSION_MASK ||
				a->flags & ATTR_IS_ENCRYPTED) {
			antfs_log_error("$MFT must be uncompressed and "
				       "unencrypted.");
			goto io_error_exit;
		}
		/*
		 * Decompress the mapping pairs array of this extent and merge
		 * the result into the existing runlist. No need for locking
		 * as we have exclusive access to the inode at this time and we
		 * are a mount in progress task, too.
		 */
		nrl = ntfs_mapping_pairs_decompress(vol, a, vol->mft_na->rl);
		if (IS_ERR(nrl)) {
			antfs_log_error("ntfs_mapping_pairs_decompress() "
					"failed");
			err = PTR_ERR(nrl);
			if (!err)
				err = -EIO;
			goto error_exit;
		}
		vol->mft_na->rl = nrl;

		/* Get the lowest vcn for the next extent. */
		highest_vcn = sle64_to_cpu(a->highest_vcn);
		next_vcn = highest_vcn + 1;

		/* Only one extent or error, which we catch below. */
		if (next_vcn <= 0)
			break;

		/* Avoid endless loops due to corruption. */
		if (next_vcn < sle64_to_cpu(a->lowest_vcn)) {
			antfs_log_error("$MFT has corrupt attribute list.");
			goto io_error_exit;
		}
	}
	if (!a) {
		antfs_log_error("$MFT/$DATA attribute not found.");
		goto io_error_exit;
	}
	if (highest_vcn && highest_vcn != last_vcn - 1) {
		antfs_log_error("Failed to load runlist for $MFT/$DATA.");
		antfs_log_error("highest_vcn = 0x%llx, last_vcn - 1 = 0x%llx",
			       (long long)highest_vcn, (long long)last_vcn - 1);
		goto io_error_exit;
	}
	/* Done with the $Mft mft record. */
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;

	/*
	 * The volume is now setup so we can use all read access functions.
	 */
	vol->mftbmp_na = ntfs_attr_open(vol->mft_ni, AT_BITMAP, AT_UNNAMED, 0);
	if (IS_ERR(vol->mftbmp_na)) {
		antfs_log_error("Failed to open $MFT/$BITMAP");
		err = PTR_ERR(vol->mftbmp_na);
		vol->mftbmp_na = NULL;
		goto error_exit;
	}
	return 0;
io_error_exit:
	err = -EIO;
error_exit:
	if (!IS_ERR_OR_NULL(ctx))
		ntfs_attr_put_search_ctx(ctx);
	if (!IS_ERR_OR_NULL(vol->mft_ni))
		ntfs_inode_close_free(&vol->mft_ni);
	antfs_log_error("Exit with error %d", err);
	return err;
}

/**
 * ntfs_mftmirr_load - load the $MFTMirr and setup the ntfs volume with it
 * @vol:	ntfs volume whose $MFTMirr to load
 *
 * Load $MFTMirr from @vol and setup @vol with it. After calling this function
 * the volume @vol is ready for use by all write access functions provided by
 * the ntfs library (assuming ntfs_mft_load() has been called successfully
 * beforehand).
 *
 * Return 0 on success and on error with the error code.
 */
static int ntfs_mftmirr_load(struct ntfs_volume *vol)
{
	int err = 0;

	vol->mftmirr_ni = ntfs_inode_open(vol, FILE_MFTMIRR, NULL);
	if (IS_ERR(vol->mftmirr_ni)) {
		err = PTR_ERR(vol->mftmirr_ni);
		vol->mftmirr_ni = NULL;
		antfs_log_error("Failed to open inode $MFTMirr");
		goto out;
	}

	vol->mftmirr_na = ANTFS_NA(vol->mftmirr_ni);

	err = ntfs_attr_map_runlist(vol->mftmirr_na, 0);
	if (err < 0) {
		antfs_log_error("Failed to map runlist of $MFTMirr/$DATA");
		vol->mftmirr_na = NULL;
		goto error_exit;
	}

out:
	return err;

error_exit:
	ntfs_inode_close_free(&vol->mftmirr_ni);
	goto out;
}

/**
 * ntfs_volume_startup - allocate and setup an ntfs volume
 * @dev:	device to open
 * @flags:	optional mount flags
 *
 * Load, verify, and parse bootsector; load and setup $MFT and $MFTMirr. After
 * calling this function, the volume is setup sufficiently to call all read
 * and write access functions provided by the library.
 *
 * Return the allocated volume structure on success and on error the error code.
 */
struct ntfs_volume *ntfs_volume_startup(struct ntfs_device *dev,
		enum ntfs_mount_flags flags)
{
	LCN mft_zone_size, mft_lcn;
	s64 br;
	struct ntfs_volume *vol;
	struct NTFS_BOOT_SECTOR *bs;
	int err = 0;

	if (!dev || !dev->d_ops || !dev->d_name) {
		vol = ERR_PTR(-EINVAL);
		goto out;
	}

	bs = ntfs_malloc(sizeof(struct NTFS_BOOT_SECTOR));
	if (!bs) {
		vol = ERR_PTR(-ENOMEM);
		goto out;
	}

	/* Allocate the volume structure. */
	vol = ntfs_volume_alloc();
	if (!vol) {
		err = -ENOMEM;
		goto error_exit;
	}

	/* Create the default upcase table. */
	vol->upcase_len = ntfs_upcase_build_default(&vol->upcase);
	if (!vol->upcase_len || !vol->upcase) {
		err = -ENOMEM;
		goto error_exit;
	}

	/* Default with no locase table and case sensitive file names */
	vol->locase = (ntfschar *)NULL;
	NVolSetCaseSensitive(vol);

		/* by default, all files are shown and not marked hidden */
	NVolSetShowSysFiles(vol);
	NVolSetShowHidFiles(vol);
	NVolClearHideDotFiles(vol);
		/* set default compression */
#if DEFAULT_COMPRESSION
	NVolSetCompression(vol);
#else
	NVolClearCompression(vol);
#endif
	if (flags & NTFS_MNT_RDONLY)
		NVolSetReadOnly(vol);
	mutex_init(&vol->mftbmp_lock);
	mutex_init(&vol->lcnbmp_lock);
	spin_lock_init(&vol->mftbmp_spin_lock);
	vol->mftbmp_start = 0;

	memset(&vol->ni_stack, 0, sizeof(vol->ni_stack));
	mutex_init(&vol->ni_lock);
	sema_init(&vol->ni_stack_sem, ARRAY_SIZE(vol->ni_stack));

	/* ...->open needs bracketing to compile with glibc 2.7 */
	err = (dev->d_ops->open)(dev, NVolReadOnly(vol) ? O_RDONLY : O_RDWR);
	if (err) {
		if (!NVolReadOnly(vol) && (err == -EROFS)) {
			err = (dev->d_ops->open)(dev, O_RDONLY);
			if (err) {
				antfs_log_error("Error opening read-only '%s'",
						dev->d_name);
				goto error_exit;
			} else {
				antfs_log_info("'%s' is read-only",
					       dev->d_name);
				NVolSetReadOnly(vol);
			}
		} else {
			antfs_log_error("Error opening '%s'", dev->d_name);
			goto error_exit;
		}
	}
	/* Attach the device to the volume. */
	vol->dev = dev;

	/* Now read the bootsector. */
	br = ntfs_pread(dev, 0, sizeof(struct NTFS_BOOT_SECTOR), bs);
	if (br != sizeof(struct NTFS_BOOT_SECTOR)) {
		if (!br) {
			antfs_log_error("Failed to read bootsector (size=0)");
			err = -EINVAL;
		} else {
			antfs_log_error("Error reading bootsector (%lld)", br);
			err = br;
		}
		goto error_exit;
	}
	if (!ntfs_boot_sector_is_ntfs(bs)) {
		err = -EINVAL;
		goto error_exit;
	}
	err = ntfs_boot_sector_parse(vol, bs);
	if (err < 0)
		goto error_exit;

	ntfs_free(bs);
	bs = NULL;
	/* Now set the device block size to the sector size. */
	if (ntfs_device_block_size_set(vol->dev, vol->cluster_size)) {
		/* Probably cluster size is too big. Try a single page instead
		 * if this is smaller than a cluster. */
		antfs_log_debug("Failed to set block size to cluster size (%u)",
				vol->cluster_size);
		if (vol->cluster_size <= BLOCK_SIZE ||
		ntfs_device_block_size_set(vol->dev, BLOCK_SIZE)) {
			antfs_log_error("Also Failed to set block size to "
					"BLOCK_SIZE (%u). Cannot continue.",
					BLOCK_SIZE);
			err = -EINVAL;
			goto error_exit;
		}
	}

	/* We now initialize the cluster allocator. */
	vol->full_zones = 0;
	mft_zone_size = vol->nr_clusters >> 3;      /* 12.5% */

	/* Setup the mft zone. */
	vol->mft_zone_start = vol->mft_zone_pos = vol->mft_lcn;
	antfs_log_debug("mft_zone_pos = 0x%llx", (long long)vol->mft_zone_pos);

	/*
	 * Calculate the mft_lcn for an unmodified NTFS volume (see mkntfs
	 * source) and if the actual mft_lcn is in the expected place or even
	 * further to the front of the volume, extend the mft_zone to cover the
	 * beginning of the volume as well. This is in order to protect the
	 * area reserved for the mft bitmap as well within the mft_zone itself.
	 * On non-standard volumes we don't protect it as the overhead would be
	 * higher than the speed increase we would get by doing it.
	 */
	mft_lcn = (8192 + 2 * vol->cluster_size - 1) / vol->cluster_size;
	if (mft_lcn * vol->cluster_size < 16 * 1024)
		mft_lcn = (16 * 1024 + vol->cluster_size - 1) /
				vol->cluster_size;
	if (vol->mft_zone_start <= mft_lcn)
		vol->mft_zone_start = 0;
	antfs_log_debug("mft_zone_start = 0x%llx",
			(long long)vol->mft_zone_start);

	/*
	 * Need to cap the mft zone on non-standard volumes so that it does
	 * not point outside the boundaries of the volume. We do this by
	 * halving the zone size until we are inside the volume.
	 */
	vol->mft_zone_end = vol->mft_lcn + mft_zone_size;
	while (vol->mft_zone_end >= vol->nr_clusters) {
		mft_zone_size >>= 1;
		vol->mft_zone_end = vol->mft_lcn + mft_zone_size;
	}
	antfs_log_debug("mft_zone_end = 0x%llx", (long long)vol->mft_zone_end);

	/*
	 * Set the current position within each data zone to the start of the
	 * respective zone.
	 */
	vol->data1_zone_pos = vol->mft_zone_end;
	antfs_log_debug("data1_zone_pos = %lld",
			(long long)vol->data1_zone_pos);
	vol->data2_zone_pos = 0;
	antfs_log_debug("data2_zone_pos = %lld",
			(long long)vol->data2_zone_pos);

	/* Set the mft data allocation position to mft record 24. */
	vol->mft_data_pos = 24;

	/*
	 * The cluster allocator is now fully operational.
	 */

	/* Need to setup $MFT so we can use the library read functions. */
	err = ntfs_mft_load(vol);
	if (err) {
		antfs_log_error("Failed to load $MFT");
		goto error_exit;
	}

	/* Need to setup $MFTMirr so we can use the write functions, too. */
	err = ntfs_mftmirr_load(vol);
	if (err) {
		antfs_log_error("Failed to load $MFTMirr");
		goto error_exit;
	}
out:
	return vol;
error_exit:
	ntfs_free(bs);
	if (vol)
		__ntfs_volume_release(vol);	
	vol = ERR_PTR(err);
	goto out;
}

/**
 * ntfs_volume_check_logfile - check logfile on target volume
 * @vol:	volume on which to check logfile
 *
 * Return 0 on success and error code on error.
 */
static int ntfs_volume_check_logfile(struct ntfs_volume *vol)
{
	struct ntfs_inode *ni;
	struct ntfs_attr *na = NULL;
	struct RESTART_PAGE_HEADER *rp = NULL;
	int ret = 0;

	ni = ntfs_inode_open(vol, FILE_LOGFILE, NULL);
	if (IS_ERR(ni)) {
		antfs_log_error("Failed to open inode FILE_LogFile");
		ret = PTR_ERR(ni);
		goto out;
	}

	na = ANTFS_NA(ni);

	if (!ntfs_check_logfile(na, &rp) || !ntfs_is_logfile_clean(na, rp))
		ret = -EOPNOTSUPP;
		/*
		 * If the latest restart page was identified as version
		 * 2.0, then Windows may have kept a cached copy of
		 * metadata for fast restarting, and we should not mount.
		 * Hibernation will be seen the same way on a non
		 * Windows-system partition, so we have to use the same
		 * error code (EPERM).
		 * The restart page may also be identified as version 2.0
		 * when access to the file system is terminated abruptly
		 * by unplugging or power cut, so mounting is also rejected
		 * after such an event.
		 */
	if (rp
	    && (rp->major_ver == const_cpu_to_le16(2))
	    && (rp->minor_ver == const_cpu_to_le16(0))) {
		antfs_log_error("Metadata kept in Windows cache, refused to "
				"mount.");
		ret = -EPERM;
	}
	ntfs_free(rp);
	ntfs_inode_close(ni);
out:
	return ret;
}

/**
 * ntfs_hiberfile_open - Find and open '/hiberfil.sys'
 * @vol:    An ntfs volume obtained from ntfs_mount
 *
 * Return:  inode  Success, hiberfil.sys is valid
 *	    error code hiberfil.sys doesn't exist or some other error occurred
 */
static struct ntfs_inode *ntfs_hiberfile_open(struct ntfs_volume *vol)
{
	u64 mft_no;
	struct ntfs_inode *ni_root;
	struct ntfs_inode *ni_hibr = NULL;
	ntfschar   *unicode = NULL;
	int unicode_len;
	const char *hiberfile = "hiberfil.sys";
	int err;

	if (!vol) {
		ni_hibr = ERR_PTR(-EINVAL);
		goto out;
	}

	ni_root = ntfs_inode_open(vol, FILE_ROOT, NULL);
	if (IS_ERR(ni_root)) {
		antfs_log_debug("Couldn't open the root directory.");
		ni_hibr = ni_root;
		goto out;
	}

	unicode_len = ntfs_mbstoucs(hiberfile, &unicode);
	if (unicode_len < 0) {
		antfs_log_error("Couldn't convert 'hiberfil.sys' to Unicode");
		ni_hibr = ERR_PTR(unicode_len);
		goto out_close;
	}

	err = ntfs_inode_lookup_by_name(ni_root, unicode, unicode_len, &mft_no,
			NULL);
	if (err) {
		antfs_log_debug("Couldn't find file '%s'.", hiberfile);
		ni_hibr = ERR_PTR(err);
		goto out_close;
	}

	mft_no = MREF(mft_no);
	ni_hibr = ntfs_inode_open(vol, mft_no, NULL);
	if (IS_ERR(ni_hibr)) {
		antfs_log_debug("Couldn't open inode %lld.", (long long)mft_no);
	}
out_close:
	ntfs_inode_close(ni_root);
	ntfs_free(unicode);
out:
	return ni_hibr;
}


#define NTFS_HIBERFILE_HEADER_SIZE	4096

/**
 * ntfs_volume_check_hiberfile - check hiberfil.sys whether Windows is
 *                               hibernated on the target volume
 * @vol:    volume on which to check hiberfil.sys
 *
 * Return:  0 if Windows isn't hibernated for sure
 *          error code otherwise
 */
int ntfs_volume_check_hiberfile(struct ntfs_volume *vol, int verbose)
{
	struct ntfs_inode *ni;
	struct ntfs_attr *na = NULL;
	int bytes_read, err = 0;
	char *buf = NULL;

	ni = ntfs_hiberfile_open(vol);
	if (IS_ERR(ni)) {
		err = PTR_ERR(ni);
		if (err == -ENOENT)
			err = 0;
		goto out;
	}

	buf = ntfs_malloc(NTFS_HIBERFILE_HEADER_SIZE);
	if (!buf) {
		err = -ENOMEM;
		goto out;
	}

	na = ANTFS_NA(ni);

	bytes_read = ntfs_attr_pread(na, 0, NTFS_HIBERFILE_HEADER_SIZE, buf);
	if (bytes_read < 0) {
		antfs_log_error("Failed to read hiberfil.sys");
		err = bytes_read;
		goto free_out;
	}
	if (bytes_read < NTFS_HIBERFILE_HEADER_SIZE) {
		if (verbose)
			antfs_log_error("Hibernated non-system partition, "
				       "refused to mount.");
		err = -EPERM;
		goto free_out;
	}
	if ((memcmp(buf, "hibr", 4) == 0)
	   ||  (memcmp(buf, "HIBR", 4) == 0)) {
		if (verbose)
			antfs_log_error("Windows is hibernated, refused to "
					"mount.");
		err = -EPERM;
		goto free_out;
	}
	/* All right, all header bytes are zero */
free_out:
	ntfs_free(buf);
	ntfs_inode_close(ni);
out:
	return err;
}

/*
 *		Make sure a LOGGED_UTILITY_STREAM attribute named "$TXF_DATA"
 *	on the root directory is resident.
 *	When it is non-resident, the partition cannot be mounted on Vista
 *	(see http://support.microsoft.com/kb/974729)
 *
 *	We take care to avoid this situation, however this can be a
 *	consequence of having used an older version (including older
 *	Windows version), so we had better fix it.
 *
 *	Returns 0 if unneeded or successful
 *		error code if there was an error
 */
static int fix_txf_data(struct ntfs_volume *vol)
{
	void *txf_data;
	s64 txf_data_size;
	struct ntfs_inode *ni;
	struct ntfs_attr *na;
	int res = 0;

	antfs_log_debug("Loading root directory");
	ni = ntfs_inode_open(vol, FILE_ROOT, NULL);
	if (IS_ERR(ni)) {
		antfs_log_error("Failed to open root directory");
		res = PTR_ERR(ni);
	} else {
		/* Get the $TXF_DATA attribute */
		na = ntfs_attr_open(ni, AT_LOGGED_UTILITY_STREAM, TXF_DATA, 9);
		if (!IS_ERR(na)) {
			if (NAttrNonResident(na)) {
				/*
				 * Fix the attribute by truncating, then
				 * rewriting it.
				 */
				antfs_log_debug("Making $TXF_DATA resident");
				txf_data = ntfs_attr_readall(ni,
						AT_LOGGED_UTILITY_STREAM,
						TXF_DATA, 9, &txf_data_size);
				if (!IS_ERR(txf_data)) {
					res = ntfs_attr_remove(ni,
						AT_LOGGED_UTILITY_STREAM,
						TXF_DATA, 9);
					/*
					 * Don't write attribute if it's to big
					 * to be resident
					 */
					if (!res && txf_data_size <=
					    vol->mft_record_size)
						res = ntfs_attr_add(ni,
							AT_LOGGED_UTILITY_STREAM,
							TXF_DATA, 9,
							txf_data, txf_data_size);
					ntfs_free(txf_data);
				}
				if (res)
					antfs_log_error("Failed to make "
							"$TXF_DATA resident");
				else
					antfs_log_error("$TXF_DATA made "
							"resident");
			}
		}
		ntfs_inode_close(ni);
	}

	return res;
}

/**
 * ntfs_device_mount - open ntfs volume
 * @dev:	device to open
 * @flags:	optional mount flags
 *
 * This function mounts an ntfs volume. @dev should describe the device which
 * to mount as the ntfs volume.
 *
 * @flags is an optional second parameter. The same flags are used as for
 * the mount system call (man 2 mount). Currently only the following flag
 * is implemented:
 *	NTFS_MNT_RDONLY	- mount volume read-only
 *
 * The function opens the device @dev and verifies that it contains a valid
 * bootsector. Then, it allocates an ntfs_volume structure and initializes
 * some of the values inside the structure from the information stored in the
 * bootsector. It proceeds to load the necessary system files and completes
 * setting up the structure.
 *
 * Return the allocated volume structure on success or the error code on error
 */
struct ntfs_volume *ntfs_device_mount(struct ntfs_device *dev,
				      enum ntfs_mount_flags flags)
{
	s64 l;
	struct ntfs_volume *vol;
	u8 *m = NULL, *m2 = NULL;
	struct ntfs_attr_search_ctx *ctx = NULL;
	struct ntfs_inode *ni;
	struct ntfs_attr *na;
	struct ATTR_RECORD *a;
	struct VOLUME_INFORMATION *vinf;
	ntfschar *vname;
	u32 record_size;
	int i, j;
	int err = 0;
	unsigned int k;
	u32 u;
	int need_fallback_ro = FALSE;

	vol = ntfs_volume_startup(dev, flags);
	antfs_log_debug("vol:%p", (void *)vol);
	if (IS_ERR(vol))
		return vol;

	/* Load data from $MFT and $MFTMirr and compare the contents. */
	m  = ntfs_malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	m2 = ntfs_malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	if (!m || !m2) {
		err = -ENOMEM;
		goto error_exit;
	}

	l = ntfs_attr_mst_pread(vol->mft_na, 0, vol->mftmirr_size,
			vol->mft_record_size_bits, m, true);
	if (l != vol->mftmirr_size) {
		if (l < 0) {
			antfs_log_error("Failed to read $MFT");
			err = l;
		} else {
			antfs_log_error("Failed to read $MFT, unexpected length"
				       " (%lld != %d).", (long long)l,
				       vol->mftmirr_size);
			err = -EIO;
		}
		goto error_exit;
	}
	l = ntfs_attr_mst_pread(vol->mftmirr_na, 0, vol->mftmirr_size,
			vol->mft_record_size_bits, m2, true);
	if (l != vol->mftmirr_size) {
		if (l < 0) {
			err = l;
			antfs_log_error("Failed to read $MFTMirr");
			goto error_exit;
		}
		vol->mftmirr_size = l;
	}
	antfs_log_debug("Comparing $MFTMirr to $MFT...");
	for (i = 0; i < vol->mftmirr_size; ++i) {
		struct MFT_RECORD *mrec, *mrec2;
		const char *ESTR[12] = { "$MFT", "$MFTMirr", "$LogFile",
			"$Volume", "$AttrDef", "root directory", "$Bitmap",
			"$Boot", "$BadClus", "$Secure", "$UpCase", "$Extend" };
		const char *s;

		if (i < 12)
			s = ESTR[i];
		else if (i < 16)
			s = "system file";
		else
			s = "mft record";

		mrec = (struct MFT_RECORD *)(m + i * vol->mft_record_size);
		if (mrec->flags & MFT_RECORD_IN_USE) {
			if (ntfs_is_baad_record(mrec->magic)) {
				antfs_log_error("$MFT error: Incomplete multi "
					       "sector transfer detected in "
					       "'%s'.", s);
				goto io_error_exit;
			}
			if (!ntfs_is_mft_record(mrec->magic)) {
				antfs_log_error("$MFT error: Invalid mft "
						"record for '%s'.", s);
				goto io_error_exit;
			}
		}
		mrec2 = (struct MFT_RECORD *)(m2 + i * vol->mft_record_size);
		if (mrec2->flags & MFT_RECORD_IN_USE) {
			if (ntfs_is_baad_record(mrec2->magic)) {
				antfs_log_error("$MFTMirr error: Incomplete "
						"multi sector transfer "
						"detected in '%s'.", s);
				goto io_error_exit;
			}
			if (!ntfs_is_mft_record(mrec2->magic)) {
				antfs_log_error("$MFTMirr error: Invalid mft "
						"record for '%s'.", s);
				goto io_error_exit;
			}
		}
		record_size = ntfs_mft_record_get_data_size(mrec);
		if ((record_size <= sizeof(struct MFT_RECORD))
		    || (record_size > vol->mft_record_size)
		    || memcmp(mrec, mrec2, record_size)) {
			antfs_log_error("$MFTMirr does not match $MFT (record "
				       "%d).", i);
			goto io_error_exit;
		}
	}

	ntfs_free(m2);
	ntfs_free(m);
	m = m2 = NULL;

	/* Now load the bitmap from $Bitmap. */
	antfs_log_debug("Loading $Bitmap...");
	vol->lcnbmp_ni = ntfs_inode_open(vol, FILE_BITMAP, NULL);
	if (IS_ERR(vol->lcnbmp_ni)) {
		antfs_log_error("Failed to open inode FILE_Bitmap");
		err = PTR_ERR(vol->lcnbmp_ni);
		vol->lcnbmp_ni = NULL;
		goto error_exit;
	}

	vol->lcnbmp_na = ANTFS_NA(vol->lcnbmp_ni);

	if (vol->lcnbmp_na->data_size > vol->lcnbmp_na->allocated_size) {
		antfs_log_error("Corrupt cluster map size (%lld > %lld)",
				(long long)vol->lcnbmp_na->data_size,
				(long long)vol->lcnbmp_na->allocated_size);
		goto io_error_exit;
	}

	/* Now load the upcase table from $UpCase. */
	antfs_log_debug("Loading $UpCase...");
	ni = ntfs_inode_open(vol, FILE_UPCASE, NULL);
	if (IS_ERR(ni)) {
		antfs_log_error("Failed to open inode FILE_UpCase");
		err = PTR_ERR(ni);
		goto error_exit;
	}
	/* Get an ntfs attribute for $UpCase/$DATA. */
	na = ANTFS_NA(ni);
	/*
	 * Note: Normally, the upcase table has a length equal to 65536
	 * 2-byte Unicode characters but allow for different cases, so no
	 * checks done. Just check we don't overflow 32-bits worth of Unicode
	 * characters.
	 */
	if (na->data_size & ~0x1ffffffffULL) {
		antfs_log_error("Error: Upcase table is too big (max 32-bit "
				"allowed).");
		err = -EINVAL;
		goto error_exit;
	}
	if (vol->upcase_len != na->data_size >> 1) {
		vol->upcase_len = na->data_size >> 1;
		/* Throw away default table. */
		ntfs_free(vol->upcase);
		vol->upcase = ntfs_malloc(na->data_size);
		if (!vol->upcase) {
			err = -ENOMEM;
			goto error_exit;
		}
	}
	/* Read in the $DATA attribute value into the buffer. */
	l = ntfs_attr_pread(na, 0, na->data_size, vol->upcase);
	/* Done with the $UpCase mft record. */
	ntfs_inode_close(ni);
	if (l != na->data_size) {
		antfs_log_error("Failed to read $UpCase, unexpected length "
			       "(%lld != %lld).", (long long)l,
			       (long long)na->data_size);
		if (l >= 0)
			goto io_error_exit;
		err = l;
		goto error_exit;
	}
	/* Consistency check of $UpCase, restricted to plain ASCII chars */
	k = 0x20;
	while ((k < vol->upcase_len)
	    && (k < 0x7f)
	    && (le16_to_cpu(vol->upcase[k])
			== ((k < 'a') || (k > 'z') ? k : k + 'A' - 'a')))
		k++;
	if (k < 0x7f) {
		antfs_log_error("Corrupted file $UpCase");
		goto io_error_exit;
	}

	/*
	 * Now load $Volume and set the version information and flags in the
	 * vol structure accordingly.
	 */
	antfs_log_debug("Loading $Volume...");
	vol->vol_ni = ntfs_inode_open(vol, FILE_VOLUME, NULL);
	if (IS_ERR(vol->vol_ni)) {
		antfs_log_error("Failed to open inode FILE_Volume");
		err = PTR_ERR(vol->vol_ni);
		vol->vol_ni = NULL;
		goto error_exit;
	}
	/* Get a search context for the $Volume/$VOLUME_INFORMATION lookup. */
	ctx = ntfs_attr_get_search_ctx(vol->vol_ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto error_exit;
	}

	/* Find the $VOLUME_INFORMATION attribute. */
	err = ntfs_attr_lookup(AT_VOLUME_INFORMATION, AT_UNNAMED, 0, 0, 0, NULL,
			0, ctx);
	if (err) {
		antfs_log_error("$VOLUME_INFORMATION attribute not found in "
				"$Volume");
		goto error_exit;
	}
	a = ctx->attr;
	/* Has to be resident. */
	if (a->non_resident) {
		antfs_log_error("Attribute $VOLUME_INFORMATION must be "
			       "resident but it isn't.");
		goto io_error_exit;
	}
	/* Get a pointer to the value of the attribute. */
	vinf = (struct VOLUME_INFORMATION *)(le16_to_cpu(a->value_offset) +
		(char *)a);
	/* Sanity checks. */
	if ((char *)vinf + le32_to_cpu(a->value_length) > (char *)ctx->mrec +
	le32_to_cpu(ctx->mrec->bytes_in_use) || le16_to_cpu(a->value_offset) +
	le32_to_cpu(a->value_length) > le32_to_cpu(a->length)) {
		antfs_log_error("$VOLUME_INFORMATION in $Volume is corrupt.");
		goto io_error_exit;
	}
	/* Setup vol from the volume information attribute value. */
	vol->major_ver = vinf->major_ver;
	vol->minor_ver = vinf->minor_ver;
	/* Do not use le16_to_cpu() macro here as our VOLUME_FLAGS are
	   defined using cpu_to_le16() macro and hence are consistent. */
	vol->flags = vinf->flags;
	/*
	 * Reinitialize the search context for the $Volume/$VOLUME_NAME lookup.
	 */
	ntfs_attr_reinit_search_ctx(ctx);
	err = ntfs_attr_lookup(AT_VOLUME_NAME, AT_UNNAMED, 0, 0, 0, NULL, 0,
			ctx);
	if (err) {
		if (err != -ENOENT) {
			antfs_log_error("Failed to lookup of $VOLUME_NAME in "
					"$Volume failed");
			goto error_exit;
		}
		/*
		 * Attribute not present.  This has been seen in the field.
		 * Treat this the same way as if the attribute was present but
		 * had zero length.
		 */
		vol->vol_name = ntfs_malloc(1);
		if (!vol->vol_name) {
			err = -ENOMEM;
			goto error_exit;
		}
		vol->vol_name[0] = '\0';
	} else {
		a = ctx->attr;
		/* Has to be resident. */
		if (a->non_resident) {
			antfs_log_error("$VOLUME_NAME must be resident.");
			goto io_error_exit;
		}
		/* Get a pointer to the value of the attribute. */
		vname = (ntfschar *)(le16_to_cpu(a->value_offset) + (char *)a);
		u = le32_to_cpu(a->value_length) / 2;
		/*
		 * Convert Unicode volume name to current locale multibyte
		 * format.
		 */
		vol->vol_name = NULL;
		if (ntfs_ucstombs(vname, u, &vol->vol_name, 0) < 0) {
			antfs_log_error("Volume name could not be converted "
					"to current locale");
			antfs_log_debug("Forcing name into ASCII by replacing "
				"non-ASCII characters with underscores.");
			vol->vol_name = ntfs_malloc(u + 1);
			if (!vol->vol_name) {
				err = -ENOMEM;
				goto error_exit;
			}

			for (j = 0; j < (s32)u; j++) {
				u16 uc = le16_to_cpu(vname[j]);
				if (uc > 0xff)
					uc = (u16)'_';
				vol->vol_name[j] = (char)uc;
			}
			vol->vol_name[u] = '\0';
		}
	}
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;
	/* Now load the attribute definitions from $AttrDef. */
	antfs_log_debug("Loading $AttrDef...");
	ni = ntfs_inode_open(vol, FILE_ATTRDEF, NULL);
	if (IS_ERR(ni)) {
		antfs_log_error("Failed to open $AttrDef");
		err = PTR_ERR(ni);
		goto error_exit;
	}
	/* Get an ntfs attribute for $AttrDef/$DATA. */
	na = ANTFS_NA(ni);
	/* Check we don't overflow 32-bits. */
	if (na->data_size > 0xffffffffLL) {
		antfs_log_error("Attribute definition table is too big (max "
			       "32-bit allowed).");
		err = -EINVAL;
		goto error_exit;
	}
	vol->attrdef_len = na->data_size;
	vol->attrdef = ntfs_malloc(na->data_size);
	if (!vol->attrdef) {
		err = -ENOMEM;
		goto error_exit;
	}
	/* Read in the $DATA attribute value into the buffer. */
	l = ntfs_attr_pread(na, 0, na->data_size, vol->attrdef);
	/* Done with the $AttrDef mft record. */
	ntfs_inode_close(ni);
	if (l != na->data_size) {
		antfs_log_error("Failed to read $AttrDef, unexpected length "
			       "(%lld != %lld).", (long long)l,
			       (long long)na->data_size);
		if (l >= 0)
			goto io_error_exit;
		err = l;
		goto error_exit;
	}
	/*
	 * Check for dirty logfile and hibernated Windows.
	 * We care only about read-write mounts.
	 */
	if (!(flags & (NTFS_MNT_RDONLY | NTFS_MNT_FORENSIC))) {
		if (!(flags & NTFS_MNT_IGNORE_HIBERFILE)) {
			err = ntfs_volume_check_hiberfile(vol, 1);
			if (err < 0) {
				if (flags & NTFS_MNT_MAY_RDONLY)
					need_fallback_ro = TRUE;
				else
					goto error_exit;
			}
		}
		err = ntfs_volume_check_logfile(vol);
		if (err < 0) {
			/* Always reject cached metadata for now */
			if (!(flags & NTFS_MNT_RECOVER) || (err == -EPERM)) {
				if (flags & NTFS_MNT_MAY_RDONLY)
					need_fallback_ro = TRUE;
				else
					goto error_exit;
			} else {
				antfs_log_info("The file system wasn't safely "
					       "closed on Windows. Fixing.");
				err = ntfs_logfile_reset(vol);
				if (err != 0)
					goto error_exit;
			}
		}
		/* make $TXF_DATA resident if present on the root directory */
		if (!(flags & NTFS_MNT_RDONLY) && !need_fallback_ro) {
			err = fix_txf_data(vol);
			if (err != 0)
				goto error_exit;
		}
	}

	if (need_fallback_ro) {
		NVolSetReadOnly(vol);
		antfs_log_error("Falling back to read-only mount because the "
				"NTFS partition is in an\nunsafe state. Please "
				"resume and shutdown Windows fully (no "
				"hibernation\nor fast restarting.)");
	}

	return vol;
io_error_exit:
	err = -EIO;
error_exit:
	antfs_log_error("Exit err=%d", err);
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	ntfs_free(m);
	ntfs_free(m2);
	__ntfs_volume_release(vol);
	return ERR_PTR(err);
}

/*
 *		Set appropriate flags for showing NTFS metafiles
 *	or files marked as hidden.
 *	Not set in ntfs_mount() to avoid breaking existing tools.
 */
int ntfs_set_shown_files(struct ntfs_volume *vol,
			bool show_sys_files, bool show_hid_files,
			bool hide_dot_files)
{
	int res;

	res = -1;
	if (vol) {
		NVolClearShowSysFiles(vol);
		NVolClearShowHidFiles(vol);
		NVolClearHideDotFiles(vol);
		if (show_sys_files)
			NVolSetShowSysFiles(vol);
		if (show_hid_files)
			NVolSetShowHidFiles(vol);
		if (hide_dot_files)
			NVolSetHideDotFiles(vol);
		res = 0;
	}
	if (res)
		antfs_log_error("Failed to set file visibility");
	return res;
}

/**
 * ntfs_mount - open ntfs volume
 * @name:	name of device/file to open
 * @flags:	optional mount flags
 *
 * This function mounts an ntfs volume. @name should contain the name of the
 * device/file to mount as the ntfs volume.
 *
 * @flags is an optional second parameter. The same flags are used as for
 * the mount system call (man 2 mount). Currently only the following flags
 * is implemented:
 *	NTFS_MNT_RDONLY	- mount volume read-only
 *
 * The function opens the device or file @name and verifies that it contains a
 * valid bootsector. Then, it allocates an ntfs_volume structure and initializes
 * some of the values inside the structure from the information stored in the
 * bootsector. It proceeds to load the necessary system files and completes
 * setting up the structure.
 *
 * Return the allocated volume structure on success and error code on error
 *
 * Note, that a copy is made of @name, and hence it can be discarded as
 * soon as the function returns.
 */
struct ntfs_volume *ntfs_mount(struct super_block *sb,
		enum ntfs_mount_flags flags __attribute__((unused)))
{
	struct ntfs_device *dev;
	struct antfs_sb_info *sbi = ANTFS_SB(sb);
	struct ntfs_volume *vol;

	/* Allocate an ntfs_device structure. */
	dev = ntfs_device_alloc(sbi->dev, 0, &ntfs_device_unix_io_ops, NULL);
	antfs_log_debug("dev:%p", (void *)dev);
	if (IS_ERR(dev)) {
		/* This copies the error pointer */
		vol = (struct ntfs_volume *)dev;
		goto out;
	}

	dev->d_sb = sb;
	/* Call ntfs_device_mount() to do the actual mount. */
	vol = ntfs_device_mount(dev, flags);
	antfs_log_debug("vol:%p", (void *)vol);
	if (IS_ERR(vol))
		ntfs_device_free(dev);
out:
	return vol;
}

/**
 * ntfs_umount - close ntfs volume
 * @vol: address of ntfs_volume structure of volume to close
 * @force: if true force close the volume even if it is busy
 *
 * Deallocate all structures (including @vol itself) associated with the ntfs
 * volume @vol.
 *
 * Return 0 on success. On error return the error code (most likely to one of
 * EAGAIN, EBUSY or EINVAL). The EAGAIN error means that an operation is in
 * progress and if you try the close later the operation might be completed and
 * the close succeed.
 *
 * If @force is true (i.e. not zero) this function will close the volume even
 * if this means that data might be lost.
 *
 * @vol must have previously been returned by a call to ntfs_mount().
 *
 * @vol itself is deallocated and should no longer be dereferenced after this
 * function returns success. If it returns an error then nothing has been done
 * so it is safe to continue using @vol.
 */
int ntfs_umount(struct ntfs_volume *vol,
		const bool force __attribute__((unused)))
{
	struct ntfs_device *dev;
	int ret;

	if (IS_ERR_OR_NULL(vol))
		return -EINVAL;
	dev = vol->dev;
	ret = __ntfs_volume_release(vol);
	ntfs_device_free(dev);
	return ret;
}

/**
 * ntfs_logfile_reset - "empty" $LogFile data attribute value
 * @vol:	ntfs volume whose $LogFile we intend to reset.
 *
 * Fill the value of the $LogFile data attribute, i.e. the contents of
 * the file, with 0xff's, thus marking the journal as empty.
 *
 * FIXME(?): We might need to zero the LSN field of every single mft
 * record as well. (But, first try without doing that and see what
 * happens, since chkdsk might pickup the pieces and do it for us...)
 *
 * On success return 0.
 *
 * On error return the error code.
 */
int ntfs_logfile_reset(struct ntfs_volume *vol)
{
	struct ntfs_inode *ni;
	int err = 0;

	if (!vol) {
		err = -EINVAL;
		goto out;
	}

	ni = ntfs_inode_open(vol, FILE_LOGFILE, NULL);
	if (IS_ERR(ni)) {
		antfs_log_error("Failed to open inode FILE_LogFile");
		err = PTR_ERR(ni);
		goto out;
	}

	err = ntfs_empty_logfile(ANTFS_NA(ni));
	ntfs_inode_close(ni);
out:
	return err;
}
