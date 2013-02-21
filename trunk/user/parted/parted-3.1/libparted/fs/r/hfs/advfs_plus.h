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

#ifndef _ADVFS_PLUS_H
#define _ADVFS_PLUS_H

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include "hfs.h"

int
hfsplus_btree_search (HfsPPrivateFile* b_tree_file, HfsPPrivateGenericKey* key,
		      void *record_out, unsigned int record_size,
		      HfsCPrivateLeafRec* record_ref);

void
hfsplus_free_bad_blocks_list(HfsPPrivateLinkExtent* first);

int
hfsplus_read_bad_blocks (const PedFileSystem *fs);

int
hfsplus_is_bad_block (const PedFileSystem *fs, unsigned int fblock);

PedSector
hfsplus_get_empty_end (const PedFileSystem *fs);

PedSector
hfsplus_get_min_size (const PedFileSystem *fs);

unsigned int
hfsplus_find_start_pack (const PedFileSystem *fs, unsigned int fblock);

#endif /* _ADVFS_PLUS_H */
