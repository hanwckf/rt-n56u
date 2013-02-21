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

#ifndef TRAVERSE_H_INCLUDED
#define TRAVERSE_H_INCLUDED

#include "fatio.h"

typedef struct _FatTraverseInfo		FatTraverseInfo;

struct _FatTraverseInfo {
	PedFileSystem*		fs;
	char*			dir_name;

	int			is_legacy_root_dir;
	int			dirty;
	int			eof;

	FatDirEntry*		dir_entries;
	int			current_entry;
	FatCluster		this_buffer, next_buffer;
	int			buffer_size;
};

extern int fat_traverse_entries_per_buffer (FatTraverseInfo* trav_info);

/* starts traversal at an arbitary cluster.  if start_cluster==0, then uses
   root directory */
extern FatTraverseInfo* fat_traverse_begin (PedFileSystem* fs,
					    FatCluster start_cluster,
					    const char* dir_name);

extern int fat_traverse_complete (FatTraverseInfo* trav_info);

extern FatTraverseInfo* fat_traverse_directory (FatTraverseInfo* trav_info,
						FatDirEntry* parent);

extern void fat_traverse_mark_dirty (FatTraverseInfo* trav_info);

extern FatDirEntry* fat_traverse_next_dir_entry (FatTraverseInfo* trav_info);

extern FatCluster fat_dir_entry_get_first_cluster (FatDirEntry* dir_entry,
						   PedFileSystem* fs);

extern void fat_dir_entry_set_first_cluster (FatDirEntry* dir_entry,
					PedFileSystem* fs, FatCluster cluster);

extern uint32_t fat_dir_entry_get_length (FatDirEntry* dir_entry);

extern int fat_dir_entry_is_null_term (const FatDirEntry* dir_entry);
extern int fat_dir_entry_is_file (FatDirEntry* dir_entry);
extern int fat_dir_entry_is_system_file (FatDirEntry* dir_entry);
extern int fat_dir_entry_is_directory (FatDirEntry* dir_entry);
extern void fat_dir_entry_get_name (const FatDirEntry* dir_entry, char* result);
extern int fat_dir_entry_is_active (FatDirEntry* dir_entry);
extern int fat_dir_entry_has_first_cluster (FatDirEntry* dir_entry,
					    PedFileSystem* fs);

#endif /* TRAVERSE_H_INCLUDED */
