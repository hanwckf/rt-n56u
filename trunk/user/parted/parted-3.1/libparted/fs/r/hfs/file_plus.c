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
#include "advfs_plus.h"

#include "file_plus.h"

/* Open the data fork of a file with its first eight extents and its CNID */
/* CNID and ext_desc must be in disc order, sect_nb in CPU order */
/* return null on failure */
HfsPPrivateFile*
hfsplus_file_open (PedFileSystem *fs, HfsPNodeID CNID,
		   HfsPExtDataRec ext_desc, PedSector sect_nb)
{
	HfsPPrivateFile* file;

	file = (HfsPPrivateFile*) ped_malloc (sizeof (HfsPPrivateFile));
	if (!file) return NULL;

	file->fs = fs;
	file->sect_nb = sect_nb;
	file->CNID = CNID;
	memcpy(file->first, ext_desc, sizeof (HfsPExtDataRec));
	file->start_cache = 0;

	return file;
}

/* Close an HFS+ file */
void
hfsplus_file_close (HfsPPrivateFile* file)
{
	free (file);
}

/* warning : only works on data forks */
static int
hfsplus_get_extent_containing (HfsPPrivateFile* file, unsigned int block,
			       HfsPExtDataRec cache, uint32_t* ptr_start_cache)
{
	uint8_t			record[sizeof (HfsPExtentKey)
				       + sizeof (HfsPExtDataRec)];
	HfsPExtentKey		search;
	HfsPExtentKey*		ret_key = (HfsPExtentKey*) record;
	HfsPExtDescriptor*	ret_cache = (HfsPExtDescriptor*)
					      (record + sizeof (HfsPExtentKey));
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						file->fs->type_specific;

	search.key_length = PED_CPU_TO_BE16 (sizeof (HfsPExtentKey) - 2);
	search.type = HFS_DATA_FORK;
	search.pad = 0;
	search.file_ID = file->CNID;
	search.start = PED_CPU_TO_BE32 (block);

	if (!hfsplus_btree_search (priv_data->extents_file,
				   (HfsPPrivateGenericKey*) &search,
				   record, sizeof (record), NULL))
		return 0;

	if (ret_key->file_ID != search.file_ID || ret_key->type != search.type)
		return 0;

	memcpy (cache, ret_cache, sizeof(HfsPExtDataRec));
	*ptr_start_cache = PED_BE32_TO_CPU (ret_key->start);

	return 1;
}

/* find a sub extent contained in the desired area */
/* and with the same starting point */
/* return 0 in sector_count on error, or the physical area */
/* on the volume corresponding to the logical area in the file */
static HfsPPrivateExtent
hfsplus_file_find_extent (HfsPPrivateFile* file, PedSector sector,
			  unsigned int nb)
{
	HfsPPrivateExtent ret = {0,0};
	HfsPPrivateFSData* priv_data = (HfsPPrivateFSData*)
					file->fs->type_specific;
	unsigned int	sect_by_block = PED_BE32_TO_CPU (
					    priv_data->vh->block_size)
					/ PED_SECTOR_SIZE_DEFAULT;
	unsigned int	i, s, vol_block, size;
	PedSector	sect_size;
	unsigned int	block  = sector / sect_by_block;
	unsigned int	offset = sector % sect_by_block;

	/* in the 8 first extent */
	for (s = 0, i = 0; i < HFSP_EXT_NB; i++) {
		if ((block >= s) && (block < s + PED_BE32_TO_CPU (
						file->first[i].block_count))) {
			vol_block = (block - s)
				    + PED_BE32_TO_CPU (file->first[i]
						       .start_block);
			size = PED_BE32_TO_CPU (file->first[i].block_count)
				+ s - block;
			goto plus_sector_found;
		}
		s += PED_BE32_TO_CPU (file->first[i].block_count);
	}

	/* in the 8 cached extent */
	if (file->start_cache && block >= file->start_cache)
	for (s = file->start_cache, i = 0; i < HFSP_EXT_NB; i++) {
		if ((block >= s) && (block < s + PED_BE32_TO_CPU (
						file->cache[i].block_count))) {
			vol_block = (block - s)
				    + PED_BE32_TO_CPU (file->cache[i]
						       .start_block);
			size = PED_BE32_TO_CPU (file->cache[i].block_count)
				+ s - block;
			goto plus_sector_found;
		}
		s += PED_BE32_TO_CPU (file->cache[i].block_count);
	}

	/* update cache */
	if (!hfsplus_get_extent_containing (file, block, file->cache,
					    &(file->start_cache))) {
		ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_CANCEL,
			_("Could not update the extent cache for HFS+ file "
			  "with CNID %X."),
			PED_BE32_TO_CPU(file->CNID));
		return ret; /* ret == {0,0} */
	}

	/* ret == {0,0} */
	PED_ASSERT(file->start_cache && block >= file->start_cache);

	for (s = file->start_cache, i = 0; i < HFSP_EXT_NB; i++) {
		if ((block >= s) && (block < s + PED_BE32_TO_CPU (
						file->cache[i].block_count))) {
			vol_block = (block - s)
				    + PED_BE32_TO_CPU (file->cache[i]
						       .start_block);
			size = PED_BE32_TO_CPU (file->cache[i].block_count)
				+ s - block;
			goto plus_sector_found;
		}
		s += PED_BE32_TO_CPU (file->cache[i].block_count);
	}

	return ret;

plus_sector_found:
	sect_size = (PedSector) size * sect_by_block - offset;
	ret.start_sector = vol_block * sect_by_block + offset;
	ret.sector_count = (sect_size < nb) ? sect_size : nb;
	return ret;
}

int
hfsplus_file_read(HfsPPrivateFile* file, void *buf, PedSector sector,
		  unsigned int nb)
{
	HfsPPrivateExtent phy_area;
	HfsPPrivateFSData* priv_data = (HfsPPrivateFSData*)
					file->fs->type_specific;
        char *b = buf;

	if (sector+nb < sector /* detect overflow */
	    || sector+nb > file->sect_nb) /* out of file */ {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Trying to read HFS+ file with CNID %X behind EOF."),
			PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	while (nb) {
		phy_area = hfsplus_file_find_extent(file, sector, nb);
		if (phy_area.sector_count == 0) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Could not find sector %lli of HFS+ file "
				  "with CNID %X."),
				sector, PED_BE32_TO_CPU(file->CNID));
			return 0;
		}
                if (!ped_geometry_read(priv_data->plus_geom, b,
				       phy_area.start_sector,
				       phy_area.sector_count))
			return 0;

		nb -= phy_area.sector_count; /* < nb anyway ... */
		sector += phy_area.sector_count;
                b += phy_area.sector_count * PED_SECTOR_SIZE_DEFAULT;
	}

	return 1;
}

int
hfsplus_file_write(HfsPPrivateFile* file, void *buf, PedSector sector,
		  unsigned int nb)
{
	HfsPPrivateExtent phy_area;
	HfsPPrivateFSData* priv_data = (HfsPPrivateFSData*)
					file->fs->type_specific;
        char *b = buf;

	if (sector+nb < sector /* detect overflow */
	    || sector+nb > file->sect_nb) /* out of file */ {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Trying to write HFS+ file with CNID %X behind EOF."),
			PED_BE32_TO_CPU(file->CNID));
		return 0;
	}

	while (nb) {
		phy_area = hfsplus_file_find_extent(file, sector, nb);
		if (phy_area.sector_count == 0) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Could not find sector %lli of HFS+ file "
				  "with CNID %X."),
				sector, PED_BE32_TO_CPU(file->CNID));
			return 0;
		}
                if (!ped_geometry_write(priv_data->plus_geom, b,
				       phy_area.start_sector,
				       phy_area.sector_count))
			return 0;

		nb -= phy_area.sector_count; /* < nb anyway ... */
		sector += phy_area.sector_count;
                b += phy_area.sector_count * PED_SECTOR_SIZE_DEFAULT;
	}

	return 1;
}

#endif /* !DISCOVER_ONLY */
