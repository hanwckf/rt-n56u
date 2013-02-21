/*
    Copyright (C) 1985 MIPS Computer Systems, Inc.
    Copyright (C) 2000 Silicon Graphics Computer Systems, Inc.

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

#ifndef _SYS_DVH_H
#define _SYS_DVH_H

/*
 * Format for volume header information
 *
 * The volume header is a block located at the beginning of all disk
 * media (sector 0).  It contains information pertaining to physical
 * device parameters and logical partition information.
 *
 * The volume header is manipulated by disk formatters/verifiers,
 * partition builders (e.g. fx, dvhtool, and mkfs), and disk drivers.
 *
 * Previous versions of IRIX wrote a copy of the volume header is
 * located at sector 0 of each track of cylinder 0.  These copies were
 * never used, and reduced the capacity of the volume header to hold large
 * files, so this practice was discontinued.
 * The volume header is constrained to be less than or equal to 512
 * bytes long.  A particular copy is assumed valid if no drive errors
 * are detected, the magic number is correct, and the 32 bit 2's complement
 * of the volume header is correct.  The checksum is calculated by initially
 * zeroing vh_csum, summing the entire structure and then storing the
 * 2's complement of the sum.  Thus a checksum to verify the volume header
 * should be 0.
 *
 * The error summary table, bad sector replacement table, and boot blocks are
 * located by searching the volume directory within the volume header.
 *
 * Tables are sized simply by the integral number of table records that
 * will fit in the space indicated by the directory entry.
 *
 * The amount of space allocated to the volume header, replacement blocks,
 * and other tables is user defined when the device is formatted.
 */

/*
 * device parameters are in the volume header to determine mapping
 * from logical block numbers to physical device addresses
 *
 * Linux doesn't care ...
 */
struct device_parameters {
	unsigned char	dp_skew;	/* spiral addressing skew */
	unsigned char	dp_gap1;	/* words of 0 before header */
	unsigned char	dp_gap2;	/* words of 0 between hdr and data */
	unsigned char	dp_spares_cyl;	/* This is for drives (such as SCSI
		that support zone oriented sparing, where the zone is larger
		than one track.  It gets subracteded from the cylinder size
		( dp_trks0 * dp_sec) when doing partition size calculations */
	unsigned short	dp_cyls;	/* number of usable cylinders (i.e.,
		doesn't include cylinders reserved by the drive for badblocks,
		etc.). For drives with variable geometry, this number may be
		decreased so that:
		dp_cyls * ((dp_heads * dp_trks0) - dp_spares_cyl) <= actualcapacity
		This happens on SCSI drives such as the Wren IV and Toshiba 156
		Also see dp_cylshi below */
	unsigned short	dp_shd0;	/* starting head vol 0 */
	unsigned short	dp_trks0;	/* number of tracks / cylinder vol 0*/
	unsigned char	dp_ctq_depth;	/* Depth of CTQ queue */
	unsigned char	dp_cylshi;	/* high byte of 24 bits of cylinder count */
	unsigned short	dp_unused;	/* not used */
	unsigned short	dp_secs;	/* number of sectors/track */
	unsigned short	dp_secbytes;	/* length of sector in bytes */
	unsigned short	dp_interleave;	/* sector interleave */
	int	dp_flags;		/* controller characteristics */
	int	dp_datarate;		/* bytes/sec for kernel stats */
	int	dp_nretries;		/* max num retries on data error */
	int	dp_mspw;		/* ms per word to xfer, for iostat */
	unsigned short dp_xgap1;	/* Gap 1 for xylogics controllers */
	unsigned short dp_xsync;    /* sync delay for xylogics controllers */
	unsigned short dp_xrdly;    /* read delay for xylogics controllers */
	unsigned short dp_xgap2;    /* gap 2 for xylogics controllers */
	unsigned short dp_xrgate;   /* read gate for xylogics controllers */
	unsigned short dp_xwcont;   /* write continuation for xylogics */
};

/*
 * Device characterization flags
 * (dp_flags)
 */
#define	DP_SECTSLIP	0x00000001	/* sector slip to spare sector */
#define	DP_SECTFWD	0x00000002	/* forward to replacement sector */
#define	DP_TRKFWD	0x00000004	/* forward to replacement track */
#define	DP_MULTIVOL	0x00000008	/* multiple volumes per spindle */
#define	DP_IGNOREERRORS	0x00000010	/* transfer data regardless of errors */
#define DP_RESEEK	0x00000020	/* recalibrate as last resort */
#define	DP_CTQ_EN	0x00000040	/* enable command tag queueing */

/*
 * Boot blocks, bad sector tables, and the error summary table, are located
 * via the volume_directory.
 */
#define VDNAMESIZE	8

struct volume_directory {
	char	vd_name[VDNAMESIZE];	/* name */
	int	vd_lbn;			/* logical block number */
	int	vd_nbytes;		/* file length in bytes */
};

/*
 * partition table describes logical device partitions
 * (device drivers examine this to determine mapping from logical units
 * to cylinder groups, device formatters/verifiers examine this to determine
 * location of replacement tracks/sectors, etc)
 *
 * NOTE: pt_firstlbn SHOULD BE CYLINDER ALIGNED
 */
struct partition_table {		/* one per logical partition */
	int	pt_nblks;		/* # of logical blks in partition */
	int	pt_firstlbn;		/* first lbn of partition */
	int	pt_type;		/* use of partition */
};

#define	PTYPE_VOLHDR	0		/* partition is volume header */
#define	PTYPE_TRKREPL	1		/* partition is used for repl trks */
#define	PTYPE_SECREPL	2		/* partition is used for repl secs */
#define	PTYPE_RAW	3		/* partition is used for data */
#define	PTYPE_BSD42	4		/* partition is 4.2BSD file system */
#define	PTYPE_BSD	4		/* partition is 4.2BSD file system */
#define	PTYPE_SYSV	5		/* partition is SysV file system */
#define	PTYPE_VOLUME	6		/* partition is entire volume */
#define	PTYPE_EFS	7		/* partition is sgi EFS */
#define	PTYPE_LVOL	8		/* partition is part of a logical vol */
#define	PTYPE_RLVOL	9		/* part of a "raw" logical vol */
#define	PTYPE_XFS	10		/* partition is sgi XFS */
#define	PTYPE_XFSLOG	11		/* partition is sgi XFS log */
#define	PTYPE_XLV	12		/* partition is part of an XLV vol */
#define	PTYPE_XVM	13		/* partition is sgi XVM */
#define NPTYPES		16

#define	VHMAGIC		0xbe5a941	/* randomly chosen value */
#define	NPARTAB		16		/* 16 unix partitions */
#define	NVDIR		15		/* max of 15 directory entries */
#define BFNAMESIZE	16		/* max 16 chars in boot file name */

/* Partition types for ARCS */
#define NOT_USED        0       /* Not used 				*/
#define FAT_SHORT       1       /* FAT file system, 12-bit FAT entries 	*/
#define FAT_LONG        4       /* FAT file system, 16-bit FAT entries 	*/
#define EXTENDED        5       /* extended partition 			*/
#define HUGE            6       /* huge partition- MS/DOS 4.0 and later */

/* Active flags for ARCS */
#define BOOTABLE        0x00;
#define NOT_BOOTABLE    0x80;

struct volume_header {
	int vh_magic;				/* identifies volume header */
	short vh_rootpt;			/* root partition number */
	short vh_swappt;			/* swap partition number */
	char vh_bootfile[BFNAMESIZE];		/* name of file to boot */
	struct device_parameters vh_dp;		/* device parameters */
	struct volume_directory vh_vd[NVDIR];	/* other vol hdr contents */
	struct partition_table vh_pt[NPARTAB];	/* device partition layout */
	int vh_csum;				/* volume header checksum */
	int vh_fill;	/* fill out to 512 bytes */
};

#endif /* _SYS_DVH_H */
