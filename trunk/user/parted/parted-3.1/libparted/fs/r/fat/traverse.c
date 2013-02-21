/*
    libparted
    Copyright (C) 1998-2000, 2005, 2007-2012 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DISCOVER_ONLY

#define NO_CLUSTER -1

static char tmp_buffer [4096];

int
fat_traverse_entries_per_buffer (FatTraverseInfo* trav_info)
{
	return trav_info->buffer_size / sizeof (FatDirEntry);
}

/* returns 1 if there are no more directory entries in the directory being
 * traversed, 0 otherwise.
 */
static int
is_last_buffer (FatTraverseInfo* trav_info) {
	FatSpecific*	fs_info = FAT_SPECIFIC (trav_info->fs);

	if (trav_info->is_legacy_root_dir)
		return 1;
	else
		return fat_table_is_eof (fs_info->fat, trav_info->next_buffer);
}

static int
write_root_dir (FatTraverseInfo* trav_info)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (trav_info->fs);

	if (!ped_geometry_write (trav_info->fs->geom, trav_info->dir_entries,
				 fs_info->root_dir_offset,
				 fs_info->root_dir_sector_count))
		return 0;
	if (!ped_geometry_sync (trav_info->fs->geom))
		return 0;
	trav_info->dirty = 0;
	return 1;
}

static int
write_dir_cluster (FatTraverseInfo* trav_info)
{
	if (!fat_write_sync_cluster (trav_info->fs,
				     (void*) trav_info->dir_entries,
				     trav_info->this_buffer))
		return 0;
	trav_info->dirty = 0;
	return 1;
}

static int
write_dir_buffer (FatTraverseInfo* trav_info)
{
	if (trav_info->is_legacy_root_dir)
		return write_root_dir (trav_info);
	else
		return write_dir_cluster (trav_info);
}

static int
read_next_dir_buffer (FatTraverseInfo* trav_info)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (trav_info->fs);

	PED_ASSERT (!trav_info->is_legacy_root_dir);

	trav_info->this_buffer = trav_info->next_buffer;

	if (trav_info->this_buffer < 2
	    || trav_info->this_buffer >= fs_info->cluster_count + 2) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			"Cluster %ld in directory %s is outside file system!",
			(long) trav_info->this_buffer,
			trav_info->dir_name);
		return 0;
	}

	trav_info->next_buffer
		= fat_table_get (fs_info->fat, trav_info->this_buffer);

	return fat_read_cluster (trav_info->fs, (void *) trav_info->dir_entries,
				 trav_info->this_buffer);
}

/* FIXME: put into fat_dir_entry_* operations */
void
fat_traverse_mark_dirty (FatTraverseInfo* trav_info)
{
	trav_info->dirty = 1;
}

FatTraverseInfo*
fat_traverse_begin (PedFileSystem* fs, FatCluster start_cluster,
		    const char* dir_name)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);
	FatTraverseInfo*	trav_info;

	trav_info = (FatTraverseInfo*) ped_malloc (sizeof (FatTraverseInfo));
	if (!trav_info)
		goto error;

	trav_info->dir_name = strdup (dir_name);
	if (!trav_info->dir_name)
		goto error_free_trav_info;

	trav_info->fs = fs;
	trav_info->is_legacy_root_dir
		= (fs_info->fat_type == FAT_TYPE_FAT16) && (start_cluster == 0);
	trav_info->dirty = 0;
	trav_info->eof = 0;
	trav_info->current_entry = -1;

	if (trav_info->is_legacy_root_dir) {
		trav_info->buffer_size = 512 * fs_info->root_dir_sector_count;
	} else {
		trav_info->next_buffer = start_cluster;
		trav_info->buffer_size = fs_info->cluster_size;
	}

	trav_info->dir_entries
		= (FatDirEntry*) ped_malloc (trav_info->buffer_size);
	if (!trav_info->dir_entries)
		goto error_free_dir_name;

	if (trav_info->is_legacy_root_dir) {
		if (!ped_geometry_read (fs->geom, trav_info->dir_entries,
					fs_info->root_dir_offset,
					fs_info->root_dir_sector_count))
			goto error_free_dir_entries;
	} else {
		if (!read_next_dir_buffer (trav_info))
			goto error_free_dir_entries;
	}

	return trav_info;

error_free_dir_entries:
	free (trav_info->dir_entries);
error_free_dir_name:
	free (trav_info->dir_name);
error_free_trav_info:
	free (trav_info);
error:
	return NULL;
}

int
fat_traverse_complete (FatTraverseInfo* trav_info)
{
	if (trav_info->dirty) {
		if (!write_dir_buffer (trav_info))
			return 0;
	}
	free (trav_info->dir_entries);
	free (trav_info->dir_name);
	free (trav_info);
	return 1;
}

FatTraverseInfo*
fat_traverse_directory (FatTraverseInfo *trav_info, FatDirEntry* parent)
{
	strcpy (tmp_buffer, trav_info->dir_name);
	fat_dir_entry_get_name (parent,
				tmp_buffer + strlen (trav_info->dir_name));
	strcat (tmp_buffer, "\\");

	return fat_traverse_begin (trav_info->fs,
			fat_dir_entry_get_first_cluster (parent, trav_info->fs),
			tmp_buffer);
}

FatDirEntry*
fat_traverse_next_dir_entry (FatTraverseInfo *trav_info)
{
	if (trav_info->eof)
		return NULL;

	trav_info->current_entry++;
	if (trav_info->current_entry
			>= fat_traverse_entries_per_buffer (trav_info)) {
		if (trav_info->dirty) {
			if (!write_dir_buffer (trav_info))
				return NULL;
		}

		trav_info->current_entry = 0;
		if (is_last_buffer (trav_info)) {
			trav_info->eof = 1;
			return NULL;
		}
		if (!read_next_dir_buffer (trav_info))
			return NULL;
	}
	return trav_info->dir_entries + trav_info->current_entry;
}

FatCluster
fat_dir_entry_get_first_cluster (FatDirEntry* dir_entry, PedFileSystem *fs)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);

	switch (fs_info->fat_type) {
	case FAT_TYPE_FAT12:
	case FAT_TYPE_FAT16:
		return PED_LE16_TO_CPU (dir_entry->first_cluster);

        case FAT_TYPE_FAT32:
		return PED_LE16_TO_CPU (dir_entry->first_cluster_high)
				* 65536L
			  + PED_LE16_TO_CPU (dir_entry->first_cluster);
	}

	return 0;
}

void
fat_dir_entry_set_first_cluster (FatDirEntry* dir_entry, PedFileSystem* fs,
				 FatCluster cluster)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);

	switch (fs_info->fat_type) {
                case FAT_TYPE_FAT12:
                PED_ASSERT (0);
                break;

		case FAT_TYPE_FAT16:
		dir_entry->first_cluster = PED_CPU_TO_LE16 (cluster);
		break;

		case FAT_TYPE_FAT32:
		dir_entry->first_cluster
			= PED_CPU_TO_LE16 (cluster & 0xffff);
		dir_entry->first_cluster_high
			= PED_CPU_TO_LE16 (cluster / 0x10000);
		break;
	}
}

uint32_t
fat_dir_entry_get_length (FatDirEntry* dir_entry)
{
	return PED_LE32_TO_CPU (dir_entry->length);
}

int
fat_dir_entry_is_null_term (const FatDirEntry* dir_entry)
{
	FatDirEntry	null_entry;

	memset (&null_entry, 0, sizeof (null_entry));
	return memcmp (&null_entry, dir_entry, sizeof (null_entry)) == 0;
}

int
fat_dir_entry_is_active (FatDirEntry* dir_entry)
{
	if ((unsigned char) dir_entry->name[0] == DELETED_FLAG) return 0;
	if ((unsigned char) dir_entry->name[0] == 0) return 0;
	if ((unsigned char) dir_entry->name[0] == 0xF6) return 0;
	return 1;
}

int
fat_dir_entry_is_file (FatDirEntry* dir_entry) {
	if (dir_entry->attributes == VFAT_ATTR) return 0;
	if (dir_entry->attributes & VOLUME_LABEL_ATTR) return 0;
	if (!fat_dir_entry_is_active (dir_entry)) return 0;
	if ((dir_entry->attributes & DIRECTORY_ATTR) == DIRECTORY_ATTR) return 0;
	return 1;
}

int
fat_dir_entry_is_system_file (FatDirEntry* dir_entry)
{
	if (!fat_dir_entry_is_file (dir_entry)) return 0;
	return (dir_entry->attributes & SYSTEM_ATTR)
		|| (dir_entry->attributes & HIDDEN_ATTR);
}

int
fat_dir_entry_is_directory (FatDirEntry* dir_entry)
{
	if (dir_entry->attributes == VFAT_ATTR) return 0;
	if (dir_entry->attributes & VOLUME_LABEL_ATTR) return 0;
	if (!fat_dir_entry_is_active (dir_entry)) return 0;
	return (dir_entry->attributes & DIRECTORY_ATTR) == DIRECTORY_ATTR;
}

int
fat_dir_entry_has_first_cluster (FatDirEntry* dir_entry, PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	first_cluster;

	if (!fat_dir_entry_is_file (dir_entry)
		&& !fat_dir_entry_is_directory (dir_entry))
		return 0;

	first_cluster = fat_dir_entry_get_first_cluster (dir_entry, fs);
	if (first_cluster == 0
		|| fat_table_is_eof (fs_info->fat, first_cluster))
		return 0;

	return 1;
}

/*
    decrypts silly DOS names to FILENAME.EXT
*/
void
fat_dir_entry_get_name (const FatDirEntry *dir_entry, char *result) {
	size_t i;
	const char *src;
	const char *ext;

	src = dir_entry->name;

	for (i=0; i < sizeof dir_entry->name; i++) {
		if (src[i] == ' ' || src[i] == 0) break;
		*result++ = src[i];
	}

	ext = (const char *) dir_entry->extension;
	if (ext[0] != ' ' && ext[0] != 0) {
		*result++ = '.';
		for (i=0; i < sizeof dir_entry->extension; i++) {
			if (ext[i] == ' ' || ext[i] == 0) break;
			*result++ = ext[i];
		}
	}

	*result = 0;
}

#endif /* !DISCOVER_ONLY */
