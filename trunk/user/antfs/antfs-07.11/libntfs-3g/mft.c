/**
 * mft.c - Mft record handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2004 Anton Altaparmakov
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2004-2008 Szabolcs Szakacsits
 * Copyright (c)      2005 Yura Pakhuchiy
 * Copyright (c) 2014-2015 Jean-Pierre Andre
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

#include <linux/slab.h>
#include <linux/time.h>

#include "types.h"
#include "device.h"
#include "debug.h"
#include "bitmap.h"
#include "attrib.h"
#include "inode.h"
#include "volume.h"
#include "layout.h"
#include "lcnalloc.h"
#include "mft.h"
#include "misc.h"

/**
 * ntfs_mft_records_read - read records from the mft from disk
 * @vol:	volume to read from
 * @mref:	starting mft record number to read
 * @count:	number of mft records to read
 * @b:		output data buffer
 *
 * Read @count mft records starting at @mref from volume @vol into buffer
 * @b. Return 0 on success or error code on error.
 *
 * If any of the records exceed the initialized size of the $MFT/$DATA
 * attribute, i.e. they cannot possibly be allocated mft records, assume this
 * is a bug and return error code ESPIPE.
 *
 * The read mft records are mst deprotected and are hence ready to use. The
 * caller should check each record with is_baad_record() in case mst
 * deprotection failed.
 *
 * NOTE: @b has to be at least of size @count * vol->mft_record_size.
 */
int ntfs_mft_records_read(const struct ntfs_volume *vol, const MFT_REF mref,
			  const s64 count, struct MFT_RECORD *b,
			  bool warn_ov)
{
	s64 br;
	VCN m;

	antfs_log_enter("inode %llu", (unsigned long long)MREF(mref));

	if (!vol || !vol->mft_na || !b || count < 0) {
		antfs_log_error
		    ("vol=%p vol->mft_ma=%p b=%p  count=%lld  mft=%llu",
		     vol, vol ? vol->mft_na : NULL, b,
		     (long long)count, (unsigned long long)MREF(mref));
		return -EINVAL;
	}
	m = MREF(mref);
	/* Refuse to read non-allocated mft records. */
	if (m + count > vol->mft_na->initialized_size >>
	    vol->mft_record_size_bits) {
		antfs_log_error("Trying to read non-allocated mft records "
				"(%lld > %lld)", (long long)m + count,
				(long long)vol->mft_na->initialized_size >>
				vol->mft_record_size_bits);
		return -ESPIPE;
	}
	br = ntfs_attr_mst_pread(vol->mft_na, m << vol->mft_record_size_bits,
				 count, vol->mft_record_size_bits, b, warn_ov);
	if (br != count) {
		int err;

		if (br >= 0 || br < -MAX_ERRNO)
			err = -EIO;
		else
			err = (int)br;
		antfs_log_error("Failed to read MFT, mft=%llu count=%lld "
				"br=%lld; warn_ov: %d", (long long)m,
				(long long)count,
				(long long)br, warn_ov);
		return err;
	}
	return 0;
}

/**
 * ntfs_mft_records_write - write mft records to disk
 * @vol:	volume to write to
 * @mref:	starting mft record number to write
 * @count:	number of mft records to write
 * @b:		data buffer containing the mft records to write
 *
 * Write @count mft records starting at @mref from data buffer @b to volume
 * @vol. Return 0 on success or error code on error.
 *
 * If any of the records exceed the initialized size of the $MFT/$DATA
 * attribute, i.e. they cannot possibly be allocated mft records, assume this
 * is a bug and return error code ESPIPE.
 *
 * Before the mft records are written, they are mst protected. After the write,
 * they are deprotected again, thus resulting in an increase in the update
 * sequence number inside the data buffer @b.
 *
 * If any mft records are written which are also represented in the mft mirror
 * $MFTMirr, we make a copy of the relevant parts of the data buffer @b into a
 * temporary buffer before we do the actual write. Then if at least one mft
 * record was successfully written, we write the appropriate mft records from
 * the copied buffer to the mft mirror, too.
 */
int ntfs_mft_records_write(const struct ntfs_volume *vol, const MFT_REF mref,
			   const s64 count, struct MFT_RECORD *b)
{
	s64 bw;
	VCN m;
	void *bmirr = NULL;
	int cnt = 0, res = 0;

	if (!vol || !vol->mft_na || vol->mftmirr_size <= 0 || !b || count < 0)
		return -EINVAL;
	m = MREF(mref);
	/* Refuse to write non-allocated mft records. */
	if (m + count > vol->mft_na->initialized_size >>
	    vol->mft_record_size_bits) {
		antfs_log_error("Trying to write non-allocated mft records "
				"(%lld > %lld)", (long long)m + count,
				(long long)vol->mft_na->initialized_size >>
				vol->mft_record_size_bits);
		return -ESPIPE;
	}
	if (m < vol->mftmirr_size) {
		if (!vol->mftmirr_na)
			return -EINVAL;
		cnt = vol->mftmirr_size - m;
		if (cnt > count)
			cnt = count;
		bmirr = ntfs_malloc(cnt * vol->mft_record_size);
		if (!bmirr)
			return -ENOMEM;
		memcpy(bmirr, b, cnt * vol->mft_record_size);
	}
	bw = ntfs_attr_mst_pwrite(vol->mft_na, m << vol->mft_record_size_bits,
				  count, vol->mft_record_size_bits, b);
	if (bw != count) {
		if (bw >= 0) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Partial write while writing $Mft "
					"record(s)");
			res = -EIO;
		} else {
			res = bw;
			antfs_log_error("Error writing $Mft record(s) (%d)",
					res);
		}
	}
	if (bmirr && bw > 0) {
		if (bw < cnt)
			cnt = bw;
		bw = ntfs_attr_mst_pwrite(vol->mftmirr_na,
					  m << vol->mft_record_size_bits, cnt,
					  vol->mft_record_size_bits, bmirr);
		if (bw != cnt) {
			if (bw >= 0) {
				res = -EIO;
			} else {
				res = bw;
				antfs_logger(vol->dev->d_sb->s_id,
					"Failed to sync $MFTMirr! Run "
					"chkdsk (%d)", res);
			}
		}
	}
	ntfs_free(bmirr);
	return res;
}

int ntfs_mft_record_check(const struct ntfs_volume *vol, const MFT_REF mref,
			  struct MFT_RECORD *m)
{
	struct ATTR_RECORD *a;
	int ret = -EIO;

	if (!ntfs_is_file_record(m->magic)) {
		if (!NVolNoFixupWarn(vol))
			antfs_log_error("Record %llu has no FILE magic (0x%x)",
			     (unsigned long long)MREF(mref),
			     (int)le32_to_cpu(*(le32 *) m));
		goto err_out;
	}

	if (le32_to_cpu(m->bytes_allocated) != vol->mft_record_size) {
		antfs_log_error("Record %llu has corrupt allocation size "
				"(%u <> %u)", (unsigned long long)MREF(mref),
				vol->mft_record_size,
				le32_to_cpu(m->bytes_allocated));
		goto err_out;
	}

	a = (struct ATTR_RECORD *) ((char *)m + le16_to_cpu(m->attrs_offset));
	if (p2n(a) < p2n(m) || (char *)a > (char *)m + vol->mft_record_size) {
		antfs_log_error("Record %llu is corrupt",
				(unsigned long long)MREF(mref));
		goto err_out;
	}

	ret = 0;
err_out:
	return ret;
}

/**
 * ntfs_file_record_read - read a FILE record from the mft from disk
 * @vol:	volume to read from
 * @mref:	mft reference specifying mft record to read
 * @mrec:	address of pointer in which to return the mft record
 * @attr:	address of pointer in which to return the first attribute
 *
 * Read a FILE record from the mft of @vol from the storage medium. @mref
 * specifies the mft record to read, including the sequence number, which can
 * be 0 if no sequence number checking is to be performed.
 *
 * The function allocates a buffer large enough to hold the mft record and
 * reads the record into the buffer (mst deprotecting it in the process).
 * *@mrec is then set to point to the buffer.
 *
 * If @attr is not NULL, *@attr is set to point to the first attribute in the
 * mft record, i.e. *@attr is a pointer into *@mrec.
 *
 * Return 0 on success, or the error code on failure.
 *
 * The read mft record is checked for having the magic FILE,
 * and for having a matching sequence number (if MSEQNO(*@mref) != 0).
 * If either of these fails, -EIO is returned. If you get
 * this, but you still want to read the mft record (e.g. in order to correct
 * it), use ntfs_mft_record_read() directly.
 *
 * Note: Caller has to free *@mrec when finished.
 *
 * Note: We do not check if the mft record is flagged in use. The caller can
 *	 check if desired.
 */
int ntfs_file_record_read(const struct ntfs_volume *vol, const MFT_REF mref,
			  struct MFT_RECORD **mrec, struct ATTR_RECORD **attr)
{
	struct MFT_RECORD *m;
	int err;

	if (!vol || !mrec) {
		antfs_log_error("mrec=%p", mrec);
		return -EINVAL;
	}

	m = *mrec;
	if (!m) {
		m = ntfs_malloc(vol->mft_record_size);
		if (!m)
			return -EINVAL;
	}
	err = ntfs_mft_record_read(vol, mref, m, true);
	if (err)
		goto err_out;

	err = ntfs_mft_record_check(vol, mref, m);
	if (err)
		goto err_out;

	if (MSEQNO(mref) && MSEQNO(mref) != le16_to_cpu(m->sequence_number)) {
		antfs_log_error("Record %llu has wrong SeqNo (%d <> %d)",
				(unsigned long long)MREF(mref), MSEQNO(mref),
				le16_to_cpu(m->sequence_number));
		err = -EIO;
		goto err_out;
	}
	*mrec = m;
	if (attr)
		*attr = (struct ATTR_RECORD *) ((char *)m +
					 le16_to_cpu(m->attrs_offset));
	return 0;
err_out:
	if (m != *mrec)
		ntfs_free(m);
	return err;
}

/**
 * ntfs_mft_record_layout - layout an mft record into a memory buffer
 * @vol:	volume to which the mft record will belong
 * @mref:	mft reference specifying the mft record number
 * @mrec:	destination buffer of size >= @vol->mft_record_size bytes
 *
 * Layout an empty, unused mft record with the mft reference @mref into the
 * buffer @m.  The volume @vol is needed because the mft record structure was
 * modified in NTFS 3.1 so we need to know which volume version this mft record
 * will be used on.
 *
 * On success return 0 and on error return the error code.
 */
int ntfs_mft_record_layout(const struct ntfs_volume *vol, const MFT_REF mref,
			   struct MFT_RECORD *mrec)
{
	struct ATTR_RECORD *a;
	int err = 0;

	if (!vol || !mrec) {
		err = -EINVAL;
		antfs_log_error("mrec=%p", mrec);
		goto out;
	}
	/* Aligned to 2-byte boundary. */
	if (vol->major_ver < 3 || (vol->major_ver == 3 && !vol->minor_ver))
		mrec->usa_ofs = cpu_to_le16((sizeof(struct MFT_RECORD_OLD) + 1)
					    & ~1);
	else {
		/* Abort if mref is > 32 bits. */
		if (MREF(mref) & 0x0000ffff00000000ull) {
			err = -ERANGE;
			antfs_log_error("Mft reference exceeds 32 bits");
			goto out;
		}
		mrec->usa_ofs = cpu_to_le16((sizeof(struct MFT_RECORD) + 1)
					    & ~1);
		/*
		 * Set the NTFS 3.1+ specific fields while we know that the
		 * volume version is 3.1+.
		 */
		mrec->reserved = const_cpu_to_le16(0);
		mrec->mft_record_number = cpu_to_le32(MREF(mref));
	}
	mrec->magic = magic_FILE;
	if (vol->mft_record_size >= NTFS_BLOCK_SIZE) {
		mrec->usa_count = cpu_to_le16(vol->mft_record_size /
					      NTFS_BLOCK_SIZE + 1);
	} else {
		mrec->usa_count = const_cpu_to_le16(1);
		antfs_log_error("Sector size is bigger than MFT record size.  "
				"Setting usa_count to 1.  If Windows chkdsk "
				"reports this as corruption, please email %s "
				"stating that you saw this message and that "
				"the file system created was corrupt.  "
				"Thank you.", NTFS_DEV_LIST);
	}
	/* Set the update sequence number to 1. */
	*(le16 *) ((u8 *) mrec + le16_to_cpu(mrec->usa_ofs)) =
	    const_cpu_to_le16(1);
	mrec->lsn = const_cpu_to_sle64(0ll);
	mrec->sequence_number = const_cpu_to_le16(1);
	mrec->link_count = const_cpu_to_le16(0);
	/* Aligned to 8-byte boundary. */
	mrec->attrs_offset = cpu_to_le16((le16_to_cpu(mrec->usa_ofs) +
					  (le16_to_cpu(mrec->usa_count) << 1) +
					  7) & ~7);
	mrec->flags = const_cpu_to_le16(0);
	/*
	 * Using attrs_offset plus eight bytes (for the termination attribute),
	 * aligned to 8-byte boundary.
	 */
	mrec->bytes_in_use = cpu_to_le32((le16_to_cpu(mrec->attrs_offset) + 8 +
					  7) & ~7);
	mrec->bytes_allocated = cpu_to_le32(vol->mft_record_size);
	mrec->base_mft_record = const_cpu_to_le64((MFT_REF) 0);
	mrec->next_attr_instance = const_cpu_to_le16(0);
	a = (struct ATTR_RECORD *) ((u8 *) mrec +
		le16_to_cpu(mrec->attrs_offset));
	a->type = AT_END;
	a->length = const_cpu_to_le32(0);
	/* Finally, clear the unused part of the mft record. */
	memset((u8 *) a + 8, 0,
	       vol->mft_record_size - ((u8 *) a + 8 - (u8 *) mrec));
out:
	return err;
}

/**
 * ntfs_mft_record_format - format an mft record on an ntfs volume
 * @vol:	volume on which to format the mft record
 * @mref:	mft reference specifying mft record to format
 *
 * Format the mft record with the mft reference @mref in $MFT/$DATA, i.e. lay
 * out an empty, unused mft record in memory and write it to the volume @vol.
 *
 * On success return 0 and on error return the error code.
 */
static int ntfs_mft_record_format(const struct ntfs_volume *vol,
		const MFT_REF mref)
{
	struct MFT_RECORD *m;
	int ret = 0;

	antfs_log_enter("mref %lld", (long long)MREF(mref));

	m = ntfs_calloc(vol->mft_record_size);
	if (!m) {
		ret = -ENOMEM;
		goto out;
	}

	ret = ntfs_mft_record_layout(vol, mref, m);
	if (ret)
		goto free_m;

	ret = ntfs_mft_record_write(vol, mref, m);
	if (ret)
		goto free_m;
free_m:
	kfree(m);
out:
	antfs_log_leave();
	return ret;
}

static const char *es = "  Leaving inconsistent metadata.  Run chkdsk.";

/**
 * ntfs_ffz - Find the first unset (zero) bit in a word
 * @word:
 *
 * Description...
 *
 * Returns:
 */
static inline unsigned int ntfs_ffz(unsigned int word)
{
	return ffs(~word) - 1;
}

static int ntfs_is_mft(struct ntfs_inode *ni)
{
	if (ni && ni->mft_no == FILE_MFT)
		return 1;
	return 0;
}

/**
 * @brief clears the bit from the bitmap that represents given mft_no
 *
 * @param pos	mft_no that should be cleared from the bitmask
 *
 * @return  returns 0 on success or negative error code otherwise
 *
 * ntfs_mft_bitmap_clear_bit is searching for the right position in the
 * mft bitmap for the given mft_no @pos. Then it sets that particular bit
 * to zero and marks the buffer_head dirty.
 * NOTE: we don't increment the free_mft_records field in @vol here, since
 * the caller has to handle that!
 */
static int ntfs_mft_bitmap_clear_bit(struct ntfs_volume *vol, s64 pos)
{
	/* - bh position we should have - */
	s64 curr_bh_pos = pos >> (3 + vol->dev->d_sb->s_blocksize_bits);
	/* - offset inside the current bh in bit - */
	s64 bh_ofs = pos & ((vol->dev->d_sb->s_blocksize << 3) - 1);
	u8 *buf;
	int err = 0;

	antfs_log_enter("mft_no(%lld)", pos);
	if (!vol->mftbmp_bh || curr_bh_pos != vol->mftbmp_start) {
		/* - wrong/none bufferhead! change it! - */
		if (vol->mftbmp_bh) {
			antfs_log_debug("wrong bufferhead (%lld) | (%lld)",
					vol->mftbmp_start, pos);
			brelse(vol->mftbmp_bh);
		}
		vol->mftbmp_bh = ntfs_load_bitmap_attr(vol,
						       vol->mftbmp_na, pos);
		if (IS_ERR_OR_NULL(vol->mftbmp_bh)) {
			err = PTR_ERR(vol->mftbmp_bh);
			if (err == 0)
				err = -EIO;
			antfs_log_error("Reading $MFT Bitmap @0x%llx/0x%llx "
					"failed: %d",
					(long long)pos,
					(long long)(vol->mftbmp_na->data_size <<
						3),
					err);
			vol->mftbmp_bh = NULL;
			goto leave;
		}
		antfs_log_debug
		    ("successfully switched buffer_head blocknr(%llu)",
		     (unsigned long long)vol->mftbmp_bh->b_blocknr);
		vol->mftbmp_start = curr_bh_pos;
		antfs_log_debug("bitmap_start(0x%hhx)",
				*(vol->mftbmp_bh->b_data));
	} else {
		/* - correct bufferhead, continue working! - */
		antfs_log_debug("correct bufferhead (%lld) | (%lld)",
				(long long)vol->mftbmp_start,
				(long long)pos);
	}
	buf = vol->mftbmp_bh->b_data;
	buf[bh_ofs >> 3] &= ~(1 << (bh_ofs & 7));
	mark_buffer_dirty(vol->mftbmp_bh);
leave:
	antfs_log_leave("err= %d", err);
	return err;
}

/**
 * ntfs_mft_bitmap_find_free_rec - find a free mft record in the mft bitmap
 * @vol:	volume on which to search for a free mft record
 * @base_ni:	open base inode if allocating an extent mft record or NULL
 *
 * Search for a free mft record in the mft bitmap attribute on the ntfs volume
 * @vol.
 *
 * If @base_ni is NULL start the search at the default allocator position.
 *
 * If @base_ni is not NULL start the search at the mft record after the base
 * mft record @base_ni.
 *
 * Return the free mft record on success and error code on error.
 * An error code of ENOSPC means that there are no free mft
 * records in the currently initialized mft bitmap.
 */
static s64 ntfs_mft_bitmap_find_free_rec(struct ntfs_volume *vol,
					 struct ntfs_inode *base_ni)
{
	s64 pass_end, ll, data_pos, pass_start;
	struct ntfs_attr *mftbmp_na;
	const unsigned char blocksize_bits =
	    vol->dev->d_sb->s_blocksize_bits + 3;
	const unsigned long block_mask = vol->dev->d_sb->s_blocksize - 1;
	u8 *buf;
	size_t size;
	u8 pass, b = 0;
	s64 curr_bh_pos;
	s64 ret = -EIO;

	antfs_log_enter();
	mftbmp_na = vol->mftbmp_na;
	/*
	 * Set the end of the pass making sure we do not overflow the mft
	 * bitmap.
	 */
	pass_end = vol->mft_na->allocated_size >> vol->mft_record_size_bits;
	ll = mftbmp_na->initialized_size << 3;
	if (pass_end > ll)
		pass_end = ll;
	pass = 1;
	if (!base_ni)
		data_pos = vol->mft_data_pos;
	else
		data_pos = base_ni->mft_no + 1;
	if (data_pos < RESERVED_MFT_RECORDS)
		data_pos = RESERVED_MFT_RECORDS;
	if (data_pos >= pass_end) {
		data_pos = RESERVED_MFT_RECORDS;
		pass = 2;
		/* This happens on a freshly formatted volume. */
		if (data_pos >= pass_end) {
			antfs_log_debug("data_pos (%lld) >= pass_end (%lld)",
					(long long)data_pos,
					(long long)pass_end);
			ret = -ENOSPC;
			goto leave;
		}
	}
	if (ntfs_is_mft(base_ni)) {
		/* FIXME:
		 * This looks like a nasty hack in ntfs-3g to "deus-ex-machina"
		 * some free mft records in the range from 16 to 24 if we need
		 * to extend $MFT itself.
		 * Otherwise allocating new clusters for the MFT or MFT bitmap
		 * is not allowed ... and just would not work the way the code
		 * is currently constructed.
		 *
		 * Same commit as the infamous "ntfs_mft_rec_init" stuff.
		 *
		 * @jpa: I srsly wonder what reasoning was behind all this
		 *       (if any).
		 */
		data_pos = 0;
		pass = 2;
	}
	pass_start = data_pos;

	/* Loop until a free mft record is found. */
	while (pass <= 2) {
		curr_bh_pos = data_pos >> blocksize_bits;

		/* we reached the end of allocated space */
		if (data_pos >= pass_end)
			goto out;

		if (!vol->mftbmp_bh || (curr_bh_pos != vol->mftbmp_start)) {
			/* - wrong/none bufferhead! change it! - */
			if (vol->mftbmp_bh) {
				antfs_log_debug
				    ("wrong bufferhead (%lld) | (%lld)",
				     vol->mftbmp_start, data_pos);
				brelse(vol->mftbmp_bh);
			}
			vol->mftbmp_bh = ntfs_load_bitmap_attr(vol,
							       vol->mftbmp_na,
							       data_pos);
			if (IS_ERR_OR_NULL(vol->mftbmp_bh)) {
				ret = PTR_ERR(vol->mftbmp_bh);
				if (ret == 0)
					ret = -EIO;
				antfs_log_error("Reading $MFT Bitmap "
						"@0x%llx/0x%llx failed: %d",
						(long long)data_pos,
						(long long)(vol->mftbmp_na->
							data_size << 3),
					(int)ret);
				vol->mftbmp_bh = NULL;
				goto leave;
			}
			vol->mftbmp_start = curr_bh_pos;
			antfs_log_debug("Read 0x%llx bytes.",
					(long long)vol->mftbmp_bh->b_size);
		} else {
			/* - correct bufferhead, continue working! - */
			antfs_log_debug("correct bufferhead (%lld) | (%lld)",
					vol->mftbmp_start, data_pos);
		}

		/* We read at least one byte here: Search @buf for a zero bit */
		buf = vol->mftbmp_bh->b_data + ((data_pos >> 3) & block_mask);

		/* Got bit fraction as start position? */
		b = data_pos & 7;
		if (b) {
			int i = *buf >> b;

			data_pos &= ~7ULL;
			/* XXX: Find first zero with start position hack.
			 * Ugly, but should do the job. */
			for (; b < 8; b++, i >>= 1)
				if (!(i & 1))
					goto found_mft;
			data_pos += 8;
			buf++;
			/* Go back to square one and check bitmap buffer again
			 * if we step into next buffer.
			 */
			if (!((data_pos >> 3) & block_mask))
				continue;
		}

		/* data_pos is byte aligned here.
		 * pass_end is byte aligned too. */
		for (size = vol->mftbmp_bh->b_size -
				((data_pos >> 3) & block_mask);
		     size && data_pos < pass_end;
		     data_pos += 8, buf++, size--) {
			/*
			 * If we're extending $MFT and running out of the first
			 * mft record (base record) then give up searching since
			 * no guarantee that the found record will be accessible
			 */
			if (ntfs_is_mft(base_ni)
			    && buf - (u8 *) vol->mftbmp_bh->b_data > 400 / 8)
				goto out;

			/*antfs_log_debug("bit(%lld) byte(%u)", bit, *buf);*/
			if (*buf == 0xff)
				continue;

			/* Note: ffz() result must be zero based. */
			b = ntfs_ffz((unsigned long)*buf);
			/* This will ALWAYS find something in range here. */
found_mft:
			ret = data_pos + b;
			*buf |= 1 << b;
			mark_buffer_dirty(vol->mftbmp_bh);
			antfs_log_debug("found free mft record!" "(%lld)", ret);

			goto leave;
		}
		/* Did not find clear mft bit in buffer here. */
		antfs_log_debug("After inner for loop: size 0x%zx, "
				"data_pos 0x%llx, bit 0x%llx, "
				"*byte 0x%hhx, b %d.",
				size, (long long)data_pos,
				(long long)(data_pos & ((block_mask << 3) | 7)),
				(u8)*buf, b);
		if (vol->mftbmp_bh->b_size != vol->dev->d_sb->s_blocksize) {
			antfs_log_warning
			    ("b_size (%u) != s_blocksize (%u); Skipping the "
			     "difference!",
			     (unsigned int)vol->mftbmp_bh->b_size,
			     (unsigned int)vol->dev->d_sb->s_blocksize);
			/* Need to advance for at least device block size or we
			 * end up searching the same area. */
			size = vol->dev->d_sb->s_blocksize << 3;
		}
		/*
		 * If the end of the pass has not been reached yet,
		 * continue searching the mft bitmap for a zero bit.
		 */
		if (data_pos < pass_end)
			continue;
		/* Do the next pass. */
		pass++;
		if (pass == 2) {
			/*
			 * Starting the second pass, in which we scan the first
			 * part of the zone which we omitted earlier.
			 */
			pass_end = pass_start;
			data_pos = pass_start = RESERVED_MFT_RECORDS;
			antfs_log_debug("pass %i, pass_start 0x%llx, pass_end "
					"0x%llx.", pass, (long long)pass_start,
					(long long)pass_end);
			if (data_pos >= pass_end)
				break;
		}
	}
	/* No free mft records in currently initialized mft bitmap. */
out:
	ret = -ENOSPC;
leave:
	antfs_log_leave("ret: %lld", (long long)ret);
	return ret;
}

static int ntfs_mft_attr_extend(struct ntfs_attr *na)
{
	int ret = STATUS_ERROR;
	antfs_log_enter();

	if (!NInoAttrList(na->ni)) {
		if (ntfs_inode_add_attrlist(na->ni)) {
			antfs_log_error("Can not add attrlist #3");
			goto out;
		}
		/* We can't sync the $MFT inode since its runlist is bogus. */
		ret = STATUS_KEEP_SEARCHING;
		goto out;
	}

	ret = ntfs_attr_update_mapping_pairs(na, 0);
	if (ret) {
		if (ret != -ENOSPC)
			antfs_log_error("MP update failed: %d", ret);
		goto out;
	}

	ret = STATUS_OK;
out:
	antfs_log_leave();
	return ret;
}

/**
 * ntfs_mft_bitmap_extend_allocation_i - see ntfs_mft_bitmap_extend_allocation
 */
static int ntfs_mft_bitmap_extend_allocation_i(struct ntfs_volume *vol)
{
	LCN lcn;
	s64 ll = 0;		/* silence compiler warning */
	struct ntfs_attr *mftbmp_na;
	struct runlist_element *rl, *rl2 = NULL;/* silence compiler warning */
	struct ntfs_attr_search_ctx *ctx;
	struct MFT_RECORD *m = NULL;	/* silence compiler warning */
	struct ATTR_RECORD *a = NULL;	/* silence compiler warning */
	int err, mp_size;
	u32 old_alen = 0;	/* silence compiler warning */
	bool mp_rebuilt = FALSE;
	bool update_mp = FALSE;

	mftbmp_na = vol->mftbmp_na;
	/*
	 * Determine the last lcn of the mft bitmap.  The allocated size of the
	 * mft bitmap cannot be zero so we are ok to do this.
	 */
	rl = ntfs_attr_find_vcn(mftbmp_na, (mftbmp_na->allocated_size - 1) >>
				vol->cluster_size_bits);
	if (IS_ERR(rl) || !rl->length || rl->lcn < 0) {
		antfs_log_error("Failed to determine last allocated "
				"cluster of mft bitmap attribute.");
		return IS_ERR(rl) ? PTR_ERR(rl) : -EIO;
	}
	lcn = rl->lcn + rl->length;

	rl2 = ntfs_cluster_alloc(vol, rl[1].vcn, 1, lcn, DATA_ZONE,
				 mftbmp_na->allocated_size);
	if (IS_ERR(rl2)) {
		antfs_log_error("Failed to allocate a cluster for "
				"the mft bitmap.");
		return PTR_ERR(rl2);
	}
	rl = ntfs_runlists_merge(mftbmp_na->rl, rl2);
	if (IS_ERR(rl)) {
		antfs_log_error("Failed to merge runlists for mft bitmap.");
		if (ntfs_cluster_free_from_rl(vol, rl2)) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Failed to deallocate cluster.%s", es);
		}
		ntfs_free(rl2);
		return PTR_ERR(rl);
	}
	mftbmp_na->rl = rl;
	mftbmp_na->allocated_size += vol->cluster_size;
	antfs_log_debug("Adding one run to mft bitmap.");
	/* Find the last run in the new runlist. */
	for (; rl[1].length; rl++)
		/*go through rl*/;
	/*
	 * Update the attribute record as well.  Note: @rl is the last
	 * (non-terminator) runlist element of mft bitmap.
	 */
	ctx = ntfs_attr_get_search_ctx(mftbmp_na->ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto undo_alloc;
	}

	err = ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			       mftbmp_na->name_len, 0, rl[1].vcn, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Failed to find last attribute extent of "
				"mft bitmap attribute.");
		goto undo_alloc;
	}
	m = ctx->mrec;
	a = ctx->attr;
	ll = sle64_to_cpu(a->lowest_vcn);
	rl2 = ntfs_attr_find_vcn(mftbmp_na, ll);
	if (IS_ERR(rl2) || !rl2->length) {
		antfs_log_error("Failed to determine previous last "
				"allocated cluster of mft bitmap attribute.");
		if (!IS_ERR(rl2)) {
			err = -EIO;
		} else {
			err = PTR_ERR(rl2);
			rl2 = NULL;
		}
		goto undo_alloc;
	}
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll, INT_MAX);
	if (mp_size <= 0) {
		antfs_log_error("Get size for mapping pairs failed for "
				"mft bitmap attribute extent.");
		err = mp_size;
		goto undo_alloc;
	}
	/* Expand the attribute record if necessary. */
	old_alen = le32_to_cpu(a->length);
	if (ntfs_attr_record_resize(m, a, mp_size +
				    le16_to_cpu(a->mapping_pairs_offset))) {
		antfs_log_info("extending $MFT bitmap");
		err = ntfs_mft_attr_extend(vol->mftbmp_na);
		if (err == STATUS_OK)
			goto ok;
		if (IS_STATUS_ERROR(err)) {
			antfs_log_error("ntfs_mft_attr_extend failed: %d", err);
			update_mp = TRUE;
		}
		goto undo_alloc;
	}
	mp_rebuilt = TRUE;
	/* Generate the mapping pairs array directly into the attr record. */
	if (ntfs_mapping_pairs_build(vol, (u8 *) a +
				     le16_to_cpu(a->mapping_pairs_offset),
				     mp_size, rl2, ll, NULL)) {
		antfs_log_error("Failed to build mapping pairs array for "
				"mft bitmap attribute.");
		err = -EIO;
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->highest_vcn = cpu_to_sle64(rl[1].vcn - 1);
	/*
	 * We now have extended the mft bitmap allocated_size by one cluster.
	 * Reflect this in the ntfs_attr structure and the attribute record.
	 */
	if (a->lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		err = ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
				       mftbmp_na->name_len, 0, 0, NULL, 0, ctx);
		if (err) {
			antfs_log_error("Failed to find first attribute "
					"extent of mft bitmap attribute.");
			goto restore_undo_alloc;
		}
		a = ctx->attr;
	}
ok:
	a->allocated_size = cpu_to_sle64(mftbmp_na->allocated_size);
	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	return STATUS_OK;

restore_undo_alloc:
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			     mftbmp_na->name_len, 0, rl[1].vcn, NULL, 0, ctx)) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Failed to find last attribute extent of "
				"mft bitmap attribute.%s", es);
		ntfs_attr_put_search_ctx(ctx);
		mftbmp_na->allocated_size += vol->cluster_size;
		/*
		 * The only thing that is now wrong is ->allocated_size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		return err;
	}
	m = ctx->mrec;
	a = ctx->attr;
	a->highest_vcn = cpu_to_sle64(rl[1].vcn - 2);
undo_alloc:
	/* Remove the last run from the runlist. */
	lcn = rl->lcn;
	rl->lcn = rl[1].lcn;
	rl->length = 0;
	mftbmp_na->allocated_size -= vol->cluster_size;

	if (ntfs_lcn_bitmap_clear_run(vol, lcn, 1)) {
		antfs_logger(vol->dev->d_sb->s_id,
					"Failed to free cluster.%s", es);
	}
	else
		vol->free_clusters++;
	if (mp_rebuilt) {
		if (ntfs_mapping_pairs_build(vol, (u8 *) a +
			le16_to_cpu(a->mapping_pairs_offset), old_alen -
			le16_to_cpu(a->mapping_pairs_offset), rl2, ll, NULL)) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Failed to restore maping pairs "
					"array.%s", es);
		}
		if (ntfs_attr_record_resize(m, a, old_alen)) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Failed to restore attribute "
					"record.%s", es);
		}
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
	}
	if (update_mp) {
		if (ntfs_attr_update_mapping_pairs(vol->mftbmp_na, 0))
			antfs_log_error("MP update failed");
	}
	if (!IS_ERR_OR_NULL(ctx))
		ntfs_attr_put_search_ctx(ctx);
	return err;
}

/**
 * ntfs_mft_bitmap_extend_allocation - extend mft bitmap attribute by a cluster
 * @vol:	volume on which to extend the mft bitmap attribute
 *
 * Extend the mft bitmap attribute on the ntfs volume @vol by one cluster.
 *
 * Note:  Only changes allocated_size, i.e. does not touch initialized_size or
 * data_size.
 *
 * Return 0 on success and on error the error code.
 */
static int ntfs_mft_bitmap_extend_allocation(struct ntfs_volume *vol)
{
	int ret;

	antfs_log_enter();
	ret = ntfs_mft_bitmap_extend_allocation_i(vol);
	antfs_log_leave();
	return ret;
}

/**
 * ntfs_mft_bitmap_extend_initialized - extend mft bitmap initialized data
 * @vol:	volume on which to extend the mft bitmap attribute
 *
 * Extend the initialized portion of the mft bitmap attribute on the ntfs
 * volume @vol by 8 bytes.
 *
 * Note:  Only changes initialized_size and data_size, i.e. requires that
 * allocated_size is big enough to fit the new initialized_size.
 *
 * Return 0 on success and the error code on error.
 */
static int ntfs_mft_bitmap_extend_initialized(struct ntfs_volume *vol)
{
	s64 old_data_size, old_initialized_size;
	s64 block_mask = vol->dev->d_sb->s_blocksize - 1;
	struct ntfs_attr *mftbmp_na;
	struct ntfs_attr_search_ctx *ctx;
	struct ATTR_RECORD *a;
	u8 *buf;
	int fill_s, buf_i, i;
	int err = 0;

	antfs_log_enter();

	mftbmp_na = vol->mftbmp_na;
	ctx = ntfs_attr_get_search_ctx(mftbmp_na->ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto out;
	}

	err = ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			       mftbmp_na->name_len, 0, 0, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Failed to find first attribute extent of "
				"mft bitmap attribute.");
		goto put_err_out;
	}
	a = ctx->attr;
	old_data_size = mftbmp_na->data_size;
	old_initialized_size = mftbmp_na->initialized_size;
	mftbmp_na->initialized_size += 8;
	a->initialized_size = cpu_to_sle64(mftbmp_na->initialized_size);
	if (mftbmp_na->initialized_size > mftbmp_na->data_size) {
		mftbmp_na->data_size = mftbmp_na->initialized_size;
		a->data_size = cpu_to_sle64(mftbmp_na->data_size);
	}
	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);

	buf_i = old_initialized_size & block_mask;

	for (i = 0; i < 8; i += fill_s) {
		s64 curr_bh;

		curr_bh = (old_initialized_size + i) >>
			vol->dev->d_sb->s_blocksize_bits;
		if (!vol->mftbmp_bh || (curr_bh != vol->mftbmp_start)) {
			if (vol->mftbmp_bh) {
				antfs_log_debug
				    ("wrong buffer_head (%lld) | (%lld)",
				     (long long)vol->mftbmp_start,
				     (long long)curr_bh);
				if (i)
					mark_buffer_dirty(vol->mftbmp_bh);
				brelse(vol->mftbmp_bh);
			}
			vol->mftbmp_bh = ntfs_load_bitmap_attr(vol,
					       vol->mftbmp_na,
					       (old_initialized_size + i) << 3);
			if (IS_ERR_OR_NULL(vol->mftbmp_bh)) {
				err = PTR_ERR(vol->mftbmp_bh);
				if (err == 0)
					err = -EIO;
				antfs_log_error("Reading $MFT Bitmap "
						"@0x%llx/0x%llx failed: %d",
						(long long)(
							(old_initialized_size +
							 i) << 3),
						(long long)(vol->mftbmp_na->
							data_size << 3),
					err);
				vol->mftbmp_bh = NULL;
				goto undo_init;
			}
			vol->mftbmp_start = curr_bh;
			buf_i = 0;
		}

		buf = vol->mftbmp_bh->b_data;
		fill_s = vol->mftbmp_bh->b_size - buf_i;
		if (fill_s > 8 - i)
			fill_s = 8 - i;
		memset(&buf[buf_i], 0, fill_s);
	}

put_err_out:
	ntfs_attr_put_search_ctx(ctx);
out:
	antfs_log_leave();
	return err;

undo_init:
	antfs_log_error("Failed to write to mft bitmap.");
	/* Try to recover from the error. */
	mftbmp_na->initialized_size = old_initialized_size;
	a->initialized_size = cpu_to_sle64(old_initialized_size);
	if (mftbmp_na->data_size != old_data_size) {
		mftbmp_na->data_size = old_data_size;
		a->data_size = cpu_to_sle64(old_data_size);
	}
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	antfs_log_debug("Restored status of mftbmp: allocated_size 0x%llx, "
			"data_size 0x%llx, initialized_size 0x%llx.",
			(long long)mftbmp_na->allocated_size,
			(long long)mftbmp_na->data_size,
			(long long)mftbmp_na->initialized_size);
	goto put_err_out;
}

/**
 * ntfs_mft_data_extend_allocation - extend mft data attribute
 * @vol:	volume on which to extend the mft data attribute
 *
 * Extend the mft data attribute on the ntfs volume @vol by 16 mft records
 * worth of clusters or if not enough space for this by one mft record worth
 * of clusters.
 *
 * Note:  Only changes allocated_size, i.e. does not touch initialized_size or
 * data_size.
 *
 * Return 0 on success and error code on error.
 */
static int ntfs_mft_data_extend_allocation(struct ntfs_volume *vol)
{
	LCN lcn;
	VCN old_last_vcn;
	s64 min_nr, nr, ll = 0;	/* silence compiler warning */
	struct ntfs_attr *mft_na;
	struct runlist_element *rl, *rl2;
	struct ntfs_attr_search_ctx *ctx;
	struct MFT_RECORD *m = NULL;	/* silence compiler warning */
	struct ATTR_RECORD *a = NULL;	/* silence compiler warning */
	int err, mp_size;
	u32 old_alen = 0;	/* silence compiler warning */
	bool mp_rebuilt = FALSE;
	bool update_mp = FALSE;

	antfs_log_enter("Extending mft data allocation.");

	mft_na = vol->mft_na;
	/*
	 * Determine the preferred allocation location, i.e. the last lcn of
	 * the mft data attribute.  The allocated size of the mft data
	 * attribute cannot be zero so we are ok to do this.
	 */
	rl = ntfs_attr_find_vcn(mft_na, (mft_na->allocated_size - 1) >>
				vol->cluster_size_bits);

	if (IS_ERR(rl) || !rl->length || rl->lcn < 0) {
		antfs_log_error("Failed to determine last allocated "
				"cluster of mft data attribute.");
		err = IS_ERR(rl) ? PTR_ERR(rl) : -EIO;
		goto out;
	}

	lcn = rl->lcn + rl->length;
	antfs_log_debug("Last lcn of mft data attribute is 0x%llx.",
			(long long)lcn);
	/* Minimum allocation is one mft record worth of clusters. */
	min_nr = vol->mft_record_size >> vol->cluster_size_bits;
	if (!min_nr)
		min_nr = 1;
	/* Want to allocate a certain number of mft records at once. */
	nr = vol->mft_record_size << MFT_DATA_BURST_ALLOC_SHIFT >>
		vol->cluster_size_bits;
	if (!nr)
		nr = min_nr;

	old_last_vcn = rl[1].vcn;
retry:
	rl2 = ntfs_cluster_alloc(vol, old_last_vcn, nr, lcn, MFT_ZONE, -1);
	if (IS_ERR(rl2)) {
		err = PTR_ERR(rl2);
		if (err != -ENOSPC || nr == min_nr) {
			antfs_log_error("Failed to allocate (%lld) clusters "
					"for $MFT", (long long)nr);
			goto out;
		}
init_retry:
		/*
		 * There is not enough space to do the allocation, but there
		 * might be enough space to do a minimal allocation so try that
		 * before failing.
		 */
		nr = min_nr;
		antfs_log_debug("Retrying mft data allocation with minimal "
				"cluster count %lli.", (long long)nr);
		goto retry;
	}

	antfs_log_debug("Allocated %lld clusters.", (long long)nr);

	rl = ntfs_runlists_merge(mft_na->rl, rl2);
	if (IS_ERR(rl)) {
		err = PTR_ERR(rl);
		antfs_log_error_ext("Failed to merge runlists for mft data "
				"attribute.");
		if (ntfs_cluster_free_from_rl(vol, rl2)) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Failed to deallocate clusters "
					"from the mft data attribute.%s", es);
		}
		ntfs_free(rl2);
		goto out;
	}
	mft_na->rl = rl;

	/* Find the last run in the new runlist. */
	for (; rl[1].length; rl++)
		/*go through rl*/;
	/* Update the attribute record as well. */
	ctx = ntfs_attr_get_search_ctx(mft_na->ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto undo_alloc;
	}

	err = ntfs_attr_lookup(mft_na->type, mft_na->name, mft_na->name_len, 0,
			       rl[1].vcn, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Failed to find last attribute extent of "
				"mft data attribute.");
		goto undo_alloc;
	}
	m = ctx->mrec;
	a = ctx->attr;
	ll = sle64_to_cpu(a->lowest_vcn);
	rl2 = ntfs_attr_find_vcn(mft_na, ll);
	if (IS_ERR(rl2) || !rl2->length) {
		antfs_log_error("Failed to determine previous last "
				"allocated cluster of mft data attribute");
		err = IS_ERR(rl2) ? PTR_ERR(rl2) : -EIO;
		goto undo_alloc;
	}
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll, INT_MAX);
	if (mp_size <= 0) {
		antfs_log_error("Get size for mapping pairs failed for "
				"mft data attribute extent");
		goto undo_alloc;
	}
	/* Expand the attribute record if necessary. */
	old_alen = le32_to_cpu(a->length);
	antfs_log_debug("Expand from 0x%x to 0x%x (mp_size=0x%x; "
			"mapping_pairs_offset=0x%x)", (int)old_alen,
			(int)(mp_size + le16_to_cpu(a->mapping_pairs_offset)),
			(int)mp_size, (int)le16_to_cpu(a->mapping_pairs_offset)
			);
	if (ntfs_attr_record_resize(m, a,
				    mp_size +
				    le16_to_cpu(a->mapping_pairs_offset))) {
		err = ntfs_mft_attr_extend(vol->mft_na);
		if (err == STATUS_OK)
			goto ok;
		if (IS_STATUS_ERROR(err)) {
			if (err != -ENOSPC)
				antfs_log_error("ntfs_mft_attr_extend failed: "
						"%d", err);
			update_mp = TRUE;
		}
		goto undo_alloc;
	}
	mp_rebuilt = TRUE;
	/* Leave some space free for mapping pairs. Windows/chkdsk don't like
	 * a completely cluttered file system.
	 */
	if (vol->mft_ni->nr_extents >= 12 &&
			(vol->mft_record_size - le32_to_cpu(m->bytes_in_use) <
			 512)) {
		err = -ENOSPC;
		goto undo_alloc;
	}
	/*
	 * Generate the mapping pairs array directly into the attribute record.
	 */
	if (ntfs_mapping_pairs_build(vol,
				     (u8 *) a +
				     le16_to_cpu(a->mapping_pairs_offset),
				     mp_size, rl2, ll, NULL)) {
		antfs_log_error("Failed to build mapping pairs array of "
				"mft data attribute.");
		err = -EIO;
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->highest_vcn = cpu_to_sle64(rl[1].vcn - 1);
	/*
	 * We now have extended the mft data allocated_size by nr clusters.
	 * Reflect this in the ntfs_attr structure and the attribute record.
	 * @rl is the last (non-terminator) runlist element of mft data
	 * attribute.
	 */
	if (a->lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		err = ntfs_attr_lookup(mft_na->type, mft_na->name,
				       mft_na->name_len, 0, 0, NULL, 0, ctx);
		if (err) {
			antfs_log_error("Failed to find first attribute "
					"extent of mft data attribute.");
			goto restore_undo_alloc;
		}
		a = ctx->attr;
	}
ok:
	mft_na->allocated_size += nr << vol->cluster_size_bits;
	a->allocated_size = cpu_to_sle64(mft_na->allocated_size);
	/* Ensure the changes make it to disk. $FILENAME has to be marked dirty
	 * later.
	 */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	err = STATUS_OK;
out:
	antfs_log_leave("err: %d", err);
	return err;

restore_undo_alloc:
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(mft_na->type, mft_na->name, mft_na->name_len, 0,
			     rl[1].vcn, NULL, 0, ctx)) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Failed to find last attribute extent of "
				"mft data attribute.%s", es);
		ntfs_attr_put_search_ctx(ctx);
		mft_na->allocated_size += nr << vol->cluster_size_bits;
		/*
		 * The only thing that is now wrong is ->allocated_size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		goto out;
	}
	m = ctx->mrec;
	a = ctx->attr;
	a->highest_vcn = cpu_to_sle64(old_last_vcn - 1);
undo_alloc:
	if (ntfs_cluster_free(vol, mft_na, old_last_vcn, -1) < 0) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Failed to freeclusters from mft data "
				"attribute.%s", es);
	}
	if (ntfs_rl_truncate(&mft_na->rl, old_last_vcn)) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Failed to truncate mft data attribute "
				"runlist.%s", es);
	}
	if (mp_rebuilt) {
		if (ntfs_mapping_pairs_build(vol, (u8 *) a +
			le16_to_cpu(a->mapping_pairs_offset), old_alen -
			le16_to_cpu(a->mapping_pairs_offset), rl2, ll, NULL)) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Failed to restore mapping pairs "
					"array.%s", es);
		}
		if (ntfs_attr_record_resize(m, a, old_alen)) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Failed to restore attribute "
					"record.%s", es);
		}
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		mp_rebuilt = false;
	}
	if (update_mp) {
		if (ntfs_attr_update_mapping_pairs(vol->mft_na, 0))
			antfs_log_error("MP update failed");
		update_mp = false;
	}
	if (!IS_ERR_OR_NULL(ctx))
		ntfs_attr_put_search_ctx(ctx);
	if ((!IS_STATUS_ERROR(err) || err == -ENOSPC) && nr != min_nr)
		goto init_retry;
	goto out;
}

static int ntfs_mft_record_init(struct ntfs_volume *vol, s64 size)
{
	int err;
	struct ntfs_attr *mft_na;
	s64 old_data_initialized, old_data_size;
	struct ntfs_attr_search_ctx *ctx;

	antfs_log_enter("size = %lld", (long long)size);

	/* NOTE: Caller must sanity check vol, vol->mft_na and vol->mftbmp_na */

	mft_na = vol->mft_na;

	/*
	 * The mft record is outside the initialized data. Extend the mft data
	 * attribute until it covers the allocated record. The loop is only
	 * actually traversed more than once when a freshly formatted volume
	 * is first written to so it optimizes away nicely in the common case.
	 */
	antfs_log_debug("Status of mft data before extension: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.",
			(long long)mft_na->allocated_size,
			(long long)mft_na->data_size,
			(long long)mft_na->initialized_size);
	while (size > mft_na->allocated_size) {
		err = ntfs_mft_data_extend_allocation(vol);
		if (err == STATUS_KEEP_SEARCHING)
			err = ntfs_mft_data_extend_allocation(vol);
		if (err < 0)
			goto out;
		antfs_log_debug
		    ("Status of mft data after allocation extension: "
		     "allocated_size 0x%llx, data_size 0x%llx, "
		     "initialized_size 0x%llx, size 0x%llx",
		     (long long)mft_na->allocated_size,
		     (long long)mft_na->data_size,
		     (long long)mft_na->initialized_size,
		     (long long)size);
	}

	old_data_initialized = mft_na->initialized_size;
	old_data_size = mft_na->data_size;

	/*
	 * Extend mft data initialized size (and data size of course) to reach
	 * the allocated mft record, formatting the mft records along the way.
	 * Note: We only modify the ntfs_attr structure as that is all that is
	 * needed by ntfs_mft_record_format().  We will update the attribute
	 * record itself in one fell swoop later on.
	 */
	while (size > mft_na->initialized_size) {
		s64 ll2 = mft_na->initialized_size >> vol->mft_record_size_bits;

		mft_na->initialized_size += vol->mft_record_size;
		if (mft_na->initialized_size > mft_na->data_size)
			mft_na->data_size = mft_na->initialized_size;
		antfs_log_debug("Initializing mft record 0x%llx.",
				(long long)ll2);
		err = ntfs_mft_record_format(vol, ll2);
		if (err < 0) {
			antfs_log_error("Failed to format mft record");
			goto undo_data_init;
		}
		antfs_log_debug("done!");
	}

	/* Update the mft data attribute record to reflect the new sizes. */
	ctx = ntfs_attr_get_search_ctx(mft_na->ni, NULL);
	if (IS_ERR(ctx)) {
		err = PTR_ERR(ctx);
		goto undo_data_init;
	}

	err = ntfs_attr_lookup(mft_na->type, mft_na->name, mft_na->name_len, 0,
			       0, NULL, 0, ctx);
	if (err) {
		antfs_log_error("Failed to find first attribute extent of "
				"mft data attribute.");
		ntfs_attr_put_search_ctx(ctx);
		goto undo_data_init;
	}
	ctx->attr->initialized_size = cpu_to_sle64(mft_na->initialized_size);
	ctx->attr->data_size = cpu_to_sle64(mft_na->data_size);
	ctx->attr->allocated_size = cpu_to_sle64(mft_na->allocated_size);

	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	/* Never sync $FILE_NAME here. This gives a deadlock if we work on root
	 * directory. Sync it later instead.
	 */
	NInoFileNameClearDirty(ctx->ntfs_ino);
	ntfs_inode_sync(ctx->ntfs_ino);
	NInoFileNameSetDirty(ctx->ntfs_ino);
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	antfs_log_debug("Status of mft data after mft record initialization: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.",
			(long long)mft_na->allocated_size,
			(long long)mft_na->data_size,
			(long long)mft_na->initialized_size);

	/* Sanity checks. */
	if (mft_na->data_size > mft_na->allocated_size ||
	    mft_na->initialized_size > mft_na->data_size)
		NTFS_BUG("mft_na sanity checks failed");

	err = 0;
out:
	antfs_log_leave("err: %d", err);
	return err;

undo_data_init:
	mft_na->initialized_size = old_data_initialized;
	mft_na->data_size = old_data_size;
	goto out;
}

/**
 * ntfs_mft_record_alloc - allocate an mft record on an ntfs volume
 * @vol:	volume on which to allocate the mft record
 * @base_ni:	open base inode if allocating an extent mft record or NULL
 *
 * Allocate an mft record in $MFT/$DATA of an open ntfs volume @vol.
 *
 * If @base_ni is NULL make the mft record a base mft record and allocate it at
 * the default allocator position.
 *
 * If @base_ni is not NULL make the allocated mft record an extent record,
 * allocate it starting at the mft record after the base mft record and attach
 * the allocated and opened ntfs inode to the base inode @base_ni.
 *
 * On success return the now opened ntfs (extent) inode of the mft record.
 *
 * On error return the error code.
 *
 * To find a free mft record, we scan the mft bitmap for a zero bit.  To
 * optimize this we start scanning at the place specified by @base_ni or if
 * @base_ni is NULL we start where we last stopped and we perform wrap around
 * when we reach the end.  Note, we do not try to allocate mft records below
 * number 24 because numbers 0 to 15 are the defined system files anyway and 16
 * to 24 are used for storing extension mft records or used by chkdsk to store
 * its log. However the record number 15 is dedicated to the first extent to
 * the $DATA attribute of $MFT.  This is required to avoid the possibility
 * of creating a run list with a circular dependence which once written to disk
 * can never be read in again.  Windows will only use records 16 to 24 for
 * normal files if the volume is completely out of space.  We never use them
 * which means that when the volume is really out of space we cannot create any
 * more files while Windows can still create up to 8 small files.  We can start
 * doing this at some later time, it does not matter much for now.
 *
 * When scanning the mft bitmap, we only search up to the last allocated mft
 * record.  If there are no free records left in the range 24 to number of
 * allocated mft records, then we extend the $MFT/$DATA attribute in order to
 * create free mft records.  We extend the allocated size of $MFT/$DATA by 16
 * records at a time or one cluster, if cluster size is above 16kiB.  If there
 * is not sufficient space to do this, we try to extend by a single mft record
 * or one cluster, if cluster size is above the mft record size, but we only do
 * this if there is enough free space, which we know from the values returned
 * by the failed cluster allocation function when we tried to do the first
 * allocation.
 *
 * No matter how many mft records we allocate, we initialize only the first
 * allocated mft record, incrementing mft data size and initialized size
 * accordingly, open an ntfs_inode for it and return it to the caller, unless
 * there are less than 24 mft records, in which case we allocate and initialize
 * mft records until we reach record 24 which we consider as the first free mft
 * record for use by normal files.
 *
 * If during any stage we overflow the initialized data in the mft bitmap, we
 * extend the initialized size (and data size) by 8 bytes, allocating another
 * cluster if required.  The bitmap data size has to be at least equal to the
 * number of mft records in the mft, but it can be bigger, in which case the
 * superfluous bits are padded with zeroes.
 *
 * Thus, when we return successfully (return value non-zero), we will have:
 *	- initialized / extended the mft bitmap if necessary,
 *	- initialized / extended the mft data if necessary,
 *	- set the bit corresponding to the mft record being allocated in the
 *	  mft bitmap,
 *	- open an ntfs_inode for the allocated mft record, and we will
 *	- return the ntfs_inode.
 *
 * On error (return value zero), nothing will have changed.  If we had changed
 * anything before the error occurred, we will have reverted back to the
 * starting state before returning to the caller.  Thus, except for bugs, we
 * should always leave the volume in a consistent state when returning from
 * this function.
 *
 * Note, this function cannot make use of most of the normal functions, like
 * for example for attribute resizing, etc, because when the run list overflows
 * the base mft record and an attribute list is used, it is very important that
 * the extension mft records used to store the $DATA attribute of $MFT can be
 * reached without having to read the information contained inside them, as
 * this would make it impossible to find them in the first place after the
 * volume is dismounted.  $MFT/$BITMAP probably does not need to follow this
 * rule because the bitmap is not essential for finding the mft records, but on
 * the other hand, handling the bitmap in this special way would make life
 * easier because otherwise there might be circular invocations of functions
 * when reading the bitmap but if we are careful, we should be able to avoid
 * all problems.
 */
struct ntfs_inode *ntfs_mft_record_alloc(struct ntfs_volume *vol,
					 struct ntfs_inode *base_ni)
{
	s64 ll, bit;
	struct ntfs_attr *mft_na, *mftbmp_na;
	struct MFT_RECORD *m = NULL;
	struct ntfs_inode *ni = NULL;
	struct inode *inode;
	unsigned char blocksize_bits = vol->dev->d_sb->s_blocksize_bits;
	unsigned long block_mask = vol->dev->d_sb->s_blocksize - 1;
	int ret = 0, tmp;
	u32 usa_ofs;
	le16 seq_no, usn;
	s64 curr_bh_pos;
	u8 *byte;

	if (base_ni) {
		antfs_log_enter("Entering (allocating an extent mft record for "
			       "base mft record %lld).",
			       (long long)base_ni->mft_no);
	} else {
		antfs_log_enter("Entering (allocating a base mft record)");
	}
	if (!vol || !vol->mft_na || !vol->mftbmp_na) {
		ret = -EINVAL;
		antfs_log_error("Invalid argument.");
		goto out;
	}

	mft_na = vol->mft_na;
	mftbmp_na = vol->mftbmp_na;

retry:
	ret = ntfs_mftbmp_lock(vol);
	if (ret)
		goto out;
	bit = ntfs_mft_bitmap_find_free_rec(vol, base_ni);
	if (bit >= 0) {
		antfs_log_debug("found free record (#1) at %lld",
				(long long)bit);
		goto took_free_rec;
	}
	/* FIXME: We don't extent $MFT itself here. If we tried, we'd currently
	 *        end up in recursion hell. Complete mess.
	 * TODO:  Extending $MFT/$Data is indeed special, but we should be able
	 *        to handle extending $MFT/$Bitmap here.
	 */
	if (bit != -ENOSPC || ntfs_is_mft(base_ni)) {
		ntfs_mftbmp_unlock(vol);
		ret = bit;
		goto out;
	}
	/*
	 * No free mft records left.  If the mft bitmap already covers more
	 * than the currently used mft records, the next records are all free,
	 * so we can simply allocate the first unused mft record.
	 * Note: We also have to make sure that the mft bitmap at least covers
	 * the first 24 mft records as they are special and whilst they may not
	 * be in use, we do not allocate from them.
	 */
	ll = mft_na->initialized_size >> vol->mft_record_size_bits;
	antfs_log_debug("No free mft records left! ll(%lld)", (long long)ll);
	if ((mftbmp_na->initialized_size << 3 > ll) &&
	    (mftbmp_na->initialized_size > RESERVED_MFT_RECORDS >> 3)) {
		bit = ll;
		if (bit < RESERVED_MFT_RECORDS)
			bit = RESERVED_MFT_RECORDS;
		antfs_log_debug("found free record (#2) at %lld",
				(long long)bit);
		goto found_free_rec;
	}
	/*
	 * The mft bitmap needs to be expanded until it covers the first unused
	 * mft record that we can allocate.
	 * Note: The smallest mft record we allocate is mft record 24.
	 */
	antfs_log_debug
	    ("Status of mftbmp before extension: allocated_size 0x%llx, "
	     "data_size 0x%llx, initialized_size 0x%llx.",
	     (long long)mftbmp_na->allocated_size,
	     (long long)mftbmp_na->data_size,
	     (long long)mftbmp_na->initialized_size);
	/* +8 because @ntfs_mft_bitmap_extend_initialized also works with
	 * 8 byte (=64 bits) steps.
	 * TODO: These hardcoded wild constants are crap. Clean this up!
	 */
	if (mftbmp_na->initialized_size + 8 > mftbmp_na->allocated_size) {

		int ret = ntfs_mft_bitmap_extend_allocation(vol);

		if (ret == STATUS_KEEP_SEARCHING) {
			ret = ntfs_mft_bitmap_extend_allocation(vol);
			if (ret != STATUS_OK) {
				ntfs_mftbmp_unlock(vol);
				goto err_out;
			}
		} else if (ret < 0) {
			ntfs_mftbmp_unlock(vol);
			goto err_out;
		}

		antfs_log_debug("Status of mftbmp after allocation extension: "
				"allocated_size 0x%llx, data_size 0x%llx, "
				"initialized_size 0x%llx.",
				(long long)mftbmp_na->allocated_size,
				(long long)mftbmp_na->data_size,
				(long long)mftbmp_na->initialized_size);
	}
	/*
	 * We now have sufficient allocated space, extend the initialized_size
	 * as well as the data_size if necessary and fill the new space with
	 * zeroes.
	 */
	bit = mftbmp_na->initialized_size << 3;
	ret = ntfs_mft_bitmap_extend_initialized(vol);
	if (ret) {
		ntfs_mftbmp_unlock(vol);
		goto err_out;
	}

	antfs_log_debug("Status of mftbmp after initialized extension: "
			"allocated_size %lld, data_size %lld, "
			"initialized_size %lld, bit: %lld",
			(long long)mftbmp_na->allocated_size,
			(long long)mftbmp_na->data_size,
			(long long)mftbmp_na->initialized_size, (long long)bit);
	antfs_log_debug("found free record (#3) at %lld", (long long)bit);
found_free_rec:
	/* @bit is the found free mft record, allocate it in the mft bitmap. */
	curr_bh_pos = (bit >> 3) >> blocksize_bits;
	if (curr_bh_pos != vol->mftbmp_start || !vol->mftbmp_bh) {
		if (vol->mftbmp_bh) {
			antfs_log_debug
			    ("wrong buffer_head (%lld) | (%lld)",
			     vol->mftbmp_start, bit);
			brelse(vol->mftbmp_bh);
		}
		vol->mftbmp_bh = ntfs_load_bitmap_attr(vol, vol->mftbmp_na,
				bit);
		if (IS_ERR_OR_NULL(vol->mftbmp_bh)) {
			ret = PTR_ERR(vol->mftbmp_bh);
			if (ret == 0)
				ret = -EIO;
			vol->mftbmp_bh = NULL;
			ntfs_mftbmp_unlock(vol);
			antfs_log_error("Reading $MFT Bitmap @0x%llx/0x%llx "
					"failed: %d",
					(long long)bit,
					(long long)(vol->mftbmp_na->data_size <<
						3),
					ret);
			goto err_out;
		}
		vol->mftbmp_start = curr_bh_pos;
	}

	byte = vol->mftbmp_bh->b_data;
	byte[(bit >> 3) & block_mask] |= 1 << (bit & 7);
	mark_buffer_dirty(vol->mftbmp_bh);

took_free_rec:
	/* The mft bitmap is now uptodate.  Deal with mft data attribute now. */
	ll = (bit + 1) << vol->mft_record_size_bits;
	antfs_log_debug("mft_record_init: ll: %lld; bit %lld",
			(long long)ll, (long long)bit);
	if (ll > mft_na->initialized_size)
		ret = ntfs_mft_record_init(vol, ll);
	ntfs_mftbmp_unlock(vol);
	if (ret < 0)
		goto undo_mftbmp_alloc;

	/* FIXME:
	 * Double check if this inode is REALLY free.
	 * Basically a workarround for weird problems with mft bitmap and
	 * inode numbers that get allocated multiple times.
	 *
	 * Assumed we have bitmap problems this is necessary as checking the
	 * MFT entry for the used flag is not always reliable: We don't write it
	 * back synchronously here.
	 */
	{
		struct inode *c_inode = ilookup(vol->sb, bit);

		if (c_inode) {
			antfs_logger(vol->dev->d_sb->s_id,
					"Inode 0x%llx has VFS inode but it "
					"wasn't marked in "
					"$MFT bitmap. Fixed, but this is a "
					"serious Problem!", (long long)bit);
#if KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE
			antfs_log_error("Colliding inode i_mode: 0x%lx"
					" i_count: %ld; "
					"i_nlink: %d; "
					"inode_unhashed: %d; "
					"i_state: 0x%x",
					(unsigned long)c_inode->i_mode,
					(long)atomic_read
					(&c_inode->i_count) - 1,
					(int)c_inode->i_nlink,
					inode_unhashed(c_inode),
					(int)c_inode->i_state);
#else
			antfs_log_error("Colliding inode i_mode: 0x%lx"
					" i_count: %ld; "
					"i_nlink: %d; "
					"i_state: 0x%x",
					(unsigned long)c_inode->i_mode,
					(long)atomic_read
					(&c_inode->i_count) - 1,
					(int)c_inode->i_nlink,
					(int)c_inode->i_state);
#endif
			iput(c_inode);
			goto retry;
		}
	}

	/*
	 * We now have allocated and initialized the mft record.  Need to read
	 * it from disk and re-format it, preserving the sequence number if it
	 * is not zero as well as the update sequence number if it is not zero
	 * or -1 (0xffff).
	 *
	 * FIXME: This is a load of extra work if we just initialized our mft
	 *        record a few lines above. ... when you're bored find a way
	 *        to optimize out this double initialization.
	 */
	m = ntfs_malloc(vol->mft_record_size);
	if (!m) {
		ret = -ENOMEM;
		goto undo_mftbmp_alloc;
	}

	/*
	 * As this is allocating a new record, do not expect it to have
	 * been initialized previously, so do not warn over bad fixups
	 * (hence avoid warn flooding when an NTFS partition has been wiped).
	 */
	ret = ntfs_mft_record_read(vol, bit, m, false);
	if (ret)
		goto undo_mftbmp_alloc;

	/* Sanity check that the mft record is really not in use. */
	ret = ntfs_is_file_record(m->magic);
	if (ret && (m->flags & MFT_RECORD_IN_USE)) {
		antfs_log_debug("Inode %lld is used but it wasn't marked in "
				"$MFT bitmap. Fixed.", (long long)bit);
		ntfs_free(m);
		goto retry;
	}
	seq_no = m->sequence_number;
	/*
	 * As ntfs_mft_record_read() returns what has been read
	 * even when the fixups have been found bad, we have to
	 * check where we fetch the initial usn from.
	 * Small sanity check to see if usa_ofs makes sense and
	 * we may expect our usn there -- use 1 as fallback.
	 */
	usa_ofs = le16_to_cpu(m->usa_ofs);
	if (!(usa_ofs & 1) && (usa_ofs < NTFS_BLOCK_SIZE))
		usn = *(le16 *) ((u8 *) m + usa_ofs);
	else
		usn = const_cpu_to_le16(1);

	antfs_log_debug("ntfs_mft_record_layout!");
	ret = ntfs_mft_record_layout(vol, bit, m);
	if (ret) {
		antfs_log_error("Failed to re-format mft record.");
		goto undo_mftbmp_alloc;
	}

	if (seq_no)
		m->sequence_number = seq_no;

	seq_no = usn;
	if (seq_no && seq_no != const_cpu_to_le16(0xffff))
		*(le16 *) ((u8 *) m + le16_to_cpu(m->usa_ofs)) = usn;

	/* Set the mft record itself in use. */
	m->flags |= MFT_RECORD_IN_USE;

	/* Need to trigger writeback NOW. Update buffers and mark them dirty. */
	ret = ntfs_mft_record_write(vol, bit, m);
	if (ret)
		antfs_log_error("Failed to write back mrec, ino 0x%llx",
				(long long)bit);

	/* Now need to open an ntfs inode for the mft record. */
	ni = ntfs_inode_allocate(vol);
	if (IS_ERR(ni)) {
		antfs_log_error("Failed to allocate buffer for inode.");
		ret = PTR_ERR(ni);
		goto undo_mftbmp_alloc;
	}
	ni->mft_no = bit;
	ni->mrec = m;

	/*
	 * If we are allocating an extent mft record, make the opened inode an
	 * extent inode and attach it to the base inode.  Also, set the base
	 * mft record reference in the extent inode.
	 */
	if (base_ni) {
		ni->nr_extents = -1;
		ni->base_ni = base_ni;
		m->base_mft_record = MK_LE_MREF(base_ni->mft_no,
				le16_to_cpu(base_ni->mrec->sequence_number));

		inode = ANTFS_I(ni);
		ret = antfs_inode_init(&inode, ANTFS_INODE_INIT_DELETE);
		if (ret) {
		/* Most of the time this means insert_inode_locked failed.
		 * If this happens, we screwed up:
		 * We checked that this record is NOT in use above so we have
		 * some weird leftover inode in VFS that just must not be there.
		 */
			antfs_logger(ANTFS_I(ni)->i_sb->s_id,
					"Failed to initialize extent inode"
					"(%lld): %d for base_ni(%lld)",
					ni->mft_no, (int)ret,
					(long long)base_ni->mft_no);
			goto err_out;
		}
		/*
		 * Attach the extent inode to the base inode, reallocating
		 * memory if needed.
		 */
		if (!(base_ni->nr_extents & 3)) {
			struct ntfs_inode **extent_nis;
			int i;

			i = (base_ni->nr_extents +
			     4) * sizeof(struct ntfs_inode *);
			extent_nis = ntfs_malloc(i);
			if (!extent_nis) {
				ret = -ENOMEM;
				goto undo_extent_init;
			}
			if (base_ni->nr_extents) {
				memcpy(extent_nis, base_ni->extent_nis,
				       i - 4 * sizeof(struct ntfs_inode *));
				ntfs_free(base_ni->extent_nis);
			}
			base_ni->extent_nis = extent_nis;
		}
		base_ni->extent_nis[base_ni->nr_extents++] = ni;
		ret = ntfs_mftbmp_lock(vol);
		if (ret)
			goto out;
	} else {
		ret = ntfs_mftbmp_lock(vol);
		if (ret)
			goto out;
		vol->mft_data_pos = bit + 1;
	}

	vol->free_mft_records--;
	ntfs_mftbmp_unlock(vol);

	/* Initialize time, allocated and data size in ntfs_inode struct. */
	ni->flags = const_cpu_to_le32(0);
	ni->creation_time = ni->last_data_change_time =
	    ni->last_mft_change_time =
	    ni->last_access_time = ntfs_current_time();

	/* Make sure the allocated inode is written out to disk later. */
	ntfs_inode_mark_dirty(ni);

	/* Update the default mft allocation position if it was used. */
	/* Return the opened, allocated inode of the allocated mft record. */
	antfs_log_debug("allocated %sinode 0x%llx.",
			base_ni ? "extent " : "", (long long)bit);

out:
	if (ret)
		ni = ERR_PTR(ret);

	antfs_log_leave("err: %d", ret);
	return ni;

undo_extent_init:
	ntfs_inode_close(ni);
	goto err_out;
undo_mftbmp_alloc:
	ntfs_free(m);
	tmp = ntfs_mftbmp_lock(vol);
	if (tmp) {
		antfs_logger(vol->dev->d_sb->s_id,
				"mftbmp_lock failed. free_mft_records now "
				"garbage");

		ret = -ERESTARTSYS;
		goto err_out;
	}
	if (ntfs_mft_bitmap_clear_bit(vol, bit)) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Failed to clear bit in mft bitmap");
	}
	ntfs_mftbmp_unlock(vol);
err_out:
	if (!ret)
		ret = -EIO;
	goto out;
}

/**
 * ntfs_mft_record_free - free an mft record on an ntfs volume
 * @ni:		open ntfs inode of the mft record to free
 *
 * Free the mft record of the open inode @ni.
 * Note that this function calls ntfs_inode_close() internally and hence you
 * cannot use the pointer @ni any more after this function returns success.
 *
 * On success return 0 and on error return the error code.
 */
int ntfs_mft_record_free(struct ntfs_inode *ni)
{
	u64 mft_no;
	int err;
	u16 seq_no;
	le16 old_seq_no;
	struct ntfs_volume *vol;

	antfs_log_enter("inode 0x%llx.", (long long)ni->mft_no);

	if (!ni || !ni->vol || !ni->vol->mftbmp_na) {
		antfs_log_error("Wrong arguments");
		return -EINVAL;
	}

	if (NInoCollided(ni)) {
		antfs_log_leave("Not freeing collided mrec.");
		return 0;
	}

	vol = ni->vol;

	/* Cache the mft reference for later. */
	mft_no = ni->mft_no;

	/* Mark the mft record as not in use. */
	ni->mrec->flags &= ~MFT_RECORD_IN_USE;

	/* Increment the sequence number, skipping zero, if it is not zero. */
	old_seq_no = ni->mrec->sequence_number;
	seq_no = le16_to_cpu(old_seq_no);
	if (seq_no == 0xffff)
		seq_no = 1;
	else if (seq_no)
		seq_no++;
	ni->mrec->sequence_number = cpu_to_le16(seq_no);

	err = ntfs_mft_record_write(vol, ni->mft_no, ni->mrec);
	if (err) {
		if (err != -EIO)
			err = -EBUSY;
		NInoSetDirty(ni);
		antfs_log_error("MFT record sync failed, inode %lld",
				(long long)ni->mft_no);
		goto sync_rollback;
	}
	/* Clear the bit in the $MFT/$BITMAP corresponding to this record. */
	if (ntfs_mftbmp_lock(vol))
		goto sync_rollback;
	err = ntfs_mft_bitmap_clear_bit(vol, mft_no);
	if (err) {
		ntfs_mftbmp_unlock(vol);
		goto sync_rollback;
	}

	vol->free_mft_records++;
	ntfs_mftbmp_unlock(vol);

	/* Throw away the now freed inode. */
	/* This thing is gone shortly anyway  - don't sync. */
	NInoClearDirty(ni);

	/* Don't close ni here: This is done in inode destroy. */
	antfs_log_leave();
	return 0;

	/* Rollback what we did... */
sync_rollback:
	antfs_log_error("rollback!");
	ni->mrec->flags |= MFT_RECORD_IN_USE;
	ni->mrec->sequence_number = old_seq_no;
	NInoFileNameClearDirty(ni); /* Never sync $FILE_NAME here. */
	NInoSetDirty(ni);
	ntfs_inode_sync(ni);
	antfs_log_leave("rolled back!");
	return err;
}
