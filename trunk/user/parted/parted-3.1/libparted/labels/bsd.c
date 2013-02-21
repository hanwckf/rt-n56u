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

    Contributor:  Matt Wilson <msw@redhat.com>
*/

#include <config.h>

#include <stdbool.h>
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

/* struct's & #define's stolen from libfdisk, which probably came from
 * Linux...
 */

#define BSD_DISKMAGIC	(0x82564557UL)	/* The disk magic number */
#define BSD_MAXPARTITIONS	8
#define BSD_FS_UNUSED		0	/* disklabel unused partition entry ID */
#define BSD_LABEL_OFFSET	64

#define	BSD_DTYPE_SMD		1		/* SMD, XSMD; VAX hp/up */
#define	BSD_DTYPE_MSCP		2		/* MSCP */
#define	BSD_DTYPE_DEC		3		/* other DEC (rk, rl) */
#define	BSD_DTYPE_SCSI		4		/* SCSI */
#define	BSD_DTYPE_ESDI		5		/* ESDI interface */
#define	BSD_DTYPE_ST506		6		/* ST506 etc. */
#define	BSD_DTYPE_HPIB		7		/* CS/80 on HP-IB */
#define BSD_DTYPE_HPFL		8		/* HP Fiber-link */
#define	BSD_DTYPE_FLOPPY	10		/* floppy */

#define	BSD_BBSIZE	8192		/* size of boot area, with label */
#define	BSD_SBSIZE	8192		/* max size of fs superblock */

typedef struct _BSDRawPartition		BSDRawPartition;
typedef struct _BSDRawLabel		BSDRawLabel;

struct _BSDRawPartition {		/* the partition table */
	uint32_t	p_size;		/* number of sectors in partition */
	uint32_t	p_offset;	/* starting sector */
	uint32_t	p_fsize;	/* file system basic fragment size */
	uint8_t		p_fstype;	/* file system type, see below */
	uint8_t		p_frag;		/* file system fragments per block */
	uint16_t	p_cpg;		/* file system cylinders per group */
} __attribute__((packed));

struct _BSDRawLabel {
	uint32_t	d_magic;		/* the magic number */
	int16_t		d_type;			/* drive type */
	int16_t		d_subtype;		/* controller/d_type specific */
	int8_t		d_typename[16];		/* type name, e.g. "eagle" */
	int8_t		d_packname[16];		/* pack identifier */
	uint32_t	d_secsize;		/* # of bytes per sector */
	uint32_t	d_nsectors;		/* # of data sectors per track */
	uint32_t	d_ntracks;		/* # of tracks per cylinder */
	uint32_t	d_ncylinders;		/* # of data cylinders per unit */
	uint32_t	d_secpercyl;		/* # of data sectors per cylinder */
	uint32_t	d_secperunit;		/* # of data sectors per unit */
	uint16_t	d_sparespertrack;	/* # of spare sectors per track */
	uint16_t	d_sparespercyl;		/* # of spare sectors per cylinder */
	uint32_t	d_acylinders;		/* # of alt. cylinders per unit */
	uint16_t	d_rpm;			/* rotational speed */
	uint16_t	d_interleave;		/* hardware sector interleave */
	uint16_t	d_trackskew;		/* sector 0 skew, per track */
	uint16_t	d_cylskew;		/* sector 0 skew, per cylinder */
	uint32_t	d_headswitch;		/* head switch time, usec */
	uint32_t	d_trkseek;		/* track-to-track seek, usec */
	uint32_t	d_flags;		/* generic flags */
#define NDDATA 5
	uint32_t	d_drivedata[NDDATA];	/* drive-type specific information */
#define NSPARE 5
	uint32_t	d_spare[NSPARE];	/* reserved for future use */
	uint32_t	d_magic2;		/* the magic number (again) */
	uint16_t	d_checksum;		/* xor of data incl. partitions */

	/* file system and partition information: */
	uint16_t	d_npartitions;		/* number of partitions in following */
	uint32_t	d_bbsize;		/* size of boot area at sn0, bytes */
	uint32_t	d_sbsize;		/* max size of fs superblock, bytes */
	BSDRawPartition d_partitions[BSD_MAXPARTITIONS];	/* actually may be more */
} __attribute__((packed));

typedef struct {
	char		boot_code [512];
} BSDDiskData;

typedef struct {
	uint8_t		type;
	int			boot;
	int			raid;
	int			lvm;
} BSDPartitionData;

static PedDiskType bsd_disk_type;

/* XXX fixme: endian? */
static unsigned short
xbsd_dkcksum (BSDRawLabel *lp) {
	unsigned short *start, *end;
	unsigned short sum = 0;

	lp->d_checksum = 0;
	start = (u_short*) lp;
	end = (u_short*) &lp->d_partitions [
				PED_LE16_TO_CPU (lp->d_npartitions)];
	while (start < end)
		sum ^= *start++;
	return sum;
}

/* XXX fixme: endian? */
static void
alpha_bootblock_checksum (char *boot) {
	uint64_t *dp, sum;
	int i;

	dp = (uint64_t *)boot;
	sum = 0;
	for (i = 0; i < 63; i++)
		sum += dp[i];
	dp[63] = sum;
}

static int
bsd_probe (const PedDevice *dev)
{
	BSDRawLabel	*partition;

	PED_ASSERT (dev != NULL);

        if (dev->sector_size < 512)
                return 0;

	void *label;
	if (!ptt_read_sector (dev, 0, &label))
		return 0;

	partition = (BSDRawLabel *) ((char *) label + BSD_LABEL_OFFSET);

	alpha_bootblock_checksum(label);

	/* check magic */
        bool found = PED_LE32_TO_CPU (partition->d_magic) == BSD_DISKMAGIC;
	free (label);
	return found;
}

static PedDisk*
bsd_alloc (const PedDevice* dev)
{
	PedDisk*	disk;
	BSDDiskData*	bsd_specific;
	BSDRawLabel*	label;

	PED_ASSERT(dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

	disk = _ped_disk_alloc ((PedDevice*)dev, &bsd_disk_type);
	if (!disk)
		goto error;
	disk->disk_specific = bsd_specific = ped_malloc (sizeof (BSDDiskData));
	if (!bsd_specific)
		goto error_free_disk;
        /* Initialize the first byte to zero, so that the code in bsd_write
           knows to call _probe_and_add_boot_code.  Initializing all of the
           remaining buffer is a little wasteful, but the alternative is to
           figure out why a block at offset 340 would otherwise be used
           uninitialized.  */
	memset(bsd_specific->boot_code, 0, sizeof (bsd_specific->boot_code));

	label = (BSDRawLabel*) (bsd_specific->boot_code + BSD_LABEL_OFFSET);

	label->d_magic = PED_CPU_TO_LE32 (BSD_DISKMAGIC);
	label->d_type = PED_CPU_TO_LE16 (BSD_DTYPE_SCSI);
	label->d_flags = 0;
	label->d_secsize = PED_CPU_TO_LE16 (dev->sector_size);
	label->d_nsectors = PED_CPU_TO_LE32 (dev->bios_geom.sectors);
	label->d_ntracks = PED_CPU_TO_LE32 (dev->bios_geom.heads);
	label->d_ncylinders = PED_CPU_TO_LE32 (dev->bios_geom.cylinders);
	label->d_secpercyl = PED_CPU_TO_LE32 (dev->bios_geom.sectors
						* dev->bios_geom.heads);
	label->d_secperunit
		= PED_CPU_TO_LE32 (dev->bios_geom.sectors
				   * dev->bios_geom.heads
				   * dev->bios_geom.cylinders);

	label->d_rpm = PED_CPU_TO_LE16 (3600);
	label->d_interleave = PED_CPU_TO_LE16 (1);;
	label->d_trackskew = 0;
	label->d_cylskew = 0;
	label->d_headswitch = 0;
	label->d_trkseek = 0;

	label->d_magic2 = PED_CPU_TO_LE32 (BSD_DISKMAGIC);
	label->d_bbsize = PED_CPU_TO_LE32 (BSD_BBSIZE);
	label->d_sbsize = PED_CPU_TO_LE32 (BSD_SBSIZE);

	label->d_npartitions = 0;
	label->d_checksum = xbsd_dkcksum (label);
	return disk;

error_free_disk:
	free (disk);
error:
	return NULL;
}

static PedDisk*
bsd_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;
	BSDDiskData*	new_bsd_data;
	BSDDiskData*	old_bsd_data = (BSDDiskData*) disk->disk_specific;

	new_disk = ped_disk_new_fresh (disk->dev, &bsd_disk_type);
	if (!new_disk)
		return NULL;

	new_bsd_data = (BSDDiskData*) new_disk->disk_specific;
	memcpy (new_bsd_data->boot_code, old_bsd_data->boot_code, 512);
	return new_disk;
}

static void
bsd_free (PedDisk* disk)
{
	free (disk->disk_specific);
	_ped_disk_free (disk);
}

static int
bsd_read (PedDisk* disk)
{
	BSDDiskData*	bsd_specific = (BSDDiskData*) disk->disk_specific;
	BSDRawLabel*	label;
	int 		i;

	ped_disk_delete_all (disk);

	void *s0;
	if (!ptt_read_sector (disk->dev, 0, &s0))
		return 0;

	memcpy (bsd_specific->boot_code, s0, sizeof (bsd_specific->boot_code));
	free (s0);

	label = (BSDRawLabel *) (bsd_specific->boot_code + BSD_LABEL_OFFSET);

	for (i = 1; i <= BSD_MAXPARTITIONS; i++) {
		PedPartition* 		part;
		BSDPartitionData*	bsd_part_data;
		PedSector		start;
		PedSector		end;

		if (!label->d_partitions[i - 1].p_size
		    || !label->d_partitions[i - 1].p_fstype)
			continue;
		start = PED_LE32_TO_CPU(label->d_partitions[i - 1].p_offset);
		end = PED_LE32_TO_CPU(label->d_partitions[i - 1].p_offset)
		      + PED_LE32_TO_CPU(label->d_partitions[i - 1].p_size) - 1;
		part = ped_partition_new (disk, PED_PARTITION_NORMAL,
                                          NULL, start, end);
		if (!part)
			goto error;
		bsd_part_data = part->disk_specific;
		bsd_part_data->type = label->d_partitions[i - 1].p_fstype;
		part->num = i;
		part->fs_type = ped_file_system_probe (&part->geom);

		PedConstraint *constraint_exact
			= ped_constraint_exact (&part->geom);
		if (constraint_exact == NULL)
			goto error;
		bool ok = ped_disk_add_partition (disk, part, constraint_exact);
		ped_constraint_destroy (constraint_exact);
		if (!ok)
			goto error;
	}

	return 1;

error:
	return 0;
}

static void
_probe_and_add_boot_code (const PedDisk* disk)
{
	void *s0;
	if (!ptt_read_sector (disk->dev, 0, &s0))
		return;
	char *old_boot_code = s0;
	BSDRawLabel *old_label
                = (BSDRawLabel*) (old_boot_code + BSD_LABEL_OFFSET);

	if (old_boot_code [0]
	    && old_label->d_magic == PED_CPU_TO_LE32 (BSD_DISKMAGIC)) {
		BSDDiskData *bsd_specific = (BSDDiskData*) disk->disk_specific;
		memcpy (bsd_specific->boot_code, old_boot_code,
                        sizeof (BSDDiskData));
        }
	free (s0);
}

#ifndef DISCOVER_ONLY
static int
bsd_write (const PedDisk* disk)
{
	BSDDiskData*		bsd_specific;
	BSDRawLabel*		label;
	BSDPartitionData*	bsd_data;
	PedPartition*		part;
	int			i;
	int			max_part = 0;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	bsd_specific = (BSDDiskData*) disk->disk_specific;
	label = (BSDRawLabel *) (bsd_specific->boot_code + BSD_LABEL_OFFSET);

	if (!bsd_specific->boot_code [0])
		_probe_and_add_boot_code (disk);

	memset (label->d_partitions, 0,
		sizeof (BSDRawPartition) * BSD_MAXPARTITIONS);

	for (i = 1; i <= BSD_MAXPARTITIONS; i++) {
		part = ped_disk_get_partition (disk, i);
		if (!part)
			continue;
		bsd_data = part->disk_specific;
		label->d_partitions[i - 1].p_fstype = bsd_data->type;
		label->d_partitions[i - 1].p_offset
			= PED_CPU_TO_LE32 (part->geom.start);
		label->d_partitions[i - 1].p_size
			= PED_CPU_TO_LE32 (part->geom.length);
		max_part = i;
	}

	label->d_npartitions = PED_CPU_TO_LE16 (max_part) + 1;
	label->d_checksum = xbsd_dkcksum (label);

	alpha_bootblock_checksum (bsd_specific->boot_code);

        if (!ptt_write_sector (disk, bsd_specific->boot_code,
                               sizeof (BSDDiskData)))
                goto error;
	return ped_device_sync (disk->dev);

error:
	return 0;
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
bsd_partition_new (const PedDisk* disk, PedPartitionType part_type,
		   const PedFileSystemType* fs_type,
		   PedSector start, PedSector end)
{
	PedPartition*		part;
	BSDPartitionData*	bsd_data;

	part = _ped_partition_alloc (disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	if (ped_partition_is_active (part)) {
		part->disk_specific
		       	= bsd_data = ped_malloc (sizeof (BSDPartitionData));
		if (!bsd_data)
			goto error_free_part;
		bsd_data->type = 0;
		bsd_data->boot = 0;
		bsd_data->raid = 0;
		bsd_data->lvm  = 0;
	} else {
		part->disk_specific = NULL;
	}
	return part;

error_free_part:
	free (part);
error:
	return 0;
}

static PedPartition*
bsd_partition_duplicate (const PedPartition* part)
{
	PedPartition*		new_part;
	BSDPartitionData*	new_bsd_data;
	BSDPartitionData*	old_bsd_data;

	new_part = ped_partition_new (part->disk, part->type,
				      part->fs_type, part->geom.start,
				      part->geom.end);
	if (!new_part)
		return NULL;
	new_part->num = part->num;

	old_bsd_data = (BSDPartitionData*) part->disk_specific;
	new_bsd_data = (BSDPartitionData*) new_part->disk_specific;
	new_bsd_data->type = old_bsd_data->type;
	new_bsd_data->boot = old_bsd_data->boot;
	new_bsd_data->raid = old_bsd_data->raid;
	new_bsd_data->lvm = old_bsd_data->lvm;
	return new_part;
}

static void
bsd_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	if (ped_partition_is_active (part))
		free (part->disk_specific);
	_ped_partition_free (part);
}

static int
bsd_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
	BSDPartitionData* bsd_data = part->disk_specific;

	part->fs_type = fs_type;

	if (!fs_type)
		bsd_data->type = 0x8;
	else if (is_linux_swap (fs_type->name))
		bsd_data->type = 0x1;
	else
		bsd_data->type = 0x8;

	return 1;
}

static int
bsd_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
//	PedPartition*		walk; // since -Werror, this unused variable would break build
	BSDPartitionData*	bsd_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	PED_ASSERT (part->disk != NULL);

	bsd_data = part->disk_specific;

	switch (flag) {
		case PED_PARTITION_BOOT:
			bsd_data->boot = state;
			return 1;
		case PED_PARTITION_RAID:
			if (state) {
				bsd_data->lvm = 0;
			}
			bsd_data->raid = state;
			return 1;
		case PED_PARTITION_LVM:
			if (state) {
				bsd_data->raid = 0;
			}
			bsd_data->lvm = state;
		default:
			;
	}
	return 0;
}

static int _GL_ATTRIBUTE_PURE
bsd_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	BSDPartitionData*		bsd_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	bsd_data = part->disk_specific;
	switch (flag) {
		case PED_PARTITION_BOOT:
			return bsd_data->boot;

		case PED_PARTITION_RAID:
			return bsd_data->raid;

		case PED_PARTITION_LVM:
			return bsd_data->lvm;

		default:
			;
	}
	return 0;
}

static int
bsd_partition_is_flag_available (const PedPartition* part,
				 PedPartitionFlag flag)
{
	switch (flag) {
		case PED_PARTITION_BOOT:
		case PED_PARTITION_RAID:
		case PED_PARTITION_LVM:
			return 1;
		default:
			;
	}
	return 0;
}


static int
bsd_get_max_primary_partition_count (const PedDisk* disk)
{
	return BSD_MAXPARTITIONS;
}

static bool
bsd_get_max_supported_partition_count(const PedDisk* disk, int *max_n)
{
	*max_n = BSD_MAXPARTITIONS;
	return true;
}

static PedConstraint*
_get_constraint (const PedDevice* dev)
{
	PedGeometry	max;

	ped_geometry_init (&max, dev, 1, dev->length - 1);
	return ped_constraint_new_from_max (&max);
}

static int
bsd_partition_align (PedPartition* part, const PedConstraint* constraint)
{
	if (_ped_partition_attempt_align (part, constraint,
					  _get_constraint (part->disk->dev)))
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
bsd_partition_enumerate (PedPartition* part)
{
	int i;
	PedPartition* p;

	/* never change the partition numbers */
	if (part->num != -1)
		return 1;
	for (i = 1; i <= BSD_MAXPARTITIONS; i++) {
		p = ped_disk_get_partition (part->disk, i);
		if (!p) {
			part->num = i;
			return 1;
		}
	}

	/* failed to allocate a number */
#ifndef DISCOVER_ONLY
	ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			     _("Unable to allocate a bsd disklabel slot."));
#endif
	return 0;
}

static int
bsd_alloc_metadata (PedDisk* disk)
{
	PedPartition*		new_part;
	PedConstraint*		constraint_any = NULL;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	constraint_any = ped_constraint_any (disk->dev);

	/* allocate 1 sector for the disk label at the start */
	new_part = ped_partition_new (disk, PED_PARTITION_METADATA, NULL, 0, 0);
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

#include "pt-common.h"
PT_define_limit_functions (bsd)

static PedDiskOps bsd_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (bsd_write),

	partition_set_name:	NULL,
	partition_get_name:	NULL,

	PT_op_function_initializers (bsd)
};

static PedDiskType bsd_disk_type = {
	next:		NULL,
	name:		"bsd",
	ops:		&bsd_disk_ops,
	features:	0
};

void
ped_disk_bsd_init ()
{
	PED_ASSERT (sizeof (BSDRawPartition) == 16);
	PED_ASSERT (sizeof (BSDRawLabel) == 276);

	ped_disk_type_register (&bsd_disk_type);
}

void
ped_disk_bsd_done ()
{
	ped_disk_type_unregister (&bsd_disk_type);
}
