/*
    asfs.c -- parted asfs filesystem support
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
#include "asfs.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

static int
_asfs_probe_root (PedGeometry *geom, uint32_t *block, int blocksize, PedSector root) {
	int i, sum;
	PedSector start, end;

	if (PED_BE32_TO_CPU (block[0]) != 0x53465300) return 0;
	for (i = 0, sum = 1; i < 128*blocksize; i++) sum += PED_BE32_TO_CPU (block[i]);
	if (sum != 0) return 0;
	if (PED_BE32_TO_CPU (block[2]) * blocksize + geom->start != root) {
		return 0;
	}
	start = ((((PedSector) PED_BE32_TO_CPU (block[8])) << 32)
		+ (PedSector) PED_BE32_TO_CPU (block[9])) / 512;
	end = (((((PedSector) PED_BE32_TO_CPU (block[10])) << 32)
		+ (PedSector) PED_BE32_TO_CPU (block[11])) / 512) - 1;
	if (start != geom->start || end != geom->end) return 0;
	return 1;
}

static PedGeometry*
_asfs_probe (PedGeometry* geom)
{
	uint32_t *block;
	struct PartitionBlock * part;
	int blocksize = 1;
        PedSector root;
        int found = 0;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (geom->dev != NULL);

	/* Finds the blocksize of the partition block */
	if (!(part = ped_malloc (PED_SECTOR_SIZE_DEFAULT*blocksize))) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Failed to allocate partition block\n"), __func__);
		goto error_part;
	}
	if (amiga_find_part(geom, part) != NULL) {
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
	root = geom->start;
	if (!ped_device_read (geom->dev, block, root, blocksize)) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Couldn't read root block %llu\n"), __func__, root);
		goto error;
	}
	if (PED_BE32_TO_CPU (block[0]) != 0x53465300) {
		goto error;
	}

	/* Find and test the root blocks */
	if (_asfs_probe_root(geom, block, blocksize, root)) {
		found++;
	}
	root = geom->end - blocksize - (geom->length % blocksize) + 1;
	if (!ped_device_read (geom->dev, block, root, 1)) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Couldn't read root block %llu\n"), __func__, root);
		goto error;
	}
	if (_asfs_probe_root(geom, block, blocksize, root)) {
		found++;
	}
	if (found != 0) {
		free (block);
		return ped_geometry_duplicate (geom);
	}

error:
	free (block);
error_block:
error_part:
	return NULL;
}

static PedFileSystemOps _asfs_ops = {
	probe:		_asfs_probe,
};

PedFileSystemType _asfs_type = {
       next:		 NULL,
       ops:		 &_asfs_ops,
       name:		 "asfs",
       block_sizes:      ((int[2]){512, 0})
};
