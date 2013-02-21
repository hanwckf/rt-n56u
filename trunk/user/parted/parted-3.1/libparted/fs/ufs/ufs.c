/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2001, 2007, 2009-2012 Free Software Foundation, Inc.

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

    Contributor: Ben Collins <bcollins@debian.org>
*/

#include <config.h>

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include <unistd.h>
#include <string.h>

#define SUN_UFS_BLOCK_SIZES       ((int[2]){512, 0})
#define HP_UFS_BLOCK_SIZES        ((int[2]){512, 0})


/* taken from ufs_fs.h in Linux */
#define	UFS_MAXNAMLEN 255
#define UFS_MAXMNTLEN 512
#define UFS_MAXCSBUFS 31
#define UFS_LINK_MAX 32000

#define UFS_MAGIC	0x00011954
#define UFS_MAGIC_LFN	0x00095014
#define UFS_MAGIC_FEA	0x00195612
#define UFS_MAGIC_4GB	0x05231994

struct ufs_csum {
	uint32_t	cs_ndir;	/* number of directories */
	uint32_t	cs_nbfree;	/* number of free blocks */
	uint32_t	cs_nifree;	/* number of free inodes */
	uint32_t	cs_nffree;	/* number of free frags */
};

struct ufs_super_block {
	uint32_t	fs_link;	/* UNUSED */
	uint32_t	fs_rlink;	/* UNUSED */
	uint32_t	fs_sblkno;	/* addr of super-block in filesys */
	uint32_t	fs_cblkno;	/* offset of cyl-block in filesys */
	uint32_t	fs_iblkno;	/* offset of inode-blocks in filesys */
	uint32_t	fs_dblkno;	/* offset of first data after cg */
	uint32_t	fs_cgoffset;	/* cylinder group offset in cylinder */
	uint32_t	fs_cgmask;	/* used to calc mod fs_ntrak */
	uint32_t	fs_time;	/* last time written -- time_t */
	uint32_t	fs_size;	/* number of blocks in fs */
	uint32_t	fs_dsize;	/* number of data blocks in fs */
	uint32_t	fs_ncg;		/* number of cylinder groups */
	uint32_t	fs_bsize;	/* size of basic blocks in fs */
	uint32_t	fs_fsize;	/* size of frag blocks in fs */
	uint32_t	fs_frag;	/* number of frags in a block in fs */
/* these are configuration parameters */
	uint32_t	fs_minfree;	/* minimum percentage of free blocks */
	uint32_t	fs_rotdelay;	/* num of ms for optimal next block */
	uint32_t	fs_rps;		/* disk revolutions per second */
/* these fields can be computed from the others */
	uint32_t	fs_bmask;	/* ``blkoff'' calc of blk offsets */
	uint32_t	fs_fmask;	/* ``fragoff'' calc of frag offsets */
	uint32_t	fs_bshift;	/* ``lblkno'' calc of logical blkno */
	uint32_t	fs_fshift;	/* ``numfrags'' calc number of frags */
/* these are configuration parameters */
	uint32_t	fs_maxcontig;	/* max number of contiguous blks */
	uint32_t	fs_maxbpg;	/* max number of blks per cyl group */
/* these fields can be computed from the others */
	uint32_t	fs_fragshift;	/* block to frag shift */
	uint32_t	fs_fsbtodb;	/* fsbtodb and dbtofsb shift constant */
	uint32_t	fs_sbsize;	/* actual size of super block */
	uint32_t	fs_csmask;	/* csum block offset */
	uint32_t	fs_csshift;	/* csum block number */
	uint32_t	fs_nindir;	/* value of NINDIR */
	uint32_t	fs_inopb;	/* value of INOPB */
	uint32_t	fs_nspf;	/* value of NSPF */
/* yet another configuration parameter */
	uint32_t	fs_optim;	/* optimization preference, see below */
/* these fields are derived from the hardware */
	union {
		struct {
			uint32_t	fs_npsect;	/* # sectors/track including spares */
		} fs_sun;
		struct {
			int32_t		fs_state;	/* file system state time stamp */
		} fs_sunx86;
	} fs_u1;
	uint32_t	fs_interleave;	/* hardware sector interleave */
	uint32_t	fs_trackskew;	/* sector 0 skew, per track */
/* a unique id for this file system (currently unused and unmaintained) */
/* In 4.3 Tahoe this space is used by fs_headswitch and fs_trkseek */
/* Neither of those fields is used in the Tahoe code right now but */
/* there could be problems if they are.                            */
	uint32_t	fs_id[2];	/* file system id */
/* sizes determined by number of cylinder groups and their sizes */
	uint32_t	fs_csaddr;	/* blk addr of cyl grp summary area */
	uint32_t	fs_cssize;	/* size of cyl grp summary area */
	uint32_t	fs_cgsize;	/* cylinder group size */
/* these fields are derived from the hardware */
	uint32_t	fs_ntrak;	/* tracks per cylinder */
	uint32_t	fs_nsect;	/* sectors per track */
	uint32_t	fs_spc;		/* sectors per cylinder */
/* this comes from the disk driver partitioning */
	uint32_t	fs_ncyl;	/* cylinders in file system */
/* these fields can be computed from the others */
	uint32_t	fs_cpg;		/* cylinders per group */
	uint32_t	fs_ipg;		/* inodes per group */
	uint32_t	fs_fpg;		/* blocks per group * fs_frag */
/* this data must be re-computed after crashes */
	struct ufs_csum fs_cstotal;	/* cylinder summary information */
/* these fields are cleared at mount time */
	int8_t		fs_fmod;	/* super block modified flag */
	int8_t		fs_clean;	/* file system is clean flag */
	int8_t		fs_ronly;	/* mounted read-only flag */
	int8_t		fs_flags;	/* currently unused flag */
	int8_t		fs_fsmnt[UFS_MAXMNTLEN];	/* name mounted on */
/* these fields retain the current block allocation info */
	uint32_t	fs_cgrotor;	/* last cg searched */
	uint32_t	fs_csp[UFS_MAXCSBUFS];	/* list of fs_cs info buffers */
	uint32_t	fs_maxcluster;
	uint32_t	fs_cpc;		/* cyl per cycle in postbl */
	uint16_t	fs_opostbl[16][8];	/* old rotation block list head */
	union {
		struct {
			int32_t		fs_sparecon[53];/* reserved for future constants */
			int32_t		fs_reclaim;
			int32_t		fs_sparecon2[1];
			int32_t		fs_state;	/* file system state time stamp */
			uint32_t	fs_qbmask[2];	/* ~usb_bmask */
			uint32_t	fs_qfmask[2];	/* ~usb_fmask */
		} fs_sun;
		struct {
			int32_t		fs_sparecon[53];/* reserved for future constants */
			int32_t		fs_reclaim;
			int32_t		fs_sparecon2[1];
			uint32_t	fs_npsect;	/* # sectors/track including spares */
			uint32_t	fs_qbmask[2];	/* ~usb_bmask */
			uint32_t	fs_qfmask[2];	/* ~usb_fmask */
		} fs_sunx86;
		struct {
			int32_t		fs_sparecon[50];/* reserved for future constants */
			int32_t		fs_contigsumsize;/* size of cluster summary array */
			int32_t		fs_maxsymlinklen;/* max length of an internal symlink */
			int32_t		fs_inodefmt;	/* format of on-disk inodes */
			uint32_t	fs_maxfilesize[2];	/* max representable file size */
			uint32_t	fs_qbmask[2];	/* ~usb_bmask */
			uint32_t	fs_qfmask[2];	/* ~usb_fmask */
			int32_t		fs_state;	/* file system state time stamp */
		} fs_44;
	} fs_u2;
	int32_t	fs_postblformat;	/* format of positional layout tables */
	int32_t	fs_nrpos;		/* number of rotational positions */
	int32_t	fs_postbloff;		/* (__s16) rotation block list head */
	int32_t	fs_rotbloff;		/* (uint8_t) blocks for each rotation */
	int32_t	fs_magic;		/* magic number */
	uint8_t	fs_space[4];		/* list of blocks for each rotation */
};

static PedGeometry*
ufs_probe_sun (PedGeometry* geom)
{
	int8_t buf[512 * 3];
	struct ufs_super_block *sb;

	if (geom->length < 5)
		return 0;
	if (!ped_geometry_read (geom, buf, 16, 3))
		return 0;

	sb = (struct ufs_super_block *)buf;

	if (PED_BE32_TO_CPU(sb->fs_magic) == UFS_MAGIC) {
		PedSector block_size = PED_BE32_TO_CPU(sb->fs_bsize) / 512;
		PedSector block_count = PED_BE32_TO_CPU(sb->fs_size);
		return ped_geometry_new (geom->dev, geom->start,
					 block_size * block_count);
	}
	if (PED_LE32_TO_CPU(sb->fs_magic) == UFS_MAGIC) {
		PedSector block_size = PED_LE32_TO_CPU(sb->fs_bsize) / 512;
		PedSector block_count = PED_LE32_TO_CPU(sb->fs_size);
		return ped_geometry_new (geom->dev, geom->start,
					 block_size * block_count);
	}
	return NULL;
}

static PedGeometry*
ufs_probe_hp (PedGeometry* geom)
{
	int8_t buf[1536];
	struct ufs_super_block *sb;
	PedSector block_size;
	PedSector block_count;

	if (geom->length < 5)
		return 0;
	if (!ped_geometry_read (geom, buf, 16, 3))
		return 0;

	sb = (struct ufs_super_block *)buf;

	/* Try sane bytesex */
	switch (PED_BE32_TO_CPU(sb->fs_magic)) {
		case UFS_MAGIC_LFN:
		case UFS_MAGIC_FEA:
		case UFS_MAGIC_4GB:
			block_size = PED_BE32_TO_CPU(sb->fs_bsize) / 512;
			block_count = PED_BE32_TO_CPU(sb->fs_size);
			return ped_geometry_new (geom->dev, geom->start,
						 block_size * block_count);
	}

	/* Try perverted bytesex */
	switch (PED_LE32_TO_CPU(sb->fs_magic)) {
		case UFS_MAGIC_LFN:
		case UFS_MAGIC_FEA:
		case UFS_MAGIC_4GB:
			block_size = PED_LE32_TO_CPU(sb->fs_bsize) / 512;
			block_count = PED_LE32_TO_CPU(sb->fs_size);
			return ped_geometry_new (geom->dev, geom->start,
						 block_size * block_count);
	}
	return NULL;
}

static PedFileSystemOps ufs_ops_sun = {
	probe:		ufs_probe_sun,
};

static PedFileSystemOps ufs_ops_hp = {
	probe:		ufs_probe_hp,
};

static PedFileSystemType ufs_type_sun = {
	next:	NULL,
	ops:	&ufs_ops_sun,
	name:	"sun-ufs",
	block_sizes: SUN_UFS_BLOCK_SIZES
};

static PedFileSystemType ufs_type_hp = {
	next:   NULL,
	ops:    &ufs_ops_hp,
	name:   "hp-ufs",
	block_sizes: HP_UFS_BLOCK_SIZES
};

void
ped_file_system_ufs_init ()
{
	PED_ASSERT (sizeof (struct ufs_super_block) == 1380);

	ped_file_system_type_register (&ufs_type_sun);
	ped_file_system_type_register (&ufs_type_hp);
}

void
ped_file_system_ufs_done ()
{
	ped_file_system_type_unregister (&ufs_type_hp);
	ped_file_system_type_unregister (&ufs_type_sun);
}
