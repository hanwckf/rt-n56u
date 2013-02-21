/*
    ext2.h -- ext2 header
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

#ifndef _EXT2_H
#define _EXT2_H

#include <parted/parted.h>
#include <parted/debug.h>
#include <sys/types.h>

#include <inttypes.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

typedef u_int32_t blk_t;

#ifdef HAVE_LINUX_EXT2_FS_H__FAILS_TO_COMPILE
#include <linux/ext2_fs.h>
#else
#include "ext2_fs.h"
#endif

struct ext2_fs
{
	struct ext2_dev_handle		 *devhandle;

	struct ext2_super_block		  sb;
	struct ext2_group_desc		 *gd;
	struct ext2_buffer_cache	 *bc;
	int				  metadirty;			/* 0:all sb&gd copies clean
									   1:all sb&gd copies dirty
									   2:only first sb&gd copy clean */

	int				  dynamic_version;
	int				  sparse;			/* sparse superblocks */
	int				  has_journal;			/* journal */
	int				  has_internal_journal;

	int				  blocksize;
	int				  logsize;
	blk_t				  adminblocks;
	blk_t				  gdblocks;
	blk_t				  itoffset;
	blk_t				  inodeblocks;
	int				  numgroups;
	int				  r_frac;			/* reserved % of blocks */

	unsigned char			 *relocator_pool;
	unsigned char			 *relocator_pool_end;

	int				 opt_debug;
	int				 opt_safe;
	int				 opt_verbose;

	void				 *journal;
};

#endif
