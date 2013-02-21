/*
    libparted
    Copyright (C) 1998-2001, 2007-2012 Free Software Foundation, Inc.

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
#include <string.h>
#include <uuid/uuid.h>

#include "fat.h"

PedFileSystem*
fat_alloc (const PedGeometry* geom)
{
	PedFileSystem*		fs;

	fs = (PedFileSystem*) ped_malloc (sizeof (PedFileSystem));
	if (!fs)
		goto error;

	fs->type_specific = (FatSpecific*) ped_malloc (sizeof (FatSpecific));
	if (!fs->type_specific)
		goto error_free_fs;

	fs->geom = ped_geometry_duplicate (geom);
	if (!fs->geom)
		goto error_free_type_specific;

	fs->checked = 0;
	return fs;

error_free_type_specific:
	free (fs->type_specific);
error_free_fs:
	free (fs);
error:
	return NULL;
}

void
fat_free (PedFileSystem* fs)
{
	ped_geometry_destroy (fs->geom);
	free (fs->type_specific);
	free (fs);
}

PedGeometry*
fat_probe (PedGeometry* geom, FatType* fat_type)
{
	PedFileSystem*		fs;
	FatSpecific*		fs_info;
	PedGeometry*		result;

	fs = fat_alloc (geom);
	if (!fs)
		goto error;
	fs_info = (FatSpecific*) fs->type_specific;

	if (!fat_boot_sector_read (&fs_info->boot_sector, geom))
		goto error_free_fs;
	if (!fat_boot_sector_analyse (&fs_info->boot_sector, fs))
		goto error_free_fs;

	*fat_type = fs_info->fat_type;
	result = ped_geometry_new (geom->dev, geom->start,
				   fs_info->sector_count);

	fat_free (fs);
	return result;

error_free_fs:
	fat_free (fs);
error:
	return NULL;
}

PedGeometry*
fat_probe_fat16 (PedGeometry* geom)
{
	FatType		fat_type;
	PedGeometry*	probed_geom = fat_probe (geom, &fat_type);

	if (probed_geom) {
		if (fat_type == FAT_TYPE_FAT16)
			return probed_geom;
		ped_geometry_destroy (probed_geom);
	}
	return NULL;
}

PedGeometry*
fat_probe_fat32 (PedGeometry* geom)
{
	FatType		fat_type;
	PedGeometry*	probed_geom = fat_probe (geom, &fat_type);

	if (probed_geom) {
		if (fat_type == FAT_TYPE_FAT32)
			return probed_geom;
		ped_geometry_destroy (probed_geom);
	}
	return NULL;
}

static PedFileSystemOps fat16_ops = {
	probe:		fat_probe_fat16,
};

static PedFileSystemOps fat32_ops = {
	probe:		fat_probe_fat32,
};

#define FAT_BLOCK_SIZES ((int[2]){512, 0})

PedFileSystemType fat16_type = {
	next:	        NULL,
	ops:	        &fat16_ops,
	name:	        "fat16",
        block_sizes:    FAT_BLOCK_SIZES
};

PedFileSystemType fat32_type = {
	next:	        NULL,
	ops:	        &fat32_ops,
	name:	        "fat32",
        block_sizes:    FAT_BLOCK_SIZES
};

void
ped_file_system_fat_init ()
{
	if (sizeof (FatBootSector) != 512) {
		ped_exception_throw (PED_EXCEPTION_BUG, PED_EXCEPTION_CANCEL,
			_("GNU Parted was miscompiled: the FAT boot sector "
			"should be 512 bytes.  FAT support will be disabled."));
	} else {
		ped_file_system_type_register (&fat16_type);
		ped_file_system_type_register (&fat32_type);
	}
}

void
ped_file_system_fat_done ()
{
	ped_file_system_type_unregister (&fat16_type);
	ped_file_system_type_unregister (&fat32_type);
}
