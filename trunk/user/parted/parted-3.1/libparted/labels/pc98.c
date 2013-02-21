/*
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
*/

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>

#include "pt-tools.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

/* hacked from Linux/98 source: fs/partitions/nec98.h
 *
 * See also:
 *      http://people.FreeBSD.org/~kato/pc98.html
 *      http://www.kmc.kyoto-u.ac.jp/proj/linux98/index-english.html
 *
 * Partition types:
 *
 *   id0(mid):
 *      bit 7: 1=bootable, 0=not bootable
 *        # Linux uses this flag to make a distinction between ext2 and swap.
 *      bit 6--0:
 *        00H      : N88-BASIC(data)?, PC-UX(data)?
 *        04H      : PC-UX(data)
 *        06H      : N88-BASIC
 *        10H      : N88-BASIC
 *        14H      : *BSD, PC-UX
 *        20H      : DOS(data), Windows95/98/NT, Linux
 *        21H..2FH : DOS(system#1 .. system#15)
 *        40H      : Minix
 *
 *   id1(sid):
 *      bit 7: 1=active, 0=sleep(hidden)
 *        # PC-UX uses this flag to make a distinction between its file system
 *        # and its swap.
 *      bit 6--0:
 *        01H: FAT12
 *        11H: FAT16, <32MB [accessible to DOS 3.3]
 *        21H: FAT16, >=32MB [Large Partition]
 *        31H: NTFS
 *        28H: Windows NT (Volume/Stripe Set?)
 *        41H: Windows NT (Volume/Stripe Set?)
 *        48H: Windows NT (Volume/Stripe Set?)
 *        61H: FAT32
 *        04H: PC-UX
 *        06H: N88-BASIC
 *        44H: *BSD
 *        62H: ext2, linux-swap
 */

#define MAX_PART_COUNT 16
#define PC9800_EXTFMT_MAGIC 0xAA55

#define BIT(x) (1 << (x))
#define GET_BIT(n,bit) (((n) & BIT(bit)) != 0)
#define SET_BIT(n,bit,val) n = (val)?  (n | BIT(bit))  :  (n & ~BIT(bit))

typedef struct _PC98RawPartition	PC98RawPartition;
typedef struct _PC98RawTable		PC98RawTable;

/* ripped from Linux/98 source */
struct _PC98RawPartition {
	uint8_t		mid;		/* 0x80 - boot */
	uint8_t		sid;		/* 0x80 - active */
	uint8_t		dum1;		/* dummy for padding */
	uint8_t		dum2;		/* dummy for padding */
	uint8_t		ipl_sect;	/* IPL sector */
	uint8_t		ipl_head;	/* IPL head */
	uint16_t	ipl_cyl;	/* IPL cylinder */
	uint8_t		sector;		/* starting sector */
	uint8_t		head;		/* starting head */
	uint16_t	cyl;		/* starting cylinder */
	uint8_t		end_sector;	/* end sector */
	uint8_t		end_head;	/* end head */
	uint16_t	end_cyl;	/* end cylinder */
	char		name[16];
} __attribute__((packed));

struct _PC98RawTable {
	uint8_t			boot_code [510];
	uint16_t		magic;
	PC98RawPartition	partitions [MAX_PART_COUNT];
} __attribute__((packed));

typedef struct {
	PedSector	ipl_sector;
	int		system;
	int		boot;
	int		hidden;
	char		name [17];
} PC98PartitionData;

/* this MBR boot code is dummy */
static const char MBR_BOOT_CODE[] = {
	0xcb,			/* retf */
	0x00, 0x00, 0x00,	/* */
	0x49, 0x50, 0x4c, 0x31  /* "IPL1" */
};

static PedDiskType pc98_disk_type;

static PedSector chs_to_sector (const PedDevice* dev, int c, int h, int s);
static void sector_to_chs (const PedDevice* dev, PedSector sector,
			   int* c, int* h, int* s);

/* magic(?) check */
static int
pc98_check_magic (const PC98RawTable *part_table)
{
	/* check "extended-format" (have partition table?) */
	if (PED_LE16_TO_CPU(part_table->magic) != PC9800_EXTFMT_MAGIC)
		return 0;

	return 1;
}

static int
pc98_check_ipl_signature (const PC98RawTable *part_table)
{
	if (memcmp (part_table->boot_code + 4, "IPL1", 4) == 0)
		return 1;
	else if (memcmp (part_table->boot_code + 4, "Linux 98", 8) == 0)
		return 1;
	else if (memcmp (part_table->boot_code + 4, "GRUB/98 ", 8) == 0)
		return 1;
	else
		return 0;
}

static int
pc98_probe (const PedDevice *dev)
{
	PC98RawTable		part_table;

	PED_ASSERT (dev != NULL);

        if (dev->sector_size != 512)
                return 0;

	if (!ped_device_read (dev, &part_table, 0, 2))
		return 0;

	/* check magic */
	if (!pc98_check_magic (&part_table))
		return 0;

	/* check for boot loader signatures */
	return pc98_check_ipl_signature (&part_table);
}

static PedDisk*
pc98_alloc (const PedDevice* dev)
{
	PED_ASSERT (dev != NULL);

	return _ped_disk_alloc (dev, &pc98_disk_type);
}

static PedDisk*
pc98_duplicate (const PedDisk* disk)
{
	return ped_disk_new_fresh (disk->dev, &pc98_disk_type);
}

static void
pc98_free (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);

	_ped_disk_free (disk);
}

static PedSector _GL_ATTRIBUTE_PURE
chs_to_sector (const PedDevice* dev, int c, int h, int s)
{
	PED_ASSERT (dev != NULL);
	return (c * dev->hw_geom.heads + h) * dev->hw_geom.sectors + s;
}

static void
sector_to_chs (const PedDevice* dev, PedSector sector, int* c, int* h, int* s)
{
	PedSector cyl_size;

	PED_ASSERT (dev != NULL);
	PED_ASSERT (c != NULL);
	PED_ASSERT (h != NULL);
	PED_ASSERT (s != NULL);

	cyl_size = dev->hw_geom.heads * dev->hw_geom.sectors;

	*c = sector / cyl_size;
	*h = (sector) % cyl_size / dev->hw_geom.sectors;
	*s = (sector) % cyl_size % dev->hw_geom.sectors;
}

static PedSector _GL_ATTRIBUTE_PURE
legacy_start (const PedDisk* disk, const PC98RawPartition* raw_part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	return chs_to_sector (disk->dev, PED_LE16_TO_CPU(raw_part->cyl),
			      raw_part->head, raw_part->sector);
}

static PedSector _GL_ATTRIBUTE_PURE
legacy_end (const PedDisk* disk, const PC98RawPartition* raw_part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	if (raw_part->end_head == 0 && raw_part->end_sector == 0) {
		return chs_to_sector (disk->dev,
				      PED_LE16_TO_CPU(raw_part->end_cyl),
				      disk->dev->hw_geom.heads - 1,
				      disk->dev->hw_geom.sectors - 1);
	} else {
		return chs_to_sector (disk->dev,
				      PED_LE16_TO_CPU(raw_part->end_cyl),
				      raw_part->end_head,
				      raw_part->end_sector);
	}
}

static int
is_unused_partition(const PC98RawPartition* raw_part)
{
	if (raw_part->mid || raw_part->sid
	    || raw_part->ipl_sect
	    || raw_part->ipl_head
	    || PED_LE16_TO_CPU(raw_part->ipl_cyl)
	    || raw_part->sector
	    || raw_part->head
	    || PED_LE16_TO_CPU(raw_part->cyl)
	    || raw_part->end_sector
	    || raw_part->end_head
	    || PED_LE16_TO_CPU(raw_part->end_cyl))
		return 0;
	return 1;
}

static int
read_table (PedDisk* disk)
{
	int			i;
	PC98RawTable		table;
	PedConstraint*		constraint_any;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	constraint_any = ped_constraint_any (disk->dev);

	if (!ped_device_read (disk->dev, (void*) &table, 0, 2))
		goto error;

	if (!pc98_check_magic(&table)) {
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_IGNORE_CANCEL,
			_("Invalid partition table on %s."),
			disk->dev->path))
			goto error;
	}

	for (i = 0; i < MAX_PART_COUNT; i++) {
		PC98RawPartition*	raw_part;
		PedPartition*		part;
		PC98PartitionData*	pc98_data;
		PedSector		part_start;
		PedSector		part_end;

		raw_part = &table.partitions [i];

		if (is_unused_partition(raw_part))
			continue;

		part_start = legacy_start (disk, raw_part);
		part_end   = legacy_end (disk, raw_part);

		part = ped_partition_new (disk, PED_PARTITION_NORMAL,
                                          NULL, part_start, part_end);
		if (!part)
			goto error;
		pc98_data = part->disk_specific;
		PED_ASSERT (pc98_data != NULL);

		pc98_data->system = (raw_part->mid << 8) | raw_part->sid;
		pc98_data->boot = GET_BIT(raw_part->mid, 7);
		pc98_data->hidden = !GET_BIT(raw_part->sid, 7);

		ped_partition_set_name (part, raw_part->name);

		pc98_data->ipl_sector = chs_to_sector (
			disk->dev,
			PED_LE16_TO_CPU(raw_part->ipl_cyl),
			raw_part->ipl_head,
			raw_part->ipl_sect);

		/* hack */
		if (pc98_data->ipl_sector == part->geom.start)
			pc98_data->ipl_sector = 0;

		part->num = i + 1;

		if (!ped_disk_add_partition (disk, part, constraint_any))
			goto error;

		if (part->geom.start != part_start
		    || part->geom.end != part_end) {
			ped_exception_throw (
				PED_EXCEPTION_NO_FEATURE,
				PED_EXCEPTION_CANCEL,
				_("Partition %d isn't aligned to cylinder "
				  "boundaries.  This is still unsupported."),
				part->num);
			goto error;
		}

		part->fs_type = ped_file_system_probe (&part->geom);
	}

	ped_constraint_destroy (constraint_any);
	return 1;

error:
	ped_disk_delete_all (disk);
	ped_constraint_destroy (constraint_any);
	return 0;
}

static int
pc98_read (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	ped_disk_delete_all (disk);
	return read_table (disk);
}

#ifndef DISCOVER_ONLY
static int
fill_raw_part (PC98RawPartition* raw_part, const PedPartition* part)
{
	PC98PartitionData*	pc98_data;
	int			c, h, s;
	const char*		name;

	PED_ASSERT (raw_part != NULL);
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	pc98_data = part->disk_specific;
	raw_part->mid = (pc98_data->system >> 8) & 0xFF;
	raw_part->sid = pc98_data->system & 0xFF;

	SET_BIT(raw_part->mid, 7, pc98_data->boot);
	SET_BIT(raw_part->sid, 7, !pc98_data->hidden);

	memset (raw_part->name, ' ', sizeof(raw_part->name));
	name = ped_partition_get_name (part);
	PED_ASSERT (name != NULL);
	PED_ASSERT (strlen (name) <= 16);
	if (!strlen (name) && part->fs_type)
		name = part->fs_type->name;
	memcpy (raw_part->name, name, strlen (name));

	sector_to_chs (part->disk->dev, part->geom.start, &c, &h, &s);
	raw_part->cyl	 = PED_CPU_TO_LE16(c);
	raw_part->head	 = h;
	raw_part->sector = s;

	if (pc98_data->ipl_sector) {
		sector_to_chs (part->disk->dev, pc98_data->ipl_sector,
			       &c, &h, &s);
		raw_part->ipl_cyl  = PED_CPU_TO_LE16(c);
		raw_part->ipl_head = h;
		raw_part->ipl_sect = s;
	} else {
		raw_part->ipl_cyl  = raw_part->cyl;
		raw_part->ipl_head = raw_part->head;
		raw_part->ipl_sect = raw_part->sector;
	}

	sector_to_chs (part->disk->dev, part->geom.end, &c, &h, &s);
	if (h != part->disk->dev->hw_geom.heads - 1
	    || s != part->disk->dev->hw_geom.sectors - 1) {
		ped_exception_throw (
		    PED_EXCEPTION_NO_FEATURE,
		    PED_EXCEPTION_CANCEL,
		    _("Partition %d isn't aligned to cylinder "
		      "boundaries.  This is still unsupported."),
		    part->num);
		return 0;
	}
	raw_part->end_cyl    = PED_CPU_TO_LE16(c);
#if 0
	raw_part->end_head   = h;
	raw_part->end_sector = s;
#else
	raw_part->end_head   = 0;
	raw_part->end_sector = 0;
#endif

	return 1;
}

static int
pc98_write (const PedDisk* disk)
{
	PedPartition*		part;
	int			i;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	void *s0;
	if (!ptt_read_sectors (disk->dev, 0, 2, &s0))
		return 0;
	PC98RawTable *table = s0;

	if (!pc98_check_ipl_signature (table)) {
		memset (table->boot_code, 0, sizeof(table->boot_code));
		memcpy (table->boot_code, MBR_BOOT_CODE, sizeof(MBR_BOOT_CODE));
	}

	memset (table->partitions, 0, sizeof (table->partitions));
	table->magic = PED_CPU_TO_LE16(PC9800_EXTFMT_MAGIC);

	for (i = 1; i <= MAX_PART_COUNT; i++) {
		part = ped_disk_get_partition (disk, i);
		if (!part)
			continue;

		if (!fill_raw_part (&table->partitions [i - 1], part))
			return 0;
	}

	int write_ok = ped_device_write (disk->dev, table, 0, 2);
	free (s0);
	if (!write_ok)
		return 0;
	return ped_device_sync (disk->dev);
}
#endif /* !DISCOVER_ONLY */

static PedPartition*
pc98_partition_new (
	const PedDisk* disk, PedPartitionType part_type,
	const PedFileSystemType* fs_type, PedSector start, PedSector end)
{
	PedPartition*		part;
	PC98PartitionData*	pc98_data;

	part = _ped_partition_alloc (disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	if (ped_partition_is_active (part)) {
		part->disk_specific
			= pc98_data = ped_malloc (sizeof (PC98PartitionData));
		if (!pc98_data)
			goto error_free_part;
		pc98_data->ipl_sector = 0;
		pc98_data->hidden = 0;
		pc98_data->boot = 0;
		strcpy (pc98_data->name, "");
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
pc98_partition_duplicate (const PedPartition* part)
{
	PedPartition*		new_part;
	PC98PartitionData*	new_pc98_data;
	PC98PartitionData*	old_pc98_data;

	new_part = ped_partition_new (part->disk, part->type,
				      part->fs_type, part->geom.start,
				      part->geom.end);
	if (!new_part)
		return NULL;
	new_part->num = part->num;

	old_pc98_data = (PC98PartitionData*) part->disk_specific;
	new_pc98_data = (PC98PartitionData*) new_part->disk_specific;

	/* ugly, but C is ugly :p */
	memcpy (new_pc98_data, old_pc98_data, sizeof (PC98PartitionData));
	return new_part;
}

static void
pc98_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	if (ped_partition_is_active (part))
		free (part->disk_specific);
	free (part);
}

static int
pc98_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
	PC98PartitionData* pc98_data = part->disk_specific;

	part->fs_type = fs_type;

	pc98_data->system = 0x2062;
	if (fs_type) {
		if (!strcmp (fs_type->name, "fat16")) {
			if (part->geom.length * 512 >= 32 * 1024 * 1024)
				pc98_data->system = 0x2021;
			else
				pc98_data->system = 0x2011;
		} else if (!strcmp (fs_type->name, "fat32")) {
			pc98_data->system = 0x2061;
		} else if (!strcmp (fs_type->name, "ntfs")) {
			pc98_data->system = 0x2031;
		} else if (!strncmp (fs_type->name, "ufs", 3)) {
			pc98_data->system = 0x2044;
		} else { /* ext2, reiser, xfs, etc. */
			/* ext2 partitions must be marked boot */
			pc98_data->boot = 1;
			pc98_data->system = 0xa062;
		}
	}

	if (pc98_data->boot)
		pc98_data->system |= 0x8000;
	if (!pc98_data->hidden)
		pc98_data->system |= 0x0080;
	return 1;
}

static int
pc98_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	PC98PartitionData*		pc98_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	pc98_data = part->disk_specific;

	switch (flag) {
	case PED_PARTITION_HIDDEN:
		pc98_data->hidden = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_BOOT:
		pc98_data->boot = state;
		return ped_partition_set_system (part, part->fs_type);

	default:
		return 0;
	}
}

static int _GL_ATTRIBUTE_PURE
pc98_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	PC98PartitionData*	pc98_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	pc98_data = part->disk_specific;
	switch (flag) {
	case PED_PARTITION_HIDDEN:
		return pc98_data->hidden;

	case PED_PARTITION_BOOT:
		return pc98_data->boot;

	default:
		return 0;
	}
}

static int
pc98_partition_is_flag_available (
	const PedPartition* part, PedPartitionFlag flag)
{
	switch (flag) {
	case PED_PARTITION_HIDDEN:
	case PED_PARTITION_BOOT:
		return 1;

	default:
		return 0;
	}
}

static void
pc98_partition_set_name (PedPartition* part, const char* name)
{
	PC98PartitionData*	pc98_data;
	int			i;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	pc98_data = part->disk_specific;

	strncpy (pc98_data->name, name, 16);
	pc98_data->name [16] = 0;
	for (i = strlen (pc98_data->name) - 1; pc98_data->name[i] == ' '; i--)
		pc98_data->name [i] = 0;
}

static const char* _GL_ATTRIBUTE_PURE
pc98_partition_get_name (const PedPartition* part)
{
	PC98PartitionData*	pc98_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	pc98_data = part->disk_specific;

	return pc98_data->name;
}

static PedAlignment*
pc98_get_partition_alignment(const PedDisk *disk)
{
	PedSector cylinder_size =
		disk->dev->hw_geom.sectors * disk->dev->hw_geom.heads;

        return ped_alignment_new(0, cylinder_size);
}

static PedConstraint*
_primary_constraint (PedDisk* disk)
{
	PedDevice*	dev = disk->dev;
	PedAlignment	start_align;
	PedAlignment	end_align;
	PedGeometry	max_geom;
	PedSector	cylinder_size;

	cylinder_size = dev->hw_geom.sectors * dev->hw_geom.heads;

	if (!ped_alignment_init (&start_align, 0, cylinder_size))
		return NULL;
	if (!ped_alignment_init (&end_align, -1, cylinder_size))
		return NULL;
	if (!ped_geometry_init (&max_geom, dev, cylinder_size,
			       	dev->length - cylinder_size))
		return NULL;

	return ped_constraint_new (&start_align, &end_align, &max_geom,
				   &max_geom, 1, dev->length);
}

static int
pc98_partition_align (PedPartition* part, const PedConstraint* constraint)
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
next_primary (PedDisk* disk)
{
	int	i;
	for (i=1; i<=MAX_PART_COUNT; i++) {
		if (!ped_disk_get_partition (disk, i))
			return i;
	}
	return 0;
}

static int
pc98_partition_enumerate (PedPartition* part)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);

	/* don't re-number a partition */
	if (part->num != -1)
		return 1;

	PED_ASSERT (ped_partition_is_active (part));

	part->num = next_primary (part->disk);
	if (!part->num) {
		ped_exception_throw (PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't add another partition."));
		return 0;
	}

	return 1;
}

static int
pc98_alloc_metadata (PedDisk* disk)
{
	PedPartition*		new_part;
	PedConstraint*		constraint_any = NULL;
	PedSector		cyl_size;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	constraint_any = ped_constraint_any (disk->dev);

	cyl_size = disk->dev->hw_geom.sectors * disk->dev->hw_geom.heads;
	new_part = ped_partition_new (disk, PED_PARTITION_METADATA, NULL,
				      0, cyl_size - 1);
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
pc98_get_max_primary_partition_count (const PedDisk* disk)
{
	return MAX_PART_COUNT;
}

static bool
pc98_get_max_supported_partition_count (const PedDisk* disk, int *max_n)
{
	*max_n = MAX_PART_COUNT;
	return true;
}

#include "pt-common.h"
PT_define_limit_functions (pc98)

static PedDiskOps pc98_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (pc98_write),

	partition_set_name:	pc98_partition_set_name,
	partition_get_name:	pc98_partition_get_name,

	get_partition_alignment: pc98_get_partition_alignment,

	PT_op_function_initializers (pc98)
};

static PedDiskType pc98_disk_type = {
	next:		NULL,
	name:		"pc98",
	ops:		&pc98_disk_ops,
	features:	PED_DISK_TYPE_PARTITION_NAME
};

void
ped_disk_pc98_init ()
{
	PED_ASSERT (sizeof (PC98RawTable) == 512 * 2);
	ped_disk_type_register (&pc98_disk_type);
}

void
ped_disk_pc98_done ()
{
	ped_disk_type_unregister (&pc98_disk_type);
}
