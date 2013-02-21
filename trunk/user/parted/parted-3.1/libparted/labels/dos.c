/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2001, 2004-2005, 2007-2012 Free Software Foundation,
    Inc.

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

#include <sys/time.h>
#include <stdbool.h>
#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "misc.h"
#include "pt-tools.h"

/* this MBR boot code is loaded into 0000:7c00 by the BIOS.  See mbr.s for
 * the source, and how to build it
 */

static const char MBR_BOOT_CODE[] = {
	0xfa, 0xb8, 0x00, 0x10, 0x8e, 0xd0, 0xbc, 0x00,
	0xb0, 0xb8, 0x00, 0x00, 0x8e, 0xd8, 0x8e, 0xc0,
	0xfb, 0xbe, 0x00, 0x7c, 0xbf, 0x00, 0x06, 0xb9,
	0x00, 0x02, 0xf3, 0xa4, 0xea, 0x21, 0x06, 0x00,
	0x00, 0xbe, 0xbe, 0x07, 0x38, 0x04, 0x75, 0x0b,
	0x83, 0xc6, 0x10, 0x81, 0xfe, 0xfe, 0x07, 0x75,
	0xf3, 0xeb, 0x16, 0xb4, 0x02, 0xb0, 0x01, 0xbb,
	0x00, 0x7c, 0xb2, 0x80, 0x8a, 0x74, 0x01, 0x8b,
	0x4c, 0x02, 0xcd, 0x13, 0xea, 0x00, 0x7c, 0x00,
	0x00, 0xeb, 0xfe
};

#define MSDOS_MAGIC		0xAA55
#define PARTITION_MAGIC_MAGIC	0xf6f6

/* The maximum number of DOS primary partitions.  */
#define DOS_N_PRI_PARTITIONS	4

#define PARTITION_EMPTY		0x00
#define PARTITION_FAT12		0x01
#define PARTITION_FAT16_SM	0x04
#define PARTITION_DOS_EXT	0x05
#define PARTITION_FAT16		0x06
#define PARTITION_NTFS		0x07
#define PARTITION_HPFS		0x07
#define PARTITION_FAT32		0x0b
#define PARTITION_FAT32_LBA	0x0c
#define PARTITION_FAT16_LBA	0x0e
#define PARTITION_EXT_LBA	0x0f

#define PART_FLAG_HIDDEN	0x10	/* Valid for FAT/NTFS only */
#define PARTITION_FAT12_H	(PARTITION_FAT12	| PART_FLAG_HIDDEN)
#define PARTITION_FAT16_SM_H	(PARTITION_FAT16_SM	| PART_FLAG_HIDDEN)
#define PARTITION_DOS_EXT_H	(PARTITION_DOS_EXT	| PART_FLAG_HIDDEN)
#define PARTITION_FAT16_H	(PARTITION_FAT16	| PART_FLAG_HIDDEN)
#define PARTITION_NTFS_H	(PARTITION_NTFS		| PART_FLAG_HIDDEN)
#define PARTITION_FAT32_H	(PARTITION_FAT32	| PART_FLAG_HIDDEN)
#define PARTITION_FAT32_LBA_H	(PARTITION_FAT32_LBA	| PART_FLAG_HIDDEN)
#define PARTITION_FAT16_LBA_H	(PARTITION_FAT16_LBA	| PART_FLAG_HIDDEN)

#define PARTITION_COMPAQ_DIAG	0x12
#define PARTITION_MSFT_RECOVERY	0x27
#define PARTITION_LDM		0x42
#define PARTITION_LINUX_SWAP	0x82
#define PARTITION_LINUX		0x83
#define PARTITION_LINUX_EXT	0x85
#define PARTITION_LINUX_LVM	0x8e
#define PARTITION_HFS		0xaf
#define PARTITION_SUN_UFS	0xbf
#define PARTITION_DELL_DIAG	0xde
#define PARTITION_GPT		0xee
#define PARTITION_PALO		0xf0
#define PARTITION_PREP		0x41
#define PARTITION_LINUX_RAID	0xfd
#define PARTITION_LINUX_LVM_OLD 0xfe

/* This constant contains the maximum cylinder number that can be represented
 * in (C,H,S) notation.  Higher cylinder numbers are reserved for
 * "too big" indicators (in which case only LBA addressing can be used).
 * 	Some partition tables in the wild indicate this number is 1021.
 * (i.e. 1022 is sometimes used to indicate "use LBA").
 */
#define MAX_CHS_CYLINDER	1021
#define MAX_TOTAL_PART		64

typedef struct _DosRawPartition		DosRawPartition;
typedef struct _DosRawTable		DosRawTable;

/* note: lots of bit-bashing here, thus, you shouldn't look inside it.
 * Use chs_to_sector() and sector_to_chs() instead.
 */
typedef struct {
	uint8_t		head;
	uint8_t		sector;
	uint8_t		cylinder;
} __attribute__((packed)) RawCHS;

/* ripped from Linux source */
struct _DosRawPartition {
        uint8_t		boot_ind;	/* 00:  0x80 - active */
	RawCHS		chs_start;	/* 01: */
	uint8_t		type;		/* 04: partition type */
	RawCHS		chs_end;	/* 05: */
	uint32_t	start;		/* 08: starting sector counting from 0 */
	uint32_t	length;		/* 0c: nr of sectors in partition */
} __attribute__((packed));

struct _DosRawTable {
	char			boot_code [440];
	uint32_t                mbr_signature;	/* really a unique ID */
	uint16_t                Unknown;
	DosRawPartition		partitions [DOS_N_PRI_PARTITIONS];
	uint16_t		magic;
} __attribute__((packed));

/* OrigState is information we want to preserve about the partition for
 * dealing with CHS issues
 */
typedef struct {
	PedGeometry	geom;
	DosRawPartition	raw_part;
	PedSector	lba_offset;	/* needed for computing start/end for
					 * logical partitions */
} OrigState;

typedef struct {
        int             cylinder_alignment;
} DosDiskData;

typedef struct {
	unsigned char	system;
	int		boot;
	int		hidden;
	int		raid;
	int		lvm;
	int		lba;
	int		palo;
	int		prep;
	int		diag;
	OrigState*	orig;			/* used for CHS stuff */
} DosPartitionData;

static PedDiskType msdos_disk_type;

#if 0
From http://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html

The 2-byte numbers are stored little endian (low order byte first).

Here the FAT12 version, that is also the common part of the FAT12, FAT16 and FAT32 boot sectors. See further below.

Bytes   Content
0-2     Jump to bootstrap (E.g. eb 3c 90; on i86: JMP 003E NOP.
        One finds either eb xx 90, or e9 xx xx.
        The position of the bootstrap varies.)
3-10    OEM name/version (E.g. "IBM  3.3", "IBM 20.0", "MSDOS5.0", "MSWIN4.0".
        Various format utilities leave their own name, like "CH-FOR18".
        Sometimes just garbage. Microsoft recommends "MSWIN4.1".)
        /* BIOS Parameter Block starts here */
11-12   Number of bytes per sector (512)
        Must be one of 512, 1024, 2048, 4096.
13      Number of sectors per cluster (1)
        Must be one of 1, 2, 4, 8, 16, 32, 64, 128.
        A cluster should have at most 32768 bytes. In rare cases 65536 is OK.
14-15   Number of reserved sectors (1)
        FAT12 and FAT16 use 1. FAT32 uses 32.
16      Number of FAT copies (2)
17-18   Number of root directory entries (224)
        0 for FAT32. 512 is recommended for FAT16.
19-20   Total number of sectors in the filesystem (2880)
        (in case the partition is not FAT32 and smaller than 32 MB)
21      Media descriptor type (f0: 1.4 MB floppy, f8: hard disk; see below)
22-23   Number of sectors per FAT (9)
        0 for FAT32.
24-25   Number of sectors per track (12)
26-27   Number of heads (2, for a double-sided diskette)
28-29   Number of hidden sectors (0)
        Hidden sectors are sectors preceding the partition.
        /* BIOS Parameter Block ends here */
30-509  Bootstrap
510-511 Signature 55 aa
#endif

/* There is a significant risk of misclassifying (as msdos)
   a disk that is composed solely of a single FAT partition.
   Return false if sector S could not be a valid FAT boot sector.
   Otherwise, return true.  */
static bool
maybe_FAT (unsigned char const *s)
{
  if (! (s[0] == 0xeb || s[0] == 0xe9))
    return false;

  unsigned int sector_size = PED_LE16_TO_CPU (*(uint16_t *) (s + 11));
  switch (sector_size)
    {
    case 512:
    case 1024:
    case 2048:
    case 4096:
      break;
    default:
      return false;
    }

  if (! (s[21] == 0xf0 || s[21] == 0xf8))
    return false;

  return true;
}

static int
msdos_probe (const PedDevice *dev)
{
	PedDiskType*	disk_type;
	DosRawTable*	part_table;
	int		i;

	PED_ASSERT (dev != NULL);

        if (dev->sector_size < sizeof *part_table)
                return 0;

	void *label;
	if (!ptt_read_sector (dev, 0, &label))
		return 0;

	part_table = (DosRawTable *) label;

	/* check magic */
	if (PED_LE16_TO_CPU (part_table->magic) != MSDOS_MAGIC)
		goto probe_fail;

	/* If this is a FAT fs, fail here.  Checking for the FAT signature
	 * has some false positives; instead, do what the Linux kernel does
	 * and ensure that each partition has a boot indicator that is
	 * either 0 or 0x80.
	 */
	unsigned int n_active = 0;
	for (i = 0; i < DOS_N_PRI_PARTITIONS; i++) {
		if (part_table->partitions[i].boot_ind == 0x80)
			++n_active;
		if (part_table->partitions[i].boot_ind != 0
		    && part_table->partitions[i].boot_ind != 0x80)
			goto probe_fail;
	}

	/* If there are no active partitions and this is probably
	   a FAT file system, do not classify it as msdos.  */
	if (n_active == 0 && maybe_FAT (label))
	  goto probe_fail;

	/* If this is a GPT disk, fail here */
	for (i = 0; i < DOS_N_PRI_PARTITIONS; i++) {
		if (part_table->partitions[i].type == PARTITION_GPT)
			goto probe_fail;
	}

	/* If this is an AIX Physical Volume, fail here.  IBMA in EBCDIC */
	if (part_table->boot_code[0] == (char) 0xc9 &&
	    part_table->boot_code[1] == (char) 0xc2 &&
	    part_table->boot_code[2] == (char) 0xd4 &&
	    part_table->boot_code[3] == (char) 0xc1)
		goto probe_fail;

#ifdef ENABLE_PC98
	/* HACK: it's impossible to tell PC98 and msdos disk labels apart.
	 * Someone made the signatures the same (very clever).  Since
	 * PC98 has some idiosyncracies with it's boot-loader, it's detection
	 * is more reliable */
	disk_type = ped_disk_type_get ("pc98");
	if (disk_type && disk_type->ops->probe (dev))
		goto probe_fail;
#endif /* ENABLE_PC98 */

	free (label);
	return 1;

 probe_fail:
	free (label);
	return 0;
}

static PedDisk*
msdos_alloc (const PedDevice* dev)
{
	PedDisk* disk;
	PED_ASSERT (dev != NULL);

	disk = _ped_disk_alloc ((PedDevice*)dev, &msdos_disk_type);
        if (disk) {
		DosDiskData *disk_specific = ped_malloc(sizeof *disk_specific);
                if (!disk_specific) {
                        free (disk);
                        return NULL;
                }
                disk_specific->cylinder_alignment = 1;
                disk->disk_specific = disk_specific;
        }

	return disk;
}

static PedDisk*
msdos_duplicate (const PedDisk* disk)
{
	PedDisk*	new_disk;

	new_disk = ped_disk_new_fresh (disk->dev, &msdos_disk_type);
	if (!new_disk)
		return NULL;

        memcpy(new_disk->disk_specific, disk->disk_specific,
               sizeof(DosDiskData));

	return new_disk;
}

static void
msdos_free (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);

	DosDiskData *disk_specific = disk->disk_specific;
	_ped_disk_free (disk);
	free(disk_specific);
}

static int
msdos_disk_set_flag (PedDisk *disk, PedDiskFlag flag, int state)
{
        DosDiskData *disk_specific = disk->disk_specific;
        switch (flag) {
        case PED_DISK_CYLINDER_ALIGNMENT:
                disk_specific->cylinder_alignment = !!state;
                return 1;
        default:
                return 0;
        }
}

static int
msdos_disk_get_flag (const PedDisk *disk, PedDiskFlag flag)
{
        DosDiskData *disk_specific = disk->disk_specific;
        switch (flag) {
        case PED_DISK_CYLINDER_ALIGNMENT:
                return disk_specific->cylinder_alignment;
        default:
                return 0;
        }
}

static int
msdos_disk_is_flag_available (const PedDisk *disk, PedDiskFlag flag)
{
        switch (flag) {
        case PED_DISK_CYLINDER_ALIGNMENT:
               return 1;
        default:
               return 0;
        }
}

static int
chs_get_cylinder (const RawCHS* chs)
{
	return chs->cylinder + ((chs->sector >> 6) << 8);
}

static int
chs_get_head (const RawCHS* chs)
{
	return chs->head;
}

/* counts from 0 */
static int
chs_get_sector (const RawCHS* chs)
{
	return (chs->sector & 0x3f) - 1;
}

static PedSector _GL_ATTRIBUTE_PURE
chs_to_sector (const PedDevice* dev, const PedCHSGeometry *bios_geom,
	       const RawCHS* chs)
{
	PedSector	c;		/* not measured in sectors, but need */
	PedSector	h;		/* lots of bits */
	PedSector	s;

	PED_ASSERT (bios_geom != NULL);
	PED_ASSERT (chs != NULL);

	c = chs_get_cylinder (chs);
	h = chs_get_head (chs);
	s = chs_get_sector (chs);

	if (c > MAX_CHS_CYLINDER)		/* MAGIC: C/H/S is irrelevant */
		return 0;
	if (s < 0)
		return 0;
	return (c * bios_geom->heads + h) * bios_geom->sectors + s;
}

static void
sector_to_chs (const PedDevice* dev, const PedCHSGeometry* bios_geom,
	       PedSector sector, RawCHS* chs)
{
	PedSector	real_c, real_h, real_s;

	PED_ASSERT (dev != NULL);
	PED_ASSERT (chs != NULL);

	if (!bios_geom)
		bios_geom = &dev->bios_geom;

	real_c = sector / (bios_geom->heads * bios_geom->sectors);
	real_h = (sector / bios_geom->sectors) % bios_geom->heads;
	real_s = sector % bios_geom->sectors;

	if (real_c > MAX_CHS_CYLINDER) {
		real_c = 1023;
		real_h = bios_geom->heads - 1;
		real_s = bios_geom->sectors - 1;
	}

	chs->cylinder = real_c % 0x100;
	chs->head = real_h;
	chs->sector = real_s + 1 + (real_c >> 8 << 6);
}

static PedSector _GL_ATTRIBUTE_PURE
legacy_start (const PedDisk* disk, const PedCHSGeometry* bios_geom,
	      const DosRawPartition* raw_part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	return chs_to_sector (disk->dev, bios_geom, &raw_part->chs_start);
}

static PedSector _GL_ATTRIBUTE_PURE
legacy_end (const PedDisk* disk, const PedCHSGeometry* bios_geom,
	    const DosRawPartition* raw_part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	return chs_to_sector (disk->dev, bios_geom, &raw_part->chs_end);
}

static PedSector _GL_ATTRIBUTE_PURE
linear_start (const PedDisk* disk, const DosRawPartition* raw_part,
	      PedSector offset)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	return offset + PED_LE32_TO_CPU (raw_part->start);
}

static PedSector _GL_ATTRIBUTE_PURE
linear_end (const PedDisk* disk, const DosRawPartition* raw_part,
	    PedSector offset)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	return (linear_start (disk, raw_part, offset)
                + (PED_LE32_TO_CPU (raw_part->length) - 1));
}

#ifndef DISCOVER_ONLY
static int _GL_ATTRIBUTE_PURE
partition_check_bios_geometry (PedPartition* part, PedCHSGeometry* bios_geom)
{
	PedSector		leg_start, leg_end;
	DosPartitionData*	dos_data;
	PedDisk*		disk;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	dos_data = part->disk_specific;

	if (!dos_data->orig)
		return 1;

	disk = part->disk;
	leg_start = legacy_start (disk, bios_geom, &dos_data->orig->raw_part);
	leg_end = legacy_end (disk, bios_geom, &dos_data->orig->raw_part);

	if (leg_start && leg_start != dos_data->orig->geom.start)
		return 0;
	if (leg_end && leg_end != dos_data->orig->geom.end)
		return 0;
	return 1;
}

static int _GL_ATTRIBUTE_PURE
disk_check_bios_geometry (const PedDisk* disk, PedCHSGeometry* bios_geom)
{
	PedPartition* part = NULL;

	PED_ASSERT (disk != NULL);

	while ((part = ped_disk_next_partition (disk, part))) {
		if (ped_partition_is_active (part)) {
			if (!partition_check_bios_geometry (part, bios_geom))
				return 0;
		}
	}

	return 1;
}

static int
probe_filesystem_for_geom (const PedPartition* part, PedCHSGeometry* bios_geom)
{
	const char* ms_types[] = {"ntfs", "fat16", "fat32", NULL};
	int i;
	int found;
	unsigned char* buf;
	int sectors;
	int heads;
	int res = 0;

	PED_ASSERT (bios_geom        != NULL);
        PED_ASSERT (part             != NULL);
        PED_ASSERT (part->disk       != NULL);
        PED_ASSERT (part->disk->dev  != NULL);
        PED_ASSERT (part->disk->dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

        buf = ped_malloc (part->disk->dev->sector_size);

	if (!buf)
		return 0;

	if (!part->fs_type)
		goto end;

	found = 0;
	for (i = 0; ms_types[i]; i++) {
		if (!strcmp(ms_types[i], part->fs_type->name))
			found = 1;
	}
	if (!found)
		goto end;

	if (!ped_geometry_read(&part->geom, buf, 0, 1))
		goto end;

	/* shared by the start of all Microsoft file systems */
	sectors = buf[0x18] + (buf[0x19] << 8);
	heads = buf[0x1a] + (buf[0x1b] << 8);

	if (sectors < 1 || sectors > 63)
		goto end;
	if (heads > 255 || heads < 1)
		goto end;

	bios_geom->sectors = sectors;
	bios_geom->heads = heads;
	bios_geom->cylinders = part->disk->dev->length / (sectors * heads);
	res = 1;
end:
	free(buf);
	return res;
}

/* This function attempts to infer the BIOS CHS geometry of the hard disk
 * from the CHS + LBA information contained in the partition table from
 * a single partition's entry.
 *
 * This involves some maths.  Let (c,h,s,a) be the starting cylinder,
 * starting head, starting sector and LBA start address of the partition.
 * Likewise, (C,H,S,A) the end addresses.  Using both of these pieces
 * of information, we want to deduce cyl_sectors and head_sectors which
 * are the sizes of a single cylinder and a single head, respectively.
 *
 * The relationships are:
 * c*cyl_sectors + h * head_sectors + s = a
 * C*cyl_sectors + H * head_sectors + S = A
 *
 * We can rewrite this in matrix form:
 *
 * [ c h ] [ cyl_sectors  ]  =  [ s - a ]  =  [ a_ ]
 * [ C H ] [ head_sectors ]     [ S - A ]     [ A_ ].
 *
 * (s - a is abbreviated to a_to simplify the notation.)
 *
 * This can be abbreviated into augmented matrix form:
 *
 * [ c h | a_ ]
 * [ C H | A_ ].
 *
 * Solving these equations requires following the row reduction algorithm.  We
 * need to be careful about a few things though:
 * 	- the equations might be linearly dependent, in which case there
 * 	are many solutions.
 * 	- the equations might be inconsistent, in which case there
 * 	are no solutions.  (Inconsistent partition table entry!)
 * 	- there might be zeros, so we need to be careful about applying
 * 	the algorithm.  We know, however, that C > 0.
 */
static int
probe_partition_for_geom (const PedPartition* part, PedCHSGeometry* bios_geom)
{
	DosPartitionData* dos_data;
	RawCHS* start_chs;
	RawCHS* end_chs;
	PedSector c, h, s, a, a_;	/* start */
	PedSector C, H, S, A, A_;	/* end */
	PedSector dont_overflow, denum;
	PedSector cyl_size, head_size;
	PedSector cylinders, heads, sectors;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	PED_ASSERT (bios_geom != NULL);

	dos_data = part->disk_specific;

	if (!dos_data->orig)
		return 0;

	start_chs = &dos_data->orig->raw_part.chs_start;
	c = chs_get_cylinder (start_chs);
	h = chs_get_head (start_chs);
	s = chs_get_sector (start_chs);
	a = dos_data->orig->geom.start;
	a_ = a - s;

	end_chs = &dos_data->orig->raw_part.chs_end;
	C = chs_get_cylinder (end_chs);
	H = chs_get_head (end_chs);
	S = chs_get_sector (end_chs);
	A = dos_data->orig->geom.end;
	A_ = A - S;

	if (h < 0 || H < 0 || h > 254 || H > 254)
		return 0;
	if (c > C)
		return 0;

	/* If no geometry is feasible, then don't even bother.
	 * Useful for eliminating assertions for broken partition
	 * tables generated by Norton Ghost et al.
	 */
	if (A > (C+1) * 255 * 63)
		return 0;

	/* Not enough information.  In theory, we can do better.  Should we? */
	if (C > MAX_CHS_CYLINDER)
		return 0;
	if (C == 0)
		return 0;

	/* Calculate the maximum number that can be multiplied by
	 * any head count without overflowing a PedSector
	 * 2^8 = 256, 8 bits + 1(sign bit) = 9
	 */
	dont_overflow = 1;
	dont_overflow <<= (8*sizeof(dont_overflow)) - 9;
	dont_overflow--;

	if (a_ > dont_overflow || A_ > dont_overflow)
		return 0;

	/* The matrix is solved by :
	 *
	 * [ c h | a_]			R1
	 * [ C H | A_]			R2
	 *
	 * (cH - Ch) cyl_size = a_H - A_h		H R1 - h R2
	 * => (if cH - Ch != 0) cyl_size = (a_H - A_h) / (cH - Ch)
	 *
	 * (Hc - hC) head_size = A_c - a_C		c R2 - C R1
	 * => (if cH - Ch != 0) head_size = (A_c - a_C) / (cH - Ch)
	 *
	 *   But this calculation of head_size would need
	 *   not overflowing A_c or a_C
	 *   So substitution is use instead, to minimize dimension
	 *   of temporary results :
	 *
	 * If h != 0 : head_size = ( a_ - c cyl_size ) / h
	 * If H != 0 : head_size = ( A_ - C cyl_size ) / H
	 *
	 */
	denum = c * H - C * h;
	if (denum == 0)
		return 0;

	cyl_size = (a_*H - A_*h) / denum;
	/* Check for non integer result */
	if (cyl_size * denum != a_*H - A_*h)
		return 0;

	if (!(cyl_size > 0))
		return 0;
	if (!(cyl_size <= 255 * 63))
		return 0;

	if (h > 0)
		head_size = ( a_ - c * cyl_size ) / h;
	else if (H > 0)
		head_size = ( A_ - C * cyl_size ) / H;
	else {
		/* should not happen because denum != 0 */
		PED_ASSERT (0);
	}

	if (!(head_size > 0))
		return 0;
	if (!(head_size <= 63))
		return 0;

	cylinders = part->disk->dev->length / cyl_size;
	heads = cyl_size / head_size;
	sectors = head_size;

	if (!(heads > 0))
		return 0;
	if (!(heads < 256))
		return 0;

	if (!(sectors > 0))
		return 0;
	if (!(sectors <= 63))
		return 0;

	/* Some broken OEM partitioning program(s) seem to have an out-by-one
	 * error on the end of partitions.  We should offer to fix the
	 * partition table...
	 */
	if (((C + 1) * heads + H) * sectors + S == A)
		C++;

	if (!((c * heads + h) * sectors + s == a))
		return 0;
	if (!((C * heads + H) * sectors + S == A))
		return 0;

	bios_geom->cylinders = cylinders;
	bios_geom->heads = heads;
	bios_geom->sectors = sectors;

	return 1;
}

static void
partition_probe_bios_geometry (const PedPartition* part,
                               PedCHSGeometry* bios_geom)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (bios_geom != NULL);

	if (ped_partition_is_active (part)) {
		if (probe_partition_for_geom (part, bios_geom))
			return;
		if (part->type & PED_PARTITION_EXTENDED) {
			if (probe_filesystem_for_geom (part, bios_geom))
				return;
		}
	}
	if (part->type & PED_PARTITION_LOGICAL) {
		PedPartition* ext_part;
		ext_part = ped_disk_extended_partition (part->disk);
		PED_ASSERT (ext_part != NULL);
		partition_probe_bios_geometry (ext_part, bios_geom);
	} else {
		*bios_geom = part->disk->dev->bios_geom;
	}
}

static void
disk_probe_bios_geometry (const PedDisk* disk, PedCHSGeometry* bios_geom)
{
	PedPartition*	part;

	/* first look at the boot partition */
	part = NULL;
	while ((part = ped_disk_next_partition (disk, part))) {
		if (!ped_partition_is_active (part))
			continue;
		if (ped_partition_get_flag (part, PED_PARTITION_BOOT)) {
			if (probe_filesystem_for_geom (part, bios_geom))
				return;
			if (probe_partition_for_geom (part, bios_geom))
				return;
		}
	}

	/* that didn't work... try all partition table entries */
	part = NULL;
	while ((part = ped_disk_next_partition (disk, part))) {
		if (ped_partition_is_active (part)) {
			if (probe_partition_for_geom (part, bios_geom))
				return;
		}
	}

	/* that didn't work... look at all file systems */
	part = NULL;
	while ((part = ped_disk_next_partition (disk, part))) {
		if (ped_partition_is_active (part)) {
			if (probe_filesystem_for_geom (part, bios_geom))
				return;
		}
	}
}
#endif /* !DISCOVER_ONLY */

static int _GL_ATTRIBUTE_PURE
raw_part_is_extended (const DosRawPartition* raw_part)
{
	PED_ASSERT (raw_part != NULL);

	switch (raw_part->type) {
	case PARTITION_DOS_EXT:
	case PARTITION_EXT_LBA:
	case PARTITION_LINUX_EXT:
		return 1;

	default:
		return 0;
	}

	return 0;
}

static int _GL_ATTRIBUTE_PURE
raw_part_is_hidden (const DosRawPartition* raw_part)
{
	PED_ASSERT (raw_part != NULL);

	switch (raw_part->type) {
	case PARTITION_FAT12_H:
	case PARTITION_FAT16_SM_H:
	case PARTITION_FAT16_H:
	case PARTITION_FAT32_H:
	case PARTITION_NTFS_H:
	case PARTITION_FAT32_LBA_H:
	case PARTITION_FAT16_LBA_H:
		return 1;

	default:
		return 0;
	}

	return 0;
}

static int _GL_ATTRIBUTE_PURE
raw_part_is_lba (const DosRawPartition* raw_part)
{
	PED_ASSERT (raw_part != NULL);

	switch (raw_part->type) {
	case PARTITION_FAT32_LBA:
	case PARTITION_FAT16_LBA:
	case PARTITION_EXT_LBA:
	case PARTITION_FAT32_LBA_H:
	case PARTITION_FAT16_LBA_H:
		return 1;

	default:
		return 0;
	}

	return 0;
}

static PedPartition*
raw_part_parse (const PedDisk* disk, const DosRawPartition* raw_part,
	        PedSector lba_offset, PedPartitionType type)
{
	PedPartition* part;
	DosPartitionData* dos_data;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (raw_part != NULL);

	part = ped_partition_new (
		disk, type, NULL,
		linear_start (disk, raw_part, lba_offset),
		linear_end (disk, raw_part, lba_offset));
	if (!part)
		return NULL;
	dos_data = part->disk_specific;
	dos_data->system = raw_part->type;
	dos_data->boot = raw_part->boot_ind != 0;
	dos_data->diag = raw_part->type == PARTITION_COMPAQ_DIAG ||
			 raw_part->type == PARTITION_MSFT_RECOVERY ||
			 raw_part->type == PARTITION_DELL_DIAG;
	dos_data->hidden = raw_part_is_hidden (raw_part);
	dos_data->raid = raw_part->type == PARTITION_LINUX_RAID;
	dos_data->lvm = raw_part->type == PARTITION_LINUX_LVM_OLD
			|| raw_part->type == PARTITION_LINUX_LVM;
	dos_data->lba = raw_part_is_lba (raw_part);
	dos_data->palo = raw_part->type == PARTITION_PALO;
	dos_data->prep = raw_part->type == PARTITION_PREP;
	dos_data->orig = ped_malloc (sizeof (OrigState));
	if (!dos_data->orig) {
		ped_partition_destroy (part);
		return NULL;
	}
	dos_data->orig->geom = part->geom;
	dos_data->orig->raw_part = *raw_part;
	dos_data->orig->lba_offset = lba_offset;
	return part;
}

static int
read_table (PedDisk* disk, PedSector sector, int is_extended_table)
{
	int			i;
	DosRawTable*		table;
	DosRawPartition*	raw_part;
	PedPartition*		part;
	PedPartitionType	type;
	PedSector		lba_offset;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	void *label = NULL;
	if (!ptt_read_sector (disk->dev, sector, &label))
		goto error;

        table = (DosRawTable *) label;

	/* weird: empty extended partitions are filled with 0xf6 by PM */
	if (is_extended_table
	    && PED_LE16_TO_CPU (table->magic) == PARTITION_MAGIC_MAGIC)
		goto read_ok;

#ifndef DISCOVER_ONLY
	if (PED_LE16_TO_CPU (table->magic) != MSDOS_MAGIC) {
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_IGNORE_CANCEL,
			_("Invalid partition table on %s "
			  "-- wrong signature %x."),
			disk->dev->path,
			PED_LE16_TO_CPU (table->magic))
				!= PED_EXCEPTION_IGNORE)
			goto error;
		goto read_ok;
	}
#endif

	/* parse the partitions from this table */
	for (i = 0; i < DOS_N_PRI_PARTITIONS; i++) {
		raw_part = &table->partitions [i];
		if (raw_part->type == PARTITION_EMPTY || !raw_part->length)
			continue;

		/* process nested extended partitions after normal logical
		 * partitions, to make sure we get the order right.
		 */
		if (is_extended_table && raw_part_is_extended (raw_part))
			continue;

		lba_offset = is_extended_table ? sector : 0;

		if (linear_start (disk, raw_part, lba_offset) == sector) {
			if (ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("Invalid partition table - recursive "
				"partition on %s."),
				disk->dev->path)
					!= PED_EXCEPTION_IGNORE)
				goto error;
			continue;	/* avoid infinite recursion */
		}

		if (is_extended_table)
			type = PED_PARTITION_LOGICAL;
		else if (raw_part_is_extended (raw_part))
			type = PED_PARTITION_EXTENDED;
		else
			type = PED_PARTITION_NORMAL;

		part = raw_part_parse (disk, raw_part, lba_offset, type);
		if (!part)
			goto error;
		if (!is_extended_table)
			part->num = i + 1;
		if (type != PED_PARTITION_EXTENDED)
			part->fs_type = ped_file_system_probe (&part->geom);

		PedConstraint *constraint_exact
		  = ped_constraint_exact (&part->geom);
		bool ok = ped_disk_add_partition (disk, part, constraint_exact);
		ped_constraint_destroy (constraint_exact);
		if (!ok)
			goto error;

		/* non-nested extended partition */
		if (part->type == PED_PARTITION_EXTENDED) {
			if (!read_table (disk, part->geom.start, 1))
				goto error;
		}
	}

	if (is_extended_table) {
		/* process the nested extended partitions */
		for (i = 0; i < DOS_N_PRI_PARTITIONS; i++) {
			PedSector part_start;

			raw_part = &table->partitions [i];
			if (!raw_part_is_extended (raw_part))
				continue;

			lba_offset = ped_disk_extended_partition
					(disk)->geom.start;
			part_start = linear_start (disk, raw_part, lba_offset);
			if (part_start == sector) {
				/* recursive table - already threw an
				 * exception above.
				 */
				continue;
			}
			if (!read_table (disk, part_start, 1))
				goto error;
		}
	}

read_ok:
	free (label);
	return 1;

error:
	free (label);
	ped_disk_delete_all (disk);
	return 0;
}

static int
msdos_read (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	ped_disk_delete_all (disk);
	if (!read_table (disk, 0, 0))
		return 0;

#ifndef DISCOVER_ONLY
	/* try to figure out the correct BIOS CHS values */
	if (!disk_check_bios_geometry (disk, &disk->dev->bios_geom)) {
		PedCHSGeometry bios_geom = disk->dev->bios_geom;
		disk_probe_bios_geometry (disk, &bios_geom);

		/* if the geometry was wrong, then we should reread, to
		 * make sure the metadata is allocated in the right places.
		 */
		if (disk->dev->bios_geom.cylinders != bios_geom.cylinders
		    || disk->dev->bios_geom.heads != bios_geom.heads
		    || disk->dev->bios_geom.sectors != bios_geom.sectors) {
			disk->dev->bios_geom = bios_geom;
			return msdos_read (disk);
		}
	}
#endif

	return 1;
}

#ifndef DISCOVER_ONLY
static int
fill_raw_part (DosRawPartition* raw_part,
               const PedPartition* part, PedSector offset)
{
	DosPartitionData*	dos_data;
	PedCHSGeometry		bios_geom;

	PED_ASSERT (raw_part != NULL);
	PED_ASSERT (part != NULL);

	partition_probe_bios_geometry (part, &bios_geom);

	dos_data = part->disk_specific;

	raw_part->boot_ind = 0x80 * dos_data->boot;
	raw_part->type = dos_data->system;
	raw_part->start = PED_CPU_TO_LE32 (part->geom.start - offset);
	raw_part->length = PED_CPU_TO_LE32 (part->geom.length);

	sector_to_chs (part->disk->dev, &bios_geom, part->geom.start,
		       &raw_part->chs_start);
	sector_to_chs (part->disk->dev, &bios_geom, part->geom.end,
		       &raw_part->chs_end);

	if (dos_data->orig) {
		DosRawPartition* orig_raw_part = &dos_data->orig->raw_part;
		if (dos_data->orig->geom.start == part->geom.start)
			raw_part->chs_start = orig_raw_part->chs_start;
		if (dos_data->orig->geom.end == part->geom.end)
			raw_part->chs_end = orig_raw_part->chs_end;
	}

	return 1;
}

static int
fill_ext_raw_part_geom (DosRawPartition* raw_part,
                        const PedCHSGeometry* bios_geom,
			const PedGeometry* geom, PedSector offset)
{
	PED_ASSERT (raw_part != NULL);
	PED_ASSERT (geom != NULL);
	PED_ASSERT (geom->dev != NULL);

	raw_part->boot_ind = 0;
	raw_part->type = PARTITION_DOS_EXT;
	raw_part->start = PED_CPU_TO_LE32 (geom->start - offset);
	raw_part->length = PED_CPU_TO_LE32 (geom->length);

	sector_to_chs (geom->dev, bios_geom, geom->start, &raw_part->chs_start);
	sector_to_chs (geom->dev, bios_geom, geom->start + geom->length - 1,
		       &raw_part->chs_end);

	return 1;
}

static int
write_ext_table (const PedDisk* disk,
                 PedSector sector, const PedPartition* logical)
{
	PedPartition*		part;
	PedSector		lba_offset;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (ped_disk_extended_partition (disk) != NULL);
	PED_ASSERT (logical != NULL);

	lba_offset = ped_disk_extended_partition (disk)->geom.start;

	void* s;
	if (!ptt_read_sector (disk->dev, sector, &s))
		return 0;

	DosRawTable *table = s;
	memset(&(table->partitions), 0, sizeof (table->partitions));
	table->magic = PED_CPU_TO_LE16 (MSDOS_MAGIC);

	int ok = 0;
	if (!fill_raw_part (&table->partitions[0], logical, sector))
		goto cleanup;

	part = ped_disk_get_partition (disk, logical->num + 1);
	if (part) {
		PedGeometry*		geom;
		PedCHSGeometry		bios_geom;

		geom = ped_geometry_new (disk->dev, part->prev->geom.start,
				part->geom.end - part->prev->geom.start + 1);
		if (!geom)
			goto cleanup;
		partition_probe_bios_geometry (part, &bios_geom);
		fill_ext_raw_part_geom (&table->partitions[1], &bios_geom,
				        geom, lba_offset);
		ped_geometry_destroy (geom);

		if (!write_ext_table (disk, part->prev->geom.start, part))
			goto cleanup;
	}

	ok = ped_device_write (disk->dev, table, sector, 1);
 cleanup:
	free (s);
	return ok;
}

static int
write_empty_table (const PedDisk* disk, PedSector sector)
{
	DosRawTable		table;
	void*			table_sector;

	PED_ASSERT (disk != NULL);

	if (ptt_read_sector (disk->dev, sector, &table_sector)) {
		memcpy (&table, table_sector, sizeof (table));
		free(table_sector);
	}
	memset (&(table.partitions), 0, sizeof (table.partitions));
	table.magic = PED_CPU_TO_LE16 (MSDOS_MAGIC);

	return ped_device_write (disk->dev, (void*) &table, sector, 1);
}

/* Find the first logical partition, and write the partition table for it.
 */
static int
write_extended_partitions (const PedDisk* disk)
{
	PedPartition*		ext_part;
	PedPartition*		part;
	PedCHSGeometry		bios_geom;

	PED_ASSERT (disk != NULL);

	ext_part = ped_disk_extended_partition (disk);
	partition_probe_bios_geometry (ext_part, &bios_geom);
	part = ped_disk_get_partition (disk, 5);
	if (part)
		return write_ext_table (disk, ext_part->geom.start, part);
	else
		return write_empty_table (disk, ext_part->geom.start);
}

static inline uint32_t generate_random_id (void)
{
	struct timeval tv;
	int rc;
	rc = gettimeofday(&tv, NULL);
	if (rc == -1)
		return 0;
	return (uint32_t)(tv.tv_usec & 0xFFFFFFFFUL);
}

static int
msdos_write (const PedDisk* disk)
{
	PedPartition*		part;
	int			i;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	void *s0;
	if (!ptt_read_sector (disk->dev, 0, &s0))
		return 0;
	DosRawTable *table = (DosRawTable *) s0;

	if (!table->boot_code[0]) {
		memset (table->boot_code, 0, 512);
		memcpy (table->boot_code, MBR_BOOT_CODE, sizeof (MBR_BOOT_CODE));
	}

	/* If there is no unique identifier, generate a random one */
	if (!table->mbr_signature)
		table->mbr_signature = generate_random_id();

	memset (table->partitions, 0, sizeof (table->partitions));
	table->magic = PED_CPU_TO_LE16 (MSDOS_MAGIC);

	for (i=1; i<=DOS_N_PRI_PARTITIONS; i++) {
		part = ped_disk_get_partition (disk, i);
		if (!part)
			continue;

		if (!fill_raw_part (&table->partitions [i - 1], part, 0))
			goto write_fail;

		if (part->type == PED_PARTITION_EXTENDED) {
			if (!write_extended_partitions (disk))
				goto write_fail;
		}
	}

        int write_ok = ped_device_write (disk->dev, (void*) table, 0, 1);
        free (s0);
	if (!write_ok)
		return 0;
	return ped_device_sync (disk->dev);

 write_fail:
        free (s0);
        return 0;

}
#endif /* !DISCOVER_ONLY */

static PedPartition*
msdos_partition_new (const PedDisk* disk, PedPartitionType part_type,
		     const PedFileSystemType* fs_type,
		     PedSector start, PedSector end)
{
	PedPartition*		part;
	DosPartitionData*	dos_data;

	part = _ped_partition_alloc (disk, part_type, fs_type, start, end);
	if (!part)
		goto error;

	if (ped_partition_is_active (part)) {
		part->disk_specific
		       	= dos_data = ped_malloc (sizeof (DosPartitionData));
		if (!dos_data)
			goto error_free_part;
		dos_data->orig = NULL;
		dos_data->system = PARTITION_LINUX;
		dos_data->hidden = 0;
		dos_data->boot = 0;
		dos_data->diag = 0;
		dos_data->raid = 0;
		dos_data->lvm = 0;
		dos_data->lba = 0;
		dos_data->palo = 0;
		dos_data->prep = 0;
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
msdos_partition_duplicate (const PedPartition* part)
{
	PedPartition*		new_part;
	DosPartitionData*	new_dos_data;
	DosPartitionData*	old_dos_data;

	new_part = ped_partition_new (part->disk, part->type, part->fs_type,
				      part->geom.start, part->geom.end);
	if (!new_part)
		return NULL;
	new_part->num = part->num;

	old_dos_data = (DosPartitionData*) part->disk_specific;
	new_dos_data = (DosPartitionData*) new_part->disk_specific;
	new_dos_data->system = old_dos_data->system;
	new_dos_data->boot = old_dos_data->boot;
	new_dos_data->diag = old_dos_data->diag;
	new_dos_data->hidden = old_dos_data->hidden;
	new_dos_data->raid = old_dos_data->raid;
	new_dos_data->lvm = old_dos_data->lvm;
	new_dos_data->lba = old_dos_data->lba;
	new_dos_data->palo = old_dos_data->palo;
	new_dos_data->prep = old_dos_data->prep;

	if (old_dos_data->orig) {
		new_dos_data->orig = ped_malloc (sizeof (OrigState));
		if (!new_dos_data->orig) {
			ped_partition_destroy (new_part);
			return NULL;
		}
		new_dos_data->orig->geom = old_dos_data->orig->geom;
		new_dos_data->orig->raw_part = old_dos_data->orig->raw_part;
		new_dos_data->orig->lba_offset = old_dos_data->orig->lba_offset;
	}
	return new_part;
}

static void
msdos_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);

	if (ped_partition_is_active (part)) {
		DosPartitionData* dos_data;
		dos_data = (DosPartitionData*) part->disk_specific;
		free (dos_data->orig);
		free (part->disk_specific);
	}
	free (part);
}

static int
msdos_partition_set_system (PedPartition* part,
			    const PedFileSystemType* fs_type)
{
	DosPartitionData* dos_data = part->disk_specific;

	part->fs_type = fs_type;

	if (dos_data->hidden
		    && fs_type
		    && strncmp (fs_type->name, "fat", 3) != 0
		    && strcmp (fs_type->name, "ntfs") != 0)
		dos_data->hidden = 0;

	if (part->type & PED_PARTITION_EXTENDED) {
		dos_data->diag = 0;
		dos_data->raid = 0;
		dos_data->lvm = 0;
		dos_data->palo = 0;
		dos_data->prep = 0;
		if (dos_data->lba)
			dos_data->system = PARTITION_EXT_LBA;
		else
			dos_data->system = PARTITION_DOS_EXT;
		return 1;
	}

	if (dos_data->diag) {
		/* Don't change the system if it already is a diag type,
		   otherwise use Compaq as almost all vendors use that. */
		if (dos_data->system != PARTITION_COMPAQ_DIAG &&
		    dos_data->system != PARTITION_MSFT_RECOVERY &&
		    dos_data->system != PARTITION_DELL_DIAG)
			dos_data->system = PARTITION_COMPAQ_DIAG;
		return 1;
	}
	if (dos_data->lvm) {
		dos_data->system = PARTITION_LINUX_LVM;
		return 1;
	}
	if (dos_data->raid) {
		dos_data->system = PARTITION_LINUX_RAID;
		return 1;
	}
	if (dos_data->palo) {
		dos_data->system = PARTITION_PALO;
		return 1;
	}
	if (dos_data->prep) {
		dos_data->system = PARTITION_PREP;
		return 1;
	}

	if (!fs_type)
		dos_data->system = PARTITION_LINUX;
	else if (!strcmp (fs_type->name, "fat16")) {
		dos_data->system = dos_data->lba
				   ? PARTITION_FAT16_LBA : PARTITION_FAT16;
		dos_data->system |= dos_data->hidden ? PART_FLAG_HIDDEN : 0;
	} else if (!strcmp (fs_type->name, "fat32")) {
		dos_data->system = dos_data->lba
				   ? PARTITION_FAT32_LBA : PARTITION_FAT32;
		dos_data->system |= dos_data->hidden ? PART_FLAG_HIDDEN : 0;
	} else if (!strcmp (fs_type->name, "ntfs")
		   || !strcmp (fs_type->name, "hpfs")) {
		dos_data->system = PARTITION_NTFS;
		dos_data->system |= dos_data->hidden ? PART_FLAG_HIDDEN : 0;
	} else if (!strcmp (fs_type->name, "hfs")
		   || !strcmp (fs_type->name, "hfs+"))
		dos_data->system = PARTITION_HFS;
	else if (!strcmp (fs_type->name, "sun-ufs"))
		dos_data->system = PARTITION_SUN_UFS;
	else if (is_linux_swap (fs_type->name))
		dos_data->system = PARTITION_LINUX_SWAP;
	else
		dos_data->system = PARTITION_LINUX;

	return 1;
}

static void
clear_flags (DosPartitionData *dos_data)
{
  dos_data->diag = 0;
  dos_data->hidden = 0;
  dos_data->lvm = 0;
  dos_data->palo = 0;
  dos_data->prep = 0;
  dos_data->raid = 0;
}

static int
msdos_partition_set_flag (PedPartition* part,
                          PedPartitionFlag flag, int state)
{
	PedDisk*			disk;
	PedPartition*			walk;
	DosPartitionData*		dos_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);
	PED_ASSERT (part->disk != NULL);

	dos_data = part->disk_specific;
	disk = part->disk;

	switch (flag) {
	case PED_PARTITION_HIDDEN:
		if (part->type == PED_PARTITION_EXTENDED) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Extended partitions cannot be hidden on "
				  "msdos disk labels."));
			return 0;
		}
		dos_data->hidden = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_BOOT:
		dos_data->boot = state;
		if (!state)
			return 1;

		walk = ped_disk_next_partition (disk, NULL);
		for (; walk; walk = ped_disk_next_partition (disk, walk)) {
			if (walk == part || !ped_partition_is_active (walk))
				continue;
			msdos_partition_set_flag (walk, PED_PARTITION_BOOT, 0);
		}
		return 1;

	case PED_PARTITION_DIAG:
		if (state)
			clear_flags (dos_data);
		dos_data->diag = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_RAID:
		if (state)
			clear_flags (dos_data);
		dos_data->raid = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_LVM:
		if (state)
			clear_flags (dos_data);
		dos_data->lvm = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_LBA:
		dos_data->lba = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_PALO:
		if (state)
			clear_flags (dos_data);
		dos_data->palo = state;
		return ped_partition_set_system (part, part->fs_type);

	case PED_PARTITION_PREP:
		if (state)
			clear_flags (dos_data);
		dos_data->prep = state;
		return ped_partition_set_system (part, part->fs_type);

	default:
		return 0;
	}
}

static int _GL_ATTRIBUTE_PURE
msdos_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	DosPartitionData*	dos_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	dos_data = part->disk_specific;
	switch (flag) {
	case PED_PARTITION_HIDDEN:
		if (part->type == PED_PARTITION_EXTENDED)
			return 0;
		else
			return dos_data->hidden;

	case PED_PARTITION_BOOT:
		return dos_data->boot;

	case PED_PARTITION_DIAG:
		return dos_data->diag;

	case PED_PARTITION_RAID:
		return dos_data->raid;

	case PED_PARTITION_LVM:
		return dos_data->lvm;

	case PED_PARTITION_LBA:
		return dos_data->lba;

	case PED_PARTITION_PALO:
		return dos_data->palo;

	case PED_PARTITION_PREP:
		return dos_data->prep;

	default:
		return 0;
	}
}

static int
msdos_partition_is_flag_available (const PedPartition* part,
				   PedPartitionFlag flag)
{
	switch (flag) {
	case PED_PARTITION_HIDDEN:
		if (part->type == PED_PARTITION_EXTENDED)
			return 0;
		else
			return 1;

	case PED_PARTITION_BOOT:
	case PED_PARTITION_RAID:
	case PED_PARTITION_LVM:
	case PED_PARTITION_LBA:
	case PED_PARTITION_PALO:
	case PED_PARTITION_PREP:
	case PED_PARTITION_DIAG:
		return 1;

	default:
		return 0;
	}
}

static PedGeometry*
_try_constraint (const PedPartition* part, const PedConstraint* external,
		 PedConstraint* internal)
{
	PedConstraint*		intersection;
	PedGeometry*		solution;

	intersection = ped_constraint_intersect (external, internal);
	ped_constraint_destroy (internal);
	if (!intersection)
		return NULL;

	solution = ped_constraint_solve_nearest (intersection, &part->geom);
	ped_constraint_destroy (intersection);
	return solution;
}

static PedGeometry*
_best_solution (const PedPartition* part, const PedCHSGeometry* bios_geom,
		PedGeometry* a, PedGeometry* b)
{
	PedSector	cyl_size = bios_geom->heads * bios_geom->sectors;
	int		a_cylinder;
	int		b_cylinder;

	if (!a)
		return b;
	if (!b)
		return a;

	a_cylinder = a->start / cyl_size;
	b_cylinder = b->start / cyl_size;

	if (a_cylinder == b_cylinder) {
		if ( (a->start / bios_geom->sectors) % bios_geom->heads
			  < (b->start / bios_geom->sectors) % bios_geom->heads)
	       		goto choose_a;
		else
			goto choose_b;
	} else {
		PedSector	a_delta;
		PedSector	b_delta;

		a_delta = abs (part->geom.start - a->start);
		b_delta = abs (part->geom.start - b->start);

		if (a_delta < b_delta)
			goto choose_a;
		else
			goto choose_b;
	}

	return NULL;	/* never get here! */

choose_a:
	ped_geometry_destroy (b);
	return a;

choose_b:
	ped_geometry_destroy (a);
	return b;
}

/* This constraint is for "normal" primary partitions, that start at the
 * beginning of a cylinder, and end at the end of a cylinder.
 * 	Note: you can't start a partition at the beginning of the 1st
 * cylinder, because that's where the partition table is!  There are different
 * rules for that - see the _primary_start_constraint.
 */
static PedConstraint*
_primary_constraint (const PedDisk* disk, const PedCHSGeometry* bios_geom,
		     PedGeometry* min_geom)
{
	PedDevice*	dev = disk->dev;
	PedSector	cylinder_size = bios_geom->sectors * bios_geom->heads;
	PedAlignment	start_align;
	PedAlignment	end_align;
	PedGeometry	start_geom;
	PedGeometry	end_geom;

	if (!ped_alignment_init (&start_align, 0, cylinder_size))
		return NULL;
	if (!ped_alignment_init (&end_align, -1, cylinder_size))
		return NULL;

	if (min_geom) {
		if (min_geom->start < cylinder_size)
			return NULL;
		if (!ped_geometry_init (&start_geom, dev, cylinder_size,
			       		min_geom->start + 1 - cylinder_size))
			return NULL;
		if (!ped_geometry_init (&end_geom, dev, min_geom->end,
			       		dev->length - min_geom->end))
			return NULL;
	} else {
		/* Use cylinder_size as the starting sector number
		   when the device is large enough to accommodate that.
		   Otherwise, use sector 1.  */
		PedSector start = (cylinder_size < dev->length
				   ? cylinder_size : 1);
		if (!ped_geometry_init (&start_geom, dev, start,
					dev->length - start))
			return NULL;
		if (!ped_geometry_init (&end_geom, dev, 0, dev->length))
			return NULL;
	}

	return ped_constraint_new (&start_align, &end_align, &start_geom,
				   &end_geom, 1, dev->length);
}

/* This constraint is for partitions starting on the first cylinder.  They
 * must start on the 2nd head of the 1st cylinder.
 *
 * NOTE: We don't always start on the 2nd head of the 1st cylinder.  Windows
 * Vista aligns starting partitions at sector 2048 (0x800) by default.  See:
 * http://support.microsoft.com/kb/923332
 */
static PedConstraint*
_primary_start_constraint (const PedDisk* disk,
                           const PedPartition *part,
                           const PedCHSGeometry* bios_geom,
                           const PedGeometry* min_geom)
{
	PedDevice*	dev = disk->dev;
	PedSector	cylinder_size = bios_geom->sectors * bios_geom->heads;
	PedAlignment	start_align;
	PedAlignment	end_align;
	PedGeometry	start_geom;
	PedGeometry	end_geom;
	PedSector start_pos;

	if (part->geom.start == 2048)
		/* check for known Windows Vista (NTFS >= 3.1) alignments */
		/* sector 0x800 == 2048                                   */
		start_pos = 2048;
	else
		/* all other primary partitions on a DOS label align to   */
		/* the 2nd head of the first cylinder (0x3F == 63)        */
		start_pos = bios_geom->sectors;

	if (!ped_alignment_init (&start_align, start_pos, 0))
		return NULL;
	if (!ped_alignment_init (&end_align, -1, cylinder_size))
		return NULL;
	if (min_geom) {
		if (!ped_geometry_init (&start_geom, dev, start_pos, 1))
			return NULL;
		if (!ped_geometry_init (&end_geom, dev, min_geom->end,
			       		dev->length - min_geom->end))
			return NULL;
	} else {
		if (!ped_geometry_init (&start_geom, dev, start_pos,
			dev->length - start_pos))
			return NULL;
		if (!ped_geometry_init (&end_geom, dev, 0, dev->length))
			return NULL;
	}

	return ped_constraint_new (&start_align, &end_align, &start_geom,
				   &end_geom, 1, dev->length);
}

/* constraints for logical partitions:
 * 	- start_offset is the offset in the start alignment.  "normally",
 * this is bios_geom->sectors.  exceptions: MINOR > 5 at the beginning of the
 * extended partition, or MINOR == 5 in the middle of the extended partition
 * 	- is_start_part == 1 if the constraint is for the first cylinder of
 * the extended partition, or == 0 if the constraint is for the second cylinder
 * onwards of the extended partition.
 */
static PedConstraint*
_logical_constraint (const PedDisk* disk, const PedCHSGeometry* bios_geom,
		     PedSector start_offset, int is_start_part)
{
	PedPartition*	ext_part = ped_disk_extended_partition (disk);
	PedDevice*	dev = disk->dev;
	PedSector	cylinder_size = bios_geom->sectors * bios_geom->heads;
	PedAlignment	start_align;
	PedAlignment	end_align;
	PedGeometry	max_geom;

	PED_ASSERT (ext_part != NULL);

	if (!ped_alignment_init (&start_align, start_offset, cylinder_size))
		return NULL;
	if (!ped_alignment_init (&end_align, -1, cylinder_size))
		return NULL;
	if (is_start_part) {
		if (!ped_geometry_init (&max_geom, dev,
					ext_part->geom.start,
					ext_part->geom.length))
			return NULL;
	} else {
		PedSector	min_start;
		PedSector	max_length;

		min_start = ped_round_up_to (ext_part->geom.start + 1,
					     cylinder_size);
		max_length = ext_part->geom.end - min_start + 1;
		if (min_start >= ext_part->geom.end)
			return NULL;

		if (!ped_geometry_init (&max_geom, dev, min_start, max_length))
			return NULL;
	}

	return ped_constraint_new (&start_align, &end_align, &max_geom,
		       		   &max_geom, 1, dev->length);
}

/* returns the minimum geometry for the extended partition, given that the
 * extended partition must contain:
 *   * all logical partitions
 *   * all partition tables for all logical partitions (except the first)
 *   * the extended partition table
 */
static PedGeometry*
_get_min_extended_part_geom (const PedPartition* ext_part,
			     const PedCHSGeometry* bios_geom)
{
	PedDisk*		disk = ext_part->disk;
	PedSector		head_size = bios_geom ? bios_geom->sectors : 1;
	PedPartition*		walk;
	PedGeometry*		min_geom;

	walk = ped_disk_get_partition (disk, 5);
	if (!walk)
		return NULL;

	min_geom = ped_geometry_duplicate (&walk->geom);
	if (!min_geom)
		return NULL;
	/* We must always allow at least two sectors at the start, to leave
	 * room for LILO.  See linux/fs/partitions/msdos.c.
	 */
	ped_geometry_set_start (min_geom,
				walk->geom.start - PED_MAX (1 * head_size, 2));

	for (walk = ext_part->part_list; walk; walk = walk->next) {
		if (!ped_partition_is_active (walk) || walk->num == 5)
			continue;
		if (walk->geom.start < min_geom->start)
			ped_geometry_set_start (min_geom,
					walk->geom.start - 2 * head_size);
		if (walk->geom.end > min_geom->end)
			ped_geometry_set_end (min_geom, walk->geom.end);
	}

	return min_geom;
}

static int
_align_primary (PedPartition* part, const PedCHSGeometry* bios_geom,
		const PedConstraint* constraint)
{
	PedDisk*	disk = part->disk;
	PedGeometry*	min_geom = NULL;
	PedGeometry*	solution = NULL;

	if (part->type == PED_PARTITION_EXTENDED)
		min_geom = _get_min_extended_part_geom (part, bios_geom);

	solution = _best_solution (part, bios_geom, solution,
			_try_constraint (part, constraint,
					 _primary_start_constraint (disk, part,
						 bios_geom, min_geom)));

	solution = _best_solution (part, bios_geom, solution,
			_try_constraint (part, constraint,
				_primary_constraint (disk, bios_geom,
				min_geom)));

	if (min_geom)
		ped_geometry_destroy (min_geom);

	if (solution) {
		ped_geometry_set (&part->geom, solution->start,
				  solution->length);
		ped_geometry_destroy (solution);
		return 1;
	}

	return 0;
}

static int
_logical_min_start_head (const PedPartition* part,
                         const PedCHSGeometry* bios_geom,
			 const PedPartition* ext_part,
                         int is_start_ext_part)
{
	PedSector	cylinder_size = bios_geom->sectors * bios_geom->heads;
	PedSector	base_head;

	if (is_start_ext_part)
		base_head = 1 + (ext_part->geom.start % cylinder_size)
					/ bios_geom->sectors;
	else
		base_head = 0;

	if (part->num == 5)
		return base_head + 0;
	else
		return base_head + 1;
}

/* Shamelessly copied and adapted from _partition_get_overlap_constraint
 * (in disk.c)
 * This should get rid of the infamous Assertion (metadata_length > 0) failed
 * bug for extended msdos disklabels generated by Parted.
 * 1) There always is a partition table at the start of ext_part, so we leave
 *    a one sector gap there.
 * 2)*The partition table of part5 is always at the beginning of the ext_part
 *    so there is no need to leave a one sector gap before part5.
 *   *There always is a partition table at the beginning of each partition != 5.
 * We don't need to worry to much about consistency with
 * _partition_get_overlap_constraint because missing it means we are in edge
 * cases anyway, and we don't lose anything by just refusing to do the job in
 * those cases.
 */
static PedConstraint*
_log_meta_overlap_constraint (PedPartition* part, const PedGeometry* geom)
{
	PedGeometry	safe_space;
	PedSector	min_start;
	PedSector	max_end;
	PedPartition*	ext_part = ped_disk_extended_partition (part->disk);
	PedPartition*	walk;
	int		not_5 = (part->num != 5);

	PED_ASSERT (ext_part != NULL);

	walk = ext_part->part_list;

	/*                                 1)  2)     */
	min_start = ext_part->geom.start + 1 + not_5;
	max_end = ext_part->geom.end;

	while (walk != NULL             /*      2)                         2) */
		&& (   walk->geom.start - (walk->num != 5) < geom->start - not_5
		    || walk->geom.start - (walk->num != 5) <= min_start )) {
		if (walk != part && ped_partition_is_active (walk))
			min_start = walk->geom.end + 1 + not_5; /* 2) */
		walk = walk->next;
	}

	while (walk && (walk == part || !ped_partition_is_active (walk)))
		walk = walk->next;

	if (walk)
		max_end = walk->geom.start - 1 - (walk->num != 5); /* 2) */

	if (min_start >= max_end)
		return NULL;

	ped_geometry_init (&safe_space, part->disk->dev,
			   min_start, max_end - min_start + 1);
	return ped_constraint_new_from_max (&safe_space);
}

static int
_align_logical (PedPartition* part, const PedCHSGeometry* bios_geom,
		const PedConstraint* constraint)
{
	PedDisk*	disk = part->disk;
	PedPartition*	ext_part = ped_disk_extended_partition (disk);
	PedSector	cyl_size = bios_geom->sectors * bios_geom->heads;
	PedSector	start_base;
	int		head;
	PedGeometry*	solution = NULL;
	PedConstraint   *intersect, *log_meta_overlap;

	PED_ASSERT (ext_part != NULL);

	log_meta_overlap = _log_meta_overlap_constraint(part, &part->geom);
	intersect = ped_constraint_intersect (constraint, log_meta_overlap);
	ped_constraint_destroy (log_meta_overlap);
	if (!intersect)
		return 0;

	start_base = ped_round_down_to (part->geom.start, cyl_size);

	for (head = _logical_min_start_head (part, bios_geom, ext_part, 0);
	     head < PED_MIN (5, bios_geom->heads); head++) {
		PedConstraint*	disk_constraint;
		PedSector	start = start_base + head * bios_geom->sectors;

		if (head >= _logical_min_start_head (part, bios_geom,
						     ext_part, 1))
			disk_constraint =
				_logical_constraint (disk, bios_geom, start, 1);
		else
			disk_constraint =
				_logical_constraint (disk, bios_geom, start, 0);

		solution = _best_solution (part, bios_geom, solution,
				_try_constraint (part, intersect,
						 disk_constraint));
	}

	ped_constraint_destroy (intersect);

	if (solution) {
		ped_geometry_set (&part->geom, solution->start,
				  solution->length);
		ped_geometry_destroy (solution);
		return 1;
	}

	return 0;
}

static int
_align (PedPartition* part, const PedCHSGeometry* bios_geom,
	const PedConstraint* constraint)
{
	if (part->type == PED_PARTITION_LOGICAL)
		return _align_logical (part, bios_geom, constraint);
	else
		return _align_primary (part, bios_geom, constraint);
}

static PedConstraint*
_no_geom_constraint (const PedDisk* disk, PedSector start, PedSector end)
{
	PedGeometry	 max;

	ped_geometry_init (&max, disk->dev, start, end - start + 1);
	return ped_constraint_new_from_max (&max);
}

static PedConstraint*
_no_geom_extended_constraint (const PedPartition* part)
{
	PedDevice*	dev = part->disk->dev;
	PedGeometry*	min = _get_min_extended_part_geom (part, NULL);
	PedGeometry	start_range;
	PedGeometry	end_range;
	PedConstraint*	constraint;

	if (min) {
		ped_geometry_init (&start_range, dev, 1, min->start);
		ped_geometry_init (&end_range, dev, min->end,
				   dev->length - min->end);
		ped_geometry_destroy (min);
	} else {
		ped_geometry_init (&start_range, dev, 1, dev->length - 1);
		ped_geometry_init (&end_range, dev, 1, dev->length - 1);
	}
	constraint = ped_constraint_new (ped_alignment_any, ped_alignment_any,
			&start_range, &end_range, 1, dev->length);
	return constraint;
}

static int
_align_primary_no_geom (PedPartition* part, const PedConstraint* constraint)
{
	PedDisk*	disk = part->disk;
	PedGeometry*	solution;

	if (part->type == PED_PARTITION_EXTENDED) {
		solution = _try_constraint (part, constraint,
				_no_geom_extended_constraint (part));
	} else {
		solution = _try_constraint (part, constraint,
				_no_geom_constraint (disk, 1,
						     disk->dev->length - 1));
	}

	if (solution) {
		ped_geometry_set (&part->geom, solution->start,
				  solution->length);
		ped_geometry_destroy (solution);
		return 1;
	}
	return 0;
}

static int
_align_logical_no_geom (PedPartition* part, const PedConstraint* constraint)
{
	PedGeometry*	solution;

	solution = _try_constraint (part, constraint,
			_log_meta_overlap_constraint (part, &part->geom));

	if (solution) {
		ped_geometry_set (&part->geom, solution->start,
				  solution->length);
		ped_geometry_destroy (solution);
		return 1;
	}
	return 0;
}

static int
_align_no_geom (PedPartition* part, const PedConstraint* constraint)
{
	if (part->type == PED_PARTITION_LOGICAL)
		return _align_logical_no_geom (part, constraint);
	else
		return _align_primary_no_geom (part, constraint);
}

static int
msdos_partition_align (PedPartition* part, const PedConstraint* constraint)
{
	PedCHSGeometry	bios_geom;
	DosPartitionData* dos_data;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk_specific != NULL);

	dos_data = part->disk_specific;

	if (dos_data->system == PARTITION_LDM && dos_data->orig) {
		PedGeometry *orig_geom = &dos_data->orig->geom;

		if (ped_geometry_test_equal (&part->geom, orig_geom)
		    && ped_constraint_is_solution (constraint, &part->geom))
			return 1;

		ped_geometry_set (&part->geom, orig_geom->start,
				  orig_geom->length);
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Parted can't resize partitions managed by "
			  "Windows Dynamic Disk."));
		return 0;
	}

	partition_probe_bios_geometry (part, &bios_geom);

	DosDiskData *disk_specific = part->disk->disk_specific;
	if (disk_specific->cylinder_alignment
	    && _align(part, &bios_geom, constraint))
		return 1;
	if (_align_no_geom (part, constraint))
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
add_metadata_part (PedDisk* disk, PedPartitionType type, PedSector start,
		   PedSector end)
{
	PedPartition*		new_part;

	PED_ASSERT (disk != NULL);

	new_part = ped_partition_new (disk, type | PED_PARTITION_METADATA, NULL,
				      start, end);
	if (!new_part)
		goto error;
	if (!ped_disk_add_partition (disk, new_part, NULL))
		goto error_destroy_new_part;

	return 1;

error_destroy_new_part:
	ped_partition_destroy (new_part);
error:
	return 0;
}

/* There are a few objectives here:
 * 	- avoid having lots of "free space" partitions lying around, to confuse
 * the front end.
 * 	- ensure that there's enough room to put in the extended partition
 * tables, etc.
 */
static int
add_logical_part_metadata (PedDisk* disk, const PedPartition* log_part)
{
	PedPartition*	ext_part = ped_disk_extended_partition (disk);
	PedPartition*	prev = log_part->prev;
	PedCHSGeometry	bios_geom;
	PedSector	cyl_size;
	PedSector	metadata_start;
	PedSector	metadata_end;
	PedSector	metadata_length;

	partition_probe_bios_geometry (ext_part, &bios_geom);
	cyl_size = bios_geom.sectors * bios_geom.heads;

	/* if there's metadata shortly before the partition (on the same
	 * cylinder), then make this new metadata partition touch the end of
	 * the other.  No point having 63 bytes (or whatever) of free space
	 * partition - just confuses front-ends, etc.
	 * 	Otherwise, start the metadata at the start of the cylinder
	 */

	metadata_end = log_part->geom.start - 1;
	metadata_start = ped_round_down_to (metadata_end, cyl_size);
	if (prev)
		metadata_start = PED_MAX (metadata_start, prev->geom.end + 1);
	else
		metadata_start = PED_MAX (metadata_start,
					  ext_part->geom.start + 1);
	metadata_length = metadata_end - metadata_start + 1;

	/* partition 5 doesn't need to have any metadata */
	if (log_part->num == 5 && metadata_length < bios_geom.sectors)
		return 1;

	PED_ASSERT (metadata_length > 0);

	return add_metadata_part (disk, PED_PARTITION_LOGICAL,
				  metadata_start, metadata_end);
}

/*
 * Find the starting sector number of the first non-free partition,
 * set *SECTOR to that value, and return 1.
 * If there is no non-free partition, don't modify *SECTOR and return 0.
 */
static int
get_start_first_nonfree_part (const PedDisk* disk, PedSector *sector)
{
	PedPartition* walk;

	// disk->part_list is the first partition on the disk.
	if (!disk->part_list)
		return 0;

	for (walk = disk->part_list; walk; walk = walk->next) {
		if (walk->type == PED_PARTITION_NORMAL ||
				walk->type == PED_PARTITION_EXTENDED) {
			*sector = walk->geom.start;
			return 1;
		}
	}
	return 0;
}

/*
 * Find the ending sector number of the last non-free partition,
 * set *SECTOR to that value, and return 1.
 * If there is no non-free partition, don't modify *SECTOR and return 0.
 */
static int
get_end_last_nonfree_part (const PedDisk* disk, PedSector *sector)
{
	PedPartition* last_part = NULL;
	PedPartition* walk;

	// disk->part_list is the first partition on the disk.
	if (!disk->part_list)
		return 0;

	for (walk = disk->part_list; walk; walk = walk->next) {
		if (walk->type == PED_PARTITION_NORMAL ||
				walk->type == PED_PARTITION_EXTENDED) {
			last_part = walk;
		}
	}

	if (!last_part)
		return 0;
	else {
		*sector = last_part->geom.end;
		return 1;
	}
}

/* Adds metadata placeholder partitions to cover the partition table (and
 * "free" space after it that often has bootloader stuff), and the last
 * incomplete cylinder at the end of the disk.
 * 	Parted has to be mindful of the uncertainty of dev->bios_geom.
 * It therefore makes sure this metadata doesn't overlap with partitions.
 */
static int
add_startend_metadata (PedDisk* disk)
{
	PedDevice* dev = disk->dev;
	PedSector cyl_size = dev->bios_geom.sectors * dev->bios_geom.heads;
	PedSector init_start, init_end, final_start, final_end;

	// Ranges for the initial and final metadata partition.
	init_start = 0;
	if (!get_start_first_nonfree_part(disk, &init_end))
		init_end = dev->bios_geom.sectors - 1;
	else
		init_end = PED_MIN (dev->bios_geom.sectors - 1, init_end - 1);

        DosDiskData *disk_specific = disk->disk_specific;
        if (!disk_specific->cylinder_alignment)
                final_start = dev->length - 1;
        else if (!get_end_last_nonfree_part(disk, &final_start))
		final_start = ped_round_down_to (dev->length, cyl_size);
	else
		final_start = PED_MAX (final_start + 1,
				ped_round_down_to (dev->length, cyl_size));
	final_end = dev->length - 1;

	// Create the metadata partitions.
	// init_end <= dev->length for devices that are _real_ small.
	if (init_start < init_end &&
			init_end <= dev->length &&
			!add_metadata_part (disk, PED_PARTITION_NORMAL,
				init_start, init_end))
			return 0;

	// init_end < final_start so they dont overlap.  For very small devs.
	if (final_start < final_end &&
			init_end < final_start &&
			final_end <= dev->length &&
			!add_metadata_part (disk, PED_PARTITION_NORMAL,
				final_start, final_end))
			return 0;

	return 1;
}

static int
msdos_alloc_metadata (PedDisk* disk)
{
	PedPartition*		ext_part;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->dev != NULL);

	if (!add_startend_metadata (disk))
		return 0;

	ext_part = ped_disk_extended_partition (disk);
	if (ext_part) {
		int		i;
		PedSector	start, end;
		PedCHSGeometry	bios_geom;

		for (i=5; 1; i++) {
			PedPartition* log_part;
			log_part = ped_disk_get_partition (disk, i);
			if (!log_part)
				break;
			if (!add_logical_part_metadata (disk, log_part))
				return 0;
		}

		partition_probe_bios_geometry (ext_part, &bios_geom);
		start = ext_part->geom.start;
		end = start + bios_geom.sectors - 1;
		if (ext_part->part_list)
			end = PED_MIN (end,
				       ext_part->part_list->geom.start - 1);
		if (!add_metadata_part (disk, PED_PARTITION_LOGICAL,
					start, end))
			return 0;
	}

	return 1;
}

static int
next_primary (const PedDisk* disk)
{
	int	i;
	for (i=1; i<=DOS_N_PRI_PARTITIONS; i++) {
		if (!ped_disk_get_partition (disk, i))
			return i;
	}
	return -1;
}

static int _GL_ATTRIBUTE_PURE
next_logical (const PedDisk* disk)
{
	int	i;
	for (i=5; i<=MAX_TOTAL_PART; i++) {
		if (!ped_disk_get_partition (disk, i))
			return i;
	}
	ped_exception_throw (
		PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
		_("cannot create any more partitions"),
		disk->dev->path);
	return -1;
}

static int
msdos_partition_enumerate (PedPartition* part)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);

	/* don't re-number a primary partition */
	if (part->num != -1 && part->num <= DOS_N_PRI_PARTITIONS)
		return 1;

	part->num = -1;

	if (part->type & PED_PARTITION_LOGICAL)
		part->num = next_logical (part->disk);
	else
		part->num = next_primary (part->disk);
	if (part->num == -1)
		return 0;
	return 1;
}

static int
msdos_get_max_primary_partition_count (const PedDisk* disk)
{
	return DOS_N_PRI_PARTITIONS;
}

static bool
msdos_get_max_supported_partition_count(const PedDisk* disk, int *max_n)
{
	*max_n = MAX_TOTAL_PART;
	return true;
}

#include "pt-common.h"
PT_define_limit_functions (msdos)

static PedDiskOps msdos_disk_ops = {
	clobber:		NULL,
	write:			NULL_IF_DISCOVER_ONLY (msdos_write),

	disk_set_flag:          msdos_disk_set_flag,
	disk_get_flag:          msdos_disk_get_flag,
	disk_is_flag_available: msdos_disk_is_flag_available,

	partition_set_name:	NULL,
	partition_get_name:	NULL,

  PT_op_function_initializers (msdos)
};

static PedDiskType msdos_disk_type = {
	next:		NULL,
	name:		"msdos",
	ops:		&msdos_disk_ops,
	features:	PED_DISK_TYPE_EXTENDED
};

void
ped_disk_msdos_init ()
{
	PED_ASSERT (sizeof (DosRawPartition) == 16);
	PED_ASSERT (sizeof (DosRawTable) == 512);

	ped_disk_type_register (&msdos_disk_type);
}

void
ped_disk_msdos_done ()
{
	ped_disk_type_unregister (&msdos_disk_type);
}
