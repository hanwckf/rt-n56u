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

#ifndef PED_FAT_CALC_H
#define PED_FAT_CALC_H

extern PedSector fat_min_cluster_size (FatType fat_type);
extern PedSector fat_max_cluster_size (FatType fat_type);
extern FatCluster fat_min_cluster_count (FatType fat_type);
extern FatCluster fat_max_cluster_count (FatType fat_type);

extern PedSector fat_min_reserved_sector_count (FatType fat_type);

extern int fat_check_resize_geometry (const PedFileSystem* fs,
	       			      const PedGeometry* geom,
				      PedSector new_cluster_sectors,
				      FatCluster new_cluster_count);

extern int fat_calc_sizes (PedSector size,
			   PedSector align,
			   FatType fat_type,
			   PedSector root_dir_sectors,
			   PedSector* out_cluster_sectors,
			   FatCluster* out_cluster_count,
			   PedSector* out_fat_size);

extern int fat_calc_resize_sizes (const PedGeometry* geom,
				  PedSector align,
				  FatType fat_type,
				  PedSector root_dir_sectors,
				  PedSector cluster_sectors,
				  PedSector* out_cluster_sectors,
				  FatCluster* out_cluster_count,
				  PedSector* out_fat_size);

extern PedSector
fat_calc_align_sectors (const PedFileSystem* new_fs,
			const PedFileSystem* old_fs);

extern int
fat_is_sector_in_clusters (const PedFileSystem* fs, PedSector sector);

extern FatFragment
fat_cluster_to_frag (const PedFileSystem* fs, FatCluster cluster);

extern FatCluster
fat_frag_to_cluster (const PedFileSystem* fs, FatFragment frag);

extern PedSector
fat_frag_to_sector (const PedFileSystem* fs, FatFragment frag);

extern FatFragment
fat_sector_to_frag (const PedFileSystem* fs, PedSector sector);

extern PedSector
fat_cluster_to_sector (const PedFileSystem* fs, FatCluster cluster);

extern FatCluster
fat_sector_to_cluster (const PedFileSystem* fs, PedSector sector);

#endif /* PED_FAT_CALC_H */
