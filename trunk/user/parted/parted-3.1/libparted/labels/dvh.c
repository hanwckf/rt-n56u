/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2001-2002, 2005, 2007-2012 Free Software Foundation, Inc.

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

#include "dvh.h"
#include "pt-tools.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

/* Default size for volhdr part, same val as IRIX's fx uses */
#define PTYPE_VOLHDR_DFLTSZ 4096

/* Partition numbers that seem to be strongly held convention */
#define PNUM_VOLHDR 8
#define PNUM_VOLUME 10

/* Other notes of interest:
 *  PED_PARTITION_EXTENDED is used for volume headers
 *  PED_PARTITION_LOGICAL  is used for bootfiles
 *  PED_PARTITION_NORMAL   is used for all else
 */

typedef struct _DVHDiskData {
	struct device_parameters	dev_params;
	int				swap;	/* part num of swap, 0=none */
	int				root;	/* part num of root, 0=none */
	int				boot;	/* part num of boot, 0=none */
} DVHDiskData;

typedef struct _DVHPartData {
	int	type;
	char	name[VDNAMESIZE + 1];	/* boot volumes only */
	int	real_file_size;		/* boot volumes only */
} DVHPartData;

static PedDiskType dvh_disk_type;

static int
dvh_probe (const PedDevice *dev)
{
	struct volume_header *vh;

	void *label;
	if (!ptt_read_sector (dev, 0, &label))
		return 0;

	vh = (struct volume_header *) label;

        bool found = PED_BE32_TO_CPU (vh->vh_magic) == VHMAGIC;
	free (label);
	return found;
}

static PedDisk*
dvh_alloc (const PedDevice* dev)
{
	PedDisk*	disk;
	DVHDiskData*	dvh_disk_data;
	PedPartition*	volume_part;
	PedConstraint*	constraint_any;

	disk = _ped_disk_alloc (dev, &dvh_disk_type);
	if (!disk)
		goto error;

	disk->disk_specific = dvh_disk_data
		= ped_malloc (sizeof (DVHDiskData));
	if (!dvh_disk_data)
		goto error_free_disk;

	memset (&dvh_disk_data->dev_params, 0,
		sizeof (struct device_parameters));
	dvh_disk_data->swap = 0;
	dvh_disk_data->root = 0;
	dvh_disk_data->boot = 0;

	volume_part = ped_partition_new (disk, PED_PARTITION_EXTENDED, NULL,
					 0, PTYPE_VOLHDR_DFLTSZ - 1);
	if (!volume_part)
		goto error_free_disk_specific;
	volume_part->num = PNUM_VOLHDR + 1;
	constraint_any = ped_constraint_any (dev);
	if (!ped_disk_add_partition (disk, volume_part, constraint_any))
		goto error_destroy_constraint_any;
	ped_constraint_destroy (constraint_any);
	return disk;

error_destroy_constraint_any:
	ped_constraint_destroy (constraint_any);
	ped_partition_destroy (volume_part);
error_free_disk_specific:
	free (disk->disk_specific);
error_free_disk:
	free (disk);
error:
	return NULL;
}

static PedDisk*
dvh_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;
	DVHDiskData*	new_dvh_disk_data;
	DVHDiskData*	old_dvh_disk_data = disk->disk_specific;

	PED_ASSERT (old_dvh_disk_data != NULL);

	new_disk = ped_disk_new_fresh (disk->dev, &dvh_disk_type);
	if (!new_disk)
		goto error;

	new_disk->disk_specific = new_dvh_disk_data
		= ped_malloc (sizeof (DVHDiskData));
	if (!new_dvh_disk_data)
		goto error_free_new_disk;

	new_dvh_disk_data->dev_params = old_dvh_disk_data->dev_params;
	return new_disk;

error_free_new_disk:
	free (new_disk);
error:
	return NULL;
}

static void
dvh_free (PedDisk* disk)
{
	free (disk->disk_specific);
	_ped_disk_free (disk);
}

/* two's complement 32-bit checksum */
static uint32_t _GL_ATTRIBUTE_PURE
_checksum (const uint32_t* base, size_t size)
{
	uint32_t	sum = 0;
	size_t		i;

	for (i = 0; i < size / sizeof (uint32_t); i++)
		sum = sum - PED_BE32_TO_CPU (base[i]);

	return sum;
}

/* try to make a reasonable volume header partition... */
static PedExceptionOption
_handle_no_volume_header (PedDisk* disk)
{
	PedExceptionOption	ret;
	PedPartition*		part;
	PedConstraint*		constraint;

	switch (ped_exception_throw (
		PED_EXCEPTION_WARNING,
		PED_EXCEPTION_FIX + PED_EXCEPTION_CANCEL,
		_("%s has no extended partition (volume header partition)."),
		disk->dev->path)) {
		case PED_EXCEPTION_UNHANDLED:
		case PED_EXCEPTION_FIX:
		default:
			part = ped_partition_new (
				disk, PED_PARTITION_EXTENDED, NULL,
				0, PTYPE_VOLHDR_DFLTSZ - 1);
			if (!part)
				goto error;
			part->num = PNUM_VOLHDR + 1;
			constraint = ped_constraint_any (part->disk->dev);
			if (!constraint)
				goto error_destroy_part;
			if (!ped_disk_add_partition (disk, part, constraint))
				goto error_destroy_constraint;
			ped_constraint_destroy (constraint);
			ret = PED_EXCEPTION_FIX;
			break;

		case PED_EXCEPTION_CANCEL:
			goto error;
	}
	return ret;

error_destroy_constraint:
	ped_constraint_destroy (constraint);
error_destroy_part:
	ped_partition_destroy (part);
error:
	return PED_EXCEPTION_CANCEL;
}

static PedPartition*
_parse_partition (PedDisk* disk, struct partition_table* pt)
{
	PedPartition*	part;
	DVHPartData*	dvh_part_data;
	PedSector	start = PED_BE32_TO_CPU (pt->pt_firstlbn);
	PedSector	length = PED_BE32_TO_CPU (pt->pt_nblks);

	part = ped_partition_new (disk,
			          pt->pt_type ? 0 : PED_PARTITION_EXTENDED,
				  NULL,
				  start, start + length - 1);
	if (!part)
		return NULL;

	dvh_part_data = part->disk_specific;
	dvh_part_data->type = PED_BE32_TO_CPU (pt->pt_type);
	strcpy (dvh_part_data->name, "");

	return part;
}

static PedPartition*
_parse_boot_file (PedDisk* disk, struct volume_directory* vd)
{
	PedPartition*	part;
	DVHPartData*	dvh_part_data;
	PedSector	start = PED_BE32_TO_CPU (vd->vd_lbn);
	int		length = PED_BE32_TO_CPU (vd->vd_nbytes);

	part = ped_partition_new (disk, PED_PARTITION_LOGICAL, NULL,
				  start, start + length/512 - 1);
	if (!part)
		return NULL;

	dvh_part_data = part->disk_specific;
	dvh_part_data->real_file_size = length;

	strncpy (dvh_part_data->name, vd->vd_name, VDNAMESIZE);
	dvh_part_data->name[VDNAMESIZE] = 0;
	return part;
}

static int dvh_write (const PedDisk* disk);

/* YUCK
 *
 *  If you remove a boot/root/swap partition, the disk->disk_specific
 * thing isn't updated.  (Probably reflects a design bug somewhere...)
 * Anyway, the workaround is: flush stale flags whenever we allocate
 * new partition numbers, and before we write to disk.
 */
static void
_flush_stale_flags (const PedDisk* disk)
{
	DVHDiskData*		dvh_disk_data = disk->disk_specific;

	if (dvh_disk_data->root
		       && !ped_disk_get_partition (disk, dvh_disk_data->root))
		dvh_disk_data->root = 0;
	if (dvh_disk_data->swap
		       && !ped_disk_get_partition (disk, dvh_disk_data->swap))
		dvh_disk_data->swap = 0;
	if (dvh_disk_data->boot
		       && !ped_disk_get_partition (disk, dvh_disk_data->boot))
		dvh_disk_data->boot = 0;
}

static int
dvh_read (PedDisk* disk)
{
	DVHDiskData*		dvh_disk_data = disk->disk_specific;
	int			i;
	struct volume_header	vh;
	char			boot_name [BFNAMESIZE + 1];
#ifndef DISCOVER_ONLY
	int			write_back = 0;
#endif

	PED_ASSERT (dvh_disk_data != NULL);

	ped_disk_delete_all (disk);

	void *s0;
	if (!ptt_read_sector (disk->dev, 0, &s0))
		return 0;
	memcpy (&vh, s0, sizeof vh);
	free (s0);

	if (_checksum ((uint32_t*) &vh, sizeof (struct volume_header))) {
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Checksum is wrong, indicating the partition "
			  "table is corrupt."))
				== PED_EXCEPTION_CANCEL)
			return 0;
	}

	PED_ASSERT (PED_BE32_TO_CPU (vh.vh_magic) == VHMAGIC);

	dvh_disk_data->dev_params = vh.vh_dp;
	strncpy (boot_name, vh.vh_bootfile, BFNAMESIZE);
	boot_name[BFNAMESIZE] = 0;

	/* normal partitions */
	for (i = 0; i < NPARTAB; i++) {
		PedPartition* part;

		if (!vh.vh_pt[i].pt_nblks)
			continue;
		/* Skip the whole-disk partition, parted disklikes overlap */
		if (PED_BE32_TO_CPU (vh.vh_pt[i].pt_type) == PTYPE_VOLUME)
			continue;

		part = _parse_partition (disk, &vh.vh_pt[i]);
		if (!part)
			goto error_delete_all;

		part->fs_type = ped_file_system_probe (&part->geom);
		part->num = i + 1;

		if (PED_BE16_TO_CPU (vh.vh_rootpt) == i)
			ped_partition_set_flag (part, PED_PARTITION_ROOT, 1);
		if (PED_BE16_TO_CPU (vh.vh_swappt) == i)
			ped_partition_set_flag (part, PED_PARTITION_SWAP, 1);

		PedConstraint *constraint_exact
		  = ped_constraint_exact (&part->geom);
		bool ok = ped_disk_add_partition (disk, part, constraint_exact);
		ped_constraint_destroy (constraint_exact);
		if (!ok) {
			ped_partition_destroy (part);
			goto error_delete_all;
		}
	}

	if (!ped_disk_extended_partition (disk)) {
#ifdef DISCOVER_ONLY
		return 1;
#else
		switch (_handle_no_volume_header (disk)) {
			case PED_EXCEPTION_CANCEL:
				return 0;
			case PED_EXCEPTION_IGNORE:
				return 1;
			case PED_EXCEPTION_FIX:
				write_back = 1;
				break;
			default:
				break;
		}
#endif
	}

	/* boot partitions */
	for (i = 0; i < NVDIR; i++) {
		PedPartition* part;

		if (!vh.vh_vd[i].vd_nbytes)
			continue;

		part = _parse_boot_file (disk, &vh.vh_vd[i]);
		if (!part)
			goto error_delete_all;

		part->fs_type = ped_file_system_probe (&part->geom);
		part->num = NPARTAB + i + 1;

		if (!strcmp (boot_name, ped_partition_get_name (part)))
			ped_partition_set_flag (part, PED_PARTITION_BOOT, 1);

		PedConstraint *constraint_exact
		  = ped_constraint_exact (&part->geom);
		bool ok = ped_disk_add_partition (disk, part, constraint_exact);
		ped_constraint_destroy (constraint_exact);
		if (!ok) {
			ped_partition_destroy (part);
			goto error_delete_all;
		}
	}
#ifndef DISCOVER_ONLY
	if (write_back)
		dvh_write (disk);
#endif
	return 1;

error_delete_all:
	ped_disk_delete_all (disk);
	return 0;
}

#ifndef DISCOVER_ONLY
static void
_generate_partition (PedPartition* part, struct partition_table* pt)
{
	DVHPartData*	dvh_part_data = part->disk_specific;

	/* Assert not a bootfile */
	PED_ASSERT ((part->type & PED_PARTITION_LOGICAL) == 0);

	pt->pt_nblks = PED_CPU_TO_BE32 (part->geom.length);
	pt->pt_firstlbn = PED_CPU_TO_BE32 (part->geom.start);
	pt->pt_type = PED_CPU_TO_BE32 (dvh_part_data->type);
}

static void
_generate_boot_file (PedPartition* part, struct volume_directory* vd)
{
	DVHPartData*	dvh_part_data = part->disk_specific;

	/* Assert it's a bootfile */
	PED_ASSERT ((part->type & PED_PARTITION_LOGICAL) != 0);

	vd->vd_nbytes = PED_CPU_TO_BE32 (dvh_part_data->real_file_size);
	vd->vd_lbn = PED_CPU_TO_BE32 (part->geom.start);

	memset (vd->vd_name, 0, VDNAMESIZE);
	strncpy (vd->vd_name, dvh_part_data->name, VDNAMESIZE);
}

static int
dvh_write (const PedDisk* disk)
{
	DVHDiskData*		dvh_disk_data = disk->disk_specific;
	struct volume_header	vh;
	int			i;

	PED_ASSERT (dvh_disk_data != NULL);

	_flush_stale_flags (disk);

	memset (&vh, 0, sizeof (struct volume_header));

	vh.vh_magic = PED_CPU_TO_BE32 (VHMAGIC);
	vh.vh_rootpt = PED_CPU_TO_BE16 (dvh_disk_data->root - 1);
	vh.vh_swappt = PED_CPU_TO_BE16 (dvh_disk_data->swap - 1);

	if (dvh_disk_data->boot) {
		PedPartition* boot_part;
		boot_part = ped_disk_get_partition (disk, dvh_disk_data->boot);
		strcpy (vh.vh_bootfile, ped_partition_get_name (boot_part));
	}

	vh.vh_dp = dvh_disk_data->dev_params;
	/* Set up rudimentary device geometry */
	vh.vh_dp.dp_cyls
		= PED_CPU_TO_BE16 ((short)disk->dev->bios_geom.cylinders);
	vh.vh_dp.dp_trks0 = PED_CPU_TO_BE16 ((short)disk->dev->bios_geom.heads);
	vh.vh_dp.dp_secs
		= PED_CPU_TO_BE16 ((short)disk->dev->bios_geom.sectors);
	vh.vh_dp.dp_secbytes = PED_CPU_TO_BE16 ((short)disk->dev->sector_size);

	for (i = 0; i < NPARTAB; i++) {
		PedPartition* part = ped_disk_get_partition (disk, i + 1);
		if (part)
			_generate_partition (part, &vh.vh_pt[i]);
	}

	/* whole disk partition
	 * This is only ever written here, and never modified
	 * (or even shown) as it must contain the entire disk,
	 * and parted does not like overlapping partitions
	 */
	vh.vh_pt[PNUM_VOLUME].pt_nblks = PED_CPU_TO_BE32 (disk->dev->length);
	vh.vh_pt[PNUM_VOLUME].pt_firstlbn = PED_CPU_TO_BE32 (0);
	vh.vh_pt[PNUM_VOLUME].pt_type = PED_CPU_TO_BE32 (PTYPE_VOLUME);

	for (i = 0; i < NVDIR; i++) {
		PedPartition* part = ped_disk_get_partition (disk,
							     i + 1 + NPARTAB);
		if (part)
			_generate_boot_file (part, &vh.vh_vd[i]);
	}

	vh.vh_csum = 0;
	vh.vh_csum = PED_CPU_TO_BE32 (_checksum ((uint32_t*) &vh,
			       	      sizeof (struct volume_header)));

        return (ptt_write_sector (disk, &vh, sizeof vh)
                && ped_device_sync (disk->dev));
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
dvh_partition_new (const PedDisk* disk, PedPartitionType part_type,
		    const PedFileSystemType* fs_type,
		    PedSector start, PedSector end)
{
	PedPartition* part;
	DVHPartData* dvh_part_data;

	part = _ped_partition_alloc (disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	if (!ped_partition_is_active (part)) {
		part->disk_specific = NULL;
		return part;
	}

	dvh_part_data = part->disk_specific =
		ped_malloc (sizeof (DVHPartData));
	if (!dvh_part_data)
		goto error_free_part;

	dvh_part_data->type = (part_type == PED_PARTITION_EXTENDED)
					? PTYPE_VOLHDR
					: PTYPE_RAW;
	strcpy (dvh_part_data->name, "");
	dvh_part_data->real_file_size = part->geom.length * 512;
	return part;

error_free_part:
	_ped_partition_free (part);
error:
	return NULL;
}

static PedPartition*
dvh_partition_duplicate (const PedPartition* part)
{
	PedPartition* result;
	DVHPartData* part_data = part->disk_specific;
	DVHPartData* result_data;

	result = _ped_partition_alloc (part->disk, part->type, part->fs_type,
				       part->geom.start, part->geom.end);
	if (!result)
		goto error;
	result->num = part->num;

	if (!ped_partition_is_active (part)) {
		result->disk_specific = NULL;
		return result;
	}

	result_data = result->disk_specific =
		ped_malloc (sizeof (DVHPartData));
	if (!result_data)
		goto error_free_part;

	result_data->type = part_data->type;
	strcpy (result_data->name, part_data->name);
	result_data->real_file_size = part_data->real_file_size;
	return result;

error_free_part:
	_ped_partition_free (result);
error:
	return NULL;
}

static void
dvh_partition_destroy (PedPartition* part)
{
	if (ped_partition_is_active (part)) {
		PED_ASSERT (part->disk_specific != NULL);
		free (part->disk_specific);
	}
	_ped_partition_free (part);
}

static int
dvh_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
	DVHPartData* dvh_part_data = part->disk_specific;

	part->fs_type = fs_type;

	if (part->type == PED_PARTITION_EXTENDED) {
		dvh_part_data->type = PTYPE_VOLHDR;
		return 1;
	}

	/* Is this a bootfile? */
	if (part->type == PED_PARTITION_LOGICAL)
		return 1;

	if (fs_type && !strcmp (fs_type->name, "xfs"))
		dvh_part_data->type = PTYPE_XFS;
	else
		dvh_part_data->type = PTYPE_RAW;
	return 1;
}

static int
dvh_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	DVHDiskData* dvh_disk_data = part->disk->disk_specific;

	switch (flag) {
	case PED_PARTITION_ROOT:
		if (part->type != 0 && state) {
#ifndef DISCOVER_ONLY
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Only primary partitions can be root "
				  "partitions."));
#endif
			return 0;
		}
		dvh_disk_data->root = state ? part->num : 0;
		break;

	case PED_PARTITION_SWAP:
		if (part->type != 0 && state) {
#ifndef DISCOVER_ONLY
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Only primary partitions can be swap "
				  "partitions."));
			return 0;
#endif
		}
		dvh_disk_data->swap = state ? part->num : 0;
		break;

	case PED_PARTITION_BOOT:
		if (part->type != PED_PARTITION_LOGICAL && state) {
#ifndef DISCOVER_ONLY
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Only logical partitions can be a boot "
				  "file."));
#endif
			return 0;
		}
		dvh_disk_data->boot = state ? part->num : 0;
		break;

	case PED_PARTITION_LVM:
	case PED_PARTITION_LBA:
	case PED_PARTITION_HIDDEN:
	case PED_PARTITION_RAID:
	default:
		return 0;
	}
	return 1;
}

static int _GL_ATTRIBUTE_PURE
dvh_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	DVHDiskData* dvh_disk_data = part->disk->disk_specific;

	switch (flag) {
	case PED_PARTITION_ROOT:
		return dvh_disk_data->root == part->num;

	case PED_PARTITION_SWAP:
		return dvh_disk_data->swap == part->num;

	case PED_PARTITION_BOOT:
		return dvh_disk_data->boot == part->num;

	case PED_PARTITION_LVM:
	case PED_PARTITION_LBA:
	case PED_PARTITION_HIDDEN:
	case PED_PARTITION_RAID:
	default:
		return 0;
	}
	return 1;
}

static int
dvh_partition_is_flag_available (const PedPartition* part,
				  PedPartitionFlag flag)
{
	switch (flag) {
	case PED_PARTITION_ROOT:
	case PED_PARTITION_SWAP:
	case PED_PARTITION_BOOT:
		return 1;

	case PED_PARTITION_LVM:
	case PED_PARTITION_LBA:
	case PED_PARTITION_HIDDEN:
	case PED_PARTITION_RAID:
	default:
		return 0;
	}
	return 1;
}

static void
dvh_partition_set_name (PedPartition* part, const char* name)
{
	DVHPartData* dvh_part_data = part->disk_specific;

	if (part->type == PED_PARTITION_LOGICAL) {
		/* Bootfile */
		strncpy (dvh_part_data->name, name, VDNAMESIZE);
		dvh_part_data->name[VDNAMESIZE] = 0;
	} else {
#ifndef DISCOVER_ONLY
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("failed to set dvh partition name to %s:\n"
                          "Only logical partitions (boot files) have a name."),
                        name);
#endif
	}
}

static const char*
dvh_partition_get_name (const PedPartition* part)
{
	DVHPartData* dvh_part_data = part->disk_specific;
	return dvh_part_data->name;
}

/* The constraint for the volume header partition is different, because it must
 * contain the first sector of the disk.
 */
static PedConstraint*
_get_extended_constraint (PedDisk* disk)
{
        PedGeometry     min_geom;
	if (!ped_geometry_init (&min_geom, disk->dev, 0, 1))
		return NULL;
        return ped_constraint_new_from_min (&min_geom);
}

static PedConstraint*
_get_primary_constraint (PedDisk* disk)
{
        PedGeometry     max_geom;
	if (!ped_geometry_init (&max_geom, disk->dev, 1, disk->dev->length - 1))
		return NULL;
        return ped_constraint_new_from_max (&max_geom);
}

static int
dvh_partition_align (PedPartition* part, const PedConstraint* constraint)
{
        PED_ASSERT (part != NULL);

	if (_ped_partition_attempt_align (
			part, constraint,
			(part->type == PED_PARTITION_EXTENDED)
				? _get_extended_constraint (part->disk)
				: _get_primary_constraint (part->disk)))
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
dvh_partition_enumerate (PedPartition* part)
{
	int i;

	/* never change the partition numbers */
	if (part->num != -1)
		return 1;

	_flush_stale_flags (part->disk);

	if (part->type & PED_PARTITION_LOGICAL) {
		/* Bootfile */
		for (i = 1 + NPARTAB; i <= NPARTAB + NVDIR; i++) {
			if (!ped_disk_get_partition (part->disk, i)) {
				part->num = i;
				return 1;
			}
		}
		PED_ASSERT (0);
	} else if (part->type & PED_PARTITION_EXTENDED) {
		/* Volheader */
		part->num = PNUM_VOLHDR + 1;
	} else {
		for (i = 1; i <= NPARTAB; i++) {
			/* reserved for full volume partition */
			if (i == PNUM_VOLUME + 1)
				continue;

			if (!ped_disk_get_partition (part->disk, i)) {
				part->num = i;
				return 1;
			}
		}
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Too many primary partitions"));
	}

	return 0;
}

static int
dvh_get_max_primary_partition_count (const PedDisk* disk)
{
	return NPARTAB;
}

static bool
dvh_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = NPARTAB;
	return true;
}


static int
dvh_alloc_metadata (PedDisk* disk)
{
	PedPartition* part;
	PedPartition* extended_part;
	PedPartitionType metadata_type;
	PED_ASSERT(disk != NULL);

	/* We don't need to "protect" the start of the disk from the volume
	 * header.
	 */
	extended_part = ped_disk_extended_partition (disk);
	if (extended_part && extended_part->geom.start == 0)
		metadata_type = PED_PARTITION_METADATA | PED_PARTITION_LOGICAL;
	else
		metadata_type = PED_PARTITION_METADATA;

	part = ped_partition_new (disk, metadata_type, NULL, 0, 0);
	if (!part)
		goto error;

	PedConstraint *constraint_exact
	  = ped_constraint_exact (&part->geom);
	bool ok = ped_disk_add_partition (disk, part, constraint_exact);
	ped_constraint_destroy (constraint_exact);
	if (ok)
		return 1;

	ped_partition_destroy (part);
error:
	return 0;
}

#include "pt-common.h"
PT_define_limit_functions (dvh)

static PedDiskOps dvh_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (dvh_write),

	partition_set_name:	dvh_partition_set_name,
	partition_get_name:	dvh_partition_get_name,
	PT_op_function_initializers (dvh)
};

static PedDiskType dvh_disk_type = {
	next:		NULL,
	name:		"dvh",
	ops:		&dvh_disk_ops,
	features:	PED_DISK_TYPE_PARTITION_NAME | PED_DISK_TYPE_EXTENDED
};

void
ped_disk_dvh_init ()
{
	PED_ASSERT (sizeof (struct volume_header) == 512);

	ped_disk_type_register (&dvh_disk_type);
}

void
ped_disk_dvh_done ()
{
	ped_disk_type_unregister (&dvh_disk_type);
}
