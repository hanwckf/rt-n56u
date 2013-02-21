/*
    libparted
    Copyright (C) 1999-2000, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef PED_FAT_CONTEXT_H_INCLUDED
#define PED_FAT_CONTEXT_H_INCLUDED

#include "count.h"

enum _FatDirection {
	FAT_DIR_FORWARD,
	FAT_DIR_BACKWARD
};
typedef enum _FatDirection FatDirection;

struct _FatOpContext {
	PedFileSystem*		old_fs;
	PedFileSystem*		new_fs;

	PedSector		frag_sectors;	/* should equal old_fs and
						   new_fs's frag_sectors */

	FatDirection		start_move_dir;
	FatFragment		start_move_delta;

	FatFragment		buffer_offset;
	FatFragment		buffer_frags;
	FatFragment*		buffer_map;

	FatFragment		frags_duped;

	FatFragment*		remap;

	FatCluster		new_root_dir [32];
};
typedef struct _FatOpContext FatOpContext;

extern FatOpContext* fat_op_context_new (PedFileSystem* new_fs,
					 PedFileSystem* old_fs);

extern void fat_op_context_destroy (FatOpContext* ctx);

extern FatFragment fat_op_context_map_static_fragment (const FatOpContext* ctx,
						       FatFragment frag);
extern FatCluster fat_op_context_map_static_cluster (const FatOpContext* ctx,
						     FatCluster clst);

extern FatFragment fat_op_context_map_fragment (const FatOpContext* ctx,
					        FatFragment frag);
extern FatCluster fat_op_context_map_cluster (const FatOpContext* ctx,
					      FatCluster clst);

extern int fat_op_context_create_initial_fat (FatOpContext* ctx);

#endif /* PED_FAT_CONTEXT_H_INCLUDED */
