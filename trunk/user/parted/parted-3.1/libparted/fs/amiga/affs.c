/*
    affs.c -- parted support for affs file systems
    Copyright (C) 1998-2000, 2007, 2009-2012 Free Software Foundation, Inc.

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

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>

#include "amiga.h"
#include "affs.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

static int
_affs_probe_root (uint32_t *block, int blocksize) {
	int i;
	uint32_t sum;

	if (PED_BE32_TO_CPU (block[0]) != 2) return 0;
	if (PED_BE32_TO_CPU (block[128*blocksize-1]) != 1) return 0;
	for (i = 0, sum = 0; i < 128*blocksize; i++)
		sum += PED_BE32_TO_CPU (block[i]);
	if (sum) return 0;
	return 1;
}

static PedGeometry*
_generic_affs_probe (PedGeometry* geom, uint32_t kind)
{
	uint32_t *block;
	PedSector root, len, pos;
	struct PartitionBlock * part;
	int blocksize = 1, reserved = 2, prealloc = 0;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (geom->dev != NULL);

	/* Finds the blocksize, prealloc and reserved values of the partition block */
	if (!(part = ped_malloc (PED_SECTOR_SIZE_DEFAULT*blocksize))) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Failed to allocate partition block\n"), __func__);
		goto error_part;
	}
	if (amiga_find_part(geom, part) != NULL) {
		prealloc = PED_BE32_TO_CPU (part->de_PreAlloc);
		reserved = PED_BE32_TO_CPU (part->de_Reserved);
		reserved = reserved == 0 ? 1 : reserved;
		blocksize = PED_BE32_TO_CPU (part->de_SizeBlock)
			* PED_BE32_TO_CPU (part->de_SectorPerBlock) / 128;
	}
	free (part);

	/* Test boot block */
	if (!(block = ped_malloc (PED_SECTOR_SIZE_DEFAULT*blocksize))) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Failed to allocate block\n"), __func__);
		goto error_block;
	}
	if (!ped_device_read (geom->dev, block, geom->start, blocksize)) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Couldn't read boot block %llu\n"), __func__, geom->start);
		goto error;
	}
	if (PED_BE32_TO_CPU (block[0]) != kind) {
		goto error;
	}

	/* Find and test the root block */
	len = geom->length / blocksize - reserved;
	pos = (len - 1) / 2;
	root = geom->start + (pos + reserved) * blocksize;
	printf ("Pralloc = %d, Reserved = %d, blocksize = %d, root block at %llu\n",
		prealloc, reserved, blocksize, root);

	if (!ped_device_read (geom->dev, block, root, blocksize)) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Couldn't read root block %llu\n"), __func__, root);
		goto error;
	}
	if (_affs_probe_root(block, blocksize) == 1) {
		free (block);
		return ped_geometry_duplicate (geom);
	}

error:
	free (block);
error_block:
error_part:
	return NULL;
}
static PedGeometry*
_affs0_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5300);
}
static PedGeometry*
_affs1_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5301);
}
static PedGeometry*
_affs2_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5302);
}
static PedGeometry*
_affs3_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5303);
}
static PedGeometry*
_affs4_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5304);
}
static PedGeometry*
_affs5_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5305);
}
static PedGeometry*
_affs6_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5306);
}
static PedGeometry*
_affs7_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x444f5307);
}
static PedGeometry*
_amufs_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754653);
}
static PedGeometry*
_amufs0_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754600);
}
static PedGeometry*
_amufs1_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754601);
}
static PedGeometry*
_amufs2_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754602);
}
static PedGeometry*
_amufs3_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754603);
}
static PedGeometry*
_amufs4_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754604);
}
static PedGeometry*
_amufs5_probe (PedGeometry* geom) {
	return _generic_affs_probe (geom, 0x6d754605);
}

static PedFileSystemOps _affs0_ops = {
	probe:		_affs0_probe,
};
static PedFileSystemOps _affs1_ops = {
	probe:		_affs1_probe,
};
static PedFileSystemOps _affs2_ops = {
	probe:		_affs2_probe,
};
static PedFileSystemOps _affs3_ops = {
	probe:		_affs3_probe,
};
static PedFileSystemOps _affs4_ops = {
	probe:		_affs4_probe,
};
static PedFileSystemOps _affs5_ops = {
	probe:		_affs5_probe,
};
static PedFileSystemOps _affs6_ops = {
	probe:		_affs6_probe,
};
static PedFileSystemOps _affs7_ops = {
	probe:		_affs7_probe,
};
static PedFileSystemOps _amufs_ops = {
	probe:		_amufs_probe,
};
static PedFileSystemOps _amufs0_ops = {
	probe:		_amufs0_probe,
};
static PedFileSystemOps _amufs1_ops = {
	probe:		_amufs1_probe,
};
static PedFileSystemOps _amufs2_ops = {
	probe:		_amufs2_probe,
};
static PedFileSystemOps _amufs3_ops = {
	probe:		_amufs3_probe,
};
static PedFileSystemOps _amufs4_ops = {
	probe:		_amufs4_probe,
};
static PedFileSystemOps _amufs5_ops = {
	probe:		_amufs5_probe,
};

#define AFFS_BLOCK_SIZES        ((int[5]){512, 1024, 2048, 4096, 0})
#define AMUFS_BLOCK_SIZES       ((int[2]){512, 0})


PedFileSystemType _affs0_type = {
       next:		 NULL,
       ops:		 &_affs0_ops,
       name:		 "affs0",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs1_type = {
       next:		 NULL,
       ops:		 &_affs1_ops,
       name:		 "affs1",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs2_type = {
       next:		 NULL,
       ops:		 &_affs2_ops,
       name:		 "affs2",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs3_type = {
       next:		 NULL,
       ops:		 &_affs3_ops,
       name:		 "affs3",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs4_type = {
       next:		 NULL,
       ops:		 &_affs4_ops,
       name:		 "affs4",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs5_type = {
       next:		 NULL,
       ops:		 &_affs5_ops,
       name:		 "affs5",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs6_type = {
       next:		 NULL,
       ops:		 &_affs6_ops,
       name:		 "affs6",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _affs7_type = {
       next:		 NULL,
       ops:		 &_affs7_ops,
       name:		 "affs7",
       block_sizes:      AFFS_BLOCK_SIZES
};
PedFileSystemType _amufs_type = {
       next:		 NULL,
       ops:		 &_amufs_ops,
       name:		 "amufs",
       block_sizes:      AMUFS_BLOCK_SIZES
};
PedFileSystemType _amufs0_type = {
       next:		 NULL,
       ops:		 &_amufs0_ops,
       name:		 "amufs0",
       block_sizes:      AMUFS_BLOCK_SIZES
};
PedFileSystemType _amufs1_type = {
       next:		 NULL,
       ops:		 &_amufs1_ops,
       name:		 "amufs1",
       block_sizes:      AMUFS_BLOCK_SIZES
};
PedFileSystemType _amufs2_type = {
       next:		 NULL,
       ops:		 &_amufs2_ops,
       name:		 "amufs2",
       block_sizes:      AMUFS_BLOCK_SIZES
};
PedFileSystemType _amufs3_type = {
       next:		 NULL,
       ops:		 &_amufs3_ops,
       name:		 "amufs3",
       block_sizes:      AMUFS_BLOCK_SIZES
};
PedFileSystemType _amufs4_type = {
       next:		 NULL,
       ops:		 &_amufs4_ops,
       name:		 "amufs4",
       block_sizes:      AMUFS_BLOCK_SIZES
};
PedFileSystemType _amufs5_type = {
       next:		 NULL,
       ops:		 &_amufs5_ops,
       name:		 "amufs5",
       block_sizes:      AMUFS_BLOCK_SIZES
};
