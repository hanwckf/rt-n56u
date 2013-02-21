/*
    libparted
    Copyright (C) 1998-2001, 2007, 2009-2012 Free Software Foundation, Inc.

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

static int
needs_duplicating (const FatOpContext* ctx, FatFragment frag)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatCluster	cluster = fat_frag_to_cluster (ctx->old_fs, frag);
	FatClusterFlag	flag;

	PED_ASSERT (cluster >= 2 && cluster < old_fs_info->cluster_count + 2);

	flag = fat_get_fragment_flag (ctx->old_fs, frag);
	switch (flag) {
	case FAT_FLAG_FREE:
		return 0;

	case FAT_FLAG_DIRECTORY:
		return 1;

	case FAT_FLAG_FILE:
		return fat_op_context_map_static_fragment (ctx, frag) == -1;

	case FAT_FLAG_BAD:
		return 0;
	}

	return 0;
}

static int
search_next_fragment (FatOpContext* ctx)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (ctx->old_fs);

	for (; ctx->buffer_offset < fs_info->frag_count; ctx->buffer_offset++) {
		if (needs_duplicating (ctx, ctx->buffer_offset))
			return 1;
	}
	return 0;	/* all done! */
}

static int
read_marked_fragments (FatOpContext* ctx, FatFragment length)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (ctx->old_fs);
	int			status;
	FatFragment		i;

	ped_exception_fetch_all ();
	status = fat_read_fragments (ctx->old_fs, fs_info->buffer,
				     ctx->buffer_offset, length);
	ped_exception_leave_all ();
	if (status)
		return 1;

	ped_exception_catch ();

/* something bad happened, so read fragments one by one.  (The error may
   have occurred on an unused fragment: who cares) */
	for (i = 0; i < length; i++) {
		if (ctx->buffer_map [i]) {
			if (!fat_read_fragment (ctx->old_fs,
			      fs_info->buffer + i * fs_info->frag_size,
			      ctx->buffer_offset + i))
				return 0;
		}
	}

	return 1;
}

static int
fetch_fragments (FatOpContext* ctx)
{
	FatSpecific*	old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatFragment	fetch_length = 0;
	FatFragment	frag;

	for (frag = 0; frag < ctx->buffer_frags; frag++)
		ctx->buffer_map [frag] = -1;

	for (frag = 0;
	     frag < ctx->buffer_frags
		&& ctx->buffer_offset + frag < old_fs_info->frag_count;
	     frag++) {
		if (needs_duplicating (ctx, ctx->buffer_offset + frag)) {
			ctx->buffer_map [frag] = 1;
			fetch_length = frag + 1;
		}
	}

	if (!read_marked_fragments (ctx, fetch_length))
		return 0;

	return 1;
}

/*****************************************************************************
 * here starts the write code.  All assumes that ctx->buffer_map [first] and
 * ctx->buffer_map [last] are occupied by fragments that need to be duplicated.
 *****************************************************************************/

/* finds the first fragment that is not going to get overwritten (that needs to
   get read in) */
static FatFragment
get_first_underlay (const FatOpContext* ctx, int first, int last)
{
	int		old;
	FatFragment	new;

	PED_ASSERT (first <= last);

	new = ctx->buffer_map [first];
	for (old = first + 1; old <= last; old++) {
		if (ctx->buffer_map [old] == -1)
			continue;
		new++;
		if (ctx->buffer_map [old] != new)
			return new;
	}
	return -1;
}

/* finds the last fragment that is not going to get overwritten (that needs to
   get read in) */
static FatFragment
get_last_underlay (const FatOpContext* ctx, int first, int last)
{
	int		old;
	FatFragment	new;

	PED_ASSERT (first <= last);

	new = ctx->buffer_map [last];
	for (old = last - 1; old >= first; old--) {
		if (ctx->buffer_map [old] == -1)
			continue;
		new--;
		if (ctx->buffer_map [old] != new)
			return new;
	}
	return -1;
}

/* "underlay" refers to the "static" fragments, that remain unchanged.
 * when writing large chunks at a time, we don't want to clobber these,
 * so we read them in, and write them back again.  MUCH quicker that way.
 */
static int
quick_group_write_read_underlay (FatOpContext* ctx, int first, int last)
{
	FatSpecific*	new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	FatFragment	first_underlay;
	FatFragment	last_underlay;
	FatFragment	underlay_length;

	PED_ASSERT (first <= last);

	first_underlay = get_first_underlay (ctx, first, last);
	if (first_underlay == -1)
		return 1;
	last_underlay = get_last_underlay (ctx, first, last);

	PED_ASSERT (first_underlay <= last_underlay);

	underlay_length = last_underlay - first_underlay + 1;
	if (!fat_read_fragments (ctx->new_fs,
				new_fs_info->buffer
				   + (first_underlay - ctx->buffer_map [first])
					* new_fs_info->frag_size,
				first_underlay,
				underlay_length))
		return 0;
	return 1;
}

/* quick_group_write() makes no attempt to recover from errors - just
 * does things fast.  If there is an error, slow_group_write() is
 * called.
 *    Note: we do syncing writes, to make sure there isn't any
 * error writing out.  It's rather difficult recovering from errors
 * further on.
 */
static int
quick_group_write (FatOpContext* ctx, int first, int last)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	int			active_length;
	int			i;
	int			offset;

	PED_ASSERT (first <= last);

	ped_exception_fetch_all ();
	if (!quick_group_write_read_underlay (ctx, first, last))
		goto error;

	for (i = first; i <= last; i++) {
		if (ctx->buffer_map [i] == -1)
			continue;

		offset = ctx->buffer_map [i] - ctx->buffer_map [first];
		memcpy (new_fs_info->buffer + offset * new_fs_info->frag_size,
			old_fs_info->buffer + i * new_fs_info->frag_size,
			new_fs_info->frag_size);
	}

	active_length = ctx->buffer_map [last] - ctx->buffer_map [first] + 1;
	if (!fat_write_sync_fragments (ctx->new_fs, new_fs_info->buffer,
				       ctx->buffer_map [first], active_length))
		goto error;

	ped_exception_leave_all ();
	return 1;

error:
	ped_exception_catch ();
	ped_exception_leave_all ();
	return 0;
}

/* Writes fragments out, one at a time, avoiding errors on redundant writes
 * on damaged parts of the disk we already know about.  If there's an error
 * on one of the required fragments, it gets marked as bad, and a replacement
 * is found.
 */
static int
slow_group_write (FatOpContext* ctx, int first, int last)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	int			i;

	PED_ASSERT (first <= last);

	for (i = first; i <= last; i++) {
		if (ctx->buffer_map [i] == -1)
			continue;

		while (!fat_write_sync_fragment (ctx->new_fs,
			      old_fs_info->buffer + i * old_fs_info->frag_size,
			      ctx->buffer_map [i])) {
			fat_table_set_bad (new_fs_info->fat,
					   ctx->buffer_map [i]);
			ctx->buffer_map [i] = fat_table_alloc_cluster
						(new_fs_info->fat);
			if (ctx->buffer_map [i] == 0)
				return 0;
		}
	}
	return 1;
}

static int
update_remap (FatOpContext* ctx, int first, int last)
{
	int		i;

	PED_ASSERT (first <= last);

	for (i = first; i <= last; i++) {
		if (ctx->buffer_map [i] == -1)
			continue;
		ctx->remap [ctx->buffer_offset + i] = ctx->buffer_map [i];
	}

	return 1;
}

static int
group_write (FatOpContext* ctx, int first, int last)
{
	PED_ASSERT (first <= last);

	if (!quick_group_write (ctx, first, last)) {
		if (!slow_group_write (ctx, first, last))
			return 0;
	}
	if (!update_remap (ctx, first, last))
		return 0;
	return 1;
}

/* assumes fragment size and new_fs's cluster size are equal */
static int
write_fragments (FatOpContext* ctx)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatSpecific*		new_fs_info = FAT_SPECIFIC (ctx->new_fs);
	int			group_start;
	int			group_end = -1;	/* shut gcc up! */
	FatFragment		mapped_length;
	FatFragment		i;
	FatCluster		new_cluster;

	PED_ASSERT (ctx->buffer_offset < old_fs_info->frag_count);

	group_start = -1;
	for (i = 0; i < ctx->buffer_frags; i++) {
		if (ctx->buffer_map [i] == -1)
			continue;

		ctx->frags_duped++;

		new_cluster = fat_table_alloc_cluster (new_fs_info->fat);
		if (!new_cluster)
			return 0;
		fat_table_set_eof (new_fs_info->fat, new_cluster);
		ctx->buffer_map [i] = fat_cluster_to_frag (ctx->new_fs,
							   new_cluster);

		if (group_start == -1)
			group_start = group_end = i;

		PED_ASSERT (ctx->buffer_map [i]
				>= ctx->buffer_map [group_start]);

		mapped_length = ctx->buffer_map [i]
				- ctx->buffer_map [group_start] + 1;
		if (mapped_length <= ctx->buffer_frags) {
			group_end = i;
		} else {
			/* ran out of room in the buffer, so write this group,
			 * and start a new one...
			 */
			if (!group_write (ctx, group_start, group_end))
				return 0;
			group_start = group_end = i;
		}
	}

	PED_ASSERT (group_start != -1);

	if (!group_write (ctx, group_start, group_end))
		return 0;
	return 1;
}

/*  default all fragments to unmoved
 */
static void
init_remap (FatOpContext* ctx)
{
	FatSpecific*		old_fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatFragment		i;

	for (i = 0; i < old_fs_info->frag_count; i++)
		ctx->remap[i] = fat_op_context_map_static_fragment (ctx, i);
}

static FatFragment
count_frags_to_dup (FatOpContext* ctx)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (ctx->old_fs);
	FatFragment	i;
	FatFragment	total;

	total = 0;

	for (i = 0; i < fs_info->frag_count; i++) {
		if (needs_duplicating (ctx, i))
			total++;
	}

	return total;
}

/*  duplicates unreachable file clusters, and all directory clusters
 */
int
fat_duplicate_clusters (FatOpContext* ctx, PedTimer* timer)
{
	FatFragment	total_frags_to_dup;

	init_remap (ctx);
	total_frags_to_dup = count_frags_to_dup (ctx);

	ped_timer_reset (timer);
	ped_timer_set_state_name (timer, "moving data");

	ctx->buffer_offset = 0;
	ctx->frags_duped = 0;
	while (search_next_fragment (ctx)) {
		ped_timer_update (
			timer, 1.0 * ctx->frags_duped / total_frags_to_dup);

		if (!fetch_fragments (ctx))
			return 0;
		if (!write_fragments (ctx))
			return 0;
		ctx->buffer_offset += ctx->buffer_frags;
	}

	ped_timer_update (timer, 1.0);
	return 1;
}

#endif /* !DISCOVER_ONLY */
