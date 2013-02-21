/* -*- Mode: c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-

    libparted - a library for manipulating disk partitions
    disk_amiga.c - libparted module to manipulate amiga RDB partition tables.
    Copyright (C) 2000-2001, 2004, 2007-2012 Free Software Foundation, Inc.

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

#include "pt-tools.h"

#ifndef MAX
# define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "misc.h"

/* String manipulation */
static void _amiga_set_bstr (const char *cstr, char *bstr, int maxsize) {
	int size = strlen (cstr);
	int i;

	if (size >= maxsize) return;
	bstr[0] = size;
	for (i = 0; i<size; i++) bstr[i+1] = cstr[i];
}
static const char * _amiga_get_bstr (char * bstr) {
	char * cstr = bstr + 1;
	int size = bstr[0];

	cstr[size] = '\0';
	return cstr;
}

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

struct AmigaIds {
	uint32_t ID;
	struct AmigaIds *next;
};

static struct AmigaIds *
_amiga_add_id (uint32_t id, struct AmigaIds *ids) {
	struct AmigaIds *newid;

	if ((newid=ped_malloc(sizeof (struct AmigaIds)))==NULL)
		return 0;
	newid->ID = id;
	newid->next = ids;
	return newid;
}

static void
_amiga_free_ids (struct AmigaIds *ids) {
	struct AmigaIds *current, *next;

	for (current = ids; current != NULL; current = next) {
		next = current->next;
		free (current);
	}
}
static int _GL_ATTRIBUTE_PURE
_amiga_id_in_list (uint32_t id, struct AmigaIds *ids) {
	struct AmigaIds *current;

	for (current = ids; current != NULL; current = current->next) {
		if (id == current->ID)
			return 1;
	}
	return 0;
}

struct AmigaBlock {
	uint32_t	amiga_ID;		/* Identifier 32 bit word */
	uint32_t	amiga_SummedLongss;	/* Size of the structure for checksums */
	int32_t		amiga_ChkSum;		/* Checksum of the structure */
};
#define AMIGA(pos) ((struct AmigaBlock *)(pos))

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
_amiga_read_block (const PedDevice *dev, struct AmigaBlock *blk,
                   PedSector block, struct AmigaIds *ids)
{
	if (!ped_device_read (dev, blk, block, 1))
		return NULL;
	if (ids && !_amiga_id_in_list(PED_BE32_TO_CPU(blk->amiga_ID), ids))
		return NULL;
	if (_amiga_checksum (blk) != 0) {
		switch (ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_FIX | PED_EXCEPTION_IGNORE | PED_EXCEPTION_CANCEL,
			_("%s : Bad checksum on block %llu of type %s."),
			__func__, block, _amiga_block_id(PED_BE32_TO_CPU(blk->amiga_ID))))
		{
			case PED_EXCEPTION_CANCEL :
				return NULL;
			case PED_EXCEPTION_FIX :
				_amiga_calculate_checksum(AMIGA(blk));
				if (!ped_device_write ((PedDevice*)dev, blk, block, 1))
					return NULL;
			case PED_EXCEPTION_IGNORE :
			case PED_EXCEPTION_UNHANDLED :
			default :
				return blk;
		}
	}
	return blk;
}

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

#define RDSK(pos) ((struct RigidDiskBlock *)(pos))

#define AMIGA_RDB_NOT_FOUND ((uint32_t)0xffffffff)
#define	RDB_LOCATION_LIMIT	16
#define AMIGA_MAX_PARTITIONS 128
#define MAX_RDB_BLOCK (RDB_LOCATION_LIMIT + 2 * AMIGA_MAX_PARTITIONS + 2)

static uint32_t
_amiga_find_rdb (const PedDevice *dev, struct RigidDiskBlock *rdb) {
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

struct PartitionBlock {
    uint32_t	pb_ID;			/* Identifier 32 bit word : 'PART' */
    uint32_t	pb_SummedLongs;		/* Size of the structure for checksums */
    int32_t	pb_ChkSum;		/* Checksum of the structure */
    uint32_t	pb_HostID;		/* SCSI Target ID of host, not really used */
    uint32_t	pb_Next;		/* Block number of the next PartitionBlock */
    uint32_t	pb_Flags;		/* Part Flags (NOMOUNT and BOOTABLE) */
    uint32_t	pb_Reserved1[2];
    uint32_t	pb_DevFlags;		/* Preferred flags for OpenDevice */
    char	pb_DriveName[32];	/* Preferred DOS device name: BSTR form */
    uint32_t	pb_Reserved2[15];
    uint32_t	de_TableSize;		/* Size of Environment vector */
	/* Size of the blocks in 32 bit words, usually 128 */
    uint32_t	de_SizeBlock;
    uint32_t	de_SecOrg;	     	/* Not used; must be 0 */
    uint32_t	de_Surfaces;		/* Number of heads (surfaces) */
	/* Disk sectors per block, used with SizeBlock, usually 1 */
    uint32_t	de_SectorPerBlock;
    uint32_t	de_BlocksPerTrack;	/* Blocks per track. drive specific */
    uint32_t	de_Reserved;		/* DOS reserved blocks at start of partition. */
    uint32_t	de_PreAlloc;		/* DOS reserved blocks at end of partition */
    uint32_t	de_Interleave;		/* Not used, usually 0 */
    uint32_t	de_LowCyl;		/* First cylinder of the partition */
    uint32_t	de_HighCyl;		/* Last cylinder of the partition */
    uint32_t	de_NumBuffers;		/* Initial # DOS of buffers.  */
    uint32_t	de_BufMemType;		/* Type of mem to allocate for buffers */
    uint32_t	de_MaxTransfer;		/* Max number of bytes to transfer at a time */
    uint32_t	de_Mask;		/* Address Mask to block out certain memory */
    int32_t	de_BootPri;		/* Boot priority for autoboot */
    uint32_t	de_DosType;		/* Dostype of the file system */
    uint32_t	de_Baud;		/* Baud rate for serial handler */
    uint32_t	de_Control;		/* Control word for handler/filesystem */
    uint32_t	de_BootBlocks;		/* Number of blocks containing boot code */
    uint32_t	pb_EReserved[12];
};

#define PART(pos) ((struct PartitionBlock *)(pos))

#define	PBFB_BOOTABLE	0	/* this partition is intended to be bootable */
#define	PBFF_BOOTABLE	1L	/*   (expected directories and files exist) */
#define	PBFB_NOMOUNT	1	/* do not mount this partition (e.g. manually */
#define	PBFF_NOMOUNT	2L	/*   mounted, but space reserved here) */
#define	PBFB_RAID	2	/* this partition is intended to be part of */
#define	PBFF_RAID	4L	/*   a RAID array */
#define	PBFB_LVM	3	/* this partition is intended to be part of */
#define	PBFF_LVM	8L	/*   a LVM volume group */


struct LinkedBlock {
    uint32_t	lk_ID;			/* Identifier 32 bit word */
    uint32_t	lk_SummedLongs;		/* Size of the structure for checksums */
    int32_t	lk_ChkSum;		/* Checksum of the structure */
    uint32_t	pb_HostID;		/* SCSI Target ID of host, not really used */
    uint32_t	lk_Next;		/* Block number of the next PartitionBlock */
};
struct Linked2Block {
    uint32_t	lk2_ID;			/* Identifier 32 bit word */
    uint32_t	lk2_SummedLongs;		/* Size of the structure for checksums */
    int32_t	lk2_ChkSum;		/* Checksum of the structure */
    uint32_t	lk2_HostID;		/* SCSI Target ID of host, not really used */
    uint32_t	lk2_Next;		/* Block number of the next PartitionBlock */
    uint32_t	lk2_Reverved[13];
    uint32_t	lk2_Linked;		/* Secondary linked list */
};
#define LINK_END	(uint32_t)0xffffffff
#define LNK(pos)	((struct LinkedBlock *)(pos))
#define LNK2(pos)	((struct Linked2Block *)(pos))


static PedDiskType amiga_disk_type;

static int
amiga_probe (const PedDevice *dev)
{
	struct RigidDiskBlock *rdb;
	uint32_t found;
	PED_ASSERT(dev != NULL);

	if ((rdb=RDSK(ped_malloc(dev->sector_size)))==NULL)
		return 0;
	found = _amiga_find_rdb (dev, rdb);
	free (rdb);

	return (found == AMIGA_RDB_NOT_FOUND ? 0 : 1);
}

static PedDisk*
amiga_alloc (const PedDevice* dev)
{
	PedDisk *disk;
	struct RigidDiskBlock *rdb;
	PedSector cyl_size;
	int highest_cylinder, highest_block;

	PED_ASSERT(dev != NULL);
	cyl_size = dev->hw_geom.sectors * dev->hw_geom.heads;

	if (!(disk = _ped_disk_alloc (dev, &amiga_disk_type)))
		return NULL;

	if (!(disk->disk_specific = ped_malloc (disk->dev->sector_size))) {
		free (disk);
		return NULL;
	}
	rdb = disk->disk_specific;

        /* Upon failed assertion this does leak.  That's fine, because
           if the assertion fails, you have bigger problems than this leak. */
        PED_ASSERT(sizeof(*rdb) <= disk->dev->sector_size);

	memset(rdb, 0, disk->dev->sector_size);

	rdb->rdb_ID = PED_CPU_TO_BE32 (IDNAME_RIGIDDISK);
	rdb->rdb_SummedLongs = PED_CPU_TO_BE32 (64);
	rdb->rdb_HostID = PED_CPU_TO_BE32 (0);
	rdb->rdb_BlockBytes = PED_CPU_TO_BE32 (disk->dev->sector_size);
	rdb->rdb_Flags = PED_CPU_TO_BE32 (0);

	/* Block lists */
	rdb->rdb_BadBlockList = PED_CPU_TO_BE32 (LINK_END);
	rdb->rdb_PartitionList = PED_CPU_TO_BE32 (LINK_END);
	rdb->rdb_FileSysHeaderList = PED_CPU_TO_BE32 (LINK_END);
	rdb->rdb_DriveInit = PED_CPU_TO_BE32 (LINK_END);
	rdb->rdb_BootBlockList = PED_CPU_TO_BE32 (LINK_END);

	/* Physical drive characteristics */
	rdb->rdb_Cylinders = PED_CPU_TO_BE32 (dev->hw_geom.cylinders);
	rdb->rdb_Sectors = PED_CPU_TO_BE32 (dev->hw_geom.sectors);
	rdb->rdb_Heads = PED_CPU_TO_BE32 (dev->hw_geom.heads);
	rdb->rdb_Interleave = PED_CPU_TO_BE32 (0);
	rdb->rdb_Park = PED_CPU_TO_BE32 (dev->hw_geom.cylinders);
	rdb->rdb_WritePreComp = PED_CPU_TO_BE32 (dev->hw_geom.cylinders);
	rdb->rdb_ReducedWrite = PED_CPU_TO_BE32 (dev->hw_geom.cylinders);
	rdb->rdb_StepRate = PED_CPU_TO_BE32 (0);

	highest_cylinder = 1 + MAX_RDB_BLOCK / cyl_size;
	highest_block = highest_cylinder * cyl_size - 1;

	/* Logical driver characteristics */
	rdb->rdb_RDBBlocksLo = PED_CPU_TO_BE32 (0);
	rdb->rdb_RDBBlocksHi = PED_CPU_TO_BE32 (highest_block);
	rdb->rdb_LoCylinder = PED_CPU_TO_BE32 (highest_cylinder);
	rdb->rdb_HiCylinder = PED_CPU_TO_BE32 (dev->hw_geom.cylinders -1);
	rdb->rdb_CylBlocks = PED_CPU_TO_BE32 (cyl_size);
	rdb->rdb_AutoParkSeconds = PED_CPU_TO_BE32 (0);
	/* rdb_HighRDSKBlock will only be set when writing */
	rdb->rdb_HighRDSKBlock = PED_CPU_TO_BE32 (0);

	/* Driver identification */
	_amiga_set_bstr("", rdb->rdb_DiskVendor, 8);
	_amiga_set_bstr(dev->model, rdb->rdb_DiskProduct, 16);
	_amiga_set_bstr("", rdb->rdb_DiskRevision, 4);
	_amiga_set_bstr("", rdb->rdb_ControllerVendor, 8);
	_amiga_set_bstr("", rdb->rdb_ControllerProduct, 16);
	_amiga_set_bstr("", rdb->rdb_ControllerRevision, 4);

	/* And calculate the checksum */
	_amiga_calculate_checksum ((struct AmigaBlock *) rdb);

	return disk;
}

static PedDisk*
amiga_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;
	struct RigidDiskBlock *	new_rdb;
	struct RigidDiskBlock * old_rdb;
	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->dev != NULL);
	PED_ASSERT(disk->disk_specific != NULL);

	old_rdb = (struct RigidDiskBlock *) disk->disk_specific;

	if (!(new_disk = ped_disk_new_fresh (disk->dev, &amiga_disk_type)))
		return NULL;

	new_rdb = (struct RigidDiskBlock *) new_disk->disk_specific;
	memcpy (new_rdb, old_rdb, 256);
	return new_disk;
}

static void
amiga_free (PedDisk* disk)
{
	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->disk_specific != NULL);

	free (disk->disk_specific);
	_ped_disk_free (disk);
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
static int
amiga_read (PedDisk* disk)
{
	struct RigidDiskBlock *rdb;
	struct PartitionBlock *partition;
	uint32_t partblock;
	uint32_t partlist[AMIGA_MAX_PARTITIONS];
	PedSector cylblocks;
	int i;

	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->dev != NULL);
	PED_ASSERT(disk->dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);
	PED_ASSERT(disk->disk_specific != NULL);
	rdb = RDSK(disk->disk_specific);

	if (_amiga_find_rdb (disk->dev, rdb) == AMIGA_RDB_NOT_FOUND) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s : Didn't find rdb block, should never happen."), __func__);
		return 0;
	}

	/* Let's copy the rdb read geometry to the dev */
	/* FIXME: should this go into disk->dev->bios_geom instead? */
	disk->dev->hw_geom.cylinders = PED_BE32_TO_CPU (rdb->rdb_Cylinders);
	disk->dev->hw_geom.heads = PED_BE32_TO_CPU (rdb->rdb_Heads);
	disk->dev->hw_geom.sectors = PED_BE32_TO_CPU (rdb->rdb_Sectors);
	cylblocks = (PedSector) PED_BE32_TO_CPU (rdb->rdb_Heads) *
		(PedSector) PED_BE32_TO_CPU (rdb->rdb_Sectors);

	/* Remove all partitions in the former in memory table */
	ped_disk_delete_all (disk);

	/* Let's allocate a partition block */
	if (!(partition = ped_malloc (disk->dev->sector_size)))
		return 0;

	/* We initialize the hardblock free list to detect loops */
	for (i = 0; i < AMIGA_MAX_PARTITIONS; i++) partlist[i] = LINK_END;

	for (i = 1, partblock = PED_BE32_TO_CPU(rdb->rdb_PartitionList);
		i < AMIGA_MAX_PARTITIONS && partblock != LINK_END;
		i++, partblock = PED_BE32_TO_CPU(partition->pb_Next))
	{
		PedPartition *part;
		PedSector start, end;

		/* Let's look for loops in the partition table */
		if (_amiga_loop_check(partblock, partlist, i)) {
			break;
		}

		/* Let's allocate and read a partition block to get its geometry*/
		if (!_amiga_read_block (disk->dev, AMIGA(partition),
		                        (PedSector)partblock, NULL)) {
			free(partition);
			return 0;
		}

		start = ((PedSector) PED_BE32_TO_CPU (partition->de_LowCyl))
			* cylblocks;
		end = (((PedSector) PED_BE32_TO_CPU (partition->de_HighCyl))
			+ 1) * cylblocks - 1;

		/* We can now construct a new partition */
		if (!(part = ped_partition_new (disk, PED_PARTITION_NORMAL,
                                                NULL, start, end))) {
			free(partition);
			return 0;
		}
		/* And copy over the partition block */
		memcpy(part->disk_specific, partition, 256);

		part->num = i;
		part->type = 0;
		/* Let's probe what file system is present on the disk */
		part->fs_type = ped_file_system_probe (&part->geom);

		PedConstraint *constraint_exact
			= ped_constraint_exact (&part->geom);
		if (constraint_exact == NULL)
			return 0;
		bool ok = ped_disk_add_partition (disk, part, constraint_exact);
		ped_constraint_destroy (constraint_exact);
		if (!ok) {
			ped_partition_destroy(part);
			free(partition);
			return 0;
		}
	}
	free(partition);
	return 1;
}

static int
_amiga_find_free_blocks(const PedDisk *disk, uint32_t *table,
	struct LinkedBlock *block, uint32_t first, uint32_t type)
{
	PedSector next;

	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->dev != NULL);

	for (next = first; next != LINK_END; next = PED_BE32_TO_CPU(block->lk_Next)) {
		if (table[next] != IDNAME_FREE) {
			switch (ped_exception_throw(PED_EXCEPTION_ERROR,
				PED_EXCEPTION_FIX | PED_EXCEPTION_IGNORE | PED_EXCEPTION_CANCEL,
				_("%s : Loop detected at block %d."), __func__, next))
			{
				case PED_EXCEPTION_CANCEL :
					return 0;
				case PED_EXCEPTION_FIX :
					/* TODO : Need to add fixing code */
				case PED_EXCEPTION_IGNORE :
				case PED_EXCEPTION_UNHANDLED :
				default :
					return 1;
			}
		}

		if (!_amiga_read_block (disk->dev, AMIGA(block), next, NULL)) {
			return 0;
		}
		if (PED_BE32_TO_CPU(block->lk_ID) != type) {
			switch (ped_exception_throw(PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("%s : The %s list seems bad at block %s."),
				__func__, _amiga_block_id(PED_BE32_TO_CPU(block->lk_ID)), next))
			{
				/* TODO : to more subtile things here */
				case PED_EXCEPTION_CANCEL :
				case PED_EXCEPTION_UNHANDLED :
				default :
					return 0;
			}
		}
		table[next] = type;
		if (PED_BE32_TO_CPU(block->lk_ID) == IDNAME_FILESYSHEADER) {
			if (_amiga_find_free_blocks(disk, table, block,
				PED_BE32_TO_CPU(LNK2(block)->lk2_Linked),
				IDNAME_LOADSEG) == 0) return 0;
		}
	}
	return 1;
}
static uint32_t _GL_ATTRIBUTE_PURE
_amiga_next_free_block(uint32_t *table, uint32_t start, uint32_t type) {
	int i;

	for (i = start; table[i] != type && table[i] != IDNAME_FREE; i++);
	return i;
}
static PedPartition * _GL_ATTRIBUTE_PURE
_amiga_next_real_partition(const PedDisk *disk, PedPartition *part) {
	PedPartition *next;

	for (next = ped_disk_next_partition (disk, part);
		next != NULL && !ped_partition_is_active (next);
		next = ped_disk_next_partition (disk, next));
	return next;
}
#ifndef DISCOVER_ONLY
static int
amiga_write (const PedDisk* disk)
{
	struct RigidDiskBlock *rdb;
	struct LinkedBlock *block;
	struct PartitionBlock *partition;
	PedPartition *part, *next_part;
	PedSector cylblocks, first_hb, last_hb;
	uint32_t * table;
	uint32_t i;
	uint32_t rdb_num, part_num, block_num, next_num;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);
	PED_ASSERT (disk->disk_specific != NULL);

	if (!(rdb = ped_malloc (disk->dev->sector_size)))
		return 0;

	/* Let's read the rdb */
	if ((rdb_num = _amiga_find_rdb (disk->dev, rdb)) == AMIGA_RDB_NOT_FOUND) {
		rdb_num = 2;
		size_t pb_size = sizeof (struct PartitionBlock);
                /* Initialize only the part that won't be copied over
                   with a partition block in amiga_read.  */
		memset ((char *)(RDSK(disk->disk_specific)) + pb_size,
			0, PED_SECTOR_SIZE_DEFAULT - pb_size);
	} else {
		memcpy (RDSK(disk->disk_specific), rdb, disk->dev->sector_size);
	}
	free (rdb);
	rdb = RDSK(disk->disk_specific);

	cylblocks = (PedSector) PED_BE32_TO_CPU (rdb->rdb_Heads) *
		(PedSector) PED_BE32_TO_CPU (rdb->rdb_Sectors);
	first_hb = (PedSector) PED_BE32_TO_CPU (rdb->rdb_RDBBlocksLo);
	last_hb = (PedSector) PED_BE32_TO_CPU (rdb->rdb_RDBBlocksHi);

	/* Allocate a free block table and initialize it.
	   There must be room for at least RDB_NUM + 2 entries, since
	   the first RDB_NUM+1 entries get IDNAME_RIGIDDISK, and the
	   following one must have LINK_END to serve as sentinel.  */
	size_t tab_size = 2 + MAX (last_hb - first_hb, rdb_num);
	if (!(table = ped_malloc (tab_size * sizeof *table)))
		return 0;

	for (i = 0; i <= rdb_num; i++)
		table[i] = IDNAME_RIGIDDISK;
	for (     ; i < tab_size; i++)
		table[i] = LINK_END;

	/* Let's allocate a partition block */
	if (!(block = ped_malloc (disk->dev->sector_size))) {
		free (table);
		return 0;
	}

	/* And fill the free block table */
	if (_amiga_find_free_blocks(disk, table, block,
		PED_BE32_TO_CPU (rdb->rdb_BadBlockList), IDNAME_BADBLOCK) == 0)
	{
		ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Failed to list bad blocks."), __func__);
		goto error_free_table;
	}
	if (_amiga_find_free_blocks(disk, table, block,
		PED_BE32_TO_CPU (rdb->rdb_PartitionList), IDNAME_PARTITION) == 0)
	{
		ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Failed to list partition blocks."), __func__);
		goto error_free_table;
	}
	if (_amiga_find_free_blocks(disk, table, block,
		PED_BE32_TO_CPU (rdb->rdb_FileSysHeaderList), IDNAME_FILESYSHEADER) == 0)
	{
		ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Failed to list file system blocks."), __func__);
		goto error_free_table;
	}
	if (_amiga_find_free_blocks(disk, table, block,
		PED_BE32_TO_CPU (rdb->rdb_BootBlockList), IDNAME_BOOT) == 0)
	{
		ped_exception_throw(PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s : Failed to list boot blocks."), __func__);
		goto error_free_table;
	}

	block_num = part_num = _amiga_next_free_block(table, rdb_num+1,
                                                      IDNAME_PARTITION);
	part = _amiga_next_real_partition(disk, NULL);
	rdb->rdb_PartitionList = PED_CPU_TO_BE32(part ? part_num : LINK_END);
	for (; part != NULL; part = next_part, block_num = next_num) {
		PED_ASSERT(part->disk_specific != NULL);
		PED_ASSERT(part->geom.start % cylblocks == 0);
		PED_ASSERT((part->geom.end + 1) % cylblocks == 0);

		next_part = _amiga_next_real_partition(disk, part);
		next_num = _amiga_next_free_block(table, block_num+1, IDNAME_PARTITION);

		partition = PART(part->disk_specific);
		if (next_part == NULL)
			partition->pb_Next = PED_CPU_TO_BE32(LINK_END);
		else
			partition->pb_Next = PED_CPU_TO_BE32(next_num);
		partition->de_LowCyl = PED_CPU_TO_BE32(part->geom.start/cylblocks);
		partition->de_HighCyl = PED_CPU_TO_BE32((part->geom.end+1)/cylblocks-1);
		_amiga_calculate_checksum(AMIGA(partition));
		if (!ped_device_write (disk->dev, (void*) partition, block_num, 1)) {
			ped_exception_throw(PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Failed to write partition block at %d."),
				block_num);
			goto error_free_table;
			/* WARNING : If we fail here, we stop everything,
			 * and the partition table is lost. A better
			 * solution should be found, using the second
			 * half of the hardblocks to not overwrite the
			 * old partition table. It becomes problematic
			 * if we use more than half of the hardblocks. */
		}
	}

	if (block_num > PED_BE32_TO_CPU (rdb->rdb_HighRDSKBlock))
		rdb->rdb_HighRDSKBlock = PED_CPU_TO_BE32(block_num);

	_amiga_calculate_checksum(AMIGA(rdb));
	if (!ped_device_write (disk->dev, (void*) disk->disk_specific, rdb_num, 1))
		goto error_free_table;

	free (table);
	free (block);
	return ped_device_sync (disk->dev);

error_free_table:
	free (table);
	free (block);
	return 0;
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
amiga_partition_new (const PedDisk* disk, PedPartitionType part_type,
		   const PedFileSystemType* fs_type,
		   PedSector start, PedSector end)
{
	PedPartition *part;
	PedDevice *dev;
	PedSector cyl;
	struct PartitionBlock *partition;
	struct RigidDiskBlock *rdb;

	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->dev != NULL);
	PED_ASSERT(disk->disk_specific != NULL);
	dev = disk->dev;
	cyl = (PedSector) (dev->hw_geom.sectors * dev->hw_geom.heads);
	rdb = RDSK(disk->disk_specific);

	if (!(part = _ped_partition_alloc (disk, part_type, fs_type, start, end)))
		return NULL;

	if (ped_partition_is_active (part)) {
		if (!(part->disk_specific = ped_malloc (disk->dev->sector_size))) {
			free (part);
			return NULL;
		}
		partition = PART(part->disk_specific);
		memset(partition, 0, sizeof(struct PartitionBlock));

		partition->pb_ID = PED_CPU_TO_BE32(IDNAME_PARTITION);
		partition->pb_SummedLongs = PED_CPU_TO_BE32(64);
		partition->pb_HostID = rdb->rdb_HostID;
		partition->pb_Flags = PED_CPU_TO_BE32(0);
		/* TODO : use a scheme including the device name and the
		 * partition number, if it is possible */
		_amiga_set_bstr("dhx", partition->pb_DriveName, 32);

		partition->de_TableSize = PED_CPU_TO_BE32(19);
		partition->de_SizeBlock = PED_CPU_TO_BE32(128);
		partition->de_SecOrg = PED_CPU_TO_BE32(0);
		partition->de_Surfaces = PED_CPU_TO_BE32(dev->hw_geom.heads);
		partition->de_SectorPerBlock = PED_CPU_TO_BE32(1);
		partition->de_BlocksPerTrack
			= PED_CPU_TO_BE32(dev->hw_geom.sectors);
		partition->de_Reserved = PED_CPU_TO_BE32(2);
		partition->de_PreAlloc = PED_CPU_TO_BE32(0);
		partition->de_Interleave = PED_CPU_TO_BE32(0);
		partition->de_LowCyl = PED_CPU_TO_BE32(start/cyl);
		partition->de_HighCyl = PED_CPU_TO_BE32((end+1)/cyl-1);
		partition->de_NumBuffers = PED_CPU_TO_BE32(30);
		partition->de_BufMemType = PED_CPU_TO_BE32(0);
		partition->de_MaxTransfer = PED_CPU_TO_BE32(0x7fffffff);
		partition->de_Mask = PED_CPU_TO_BE32(0xffffffff);
		partition->de_BootPri = PED_CPU_TO_BE32(0);
		partition->de_DosType = PED_CPU_TO_BE32(0x4c4e5800);
		partition->de_Baud = PED_CPU_TO_BE32(0);
		partition->de_Control = PED_CPU_TO_BE32(0);
		partition->de_BootBlocks = PED_CPU_TO_BE32(0);

	} else {
		part->disk_specific = NULL;
	}
	return part;
}

static PedPartition*
amiga_partition_duplicate (const PedPartition* part)
{
	PedPartition *new_part;
	struct PartitionBlock *new_amiga_part;
	struct PartitionBlock *old_amiga_part;

	PED_ASSERT(part != NULL);
	PED_ASSERT(part->disk != NULL);
	PED_ASSERT(part->disk_specific != NULL);
	old_amiga_part = (struct PartitionBlock *) part->disk_specific;

	new_part = ped_partition_new (part->disk, part->type,
				      part->fs_type, part->geom.start,
				      part->geom.end);
	if (!new_part)
		return NULL;

	new_amiga_part = (struct PartitionBlock *) new_part->disk_specific;
	memcpy (new_amiga_part, old_amiga_part, 256);

	return new_part;
}

static void
amiga_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	if (ped_partition_is_active (part)) {
		PED_ASSERT (part->disk_specific != NULL);
		free (part->disk_specific);
	}
	_ped_partition_free (part);
}

static int
amiga_partition_set_system (PedPartition* part,
                            const PedFileSystemType* fs_type)
{
	struct PartitionBlock *partition;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	partition = PART(part->disk_specific);

	part->fs_type = fs_type;

	if (!fs_type)
		partition->de_DosType = PED_CPU_TO_BE32(0x4c4e5800); /* 'LNX\0' */
	else if (!strcmp (fs_type->name, "ext2"))
		partition->de_DosType = PED_CPU_TO_BE32(0x4c4e5800); /* 'LNX\0' */
	else if (!strcmp (fs_type->name, "ext3"))
		partition->de_DosType = PED_CPU_TO_BE32(0x45585403); /* 'EXT\3' */
	else if (is_linux_swap (fs_type->name))
		partition->de_DosType = PED_CPU_TO_BE32(0x53575000); /* 'SWP\0' */
	else if (!strcmp (fs_type->name, "fat16"))
		partition->de_DosType = PED_CPU_TO_BE32(0x46415400); /* 'FAT\0' */
	else if (!strcmp (fs_type->name, "fat32"))
		partition->de_DosType = PED_CPU_TO_BE32(0x46415401); /* 'FAT\1'*/
	else if (!strcmp (fs_type->name, "hfs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x48465300); /* 'HFS\0' */
	else if (!strcmp (fs_type->name, "jfs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x4a465300); /* 'JFS\0' */
	else if (!strcmp (fs_type->name, "ntfs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x4e544653); /* 'NTFS' */
	else if (!strcmp (fs_type->name, "reiserfs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x52465300); /* 'RFS\0' */
	else if (!strcmp (fs_type->name, "sun-ufs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x53554653); /* 'SUFS' */
	else if (!strcmp (fs_type->name, "hp-ufs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x48554653); /* 'HUFS' */
	else if (!strcmp (fs_type->name, "xfs"))
		partition->de_DosType = PED_CPU_TO_BE32(0x58465300); /* 'XFS\0' */
	else
		partition->de_DosType = PED_CPU_TO_BE32(0x00000000); /* unknown */
	return 1;
}

static int
amiga_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	struct PartitionBlock *partition;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	partition = PART(part->disk_specific);

	switch (flag) {
		case PED_PARTITION_BOOT:
			if (state) partition->pb_Flags |= PED_CPU_TO_BE32(PBFF_BOOTABLE);
			else partition->pb_Flags &= ~(PED_CPU_TO_BE32(PBFF_BOOTABLE));
			return 1;
		case PED_PARTITION_HIDDEN:
			if (state) partition->pb_Flags |= PED_CPU_TO_BE32(PBFF_NOMOUNT);
			else partition->pb_Flags &= ~(PED_CPU_TO_BE32(PBFF_NOMOUNT));
			return 1;
		case PED_PARTITION_RAID:
			if (state) partition->pb_Flags |= PED_CPU_TO_BE32(PBFF_RAID);
			else partition->pb_Flags &= ~(PED_CPU_TO_BE32(PBFF_RAID));
			return 1;
		case PED_PARTITION_LVM:
			if (state) partition->pb_Flags |= PED_CPU_TO_BE32(PBFF_LVM);
			else partition->pb_Flags &= ~(PED_CPU_TO_BE32(PBFF_LVM));
			return 1;
		default:
			return 0;
	}
}

static int _GL_ATTRIBUTE_PURE
amiga_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	struct PartitionBlock *partition;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	partition = PART(part->disk_specific);

	switch (flag) {
		case PED_PARTITION_BOOT:
			return (partition->pb_Flags & PED_CPU_TO_BE32(PBFF_BOOTABLE));
		case PED_PARTITION_HIDDEN:
			return (partition->pb_Flags & PED_CPU_TO_BE32(PBFF_NOMOUNT));
		case PED_PARTITION_RAID:
			return (partition->pb_Flags & PED_CPU_TO_BE32(PBFF_RAID));
		case PED_PARTITION_LVM:
			return (partition->pb_Flags & PED_CPU_TO_BE32(PBFF_LVM));
		default:
			return 0;
	}
}

static int
amiga_partition_is_flag_available (const PedPartition* part,
				 PedPartitionFlag flag)
{
	switch (flag) {
	case PED_PARTITION_BOOT:
	case PED_PARTITION_HIDDEN:
	case PED_PARTITION_RAID:
	case PED_PARTITION_LVM:
		return 1;
	default:
		return 0;
	}
}

static void
amiga_partition_set_name (PedPartition* part, const char* name)
{
	struct PartitionBlock *partition;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	partition = PART(part->disk_specific);
	_amiga_set_bstr(name, partition->pb_DriveName, 32);
}
static const char*
amiga_partition_get_name (const PedPartition* part)
{
	struct PartitionBlock *partition;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	partition = PART(part->disk_specific);

	return _amiga_get_bstr(partition->pb_DriveName);
}

static PedAlignment*
amiga_get_partition_alignment(const PedDisk *disk)
{
	PedSector cylinder_size =
		disk->dev->hw_geom.sectors * disk->dev->hw_geom.heads;

        return ped_alignment_new(0, cylinder_size);
}

static PedConstraint*
_amiga_get_constraint (const PedDisk *disk)
{
	PedDevice *dev = disk->dev;
	PedAlignment start_align, end_align;
	PedGeometry max_geom;
	PedSector cyl_size = dev->hw_geom.sectors * dev->hw_geom.heads;

	if (!ped_alignment_init(&start_align, 0, cyl_size))
		return NULL;
	if (!ped_alignment_init(&end_align, -1, cyl_size))
		return NULL;
	if (!ped_geometry_init(&max_geom, dev, MAX_RDB_BLOCK + 1,
			       dev->length - MAX_RDB_BLOCK - 1))
		return NULL;

	return ped_constraint_new (&start_align, &end_align,
		&max_geom, &max_geom, 1, dev->length);
}

static int
amiga_partition_align (PedPartition* part, const PedConstraint* constraint)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);

	if (_ped_partition_attempt_align (part, constraint,
					  _amiga_get_constraint (part->disk)))
	       	return 1;

#ifndef DISCOVER_ONLY
	ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
		_("Unable to satisfy all constraints on the partition."));
#endif
	return 0;
}

static int
amiga_partition_enumerate (PedPartition* part)
{
	int i;
	PedPartition* p;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);

	/* never change the partition numbers */
	if (part->num != -1)
		return 1;
	for (i = 1; i <= AMIGA_MAX_PARTITIONS; i++) {
		p = ped_disk_get_partition (part->disk, i);
		if (!p) {
			part->num = i;
			return 1;
		}
	}

	/* failed to allocate a number */
#ifndef DISCOVER_ONLY
	ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
		_("Unable to allocate a partition number."));
#endif
	return 0;
}

static int
amiga_alloc_metadata (PedDisk* disk)
{
	PedPartition*		new_part;
	PedConstraint*		constraint_any = NULL;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	constraint_any = ped_constraint_any (disk->dev);

	/* Allocate space for the RDB */
	new_part = ped_partition_new (disk, PED_PARTITION_METADATA, NULL,
	                              0, MAX_RDB_BLOCK);
	if (!new_part)
		goto error;

	if (!ped_disk_add_partition (disk, new_part, constraint_any)) {
		ped_partition_destroy (new_part);
		goto error;
	}

	ped_constraint_destroy (constraint_any);
	return 1;
error:
	ped_constraint_destroy (constraint_any);
	return 0;
}

static int
amiga_get_max_primary_partition_count (const PedDisk* disk)
{
	return AMIGA_MAX_PARTITIONS;
}

static bool
amiga_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = AMIGA_MAX_PARTITIONS;
	return true;
}

#include "pt-common.h"
PT_define_limit_functions (amiga)

static PedDiskOps amiga_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (amiga_write),

	partition_set_name:	amiga_partition_set_name,
	partition_get_name:	amiga_partition_get_name,

	get_partition_alignment: amiga_get_partition_alignment,

	PT_op_function_initializers (amiga)
};

static PedDiskType amiga_disk_type = {
	next:		NULL,
	name:		"amiga",
	ops:		&amiga_disk_ops,
	features:	PED_DISK_TYPE_PARTITION_NAME
};

void
ped_disk_amiga_init ()
{
	PED_ASSERT (sizeof (struct AmigaBlock) != 3);
	PED_ASSERT (sizeof (struct RigidDiskBlock) != 64);
	PED_ASSERT (sizeof (struct PartitionBlock) != 64);
	PED_ASSERT (sizeof (struct LinkedBlock) != 5);
	PED_ASSERT (sizeof (struct Linked2Block) != 18);

	ped_disk_type_register (&amiga_disk_type);
}

void
ped_disk_amiga_done ()
{
	ped_disk_type_unregister (&amiga_disk_type);
}
