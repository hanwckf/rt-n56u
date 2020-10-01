/**
 * device.c - Low level device io functions. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2004-2013 Anton Altaparmakov
 * Copyright (c) 2004-2006 Szabolcs Szakacsits
 * Copyright (c) 2010      Jean-Pierre Andre
 * Copyright (c) 2008-2013 Tuxera Inc.
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

#include <asm/div64.h>
#include <linux/fs.h>
#include <linux/fd.h>
#include <linux/hdreg.h>

#include "antfs.h"
#include "types.h"
#include "mst.h"
#include "debug.h"
#include "device.h"
#include "misc.h"

/**
 * ntfs_device_alloc - allocate an ntfs device structure and pre-initialize it
 *
 * @param name       name of the device (must be present)
 * @param state      initial device state (usually zero)
 * @param dops       ntfs device operations to use with the device (must be present)
 * @param priv_data  pointer to private data (optional)
 *
 * Allocate an ntfs device structure and pre-initialize it with the user-
 * specified device operations @dops, device state @state, device name @name,
 * and optional private data @priv_data.
 *
 * Note, @name is copied and can hence be freed after this functions returns.
 *
 * @retval pointer to the allocated ntfs device structure on success
 * @retval error ptr on failure
 */
struct ntfs_device *ntfs_device_alloc(const char *name, const long state,
				      struct ntfs_device_operations *dops,
				      void *priv_data)
{
	struct ntfs_device *dev;

	if (!name)
		return ERR_PTR(-EINVAL);

	dev = ntfs_malloc(sizeof(struct ntfs_device));
	if (!dev)
		return ERR_PTR(-ENOMEM);

	dev->d_name = kstrdup(name, GFP_KERNEL);
	if (!dev->d_name) {
		ntfs_free(dev);
		return ERR_PTR(-ENOMEM);
	}

	dev->d_ops = dops;
	dev->d_state = state;
	dev->d_private = priv_data;
	dev->d_heads = -1;
	dev->d_sectors_per_track = -1;

	return dev;
}

/**
 * ntfs_device_free - free an ntfs device structure
 *
 * @param dev  ntfs device structure to free
 *
 * Free the ntfs device structure @dev.
 *
 * @retval 0 on success
 * @retval EINVAL		Invalid pointer @dev.
 * @retval EBUSY		Device is still open. Close it before freeing it!
 */
int ntfs_device_free(struct ntfs_device *dev)
{
	if (IS_ERR_OR_NULL(dev))
		return -EINVAL;
	if (NDevOpen(dev))
		return -EBUSY;
	ntfs_free(dev->d_name);
	ntfs_free(dev);
	return 0;
}

/**
 * ntfs_pread - positioned read from disk
 *
 * @param dev    device to read from
 * @param pos    position in device to read from
 * @param count  number of bytes to read
 * @param b      output data buffer
 *
 * This function will read @count bytes from device @dev at position @pos into
 * the data buffer @b.
 *
 * On success, return the number of successfully read bytes. If this number is
 * lower than @count this means that we have either reached end of file or
 * encountered an error during the read so that the read is partial. 0 means
 * end of file or nothing to read (@count is 0).
 *
 * On error and nothing has been read, return negative error code from
 * either seek, read, or set to -EINVAL in case of invalid arguments.
 *
 * @retval 0 on success
 * @retval negative error code on failure
 */
s64 ntfs_pread(struct ntfs_device *dev, const s64 pos, s64 count, void *b)
{
	s64 br, total;
	struct ntfs_device_operations *dops;

	antfs_log_enter("pos %lld, count %lld", (long long)pos,
			(long long)count);

	if (!b || count < 0 || pos < 0)
		return -EINVAL;
	if (!count)
		return 0;

	dops = dev->d_ops;

	for (total = 0; count; count -= br, total += br) {
		br = dops->pread(dev, (char *)b + total, count, pos + total);
		/* If everything ok, continue. */
		if (br > 0)
			continue;
		/* If EOF or error return number of bytes read. */
		if (!br || total)
			return total;
		/* Nothing read and error, return error status. */
		return br;
	}
	/* Finally, return the number of bytes read. */
	antfs_log_leave("total=%lld", (long long)total);
	return total;
}

/**
 * ntfs_pwrite - positioned write to disk
 *
 * @param dev    device to write to
 * @param pos    position in file descriptor to write to
 * @param count  number of bytes to write
 * @param b      data buffer to write to disk
 *
 * This function will write @count bytes from data buffer @b to the device @dev
 * at position @pos.
 *
 * On success, return the number of successfully written bytes. If this number
 * is lower than @count this means that the write has been interrupted in
 * flight or that an error was encountered during the write so that the write
 * is partial. 0 means nothing was written (also return 0 when @count is 0).
 *
 * @retval 0 nothing was written
 * @retval positive number of bytes written
 * @retval negative error code of either seek, write, or set
 *         to EINVAL in case of invalid arguments.
 */
s64 ntfs_pwrite(struct ntfs_device *dev, const s64 pos, s64 count,
		const void *b)
{
	s64 written, total;
	struct ntfs_device_operations *dops;

	antfs_log_enter("pos %lld, count %lld", (long long)pos,
			(long long)count);

	if (!b || count < 0 || pos < 0)
		return -EINVAL;
	if (!count)
		return 0;
	if (NDevReadOnly(dev))
		return -EROFS;

	dops = dev->d_ops;

	NDevSetDirty(dev);
	for (total = 0; count; count -= written, total += written) {
		written = dops->pwrite(dev, (const char *)b + total, count,
				       pos + total);
		/* If everything ok, continue. */
		if (written > 0)
			continue;
		/*
		 * If nothing written or error return number of bytes written.
		 */
		if (!written || total)
			break;
		/* Nothing written and error, return error status. */
		total = written;
		break;
	}
	if (NDevSync(dev) && total && dops->sync(dev))
		total--;	/* on sync error, return partially written */
	antfs_log_leave("total=%lld", (long long)total);
	return total;
}

/**
 * ntfs_mst_pread - multi sector transfer (mst) positioned read
 *
 * @param dev     device to read from
 * @param pos     position in file descriptor to read from
 * @param count   number of blocks to read
 * @param bksize  size of each block that needs mst deprotecting
 * @param b       output data buffer
 *
 * Multi sector transfer (mst) positioned read. This function will read @count
 * blocks of size @bksize bytes each from device @dev at position @pos into the
 * the data buffer @b.
 *
 * On success, return the number of successfully read blocks. If this number is
 * lower than @count this means that we have reached end of file, that the read
 * was interrupted, or that an error was encountered during the read so that
 * the read is partial. 0 means end of file or nothing was read (also return 0
 * when @count or @bksize are 0).
 *
 * On error and nothing was read, return negative error code of either seek,
 * read, or set to -EINVAL in case of invalid arguments.
 *
 * NOTE: If an incomplete multi sector transfer has been detected the magic
 * will have been changed to magic_BAAD but no error will be returned. Thus it
 * is possible that we return count blocks as being read but that any number
 * (between zero and count!) of these blocks is actually subject to a multi
 * sector transfer error. This should be detected by the caller by checking for
 * the magic being "BAAD".
 */
s64 ntfs_mst_pread(struct ntfs_device *dev, const s64 pos, s64 count,
		   const u32 bksize, void *b)
{
	int64_t i;
	int64_t br;
	uint64_t u_br;

	if (bksize & (bksize - 1) || bksize % NTFS_BLOCK_SIZE)
		return -EINVAL;
	/* Do the read. */
	br = ntfs_pread(dev, pos, count * bksize, b);
	if (br < 0)
		return br;
	/*
	 * Apply fixups to successfully read data, disregarding any errors
	 * returned from the MST fixup function. This is because we want to
	 * fixup everything possible and we rely on the fact that the "BAAD"
	 * magic will be detected later on.
	 */
	u_br = br;
	do_div(u_br, bksize);
	count = u_br;
	for (i = 0; i < count; ++i)
		ntfs_mst_post_read_fixup((struct NTFS_RECORD *)
					 ((u8 *) b + i * bksize), bksize);
	/* Finally, return the number of complete blocks read. */
	return count;
}

/**
 * ntfs_device_block_size_set - set block size of a device
 *
 * @param dev         open device
 * @param block_size  block size to set @dev to
 *
 * @retval 0 on success
 * @return negative error code on failure
 */
int ntfs_device_block_size_set(struct ntfs_device *dev, int block_size)
{
	if (!dev)
		return -EINVAL;

	if (!sb_min_blocksize(dev->d_sb, block_size))
		return -EINVAL;

	return 0;
}
