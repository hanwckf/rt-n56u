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
#include "file_plus.h"
#include "advfs_plus.h"
#include "cache.h"
#include "journal.h"

#include "reloc_plus.h"

/* This function moves data of size blocks starting at block *ptr_fblock
   to block *ptr_to_fblock */
/* return new start or -1 on failure */
/* -1 is ok because there can only be 2^32-1 blocks, so the max possible
   last one is 2^32-2 (and anyway it contains Alternate VH), so
   -1 (== 2^32-1[2^32]) never represent a valid block */
static int
hfsplus_effect_move_extent (PedFileSystem *fs, unsigned int *ptr_fblock,
			    unsigned int *ptr_to_fblock, unsigned int size)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	unsigned int		i, ok = 0;
	unsigned int		next_to_fblock;
	unsigned int		start, stop;

	PED_ASSERT (hfsp_block != NULL);
	PED_ASSERT (*ptr_to_fblock <= *ptr_fblock);
	/* quiet GCC */
	start = stop = 0;

/*
	Try to fit the extent AT or _BEFORE_ the wanted place,
	or then in the gap between dest and source.
	If failed try to fit the extent after source, for 2 pass relocation
	The extent is always copied in a non overlapping way
*/

	/* Backward search */
	/* 1 pass relocation AT or BEFORE *ptr_to_fblock */
	if (*ptr_to_fblock != *ptr_fblock) {
		start = stop = *ptr_fblock < *ptr_to_fblock+size ?
			       *ptr_fblock : *ptr_to_fblock+size;
		while (start && stop-start != size) {
			--start;
			if (TST_BLOC_OCCUPATION(priv_data->alloc_map,start))
				stop = start;
		}
		ok = (stop-start == size);
	}

	/* Forward search */
	/* 1 pass relocation in the gap merged with 2 pass reloc after source */
	if (!ok && *ptr_to_fblock != *ptr_fblock) {
		start = stop = *ptr_to_fblock+1;
		while (stop < PED_BE32_TO_CPU(priv_data->vh->total_blocks)
		       && stop-start != size) {
			if (TST_BLOC_OCCUPATION(priv_data->alloc_map,stop))
				start = stop + 1;
			++stop;
		}
		ok = (stop-start == size);
	}

	/* new non overlapping room has been found ? */
	if (ok) {
		/* enough room */
		PedSector	abs_sector;
		unsigned int	ai, j, block;
		unsigned int 	block_sz = (PED_BE32_TO_CPU (
					priv_data->vh->block_size)
					/ PED_SECTOR_SIZE_DEFAULT);

		if (stop > *ptr_to_fblock && stop <= *ptr_fblock)
			/* Fit in the gap */
			next_to_fblock = stop;
		else
			/* Before or after the gap */
			next_to_fblock = *ptr_to_fblock;

		/* move blocks */
		for (i = 0; i < size; /*i++*/) {
			j = size - i; j = (j < hfsp_block_count) ?
					   j : hfsp_block_count ;

			abs_sector = (PedSector) (*ptr_fblock + i) * block_sz;
			if (!ped_geometry_read (priv_data->plus_geom,
						hfsp_block, abs_sector,
						block_sz * j))
				return -1;

			abs_sector = (PedSector) (start + i) * block_sz;
			if (!ped_geometry_write (priv_data->plus_geom,
						 hfsp_block, abs_sector,
						 block_sz * j))
				return -1;

			for (ai = i+j; i < ai; i++) {
				/* free source block */
				block = *ptr_fblock + i;
				CLR_BLOC_OCCUPATION(priv_data->alloc_map,block);
				SET_BLOC_OCCUPATION(priv_data->dirty_alloc_map,
						    block/(PED_SECTOR_SIZE_DEFAULT*8));

				/* set dest block */
				block = start + i;
				SET_BLOC_OCCUPATION(priv_data->alloc_map,block);
				SET_BLOC_OCCUPATION(priv_data->dirty_alloc_map,
						    block/(PED_SECTOR_SIZE_DEFAULT*8));
			}
		}
		if (!ped_geometry_sync_fast (priv_data->plus_geom))
			return -1;

		*ptr_fblock += size;
		*ptr_to_fblock = next_to_fblock;
	} else {
		if (*ptr_fblock != *ptr_to_fblock)
			/* not enough room */
			ped_exception_throw (PED_EXCEPTION_WARNING,
			      PED_EXCEPTION_IGNORE,
			      _("An extent has not been relocated."));
		start = *ptr_fblock;
		*ptr_fblock = *ptr_to_fblock = start + size;
	}

	return start;
}

/* Returns 0 on error */
/*         1 on succes */
int
hfsplus_update_vh (PedFileSystem *fs)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];

    	if (!ped_geometry_read (priv_data->plus_geom, node, 2, 1))
		return 0;
	memcpy (node, priv_data->vh, sizeof (HfsPVolumeHeader));
	if (!ped_geometry_write (priv_data->plus_geom, node, 2, 1)
	    || !ped_geometry_write (priv_data->plus_geom, node,
				 priv_data->plus_geom->length - 2, 1)
	    || !ped_geometry_sync_fast (priv_data->plus_geom))
		return 0;
	return 1;
}

static int
hfsplus_do_move (PedFileSystem* fs, unsigned int *ptr_src,
		 unsigned int *ptr_dest, HfsCPrivateCache* cache,
		 HfsCPrivateExtent* ref)
{
	HfsPPrivateFSData*	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPPrivateFile*	file;
	HfsPExtDescriptor*	extent;
	HfsCPrivateExtent*	move;
	int			new_start;

	new_start = hfsplus_effect_move_extent (fs, ptr_src, ptr_dest,
						ref->ext_length);

	if (new_start == -1) return -1;

	if (ref->ext_start != (unsigned) new_start) {
		switch (ref->where) {
		/************ VH ************/
		    case CR_PRIM_CAT :
			priv_data->catalog_file
			->first[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_PRIM;
		    case CR_PRIM_EXT :
			priv_data->extents_file
			->first[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_PRIM;
		    case CR_PRIM_ATTR :
			priv_data->attributes_file
			->first[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_PRIM;
		    case CR_PRIM_ALLOC :
			priv_data->allocation_file
			->first[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_PRIM;
		    case CR_PRIM_START :
			/* No startup file opened */
		    CR_PRIM :
			extent = ( HfsPExtDescriptor* )
				 ( (uint8_t*)priv_data->vh + ref->ref_offset );
			extent[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			if (!hfsplus_update_vh(fs))
				return -1;
			break;

		/************** BTREE *************/
		    case CR_BTREE_CAT_JIB :
			if (!hfsj_update_jib(fs, new_start))
				return -1;
			goto BTREE_CAT;

		    case CR_BTREE_CAT_JL :
			if (!hfsj_update_jl(fs, new_start))
				return -1;
			goto BTREE_CAT;

		    BTREE_CAT:
		    case CR_BTREE_CAT :
			file = priv_data->catalog_file;
			goto CR_BTREE;

		    case CR_BTREE_ATTR :
			file = priv_data->attributes_file;
			goto CR_BTREE;

		    case CR_BTREE_EXT_ATTR :
			if (priv_data->attributes_file
			    ->cache[ref->ref_index].start_block
			    == PED_CPU_TO_BE32(ref->ext_start))
				priv_data->attributes_file
				->cache[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_BTREE_EXT;
		    case CR_BTREE_EXT_CAT :
			if (priv_data->catalog_file
			    ->cache[ref->ref_index].start_block
			    == PED_CPU_TO_BE32(ref->ext_start))
				priv_data->catalog_file
				->cache[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_BTREE_EXT;
		    case CR_BTREE_EXT_ALLOC :
			if (priv_data->allocation_file
			    ->cache[ref->ref_index].start_block
			    == PED_CPU_TO_BE32(ref->ext_start))
				priv_data->allocation_file
				->cache[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			goto CR_BTREE_EXT;
		    case CR_BTREE_EXT_START :
		    	/* No startup file opened */
		    CR_BTREE_EXT :
		    case CR_BTREE_EXT_0 :
			file = priv_data->extents_file;

		    CR_BTREE :
			PED_ASSERT(PED_SECTOR_SIZE_DEFAULT * ref->sect_by_block
				   > ref->ref_offset);
			if (!hfsplus_file_read(file, hfsp_block,
				(PedSector)ref->ref_block * ref->sect_by_block,
				ref->sect_by_block))
				return -1;
			extent = ( HfsPExtDescriptor* )
				( hfsp_block + ref->ref_offset );
			extent[ref->ref_index].start_block =
				PED_CPU_TO_BE32(new_start);
			if (!hfsplus_file_write(file, hfsp_block,
				(PedSector)ref->ref_block * ref->sect_by_block,
				ref->sect_by_block)
			    || !ped_geometry_sync_fast (priv_data->plus_geom))
				return -1;
			break;

		    /********** BUG *********/
		    default :
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("A reference to an extent comes from a place "
				  "it should not.  You should check the file "
				  "system!"));
			return -1;
			break;
		}

		move = hfsc_cache_move_extent(cache, ref->ext_start, new_start);
		if (!move) return -1;
		PED_ASSERT(move == ref);
	}

	return new_start;
}

/* save any dirty sector of the allocation bitmap file */
static int
hfsplus_save_allocation(PedFileSystem *fs)
{
	HfsPPrivateFSData*	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	unsigned int		map_sectors, i, j;
	int			ret = 1;

	map_sectors = ( PED_BE32_TO_CPU (priv_data->vh->total_blocks)
			+ PED_SECTOR_SIZE_DEFAULT * 8 - 1 ) / (PED_SECTOR_SIZE_DEFAULT * 8);

	for (i = 0; i < map_sectors;) {
		for (j = i;
		     (TST_BLOC_OCCUPATION(priv_data->dirty_alloc_map,j));
		     ++j)
			CLR_BLOC_OCCUPATION(priv_data->dirty_alloc_map,j);
		if (j-i) {
			ret = hfsplus_file_write(priv_data->allocation_file,
				    priv_data->alloc_map + i * PED_SECTOR_SIZE_DEFAULT,
				    i, j-i) && ret;
			i = j;
		} else
			++i;
	}

	return ret;
}

/* This function moves an extent starting at block fblock
   to block to_fblock if there's enough room */
/* Return 1 if everything was fine */
/* Return -1 if an error occurred */
/* Return 0 if no extent was found */
static int
hfsplus_move_extent_starting_at (PedFileSystem *fs, unsigned int *ptr_fblock,
				 unsigned int *ptr_to_fblock,
				 HfsCPrivateCache* cache)
{
	HfsCPrivateExtent*	ref;
	unsigned int		old_start, new_start;

	ref = hfsc_cache_search_extent(cache, *ptr_fblock);
	if (!ref) return 0;

	old_start = *ptr_fblock;
	new_start = hfsplus_do_move(fs, ptr_fblock, ptr_to_fblock, cache, ref);
	if (new_start == (unsigned)-1) return -1;
	if (new_start > old_start) {
		new_start = hfsplus_do_move(fs, &new_start, ptr_to_fblock,
					    cache, ref);
		if (new_start == (unsigned)-1 || new_start > old_start)
			return -1;
	}

	hfsplus_save_allocation(fs);
	return 1;
}

static int
hfsplus_cache_from_vh(HfsCPrivateCache* cache, PedFileSystem* fs,
		      PedTimer* timer)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPExtDescriptor*	extent;
	unsigned int		j;

	extent = priv_data->vh->allocation_file.extents;
	for (j = 0; j < HFSP_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE32_TO_CPU(extent[j].start_block),
			PED_BE32_TO_CPU(extent[j].block_count),
			0, /* unused for vh */
			((uint8_t*)extent) - ((uint8_t*)priv_data->vh),
			1, /* load / save 1 sector */
			CR_PRIM_ALLOC,
			j )
		   )
			return 0;
	}

	extent = priv_data->vh->extents_file.extents;
	for (j = 0; j < HFSP_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE32_TO_CPU(extent[j].start_block),
			PED_BE32_TO_CPU(extent[j].block_count),
			0, /* unused for vh */
			((uint8_t*)extent) - ((uint8_t*)priv_data->vh),
			1, /* load / save 1 sector */
			CR_PRIM_EXT,
			j )
		   )
			return 0;
	}

	extent = priv_data->vh->catalog_file.extents;
	for (j = 0; j < HFSP_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE32_TO_CPU(extent[j].start_block),
			PED_BE32_TO_CPU(extent[j].block_count),
			0, /* unused for vh */
			((uint8_t*)extent) - ((uint8_t*)priv_data->vh),
			1, /* load / save 1 sector */
			CR_PRIM_CAT,
			j )
		   )
			return 0;
	}

	extent = priv_data->vh->attributes_file.extents;
	for (j = 0; j < HFSP_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE32_TO_CPU(extent[j].start_block),
			PED_BE32_TO_CPU(extent[j].block_count),
			0, /* unused for vh */
			((uint8_t*)extent) - ((uint8_t*)priv_data->vh),
			1, /* load / save 1 sector */
			CR_PRIM_ATTR,
			j )
		   )
			return 0;
	}

	extent = priv_data->vh->startup_file.extents;
	for (j = 0; j < HFSP_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE32_TO_CPU(extent[j].start_block),
			PED_BE32_TO_CPU(extent[j].block_count),
			0, /* unused for vh */
			((uint8_t*)extent) - ((uint8_t*)priv_data->vh),
			1, /* load / save 1 sector */
			CR_PRIM_START,
			j )
		   )
			return 0;
	}

	return 1;
}

static int
hfsplus_cache_from_catalog(HfsCPrivateCache* cache, PedFileSystem* fs,
			   PedTimer* timer)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	uint8_t			node_1[PED_SECTOR_SIZE_DEFAULT];
	uint8_t*		node;
	HfsPHeaderRecord*	header;
	HfsPCatalogKey*		catalog_key;
	HfsPCatalog*		catalog_data;
	HfsPExtDescriptor*	extent;
	unsigned int		leaf_node, record_number;
	unsigned int		i, j, size, bsize;
	uint32_t		jib = priv_data->jib_start_block,
				jl  = priv_data->jl_start_block;

	if (!priv_data->catalog_file->sect_nb) {
		ped_exception_throw (
			PED_EXCEPTION_INFORMATION,
			PED_EXCEPTION_OK,
			_("This HFS+ volume has no catalog file.  "
			  "This is very unusual!"));
		return 1;
	}

	/* Search the extent starting at *ptr_block in the catalog file */
	if (!hfsplus_file_read_sector (priv_data->catalog_file, node_1, 0))
		return 0;
	header = (HfsPHeaderRecord*) (node_1 + HFS_FIRST_REC);
	leaf_node = PED_BE32_TO_CPU (header->first_leaf_node);
	bsize = PED_BE16_TO_CPU (header->node_size);
	size = bsize / PED_SECTOR_SIZE_DEFAULT;
	PED_ASSERT(size < 256);

	node = (uint8_t*) ped_malloc(bsize);
	if (!node) return 0;
	HfsPNodeDescriptor *desc = (HfsPNodeDescriptor*) node;

	for (; leaf_node; leaf_node = PED_BE32_TO_CPU (desc->next)) {
		if (!hfsplus_file_read (priv_data->catalog_file, node,
					(PedSector) leaf_node * size, size)) {
			free (node);
			return 0;
		}
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = 1; i <= record_number; i++) {
			unsigned int	skip;
			uint8_t		where;

			catalog_key = (HfsPCatalogKey*)
			    ( node + PED_BE16_TO_CPU (*((uint16_t *)
					(node+(bsize - 2*i)))) );
			skip = ( 2 + PED_BE16_TO_CPU (catalog_key->key_length)
				 + 1) & ~1;
			catalog_data = (HfsPCatalog*)
					    (((uint8_t*)catalog_key) + skip);
			/* check for obvious error in FS */
			if (((uint8_t*)catalog_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)catalog_data - node
			        >= (signed) bsize
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				free (node);
				return 0;
			}

			if (PED_BE16_TO_CPU(catalog_data->type)!=HFS_CAT_FILE)
				continue;

			extent = catalog_data->sel.file.data_fork.extents;
			for (j = 0; j < HFSP_EXT_NB; ++j) {
				if (!extent[j].block_count) break;
				where = CR_BTREE_CAT;
				if ( PED_BE32_TO_CPU(extent[j].start_block)
				     == jib ) {
					jib = 0;
					where = CR_BTREE_CAT_JIB;
				} else
				  if ( PED_BE32_TO_CPU(extent[j].start_block)
				       == jl ) {
					jl = 0;
					where = CR_BTREE_CAT_JL;
				}
				if (!hfsc_cache_add_extent(
					cache,
					PED_BE32_TO_CPU(extent[j].start_block),
					PED_BE32_TO_CPU(extent[j].block_count),
					leaf_node,
					(uint8_t*)extent - node,
					size,
					where,
					j )
				   ) {
					free (node);
				 	return 0;
				}
			}

			extent = catalog_data->sel.file.res_fork.extents;
			for (j = 0; j < HFSP_EXT_NB; ++j) {
				if (!extent[j].block_count) break;
				if (!hfsc_cache_add_extent(
					cache,
					PED_BE32_TO_CPU(extent[j].start_block),
					PED_BE32_TO_CPU(extent[j].block_count),
					leaf_node,
					(uint8_t*)extent - node,
					size,
					CR_BTREE_CAT,
					j )
				   ) {
					free (node);
				 	return 0;
				}
			}
		}
	}

	free (node);
	return 1;
}

static int
hfsplus_cache_from_extent(HfsCPrivateCache* cache, PedFileSystem* fs,
			  PedTimer* timer)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	uint8_t			node_1[PED_SECTOR_SIZE_DEFAULT];
	uint8_t*		node;
	HfsPHeaderRecord*	header;
	HfsPExtentKey*		extent_key;
	HfsPExtDescriptor*	extent;
	unsigned int		leaf_node, record_number;
	unsigned int		i, j, size, bsize;

	if (!priv_data->extents_file->sect_nb) {
		ped_exception_throw (
			PED_EXCEPTION_INFORMATION,
			PED_EXCEPTION_OK,
			_("This HFS+ volume has no extents overflow "
			  "file.  This is quite unusual!"));
		return 1;
	}

	if (!hfsplus_file_read_sector (priv_data->extents_file, node_1, 0))
		return 0;
	header = ((HfsPHeaderRecord*) (node_1 + HFS_FIRST_REC));
	leaf_node = PED_BE32_TO_CPU (header->first_leaf_node);
	bsize = PED_BE16_TO_CPU (header->node_size);
	size = bsize / PED_SECTOR_SIZE_DEFAULT;
	PED_ASSERT(size < 256);

	node = (uint8_t*) ped_malloc (bsize);
	if (!node) return -1;
	HfsPNodeDescriptor *desc = (HfsPNodeDescriptor*) node;

	for (; leaf_node; leaf_node = PED_BE32_TO_CPU (desc->next)) {
		if (!hfsplus_file_read (priv_data->extents_file, node,
					(PedSector) leaf_node * size, size)) {
			free (node);
			return 0;
		}
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = 1; i <= record_number; i++) {
			uint8_t where;
			extent_key = (HfsPExtentKey*)
			    (node + PED_BE16_TO_CPU(*((uint16_t *)
					    (node+(bsize - 2*i)))));
			extent = (HfsPExtDescriptor*)
			    (((uint8_t*)extent_key) + sizeof (HfsPExtentKey));
			/* check for obvious error in FS */
			if (((uint8_t*)extent_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)extent - node
			        >= (signed)bsize
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				free (node);
				return -1;
			}

			switch (extent_key->file_ID) {
			    case PED_CPU_TO_BE32 (HFS_XTENT_ID) :
				if (ped_exception_throw (
					PED_EXCEPTION_WARNING,
					PED_EXCEPTION_IGNORE_CANCEL,
					_("The extents overflow file should not"
					" contain its own extents!  You should "
					"check the file system."))
						!= PED_EXCEPTION_IGNORE)
					return 0;
				where = CR_BTREE_EXT_EXT;
				break;
			    case PED_CPU_TO_BE32 (HFS_CATALOG_ID) :
				where = CR_BTREE_EXT_CAT;
				break;
			    case PED_CPU_TO_BE32 (HFSP_ALLOC_ID) :
				where = CR_BTREE_EXT_ALLOC;
				break;
			    case PED_CPU_TO_BE32 (HFSP_STARTUP_ID) :
				where = CR_BTREE_EXT_START;
				break;
			    case PED_CPU_TO_BE32 (HFSP_ATTRIB_ID) :
			    	where = CR_BTREE_EXT_ATTR;
				break;
			    default :
			    	where = CR_BTREE_EXT_0;
				break;
			}

			for (j = 0; j < HFSP_EXT_NB; ++j) {
				if (!extent[j].block_count) break;
				if (!hfsc_cache_add_extent(
					cache,
					PED_BE32_TO_CPU(extent[j].start_block),
					PED_BE32_TO_CPU(extent[j].block_count),
					leaf_node,
					(uint8_t*)extent - node,
					size,
					where,
					j )
				   ) {
					free (node);
					return 0;
				}
			}
		}
	}

	free (node);
	return 1;
}

static int
hfsplus_cache_from_attributes(HfsCPrivateCache* cache, PedFileSystem* fs,
			      PedTimer* timer)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	uint8_t			node_1[PED_SECTOR_SIZE_DEFAULT];
	uint8_t*		node;
	HfsPHeaderRecord*	header;
	HfsPPrivateGenericKey*	generic_key;
	HfsPForkDataAttr*	fork_ext_data;
	HfsPExtDescriptor*	extent;
	unsigned int		leaf_node, record_number;
	unsigned int		i, j, size, bsize;

	/* attributes file is facultative */
	if (!priv_data->attributes_file->sect_nb)
		return 1;

	/* Search the extent starting at *ptr_block in the catalog file */
	if (!hfsplus_file_read_sector (priv_data->attributes_file, node_1, 0))
		return 0;
	header = ((HfsPHeaderRecord*) (node_1 + HFS_FIRST_REC));
	leaf_node = PED_BE32_TO_CPU (header->first_leaf_node);
	bsize = PED_BE16_TO_CPU (header->node_size);
	size = bsize / PED_SECTOR_SIZE_DEFAULT;
	PED_ASSERT(size < 256);

	node = (uint8_t*) ped_malloc(bsize);
	if (!node) return 0;
	HfsPNodeDescriptor *desc = (HfsPNodeDescriptor*) node;

	for (; leaf_node; leaf_node = PED_BE32_TO_CPU (desc->next)) {
		if (!hfsplus_file_read (priv_data->attributes_file, node,
					(PedSector) leaf_node * size, size)) {
			free (node);
			return 0;
		}
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = 1; i <= record_number; i++) {
			unsigned int	skip;
			generic_key = (HfsPPrivateGenericKey*)
				(node + PED_BE16_TO_CPU(*((uint16_t *)
					    (node+(bsize - 2*i)))));
			skip = ( 2 + PED_BE16_TO_CPU (generic_key->key_length)
			         + 1 ) & ~1;
			fork_ext_data = (HfsPForkDataAttr*)
					    (((uint8_t*)generic_key) + skip);
			/* check for obvious error in FS */
			if (((uint8_t*)generic_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)fork_ext_data - node
			        >= (signed) bsize
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				free (node);
				return 0;
			}

			if (fork_ext_data->record_type
			    == PED_CPU_TO_BE32 ( HFSP_ATTR_FORK ) ) {
				extent = fork_ext_data->fork_res.fork.extents;
				for (j = 0; j < HFSP_EXT_NB; ++j) {
					if (!extent[j].block_count) break;
					if (!hfsc_cache_add_extent(
						cache,
						PED_BE32_TO_CPU (
							extent[j].start_block ),
						PED_BE32_TO_CPU (
							extent[j].block_count ),
						leaf_node,
						(uint8_t*)extent-node,
						size,
						CR_BTREE_ATTR,
						j )
					   ) {
						free(node);
						return 0;
					}
				}
			} else if (fork_ext_data->record_type
			    == PED_CPU_TO_BE32 ( HFSP_ATTR_EXTENTS ) ) {
				extent = fork_ext_data->fork_res.extents;
				for (j = 0; j < HFSP_EXT_NB; ++j) {
					if (!extent[j].block_count) break;
					if (!hfsc_cache_add_extent(
						cache,
						PED_BE32_TO_CPU (
							extent[j].start_block ),
						PED_BE32_TO_CPU (
							extent[j].block_count ),
						leaf_node,
						(uint8_t*)extent-node,
						size,
						CR_BTREE_ATTR,
						j )
					   ) {
						free(node);
						return 0;
					}
				}
			} else continue;
		}
	}

	free (node);
	return 1;
}

static HfsCPrivateCache*
hfsplus_cache_extents(PedFileSystem* fs, PedTimer* timer)
{
	HfsPPrivateFSData*	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsCPrivateCache*	ret;
	unsigned int		file_number, block_number;

	file_number = PED_BE32_TO_CPU(priv_data->vh->file_count);
	block_number = PED_BE32_TO_CPU(priv_data->vh->total_blocks);
	ret = hfsc_new_cache(block_number, file_number);
	if (!ret) return NULL;

	if (!hfsplus_cache_from_vh(ret, fs, timer) ||
	    !hfsplus_cache_from_catalog(ret, fs, timer) ||
	    !hfsplus_cache_from_extent(ret, fs, timer) ||
	    !hfsplus_cache_from_attributes(ret, fs, timer)) {
		ped_exception_throw(
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Could not cache the file system in memory."));
		hfsc_delete_cache(ret);
		return NULL;
	}

	return ret;
}

/* This function moves file's data to compact used and free space,
   starting at fblock block */
/* return 0 on error */
int
hfsplus_pack_free_space_from_block (PedFileSystem *fs, unsigned int fblock,
				    PedTimer* timer, unsigned int to_free)
{
	PedSector		bytes_buff;
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPVolumeHeader*	vh = priv_data->vh;
	HfsCPrivateCache*	cache;
	unsigned int 		to_fblock = fblock;
	unsigned int		start = fblock;
	unsigned int		divisor = PED_BE32_TO_CPU (vh->total_blocks)
				          + 1 - start - to_free;
	int			ret;

	PED_ASSERT (!hfsp_block);

	cache = hfsplus_cache_extents (fs, timer);
	if (!cache)
		return 0;

	/* Calculate the size of the copy buffer :
	 * Takes BLOCK_MAX_BUFF HFS blocks, but if > BYTES_MAX_BUFF
	 * takes the maximum number of HFS blocks so that the buffer
	 * will remain smaller than or equal to BYTES_MAX_BUFF, with
	 * a minimum of 1 HFS block */
	bytes_buff = PED_BE32_TO_CPU (priv_data->vh->block_size)
		     * (PedSector) BLOCK_MAX_BUFF;
	if (bytes_buff > BYTES_MAX_BUFF) {
		hfsp_block_count = BYTES_MAX_BUFF
				 / PED_BE32_TO_CPU (priv_data->vh->block_size);
		if (!hfsp_block_count)
			hfsp_block_count = 1;
		bytes_buff = (PedSector) hfsp_block_count
			     * PED_BE32_TO_CPU (priv_data->vh->block_size);
	} else
		hfsp_block_count = BLOCK_MAX_BUFF;

	/* If the cache code requests more space, give it to him */
	if (bytes_buff < hfsc_cache_needed_buffer (cache))
		bytes_buff = hfsc_cache_needed_buffer (cache);

	hfsp_block = (uint8_t*) ped_malloc (bytes_buff);
	if (!hfsp_block)
		goto error_cache;

	if (!hfsplus_read_bad_blocks (fs)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Bad blocks list could not be loaded."));
		goto error_alloc;
	}

	while ( fblock < ( priv_data->plus_geom->length - 2 )
			 / ( PED_BE32_TO_CPU (vh->block_size)
			     / PED_SECTOR_SIZE_DEFAULT ) ) {
		if (TST_BLOC_OCCUPATION (priv_data->alloc_map, fblock)
		    && (!hfsplus_is_bad_block (fs, fblock))) {
			if (!(ret = hfsplus_move_extent_starting_at (fs,
						&fblock, &to_fblock, cache)))
				to_fblock = ++fblock;
			else if (ret == -1) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("An error occurred during extent "
					  "relocation."));
				goto error_alloc;
			}
		} else {
			fblock++;
		}

		ped_timer_update(timer, (float)(to_fblock - start) / divisor);
	}

	free (hfsp_block); hfsp_block = NULL; hfsp_block_count = 0;
	hfsc_delete_cache (cache);
	return 1;

error_alloc:
	free (hfsp_block); hfsp_block = NULL; hfsp_block_count = 0;
error_cache:
	hfsc_delete_cache (cache);
	return 0;
}

#endif /* !DISCOVER_ONLY */
