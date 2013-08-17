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

#ifndef _EXFAT_API_H
#define _EXFAT_API_H

#include "exfat_config.h"
#include "exfat_global.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXFAT_SUPER_MAGIC       (0x2011BAB0L)
#define EXFAT_ROOT_INO          1

#define FAT12                   0x01    
#define FAT16                   0x0E    
#define FAT32                   0x0C    
#define EXFAT                   0x07    

#define MAX_CHARSET_SIZE        3       
#define MAX_PATH_DEPTH          15      
#define MAX_NAME_LENGTH         256     
#define MAX_PATH_LENGTH         260     
#define DOS_NAME_LENGTH         11      
#define DOS_PATH_LENGTH         80      

#define ATTR_NORMAL             0x0000
#define ATTR_READONLY           0x0001
#define ATTR_HIDDEN             0x0002
#define ATTR_SYSTEM             0x0004
#define ATTR_VOLUME             0x0008
#define ATTR_SUBDIR             0x0010
#define ATTR_ARCHIVE            0x0020
#define ATTR_SYMLINK            0x0040
#define ATTR_EXTEND             0x000F
#define ATTR_RWMASK             0x007E

#define FM_REGULAR              0x00
#define FM_SYMLINK              0x40

#define FFS_SUCCESS             0
#define FFS_MEDIAERR            1
#define FFS_FORMATERR           2
#define FFS_MOUNTED             3
#define FFS_NOTMOUNTED          4
#define FFS_ALIGNMENTERR        5
#define FFS_SEMAPHOREERR        6
#define FFS_INVALIDPATH         7
#define FFS_INVALIDFID          8
#define FFS_NOTFOUND            9
#define FFS_FILEEXIST           10
#define FFS_PERMISSIONERR       11
#define FFS_NOTOPENED           12
#define FFS_MAXOPENED           13
#define FFS_FULL                14
#define FFS_EOF                 15
#define FFS_DIRBUSY             16
#define FFS_MEMORYERR           17
#define FFS_NAMETOOLONG		18
#define FFS_ERROR               19      

	typedef struct {
		UINT16      Year;
		UINT16      Month;
		UINT16      Day;
		UINT16      Hour;
		UINT16      Minute;
		UINT16      Second;
		UINT16      MilliSecond;
	} DATE_TIME_T;

	typedef struct {
		UINT32      Offset;    
		UINT32      Size;      
	} PART_INFO_T;

	typedef struct {
		UINT32      SecSize;    
		UINT32      DevSize;    
	} DEV_INFO_T;

	typedef struct {
		UINT32      FatType;
		UINT32      ClusterSize;
		UINT32      NumClusters;
		UINT32      FreeClusters;
		UINT32      UsedClusters;
	} VOL_INFO_T;

	typedef struct {
		UINT32      dir;
		INT32       size;
		UINT8       flags;
	} CHAIN_T;

	typedef struct {
		CHAIN_T     dir;
		INT32       entry;
		UINT32      type;
		UINT32      attr;
		UINT32      start_clu;
		UINT64      size;
		UINT8       flags;
		INT64       rwoffset;
		INT32       hint_last_off;
		UINT32      hint_last_clu;
	} FILE_ID_T;

	typedef struct {
		INT8        Name[MAX_NAME_LENGTH *MAX_CHARSET_SIZE];
		INT8        ShortName[DOS_NAME_LENGTH + 2];     
		UINT32      Attr;
		UINT64      Size;
		UINT32      NumSubdirs;
		DATE_TIME_T CreateTimestamp;
		DATE_TIME_T ModifyTimestamp;
		DATE_TIME_T AccessTimestamp;
	} DIR_ENTRY_T;

	INT32 FsInit(void);
	INT32 FsShutdown(void);

	INT32 FsMountVol(struct super_block *sb);
	INT32 FsUmountVol(struct super_block *sb);
	INT32 FsGetVolInfo(struct super_block *sb, VOL_INFO_T *info);
	INT32 FsSyncVol(struct super_block *sb, INT32 do_sync);

	INT32 FsLookupFile(struct inode *inode, UINT8 *path, FILE_ID_T *fid);
	INT32 FsCreateFile(struct inode *inode, UINT8 *path, UINT8 mode, FILE_ID_T *fid);
	INT32 FsReadFile(struct inode *inode, FILE_ID_T *fid, void *buffer, UINT64 count, UINT64 *rcount);
	INT32 FsWriteFile(struct inode *inode, FILE_ID_T *fid, void *buffer, UINT64 count, UINT64 *wcount);
	INT32 FsTruncateFile(struct inode *inode, UINT64 old_size, UINT64 new_size);
	INT32 FsMoveFile(struct inode *old_parent_inode, FILE_ID_T *fid, struct inode *new_parent_inode, struct dentry *new_dentry);
	INT32 FsRemoveFile(struct inode *inode, FILE_ID_T *fid);
	INT32 FsSetAttr(struct inode *inode, UINT32 attr);
	INT32 FsReadStat(struct inode *inode, DIR_ENTRY_T *info);
	INT32 FsWriteStat(struct inode *inode, DIR_ENTRY_T *info);
	INT32 FsMapCluster(struct inode *inode, INT32 clu_offset, UINT32 *clu);

	INT32 FsCreateDir(struct inode *inode, UINT8 *path, FILE_ID_T *fid);
	INT32 FsReadDir(struct inode *inode, DIR_ENTRY_T *dir_entry);
	INT32 FsRemoveDir(struct inode *inode, FILE_ID_T *fid);

	INT32 FsReleaseCache(struct super_block *sb);
#ifdef __cplusplus
}
#endif 
#endif
