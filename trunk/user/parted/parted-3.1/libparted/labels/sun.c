/* -*- Mode: c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-

    libparted - a library for manipulating disk partitions
    Copyright (C) 2000-2001, 2005, 2007-2012 Free Software Foundation, Inc.

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

    Contributor:  Ben Collins <bcollins@debian.org>
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
#include "verify.h"

/* Most of this came from util-linux's sun support, which was mostly done
   by Jakub Jelinek.  */

#define SUN_DISK_MAGIC		0xDABE	/* Disk magic number */
#define SUN_DISK_MAXPARTITIONS	8

#define SUN_VTOC_VERSION	1
#define SUN_VTOC_SANITY		0x600DDEEE

#define WHOLE_DISK_ID		0x05
#define WHOLE_DISK_PART		2	/* as in 0, 1, 2 (3rd partition) */
#define LINUX_SWAP_ID		0x82

typedef struct _SunRawPartition     SunRawPartition;
typedef struct _SunPartitionInfo    SunPartitionInfo;
typedef struct _SunRawLabel         SunRawLabel;
typedef struct _SunPartitionData    SunPartitionData;
typedef struct _SunDiskData         SunDiskData;

struct __attribute__ ((packed)) _SunRawPartition {
	u_int32_t	start_cylinder; /* where the part starts... */
	u_int32_t	num_sectors;	/* ...and it's length */
};

struct __attribute__ ((packed)) _SunPartitionInfo {
	u_int8_t	spare1;
	u_int8_t	id;		/* Partition type */
	u_int8_t	spare2;
	u_int8_t	flags;		/* Partition flags */
};

struct __attribute__ ((packed)) _SunRawLabel {
	char 		info[128];	/* Informative text string */
	u_int32_t	version;	/* Layout version */
	u_int8_t	volume[8];	/* Volume name */
	u_int16_t	nparts;		/* Number of partitions */
	SunPartitionInfo infos[SUN_DISK_MAXPARTITIONS];
	u_int16_t	padding;	/* Alignment padding */
	u_int32_t	bootinfo[3];	/* Info needed by mboot */
	u_int32_t	sanity;		/* To verify vtoc sanity */
	u_int32_t	reserved[10];	/* Free space */
	u_int32_t	timestamp[8];	/* Partition timestamp */
	u_int32_t	write_reinstruct; /* sectors to skip, writes */
	u_int32_t	read_reinstruct; /* sectors to skip, reads */
	u_int8_t	spare1[148];	/* Padding */
	u_int16_t	rspeed;		/* Disk rotational speed */
	u_int16_t	pcylcount;	/* Physical cylinder count */
	u_int16_t	sparecyl;	/* extra sects per cylinder */
	u_int8_t	spare2[4];	/* More magic... */
	u_int16_t	ilfact;		/* Interleave factor */
	u_int16_t	ncyl;		/* Data cylinder count */
	u_int16_t	nacyl;		/* Alt. cylinder count */
	u_int16_t	ntrks;		/* Tracks per cylinder */
	u_int16_t	nsect;		/* Sectors per track */
	u_int8_t	spare3[4];	/* Even more magic... */
	SunRawPartition partitions[SUN_DISK_MAXPARTITIONS];
	u_int16_t	magic;		/* Magic number */
	u_int16_t	csum;		/* Label xor'd checksum */
};

struct _SunPartitionData {
	u_int8_t		type;
	int			is_boot;
	int			is_root;
	int			is_lvm;
	int			is_raid;
};

struct _SunDiskData {
	PedSector		length; /* This is based on cyl - alt-cyl */
	SunRawLabel		raw_label;
};

static PedDiskType sun_disk_type;

/* Checksum computation */
static void
sun_compute_checksum (SunRawLabel *label)
{
	u_int16_t *ush = (u_int16_t *)label;
	u_int16_t csum = 0;

        while(ush < (u_int16_t *)(&label->csum))
                csum ^= *ush++;
        label->csum = csum;
}

/* Checksum Verification */
static int
sun_verify_checksum (SunRawLabel const *label)
{
	u_int16_t *ush = ((u_int16_t *)(label + 1)) - 1;
	u_int16_t csum = 0;

	while (ush >= (u_int16_t *)label)
		csum ^= *ush--;

	return !csum;
}

static int
sun_probe (const PedDevice *dev)
{
	PED_ASSERT (dev != NULL);

	void *s0;
	if (!ptt_read_sector (dev, 0, &s0))
		return 0;
	SunRawLabel const *label = (void const *) s0;

	int ok = 1;
	/* check magic */
	if (PED_BE16_TO_CPU (label->magic) != SUN_DISK_MAGIC) {
		ok = 0;
	} else {
#ifndef DISCOVER_ONLY
		if (!sun_verify_checksum(label)) {
			ok = 0;
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Corrupted Sun disk label detected."));
		}
	}
#endif

	free (s0);
	return ok;
}

static PedDisk*
sun_alloc (const PedDevice* dev)
{
	PedDisk*	disk;
	SunRawLabel*	label;
	SunDiskData*	sun_specific;
	const PedCHSGeometry*	bios_geom = &dev->bios_geom;
	PedSector	cyl_size = bios_geom->sectors * bios_geom->heads;
	PED_ASSERT (cyl_size != 0);

        disk = _ped_disk_alloc (dev, &sun_disk_type);
	if (!disk)
		goto error;

	disk->disk_specific = (SunDiskData*) ped_malloc (sizeof (SunDiskData));
	if (!disk->disk_specific)
		goto error_free_disk;
	sun_specific = (SunDiskData*) disk->disk_specific;

	PED_ASSERT (bios_geom->cylinders == (PedSector) (dev->length / cyl_size));
	sun_specific->length = ped_round_down_to (dev->length, cyl_size);

	label = &sun_specific->raw_label;
	memset(label, 0, sizeof(SunRawLabel));

	/* #gentoo-sparc people agree that nacyl = 0 is the best option */
	label->magic	= PED_CPU_TO_BE16 (SUN_DISK_MAGIC);
	label->nacyl	= 0;
	label->pcylcount	= PED_CPU_TO_BE16 (bios_geom->cylinders);
	label->rspeed	= PED_CPU_TO_BE16 (5400);
	label->ilfact	= PED_CPU_TO_BE16 (1);
	label->sparecyl	= 0;
	label->ntrks	= PED_CPU_TO_BE16 (bios_geom->heads);
	label->nsect	= PED_CPU_TO_BE16 (bios_geom->sectors);
	label->ncyl	= PED_CPU_TO_BE16 (dev->length / cyl_size);

	label->sanity   = PED_CPU_TO_BE32 (SUN_VTOC_SANITY);
	label->version  = PED_CPU_TO_BE32 (SUN_VTOC_VERSION);
	label->nparts   = PED_CPU_TO_BE16 (SUN_DISK_MAXPARTITIONS);

	/* Add a whole disk partition at a minimum */
	label->infos[WHOLE_DISK_PART].id = WHOLE_DISK_ID;
	label->partitions[WHOLE_DISK_PART].start_cylinder = 0;
	label->partitions[WHOLE_DISK_PART].num_sectors =
		PED_CPU_TO_BE32(sun_specific->length);

	/* Now a neato string to describe this label */
	snprintf(label->info, sizeof(label->info) - 1,
		 "GNU Parted Custom cyl %d alt %d hd %d sec %d",
		 PED_BE16_TO_CPU(label->ncyl),
		 PED_BE16_TO_CPU(label->nacyl),
		 PED_BE16_TO_CPU(label->ntrks),
		 PED_BE16_TO_CPU(label->nsect));

	sun_compute_checksum(label);
	return disk;

error_free_disk:
	_ped_disk_free (disk);
error:
	return NULL;
}

static PedDisk*
sun_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;
	SunDiskData*	new_sun_data;
	SunDiskData*	old_sun_data = (SunDiskData*) disk->disk_specific;

	new_disk = ped_disk_new_fresh (disk->dev, &sun_disk_type);
	if (!new_disk)
		return NULL;

	new_sun_data = (SunDiskData*) new_disk->disk_specific;
	memcpy (new_sun_data, old_sun_data, sizeof (SunDiskData));
	return new_disk;
}

static void
sun_free (PedDisk *disk)
{
	free (disk->disk_specific);
	_ped_disk_free (disk);
}

static int
_check_geometry_sanity (PedDisk* disk, SunRawLabel* label)
{
	PedDevice*	dev = disk->dev;

	if (PED_BE16_TO_CPU(label->nsect) == dev->hw_geom.sectors &&
	    PED_BE16_TO_CPU(label->ntrks) == dev->hw_geom.heads)
		dev->bios_geom = dev->hw_geom;

	if (!!PED_BE16_TO_CPU(label->pcylcount)
	    * !!PED_BE16_TO_CPU(label->ntrks)
	    * !!PED_BE16_TO_CPU(label->nsect) == 0)
		return 0;

	if (PED_BE16_TO_CPU(label->nsect) != dev->bios_geom.sectors ||
	    PED_BE16_TO_CPU(label->ntrks) != dev->bios_geom.heads) {
#ifndef DISCOVER_ONLY
		if (ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The disk CHS geometry (%d,%d,%d) reported "
				  "by the operating system does not match "
				  "the geometry stored on the disk label "
				  "(%d,%d,%d)."),
				dev->bios_geom.cylinders,
			       	dev->bios_geom.heads,
			       	dev->bios_geom.sectors,
				PED_BE16_TO_CPU(label->pcylcount),
				PED_BE16_TO_CPU(label->ntrks),
				PED_BE16_TO_CPU(label->nsect))
			== PED_EXCEPTION_CANCEL)
			return 0;
#endif
		dev->bios_geom.sectors = PED_BE16_TO_CPU(label->nsect);
		dev->bios_geom.heads = PED_BE16_TO_CPU(label->ntrks);
		dev->bios_geom.cylinders = PED_BE16_TO_CPU(label->pcylcount);

		if (dev->bios_geom.sectors * dev->bios_geom.heads
			      	* dev->bios_geom.cylinders > dev->length) {
			if (ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The disk label describes a disk bigger than "
				  "%s."),
				dev->path)
				!= PED_EXCEPTION_IGNORE)
				return 0;
		}
	}
	return 1;
}

static int
sun_read (PedDisk* disk)
{
	SunPartitionData* sun_data;
	SunDiskData* disk_data;
	int i;
	PedPartition* part;
	PedSector end, start, block;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);
	PED_ASSERT (disk->disk_specific != NULL);

	disk_data = (SunDiskData*) disk->disk_specific;

	ped_disk_delete_all (disk);

	void *s0;
	if (!ptt_read_sector (disk->dev, 0, &s0))
		goto error;

	SunRawLabel *label = &disk_data->raw_label;
	verify (sizeof (*label) == 512);
	memcpy (label, s0, sizeof (*label));
	free (s0);

	if (!_check_geometry_sanity (disk, label))
		goto error;

	block = disk->dev->bios_geom.sectors * disk->dev->bios_geom.heads;
	disk_data->length = block * disk->dev->bios_geom.cylinders;

	for (i = 0; i < SUN_DISK_MAXPARTITIONS; i++) {
		if (!PED_BE32_TO_CPU(label->partitions[i].num_sectors))
			continue;
		if (!label->infos[i].id)
			continue;
		if (label->infos[i].id == WHOLE_DISK_ID)
	       		continue;

		start = PED_BE32_TO_CPU(label->partitions[i].start_cylinder)
				    * block;
		end = start
		      + PED_BE32_TO_CPU(label->partitions[i].num_sectors) - 1;

		part = ped_partition_new (disk, PED_PARTITION_NORMAL, NULL,
                                          start, end);
		if (!part)
			goto error;

		sun_data = part->disk_specific;
		sun_data->type = label->infos[i].id;
		sun_data->is_boot = sun_data->type == 0x1;
		sun_data->is_root = sun_data->type == 0x2;
		sun_data->is_lvm = sun_data->type == 0x8e;
		sun_data->is_raid = sun_data->type == 0xfd;

		part->num = i + 1;
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

#ifndef DISCOVER_ONLY
static int
_use_old_info (const PedDisk* disk, const void *sector_0)
{
	SunRawLabel const *old_label = sector_0;

	if (old_label->info[0]
	    && PED_BE16_TO_CPU (old_label->magic) == SUN_DISK_MAGIC) {
		SunDiskData *sun_specific = disk->disk_specific;
		memcpy (&sun_specific->raw_label, sector_0,
                        sizeof (sun_specific->raw_label));
                verify (sizeof (sun_specific->raw_label) == 512);
	}

	return 1;
}

static int
sun_write (const PedDisk* disk)
{
	SunRawLabel*		label;
	SunPartitionData*	sun_data;
	SunDiskData*		disk_data;
	PedPartition*		part;
	int			i;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	void *s0;
	if (!ptt_read_sector (disk->dev, 0, &s0))
		return 0;

	/* Calling _use_old_info here in sun_write
	   above seems wrong, because it modifies *DISK.
	   FIXME: maybe later.  */
	if (!_use_old_info (disk, s0)) {
                free (s0);
		return 0;
        }

	disk_data = (SunDiskData*) disk->disk_specific;
	label = &disk_data->raw_label;

	memset (label->partitions, 0,
		sizeof (SunRawPartition) * SUN_DISK_MAXPARTITIONS);
	memset (label->infos, 0,
		sizeof (SunPartitionInfo) * SUN_DISK_MAXPARTITIONS);

	for (i = 0; i < SUN_DISK_MAXPARTITIONS; i++) {
		part = ped_disk_get_partition (disk, i + 1);

		if (!part && i == WHOLE_DISK_PART) {
			/* Ok, nothing explicitly in the whole disk
			   partition, so let's put it there for safety
			   sake.  */

			label->infos[i].id = WHOLE_DISK_ID;
			label->partitions[i].start_cylinder = 0;
			label->partitions[i].num_sectors =
				PED_CPU_TO_BE32(disk_data->length);
			continue;
		}
		if (!part)
			continue;

		sun_data = part->disk_specific;
		label->infos[i].id = sun_data->type;
		label->partitions[i].start_cylinder
			= PED_CPU_TO_BE32 (part->geom.start
				/ (disk->dev->bios_geom.sectors
				       	* disk->dev->bios_geom.heads));
		label->partitions[i].num_sectors
			= PED_CPU_TO_BE32 (part->geom.end
					   - part->geom.start + 1);
	}

	/* We assume the harddrive is always right, and that the label may
	   be wrong. I don't think this will cause any problems, since the
	   cylinder count is always enforced by our alignment, and we
	   sanity checked the sectors/heads when we detected the device. The
	   worst that could happen here is that the drive seems bigger or
	   smaller than it really is, but we'll have that problem even if we
	   don't do this.  */

	if (disk->dev->bios_geom.cylinders > 65536) {
		ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_IGNORE,
			_("The disk has %d cylinders, which is greater than "
			  "the maximum of 65536."),
			disk->dev->bios_geom.cylinders);
	}

	label->pcylcount = PED_CPU_TO_BE16 (disk->dev->bios_geom.cylinders);
	label->ncyl = PED_CPU_TO_BE16 (disk->dev->bios_geom.cylinders
			- PED_BE16_TO_CPU (label->nacyl));

	sun_compute_checksum (label);

        verify (sizeof *label == 512);
        memcpy (s0, label, sizeof *label);
	int write_ok = ped_device_write (disk->dev, s0, 0, 1);
	free (s0);

	if (write_ok)
		return ped_device_sync (disk->dev);

	return 0;
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
sun_partition_new (const PedDisk* disk, PedPartitionType part_type,
		   const PedFileSystemType* fs_type,
		   PedSector start, PedSector end)
{
	PedPartition*		part;
	SunPartitionData*	sun_data;

	part = _ped_partition_alloc (disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	if (ped_partition_is_active (part)) {
		part->disk_specific
		       	= sun_data = ped_malloc (sizeof (SunPartitionData));
		if (!sun_data)
			goto error_free_part;
		sun_data->type = 0;
		sun_data->is_boot = 0;
		sun_data->is_root = 0;
		sun_data->is_lvm = 0;
		sun_data->is_raid = 0;
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
sun_partition_duplicate (const PedPartition* part)
{
	PedPartition*		new_part;
	SunPartitionData*	new_sun_data;
	SunPartitionData*	old_sun_data;

	new_part = ped_partition_new (part->disk, part->type,
				      part->fs_type, part->geom.start,
				      part->geom.end);
	if (!new_part)
		return NULL;
	new_part->num = part->num;

	old_sun_data = (SunPartitionData*) part->disk_specific;
	new_sun_data = (SunPartitionData*) new_part->disk_specific;
	new_sun_data->type = old_sun_data->type;
	new_sun_data->is_boot = old_sun_data->is_boot;
	new_sun_data->is_root = old_sun_data->is_root;
	new_sun_data->is_lvm = old_sun_data->is_lvm;
	new_sun_data->is_raid = old_sun_data->is_raid;
	return new_part;
}

static void
sun_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	if (ped_partition_is_active (part))
		free (part->disk_specific);
	free (part);
}

static int
sun_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
	SunPartitionData*		sun_data = part->disk_specific;

	part->fs_type = fs_type;

	if (sun_data->is_boot) {
		sun_data->type = 0x1;
		return 1;
	}
	if (sun_data->is_root) {
		sun_data->type = 0x2;
		return 1;
	}
	if (sun_data->is_lvm) {
		sun_data->type = 0x8e;
		return 1;
	}
	if (sun_data->is_raid) {
		sun_data->type = 0xfd;
		return 1;
	}

	sun_data->type = 0x83;
	if (fs_type) {
		if (is_linux_swap (fs_type->name))
			sun_data->type = 0x82;
		else if (!strcmp (fs_type->name, "ufs"))
			sun_data->type = 0x6;
	}

	return 1;
}

static int
sun_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	SunPartitionData*		sun_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	PED_ASSERT (ped_partition_is_flag_available (part, flag));

	sun_data = part->disk_specific;

	switch (flag) {
		case PED_PARTITION_BOOT:
			sun_data->is_boot = state;
			if (state) {
				sun_data->is_lvm = 0;
				sun_data->is_raid = 0;
				sun_data->is_root = 0;
			}
			return ped_partition_set_system (part, part->fs_type);

		case PED_PARTITION_ROOT:
			sun_data->is_root = state;
			if (state) {
				sun_data->is_boot = 0;
				sun_data->is_lvm = 0;
				sun_data->is_raid = 0;
			}
			return ped_partition_set_system (part, part->fs_type);

		case PED_PARTITION_LVM:
			sun_data->is_lvm = state;
			if (state) {
				sun_data->is_boot = 0;
				sun_data->is_raid = 0;
				sun_data->is_root = 0;
			}
			return ped_partition_set_system (part, part->fs_type);

		case PED_PARTITION_RAID:
			sun_data->is_raid = state;
			if (state) {
				sun_data->is_boot = 0;
				sun_data->is_lvm = 0;
				sun_data->is_root = 0;
			}
			return ped_partition_set_system (part, part->fs_type);

		default:
			return 0;
	}
}


static int _GL_ATTRIBUTE_PURE
sun_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	SunPartitionData*       sun_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	sun_data = part->disk_specific;

	switch (flag) {
		case PED_PARTITION_BOOT:
			return sun_data->is_boot;
		case PED_PARTITION_ROOT:
			return sun_data->is_root;
		case PED_PARTITION_LVM:
			return sun_data->is_lvm;
		case PED_PARTITION_RAID:
			return sun_data->is_raid;

		default:
			return 0;
	}
}


static int
sun_partition_is_flag_available (const PedPartition* part,
				 PedPartitionFlag flag)
{
	switch (flag) {
		case PED_PARTITION_BOOT:
		case PED_PARTITION_ROOT:
		case PED_PARTITION_LVM:
		case PED_PARTITION_RAID:
			return 1;

		default:
			return 0;
	}
}

static bool
sun_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = SUN_DISK_MAXPARTITIONS;
	return true;
}

static int
sun_get_max_primary_partition_count (const PedDisk* disk)
{
	return SUN_DISK_MAXPARTITIONS;
}

static PedAlignment*
sun_get_partition_alignment(const PedDisk *disk)
{
	PedSector block =
		disk->dev->hw_geom.sectors * disk->dev->hw_geom.heads;

        return ped_alignment_new(0, block);
}

static PedConstraint*
_get_strict_constraint (PedDisk* disk)
{
	PedDevice*	dev = disk->dev;
        PedAlignment    start_align;
        PedAlignment    end_align;
        PedGeometry     max_geom;
	SunDiskData*	disk_data = disk->disk_specific;
	PedSector	block = dev->bios_geom.sectors * dev->bios_geom.heads;

        if (!ped_alignment_init (&start_align, 0, block))
                return NULL;
        if (!ped_alignment_init (&end_align, -1, block))
                return NULL;
	if (!ped_geometry_init (&max_geom, dev, 0, disk_data->length))
		return NULL;

        return ped_constraint_new (&start_align, &end_align, &max_geom,
                                   &max_geom, 1, dev->length);
}

static PedConstraint*
_get_lax_constraint (PedDisk* disk)
{
	PedDevice*	dev = disk->dev;
        PedAlignment    start_align;
        PedGeometry     max_geom;
	SunDiskData*	disk_data = disk->disk_specific;
	PedSector	block = dev->bios_geom.sectors * dev->bios_geom.heads;

        if (!ped_alignment_init (&start_align, 0, block))
                return NULL;
	if (!ped_geometry_init (&max_geom, dev, 0, disk_data->length))
		return NULL;

        return ped_constraint_new (&start_align, ped_alignment_any, &max_geom,
                                   &max_geom, 1, dev->length);
}

/* _get_strict_constraint() will align the partition to the end of the cylinder.
 * This isn't required, but since partitions must start at the start of the
 * cylinder, space between the end of a partition and the end of a cylinder
 * is unusable, so there's no point wasting space!
 *	However, if they really insist (via constraint)... which they will
 * if they're reading a weird table of the disk... then we allow the end to
 * be anywhere, with _get_lax_constraint()
 */
static int
sun_partition_align (PedPartition* part, const PedConstraint* constraint)
{
        PED_ASSERT (part != NULL);

        if (_ped_partition_attempt_align (part, constraint,
                                          _get_strict_constraint (part->disk)))
                return 1;
        if (_ped_partition_attempt_align (part, constraint,
                                          _get_lax_constraint (part->disk)))
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
sun_partition_enumerate (PedPartition* part)
{
	int i;
	PedPartition* p;

	/* never change the partition numbers */
	if (part->num != -1)
		return 1;
	for (i = 1; i <= SUN_DISK_MAXPARTITIONS; i++) {
		/* skip the Whole Disk partition for now */
		if (i == WHOLE_DISK_PART + 1)
			continue;
		p = ped_disk_get_partition (part->disk, i);
		if (!p) {
			part->num = i;
			return 1;
		}
	}

#ifndef DISCOVER_ONLY
	/* Ok, now allocate the Whole disk if it isn't already */
	p = ped_disk_get_partition (part->disk, WHOLE_DISK_PART + 1);
	if (!p) {
		int j = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("The Whole Disk partition is the only "
				  "available one left.  Generally, it is not a "
				  "good idea to overwrite this partition with "
				  "a real one.  Solaris may not be able to "
				  "boot without it, and SILO (the sparc boot "
				  "loader) appreciates it as well."));
		if (j == PED_EXCEPTION_IGNORE) {
			/* bad bad bad...you will suffer your own fate */
			part->num = WHOLE_DISK_PART + 1;
			return 1;
		}
	}

	/* failed to allocate a number, this means we are full */
	ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			     _("Sun disk label is full."));
#endif
	return 0;
}

static int
sun_alloc_metadata (PedDisk* disk)
{
	PedPartition*	new_part;
	SunDiskData*	disk_data;
	PedConstraint*	constraint_any;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->disk_specific != NULL);
	PED_ASSERT (disk->dev != NULL);

	constraint_any = ped_constraint_any (disk->dev);

	/* Sun disk label does not need to allocate a sector. The disk
	   label is contained within the first 512 bytes, which should not
	   be overwritten by any boot loader or superblock. It is safe for
	   most partitions to start at sector 0. We do however, allocate
	   the space used by alt-cyl's, since we cannot use those. Put them
	   at the end of the disk.  */

	disk_data = disk->disk_specific;

	if (disk->dev->length <= 0 ||
	    disk_data->length <= 0 ||
	    disk->dev->length == disk_data->length)
		goto error;

	new_part = ped_partition_new (disk, PED_PARTITION_METADATA, NULL,
			disk_data->length, disk->dev->length - 1);
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
PT_define_limit_functions (sun)

static PedDiskOps sun_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (sun_write),

	get_partition_alignment: sun_get_partition_alignment,

	partition_set_name:		NULL,
	partition_get_name:		NULL,

	PT_op_function_initializers (sun)
};

static PedDiskType sun_disk_type = {
	next:		NULL,
	name:		"sun",
	ops:		&sun_disk_ops,
	features:	0
};

void
ped_disk_sun_init ()
{
	PED_ASSERT (sizeof (SunRawLabel) == 512);
	ped_disk_type_register (&sun_disk_type);
}

void
ped_disk_sun_done ()
{
	ped_disk_type_unregister (&sun_disk_type);
}
