/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2004, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef _FILE_PLUS_H
#define _FILE_PLUS_H

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include "hfs.h"

HfsPPrivateFile*
hfsplus_file_open (PedFileSystem *fs, HfsPNodeID CNID,
		   HfsPExtDataRec ext_desc, PedSector sect_nb);

void
hfsplus_file_close (HfsPPrivateFile* file);

int
hfsplus_file_read(HfsPPrivateFile* file, void *buf,
		  PedSector sector, unsigned int nb);

int
hfsplus_file_write(HfsPPrivateFile* file, void *buf,
		  PedSector sector, unsigned int nb);

/* Read the nth sector of a file */
/* return 0 on error */
static __inline__ int
hfsplus_file_read_sector (HfsPPrivateFile* file, void *buf, PedSector sector)
{
	return hfsplus_file_read(file, buf, sector, 1);
}

/* Write the nth sector of a file */
/* return 0 on error */
static __inline__ int
hfsplus_file_write_sector (HfsPPrivateFile* file, void *buf, PedSector sector)
{
	return hfsplus_file_write(file, buf, sector, 1);
}


#endif /* _FILE_PLUS_H */
