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
#include "calc.h"

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

/* Requires the boot sector to be analysed */
int
fat_alloc_buffers (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	fs_info->buffer_sectors = BUFFER_SIZE;
        fs_info->buffer = ped_malloc (fs_info->buffer_sectors * 512);
        if (!fs_info->buffer)
		goto error;

	fs_info->cluster_info = ped_malloc (fs_info->cluster_count + 2);
	if (!fs_info->cluster_info)
		goto error_free_buffer;

	return 1;

error_free_buffer:
	free (fs_info->buffer);
error:
	return 0;
};

void
fat_free_buffers (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	free (fs_info->cluster_info);
	free (fs_info->buffer);
}

void
fat_free (PedFileSystem* fs)
{
	ped_geometry_destroy (fs->geom);
	free (fs->type_specific);
	free (fs);
}

int
fat_set_frag_sectors (PedFileSystem* fs, PedSector frag_sectors)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (fs_info->cluster_sectors % frag_sectors == 0
			&& frag_sectors <= fs_info->cluster_sectors);

	fs_info->frag_size = frag_sectors * 512;
	fs_info->frag_sectors = frag_sectors;
	fs_info->buffer_frags = fs_info->buffer_sectors / frag_sectors;
	fs_info->cluster_frags = fs_info->cluster_sectors / frag_sectors;
	fs_info->frag_count = fs_info->cluster_count * fs_info->cluster_frags;

	return 1;
}

#ifndef DISCOVER_ONLY
int
fat_clobber (PedGeometry* geom)
{
	FatBootSector		boot_sector;

	if (!fat_boot_sector_read (&boot_sector, geom))
		return 1;

	boot_sector.system_id[0] = 0;
	boot_sector.boot_sign = 0;
	if (boot_sector.u.fat16.fat_name[0] == 'F')
		boot_sector.u.fat16.fat_name[0] = 0;
	if (boot_sector.u.fat32.fat_name[0] == 'F')
		boot_sector.u.fat32.fat_name[0] = 0;

        return ped_geometry_write (geom, &boot_sector, 0, 1);
}

static int
_init_fats (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	table_size;

	table_size = fs_info->fat_sectors * 512
		     / fat_table_entry_size (fs_info->fat_type);
	fs_info->fat = fat_table_new (fs_info->fat_type, table_size);
	if (!fs_info->fat)
		goto error;

	if (!fat_table_read (fs_info->fat, fs, 0))
		goto error_free_fat;

	return 1;

error_free_fat:
	fat_table_destroy (fs_info->fat);
error:
	return 0;
}

PedFileSystem*
fat_open (PedGeometry* geom)
{
	PedFileSystem*		fs;
	FatSpecific*		fs_info;

	fs = fat_alloc (geom);
	if (!fs)
		goto error;
	fs_info = (FatSpecific*) fs->type_specific;

	if (!fat_boot_sector_read (&fs_info->boot_sector, geom))
		goto error_free_fs;
	if (!fat_boot_sector_analyse (&fs_info->boot_sector, fs))
		goto error_free_fs;
	fs->type = (fs_info->fat_type == FAT_TYPE_FAT16)
				? &fat16_type
				: &fat32_type;
	if (fs_info->fat_type == FAT_TYPE_FAT32) {
		if (!fat_info_sector_read (&fs_info->info_sector, fs))
			goto error_free_fs;
	}

	if (!_init_fats (fs))
		goto error_free_fs;
	if (!fat_alloc_buffers (fs))
		goto error_free_fat_table;
	if (!fat_collect_cluster_info (fs))
		goto error_free_buffers;

	return fs;

error_free_buffers:
	fat_free_buffers (fs);
error_free_fat_table:
	fat_table_destroy (fs_info->fat);
error_free_fs:
	fat_free (fs);
error:
	return NULL;
}

static int
fat_root_dir_clear (PedFileSystem* fs)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);
	memset (fs_info->buffer, 0, 512 * fs_info->root_dir_sector_count);
	return ped_geometry_write (fs->geom, fs_info->buffer,
				   fs_info->root_dir_offset,
				   fs_info->root_dir_sector_count);
}

/* hack: use the ext2 uuid library to generate a reasonably random (hopefully
 * with /dev/random) number.  Unfortunately, we can only use 4 bytes of it
 */
static uint32_t
_gen_new_serial_number (void)
{
	union {
		uuid_t uuid;
		uint32_t i;
	} uu32;

	uuid_generate (uu32.uuid);
	return uu32.i;
}

PedFileSystem*
fat_create (PedGeometry* geom, FatType fat_type, PedTimer* timer)
{
	PedFileSystem*		fs;
	FatSpecific*		fs_info;
	FatCluster		table_size;

	fs = fat_alloc (geom);
	if (!fs)
		goto error;
	fs_info = (FatSpecific*) fs->type_specific;

	fs_info->logical_sector_size = 1;
	fs_info->sectors_per_track = geom->dev->bios_geom.sectors;
	fs_info->heads = geom->dev->bios_geom.heads;
	fs_info->sector_count = fs->geom->length;
	fs_info->fat_table_count = 2;
/* some initial values, to be changed later */
	fs_info->root_dir_sector_count = FAT_ROOT_DIR_ENTRY_COUNT
					  / (512 / sizeof (FatDirEntry));
	fs_info->root_dir_entry_count = FAT_ROOT_DIR_ENTRY_COUNT;

	fs_info->fat_type = fat_type;
	if (!fat_calc_sizes (fs->geom->length, 0,
			fs_info->fat_type,
			fs_info->root_dir_sector_count,
			&fs_info->cluster_sectors,
			&fs_info->cluster_count,
			&fs_info->fat_sectors)) {
		ped_exception_throw (PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Partition too big/small for a %s file system."),
			(fat_type == FAT_TYPE_FAT16)
		       		? fat16_type.name
				: fat32_type.name);
		goto error_free_fs;
	}

	fs_info->cluster_size = fs_info->cluster_sectors * 512;

	fs_info->fat_offset = fat_min_reserved_sector_count (fs_info->fat_type);
	fs_info->dir_entries_per_cluster
		= fs_info->cluster_size / sizeof (FatDirEntry);

	if (fs_info->fat_type == FAT_TYPE_FAT16) {
		/* FAT16 */
		fs->type = &fat16_type;

		if (fs_info->cluster_count
			> fat_max_cluster_count (fs_info->fat_type)) {
			fs_info->cluster_count
				= fat_max_cluster_count (fs_info->fat_type);
		}

		fs_info->root_dir_sector_count
			= FAT_ROOT_DIR_ENTRY_COUNT
				/ (512 / sizeof (FatDirEntry));
		fs_info->root_dir_entry_count = FAT_ROOT_DIR_ENTRY_COUNT;
                fs_info->root_dir_offset
			= fs_info->fat_offset
			+ fs_info->fat_sectors * fs_info->fat_table_count;
		fs_info->cluster_offset
			= fs_info->root_dir_offset
			  + fs_info->root_dir_sector_count;
	} else {
		/* FAT32 */
		fs->type = &fat32_type;

		fs_info->info_sector_offset = 1;
		fs_info->boot_sector_backup_offset = 6;

		fs_info->root_dir_sector_count = 0;
		fs_info->root_dir_entry_count = 0;
		fs_info->root_dir_offset = 0;

		fs_info->cluster_offset
			= fs_info->fat_offset
			  + fs_info->fat_sectors * fs_info->fat_table_count;
	}

	table_size = fs_info->fat_sectors * 512
		     / fat_table_entry_size (fs_info->fat_type);
	fs_info->fat = fat_table_new (fs_info->fat_type, table_size);
	if (!fs_info->fat)
		goto error_free_fs;
	fat_table_set_cluster_count (fs_info->fat, fs_info->cluster_count);
	if (!fat_alloc_buffers (fs))
		goto error_free_fat_table;

	if (fs_info->fat_type == FAT_TYPE_FAT32) {
		fs_info->root_cluster
			= fat_table_alloc_cluster (fs_info->fat);
		fat_table_set_eof (fs_info->fat, fs_info->root_cluster);
		memset (fs_info->buffer, 0, fs_info->cluster_size);
		if (!fat_write_cluster (fs, fs_info->buffer,
					fs_info->root_cluster))
			return 0;
	}

	fs_info->serial_number = _gen_new_serial_number ();

	if (!fat_boot_sector_set_boot_code (&fs_info->boot_sector))
		goto error_free_buffers;
	if (!fat_boot_sector_generate (&fs_info->boot_sector, fs))
		goto error_free_buffers;
	if (!fat_boot_sector_write (&fs_info->boot_sector, fs))
		goto error_free_buffers;
	if (fs_info->fat_type == FAT_TYPE_FAT32) {
		if (!fat_info_sector_generate (&fs_info->info_sector, fs))
			goto error_free_buffers;
		if (!fat_info_sector_write (&fs_info->info_sector, fs))
			goto error_free_buffers;
	}

	if (!fat_table_write_all (fs_info->fat, fs))
		goto error_free_buffers;

	if (fs_info->fat_type == FAT_TYPE_FAT16) {
		if (!fat_root_dir_clear (fs))
			goto error_free_buffers;
	}

	return fs;

error_free_buffers:
	fat_free_buffers (fs);
error_free_fat_table:
	fat_table_destroy (fs_info->fat);
error_free_fs:
	fat_free (fs);
error:
	return NULL;
}

PedFileSystem*
fat_create_fat16 (PedGeometry* geom, PedTimer* timer)
{
	return fat_create (geom, FAT_TYPE_FAT16, timer);
}

PedFileSystem*
fat_create_fat32 (PedGeometry* geom, PedTimer* timer)
{
	return fat_create (geom, FAT_TYPE_FAT32, timer);
}

int
fat_close (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	fat_free_buffers (fs);
	fat_table_destroy (fs_info->fat);
	fat_free (fs);
	return 1;
}

/* Hack: just resize the file system outside of its boundaries! */
PedFileSystem*
fat_copy (const PedFileSystem* fs, PedGeometry* geom, PedTimer* timer)
{
	PedFileSystem*		new_fs;

	new_fs = ped_file_system_open (fs->geom);
	if (!new_fs)
		goto error;
	if (!ped_file_system_resize (new_fs, geom, timer))
		goto error_close_new_fs;
	return new_fs;

error_close_new_fs:
	ped_file_system_close (new_fs);
error:
	return 0;
}

static int
_compare_fats (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatTable*	table_copy;
	FatCluster	table_size;
	int		i;

	table_size = fs_info->fat_sectors * 512
		     / fat_table_entry_size (fs_info->fat_type);

	table_copy = fat_table_new (fs_info->fat_type, table_size);
	if (!table_copy)
		goto error;

	for (i = 1; i < fs_info->fat_table_count; i++) {
		if (!fat_table_read (table_copy, fs, i))
			goto error_free_table_copy;
		if (!fat_table_compare (fs_info->fat, table_copy)) {
			if (ped_exception_throw (PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The FATs don't match.  If you don't know "
				  "what this means, then select cancel, run "
				  "scandisk on the file system, and then come "
				  "back."))
			    != PED_EXCEPTION_IGNORE)
				goto error_free_table_copy;
		}
	}

	fat_table_destroy (table_copy);
	return 1;

error_free_table_copy:
	fat_table_destroy (table_copy);
error:
	return 0;
}

int
fat_check (PedFileSystem* fs, PedTimer* timer)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedSector	cluster_sectors;
	FatCluster	cluster_count;
	PedSector	fat_sectors;
	PedSector	align_sectors;
	FatCluster	info_free_clusters;

	align_sectors = fs_info->fat_offset
			- fat_min_reserved_sector_count (fs_info->fat_type);

	if (!fat_calc_sizes (fs->geom->length,
			     align_sectors,
			     fs_info->fat_type,
			     fs_info->root_dir_sector_count,
			     &cluster_sectors,
			     &cluster_count,
			     &fat_sectors)) {
		if (ped_exception_throw (PED_EXCEPTION_BUG,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("There are no possible configurations for this FAT "
			  "type."))
				!= PED_EXCEPTION_IGNORE)
			goto error;
	}

	if (fs_info->fat_type == FAT_TYPE_FAT16) {
		if (cluster_sectors != fs_info->cluster_sectors
		    || cluster_count != fs_info->cluster_count
		    || fat_sectors != fs_info->fat_sectors) {
			if (ped_exception_throw (PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("File system doesn't have expected sizes for "
				  "Windows to like it.  "
				  "Cluster size is %dk (%dk expected); "
				  "number of clusters is %d (%d expected); "
				  "size of FATs is %d sectors (%d expected)."),
				(int) fs_info->cluster_sectors / 2,
					(int) cluster_sectors / 2,
				(int) fs_info->cluster_count,
					(int) cluster_count,
				(int) fs_info->fat_sectors,
					(int) fat_sectors)
					!= PED_EXCEPTION_IGNORE)
				goto error;
		}
	}

	if (fs_info->fat_type == FAT_TYPE_FAT32) {
		info_free_clusters
			= PED_LE32_TO_CPU (fs_info->info_sector.free_clusters);
		if (info_free_clusters != (FatCluster) -1
		    && info_free_clusters != fs_info->fat->free_cluster_count) {
			if (ped_exception_throw (PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("File system is reporting the free space as "
				  "%d clusters, not %d clusters."),
				info_free_clusters,
				fs_info->fat->free_cluster_count)
					!= PED_EXCEPTION_IGNORE)
				goto error;
		}
	}

	if (!_compare_fats (fs))
		goto error;

	fs->checked = 1;
	return 1;	/* existence of fs implies consistency ;-) */

error:
	return 0;
}

/* Calculates how much space there will be in clusters in:
 * 	old_fs intersect the-new-fs
 */
static PedSector
_calc_resize_data_size (
	const PedFileSystem* old_fs,
	PedSector new_cluster_sectors,
	FatCluster new_cluster_count,
	PedSector new_fat_size)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (old_fs);
	PedSector	fat_size_delta;

	fat_size_delta = old_fs_info->fat_sectors - new_fat_size;
	return new_cluster_sectors * new_cluster_count - fat_size_delta * 2;
}

static int
_test_resize_size (const PedFileSystem* fs,
		   PedSector length, PedSector min_data_size)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedGeometry	geom;
        PedSector	_cluster_sectors;
	FatCluster	_cluster_count;
	PedSector	_fat_size;

	ped_geometry_init (&geom, fs->geom->dev, fs->geom->start, length);

	if (fat_calc_resize_sizes (
				&geom,
				fs_info->cluster_sectors,
				FAT_TYPE_FAT16,
				fs_info->root_dir_sector_count,
				fs_info->cluster_sectors,
				&_cluster_sectors,
				&_cluster_count,
				&_fat_size)
	    && _calc_resize_data_size (fs, _cluster_sectors, _cluster_count,
		    		       _fat_size)
	    		>= min_data_size)
		return 1;

	if (fat_calc_resize_sizes (
				&geom,
				fs_info->cluster_sectors,
				FAT_TYPE_FAT32,
				0,
				fs_info->cluster_sectors,
				&_cluster_sectors,
				&_cluster_count,
				&_fat_size)
	    && _calc_resize_data_size (fs, _cluster_sectors, _cluster_count,
		    		       _fat_size)
	    		>= min_data_size)
		return 1;

	return 0;
}

/* does a binary search (!) for the mininum size.  Too hard to compute directly
 * (see calc_sizes() for why!)
 */
static PedSector
_get_min_resize_size (const PedFileSystem* fs, PedSector min_data_size)
{
	PedSector	min_length = 0;
	PedSector	max_length = fs->geom->length;
	PedSector	length;

	while (min_length < max_length - 1) {
		length = (min_length + max_length) / 2;
		if (_test_resize_size (fs, length, min_data_size))
			max_length = length;
		else
			min_length = length;
	}

/* adds a bit of leeway (64 sectors), for resolving extra issues, like root
 * directory allocation, that aren't covered here.
 */
	return max_length + 64;
}

PedConstraint*
fat_get_copy_constraint (const PedFileSystem* fs, const PedDevice* dev)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedGeometry	full_dev;
	PedSector	min_cluster_count;
	FatCluster	used_clusters;
	PedSector	min_data_size;

	if (!ped_geometry_init (&full_dev, dev, 0, dev->length - 1))
		return NULL;

	used_clusters = fs_info->fat->cluster_count
			- fs_info->fat->free_cluster_count;
	min_cluster_count = used_clusters + fs_info->total_dir_clusters;
	min_data_size = min_cluster_count * fs_info->cluster_sectors;

	return ped_constraint_new (ped_alignment_any, ped_alignment_any,
				   &full_dev, &full_dev,
				   _get_min_resize_size (fs, min_data_size),
				   dev->length);
}

PedConstraint*
fat_get_resize_constraint (const PedFileSystem* fs)
{
	return fat_get_copy_constraint (fs, fs->geom->dev);
}

/* FIXME: fat_calc_sizes() needs to say "too big" or "too small", or
 * something.  This is a really difficult (maths) problem to do
 * nicely...
 * 	So, this algorithm works if dev->length / 2 is a valid fat_type
 * size.  (Which is how I got the magic numbers below)
 */
#if 0
/* returns: -1 too small, 0 ok, 1 too big */
static int
_test_create_size (PedSector length, FatType fat_type,
		   PedSector cluster_sectors, PedSector cluster_count)
{
	PedSector	rootdir_sectors;
	PedSector	_cluster_sectors;
	FatCluster	_cluster_count;
	PedSector	_fat_size;

	rootdir_sectors = (fat_type == FAT_TYPE_FAT16) ? 16 : 0;

	if (!fat_calc_sizes (length, 0, fat_type, rootdir_sectors,
			     &_cluster_sectors, &_cluster_count, &_fat_size))
		return -1; // XXX: doesn't work... can't see a better way!

	if (_cluster_sectors < cluster_sectors)
		return -1;
	if (_cluster_sectors > cluster_sectors)
		return 1;

	if (_cluster_count < cluster_count)
		return -1;
	if (_cluster_count > cluster_count)
		return 1;

	return 0;
}

static PedSector
_get_create_size (PedSector upper_bound, FatType fat_type,
		  PedSector cluster_sectors, FatCluster cluster_count)
{
	PedSector	min_length = 0;
	PedSector	max_length = upper_bound;
	PedSector	length;

	while (1) {
		length = (min_length + max_length) / 2;
		switch (_test_create_size (length, fat_type, cluster_sectors,
					   cluster_count)) {
			case -1: min_length = length; break;
			case 0: return length;
			case 1: max_length = length; break;
		}
		/* hack... won't always be able to get max cluster count
		 * with max cluster size, etc. */
		if (max_length - min_length == 1)
			return min_length;
	}

	return 0;	/* shut gcc up */
}
#endif

PedConstraint*
fat_get_create_constraint_fat16 (const PedDevice* dev)
{
	PedGeometry	full_dev;
	PedSector	min_size;
	PedSector	max_size;

	if (!ped_geometry_init (&full_dev, dev, 0, dev->length - 1))
		return NULL;

#if 0
	min_size = _get_create_size (dev->length, FAT_TYPE_FAT16,
		       		     fat_min_cluster_size (FAT_TYPE_FAT16),
				     fat_min_cluster_count (FAT_TYPE_FAT16));
	max_size = _get_create_size (dev->length, FAT_TYPE_FAT16,
				     fat_max_cluster_size (FAT_TYPE_FAT16),
				     fat_max_cluster_count (FAT_TYPE_FAT16));
	if (!min_size)
		return NULL;
#else
	min_size = 65794;
	max_size = 2097153;
#endif

	return ped_constraint_new (
			ped_alignment_any, ped_alignment_any,
			&full_dev, &full_dev,
			min_size, max_size);
}

PedConstraint*
fat_get_create_constraint_fat32 (const PedDevice* dev)
{
	PedGeometry	full_dev;
	PedSector	min_size;

	if (!ped_geometry_init (&full_dev, dev, 0, dev->length - 1))
		return NULL;

#if 0
	min_size = _get_create_size (dev->length, FAT_TYPE_FAT32,
		       		     fat_min_cluster_size (FAT_TYPE_FAT32),
				     fat_min_cluster_count (FAT_TYPE_FAT32));
	if (!min_size)
		return NULL;
#else
	min_size = 525224;
#endif

	return ped_constraint_new (
			ped_alignment_any, ped_alignment_any,
			&full_dev, &full_dev,
			min_size, dev->length);
}
#endif /* !DISCOVER_ONLY */

#if 0

static PedFileSystemOps fat16_ops = {
	probe:		fat_probe_fat16,
#ifndef DISCOVER_ONLY
	clobber:	fat_clobber,
	open:		fat_open,
	create:		fat_create_fat16,
	close:		fat_close,
	check:		fat_check,
	resize:		fat_resize,
	copy:		fat_copy,
	get_create_constraint:	fat_get_create_constraint_fat16,
	get_resize_constraint:	fat_get_resize_constraint,
	get_copy_constraint:	fat_get_copy_constraint,
#else /* !DISCOVER_ONLY */
	clobber:	NULL,
	open:		NULL,
	create:		NULL,
	close:		NULL,
	check:		NULL,
	resize:		NULL,
	copy:		NULL,
	get_create_constraint:	NULL,
	get_resize_constraint:	NULL,
	get_copy_constraint:	NULL,
#endif /* !DISCOVER_ONLY */
};

static PedFileSystemOps fat32_ops = {
	probe:		fat_probe_fat32,
#ifndef DISCOVER_ONLY
	clobber:	fat_clobber,
	open:		fat_open,
	create:		fat_create_fat32,
	close:		fat_close,
	check:		fat_check,
	resize:		fat_resize,
	copy:		fat_copy,
	get_create_constraint:	fat_get_create_constraint_fat32,
	get_resize_constraint:	fat_get_resize_constraint,
	get_copy_constraint:	fat_get_copy_constraint,
#else /* !DISCOVER_ONLY */
	clobber:	NULL,
	open:		NULL,
	create:		NULL,
	close:		NULL,
	check:		NULL,
	resize:		NULL,
	copy:		NULL,
	get_create_constraint:	NULL,
	get_resize_constraint:	NULL,
	get_copy_constraint:	NULL,
#endif /* !DISCOVER_ONLY */
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

#endif
