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
#include "file.h"
#include "advfs.h"
#include "cache.h"

#include "reloc.h"

/* This function moves data of size blocks starting
   at block *ptr_fblock to block *ptr_to_fblock */
/* return new start or -1 on failure */
static int
hfs_effect_move_extent (PedFileSystem *fs, unsigned int *ptr_fblock,
			unsigned int *ptr_to_fblock, unsigned int size)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	unsigned int		i, ok = 0;
	unsigned int		next_to_fblock;
	unsigned int		start, stop;

	PED_ASSERT (hfs_block != NULL);
	PED_ASSERT (*ptr_to_fblock <= *ptr_fblock);
	/* quiet gcc */
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
		while (stop < PED_BE16_TO_CPU(priv_data->mdb->total_blocks)
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
		unsigned int j;
		unsigned int start_block =
			PED_BE16_TO_CPU (priv_data->mdb->start_block );
		unsigned int block_sz =
			(PED_BE32_TO_CPU (priv_data->mdb->block_size)
			 / PED_SECTOR_SIZE_DEFAULT);

		if (stop > *ptr_to_fblock && stop <= *ptr_fblock)
			/* Fit in the gap */
			next_to_fblock = stop;
		else
			/* Before or after the gap */
			next_to_fblock = *ptr_to_fblock;

		/* move blocks */
		for (i = 0; i < size; /*i+=j*/) {
			PedSector 	abs_sector;
			unsigned int	ai;

			j = size - i; j = (j < hfs_block_count) ?
					   j : hfs_block_count ;

			abs_sector = start_block
				     + (PedSector) (*ptr_fblock + i) * block_sz;
			if (!ped_geometry_read (fs->geom, hfs_block, abs_sector,
						block_sz * j))
				return -1;

			abs_sector = start_block
				     + (PedSector) (start + i) * block_sz;
			if (!ped_geometry_write (fs->geom,hfs_block,abs_sector,
						 block_sz * j))
				return -1;

			for (ai = i+j; i < ai; i++) {
				/* free source block */
				CLR_BLOC_OCCUPATION(priv_data->alloc_map,
						    *ptr_fblock + i);

				/* set dest block */
				SET_BLOC_OCCUPATION(priv_data->alloc_map,
						    start + i);
			}
		}
		if (!ped_geometry_sync_fast (fs->geom))
			return -1;

		*ptr_fblock += size;
		*ptr_to_fblock = next_to_fblock;
	} else {
		if (*ptr_fblock != *ptr_to_fblock)
			/* not enough room, but try to continue */
			ped_exception_throw (PED_EXCEPTION_WARNING,
					PED_EXCEPTION_IGNORE,
					_("An extent has not been relocated."));
		start = *ptr_fblock;
		*ptr_fblock = *ptr_to_fblock = start + size;
	}

	return start;
}

/* Update MDB */
/* Return 0 if an error occurred */
/* Return 1 if everything ok */
int
hfs_update_mdb (PedFileSystem *fs)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];

    	if (!ped_geometry_read (fs->geom, node, 2, 1))
		return 0;
	memcpy (node, priv_data->mdb, sizeof (HfsMasterDirectoryBlock));
	if (   !ped_geometry_write (fs->geom, node, 2, 1)
	    || !ped_geometry_write (fs->geom, node, fs->geom->length - 2, 1)
	    || !ped_geometry_sync_fast (fs->geom))
		return 0;
	return 1;
}

/* Generic relocator */
/* replace previous hfs_do_move_* */
static int
hfs_do_move (PedFileSystem* fs, unsigned int *ptr_src,
	     unsigned int *ptr_dest, HfsCPrivateCache* cache,
	     HfsCPrivateExtent* ref)
{
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsPrivateFile*		file;
	HfsExtDescriptor*	extent;
	HfsCPrivateExtent*	move;
	int			new_start;

	new_start = hfs_effect_move_extent (fs, ptr_src, ptr_dest,
					    ref->ext_length);
	if (new_start == -1) return -1;

	if (ref->ext_start != (unsigned) new_start) {
		/* Load, modify & save */
		switch (ref->where) {
		/******** MDB *********/
		    case CR_PRIM_CAT :
			priv_data->catalog_file
			->first[ref->ref_index].start_block =
				PED_CPU_TO_BE16(new_start);
			goto CR_PRIM;
		    case CR_PRIM_EXT :
			priv_data->extent_file
			->first[ref->ref_index].start_block =
				PED_CPU_TO_BE16(new_start);
		    CR_PRIM :
			extent = ( HfsExtDescriptor* )
				 ( (uint8_t*)priv_data->mdb + ref->ref_offset );
			extent[ref->ref_index].start_block =
				PED_CPU_TO_BE16(new_start);
			if (!hfs_update_mdb(fs)) return -1;
			break;

		/********* BTREE *******/
		    case CR_BTREE_EXT_CAT :
			if (priv_data->catalog_file
			    ->cache[ref->ref_index].start_block
			    == PED_CPU_TO_BE16(ref->ext_start))
				priv_data->catalog_file
				->cache[ref->ref_index].start_block =
				PED_CPU_TO_BE16(new_start);
		    case CR_BTREE_EXT_0 :
			file = priv_data->extent_file;
			goto CR_BTREE;
		    case CR_BTREE_CAT :
			file = priv_data->catalog_file;
		    CR_BTREE:
			PED_ASSERT(ref->sect_by_block == 1
			           && ref->ref_offset < PED_SECTOR_SIZE_DEFAULT);
			if (!hfs_file_read_sector(file, node, ref->ref_block))
				return -1;
			extent = ( HfsExtDescriptor* ) (node + ref->ref_offset);
			extent[ref->ref_index].start_block =
				PED_CPU_TO_BE16(new_start);
			if (!hfs_file_write_sector(file, node, ref->ref_block)
			    || !ped_geometry_sync_fast (fs->geom))
				return -1;
			break;

		/********** BUG ********/
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

		/* Update the cache */
		move = hfsc_cache_move_extent(cache, ref->ext_start, new_start);
		if (!move) return -1; /* "cleanly" fail */
		PED_ASSERT(move == ref); /* generate a bug */
	}

	return new_start;
}

/* 0 error, 1 ok */
static int
hfs_save_allocation(PedFileSystem* fs)
{
	HfsPrivateFSData*	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	unsigned int		map_sectors;

	map_sectors = ( PED_BE16_TO_CPU (priv_data->mdb->total_blocks)
			+ PED_SECTOR_SIZE_DEFAULT * 8 - 1 )
		      / (PED_SECTOR_SIZE_DEFAULT * 8);
	return ( ped_geometry_write (fs->geom, priv_data->alloc_map,
			PED_BE16_TO_CPU (priv_data->mdb->volume_bitmap_block),
			map_sectors) );
}

/* This function moves an extent starting at block fblock to block to_fblock
   if there's enough room */
/* Return 1 if everything was fine */
/* Return -1 if an error occurred */
/* Return 0 if no extent was found */
/* Generic search thanks to the file system cache */
static int
hfs_move_extent_starting_at (PedFileSystem *fs, unsigned int *ptr_fblock,
			     unsigned int *ptr_to_fblock,
			     HfsCPrivateCache* cache)
{
	HfsCPrivateExtent*	ref;
	unsigned int		old_start, new_start;

	/* Reference search powered by the cache... */
	/* This is the optimisation secret :) */
	ref = hfsc_cache_search_extent(cache, *ptr_fblock);
	if (!ref) return 0; /* not found */

	old_start = *ptr_fblock;
	new_start = hfs_do_move(fs, ptr_fblock, ptr_to_fblock, cache, ref);
	if (new_start == (unsigned int) -1) return -1;
	if (new_start > old_start) { /* detect 2 pass reloc */
		new_start = hfs_do_move(fs,&new_start,ptr_to_fblock,cache,ref);
		if (new_start == (unsigned int) -1 || new_start > old_start)
			return -1;
	}

	/* allocation bitmap save is not atomic with data relocation */
	/* so we only do it a few times, and without syncing */
	/* The unmounted bit protect us anyway */
	hfs_save_allocation(fs);
	return 1;
}

static int
hfs_cache_from_mdb(HfsCPrivateCache* cache, PedFileSystem* fs,
		   PedTimer* timer)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsExtDescriptor*	extent;
	unsigned int		j;

	extent = priv_data->mdb->extents_file_rec;
    	for (j = 0; j < HFS_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE16_TO_CPU(extent[j].start_block),
			PED_BE16_TO_CPU(extent[j].block_count),
			0, /* unused for mdb */
			((uint8_t*)extent) - ((uint8_t*)priv_data->mdb),
			1, /* load/save only 1 sector */
			CR_PRIM_EXT,
			j )
		   )
			return 0;
	}

	extent = priv_data->mdb->catalog_file_rec;
	for (j = 0; j < HFS_EXT_NB; ++j) {
		if (!extent[j].block_count) break;
		if (!hfsc_cache_add_extent(
			cache,
			PED_BE16_TO_CPU(extent[j].start_block),
			PED_BE16_TO_CPU(extent[j].block_count),
			0,
			((uint8_t*)extent) - ((uint8_t*)priv_data->mdb),
			1,
			CR_PRIM_CAT,
			j )
		   )
			return 0;
	}

	return 1;
}

static int
hfs_cache_from_catalog(HfsCPrivateCache* cache, PedFileSystem* fs,
		   PedTimer* timer)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];
	HfsHeaderRecord*	header;
	HfsNodeDescriptor*	desc = (HfsNodeDescriptor*) node;
	HfsCatalogKey*		catalog_key;
	HfsCatalog*		catalog_data;
	HfsExtDescriptor*	extent;
	unsigned int		leaf_node, record_number;
	unsigned int		i, j;

	if (!priv_data->catalog_file->sect_nb) {
		ped_exception_throw (
			PED_EXCEPTION_INFORMATION,
			PED_EXCEPTION_OK,
			_("This HFS volume has no catalog file.  "
			  "This is very unusual!"));
		return 1;
	}

	if (!hfs_file_read_sector (priv_data->catalog_file, node, 0))
		return 0;
	header = (HfsHeaderRecord*)(node +  PED_BE16_TO_CPU(*((uint16_t*)
						(node+(PED_SECTOR_SIZE_DEFAULT-2)))));

	for (leaf_node = PED_BE32_TO_CPU (header->first_leaf_node);
	     leaf_node;
	     leaf_node = PED_BE32_TO_CPU (desc->next)) {
		if (!hfs_file_read_sector (priv_data->catalog_file,
					   node, leaf_node))
			return 0;
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = 1; i <= record_number; ++i) {
		       /* undocumented alignement */
			unsigned int skip;
			catalog_key = (HfsCatalogKey*) (node + PED_BE16_TO_CPU(
				*((uint16_t*)(node+(PED_SECTOR_SIZE_DEFAULT - 2*i)))));
			skip = (1 + catalog_key->key_length + 1) & ~1;
			catalog_data = (HfsCatalog*)( ((uint8_t*)catalog_key)
							+ skip );
			/* check for obvious error in FS */
			if (((uint8_t*)catalog_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)catalog_data - node
				>= PED_SECTOR_SIZE_DEFAULT
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				return 0;
			}

			if (catalog_data->type != HFS_CAT_FILE) continue;

			extent = catalog_data->sel.file.extents_data;
			for (j = 0; j < HFS_EXT_NB; ++j) {
				if (!extent[j].block_count) break;
				if (!hfsc_cache_add_extent(
					cache,
					PED_BE16_TO_CPU(extent[j].start_block),
					PED_BE16_TO_CPU(extent[j].block_count),
					leaf_node,
					(uint8_t*)extent - node,
					1, /* hfs => btree block = 512 b */
					CR_BTREE_CAT,
					j )
				   )
					return 0;
			}

			extent = catalog_data->sel.file.extents_res;
			for (j = 0; j < HFS_EXT_NB; ++j) {
				if (!extent[j].block_count) break;
				if (!hfsc_cache_add_extent(
					cache,
					PED_BE16_TO_CPU(extent[j].start_block),
					PED_BE16_TO_CPU(extent[j].block_count),
					leaf_node,
					(uint8_t*)extent - node,
					1, /* hfs => btree block = 512 b */
					CR_BTREE_CAT,
					j )
				   )
					return 0;
			}
		}
	}

	return 1;
}

static int
hfs_cache_from_extent(HfsCPrivateCache* cache, PedFileSystem* fs,
		   PedTimer* timer)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];
	HfsHeaderRecord*	header;
	HfsNodeDescriptor*	desc = (HfsNodeDescriptor*) node;
	HfsExtentKey*		extent_key;
	HfsExtDescriptor*	extent;
	unsigned int		leaf_node, record_number;
	unsigned int		i, j;

	if (!priv_data->extent_file->sect_nb) {
		ped_exception_throw (
			PED_EXCEPTION_INFORMATION,
			PED_EXCEPTION_OK,
			_("This HFS volume has no extents overflow "
			  "file.  This is quite unusual!"));
		return 1;
	}

	if (!hfs_file_read_sector (priv_data->extent_file, node, 0))
		return 0;
	header = ((HfsHeaderRecord*) (node + PED_BE16_TO_CPU(*((uint16_t *)
						(node+(PED_SECTOR_SIZE_DEFAULT-2))))));

	for (leaf_node = PED_BE32_TO_CPU (header->first_leaf_node);
	     leaf_node;
	     leaf_node = PED_BE32_TO_CPU (desc->next)) {
		if (!hfs_file_read_sector (priv_data->extent_file, node,
					   leaf_node))
			return 0;
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = 1; i <= record_number; i++) {
			uint8_t	where;
			extent_key = (HfsExtentKey*)
					(node + PED_BE16_TO_CPU(*((uint16_t *)
					      (node+(PED_SECTOR_SIZE_DEFAULT - 2*i)))));
			/* size is cst */
			extent = (HfsExtDescriptor*)(((uint8_t*)extent_key)
						       + sizeof (HfsExtentKey));
			/* check for obvious error in FS */
			if (((uint8_t*)extent_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)extent - node
			        >= PED_SECTOR_SIZE_DEFAULT
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				return 0;
			}

			switch (extent_key->file_ID) {
			    case PED_CPU_TO_BE32 (HFS_XTENT_ID) :
				if (ped_exception_throw (
					PED_EXCEPTION_WARNING,
					PED_EXCEPTION_IGNORE_CANCEL,
					_("The extents overflow file should not"
					  " contain its own extents!  You "
					  "should check the file system."))
						!= PED_EXCEPTION_IGNORE)
					return 0;
				where = CR_BTREE_EXT_EXT;
				break;
			    case PED_CPU_TO_BE32 (HFS_CATALOG_ID) :
				where = CR_BTREE_EXT_CAT;
				break;
			    default :
				where = CR_BTREE_EXT_0;
				break;
			}

			for (j = 0; j < HFS_EXT_NB; ++j) {
				if (!extent[j].block_count) break;
				if (!hfsc_cache_add_extent(
					cache,
					PED_BE16_TO_CPU(extent[j].start_block),
					PED_BE16_TO_CPU(extent[j].block_count),
					leaf_node,
					(uint8_t*)extent - node,
					1, /* hfs => btree block = 512 b */
					where,
					j )
				   )
					return 0;
			}
		}
	}

	return 1;
}

/* This function cache every extents start and length stored in any
   fs structure into the adt defined in cache.[ch]
   Returns NULL on failure */
static HfsCPrivateCache*
hfs_cache_extents(PedFileSystem *fs, PedTimer* timer)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsCPrivateCache*	ret;
	unsigned int		file_number, block_number;

	file_number = PED_BE32_TO_CPU(priv_data->mdb->file_count);
	block_number = PED_BE16_TO_CPU(priv_data->mdb->total_blocks);
	ret = hfsc_new_cache(block_number, file_number);
	if (!ret) return NULL;

	if (!hfs_cache_from_mdb(ret, fs, timer) ||
	    !hfs_cache_from_catalog(ret, fs, timer) ||
	    !hfs_cache_from_extent(ret, fs, timer)) {
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
hfs_pack_free_space_from_block (PedFileSystem *fs, unsigned int fblock,
			        PedTimer* timer, unsigned int to_free)
{
	PedSector		bytes_buff;
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsMasterDirectoryBlock* mdb = priv_data->mdb;
	HfsCPrivateCache*	cache;
	unsigned int 		to_fblock = fblock;
	unsigned int		start = fblock;
	unsigned int		divisor = PED_BE16_TO_CPU (mdb->total_blocks)
				          + 1 - start - to_free;
	int			ret;

	PED_ASSERT (!hfs_block);

	cache = hfs_cache_extents (fs, timer);
	if (!cache)
		return 0;

	/* Calculate the size of the copy buffer :
	 * Takes BLOCK_MAX_BUFF HFS blocks, but if > BYTES_MAX_BUFF
	 * takes the maximum number of HFS blocks so that the buffer
	 * will remain smaller than or equal to BYTES_MAX_BUFF, with
	 * a minimum of 1 HFS block */
	bytes_buff = PED_BE32_TO_CPU (priv_data->mdb->block_size)
		     * (PedSector) BLOCK_MAX_BUFF;
	if (bytes_buff > BYTES_MAX_BUFF) {
		hfs_block_count = BYTES_MAX_BUFF
				 / PED_BE32_TO_CPU (priv_data->mdb->block_size);
		if (!hfs_block_count)
			hfs_block_count = 1;
		bytes_buff = (PedSector) hfs_block_count
			     * PED_BE32_TO_CPU (priv_data->mdb->block_size);
	} else
		hfs_block_count = BLOCK_MAX_BUFF;

	/* If the cache code requests more space, give it to him */
	if (bytes_buff < hfsc_cache_needed_buffer (cache))
		bytes_buff = hfsc_cache_needed_buffer (cache);

	hfs_block = (uint8_t*) ped_malloc (bytes_buff);
	if (!hfs_block)
		goto error_cache;

	if (!hfs_read_bad_blocks (fs)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Bad blocks list could not be loaded."));
		goto error_alloc;
	}

	while (fblock < PED_BE16_TO_CPU (mdb->total_blocks)) {
		if (TST_BLOC_OCCUPATION(priv_data->alloc_map,fblock)
		    && (!hfs_is_bad_block (fs, fblock))) {
			if (!(ret = hfs_move_extent_starting_at (fs, &fblock,
						&to_fblock, cache)))
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

		ped_timer_update(timer, (float)(to_fblock - start)/divisor);
	}

	free (hfs_block); hfs_block = NULL; hfs_block_count = 0;
	hfsc_delete_cache (cache);
	return 1;

error_alloc:
	free (hfs_block); hfs_block = NULL; hfs_block_count = 0;
error_cache:
	hfsc_delete_cache (cache);
	return 0;
}

#endif /* !DISCOVER_ONLY */
