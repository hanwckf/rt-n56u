/*
 * xattrs.h : definitions related to system extended attributes
 *
 * Copyright (c) 2010 Jean-Pierre Andre
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

#ifndef _NTFS_XATTR_H_
#define _NTFS_XATTR_H_

/*
 *		Identification of data mapped to the system name space
 */

enum SYSTEMXATTRS {
	XATTR_UNMAPPED,
	XATTR_NTFS_ACL,
	XATTR_NTFS_ATTRIB,
	XATTR_NTFS_ATTRIB_BE,
	XATTR_NTFS_EFSINFO,
	XATTR_NTFS_REPARSE_DATA,
	XATTR_NTFS_OBJECT_ID,
	XATTR_NTFS_DOS_NAME,
	XATTR_NTFS_TIMES,
	XATTR_NTFS_TIMES_BE,
	XATTR_NTFS_CRTIME,
	XATTR_NTFS_CRTIME_BE,
	XATTR_POSIX_ACC, 
	XATTR_POSIX_DEF
} ;

struct XATTRMAPPING {
	struct XATTRMAPPING *next;
	enum SYSTEMXATTRS xattr;
	char name[1]; /* variable length */
} ;

#ifdef XATTR_MAPPINGS

struct XATTRMAPPING *ntfs_xattr_build_mapping(ntfs_volume *vol,
			const char *path);
void ntfs_xattr_free_mapping(struct XATTRMAPPING*);

#endif /* XATTR_MAPPINGS */

enum SYSTEMXATTRS ntfs_xattr_system_type(const char *name,
			ntfs_volume *vol);

int ntfs_xattr_system_getxattr(struct SECURITY_CONTEXT *scx,
			enum SYSTEMXATTRS attr,
			ntfs_inode *ni, ntfs_inode *dir_ni,
			char *value, size_t size);
int ntfs_xattr_system_setxattr(struct SECURITY_CONTEXT *scx,
			enum SYSTEMXATTRS attr,
			ntfs_inode *ni, ntfs_inode *dir_ni,
			const char *value, size_t size, int flags);
int ntfs_xattr_system_removexattr(struct SECURITY_CONTEXT *scx,
			enum SYSTEMXATTRS attr,
			ntfs_inode *ni, ntfs_inode *dir_ni);

#endif /* _NTFS_XATTR_H_ */
