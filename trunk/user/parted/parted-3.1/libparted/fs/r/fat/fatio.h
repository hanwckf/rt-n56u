/*
    libparted
    Copyright (C) 1998-2000, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef FATIO_H_INCLUDED
#define FATIO_H_INCLUDED

#include "fat.h"

extern int fat_read_fragments (PedFileSystem* fs, char* buf, FatFragment frag,
			       FatFragment count);
extern int fat_write_fragments (PedFileSystem* fs, char* buf, FatFragment frag,
			        FatFragment count);
extern int fat_write_sync_fragments (PedFileSystem* fs, char* buf,
				     FatFragment frag, FatFragment count);

extern int fat_read_fragment (PedFileSystem* fs, char* buf, FatFragment frag);
extern int fat_write_fragment (PedFileSystem* fs, char* buf, FatFragment frag);
extern int fat_write_sync_fragment (PedFileSystem* fs, char* buf,
				    FatFragment frag);

extern int fat_read_clusters (PedFileSystem* fs, char* buf, FatCluster cluster,
			      FatCluster count);
extern int fat_write_clusters (PedFileSystem* fs, char* buf, FatCluster cluster,
			       FatCluster count);
extern int fat_write_sync_clusters (PedFileSystem* fs, char* buf,
				    FatCluster cluster, FatCluster count);

extern int fat_read_cluster (PedFileSystem* fs, char *buf, FatCluster cluster);
extern int fat_write_cluster (PedFileSystem* fs, char *buf, FatCluster cluster);
extern int fat_write_sync_cluster (PedFileSystem* fs, char *buf,
				   FatCluster cluster);

#endif /* FATIO_H_INCLUDED */
