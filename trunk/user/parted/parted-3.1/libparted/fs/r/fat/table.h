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

#ifndef PED_FAT_TABLE_H_INCLUDED
#define PED_FAT_TABLE_H_INCLUDED

typedef struct _FatTable	FatTable;

#include "fat.h"

struct _FatTable {
	void*		table;
	FatCluster	size;
	int		raw_size;

	FatType		fat_type;
	FatCluster	cluster_count;
	FatCluster	free_cluster_count;
	FatCluster	bad_cluster_count;

	FatCluster	last_alloc;
};

extern FatTable* fat_table_new (FatType fat_type, FatCluster size);
extern FatTable* fat_table_duplicate (const FatTable* ft);
extern void fat_table_destroy (FatTable* ft);
extern void fat_table_clear (FatTable* ft);
extern int fat_table_set_cluster_count (FatTable* ft,
					FatCluster new_cluster_count);

extern int fat_table_read (FatTable* ft, const PedFileSystem* fs,
			   int table_num);
extern int fat_table_write (const FatTable* ft, PedFileSystem* fs,
			    int table_num);
extern int fat_table_write_all (const FatTable* ft, PedFileSystem* fs);
extern int fat_table_compare (const FatTable* a, const FatTable* b);
extern int fat_table_count_stats (FatTable* ft);

extern FatCluster fat_table_get (const FatTable* ft, FatCluster cluster);
extern int fat_table_set (FatTable* ft, FatCluster cluster, FatCluster value);

extern FatCluster fat_table_alloc_cluster (FatTable* ft);
extern FatCluster fat_table_alloc_check_cluster (FatTable* ft,
						 PedFileSystem* fs);

extern int fat_table_is_bad (const FatTable* ft, FatCluster cluster);
extern int fat_table_is_eof (const FatTable* ft, FatCluster cluster);
extern int fat_table_is_empty (const FatTable* ft, FatCluster cluster);
extern int fat_table_is_available (const FatTable* ft, FatCluster cluster);
extern int fat_table_is_active (const FatTable* ft, FatCluster cluster);

extern int fat_table_set_eof (FatTable* ft, FatCluster cluster);
extern int fat_table_set_avail (FatTable* ft, FatCluster cluster);
extern int fat_table_set_bad (FatTable* ft, FatCluster cluster);

extern int fat_table_entry_size (FatType fat_type);

#endif /* PED_FAT_TABLE_H_INCLUDED */
