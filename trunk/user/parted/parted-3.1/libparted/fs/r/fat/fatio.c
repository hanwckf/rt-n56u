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
#include "fatio.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#ifndef DISCOVER_ONLY

int
fat_read_fragments (PedFileSystem* fs, char* buf, FatFragment frag,
		    FatFragment count)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedSector	sector = fat_frag_to_sector (fs, frag);
	PedSector	sector_count = count * fs_info->frag_sectors;

	PED_ASSERT (frag >= 0 && frag < fs_info->frag_count);

	return ped_geometry_read (fs->geom, buf, sector, sector_count);
}

int
fat_read_fragment (PedFileSystem* fs, char* buf, FatFragment frag)
{
	return fat_read_fragments (fs, buf, frag, 1);
}

int
fat_write_fragments (PedFileSystem* fs, char* buf, FatFragment frag,
		     FatFragment count)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedSector	sector = fat_frag_to_sector (fs, frag);
	PedSector	sector_count = count * fs_info->frag_sectors;

	PED_ASSERT (frag >= 0 && frag < fs_info->frag_count);

	return ped_geometry_write (fs->geom, buf, sector, sector_count);
}

int
fat_write_fragment (PedFileSystem* fs, char* buf, FatFragment frag)
{
	return fat_write_fragments (fs, buf, frag, 1);
}

int
fat_write_sync_fragments (PedFileSystem* fs, char* buf, FatFragment frag,
			  FatFragment count)
{
	if (!fat_write_fragments (fs, buf, frag, count))
		return 0;
	if (!ped_geometry_sync (fs->geom))
		return 0;
	return 1;
}

int
fat_write_sync_fragment (PedFileSystem* fs, char* buf, FatFragment frag)
{
	return fat_write_sync_fragments (fs, buf, frag, 1);
}

int
fat_read_clusters (PedFileSystem* fs, char *buf, FatCluster cluster,
		   FatCluster count)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedSector	sector = fat_cluster_to_sector (fs, cluster);
	PedSector	sector_count = count * fs_info->cluster_sectors;

	PED_ASSERT (cluster >= 2
	    	    && cluster + count - 1 < fs_info->cluster_count + 2);

	return ped_geometry_read (fs->geom, buf, sector, sector_count);
}

int
fat_read_cluster (PedFileSystem* fs, char *buf, FatCluster cluster)
{
	return fat_read_clusters (fs, buf, cluster, 1);
}

int
fat_write_clusters (PedFileSystem* fs, char *buf, FatCluster cluster,
		    FatCluster count)
{
	FatSpecific*	fs_info = FAT_SPECIFIC (fs);
	PedSector	sector = fat_cluster_to_sector (fs, cluster);
	PedSector	sector_count = count * fs_info->cluster_sectors;

	PED_ASSERT (cluster >= 2
	    	    && cluster + count - 1 < fs_info->cluster_count + 2);

	return ped_geometry_write (fs->geom, buf, sector, sector_count);
}

int
fat_write_cluster (PedFileSystem* fs, char *buf, FatCluster cluster)
{
	return fat_write_clusters (fs, buf, cluster, 1);
}

int
fat_write_sync_clusters (PedFileSystem* fs, char *buf, FatCluster cluster,
			 FatCluster count)
{
	if (!fat_write_clusters (fs, buf, cluster, count))
		return 0;
	if (!ped_geometry_sync (fs->geom))
		return 0;
	return 1;
}

int
fat_write_sync_cluster (PedFileSystem* fs, char *buf, FatCluster cluster)
{
	if (!fat_write_cluster (fs, buf, cluster))
		return 0;
	if (!ped_geometry_sync (fs->geom))
		return 0;
	return 1;
}

#endif /* !DISCOVER_ONLY */
