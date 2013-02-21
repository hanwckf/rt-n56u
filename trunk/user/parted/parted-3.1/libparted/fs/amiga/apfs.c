/*
    apfs.c -- parted support for apfs file systems
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
#include "apfs.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

static int
_apfs_probe_root (uint32_t *block, uint32_t blocksize, uint32_t kind) {
	if (PED_BE32_TO_CPU (block[0]) != kind) return 0;
	return 1;
}

static PedGeometry*
_generic_apfs_probe (PedGeometry* geom, uint32_t kind)
{
	uint32_t *block;
	PedSector root;
	struct PartitionBlock * part;
	uint32_t blocksize = 1, reserved = 2;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (geom->dev != NULL);

	/* Finds the blocksize and reserved values of the partition block */
	if (!(part = ped_malloc (PED_SECTOR_SIZE_DEFAULT*blocksize))) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Failed to allocate partition block\n"), __func__);
		goto error_part;
	}
	if (amiga_find_part(geom, part) != NULL) {
		reserved = PED_BE32_TO_CPU (part->de_Reserved);
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
	root = geom->start+reserved*blocksize;
	if (!ped_device_read (geom->dev, block, root, blocksize)) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Couldn't read root block %llu\n"), __func__, root);
		goto error;
	}
	if (_apfs_probe_root(block, blocksize, kind) == 1) {
		free(block);
		return ped_geometry_duplicate (geom);
	}

error:
	free (block);
error_block:
error_part:
	return NULL;
}

static PedGeometry*
_apfs1_probe (PedGeometry* geom) {
	return _generic_apfs_probe (geom, 0x50463101);
}

static PedGeometry*
_apfs2_probe (PedGeometry* geom) {
	return _generic_apfs_probe (geom, 0x50463102);
}

static PedFileSystemOps _apfs1_ops = {
	probe:		_apfs1_probe,
};
static PedFileSystemOps _apfs2_ops = {
	probe:		_apfs2_probe,
};

#define APFS_BLOCK_SIZES ((int[2]){512, 0})

PedFileSystemType _apfs1_type = {
       next:		 NULL,
       ops:		 &_apfs1_ops,
       name:		 "apfs1",
       block_sizes:      APFS_BLOCK_SIZES
};
PedFileSystemType _apfs2_type = {
       next:		 NULL,
       ops:		 &_apfs2_ops,
       name:		 "apfs2",
       block_sizes:      APFS_BLOCK_SIZES
};
