/*
 *  linux/include/linux/ext2_fs.h
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * EXT2_*_*() convienience macros added by Andrew Clausen <clausen@gnu.org>
 * Copyright (C) 2000, 2009-2012 Free Software Foundation, Inc.
 */

#ifndef _EXT2_FS_H
#define _EXT2_FS_H

#include <parted/endian.h>
#include <stdint.h>

/*
 * The second extended file system constants/structures
 */

#define EXT2_SUPER_MAGIC_CONST	0xEF53
#define EXT2_MIN_BLOCK_SIZE	1024
#define EXT2_NDIR_BLOCKS	12
#define EXT2_IND_BLOCK		EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK		(EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK		(EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS		(EXT2_TIND_BLOCK + 1)
#define EXT2_VALID_FS		0x0001
#define EXT2_ERROR_FS		0x0002
#define EXT2_RESERVED_INODE_COUNT	11

/*
 * Codes for operating systems
 */
#define EXT2_OS_LINUX		0
#define EXT2_OS_HURD		1
#define EXT2_OS_MASIX		2
#define EXT2_OS_FREEBSD		3
#define EXT2_OS_LITES		4

/*
 * Feature set definitions
 */
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004
#define EXT2_FEATURE_COMPAT_HAS_DIR_INDEX	0x0020

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE	0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM		0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK	0x0020

#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER		0x0004
#define EXT4_FEATURE_INCOMPAT_EXTENTS		0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT		0x0080
#define EXT4_FEATURE_INCOMPAT_FLEX_BG		0x0200

/*
 * Special inodes numbers
 */
#define EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_ACL_IDX_INO	 3	/* ACL inode */
#define EXT2_ACL_DATA_INO	 4	/* ACL inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
#define EXT2_FT_UNKNOWN		0
#define EXT2_FT_REG_FILE	1
#define EXT2_FT_DIR		2
#define EXT2_FT_CHRDEV		3
#define EXT2_FT_BLKDEV 		4
#define EXT2_FT_FIFO		5
#define EXT2_FT_SOCK		6
#define EXT2_FT_SYMLINK		7

/*
 * Behaviour when detecting errors
 */
#define EXT2_ERRORS_CONTINUE		1	/* Continue execution */
#define EXT2_ERRORS_RO			2	/* Remount fs read-only */
#define EXT2_ERRORS_PANIC		3	/* Panic */
#define EXT2_ERRORS_DEFAULT		EXT2_ERRORS_CONTINUE

struct ext2_dir_entry_2
{
	uint32_t	inode;
	uint16_t	rec_len;
	uint8_t		name_len;
	uint8_t		file_type;
	char		name[255];
};

struct ext2_group_desc
{
	uint32_t	bg_block_bitmap;
	uint32_t	bg_inode_bitmap;
	uint32_t	bg_inode_table;
	uint16_t	bg_free_blocks_count;
	uint16_t	bg_free_inodes_count;
	uint16_t	bg_used_dirs_count;
	uint16_t	bg_pad;
	uint32_t	bg_reserved[3];
};

struct ext2_inode
{
	uint16_t	i_mode;		/* File mode */
	uint16_t	i_uid;		/* Owner Uid */
	uint32_t	i_size;		/* Size in bytes */
	uint32_t	i_atime;	/* Access time */
	uint32_t	i_ctime;	/* Creation time */
	uint32_t	i_mtime;	/* Modification time */
	uint32_t	i_dtime;	/* Deletion Time */
	uint16_t	i_gid;		/* Group Id */
	uint16_t	i_links_count;	/* Links count */
	uint32_t	i_blocks;	/* Blocks count */
	uint32_t	i_flags;	/* File flags */
	union {
		struct {
			uint32_t	l_i_reserved1;
		} linux1;
		struct {
			uint32_t	h_i_translator;
		} hurd1;
		struct {
			uint32_t	m_i_reserved1;
		} masix1;
	} osd1;			/* OS dependent 1 */
	uint32_t	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	uint32_t	i_generation;	/* File version (for NFS) */
	uint32_t	i_file_acl;	/* File ACL */
	uint32_t	i_dir_acl;	/* Directory ACL */
	uint32_t	i_faddr;	/* Fragment address */
	union {
		struct {
			uint8_t		l_i_frag;	/* Fragment number */
			uint8_t		l_i_fsize;	/* Fragment size */
			uint16_t	i_pad1;
			uint32_t	l_i_reserved2[2];
		} linux2;
		struct {
			uint8_t		h_i_frag;	/* Fragment number */
			uint8_t		h_i_fsize;	/* Fragment size */
			uint16_t	h_i_mode_high;
			uint16_t	h_i_uid_high;
			uint16_t	h_i_gid_high;
			uint32_t	h_i_author;
		} hurd2;
		struct {
			uint8_t		m_i_frag;	/* Fragment number */
			uint8_t		m_i_fsize;	/* Fragment size */
			uint16_t	m_pad1;
			uint32_t	m_i_reserved2[2];
		} masix2;
	} osd2;					/* OS dependent 2 */
};

#define i_size_high	i_dir_acl

struct ext2_super_block
{
	uint32_t	s_inodes_count;		/* Inodes count */
	uint32_t	s_blocks_count;		/* Blocks count */
	uint32_t	s_r_blocks_count;	/* Reserved blocks count */
	uint32_t	s_free_blocks_count;	/* Free blocks count */
	uint32_t	s_free_inodes_count;	/* Free inodes count */
	uint32_t	s_first_data_block;	/* First Data Block */
	uint32_t	s_log_block_size;	/* Block size */
	int32_t		s_log_frag_size;	/* Fragment size */
	uint32_t	s_blocks_per_group;	/* # Blocks per group */
	uint32_t	s_frags_per_group;	/* # Fragments per group */
	uint32_t	s_inodes_per_group;	/* # Inodes per group */
	uint32_t	s_mtime;		/* Mount time */
	uint32_t	s_wtime;		/* Write time */
	uint16_t	s_mnt_count;		/* Mount count */
	int16_t		s_max_mnt_count;	/* Maximal mount count */
	uint16_t	s_magic;		/* Magic signature */
	uint16_t	s_state;		/* File system state */
	uint16_t	s_errors;		/* Behaviour when detecting errors */
	uint16_t	s_minor_rev_level;	/* minor revision level */
	uint32_t	s_lastcheck;		/* time of last check */
	uint32_t	s_checkinterval;	/* max. time between checks */
	uint32_t	s_creator_os;		/* OS */
	uint32_t	s_rev_level;		/* Revision level */
	uint16_t	s_def_resuid;		/* Default uid for reserved blocks */
	uint16_t	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the file system.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	uint32_t	s_first_ino;		/* First non-reserved inode */
	uint16_t	s_inode_size;		/* size of inode structure */
	uint16_t	s_block_group_nr;	/* block group # of this superblock */
	uint32_t	s_feature_compat;	/* compatible feature set */
	uint32_t	s_feature_incompat;	/* incompatible feature set */
	uint32_t	s_feature_ro_compat;	/* readonly-compatible feature set */
	uint8_t		s_uuid[16];		/* 128-bit uuid for volume */
	char		s_volume_name[16];	/* volume name */
	char		s_last_mounted[64];	/* directory where last mounted */
	uint32_t	s_algorithm_usage_bitmap;  /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	uint8_t		s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	uint8_t		s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	uint16_t	s_padding1;
	/*
	 * Journaling support valid if EXT2_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	uint8_t		s_journal_uuid[16];	/* uuid of journal superblock */
	uint32_t	s_journal_inum;		/* inode number of journal file */
	uint32_t	s_journal_dev;		/* device number of journal file */
	uint32_t	s_last_orphan;		/* start of list of inodes to delete */

	uint32_t	s_reserved[197];	/* Padding to the end of the block */
};

#define EXT2_DIRENT_INODE(dir_ent)	(PED_LE32_TO_CPU((dir_ent).inode))
#define EXT2_DIRENT_REC_LEN(dir_ent)	(PED_LE16_TO_CPU((dir_ent).rec_len))
#define EXT2_DIRENT_NAME_LEN(dir_ent)	((dir_ent).name_len)
#define EXT2_DIRENT_FILE_TYPE(dir_ent)	((dir_ent).file_type)

#define EXT2_GROUP_BLOCK_BITMAP(gd)	(PED_LE32_TO_CPU((gd).bg_block_bitmap))
#define EXT2_GROUP_INODE_BITMAP(gd)	(PED_LE32_TO_CPU((gd).bg_inode_bitmap))
#define EXT2_GROUP_INODE_TABLE(gd)	(PED_LE32_TO_CPU((gd).bg_inode_table))
#define EXT2_GROUP_FREE_BLOCKS_COUNT(gd) \
		(PED_LE16_TO_CPU((gd).bg_free_blocks_count))
#define EXT2_GROUP_FREE_INODES_COUNT(gd) \
		(PED_LE16_TO_CPU((gd).bg_free_inodes_count))
#define EXT2_GROUP_USED_DIRS_COUNT(gd) \
		(PED_LE16_TO_CPU((gd).bg_used_dirs_count))

#define EXT2_INODE_MODE(inode)		(PED_LE16_TO_CPU((inode).i_mode))
#define EXT2_INODE_UID(inode)		(PED_LE16_TO_CPU((inode).i_uid))
#define EXT2_INODE_SIZE(inode) \
	((uint64_t) PED_LE32_TO_CPU((inode).i_size) \
	+ ((uint64_t) PED_LE32_TO_CPU((inode).i_size_high) << 32))
#define EXT2_INODE_ATIME(inode)		(PED_LE32_TO_CPU((inode).i_atime))
#define EXT2_INODE_CTIME(inode)		(PED_LE32_TO_CPU((inode).i_ctime))
#define EXT2_INODE_MTIME(inode)		(PED_LE32_TO_CPU((inode).i_mtime))
#define EXT2_INODE_DTIME(inode)		(PED_LE32_TO_CPU((inode).i_dtime))
#define EXT2_INODE_GID(inode)		(PED_LE16_TO_CPU((inode).i_gid))
#define EXT2_INODE_LINKS_COUNT(inode)	(PED_LE16_TO_CPU((inode).i_links_count))
#define EXT2_INODE_BLOCKS(inode)	(PED_LE32_TO_CPU((inode).i_blocks))
#define EXT2_INODE_FLAGS(inode)		(PED_LE32_TO_CPU((inode).i_flags))
#define EXT2_INODE_TRANSLATOR(inode)	(PED_LE32_TO_CPU((inode).osd1.hurd1.h_i_translator))
#define EXT2_INODE_BLOCK(inode, blk)	(PED_LE32_TO_CPU((inode).i_block[blk]))

#define EXT2_SUPER_INODES_COUNT(sb)	(PED_LE32_TO_CPU((sb).s_inodes_count))
#define EXT2_SUPER_BLOCKS_COUNT(sb)	(PED_LE32_TO_CPU((sb).s_blocks_count))
#define EXT2_SUPER_R_BLOCKS_COUNT(sb)	(PED_LE32_TO_CPU((sb).s_r_blocks_count))
#define EXT2_SUPER_FREE_BLOCKS_COUNT(sb) \
		(PED_LE32_TO_CPU((sb).s_free_blocks_count))
#define EXT2_SUPER_FREE_INODES_COUNT(sb) \
		(PED_LE32_TO_CPU((sb).s_free_inodes_count))
#define EXT2_SUPER_FIRST_DATA_BLOCK(sb) \
		(PED_LE32_TO_CPU((sb).s_first_data_block))
#define EXT2_SUPER_LOG_BLOCK_SIZE(sb)	(PED_LE32_TO_CPU((sb).s_log_block_size))
#define EXT2_SUPER_LOG_FRAG_SIZE(sb) \
		((int32_t) PED_LE32_TO_CPU((sb).s_log_frag_size))
#define EXT2_SUPER_BLOCKS_PER_GROUP(sb)	\
		(PED_LE32_TO_CPU((sb).s_blocks_per_group))
#define EXT2_SUPER_FRAGS_PER_GROUP(sb) \
	(PED_LE32_TO_CPU((sb).s_frags_per_group))
#define EXT2_SUPER_INODES_PER_GROUP(sb)	\
	(PED_LE32_TO_CPU((sb).s_inodes_per_group))
#define EXT2_SUPER_MTIME(sb)		(PED_LE32_TO_CPU((sb).s_mtime))
#define EXT2_SUPER_WTIME(sb)		(PED_LE32_TO_CPU((sb).s_wtime))
#define EXT2_SUPER_MNT_COUNT(sb)	(PED_LE16_TO_CPU((sb).s_mnt_count))
#define EXT2_SUPER_MAX_MNT_COUNT(sb) \
		((int16_t) PED_LE16_TO_CPU((sb).s_max_mnt_count))
#define EXT2_SUPER_MAGIC(sb)		(PED_LE16_TO_CPU((sb).s_magic))
#define EXT2_SUPER_STATE(sb)		(PED_LE16_TO_CPU((sb).s_state))
#define EXT2_SUPER_ERRORS(sb)		(PED_LE16_TO_CPU((sb).s_errors))
#define EXT2_SUPER_MINOR_REV_LEVEL(sb) \
		(PED_LE16_TO_CPU((sb).s_minor_rev_level))
#define EXT2_SUPER_LASTCHECK(sb)	(PED_LE32_TO_CPU((sb).s_lastcheck))
#define EXT2_SUPER_CHECKINTERVAL(sb)	(PED_LE32_TO_CPU((sb).s_checkinterval))
#define EXT2_SUPER_CREATOR_OS(sb)	(PED_LE32_TO_CPU((sb).s_creator_os))
#define EXT2_SUPER_REV_LEVEL(sb)	(PED_LE32_TO_CPU((sb).s_rev_level))
#define EXT2_SUPER_DEF_RESUID(sb)	(PED_LE16_TO_CPU((sb).s_def_resuid))
#define EXT2_SUPER_DEF_RESGID(sb)	(PED_LE16_TO_CPU((sb).s_def_resgid))

#define EXT2_SUPER_FIRST_INO(sb)	(PED_LE32_TO_CPU((sb).s_first_ino))
#define EXT2_SUPER_INODE_SIZE(sb)	(PED_LE16_TO_CPU((sb).s_inode_size))
#define EXT2_SUPER_BLOCK_GROUP_NR(sb)	(PED_LE16_TO_CPU((sb).s_block_group_nr))
#define EXT2_SUPER_FEATURE_COMPAT(sb)	(PED_LE32_TO_CPU((sb).s_feature_compat))
#define EXT2_SUPER_FEATURE_INCOMPAT(sb) \
		(PED_LE32_TO_CPU((sb).s_feature_incompat))
#define EXT2_SUPER_FEATURE_RO_COMPAT(sb) \
		(PED_LE32_TO_CPU((sb).s_feature_ro_compat))
#define EXT2_SUPER_UUID(sb)		((sb).s_uuid)
#define EXT2_SUPER_VOLUME_NAME(sb)	((sb).s_volume_name)
#define EXT2_SUPER_LAST_MOUNTED(sb)	((sb).s_last_mounted)
#define EXT2_SUPER_ALGORITHM_USAGE_BITMAP(sb) \
		(PED_LE32_TO_CPU((sb).s_algorithm_usage_bitmap))

#define EXT2_SUPER_JOURNAL_UUID(sb)	((sb).s_journal_uuid)
#define EXT2_SUPER_JOURNAL_INUM(sb)	(PED_LE32_TO_CPU((sb).s_journal_inum))
#define EXT2_SUPER_JOURNAL_DEV(sb)	(PED_LE32_TO_CPU((sb).s_journal_dev))
#define EXT2_SUPER_LAST_ORPHAN(sb)	(PED_LE32_TO_CPU((sb).s_last_orphan))

#endif
