/*
    libparted/fs_amiga - amiga file system support.
    Copyright (C) 2000-2001, 2007, 2009-2012 Free Software Foundation, Inc.

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

    Contributor:  Sven Luther <luther@debian.org>
*/

#include <config.h>
#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>

#include "amiga.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#define	IDNAME_RIGIDDISK	(uint32_t)0x5244534B	/* 'RDSK' */
#define IDNAME_BADBLOCK		(uint32_t)0x42414442	/* 'BADB' */
#define	IDNAME_PARTITION	(uint32_t)0x50415254	/* 'PART' */
#define IDNAME_FILESYSHEADER	(uint32_t)0x46534844	/* 'FSHD' */
#define IDNAME_LOADSEG		(uint32_t)0x4C534547	/* 'LSEG' */
#define IDNAME_BOOT		(uint32_t)0x424f4f54	/* 'BOOT' */
#define IDNAME_FREE		(uint32_t)0xffffffff

static const char *
_amiga_block_id (uint32_t id) {
	switch (id) {
		case IDNAME_RIGIDDISK :
			return "RDSK";
		case IDNAME_BADBLOCK :
			return "BADB";
		case IDNAME_PARTITION :
			return "PART";
		case IDNAME_FILESYSHEADER :
			return "FSHD";
		case IDNAME_LOADSEG :
			return "LSEG";
		case IDNAME_BOOT :
			return "BOOT";
		case IDNAME_FREE :
			return "<free>";
		default :
			return "<unknown>";
	}
}

struct AmigaIds *
_amiga_add_id (uint32_t id, struct AmigaIds *ids) {
	struct AmigaIds *newid;

	if ((newid=ped_malloc(sizeof (struct AmigaIds)))==NULL) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Failed to allocate id list element\n"), __func__);
		return 0;
	}
	newid->ID = id;
	newid->next = ids;
	return newid;
}

void
_amiga_free_ids (struct AmigaIds *ids) {
	struct AmigaIds *current, *next;

	for (current = ids; current != NULL; current = next) {
		next = current->next;
		free (current);
	}
}
int
_amiga_id_in_list (uint32_t id, struct AmigaIds *ids) {
	struct AmigaIds *current;

	for (current = ids; current != NULL; current = current->next) {
		if (id == current->ID)
			return 1;
	}
	return 0;
}

#define AMIGA_RDB_NOT_FOUND	((uint32_t)0xffffffff)

struct AmigaBlock {
    uint32_t	amiga_ID;		/* Identifier 32 bit word */
    uint32_t	amiga_SummedLongss;	/* Size of the structure for checksums */
    int32_t	amiga_ChkSum;		/* Checksum of the structure */
};
#define AMIGA(pos) ((struct AmigaBlock *)(pos))

struct RigidDiskBlock {
    uint32_t	rdb_ID;			/* Identifier 32 bit word : 'RDSK' */
    uint32_t	rdb_SummedLongs;	/* Size of the structure for checksums */
    int32_t	rdb_ChkSum;		/* Checksum of the structure */
    uint32_t	rdb_HostID;		/* SCSI Target ID of host, not really used */
    uint32_t	rdb_BlockBytes;		/* Size of disk blocks */
    uint32_t	rdb_Flags;		/* RDB Flags */
    /* block list heads */
    uint32_t	rdb_BadBlockList;	/* Bad block list */
    uint32_t	rdb_PartitionList;	/* Partition list */
    uint32_t	rdb_FileSysHeaderList;	/* File system header list */
    uint32_t	rdb_DriveInit;		/* Drive specific init code */
    uint32_t	rdb_BootBlockList;	/* Amiga OS 4 Boot Blocks */
    uint32_t	rdb_Reserved1[5];	/* Unused word, need to be set to $ffffffff */
    /* physical drive characteristics */
    uint32_t	rdb_Cylinders;		/* Number of the cylinders of the drive */
    uint32_t	rdb_Sectors;		/* Number of sectors of the drive */
    uint32_t	rdb_Heads;		/* Number of heads of the drive */
    uint32_t	rdb_Interleave;		/* Interleave */
    uint32_t	rdb_Park;		/* Head parking cylinder */
    uint32_t	rdb_Reserved2[3];	/* Unused word, need to be set to $ffffffff */
    uint32_t	rdb_WritePreComp;	/* Starting cylinder of write precompensation */
    uint32_t	rdb_ReducedWrite;	/* Starting cylinder of reduced write current */
    uint32_t	rdb_StepRate;		/* Step rate of the drive */
    uint32_t	rdb_Reserved3[5];	/* Unused word, need to be set to $ffffffff */
    /* logical drive characteristics */
    uint32_t	rdb_RDBBlocksLo;	/* low block of range reserved for hardblocks */
    uint32_t	rdb_RDBBlocksHi;	/* high block of range for these hardblocks */
    uint32_t	rdb_LoCylinder;		/* low cylinder of partitionable disk area */
    uint32_t	rdb_HiCylinder;		/* high cylinder of partitionable data area */
    uint32_t	rdb_CylBlocks;		/* number of blocks available per cylinder */
    uint32_t	rdb_AutoParkSeconds;	/* zero for no auto park */
    uint32_t	rdb_HighRDSKBlock;	/* highest block used by RDSK */
					/* (not including replacement bad blocks) */
    uint32_t	rdb_Reserved4;
    /* drive identification */
    char	rdb_DiskVendor[8];
    char	rdb_DiskProduct[16];
    char	rdb_DiskRevision[4];
    char	rdb_ControllerVendor[8];
    char	rdb_ControllerProduct[16];
    char	rdb_ControllerRevision[4];
    uint32_t	rdb_Reserved5[10];
};

#define AMIGA_MAX_PARTITIONS	128
#define	RDB_LOCATION_LIMIT	16
#define RDSK(pos) ((struct RigidDiskBlock *)(pos))

static int
_amiga_checksum (struct AmigaBlock *blk) {
	uint32_t *rdb = (uint32_t *) blk;
	uint32_t sum;
	int i, end;

	sum = PED_BE32_TO_CPU (rdb[0]);
	end = PED_BE32_TO_CPU (rdb[1]);

	if (end > PED_SECTOR_SIZE_DEFAULT) end = PED_SECTOR_SIZE_DEFAULT;

	for (i = 1; i < end; i++) sum += PED_BE32_TO_CPU (rdb[i]);

	return sum;
}

static void
_amiga_calculate_checksum (struct AmigaBlock *blk) {

	blk->amiga_ChkSum = PED_CPU_TO_BE32(
		PED_BE32_TO_CPU(blk->amiga_ChkSum) -
		_amiga_checksum((struct AmigaBlock *) blk));
	return;
}


static struct AmigaBlock *
_amiga_read_block (PedDevice *dev, struct AmigaBlock *blk, PedSector block, struct AmigaIds *ids) {
	if (!ped_device_read (dev, blk, block, 1)) {
		switch (ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Couldn't read block %llu\n"), __func__, block))
		{
			case PED_EXCEPTION_CANCEL :
			case PED_EXCEPTION_UNHANDLED :
			default :
				return NULL;
		}
	}
	if (ids && !_amiga_id_in_list(PED_BE32_TO_CPU(blk->amiga_ID), ids))
		return NULL;
	if (_amiga_checksum (blk) != 0) {
		switch (ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_FIX | PED_EXCEPTION_IGNORE | PED_EXCEPTION_CANCEL,
			_("%s : Bad checksum on block %llu of type %s\n"),
			__func__, block, _amiga_block_id(PED_BE32_TO_CPU(blk->amiga_ID))))
		{
			case PED_EXCEPTION_CANCEL :
				return NULL;
			case PED_EXCEPTION_FIX :
				_amiga_calculate_checksum(AMIGA(blk));
				if (!ped_device_write (dev, blk, block, 1)) {
					switch (ped_exception_throw(PED_EXCEPTION_FATAL,
						PED_EXCEPTION_CANCEL,
						_("%s : Couldn't write block %d\n"), __func__, block))
					{
						case PED_EXCEPTION_CANCEL :
						case PED_EXCEPTION_UNHANDLED :
						default :
							return NULL;
					}
				}
			case PED_EXCEPTION_IGNORE :
			case PED_EXCEPTION_UNHANDLED :
			default :
				return blk;
		}
	}
	return blk;
}

static uint32_t
_amiga_find_rdb (PedDevice *dev, struct RigidDiskBlock *rdb) {
	int i;
	struct AmigaIds *ids;

	ids = _amiga_add_id (IDNAME_RIGIDDISK, NULL);

	for (i = 0; i<RDB_LOCATION_LIMIT; i++) {
		if (!_amiga_read_block (dev, AMIGA(rdb), i, ids)) {
			continue;
		}
		if (PED_BE32_TO_CPU (rdb->rdb_ID) == IDNAME_RIGIDDISK) {
			_amiga_free_ids (ids);
			return i;
		}
	}
	_amiga_free_ids (ids);
	return AMIGA_RDB_NOT_FOUND;
}

static int
_amiga_loop_check (uint32_t block, uint32_t * blocklist, uint32_t max)
{
	uint32_t i;

	for (i = 0; i < max; i++)
		if (block == blocklist[i]) {
			/* We are looping, let's stop.  */
			return 1;
		}
	blocklist[max] = block;
	return 0;
}

/* We have already allocated a rdb, we are now reading it from the disk */
struct PartitionBlock *
amiga_find_part (PedGeometry *geom, struct PartitionBlock *part)
{
	struct RigidDiskBlock *rdb;
	uint32_t partblock;
	uint32_t partlist[AMIGA_MAX_PARTITIONS];
	int i;

	PED_ASSERT(geom!= NULL);
	PED_ASSERT(geom->dev!= NULL);

	if (!(rdb = ped_malloc (PED_SECTOR_SIZE_DEFAULT))) {
		switch (ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Failed to allocate disk_specific rdb block\n"), __func__))
		{
			case PED_EXCEPTION_CANCEL :
			case PED_EXCEPTION_UNHANDLED :
			default :
				return NULL;
		}
	}
	if (_amiga_find_rdb (geom->dev, rdb) == AMIGA_RDB_NOT_FOUND) {
		switch (ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Didn't find rdb block, should never happen\n"), __func__))
		{
			case PED_EXCEPTION_CANCEL :
			case PED_EXCEPTION_UNHANDLED :
			default :
				free(rdb);
				return NULL;
		}
	}

	/* We initialize the hardblock free list to detect loops */
	for (i = 0; i < AMIGA_MAX_PARTITIONS; i++) partlist[i] = IDNAME_FREE;

	for (i = 1, partblock = PED_BE32_TO_CPU(rdb->rdb_PartitionList);
		i < AMIGA_MAX_PARTITIONS && partblock != IDNAME_FREE;
		i++, partblock = PED_BE32_TO_CPU(part->pb_Next))
	{
		PedSector start, end;
		PedSector cylblocks;

		/* Let's look for loops in the partition table */
		if (_amiga_loop_check(partblock, partlist, i)) {
			free (rdb);
			return NULL;
		}
		/* Let's read a partition block to get its geometry*/
		if (!ped_device_read (geom->dev, part, (PedSector)partblock, 1)) {
			switch (ped_exception_throw(PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("%s : Failed to read partition block %llu\n"),
				__func__, (PedSector)partblock))
			{
				case PED_EXCEPTION_CANCEL :
				case PED_EXCEPTION_UNHANDLED :
				default :
					free(rdb);
					return NULL;
			}
		}

		/* Current block is not a Partition Block */
		if (part->pb_ID != IDNAME_PARTITION) {
			free (rdb);
			return NULL;
		}

		/* Calculate the geometry of the partition */
		cylblocks = ((PedSector) PED_BE32_TO_CPU (part->de_Surfaces)) *
			((PedSector) PED_BE32_TO_CPU (part->de_BlocksPerTrack));
		start = ((PedSector) PED_BE32_TO_CPU (part->de_LowCyl)) * cylblocks;
		end = ((((PedSector) PED_BE32_TO_CPU (part->de_HighCyl))+1) * (cylblocks))-1;

		/* And check if it is the one we are searching for */
		if (start == geom->start && end == geom->end) {
			free (rdb);
			return part;
		}
	}

	free (rdb);
	return NULL;
}
