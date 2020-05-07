/*
 * volume.h - Exports for NTFS volume handling.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2004 Anton Altaparmakov
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2005-2006 Yura Pakhuchiy
 * Copyright (c) 2005-2009 Szabolcs Szakacsits
 * Copyright (c) 2010      Jean-Pierre Andre
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _NTFS_VOLUME_H
#define _NTFS_VOLUME_H

/* Forward declaration */
struct ntfs_volume;

#include <linux/semaphore.h>

#include "param.h"
#include "types.h"
#include "support.h"
#include "device.h"
#include "inode.h"
#include "attrib.h"
#include "index.h"

/**
 * enum ntfs_mount_flags -
 *
 * Flags for the ntfs_mount() function.
 */
enum ntfs_mount_flags {
	NTFS_MNT_NONE = 0x00000000UL,
	NTFS_MNT_RDONLY = 0x00000001UL,
	NTFS_MNT_MAY_RDONLY = 0x02000000UL,	/* Allow fallback to ro */
	NTFS_MNT_FORENSIC = 0x04000000UL,	/* No modification during
					 * mount. */
	NTFS_MNT_EXCLUSIVE = 0x08000000UL,
	NTFS_MNT_RECOVER = 0x10000000UL,
	NTFS_MNT_IGNORE_HIBERFILE = 0x20000000UL,
};

/**
 * enum ntfs_mounted_flags -
 *
 * Flags returned by the ntfs_check_if_mounted() function.
 */
enum ntfs_mounted_flags {
	NTFS_MF_MOUNTED = 1,	/* Device is mounted. */
	NTFS_MF_ISROOT = 2,	/* Device is mounted as system root. */
	NTFS_MF_READONLY = 4,	/* Device is mounted read-only. */
};

extern int ntfs_check_if_mounted(const char *file, unsigned long *mnt_flags);

enum ntfs_volume_status {
	NTFS_VOLUME_OK = 0,
	NTFS_VOLUME_SYNTAX_ERROR = 11,
	NTFS_VOLUME_NOT_NTFS = 12,
	NTFS_VOLUME_CORRUPT = 13,
	NTFS_VOLUME_HIBERNATED = 14,
	NTFS_VOLUME_UNCLEAN_UNMOUNT = 15,
	NTFS_VOLUME_LOCKED = 16,
	NTFS_VOLUME_RAID = 17,
	NTFS_VOLUME_UNKNOWN_REASON = 18,
	NTFS_VOLUME_NO_PRIVILEGE = 19,
	NTFS_VOLUME_OUT_OF_MEMORY = 20,
	NTFS_VOLUME_FUSE_ERROR = 21,
	NTFS_VOLUME_INSECURE = 22
};

/**
 * enum ntfs_volume_state_bits -
 *
 * Defined bits for the state field in the ntfs_volume structure.
 */
enum ntfs_volume_state_bits {
	NV_ReadOnly,		/* 1: Volume is read-only. */
	NV_CaseSensitive,	/* 1: Volume is mounted case-sensitive. */
	NV_LogFileEmpty,	/* 1: $logFile journal is empty. */
	NV_ShowSysFiles,	/* 1: Show NTFS metafiles. */
	NV_ShowHidFiles,	/* 1: Show files marked hidden. */
	NV_HideDotFiles,	/* 1: Set hidden flag on dot files */
	NV_Compression,		/* 1: allow compression */
	NV_NoFixupWarn,		/* 1: Do not log fixup errors */
};

#define  test_nvol_flag(nv, flag)	 test_bit(NV_##flag, (&(nv)->state))
#define   set_nvol_flag(nv, flag)	  set_bit(NV_##flag, (&(nv)->state))
#define clear_nvol_flag(nv, flag)	clear_bit(NV_##flag, (&(nv)->state))

#define NVolReadOnly(nv)		 test_nvol_flag(nv, ReadOnly)
#define NVolSetReadOnly(nv)		  set_nvol_flag(nv, ReadOnly)
#define NVolClearReadOnly(nv)		clear_nvol_flag(nv, ReadOnly)

#define NVolCaseSensitive(nv)		 test_nvol_flag(nv, CaseSensitive)
#define NVolSetCaseSensitive(nv)	  set_nvol_flag(nv, CaseSensitive)
#define NVolClearCaseSensitive(nv)	clear_nvol_flag(nv, CaseSensitive)

#define NVolLogFileEmpty(nv)		 test_nvol_flag(nv, LogFileEmpty)
#define NVolSetLogFileEmpty(nv)		  set_nvol_flag(nv, LogFileEmpty)
#define NVolClearLogFileEmpty(nv)	clear_nvol_flag(nv, LogFileEmpty)

#define NVolShowSysFiles(nv)		 test_nvol_flag(nv, ShowSysFiles)
#define NVolSetShowSysFiles(nv)		  set_nvol_flag(nv, ShowSysFiles)
#define NVolClearShowSysFiles(nv)	clear_nvol_flag(nv, ShowSysFiles)

#define NVolShowHidFiles(nv)		 test_nvol_flag(nv, ShowHidFiles)
#define NVolSetShowHidFiles(nv)		  set_nvol_flag(nv, ShowHidFiles)
#define NVolClearShowHidFiles(nv)	clear_nvol_flag(nv, ShowHidFiles)

#define NVolHideDotFiles(nv)		 test_nvol_flag(nv, HideDotFiles)
#define NVolSetHideDotFiles(nv)		  set_nvol_flag(nv, HideDotFiles)
#define NVolClearHideDotFiles(nv)	clear_nvol_flag(nv, HideDotFiles)

#define NVolCompression(nv)		 test_nvol_flag(nv, Compression)
#define NVolSetCompression(nv)		  set_nvol_flag(nv, Compression)
#define NVolClearCompression(nv)	clear_nvol_flag(nv, Compression)

#define NVolNoFixupWarn(nv)		 test_nvol_flag(nv, NoFixupWarn)
#define NVolSetNoFixupWarn(nv)		  set_nvol_flag(nv, NoFixupWarn)
#define NVolClearNoFixupWarn(nv)	clear_nvol_flag(nv, NoFixupWarn)

/*
 * NTFS version 1.1 and 1.2 are used by Windows NT4.
 * NTFS version 2.x is used by Windows 2000 Beta
 * NTFS version 3.0 is used by Windows 2000.
 * NTFS version 3.1 is used by Windows XP, 2003 and Vista.
 */

#define NTFS_V1_1(major, minor) ((major) == 1 && (minor) == 1)
#define NTFS_V1_2(major, minor) ((major) == 1 && (minor) == 2)
#define NTFS_V2_X(major, minor) ((major) == 2)
#define NTFS_V3_0(major, minor) ((major) == 3 && (minor) == 0)
#define NTFS_V3_1(major, minor) ((major) == 3 && (minor) == 1)

#define NTFS_BUF_SIZE 512 /*--- 8192 ---*/

/**
 * struct ntfs_volume - structure describing an open volume in memory.
 */
struct ntfs_volume {
	union {
		struct ntfs_device *dev;	/* NTFS device associated with
						   the volume. */
		void *sb;	/* For kernel porting compatibility. */
	};
	char *vol_name;		/* Name of the volume. */
	unsigned long state;	/* NTFS specific flags describing this volume.
				   See ntfs_volume_state_bits above. */

	struct ntfs_inode *vol_ni;	/* ntfs_inode for FILE_Volume. */
	u8 major_ver;		/* Ntfs major version of volume. */
	u8 minor_ver;		/* Ntfs minor version of volume. */
	le16 flags;		/* Bit array of VOLUME_* flags. */

	u16 sector_size;	/* Byte size of a sector. */
	u8 sector_size_bits;	/* Log(2) of the byte size of a sector. */
	u32 cluster_size;	/* Byte size of a cluster. */
	u32 mft_record_size;	/* Byte size of a mft record. */
	u32 indx_record_size;	/* Byte size of a INDX record. */
	u8 cluster_size_bits;	/* Log(2) of the byte size of a cluster. */
	u8 mft_record_size_bits;/* Log(2) of the byte size of a mft record. */
	u8 indx_record_size_bits;/* Log(2) of the byte size of a INDX record. */
	u64 serial_no;		/* Volume serial number from boot sector */

	/* Variables used by the cluster and mft allocators. */
	u8 mft_zone_multiplier;	/* Initial mft zone multiplier. */
	u8 full_zones;		/* cluster zones which are full */
	s64 mft_data_pos;	/* Mft record number at which to allocate the
				   next mft record. */
	LCN mft_zone_start;	/* First cluster of the mft zone. */
	LCN mft_zone_end;	/* First cluster beyond the mft zone. */
	LCN mft_zone_pos;	/* Current position in the mft zone. */
	LCN data1_zone_pos;	/* Current position in the first data zone. */
	LCN data2_zone_pos;	/* Current position in the second data zone. */

	s64 nr_clusters;	/* Volume size in clusters, hence also the
				   number of bits in lcn_bitmap. */
	struct ntfs_inode *lcnbmp_ni;	/* ntfs_inode for FILE_Bitmap. */
	struct ntfs_attr *lcnbmp_na;
				/* ntfs_attr structure for the data attribute
				   of FILE_Bitmap. Each bit represents a
				   cluster on the volume, bit 0 representing
				   lcn 0 and so on. A set bit means that the
				   cluster and vice versa. */

	LCN mft_lcn;		/* Logical cluster number of the data attribute
				   for FILE_MFT. */
	struct ntfs_inode *mft_ni;	/* ntfs_inode for FILE_MFT. */
	struct ntfs_attr *mft_na;
				/* ntfs_attr structure for the data attribute
				   of FILE_MFT. */
	struct ntfs_attr *mftbmp_na;
				/* ntfs_attr structure for the bitmap attribute
				   of FILE_MFT. Each bit represents an mft
				   record in the $DATA attribute, bit 0
				   representing mft record 0 and so on. A set
				   bit means that the mft record is in use and
				   vice versa. */

	struct ntfs_inode *secure_ni;	/* ntfs_inode for FILE $Secure */
	struct ntfs_index_context *secure_xsii;/* index for using $Secure:$SII*/
	struct ntfs_index_context *secure_xsdh;/* index for using $Secure:$SDH*/
	int secure_reentry;	/* check for non-rentries */
	/* TODO: remove secure_flags */
	unsigned int secure_flags;	/* flags, see security.h for values */

	int mftmirr_size;	/* Size of the FILE_MFTMirr in mft records. */
	LCN mftmirr_lcn;	/* Logical cluster number of the data attribute
				   for FILE_MFTMirr. */
	struct ntfs_inode *mftmirr_ni;	/* ntfs_inode for FILE_MFTMirr. */
	struct ntfs_attr *mftmirr_na;
				/* ntfs_attr structure for the data attribute
				   of FILE_MFTMirr. */

	ntfschar *upcase;	/* Upper case equivalents of all 65536 2-byte
				   Unicode characters. Obtained from
				   FILE_UpCase. */
	u32 upcase_len;		/* Length in Unicode characters of the upcase
				   table. */
	ntfschar *locase;	/* Lower case equivalents of all 65536 2-byte
				   Unicode characters. Only if option
				   case_ignore is set. */

	struct ATTR_DEF *attrdef;	/* Attribute definitions. Obtained from
				   FILE_AttrDef. */
	s32 attrdef_len;	/* Size of the attribute definition table in
				   bytes. */

	s64 free_clusters;	/* Track the number of free clusters which
				   greatly improves statfs() performance */
	s64 free_mft_records;	/* Same for free mft records (see above) */
	bool efs_raw;		/* volume is mounted for raw access to
				   efs-encrypted files */
#ifdef XATTR_MAPPINGS
	struct XATTRMAPPING *xattr_mapping;
#endif				/* XATTR_MAPPINGS */
	struct buffer_head *mftbmp_bh;	/* mft bitmap mapped to bufferhead */
	LCN mftbmp_start;	/* starting position of the bitmap currently
				   held in mftbmp_bh */
	struct mutex mftbmp_lock;
	/* FIXME:
	 * The recursive mutex defined here and at @ntfs_mftbmp_lock and
	 * @ntfs_mftbmp_unlock is a _evil and nasty hack_ to make the coarse
	 * locking scheme we currently use arround calls to
	 * @ntfs_mft_record_alloc and @ntfs_mft_record_free work "somehow".
	 *
	 * This is a) slow because it locks more than needed and b) dangerous
	 * because it opens up a whole world of possible weird locking problems.
	 * Goes hand in hand with the TODO to rework all code in index, attrib
	 * and mft to become multithread safe. Similar goes for the @lcnbmp_lock
	 * below.
	 */
	struct task_struct *mftbmp_lock_owner;
	int mftbmp_lock_count;
	spinlock_t mftbmp_spin_lock;

	struct buffer_head *lcnbmp_bh;	/* lcn bitmap mapped to bufferhead */
	LCN lcnbmp_start;	/* first lcn entry of the current lcnbmp */
	/* TODO:
	 * As for the @mftbmp_lock above, this needs some work. AFAIK it
	 * currently works, but this can be done way better.
	 */
	struct mutex lcnbmp_lock;
	struct mutex ni_lock;
	struct semaphore ni_stack_sem;
	struct ntfs_inode *ni_stack[4];  /* Hold a few ntfs inodes without VFS
					    inode. */
};

static inline int ntfs_mftbmp_lock(struct ntfs_volume *vol)
{
	unsigned long flags;

	if (mutex_trylock(&vol->mftbmp_lock)) {
lock_acquired:
		spin_lock_irqsave(&vol->mftbmp_spin_lock, flags);
		BUG_ON(vol->mftbmp_lock_count);
		vol->mftbmp_lock_owner = current;
		++vol->mftbmp_lock_count;
		spin_unlock_irqrestore(&vol->mftbmp_spin_lock, flags);
	} else {
		spin_lock_irqsave(&vol->mftbmp_spin_lock, flags);
		if (vol->mftbmp_lock_owner == current) {
			++vol->mftbmp_lock_count;
			spin_unlock_irqrestore(&vol->mftbmp_spin_lock, flags);
		} else {
			spin_unlock_irqrestore(&vol->mftbmp_spin_lock, flags);
			if (mutex_lock_interruptible(&vol->mftbmp_lock))
				return -ERESTARTSYS;
			goto lock_acquired;
		}
	}

	return 0;
}

static inline void ntfs_mftbmp_unlock(struct ntfs_volume *vol)
{
	unsigned long flags;

	BUG_ON(vol->mftbmp_lock_owner != current);
	spin_lock_irqsave(&vol->mftbmp_spin_lock, flags);
	if (!--vol->mftbmp_lock_count) {
		vol->mftbmp_lock_owner = NULL;
		spin_unlock_irqrestore(&vol->mftbmp_spin_lock, flags);
		mutex_unlock(&vol->mftbmp_lock);
	} else {
		spin_unlock_irqrestore(&vol->mftbmp_spin_lock, flags);
	}
}

extern const char *ntfs_home;

extern struct ntfs_volume *ntfs_volume_alloc(void);

extern struct ntfs_volume *ntfs_volume_startup(struct ntfs_device *dev,
					       enum ntfs_mount_flags flags);

extern struct ntfs_volume *ntfs_device_mount(struct ntfs_device *dev,
					     enum ntfs_mount_flags flags);

extern struct ntfs_volume *ntfs_mount(struct super_block *sb,
				      enum ntfs_mount_flags flags);
extern int ntfs_umount(struct ntfs_volume *vol, const bool force);

extern int ntfs_version_is_supported(struct ntfs_volume *vol);
extern int ntfs_volume_check_hiberfile(struct ntfs_volume *vol, int verbose);
extern int ntfs_logfile_reset(struct ntfs_volume *vol);

extern int ntfs_volume_write_flags(struct ntfs_volume *vol, const le16 flags);

extern int ntfs_volume_error(int err);
extern void ntfs_mount_error(const char *vol, const char *mntpoint, int err);

extern int ntfs_volume_get_free_space(struct ntfs_volume *vol);
extern int ntfs_volume_rename(struct ntfs_volume *vol, const ntfschar *label,
			      int label_len);

extern int ntfs_set_shown_files(struct ntfs_volume *vol,
				bool show_sys_files, bool show_hid_files,
				bool hide_dot_files);
extern int ntfs_set_locale(void);
extern int ntfs_set_ignore_case(struct ntfs_volume *vol);

#endif /* defined _NTFS_VOLUME_H */
