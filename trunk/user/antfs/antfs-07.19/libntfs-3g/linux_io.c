/**
 * linux_kernel_io.c - Disk io functions for use in linux kernel.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2006 Anton Altaparmakov
 * Copyright (c) 2017 AVM GmbH
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

#include "types.h"
#include "mst.h"
#include "debug.h"
#include "device.h"
#include "misc.h"

#define ANTFS_READAHEAD_SIZE		(16 * PAGE_SIZE)

struct linux_io_dev_priv {
	loff_t maxbytes;
	loff_t pos;

	struct list_head lru_list;
};

/**
 * @brief fsync replacement - the linux kernel way
 *
 * Does a sync on the underlying block device
 */
static int ntfs_fsync(struct ntfs_device *dev)
{
	struct super_block *sb = dev->d_sb;

	if (!sb)
		return -ENODEV;

	return sync_blockdev(sb->s_bdev);
}

/**
 * @brief Open a device and lock it exclusively
 *
 * This does not really open anything since the main work is already done by
 * @ref mount_bdev at this point. So we play arround with the flags in *dev a
 * bit and set d_private field to point to the super block struct.
 *
 * @param dev Pointer to ntfs_device to open
 * @param flags Flags for opening - Don't do much here
 *
 * @return Result
 * @retval 0 On success
 * @retval -1 if dev already open/oom
 */
static int ntfs_device_linux_io_open(struct ntfs_device *dev, int flags)
{
	struct linux_io_dev_priv *priv;
	struct block_device *bdev;
	int err;

	/* Should never trigger... */
	if (NDevOpen(dev))
		return -EBUSY;

	/* Only support block devs here + locking should already be done by
	 * mount */
	NDevSetBlock(dev);

	/* TODO: Is this used later? */
	if ((flags & O_RDWR) != O_RDWR)
		NDevSetReadOnly(dev);

	/* TODO: ANTFS_SB_GET needs to change when
	 *	    no extra thread is created! */
	priv = kzalloc(sizeof(struct linux_io_dev_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	bdev = dev->d_sb->s_bdev;

	/* Should be set at this point if bdev was opened by mount_bdev */
	priv->pos = 0;
	priv->maxbytes = i_size_read(bdev->bd_inode);
	if (priv->maxbytes)
		priv->maxbytes--;
	else {
		antfs_log_error("Size of bdev %s is 0??", dev->d_name);
		err = -ENODEV;
		goto err_no_dev;
	}

	INIT_LIST_HEAD(&priv->lru_list);

	dev->d_private = priv;
	NDevSetOpen(dev);

	return 0;

err_no_dev:
	kfree(priv);
	dev->d_private = NULL;
	return err;
}

/**
 * @brief Close the device, releasing the lock
 *
 * This does not really close anything.
 * Clear open bit in *dev and d_private, then return.
 *
 * @param dev Pointer to ntfs_device to open
 *
 * @return Result
 * @retval 0 On success
 * @retval -1 if dev already closed or sync failed
 */
static int ntfs_device_linux_io_close(struct ntfs_device *dev)
{
	int err = 0;

	if (!NDevOpen(dev)) {
		antfs_log_error("Device %s is not open", dev->d_name);
		return -EBADF;
	}

	if (NDevDirty(dev))
		err = ntfs_fsync(dev);
	if (err) {
		antfs_log_error("Failed to synchronize device %s", dev->d_name);
		goto err;
	}

	/* Nothing is done with locks here */
	NDevClearOpen(dev);
	kfree(dev->d_private);
	dev->d_private = NULL;

err:
	return err;
}

/**
 * ntfs_device_unix_io_seek - Seek to a place on the device
 * @dev:
 * @offset:
 * @whence:
 *
 * Description...
 *
 * Returns:
 */
static s64 ntfs_device_linux_io_seek(struct ntfs_device *dev, s64 offset,
				     int whence)
{
	struct linux_io_dev_priv *priv = dev->d_private;
	loff_t npos;

	switch (whence) {
	case SEEK_SET:
		if ((offset < 0) || (offset > priv->maxbytes)) {
			antfs_log_debug
			    ("SEEK_SET offset check: %lld; maxbytes: %lld",
			     offset, priv->maxbytes);
			goto err;
		}

		priv->pos = offset;
		break;
	case SEEK_CUR:
		npos = priv->pos + offset;
		if ((npos < 0) || (npos > priv->maxbytes))
			goto err;

		priv->pos = npos;
		break;
	case SEEK_END:
		if (offset > 0)
			goto err;

		npos = priv->maxbytes + offset;
		if (npos < 0)
			goto err;
		priv->pos = npos;
		break;
	default:
		goto err;
	}

	return priv->pos;
err:
	/* TODO: Is this really ok? */
	return -EINVAL;
}

/**
 * @brief maps (part of) an attribute bitmap into a buffer_head
 *
 * @param vol       ntfs volume to na is on
 * @param na        attribute containing the bitmap
 * @param bit       bit offset in attribute
 *
 * @return          buffer_head mapped to part of bitmap containing requested
 *                  bit
 *
 *
 * ntfs_load_bitmap_attr is reading the requested bitmap attribute (or the part
 * of it, that contains @offset) and maps it to a buffer_head which will be
 * used to manipulate the bitmap during the lifespan of the ntfs driver.
 * NOTE: there are two bitmaps we handle, the lcn bitmap (lcnbmp_na) and the mft
 * bitmap (mftbmp_na).
 */
struct buffer_head *ntfs_load_bitmap_attr(struct ntfs_volume *vol,
					  struct ntfs_attr *na, u64 bit)
{
	struct runlist_element *rl;
	LCN cn;
	const int dbits =
	    vol->cluster_size_bits - vol->dev->d_sb->s_blocksize_bits;
	sector_t block = bit >> (vol->dev->d_sb->s_blocksize_bits + 3);
	int ret;

	/* Shift the diff to clusters. */
	cn = block >> dbits;
	/* Offset of blocks into cluster. */
	block &= (1ULL << dbits) - 1ULL;
	antfs_log_enter("bit(%llu) cn(%llu) nr_clusters(%lld)",
			(unsigned long long)bit, (unsigned long long)cn,
			(long long)vol->nr_clusters);
	ret = ntfs_attr_map_whole_runlist(na);	/* XXX: Needed? */
	if (ret)
		return ERR_PTR(ret);

	rl = na->rl;
	/* XXX: This doesn't take the attribute size into account! */
	/* Get LCN from runlist */
	for (; cn >= rl->length; cn -= (rl++)->length) {
		antfs_log_debug("cn: %llu; lcn=%lld; len=%lld",
				(unsigned long long)cn, (long long)rl->lcn,
				(long long)rl->length);
		if (!rl->length) {
			antfs_log_error("Bit out of range");
			return ERR_PTR(-EINVAL);
		}
	}
	if (rl->lcn < 0) {
		antfs_log_error("Unmapped/sparse cluster (%lld)!?", rl->lcn);
		return ERR_PTR(-EIO);
	}

	antfs_log_leave("reading @ lcn %lld + %lld + block %lld = block %lld; "
	     "cluster_size %d [bits]; block_size %d [bits]",
	     (long long)rl->lcn, (long long)cn, (long long)block,
	     (long long)(((rl->lcn + cn) << dbits) + block),
	     (int)vol->cluster_size_bits,
	     (int)vol->dev->d_sb->s_blocksize_bits);
	return sb_bread(vol->dev->d_sb, ((rl->lcn + cn) << dbits) + block);
}

/**
 * ntfs_device_unix_io_pread - Perform a positioned read from the device
 * @dev:
 * @buf:
 * @count:
 * @offset:
 *
 * Description...
 *
 * Returns:
 */
static inline void antfs_readahead(struct super_block *sb, sector_t index,
				   unsigned int reada_blks)
{
	unsigned int i;

	for (i = 0; i < reada_blks; i++)
		sb_breadahead(sb, index + i);
}

/*TODO: this needs some love. it works, but there is a lot of room for
 *      improvements. this gets called if we read compressed files. In this
 *      case we are still really slow compared to other drivers! */
static s64 ntfs_device_linux_io_pread(struct ntfs_device *dev, void *buf,
				      s64 count, s64 offset)
{
	struct linux_io_dev_priv *priv = dev->d_private;
	struct super_block *sb = dev->d_sb;
	struct buffer_head *bh = NULL;
	unsigned long blks_reada, blks_left, blks_reada_half;
	unsigned long blk_curr;
	sector_t index, last_index, reada_index;
	char *dest_addr = (char *)buf;
	s64 data_addr = offset;
	s64 data_size = count;
	s64 data_offset;
	s64 blk_addr = 0;
	int blk_bits = sb->s_blocksize_bits;
	int blk_size = sb->s_blocksize;
	size_t chunk = 0;
	int err = 0;

	if (!count) {
		err = -EINVAL;
		goto err;
	}

	index = data_addr >> blk_bits;
	last_index = ((data_addr + data_size - 1) >> blk_bits) + 1;

	blks_reada = ANTFS_READAHEAD_SIZE >> blk_bits;
	blks_reada_half = blks_reada >> 1;
	blk_curr = 0;

	for (; index < last_index; index++, blk_curr++) {
		if (!blk_curr) {
			blks_left = last_index - index;
			antfs_readahead(sb, index, min(blks_reada, blks_left));
		}

		if ((blk_curr & (blks_reada - 1)) == blks_reada_half) {
			reada_index = index + blks_reada_half;
			blks_left = last_index - reada_index;
			antfs_readahead(sb, reada_index,
					min(blks_reada, blks_left));
		}

		bh = sb_bread(sb, index);
		if (unlikely(!bh)) {
			antfs_log_error("Couldn't aquire buffer for "
					"block %llu",
					(unsigned long long)index);
			err = -ENOMEM;
			goto err;
		}

		blk_addr = index << blk_bits;
		data_offset = data_addr - blk_addr;
		chunk = (size_t) min((s64) blk_size - data_offset, data_size);

		memcpy(dest_addr, bh->b_data + data_offset, chunk);

		dest_addr += chunk;
		data_addr += chunk;
		data_size -= chunk;

		priv->pos = data_addr;

		brelse(bh);
	}

	return count;
err:
	return err;
	/*--- return pread(dev, buf, count, offset); ---*/
}

/**
 * @brief: Perform a positioned write to the device
 *
 * @dev:    ntfs device to write to
 * @buf:    buffer containing content to write
 * @count:  number of bytes to write
 * @offset: location on the device to write to
 *
 * @return amount of bytes written, or 0
 *
 * ntfs_device_unix_io_pwrite writes the in @count specified amount of bytes to
 * the ntfs device specified by @dev. @offset is giving the position where on
 * the device the input contained in @buf should be written to.
 * In order to get the right buffer head for that position we first need to
 * calculate the index of the block. __getblk() will give us the right buffer
 * head already read and filled with the content of that specific block we want
 * to write to.
 * Since @offset contains the position considering the whole device but we only
 * have the block in which @offset is pointing anywhere into, we have to adjust
 * that offset to an offset considering only the page we are into. For that we
 * convert the index into an address for the block we have, and subtract this
 * block address from @offset to get a new offset for the page we can write to.
 * Using memcpy() we manipulate the page the way it should look like after the
 * write would be performed, lock the buffer, set it as to be written, set the
 * page as being written back and finally submit the buffer head.
 * From that point on Linux will take over everything and we should not have to
 * worry about anything anymore.
 * The last step is to put down the buffer head and return the amount of bytes
 * that were transferred, or 0 if something went wrong or we didn't have to copy
 * anything because @count was set to 0.
 */
static s64 ntfs_device_unix_io_pwrite(struct ntfs_device *dev, const void *buf,
				      s64 count, s64 offset)
{
	struct super_block *sb = dev->d_sb;
	struct buffer_head *bh = NULL;
	sector_t index = offset >> sb->s_blocksize_bits;
	size_t chunk, data_offset;
	int res = 0;

	if (!count)
		goto out;

	if (NDevReadOnly(dev)) {
		antfs_log_error("This is a read-only device!");
		goto out;
	}

	/* - calculate addresses needed for bh - */
	data_offset = offset & (sb->s_blocksize - 1);
	chunk = (size_t) min((s64)(sb->s_blocksize - data_offset), count);

	/*TODO: what is this for?
	 *NDevSetDirty(dev); */
	antfs_log_enter("writing to block(%llu), offs(%lld), count(%lld)",
			(unsigned long long)index,
			(long long)offset,
			(long long)count);
	bh = sb_bread(sb, index);
	if (!bh) {
		antfs_log_error("Couldn't fetch buffer_head!");
		res = -EIO;
		goto out;
	}
	/* - lock the buffer here to prevent memory corruption - */
	/* - prepare bh->b_data to be written back to device - */
	memcpy(bh->b_data + data_offset, buf, chunk);

	mark_buffer_dirty(bh);

	brelse(bh);

	res = chunk;
out:
	antfs_log_leave("res=%d", res);
	return res;
}

/**
 * ntfs_device_unix_io_read - Read from the device, from the current location
 * @dev:
 * @buf:
 * @count:
 *
 * Description...
 *
 * Returns:
 */
static s64 ntfs_device_linux_io_read(struct ntfs_device *dev, void *buf,
				     s64 count)
{
	struct linux_io_dev_priv *priv = dev->d_private;

	return ntfs_device_linux_io_pread(dev, buf, count, priv->pos);
}

/**
 * ntfs_device_unix_io_write - Write to the device, at the current location
 * @dev:
 * @buf:
 * @count:
 *
 * Description...
 *
 * Returns:
 */
static s64 ntfs_device_unix_io_write(struct ntfs_device *dev, const void *buf,
				     s64 count)
{
	/*--- if (NDevReadOnly(dev)) { ---*/
	return -EROFS;
	/*--- } ---*/
	/*--- NDevSetDirty(dev); ---*/
	/*--- return write(dev, buf, count); ---*/
}

/**
 * ntfs_device_unix_io_sync - Flush any buffered changes to the device
 * @dev:
 *
 * Description...
 *
 * Returns:
 */
static int ntfs_device_unix_io_sync(struct ntfs_device *dev)
{
	int res = 0;

	if (!NDevReadOnly(dev)) {
		res = ntfs_fsync(dev);
		if (res)
			antfs_log_error("Failed to sync device %s",
					dev->d_name);
		else
			NDevClearDirty(dev);
	}
	return res;
}

/**
 * ntfs_device_unix_io_ioctl - Perform an ioctl on the device
 * @dev:
 * @request:
 * @argp:
 *
 * Description...
 *
 * Returns:
 */
static int ntfs_device_unix_io_ioctl(struct ntfs_device *dev, int request,
				     void *argp)
{
	struct block_device *bdev = dev->d_sb->s_bdev;
	fmode_t mode = 0x1D & ~FMODE_NDELAY;

	return blkdev_ioctl(bdev, mode, request, (unsigned long)argp);
}

/**
 * Device operations for working with unix style devices and files.
 */
struct ntfs_device_operations ntfs_device_unix_io_ops = {
	.open = ntfs_device_linux_io_open,
	.close = ntfs_device_linux_io_close,
	.seek = ntfs_device_linux_io_seek,
	.read = ntfs_device_linux_io_read,
	.write = ntfs_device_unix_io_write,
	.pread = ntfs_device_linux_io_pread,
	.pwrite = ntfs_device_unix_io_pwrite,
	.sync = ntfs_device_unix_io_sync,
	.stat = NULL,
	.ioctl = ntfs_device_unix_io_ioctl,
};
