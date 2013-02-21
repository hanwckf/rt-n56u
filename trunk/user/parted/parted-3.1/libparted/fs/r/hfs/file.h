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

#ifndef _FILE_H
#define _FILE_H

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include "hfs.h"

HfsPrivateFile*
hfs_file_open (PedFileSystem *fs, uint32_t CNID,
	       HfsExtDataRec ext_desc, PedSector sect_nb);

void
hfs_file_close (HfsPrivateFile* file);

int
hfs_file_read_sector (HfsPrivateFile* file, void *buf, PedSector sector);

int
hfs_file_write_sector (HfsPrivateFile* file, void *buf, PedSector sector);

#endif /* _FILE_H */
