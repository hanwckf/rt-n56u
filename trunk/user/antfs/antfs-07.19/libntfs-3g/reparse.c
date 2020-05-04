/**
 * reparse.c - Processing of reparse points
 *
 *	This module is part of ntfs-3g library
 *
 * Copyright (c) 2008-2016 Jean-Pierre Andre
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
 */

#include "antfs.h"
#include "types.h"
#include "debug.h"
#include "layout.h"
#include "attrib.h"
#include "inode.h"
#include "dir.h"
#include "volume.h"
#include "mft.h"
#include "index.h"
#include "lcnalloc.h"
#include "misc.h"
#include "reparse.h"

struct MOUNT_POINT_REPARSE_DATA {      /* reparse data for junctions */
	le16	subst_name_offset;
	le16	subst_name_length;
	le16	print_name_offset;
	le16	print_name_length;
	char	path_buffer[0];      /* above data assume this is char array */
};

struct SYMLINK_REPARSE_DATA {          /* reparse data for symlinks */
	le16	subst_name_offset;
	le16	subst_name_length;
	le16	print_name_offset;
	le16	print_name_length;
	le32	flags;		     /* 1 for full target, otherwise 0 */
	char	path_buffer[0];      /* above data assume this is char array */
};

struct REPARSE_INDEX {			/* index entry in $Extend/$Reparse */
	struct INDEX_ENTRY_HEADER header;
	struct REPARSE_INDEX_KEY key;
	le32 filling;
};

static const ntfschar dir_junction_head[] = {
	const_cpu_to_le16('\\'),
	const_cpu_to_le16('?'),
	const_cpu_to_le16('?'),
	const_cpu_to_le16('\\')
};

static const ntfschar vol_junction_head[] = {
	const_cpu_to_le16('\\'),
	const_cpu_to_le16('?'),
	const_cpu_to_le16('?'),
	const_cpu_to_le16('\\'),
	const_cpu_to_le16('V'),
	const_cpu_to_le16('o'),
	const_cpu_to_le16('l'),
	const_cpu_to_le16('u'),
	const_cpu_to_le16('m'),
	const_cpu_to_le16('e'),
	const_cpu_to_le16('{'),
};

static ntfschar reparse_index_name[] = { const_cpu_to_le16('$'),
					 const_cpu_to_le16('R') };

static const char mappingdir[] = ".NTFS-3G/";

/*
 *		Fix a file name with doubtful case in some directory index
 *	and return the name with the casing used in directory.
 *
 *	Should only be used to translate paths stored with case insensitivity
 *	(such as directory junctions) when no case conflict is expected.
 *	If there some ambiguity, the name which collates first is returned.
 *
 *	The name is converted to upper case and searched the usual way.
 *	The collation rules for file names are such that we should get the
 *	first candidate if any.
 */

static u64 ntfs_fix_file_name(struct ntfs_inode *dir_ni, ntfschar *uname,
		int uname_len)
{
	struct ntfs_volume *vol = dir_ni->vol;
	struct ntfs_index_context *icx;
	u64 mref;
	le64 lemref;
	int lkup;
	int i;
	u32 cpuchar;
	struct INDEX_ENTRY *entry;
	struct FILE_NAME_ATTR *found;
	struct {
		struct FILE_NAME_ATTR attr;
		ntfschar file_name[NTFS_MAX_NAME_LEN + 1];
	} find;

	mref = ERR_MREF(-EIO); /* default return (not found) */
	icx = ntfs_index_ctx_get(dir_ni, NTFS_INDEX_I30, 4);
	if (IS_ERR(icx))
		return ERR_MREF(PTR_ERR(icx));

	if (mutex_lock_interruptible_nested(&dir_ni->ni_lock, NI_MUTEX_NORMAL))
		return ERR_MREF(-ERESTARTSYS);

	if (uname_len > NTFS_MAX_NAME_LEN)
		uname_len = NTFS_MAX_NAME_LEN;
	find.attr.file_name_length = uname_len;
	for (i = 0; i < uname_len; i++) {
		cpuchar = le16_to_cpu(uname[i]);
		/*
		 * We need upper or lower value, whichever is smaller,
		 * but we can only convert to upper case, so we
		 * will fail when searching for an upper case char
		 * whose lower case is smaller (such as umlauted Y)
		 */
		if ((cpuchar < vol->upcase_len)
		    && (le16_to_cpu(vol->upcase[cpuchar]) < cpuchar))
			find.attr.file_name[i] = vol->upcase[cpuchar];
		else
			find.attr.file_name[i] = uname[i];
	}
	lkup = ntfs_index_lookup((char *)&find, uname_len, icx);
	/*
	 * We generally only get the first matching candidate,
	 * so we still have to check whether this is a real match
	 */
	if (icx->entry && (icx->entry->ie_flags & INDEX_ENTRY_END))
			/* get next entry if reaching end of block */
		entry = ntfs_index_next(icx->entry, icx);
	else
		entry = icx->entry;
	if (entry) {
		found = &entry->key.file_name;
		if (lkup
		   && ntfs_names_are_equal(find.attr.file_name,
			find.attr.file_name_length,
			found->file_name, found->file_name_length,
			IGNORE_CASE,
			vol->upcase, vol->upcase_len))
				lkup = 0;
		if (!lkup) {
			/*
			 * name found :
			 *    fix original name and return inode
			 */
			lemref = entry->indexed_file;
			mref = le64_to_cpu(lemref);
			if (NVolCaseSensitive(vol) || !vol->locase) {
				for (i = 0; i < found->file_name_length; i++)
					uname[i] = found->file_name[i];
			} else {
				for (i = 0; i < found->file_name_length; i++)
					uname[i] = vol->locase[le16_to_cpu
						    (found->file_name[i])];
			}
		}
	}
	mutex_unlock(&dir_ni->ni_lock);
	ntfs_index_ctx_put(icx);

	return mref;
}

/*
 *		Search for a directory junction or a symbolic link
 *	along the target path, with target defined as a full absolute path
 *
 *	Returns the path translated to a Linux path
 *		or NULL if the path is not valid
 */

static char *search_absolute(struct ntfs_volume *vol, ntfschar *path,
				int count, bool isdir)
{
	struct ntfs_inode *ni;
	u64 inum;
	char *target;
	int start;
	int len;

	target = (char *)NULL; /* default return */
	ni = ntfs_inode_open(vol, (MFT_REF)FILE_ROOT, NULL);
	if (!IS_ERR(ni)) {
		start = 0;
		/*
		 * Examine and translate the path, until we reach either
		 *  - the end,
		 *  - an unknown item
		 *  - a non-directory
		 *  - another reparse point,
		 * A reparse point is not dereferenced, it will be
		 * examined later when the translated path is dereferenced,
		 * however the final part of the path will not be adjusted
		 * to correct case.
		 */
		do {
			len = 0;
			while (((start + len) < count) && (path[start + len] !=
			const_cpu_to_le16('\\')))
				len++;
			inum = ntfs_fix_file_name(ni, &path[start], len);
			ntfs_inode_close(ni);
			ni = (struct ntfs_inode *)ERR_PTR(-EINVAL);
			if (!IS_ERR_MREF(inum)) {
				inum = MREF(inum);
				ni = ntfs_inode_open(vol, inum, NULL);
				start += len;
				if (start < count)
					path[start++] = const_cpu_to_le16('/');
			}
		} while (!IS_ERR(ni)
				&& (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
				&& !(ni->flags & FILE_ATTR_REPARSE_POINT)
				&& (start < count));
		if (!IS_ERR(ni) && ((ni->mrec->flags & MFT_RECORD_IS_DIRECTORY ?
		isdir : !isdir) || (ni->flags & FILE_ATTR_REPARSE_POINT)))
			if (ntfs_ucstombs(path, count, &target, 0) < 0) {
				if (target) {
					ntfs_free(target);
					target = (char *)NULL;
				}
			}
		ntfs_inode_close(ni);
	}
	return target;
}

/*
 *		Search for a symbolic link along the target path,
 *	with the target defined as a relative path
 *
 *	Note : the path used to access the current inode, may be
 *	different from the one implied in the target definition,
 *	when an inode has names in several directories.
 *
 *	Returns the path translated to a Linux path
 *		or NULL if the path is not valid
 */
static char *search_relative(struct ntfs_inode *ni, ntfschar *path, int count)
{
	char *target = (char *)NULL;
	struct ntfs_inode *curni;
	struct ntfs_inode *newni;
	u64 inum;
	int pos;
	int lth;
	bool ok;
	bool morelinks;
	int max = 32; /* safety */

	pos = 0;
	ok = TRUE;
	morelinks = FALSE;
	curni = ntfs_dir_parent_inode(ni);
		/*
		 * Examine and translate the path, until we reach either
		 *  - the end,
		 *  - an unknown item
		 *  - a non-directory
		 *  - another reparse point,
		 * A reparse point is not dereferenced, it will be
		 * examined later when the translated path is dereferenced,
		 * however the final part of the path will not be adjusted
		 * to correct case.
		 */
	while (!IS_ERR(curni) && ok && !morelinks && (pos < (count - 1))
	       && --max) {
		if ((count >= (pos + 2))
		    && (path[pos] == const_cpu_to_le16('.'))
		    && (path[pos+1] == const_cpu_to_le16('\\'))) {
			path[pos+1] = const_cpu_to_le16('/');
			pos += 2;
		} else {
			if ((count >= (pos + 3))
			&& (path[pos] == const_cpu_to_le16('.'))
			&& (path[pos+1] == const_cpu_to_le16('.'))
			&& (path[pos+2] == const_cpu_to_le16('\\'))) {
				path[pos+2] = const_cpu_to_le16('/');
				pos += 3;
				newni = ntfs_dir_parent_inode(curni);
				if (curni != ni)
					ntfs_inode_close(curni);
				curni = newni;
				if (IS_ERR(curni))
					ok = FALSE;
			} else {
				lth = 0;
				while (((pos + lth) < count) &&
				(path[pos + lth] != const_cpu_to_le16('\\')))
					lth++;
				if (lth > 0)
					inum = ntfs_fix_file_name(curni,
						    &path[pos], lth);
				else
					inum = ERR_MREF(-EIO);
				if (!lth || IS_ERR_MREF(inum)) {
					ok = FALSE;
				} else {
					ntfs_inode_close(curni);
					curni = ntfs_inode_open(ni->vol,
							MREF(inum), NULL);
					if (IS_ERR(curni)) {
						ok = FALSE;
					} else {
						if (curni->flags & FILE_ATTR_REPARSE_POINT)
							morelinks = TRUE;
						if (ok && ((pos + lth) < count)) {
							path[pos + lth] = const_cpu_to_le16('/');
							pos += lth + 1;
						} else {
							pos += lth;
							if (!morelinks
							  && (ni->mrec->flags ^ curni->mrec->flags)
							    & MFT_RECORD_IS_DIRECTORY)
								ok = FALSE;
						}
					}
				}
			}
		}
	}

	ntfs_inode_close(curni);
	if (ok && (ntfs_ucstombs(path, count, &target, 0) < 0)) {
		ntfs_free(target);/*needed?*/
		target = (char *)NULL;
	}
	return target;
}

/*
 *		Check whether a drive letter has been defined in .NTFS-3G
 *
 *	Returns 1 if found,
 *		0 if not found,
 *		-1 if there was an error (described by errno)
 */

/* TODO: CRS: remove this function */
static int ntfs_drive_letter(struct ntfs_volume *vol, ntfschar letter)
{
	char defines[NTFS_MAX_NAME_LEN + 5];
	char *drive = (char *)NULL;
	int ret = -EIO;
	int sz;
	struct ntfs_inode *ni;

	sz = ntfs_ucstombs(&letter, 1, &drive, 0);
	if (sz > 0) {
		strcpy(defines, mappingdir);
		if ((*drive >= 'a') && (*drive <= 'z'))
			*drive += 'A' - 'a';
		strcat(defines, drive);
		strcat(defines, ":");
		ni = ntfs_pathname_to_inode(vol, NULL, defines);
		ntfs_inode_close(ni);
		if (!IS_ERR(ni)) {
			ret = 1;
		} else {
			if (PTR_ERR(ni) == -ENOENT)
				ret = 0;
			else if (IS_ERR(ni))
				ret = PTR_ERR(ni);
		}
	}

	if (drive)
		ntfs_free(drive);

	return ret;
}

/*
 *		Do some sanity checks on reparse data
 *
 *	Microsoft reparse points have an 8-byte header whereas
 *	non-Microsoft reparse points have a 24-byte header.  In each case,
 *	'reparse_data_length' must equal the number of non-header bytes.
 *
 *	If the reparse data looks like a junction point or symbolic
 *	link, more checks can be done.
 *
 */

static bool valid_reparse_data(struct ntfs_inode *ni,
			const struct REPARSE_POINT *reparse_attr, size_t size)
{
	bool ok;
	unsigned int offs;
	unsigned int lth;
	const struct MOUNT_POINT_REPARSE_DATA *mount_point_data;
	const struct SYMLINK_REPARSE_DATA *symlink_data;

	ok = ni && !IS_ERR(reparse_attr)
		&& (size >= sizeof(struct REPARSE_POINT))
		&& (reparse_attr->reparse_tag != IO_REPARSE_TAG_RESERVED_ZERO)
		&& (((size_t)le16_to_cpu(reparse_attr->reparse_data_length)
			 + sizeof(struct REPARSE_POINT)
			 + ((reparse_attr->reparse_tag &
			     IO_REPARSE_TAG_IS_MICROSOFT) ? 0 :
			    sizeof(struct GUID))) == size);
	if (ok) {
		switch (reparse_attr->reparse_tag) {
		case IO_REPARSE_TAG_MOUNT_POINT:
			if (size < sizeof(struct REPARSE_POINT) +
				   sizeof(struct MOUNT_POINT_REPARSE_DATA)) {
				ok = FALSE;
				break;
			}
			mount_point_data =
				(const struct MOUNT_POINT_REPARSE_DATA *)
				reparse_attr->reparse_data;
			offs = le16_to_cpu(mount_point_data->subst_name_offset);
			lth = le16_to_cpu(mount_point_data->subst_name_length);
				/* consistency checks */
			if (!(ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
			    || ((size_t)((sizeof(struct REPARSE_POINT)
				 + sizeof(struct MOUNT_POINT_REPARSE_DATA)
				 + offs + lth)) > size))
				ok = FALSE;
			break;
		case IO_REPARSE_TAG_SYMLINK:
			if (size < sizeof(struct REPARSE_POINT) +
				   sizeof(struct SYMLINK_REPARSE_DATA)) {
				ok = FALSE;
				break;
			}
			symlink_data = (const struct SYMLINK_REPARSE_DATA *)
						reparse_attr->reparse_data;
			offs = le16_to_cpu(symlink_data->subst_name_offset);
			lth = le16_to_cpu(symlink_data->subst_name_length);
			if ((size_t)((sizeof(struct REPARSE_POINT)
				 + sizeof(struct SYMLINK_REPARSE_DATA)
				 + offs + lth)) > size)
				ok = FALSE;
			break;
		default:
			break;
		}
	}

	return ok;
}

/*
 *		Check and translate the target of a junction point or
 *	a full absolute symbolic link.
 *
 *	A full target definition begins with "\??\" or "\\?\"
 *
 *	The fully defined target is redefined as a relative link,
 *		- either to the target if found on the same device.
 *		- or into the /.NTFS-3G directory for the user to define
 *	In the first situation, the target is translated to case-sensitive path.
 *
 *	returns the target converted to a relative symlink
 *		or NULL if there were some problem, as described by errno
 */

static char *ntfs_get_fulllink(struct ntfs_volume *vol, ntfschar *junction,
			int count, const char *mnt_point, bool isdir)
{
	char *target;
	char *fulltarget;
	int sz;
	char *q;
	enum { DIR_JUNCTION, VOL_JUNCTION, NO_JUNCTION } kind;

	target = (char *)NULL;
	fulltarget = (char *)NULL;
			/*
			 * For a valid directory junction we want \??\x:\
			 * where \ is an individual char and x a non-null char
			 */
	if ((count >= 7)
	    && !memcmp(junction, dir_junction_head, 8)
	    && junction[4]
	    && (junction[5] == const_cpu_to_le16(':'))
	    && (junction[6] == const_cpu_to_le16('\\')))
		kind = DIR_JUNCTION;
	else
			/*
			 * For a valid volume junction we want \\?\Volume{
			 * and a final \ (where \ is an individual char)
			 */
		if ((count >= 12)
		    && !memcmp(junction, vol_junction_head, 22)
		    && (junction[count - 1] == const_cpu_to_le16('\\')))
			kind = VOL_JUNCTION;
		else
			kind = NO_JUNCTION;
			/*
			 * Directory junction with an explicit path and
			 * no specific definition for the drive letter :
			 * try to interpret as a target on the same volume
			 */
	if ((kind == DIR_JUNCTION)
	    && junction[7]
	    && !ntfs_drive_letter(vol, junction[4])) {
		target = search_absolute(vol, &junction[7], count - 7, isdir);
		if (target) {
			fulltarget = (char *)ntfs_malloc(strlen(mnt_point)
					+ strlen(target) + 2);
			if (fulltarget) {
				strcpy(fulltarget, mnt_point);
				strcat(fulltarget, target);
			}
		}
	}
			/*
			 * Volume junctions or directory junctions with
			 * target not found on current volume :
			 * link to /.NTFS-3G/target which the user can
			 * define as a symbolic link to the real target
			 */
	if (((kind == DIR_JUNCTION) && !fulltarget)
	    || (kind == VOL_JUNCTION)) {
		sz = ntfs_ucstombs(&junction[4],
			(kind == VOL_JUNCTION ? count - 5 : count - 4),
			&target, 0);
		if ((sz > 0) && target) {
				/* reverse slashes */
			for (q = target; *q; q++)
				if (*q == '\\')
					*q = '/';
				/* force uppercase drive letter */
			if ((target[1] == ':')
			    && (target[0] >= 'a')
			    && (target[0] <= 'z'))
				target[0] += 'A' - 'a';
			fulltarget = (char *)ntfs_malloc(strlen(mnt_point)
				    + sizeof(mappingdir) + strlen(target) + 1);
			if (fulltarget) {
				strcpy(fulltarget, mnt_point);
				strcat(fulltarget, mappingdir);
				strcat(fulltarget, target);
			}
		}
	}
	if (target)
		ntfs_free(target);
	return fulltarget;
}

/*
 *		Check and translate the target of an absolute symbolic link.
 *
 *	An absolute target definition begins with "\" or "x:\"
 *
 *	The absolute target is redefined as a relative link,
 *		- either to the target if found on the same device.
 *		- or into the /.NTFS-3G directory for the user to define
 *	In the first situation, the target is translated to case-sensitive path.
 *
 *	returns the target converted to a relative symlink
 *		or NULL if there were some problem, as described by errno
 */
static char *ntfs_get_abslink(struct ntfs_volume *vol, ntfschar *junction,
			int count, const char *mnt_point, bool isdir)
{
	char *target;
	char *fulltarget;
	int sz;
	char *q;
	enum { FULL_PATH, ABS_PATH, REJECTED_PATH } kind;

	target = (char *)NULL;
	fulltarget = (char *)NULL;
			/*
			 * For a full valid path we want x:\
			 * where \ is an individual char and x a non-null char
			 */
	if ((count >= 3)
	    && junction[0]
	    && (junction[1] == const_cpu_to_le16(':'))
	    && (junction[2] == const_cpu_to_le16('\\')))
		kind = FULL_PATH;
	else
			/*
			 * For an absolute path we want an initial \
			 */
		if ((count >= 0)
		    && (junction[0] == const_cpu_to_le16('\\')))
			kind = ABS_PATH;
		else
			kind = REJECTED_PATH;
			/*
			 * Full path, with a drive letter and
			 * no specific definition for the drive letter :
			 * try to interpret as a target on the same volume.
			 * Do the same for an abs path with no drive letter.
			 */
	if (((kind == FULL_PATH)
	    && (count >= 3)
	    && junction[3]
	    && !ntfs_drive_letter(vol, junction[0]))
	    || (kind == ABS_PATH)) {
		if (kind == ABS_PATH)
			target = search_absolute(vol, &junction[1],
				count - 1, isdir);
		else
			target = search_absolute(vol, &junction[3],
				count - 3, isdir);
		if (target) {
			fulltarget = (char *)ntfs_malloc(strlen(mnt_point)
					+ strlen(target) + 2);
			if (fulltarget) {
				strcpy(fulltarget, mnt_point);
				strcat(fulltarget, target);
			}
		}
	}
			/*
			 * full path with target not found on current volume :
			 * link to /.NTFS-3G/target which the user can
			 * define as a symbolic link to the real target
			 */
	if ((kind == FULL_PATH) && !fulltarget) {
		sz = ntfs_ucstombs(&junction[0], count, &target, 0);
		if ((sz > 0) && target) {
				/* reverse slashes */
			for (q = target; *q; q++)
				if (*q == '\\')
					*q = '/';
				/* force uppercase drive letter */
			if ((target[1] == ':')
			    && (target[0] >= 'a')
			    && (target[0] <= 'z'))
				target[0] += 'A' - 'a';
			fulltarget = (char *)ntfs_malloc(strlen(mnt_point)
				    + sizeof(mappingdir) + strlen(target) + 1);
			if (fulltarget) {
				strcpy(fulltarget, mnt_point);
				strcat(fulltarget, "/");
				strcat(fulltarget, mappingdir);
				strcat(fulltarget, target);
			}
		}
	}
	if (target)
		ntfs_free(target);
	return fulltarget;
}

/*
 *		Check and translate the target of a relative symbolic link.
 *
 *	A relative target definition does not begin with "\"
 *
 *	The original definition of relative target is kept, it is just
 *	translated to a case-sensitive path.
 *
 *	returns the target converted to a relative symlink
 *		or NULL if there were some problem, as described by errno
 */
static char *ntfs_get_rellink(struct ntfs_inode *ni, ntfschar *junction,
			      int count)
{
	return search_relative(ni, junction, count);
}

/*
 *		Get the target for a junction point or symbolic link
 *	Should only be called for files or directories with reparse data
 *
 *	returns the target converted to a relative path, or NULL
 *		if some error occurred, as described by errno
 *		errno is EOPNOTSUPP if the reparse point is not a valid
 *			symbolic link or directory junction
 */
char *ntfs_make_symlink(struct ntfs_inode *ni, const char *mnt_point,
			int *pattr_size)
{
	s64 attr_size = 0;
	char *target;
	unsigned int offs;
	unsigned int lth;
	struct ntfs_volume *vol;
	struct REPARSE_POINT *reparse_attr;
	struct MOUNT_POINT_REPARSE_DATA *mount_point_data;
	struct SYMLINK_REPARSE_DATA *symlink_data;
	enum { FULL_TARGET, ABS_TARGET, REL_TARGET } kind;
	ntfschar *p;
	bool bad;
	bool isdir;

	target = (char *)NULL;
	bad = TRUE;
	isdir = (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
			 != const_cpu_to_le16(0);
	vol = ni->vol;
	reparse_attr = (struct REPARSE_POINT *)ntfs_attr_readall(ni,
			AT_REPARSE_POINT, (ntfschar *)NULL, 0, &attr_size);
	if (!IS_ERR(reparse_attr) && attr_size
			&& valid_reparse_data(ni, reparse_attr, attr_size)) {
		switch (reparse_attr->reparse_tag) {
		case IO_REPARSE_TAG_MOUNT_POINT:
			mount_point_data = (struct MOUNT_POINT_REPARSE_DATA *)
						reparse_attr->reparse_data;
			offs = le16_to_cpu(mount_point_data->subst_name_offset);
			lth = le16_to_cpu(mount_point_data->subst_name_length);
				/* reparse data consistency has been checked */
			target = ntfs_get_fulllink(vol, (ntfschar *)
				    &mount_point_data->path_buffer[offs],
				    lth/2, mnt_point, isdir);
			if (target)
				bad = FALSE;
			break;
		case IO_REPARSE_TAG_SYMLINK:
			symlink_data = (struct SYMLINK_REPARSE_DATA *)
						reparse_attr->reparse_data;
			offs = le16_to_cpu(symlink_data->subst_name_offset);
			lth = le16_to_cpu(symlink_data->subst_name_length);
			p = (ntfschar *)&symlink_data->path_buffer[offs];
				/*
				 * Predetermine the kind of target,
				 * the called function has to make a full check
				 */
			if (*p++ == const_cpu_to_le16('\\')) {
				if ((*p == const_cpu_to_le16('?'))
				    || (*p == const_cpu_to_le16('\\')))
					kind = FULL_TARGET;
				else
					kind = ABS_TARGET;
			} else
				if (*p == const_cpu_to_le16(':'))
					kind = ABS_TARGET;
				else
					kind = REL_TARGET;
			p--;
			/* reparse data consistency has been checked */
			switch (kind) {
			case FULL_TARGET:
				if (!(symlink_data->flags
				   & const_cpu_to_le32(1))) {
					target = ntfs_get_fulllink(vol,
						p, lth/2,
						mnt_point, isdir);
					if (target)
						bad = FALSE;
				}
				break;
			case ABS_TARGET:
				if (symlink_data->flags
				   & const_cpu_to_le32(1)) {
					target = ntfs_get_abslink(vol,
						p, lth/2,
						mnt_point, isdir);
					if (target)
						bad = FALSE;
				}
				break;
			case REL_TARGET:
				if (symlink_data->flags
				   & const_cpu_to_le32(1)) {
					target = ntfs_get_rellink(ni,
						p, lth/2);
					if (target)
						bad = FALSE;
				}
				break;
			}
			break;
		}
		ntfs_free(reparse_attr);
	}
	*pattr_size = attr_size;
	if (bad)
		target = ERR_PTR(-EOPNOTSUPP);

	return target;
}

/*
 *		Check whether a reparse point looks like a junction point
 *	or a symbolic link.
 *	Should only be called for files or directories with reparse data
 *
 *	The validity of the target is not checked.
 */

bool ntfs_possible_symlink(struct ntfs_inode *ni)
{
	s64 attr_size = 0;
	struct REPARSE_POINT *reparse_attr;
	bool possible;

	possible = FALSE;
	reparse_attr = (struct REPARSE_POINT *)ntfs_attr_readall(ni,
			AT_REPARSE_POINT, (ntfschar *)NULL, 0, &attr_size);
	if (reparse_attr && attr_size) {
		switch (reparse_attr->reparse_tag) {
		case IO_REPARSE_TAG_MOUNT_POINT:
		case IO_REPARSE_TAG_SYMLINK:
			possible = TRUE;
		}
		ntfs_free(reparse_attr);
	}
	return possible;
}

#ifdef HAVE_SETXATTR	/* extended attributes interface required */

/*
 *			Set the index for new reparse data
 *
 *	Returns 0 if success
 *		-1 if failure, explained by errno
 */

static int set_reparse_index(struct ntfs_inode *ni,
			     struct ntfs_index_context *xr, le32 reparse_tag)
{
	struct REPARSE_INDEX indx;
	u64 file_id_cpu;
	le64 file_id;
	le16 seqn;

	seqn = ni->mrec->sequence_number;
	file_id_cpu = MK_MREF(ni->mft_no, le16_to_cpu(seqn));
	file_id = cpu_to_le64(file_id_cpu);
	indx.header.data_offset = const_cpu_to_le16(
					sizeof(struct INDEX_ENTRY_HEADER)
					+ sizeof(struct REPARSE_INDEX_KEY));
	indx.header.data_length = const_cpu_to_le16(0);
	indx.header.reservedV = const_cpu_to_le32(0);
	indx.header.length = const_cpu_to_le16(
					sizeof(struct REPARSE_INDEX));
	indx.header.key_length = const_cpu_to_le16(
					sizeof(struct REPARSE_INDEX_KEY));
	indx.header.flags = const_cpu_to_le16(0);
	indx.header.reserved = const_cpu_to_le16(0);
	indx.key.reparse_tag = reparse_tag;
		/* danger on processors which require proper alignment ! */
	memcpy(&indx.key.file_id, &file_id, 8);
	indx.filling = const_cpu_to_le32(0);
	ntfs_index_ctx_reinit(xr);
	return ntfs_ie_add(xr, (struct INDEX_ENTRY *)&indx);
}

#endif /* HAVE_SETXATTR */

/*
 *		Remove a reparse data index entry if attribute present
 *
 *	Returns the size of existing reparse data
 *			(the existing reparse tag is returned)
 *		-1 if failure, explained by errno
 */

static int remove_reparse_index(struct ntfs_attr *na,
				struct ntfs_index_context *xr,
				le32 *preparse_tag)
{
	struct REPARSE_INDEX_KEY key;
	u64 file_id_cpu;
	le64 file_id;
	s64 size;
	le16 seqn;
	int ret;

	ret = na->data_size;
	if (ret) {
			/* read the existing reparse_tag */
		size = ntfs_attr_pread(na, 0, 4, preparse_tag);
		if (size == 4) {
			seqn = na->ni->mrec->sequence_number;
			file_id_cpu = MK_MREF(na->ni->mft_no,
					le16_to_cpu(seqn));
			file_id = cpu_to_le64(file_id_cpu);
			key.reparse_tag = *preparse_tag;
		/* danger on processors which require proper alignment ! */
			memcpy(&key.file_id, &file_id, 8);
			ret = ntfs_index_lookup(&key,
				sizeof(struct REPARSE_INDEX_KEY), xr);
			if (ret)
				return ret;

			ret = ntfs_index_rm(xr);
			if (ret)
				return ret;
		} else {
			ret = -ENODATA;
		}
	}
	return ret;
}

/*
 *		Open the $Extend/$Reparse file and its index
 *
 *	Return the index context if opened
 *		or NULL if an error occurred (errno tells why)
 *
 *	The index has to be freed and inode closed when not needed any more.
 */

static struct ntfs_index_context *open_reparse_index(struct ntfs_volume *vol)
{
	u64 inum;
	struct ntfs_inode *ni;
	struct ntfs_inode *dir_ni;
	struct ntfs_index_context *xr;
	int err;

		/* do not use path_name_to inode - could reopen root */
	dir_ni = ntfs_inode_open(vol, FILE_EXTEND, NULL);
	ni = NULL;
	if (!IS_ERR(dir_ni)) {
		err = ntfs_inode_lookup_by_mbsname(dir_ni, "$Reparse", &inum);
		if (!err)
			ni = ntfs_inode_open(vol, inum, NULL);
		else
			ni = ERR_PTR(err);
		ntfs_inode_close(dir_ni);
	}
	if (!IS_ERR(ni)) {
		xr = ntfs_index_ctx_get(ni, reparse_index_name, 2);
		if (IS_ERR(xr))
			ntfs_inode_close(ni);
	} else {
		xr = (struct ntfs_index_context *)ni;
	}

	return xr;
}

#ifdef HAVE_SETXATTR	/* extended attributes interface required */

/*
 *		Update the reparse data and index
 *
 *	The reparse data attribute should have been created, and
 *	an existing index is expected if there is an existing value.
 *
 *	Returns 0 if success
 *		-1 if failure, explained by errno
 *	If could not remove the existing index, nothing is done,
 *	If could not write the new data, no index entry is inserted
 *	If failed to insert the index, data is removed
 */

static int update_reparse_data(struct ntfs_inode *ni,
			       struct ntfs_index_context *xr,
			       const char *value, size_t size)
{
	int res;
	int written;
	int oldsize;
	struct ntfs_attr *na;
	le32 reparse_tag;

	res = 0;
	na = ntfs_attr_open(ni, AT_REPARSE_POINT, AT_UNNAMED, 0);
	if (!IS_ERR(na)) {
			/* remove the existing reparse data */
		oldsize = remove_reparse_index(na, xr, &reparse_tag);
		if (oldsize < 0)
			res = oldsize;
		else {
			/* resize attribute */
			res = ntfs_attr_truncate(na, (s64)size);
			/* overwrite value if any */
			if (!res && value) {
				written = (int)ntfs_attr_pwrite(na,
						 (s64)0, (s64)size, value);
				if (written != (s64)size) {
					ntfs_log_error("Failed to update "
						"reparse data");
					res = -EIO;
				}
			}
			if (!res) {
				res = set_reparse_index(ni, xr,
					((const struct REPARSE_POINT *)
					 value)->reparse_tag);
				if (res && (oldsize > 0)) {
					/*
					 * If cannot index, try to remove the
					 * reparse data and log the error. There
					 * will be an inconsistency if removal
					 * fails.
					 */
					ntfs_attr_rm(na);
					ntfs_log_error("Failed to index reparse"
						       " data. Possible "
						       "corruption.");
				}
			}
		}
		ntfs_attr_close(na);
		NInoSetDirty(ni);
	} else
		res = PTR_ERR(na);

	return res;
}

#endif /* HAVE_SETXATTR */

/*
 *		Delete a reparse index entry
 *
 *	Returns 0 if success
 *		-1 if failure, explained by errno
 */

int ntfs_delete_reparse_index(struct ntfs_inode *ni)
{
	struct ntfs_index_context *xr;
	struct ntfs_inode *xrni;
	struct ntfs_attr *na = ntfs_attr_open(ni, AT_REPARSE_POINT,
					      AT_UNNAMED, 0);
	le32 reparse_tag;
	int res = 0;

	if (!IS_ERR(na)) {
		/*
		 * read the existing reparse data (the tag is enough)
		 * and un-index it
		 */
		xr = open_reparse_index(ni->vol);
		if (!IS_ERR(xr)) {
			res = remove_reparse_index(na, xr, &reparse_tag);
			xrni = xr->ni;
			ntfs_index_entry_mark_dirty(xr);
			NInoSetDirty(xrni);
			ntfs_index_ctx_put(xr);
			ntfs_inode_close(xrni);
		} else {
			res = PTR_ERR(xr);
		}
		ntfs_attr_close(na);
	} else {
		res = PTR_ERR(na);
	}

	return res;
}


/*
 *		Get the ntfs reparse data into an extended attribute
 *
 *	Returns the reparse data size
 *		and the buffer is updated if it is long enough
 */

int ntfs_get_ntfs_reparse_data(struct ntfs_inode *ni, char *value, size_t size)
{
	struct REPARSE_POINT *reparse_attr;
	s64 attr_size;
	int err = -EIO;

	attr_size = 0;	/* default to no data and no error */
	if (ni) {
		if (ni->flags & FILE_ATTR_REPARSE_POINT) {
			reparse_attr =
			    (struct REPARSE_POINT *) ntfs_attr_readall(ni,
			    AT_REPARSE_POINT, (ntfschar *)NULL, 0, &attr_size);
			if (reparse_attr) {
				if (attr_size <= (s64)size) {
					if (value)
						memcpy(value, reparse_attr,
							attr_size);
					else
						err = -EINVAL;
				}
				ntfs_free(reparse_attr);
			}
		} else
			err = -ENODATA;
	}
	return attr_size ? (int)attr_size : err;
}

