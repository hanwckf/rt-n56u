/* -*- Mode: c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-

    libparted - a library for manipulating disk partitions
    Copyright (C) 2000-2001, 2007-2012 Free Software Foundation, Inc.

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

    Contributor:  Matt Wilson <msw@redhat.com>
*/

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>
#include <stdbool.h>
#include "pt-tools.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#define	AIX_LABEL_MAGIC		0xc9c2d4c1
#define	MAX_TOTAL_PART		16

static PedDiskType aix_disk_type;

static inline int
aix_label_magic_get (const char *label)
{
	return *(unsigned int *)label;
}

static inline void
aix_label_magic_set (char *label, int magic_val)
{
	*(unsigned int *)label = magic_val;
}

static int
aix_probe (const PedDevice *dev)
{
	PED_ASSERT (dev != NULL);

	void *label;
	if (!ptt_read_sector (dev, 0, &label))
		return 0;
	unsigned int magic = aix_label_magic_get (label);
	free (label);
	return magic == AIX_LABEL_MAGIC;
}

static PedDisk*
aix_alloc (const PedDevice* dev)
{
	PedDisk*	disk;

        disk = _ped_disk_alloc (dev, &aix_disk_type);
	if (!disk)
		return NULL;

	return disk;
}

static PedDisk*
aix_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;

	new_disk = ped_disk_new_fresh (disk->dev, &aix_disk_type);
	if (!new_disk)
		return NULL;

	return new_disk;
}

static void
aix_free (PedDisk *disk)
{
	_ped_disk_free (disk);
}

static int
aix_read (PedDisk* disk)
{
	ped_disk_delete_all (disk);
        ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                             PED_EXCEPTION_CANCEL,
                             _("Support for reading AIX disk labels is "
                               "is not implemented yet."));
        return 0;
}

#ifndef DISCOVER_ONLY
static int
aix_write (const PedDisk* disk)
{
        ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                             PED_EXCEPTION_CANCEL,
                             _("Support for writing AIX disk labels is "
                               "is not implemented yet."));
	return 0;
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
aix_partition_new (const PedDisk* disk, PedPartitionType part_type,
		   const PedFileSystemType* fs_type,
		   PedSector start, PedSector end)
{
        ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                             PED_EXCEPTION_CANCEL,
                             _("Support for adding partitions to AIX disk "
                               "labels is not implemented yet."));
        return NULL;
}

static PedPartition*
aix_partition_duplicate (const PedPartition* part)
{
        ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                             PED_EXCEPTION_CANCEL,
                             _("Support for duplicating partitions in AIX "
                               "disk labels is not implemented yet."));
        return NULL;
}

static void
aix_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	_ped_partition_free (part);
}

static int
aix_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
        ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                             PED_EXCEPTION_CANCEL,
                             _("Support for setting system type of partitions "
                               "in AIX disk labels is not implemented yet."));
	return 0;
}

static int
aix_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
        ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                             PED_EXCEPTION_CANCEL,
                             _("Support for setting flags "
                               "in AIX disk labels is not implemented yet."));
        return 0;
}

static int _GL_ATTRIBUTE_CONST
aix_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
        return 0;
}


static int
aix_partition_is_flag_available (const PedPartition* part,
				 PedPartitionFlag flag)
{
        return 0;
}


static int
aix_get_max_primary_partition_count (const PedDisk* disk)
{
	return 4;
}

static bool
aix_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = MAX_TOTAL_PART;
	return true;
}

static int _GL_ATTRIBUTE_PURE
aix_partition_align (PedPartition* part, const PedConstraint* constraint)
{
        PED_ASSERT (part != NULL);

        return 1;
}

static int _GL_ATTRIBUTE_PURE
aix_partition_enumerate (PedPartition* part)
{
	return 1;
}

static int _GL_ATTRIBUTE_PURE
aix_alloc_metadata (PedDisk* disk)
{
	return 1;
}

#include "pt-common.h"
PT_define_limit_functions (aix)

static PedDiskOps aix_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (aix_write),

	partition_set_name:		NULL,
	partition_get_name:		NULL,

	PT_op_function_initializers (aix)
};

static PedDiskType aix_disk_type = {
	next:		NULL,
	name:		"aix",
	ops:		&aix_disk_ops,
	features:	0
};

void
ped_disk_aix_init ()
{
	ped_disk_type_register (&aix_disk_type);
}

void
ped_disk_aix_done ()
{
	ped_disk_type_unregister (&aix_disk_type);
}
