/*
 * dir.h - Exports for directory handling.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2002      Anton Altaparmakov
 * Copyright (c) 2005-2006 Yura Pakhuchiy
 * Copyright (c) 2004-2005 Richard Russon
 * Copyright (c) 2005-2008 Szabolcs Szakacsits
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

#ifndef _NTFS_DIR_H
#define _NTFS_DIR_H

#include "types.h"

#define PATH_SEP '/'

/*
 * The little endian Unicode strings $I30, $SII, $SDH, $O, $Q, $R
 * as a global constant.
 */
extern ntfschar NTFS_INDEX_I30[5];
extern ntfschar NTFS_INDEX_SII[5];
extern ntfschar NTFS_INDEX_SDH[5];
extern ntfschar NTFS_INDEX_O[3];
extern ntfschar NTFS_INDEX_Q[3];
extern ntfschar NTFS_INDEX_R[3];

extern int ntfs_inode_lookup_by_name(struct ntfs_inode *dir_ni,
				     const ntfschar *uname,
				     const int uname_len, u64 *mref,
				     struct FILE_NAME_ATTR **fne);
extern int ntfs_inode_lookup_by_mbsname(struct ntfs_inode *dir_ni,
					const char *name, u64 *inum);
extern void ntfs_inode_update_mbsname(struct ntfs_inode *dir_ni,
				      const char *name, u64 inum);

extern struct ntfs_inode *ntfs_pathname_to_inode(struct ntfs_volume *vol,
						 struct ntfs_inode *parent,
						 const char *pathname);
extern struct ntfs_inode *ntfs_create(struct ntfs_inode *dir_ni, le32 securid,
				      const ntfschar *name, u8 name_len,
				      mode_t type);
extern struct ntfs_inode *ntfs_create_device(struct ntfs_inode *dir_ni,
					     le32 securid,
					     const ntfschar *name, u8 name_len,
					     mode_t type, dev_t dev);
extern struct ntfs_inode *ntfs_create_symlink(struct ntfs_inode *dir_ni,
					      le32 securid,
					      const ntfschar *name,
					      u8 name_len,
					      const ntfschar *target,
					      int target_len);
extern int ntfs_check_empty_dir(struct ntfs_inode *ni);
extern int ntfs_unlink(struct ntfs_volume *vol, struct ntfs_inode *ni,
		       struct ntfs_inode *dir_ni, const ntfschar *name,
		       u8 name_len);
extern int ntfs_inode_free(struct ntfs_inode *ni);
extern int ntfs_link(struct ntfs_inode *ni, struct ntfs_inode *dir_ni,
		     const ntfschar *name, u8 name_len);

/*
 * File types (adapted from include <linux/fs.h>)
 */
#define NTFS_DT_UNKNOWN		0
#define NTFS_DT_FIFO		1
#define NTFS_DT_CHR		2
#define NTFS_DT_DIR		4
#define NTFS_DT_BLK		6
#define NTFS_DT_REG		8
#define NTFS_DT_LNK		10
#define NTFS_DT_SOCK		12
#define NTFS_DT_WHT		14

/*
 * This is the "ntfs_filldir" function type, used by ntfs_readdir() to let
 * the caller specify what kind of dirent layout it wants to have.
 * This allows the caller to read directories into their application or
 * to have different dirent layouts depending on the binary type.
 */
typedef int (*ntfs_filldir_t) (void *dirent, const ntfschar *name,
			       const int name_len, const int name_type,
			       const s64 pos, const MFT_REF mref,
			       const unsigned dt_type);

extern int ntfs_readdir(struct ntfs_inode *dir_ni, s64 *pos,
			void *dirent, ntfs_filldir_t filldir);

struct ntfs_inode *ntfs_dir_parent_inode(struct ntfs_inode *ni);

int ntfs_get_ntfs_dos_name(struct ntfs_inode *ni, struct ntfs_inode *dir_ni,
			   char *value, size_t size);
int ntfs_set_ntfs_dos_name(struct ntfs_inode *ni, struct ntfs_inode *dir_ni,
			   const char *value, size_t size, int flags);
int ntfs_remove_ntfs_dos_name(struct ntfs_inode *ni, struct ntfs_inode *dir_ni);

#endif /* defined _NTFS_DIR_H */
