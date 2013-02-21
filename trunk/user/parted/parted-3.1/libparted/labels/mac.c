/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2000, 2002, 2004, 2007-2012 Free Software Foundation, Inc.

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

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>
#include <stdbool.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "misc.h"
#include "pt-tools.h"

/* struct's hacked from Linux source:  fs/partitions/mac.h
 * I believe it was originally written by Paul Mackerras (from comments in
 * Quik source)
 *
 * See also:
 *	http://developer.apple.com/documentation/mac/Devices/Devices-126.html
 *	http://developer.apple.com/documentation/mac/Devices/Devices-121.html
 *	http://devworld.apple.com/technotes/tn/tn1189.html
 *
 * Partition types:
 *	Apple_Bootstrap		new-world (HFS) boot partition
 *	Apple_partition_map	partition map (table)
 *	Apple_Driver		device driver
 *	Apple_Driver43		SCSI Manager 4.3 device driver
 *	Apple_MFS		original Macintosh File System
 *	Apple_HFS		Hierarchical File System (and +)
 *	Apple_HFSX		HFS+ with case sensitivity and more
 *	Apple_UNIX_SVR2		UNIX file system (UFS?)
 *	Apple_PRODOS		ProDOS file system
 *	Apple_Free		unused space
 *	Apple_Scratch		empty
 *	Apple_Void		padding for iso9660
 *	Apple_Extra		an unused partition map entry
 *
 * Quick explanation:
 * ------------------
 * Terminology:
 *
 * 	Parted			Apple
 * 	------			-----
 * 	device			disk/device
 * 	disk			no equivalent.
 * 	partition		volume or partition
 * 	sector			block
 *
 * 	* All space must be accounted for, except block 0 (driver block) and
 * 	block 1-X (the partition map: i.e. lots of MacRawPartitions)
 *
 * 	* It's really hard to grow/shrink the number of MacRawPartition
 * 	entries in the partition map, because the first partition starts
 * 	immediately after the partition map.  When we can move the start of
 * 	HFS and ext2 partitions, this problem will disappear ;-)
 */

#define MAC_PARTITION_MAGIC_1	0x5453		/* old */
#define MAC_PARTITION_MAGIC_2	0x504d
#define MAC_DISK_MAGIC		0x4552

#define MAC_STATUS_BOOTABLE     8       /* partition is bootable */

typedef struct _MacRawPartition     MacRawPartition;
typedef struct _MacRawDisk          MacRawDisk;
typedef struct _MacDeviceDriver     MacDeviceDriver;
typedef struct _MacPartitionData    MacPartitionData;
typedef struct _MacDiskData         MacDiskData;

struct __attribute__ ((packed)) _MacRawPartition {
	uint16_t	signature;      /* expected to be MAC_PARTITION_MAGIC */
	uint16_t	res1;
	uint32_t	map_count;      /* # blocks in partition map */
	uint32_t	start_block;    /* absolute starting block # of partition */
	uint32_t	block_count;    /* number of blocks in partition */
	char		name[32];       /* partition name */
	char		type[32];       /* string type description */
	uint32_t	data_start;     /* rel block # of first data block */
	uint32_t	data_count;     /* number of data blocks */
	uint32_t	status;         /* partition status bits */
	uint32_t	boot_start;
	uint32_t	boot_count;
	uint32_t	boot_load;
	uint32_t	boot_load2;
	uint32_t	boot_entry;
	uint32_t	boot_entry2;
	uint32_t	boot_cksum;
	char		processor[16];  /* Contains 680x0, x=0,2,3,4; or empty */
	uint32_t	driver_sig;
	char		_padding[372];
};

/* Driver descriptor structure, in block 0 */
struct __attribute__ ((packed)) _MacRawDisk {
	uint16_t	signature;      /* expected to be MAC_DRIVER_MAGIC */
	uint16_t	block_size;	/* physical sector size */
	uint32_t	block_count;	/* size of device in blocks */
	uint16_t	dev_type;	/* reserved */
	uint16_t	dev_id;		/* reserved */
	uint32_t	data;		/* reserved */
	uint16_t	driver_count;	/* # of driver descriptor entries */
	uint8_t		driverlist[488];/* info about available drivers */
	uint16_t	padding[3];	/* pad to 512 bytes */
};

struct __attribute__ ((packed)) _MacDeviceDriver {
	uint32_t	block;		/* startblock in MacRawDisk->block_size units */
	uint16_t	size;		/* size in 512 byte units */
	uint16_t	type;		/* operating system type (MacOS = 1) */
};

struct _MacPartitionData {
	char		volume_name[33];	/* eg: "Games" */
	char		system_name[33];	/* eg: "Apple_Unix_SVR2" */
	char		processor_name[17];

	int		is_boot;
	int		is_driver;
	int		has_driver;
	int		is_root;
	int		is_swap;
	int		is_lvm;
	int		is_raid;

	PedSector	data_region_length;
	PedSector	boot_region_length;

	uint32_t	boot_base_address;
	uint32_t	boot_entry_address;
	uint32_t	boot_checksum;

	uint32_t	status;
	uint32_t	driver_sig;
};

struct _MacDiskData {
	int		ghost_size;		/* sectors per "driver" block */
	int		part_map_entry_count;	/* # entries (incl. ghost) */
	int		part_map_entry_num;	/* partition map location */

	int		active_part_entry_count;	/* # real partitions */
	int		free_part_entry_count;		/* # free space */
	int		last_part_entry_num;		/* last entry number */

	uint16_t	block_size;		/* physical sector size */
	uint16_t	driver_count;
	MacDeviceDriver	driverlist[1 + 60];	/* 488 bytes */
};

static PedDiskType mac_disk_type;

static int
_check_signature (MacRawDisk const *raw_disk)
{
	if (PED_BE16_TO_CPU (raw_disk->signature) != MAC_DISK_MAGIC) {
#ifdef DISCOVER_ONLY
		return 0;
#else
		return ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Invalid signature %x for Mac disk labels."),
			(int) PED_BE16_TO_CPU (raw_disk->signature))
			== PED_EXCEPTION_IGNORE;
#endif
	}

	return 1;
}

static int
_rawpart_check_signature (MacRawPartition* raw_part)
{
	int	sig = (int) PED_BE16_TO_CPU (raw_part->signature);
	return sig == MAC_PARTITION_MAGIC_1 || sig == MAC_PARTITION_MAGIC_2;
}

static int
mac_probe (const PedDevice * dev)
{
	PED_ASSERT (dev != NULL);

        if (dev->sector_size < sizeof (MacRawDisk))
                return 0;

	void *label;
	if (!ptt_read_sector (dev, 0, &label))
		return 0;

	int valid = _check_signature (label);

	free (label);
	return valid;
}

static int
_disk_add_part_map_entry (PedDisk* disk, int warn)
{
	MacDiskData*		mac_disk_data = disk->disk_specific;
	PedPartition*		new_part;
	MacPartitionData*	mac_part_data;
	PedSector		part_map_size;
	PedConstraint*		constraint_any = ped_constraint_any (disk->dev);

#ifndef DISCOVER_ONLY
	if (warn && ped_exception_throw (
		PED_EXCEPTION_ERROR,
		PED_EXCEPTION_FIX | PED_EXCEPTION_CANCEL,
		_("Partition map has no partition map entry!"))
			!= PED_EXCEPTION_FIX)
		goto error;
#endif /* !DISCOVER_ONLY */

	part_map_size
		= ped_round_up_to (mac_disk_data->last_part_entry_num, 64);
	if (part_map_size == 0)
		part_map_size = 64;

	new_part = ped_partition_new (disk, PED_PARTITION_NORMAL, NULL,
                                      1, part_map_size - 1);
	if (!new_part)
		goto error;

	mac_part_data = new_part->disk_specific;
	strcpy (mac_part_data->volume_name, "Apple");
	strcpy (mac_part_data->system_name, "Apple_partition_map");

	if (!ped_disk_add_partition (disk, new_part, constraint_any))
		goto error_destroy_new_part;

	mac_disk_data->part_map_entry_num = new_part->num;
	mac_disk_data->part_map_entry_count
		= new_part->geom.end - mac_disk_data->ghost_size;
	ped_constraint_destroy (constraint_any);
	return 1;

error_destroy_new_part:
	ped_partition_destroy (new_part);
error:
	ped_constraint_destroy (constraint_any);
	return 0;
}

static PedDisk*
mac_alloc (const PedDevice* dev)
{
	PedDisk*		disk;
	MacDiskData*		mac_disk_data;

	PED_ASSERT (dev != NULL);

#ifndef DISCOVER_ONLY
	if (dev->length < 256) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s is too small for a Mac disk label!"),
			dev->path);
		goto error;
	}
#endif

	disk = _ped_disk_alloc (dev, &mac_disk_type);
	if (!disk)
		goto error;

	mac_disk_data = (MacDiskData*) ped_malloc (sizeof (MacDiskData));
	if (!mac_disk_data)
		goto error_free_disk;
	disk->disk_specific = mac_disk_data;
	mac_disk_data->ghost_size = disk->dev->sector_size / 512;
	mac_disk_data->active_part_entry_count = 0;
	mac_disk_data->free_part_entry_count = 1;
	mac_disk_data->last_part_entry_num = 1;
	mac_disk_data->block_size = 0;
	mac_disk_data->driver_count = 0;
	memset(&mac_disk_data->driverlist[0], 0, sizeof(mac_disk_data->driverlist));

	if (!_disk_add_part_map_entry (disk, 0))
		goto error_free_disk;
	return disk;

error_free_disk:
	_ped_disk_free (disk);
error:
	return NULL;
}

static PedDisk*
mac_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;
	MacDiskData*	new_mac_data;
	MacDiskData*	old_mac_data = (MacDiskData*) disk->disk_specific;

	new_disk = ped_disk_new_fresh (disk->dev, &mac_disk_type);
	if (!new_disk)
		return NULL;

	new_mac_data = (MacDiskData*) new_disk->disk_specific;

	/* remove the partition map partition - it will be duplicated
	 * later.
	 */
	PedSector first_part_map_sector = old_mac_data->ghost_size;
	PedPartition *partition_map
	  = ped_disk_get_partition_by_sector (new_disk, first_part_map_sector);
	PED_ASSERT (partition_map != NULL);

	/* ped_disk_remove_partition may be used only to delete a "normal"
	   partition.  Trying to delete at least "freespace" or "metadata"
	   partitions leads to a violation of assumptions in
	   ped_disk_remove_partition, since it calls _disk_push_update_mode,
	   which destroys all "freespace" and "metadata" partitions, and
	   depends on that destruction not freeing its PART parameter.  */
	PED_ASSERT (partition_map->type == PED_PARTITION_NORMAL);
	ped_disk_remove_partition (new_disk, partition_map);

	/* ugly, but C is ugly :p */
	memcpy (new_mac_data, old_mac_data, sizeof (MacDiskData));
	return new_disk;
}

static void
mac_free (PedDisk* disk)
{
	MacDiskData*	mac_disk_data = disk->disk_specific;

	_ped_disk_free (disk);
	free (mac_disk_data);
}

static int
_rawpart_cmp_type (const MacRawPartition* raw_part, const char* type)
{
	return strncasecmp (raw_part->type, type, 32) == 0;
}

static int
_rawpart_cmp_name (const MacRawPartition* raw_part, const char* name)
{
	return strncasecmp (raw_part->name, name, 32) == 0;
}

static int
_rawpart_is_partition_map (const MacRawPartition* raw_part)
{
	return _rawpart_cmp_type (raw_part, "Apple_partition_map");
}

static int
strncasestr (const char* haystack, const char* needle, int n)
{
	int	needle_size = strlen (needle);
	int	i;

	for (i = 0; haystack[i] && i < n - needle_size; i++) {
		if (strncasecmp (haystack + i, needle, needle_size) == 0)
			return 1;
	}

	return 0;
}

static int
_rawpart_is_boot (const MacRawPartition* raw_part)
{
	if (!strcasecmp(raw_part->type, "Apple_Bootstrap"))
		return 1;

	if (!strcasecmp(raw_part->type, "Apple_Boot"))
		return 1;

	return 0;
}

static int
_rawpart_is_driver (const MacRawPartition* raw_part)
{
	if (strncmp (raw_part->type, "Apple_", 6) != 0)
		return 0;
	if (!strncasestr (raw_part->type, "driver", 32))
		return 0;
	return 1;
}

static int _GL_ATTRIBUTE_PURE
_rawpart_has_driver (const MacRawPartition* raw_part, MacDiskData* mac_disk_data)
{
	MacDeviceDriver *driverlist;
	uint16_t i, bsz;
	uint32_t driver_bs, driver_be, part_be;

	driverlist = &mac_disk_data->driverlist[0];
	bsz = mac_disk_data->block_size / 512;
	for (i = 0; i < mac_disk_data->driver_count; i++) {
		driver_bs = driverlist->block * bsz;
		driver_be = driver_bs + driverlist->size;
		part_be = raw_part->start_block + raw_part->block_count;
		if (driver_bs >= raw_part->start_block && driver_be <= part_be)
			return 1;
		driverlist++;
	}
	return 0;
}

static int
_rawpart_is_root (MacRawPartition* raw_part)
{
	if (!_rawpart_cmp_type (raw_part, "Apple_UNIX_SVR2"))
		return 0;
	if (strcmp (raw_part->name, "root") != 0)
		return 0;
	return 1;
}

static int
_rawpart_is_swap (MacRawPartition* raw_part)
{
	if (!_rawpart_cmp_type (raw_part, "Apple_UNIX_SVR2"))
		return 0;
	if (strcmp (raw_part->name, "swap") != 0)
		return 0;
	return 1;
}

static int
_rawpart_is_lvm (MacRawPartition* raw_part)
{
	if (strcmp (raw_part->type, "Linux_LVM") != 0)
		return 0;
	return 1;
}

static int
_rawpart_is_raid (MacRawPartition* raw_part)
{
	if (strcmp (raw_part->type, "Linux_RAID") != 0)
		return 0;
	return 1;
}

static int
_rawpart_is_void (MacRawPartition* raw_part)
{
	return _rawpart_cmp_type (raw_part, "Apple_Void");
}

/* returns 1 if the raw_part represents a partition that is "unused space", or
 * doesn't represent a partition at all.  NOTE: some people make Apple_Free
 * partitions with MacOS, because they can't select another type.  So, if the
 * name is anything other than "Extra" or "", it is treated as a "real"
 * partition.
 */
static int
_rawpart_is_active (MacRawPartition* raw_part)
{
	if (_rawpart_cmp_type (raw_part, "Apple_Free")
	    && (_rawpart_cmp_name (raw_part, "Extra")
		|| _rawpart_cmp_name (raw_part, "")))
		return 0;
	if (_rawpart_cmp_type (raw_part, "Apple_Void"))
		return 0;
	if (_rawpart_cmp_type (raw_part, "Apple_Scratch"))
		return 0;
	if (_rawpart_cmp_type (raw_part, "Apple_Extra"))
		return 0;

	return 1;
}

static PedPartition*
_rawpart_analyse (MacRawPartition* raw_part, PedDisk* disk, int num)
{
	MacDiskData*		mac_disk_data;
	PedPartition*		part;
	MacPartitionData*	mac_part_data;
	PedSector		block_size;
	PedSector		start, length;

	if (!_rawpart_check_signature (raw_part)) {
#ifndef DISCOVER_ONLY
		if (ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Partition %d has an invalid signature %x."),
			num,
			(int) PED_BE16_TO_CPU (raw_part->signature))
				!= PED_EXCEPTION_IGNORE)
#endif
			goto error;
	}

	mac_disk_data = (MacDiskData*) disk->disk_specific;
	block_size = disk->dev->sector_size / 512;

	start = PED_BE32_TO_CPU (raw_part->start_block) * block_size;
	length = PED_BE32_TO_CPU (raw_part->block_count) * block_size;
	if (length == 0) {
#ifndef DISCOVER_ONLY
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Partition %d has an invalid length of 0 bytes!"),
			num);
#endif
		return NULL;
	}
	part = ped_partition_new (disk, PED_PARTITION_NORMAL, NULL,
                                  start, start + length - 1);
	if (!part)
		goto error;

	mac_part_data = part->disk_specific;

	strncpy (mac_part_data->volume_name, raw_part->name, 32);
	strncpy (mac_part_data->system_name, raw_part->type, 32);
	strncpy (mac_part_data->processor_name, raw_part->processor, 16);

	mac_part_data->is_boot = _rawpart_is_boot (raw_part);
	mac_part_data->is_driver = _rawpart_is_driver (raw_part);
	if (mac_part_data->is_driver)
		mac_part_data->has_driver = _rawpart_has_driver(raw_part, mac_disk_data);
	mac_part_data->is_root = _rawpart_is_root (raw_part);
	mac_part_data->is_swap = _rawpart_is_swap (raw_part);
	mac_part_data->is_lvm = _rawpart_is_lvm (raw_part);
	mac_part_data->is_raid = _rawpart_is_raid (raw_part);

	/* "data" region */
#ifndef DISCOVER_ONLY
	if (raw_part->data_start) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("The data region doesn't start at the start "
			  "of the partition."));
		goto error_destroy_part;
	}
#endif /* !DISCOVER_ONLY */
	mac_part_data->data_region_length
		= PED_BE32_TO_CPU (raw_part->data_count) * block_size;

	/* boot region - we have no idea what this is for, but Mac OSX
	 * seems to put garbage here, and doesn't pay any attention to
	 * it afterwards.  [clausen, dan burcaw]
	 */
#if 0
	if (raw_part->boot_start) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("The boot region doesn't start at the start "
			  "of the partition."));
		goto error_destroy_part;
	}
#endif
	mac_part_data->boot_region_length
		= PED_BE32_TO_CPU (raw_part->boot_count) * block_size;

#ifndef DISCOVER_ONLY
	if (mac_part_data->has_driver) {
		if (mac_part_data->boot_region_length < part->geom.length) {
			if (ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The partition's boot region doesn't occupy "
				  "the entire partition."))
					!= PED_EXCEPTION_IGNORE)
				goto error_destroy_part;
		}
	} else {
		if (mac_part_data->data_region_length < part->geom.length &&
		    !mac_part_data->is_boot) {
			if (ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The partition's data region doesn't occupy "
				  "the entire partition."))
					!= PED_EXCEPTION_IGNORE)
				goto error_destroy_part;
		}
	}
#endif /* !DISCOVER_ONLY */

	mac_part_data->boot_base_address
		= PED_BE32_TO_CPU (raw_part->boot_load);
	mac_part_data->boot_entry_address
		= PED_BE32_TO_CPU (raw_part->boot_entry);
	mac_part_data->boot_checksum
		= PED_BE32_TO_CPU (raw_part->boot_cksum);

	mac_part_data->status = PED_BE32_TO_CPU (raw_part->status);
	mac_part_data->driver_sig = PED_BE32_TO_CPU (raw_part->driver_sig);

	return part;

error_destroy_part:
	ped_partition_destroy (part);
error:
	return NULL;
}

/* looks at the partition map size field in a mac raw partition, and calculates
 * what the size of the partition map should be, from it
 */
static int
_rawpart_get_partmap_size (MacRawPartition* raw_part, PedDisk* disk)
{
	MacDiskData*	mac_disk_data = disk->disk_specific;
	PedSector	sector_size = disk->dev->sector_size / 512;
	PedSector	part_map_start;
	PedSector	part_map_end;

	part_map_start = mac_disk_data->ghost_size;
	part_map_end = sector_size * PED_BE32_TO_CPU (raw_part->map_count);

	return part_map_end - part_map_start + 1;
}

static int
_disk_analyse_block_size (PedDisk* disk, MacRawDisk* raw_disk)
{
	PedSector	block_size;

	if (PED_BE16_TO_CPU (raw_disk->block_size) % 512) {
#ifndef DISCOVER_ONLY
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Weird block size on device descriptor: %d bytes is "
			  "not divisible by 512."),
			(int) PED_BE16_TO_CPU (raw_disk->block_size));
#endif
		goto error;
	}

	block_size = PED_BE16_TO_CPU (raw_disk->block_size) / 512;
	if (block_size != disk->dev->sector_size / 512) {
#ifndef DISCOVER_ONLY
		if (ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("The driver descriptor says the physical block size "
			  "is %d bytes, but Linux says it is %d bytes."),
			(int) block_size * 512,
			(int) disk->dev->sector_size)
				!= PED_EXCEPTION_IGNORE)
			goto error;
#endif
		disk->dev->sector_size = block_size * 512;
	}

	return 1;

error:
	return 0;
}

/* Tries to figure out the block size used by the drivers, for the ghost
 * partitioning scheme.  Ghost partitioning works like this: the OpenFirmware
 * (OF) sees 512 byte blocks, but some drivers use 2048 byte blocks (and,
 * perhaps, some other number?).  To remain compatible, the partition map
 * only has "real" partition map entries on ghost-aligned block numbers (and
 * the others are padded with Apple_Void partitions).  This function tries
 * to figure out what the "ghost-aligned" size is... (which, believe-it-or-not,
 * doesn't always equal 2048!!!)
 */
static int
_disk_analyse_ghost_size (PedDisk* disk)
{
	MacDiskData*		mac_disk_data = disk->disk_specific;

	void *buf = ped_malloc (disk->dev->sector_size);
	if (!buf)
		return 0;

	int i;
	int found = 0;
	for (i = 1; i < 64; i *= 2) {
		if (!ped_device_read (disk->dev, buf, i, 1))
			break;
		if (_rawpart_check_signature (buf)
		    && !_rawpart_is_void (buf)) {
			mac_disk_data->ghost_size = i;
			found = (i <= disk->dev->sector_size / 512);
			break;
		}
	}
        free (buf);

#ifndef DISCOVER_ONLY
        if (!found)
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("No valid partition map found."));
#endif
	return found;
}

static int
mac_read (PedDisk* disk)
{
	MacDiskData*		mac_disk_data;
	PedPartition*		part;
	int			num;
	PedSector		ghost_size;
	int			last_part_entry_num = 0;

	PED_ASSERT (disk != NULL);

	mac_disk_data = disk->disk_specific;
	mac_disk_data->part_map_entry_num = 0;		/* 0 == none */

	void *buf;
	if (!ptt_read_sector (disk->dev, 0, &buf))
		return 0;

	MacRawDisk *raw_disk = (MacRawDisk *) buf;

	if (!_check_signature (raw_disk))
		goto error;

	if (!_disk_analyse_block_size (disk, raw_disk))
		goto error;
	if (!_disk_analyse_ghost_size (disk))
		goto error;
	ghost_size = mac_disk_data->ghost_size;

	if (!ped_disk_delete_all (disk))
		goto error;

	if (raw_disk->driver_count && raw_disk->driver_count < 62) {
		memcpy(&mac_disk_data->driverlist[0], &raw_disk->driverlist[0],
				sizeof(mac_disk_data->driverlist));
		mac_disk_data->driver_count = raw_disk->driver_count;
		mac_disk_data->block_size = raw_disk->block_size;
	}

	for (num=1; num==1 || num <= last_part_entry_num; num++) {
		void *raw_part = buf;
		if (!ped_device_read (disk->dev, raw_part,
				      num * ghost_size, 1))
			goto error_delete_all;

		if (!_rawpart_check_signature (raw_part))
			continue;

		if (num == 1)
			last_part_entry_num
				= _rawpart_get_partmap_size (raw_part, disk);
		if (_rawpart_get_partmap_size (raw_part, disk)
				!= last_part_entry_num) {
			if (ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("Conflicting partition map entry sizes!  "
				  "Entry 1 says it is %d, but entry %d says "
				  "it is %d!"),
				last_part_entry_num,
				_rawpart_get_partmap_size (raw_part, disk))
					!= PED_EXCEPTION_IGNORE)
				goto error_delete_all;
		}

		if (!_rawpart_is_active (raw_part))
			continue;

		part = _rawpart_analyse (raw_part, disk, num);
		if (!part)
			goto error_delete_all;
		part->num = num;
		part->fs_type = ped_file_system_probe (&part->geom);
		PedConstraint *constraint_exact
			= ped_constraint_exact (&part->geom);
		if (constraint_exact == NULL)
			goto error_delete_all;
		bool ok = ped_disk_add_partition (disk, part, constraint_exact);
		ped_constraint_destroy (constraint_exact);
		if (!ok)
			goto error_delete_all;

		if (_rawpart_is_partition_map (raw_part)) {
			if (mac_disk_data->part_map_entry_num
			    && ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_IGNORE_CANCEL,
					_("Weird!  There are 2 partitions "
					  "map entries!"))
			    != PED_EXCEPTION_IGNORE)
				goto error_delete_all;

			mac_disk_data->part_map_entry_num = num;
			mac_disk_data->part_map_entry_count
				= part->geom.end - ghost_size + 1;
		}
	}

	if (!mac_disk_data->part_map_entry_num) {
		if (!_disk_add_part_map_entry (disk, 1))
			goto error_delete_all;
		ped_disk_commit_to_dev (disk);
	}
	free (buf);
	return 1;

error_delete_all:
	ped_disk_delete_all (disk);
error:
	free (buf);
	return 0;
}

#ifndef DISCOVER_ONLY
/* The Ghost partition: is a blank entry, used to pad out each block (where
 * there physical block size > 512 bytes).  This is because OpenFirmware uses
 * 512 byte blocks, but device drivers Think Different TM, with a different
 * lbock size, so we need to do this to avoid a clash (!)
 */
static int
_pad_raw_part (PedDisk* disk, int num, MacRawPartition* part_map)
{
	MacDiskData*		mac_disk_data = disk->disk_specific;
	int			i;

	size_t ss = disk->dev->sector_size;
	void *buf = ped_calloc (ss);
	if (!buf)
		return 0;

	MacRawPartition	*ghost_entry = buf;
	ghost_entry->signature = PED_CPU_TO_BE16 (MAC_PARTITION_MAGIC_2);
	strcpy (ghost_entry->type, "Apple_Void");
	ghost_entry->map_count
		= PED_CPU_TO_BE32 (mac_disk_data->last_part_entry_num);

	for (i=0; i < mac_disk_data->ghost_size - 1; i++) {
		PedSector idx = i + (num - 1) * mac_disk_data->ghost_size;
		memcpy ((char*)part_map + idx * ss, ghost_entry, ss);
        }

	free (buf);
	return 1;
}

static void
_update_driver_count (MacRawPartition* part_map_entry,
		      MacDiskData *mac_driverdata, const MacDiskData* mac_disk_data)
{
	uint16_t	i, count_orig, count_cur, bsz;
	uint32_t	driver_bs, driver_be, part_be;

	bsz = mac_disk_data->block_size / 512;
	count_cur = mac_driverdata->driver_count;
	count_orig = mac_disk_data->driver_count;
	for (i = 0; i < count_orig; i++) {
		driver_bs = mac_disk_data->driverlist[i].block * bsz;
		driver_be = driver_bs + mac_disk_data->driverlist[i].size;
		part_be = part_map_entry->start_block + part_map_entry->block_count;
		if (driver_bs >= part_map_entry->start_block
				&& driver_be <= part_be) {
			mac_driverdata->driverlist[count_cur].block
				= mac_disk_data->driverlist[i].block;
			mac_driverdata->driverlist[count_cur].size
				= mac_disk_data->driverlist[i].size;
			mac_driverdata->driverlist[count_cur].type
				= mac_disk_data->driverlist[i].type;
			mac_driverdata->driver_count++;
			break;
		}
	}
}

static MacRawPartition *
get_pme (MacRawPartition const *part_map, PedSector i, PedDisk const *disk)
{
	MacDiskData const *mac_disk_data = disk->disk_specific;
	PedSector idx = i * mac_disk_data->ghost_size - 1;
	return (MacRawPartition *) ((char*)part_map
                                    + idx * disk->dev->sector_size);
}

/* Initialize the disk->dev->sector_size bytes of part_map[part->num]. */
static int
_generate_raw_part (PedDisk* disk, PedPartition* part,
	       	    MacRawPartition* part_map, MacDiskData *mac_driverdata)
{
	MacDiskData*		mac_disk_data;
	MacPartitionData*	mac_part_data;
	PedSector		block_size = disk->dev->sector_size / 512;

	PED_ASSERT (part->num > 0);

	mac_disk_data = disk->disk_specific;
	mac_part_data = part->disk_specific;

	MacRawPartition *part_map_entry = get_pme (part_map, part->num, disk);
	memset (part_map_entry, 0, disk->dev->sector_size);

	part_map_entry->signature = PED_CPU_TO_BE16 (MAC_PARTITION_MAGIC_2);
	part_map_entry->map_count
		= PED_CPU_TO_BE32 (mac_disk_data->last_part_entry_num);
	part_map_entry->start_block
		= PED_CPU_TO_BE32 (part->geom.start / block_size);
	part_map_entry->block_count
		= PED_CPU_TO_BE32 (part->geom.length / block_size);
	strcpy (part_map_entry->name, mac_part_data->volume_name);
	strcpy (part_map_entry->type, mac_part_data->system_name);

	if (mac_part_data->is_driver) {
		mac_part_data->boot_region_length = part->geom.length;
		if (mac_part_data->has_driver)
			_update_driver_count(part_map_entry, mac_driverdata,
					mac_disk_data);
	} else
		mac_part_data->data_region_length = part->geom.length;
	part_map_entry->data_count = PED_CPU_TO_BE32 (
			mac_part_data->data_region_length / block_size);
	part_map_entry->boot_count = PED_CPU_TO_BE32 (
			mac_part_data->boot_region_length / block_size);
	part_map_entry->status = PED_CPU_TO_BE32 (mac_part_data->status);
	part_map_entry->driver_sig
		= PED_CPU_TO_BE32 (mac_part_data->driver_sig);

	part_map_entry->boot_load =
		PED_CPU_TO_BE32 (mac_part_data->boot_base_address);
	part_map_entry->boot_entry =
		PED_CPU_TO_BE32 (mac_part_data->boot_entry_address);
	part_map_entry->boot_cksum =
		PED_CPU_TO_BE32 (mac_part_data->boot_checksum);

	strncpy (part_map_entry->processor, mac_part_data->processor_name, 16);

	if (!_pad_raw_part (disk, part->num, part_map))
		goto error;

	return 1;

error:
	return 0;
}

static int
_generate_raw_freespace_part (PedDisk* disk, PedGeometry* geom, int num,
			      MacRawPartition* part_map)
{
	MacDiskData*		mac_disk_data = disk->disk_specific;
	PedSector		block_size = disk->dev->sector_size / 512;

	PED_ASSERT (num > 0);

	MacRawPartition *part_map_entry = get_pme (part_map, num, disk);

	part_map_entry->signature = PED_CPU_TO_BE16 (MAC_PARTITION_MAGIC_2);
	part_map_entry->map_count
		= PED_CPU_TO_BE32 (mac_disk_data->last_part_entry_num);
	part_map_entry->start_block
		= PED_CPU_TO_BE32 (geom->start / block_size);
	part_map_entry->block_count
		= PED_CPU_TO_BE32 (geom->length / block_size);
	strcpy (part_map_entry->name, "Extra");
	strcpy (part_map_entry->type, "Apple_Free");

	part_map_entry->data_count = PED_CPU_TO_BE32 (geom->length);
	part_map_entry->status = 0;
	part_map_entry->driver_sig = 0;

	if (!_pad_raw_part (disk, num, part_map))
		goto error;

	return 1;

error:
	return 0;
}

static int
_generate_empty_part (PedDisk* disk, int num, MacRawPartition* part_map)
{
	MacDiskData*		mac_disk_data = disk->disk_specific;

	PED_ASSERT (num > 0);

	MacRawPartition *part_map_entry = get_pme (part_map, num, disk);
	part_map_entry->signature = PED_CPU_TO_BE16 (MAC_PARTITION_MAGIC_2);
	part_map_entry->map_count
		= PED_CPU_TO_BE32 (mac_disk_data->last_part_entry_num);
	strcpy (part_map_entry->type, "Apple_Void");

	return _pad_raw_part (disk, num, part_map);
}

/* returns the first empty entry in the partition map */
static int _GL_ATTRIBUTE_PURE
_get_first_empty_part_entry (PedDisk* disk, MacRawPartition* part_map)
{
	MacDiskData*	mac_disk_data = disk->disk_specific;
	int		i;

	for (i=1; i <= mac_disk_data->last_part_entry_num; i++) {
		MacRawPartition *part_map_entry = get_pme (part_map, i, disk);
		if (!part_map_entry->signature)
			return i;
	}

	return 0;
}

static int
write_block_zero (PedDisk* disk, MacDiskData* mac_driverdata)
{
	PedDevice*	dev = disk->dev;
	void *s0;
	if (!ptt_read_sector (dev, 0, &s0))
		return 0;
	MacRawDisk *raw_disk = (MacRawDisk *) s0;

	raw_disk->signature = PED_CPU_TO_BE16 (MAC_DISK_MAGIC);
	raw_disk->block_size = PED_CPU_TO_BE16 (dev->sector_size);
	raw_disk->block_count
		= PED_CPU_TO_BE32 (dev->length / (dev->sector_size / 512));

	raw_disk->driver_count = mac_driverdata->driver_count;
	memcpy(&raw_disk->driverlist[0], &mac_driverdata->driverlist[0],
			sizeof(raw_disk->driverlist));

	int write_ok = ped_device_write (dev, raw_disk, 0, 1);
        free (s0);
	return write_ok;
}

static int
mac_write (PedDisk* disk)
{
	MacRawPartition*	part_map;
	MacDiskData*		mac_disk_data;
	MacDiskData*		mac_driverdata;	/* updated driver list */
	PedPartition*		part;
	int			num;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->disk_specific != NULL);
	PED_ASSERT (disk->dev != NULL);
	PED_ASSERT (!disk->update_mode);

	mac_disk_data = disk->disk_specific;

	if (!ped_disk_get_partition (disk, mac_disk_data->part_map_entry_num)) {
		if (!_disk_add_part_map_entry (disk, 1))
			goto error;
	}

	mac_driverdata = ped_malloc(sizeof(MacDiskData));
	if (!mac_driverdata)
		goto error;
	memset (mac_driverdata, 0, sizeof(MacDiskData));

        size_t pmap_bytes = (mac_disk_data->part_map_entry_count
                             * mac_disk_data->ghost_size
                             * disk->dev->sector_size);
	part_map = (MacRawPartition*) ped_calloc (pmap_bytes);
	if (!part_map)
		goto error_free_driverdata;

/* write (to memory) the "real" partitions */
	for (part = ped_disk_next_partition (disk, NULL); part;
	     part = ped_disk_next_partition (disk, part)) {
		if (!ped_partition_is_active (part))
			continue;
		if (!_generate_raw_part (disk, part, part_map, mac_driverdata))
			goto error_free_part_map;
	}

/* write the "free space" partitions */
	for (part = ped_disk_next_partition (disk, NULL); part;
	     part = ped_disk_next_partition (disk, part)) {
		if (part->type != PED_PARTITION_FREESPACE)
			continue;
		num = _get_first_empty_part_entry (disk, part_map);
		if (!_generate_raw_freespace_part (disk, &part->geom, num,
						   part_map))
			goto error_free_part_map;
	}

/* write the "void" (empty) partitions */
	for (num = _get_first_empty_part_entry (disk, part_map); num;
	     num = _get_first_empty_part_entry (disk, part_map))
		_generate_empty_part (disk, num, part_map);

/* write to disk */
	if (!ped_device_write (disk->dev, part_map, 1,
			       mac_disk_data->part_map_entry_count))
		goto error_free_part_map;
	free (part_map);
	int write_ok = write_block_zero (disk, mac_driverdata);
	free (mac_driverdata);
	return write_ok;

error_free_part_map:
	free (part_map);
error_free_driverdata:
	free (mac_driverdata);
error:
	return 0;
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
mac_partition_new (
	const PedDisk* disk, PedPartitionType part_type,
	const PedFileSystemType* fs_type, PedSector start, PedSector end)
{
	PedPartition*		part;
	MacPartitionData*	mac_data;

	part = _ped_partition_alloc (disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	if (ped_partition_is_active (part)) {
		part->disk_specific
			= mac_data = ped_malloc (sizeof (MacPartitionData));
		if (!mac_data)
			goto error_free_part;

		memset (mac_data, 0, sizeof (MacPartitionData));
		strcpy (mac_data->volume_name, "untitled");
	} else {
		part->disk_specific = NULL;
	}
	return part;

error_free_part:
	free (part);
error:
	return NULL;
}

static PedPartition*
mac_partition_duplicate (const PedPartition* part)
{
	PedPartition*		new_part;
	MacPartitionData*	new_mac_data;
	MacPartitionData*	old_mac_data;

	new_part = ped_partition_new (part->disk, part->type,
				      part->fs_type, part->geom.start,
				      part->geom.end);
	if (!new_part)
		return NULL;
	new_part->num = part->num;

	old_mac_data = (MacPartitionData*) part->disk_specific;
	new_mac_data = (MacPartitionData*) new_part->disk_specific;

	/* ugly, but C is ugly :p */
	memcpy (new_mac_data, old_mac_data, sizeof (MacPartitionData));
	return new_part;
}

static void
mac_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	if (ped_partition_is_active (part))
		free (part->disk_specific);
	free (part);
}

static int
mac_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
	MacPartitionData* mac_data = part->disk_specific;

	part->fs_type = fs_type;

	if (fs_type && is_linux_swap (fs_type->name))
		ped_partition_set_flag (part, PED_PARTITION_SWAP, 1);

	if (mac_data->is_boot) {
		strcpy (mac_data->system_name, "Apple_Bootstrap");
		mac_data->status = 0x33;
		return 1;
	}

	if (fs_type && (!strcmp (fs_type->name, "hfs")
			|| !strcmp (fs_type->name, "hfs+"))) {
		strcpy (mac_data->system_name, "Apple_HFS");
		mac_data->status |= 0x7f;
	} else if (fs_type && !strcmp (fs_type->name, "hfsx")) {
		strcpy (mac_data->system_name, "Apple_HFSX");
		mac_data->status |= 0x7f;
	} else {
		strcpy (mac_data->system_name, "Apple_UNIX_SVR2");
		mac_data->status = 0x33;
	}

	return 1;
}

static int
mac_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	MacPartitionData*	mac_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	mac_data = part->disk_specific;

	switch (flag) {
	case PED_PARTITION_BOOT:
		mac_data->is_boot = state;

		if (part->fs_type)
			return mac_partition_set_system (part, part->fs_type);

		if (state) {
			strcpy (mac_data->system_name, "Apple_Bootstrap");
			mac_data->status = 0x33;
		}
		return 1;

	case PED_PARTITION_ROOT:
		if (state) {
			strcpy (mac_data->volume_name, "root");
			mac_data->is_swap = 0;
		} else {
			if (mac_data->is_root)
				strcpy (mac_data->volume_name, "untitled");
		}
		mac_data->is_root = state;
		return 1;

	case PED_PARTITION_SWAP:
		if (state) {
			strcpy (mac_data->volume_name, "swap");
			mac_data->is_root = 0;
		} else {
			if (mac_data->is_swap)
				strcpy (mac_data->volume_name, "untitled");
		}
		mac_data->is_swap = state;
		return 1;

	case PED_PARTITION_LVM:
		if (state) {
			strcpy (mac_data->system_name, "Linux_LVM");
			mac_data->is_lvm = state;
		} else {
			if (mac_data->is_lvm)
				mac_partition_set_system (part, part->fs_type);
		}
		return 1;

	case PED_PARTITION_RAID:
		if (state) {
			strcpy (mac_data->system_name, "Linux_RAID");
			mac_data->is_raid = state;
		} else {
			if (mac_data->is_raid)
				mac_partition_set_system (part, part->fs_type);
		}
		return 1;

	default:
		return 0;
	}
}

static int _GL_ATTRIBUTE_PURE
mac_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	MacPartitionData*	mac_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	mac_data = part->disk_specific;
	switch (flag) {
	case PED_PARTITION_BOOT:
		return mac_data->is_boot;

	case PED_PARTITION_ROOT:
		return mac_data->is_root;

	case PED_PARTITION_SWAP:
		return mac_data->is_swap;

	case PED_PARTITION_LVM:
		return mac_data->is_lvm;

	case PED_PARTITION_RAID:
		return mac_data->is_raid;

	default:
		return 0;
	}
}

static int
mac_partition_is_flag_available (
	const PedPartition* part, PedPartitionFlag flag)
{
	switch (flag) {
	case PED_PARTITION_BOOT:
	case PED_PARTITION_ROOT:
	case PED_PARTITION_SWAP:
	case PED_PARTITION_LVM:
	case PED_PARTITION_RAID:
		return 1;

	default:
		return 0;
	}
}

static void
mac_partition_set_name (PedPartition* part, const char* name)
{
	MacPartitionData*	mac_data;
	int			i;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	mac_data = part->disk_specific;

#ifndef DISCOVER_ONLY
	if (mac_data->is_root || mac_data->is_swap) {
		if (ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Changing the name of a root or swap partition "
			  "will prevent Linux from recognising it as such."))
				!= PED_EXCEPTION_IGNORE)
			return;
		mac_data->is_root = mac_data->is_swap = 0;
	}
#endif

	strncpy (mac_data->volume_name, name, 32);
	mac_data->volume_name [32] = 0;
	for (i = strlen (mac_data->volume_name) - 1;
			mac_data->volume_name[i] == ' '; i--)
		mac_data->volume_name [i] = 0;
}

static const char* _GL_ATTRIBUTE_PURE
mac_partition_get_name (const PedPartition* part)
{
	MacPartitionData*	mac_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	mac_data = part->disk_specific;

	return mac_data->volume_name;
}

static PedAlignment*
mac_get_partition_alignment(const PedDisk *disk)
{
        PedSector sector_size = disk->dev->sector_size / 512;

        return ped_alignment_new(0, sector_size);
}

static PedConstraint*
_primary_constraint (PedDisk* disk)
{
	PedAlignment	start_align;
	PedAlignment	end_align;
	PedGeometry	max_geom;
	PedSector	sector_size;

	sector_size = disk->dev->sector_size / 512;

	if (!ped_alignment_init (&start_align, 0, sector_size))
		return NULL;
	if (!ped_alignment_init (&end_align, -1, sector_size))
		return NULL;
	if (!ped_geometry_init (&max_geom, disk->dev, 1, disk->dev->length - 1))
		return NULL;

	return ped_constraint_new (&start_align, &end_align, &max_geom,
				   &max_geom, 1, disk->dev->length);
}

static int
mac_partition_align (PedPartition* part, const PedConstraint* constraint)
{
	PED_ASSERT (part != NULL);

	if (_ped_partition_attempt_align (part, constraint,
					  _primary_constraint (part->disk)))
	       	return 1;

#ifndef DISCOVER_ONLY
	ped_exception_throw (
		PED_EXCEPTION_ERROR,
		PED_EXCEPTION_CANCEL,
		_("Unable to satisfy all constraints on the partition."));
#endif
	return 0;
}

static int
mac_partition_enumerate (PedPartition* part)
{
	PedDisk*		disk;
	MacDiskData*		mac_disk_data;
	int			i;
	int			max_part_count;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);

	disk = part->disk;
	mac_disk_data = (MacDiskData*) disk->disk_specific;

	max_part_count = ped_disk_get_max_primary_partition_count (disk);

	if (part->num > 0 && part->num <= mac_disk_data->part_map_entry_count)
		return 1;

	for (i = 1; i <= max_part_count; i++) {
		if (!ped_disk_get_partition (disk, i)) {
			part->num = i;
			return 1;
		}
	}

#ifndef DISCOVER_ONLY
	ped_exception_throw (
		PED_EXCEPTION_ERROR,
		PED_EXCEPTION_CANCEL,
		_("Can't add another partition -- the partition map is too "
		  "small!"));
#endif

	return 0;
}

static int
_disk_count_partitions (PedDisk* disk)
{
	MacDiskData*		mac_disk_data = disk->disk_specific;
	PedPartition*		part = NULL;
	PedPartition*		last = NULL;

	PED_ASSERT (disk->update_mode);

	mac_disk_data->active_part_entry_count = 0;
	mac_disk_data->free_part_entry_count = 0;
	mac_disk_data->last_part_entry_num = 0;

	/* subtle: we only care about free space after the partition map.
	 * the partition map is an "active" partition, BTW... */
	for (part = ped_disk_next_partition (disk, part); part;
	     part = ped_disk_next_partition (disk, part)) {
		if (!ped_partition_is_active (part))
			continue;

		mac_disk_data->active_part_entry_count++;
		if (last && last->geom.end + 1 < part->geom.start)
			mac_disk_data->free_part_entry_count++;
		mac_disk_data->last_part_entry_num
			= PED_MAX (mac_disk_data->last_part_entry_num,
				   part->num);

		last = part;
	}

	if (last && last->geom.end < disk->dev->length - 1)
		mac_disk_data->free_part_entry_count++;

	mac_disk_data->last_part_entry_num
		= PED_MAX (mac_disk_data->last_part_entry_num,
			   mac_disk_data->active_part_entry_count
				+ mac_disk_data->free_part_entry_count);
	return 1;
}

static int
add_metadata_part (PedDisk* disk, PedSector start, PedSector end)
{
	PedPartition*		new_part;
	PedConstraint*		constraint_any = ped_constraint_any (disk->dev);

	PED_ASSERT (disk != NULL);

	new_part = ped_partition_new (disk, PED_PARTITION_METADATA, NULL,
				      start, end);
	if (!new_part)
		goto error;
	if (!ped_disk_add_partition (disk, new_part, constraint_any))
		goto error_destroy_new_part;

	ped_constraint_destroy (constraint_any);
	return 1;

error_destroy_new_part:
	ped_partition_destroy (new_part);
error:
	ped_constraint_destroy (constraint_any);
	return 0;
}

static int
mac_alloc_metadata (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->disk_specific != NULL);
	PED_ASSERT (disk->dev != NULL);

	if (!add_metadata_part (disk, 0, disk->dev->sector_size / 512 - 1))
		return 0;

	/* hack: this seems to be a good place, to update the partition	map
	 * entry count, since mac_alloc_metadata() gets called during
	 * _disk_pop_update_mode()
	 */
	return _disk_count_partitions (disk);
}

static int
mac_get_max_primary_partition_count (const PedDisk* disk)
{
	MacDiskData*	mac_disk_data = disk->disk_specific;
	PedPartition*	part_map_partition;

	part_map_partition = ped_disk_get_partition (disk,
		       			mac_disk_data->part_map_entry_num);

	/* HACK: if we haven't found the partition map partition (yet),
	 * we return this.
	 */
	if (!part_map_partition) {
		mac_disk_data->part_map_entry_num = 0;
		return 65536;
	}

	/* HACK: since Mac labels need an entry for free-space regions, we
	 * must allow half plus 1 entries for free-space partitions.  I hate
	 * this, but things get REALLY complicated, otherwise.
	 *     (I'm prepared to complicate things later, but I want to get
	 * everything working, first)
	 */
	return mac_disk_data->part_map_entry_count / mac_disk_data->ghost_size
		- mac_disk_data->free_part_entry_count + 1;
}

static bool
mac_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = 65536;
	return true;
}

#include "pt-common.h"
PT_define_limit_functions (mac)

static PedDiskOps mac_disk_ops = {
	clobber: NULL,
        /* FIXME: remove this cast, once mac_write is fixed not to
           modify its *DISK parameter.  */
	write:	NULL_IF_DISCOVER_ONLY ((int (*) (const PedDisk*)) mac_write),

	partition_set_name:	mac_partition_set_name,
	partition_get_name:	mac_partition_get_name,

	get_partition_alignment: mac_get_partition_alignment,

	PT_op_function_initializers (mac)
};

static PedDiskType mac_disk_type = {
	next:		NULL,
	name:		"mac",
	ops:		&mac_disk_ops,
	features:	PED_DISK_TYPE_PARTITION_NAME
};

void
ped_disk_mac_init ()
{
	PED_ASSERT (sizeof (MacRawPartition) == 512);
	PED_ASSERT (sizeof (MacRawDisk) == 512);

	ped_disk_type_register (&mac_disk_type);
}

void
ped_disk_mac_done ()
{
	ped_disk_type_unregister (&mac_disk_type);
}
