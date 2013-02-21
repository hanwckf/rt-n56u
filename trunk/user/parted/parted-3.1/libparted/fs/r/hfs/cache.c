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

#include "cache.h"

static HfsCPrivateCacheTable*
hfsc_new_cachetable(unsigned int size)
{
	HfsCPrivateCacheTable* ret;

	ret = (HfsCPrivateCacheTable*) ped_malloc(sizeof(*ret));
	if (!ret) return NULL;

	ret->next_cache = NULL;
	ret->table_size = size;
	ret->table_first_free = 0;

	ret->table = ped_malloc(sizeof(*ret->table)*size);
	if (!ret->table) { free(ret); return NULL; }
	memset(ret->table, 0, sizeof(*ret->table)*size);

	return ret;
}

HfsCPrivateCache*
hfsc_new_cache(unsigned int block_number, unsigned int file_number)
{
	unsigned int		cachetable_size, i;
	HfsCPrivateCache*	ret;

	ret = (HfsCPrivateCache*) ped_malloc(sizeof(*ret));
	if (!ret) return NULL;
	ret->block_number = block_number;
	/* following code avoid integer overflow */
	ret->linked_ref_size = block_number > block_number + ((1<<CR_SHIFT)-1) ?
				( block_number >> CR_SHIFT ) + 1 :
				( block_number + ((1<<CR_SHIFT)-1) ) >> CR_SHIFT
			     ;

	ret->linked_ref = (HfsCPrivateExtent**)
			   ped_malloc( sizeof(*ret->linked_ref)
			   		* ret->linked_ref_size );
	if (!ret->linked_ref) { free(ret); return NULL; }

	cachetable_size = file_number + file_number / CR_OVER_DIV + CR_ADD_CST;
	if (cachetable_size < file_number) cachetable_size = (unsigned) -1;
	ret->first_cachetable_size = cachetable_size;
	ret->table_list = hfsc_new_cachetable(cachetable_size);
	if (!ret->table_list) {
		free(ret->linked_ref);
		free(ret);
		return NULL;
	}
	ret->last_table = ret->table_list;

	for (i = 0; i < ret->linked_ref_size; ++i)
		ret->linked_ref[i] = NULL;

	ret->needed_alloc_size = 0;

	return ret;
}

static void
hfsc_delete_cachetable(HfsCPrivateCacheTable* list)
{
	HfsCPrivateCacheTable* next;

	while (list) {
		free (list->table);
		next = list->next_cache;
		free (list);
		list = next;
	}
}

void
hfsc_delete_cache(HfsCPrivateCache* cache)
{
	hfsc_delete_cachetable(cache->table_list);
	free(cache->linked_ref);
	free(cache);
}

HfsCPrivateExtent*
hfsc_cache_add_extent(HfsCPrivateCache* cache, uint32_t start, uint32_t length,
		      uint32_t block, uint16_t offset, uint8_t sbb,
		      uint8_t where, uint8_t ref_index)
{
	HfsCPrivateExtent*	ext;
	unsigned int		idx = start >> CR_SHIFT;

	PED_ASSERT(idx < cache->linked_ref_size);

	for (ext = cache->linked_ref[idx];
	     ext && start != ext->ext_start;
	     ext = ext->next);

	if (ext) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Trying to register an extent starting at block "
			  "0x%X, but another one already exists at this "
			  "position.  You should check the file system!"),
			start);
		return NULL;
	}

	if ( cache->last_table->table_first_free
	     == cache->last_table->table_size ) {
		cache->last_table->next_cache =
			hfsc_new_cachetable( ( cache->first_cachetable_size
					       / CR_NEW_ALLOC_DIV )
					     + CR_ADD_CST );
		if (!cache->last_table->next_cache)
			return NULL;
		cache->last_table = cache->last_table->next_cache;
	}

	ext = cache->last_table->table+(cache->last_table->table_first_free++);

	ext->ext_start = start;
	ext->ext_length = length;
	ext->ref_block = block;
	ext->ref_offset = offset;
	ext->sect_by_block = sbb;
	ext->where = where;
	ext->ref_index = ref_index;

	ext->next = cache->linked_ref[idx];
	cache->linked_ref[idx] = ext;

	cache->needed_alloc_size = cache->needed_alloc_size >
				   (unsigned) PED_SECTOR_SIZE_DEFAULT * sbb ?
				   cache->needed_alloc_size :
				   (unsigned) PED_SECTOR_SIZE_DEFAULT * sbb;

	return ext;
}

HfsCPrivateExtent*
hfsc_cache_search_extent(HfsCPrivateCache* cache, uint32_t start)
{
	HfsCPrivateExtent*	ret;
	unsigned int	idx = start >> CR_SHIFT;

	PED_ASSERT(idx < cache->linked_ref_size);

	for (ret = cache->linked_ref[idx];
	     ret && start != ret->ext_start;
	     ret = ret->next);

	return ret;
}

/* Can't fail if extent begining at old_start exists */
/* Returns 0 if no such extent, or on error */
HfsCPrivateExtent*
hfsc_cache_move_extent(HfsCPrivateCache* cache, uint32_t old_start,
			uint32_t new_start)
{
	HfsCPrivateExtent**	ppext;
	HfsCPrivateExtent*	pext;

	unsigned int 		idx1 = old_start >> CR_SHIFT;
	unsigned int		idx2 = new_start >> CR_SHIFT;

	PED_ASSERT(idx1 < cache->linked_ref_size);
	PED_ASSERT(idx2 < cache->linked_ref_size);

	for (pext = cache->linked_ref[idx2];
	     pext && new_start != pext->ext_start;
	     pext = pext->next);

	if (pext) {
		ped_exception_throw (
			PED_EXCEPTION_BUG,
			PED_EXCEPTION_CANCEL,
			_("Trying to move an extent from block Ox%X to block "
			  "Ox%X, but another one already exists at this "
			  "position.  This should not happen!"),
			old_start, new_start);
		return NULL;
	}

	for (ppext = &(cache->linked_ref[idx1]);
	     (*ppext) && old_start != (*ppext)->ext_start;
	     ppext = &((*ppext)->next));

	if (!(*ppext)) return NULL;

	/* removing the extent from the cache */
	pext = *ppext;
	(*ppext) = pext->next;

	/* change ext_start and insert the extent again */
	pext->ext_start = new_start;
	pext->next = cache->linked_ref[idx2];
	cache->linked_ref[idx2] = pext;

	return pext;
}

#endif /* DISCOVER_ONLY */
