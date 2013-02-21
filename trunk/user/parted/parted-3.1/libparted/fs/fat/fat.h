/*
    libparted
    Copyright (C) 1998-2001, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef FAT_H_INCLUDED
#define FAT_H_INCLUDED

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE  1024	/* buffer size in sectors (512 bytes) */

typedef uint32_t		FatCluster;
typedef int32_t			FatFragment;

enum _FatType {
	FAT_TYPE_FAT12,
	FAT_TYPE_FAT16,
	FAT_TYPE_FAT32
};
typedef enum _FatType		FatType;

typedef struct _FatSpecific	FatSpecific;
typedef struct _FatDirEntry	FatDirEntry;

#include "bootsector.h"
#include "count.h"

struct _FatTable {
	void*		table;
	FatCluster	size;
	int		raw_size;

	FatType		fat_type;
	FatCluster	cluster_count;
	FatCluster	free_cluster_count;
	FatCluster	bad_cluster_count;

	FatCluster	last_alloc;
};
typedef struct _FatTable	FatTable;

struct __attribute__ ((packed)) _FatDirEntry {
	char		name[8];
	uint8_t		extension[3];
	uint8_t		attributes;
	uint8_t		is_upper_case_name;
	uint8_t		creation_time_low;      /* milliseconds */
	uint16_t	creation_time_high;
	uint16_t	creation_date;
	uint16_t	access_date;
	uint16_t	first_cluster_high;     /* for FAT32 */
	uint16_t	time;
	uint16_t	date;
	uint16_t	first_cluster;
	uint32_t	length;
};

struct _FatSpecific {
	FatBootSector	boot_sector;    /* structure of boot sector */
	FatInfoSector	info_sector;    /* fat32-only information sector */

	int		logical_sector_size;	/* illogical sector size :-) */
	PedSector	sector_count;

	int		sectors_per_track;	/* BIOS CHS stuff (S) */
	int		heads;			/* BIOS CHS stuff (H) */

	int		cluster_size;
	PedSector	cluster_sectors;
	FatCluster	cluster_count;
	int		dir_entries_per_cluster;

	FatType		fat_type;
	int		fat_table_count;
	PedSector	fat_sectors;

	uint32_t	serial_number;

	PedSector	info_sector_offset;     /* FAT32 only */
	PedSector	fat_offset;
	PedSector	root_dir_offset;	/* non-FAT32 */
	PedSector	cluster_offset;
	PedSector	boot_sector_backup_offset;

	FatCluster	root_cluster;           /* FAT32 only */
	int		root_dir_entry_count;   /* non-FAT32 */
	PedSector	root_dir_sector_count;  /* non-FAT32 */
	FatCluster	total_dir_clusters;

	FatTable*	fat;
	FatClusterInfo*	cluster_info;

	PedSector	buffer_sectors;
	char*		buffer;

	int		frag_size;
	PedSector	frag_sectors;
	FatFragment	frag_count;
	FatFragment	buffer_frags;
	FatFragment	cluster_frags;
};

#define FAT_SPECIFIC(fs)	((FatSpecific*) fs->type_specific)

#define FAT_ROOT		0

#define DELETED_FLAG		0xe5

#define READONLY_ATTR		0x01
#define HIDDEN_ATTR		0x02
#define SYSTEM_ATTR		0x04
#define VOLUME_LABEL_ATTR	0x08
#define VFAT_ATTR		0x0f
#define DIRECTORY_ATTR		0x10
#define ARCH_ATTR		0x20

#define MAX_FAT12_CLUSTERS	4086
#define MAX_FAT16_CLUSTERS	65526
#define MAX_FAT32_CLUSTERS	2000000

#define FAT_ROOT_DIR_ENTRY_COUNT	512

extern PedFileSystemType fat16_type;
extern PedFileSystemType fat32_type;

extern void fat_print (const PedFileSystem* fs);

extern PedFileSystem* fat_alloc (const PedGeometry* geom);
extern void fat_free (PedFileSystem* fs);
extern int fat_alloc_buffers (PedFileSystem* fs);

extern int fat_resize (PedFileSystem* fs, PedGeometry* geom, PedTimer* timer);

#endif /* FAT_H_INCLUDED */
