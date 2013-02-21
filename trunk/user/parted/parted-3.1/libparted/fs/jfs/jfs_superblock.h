/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef	_H_JFS_SUPERBLOCK
#define _H_JFS_SUPERBLOCK
/*
 *	jfs_superblock.h
 */

/*
 * make the magic number something a human could read
 */
#define JFS_MAGIC 	"JFS1"	/* Magic word: Version 1 */

#define JFS_VERSION	1	/* Version number: Version 1 */

#define LV_NAME_SIZE	11	/* MUST BE 11 for OS/2 boot sector */

/*
 *	aggregate superblock
 *
 * The name superblock is too close to super_block, so the name has been
 * changed to jfs_superblock.  The utilities are still using the old name.
 */
#ifdef _JFS_UTILITY
struct superblock
#else
struct jfs_superblock
#endif
{
	char s_magic[4];	/* 4: magic number */
	u32 s_version;		/* 4: version number */

	s64 s_size;		/* 8: aggregate size in hardware/LVM blocks;
				 * VFS: number of blocks
				 */
	s32 s_bsize;		/* 4: aggregate block size in bytes;
				 * VFS: fragment size
				 */
	s16 s_l2bsize;		/* 2: log2 of s_bsize */
	s16 s_l2bfactor;	/* 2: log2(s_bsize/hardware block size) */
	s32 s_pbsize;		/* 4: hardware/LVM block size in bytes */
	s16 s_l2pbsize;		/* 2: log2 of s_pbsize */
	s16 pad;		/* 2: padding necessary for alignment */

	u32 s_agsize;		/* 4: allocation group size in aggr. blocks */

	u32 s_flag;		/* 4: aggregate attributes:
				 *    see jfs_filsys.h
				 */
	u32 s_state;		/* 4: mount/unmount/recovery state:
				 *    see jfs_filsys.h
				 */
	s32 s_compress;		/* 4: > 0 if data compression */

	pxd_t s_ait2;		/* 8: first extent of secondary
				 *    aggregate inode table
				 */

	pxd_t s_aim2;		/* 8: first extent of secondary
				 *    aggregate inode map
				 */
	u32 s_logdev;		/* 4: device address of log */
	s32 s_logserial;	/* 4: log serial number at aggregate mount */
	pxd_t s_logpxd;		/* 8: inline log extent */

	pxd_t s_fsckpxd;	/* 8: inline fsck work space extent */

	struct timestruc_t s_time;	/* 8: time last updated */

	s32 s_fsckloglen;	/* 4: Number of file system blocks reserved for
				 *    the fsck service log.
				 *    N.B. These blocks are divided among the
				 *         versions kept.  This is not a per
				 *         version size.
				 *    N.B. These blocks are included in the
				 *         length field of s_fsckpxd.
				 */
	s8 s_fscklog;		/* 1: which fsck service log is most recent
				 *    0 => no service log data yet
				 *    1 => the first one
				 *    2 => the 2nd one
				 */
	char s_fpack[11];	/* 11: file system volume name
				 *     N.B. This must be 11 bytes to
				 *          conform with the OS/2 BootSector
				 *          requirements
				 */

	/* extendfs() parameter under s_state & FM_EXTENDFS */
	s64 s_xsize;		/* 8: extendfs s_size */
	pxd_t s_xfsckpxd;	/* 8: extendfs fsckpxd */
	pxd_t s_xlogpxd;	/* 8: extendfs logpxd */
	/* - 128 byte boundary - */

	/*
	 *      DFS VFS support (preliminary)
	 */
	char s_attach;		/* 1: VFS: flag: set when aggregate is attached
				 */
	u8 rsrvd4[7];		/* 7: reserved - set to 0 */

	u64 totalUsable;	/* 8: VFS: total of 1K blocks which are
				 * available to "normal" (non-root) users.
				 */
	u64 minFree;		/* 8: VFS: # of 1K blocks held in reserve for
				 * exclusive use of root.  This value can be 0,
				 * and if it is then totalUsable will be equal
				 * to # of blocks in aggregate.  I believe this
				 * means that minFree + totalUsable = # blocks.
				 * In that case, we don't need to store both
				 * totalUsable and minFree since we can compute
				 * one from the other.  I would guess minFree
				 * would be the one we should store, and
				 * totalUsable would be the one we should
				 * compute.  (Just a guess...)
				 */

	u64 realFree;		/* 8: VFS: # of free 1K blocks can be used by
				 * "normal" users.  It may be this is something
				 * we should compute when asked for instead of
				 * storing in the superblock.  I don't know how
				 * often this information is needed.
				 */
	/*
	 *      graffiti area
	 */
};

#endif /*_H_JFS_SUPERBLOCK */
