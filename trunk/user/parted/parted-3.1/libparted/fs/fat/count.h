/*
    libparted
    Copyright (C) 1999-2000, 2007-2012 Free Software Foundation, Inc.

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

#ifndef COUNT_H_INCLUDED
#define COUNT_H_INCLUDED

typedef enum _FatClusterFlag FatClusterFlag;
typedef struct _FatClusterInfo FatClusterInfo;

enum _FatClusterFlag {
	FAT_FLAG_FREE=0,
	FAT_FLAG_FILE=1,
	FAT_FLAG_DIRECTORY=2,
	FAT_FLAG_BAD=3
};

struct __attribute__ ((packed)) _FatClusterInfo {
	unsigned int    units_used:6;   /* 1 unit = cluster_size / 64 */
	FatClusterFlag  flag:2;
};

extern int fat_collect_cluster_info (PedFileSystem *fs);
extern FatClusterFlag fat_get_cluster_flag (PedFileSystem* fs,
					    FatCluster cluster);
extern PedSector fat_get_cluster_usage (PedFileSystem* fs, FatCluster cluster);
extern FatClusterFlag fat_get_fragment_flag (PedFileSystem* fs,
					     FatFragment frag);
extern int fat_is_fragment_active (PedFileSystem* fs, FatFragment frag);

#endif /* COUNT_H_INCLUDED */
