/*
    libparted
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
#include <string.h>

#include "fat.h"

#ifndef DISCOVER_ONLY

/* Note: this deals with file system start and end sectors, even if the physical
 * devices are different (eg for fat_copy())  Perhaps this is a hack, but it
 * works ;-)
 */
static int
calc_deltas (FatOpContext* ctx)
{
	PedFileSystem*	old_fs = ctx->old_fs;
	PedFileSystem*	new_fs = ctx->new_fs;
	FatSpecific*	old_fs_info = FAT_SPECIFIC (old_fs);
	FatSpecific*	new_fs_info = FAT_SPECIFIC (new_fs);
	PedSector	old_cluster_ofs;
	PedSector	new_cluster_ofs;
	PedSector	sector_delta;

	old_cluster_ofs = old_fs->geom->start + old_fs_info->cluster_offset;
	new_cluster_ofs = new_fs->geom->start + new_fs_info->cluster_offset;

	if (new_cluster_ofs > old_cluster_ofs) {
		ctx->start_move_dir = FAT_DIR_FORWARD;
		sector_delta = new_cluster_ofs - old_cluster_ofs;
	} else {
		ctx->start_move_dir = FAT_DIR_BACKWARD;
		sector_delta = old_cluster_ofs - new_cluster_ofs;
	}

	if (sector_delta % new_fs_info->cluster_sectors) {
		ped_exception_throw (
			PED_EXCEPTION_BUG, PED_EXCEPTION_CANCEL,
			_("Cluster start delta = %d, which is not a multiple "
			  "of the cluster size %d."),
			(int) sector_delta,
			(int) new_fs_info->cluster_sectors);
		return 0;
	}

	ctx->start_move_delta = sector_delta / ctx->frag_sectors;

#ifdef PED_VERBOSE
	printf ("Start move delta is: %d %s.\n",
		(int) ctx->start_move_delta,
		(ctx->start_move_dir == FAT_DIR_FORWARD)?
			"forwards" : "backwards");
#endif

	return 1;
}

FatOpContext*
fat_op_context_new (PedFileSystem* new_fs, PedFileSystem* old_fs)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (old_fs);
	FatSpecific*	new_fs_info = FAT_SPECIFIC (new_fs);
	FatOpContext*	ctx;

	ctx = (FatOpContext*) ped_malloc (sizeof (FatOpContext));
	if (!ctx)
		goto error;

	ctx->frag_sectors = PED_MIN (old_fs_info->cluster_sectors,
				     new_fs_info->cluster_sectors);
	if (!fat_set_frag_sectors (new_fs, ctx->frag_sectors))
		goto error;
	if (!fat_set_frag_sectors (old_fs, ctx->frag_sectors))
		goto error;

	ctx->buffer_frags = old_fs_info->buffer_sectors / ctx->frag_sectors;
	ctx->buffer_map = (FatFragment*) ped_malloc (sizeof (FatFragment)
						     * ctx->buffer_frags);
	if (!ctx->buffer_map)
		goto error_free_ctx;

	ctx->remap = (FatFragment*) ped_malloc (sizeof (FatFragment)
						   * old_fs_info->frag_count);
	if (!ctx->remap)
		goto error_free_buffer_map;

	ctx->new_fs = new_fs;
	ctx->old_fs = old_fs;
	if (!calc_deltas (ctx))
		goto error_free_buffer_map;

	return ctx;

error_free_buffer_map:
	free (ctx->buffer_map);
error_free_ctx:
	free (ctx);
error:
	return NULL;
}

void
fat_op_context_destroy (FatOpContext* ctx)
{
	free (ctx->buffer_map);
	free (ctx->remap);
	free (ctx);
}

FatFragment
fat_op_context_map_static_fragment (const FatOpContext* ctx, FatFragment frag)
{
	FatSpecific*	new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatFragment	result;

	if (ctx->new_fs->geom->dev != ctx->old_fs->geom->dev)
		return -1;

	if (ctx->start_move_dir == FAT_DIR_FORWARD) {
		if (frag < ctx->start_move_delta)
			return -1;
		result = frag - ctx->start_move_delta;
	} else {
		result = frag + ctx->start_move_delta;
	}

	if (result >= new_fs_info->frag_count)
		return -1;

	return result;
}

FatCluster
fat_op_context_map_static_cluster (const FatOpContext* ctx, FatCluster clst)
{
	FatFragment	mapped_frag;

	mapped_frag = fat_op_context_map_static_fragment (ctx,
				fat_cluster_to_frag (ctx->old_fs, clst));
	if (mapped_frag != -1)
		return fat_frag_to_cluster (ctx->new_fs, mapped_frag);
	else
		return 0;
}

FatFragment
fat_op_context_map_fragment (const FatOpContext* ctx, FatFragment frag)
{
	return ctx->remap [frag];
}

FatCluster
fat_op_context_map_cluster (const FatOpContext* ctx, FatCluster clst)
{
	FatFragment	mapped_frag;

	mapped_frag = fat_op_context_map_fragment (ctx,
				fat_cluster_to_frag (ctx->old_fs, clst));
	if (mapped_frag != -1)
		return fat_frag_to_cluster (ctx->new_fs, mapped_frag);
	else
		return 0;
}

/* This function sets the initial fat for the new resized file system.
   This is in *NO WAY* a proper FAT table - all it does is:
	a) mark bad clusters as bad.
	b) mark used clusters (that is, clusters from the original FS that are
	   reachable from the resized one).  Marks as EOF (i.e. used, end of
	   file chain).
	c) mark original file system metadata as EOF (i.e. used), to prevent
	   it from being clobbered.  This will leave the original file system
	   intact, until the partition table is modified, if the start of
	   the partition is moved.

   The FATs are rebuilt *properly* after cluster relocation.  This here is
   only to mark clusters as used, so when cluster relocation occurs, clusters
   aren't relocated on top of ones marked in a, b or c.
*/
int
fat_op_context_create_initial_fat (FatOpContext* ctx)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*	new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatCluster	clst;
	FatCluster	new_clst;
	PedSector	sect;
	PedSector	new_sect;
	FatFragment	frag;
	FatFragment	new_frag;
	FatClusterFlag	frag_flag;

	new_fs_info->fat = fat_table_new (
		new_fs_info->fat_type,
		new_fs_info->fat_sectors * 512
			/ fat_table_entry_size (new_fs_info->fat_type));
	if (!new_fs_info->fat)
		return 0;

	if (!fat_table_set_cluster_count (new_fs_info->fat,
					  new_fs_info->cluster_count))
		return 0;

/* mark bad and used clusters */
	for (frag = 0; frag < old_fs_info->frag_count; frag++) {
		frag_flag = fat_get_fragment_flag (ctx->old_fs, frag);
		if (frag_flag == FAT_FLAG_FREE)
			continue;

		new_frag = fat_op_context_map_static_fragment (ctx, frag);
		if (new_frag == -1)
			continue;

		new_clst = fat_frag_to_cluster (ctx->new_fs, new_frag);
		PED_ASSERT (new_clst != 0);

		if (frag_flag == FAT_FLAG_BAD) {
			if (!fat_table_set_bad (new_fs_info->fat, new_clst))
				return 0;
		} else {
			if (!fat_table_set_eof (new_fs_info->fat, new_clst))
				return 0;
		}
	}

/* mark metadata regions that map to clusters on the new FS */
	for (sect = 0; sect < old_fs_info->cluster_offset; sect++) {
		new_sect = ped_geometry_map (ctx->new_fs->geom,
					     ctx->old_fs->geom, sect);
		if (new_sect == -1
		    || !fat_is_sector_in_clusters (ctx->new_fs, new_sect))
			continue;

		clst = fat_sector_to_cluster (ctx->new_fs, new_sect);
		PED_ASSERT (clst != 0);

		if (!fat_table_set_eof (new_fs_info->fat, clst))
			return 0;
	}

	return 1;
}

#endif /* !DISCOVER_ONLY */
