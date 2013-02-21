/*
    libparted
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

#include <config.h>
#include <parted/endian.h>
#include "fat.h"

#ifndef DISCOVER_ONLY

FatTable*
fat_table_new (FatType fat_type, FatCluster size)
{
	FatTable*	ft;
	int		entry_size = fat_table_entry_size (fat_type);

	ft = (FatTable*) ped_malloc (sizeof (FatTable));
	if (!ft) return NULL;

	ft->cluster_count = ft->free_cluster_count = size - 2;

/* ensure there's some free room on the end, to finish off the sector */
	ft->size = ped_div_round_up (size * entry_size, 512) * 512 / entry_size;
	ft->fat_type = fat_type;
	ft->raw_size = ft->size * entry_size;

	ft->table = ped_malloc (ft->raw_size);
	if (!ft->table) {
		free (ft);
		return NULL;
	}

	fat_table_clear (ft);
	return ft;
}

void
fat_table_destroy (FatTable* ft)
{
	free (ft->table);
	free (ft);
}

FatTable*
fat_table_duplicate (const FatTable* ft)
{
	FatTable*	dup_ft;

	dup_ft = fat_table_new (ft->fat_type, ft->size);
	if (!dup_ft) return NULL;

	dup_ft->cluster_count	= ft->cluster_count;
	dup_ft->free_cluster_count	= ft->free_cluster_count;
	dup_ft->bad_cluster_count	= ft->bad_cluster_count;
	dup_ft->last_alloc		= ft->last_alloc;

	memcpy (dup_ft->table, ft->table, ft->raw_size);

	return dup_ft;
}

void
fat_table_clear (FatTable* ft)
{
	memset (ft->table, 0, ft->raw_size);

	fat_table_set (ft, 0, 0x0ffffff8);
	fat_table_set (ft, 1, 0x0fffffff);

	ft->free_cluster_count = ft->cluster_count;
	ft->bad_cluster_count = 0;
	ft->last_alloc = 1;
}

int
fat_table_set_cluster_count (FatTable* ft, FatCluster new_cluster_count)
{
	PED_ASSERT (new_cluster_count + 2 <= ft->size);

	ft->cluster_count = new_cluster_count;
	return fat_table_count_stats (ft);
}

int
fat_table_count_stats (FatTable* ft)
{
	FatCluster	i;

	PED_ASSERT (ft->cluster_count + 2 <= ft->size);

	ft->free_cluster_count = 0;
	ft->bad_cluster_count = 0;

	for (i=2; i < ft->cluster_count + 2; i++) {
		if (fat_table_is_available (ft, i))
			ft->free_cluster_count++;
		if (fat_table_is_bad (ft, i))
			ft->bad_cluster_count++;
	}
	return 1;
}

int
fat_table_read (FatTable* ft, const PedFileSystem* fs, int table_num)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (ft->raw_size >= fs_info->fat_sectors * 512);

	memset (ft->table, 0, ft->raw_size);

        if (!ped_geometry_read (fs->geom, (void *) ft->table,
				fs_info->fat_offset
					+ table_num * fs_info->fat_sectors,
				fs_info->fat_sectors))
		return 0;

        if ( *((unsigned char*) ft->table) != fs_info->boot_sector.media) {
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("FAT %d media %x doesn't match the boot sector's "
			  "media %x.  You should probably run scandisk."),
			(int) table_num + 1,
			(int) *((unsigned char*) ft->table),
			(int) fs_info->boot_sector.media)
				!= PED_EXCEPTION_IGNORE)
			return 0;
        }

	ft->cluster_count = fs_info->cluster_count;

	fat_table_count_stats (ft);

	return 1;
}

int
fat_table_write (const FatTable* ft, PedFileSystem* fs, int table_num)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (ft->raw_size >= fs_info->fat_sectors * 512);

        if (!ped_geometry_write (fs->geom, (void *) ft->table,
				 fs_info->fat_offset
					+ table_num * fs_info->fat_sectors,
				 fs_info->fat_sectors))
		return 0;
	if (!ped_geometry_sync (fs->geom))
		return 0;

	return 1;
}

int
fat_table_write_all (const FatTable* ft, PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	int		i;

	for (i = 0; i < fs_info->fat_table_count; i++) {
		if (!fat_table_write (ft, fs, i))
			return 0;
	}

	return 1;
}

int
fat_table_compare (const FatTable* a, const FatTable* b)
{
	FatCluster	i;

	if (a->cluster_count != b->cluster_count)
		return 0;

	for (i = 0; i < a->cluster_count + 2; i++) {
		if (fat_table_get (a, i) != fat_table_get (b, i))
			return 0;
	}

	return 1;
}

static int
_test_code_available (const FatTable* ft, FatCluster code)
{
	return code == 0;
}

static int
_test_code_bad (const FatTable* ft, FatCluster code)
{
	switch (ft->fat_type) {
                case FAT_TYPE_FAT12:
                if (code == 0xff7) return 1;
                break;

		case FAT_TYPE_FAT16:
		if (code == 0xfff7) return 1;
		break;

		case FAT_TYPE_FAT32:
		if (code == 0x0ffffff7) return 1;
		break;
	}
	return 0;
}

static int
_test_code_eof (const FatTable* ft, FatCluster code)
{
	switch (ft->fat_type) {
                case FAT_TYPE_FAT12:
                if (code >= 0xff7) return 1;
                break;

		case FAT_TYPE_FAT16:
		if (code >= 0xfff7) return 1;
		break;

		case FAT_TYPE_FAT32:
		if (code >= 0x0ffffff7) return 1;
		break;
	}
	return 0;
}

void
_update_stats (FatTable* ft, FatCluster cluster, FatCluster value)
{
	if (_test_code_available (ft, value)
	    && !fat_table_is_available (ft, cluster)) {
		ft->free_cluster_count++;
		if (fat_table_is_bad (ft, cluster))
			ft->bad_cluster_count--;
	}

	if (!_test_code_available (ft, value)
	    && fat_table_is_available (ft, cluster)) {
		ft->free_cluster_count--;
		if (_test_code_bad (ft, cluster))
			ft->bad_cluster_count--;
	}
}

int
fat_table_set (FatTable* ft, FatCluster cluster, FatCluster value)
{
	if (cluster >= ft->cluster_count + 2) {
		ped_exception_throw (PED_EXCEPTION_BUG,
				     PED_EXCEPTION_CANCEL,
				     _("fat_table_set: cluster %ld outside "
				       "file system"),
				     (long) cluster);
		return 0;
	}

	_update_stats (ft, cluster, value);

	switch (ft->fat_type) {
                case FAT_TYPE_FAT12:
                PED_ASSERT (0);
                break;

		case FAT_TYPE_FAT16:
		((unsigned short *) ft->table) [cluster]
			= PED_CPU_TO_LE16 (value);
		break;

		case FAT_TYPE_FAT32:
		((unsigned int *) ft->table) [cluster]
			= PED_CPU_TO_LE32 (value);
		break;
	}
	return 1;
}

FatCluster
fat_table_get (const FatTable* ft, FatCluster cluster)
{
	if (cluster >= ft->cluster_count + 2) {
		ped_exception_throw (PED_EXCEPTION_BUG,
				     PED_EXCEPTION_CANCEL,
				     _("fat_table_get: cluster %ld outside "
				       "file system"),
				     (long) cluster);
		exit (EXIT_FAILURE);	/* FIXME */
	}

	switch (ft->fat_type) {
                case FAT_TYPE_FAT12:
                PED_ASSERT (0);
                break;

		case FAT_TYPE_FAT16:
		return PED_LE16_TO_CPU
			(((unsigned short *) ft->table) [cluster]);

		case FAT_TYPE_FAT32:
		return PED_LE32_TO_CPU
			(((unsigned int *) ft->table) [cluster]);
	}

	return 0;
}

FatCluster
fat_table_alloc_cluster (FatTable* ft)
{
	FatCluster	i;
	FatCluster	cluster;

/* hack: assumes the first two FAT entries are marked as used (which they
 * always should be)
 */
	for (i=1; i < ft->cluster_count + 1; i++) {
		cluster = (i + ft->last_alloc) % ft->cluster_count;
		if (fat_table_is_available (ft, cluster)) {
			ft->last_alloc = cluster;
			return cluster;
		}
	}

	ped_exception_throw (PED_EXCEPTION_ERROR,
			     PED_EXCEPTION_CANCEL,
			     _("fat_table_alloc_cluster: no free clusters"));
	return 0;
}

FatCluster
fat_table_alloc_check_cluster (FatTable* ft, PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	result;

	while (1) {
		result = fat_table_alloc_cluster (ft);
		if (!result)
			return 0;
		if (fat_read_cluster (fs, fs_info->buffer, result))
			return result;
		fat_table_set_bad (ft, result);
	}
}

/*
    returns true if <cluster> is marked as bad
*/
int
fat_table_is_bad (const FatTable* ft, FatCluster cluster)
{
	return _test_code_bad (ft, fat_table_get (ft, cluster));
}

/*
    returns true if <cluster> represents an EOF marker
*/
int
fat_table_is_eof (const FatTable* ft, FatCluster cluster)
{
	return _test_code_eof (ft, cluster);
}

/*
    returns true if <cluster> is available.
*/
int
fat_table_is_available (const FatTable* ft, FatCluster cluster)
{
	return _test_code_available (ft, fat_table_get (ft, cluster));
}

/*
    returns true if <cluster> is empty.  Note that this includes bad clusters.
*/
int
fat_table_is_empty (const FatTable* ft, FatCluster cluster)
{
	return fat_table_is_available (ft, cluster)
		|| fat_table_is_bad (ft, cluster);
}

/*
    returns true if <cluster> is being used for something constructive.
*/
int
fat_table_is_active (const FatTable* ft, FatCluster cluster)
{
	return !fat_table_is_bad (ft, cluster)
		&& !fat_table_is_available (ft, cluster);
}

/*
    marks <cluster> as the last cluster in the chain
*/
int
fat_table_set_eof (FatTable* ft, FatCluster cluster)
{

	switch (ft->fat_type) {
                case FAT_TYPE_FAT12:
                PED_ASSERT (0);
                break;

		case FAT_TYPE_FAT16:
		return fat_table_set (ft, cluster, 0xfff8);

		case FAT_TYPE_FAT32:
		return fat_table_set (ft, cluster, 0x0fffffff);
	}

	return 0;
}

/*
	Marks a clusters as unusable, due to physical disk damage.
*/
int
fat_table_set_bad (FatTable* ft, FatCluster cluster)
{
	if (!fat_table_is_bad (ft, cluster))
		ft->bad_cluster_count++;

	switch (ft->fat_type) {
                case FAT_TYPE_FAT12:
		return fat_table_set (ft, cluster, 0xff7);

		case FAT_TYPE_FAT16:
		return fat_table_set (ft, cluster, 0xfff7);

		case FAT_TYPE_FAT32:
		return fat_table_set (ft, cluster, 0x0ffffff7);
	}

	return 0;
}

/*
    marks <cluster> as unused/free/available
*/
int
fat_table_set_avail (FatTable* ft, FatCluster cluster)
{
	return fat_table_set (ft, cluster, 0);
}

#endif /* !DISCOVER_ONLY */

int
fat_table_entry_size (FatType fat_type)
{
	switch (fat_type) {
		case FAT_TYPE_FAT12:
		return 2;		/* FIXME: how? */

		case FAT_TYPE_FAT16:
		return 2;

		case FAT_TYPE_FAT32:
		return 4;
	}

	return 0;
}
