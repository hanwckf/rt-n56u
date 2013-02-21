/*
 *  nilfs2.c - New Implementation of Log filesystem
 *
 *  Written by  Jiro SEKIBA <jir@unicus.jp>
 *
 *  Copyright (C) 2011-2012 Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <parted/parted.h>
#include <parted/crc32.h>
#include <parted/endian.h>

/* Magic value for nilfs2 superblock. */
#define	NILFS2_SUPER_MAGIC		0x3434

/* primariy superblock offset in 512bytes blocks. */
#define NILFS_SB_OFFSET			2

/* secondary superblock offset in 512byte blocks. */
#define NILFS_SB2_OFFSET(devsize)	((((devsize)>>3) - 1) << 3)

struct nilfs2_super_block
{
	uint32_t	s_rev_level;
	uint16_t	s_minor_rev_level;
	uint16_t	s_magic;
	uint16_t	s_bytes;
	uint16_t	s_flags;
	uint32_t	s_crc_seed;
	uint32_t	s_sum;
	uint32_t	s_log_block_size;
	uint64_t	s_nsegments;
	uint64_t	s_dev_size;
	uint64_t	s_first_data_block;
	uint32_t	s_blocks_per_segment;
	uint32_t	s_r_segments_percentage;
	uint64_t	s_last_cno;
	uint64_t	s_last_pseg;
	uint64_t	s_last_seq;
	uint64_t	s_free_blocks_count;
	uint64_t	s_ctime;
	uint64_t	s_mtime;
	uint64_t	s_wtime;
	uint16_t	s_mnt_count;
	uint16_t	s_max_mnt_count;
	uint16_t	s_state;
	uint16_t	s_errors;
	uint64_t	s_lastcheck;
	uint32_t	s_checkinterval;
	uint32_t	s_creator_os;
	uint16_t	s_def_resuid;
	uint16_t	s_def_resgid;
	uint32_t	s_first_ino;
	uint16_t	s_inode_size;
	uint16_t	s_dat_entry_size;
	uint16_t	s_checkpoint_size;
	uint16_t	s_segment_usage_size;
	uint8_t		s_uuid[16];
	char		s_volume_name[80];
	uint32_t	s_c_interval;
	uint32_t	s_c_block_max;
	uint32_t	s_reserved[192];
};

static int
is_valid_nilfs_sb(struct nilfs2_super_block *sb)
{
	static unsigned char sum[4];
	const int sumoff = offsetof (struct nilfs2_super_block, s_sum);
	size_t bytes;
	uint32_t crc;

	if (PED_LE16_TO_CPU(sb->s_magic) != NILFS2_SUPER_MAGIC)
		return 0;

	bytes = PED_LE16_TO_CPU(sb->s_bytes);
	if (bytes > 1024)
		return 0;

	crc = __efi_crc32(sb, sumoff, PED_LE32_TO_CPU(sb->s_crc_seed));
	crc = __efi_crc32(sum, 4, crc);
	crc = __efi_crc32((unsigned char *)sb + sumoff + 4,
			  bytes - sumoff - 4, crc);

	return crc == PED_LE32_TO_CPU(sb->s_sum);
}

PedGeometry*
nilfs2_probe (PedGeometry* geom)
{
	void *sb_v;
	void *sb2_v;
	struct nilfs2_super_block *sb = NULL;
	struct nilfs2_super_block *sb2 = NULL;
	PedSector length = geom->length;

	/* ignore if sector size is not 512bytes for now  */
	if (geom->dev->sector_size != PED_SECTOR_SIZE_DEFAULT)
		return NULL;

	PedSector sb2off = NILFS_SB2_OFFSET(length);
	if (sb2off <= 2)
		return NULL;

	if (ped_geometry_read_alloc(geom, &sb_v, 2, 1))
		sb = sb_v;

	if (ped_geometry_read_alloc(geom, &sb2_v, sb2off, 1))
		sb2 = sb2_v;

	if ((!sb || !is_valid_nilfs_sb(sb)) &&
	    (!sb2 || !is_valid_nilfs_sb(sb2)) ) {
		free(sb);
		free(sb2);
		return NULL;
	}

	/* reserve 4k bytes for secondary superblock */
	length = sb2off + 8;

	free(sb);
	free(sb2);
	return ped_geometry_new(geom->dev, geom->start, length);
}

static PedFileSystemOps nilfs2_ops = {
	probe:			nilfs2_probe,
};

#define NILFS2_BLOCK_SIZES ((int[5]){1024, 2048, 4096, 8192, 0})

static PedFileSystemType nilfs2_type = {
	next:   NULL,
	ops:    &nilfs2_ops,
	name:   "nilfs2",
	block_sizes: NILFS2_BLOCK_SIZES
};

void
ped_file_system_nilfs2_init ()
{
	ped_file_system_type_register (&nilfs2_type);
}

void
ped_file_system_nilfs2_done ()
{
	ped_file_system_type_unregister (&nilfs2_type);
}
