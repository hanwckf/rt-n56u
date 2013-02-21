/*
    libparted
    Copyright (C) 1998-2000, 2002, 2007, 2009-2012 Free Software Foundation,
    Inc.

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
#include "fat.h"

#ifndef DISCOVER_ONLY

/* returns the minimum size of clusters for a given file system type */
PedSector
fat_min_cluster_size (FatType fat_type) {
	switch (fat_type) {
		case FAT_TYPE_FAT12: return 1;
		case FAT_TYPE_FAT16: return 1024/512;
		case FAT_TYPE_FAT32: return 4096/512;
	}
	return 0;
}

static PedSector
_smallest_power2_over (PedSector ceiling)
{
	PedSector	result = 1;

	while (result < ceiling)
		result *= 2;

	return result;
}

/* returns the minimum size of clusters for a given file system type */
PedSector
fat_recommend_min_cluster_size (FatType fat_type, PedSector size) {
	switch (fat_type) {
		case FAT_TYPE_FAT12: return 1;
		case FAT_TYPE_FAT16: return fat_min_cluster_size(fat_type);
		case FAT_TYPE_FAT32:
			return PED_MAX(_smallest_power2_over(size
						/ MAX_FAT32_CLUSTERS),
				       fat_min_cluster_size (fat_type));
	}
	return 0;
}

/* returns the maxmimum size of clusters for a given file system type */
PedSector
fat_max_cluster_size (FatType fat_type) {
	switch (fat_type) {
		case FAT_TYPE_FAT12: return 1;	/* dunno... who cares? */
		case FAT_TYPE_FAT16: return 65536/512;
		case FAT_TYPE_FAT32: return 65536/512;
	}
	return 0;
}

/* returns the minimum number of clusters for a given file system type */
FatCluster
fat_min_cluster_count (FatType fat_type) {
	switch (fat_type) {
		case FAT_TYPE_FAT12:
		case FAT_TYPE_FAT16:
			return fat_max_cluster_count (fat_type) / 2;

		case FAT_TYPE_FAT32: return 0xfff0;
	}
	return 0;
}

/* returns the maximum number of clusters for a given file system type */
FatCluster
fat_max_cluster_count (FatType fat_type) {
	switch (fat_type) {
		case FAT_TYPE_FAT12: return 0xff0;
		case FAT_TYPE_FAT16: return 0xfff0;
		case FAT_TYPE_FAT32: return 0x0ffffff0;
	}
	return 0;
}

/* what is this supposed to be?  What drugs are M$ on?  (Can I have some? :-) */
PedSector
fat_min_reserved_sector_count (FatType fat_type)
{
	return (fat_type == FAT_TYPE_FAT32) ? 32 : 1;
}

int
fat_check_resize_geometry (const PedFileSystem* fs,
			   const PedGeometry* geom,
			   PedSector new_cluster_sectors,
			   FatCluster new_cluster_count)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedSector	free_space;
	PedSector	min_free_space;
	PedSector	total_space;
	PedSector	new_total_space;
	PedSector	dir_space;

	PED_ASSERT (geom != NULL);

	dir_space = fs_info->total_dir_clusters * fs_info->cluster_sectors;
	free_space = fs_info->fat->free_cluster_count
			* fs_info->cluster_sectors;
	total_space = fs_info->fat->cluster_count * fs_info->cluster_sectors;
	new_total_space = new_cluster_count * new_cluster_sectors;
	min_free_space = total_space - new_total_space + dir_space;

	PED_ASSERT (new_cluster_count
		    <= fat_max_cluster_count (FAT_TYPE_FAT32));

	if (free_space < min_free_space) {
		char* needed = ped_unit_format (geom->dev, min_free_space);
		char* have = ped_unit_format (geom->dev, free_space);
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("You need %s of free disk space to shrink this "
			  "partition to this size.  Currently, only %s is "
			  "free."),
			needed, have);
		free (needed);
		free (have);
		return 0;
	}

	return 1;
}


/******************************************************************************/

/* DO NOT EDIT THIS ALGORITHM!
 * As far as I can tell, this is the same algorithm used by Microsoft to
 * calculate the size of the file allocaion tables, and the number of clusters.
 * I have not verified this by dissassembling Microsoft code - I came to this
 * conclusion by empirical analysis (i.e. trial and error - this was HORRIBLE).
 *
 * If you think this code makes no sense, then you are right.  I will restrain
 * the urge to inflict serious bodily harm on Microsoft people.
 */

static int
entries_per_sector (FatType fat_type)
{
	switch (fat_type) {
		case FAT_TYPE_FAT12:
			return 512 * 3 / 2;
		case FAT_TYPE_FAT16:
			return 512 / 2;
		case FAT_TYPE_FAT32:
			return 512 / 4;
	}
	return 0;
}

static int
calc_sizes (PedSector size, PedSector align, FatType fat_type,
	    PedSector root_dir_sectors, PedSector cluster_sectors,
	    FatCluster* out_cluster_count, PedSector* out_fat_size)
{
	PedSector	data_fat_space; /* space available to clusters + FAT */
	PedSector	fat_space;	/* space taken by each FAT */
	PedSector	cluster_space;	/* space taken by clusters */
	FatCluster	cluster_count;
	int		i;

	PED_ASSERT (out_cluster_count != NULL);
	PED_ASSERT (out_fat_size != NULL);

	data_fat_space = size - fat_min_reserved_sector_count (fat_type)
			 - align;
	if (fat_type == FAT_TYPE_FAT16)
		data_fat_space -= root_dir_sectors;

	fat_space = 0;
	for (i = 0; i < 2; i++) {
		if (fat_type == FAT_TYPE_FAT32)
			cluster_space = data_fat_space - fat_space;
		else
			cluster_space = data_fat_space - 2 * fat_space;

		cluster_count = cluster_space / cluster_sectors;
		fat_space = ped_div_round_up (cluster_count + 2,
					      entries_per_sector (fat_type));
	}

	cluster_space = data_fat_space - 2 * fat_space;
	cluster_count = cluster_space / cluster_sectors;

	/* looks like this should be part of the loop condition?
	 * Need to build the Big Table TM again to check
	 */
	if (fat_space < ped_div_round_up (cluster_count + 2,
				          entries_per_sector (fat_type))) {
		fat_space = ped_div_round_up (cluster_count + 2,
					      entries_per_sector (fat_type));
	}

	if (cluster_count > fat_max_cluster_count (fat_type)
	    || cluster_count < fat_min_cluster_count (fat_type))
		return 0;

	*out_cluster_count = cluster_count;
	*out_fat_size = fat_space;

	return 1;
}

/****************************************************************************/

int
fat_calc_sizes (PedSector size, PedSector align, FatType fat_type,
		PedSector root_dir_sectors,
		PedSector* out_cluster_sectors, FatCluster* out_cluster_count,
		PedSector* out_fat_size)
{
	PedSector	cluster_sectors;

	PED_ASSERT (out_cluster_sectors != NULL);
	PED_ASSERT (out_cluster_count != NULL);
	PED_ASSERT (out_fat_size != NULL);

	for (cluster_sectors = fat_recommend_min_cluster_size (fat_type, size);
	     cluster_sectors <= fat_max_cluster_size (fat_type);
	     cluster_sectors *= 2) {
		if (calc_sizes (size, align, fat_type, root_dir_sectors,
				cluster_sectors,
			        out_cluster_count, out_fat_size)) {
			*out_cluster_sectors = cluster_sectors;
			return 1;
		}
	}

	for (cluster_sectors = fat_recommend_min_cluster_size (fat_type, size);
	     cluster_sectors >= fat_min_cluster_size (fat_type);
	     cluster_sectors /= 2) {
		if (calc_sizes (size, align, fat_type, root_dir_sectors,
				cluster_sectors,
			        out_cluster_count, out_fat_size)) {
			*out_cluster_sectors = cluster_sectors;
			return 1;
		}
	}

	/* only make the cluster size really small (<4k) if a bigger one is
	 * isn't possible.  Windows never makes FS's like this, but it
	 * seems to work...  (do more tests!)
	 */
	for (cluster_sectors = 4; cluster_sectors > 0; cluster_sectors /= 2) {
		if (calc_sizes (size, align, fat_type, root_dir_sectors,
				cluster_sectors,
				out_cluster_count, out_fat_size)) {
			*out_cluster_sectors = cluster_sectors;
			return 1;
		}
	}

	return 0;
}

/* Same as fat_calc_sizes, except it only attempts to match a particular
 * cluster size.  This is useful, because the FAT resizer can only shrink the
 * cluster size.
 */
int
fat_calc_resize_sizes (
	const PedGeometry* geom,
	PedSector align,
	FatType fat_type,
	PedSector root_dir_sectors,
	PedSector cluster_sectors,
	PedSector* out_cluster_sectors,
	FatCluster* out_cluster_count,
	PedSector* out_fat_size)
{
	PED_ASSERT (geom != NULL);
	PED_ASSERT (out_cluster_sectors != NULL);
	PED_ASSERT (out_cluster_count != NULL);
	PED_ASSERT (out_fat_size != NULL);

/* libparted can only reduce the cluster size at this point */
	for (*out_cluster_sectors = cluster_sectors;
	     *out_cluster_sectors >= fat_min_cluster_size (fat_type);
	     *out_cluster_sectors /= 2) {
		if (calc_sizes (geom->length, align, fat_type, root_dir_sectors,
				*out_cluster_sectors,
				out_cluster_count, out_fat_size))
			return 1;
	}
	return 0;
}

/*  Calculates the number of sectors needed to be added to cluster_offset,
    to make the cluster on the new file system match up with the ones
    on the old file system.
	However, some space is reserved by fat_calc_resize_sizes() and
    friends, to allow room for this space.  If too much of this space is left
    over, everyone will complain, so we have to be greedy, and use it all up...
 */
PedSector
fat_calc_align_sectors (const PedFileSystem* new_fs,
			const PedFileSystem* old_fs)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (old_fs);
	FatSpecific*	new_fs_info = FAT_SPECIFIC (new_fs);
	PedSector	raw_old_meta_data_end;
	PedSector	new_meta_data_size;
	PedSector	min_new_meta_data_end;
	PedSector	new_data_size;
	PedSector	new_clusters_size;
	PedSector	align;

	new_meta_data_size
		= fat_min_reserved_sector_count (new_fs_info->fat_type)
		  + new_fs_info->fat_sectors * 2;

	if (new_fs_info->fat_type == FAT_TYPE_FAT16)
		new_meta_data_size += new_fs_info->root_dir_sector_count;

	raw_old_meta_data_end = old_fs->geom->start
				 + old_fs_info->cluster_offset;

	min_new_meta_data_end = new_fs->geom->start + new_meta_data_size;

	if (raw_old_meta_data_end > min_new_meta_data_end)
		align = (raw_old_meta_data_end - min_new_meta_data_end)
			% new_fs_info->cluster_sectors;
	else
		align = (new_fs_info->cluster_sectors
		         - (   (min_new_meta_data_end - raw_old_meta_data_end)
				% new_fs_info->cluster_sectors   ))
			% new_fs_info->cluster_sectors;

	new_data_size = new_fs->geom->length - new_meta_data_size;
	new_clusters_size = new_fs_info->cluster_count
				* new_fs_info->cluster_sectors;

	while (new_clusters_size + align + new_fs_info->cluster_sectors
			<= new_data_size)
		align += new_fs_info->cluster_sectors;

	return align;
}

int
fat_is_sector_in_clusters (const PedFileSystem* fs, PedSector sector)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	return sector >= fs_info->cluster_offset
	       && sector < fs_info->cluster_offset
	      		   + fs_info->cluster_sectors * fs_info->cluster_count;
}

FatFragment
fat_cluster_to_frag (const PedFileSystem* fs, FatCluster cluster)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (cluster >= 2 && cluster < fs_info->cluster_count + 2);

	return (cluster - 2) * fs_info->cluster_frags;
}

FatCluster
fat_frag_to_cluster (const PedFileSystem* fs, FatFragment frag)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (frag >= 0 && frag < fs_info->frag_count);

	return frag / fs_info->cluster_frags + 2;
}

PedSector
fat_frag_to_sector (const PedFileSystem* fs, FatFragment frag)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (frag >= 0 && frag < fs_info->frag_count);

	return frag * fs_info->frag_sectors + fs_info->cluster_offset;
}

FatFragment
fat_sector_to_frag (const PedFileSystem* fs, PedSector sector)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (sector >= fs_info->cluster_offset);

	return (sector - fs_info->cluster_offset) / fs_info->frag_sectors;
}

PedSector
fat_cluster_to_sector (const PedFileSystem* fs, FatCluster cluster)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (cluster >= 2 && cluster < fs_info->cluster_count + 2);

	return (cluster - 2) * fs_info->cluster_sectors
		+ fs_info->cluster_offset;
}

FatCluster
fat_sector_to_cluster (const PedFileSystem* fs, PedSector sector)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);

	PED_ASSERT (sector >= fs_info->cluster_offset);

	return (sector - fs_info->cluster_offset) / fs_info->cluster_sectors
		+ 2;
}
#endif /* !DISCOVER_ONLY */
