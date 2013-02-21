/* libparted - a library for manipulating disk partitions
   Copyright (C) 2008-2012 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stddef.h>
#include <parted/disk.h>

int ptt_write_sector (PedDisk const *disk, void const *buf, size_t buflen);
int ptt_read_sector (PedDevice const *dev, PedSector sector_num, void **buf);
int ptt_read_sectors (PedDevice const *dev, PedSector start_sector,
		      PedSector n_sectors, void **buf);
int ptt_clear_sectors (PedDevice *dev, PedSector start, PedSector count);
int ptt_geom_clear_sectors (PedGeometry *geom, PedSector start,
			    PedSector count);
int ptt_partition_max_start_len (char const *label_type,
                const PedPartition *part);

int ptt_partition_max_start_sector (char const *pt_type, PedSector *max);
int ptt_partition_max_length (char const *pt_type, PedSector *max);
