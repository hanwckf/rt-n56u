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

int
hfsc_can_use_geom (PedGeometry* geom)
{
	PedDevice* dev;

	dev = geom->dev;
	PED_ASSERT (geom != NULL);
	PED_ASSERT (dev != NULL);

	if (dev->sector_size != PED_SECTOR_SIZE_DEFAULT) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Parted can't use HFS file systems on disks "
			  "with a sector size not equal to %d bytes."),
			(int)PED_SECTOR_SIZE_DEFAULT );
		return 0;
	}

	return 1;
}

/* Probe an HFS volume, detecting it even if
it is in fact a wrapper to an HFS+ volume */
/* Used by hfsplus_probe and hfs_probe */
PedGeometry*
hfs_and_wrapper_probe (PedGeometry* geom)
{
	uint8_t		buf[PED_SECTOR_SIZE_DEFAULT];
	HfsMasterDirectoryBlock	*mdb;
	PedGeometry*	geom_ret;
	PedSector	search, max;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (hfsc_can_use_geom (geom));

	mdb = (HfsMasterDirectoryBlock *) buf;

	/* is 5 an intelligent value ? */
	if ((geom->length < 5)
	    || (!ped_geometry_read (geom, buf, 2, 1))
	    || (mdb->signature != PED_CPU_TO_BE16 (HFS_SIGNATURE)) )
		return NULL;

	search = ((PedSector) PED_BE16_TO_CPU (mdb->start_block)
		  + ((PedSector) PED_BE16_TO_CPU (mdb->total_blocks)
		     * (PED_BE32_TO_CPU (mdb->block_size) / PED_SECTOR_SIZE_DEFAULT )));
	max = search + (PED_BE32_TO_CPU (mdb->block_size) / PED_SECTOR_SIZE_DEFAULT);
	if ((search < 0)
	    || !(geom_ret = ped_geometry_new (geom->dev, geom->start, search + 2)))
		return NULL;

	for (; search < max; search++) {
		if (!ped_geometry_set (geom_ret, geom_ret->start, search + 2)
		    || !ped_geometry_read (geom_ret, buf, search, 1))
			break;
		if (mdb->signature == PED_CPU_TO_BE16 (HFS_SIGNATURE))
			return geom_ret;
	}

	ped_geometry_destroy (geom_ret);
	return NULL;
}

PedGeometry*
hfsplus_probe (PedGeometry* geom)
{
	PedGeometry*	geom_ret;
	uint8_t		buf[PED_SECTOR_SIZE_DEFAULT];

	PED_ASSERT (geom != NULL);

	if (!hfsc_can_use_geom (geom))
		return NULL;

	if ((geom_ret = hfs_and_wrapper_probe(geom))) {
		/* HFS+ is embedded in an HFS volume ? */
		HfsMasterDirectoryBlock	*mdb;
		mdb = (HfsMasterDirectoryBlock *) buf;

		if (!ped_geometry_read (geom, buf, 2, 1)
		    || (mdb->old_new.embedded.signature
			!= PED_CPU_TO_BE16 (HFSP_SIGNATURE))) {
			ped_geometry_destroy (geom_ret);
			return NULL;
		} else
			return geom_ret;
	} else {
		/* This is a standalone HFS+ volume ? */
		PedSector	search, max;
		HfsPVolumeHeader *vh;
		vh = (HfsPVolumeHeader *) buf;

		if ((geom->length < 5)
		    || !ped_geometry_read (geom, buf, 2, 1)
		    || (vh->signature != PED_CPU_TO_BE16 (HFSP_SIGNATURE)))
			return NULL;

		/* Correct range is indeed [ blocks*sz-2;(blocs+1)*sz-2 ( */
		/* But previous versions of my implementation used to */
		/* assume range is [(blocks-1)*sz-1;(blocks*sz) ( */
		/* (blocks-1)*sz-1 has to be scanned last, because */
		/* it can belong to a regular file */
		max = ((PedSector) PED_BE32_TO_CPU (vh->total_blocks) + 1)
		      * ( PED_BE32_TO_CPU (vh->block_size) / PED_SECTOR_SIZE_DEFAULT )
		      - 2;
		search = max - 2 * ( PED_BE32_TO_CPU (vh->block_size)
				     / PED_SECTOR_SIZE_DEFAULT ) + 2;
		if ((search < 0)
		    || !(geom_ret = ped_geometry_new (geom->dev, geom->start,
						      search + 2)))
			return NULL;

		for (; search < max; search++) {
			if (!ped_geometry_set (geom_ret, geom_ret->start,
					       search + 2)
			    || !ped_geometry_read (geom_ret, buf, search, 1))
				break;
			if (vh->signature == PED_CPU_TO_BE16 (HFSP_SIGNATURE))
				return geom_ret;
		}
		search = ((PedSector) PED_BE32_TO_CPU (vh->total_blocks) - 1)
		      * ( PED_BE32_TO_CPU (vh->block_size) / PED_SECTOR_SIZE_DEFAULT )
		      - 1;
		if ((search < 0)
		    || !ped_geometry_set (geom_ret, geom_ret->start,
					  search + 2)
		    || !ped_geometry_read (geom_ret, buf, search, 1)
		    || vh->signature != PED_CPU_TO_BE16 (HFSP_SIGNATURE)) {
		    	ped_geometry_destroy (geom_ret);
			return NULL;
		} else
			return geom_ret;
	}
}

PedGeometry*
hfs_probe (PedGeometry* geom)
{
	PedGeometry*	geom_base;
	PedGeometry*	geom_plus = NULL;

	PED_ASSERT (geom != NULL);

	if (!hfsc_can_use_geom (geom))
		return NULL;

	if ((geom_base = hfs_and_wrapper_probe(geom))
	    && (!(geom_plus = hfsplus_probe(geom_base))))
		return geom_base;
	else {
		if (geom_base) ped_geometry_destroy (geom_base);
		if (geom_plus) ped_geometry_destroy (geom_plus);
		return NULL;
	}
}

PedGeometry*
hfsx_probe (PedGeometry* geom)
{
	PedGeometry*	geom_ret;
	uint8_t		buf[PED_SECTOR_SIZE_DEFAULT];
	PedSector	search, max;
	HfsPVolumeHeader *vh = (HfsPVolumeHeader *) buf;

	PED_ASSERT (geom != NULL);

	if (!hfsc_can_use_geom (geom))
		return NULL;

	if ((geom->length < 5)
	    || !ped_geometry_read (geom, buf, 2, 1)
	    || (vh->signature != PED_CPU_TO_BE16 (HFSX_SIGNATURE)))
		return NULL;

	/* unlike the hfs+ code, which should be kept compatible
	with my old previous implementations, we only care here
	about legal alternate VH positions, like TN1150 describes them */
	max = ((PedSector) PED_BE32_TO_CPU (vh->total_blocks) + 1)
		      * ( PED_BE32_TO_CPU (vh->block_size) / PED_SECTOR_SIZE_DEFAULT )
		      - 2;
	search = max - ( PED_BE32_TO_CPU (vh->block_size) / PED_SECTOR_SIZE_DEFAULT );
	if ((search < 0)
	    || !(geom_ret = ped_geometry_new (geom->dev, geom->start,
					      search + 2)))
		return NULL;
	for (; search < max; search++) {
		if (!ped_geometry_set (geom_ret, geom_ret->start,
				       search + 2)
		    || !ped_geometry_read (geom_ret, buf, search, 1))
			break;
		if (vh->signature == PED_CPU_TO_BE16 (HFSX_SIGNATURE))
			return geom_ret;
	}

	ped_geometry_destroy (geom_ret);
	return NULL;
}
