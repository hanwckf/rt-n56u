/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2004-2005, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef DISCOVER_ONLY

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
#include "advfs.h"

#include "file.h"

/* Open the data fork of a file with its first three extents and its CNID */
HfsPrivateFile*
hfs_file_open (PedFileSystem *fs, uint32_t CNID,
	       HfsExtDataRec ext_desc, PedSector sect_nb)
{
	HfsPrivateFile* file;

	file = (HfsPrivateFile*) ped_malloc (sizeof (HfsPrivateFile));
	if (!file) return NULL;

	file->fs = fs;
	file->sect_nb = sect_nb;
	file->CNID = CNID;
	memcpy(file->first, ext_desc, sizeof (HfsExtDataRec));
	file->start_cache = 0;

	return file;
}

/* Close an HFS file */
void
hfs_file_close (HfsPrivateFile* file)
{
	free (file);
}

/* warning : only works on data forks */
static int
hfs_get_extent_containing (HfsPrivateFile* file, unsigned int block,
			   HfsExtDataRec cache, uint16_t* ptr_start_cache)
{
	uint8_t			record[sizeof (HfsExtentKey)
				       + sizeof (HfsExtDataRec)];
	HfsExtentKey		search;
	HfsExtentKey*		ret_key = (HfsExtentKey*) record;
	HfsExtDescriptor*	ret_cache = (HfsExtDescriptor*)
					      (record + sizeof (HfsExtentKey));
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
					      file->fs->type_specific;

	search.key_length = sizeof (HfsExtentKey) - 1;
	search.type = HFS_DATA_FORK;
	search.file_ID = file->CNID;
	search.start = PED_CPU_TO_BE16 (block);

	if (!hfs_btree_search (priv_data->extent_file,
			       (HfsPrivateGenericKey*) &search,
			       record, sizeof (record), NULL))
		return 0;

	if (ret_key->file_ID != search.file_ID || ret_key->type != search.type)
		return 0;

	memcpy (cache, ret_cache, sizeof(HfsExtDataRec));
	*ptr_start_cache = PED_BE16_TO_CPU (ret_key->start);

	return 1;
}

/* find and return the nth sector of a file */
/* return 0 on error */
static PedSector
hfs_file_find_sector (HfsPrivateFile* file, PedSector sector)
{
	HfsPrivateFSData* priv_data = (HfsPrivateFSData*)
				      file->fs->type_specific;
	unsigned int 	sect_by_block = PED_BE32_TO_CPU (
					    priv_data->mdb->block_size)
					/ PED_SECTOR_SIZE_DEFAULT;
	unsigned int 	i, s, vol_block;
	unsigned int 	block  = sector / sect_by_block;
	unsigned int	offset = sector % sect_by_block;

	/* in the three first extent */
	for (s = 0, i = 0; i < HFS_EXT_NB; i++) {
			if ((block >= s) && ( block < s + PED_BE16_TO_CPU (
						file->first[i].block_count))) {
			vol_block = (block - s) + PED_BE16_TO_CPU (
						    file->first[i].start_block);
			goto sector_found;
		}
		s += PED_BE16_TO_CPU (file->first[i].block_count);
	}

	/* in the three cached extent */
	if (file->start_cache && block >= file->start_cache)
	for (s = file->start_cache, i = 0; i < HFS_EXT_NB; i++) {
		if ((block >= s) && (block < s + PED_BE16_TO_CPU (
						file->cache[i].block_count))) {
			vol_block = (block - s) + PED_BE16_TO_CPU (
						    file->cache[i].start_block);
			goto sector_found;
		}
		s += PED_BE16_TO_CPU (file->cache[i].block_count);
	}

	/* update cache */
	if (!hfs_get_extent_containing (file, block, file->cache,
					&(file->start_cache))) {
		ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_CANCEL,
			_("Could not update the extent cache for HFS file with "
			  "CNID %X."),
			PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	/* in the three cached extent */
	PED_ASSERT(file->start_cache && block >= file->start_cache);
	for (s = file->start_cache, i = 0; i < HFS_EXT_NB; i++) {
		if ((block >= s) && (block < s + PED_BE16_TO_CPU (
						file->cache[i].block_count))) {
			vol_block = (block - s) + PED_BE16_TO_CPU (
						    file->cache[i].start_block);
			goto sector_found;
		}
		s += PED_BE16_TO_CPU (file->cache[i].block_count);
	}

	return 0;

    sector_found:
	return (PedSector) PED_BE16_TO_CPU (priv_data->mdb->start_block)
		+ (PedSector) vol_block * sect_by_block
		+ offset;
}

/* Read the nth sector of a file */
/* return 0 on error */
int
hfs_file_read_sector (HfsPrivateFile* file, void *buf, PedSector sector)
{
	PedSector	abs_sector;

	if (sector >= file->sect_nb) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Trying to read HFS file with CNID %X behind EOF."),
			PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	abs_sector = hfs_file_find_sector (file, sector);
	if (!abs_sector) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Could not find sector %lli of HFS file with "
			  "CNID %X."),
			sector, PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	return ped_geometry_read (file->fs->geom, buf, abs_sector, 1);
}

/* Write the nth sector of a file */
/* return 0 on error */
int
hfs_file_write_sector (HfsPrivateFile* file, void *buf, PedSector sector)
{
	PedSector	abs_sector;

	if (sector >= file->sect_nb) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Trying to write HFS file with CNID %X behind EOF."),
			  PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	abs_sector = hfs_file_find_sector (file, sector);
	if (!abs_sector) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Could not find sector %lli of HFS file with "
			  "CNID %X."),
			sector, PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	return ped_geometry_write (file->fs->geom, buf, abs_sector, 1);
}

#endif /* !DISCOVER_ONLY */
