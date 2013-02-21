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

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "hfs.h"
#include "advfs.h"
#include "file_plus.h"

#include "advfs_plus.h"

/* - if a < b, 0 if a == b, + if a > b */
/* Comparaison is done in the following order : */
/* CNID, then fork type, then start block */
static int
hfsplus_extent_key_cmp(HfsPPrivateGenericKey* a, HfsPPrivateGenericKey* b)
{
	HfsPExtentKey* key1 = (HfsPExtentKey*) a;
	HfsPExtentKey* key2 = (HfsPExtentKey*) b;

	if (key1->file_ID != key2->file_ID)
		return PED_BE32_TO_CPU(key1->file_ID) <
		       PED_BE32_TO_CPU(key2->file_ID) ?
				-1 : +1;

	if (key1->type != key2->type)
		return (int)(key1->type - key2->type);

	if (key1->start == key2->start)
		return 0;
	return PED_BE32_TO_CPU(key1->start) <
	       PED_BE32_TO_CPU(key2->start) ?
			-1 : +1;
}

/* do a B-Tree lookup */
/* read the first record immediatly inferior or egal to the given key */
/* return 0 on error */
/* record_out _must_ be large enough to receive the whole record (key + data) */
/* WARNING : the search function called only handle Extents BTree */
/*	     so modify this function if you want to do lookup in */
/* 	     other BTrees has well */
int
hfsplus_btree_search (HfsPPrivateFile* b_tree_file, HfsPPrivateGenericKey* key,
		      void *record_out, unsigned int record_size,
		      HfsCPrivateLeafRec* record_ref)
{
	uint8_t			node_1[PED_SECTOR_SIZE_DEFAULT];
	uint8_t*		node;
	HfsPHeaderRecord*	header;
	HfsPPrivateGenericKey*	record_key = NULL;
	unsigned int		node_number, record_number, size, bsize;
	int			i;

	/* Read the header node */
	if (!hfsplus_file_read_sector(b_tree_file, node_1, 0))
		return 0;
	header = (HfsPHeaderRecord*) (node_1 + HFS_FIRST_REC);

	/* Get the node number of the root */
	node_number = PED_BE32_TO_CPU (header->root_node);
	if (!node_number)
		return 0;

	/* Get the size of a node in sectors and allocate buffer */
	size = (bsize = PED_BE16_TO_CPU (header->node_size)) / PED_SECTOR_SIZE_DEFAULT;
	node = (uint8_t*) ped_malloc (bsize);
	if (!node)
		return 0;
	HfsPNodeDescriptor *desc = (HfsPNodeDescriptor*) node;

	/* Read the root node */
	if (!hfsplus_file_read (b_tree_file, node,
				(PedSector) node_number * size, size))
		return 0;

	/* Follow the white rabbit */
	while (1) {
		record_number = PED_BE16_TO_CPU (desc->rec_nb);
		for (i = record_number; i; i--) {
			record_key = (HfsPPrivateGenericKey*)
			    (node + PED_BE16_TO_CPU(*((uint16_t *)
					(node+(bsize - 2*i)))));
			/* check for obvious error in FS */
			if (((uint8_t*)record_key - node < HFS_FIRST_REC)
			    || ((uint8_t*)record_key - node
				>= (signed)bsize
				   - 2 * (signed)(record_number+1))) {
				ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("The file system contains errors."));
				free (node);
				return 0;
			}
			if (hfsplus_extent_key_cmp(record_key, key) <= 0)
				break;
		}
		if (!i) { free (node); return 0; }
		if (desc->type == HFS_IDX_NODE) {
			unsigned int 	skip;

			skip = ( 2 + PED_BE16_TO_CPU (record_key->key_length)
			         + 1 ) & ~1;
			node_number = PED_BE32_TO_CPU (*((uint32_t *)
					(((uint8_t *) record_key) + skip)));
			if (!hfsplus_file_read(b_tree_file, node,
					       (PedSector) node_number * size,
					       size)) {
				free (node);
				return 0;
			}
		} else
			break;
	}

	/* copy the result if needed */
	if (record_size)
		memcpy (record_out, record_key, record_size);

	/* send record reference if needed */
	if (record_ref) {
		record_ref->node_size = size;	/* in sectors */
		record_ref->node_number = node_number;
		record_ref->record_pos = (uint8_t*)record_key - node;
		record_ref->record_number = i;
	}

	/* success */
	free (node);
	return 1;
}

/* free the bad blocks linked list */
void
hfsplus_free_bad_blocks_list(HfsPPrivateLinkExtent* first)
{
	HfsPPrivateLinkExtent*	next;

	while (first) {
		next = first->next;
		free (first);
		first = next;
	}
}

/* This function reads bad blocks extents in the extents file
   and store it in f.s. specific data of fs */
int
hfsplus_read_bad_blocks (const PedFileSystem *fs)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						    fs->type_specific;

	if (priv_data->bad_blocks_loaded)
		return 1;

	{
	uint8_t			record[sizeof (HfsPExtentKey)
				       + sizeof (HfsPExtDataRec)];
	HfsPExtentKey		search;
	HfsPExtentKey*		ret_key = (HfsPExtentKey*) record;
	HfsPExtDescriptor*	ret_cache = (HfsPExtDescriptor*)
				    (record + sizeof (HfsPExtentKey));
	int			block, first_pass = 1;
	unsigned int		last_start;

	search.key_length = sizeof (HfsExtentKey) - 2;
	search.type = HFS_DATA_FORK;
	search.pad = 0;
	search.file_ID = PED_CPU_TO_BE32 (HFS_BAD_BLOCK_ID);

	last_start = -1; block = 0;
	while (1) {
		int i;

		search.start = PED_CPU_TO_BE32 (block);
		if (!hfsplus_btree_search (priv_data->extents_file,
					   (HfsPPrivateGenericKey*) &search,
					   record, sizeof (record), NULL)
		    || ret_key->file_ID != search.file_ID
		    || ret_key->type != search.type) {
			if (first_pass)
				break;
			else
				goto errbbp;
		}
		if (PED_BE32_TO_CPU (ret_key->start) == last_start)
			break;

		last_start = PED_BE32_TO_CPU (ret_key->start);
		for (i = 0; i < HFSP_EXT_NB; i++) {
			if (ret_cache[i].block_count) {
				HfsPPrivateLinkExtent*	new_xt =
				  (HfsPPrivateLinkExtent*) ped_malloc (
				    sizeof (HfsPPrivateLinkExtent));
				if (!new_xt)
					goto errbbp;
				new_xt->next = priv_data->bad_blocks_xtent_list;
				memcpy (&(new_xt->extent), ret_cache+i,
					sizeof (HfsPExtDescriptor));
				priv_data->bad_blocks_xtent_list = new_xt;
				priv_data->bad_blocks_xtent_nb++;
				block += PED_BE32_TO_CPU (
						ret_cache[i].block_count);
			}
		}
		first_pass = 0;
	}

	priv_data->bad_blocks_loaded = 1;
	return 1;}

errbbp: hfsplus_free_bad_blocks_list(priv_data->bad_blocks_xtent_list);
	priv_data->bad_blocks_xtent_list=NULL;
	priv_data->bad_blocks_xtent_nb=0;
	return 0;
}

/* This function check if fblock is a bad block */
int
hfsplus_is_bad_block (const PedFileSystem *fs, unsigned int fblock)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsPPrivateLinkExtent*	walk;

	for (walk = priv_data->bad_blocks_xtent_list; walk; walk = walk->next) {
		/* Won't compile without the strange cast ! gcc bug ? */
		/* or maybe C subtilties... */
		if ((fblock >= PED_BE32_TO_CPU (walk->extent.start_block)) &&
		    (fblock <  (unsigned int)(PED_BE32_TO_CPU (
						walk->extent.start_block)
			       + PED_BE32_TO_CPU (walk->extent.block_count))))
			return 1;
	}

	return 0;
}

/* This function returns the first sector of the last free block of
   an HFS+ volume we can get after a hfsplus_pack_free_space_from_block call */
PedSector
hfsplus_get_empty_end (const PedFileSystem *fs)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						    fs->type_specific;
	HfsPVolumeHeader* 	vh = priv_data->vh;
	unsigned int		block, last_bad, end_free_blocks;

	/* find the next block to the last bad block of the volume */
	if (!hfsplus_read_bad_blocks (fs)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Bad blocks could not be read."));
		return 0;
	}

	HfsPPrivateLinkExtent*	l;
	last_bad = 0;
	for (l = priv_data->bad_blocks_xtent_list; l; l = l->next) {
		if ((unsigned int) PED_BE32_TO_CPU (l->extent.start_block)
		    + PED_BE32_TO_CPU (l->extent.block_count) > last_bad)
			last_bad = PED_BE32_TO_CPU (l->extent.start_block)
			           + PED_BE32_TO_CPU (l->extent.block_count);
	}

	/* Count the free blocks from last_bad to the end of the volume */
	end_free_blocks = 0;
	for (block = last_bad;
	     block < PED_BE32_TO_CPU (vh->total_blocks);
	     block++) {
		if (!TST_BLOC_OCCUPATION(priv_data->alloc_map,block))
			end_free_blocks++;
	}

	/* Calculate the block that will by the first free at
	   the end of the volume */
	block = PED_BE32_TO_CPU (vh->total_blocks) - end_free_blocks;

	return (PedSector) block * ( PED_BE32_TO_CPU (vh->block_size)
				     / PED_SECTOR_SIZE_DEFAULT );
}

/* On error, returns 0 */
PedSector
hfsplus_get_min_size (const PedFileSystem *fs)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	PedSector		min_size;

	/* don't need to add anything because every sector
	   can be part of allocation blocks in HFS+, and
	   the last block _must_ be reserved */
	min_size = hfsplus_get_empty_end(fs);
	if (!min_size) return 0;

	if (priv_data->wrapper) {
		HfsPrivateFSData* 	hfs_priv_data = (HfsPrivateFSData*)
					    priv_data->wrapper->type_specific;
		unsigned int		hfs_sect_block;
		PedSector		hgee;
		hfs_sect_block =
		    PED_BE32_TO_CPU (hfs_priv_data->mdb->block_size)
		    / PED_SECTOR_SIZE_DEFAULT;
		/*
		 * if hfs+ is embedded in an hfs wrapper then the new size is :
		 * the new size of the hfs+ volume rounded up to the size
		 *     of hfs blocks
		 * + the minimum size of the hfs wrapper without any hfs+
		 *     modification
		 * - the current size of the hfs+ volume in the hfs wrapper
		 */
		hgee = hfs_get_empty_end(priv_data->wrapper);
		if (!hgee) return 0;
		min_size = ((min_size + hfs_sect_block - 1) / hfs_sect_block)
			   * hfs_sect_block
			 + hgee + 2
			 - (PedSector) PED_BE16_TO_CPU ( hfs_priv_data->mdb
							->old_new.embedded
							.location.block_count )
			   * hfs_sect_block;
	}

	return min_size;
}

/* return the block which should be used to pack data to have
   at least free fblock blocks at the end of the volume */
unsigned int
hfsplus_find_start_pack (const PedFileSystem *fs, unsigned int fblock)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	unsigned int		block;

	for (block = PED_BE32_TO_CPU (priv_data->vh->total_blocks) - 1;
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
