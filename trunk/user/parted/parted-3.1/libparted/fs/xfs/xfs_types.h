/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 */
#ifndef __XFS_TYPES_H__
#define	__XFS_TYPES_H__

/*
 * Some types are conditional based on the selected configuration.
 * Set XFS_BIG_FILES=1 or 0 and XFS_BIG_FILESYSTEMS=1 or 0 depending
 * on the desired configuration.
 * XFS_BIG_FILES needs pgno_t to be 64 bits (64-bit kernels).
 * XFS_BIG_FILESYSTEMS needs daddr_t to be 64 bits (N32 and 64-bit kernels).
 *
 * Expect these to be set from klocaldefs, or from the machine-type
 * defs files for the normal case.
 */

#define	XFS_BIG_FILES		1
#define	XFS_BIG_FILESYSTEMS	1

typedef uint32_t	xfs_agblock_t;	/* blockno in alloc. group */
typedef	uint32_t	xfs_extlen_t;	/* extent length in blocks */
typedef	uint32_t	xfs_agnumber_t;	/* allocation group number */
typedef int32_t	xfs_extnum_t;	/* # of extents in a file */
typedef int16_t	xfs_aextnum_t;	/* # extents in an attribute fork */
typedef	int64_t	xfs_fsize_t;	/* bytes in a file */
typedef uint64_t	xfs_ufsize_t;	/* unsigned bytes in a file */

typedef	int32_t	xfs_suminfo_t;	/* type of bitmap summary info */
typedef	int32_t	xfs_rtword_t;	/* word type for bitmap manipulations */

typedef	int64_t	xfs_lsn_t;	/* log sequence number */
typedef	int32_t	xfs_tid_t;	/* transaction identifier */

typedef	uint32_t	xfs_dablk_t;	/* dir/attr block number (in file) */
typedef	uint32_t	xfs_dahash_t;	/* dir/attr hash value */

typedef uint16_t	xfs_prid_t;	/* prid_t truncated to 16bits in XFS */

/*
 * These types are 64 bits on disk but are either 32 or 64 bits in memory.
 * Disk based types:
 */
typedef uint64_t	xfs_dfsbno_t;	/* blockno in filesystem (agno|agbno) */
typedef uint64_t	xfs_drfsbno_t;	/* blockno in filesystem (raw) */
typedef	uint64_t	xfs_drtbno_t;	/* extent (block) in realtime area */
typedef	uint64_t	xfs_dfiloff_t;	/* block number in a file */
typedef	uint64_t	xfs_dfilblks_t;	/* number of blocks in a file */

/*
 * Memory based types are conditional.
 */
#if XFS_BIG_FILESYSTEMS
typedef	uint64_t	xfs_fsblock_t;	/* blockno in filesystem (agno|agbno) */
typedef uint64_t	xfs_rfsblock_t;	/* blockno in filesystem (raw) */
typedef uint64_t	xfs_rtblock_t;	/* extent (block) in realtime area */
typedef	int64_t	xfs_srtblock_t;	/* signed version of xfs_rtblock_t */
#else
typedef	uint32_t	xfs_fsblock_t;	/* blockno in filesystem (agno|agbno) */
typedef uint32_t	xfs_rfsblock_t;	/* blockno in filesystem (raw) */
typedef uint32_t	xfs_rtblock_t;	/* extent (block) in realtime area */
typedef	int32_t	xfs_srtblock_t;	/* signed version of xfs_rtblock_t */
#endif
#if XFS_BIG_FILES
typedef	uint64_t	xfs_fileoff_t;	/* block number in a file */
typedef	int64_t	xfs_sfiloff_t;	/* signed block number in a file */
typedef	uint64_t	xfs_filblks_t;	/* number of blocks in a file */
#else
typedef	uint32_t	xfs_fileoff_t;	/* block number in a file */
typedef	int32_t	xfs_sfiloff_t;	/* signed block number in a file */
typedef	uint32_t	xfs_filblks_t;	/* number of blocks in a file */
#endif

typedef uint8_t       xfs_arch_t;     /* architecutre of an xfs fs */

/*
 * Null values for the types.
 */
#define	NULLDFSBNO	((xfs_dfsbno_t)-1)
#define	NULLDRFSBNO	((xfs_drfsbno_t)-1)
#define	NULLDRTBNO	((xfs_drtbno_t)-1)
#define	NULLDFILOFF	((xfs_dfiloff_t)-1)

#define	NULLFSBLOCK	((xfs_fsblock_t)-1)
#define	NULLRFSBLOCK	((xfs_rfsblock_t)-1)
#define	NULLRTBLOCK	((xfs_rtblock_t)-1)
#define	NULLFILEOFF	((xfs_fileoff_t)-1)

#define	NULLAGBLOCK	((xfs_agblock_t)-1)
#define	NULLAGNUMBER	((xfs_agnumber_t)-1)
#define	NULLEXTNUM	((xfs_extnum_t)-1)

#define NULLCOMMITLSN	((xfs_lsn_t)-1)

/*
 * Max values for extlen, extnum, aextnum.
 */
#define	MAXEXTLEN	((xfs_extlen_t)0x001fffff)	/* 21 bits */
#define	MAXEXTNUM	((xfs_extnum_t)0x7fffffff)	/* signed int */
#define	MAXAEXTNUM	((xfs_aextnum_t)0x7fff)		/* signed short */

/*
 * MAXNAMELEN is the length (including the terminating null) of
 * the longest permissible file (component) name.
 */
#define MAXNAMELEN	256

typedef enum {
	XFS_LOOKUP_EQi, XFS_LOOKUP_LEi, XFS_LOOKUP_GEi
} xfs_lookup_t;

typedef enum {
	XFS_BTNUM_BNOi, XFS_BTNUM_CNTi, XFS_BTNUM_BMAPi, XFS_BTNUM_INOi,
	XFS_BTNUM_MAX
} xfs_btnum_t;


#ifdef CONFIG_PROC_FS
/*
 * XFS global statistics
 */
struct xfsstats {
# define XFSSTAT_END_EXTENT_ALLOC	4
	uint32_t		xs_allocx;
	uint32_t		xs_allocb;
	uint32_t		xs_freex;
	uint32_t		xs_freeb;
# define XFSSTAT_END_ALLOC_BTREE   	(XFSSTAT_END_EXTENT_ALLOC+4)
	uint32_t		xs_abt_lookup;
	uint32_t		xs_abt_compare;
	uint32_t		xs_abt_insrec;
	uint32_t		xs_abt_delrec;
# define XFSSTAT_END_BLOCK_MAPPING	(XFSSTAT_END_ALLOC_BTREE+7)
	uint32_t		xs_blk_mapr;
	uint32_t		xs_blk_mapw;
	uint32_t		xs_blk_unmap;
	uint32_t		xs_add_exlist;
	uint32_t		xs_del_exlist;
	uint32_t		xs_look_exlist;
	uint32_t		xs_cmp_exlist;
# define XFSSTAT_END_BLOCK_MAP_BTREE	(XFSSTAT_END_BLOCK_MAPPING+4)
	uint32_t		xs_bmbt_lookup;
	uint32_t		xs_bmbt_compare;
	uint32_t		xs_bmbt_insrec;
	uint32_t		xs_bmbt_delrec;
# define XFSSTAT_END_DIRECTORY_OPS	(XFSSTAT_END_BLOCK_MAP_BTREE+4)
	uint32_t		xs_dir_lookup;
	uint32_t		xs_dir_create;
	uint32_t		xs_dir_remove;
	uint32_t		xs_dir_getdents;
# define XFSSTAT_END_TRANSACTIONS	(XFSSTAT_END_DIRECTORY_OPS+3)
	uint32_t		xs_trans_sync;
	uint32_t		xs_trans_async;
	uint32_t		xs_trans_empty;
# define XFSSTAT_END_INODE_OPS		(XFSSTAT_END_TRANSACTIONS+7)
	uint32_t		xs_ig_attempts;
	uint32_t		xs_ig_found;
	uint32_t		xs_ig_frecycle;
	uint32_t		xs_ig_missed;
	uint32_t		xs_ig_dup;
	uint32_t		xs_ig_reclaims;
	uint32_t		xs_ig_attrchg;
# define XFSSTAT_END_LOG_OPS		(XFSSTAT_END_INODE_OPS+5)
	uint32_t		xs_log_writes;
	uint32_t		xs_log_blocks;
	uint32_t		xs_log_noiclogs;
	uint32_t		xs_log_force;
	uint32_t		xs_log_force_sleep;
# define XFSSTAT_END_TAIL_PUSHING	(XFSSTAT_END_LOG_OPS+10)
	uint32_t		xs_try_logspace;
	uint32_t		xs_sleep_logspace;
	uint32_t		xs_push_ail;
	uint32_t		xs_push_ail_success;
	uint32_t		xs_push_ail_pushbuf;
	uint32_t		xs_push_ail_pinned;
	uint32_t		xs_push_ail_locked;
	uint32_t		xs_push_ail_flushing;
	uint32_t		xs_push_ail_restarts;
	uint32_t		xs_push_ail_flush;
# define XFSSTAT_END_WRITE_CONVERT	(XFSSTAT_END_TAIL_PUSHING+2)
	uint32_t		xs_xstrat_quick;
	uint32_t		xs_xstrat_split;
# define XFSSTAT_END_READ_WRITE_OPS	(XFSSTAT_END_WRITE_CONVERT+2)
	uint32_t		xs_write_calls;
	uint32_t		xs_read_calls;
# define XFSSTAT_END_ATTRIBUTE_OPS	(XFSSTAT_END_READ_WRITE_OPS+4)
	uint32_t		xs_attr_get;
	uint32_t		xs_attr_set;
	uint32_t		xs_attr_remove;
	uint32_t		xs_attr_list;
# define XFSSTAT_END_QUOTA_OPS		(XFSSTAT_END_ATTRIBUTE_OPS+8)
	uint32_t		xs_qm_dqreclaims;
	uint32_t		xs_qm_dqreclaim_misses;
	uint32_t		xs_qm_dquot_dups;
	uint32_t		xs_qm_dqcachemisses;
	uint32_t		xs_qm_dqcachehits;
	uint32_t		xs_qm_dqwants;
	uint32_t		xs_qm_dqshake_reclaims;
	uint32_t		xs_qm_dqinact_reclaims;
# define XFSSTAT_END_INODE_CLUSTER	(XFSSTAT_END_QUOTA_OPS+3)
	uint32_t		xs_iflush_count;
	uint32_t		xs_icluster_flushcnt;
	uint32_t		xs_icluster_flushinode;
# define XFSSTAT_END_VNODE_OPS		(XFSSTAT_END_INODE_CLUSTER+8)
	uint32_t		vn_active;	/* # vnodes not on free lists */
	uint32_t		vn_alloc;	/* # times vn_alloc called */
	uint32_t		vn_get;		/* # times vn_get called */
	uint32_t		vn_hold;	/* # times vn_hold called */
	uint32_t		vn_rele;	/* # times vn_rele called */
	uint32_t		vn_reclaim;	/* # times vn_reclaim called */
	uint32_t		vn_remove;	/* # times vn_remove called */
	uint32_t		vn_free;	/* # times vn_free called */
	struct xfsstats_xpc {
		uint64_t	xs_xstrat_bytes;
		uint64_t	xs_write_bytes;
		uint64_t	xs_read_bytes;
	} xpc;
} xfsstats;

# define XFS_STATS_INC(count)		( xfsstats.##count ++ )
# define XFS_STATS_DEC(count)		( xfsstats.##count -- )
# define XFS_STATS_ADD(count, inc)	( xfsstats.##count += (inc) )
# define XFS_STATS64_INC(count)		( xfsstats.xpc.##count ++ )
# define XFS_STATS64_ADD(count, inc)	( xfsstats.xpc.##count += (inc) )
#else	/* !CONFIG_PROC_FS */
# define XFS_STATS_INC(count)
# define XFS_STATS_DEC(count)
# define XFS_STATS_ADD(count, inc)
# define XFS_STATS64_INC(count)
# define XFS_STATS64_ADD(count, inc)
#endif	/* !CONFIG_PROC_FS */


#ifdef __KERNEL__

/* juggle IRIX device numbers - still used in ondisk structures */

#define IRIX_DEV_BITSMAJOR      14
#define IRIX_DEV_BITSMINOR      18
#define IRIX_DEV_MAXMAJ         0x1ff
#define IRIX_DEV_MAXMIN         0x3ffff
#define IRIX_DEV_MAJOR(dev)     ((int)(((unsigned)(dev)>>IRIX_DEV_BITSMINOR) \
                                    & IRIX_DEV_MAXMAJ))
#define IRIX_DEV_MINOR(dev)     ((int)((dev)&IRIX_DEV_MAXMIN))
#define IRIX_MKDEV(major,minor) ((xfs_dev_t)(((major)<<IRIX_DEV_BITSMINOR) \
                                    | (minor&IRIX_DEV_MAXMIN)))

#define IRIX_DEV_TO_KDEVT(dev)  MKDEV(IRIX_DEV_MAJOR(dev),IRIX_DEV_MINOR(dev))
#define IRIX_DEV_TO_DEVT(dev)   ((IRIX_DEV_MAJOR(dev)<<8)|IRIX_DEV_MINOR(dev))

/* __psint_t is the same size as a pointer */
#if (BITS_PER_LONG == 32)
typedef int32_t __psint_t;
typedef uint32_t __psunsigned_t;
#elif (BITS_PER_LONG == 64)
typedef int64_t __psint_t;
typedef uint64_t __psunsigned_t;
#else
#error BITS_PER_LONG must be 32 or 64
#endif


/*
 * struct for passing owner/requestor id
 */
typedef struct flid {
#ifdef CELL_CAPABLE
        pid_t   fl_pid;
        sysid_t fl_sysid;
#endif
} flid_t;

#endif	/* __KERNEL__ */

#endif	/* !__XFS_TYPES_H */
