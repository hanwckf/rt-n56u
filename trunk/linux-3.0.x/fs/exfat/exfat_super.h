/* Some of the source code in this file came from "linux/fs/fat/fat.h".  */

/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _EXFAT_LINUX_H
#define _EXFAT_LINUX_H

#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/nls.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/swap.h>

#include "exfat_config.h"
#include "exfat_global.h"
#include "exfat_data.h"
#include "exfat_oal.h"

#include "exfat_blkdev.h"
#include "exfat_cache.h"
#include "exfat_part.h"
#include "exfat_nls.h"
#include "exfat_api.h"
#include "exfat.h"

#define EXFAT_ERRORS_CONT  1
#define EXFAT_ERRORS_PANIC 2
#define EXFAT_ERRORS_RO    3

#define EXFAT_IOCTL_GET_VOLUME_ID _IOR('r', 0x12, __u32)

struct exfat_mount_options {
	uid_t fs_uid;
	gid_t fs_gid;
	unsigned short fs_fmask;
	unsigned short fs_dmask;
	unsigned short allow_utime;
	unsigned short codepage;  
	char *iocharset;         
	unsigned char casesensitive;
	unsigned char errors;   
#if EXFAT_CONFIG_DISCARD
	unsigned char discard; 
#endif
};

#define EXFAT_HASH_BITS    8
#define EXFAT_HASH_SIZE    (1UL << EXFAT_HASH_BITS)

struct exfat_sb_info {
	FS_INFO_T fs_info;
	BD_INFO_T bd_info;

	struct exfat_mount_options options;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,00)
	int s_dirt;
	struct mutex s_lock;
#endif
	struct nls_table *nls_disk;
	struct nls_table *nls_io; 

	struct inode *fat_inode;

	spinlock_t inode_hash_lock;
	struct hlist_head inode_hashtable[EXFAT_HASH_SIZE];
#if EXFAT_CONFIG_KERNEL_DEBUG
	long debug_flags;
#endif
};

struct exfat_inode_info {
	FILE_ID_T fid;
	char  *target;
	loff_t mmu_private;    
	loff_t i_pos;         
	struct hlist_node i_hash_fat; 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,00)
	struct rw_semaphore truncate_lock;
#endif
	struct inode vfs_inode;
};

#define EXFAT_SB(sb)		((struct exfat_sb_info *)((sb)->s_fs_info))

static inline struct exfat_inode_info *EXFAT_I(struct inode *inode) {
	return container_of(inode, struct exfat_inode_info, vfs_inode);
}

static inline int exfat_mode_can_hold_ro(struct inode *inode)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);

	if (S_ISDIR(inode->i_mode))
		return 0;

	if ((~sbi->options.fs_fmask) & S_IWUGO)
		return 1;
	return 0;
}

static inline mode_t exfat_make_mode(struct exfat_sb_info *sbi,
									 u32 attr, mode_t mode)
{
	if ((attr & ATTR_READONLY) && !(attr & ATTR_SUBDIR))
		mode &= ~S_IWUGO;

	if (attr & ATTR_SUBDIR)
		return (mode & ~sbi->options.fs_dmask) | S_IFDIR;
	else if (attr & ATTR_SYMLINK)
		return (mode & ~sbi->options.fs_dmask) | S_IFLNK;
	else
		return (mode & ~sbi->options.fs_fmask) | S_IFREG;
}

static inline u32 exfat_make_attr(struct inode *inode)
{
	if (exfat_mode_can_hold_ro(inode) && !(inode->i_mode & S_IWUGO))
		return ((EXFAT_I(inode)->fid.attr) | ATTR_READONLY);
	else
		return (EXFAT_I(inode)->fid.attr);
}

static inline void exfat_save_attr(struct inode *inode, u32 attr)
{
	if (exfat_mode_can_hold_ro(inode))
		EXFAT_I(inode)->fid.attr = attr & ATTR_RWMASK;
	else
		EXFAT_I(inode)->fid.attr = attr & (ATTR_RWMASK | ATTR_READONLY);
}

#endif
