/*
    reiserfs.c -- libparted / libreiserfs glue
    Copyright (C) 2001-2002, 2007, 2009-2012 Free Software Foundation, Inc.

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

    This is all rather complicated.  There are a few combinations:
	* shared libraries full support
	* dynamic libraries present full support (via dlopen)
	* dynamic libraries absent (full support disabled) (via dlopen)
	* discover only

    We'd love to hear comments...

    So far, we've opted for maximum flexibility for the user.  Is it
    all worth it?
*/

#include <config.h>

#include <uuid/uuid.h>
#include <fcntl.h>
#include <errno.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>

#if ENABLE_NLS
#	include <libintl.h>
#	define _(String) dgettext (PACKAGE, String)
#else
#	define _(String) (String)
#endif

#include "reiserfs.h"

#define REISERFS_BLOCK_SIZES       ((int[2]){512, 0})

static PedSector reiserfs_super_offset[] = { 128, 16, -1 };
static PedFileSystemType* reiserfs_type;

#define FPTR
#define FCLASS extern

static PedGeometry *reiserfs_probe(PedGeometry *geom)
{
	int i;
	reiserfs_super_block_t sb;

	PED_ASSERT(geom != NULL);

	for (i = 0; reiserfs_super_offset[i] != -1; i++) {
		if (reiserfs_super_offset[i] >= geom->length)
			continue;
		if (!ped_geometry_read (geom, &sb, reiserfs_super_offset[i], 1))
			continue;

		if (strncmp(REISERFS_SIGNATURE, sb.s_magic,
		            strlen(REISERFS_SIGNATURE)) == 0
		    || strncmp(REISER2FS_SIGNATURE, sb.s_magic,
			       strlen(REISER2FS_SIGNATURE)) == 0
		    || strncmp(REISER3FS_SIGNATURE, sb.s_magic,
			       strlen(REISER3FS_SIGNATURE)) == 0) {
			PedSector block_size;
			PedSector block_count;

			block_size = PED_LE16_TO_CPU(sb.s_blocksize)
					/ PED_SECTOR_SIZE_DEFAULT;
			block_count = PED_LE32_TO_CPU(sb.s_block_count);

			return ped_geometry_new(geom->dev, geom->start,
						block_size * block_count);
		}
	}
	return NULL;
}


#define REISER_BLOCK_SIZES ((int[]){512, 1024, 2048, 4096, 8192, 0})

static PedFileSystemOps reiserfs_simple_ops = {
	probe:		reiserfs_probe,
};

static PedFileSystemType reiserfs_simple_type = {
	next:	        NULL,
	ops:	        &reiserfs_simple_ops,
	name:	        "reiserfs",
        block_sizes:    REISER_BLOCK_SIZES
};

void ped_file_system_reiserfs_init()
{
	reiserfs_type = &reiserfs_simple_type;
	ped_file_system_type_register(reiserfs_type);
}

void ped_file_system_reiserfs_done()
{
	ped_file_system_type_unregister(reiserfs_type);
}
