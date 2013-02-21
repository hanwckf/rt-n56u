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

#ifndef _CACHE_H
#define _CACHE_H

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include "hfs.h"

/* CR => CACHE REF */
#define CR_NULL			 0 /* reserved */
#define CR_PRIM_CAT		 1
#define CR_PRIM_EXT		 2
#define CR_PRIM_ATTR		 3
#define CR_PRIM_ALLOC		 4
#define CR_PRIM_START		 5
#define CR_BTREE_CAT		 6
#define CR_BTREE_ATTR		 7
#define CR_BTREE_EXT_0		 8
#define CR_BTREE_EXT_CAT	 9
#define CR_BTREE_EXT_EXT	10 /* should not happen ! */
#define CR_BTREE_EXT_ATTR	11
#define CR_BTREE_EXT_ALLOC	12
#define CR_BTREE_EXT_START	13 /* unneeded in current code */
#define CR_BTREE_CAT_JIB	14 /* journal info block */
#define CR_BTREE_CAT_JL		15 /* journal */
/* 16 -> 31 || high order bit */   /* reserved */

/* tuning */
#define CR_SHIFT		 8 /* number of bits to shift start_block by */
				   /* to get the index of the linked list */
#define CR_OVER_DIV		16 /* alloc a table for (1+1/CR_OVER_DIV) *
				      file_number + CR_ADD_CST */
#define CR_ADD_CST		16
#define CR_NEW_ALLOC_DIV	 4 /* divide the size of the first alloc table
				      by this value to allocate next tables */

/* See DOC for an explaination of this structure */
/* Access read only from outside cache.c */
struct _HfsCPrivateExtent {
	struct _HfsCPrivateExtent*	next;
	uint32_t			ext_start;
	uint32_t			ext_length;
	uint32_t			ref_block;
	uint16_t			ref_offset;
	uint8_t				sect_by_block;
	unsigned			where : 5;
	unsigned			ref_index : 3; /* 0 -> 7 */
};
typedef struct _HfsCPrivateExtent HfsCPrivateExtent;

/* Internaly used by cache.c for custom memory managment only */
struct _HfsCPrivateCacheTable {
	struct _HfsCPrivateCacheTable*	next_cache;
	HfsCPrivateExtent*		table;
	unsigned int			table_size;
	unsigned int			table_first_free;
	/* first_elemt ? */
};
typedef struct _HfsCPrivateCacheTable HfsCPrivateCacheTable;

/* Internaly used by cache.c for custom memory managment
   and cache handling only */
struct _HfsCPrivateCache {
	HfsCPrivateCacheTable*		table_list;
	HfsCPrivateCacheTable*		last_table;
	HfsCPrivateExtent**		linked_ref;
	unsigned int			linked_ref_size;
	unsigned int			block_number;
	unsigned int			first_cachetable_size;
	unsigned int			needed_alloc_size;
};
typedef struct _HfsCPrivateCache HfsCPrivateCache;

HfsCPrivateCache*
hfsc_new_cache(unsigned int block_number, unsigned int file_number);

void
hfsc_delete_cache(HfsCPrivateCache* cache);

HfsCPrivateExtent*
hfsc_cache_add_extent(HfsCPrivateCache* cache, uint32_t start, uint32_t length,
		      uint32_t block, uint16_t offset, uint8_t sbb,
		      uint8_t where, uint8_t index);

HfsCPrivateExtent*
hfsc_cache_search_extent(HfsCPrivateCache* cache, uint32_t start);

HfsCPrivateExtent*
hfsc_cache_move_extent(HfsCPrivateCache* cache, uint32_t old_start,
			uint32_t new_start);

static __inline__ unsigned int
hfsc_cache_needed_buffer(HfsCPrivateCache* cache)
{
	return cache->needed_alloc_size;
}

#endif /* _CACHE_H */
