/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2000, 2003-2005, 2007, 2009-2012 Free Software Foundation,
    Inc.

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

/*
   Author : Guillaume Knispel <k_guillaume@libertysurf.fr>
   Report bug to <bug-parted@gnu.org>
*/

#include <config.h>

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>
#include <stdint.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "hfs.h"
#include "probe.h"

uint8_t* hfs_block = NULL;
uint8_t* hfsp_block = NULL;
unsigned hfs_block_count;
unsigned hfsp_block_count;

#define HFS_BLOCK_SIZES       ((int[2]){512, 0})
#define HFSP_BLOCK_SIZES       ((int[2]){512, 0})
#define HFSX_BLOCK_SIZES       ((int[2]){512, 0})

static PedFileSystemOps hfs_ops = {
	probe:		hfs_probe,
};

static PedFileSystemOps hfsplus_ops = {
	probe:		hfsplus_probe,
};

static PedFileSystemOps hfsx_ops = {
	probe:		hfsx_probe,
};


static PedFileSystemType hfs_type = {
	next:	NULL,
	ops:	&hfs_ops,
	name:	"hfs",
	block_sizes: HFS_BLOCK_SIZES
};

static PedFileSystemType hfsplus_type = {
	next:	NULL,
	ops:	&hfsplus_ops,
	name:	"hfs+",
	block_sizes: HFSP_BLOCK_SIZES
};

static PedFileSystemType hfsx_type = {
	next:	NULL,
	ops:	&hfsx_ops,
	name:	"hfsx",
	block_sizes: HFSX_BLOCK_SIZES
};

void
ped_file_system_hfs_init ()
{
	ped_file_system_type_register (&hfs_type);
	ped_file_system_type_register (&hfsplus_type);
	ped_file_system_type_register (&hfsx_type);
}

void
ped_file_system_hfs_done ()
{
	ped_file_system_type_unregister (&hfs_type);
	ped_file_system_type_unregister (&hfsplus_type);
	ped_file_system_type_unregister (&hfsx_type);
}
