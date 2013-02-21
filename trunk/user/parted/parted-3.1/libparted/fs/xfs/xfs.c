/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2001, 2009-2012 Free Software Foundation, Inc.

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

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include <uuid/uuid.h>
#include "platform_defs.h"
#include "xfs_types.h"
#include "xfs_sb.h"

#define XFS_BLOCK_SIZES		((int[2]){512, 0})

static PedGeometry*
xfs_probe (PedGeometry* geom)
{
	PedSector	block_size;
	PedSector	block_count;
	union {
		struct xfs_sb	sb;
		char		bytes [512];
	} buf;

	if (geom->length < XFS_SB_DADDR + 1)
		return NULL;
	if (!ped_geometry_read (geom, &buf, XFS_SB_DADDR, 1))
		return NULL;

	if (PED_LE32_TO_CPU (buf.sb.sb_magicnum) == XFS_SB_MAGIC) {
		block_size = PED_LE32_TO_CPU (buf.sb.sb_blocksize) / 512;
		block_count = PED_LE64_TO_CPU (buf.sb.sb_dblocks);

		return ped_geometry_new (geom->dev, geom->start,
					 block_size * block_count);
	}

	if (PED_BE32_TO_CPU (buf.sb.sb_magicnum) == XFS_SB_MAGIC) {
		block_size = PED_BE32_TO_CPU (buf.sb.sb_blocksize) / 512;
		block_count = PED_BE64_TO_CPU (buf.sb.sb_dblocks);

		return ped_geometry_new (geom->dev, geom->start,
					 block_size * block_count);
	}

	return NULL;
}

static PedFileSystemOps xfs_ops = {
	probe:		xfs_probe,
};

static PedFileSystemType xfs_type = {
	next:	NULL,
	ops:	&xfs_ops,
	name:	"xfs",
	block_sizes: XFS_BLOCK_SIZES
};

void
ped_file_system_xfs_init ()
{
	ped_file_system_type_register (&xfs_type);
}

void
ped_file_system_xfs_done ()
{
	ped_file_system_type_unregister (&xfs_type);
}
