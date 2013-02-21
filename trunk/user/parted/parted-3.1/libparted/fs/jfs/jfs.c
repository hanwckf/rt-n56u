/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2001, 2007, 2009-2012 Free Software Foundation, Inc.

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
#include <parted/endian.h>

#define _JFS_UTILITY
#include "jfs_types.h"
#include "jfs_superblock.h"

#define JFS_SUPER_SECTOR 64

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#define JFS_BLOCK_SIZES		((int[2]){512, 0})

static PedGeometry*
jfs_probe (PedGeometry* geom)
{
	union {
		struct superblock	sb;
		char			bytes[512];
	} buf;

        /* FIXME: for now, don't even try to deal with larger sector size.  */
	if (geom->dev->sector_size != PED_SECTOR_SIZE_DEFAULT)
		return NULL;

	if (geom->length < JFS_SUPER_SECTOR + 1)
		return NULL;
	if (!ped_geometry_read (geom, &buf, JFS_SUPER_SECTOR, 1))
		return NULL;

	if (strncmp (buf.sb.s_magic, JFS_MAGIC, 4) == 0) {
		PedSector block_size = PED_LE32_TO_CPU (buf.sb.s_pbsize) / 512;
		PedSector block_count = PED_LE64_TO_CPU (buf.sb.s_size);

		return ped_geometry_new (geom->dev, geom->start,
					 block_size * block_count);
	} else {
		return NULL;
	}
}

static PedFileSystemOps jfs_ops = {
	probe:		jfs_probe,
};

static PedFileSystemType jfs_type = {
	next:	NULL,
	ops:	&jfs_ops,
	name:	"jfs",
	block_sizes: JFS_BLOCK_SIZES
};

void
ped_file_system_jfs_init ()
{
	ped_file_system_type_register (&jfs_type);
}

void
ped_file_system_jfs_done ()
{
	ped_file_system_type_unregister (&jfs_type);
}
