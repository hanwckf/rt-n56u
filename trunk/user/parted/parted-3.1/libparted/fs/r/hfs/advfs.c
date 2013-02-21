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

/* - if a < b, 0 if a == b, + if a > b */
/* Comparaison is done in the following order : */
/* CNID, then fork type, then start block */
/* Note that HFS implementation in linux has a bug */
/* in this function */
static int
hfs_extent_key_cmp(HfsPrivateGenericKey* a, HfsPrivateGenericKey* b)
{
	HfsExtentKey* key1 = (HfsExtentKey*) a;
	HfsExtentKey* key2 = (HfsExtentKey*) b;

	/* do NOT use a substraction, because */
	/* 0xFFFFFFFF - 1 = 0xFFFFFFFE so this */
	/* would return -2, despite the fact */
	/* 0xFFFFFFFF > 1 !!! (this is the 2.4 bug) */
	if (key1->file_ID != key2->file_ID)
		return PED_BE32_TO_CPU(key1->file_ID) <
		       PED_BE32_TO_CPU(key2->file_ID) ?
				-1 : +1;

	if (key1->type != key2->type)
		return (int)(key1->type - key2->type);

	if (key1->start == key2->start)
		return 0;
	/* the whole thing wont work with 16 bits ints */
	/* anyway */
	return (int)( PED_BE16_TO_CPU(key1->start) -
		      PED_BE16_TO_CPU(key2->start) );
}

/* do a B-Tree lookup */
/* read the first record immediatly inferior or egal to the given key */
/* return 0 on error */
/* record_out _must_ be large enough to receive record_size bytes */
/* WARNING : the compare function called only handle Extents BTree */
/*	     so modify this function if you want to do lookup in */
/* 	     other BTrees has well */
int
hfs_btree_search (HfsPrivateFile* b_tree_file, HfsPrivateGenericKey* key,
		  void *record_out, unsigned int record_size,
		  HfsCPrivateLeafRec* record_ref)
{
	uint8_t			node[PED_SECTOR_SIZE_DEFAULT];
	HfsHeaderRecord*	header;
	HfsNodeDescriptor*	desc = (HfsNodeDescriptor*) node;
	HfsPrivateGenericKey*	record_key = NULL;
	unsigned int		node_number, record_number;
	int			i;

	/* Read the header node */
	if (!hfs_file_read_sector(b_tree_file, node, 0))
		return 0;
	header = ((HfsHeaderRecord*) (node + PED_BE16_TO_CPU(*((uint16_t *)
						(node+(PED_SECTOR_SIZE_DEFAULT-2))))));

	/* Get the node number of the root */
	node_number = PED_BE32_TO_CPU(header->root_node);
	if (!node_number)
		return 0;

	/* Read the root node */
	if (!hfs_file_read_sector(b_tree_file, node, node_number))
		return 0;

	/* Follow the white rabbit */
	while (1) {
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = record_number; i; i--) {
			record_key = (HfsPrivateGenericKey*)
				(node + PED_BE16_TO_CPU(*((uint16_t *)
					    (node+(PED_SECTOR_SIZE_DEFAULT - 2*i)))));
			/* check for obvious error in FS */
			if (((uint8_t*)record_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)record_key - node
				>= PED_SECTOR_SIZE_DEFAULT
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				return 0;
			}
			if (hfs_extent_key_cmp(record_key, key) <= 0)
				break;
		}
		if (!i) return 0;
		if (desc->type == HFS_IDX_NODE) {
			unsigned int 	skip;

			skip = (1 + record_key->key_length + 1) & ~1;
			node_number = PED_BE32_TO_CPU (*((uint32_t *)
					(((uint8_t *) record_key) + skip)));
			if (!hfs_file_read_sector(b_tree_file, node,
						  node_number))
				return 0;
		} else
			break;
	}

	/* copy the result if needed */
	if (record_size)
		memcpy (record_out, record_key, record_size);

	/* send record reference if needed */
	if (record_ref) {
		record_ref->node_size = 1;	/* in sectors */
		record_ref->node_number = node_number;
		record_ref->record_pos = (uint8_t*)record_key - node;
		record_ref->record_number = i;
	}

	/* success */
	return 1;
}

/* free the bad blocks linked list */
void
hfs_free_bad_blocks_list(HfsPrivateLinkExtent* first)
{
	HfsPrivateLinkExtent*	next;

	while (first) {
		next = first->next;
		free (first);
		first = next;
	}
}

/* This function reads bad blocks extents in the extents file
   and store it in f.s. specific data of fs */
int
hfs_read_bad_blocks (const PedFileSystem *fs)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;

	if (priv_data->bad_blocks_loaded)
		return 1;

	{
	uint8_t			record[sizeof (HfsExtentKey)
					       + sizeof (HfsExtDataRec)];
	HfsExtentKey		search;
	HfsExtentKey*		ret_key = (HfsExtentKey*) record;
	HfsExtDescriptor*	ret_cache = (HfsExtDescriptor*)
					     (record + sizeof (HfsExtentKey));
	unsigned int		block, last_start, first_pass = 1;

	search.key_length = sizeof (HfsExtentKey) - 1;
	search.type = HFS_DATA_FORK;
	search.file_ID = PED_CPU_TO_BE32 (HFS_BAD_BLOCK_ID);

	last_start = -1; block = 0;
	while (1) {
		int i;

		search.start = PED_CPU_TO_BE16 (block);
		if (!hfs_btree_search (priv_data->extent_file,
				       (HfsPrivateGenericKey*) &search,
				       record, sizeof (record), NULL)
		    || ret_key->file_ID != search.file_ID
		    || ret_key->type != search.type) {
			if (first_pass)
				break;
			else
				goto errbb;
		}
		if (PED_BE16_TO_CPU (ret_key->start) == last_start)
		    break;

		last_start = PED_BE16_TO_CPU (ret_key->start);
		for (i = 0; i < HFS_EXT_NB; i++) {
			if (ret_cache[i].block_count) {
				HfsPrivateLinkExtent*	new_xt =
				   (HfsPrivateLinkExtent*) ped_malloc (
					sizeof (HfsPrivateLinkExtent));
				if (!new_xt)
					goto errbb;
				new_xt->next = priv_data->bad_blocks_xtent_list;
				memcpy(&(new_xt->extent), ret_cache+i,
					sizeof (HfsExtDescriptor));
				priv_data->bad_blocks_xtent_list = new_xt;
				priv_data->bad_blocks_xtent_nb++;
				block += PED_BE16_TO_CPU (
						ret_cache[i].block_count);
			}
		}
		first_pass = 0;
	}

	priv_data->bad_blocks_loaded = 1;
	return 1;}

errbb:	hfs_free_bad_blocks_list(priv_data->bad_blocks_xtent_list);
	priv_data->bad_blocks_xtent_list=NULL;
	priv_data->bad_blocks_xtent_nb=0;
	return 0;
}

/* This function check if fblock is a bad block */
int
hfs_is_bad_block (const PedFileSystem *fs, unsigned int fblock)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsPrivateLinkExtent*	walk;

	for (walk = priv_data->bad_blocks_xtent_list; walk; walk = walk->next) {
		/* Won't compile without the strange cast ! gcc bug ? */
		/* or maybe C subtilties... */
		if ((fblock >= PED_BE16_TO_CPU (walk->extent.start_block)) &&
		    (fblock <  (unsigned int) (PED_BE16_TO_CPU (
						    walk->extent.start_block)
					       + PED_BE16_TO_CPU (
						    walk->extent.block_count))))
			return 1;
	}

	return 0;
}

/* This function returns the first sector of the last free block of an
   HFS volume we can get after a hfs_pack_free_space_from_block call */
/* On error this function returns 0 */
PedSector
hfs_get_empty_end (const PedFileSystem *fs)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	HfsMasterDirectoryBlock* mdb = priv_data->mdb;
	unsigned int		block, last_bad, end_free_blocks;

	/* find the next block to the last bad block of the volume */
	if (!hfs_read_bad_blocks (fs))
		return 0;

	HfsPrivateLinkExtent*	l;
	last_bad = 0;
	for (l = priv_data->bad_blocks_xtent_list; l; l = l->next) {
		if ((unsigned int) PED_BE16_TO_CPU (l->extent.start_block)
		    + PED_BE16_TO_CPU (l->extent.block_count) > last_bad)
			last_bad = PED_BE16_TO_CPU (l->extent.start_block)
			           + PED_BE16_TO_CPU (l->extent.block_count);
	}

	/* Count the free blocks from last_bad to the end of the volume */
	end_free_blocks = 0;
	for (block = last_bad;
	     block < PED_BE16_TO_CPU (mdb->total_blocks);
	     block++) {
		if (!TST_BLOC_OCCUPATION(priv_data->alloc_map,block))
			end_free_blocks++;
	}

	/* Calculate the block that will by the first free at the
	   end of the volume */
	block = PED_BE16_TO_CPU (mdb->total_blocks) - end_free_blocks;

	return (PedSector) PED_BE16_TO_CPU (mdb->start_block)
		+ (PedSector) block * (PED_BE32_TO_CPU (mdb->block_size)
				       / PED_SECTOR_SIZE_DEFAULT);
}

/* return the block which should be used to pack data to have at
   least free fblock blocks at the end of the volume */
unsigned int
hfs_find_start_pack (const PedFileSystem *fs, unsigned int fblock)
{
	HfsPrivateFSData* 	priv_data = (HfsPrivateFSData*)
						fs->type_specific;
	unsigned int		block;

	for (block = PED_BE16_TO_CPU (priv_data->mdb->total_blocks) - 1;
	     block && fblock;
	     block--) {
		if (!TST_BLOC_OCCUPATION(priv_data->alloc_map,block))
			fblock--;
	}

	while (block && !TST_BLOC_OCCUPATION(priv_data->alloc_map,block))
		block--;
	if (TST_BLOC_OCCUPATION(priv_data->alloc_map,block))
		block++;

	return block;
}

#endif /* !DISCOVER_ONLY */
