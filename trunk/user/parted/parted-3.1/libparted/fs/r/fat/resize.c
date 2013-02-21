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
#include "fat.h"
#include "traverse.h"
#include "count.h"
#include "fatio.h"
#include "calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#ifndef DISCOVER_ONLY

/* Recursively builds (i.e. makes consistent) the duplicated directory tree
 * (leaving the original directory tree in tact)
 */
static int
fat_construct_directory (FatOpContext* ctx, FatTraverseInfo* trav_info)
{
	FatTraverseInfo*	sub_dir_info;
	FatDirEntry*		dir_entry;
	FatCluster		old_first_cluster;

	while ( (dir_entry = fat_traverse_next_dir_entry (trav_info)) ) {
		if (fat_dir_entry_is_null_term (dir_entry))
			break;
		if (!fat_dir_entry_has_first_cluster (dir_entry, ctx->old_fs))
			continue;

		fat_traverse_mark_dirty (trav_info);

		old_first_cluster = fat_dir_entry_get_first_cluster (dir_entry,
						ctx->old_fs);
		fat_dir_entry_set_first_cluster (dir_entry, ctx->new_fs,
			fat_op_context_map_cluster (ctx, old_first_cluster));

		if (fat_dir_entry_is_directory (dir_entry)
				&& dir_entry->name [0] != '.') {
			sub_dir_info
				= fat_traverse_directory (trav_info, dir_entry);
			if (!sub_dir_info)
				return 0;
			if (!fat_construct_directory (ctx, sub_dir_info))
				return 0;
		}
	}
	/* remove "stale" entries at the end */
	while ((dir_entry = fat_traverse_next_dir_entry (trav_info))) {
		memset (dir_entry, 0, sizeof (FatDirEntry));
		fat_traverse_mark_dirty (trav_info);
	}
	fat_traverse_complete (trav_info);
	return 1;
}

static int
duplicate_legacy_root_dir (FatOpContext* ctx)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);

	PED_ASSERT (old_fs_info->root_dir_sector_count
			== new_fs_info->root_dir_sector_count);

	if (!ped_geometry_read (ctx->old_fs->geom, old_fs_info->buffer,
				old_fs_info->root_dir_offset,
				old_fs_info->root_dir_sector_count))
		return 0;

	if (!ped_geometry_write (ctx->new_fs->geom, old_fs_info->buffer,
				 new_fs_info->root_dir_offset,
				 new_fs_info->root_dir_sector_count))
		return 0;

	return 1;
}

/*
    Constructs the new directory tree for legacy (FAT16) file systems.
*/
static int
fat_construct_legacy_root (FatOpContext* ctx)
{
	FatTraverseInfo*	trav_info;

	if (!duplicate_legacy_root_dir (ctx))
		return 0;
	trav_info = fat_traverse_begin (ctx->new_fs, FAT_ROOT, "\\");
	return fat_construct_directory (ctx, trav_info);
}

/*
    Constructs the new directory tree for new (FAT32) file systems.
*/
static int
fat_construct_root (FatOpContext* ctx)
{
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatTraverseInfo*	trav_info;

	trav_info = fat_traverse_begin (ctx->new_fs, new_fs_info->root_cluster,
					"\\");
	fat_construct_directory (ctx, trav_info);
	return 1;
}

/* Converts the root directory between FAT16 and FAT32.  NOTE: this code
 * can also do no conversion.  I'm leaving fat_construct_directory(), because
 * it's really pretty :-)  It also leaves a higher chance of deleted file
 * recovery, because it doesn't remove redundant entries.  (We do this here,
 * because brain-damaged FAT16 has an arbitary limit on root directory entries,
 * so we save room)
 */
static int
fat_convert_directory (FatOpContext* ctx, FatTraverseInfo* old_trav,
		       FatTraverseInfo* new_trav)
{
	FatTraverseInfo*	sub_old_dir_trav;
	FatTraverseInfo*	sub_new_dir_trav;
	FatDirEntry*		new_dir_entry;
	FatDirEntry*		old_dir_entry;
	FatCluster		old_first_cluster;

	while ( (old_dir_entry = fat_traverse_next_dir_entry (old_trav)) ) {
		if (fat_dir_entry_is_null_term (old_dir_entry))
			break;
		if (!fat_dir_entry_is_active (old_dir_entry))
			continue;

		new_dir_entry = fat_traverse_next_dir_entry (new_trav);
		if (!new_dir_entry) {
			return ped_exception_throw (PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("There's not enough room in the root "
				  "directory for all of the files.  Either "
				  "cancel, or ignore to lose the files."))
					== PED_EXCEPTION_IGNORE;
		}

		*new_dir_entry = *old_dir_entry;
		fat_traverse_mark_dirty (new_trav);

		if (!fat_dir_entry_has_first_cluster (old_dir_entry,
						      ctx->old_fs))
			continue;

		old_first_cluster = fat_dir_entry_get_first_cluster (
						old_dir_entry, ctx->old_fs);
		fat_dir_entry_set_first_cluster (new_dir_entry, ctx->new_fs,
			fat_op_context_map_cluster (ctx, old_first_cluster));

		if (fat_dir_entry_is_directory (old_dir_entry)
				&& old_dir_entry->name [0] != '.') {
			sub_old_dir_trav
			    = fat_traverse_directory (old_trav, old_dir_entry);
			sub_new_dir_trav
			    = fat_traverse_directory (new_trav, new_dir_entry);
			if (!sub_old_dir_trav || !sub_new_dir_trav)
				return 0;

			if (!fat_convert_directory (ctx, sub_old_dir_trav,
						    sub_new_dir_trav))
				return 0;
		}
	}

	/* remove "stale" entries at the end, just in case there is some
	 * overlap
	 */
	while ((new_dir_entry = fat_traverse_next_dir_entry (new_trav))) {
		memset (new_dir_entry, 0, sizeof (FatDirEntry));
		fat_traverse_mark_dirty (new_trav);
	}

	fat_traverse_complete (old_trav);
	fat_traverse_complete (new_trav);
	return 1;
}

static void
clear_cluster (PedFileSystem* fs, FatCluster cluster)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);

	memset (fs_info->buffer, 0, fs_info->cluster_size);
	fat_write_cluster (fs, fs_info->buffer, cluster);
}

/* This MUST be called BEFORE the fat_construct_new_fat(), because cluster
 * allocation depend on the old FAT.  The reason is, old clusters may
 * still be needed during the resize, (particularly clusters in the directory
 * tree) even if they will be discarded later.
 */
static int
alloc_root_dir (FatOpContext* ctx)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatCluster		i;
	FatCluster		cluster;
	FatCluster		cluster_count;

	PED_ASSERT (new_fs_info->fat_type == FAT_TYPE_FAT32);

	cluster_count = ped_div_round_up (
			   PED_MAX (16, old_fs_info->root_dir_sector_count),
			   new_fs_info->cluster_sectors);

	for (i = 0; i < cluster_count; i++) {
		cluster = fat_table_alloc_check_cluster (new_fs_info->fat,
							 ctx->new_fs);
		if (!cluster)
			return 0;
		ctx->new_root_dir [i] = cluster;
		clear_cluster (ctx->new_fs, cluster);
	}
	ctx->new_root_dir [i] = 0;
	new_fs_info->root_cluster = ctx->new_root_dir [0];
	return 1;
}

/* when converting FAT32 -> FAT16
 * fat_duplicate clusters() duplicated the root directory unnecessarily.
 * Let's free it.
 *
 * This must be called AFTER fat_construct_new_fat().  (otherwise, our
 * changes just get overwritten)
 */
static int
free_root_dir (FatOpContext* ctx)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatCluster		old_cluster;
	FatFragment		i;

	PED_ASSERT (old_fs_info->fat_type == FAT_TYPE_FAT32);
	PED_ASSERT (new_fs_info->fat_type == FAT_TYPE_FAT16);

	for (old_cluster = old_fs_info->root_cluster;
	     !fat_table_is_eof (old_fs_info->fat, old_cluster);
	     old_cluster = fat_table_get (old_fs_info->fat, old_cluster)) {
		FatFragment old_frag;
		old_frag = fat_cluster_to_frag (ctx->old_fs, old_cluster);
		for (i = 0; i < new_fs_info->cluster_frags; i++) {
			FatFragment new_frag;
			FatCluster new_clst;
			new_frag = fat_op_context_map_fragment (ctx,
								old_frag + i);
			new_clst = fat_frag_to_cluster (ctx->old_fs, new_frag);
			if (!fat_table_set_avail (new_fs_info->fat, new_clst))
				return 0;
		}
	}

	return 1;
}

static int
fat_clear_root_dir (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	int		i;

	PED_ASSERT (fs_info->fat_type == FAT_TYPE_FAT16);
	PED_ASSERT (fs_info->root_dir_sector_count);

	memset (fs_info->buffer, 0, 512);

	for (i = 0; i < fs_info->root_dir_sector_count; i++) {
		if (!ped_geometry_write (fs->geom, fs_info->buffer,
					 fs_info->root_dir_offset + i, 1)) {
			if (ped_exception_throw (PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("Error writing to the root directory."))
					== PED_EXCEPTION_CANCEL)
				return 0;
		}
	}
	return 1;
}

static int
fat_construct_converted_tree (FatOpContext* ctx)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatTraverseInfo*	old_trav_info;
	FatTraverseInfo*	new_trav_info;

	if (new_fs_info->fat_type == FAT_TYPE_FAT32) {
		new_trav_info = fat_traverse_begin (ctx->new_fs,
					    new_fs_info->root_cluster, "\\");
		old_trav_info = fat_traverse_begin (ctx->old_fs, FAT_ROOT,
						    "\\");
	} else {
		fat_clear_root_dir (ctx->new_fs);
		new_trav_info = fat_traverse_begin (ctx->new_fs, FAT_ROOT,
						    "\\");
		old_trav_info = fat_traverse_begin (ctx->old_fs,
					    old_fs_info->root_cluster, "\\");
	}
	if (!new_trav_info || !old_trav_info)
		return 0;
	if (!fat_convert_directory (ctx, old_trav_info, new_trav_info))
		return 0;
	return 1;
}

/*
    Constructs the new directory tree to match the new file locations.
*/
static int
fat_construct_dir_tree (FatOpContext* ctx)
{
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);

	if (new_fs_info->fat_type == old_fs_info->fat_type) {
		switch (old_fs_info->fat_type) {
                        case FAT_TYPE_FAT12:
                        PED_ASSERT (0);
                        break;

			case FAT_TYPE_FAT16:
			return fat_construct_legacy_root (ctx);

			case FAT_TYPE_FAT32:
			return fat_construct_root (ctx);
		}
	} else {
		return fat_construct_converted_tree (ctx);
	}

	return 0;
}

static FatFragment
_get_next_old_frag (FatOpContext* ctx, FatFragment frag)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatCluster	cluster;
	FatCluster	next_cluster;

	if ((frag + 1) % old_fs_info->cluster_frags != 0) {
		if (fat_is_fragment_active (ctx->old_fs, frag + 1))
			return frag + 1;
		else
			return -1;
	} else {
		cluster = fat_frag_to_cluster (ctx->old_fs, frag);
		next_cluster = fat_table_get (old_fs_info->fat, cluster);

		if (fat_table_is_eof (old_fs_info->fat, next_cluster))
			return -1;
		else
			return fat_cluster_to_frag (ctx->old_fs, next_cluster);
	}
}

/*
    Constructs the new fat for the resized file system.
*/
static int
fat_construct_new_fat (FatOpContext* ctx)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*	new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatFragment	old_frag;
	FatCluster	new_cluster;
	FatFragment	new_frag;
	FatFragment	old_next_frag;
	FatFragment	new_next_frag;
	FatCluster	new_next_cluster;
	FatClusterFlag	flag;
	int		i;

	fat_table_clear (new_fs_info->fat);
	if (!fat_table_set_cluster_count (new_fs_info->fat,
					  new_fs_info->cluster_count))
		return 0;

	for (old_frag = 0; old_frag < old_fs_info->frag_count; old_frag++) {
		flag = fat_get_fragment_flag (ctx->old_fs, old_frag);
		if (flag == FAT_FLAG_FREE)
			continue;
		if (flag == FAT_FLAG_BAD) {
			new_frag = fat_op_context_map_static_fragment (
						ctx, old_frag);
			if (new_frag == -1)
				continue;
			new_cluster = fat_frag_to_cluster (ctx->new_fs,
							   new_frag);
			fat_table_set_bad (new_fs_info->fat, new_cluster);
			continue;
		}

		new_frag = fat_op_context_map_fragment (ctx, old_frag);
		new_cluster = fat_frag_to_cluster (ctx->new_fs, new_frag);

		old_next_frag = _get_next_old_frag (ctx, old_frag);
		if (old_next_frag == -1) {
			fat_table_set_eof (new_fs_info->fat, new_cluster);
			continue;
		}

		new_next_frag = fat_op_context_map_fragment (ctx,
							     old_next_frag);
		PED_ASSERT (new_next_frag != -1);

		new_next_cluster = fat_frag_to_cluster (ctx->new_fs,
							new_next_frag);
		PED_ASSERT (new_next_cluster != new_cluster);

		fat_table_set (new_fs_info->fat, new_cluster, new_next_cluster);
	}

#if 0
#ifdef PED_VERBOSE
	for (old_cluster=2; old_cluster < old_fs_info->cluster_count+2;
	     old_cluster++) {
		if (fat_table_is_available (old_fs_info->fat, old_cluster))
			continue;

		printf ("%d->%d\t(next: %d->%d)\n",
			old_cluster,
			ctx->remap [old_cluster],
			fat_table_get (old_fs_info->fat, old_cluster),
			fat_table_get (new_fs_info->fat,
				       ctx->remap [old_cluster]));
	}
#endif /* PED_VERBOSE */
#endif

	if (old_fs_info->fat_type == FAT_TYPE_FAT32
	    && new_fs_info->fat_type == FAT_TYPE_FAT32) {
		new_fs_info->root_cluster
			= fat_op_context_map_cluster (ctx,
					old_fs_info->root_cluster);
	}

	if (old_fs_info->fat_type == FAT_TYPE_FAT16
	    && new_fs_info->fat_type == FAT_TYPE_FAT32) {
		for (i=0; ctx->new_root_dir[i+1]; i++) {
			fat_table_set (new_fs_info->fat,
				       ctx->new_root_dir[i],
				       ctx->new_root_dir[i+1]);
		}
		fat_table_set_eof (new_fs_info->fat, ctx->new_root_dir[i]);
	}

	return 1;
}

static int
ask_type (PedFileSystem* fs, int fat16_ok, int fat32_ok, FatType* out_fat_type)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);
	PedExceptionOption	status;
	const char*		fat16_msg;
	const char*		fat32_msg;

	if (fs_info->fat_type == FAT_TYPE_FAT16)
		fat16_msg = _("If you leave your file system as FAT16, "
			      "then you will have no problems.");
	else
		fat16_msg = _("If you convert to FAT16, and MS Windows "
			      "is installed on this partition, then "
			      "you must re-install the MS Windows boot "
			      "loader.  If you want to do this, you "
			      "should consult the Parted manual (or "
			      "your distribution's manual).");

	if (fs_info->fat_type == FAT_TYPE_FAT32)
		fat32_msg = _("If you leave your file system as FAT32, "
			      "then you will not introduce any new "
			      "problems.");
	else
		fat32_msg = _("If you convert to FAT32, and MS Windows "
			      "is installed on this partition, then "
			      "you must re-install the MS Windows boot "
			      "loader.  If you want to do this, you "
			      "should consult the Parted manual (or "
			      "your distribution's manual).  Also, "
			      "converting to FAT32 will make the file "
			      "system unreadable by MS DOS, MS Windows "
			      "95a, and MS Windows NT.");

	if (fat16_ok && fat32_ok) {
		status = ped_exception_throw (
			 PED_EXCEPTION_INFORMATION,
			 PED_EXCEPTION_YES_NO_CANCEL,
			 _("%s  %s  %s"),
			 _("Would you like to use FAT32?"),
			 fat16_msg,
			 fat32_msg);

		switch (status) {
		case PED_EXCEPTION_YES:
			*out_fat_type = FAT_TYPE_FAT32;
			return 1;

		case PED_EXCEPTION_NO:
			*out_fat_type = FAT_TYPE_FAT16;
			return 1;

		case PED_EXCEPTION_UNHANDLED:
			*out_fat_type = fs_info->fat_type;
			return 1;

		case PED_EXCEPTION_CANCEL:
			return 0;

                default:
                        PED_ASSERT (0);
                        break;
		}
	}

	if (fat16_ok) {
		if (fs_info->fat_type != FAT_TYPE_FAT16) {
			status = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_OK_CANCEL,
				_("%s  %s"),
				_("The file system can only be resized to this "
				  "size by converting to FAT16."),
				fat16_msg);
			if (status == PED_EXCEPTION_CANCEL)
				return 0;
		}
		*out_fat_type = FAT_TYPE_FAT16;
		return 1;
	}

	if (fat32_ok) {
		if (fs_info->fat_type != FAT_TYPE_FAT32) {
			status = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_OK_CANCEL,
				_("%s  %s"),
				_("The file system can only be resized to this "
				  "size by converting to FAT32."),
				fat32_msg);
			if (status == PED_EXCEPTION_CANCEL)
				return 0;
		}
		*out_fat_type = FAT_TYPE_FAT32;
		return 1;
	}

	ped_exception_throw (
		PED_EXCEPTION_NO_FEATURE,
		PED_EXCEPTION_CANCEL,
		_("GNU Parted cannot resize this partition to this size.  "
		  "We're working on it!"));

	return 0;
}

/*  For resize operations: determine if the file system must be FAT16 or FAT32,
 *  or either.  If the new file system must be FAT32, then query for
 *  confirmation.  If either file system can be used, query for which one.
 */
static int
get_fat_type (PedFileSystem* fs, const PedGeometry* new_geom,
	      FatType* out_fat_type)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);
	PedSector		fat16_cluster_sectors;
	PedSector		fat32_cluster_sectors;
	FatCluster		dummy_cluster_count;
	PedSector		dummy_fat_sectors;
	int			fat16_ok;
	int			fat32_ok;

	fat16_ok = fat_calc_resize_sizes (
				    new_geom,
				    fs_info->cluster_sectors,
				    FAT_TYPE_FAT16,
				    fs_info->root_dir_sector_count,
				    fs_info->cluster_sectors,
				    &fat16_cluster_sectors,
				    &dummy_cluster_count,
				    &dummy_fat_sectors);

	fat32_ok = fat_calc_resize_sizes (
				    new_geom,
				    fs_info->cluster_sectors,
				    FAT_TYPE_FAT32,
				    fs_info->root_dir_sector_count,
				    fs_info->cluster_sectors,
				    &fat32_cluster_sectors,
				    &dummy_cluster_count,
				    &dummy_fat_sectors);

	return ask_type (fs, fat16_ok, fat32_ok, out_fat_type);
}

/*  Creates the PedFileSystem struct for the new resized file system, and
    sticks it in a FatOpContext.  At the end of the process, the original
    (ctx->old_fs) is destroyed, and replaced with the new one (ctx->new_fs).
 */
static FatOpContext*
create_resize_context (PedFileSystem* fs, const PedGeometry* new_geom)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatSpecific*	new_fs_info;
	PedFileSystem*	new_fs;
	PedSector	new_cluster_sectors;
	FatCluster	new_cluster_count;
	PedSector	new_fat_sectors;
	FatType		new_fat_type;
	PedSector	root_dir_sector_count;
	FatOpContext*	context;

	/* hypothetical number of root dir sectors, if we end up using
	 * FAT16
	 */
	if (fs_info->root_dir_sector_count)
		root_dir_sector_count = fs_info->root_dir_sector_count;
	else
		root_dir_sector_count = FAT_ROOT_DIR_ENTRY_COUNT
						* sizeof (FatDirEntry) / 512;

	if (!get_fat_type (fs, new_geom, &new_fat_type))
		return 0;

	fat_calc_resize_sizes (new_geom, fs_info->cluster_sectors, new_fat_type,
		root_dir_sector_count, fs_info->cluster_sectors,
		&new_cluster_sectors, &new_cluster_count, &new_fat_sectors);

	if (!fat_check_resize_geometry (fs, new_geom, new_cluster_sectors,
				        new_cluster_count))
		goto error;

	new_fs = fat_alloc (new_geom);
	if (!new_fs)
		goto error;

	new_fs_info = FAT_SPECIFIC (new_fs);
	if (!new_fs_info)
		goto error_free_new_fs;

/* preserve boot code, etc. */
	memcpy (&new_fs_info->boot_sector, &fs_info->boot_sector,
		sizeof (FatBootSector));
	memcpy (&new_fs_info->info_sector, &fs_info->info_sector,
		sizeof (FatInfoSector));

	new_fs_info->logical_sector_size = fs_info->logical_sector_size;
	new_fs_info->sector_count = new_geom->length;

	new_fs_info->sectors_per_track = fs_info->sectors_per_track;
	new_fs_info->heads = fs_info->heads;

	new_fs_info->cluster_size = new_cluster_sectors * 512;
	new_fs_info->cluster_sectors = new_cluster_sectors;
	new_fs_info->cluster_count = new_cluster_count;
	new_fs_info->dir_entries_per_cluster = fs_info->dir_entries_per_cluster;

	new_fs_info->fat_type = new_fat_type;
	new_fs_info->fat_table_count = 2;
	new_fs_info->fat_sectors = new_fat_sectors;

	/* what about copying? */
	new_fs_info->serial_number = fs_info->serial_number;

	if (new_fs_info->fat_type == FAT_TYPE_FAT32) {
		new_fs_info->info_sector_offset	= 1;
		new_fs_info->boot_sector_backup_offset = 6;

		new_fs_info->root_dir_offset = 0;
		new_fs_info->root_dir_entry_count = 0;
		new_fs_info->root_dir_sector_count = 0;

		/* we add calc_align_sectors to push the cluster_offset
		   forward, to keep the clusters aligned between the new
		   and old file systems
		 */
		new_fs_info->fat_offset
			= fat_min_reserved_sector_count (FAT_TYPE_FAT32)
			  + fat_calc_align_sectors (new_fs, fs);

		new_fs_info->cluster_offset
			= new_fs_info->fat_offset
			  + 2 * new_fs_info->fat_sectors;
	} else {
		new_fs_info->root_dir_sector_count = root_dir_sector_count;
		new_fs_info->root_dir_entry_count
			= root_dir_sector_count * 512 / sizeof (FatDirEntry);

		new_fs_info->fat_offset
			= fat_min_reserved_sector_count (FAT_TYPE_FAT16)
			  + fat_calc_align_sectors (new_fs, fs);

		new_fs_info->root_dir_offset = new_fs_info->fat_offset
					       + 2 * new_fs_info->fat_sectors;

		new_fs_info->cluster_offset = new_fs_info->root_dir_offset
					  + new_fs_info->root_dir_sector_count;
	}

	new_fs_info->total_dir_clusters = fs_info->total_dir_clusters;

	context = fat_op_context_new (new_fs, fs);
	if (!context)
		goto error_free_new_fs_info;

	if (!fat_op_context_create_initial_fat (context))
		goto error_free_context;

	if (!fat_alloc_buffers (new_fs))
		goto error_free_fat;

	return context;

error_free_fat:
	fat_table_destroy (new_fs_info->fat);
error_free_context:
	free (context);
error_free_new_fs_info:
	free (new_fs_info);
error_free_new_fs:
	free (new_fs);
error:
	return NULL;
}

static int
resize_context_assimilate (FatOpContext* ctx)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*	new_fs_info = FAT_SPECIFIC (ctx->new_fs);

	fat_free_buffers (ctx->old_fs);
	fat_table_destroy (old_fs_info->fat);
	free (old_fs_info);
	ped_geometry_destroy (ctx->old_fs->geom);

	ctx->old_fs->type_specific = ctx->new_fs->type_specific;
	ctx->old_fs->geom = ctx->new_fs->geom;
	ctx->old_fs->type = (new_fs_info->fat_type == FAT_TYPE_FAT16)
				? &fat16_type
			       	: &fat32_type;

	free (ctx->new_fs);

	fat_op_context_destroy (ctx);

	return 1;
}

static int
resize_context_abort (FatOpContext* ctx)
{
	FatSpecific*	new_fs_info = FAT_SPECIFIC (ctx->new_fs);

	fat_free_buffers (ctx->new_fs);
	fat_table_destroy (new_fs_info->fat);
	free (new_fs_info);
	ped_geometry_destroy (ctx->new_fs->geom);
	free (ctx->new_fs);

	fat_op_context_destroy (ctx);

	return 1;
}

/* copies the "hidden" sectors, between the boot sector and the FAT.  Required,
 * for the Windows 98 FAT32 boot loader
 */
int
_copy_hidden_sectors (FatOpContext* ctx)
{
	FatSpecific*    old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*    new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	PedSector       first = 1;
	PedSector       last;
	PedSector       count;

	/* nothing to copy for FAT16 */
	if (old_fs_info->fat_type == FAT_TYPE_FAT16
			|| new_fs_info->fat_type == FAT_TYPE_FAT16)
		return 1;

	last = PED_MIN (old_fs_info->fat_offset, new_fs_info->fat_offset) - 1;
	count = last - first + 1;

	PED_ASSERT (count < BUFFER_SIZE);

	if (!ped_geometry_read (ctx->old_fs->geom, old_fs_info->buffer,
				first, count))
		return 0;
	if (!ped_geometry_write (ctx->new_fs->geom, old_fs_info->buffer,
				 first, count))
		return 0;
	return 1;
}

int
fat_resize (PedFileSystem* fs, PedGeometry* geom, PedTimer* timer)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatSpecific*	new_fs_info;
	FatOpContext*	ctx;
	PedFileSystem*	new_fs;

	ctx = create_resize_context (fs, geom);
	if (!ctx)
		goto error;
	new_fs = ctx->new_fs;
	new_fs_info = FAT_SPECIFIC (new_fs);

	if (!fat_duplicate_clusters (ctx, timer))
		goto error_abort_ctx;
	if (fs_info->fat_type == FAT_TYPE_FAT16
			&& new_fs_info->fat_type == FAT_TYPE_FAT32) {
		if (!alloc_root_dir (ctx))
			goto error_abort_ctx;
	}
	if (!fat_construct_new_fat (ctx))
		goto error_abort_ctx;
	if (fs_info->fat_type == FAT_TYPE_FAT32
			&& new_fs_info->fat_type == FAT_TYPE_FAT16) {
		if (!free_root_dir (ctx))
			goto error_abort_ctx;
	}
	if (!fat_construct_dir_tree (ctx))
		goto error_abort_ctx;
	if (!fat_table_write_all (new_fs_info->fat, new_fs))
		goto error_abort_ctx;

	_copy_hidden_sectors (ctx);
	fat_boot_sector_generate (&new_fs_info->boot_sector, new_fs);
	fat_boot_sector_write (&new_fs_info->boot_sector, new_fs);
	if (new_fs_info->fat_type == FAT_TYPE_FAT32) {
		fat_info_sector_generate (&new_fs_info->info_sector, new_fs);
		fat_info_sector_write (&new_fs_info->info_sector, new_fs);
	}

	if (!resize_context_assimilate (ctx))
		goto error;

	return 1;

error_abort_ctx:
	resize_context_abort (ctx);
error:
	return 0;
}

#endif /* !DISCOVER_ONLY */
