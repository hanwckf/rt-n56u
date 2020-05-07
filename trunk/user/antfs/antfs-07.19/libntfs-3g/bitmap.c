/**
 * bitmap.c - Bitmap handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2002-2006 Anton Altaparmakov
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2004-2008 Szabolcs Szakacsits
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
#include "types.h"
#include "attrib.h"
#include "bitmap.h"
#include "debug.h"
#include "misc.h"

/**
 * ntfs_bitmap_set_bits_in_run - set a run of bits in a bitmap to a value
 *
 * @param na         attribute containing the bitmap
 * @param start_bit  first bit to set
 * @param count      number of bits to set
 * @param value	     value to set the bits to (i.e. 0 or 1)
 *
 * Set @count bits starting at bit @start_bit in the bitmap described by the
 * attribute @na to @value, where @value is either 0 or 1.
 *
 * @retval 0 on success
 * @retval negative error code on failure
 */
static int ntfs_bitmap_set_bits_in_run(ntfs_attr *na, s64 start_bit,
				       s64 count, int value)
{
	s64 bufsize, br;
	u8 *buf, *lastbyte_buf;
	int bit, firstbyte, lastbyte, lastbyte_pos, tmp, err;

	if (IS_ERR_OR_NULL(na) || start_bit < 0 || count < 0) {
		antfs_log_error("Invalid argument (%p, %lld, %lld)",
				na, (long long)start_bit, (long long)count);
		return -EINVAL;
	}

	bit = start_bit & 7;
	if (bit)
		firstbyte = 1;
	else
		firstbyte = 0;

	/* Calculate the required buffer size in bytes, capping it at 8kiB. */
	bufsize = ((count - (bit ? 8 - bit : 0) + 7) >> 3) + firstbyte;
	if (bufsize > 8192)
		bufsize = 8192;

	buf = ntfs_malloc(bufsize);
	if (!buf)
		return -ENOMEM;

	/* Depending on @value, zero or set all bits in the allocated buffer. */
	memset(buf, value ? 0xff : 0, bufsize);

	/* If there is a first partial byte... */
	if (bit) {
		/* read it in... */
		br = ntfs_attr_pread(na, start_bit >> 3, 1, buf);
		if (br != 1) {
			if (br >= 0)
				err = -EIO;
			else
				err = (int)br;
			goto free_err_out;
		}
		/* and set or clear the appropriate bits in it. */
		while ((bit & 7) && count--) {
			if (value)
				*buf |= 1 << bit++;
			else
				*buf &= ~(1 << bit++);
		}
		/* Update @start_bit to the new position. */
		start_bit = (start_bit + 7) & ~7;
	}

	/* Loop until @count reaches zero. */
	lastbyte = 0;
	lastbyte_buf = NULL;
	bit = count & 7;
	do {
		/* If there is a last partial byte... */
		if (count > 0 && bit) {
			lastbyte_pos = ((count + 7) >> 3) + firstbyte;
			if (!lastbyte_pos) {
				/* FIXME: Eeek! BUG! */
				ntfs_log_error("Lastbyte is zero. Leaving "
					       "inconsistent metadata.");
				err = -EIO;
				goto free_err_out;
			}
			/* and it is in the currently loaded bitmap window... */
			if (lastbyte_pos <= bufsize) {
				lastbyte_buf = buf + lastbyte_pos - 1;

				/* read the byte in... */
				br = ntfs_attr_pread(na, (start_bit + count) >>
						     3, 1, lastbyte_buf);
				if (br != 1) {
					/* FIXME: Eeek!We need rollback!(AIA) */
					if (br >= 0)
						err = -EIO;
					else
						err = (int)br;
					ntfs_log_perror("Reading of last byte "
							"failed (%lld). Leaving"
							" inconsistent "
							"metadata",
							(long long)br);
					goto free_err_out;
				}
				/* and set/clear the appropriate bits in it. */
				while (bit && count--) {
					if (value)
						*lastbyte_buf |= 1 << --bit;
					else
						*lastbyte_buf &= ~(1 << --bit);
				}
				/* We don't want to come back here... */
				bit = 0;
				/* We have a last byte that we have handled. */
				lastbyte = 1;
			}
		}

		/* Write the prepared buffer to disk. */
		tmp = (start_bit >> 3) - firstbyte;
		br = ntfs_attr_pwrite(na, tmp, bufsize, buf);
		if (br != bufsize) {
			/* FIXME: Eeek! We need rollback! (AIA) */
			if (br >= 0)
				err = -EIO;
			else
				err = (int)br;
			ntfs_log_perror("Failed to write buffer to bitmap "
					"(%lld != %lld). Leaving inconsistent metadata",
					(long long)br, (long long)bufsize);
			goto free_err_out;
		}

		/* Update counters. */
		tmp = (bufsize - firstbyte - lastbyte) << 3;
		if (firstbyte) {
			firstbyte = 0;
			/*
			 * Re-set the partial first byte so a subsequent write
			 * of the buffer does not have stale, incorrect bits.
			 */
			*buf = value ? 0xff : 0;
		}
		start_bit += tmp;
		count -= tmp;
		tmp = (count + 7) >> 3;
		if (bufsize > tmp)
			bufsize = tmp;

		if (lastbyte && count != 0) {
			/* FIXME: Eeek! BUG! */
			ntfs_log_error("Last buffer but count is not zero "
				       "(%lld). Leaving inconsistent metadata.",
				       (long long)count);
			err = -EIO;
			goto free_err_out;
		}
	} while (count > 0);

	err = 0;

free_err_out:
	ntfs_free(buf);
	return err;
}

/**
 * ntfs_bitmap_set_run - set a run of bits in a bitmap
 *
 * @param na         attribute containing the bitmap
 * @param start_bit  first bit to set
 * @param count      number of bits to set
 *
 * Set @count bits starting at bit @start_bit in the bitmap described by the
 * attribute @na.
 *
 * @retval 0 on success
 * @retval negative error code on failure
 */
int ntfs_bitmap_set_run(ntfs_attr *na, s64 start_bit, s64 count)
{
	int err;

	ntfs_log_enter("Set from bit %lld, count %lld",
		       (long long)start_bit, (long long)count);
	err = ntfs_bitmap_set_bits_in_run(na, start_bit, count, 1);
	ntfs_log_leave();
	return err;
}

/**
 * ntfs_bitmap_clear_run - clear a run of bits in a bitmap
 *
 * @param na         attribute containing the bitmap
 * @param start_bit  first bit to clear
 * @param count      number of bits to clear
 *
 * Clear @count bits starting at bit @start_bit in the bitmap described by the
 * attribute @na.
 *
 * @retval 0 on success
 * @retval negative error code on failure
 */
int ntfs_bitmap_clear_run(ntfs_attr *na, s64 start_bit, s64 count)
{
	int err;

	ntfs_log_enter("Clear from bit %lld, count %lld",
		       (long long)start_bit, (long long)count);
	err = ntfs_bitmap_set_bits_in_run(na, start_bit, count, 0);
	ntfs_log_leave("err: %d", err);
	return err;
}
