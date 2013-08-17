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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>

#include "exfat_version.h"
#include "exfat_config.h"
#include "exfat_global.h"
#include "exfat_data.h"
#include "exfat_oal.h"

#include "exfat_part.h"
#include "exfat_nls.h"
#include "exfat_api.h"
#include "exfat_super.h"
#include "exfat.h"

extern FS_STRUCT_T      fs_struct[];

extern struct semaphore z_sem;

INT32 FsInit(void)
{
	INT32 i;

	for (i = 0; i < MAX_DRIVE; i++) {
		fs_struct[i].mounted = FALSE;
		fs_struct[i].sb = NULL;
		sm_init(&(fs_struct[i].v_sem));
	}

	return(ffsInit());
}

INT32 FsShutdown(void)
{
	INT32 i;

	for (i = 0; i < MAX_DRIVE; i++) {
		if (!fs_struct[i].mounted) continue;

		ffsUmountVol(fs_struct[i].sb);
	}

	return(ffsShutdown());
}

INT32 FsMountVol(struct super_block *sb)
{
	INT32 err, drv;

	sm_P(&z_sem);

	for (drv = 0; drv < MAX_DRIVE; drv++) {
		if (!fs_struct[drv].mounted) break;
	}

	if (drv >= MAX_DRIVE) return(FFS_ERROR);

	sm_P(&(fs_struct[drv].v_sem));

	err = buf_init(sb);
	if (!err) {
		err = ffsMountVol(sb, drv);
	}

	sm_V(&(fs_struct[drv].v_sem));

	if (!err) {
		fs_struct[drv].mounted = TRUE;
		fs_struct[drv].sb = sb;
	} else {
		buf_shutdown(sb);
	}

	sm_V(&z_sem);

	return(err);
}

INT32 FsUmountVol(struct super_block *sb)
{
	INT32 err;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&z_sem);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsUmountVol(sb);
	buf_shutdown(sb);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	fs_struct[p_fs->drv].mounted = FALSE;
	fs_struct[p_fs->drv].sb = NULL;

	sm_V(&z_sem);

	return(err);
}

INT32 FsGetVolInfo(struct super_block *sb, VOL_INFO_T *info)
{
	INT32 err;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (info == NULL) return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsGetVolInfo(sb, info);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsSyncVol(struct super_block *sb, INT32 do_sync)
{
	INT32 err;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsSyncVol(sb, do_sync);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsLookupFile(struct inode *inode, UINT8 *path, FILE_ID_T *fid)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if ((fid == NULL) || (path == NULL) || (STRLEN(path) == 0))
		return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsLookupFile(inode, path, fid);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsCreateFile(struct inode *inode, UINT8 *path, UINT8 mode, FILE_ID_T *fid)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if ((fid == NULL) || (path == NULL) || (STRLEN(path) == 0))
		return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsCreateFile(inode, path, mode, fid);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsReadFile(struct inode *inode, FILE_ID_T *fid, void *buffer, UINT64 count, UINT64 *rcount)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (fid == NULL) return(FFS_INVALIDFID);

	if (buffer == NULL) return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsReadFile(inode, fid, buffer, count, rcount);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
} 

INT32 FsWriteFile(struct inode *inode, FILE_ID_T *fid, void *buffer, UINT64 count, UINT64 *wcount)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (fid == NULL) return(FFS_INVALIDFID);

	if (buffer == NULL) return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsWriteFile(inode, fid, buffer, count, wcount);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsTruncateFile(struct inode *inode, UINT64 old_size, UINT64 new_size)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	PRINTK("FsTruncateFile entered (inode %p size %llu)\n", inode, new_size);
	
	err = ffsTruncateFile(inode, old_size, new_size);
 
	PRINTK("FsTruncateFile exitted (%d)\n", err);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsMoveFile(struct inode *old_parent_inode, FILE_ID_T *fid, struct inode *new_parent_inode, struct dentry *new_dentry)
{
	INT32 err;
	struct super_block *sb = old_parent_inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (fid == NULL) return(FFS_INVALIDFID);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsMoveFile(old_parent_inode, fid, new_parent_inode, new_dentry);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsRemoveFile(struct inode *inode, FILE_ID_T *fid)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (fid == NULL) return(FFS_INVALIDFID);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsRemoveFile(inode, fid);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsSetAttr(struct inode *inode, UINT32 attr)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsSetAttr(inode, attr);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsReadStat(struct inode *inode, DIR_ENTRY_T *info)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsGetStat(inode, info);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsWriteStat(struct inode *inode, DIR_ENTRY_T *info)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	PRINTK("FsWriteStat entered (inode %p info %p\n", inode, info);

	err = ffsSetStat(inode, info);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	PRINTK("FsWriteStat exited (%d)\n", err);

	return(err);
} 

INT32 FsMapCluster(struct inode *inode, INT32 clu_offset, UINT32 *clu)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (clu == NULL) return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsMapCluster(inode, clu_offset, clu);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

INT32 FsCreateDir(struct inode *inode, UINT8 *path, FILE_ID_T *fid)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if ((fid == NULL) || (path == NULL) || (STRLEN(path) == 0))
		return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsCreateDir(inode, path, fid);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
} 

INT32 FsReadDir(struct inode *inode, DIR_ENTRY_T *dir_entry)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (dir_entry == NULL) return(FFS_ERROR);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsReadDir(inode, dir_entry);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
} 

INT32 FsRemoveDir(struct inode *inode, FILE_ID_T *fid)
{
	INT32 err;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	if (fid == NULL) return(FFS_INVALIDFID);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	err = ffsRemoveDir(inode, fid);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return(err);
}

EXPORT_SYMBOL(FsMountVol);
EXPORT_SYMBOL(FsUmountVol);
EXPORT_SYMBOL(FsGetVolInfo);
EXPORT_SYMBOL(FsSyncVol);
EXPORT_SYMBOL(FsLookupFile);
EXPORT_SYMBOL(FsCreateFile);
EXPORT_SYMBOL(FsReadFile);
EXPORT_SYMBOL(FsWriteFile);
EXPORT_SYMBOL(FsTruncateFile);
EXPORT_SYMBOL(FsMoveFile);
EXPORT_SYMBOL(FsRemoveFile);
EXPORT_SYMBOL(FsSetAttr);
EXPORT_SYMBOL(FsReadStat);
EXPORT_SYMBOL(FsWriteStat);
EXPORT_SYMBOL(FsMapCluster);
EXPORT_SYMBOL(FsCreateDir);
EXPORT_SYMBOL(FsReadDir);
EXPORT_SYMBOL(FsRemoveDir);

#if EXFAT_CONFIG_KERNEL_DEBUG
INT32 FsReleaseCache(struct super_block *sb)
{
	FS_INFO_T *p_fs = &(EXFAT_SB(sb)->fs_info);

	sm_P(&(fs_struct[p_fs->drv].v_sem));

	FAT_release_all(sb);
	buf_release_all(sb);

	sm_V(&(fs_struct[p_fs->drv].v_sem));

	return 0;
}

EXPORT_SYMBOL(FsReleaseCache);
#endif

static int __init init_exfat_core(void)
{
	int err;

	printk(KERN_INFO "exFAT: Core Version %s\n", EXFAT_VERSION);
	
	err = FsInit();
	if (err) {
		if (err == FFS_MEMORYERR)
			return -ENOMEM;
		else
			return -EIO;
	}

	return 0;
}

static void __exit exit_exfat_core(void)
{
	FsShutdown();
}

module_init(init_exfat_core);
module_exit(exit_exfat_core);

MODULE_LICENSE("GPL");
