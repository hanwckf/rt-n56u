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

#ifndef DISCOVER_ONLY
#include "file.h"
#include "reloc.h"
#include "advfs.h"

static PedFileSystemType hfs_type;
static PedFileSystemType hfsplus_type;


/* ----- HFS ----- */

/* This is a very unundoable operation */
/* Maybe I shouldn't touch the alternate MDB ? */
/* Anyway clobber is call before other fs creation */
/* So this is a non-issue */
static int
hfs_clobber (PedGeometry* geom)
{
	uint8_t	buf[PED_SECTOR_SIZE_DEFAULT];

	memset (buf, 0, PED_SECTOR_SIZE_DEFAULT);

	/* destroy boot blocks, mdb, alternate mdb ... */
	return	(!!ped_geometry_write (geom, buf, 0, 1)) &
		(!!ped_geometry_write (geom, buf, 1, 1)) &
		(!!ped_geometry_write (geom, buf, 2, 1)) &
		(!!ped_geometry_write (geom, buf, geom->length - 2, 1)) &
		(!!ped_geometry_write (geom, buf, geom->length - 1, 1)) &
		(!!ped_geometry_sync  (geom));
}

PedFileSystem *
hfs_open (PedGeometry* geom)
{
	uint8_t			buf[PED_SECTOR_SIZE_DEFAULT];
	PedFileSystem*		fs;
	HfsMasterDirectoryBlock* mdb;
	HfsPrivateFSData* 	priv_data;

	if (!hfsc_can_use_geom (geom))
		return NULL;

	/* Read MDB */
	if (!ped_geometry_read (geom, buf, 2, 1))
		return NULL;

	/* Allocate memory */
	fs = (PedFileSystem*) ped_malloc (sizeof (PedFileSystem));
	if (!fs) goto ho;
	mdb = (HfsMasterDirectoryBlock*)
		ped_malloc (sizeof (HfsMasterDirectoryBlock));
	if (!mdb) goto ho_fs;
	priv_data = (HfsPrivateFSData*)
		ped_malloc (sizeof (HfsPrivateFSData));
	if (!priv_data) goto ho_mdb;

	memcpy (mdb, buf, sizeof (HfsMasterDirectoryBlock));

	/* init structures */
	priv_data->mdb = mdb;
	priv_data->bad_blocks_loaded = 0;
	priv_data->bad_blocks_xtent_nb = 0;
	priv_data->bad_blocks_xtent_list = NULL;
	priv_data->extent_file =
	    hfs_file_open (fs, PED_CPU_TO_BE32 (HFS_XTENT_ID),
			   mdb->extents_file_rec,
			   PED_CPU_TO_BE32 (mdb->extents_file_size)
			   / PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->extent_file) goto ho_pd;
	priv_data->catalog_file =
	    hfs_file_open (fs, PED_CPU_TO_BE32 (HFS_CATALOG_ID),
			   mdb->catalog_file_rec,
			   PED_CPU_TO_BE32 (mdb->catalog_file_size)
			   / PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->catalog_file) goto ho_ce;
	/* Read allocation blocks */
	if (!ped_geometry_read(geom, priv_data->alloc_map,
			       PED_BE16_TO_CPU (mdb->volume_bitmap_block),
			       ( PED_BE16_TO_CPU (mdb->total_blocks)
			         + PED_SECTOR_SIZE_DEFAULT * 8 - 1 )
			       / (PED_SECTOR_SIZE_DEFAULT * 8) ) )
		goto ho_cf;

	fs->type = &hfs_type;
	fs->geom = ped_geometry_duplicate (geom);
	if (!fs->geom) goto ho_cf;
	fs->type_specific = (void*) priv_data;
	fs->checked = ( PED_BE16_TO_CPU (mdb->volume_attributes)
			>> HFS_UNMOUNTED ) & 1;

	return fs;

/*--- clean error handling ---*/
ho_cf:	hfs_file_close(priv_data->catalog_file);
ho_ce:	hfs_file_close(priv_data->extent_file);
ho_pd:	free(priv_data);
ho_mdb: free(mdb);
ho_fs:	free(fs);
ho:	return NULL;
}

int
hfs_close (PedFileSystem *fs)
{
	HfsPrivateFSData* priv_data = (HfsPrivateFSData*) fs->type_specific;

	hfs_file_close (priv_data->extent_file);
	hfs_file_close (priv_data->catalog_file);
	if (priv_data->bad_blocks_loaded)
		hfs_free_bad_blocks_list (priv_data->bad_blocks_xtent_list);
	free (priv_data->mdb);
	free (priv_data);
	ped_geometry_destroy (fs->geom);
	free (fs);

	return 1;
}

PedConstraint *
hfs_get_resize_constraint (const PedFileSystem *fs)
{
	PedDevice*	dev = fs->geom->dev;
	PedAlignment	start_align;
	PedGeometry	start_sector;
	PedGeometry	full_dev;
	PedSector	min_size;

	if (!ped_alignment_init (&start_align, fs->geom->start, 0))
		return NULL;
	if (!ped_geometry_init (&start_sector, dev, fs->geom->start, 1))
		return NULL;
	if (!ped_geometry_init (&full_dev, dev, 0, dev->length - 1))
		return NULL;
	/* 2 = last two sectors (alternate MDB and unused sector) */
	min_size = hfs_get_empty_end(fs) + 2;
	if (min_size == 2) return NULL;

	return ped_constraint_new (&start_align, ped_alignment_any,
				   &start_sector, &full_dev, min_size,
				   fs->geom->length);
}

int
hfs_resize (PedFileSystem* fs, PedGeometry* geom, PedTimer* timer)
{
	uint8_t			buf[PED_SECTOR_SIZE_DEFAULT];
	unsigned int		nblock, nfree;
	unsigned int		block, to_free;
	HfsPrivateFSData* 	priv_data;
	HfsMasterDirectoryBlock* mdb;
	int			resize = 1;
	unsigned int		hfs_sect_block;
	PedSector		hgee;

	/* check preconditions */
	PED_ASSERT (fs != NULL);
	PED_ASSERT (fs->geom != NULL);
	PED_ASSERT (geom != NULL);
#ifdef DEBUG
        PED_ASSERT ((hgee = hfs_get_empty_end(fs)) != 0);
#else
        if ((hgee = hfs_get_empty_end(fs)) == 0)
                return 0;
#endif

	PED_ASSERT ((hgee = hfs_get_empty_end(fs)) != 0);

	if (ped_geometry_test_equal(fs->geom, geom))
		return 1;

	priv_data = (HfsPrivateFSData*) fs->type_specific;
	mdb = priv_data->mdb;
	hfs_sect_block = PED_BE32_TO_CPU (mdb->block_size)
			 / PED_SECTOR_SIZE_DEFAULT;

	if (fs->geom->start != geom->start
	    || geom->length > fs->geom->length
	    || geom->length < hgee + 2) {
		ped_exception_throw (
			PED_EXCEPTION_NO_FEATURE,
			PED_EXCEPTION_CANCEL,
			_("Sorry, HFS cannot be resized that way yet."));
		return 0;
	}

	/* Flush caches */
	if (!ped_geometry_sync(fs->geom))
		return 0;

	/* Clear the unmounted bit */
	mdb->volume_attributes &= PED_CPU_TO_BE16 (~( 1 << HFS_UNMOUNTED ));
	if (!ped_geometry_read (fs->geom, buf, 2, 1))
		return 0;
	memcpy (buf, mdb, sizeof (HfsMasterDirectoryBlock));
	if (   !ped_geometry_write (fs->geom, buf, 2, 1)
	    || !ped_geometry_sync  (fs->geom))
		return 0;

	ped_timer_reset (timer);
	ped_timer_set_state_name(timer, _("shrinking"));
	ped_timer_update(timer, 0.0);
	/* relocate data */
	to_free = ( fs->geom->length - geom->length
		    + hfs_sect_block - 1 )
		  / hfs_sect_block ;
	block = hfs_find_start_pack (fs, to_free);
	if (!hfs_pack_free_space_from_block (fs, block,	timer, to_free)) {
		resize = 0;
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Data relocation has failed."));
		goto write_MDB;
	}

	/* Calculate new block number and other MDB field */
	nblock = ( geom->length - (PED_BE16_TO_CPU (mdb->start_block) + 2) )
		 / hfs_sect_block;
	nfree = PED_BE16_TO_CPU (mdb->free_blocks)
		- ( PED_BE16_TO_CPU (mdb->total_blocks) - nblock );

	/* Check that all block after future end are really free */
	for (block = nblock;
	     block < PED_BE16_TO_CPU (mdb->total_blocks);
	     block++) {
		if (TST_BLOC_OCCUPATION(priv_data->alloc_map,block)) {
			resize = 0;
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Data relocation left some data in the end "
				  "of the volume."));
			goto write_MDB;
		}
	}

	/* Mark out of volume blocks as used
	(broken implementations compatibility) */
	for ( block = nblock; block < (1 << 16); ++block)
		SET_BLOC_OCCUPATION(priv_data->alloc_map,block);

	/* save the allocation map
	I do not write until start of allocation blocks
	but only until pre-resize end of bitmap blocks
	because the specifications do _not_ assert that everything
	until allocation blocks is boot, mdb and alloc */
	ped_geometry_write(fs->geom, priv_data->alloc_map,
		PED_BE16_TO_CPU (priv_data->mdb->volume_bitmap_block),
		( PED_BE16_TO_CPU (priv_data->mdb->total_blocks)
		  + PED_SECTOR_SIZE_DEFAULT * 8 - 1)
		/ (PED_SECTOR_SIZE_DEFAULT * 8));

	/* Update geometry */
	if (resize) {
		/* update in fs structure */
		if (PED_BE16_TO_CPU (mdb->next_allocation) >= nblock)
			mdb->next_allocation = PED_CPU_TO_BE16 (0);
		mdb->total_blocks = PED_CPU_TO_BE16 (nblock);
		mdb->free_blocks = PED_CPU_TO_BE16 (nfree);
		/* update parted structure */
		fs->geom->length = geom->length;
		fs->geom->end = fs->geom->start + geom->length - 1;
	}

	/* Set the unmounted bit */
	mdb->volume_attributes |= PED_CPU_TO_BE16 ( 1 << HFS_UNMOUNTED );

	/* Effective write */
    write_MDB:
	ped_timer_set_state_name(timer,_("writing HFS Master Directory Block"));

	if (!hfs_update_mdb(fs)) {
		ped_geometry_sync(geom);
		return 0;
	}

	if (!ped_geometry_sync(geom))
		return 0;

	ped_timer_update(timer, 1.0);

	return (resize);
}

/* ----- HFS+ ----- */

#include "file_plus.h"
#include "advfs_plus.h"
#include "reloc_plus.h"
#include "journal.h"

static int
hfsplus_clobber (PedGeometry* geom)
{
	unsigned int i = 1;
	uint8_t				buf[PED_SECTOR_SIZE_DEFAULT];
	HfsMasterDirectoryBlock		*mdb;

	mdb = (HfsMasterDirectoryBlock *) buf;

	if (!ped_geometry_read (geom, buf, 2, 1))
		return 0;

	if (PED_BE16_TO_CPU (mdb->signature) == HFS_SIGNATURE) {
		/* embedded hfs+ */
		PedGeometry	*embedded;

		i = PED_BE32_TO_CPU(mdb->block_size) / PED_SECTOR_SIZE_DEFAULT;
		embedded = ped_geometry_new (
		    geom->dev,
		    (PedSector) geom->start
		     + PED_BE16_TO_CPU (mdb->start_block)
		     + (PedSector) PED_BE16_TO_CPU (
			mdb->old_new.embedded.location.start_block ) * i,
		    (PedSector) PED_BE16_TO_CPU (
			mdb->old_new.embedded.location.block_count ) * i );
		if (!embedded) i = 0;
		else {
			i = hfs_clobber (embedded);
			ped_geometry_destroy (embedded);
		}
	}

	/* non-embedded or envelop destroy as hfs */
	return ( hfs_clobber (geom) && i );
}

int
hfsplus_close (PedFileSystem *fs)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;

	if (priv_data->bad_blocks_loaded)
		hfsplus_free_bad_blocks_list(priv_data->bad_blocks_xtent_list);
	free(priv_data->alloc_map);
	free(priv_data->dirty_alloc_map);
	hfsplus_file_close (priv_data->allocation_file);
	hfsplus_file_close (priv_data->attributes_file);
	hfsplus_file_close (priv_data->catalog_file);
	hfsplus_file_close (priv_data->extents_file);
	if (priv_data->free_geom) ped_geometry_destroy (priv_data->plus_geom);
	if (priv_data->wrapper) hfs_close(priv_data->wrapper);
	ped_geometry_destroy (fs->geom);
	free(priv_data->vh);
	free(priv_data);
	free(fs);

	return 1;
}

PedFileSystem*
hfsplus_open (PedGeometry* geom)
{
	uint8_t			buf[PED_SECTOR_SIZE_DEFAULT];
	PedFileSystem*		fs;
	HfsPVolumeHeader*	vh;
	HfsPPrivateFSData* 	priv_data;
	PedGeometry*		wrapper_geom;
	unsigned int		map_sectors;

	if (!hfsc_can_use_geom (geom))
		return NULL;

	fs = (PedFileSystem*) ped_malloc (sizeof (PedFileSystem));
	if (!fs) goto hpo;
	vh = (HfsPVolumeHeader*) ped_malloc (sizeof (HfsPVolumeHeader));
	if (!vh) goto hpo_fs;
	priv_data = (HfsPPrivateFSData*)ped_malloc (sizeof (HfsPPrivateFSData));
	if (!priv_data) goto hpo_vh;

	fs->geom = ped_geometry_duplicate (geom);
	if (!fs->geom) goto hpo_pd;
	fs->type_specific = (void*) priv_data;

	if ((wrapper_geom = hfs_and_wrapper_probe (geom))) {
		HfsPrivateFSData* 	hfs_priv_data;
		PedSector		abs_sect, length;
		unsigned int		bs;

		ped_geometry_destroy (wrapper_geom);
		priv_data->wrapper = hfs_open(geom);
		if (!priv_data->wrapper) goto hpo_gm;
		hfs_priv_data = (HfsPrivateFSData*)
			priv_data->wrapper->type_specific;
		bs = PED_BE32_TO_CPU (hfs_priv_data->mdb->block_size)
		     / PED_SECTOR_SIZE_DEFAULT;
		abs_sect = (PedSector) geom->start
			   + (PedSector) PED_BE16_TO_CPU (
					    hfs_priv_data->mdb->start_block)
			   + (PedSector) PED_BE16_TO_CPU (
					    hfs_priv_data->mdb->old_new
					    .embedded.location.start_block )
			                 * bs;
		length = (PedSector) PED_BE16_TO_CPU (
					    hfs_priv_data->mdb->old_new
					    .embedded.location.block_count)
				     * bs;
		priv_data->plus_geom = ped_geometry_new (geom->dev, abs_sect,
							 length);
		if (!priv_data->plus_geom) goto hpo_wr;
		priv_data->free_geom = 1;
	} else {
		priv_data->wrapper = NULL;
		priv_data->plus_geom = fs->geom;
		priv_data->free_geom = 0;
	}

	if (!ped_geometry_read (priv_data->plus_geom, buf, 2, 1)) goto hpo_pg;
	memcpy (vh, buf, sizeof (HfsPVolumeHeader));
	priv_data->vh = vh;

	if (vh->signature != PED_CPU_TO_BE16(HFSP_SIGNATURE)
	    && vh->signature != PED_CPU_TO_BE16(HFSX_SIGNATURE)) {
		ped_exception_throw (
			PED_EXCEPTION_BUG,
			PED_EXCEPTION_CANCEL,
			_("No valid HFS[+X] signature has been found while "
			  "opening."));
		goto hpo_pg;
	}

	if (vh->signature == PED_CPU_TO_BE16(HFSP_SIGNATURE)
	    && vh->version != PED_CPU_TO_BE16(HFSP_VERSION)) {
		if (ped_exception_throw (
			PED_EXCEPTION_NO_FEATURE,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Version %d of HFS+ isn't supported."),
			PED_BE16_TO_CPU(vh->version))
				!= PED_EXCEPTION_IGNORE)
			goto hpo_pg;
	}

	if (vh->signature == PED_CPU_TO_BE16(HFSX_SIGNATURE)
	    && vh->version != PED_CPU_TO_BE16(HFSX_VERSION)) {
		if (ped_exception_throw (
			PED_EXCEPTION_NO_FEATURE,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Version %d of HFSX isn't supported."),
			PED_BE16_TO_CPU(vh->version))
				!= PED_EXCEPTION_IGNORE)
			goto hpo_pg;
	}

	priv_data->jib_start_block = 0;
	priv_data->jl_start_block = 0;
	if (vh->attributes & PED_CPU_TO_BE32(1<<HFSP_JOURNALED)) {
		if (!hfsj_replay_journal(fs))
			goto hpo_pg;
	}

	priv_data->bad_blocks_loaded = 0;
	priv_data->bad_blocks_xtent_nb = 0;
	priv_data->bad_blocks_xtent_list = NULL;
	priv_data->extents_file =
		hfsplus_file_open (fs, PED_CPU_TO_BE32 (HFS_XTENT_ID),
				   vh->extents_file.extents,
				   PED_BE64_TO_CPU (
					vh->extents_file.logical_size )
				   / PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->extents_file) goto hpo_pg;
	priv_data->catalog_file =
		hfsplus_file_open (fs, PED_CPU_TO_BE32 (HFS_CATALOG_ID),
				   vh->catalog_file.extents,
				   PED_BE64_TO_CPU (
					vh->catalog_file.logical_size )
				   / PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->catalog_file) goto hpo_ce;
	priv_data->attributes_file =
		hfsplus_file_open (fs, PED_CPU_TO_BE32 (HFSP_ATTRIB_ID),
				   vh->attributes_file.extents,
				   PED_BE64_TO_CPU (
					vh->attributes_file.logical_size)
				   / PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->attributes_file) goto hpo_cc;

	map_sectors = ( PED_BE32_TO_CPU (vh->total_blocks)
	                + PED_SECTOR_SIZE_DEFAULT * 8 - 1 )
		      / (PED_SECTOR_SIZE_DEFAULT * 8);
	priv_data->dirty_alloc_map = (uint8_t*)
		ped_malloc ((map_sectors + 7) / 8);
	if (!priv_data->dirty_alloc_map) goto hpo_cl;
	memset(priv_data->dirty_alloc_map, 0, (map_sectors + 7) / 8);
	priv_data->alloc_map = (uint8_t*)
		ped_malloc (map_sectors * PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->alloc_map) goto hpo_dm;

	priv_data->allocation_file =
		hfsplus_file_open (fs, PED_CPU_TO_BE32 (HFSP_ALLOC_ID),
				   vh->allocation_file.extents,
				   PED_BE64_TO_CPU (
					vh->allocation_file.logical_size)
				   / PED_SECTOR_SIZE_DEFAULT);
	if (!priv_data->allocation_file) goto hpo_am;
	if (!hfsplus_file_read (priv_data->allocation_file,
				priv_data->alloc_map, 0, map_sectors)) {
		hfsplus_close(fs);
		return NULL;
	}

	fs->type = &hfsplus_type;
	fs->checked = ((PED_BE32_TO_CPU (vh->attributes) >> HFS_UNMOUNTED) & 1)
	      && !((PED_BE32_TO_CPU (vh->attributes) >> HFSP_INCONSISTENT) & 1);

	return fs;

/*--- clean error handling ---*/
hpo_am: free(priv_data->alloc_map);
hpo_dm: free(priv_data->dirty_alloc_map);
hpo_cl: hfsplus_file_close (priv_data->attributes_file);
hpo_cc:	hfsplus_file_close (priv_data->catalog_file);
hpo_ce:	hfsplus_file_close (priv_data->extents_file);
hpo_pg: if (priv_data->free_geom) ped_geometry_destroy (priv_data->plus_geom);
hpo_wr: if (priv_data->wrapper) hfs_close(priv_data->wrapper);
hpo_gm: ped_geometry_destroy (fs->geom);
hpo_pd: free(priv_data);
hpo_vh: free(vh);
hpo_fs: free(fs);
hpo:	return NULL;
}

PedConstraint *
hfsplus_get_resize_constraint (const PedFileSystem *fs)
{
	PedDevice*	dev = fs->geom->dev;
	PedAlignment	start_align;
	PedGeometry	start_sector;
	PedGeometry	full_dev;
	PedSector	min_size;

	if (!ped_alignment_init (&start_align, fs->geom->start, 0))
		return NULL;
	if (!ped_geometry_init (&start_sector, dev, fs->geom->start, 1))
		return NULL;
	if (!ped_geometry_init (&full_dev, dev, 0, dev->length - 1))
		return NULL;

	min_size = hfsplus_get_min_size (fs);
	if (!min_size) return NULL;

	return ped_constraint_new (&start_align, ped_alignment_any,
				   &start_sector, &full_dev, min_size,
				   fs->geom->length);
}

static int
hfsplus_volume_resize (PedFileSystem* fs, PedGeometry* geom, PedTimer* timer)
{
	uint8_t			buf[PED_SECTOR_SIZE_DEFAULT];
	unsigned int		nblock, nfree, mblock;
	unsigned int		block, to_free, old_blocks;
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPVolumeHeader* 	vh = priv_data->vh;
	int			resize = 1;
	unsigned int		hfsp_sect_block =
				    ( PED_BE32_TO_CPU (vh->block_size)
				      / PED_SECTOR_SIZE_DEFAULT );
	unsigned int		map_sectors;

	old_blocks = PED_BE32_TO_CPU (vh->total_blocks);

	/* Flush caches */
	if (!ped_geometry_sync(priv_data->plus_geom))
		return 0;

	/* Clear the unmounted bit */
	/* and set the implementation code (Apple Creator Code) */
	vh->attributes &= PED_CPU_TO_BE32 (~( 1 << HFS_UNMOUNTED ));
	vh->last_mounted_version = PED_CPU_TO_BE32(HFSP_IMPL_Shnk);
	if (!ped_geometry_read (priv_data->plus_geom, buf, 2, 1))
		return 0;
	memcpy (buf, vh, sizeof (HfsPVolumeHeader));
	if (   !ped_geometry_write (priv_data->plus_geom, buf, 2, 1)
	    || !ped_geometry_sync (priv_data->plus_geom))
		return 0;

	ped_timer_reset (timer);
	ped_timer_set_state_name(timer, _("shrinking"));
	ped_timer_update(timer, 0.0);
	/* relocate data */
	to_free = ( priv_data->plus_geom->length
	          - geom->length + hfsp_sect_block
		  - 1 ) / hfsp_sect_block;
	block = hfsplus_find_start_pack (fs, to_free);
	if (!hfsplus_pack_free_space_from_block (fs, block, timer, to_free)) {
		resize = 0;
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Data relocation has failed."));
		goto write_VH;
	}

	/* Calculate new block number and other VH field */
	/* nblock must be rounded _down_ */
	nblock = geom->length / hfsp_sect_block;
	nfree = PED_BE32_TO_CPU (vh->free_blocks)
		- (old_blocks - nblock);
	/* free block readjustement is only needed when incorrect nblock
	   was used by my previous implementation, so detect the case */
	if (priv_data->plus_geom->length < old_blocks
					   * ( PED_BE32_TO_CPU (vh->block_size)
					       / PED_SECTOR_SIZE_DEFAULT) ) {
		if (priv_data->plus_geom->length % hfsp_sect_block == 1)
			nfree++;
	}

	/* Check that all block after future end are really free */
	mblock = ( priv_data->plus_geom->length - 2 )
		 / hfsp_sect_block;
	if (mblock > old_blocks - 1)
		mblock = old_blocks - 1;
	for ( block = nblock;
	      block < mblock;
	      block++ ) {
		if (TST_BLOC_OCCUPATION(priv_data->alloc_map,block)) {
			resize = 0;
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Data relocation left some data at the end "
				  "of the volume."));
			goto write_VH;
		}
	}

	/* Mark out of volume blocks as used */
	map_sectors = ( ( old_blocks + PED_SECTOR_SIZE_DEFAULT * 8 - 1 )
	                / (PED_SECTOR_SIZE_DEFAULT * 8) )
		      * (PED_SECTOR_SIZE_DEFAULT * 8);
	for ( block = nblock; block < map_sectors; ++block)
		SET_BLOC_OCCUPATION(priv_data->alloc_map, block);

	/* Update geometry */
	if (resize) {
		/* update in fs structure */
		if (PED_BE32_TO_CPU (vh->next_allocation) >= nblock)
			vh->next_allocation = PED_CPU_TO_BE32 (0);
		vh->total_blocks = PED_CPU_TO_BE32 (nblock);
		vh->free_blocks = PED_CPU_TO_BE32 (nfree);
		/* update parted structure */
		priv_data->plus_geom->length = geom->length;
		priv_data->plus_geom->end = priv_data->plus_geom->start
					    + geom->length - 1;
	}

	/* Effective write */
    write_VH:
    	/* lasts two sectors are allocated by the alternate VH
	   and a reserved sector, and last block is always reserved */
	block = (priv_data->plus_geom->length - 1) / hfsp_sect_block;
	if (block < PED_BE32_TO_CPU (vh->total_blocks))
		SET_BLOC_OCCUPATION(priv_data->alloc_map, block);
	block = (priv_data->plus_geom->length - 2) / hfsp_sect_block;
	if (block < PED_BE32_TO_CPU (vh->total_blocks))
		SET_BLOC_OCCUPATION(priv_data->alloc_map, block);
	SET_BLOC_OCCUPATION(priv_data->alloc_map,
			    PED_BE32_TO_CPU (vh->total_blocks) - 1);

	/* Write the _old_ area to set out of volume blocks as used */
	map_sectors = ( old_blocks + PED_SECTOR_SIZE_DEFAULT * 8 - 1 )
	              / (PED_SECTOR_SIZE_DEFAULT * 8);
	if (!hfsplus_file_write (priv_data->allocation_file,
				 priv_data->alloc_map, 0, map_sectors)) {
		resize = 0;
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Error while writing the allocation file."));
	} else {
	/* Write remaining part of allocation bitmap */
	/* This is necessary to handle pre patch-11 and third party */
	/* implementations */
		memset(buf, 0xFF, PED_SECTOR_SIZE_DEFAULT);
		for (block = map_sectors;
		     block < priv_data->allocation_file->sect_nb;
		     ++block) {
			if (!hfsplus_file_write_sector (
					priv_data->allocation_file,
					buf, block)) {
				ped_exception_throw (
					PED_EXCEPTION_WARNING,
					PED_EXCEPTION_IGNORE,
					_("Error while writing the "
					  "compatibility part of the "
					  "allocation file."));
				break;
			}
		}
	}
	ped_geometry_sync (priv_data->plus_geom);

	if (resize) {
		/* Set the unmounted bit and clear the inconsistent bit */
		vh->attributes |= PED_CPU_TO_BE32 ( 1 << HFS_UNMOUNTED );
		vh->attributes &= ~ PED_CPU_TO_BE32 ( 1 << HFSP_INCONSISTENT );
	}

	ped_timer_set_state_name(timer, _("writing HFS+ Volume Header"));
	if (!hfsplus_update_vh(fs)) {
		ped_geometry_sync(priv_data->plus_geom);
		return 0;
	}

	if (!ped_geometry_sync(priv_data->plus_geom))
		return 0;

	ped_timer_update(timer, 1.0);

	return (resize);
}

/* Update the HFS wrapper mdb and bad blocks file to reflect
   the new geometry of the embedded HFS+ volume */
static int
hfsplus_wrapper_update (PedFileSystem* fs)
{
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];
	HfsCPrivateLeafRec	ref;
	HfsExtentKey		key;
	HfsNodeDescriptor*	node_desc = (HfsNodeDescriptor*) node;
	HfsExtentKey*		ret_key;
	HfsExtDescriptor*	ret_data;
	unsigned int		i;
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPrivateFSData* 	hfs_priv_data = (HfsPrivateFSData*)
					    priv_data->wrapper->type_specific;
	unsigned int		hfs_sect_block =
			PED_BE32_TO_CPU (hfs_priv_data->mdb->block_size)
			/ PED_SECTOR_SIZE_DEFAULT ;
	PedSector		hfsplus_sect = (PedSector)
			PED_BE32_TO_CPU (priv_data->vh->total_blocks)
			* ( PED_BE32_TO_CPU (priv_data->vh->block_size)
			    / PED_SECTOR_SIZE_DEFAULT );
	unsigned int		hfs_blocks_embedded =
				    (hfsplus_sect + hfs_sect_block - 1)
				    / hfs_sect_block;
	unsigned int		hfs_blocks_embedded_old;

	/* update HFS wrapper MDB */
	hfs_blocks_embedded_old = PED_BE16_TO_CPU (
					hfs_priv_data->mdb->old_new
					.embedded.location.block_count );
	hfs_priv_data->mdb->old_new.embedded.location.block_count =
		PED_CPU_TO_BE16 (hfs_blocks_embedded);
	/* maybe macOS will boot with this */
	/* update : yes it does \o/ :) */
	hfs_priv_data->mdb->free_blocks =
	    PED_CPU_TO_BE16 ( PED_BE16_TO_CPU (hfs_priv_data->mdb->free_blocks)
	                    + hfs_blocks_embedded_old
			    - hfs_blocks_embedded );

	if (!hfs_update_mdb(priv_data->wrapper))
		return 0;

	/* force reload bad block list */
	if (hfs_priv_data->bad_blocks_loaded) {
		hfs_free_bad_blocks_list (hfs_priv_data->bad_blocks_xtent_list);
		hfs_priv_data->bad_blocks_xtent_list = NULL;
		hfs_priv_data->bad_blocks_xtent_nb = 0;
		hfs_priv_data->bad_blocks_loaded = 0;
	}

	/* clean HFS wrapper allocation map */
	for (i = PED_BE16_TO_CPU (
			hfs_priv_data->mdb->old_new.embedded
			.location.start_block )
		 + hfs_blocks_embedded;
	     i < PED_BE16_TO_CPU (
	    		hfs_priv_data->mdb->old_new.embedded
			.location.start_block )
		 + hfs_blocks_embedded_old;
	     i++ ) {
		CLR_BLOC_OCCUPATION(hfs_priv_data->alloc_map, i);
	}
	/* and save it */
	if (!ped_geometry_write (fs->geom, hfs_priv_data->alloc_map,
				 PED_BE16_TO_CPU (
				      hfs_priv_data->mdb->volume_bitmap_block ),
				 ( PED_BE16_TO_CPU (
				        hfs_priv_data->mdb->total_blocks )
				   + PED_SECTOR_SIZE_DEFAULT * 8 - 1 )
				 / (PED_SECTOR_SIZE_DEFAULT * 8)))
		return 0;
	if (!ped_geometry_sync (fs->geom))
		return 0;

	/* search and update the bad blocks file */
	key.key_length = sizeof(key) - 1;
	key.type = HFS_DATA_FORK;
	key.file_ID = PED_CPU_TO_BE32 (HFS_BAD_BLOCK_ID);
	key.start = 0;
	if (!hfs_btree_search (hfs_priv_data->extent_file,
			       (HfsPrivateGenericKey*) &key, NULL, 0, &ref)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("An error occurred while looking for the mandatory "
			  "bad blocks file."));
		return 0;
	}
	if (!hfs_file_read_sector (hfs_priv_data->extent_file, node,
				   ref.node_number))
		return 0;
	ret_key = (HfsExtentKey*) (node + ref.record_pos);
	ret_data = (HfsExtDescriptor*) ( node + ref.record_pos
					 + sizeof (HfsExtentKey) );

	while (ret_key->type == key.type && ret_key->file_ID == key.file_ID) {
		for (i = 0; i < HFS_EXT_NB; i++) {
			if ( ret_data[i].start_block
			     == hfs_priv_data->mdb->old_new
			        .embedded.location.start_block) {
				ret_data[i].block_count =
				    hfs_priv_data->mdb->old_new
				    .embedded.location.block_count;
				/* found ! : update */
				if (!hfs_file_write_sector (
					  hfs_priv_data->extent_file,
					  node, ref.node_number)
				    || !ped_geometry_sync(fs->geom))
					return 0;
				return 1;
			}
		}

		if (ref.record_number < PED_BE16_TO_CPU (node_desc->rec_nb)) {
			ref.record_number++;
		} else {
			ref.node_number = PED_BE32_TO_CPU (node_desc->next);
			if (!ref.node_number
			    || !hfs_file_read_sector(hfs_priv_data->extent_file,
						     node, ref.node_number))
				goto bb_not_found;
			ref.record_number = 1;
		}

		ref.record_pos =
			PED_BE16_TO_CPU (*((uint16_t *)
				(node + (PED_SECTOR_SIZE_DEFAULT
				         - 2*ref.record_number))));
		ret_key = (HfsExtentKey*) (node + ref.record_pos);
		ret_data = (HfsExtDescriptor*) ( node + ref.record_pos
						 + sizeof (HfsExtentKey) );
	}

bb_not_found:
	/* not found : not a valid hfs+ wrapper : failure */
	ped_exception_throw (
		PED_EXCEPTION_ERROR,
		PED_EXCEPTION_CANCEL,
		_("It seems there is an error in the HFS wrapper: the bad "
		  "blocks file doesn't contain the embedded HFS+ volume."));
	return 0;
}

int
hfsplus_resize (PedFileSystem* fs, PedGeometry* geom, PedTimer* timer)
{
	HfsPPrivateFSData* 	priv_data;
	PedTimer*		timer_plus;
	PedGeometry*		embedded_geom;
	PedSector		hgms;

	/* check preconditions */
	PED_ASSERT (fs != NULL);
	PED_ASSERT (fs->geom != NULL);
	PED_ASSERT (geom != NULL);
	PED_ASSERT (fs->geom->dev == geom->dev);
#ifdef DEBUG
        PED_ASSERT ((hgms = hfsplus_get_min_size (fs)) != 0);
#else
        if ((hgms = hfsplus_get_min_size (fs)) == 0)
                return 0;
#endif

	if (ped_geometry_test_equal(fs->geom, geom))
		return 1;

	priv_data = (HfsPPrivateFSData*) fs->type_specific;

	if (fs->geom->start != geom->start
	    || geom->length > fs->geom->length
	    || geom->length < hgms) {
		ped_exception_throw (
			PED_EXCEPTION_NO_FEATURE,
			PED_EXCEPTION_CANCEL,
			_("Sorry, HFS+ cannot be resized that way yet."));
		return 0;
	}

	if (priv_data->wrapper) {
		PedSector		red, hgee;
		HfsPrivateFSData* 	hfs_priv_data = (HfsPrivateFSData*)
					    priv_data->wrapper->type_specific;
		unsigned int		hfs_sect_block =
			    PED_BE32_TO_CPU (hfs_priv_data->mdb->block_size)
			    / PED_SECTOR_SIZE_DEFAULT;

		/* There is a wrapper so we must calculate the new geometry
		   of the embedded HFS+ volume */
		red = ( (fs->geom->length - geom->length + hfs_sect_block - 1)
			/ hfs_sect_block ) * hfs_sect_block;
		/* Can't we shrink the hfs+ volume by the desired size ? */
		hgee = hfsplus_get_empty_end (fs);
		if (!hgee) return 0;
		if (red > priv_data->plus_geom->length - hgee) {
			/* No, shrink hfs+ by the greatest possible value */
			hgee = ((hgee + hfs_sect_block - 1) / hfs_sect_block)
			       * hfs_sect_block;
			red = priv_data->plus_geom->length - hgee;
		}
		embedded_geom = ped_geometry_new (geom->dev,
						  priv_data->plus_geom->start,
						  priv_data->plus_geom->length
						  - red);

		/* There is a wrapper so the resize process is a two stages
		   process (embedded resizing then wrapper resizing) :
		   we create a sub timer */
		ped_timer_reset (timer);
		ped_timer_set_state_name (timer,
					  _("shrinking embedded HFS+ volume"));
		ped_timer_update(timer, 0.0);
		timer_plus = ped_timer_new_nested (timer, 0.98);
	} else {
		/* No wrapper : the desired geometry is the desired
		   HFS+ volume geometry */
		embedded_geom = geom;
		timer_plus = timer;
	}

	/* Resize the HFS+ volume */
	if (!hfsplus_volume_resize (fs, embedded_geom, timer_plus)) {
		if (timer_plus != timer) ped_timer_destroy_nested (timer_plus);
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Resizing the HFS+ volume has failed."));
		return 0;
	}

	if (priv_data->wrapper) {
		ped_geometry_destroy (embedded_geom);
		ped_timer_destroy_nested (timer_plus);
		ped_timer_set_state_name(timer, _("shrinking HFS wrapper"));
		timer_plus = ped_timer_new_nested (timer, 0.02);
		/* There's a wrapper : second stage = resizing it */
		if (!hfsplus_wrapper_update (fs)
		    || !hfs_resize (priv_data->wrapper, geom, timer_plus)) {
			ped_timer_destroy_nested (timer_plus);
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Updating the HFS wrapper has failed."));
			return 0;
		}
		ped_timer_destroy_nested (timer_plus);
	}
	ped_timer_update(timer, 1.0);

	return 1;
}

#ifdef HFS_EXTRACT_FS
/* The following is for debugging purpose only, NOT for packaging */

#include <stdio.h>

uint8_t* extract_buffer = NULL;

static int
hfs_extract_file(const char* filename, HfsPrivateFile* hfs_file)
{
	FILE*		fout;
	PedSector	sect;

	fout = fopen(filename, "w");
	if (!fout) return 0;

	for (sect = 0; sect < hfs_file->sect_nb; ++sect) {
		if (!hfs_file_read_sector(hfs_file, extract_buffer, sect))
			goto err_close;
		if (!fwrite(extract_buffer, PED_SECTOR_SIZE_DEFAULT, 1, fout))
			goto err_close;
	}

	return (fclose(fout) == 0 ? 1 : 0);

err_close:
	fclose(fout);
	return 0;
}

static int
hfs_extract_bitmap(const char* filename, PedFileSystem* fs)
{
	HfsPrivateFSData*		priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsMasterDirectoryBlock*	mdb = priv_data->mdb;
	unsigned int 	count;
	FILE*		fout;
	PedSector	sect;

	fout = fopen(filename, "w");
	if (!fout) return 0;

	for (sect = PED_BE16_TO_CPU(mdb->volume_bitmap_block);
	     sect < PED_BE16_TO_CPU(mdb->start_block);
	     sect += count) {
		uint16_t st_block = PED_BE16_TO_CPU(mdb->start_block);
		count = (st_block-sect) < BLOCK_MAX_BUFF ?
			(st_block-sect) : BLOCK_MAX_BUFF;
		if (!ped_geometry_read(fs->geom, extract_buffer, sect, count))
			goto err_close;
		if (!fwrite (extract_buffer, count * PED_SECTOR_SIZE_DEFAULT,
			     1, fout))
			goto err_close;
	}

	return (fclose(fout) == 0 ? 1 : 0);

err_close:
	fclose(fout);
	return 0;
}

static int
hfs_extract_mdb (const char* filename, PedFileSystem* fs)
{
	FILE*		fout;

	fout = fopen(filename, "w");
	if (!fout) return 0;

	if (!ped_geometry_read(fs->geom, extract_buffer, 2, 1))
		goto err_close;
	if (!fwrite(extract_buffer, PED_SECTOR_SIZE_DEFAULT, 1, fout))
		goto err_close;

	return (fclose(fout) == 0 ? 1 : 0);

err_close:
	fclose(fout);
	return 0;
}

static int
hfs_extract (PedFileSystem* fs, PedTimer* timer)
{
	HfsPrivateFSData*	priv_data = (HfsPrivateFSData*)
						fs->type_specific;

	ped_exception_throw (
		PED_EXCEPTION_INFORMATION,
		PED_EXCEPTION_OK,
		_("This is not a real %s check.  This is going to extract "
		  "special low level files for debugging purposes."),
		"HFS");

	extract_buffer = ped_malloc(BLOCK_MAX_BUFF * PED_SECTOR_SIZE_DEFAULT);
	if (!extract_buffer) return 0;

	hfs_extract_mdb(HFS_MDB_FILENAME, fs);
	hfs_extract_file(HFS_CATALOG_FILENAME, priv_data->catalog_file);
	hfs_extract_file(HFS_EXTENTS_FILENAME, priv_data->extent_file);
	hfs_extract_bitmap(HFS_BITMAP_FILENAME, fs);

	free(extract_buffer); extract_buffer = NULL;
	return 0; /* nothing has been fixed by us ! */
}

static int
hfsplus_extract_file(const char* filename, HfsPPrivateFile* hfsp_file)
{
	FILE*		fout;
	unsigned int	cp_sect;
	PedSector	rem_sect;

	fout = fopen(filename, "w");
	if (!fout) return 0;

	for (rem_sect = hfsp_file->sect_nb; rem_sect; rem_sect -= cp_sect) {
		cp_sect = rem_sect < BLOCK_MAX_BUFF ? rem_sect : BLOCK_MAX_BUFF;
		if (!hfsplus_file_read(hfsp_file, extract_buffer,
				       hfsp_file->sect_nb - rem_sect, cp_sect))
			goto err_close;
		if (!fwrite (extract_buffer, cp_sect * PED_SECTOR_SIZE_DEFAULT,
			     1, fout))
			goto err_close;
	}

	return (fclose(fout) == 0 ? 1 : 0);

err_close:
	fclose(fout);
	return 0;
}

static int
hfsplus_extract_vh (const char* filename, PedFileSystem* fs)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	FILE*		fout;
	PedGeometry*	geom = priv_data->plus_geom;


	fout = fopen(filename, "w");
	if (!fout) return 0;

	if (!ped_geometry_read(geom, extract_buffer, 2, 1))
		goto err_close;
	if (!fwrite(extract_buffer, PED_SECTOR_SIZE_DEFAULT, 1, fout))
		goto err_close;

	return (fclose(fout) == 0 ? 1 : 0);

err_close:
	fclose(fout);
	return 0;
}

/* TODO : use the timer to report what is happening */
/* TODO : use exceptions to report errors */
static int
hfsplus_extract (PedFileSystem* fs, PedTimer* timer)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPVolumeHeader*	vh = priv_data->vh;
	HfsPPrivateFile*	startup_file;

	if (priv_data->wrapper) {
		/* TODO : create nested timer */
		hfs_extract (priv_data->wrapper, timer);
	}

	ped_exception_throw (
		PED_EXCEPTION_INFORMATION,
		PED_EXCEPTION_OK,
		_("This is not a real %s check.  This is going to extract "
		  "special low level files for debugging purposes."),
		"HFS+");

	extract_buffer = ped_malloc(BLOCK_MAX_BUFF * PED_SECTOR_SIZE_DEFAULT);
	if (!extract_buffer) return 0;

	hfsplus_extract_vh(HFSP_VH_FILENAME, fs);
	hfsplus_extract_file(HFSP_CATALOG_FILENAME, priv_data->catalog_file);
	hfsplus_extract_file(HFSP_EXTENTS_FILENAME, priv_data->extents_file);
	hfsplus_extract_file(HFSP_ATTRIB_FILENAME, priv_data->attributes_file);
	hfsplus_extract_file(HFSP_BITMAP_FILENAME, priv_data->allocation_file);

	startup_file = hfsplus_file_open(fs, PED_CPU_TO_BE32(HFSP_STARTUP_ID),
					vh->startup_file.extents,
					PED_BE64_TO_CPU (
					   vh->startup_file.logical_size)
					/ PED_SECTOR_SIZE_DEFAULT);
	if (startup_file) {
		hfsplus_extract_file(HFSP_STARTUP_FILENAME, startup_file);
		hfsplus_file_close(startup_file); startup_file = NULL;
	}

	free(extract_buffer); extract_buffer = NULL;
	return 0; /* nothing has been fixed by us ! */
}
#endif /* HFS_EXTRACT_FS */

#endif /* !DISCOVER_ONLY */

#if 0
static PedFileSystemOps hfs_ops = {
	probe:		hfs_probe,
#ifndef DISCOVER_ONLY
	clobber:	hfs_clobber,
	open:		hfs_open,
	create:		NULL,
	close:		hfs_close,
#ifndef HFS_EXTRACT_FS
	check:		NULL,
#else
	check:		hfs_extract,
#endif
	copy:		NULL,
	resize:		hfs_resize,
	get_create_constraint:	NULL,
	get_resize_constraint:	hfs_get_resize_constraint,
	get_copy_constraint:	NULL,
#else /* DISCOVER_ONLY */
	clobber:	NULL,
	open:		NULL,
	create:		NULL,
	close:		NULL,
	check:		NULL,
	copy:		NULL,
	resize:		NULL,
	get_create_constraint:	NULL,
	get_resize_constraint:	NULL,
	get_copy_constraint:	NULL,
#endif /* DISCOVER_ONLY */
};

static PedFileSystemOps hfsplus_ops = {
	probe:		hfsplus_probe,
#ifndef DISCOVER_ONLY
	clobber:	hfsplus_clobber,
	open:		hfsplus_open,
	create:		NULL,
	close:		hfsplus_close,
#ifndef HFS_EXTRACT_FS
	check:		NULL,
#else
	check:		hfsplus_extract,
#endif
	copy:		NULL,
	resize:		hfsplus_resize,
	get_create_constraint:	NULL,
	get_resize_constraint:	hfsplus_get_resize_constraint,
	get_copy_constraint:	NULL,
#else /* DISCOVER_ONLY */
	clobber:	NULL,
	open:		NULL,
	create:		NULL,
	close:		NULL,
	check:		NULL,
	copy:		NULL,
	resize:		NULL,
	get_create_constraint:	NULL,
	get_resize_constraint:	NULL,
	get_copy_constraint:	NULL,
#endif /* DISCOVER_ONLY */
};

static PedFileSystemOps hfsx_ops = {
	probe:		hfsx_probe,
#ifndef DISCOVER_ONLY
	clobber:	hfs_clobber, /* NOT hfsplus_clobber !
					HFSX can't be embedded */
	open:		hfsplus_open,
	create:		NULL,
	close:		hfsplus_close,
#ifndef HFS_EXTRACT_FS
	check:		NULL,
#else
	check:		hfsplus_extract,
#endif
	copy:		NULL,
	resize:		hfsplus_resize,
	get_create_constraint:	NULL,
	get_resize_constraint:	hfsplus_get_resize_constraint,
	get_copy_constraint:	NULL,
#else /* DISCOVER_ONLY */
	clobber:	NULL,
	open:		NULL,
	create:		NULL,
	close:		NULL,
	check:		NULL,
	copy:		NULL,
	resize:		NULL,
	get_create_constraint:	NULL,
	get_resize_constraint:	NULL,
	get_copy_constraint:	NULL,
#endif /* DISCOVER_ONLY */
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
#endif
