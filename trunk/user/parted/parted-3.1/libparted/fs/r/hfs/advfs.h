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

#ifndef _ADVFS_H
#define _ADVFS_H

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include "hfs.h"

int
hfs_btree_search (HfsPrivateFile* b_tree_file, HfsPrivateGenericKey* key,
		  void *record_out, unsigned int record_size,
		  HfsCPrivateLeafRec* record_ref);

void
hfs_free_bad_blocks_list(HfsPrivateLinkExtent* first);

int
hfs_read_bad_blocks (const PedFileSystem *fs);

int
hfs_is_bad_block (const PedFileSystem *fs, unsigned int fblock);

PedSector
hfs_get_empty_end (const PedFileSystem *fs);

unsigned int
hfs_find_start_pack (const PedFileSystem *fs, unsigned int fblock);

#endif /* _ADVFS_H */
