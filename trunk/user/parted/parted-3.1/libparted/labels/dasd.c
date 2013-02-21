/* -*- Mode: c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-

    libparted - a library for manipulating disk partitions
    Copyright (C) 2000-2001, 2007-2012 Free Software Foundation, Inc.

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

    Contributor:  Phil Knirsch <phil@redhat.de>
                  Harald Hoyer <harald@redhat.de>
*/

#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include <parted/vtoc.h>
#include <parted/fdasd.h>
#include <arch/linux.h>

#include <libintl.h>
#if ENABLE_NLS
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "misc.h"
#include "pt-tools.h"

#define PARTITION_LINUX_SWAP 0x82
#define PARTITION_LINUX 0x83
#define PARTITION_LINUX_EXT 0x85
#define PARTITION_LINUX_LVM 0x8e
#define PARTITION_LINUX_RAID 0xfd
#define PARTITION_LINUX_LVM_OLD 0xfe

extern void ped_disk_dasd_init ();
extern void ped_disk_dasd_done ();

#define DASD_NAME "dasd"

typedef struct {
	int type;
	int system;
	int	raid;
	int	lvm;
} DasdPartitionData;

typedef struct {
	unsigned int format_type;
	volume_label_t vlabel;
} DasdDiskSpecific;

static int dasd_probe (const PedDevice *dev);
static int dasd_read (PedDisk* disk);
static int dasd_write (const PedDisk* disk);

static PedPartition* dasd_partition_new (const PedDisk* disk,
										 PedPartitionType part_type,
										 const PedFileSystemType* fs_type,
										 PedSector start,
										 PedSector end);
static PedPartition* dasd_partition_duplicate (const PedPartition *part);
static void dasd_partition_destroy (PedPartition* part);
static int dasd_partition_set_flag (PedPartition* part,
									PedPartitionFlag flag,
									int state);
static int dasd_partition_get_flag (const PedPartition* part,
									PedPartitionFlag flag);
static int dasd_partition_is_flag_available (const PedPartition* part,
											 PedPartitionFlag flag);
static int dasd_partition_align (PedPartition* part,
								 const PedConstraint* constraint);
static int dasd_partition_enumerate (PedPartition* part);
static int dasd_get_max_primary_partition_count (const PedDisk* disk);
static bool dasd_get_max_supported_partition_count (const PedDisk* disk, int *max_n);
static PedAlignment *dasd_get_partition_alignment(const PedDisk *disk);

static PedDisk* dasd_alloc (const PedDevice* dev);
static PedDisk* dasd_duplicate (const PedDisk* disk);
static void dasd_free (PedDisk* disk);
static int dasd_partition_set_system (PedPartition* part,
									  const PedFileSystemType* fs_type);
static int dasd_alloc_metadata (PedDisk* disk);

#include "pt-common.h"
PT_define_limit_functions (dasd)

static PedDiskOps dasd_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (dasd_write),

	partition_set_name:	NULL,
	partition_get_name:	NULL,

	get_partition_alignment: dasd_get_partition_alignment,

	PT_op_function_initializers (dasd)
};

static PedDiskType dasd_disk_type = {
	next: NULL,
	name: "dasd",
	ops: &dasd_disk_ops,
	features: 0
};

static PedDisk*
dasd_alloc (const PedDevice* dev)
{
	PedDisk* disk;
	LinuxSpecific* arch_specific;
	DasdDiskSpecific *disk_specific;
	char volser[7];

	PED_ASSERT (dev != NULL);

	arch_specific = LINUX_SPECIFIC (dev);
	disk = _ped_disk_alloc (dev, &dasd_disk_type);
	if (!disk)
		return NULL;

	disk->disk_specific = disk_specific = ped_malloc(sizeof(DasdDiskSpecific));
	if (!disk->disk_specific) {
		free (disk);
		return NULL;
	}

	/* CDL format, newer */
	disk_specific->format_type = 2;

	/* Setup volume label (for fresh disks) */
	snprintf(volser, sizeof(volser), "0X%04X", arch_specific->devno);
	vtoc_volume_label_init(&disk_specific->vlabel);
	vtoc_volume_label_set_key(&disk_specific->vlabel, "VOL1");
	vtoc_volume_label_set_label(&disk_specific->vlabel, "VOL1");
	vtoc_volume_label_set_volser(&disk_specific->vlabel, volser);
	vtoc_set_cchhb(&disk_specific->vlabel.vtoc,
		       VTOC_START_CC, VTOC_START_HH, 0x01);

	return disk;
}

static PedDisk*
dasd_duplicate (const PedDisk* disk)
{
	PedDisk* new_disk;

	new_disk = ped_disk_new_fresh(disk->dev, &dasd_disk_type);

	if (!new_disk)
		return NULL;

	memcpy(new_disk->disk_specific, disk->disk_specific,
	       sizeof(DasdDiskSpecific));

	return new_disk;
}

static void
dasd_free (PedDisk* disk)
{
	PED_ASSERT(disk != NULL);
	/* Don't free disk->disk_specific first, in case _ped_disk_free
	   or one of its eventual callees ever accesses it.  */
	void *p = disk->disk_specific;
	_ped_disk_free(disk);
	free(p);
}


void
ped_disk_dasd_init ()
{
	ped_disk_type_register(&dasd_disk_type);
}

void
ped_disk_dasd_done ()
{
	ped_disk_type_unregister(&dasd_disk_type);
}

static int
dasd_probe (const PedDevice *dev)
{
	LinuxSpecific* arch_specific;
	struct fdasd_anchor anchor;

	PED_ASSERT(dev != NULL);

	if (!(dev->type == PED_DEVICE_DASD
              || dev->type == PED_DEVICE_VIODASD
              || dev->type == PED_DEVICE_FILE))
		return 0;

	arch_specific = LINUX_SPECIFIC(dev);

	/* add partition test here */
	fdasd_initialize_anchor(&anchor);

	fdasd_get_geometry(dev, &anchor, arch_specific->fd);

	fdasd_check_api_version(&anchor, arch_specific->fd);

	if (fdasd_check_volume(&anchor, arch_specific->fd))
		goto error_cleanup;

	fdasd_cleanup(&anchor);

	return 1;

 error_cleanup:
	fdasd_cleanup(&anchor);
	ped_exception_throw(PED_EXCEPTION_ERROR,PED_EXCEPTION_IGNORE_CANCEL,
			    "Error while probing device %s.", dev->path);

	return 0;
}

static int
dasd_read (PedDisk* disk)
{
	int i;
	char str[20];
	PedDevice* dev;
	PedPartition* part;
	PedFileSystemType *fs;
	PedSector start, end;
	PedConstraint* constraint_exact;
	partition_info_t *p;
	LinuxSpecific* arch_specific;
	DasdDiskSpecific* disk_specific;
	struct fdasd_anchor anchor;

	PDEBUG;

	PED_ASSERT (disk != NULL);
	PDEBUG;
	PED_ASSERT (disk->dev != NULL);
	PDEBUG;

	dev = disk->dev;

	arch_specific = LINUX_SPECIFIC(dev);
	disk_specific = disk->disk_specific;

	PDEBUG;

	fdasd_initialize_anchor(&anchor);

	fdasd_get_geometry(disk->dev, &anchor, arch_specific->fd);

	/* check dasd for labels and vtoc */
	if (fdasd_check_volume(&anchor, arch_specific->fd))
		goto error_close_dev;

	/* Save volume label (read by fdasd_check_volume) for writing */
	memcpy(&disk_specific->vlabel, anchor.vlabel, sizeof(volume_label_t));

	if ((anchor.geo.cylinders * anchor.geo.heads) > BIG_DISK_SIZE)
		anchor.big_disk++;

	ped_disk_delete_all (disk);

	bool is_ldl = strncmp(anchor.vlabel->volkey,
			 vtoc_ebcdic_enc("LNX1", str, 4), 4) == 0;
	bool is_cms = strncmp(anchor.vlabel->volkey,
			 vtoc_ebcdic_enc("CMS1", str, 4), 4) == 0;
	if (is_ldl || is_cms) {
		DasdPartitionData* dasd_data;

		union vollabel {
			volume_label_t unused;
			ldl_volume_label_t ldl;
			cms_volume_label_t cms;
		};
		union vollabel *cms_ptr1 = (union vollabel *) anchor.vlabel;
		cms_volume_label_t *cms_ptr = &cms_ptr1->cms;
		ldl_volume_label_t *ldl_ptr = &cms_ptr1->ldl;
		int partition_start_block;

		disk_specific->format_type = 1;

		if (is_cms && cms_ptr->usable_count >= cms_ptr->block_count)
			partition_start_block = 2;   /* FBA DASD */
		else
			partition_start_block = 3;   /* CKD DASD */

		if (is_ldl)
			start = (long long) arch_specific->real_sector_size
				/ (long long) disk->dev->sector_size
				* (long long) partition_start_block;
		else if (cms_ptr->disk_offset == 0)
			start = (long long) cms_ptr->block_size
				/ (long long) disk->dev->sector_size
				* (long long) partition_start_block;
		else
			start = (long long) cms_ptr->block_size
				/ (long long) disk->dev->sector_size
				* (long long) cms_ptr->disk_offset;

		if (is_ldl)
		   if (strncmp(ldl_ptr->ldl_version,
			       vtoc_ebcdic_enc("2", str, 1), 1) >= 0)
		      end = (long long) arch_specific->real_sector_size
			    / (long long) disk->dev->sector_size
			    * (long long) ldl_ptr->formatted_blocks - 1;
		   else
		      end = disk->dev->length - 1;
		else
		   if (cms_ptr->disk_offset == 0)
		      end = (long long) cms_ptr->block_size
			    / (long long) disk->dev->sector_size
			    * (long long) cms_ptr->block_count - 1;
		   else
		      /*
			 Frankly, I do not understand why the last block
			 of the CMS reserved file is not included in the
			 partition; but this is the algorithm used by the
			 Linux kernel.  See fs/partitions/ibm.c in the
			 Linux kernel source code.
		      */
		      end = (long long) cms_ptr->block_size
			    / (long long) disk->dev->sector_size
			    * (long long) (cms_ptr->block_count - 1) - 1;

		part = ped_partition_new (disk, PED_PARTITION_PROTECTED, NULL, start, end);
		if (!part)
			goto error_close_dev;

		part->num = 1;
		part->fs_type = ped_file_system_probe (&part->geom);
		dasd_data = part->disk_specific;
		dasd_data->raid = 0;
		dasd_data->lvm = 0;
		dasd_data->type = 0;

		if (!ped_disk_add_partition (disk, part, NULL))
			goto error_close_dev;

		fdasd_cleanup(&anchor);

		return 1;
	}

	/* CDL format, newer */
	disk_specific->format_type = 2;

	p = anchor.first;
	PDEBUG;

	for (i = 1 ; i <= USABLE_PARTITIONS; i++) {
		char *ch = p->f1->DS1DSNAM;
		DasdPartitionData* dasd_data;


		if (p->used != 0x01)
			continue;

        PDEBUG;

		start = (long long)(long long) p->start_trk
				* (long long) disk->dev->hw_geom.sectors
				* (long long) arch_specific->real_sector_size
				/ (long long) disk->dev->sector_size;
		end   = (long long)((long long) p->end_trk + 1)
				* (long long) disk->dev->hw_geom.sectors
				* (long long) arch_specific->real_sector_size
				/ (long long) disk->dev->sector_size - 1;
		part = ped_partition_new(disk, PED_PARTITION_NORMAL, NULL,
                                         start, end);
        PDEBUG;

		if (!part)
			goto error_close_dev;

        PDEBUG;

		part->num = i;
		part->fs_type = ped_file_system_probe(&part->geom);

		vtoc_ebcdic_dec(p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);
		ch = strstr(p->f1->DS1DSNAM, "PART");

		if (ch != NULL) {
			strncpy(str, ch+9, 6);
			str[6] = '\0';
		}

		dasd_data = part->disk_specific;

		if ((strncmp(PART_TYPE_RAID, str, 6) == 0) &&
		    (ped_file_system_probe(&part->geom) == NULL))
			ped_partition_set_flag(part, PED_PARTITION_RAID, 1);
		else
			ped_partition_set_flag(part, PED_PARTITION_RAID, 0);

		if ((strncmp(PART_TYPE_LVM, str, 6) == 0) &&
		    (ped_file_system_probe(&part->geom) == NULL))
			ped_partition_set_flag(part, PED_PARTITION_LVM, 1);
		else
			ped_partition_set_flag(part, PED_PARTITION_LVM, 0);

		if (strncmp(PART_TYPE_SWAP, str, 6) == 0) {
			fs = ped_file_system_probe(&part->geom);
			if (fs && is_linux_swap(fs->name)) {
				dasd_data->system = PARTITION_LINUX_SWAP;
				PDEBUG;
			}
		}

		vtoc_ebcdic_enc(p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);

		dasd_data->type = 0;

		constraint_exact = ped_constraint_exact (&part->geom);
		if (!constraint_exact)
			goto error_close_dev;
		if (!ped_disk_add_partition(disk, part, constraint_exact)) {
			ped_constraint_destroy(constraint_exact);
			goto error_close_dev;
		}
		ped_constraint_destroy(constraint_exact);

		if (p->fspace_trk > 0) {
			start = (long long)((long long) p->end_trk + 1)
					* (long long) disk->dev->hw_geom.sectors
					* (long long) arch_specific->real_sector_size
					/ (long long) disk->dev->sector_size;
			end   = (long long)((long long) p->end_trk + 1 + p->fspace_trk)
					* (long long) disk->dev->hw_geom.sectors
					* (long long) arch_specific->real_sector_size
					/ (long long) disk->dev->sector_size - 1;
			part = ped_partition_new (disk, PED_PARTITION_NORMAL,
                                                  NULL, start, end);

			if (!part)
				goto error_close_dev;

			part->type = PED_PARTITION_FREESPACE;
			constraint_exact = ped_constraint_exact(&part->geom);

			if (!constraint_exact)
				goto error_close_dev;
			if (!ped_disk_add_partition(disk, part, constraint_exact)) {
				ped_constraint_destroy(constraint_exact);
				goto error_close_dev;
			}

			ped_constraint_destroy (constraint_exact);
		}

		p = p->next;
	}

	PDEBUG;
	fdasd_cleanup(&anchor);
	return 1;

error_close_dev:
	PDEBUG;
	fdasd_cleanup(&anchor);
	return 0;
}

static int
dasd_update_type (const PedDisk* disk, struct fdasd_anchor *anchor,
		  partition_info_t *part_info[USABLE_PARTITIONS])
{
	PedPartition* part;
	LinuxSpecific* arch_specific;
	DasdDiskSpecific* disk_specific;

	arch_specific = LINUX_SPECIFIC(disk->dev);
	disk_specific = disk->disk_specific;

	PDEBUG;

	unsigned int i;
	for (i = 1; i <= USABLE_PARTITIONS; i++) {
		partition_info_t *p;
		char *ch = NULL;
		DasdPartitionData* dasd_data;

		PDEBUG;

		part = ped_disk_get_partition(disk, i);
		if (!part)
			continue;

		PDEBUG;

		dasd_data = part->disk_specific;
		p = part_info[i - 1];

		if (!p ) {
			PDEBUG;
			continue;
		}

		vtoc_ebcdic_dec(p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);
		ch = strstr(p->f1->DS1DSNAM, "PART");

		PDEBUG;
		if (ch == NULL) {
			vtoc_ebcdic_enc(p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);
			PDEBUG;
			continue;
		}

		ch += 9;

		switch (dasd_data->system) {
			case PARTITION_LINUX_LVM:
				PDEBUG;
				strncpy(ch, PART_TYPE_LVM, 6);
				break;
			case PARTITION_LINUX_RAID:
				PDEBUG;
				strncpy(ch, PART_TYPE_RAID, 6);
				break;
			case PARTITION_LINUX:
				PDEBUG;
				strncpy(ch, PART_TYPE_NATIVE, 6);
				break;
			case PARTITION_LINUX_SWAP:
				PDEBUG;
				strncpy(ch, PART_TYPE_SWAP, 6);
				break;
			default:
				PDEBUG;
				strncpy(ch, PART_TYPE_NATIVE, 6);
				break;
		}

		anchor->vtoc_changed++;
		vtoc_ebcdic_enc(p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);
	}

	return 1;
}

static int
dasd_write (const PedDisk* disk)
{
	DasdPartitionData* dasd_data;
	PedPartition* part;
	int i;
	partition_info_t *p;
	LinuxSpecific* arch_specific;
	DasdDiskSpecific* disk_specific;
	struct fdasd_anchor anchor;
	partition_info_t *part_info[USABLE_PARTITIONS];

	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->dev != NULL);

	arch_specific = LINUX_SPECIFIC (disk->dev);
	disk_specific = disk->disk_specific;

	PDEBUG;

	/* If not formated in CDL, don't write anything. */
	if (disk_specific->format_type == 1)
		return 1;

	/* initialize the anchor */
	fdasd_initialize_anchor(&anchor);
	fdasd_get_geometry(disk->dev, &anchor, arch_specific->fd);
	memcpy(anchor.vlabel, &disk_specific->vlabel, sizeof(volume_label_t));
	anchor.vlabel_changed++;

	if ((anchor.geo.cylinders * anchor.geo.heads) > BIG_DISK_SIZE)
		anchor.big_disk++;

	fdasd_recreate_vtoc(&anchor);

	for (i = 1; i <= USABLE_PARTITIONS; i++) {
		unsigned int start, stop;

		PDEBUG;
		part = ped_disk_get_partition(disk, i);
		if (!part)
			continue;

		PDEBUG;

		start = part->geom.start * disk->dev->sector_size
				/ arch_specific->real_sector_size / disk->dev->hw_geom.sectors;
		stop = (part->geom.end + 1)
			   * disk->dev->sector_size / arch_specific->real_sector_size
			   / disk->dev->hw_geom.sectors - 1;

		PDEBUG;
		dasd_data = part->disk_specific;

		p = fdasd_add_partition(&anchor, start, stop);
		if (!p) {
			PDEBUG;
			goto error;
		}
		part_info[i - 1] = p;
		p->type = dasd_data->system;
	}

	PDEBUG;

	if (!fdasd_prepare_labels(&anchor, arch_specific->fd))
		goto error;

	dasd_update_type(disk, &anchor, part_info);
	PDEBUG;

	if (!fdasd_write_labels(&anchor, arch_specific->fd))
		goto error;

	fdasd_cleanup(&anchor);
	return 1;

error:
	PDEBUG;
	fdasd_cleanup(&anchor);
	return 0;
}

static PedPartition*
dasd_partition_new (const PedDisk* disk, PedPartitionType part_type,
                    const PedFileSystemType* fs_type,
                    PedSector start, PedSector end)
{
	PedPartition* part;

	part = _ped_partition_alloc(disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	part->disk_specific = ped_malloc (sizeof (DasdPartitionData));
	return part;

error:
	return 0;
}

static PedPartition*
dasd_partition_duplicate (const PedPartition *part)
{
	PedPartition *new_part;

	new_part = ped_partition_new (part->disk, part->type, part->fs_type,
				      part->geom.start, part->geom.end);
	if (!new_part)
		return NULL;
	new_part->num = part->num;

	memcpy(new_part->disk_specific, part->disk_specific,
	       sizeof(DasdPartitionData));

	return new_part;
}

static void
dasd_partition_destroy (PedPartition* part)
{
	PED_ASSERT(part != NULL);

	if (ped_partition_is_active(part))
		free(part->disk_specific);
	free(part);
}

static int
dasd_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	DasdPartitionData* dasd_data;

	PED_ASSERT(part != NULL);
	PED_ASSERT(part->disk_specific != NULL);
	dasd_data = part->disk_specific;

	switch (flag) {
		case PED_PARTITION_RAID:
			if (state)
				dasd_data->lvm = 0;
			dasd_data->raid = state;
			return ped_partition_set_system(part, part->fs_type);
		case PED_PARTITION_LVM:
			if (state)
				dasd_data->raid = 0;
			dasd_data->lvm = state;
			return ped_partition_set_system(part, part->fs_type);
		default:
			return 0;
	}
}

static int
dasd_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	DasdPartitionData* dasd_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	dasd_data = part->disk_specific;

	switch (flag) {
		case PED_PARTITION_RAID:
			return dasd_data->raid;
		case PED_PARTITION_LVM:
			return dasd_data->lvm;
		default:
			return 0;
	}
}

static int
dasd_partition_is_flag_available (const PedPartition* part,
                                  PedPartitionFlag flag)
{
	switch (flag) {
		case PED_PARTITION_RAID:
			return 1;
		case PED_PARTITION_LVM:
			return 1;
		default:
			return 0;
	}
}


static int
dasd_get_max_primary_partition_count (const PedDisk* disk)
{
	DasdDiskSpecific* disk_specific;

	disk_specific = disk->disk_specific;
	/* If formated in LDL, maximum partition number is 1 */
	if (disk_specific->format_type == 1)
		return 1;

	return USABLE_PARTITIONS;
}

static bool
dasd_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = dasd_get_max_primary_partition_count(disk);
	return true;
}

static PedAlignment*
dasd_get_partition_alignment(const PedDisk *disk)
{
        LinuxSpecific *arch_specific = LINUX_SPECIFIC(disk->dev);
        PedSector sector_size =
                arch_specific->real_sector_size / disk->dev->sector_size;

        return ped_alignment_new(0, disk->dev->hw_geom.sectors * sector_size);
}

static PedConstraint*
_primary_constraint (PedDisk* disk)
{
	PedAlignment start_align;
	PedAlignment end_align;
	PedGeometry	max_geom;
	PedSector sector_size;
	LinuxSpecific* arch_specific;
	DasdDiskSpecific* disk_specific;

	PDEBUG;

	arch_specific = LINUX_SPECIFIC (disk->dev);
	disk_specific = disk->disk_specific;
	sector_size = arch_specific->real_sector_size / disk->dev->sector_size;

	if (!ped_alignment_init (&start_align, 0,
							 disk->dev->hw_geom.sectors * sector_size))
		return NULL;
	if (!ped_alignment_init (&end_align, -1,
						     disk->dev->hw_geom.sectors * sector_size))
		return NULL;
	if (!ped_geometry_init (&max_geom, disk->dev, 0, disk->dev->length))
		return NULL;

	return ped_constraint_new(&start_align, &end_align, &max_geom,
							  &max_geom, 1, disk->dev->length);
}

static int
dasd_partition_align (PedPartition* part, const PedConstraint* constraint)
{
	DasdDiskSpecific* disk_specific;

	PED_ASSERT (part != NULL);

	disk_specific = part->disk->disk_specific;
	/* If formated in LDL, ignore metadata partition */
	if (disk_specific->format_type == 1)
		return 1;

	if (_ped_partition_attempt_align(part, constraint,
								     _primary_constraint(part->disk)))
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
dasd_partition_enumerate (PedPartition* part)
{
	int i;
	PedPartition* p;

	/* never change the partition numbers */
	if (part->num != -1)
		return 1;

	for (i = 1; i <= USABLE_PARTITIONS; i++) {
		p = ped_disk_get_partition (part->disk, i);
		if (!p) {
			part->num = i;
			return 1;
		}
	}

	/* failed to allocate a number */
	ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
						_("Unable to allocate a dasd disklabel slot"));
	return 0;
}

static int
dasd_partition_set_system (PedPartition* part,
                           const PedFileSystemType* fs_type)
{
	DasdPartitionData* dasd_data = part->disk_specific;
	PedSector cyl_size;

	cyl_size=part->disk->dev->hw_geom.sectors * part->disk->dev->hw_geom.heads;
	PDEBUG;

	part->fs_type = fs_type;

	if (dasd_data->lvm) {
		dasd_data->system = PARTITION_LINUX_LVM;
        PDEBUG;
		return 1;
	}

	if (dasd_data->raid) {
		dasd_data->system = PARTITION_LINUX_RAID;
        PDEBUG;
		return 1;
	}

	if (!fs_type) {
		dasd_data->system = PARTITION_LINUX;
        PDEBUG;
	} else if (is_linux_swap (fs_type->name)) {
		dasd_data->system = PARTITION_LINUX_SWAP;
        PDEBUG;
	} else {
		dasd_data->system = PARTITION_LINUX;
        PDEBUG;
	}

	return 1;
}

static int
dasd_alloc_metadata (PedDisk* disk)
{
	PedPartition* new_part;
	PedConstraint* constraint_any = NULL;
	PedSector vtoc_end;
	LinuxSpecific* arch_specific;
	DasdDiskSpecific* disk_specific;
	PedPartition* part = NULL; /* initialize solely to placate gcc */
	PedPartition* new_part2;
	PedSector trailing_meta_start, trailing_meta_end;
	struct fdasd_anchor anchor;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	arch_specific = LINUX_SPECIFIC (disk->dev);
	disk_specific = disk->disk_specific;

	constraint_any = ped_constraint_any (disk->dev);

	/* For LDL or CMS, the leading metadata ends at the sector before
	   the start of the first partition */
	if (disk_specific->format_type == 1) {
	        part = ped_disk_get_partition(disk, 1);
		vtoc_end = part->geom.start - 1;
	}
	else {
                if (disk->dev->type == PED_DEVICE_FILE)
                        arch_specific->real_sector_size = disk->dev->sector_size;
        /* Mark the start of the disk as metadata. */
		vtoc_end = (FIRST_USABLE_TRK * (long long) disk->dev->hw_geom.sectors
				   * (long long) arch_specific->real_sector_size
				   / (long long) disk->dev->sector_size) - 1;
        }

	new_part = ped_partition_new (disk,PED_PARTITION_METADATA,NULL,0,vtoc_end);
	if (!new_part)
		goto error;

	if (!ped_disk_add_partition (disk, new_part, constraint_any)) {
		ped_partition_destroy (new_part);
		goto error;
	}

	if (disk_specific->format_type == 1) {
	   /*
	      For LDL or CMS there may be trailing metadata as well.
	      For example: the last block of a CMS reserved file,
	      the "recomp" area of a CMS minidisk that has been
	      formatted and then formatted again with the RECOMP
	      option specifying fewer than the maximum number of
	      cylinders, a disk that was formatted at one size,
	      backed up, then restored to a larger size disk, etc.
	   */
	   trailing_meta_start = part->geom.end + 1;
	   fdasd_initialize_anchor(&anchor);
	   fdasd_get_geometry(disk->dev, &anchor, arch_specific->fd);
	   trailing_meta_end = (long long) disk->dev->length - 1;
	   fdasd_cleanup(&anchor);
	   if (trailing_meta_end >= trailing_meta_start) {
		new_part2 = ped_partition_new (disk,PED_PARTITION_METADATA,
		   NULL, trailing_meta_start, trailing_meta_end);
		if (!new_part2) {
		   ped_partition_destroy (new_part);
		   goto error;
		}
		if (!ped_disk_add_partition (disk, new_part2,
		   constraint_any)) {
		   ped_partition_destroy (new_part2);
		   ped_partition_destroy (new_part);
		   goto error;
		}
	   }
	}

	ped_constraint_destroy (constraint_any);
	return 1;

error:
	ped_constraint_destroy (constraint_any);
	return 0;
}
