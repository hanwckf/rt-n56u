/*
    interface.c -- parted binding glue to libext2resize
    Copyright (C) 1998-2000, 2007-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* VERSION: libext2resize 1.1.6 (by Lennert)
 * merged 1.1.11 changes (by Andrew)
 */

#include <config.h>

#include <parted/parted.h>
#include "ext2.h"

static PedFileSystemType _ext2_type;
static PedFileSystemType _ext3_type;

struct ext2_dev_handle* ext2_make_dev_handle_from_parted_geometry(PedGeometry* geom);

static PedGeometry*
_ext2_generic_probe (PedGeometry* geom, int expect_ext_ver)
{
	void *sb_v;
	if (!ped_geometry_read_alloc(geom, &sb_v, 2, 2))
		return NULL;
	struct ext2_super_block *sb = sb_v;

	if (EXT2_SUPER_MAGIC(*sb) == EXT2_SUPER_MAGIC_CONST) {
		PedSector block_size = 1 << (EXT2_SUPER_LOG_BLOCK_SIZE(*sb) + 1);
		PedSector block_count = EXT2_SUPER_BLOCKS_COUNT(*sb);
		PedSector group_blocks = EXT2_SUPER_BLOCKS_PER_GROUP(*sb);
		PedSector group_nr = EXT2_SUPER_BLOCK_GROUP_NR(*sb);
		PedSector first_data_block = EXT2_SUPER_FIRST_DATA_BLOCK(*sb);
		int version = EXT2_SUPER_REV_LEVEL(*sb);
		int is_ext3 = 0;
		int is_ext4 = 0;

		is_ext3 = (EXT2_SUPER_FEATURE_COMPAT (*sb)
			   & EXT3_FEATURE_COMPAT_HAS_JOURNAL) != 0;
		if (is_ext3) {
			is_ext4 = ((EXT2_SUPER_FEATURE_RO_COMPAT (*sb)
				    & EXT4_FEATURE_RO_COMPAT_HUGE_FILE)
				   || (EXT2_SUPER_FEATURE_RO_COMPAT (*sb)
				       & EXT4_FEATURE_RO_COMPAT_GDT_CSUM)
				   || (EXT2_SUPER_FEATURE_RO_COMPAT (*sb)
				       & EXT4_FEATURE_RO_COMPAT_DIR_NLINK)
				   || (EXT2_SUPER_FEATURE_INCOMPAT (*sb)
				       & EXT4_FEATURE_INCOMPAT_EXTENTS)
				   || (EXT2_SUPER_FEATURE_INCOMPAT (*sb)
				       & EXT4_FEATURE_INCOMPAT_64BIT)
				   || (EXT2_SUPER_FEATURE_INCOMPAT (*sb)
				       & EXT4_FEATURE_INCOMPAT_FLEX_BG));
			if (is_ext4)
				is_ext3 = 0;
		}
		free (sb);

		if (expect_ext_ver == 2 && (is_ext3 || is_ext4))
			return NULL;
		if (expect_ext_ver == 3 && !is_ext3)
			return NULL;
		else if (expect_ext_ver == 4 && !is_ext4)
			return NULL;

		if (version > 0 && group_nr > 0) {
			PedSector start;
			PedGeometry probe_geom;

			start = geom->start
					- group_blocks * group_nr
					- first_data_block;

			if (start < 0)
				return NULL;
			ped_geometry_init (&probe_geom, geom->dev,
					   start, block_count * block_size);
			return _ext2_generic_probe (&probe_geom,
                                                    expect_ext_ver);
		} else {
			return ped_geometry_new (geom->dev, geom->start,
						 block_count * block_size);
		}
	}
        else {
		free (sb);
        }

	return NULL;
}

static PedGeometry*
_ext2_probe (PedGeometry* geom)
{
	return _ext2_generic_probe (geom, 2);
}

static PedGeometry*
_ext3_probe (PedGeometry* geom)
{
	return _ext2_generic_probe (geom, 3);
}

static PedGeometry*
_ext4_probe (PedGeometry* geom)
{
	return _ext2_generic_probe (geom, 4);
}

static PedFileSystemOps _ext2_ops = {
	probe:		_ext2_probe,
};

static PedFileSystemOps _ext3_ops = {
	probe:		_ext3_probe,
};

static PedFileSystemOps _ext4_ops = {
	probe:		_ext4_probe,
};

#define EXT23_BLOCK_SIZES ((int[6]){512, 1024, 2048, 4096, 8192, 0})

static PedFileSystemType _ext2_type = {
       next:		 NULL,
       ops:		 &_ext2_ops,
       name:		 "ext2",
       block_sizes:      EXT23_BLOCK_SIZES
};

static PedFileSystemType _ext3_type = {
       next:		 NULL,
       ops:		 &_ext3_ops,
       name:		 "ext3",
       block_sizes:      EXT23_BLOCK_SIZES
};

static PedFileSystemType _ext4_type = {
       next:		 NULL,
       ops:		 &_ext4_ops,
       name:		 "ext4",
       block_sizes:      EXT23_BLOCK_SIZES
};

void ped_file_system_ext2_init ()
{
	ped_file_system_type_register (&_ext2_type);
	ped_file_system_type_register (&_ext3_type);
	ped_file_system_type_register (&_ext4_type);
}

void ped_file_system_ext2_done ()
{
	ped_file_system_type_unregister (&_ext2_type);
	ped_file_system_type_unregister (&_ext3_type);
	ped_file_system_type_unregister (&_ext4_type);
}
