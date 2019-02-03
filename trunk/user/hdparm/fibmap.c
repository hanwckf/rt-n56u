/*
 * Based upon original code by Ric Wheeler and Mark Lord.
 *
 * Copyright (c) EMC Corporation 2008
 * Copyright (c) Mark Lord 2008
 *
 * You may use/distribute this freely, under the terms of either
 * (your choice) the GNU General Public License version 2,
 * or a BSD style license.
 */
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "hdparm.h"

struct file_extent {
	__u64 byte_offset;
	__u64 first_block;
	__u64 last_block;
	__u64 block_count;
};

static void handle_extent (struct file_extent ext, unsigned int sectors_per_block, __u64 start_lba)
{
	char lba_info[64], len_info[32];
	__u64 begin_lba, end_lba;
	__u64 nsectors = ext.block_count * sectors_per_block;

	if (ext.first_block) {
		begin_lba = start_lba + ( ext.first_block     * sectors_per_block);
		end_lba   = start_lba + ((ext.last_block + 1) * sectors_per_block) - 1;
	} else {
		begin_lba = end_lba = 0;
	}

	if (ext.first_block)
		sprintf(lba_info, "%10llu %10llu", begin_lba, end_lba);
	else
		strcpy(lba_info, "      -          -   ");
	if (!ext.first_block && !nsectors)
		strcpy(len_info, "      -   ");
	else
		sprintf(len_info, "%10llu", nsectors);
	printf("%12llu %s %s\n", ext.byte_offset, lba_info, len_info);
}

static int walk_fibmap (int fd, struct stat *st, unsigned int blksize, unsigned int sectors_per_block, __u64 start_lba)
{
	struct file_extent ext;
	unsigned long num_blocks;
	__u64 blk_idx, hole = ~0ULL;

	/*
	 * How many calls to FIBMAP do we need?
	 * FIBMAP returns a filesystem block number (counted from the start of the device)
	 * for each file block.  This can be converted to a disk LBA using the filesystem
	 * blocksize and LBA offset obtained earlier.
	 */
	num_blocks = (st->st_size + blksize - 1) / blksize;
	memset(&ext, 0, sizeof(ext));

	/*
	 * Loop through the file, building a map of the extents.
	 * All of this is done in filesystem blocks size units.
	 *
	 * Assumptions:
	 * Throughout the file, there can be any number of blocks backed by holes
	 * or by allocated blocks.  Tail-packed files are special - if we find a file
	 * that has a size and has no allocated blocks, we could flag it as a "tail-packed"
	 * file if we cared: data is packed into the tail space of the inode block.
	 */
	for (blk_idx = 0; blk_idx < num_blocks; blk_idx++) {
		unsigned int blknum = blk_idx;
		__u64 blknum64;
		/*
		 * FIBMAP takes a block index as input and on return replaces it with a
		 * block number relative to the beginning of the filesystem/partition.
		 * An output value of zero means "unallocated", or a "hole" in a sparse file.
		 * Note that this is a 32-bit value, so it will not work properly on
		 * files/filesystems with more than 4 billion blocks (~16TB),
		 */
		if (ioctl(fd, FIBMAP, &blknum) == -1) {
			int err = errno;
			perror("ioctl(FIBMAP)");
			return err;
		}
		blknum64 = blknum;	/* work in 64-bits as much as possible */

		if (blk_idx && blknum64 == (ext.last_block + 1)) {
			/*
			 * Continuation of extent: Bump last_block and block_count.
			 */
			ext.last_block = blknum64 ? blknum64 : hole;
			ext.block_count++;
		} else {
			/*
			 * New extent: print previous extent (if any), and re-init the extent record.
			 */
			if (blk_idx)
				handle_extent(ext, sectors_per_block, start_lba);
			ext.first_block = blknum64;
			ext.last_block  = blknum64 ? blknum64 : hole;
			ext.block_count = 1;
			ext.byte_offset = blk_idx * blksize;
		}
	}
	handle_extent(ext, sectors_per_block, start_lba);
	return 0;
}

#define FE_COUNT	8000
#define FE_FLAG_LAST	(1 <<  0)
#define FE_FLAG_UNKNOWN	(1 <<  1)
#define FE_FLAG_UNALLOC	(1 <<  2)
#define FE_FLAG_NOALIGN	(1 <<  8)

#define EXTENT_UNKNOWN (FE_FLAG_UNKNOWN | FE_FLAG_UNALLOC | FE_FLAG_NOALIGN)

struct fe_s {
	__u64 logical;
	__u64 physical;
	__u64 length;
	__u64 reserved64[2];
	__u32 flags;
	__u32 reserved32[3];
};

struct fm_s {
	__u64 start;
	__u64 length;
	__u32 flags;
	__u32 mapped_extents;
	__u32 extent_count;
	__u32 reserved;
};

struct fs_s {
	struct fm_s fm;
	struct fe_s fe[FE_COUNT];
};

#define FIEMAP	_IOWR('f', 11, struct fm_s)

static int walk_fiemap (int fd, unsigned int sectors_per_block, __u64 start_lba, unsigned int sector_bytes)
{
	unsigned int i, done = 0;
	unsigned int blksize = sectors_per_block * sector_bytes;
	struct fs_s fs;

	memset(&fs, 0, sizeof(fs));
	do {
		fs.fm.length = ~0ULL;
		fs.fm.flags  = 0;
		fs.fm.extent_count = FE_COUNT;

		if (-1 == ioctl(fd, FIEMAP, &fs)) {
			int err = errno;
			//perror("ioctl(FIEMAP)");
			return err;
		}

		if (0) fprintf(stderr, "ioctl(FIEMAP) returned %llu extents\n", (__u64)fs.fm.mapped_extents);
		if (!fs.fm.mapped_extents) {
			done = 1;
		} else {
			struct file_extent ext;
			memset(&ext, 0, sizeof(ext));
			for (i = 0; i < fs.fm.mapped_extents; i++) {
				__u64 phy_blk, ext_len;

				ext.byte_offset = fs.fe[i].logical;
				if (0) fprintf(stderr, "log=%llu phy=%llu len=%llu flags=0x%x\n", fs.fe[i].logical,
						fs.fe[i].physical, fs.fe[i].length, fs.fe[i].flags);
				if (fs.fe[i].flags & EXTENT_UNKNOWN) {
					ext.first_block = 0;
					ext.last_block  = 0;
					ext.block_count = 0; /* FIEMAP returns garbage for this. Ugh. */
				} else {
					phy_blk = fs.fe[i].physical / blksize;
					ext_len = fs.fe[i].length   / blksize;

					ext.first_block = phy_blk;
					ext.last_block  = phy_blk + ext_len - 1;
					ext.block_count = ext_len;
				}
				handle_extent(ext, sectors_per_block, start_lba);

				if (fs.fe[i].flags & FE_FLAG_LAST) {
					/*
					 * Hit an ext4 bug in 2.6.29.4, where some FIEMAP calls
					 * had the LAST flag set in the final returned extent,
					 * even though there were *plenty* more extents to be had
					 * from continued FIEMAP calls.
					 *
					 * So, we'll ignore it here, and instead rely on getting
					 * a zero count back from fs.fm.mapped_extents at the end.
					 */
					if (0) fprintf(stderr, "%s: ignoring LAST bit\n", __func__);
					//done = 1;
				}

			}
			fs.fm.start = (fs.fe[i-1].logical + fs.fe[i-1].length);
		}
	} while (!done);
	return 0;
}

int do_filemap (const char *file_name)
{
	int fd, err;
	struct stat st;
	__u64 start_lba = 0;
	unsigned int sectors_per_block, blksize, sector_bytes;

	if ((fd = open(file_name, O_RDONLY)) == -1) {
		err = errno;
		perror(file_name);
		return err;
	}
	if (fstat(fd, &st) == -1) {
		err = errno;
		perror(file_name);
		return err;
	}
	if (!S_ISREG(st.st_mode)) {
		fprintf(stderr, "%s: not a regular file\n", file_name);
		close(fd);
		return EINVAL;
	}

	/*
	 * Get the filesystem starting LBA:
	 */
	err = get_dev_t_geometry(st.st_dev, NULL, NULL, NULL, &start_lba, NULL, &sector_bytes);
	if (err) {
		close(fd);
		return err;
	}
	if (start_lba == START_LBA_UNKNOWN) {
		fprintf(stderr, "Unable to determine start offset LBA for device, aborting.\n");
		close(fd);
		return EIO;
	}
	if((err=ioctl(fd,FIGETBSZ,&blksize))){
		fprintf(stderr, "Unable to determine block size, aborting.\n");
		close(fd);
		return err;
	};
	sectors_per_block = blksize / sector_bytes;
	printf("\n%s:\n filesystem blocksize %u, begins at LBA %llu;"
	       " assuming %u byte sectors.\n",
	       file_name, blksize, start_lba, sector_bytes);
	printf("%12s %10s %10s %10s\n", "byte_offset", "begin_LBA", "end_LBA", "sectors");

	if (st.st_size == 0) {
		struct file_extent ext;
		memset(&ext, 0, sizeof(ext));
		handle_extent(ext, sectors_per_block, start_lba);
		close(fd);
		return 0;
	}

	err = walk_fiemap(fd, sectors_per_block, start_lba, sector_bytes);
	if (err)
		err = walk_fibmap(fd, &st, blksize, sectors_per_block, start_lba);
	close (fd);
	return 0;
}
