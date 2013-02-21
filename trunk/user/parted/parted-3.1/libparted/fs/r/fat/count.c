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
#include "fat.h"
#include "traverse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DISCOVER_ONLY

#if 0
/* extremely ugly hack: stick everything that obviously isn't an unmovable file
 * in here.  Note: DAT is a bit dubious.  Unfortunately, it's used by the
 * registry, so it'll be all over the place :-(
 */
static char*	movable_extensions[] = {
	"",
	"1ST",
	"AVI",
	"BAK", "BAT", "BMP",
	"CFG", "COM", "CSS",
	"DAT", "DLL", "DOC", "DRV",
	"EXE",
	"FAQ", "FLT", "FON",
	"GID", "GIF",
	"HLP", "HTT", "HTM",
	"ICO", "INI",
	"JPG",
	"LNK", "LOG",
	"KBD",
	"ME", "MID", "MSG",
	"OCX", "OLD",
	"PIF", "PNG", "PRV",
	"RTF",
	"SCR", "SYS",
	"TMP", "TTF", "TXT",
	"URL",
	"WAV",
	"VBX", "VOC", "VXD",
	NULL
};

static char*
get_extension (char* file_name)
{
	char*		ext;

	ext = strrchr (file_name, '.');
	if (!ext)
		return "";
	if (strchr (ext, '\\'))
		return "";
	return ext + 1;
}

static int
is_movable_system_file (char* file_name)
{
	char*		ext = get_extension (file_name);
	int		i;

	for (i = 0; movable_extensions [i]; i++) {
		if (strcasecmp (ext, movable_extensions [i]) == 0)
			return 1;
	}

	return 0;
}
#endif /* 0 */

/*
    prints out the sequence of clusters for a given file chain, beginning
    at start_cluster.
*/
#ifdef PED_VERBOSE
static void
print_chain (PedFileSystem* fs, FatCluster start)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	clst;
	int		this_row;

	this_row = 0;
	for (clst = start; !fat_table_is_eof (fs_info->fat, clst);
	     clst = fat_table_get (fs_info->fat, clst)) {
		printf ("  %d", (int) clst);
		if (++this_row == 7) {
                        putchar ('\n');
			this_row = 0;
		}
	}
	putchar ('\n');
}
#endif /* PED_VERBOSE */

static PedSector
remainder_round_up (PedSector a, PedSector b)
{
	PedSector	result;

	result = a % b;
	if (!result)
		result = b;
	return result;
}

/*
    traverse the FAT for a file/directory, marking each entry's flag
    to "flag".
*/
static int
flag_traverse_fat (PedFileSystem* fs, const char* chain_name, FatCluster start,
		   FatClusterFlag flag, PedSector size)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	clst;
	FatCluster	prev_clst;
	int		last_cluster_usage;
	FatCluster	chain_length = 0;

	if (fat_table_is_eof (fs_info->fat, start)) {
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Bad directory entry for %s: first cluster is the "
			  "end of file marker."),
			chain_name)
				!= PED_EXCEPTION_IGNORE)
			return 0;
	}

	for (prev_clst = clst = start; !fat_table_is_eof (fs_info->fat, clst);
	     prev_clst = clst, clst = fat_table_get (fs_info->fat, clst)) {
		chain_length++;
		if (!clst) {
			ped_exception_throw (PED_EXCEPTION_FATAL,
				PED_EXCEPTION_CANCEL,
				_("Bad FAT: unterminated chain for %s.  You "
				  "should run dosfsck or scandisk."),
				chain_name);
			return 0;
		}

		if (clst >= fs_info->fat->cluster_count + 2) {
			ped_exception_throw (PED_EXCEPTION_FATAL,
				PED_EXCEPTION_CANCEL,
				_("Bad FAT: cluster %d outside file system "
				  "in chain for %s.  You should run dosfsck "
				  "or scandisk."),
				(int) clst, chain_name);
			return 0;
		}

		if (fs_info->cluster_info [clst].flag != FAT_FLAG_FREE ) {
			ped_exception_throw (PED_EXCEPTION_FATAL,
				PED_EXCEPTION_CANCEL,
				_("Bad FAT: cluster %d is cross-linked for "
				  "%s.  You should run dosfsck or scandisk."),
				(int) clst, chain_name);
			return 0;
		}

		if (flag == FAT_FLAG_DIRECTORY)
			fs_info->total_dir_clusters++;

		fs_info->cluster_info [clst].flag = flag;
		fs_info->cluster_info [clst].units_used = 0;	/* 0 == 64 */
	}

	if (size
	    && chain_length
	    		!= ped_div_round_up (size, fs_info->cluster_sectors)) {
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("%s is %dk, but it has %d clusters (%dk)."),
			chain_name,
			(int) size / 2,
			(int) chain_length,
			(int) chain_length * fs_info->cluster_sectors / 2)
				!= PED_EXCEPTION_IGNORE)
			return 0;
	}

	last_cluster_usage
		= ped_div_round_up (64 * remainder_round_up (size,
						fs_info->cluster_sectors),
				fs_info->cluster_sectors);

	fs_info->cluster_info [prev_clst].units_used = last_cluster_usage;

	return 1;
}

/*
    recursively traverses a directory, flagging all clusters in the process.
    It frees the traverse_info structure before returning.
*/
static int
flag_traverse_dir (FatTraverseInfo* trav_info) {
	PedFileSystem*		fs = trav_info->fs;
	FatDirEntry*		this_entry;
	FatTraverseInfo*	subdir_trav_info;
	char			file_name [512];
	char*			file_name_start;
	FatCluster		first_cluster;
	PedSector		size;

	PED_ASSERT (trav_info != NULL);

	strcpy (file_name, trav_info->dir_name);
	file_name_start = file_name + strlen (file_name);

	while ( (this_entry = fat_traverse_next_dir_entry (trav_info)) ) {
		if (fat_dir_entry_is_null_term (this_entry))
			break;
		if (!fat_dir_entry_has_first_cluster (this_entry, fs))
			continue;
		if (this_entry->name [0] == '.')
			continue;	/* skip . and .. entries */

		fat_dir_entry_get_name (this_entry, file_name_start);
		first_cluster = fat_dir_entry_get_first_cluster(this_entry, fs);
		size = ped_div_round_up (fat_dir_entry_get_length (this_entry),
					 512);

#ifdef PED_VERBOSE
		printf ("%s: ", file_name);
		print_chain (fs, first_cluster);
#endif

#if 0
		if (fat_dir_entry_is_system_file (this_entry)
		    && !is_movable_system_file (file_name)) {
                        PedExceptionOption ex_status;
			ex_status = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The file %s is marked as a system file.  "
				"This means moving it could cause some "
				"programs to stop working."),
				file_name);

			switch (ex_status) {
				case PED_EXCEPTION_CANCEL:
					return 0;

				case PED_EXCEPTION_UNHANDLED:
					ped_exception_catch ();
				case PED_EXCEPTION_IGNORE:
			}
		}
#endif /* 0 */

		if (fat_dir_entry_is_directory (this_entry)) {
			if (!flag_traverse_fat (fs, file_name, first_cluster,
						FAT_FLAG_DIRECTORY, size))
				return 0;

			subdir_trav_info = fat_traverse_directory (trav_info,
								   this_entry);
			if (!subdir_trav_info)
				return 0;
			if (!flag_traverse_dir (subdir_trav_info))
				return 0;
		} else if (fat_dir_entry_is_file (this_entry)) {
			if (!flag_traverse_fat (fs, file_name, first_cluster,
						FAT_FLAG_FILE, size))
				return 0;
		}
	}

	fat_traverse_complete (trav_info);
	return 1;
}

static void
_mark_bad_clusters (PedFileSystem* fs)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	cluster;

	for (cluster = 2; cluster < fs_info->cluster_count + 2; cluster++) {
		if (fat_table_is_bad (fs_info->fat, cluster))
			fs_info->cluster_info [cluster].flag = FAT_FLAG_BAD;
	}
}

/*
    fills in cluster_info.  Each FAT entry (= cluster) is flagged as either
    FAT_FLAG_FREE, FAT_FLAG_FILE or FAT_FLAG_DIRECTORY.

    Also, the fraction of each cluster (x/64) is recorded
*/
int
fat_collect_cluster_info (PedFileSystem* fs) {
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);
	FatTraverseInfo*	trav_info;

	/* set all clusters to unused as a default */
	memset (fs_info->cluster_info, 0, fs_info->fat->cluster_count + 2);
	fs_info->total_dir_clusters = 0;

	if (fs_info->fat_type == FAT_TYPE_FAT32) {
		trav_info = fat_traverse_begin (fs, fs_info->root_cluster,
						"\\");
		if (!flag_traverse_dir (trav_info))
			return 0;
		if (!flag_traverse_fat (fs, "\\", fs_info->root_cluster,
                                        FAT_FLAG_DIRECTORY, 0))
			return 0;
	} else {
		trav_info = fat_traverse_begin (fs, FAT_ROOT, "\\");
		if (!flag_traverse_dir (trav_info))
			return 0;
	}

	_mark_bad_clusters (fs);
	return 1;
}

FatClusterFlag
fat_get_cluster_flag (PedFileSystem* fs, FatCluster cluster)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);

	return fs_info->cluster_info [cluster].flag;
}

PedSector
fat_get_cluster_usage (PedFileSystem* fs, FatCluster cluster)
{
	FatSpecific*		fs_info = FAT_SPECIFIC (fs);
	int			fraction;

	if (fs_info->cluster_info [cluster].flag == FAT_FLAG_FREE)
		return 0;

	fraction = fs_info->cluster_info [cluster].units_used;
	if (fraction == 0)
		fraction = 64;

	return fraction * fs_info->cluster_sectors / 64;
}

FatClusterFlag
fat_get_fragment_flag (PedFileSystem* fs, FatFragment frag)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	FatCluster	cluster = fat_frag_to_cluster (fs, frag);
	FatFragment	offset = frag % fs_info->cluster_frags;
	FatFragment	last_frag_used;
	FatClusterFlag	flag;

	PED_ASSERT (cluster >= 2 && cluster < fs_info->cluster_count + 2);

	flag = fat_get_cluster_flag (fs, cluster);
	if (flag != FAT_FLAG_FILE && flag != FAT_FLAG_DIRECTORY)
		return flag;
	last_frag_used = (fat_get_cluster_usage (fs, cluster) - 1)
				/ fs_info->frag_sectors;
	if (offset > last_frag_used)
		return FAT_FLAG_FREE;
	else
		return flag;
}

int
fat_is_fragment_active (PedFileSystem* fs, FatFragment frag)
{
	switch (fat_get_fragment_flag (fs, frag)) {
		case FAT_FLAG_FREE:
		case FAT_FLAG_BAD:
			return 0;

		case FAT_FLAG_FILE:
		case FAT_FLAG_DIRECTORY:
			return 1;
	}
	return 0;
}

#endif /* !DISCOVER_ONLY */
