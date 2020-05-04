/**
 * dir.c - NTFS Driver by AVM GmbH (ANTFS)
 *         Based on ntfs-3g
 *
 * Copyright (c) 2016 Martin Pommerenke, Jens Krieg, Arwed Meyer,
 *		      Christian René Sechting
 *
 * This file is originated from the Linux-NTFS project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "antfs.h"

#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/slab.h>

#include "misc.h"
#include "dir.h"
#include "reparse.h"

/**
 * Check if we are in the /FRITZ directory.
 *
 * @entry   dentry to check
 *
 * Returns @TRUE if the dentry is inside a folder named "FRITZ" in the disks
 * root.
 */
static bool in_fritz_dir(struct dentry *entry)
{
	struct dentry *tmp_entry = entry;
	struct dentry *prev_entry = NULL;

	/* walk up */
	while (tmp_entry->d_parent && tmp_entry->d_parent != tmp_entry) {
		prev_entry = tmp_entry;
		tmp_entry = tmp_entry->d_parent;
	}

	/* FRITZ dir? */
	return prev_entry && !strcmp(prev_entry->d_name.name, "FRITZ");
}

/**
 * create a link from @entry to the filesystem root
 *
 * @entry      dentry to check
 * @mnt_point  result string
 * @size       size of @mnt_point
 *
 * returns 0 on success and -EINVAL if @mnt_point is to small
 */
static int antfs_root_path(struct dentry *entry, char *mnt_point,
				  size_t size)
{
	struct dentry *tmp_entry = entry->d_parent;

	/* walk up */
	while (tmp_entry->d_parent && tmp_entry->d_parent != tmp_entry) {
		tmp_entry = tmp_entry->d_parent;
		/* check if we still have enough space */
		if (strlen(mnt_point) >= size - 4)
			return -EINVAL;
		strlcat(mnt_point, "../", size);
	}

	return 0;
}

static inline void antfs_inode_update_times(struct inode *inode,
					    enum ntfs_time_update_flags mask)
{
	if (mask & NTFS_UPDATE_ATIME)
		inode->i_atime = current_time(inode);

	if (mask & NTFS_UPDATE_MTIME)
		inode->i_mtime = current_time(inode);

	if (mask & NTFS_UPDATE_CTIME)
		inode->i_ctime = current_time(inode);
}

/**
 *  @brief  antfs_lookup checks if an entry is kept inside a directory.
 *
 *  @param dir	    the directory's inode we are looking in for the entry
 *  @param entry    the entry we are looking for
 *  @param flags    TODO: idk what these are for even ext2 doesnt use them...
 *
 *  @return the dentry we looked for which is now connected to its corresponding
 *	    vfs inode if everything worked out. Errorcode pointer if something
 *	    went wrong.
 *
 *  antfs_lookup gets called by the vfs to find an entry inside a given
 *  directory. It finds the corresponding ntfs_inode to the entry in question,
 *  and if it is an actual file/directory and not the root node  creates the new
 *  vfs inode. The dentry gets connected to the newly created vfs inode in form
 *  of a new dentry which the argument dentry should point to and gets returned.
 *  In case of an error during the connection of inode and dentry the error code
 *  gets returned for the caller to handle. If the name of the entry to look for
 *  exceeds 1024 characters it returns the -ENAMETOOLONG error.
 *  In case the entry in question is not existent in the given directory the
 *  argument pointer to the entry to look for stays untouched and NULL gets
 *  returned. In case of a ls call the returned NULL pointer will signalize that
 *  there is no entry with that name, but in case of an mkdir call the untouched
 *  argument pointer combined with the returned NULL pointer signalises that a
 *  new file/directory with that name we were looking for can be created.
 *  We need to feed the ntfs inode to the cache in case we just newly created
 *  it. Since we can't be sure about that antfs_check_feed_cache() is used.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0)
static struct dentry *antfs_lookup(struct inode *dir, struct dentry *entry,
				   struct nameidata *nd)
#else
static struct dentry *antfs_lookup(struct inode *dir, struct dentry *entry,
				   unsigned int flags __attribute__((unused)))
#endif
{
	struct dentry *newent;
	struct inode *inode = NULL;
	struct ntfs_inode *dir_ni = ANTFS_NI(dir);
	struct antfs_sb_info *sbi = ANTFS_SB(dir->i_sb);
	struct ntfs_inode *ni;
	struct FILE_NAME_ATTR *fn;
	int err;
	ntfschar *unicode = NULL;
	u64 inum;

	antfs_log_enter("dir ino: %lu; dentry: %s", (unsigned long)dir->i_ino,
			entry->d_name.name);

	if (entry->d_name.len > 1024) {
		err = -ENAMETOOLONG;
		goto out_err;
	}

	if (mutex_lock_interruptible_nested(&dir_ni->ni_lock,
				NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out_err;
	}

	err = ntfs_mbstoucs(entry->d_name.name, &unicode);
	if (err < 0) {
		antfs_log_error("Could not convert filename to Unicode:"
				" '%s'", entry->d_name.name);
		goto out_err_locked;
	}
	if (err > NTFS_MAX_NAME_LEN) {
		ntfs_free(unicode);
		err = -ENAMETOOLONG;
		goto out_err_locked;
	}
	err = ntfs_inode_lookup_by_name(dir_ni, unicode, err, &inum, &fn);
	ntfs_free(unicode);
	if (err) {
		antfs_log_debug("Couldn't find name '%s'.", entry->d_name.name);
		if (err != -ENOENT)
			goto out_err_locked;
		mutex_unlock(&dir_ni->ni_lock);
	} else {
		inum = MREF(inum);
		ni = ntfs_inode_open(sbi->vol, inum, fn);
		if (IS_ERR(ni)) {
			err = PTR_ERR(ni);
			/* -ENOENT or -EIO should hint to permanent errors
			 * on disk. -EIO could also mean we cannot access
			 * physical media, but in this case there isn't
			 * likely anything left we could damage.
			 *
			 * To be even more cautious, only delete stuff in
			 * FRITZ directory.
			 */
			if ((err == -ENOENT || err == -EIO) &&
					in_fritz_dir(entry)) {
				antfs_log_error("Cannot open inode %llu, "
						"err %d. Deleting dentry.",
						(unsigned long long)inum, err);
				/* We have an orphaned dentry.
				 * Kill it with fire!
				 */
				err = ntfs_index_remove(dir_ni, NULL, fn,
						(fn->file_name_length * 2) +
						sizeof(*fn));
				ntfs_inode_sync(dir_ni);
				mutex_unlock(&dir_ni->ni_lock);
				ntfs_free(fn);
				if (err)
					antfs_log_error("Removing orphaned "
							"index failed with %d",
							err);
			} else {
				mutex_unlock(&dir_ni->ni_lock);
				ntfs_free(fn);
			}
		} else {
			mutex_unlock(&dir_ni->ni_lock);
			ntfs_free(fn);
			if (ni->mft_no == FILE_ROOT) {
				err = -EINVAL;
				goto out_err_free_ni;
			}

			inode = ANTFS_I(ni);
		}
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
	newent = d_materialise_unique(entry, inode);
#else
	newent = d_splice_alias(inode, entry);
#endif

	if (IS_ERR(newent)) {
		err = PTR_ERR(newent);
		goto out_err;
	}
	antfs_log_leave("ok");
	return newent;

out_err_locked:
	mutex_unlock(&dir_ni->ni_lock);
out_err:
	antfs_log_leave("err: %d", err);
	return ERR_PTR(err);

out_err_free_ni:
	ntfs_inode_close(ni);
	goto out_err;
}

/**
 * @brief creates a new file/symlink/directory
 *
 * @param dir	parent directory to create the new file/symlink/directory in
 * @param entry	entry for the new file/symlink/directory
 * @param mode	i_mode that should be set for the new file/symlink/directory
 *
 * @return 0 if everything is ok, error code otherwise
 *
 * antfs_create is creating a new file/symlink/directory on the ntfs device
 * @dir resides on. The creation happens in two steps, first the new file is
 * created on the ntfs device and stored there. For that the name has to be
 * transformed into an unicode string. After that, the ntfs inode and its
 * corresponding vfs inode will be allocated and updated. After all zero will
 * be returned. This function is called by antfs_create to create regular files
 * and from antfs_mkdir to create directories.
 */
static int antfs_create_i(struct inode *dir, struct dentry *entry, int mode)
{
	struct ntfs_inode *dir_ni = ANTFS_NI(dir);
	struct inode *inode = NULL;
	struct ntfs_inode *ni;
	ntfschar *uname = NULL;
	le32 securid = 0;
	int uname_len;
	int err = 0;

	antfs_log_enter("%s", entry->d_name.name);

	if (dir_ni->vol->free_clusters - 1 <
	    antfs_reserved_clusters(dir_ni->vol)) {
		err = -ENOSPC;
		goto out;
	}


	/* - check if the file already exists or no creation allowed - */
	if (entry->d_inode) {
		err = -EEXIST;
		goto out;
	}

	if (dir_ni->mft_no == FILE_EXTEND) {
		antfs_log_debug("Deny creating files in $Extend");
		err = -EPERM;
		goto out;
	}

	/* - we are going to create a new file - */
	uname_len = ntfs_mbstoucs(entry->d_name.name, &uname);
	if (uname_len < 0) {
		antfs_log_error("Could not convert name to ucs");
		err = uname_len;
		goto out;
	}

	if (mutex_lock_interruptible_nested(&dir_ni->ni_lock,
				NI_MUTEX_PARENT)) {
		err = -ERESTARTSYS;
		goto out;
	}
	ni = ntfs_create(dir_ni, securid, uname, uname_len, mode & S_IFMT);
	mutex_unlock(&dir_ni->ni_lock);
	if (IS_ERR(ni)) {
		err = PTR_ERR(ni);
		antfs_log_info("Could not acquire ni: err=%d", err);
		goto free_name;
	}

	inode = ANTFS_I(ni);
	ni->flags |= FILE_ATTR_ARCHIVE;

	/* - write dir back to disk - */

	err = antfs_inode_init(&inode, ANTFS_INODE_INIT_DELETE);
	/* inode_init does not discard collided inodes here so we can
	 * unlink below.
	 */
	if (err) {
		antfs_log_error("Could not fetch VFS inode!");
		/* - we have to rewind the allocated ni - */
		if (mutex_lock_interruptible_nested(&ni->ni_lock,
					NI_MUTEX_NORMAL)) {
			err = -ERESTARTSYS;
			goto free_name;
		}
		if (mutex_lock_interruptible_nested(&dir_ni->ni_lock,
					NI_MUTEX_PARENT)) {
			mutex_unlock(&ni->ni_lock);
			err = -ERESTARTSYS;
			goto free_name;
		}
		ntfs_unlink(dir_ni->vol, ni, dir_ni, uname, uname_len);
		mutex_unlock(&dir_ni->ni_lock);
		mutex_unlock(&ni->ni_lock);

		clear_nlink(inode);
		iget_failed(inode);

		goto free_name;
	} else {
		/* the index_root attribute changed it's data_size, we have to
		 * pass that to the vfs inode's i_size
		 */
		i_size_write(dir, ANTFS_NA(dir_ni)->data_size);
		antfs_inode_update_times(dir, NTFS_UPDATE_MCTIME);
		ntfs_inode_mark_dirty(ni);
		ntfs_inode_mark_dirty(dir_ni);
		mark_inode_dirty(inode);
		mark_inode_dirty(dir);
	}

	/* - finish vfs inode! even if we fail to open,
	 * the file exists now -
	 */
	d_instantiate(entry, inode);

free_name:
	ntfs_free(uname);
out:
	antfs_log_leave("inode: %ld; err: %d", inode ? inode->i_ino : -1, err);
	return err;

}

/**
 * @brief creates a new file/symlink/directory
 *
 * @param dir	parent directory to create the new file in
 * @param entry	entry for the new file
 * @param mode	i_mode that should be set for the new file
 * @param excl	TODO
 *
 * @return 0 if everything is ok, error code otherwise
 *
 * antfs_create is creating a new regular file on the ntfs device
 * @dir resides on. The creation happens in two steps, first the new file is
 * created on the ntfs device and stored there. For that the name has to be
 * transformed into an unicode string. After that, the ntfs inode and its
 * corresponding vfs inode will be allocated and updated. After all zero will
 * be returned.
 */
#if KERNEL_VERSION(3, 3, 0) > LINUX_VERSION_CODE
static int antfs_create(struct inode *dir, struct dentry *entry, int mode,
			struct nameidata *nd __attribute__ ((unused)))
#elif KERNEL_VERSION(3, 6, 0) > LINUX_VERSION_CODE
static int antfs_create(struct inode *dir, struct dentry *entry, umode_t mode,
			struct nameidata *nd __attribute__ ((unused)))
#else
static int antfs_create(struct inode *dir, struct dentry *entry, umode_t mode,
			bool excl __attribute__((unused)))
#endif
{
	return antfs_create_i(dir, entry, mode | S_IFREG);
}

/**
 *  @brief creates a new directory.
 *
 *  @param dir	    parent directory we want to create a new directory in
 *  @param entry    the dentry for the new directory to create
 *  @mode	    the mode in which the new directory can be accessed
 *
 *  @return 0 if everything worked out and the new directory is created.
 *	    error codes in case something went wrong.
 *
 *  After the vfs checked if the name for the new directory is not already in
 *  use antfs_mkdir gets called to actually create a vfs inode and its
 *  corresponding ntfs_inode which will be written onto the ntfs device. For
 *  that, the name provided by the entry has to be converted into a unicode
 *  string since thats the way ntfs stores file names.
 *  Access permissions need to be set up according to the ntfs standard as a
 *  le32 integer and ntfs_create() gets called with the parent directory's
 *  ntfs_inode, the access permissions, name, name length and directory type.
 *  If everything worked out the ntfs_inode's flags need to be set with the
 *  FILE_ATTR_ARCHIVE flag, the atime, mtime and ctime need to be set for
 *  both parent and new directory.
 *  We have to push the new ntfs inode onto the ntfs inode cache. This is
 *  important because ntfs_inode_sync() tries to open the parent directory of
 *  the file to sync. A new ntfs inode would be allocated if there is no entry
 *  for that inode in the cache. That would lead to inconsistent ntfs inodes
 *  since we always keep an open ntfs inode for every vfs inode. The result
 *  would be a corrupted filesystem. Therefore we need to use the same ntfs
 *  inode for the same mft_no at all times! Even tho that means that some other
 *  process could work on the same inode at the same time.
 *  TODO CRS: change description... too much has changed!
 *  Creating the vfs inode seems trivial because the corresponding vfs inode
 *  will always have the same i_ino as the ntfs inode's mft_no. So if we find
 *  a ntfs inode, we will have a free vfs inode for that. But in rare instances
 *  we could allocate a just released ntfs inode which still holds a vfs inode
 *  which needs to be released as well. This case is handled in
 *  antfs_inode_init(). But in case we can't resolve that problem here, we have
 *  to rewind the changes we made and wrote back on the disk. This should only
 *  happen with a very high load so that the process removing the vfs inode gets
 *  stalled for a longer time. We should not keep our lock forever, so we gently
 *  fail antfs_mkdir.
 *  Lastly the ntfs inode gets written back to the disk and the vfs inode gets
 *  connected to the dentry.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
static int antfs_mkdir(struct inode *dir, struct dentry *entry, int mode)
#else
static int antfs_mkdir(struct inode *dir, struct dentry *entry, umode_t mode)
#endif
{
	return antfs_create_i(dir, entry, mode | S_IFDIR);
}

/**
 * @brief removes a file
 *
 * @param dir	    parent directory of the file to delete
 * @param entry	    corresponding dentry to the file to delete
 *
 * @return 0 if @entry was successfully deleted, error code otherwise
 *
 * antfs_unlinnk deletes a file from the ntfs device. To use the ntfs-3g lib
 * function ntfs_delete() we need to translate the name of the directory into an
 * unicode string. Before we can remove the file from the disk we have to remove
 * the cache entry. The removal of the file from the ntfs device is then
 * done by ntfs_delete().
 */
static int antfs_unlink(struct inode *dir, struct dentry *entry)
{
	struct inode *inode = entry->d_inode;
	struct antfs_sb_info *sbi = ANTFS_SB(dir->i_sb);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_inode *dir_ni;
	ntfschar *uname = NULL;
	int uname_len;
	int err = 0;

	antfs_log_enter();
	if (ni->mft_no < FILE_FIRST_USER) {
		err = -EPERM;
		antfs_log_error("Wrong arguments");
		goto out;
	}

	dir_ni = ANTFS_NI(dir);
	if (dir_ni->mft_no == FILE_EXTEND) {
		err = -EPERM;
		antfs_log_debug("deny unlinking metadata files from $Extend");
		goto out;
	}

	uname_len = ntfs_mbstoucs(entry->d_name.name, &uname);
	if (uname_len < 0) {
		antfs_log_error("Could not convert ucs to char *");
		err = uname_len;
		goto out;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}
	if (mutex_lock_interruptible_nested(&dir_ni->ni_lock,
				NI_MUTEX_PARENT)) {
		mutex_unlock(&ni->ni_lock);
		err = -ERESTARTSYS;
		goto out;
	}

	/* - unlink inode on disk - */
	err = ntfs_unlink(sbi->vol, ni, dir_ni, uname, uname_len);
	mutex_unlock(&dir_ni->ni_lock);
	mutex_unlock(&ni->ni_lock);
	if (err) {
		antfs_log_error("Something went wrong while deleting: %d", err);
		goto free_name;
	}

	/* - prepare nii for the vfs inode's destruction and sync dir_ni - */

	/* the index_root attribute changed it's data_size, we have to
	 * pass that to the vfs inode's i_size
	 */
	i_size_write(dir, ANTFS_NA(dir_ni)->data_size);
	antfs_inode_update_times(dir, NTFS_UPDATE_MCTIME);
	ntfs_inode_mark_dirty(dir_ni);
	mark_inode_dirty(dir);
	if (unlikely(ni->mrec->link_count))
		ntfs_inode_mark_dirty(ni);
	inode->i_ctime = dir->i_ctime;
	/* decrementing the linkcount marks the inode dirty */
	inode_dec_link_count(inode);

free_name:
	ntfs_free(uname);
out:
	return err;
}

/**
 * @brief removes a directory
 *
 * @param dir	    parent directory of the directory to delete
 * @param entry	    corresponding dentry to the directory to delete
 *
 * @return 0 if @entry was successfully deleted, error code otherwise
 *
 * antfs_rmdir deletes a directory from the ntfs device. To use the ntfs-3g
 * lib function ntfs_delete() we need to translate the name of the directory
 * into an unicode string. Before the ntfs inode can be deleted off the disk,
 * we have to remove the cache entry of that ntfs inode. Removing the
 * directory from the ntfs device is then done by ntfs_delete().
 */
static int antfs_rmdir(struct inode *dir, struct dentry *entry)
{
	struct inode *inode = entry->d_inode;
	struct antfs_sb_info *sbi = ANTFS_SB(dir->i_sb);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_inode *dir_ni;
	ntfschar *uname = NULL;
	int err = 0, uname_len;

	antfs_log_enter("%s (%llu) from (%lu)", entry->d_name.name, ni->mft_no,
			dir->i_ino);
	/* - never remove meta data files - */
	if (ni->mft_no < FILE_FIRST_USER) {
		err = -EPERM;
		goto out;
	}

	dir_ni = ANTFS_NI(dir);

	/* - don't try to delete a directory which is not empty - */
	err = ntfs_check_empty_dir(ni);
	if (err) {
		/* TODO: This happens in antfs_mv.sh
		 *   1. Why does this happen in 1st test cycle:
		 *      What is in index root?
		 *   2. Make this a warning. */
		antfs_log_debug("Directory is not empty. err:%d", err);
		goto out;
	}

	uname_len = ntfs_mbstoucs(entry->d_name.name, &uname);
	if (uname_len < 0) {
		antfs_log_error("Couldn't convert ucs to char *");
		err = uname_len;
		goto out;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}
	if (mutex_lock_interruptible_nested(&dir_ni->ni_lock,
				NI_MUTEX_PARENT)) {
		mutex_unlock(&ni->ni_lock);
		err = -ERESTARTSYS;
		goto out;
	}
	/* remove the cache entry just before ntfs_delete() so we can be sure
	 * that we will at leat try to remove it.
	 */
	err = ntfs_unlink(sbi->vol, ni, dir_ni, uname, uname_len);
	mutex_unlock(&dir_ni->ni_lock);
	mutex_unlock(&ni->ni_lock);
	if (err) {
		antfs_log_error("Could not unlink (%llu)", ni->mft_no);
		goto free_name;
	}

	/* the index_root attribute changed it's data_size, we have to
	 * pass that to the vfs inode's i_size
	 */
	i_size_write(dir, ANTFS_NA(dir_ni)->data_size);
	antfs_inode_update_times(dir, NTFS_UPDATE_MCTIME);
	ntfs_inode_mark_dirty(dir_ni);
	mark_inode_dirty(dir);
	if (unlikely(ni->mrec->link_count))
		ntfs_inode_mark_dirty(ni);
	/* - notify vfs about deletion. - */
	inode_dec_link_count(inode);
	if (inode->i_nlink)
		antfs_log_error("i_nlink not 0!");
free_name:
	ntfs_free(uname);
out:
	antfs_log_leave();
	return err;
}

/**
 * @brief renames a file and moves it to a new location if needed.
 *
 * @param olddir    inode of the old parent directory
 * @param oldent    old dentry of the file to rename
 * @param newdir    inode of the new parent directory
 * @param newent    new dentry of the file with new name already set
 *
 * @return 0 if name is changed, error code otherwise
 *
 * antfs_rename is changing the name of a file/directory. If needed
 * the file will be relocated to a new parent directory. Any metadata file
 * should not be renamed. If the new entry already has a ntfs inode on the
 * device, then we error out since this shouldn't be the case.
 * The renaming process is very simple, first we create a new link from the
 * new parent directory to the file. Since the file's inode doesn't have a
 * name we can just link the existing inode now with a new name. Then only
 * the modify and change times need to be adjusted from the new parent dir
 * and the change time of the file. The file will have now a new link with
 * the desired name and location. At this point @newdir's ntfs inode needs
 * to be written back to the disk. Otherwise we would corrupt the inode's
 * index header if we would continue without synchronizing.
 * The second step is to actually remove the old link to pretent that we
 * actually renamed the file, even though in fact we never renamed anything
 * but created a new link with the desired name. ntfs_delete() is used to
 * remove the unwanted link. We need to synchronize @olddir's und @newent's
 * ntfs inode's after ntfs_delete().
 * NOTE: that we don't need to remove the cache entry here since we didn't
 * completely deleted that file/directory but just remove the link with the
 * old name. ntfs_delete() won't go to the point of invalidating the cache
 * as long as the link count is greater than 0.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
static int antfs_rename(struct inode *olddir, struct dentry *oldent,
			struct inode *newdir, struct dentry *newent,
			unsigned int flags)
#else
static int antfs_rename(struct inode *olddir, struct dentry *oldent,
			struct inode *newdir, struct dentry *newent)
#endif
{
	struct antfs_sb_info *sbi = ANTFS_SB(olddir->i_sb);
	struct ntfs_inode *ni, *dir_ni, *check_ni, *old_dir_ni;
	ntfschar *uname = NULL, *old_uname = NULL;
	int uname_len;
	int err = 0;
	bool ni_locked = false, dir_ni_locked = false,
	     old_dir_ni_locked = false;

	/* ni is the ntfs inode of the directory that will be renamed
	 * dir_ni is the ntfs inode of the new parent, which might be the
	 * same as the old parent.
	 */
	ni = ANTFS_NI(oldent->d_inode);
	dir_ni = ANTFS_NI(newdir);
	old_dir_ni = ANTFS_NI(olddir);

	antfs_log_enter("%p(%llu) from %p(%lu) to %p(%lu)", ni, ni->mft_no,
			dir_ni, olddir->i_ino, ANTFS_NI(newdir), newdir->i_ino);
	antfs_log_debug("%s -> %s", oldent->d_name.name, newent->d_name.name);

	/* we don't unlink metadata files. Here is the right point of time to
	 * check that. We also don't want to proceed if our input is broken!
	 * Either way our arguments are invalid: return EINVAL
	 */
	if (IS_ERR_OR_NULL(ni) || IS_ERR_OR_NULL(dir_ni)
	    || ni->mft_no < FILE_FIRST_USER || (ni->mft_no == dir_ni->mft_no)) {
		antfs_log_error("Invalid parameters");
		err = -EINVAL;
		goto out;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}
	ni_locked = true;
	if (mutex_lock_interruptible_nested(&dir_ni->ni_lock,
				NI_MUTEX_PARENT)) {
		err = -ERESTARTSYS;
		goto out;
	}
	dir_ni_locked = true;
	if (dir_ni != old_dir_ni) {
		if (mutex_lock_interruptible_nested(&old_dir_ni->ni_lock,
					NI_MUTEX_PARENT2)) {
			err = -ERESTARTSYS;
			goto out;
		}
		old_dir_ni_locked = true;
	}


	/* - check if the newent already has an existing ni - */
	/* TODO: CRS: that should be possible without ntfs_pathname_to_inode */
	check_ni = ntfs_pathname_to_inode(sbi->vol, dir_ni,
					  newent->d_name.name);
	if (!IS_ERR(check_ni)) {
		/* we could try to do the operation if we are a directory and
		 * the target is an empty directory, for that we have to:
		 */
		/*TODO: check if empty, we dont do stuff with not empty check_ni
		 *TODO: check if newent ni and oldent ni are the same */
		/* for now we dont do anything here */
		err = -EEXIST;
		/* check_ni was opened to verify that there is no record with
		 * that name already present. In case we actually had a record
		 * with that name, we now opened it, and it has to be closed
		 * in order not to leak memory since we lose any reference to
		 * this check_ni past this function
		 */
		iput(ANTFS_I(check_ni));
		goto out;
	}
	/* - link the old directory to the new name - */
	uname_len = ntfs_mbstoucs(newent->d_name.name, &uname);
	if (uname_len < 0) {
		err = uname_len;
		goto out;
	}

	/* we want the parent directory of @newent to link towards the @oldent
	 * with @newent's ucs name. After this step @newent will appear in
	 * @newdir as a subdirectory pointing to the content of @oldent.
	 */
	err = ntfs_link(ni, dir_ni, uname, uname_len);
	if (err) {
		antfs_log_error("Couldn't link new->parent to ino!");
		goto free_name;
	}

	/* - update ni and dir_ni and sync it back to disk - */
	ni->flags |= FILE_ATTR_ARCHIVE;

	/* - unlink the old entry - */
	uname_len = ntfs_mbstoucs(oldent->d_name.name, &old_uname);
	if (uname_len < 0) {
		err = uname_len;
		goto free_name;
	}
	/* Just unlink. Link counter is still at least 1 after this. */
	err = ntfs_unlink(sbi->vol, ni, old_dir_ni, old_uname, uname_len);
	if (err) {
		antfs_log_error("During unlink of oldent: %d", err);
		goto free_names;
	}

	/* the index_root attribute changed it's data_size, we have to
	 * pass that to the vfs inode's i_size
	 */
	i_size_write(newdir, ANTFS_NA(dir_ni)->data_size);
	i_size_write(olddir, ANTFS_NA(old_dir_ni)->data_size);
	antfs_inode_update_times(olddir, NTFS_UPDATE_MCTIME);
	antfs_inode_update_times(newdir, NTFS_UPDATE_MCTIME);
	antfs_inode_update_times(oldent->d_inode, NTFS_UPDATE_CTIME);
	ntfs_inode_mark_dirty(ni);
	ntfs_inode_mark_dirty(dir_ni);
	if (dir_ni != old_dir_ni)
		ntfs_inode_mark_dirty(old_dir_ni);
	mark_inode_dirty(oldent->d_inode);
	mark_inode_dirty(newdir);
	if (newdir != olddir)
		mark_inode_dirty(olddir);
	err = 0;

free_names:
	ntfs_free(old_uname);
free_name:
	ntfs_free(uname);
out:
	if (old_dir_ni_locked)
		mutex_unlock(&old_dir_ni->ni_lock);
	if (dir_ni_locked)
		mutex_unlock(&dir_ni->ni_lock);
	if (ni_locked)
		mutex_unlock(&ni->ni_lock);
	return err;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
/**
 * @brief fills in a dirent to create a dentry from the information.
 *
 * @param buf	    pointer to struct antfs_filler
 * @param name	    unicode string name of entry
 * @param name_len  length of name
 * @param name_type type of the name, see enum FILE_NAME_TYPE_FLAGS
 * @param pos	    position used by ntfs_readdir()
 * @param mref	    position in the mft table of the entry
 * @param type	    type of the entry (directory, symlink, regular file etc.)
 *
 * @return 0 if we filled in a dirent, -ENOMEM if we were not able too.
 *
 * antfs_dir_read uses the filldir() function that is provided by the vfs
 * to write dirents into a buffer that will be used to create dentries from
 * that. filldir() takes a regular char string instead of a uc string as a
 * parameter so the name needs to be converted into a regular string. All
 * other paramters can be just taken as is to use filldir().
 * The antfs_filler struct is used to keep track of the progress of reading
 * entries from the disk. It keeps the file, so that the f_pos counter gets
 * incremented for every dirent that gets read from the disk. In case of too
 * many entries in the directory the provided buffer will be filled before
 * done reading the entries. The vfs will call readdir() as long as we will
 * fill in more entries.
 * For not adding duplicates we need to check if the current entry is already
 * filled in, in which case @pos would equal the filler->file->f_pos and is not
 * the root entry '.' with @pos = 0. This could mean two things: this is the
 * last entry that was added before we run out of memory to fill in more
 * entries, or we read all entries already. In both cases we just ignore the
 * current entry, ntfs_readdir() will either way call us with more entries,
 * or return to the caller antfs_readdir().
 */
static int antfs_dir_read(void *buf, ntfschar *name, int name_len,
			  int name_type, long long pos, MFT_REF mref, unsigned type)
{

	struct antfs_filler *filler = (struct antfs_filler *)buf;
	char *filename = NULL;
	int err = 0;

	/*   To emulate the behaviour of the ntfs-3g we don't show DOS file names.
	 *   Files created on Windows have two file name attributes with type DOS
	 *   and WIN32 or one attribute with type DOS_AND_WIN32 if both would be
	 *   the same name.
	 */
	if (name_type == FILE_NAME_DOS)
		goto out;

	/* - if we already read this part, just return 0 - */
	if (pos < filler->file->f_pos && pos != 0)
		goto out;

	err = ntfs_ucstombs(name, name_len, &filename, 0);
	if (err < 0) {
		/* - we have to return an error, so we take out of memory - */
		antfs_log_error("Failed to convert ucs to char *");
		goto out;
	} else {
		name_len = err;
		err = 0;
	}

	/*  We don't want to show the first 16 MFT entries except root.
	 *  Also we dont wanna blend out '..' which can have any ino, therefore
	 *  we check for a leading '$'
	 */
	if (MREF(mref) < FILE_FIRST_USER)
		if (filename[0] == '$')
			goto free;

	err = filler->filldir(filler->buffer, filename, name_len,
			      pos, MREF(mref), type);
	if (err) {
		/* - if we cant submit more dirent's -> out of memory - */
		antfs_log_debug("Out of memory");
		goto free;
	}

	filler->file->f_pos = pos;
free:
	ntfs_free(filename);
out:
	return err;
}

/**
 * @brief reading the entries of an directory.
 *
 * @param filp	    filp struct of the directory to read
 * @param dstbuf    dirent buffer vfs wants to be filled with entries
 * @param filldir   function that has to be used to fill in the dstbuf
 *
 * @return 0 if successfully read the directory in question
 *
 * antfs_readdir is called by the vfs to eventually provide the filldir()
 * function with the right parameters to fill in the given buffer. With the
 * use of ntfs_readdir which is using a given function to fill in dirent's
 * a couple problems occur:
 *  1.)	ntfs_readdir() is using the provided function with one argument more,
 *	than the one provided by the vfs filldir() function, so we can't use
 *	that one straight away.
 *  2.)	ntfs uses unicode strings for saving file names, instead of regular
 *	char strings like the vfs wants them. In addition all dirents are gonna
 *	read at once without knowing beforehand how many entries there will be.
 * To solve these issues we take another function to wrap around filldir().
 * antfs_dir_read() is used to convert the uc strings into regular strings
 * and use filldir() to fill in the buffer the vfs provided. To pass along
 * filldir() and dstbuf to antfs_dir_read() a antfs_filler struct is used
 * which also keeps the file struct for a check if we already read that dirent.
 */
static int antfs_readdir(struct file *filp, void *dstbuf, filldir_t filldir)
{

	struct antfs_filler filler;
	struct ntfs_inode *ni = ANTFS_NI(file_inode(filp));
	long long pos = filp->f_pos;
	int err;

	filler.filldir = filldir;
	filler.buffer = dstbuf;
	filler.file = filp;

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL))
		return -ERESTARTSYS;
	err = ntfs_readdir(ni, &pos, &filler, (ntfs_filldir_t) antfs_dir_read);
	mutex_unlock(&ni->ni_lock);

	filp->f_pos = (loff_t) pos;
	return err;
}
#else
/**
 * @brief fills in a dirent to create a dentry from the information.
 *
 * @param buf	    pointer to struct antfs_filler
 * @param name	    unicode string name of entry
 * @param name_len  length of name
 * @param name_type type of the name, see enum FILE_NAME_TYPE_FLAGS
 * @param pos	    position used by ntfs_readdir() unused
 * @param mref	    position in the mft table of the entry
 * @param type	    type of the entry (directory, symlink, regular file etc.)
 *
 * @return 0 if we filled in a dirent, -ENOMEM if we were not able too.
 *
 * antfs_dir_iterate uses the @ctx->actor() function that is provided by the
 * vfs to write dirents into a buffer that will be used to create dentries from
 * that. @ctx->actor() takes a regular char string instead of a uc string as a
 * parameter so the name needs to be converted into a regular string. All
 * other paramters can be just taken as is to use @ctx->actor().
 * The antfs_filler struct is used to keep track of the progress of reading
 * entries from the disk. The ctx->pos counter gets incremented for every dirent
 * that gets read from the disk. In case of too many entries in the directory
 * the provided buffer will be filled before done reading the entries. The vfs
 * will call readdir() as long as we will fill in more entries.
 * For not adding duplicates we need to check if the current entry is already
 * filled in, in which case @pos would equal the filler->ctx->pos and is not
 * the root entry '.' with @pos = 0. This could mean two things: this is the
 * last entry that was added before we run out of memory to fill in more
 * entries, or we read all entries already. In both cases we just ignore the
 * current entry, ntfs_readdir() will either way call us with more entries,
 * or return to the caller antfs_iterate().
 *
 * This is also used as filldir function in e.g. @ref ntfs_readdir
 */
static int antfs_dir_iterate(void *buf, ntfschar *name, int name_len,
			     int name_type, long long pos,
			     MFT_REF mref, unsigned type)
{

	struct antfs_filler *filler = (struct antfs_filler *)buf;
	char *filename = NULL;
	int err = 0;

	/*   To emulate the behaviour of the ntfs-3g we don't show DOS file names.
	 *   Files created on Windows have two file name attributes with type DOS
	 *   and WIN32 or one attribute with type DOS_AND_WIN32 if both would be
	 *   the same name.
	 */
	if (name_type == FILE_NAME_DOS)
		goto out;

	/* - if we already read this part, just return 0 - */
	if (pos < filler->ctx->pos && pos != 0)
		goto out;

	err = ntfs_ucstombs(name, name_len, &filename, 0);
	if (err < 0) {
		/* - we have to return an error, so we take out of memory - */
		antfs_log_error("Failed to convert ucs to char *");
		goto out;
	} else {
		name_len = err;
		err = 0;
	}

	/*  We don't want to show the first 16 MFT entries except root.
	 *  Also we dont wanna blend out '..' which can have any ino, therefore
	 *  we check for a leading '$'
	 */
	if (MREF(mref) < FILE_FIRST_USER)
		if (filename[0] == '$')
			goto free;

	antfs_log_debug("dirent: %s | ino: %llu | type: %x",
			filename, MREF(mref), type);
	err = !dir_emit(filler->ctx, filename, name_len, MREF(mref), type);
	if (err) {
		/* If we can't submit more dirent's -> out of memory;
		 * This is not really an error. */
		antfs_log_info("out of Memory");
		err = -ENOMEM;
		goto free;
	}

	filler->ctx->pos = pos;
free:
	ntfs_free(filename);
out:
	antfs_log_leave("Exit (%d)", err);
	return err;
}

/**
 * @brief reading the entries of an directory.
 *
 * @param filp	    file struct of the directory to read
 * @param ctx	    dirent context that hold a filler function and a buffer
 *
 * @return 0 if successfully read the directory in question
 *
 * antfs_iterate is called by the vfs to eventually provide the @ctx->actor()
 * function with the right parameters to fill in the given buffer. With the
 * use of ntfs_readdir which is using a given function to fill in dirent's
 * a couple problems occur:
 *  1.)	ntfs_readdir() is using the provided function with one argument more,
 *	than the one provided by the vfs filldir() function, so we can't use
 *	that one straight away.
 *  2.)	ntfs uses unicode strings for saving file names, instead of regular
 *	char strings like the vfs wants them. In addition all dirents are gonna
 *	read at once without knowing beforehand how many entries there will be.
 * To solve these issues we take another function to wrap around filldir().
 * antfs_dir_iterate() is used to convert the uc strings into regular strings
 * and use @ctx->actor() to fill in the buffer inside of ctx. To pass along the
 * ctx to antfs_dir_iterate() a antfs_filler struct is used.
 */
static int antfs_iterate(struct file *filp, struct dir_context *ctx)
{
	struct ntfs_inode *ni = ANTFS_NI(file_inode(filp));
	struct antfs_filler filler;
	long long pos = ctx->pos;
	int err;

	filler.ctx = ctx;

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL))
		return -ERESTARTSYS;
	err = ntfs_readdir(ni, &pos, &filler,
			   (ntfs_filldir_t) antfs_dir_iterate);
	mutex_unlock(&ni->ni_lock);
	ctx->pos = (loff_t) pos;
	return err;
}
#endif

/**
 * @brief changes the attributes of a ntfs and its corresponding vfs inode
 *
 * @param entry	    dentry of the inode in question
 * @param attr	    struct that holds all the changes to be made
 *
 * @return 0 if everything worked out fine, error code otherwise
 *
 * antfs_setattr is checking the flags in @attr to see which attributes
 * need to be adjusted. According to these flags multiple actions can be
 * performed:
 * 1.) changing of the mode of the inode
 * 2.) changing of the user and group (seems silly - we don't support it ATM)
 * 3.) increasing or decreasing the size of the file. If the size is already
 *	uptodate, then we won't do anything, otherwise we have to make changes
 *	in the ntfs and vfs inode.
 * 4.) set timestamps for atime, and/or mtime. The timestamps can either be
 *	stored beforehand in @attr, or the actual now can be used.
 */
static int antfs_setattr(struct dentry *entry, struct iattr *attr)
{
	struct inode *inode = entry->d_inode;
	struct ntfs_inode *ni = ANTFS_NI(inode);
	int err = 0;

	antfs_log_debug("iattr->ia_valid: 0x%x", attr->ia_valid);

	if (attr->ia_valid & ATTR_MODE) {
		/*TODO: change the mode*/
		antfs_log_debug("not implemented mode change");
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	setattr_copy(inode, attr);
	mark_inode_dirty(inode);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	generic_setattr(inode, attr);
	mark_inode_dirty(inode);
#else
	err = inode_setattr(inode, attr);
	if (err)
		goto end_size;
#endif

	if (attr->ia_valid & ATTR_SIZE) {
		if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
					S_ISLNK(inode->i_mode))) {
			err = -EINVAL;
			goto end_size;
		}
		antfs_log_debug("attr->ia_size: %llu", attr->ia_size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0)
		inode_dio_wait(inode);
#endif
		/* Always truncate if we try to set the size to 0
		 * so we can free memory that was previously allocated
		 * by fallocate
		 */
		if (inode->i_size == attr->ia_size &&
		    !(ANTFS_NA(ni)->allocated_size > 0 && attr->ia_size == 0)) {
			antfs_log_debug("no change needed!");
			goto end_size;
		}
		err = block_truncate_page(inode->i_mapping, attr->ia_size,
				antfs_get_block);
		if (err) {
			antfs_log_error("block_truncate_page failed: %d", err);
			goto end_size;
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
		/* Update inode and truncate buffers/caches if needed.
		 * Linus says we call this BEFORE doing filesystem specific
		 * stuff. */
		truncate_setsize(inode, attr->ia_size);
#else
		{
			loff_t oldsize = inode->i_size;

			i_size_write(inode, attr->ia_size);
			truncate_pagecache(inode, oldsize, attr->ia_size);
		}
#endif
		if (mutex_lock_interruptible_nested(&ni->ni_lock,
					NI_MUTEX_NORMAL)) {
			err = -ERESTARTSYS;
			goto end_size;
		}
		err = ntfs_attr_truncate(ANTFS_NA(ni), attr->ia_size);
		mutex_unlock(&ni->ni_lock);
		if (err) {
			antfs_log_error("Could not truncate data_size: %d",
					err);
			/* If this fails, better don't touch anything else. */
			goto end_size;
		}
		antfs_inode_update_times(inode, NTFS_UPDATE_MCTIME);
		if (inode_needs_sync(inode)) {
			sync_mapping_buffers(inode->i_mapping);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
			sync_inode_metadata(inode, 1);
#else
			{
				struct writeback_control wbc = {
					.sync_mode = WB_SYNC_ALL,
					.nr_to_write = 0, /* metadata-only */
				};

				sync_inode(inode, &wbc);
			}
#endif
		} else {
			mark_inode_dirty(inode);
		}
	}
end_size:
	return err;
}

/**
 * @brief called by the vfs to fill an inodes attributes into a kstat struct.
 *
 * @param mnt	TODO idk what this is for
 * @param entry	dentry of the file that vfs needs the inodes attributes from
 * @param stat	kstat struct that vfs want's to be filled in
 *
 * @return 0 if stat is filled with attributes, -EIO if no stat was given
 *
 * antfs_getattr gets called to transfer inode information from a given dentry
 * to a kstat struct that the vfs uses to evaluate the information. Since all
 * relevant information is stored in the vfs inode, we just need to fetch the
 * vfs inode which is stored in the dentry and use generic_fillattr() to fill
 * stat.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
static int antfs_getattr(const struct path *path, struct kstat *stat,
			 u32 request_mask __attribute__((unused)),
			 unsigned int flags __attribute__((unused)))
{
	struct inode *inode = path->dentry->d_inode;
#else
static int antfs_getattr(struct vfsmount *mnt __attribute__((unused)),
			 struct dentry *entry, struct kstat *stat)
{
	struct inode *inode = entry->d_inode;
#endif
	int err = 0;

	if (!stat) {
		err = -EIO;
		goto out;
	}
	generic_fillattr(inode, stat);
out:
	return err;
}

/**
 * @brief set's the target of a symlink in given nameidata struct
 *
 * @param dentry    symlink to follow
 * @param nd	    nameidata struct which needs to be set
 *
 * @return  returns NULL since we have nothing to be free'd by put_link()
 *
 * antfs_follow_link is setting up the given nameidata struct @nd so that
 * the vfs can resolve the symlink to it's intended target. There are two ways
 * to resolve a symlink, first it could be an actual ntfs symlink that has a
 * reparse point which can be used to find the target file, and second there
 * are interix symlinks which are set as system files and contains the target
 * in the unnamed AT_DATA attribute.
 * In case of a possible interix symlink we check for the validity of the
 * symlink:
 *	1.) is a system file
 *	2.) has an unnamed AT_DATA attribute
 *	3.) attribute size is in the range of a interix symlink
 *	    (means bigger than the interix file type and smaller than
 *	     the maximal path name length)
 *	4.) check if the interix file is a symbolic link
 * and then convert the uc string to a regular string and set the link with
 * nd_set_link().
 * NOTE: we can only follow relative paths and not absolute paths if they were
 * not created with the same mount point we use.
 * For symlinks with reparse point ntfs_make_symlink() is called with the during
 * the mount process saved mount point of the ntfs device and the ntfs inode of
 * the symlink.
 * NOTE: this can lead to unreachable symlinks since the reparse points are
 * always absolute paths. They are stored in a Windows fashioned way 'C:\'
 * and can target other devices. In the case of a target on a different disk, we
 * will not be able to reach it, since we can't possibly know if that device is
 * mounted in our system.
 * NOTE: the follow_link call is not mutex protected! It could happen that,
 * during our evaluation of the symlink, the symlink gets deleted, resulting in
 * a possible kernel crash. That's why we take the vfs inode's i_mutex and
 * hold it till we are done, so that no one can remove our symlink while we
 * evaluate it!
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
static const char *antfs_get_link(struct dentry *dentry, struct inode *inode,
				  struct delayed_call *done)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
static const char *antfs_follow_link(struct dentry *dentry, void **cookie)
#else
static void *antfs_follow_link(struct dentry *dentry, struct nameidata *nd)
#endif
{
	struct ntfs_inode *ni = NULL;
	struct ntfs_attr *na = NULL;
	struct INTX_FILE *intx_file = NULL;
	char *link = NULL;
	int attr_size = 0;
	int err = 0;
	s64 br;

	if (!dentry)
		return ERR_PTR(-ECHILD);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	down_read(&dentry->d_inode->i_rwsem);
#else
	mutex_lock(&dentry->d_inode->i_mutex);
#endif
	ni = ANTFS_NI(dentry->d_inode);
	if (IS_ERR_OR_NULL(ni)) {
		err = -EINVAL;
		antfs_log_error("Invalid arguments");
		goto out;
	}

	/* we can't resolve all reparse points since these links can point to
	 * different disk's that we can't find out how they are named in linux.
	 * the reparse point is defined by the drive letter which we can't use.
	 */
	if (ni->flags & FILE_ATTR_REPARSE_POINT) {
		char mnt_point[128] = {'\0'};

		err = antfs_root_path(dentry, mnt_point,
					  ARRAY_SIZE(mnt_point));
		if (err) {
			antfs_log_error("err: %d", err);
			goto out;
		}

		link = ntfs_make_symlink(ni, mnt_point, &attr_size);
		if (!IS_ERR(link))
			goto done;
		/* - in case we fail to create a symlink we just ignore it - */
		err = PTR_ERR(link);
		goto out;
	}
	if (!(ni->flags & FILE_ATTR_SYSTEM)) {
		antfs_log_error("Not a interix file");
		goto out;
	}

	na = ANTFS_NA(ni);

	if ((size_t) na->data_size <= sizeof(enum INTX_FILE_TYPES)) {
		antfs_log_error("Attribute size too small");
		goto out;
	}
	if ((size_t) na->data_size > sizeof(enum INTX_FILE_TYPES) +
	    sizeof(ntfschar) * PATH_MAX) {
		antfs_log_error("Attribute size too big");
		err = -EIO;
		goto out;
	}
	intx_file = kmalloc(na->data_size, GFP_KERNEL);
	if (!intx_file) {
		antfs_log_error("Intx_file: OOM");
		err = -ENOMEM;
		goto out;
	}
	memset(intx_file, 0, na->data_size);
	br = ntfs_attr_pread(na, 0, na->data_size, intx_file);
	if (br != na->data_size) {
		antfs_log_error("Failed to pread: %lld", br);
		if (br < 0)
			err = (int)br;
		else if (!br)
			err = -EIO;
		goto free_n_out;
	}
	if (intx_file->magic != INTX_SYMBOLIC_LINK) {
		antfs_log_error("Not an interix symbolic link");
		err = -EIO;
		goto free_n_out;
	}
	err = ntfs_ucstombs(intx_file->target, (na->data_size -
			    offsetof(struct INTX_FILE, target)) /
			    sizeof(ntfschar), &link, 0);
	if (err < 0) {
		antfs_log_error("Failed to convert ucs to mbs");
		goto free_n_out;
	}
	/*  dont free link here, since nd will point to it link will be free'd
	 *  in antfs_put_link!
	 */
done:
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
	nd_set_link(nd, link);
#endif
free_n_out:
	kfree(intx_file);
out:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	up_read(&dentry->d_inode->i_rwsem);
#else
	mutex_unlock(&dentry->d_inode->i_mutex);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
	set_delayed_call(done, kfree_link, link);
	return err < 0 ? ERR_PTR(err) : link;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
	return err < 0 ? ERR_PTR(err) : (*cookie = link);
#else
	return err < 0 ? ERR_PTR(err) : NULL;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
/**
 * @brief cleans up what follow_link() was allocating
 *
 * @param dentry    symlink that needs to be cleaned up
 * @param nd	    nameidata struct containing the link
 * @param c	    fs specifics that were allocated by follow_link()
 *
 * antfs_put_link is freeing what was allocated during antfs_follow_link().
 * In our case it is only the actual link that needs to be free'd. @c should be
 * NULL and doesn't need any attention.
 */
static void antfs_put_link(struct dentry *dentry __attribute__((unused)),
			   struct nameidata *nd,
			   void *c __attribute__((unused)))
{
	char *link = nd_get_link(nd);

	if (!IS_ERR(link))
		kfree(link);
}
#endif

static const struct inode_operations antfs_common_inode_operations = {
	.lookup = antfs_lookup,
	.mkdir = antfs_mkdir,
	.unlink = antfs_unlink,
	.rmdir = antfs_rmdir,
	.rename = antfs_rename,
	.setattr = antfs_setattr,
	.create = antfs_create,
	.getattr = antfs_getattr,
};

static const struct file_operations antfs_dir_operations = {
	.llseek = generic_file_llseek,
	.read = generic_read_dir,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
	.readdir = antfs_readdir,
#else
	.iterate = antfs_iterate,
#endif
	.fsync = antfs_fsync,
};

static const struct inode_operations antfs_symlink_inode_operations
__maybe_unused = {
	.setattr = antfs_setattr,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
	.follow_link = antfs_follow_link,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
	.put_link = antfs_put_link,
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
	.put_link = kfree_put_link,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
	.get_link = antfs_get_link,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
	.readlink = generic_readlink,
#endif
	.getattr = antfs_getattr,
};

/**
 * @brief set inode operations all inode's are using.
 *
 * @param inode	the inode to set the operations
 *
 * antfs_inode_init_common provides all inodes with the function calls
 * that the vfs is going to use on them.
 */
void antfs_inode_init_common(struct inode *inode)
{
	inode->i_op = &antfs_common_inode_operations;
}

/**
 * @brief set inode operations for directories
 *
 * @param inode	the inode to set the operations
 *
 * antfs_dir_init provides directory inodes with the function calls
 * that the vfs is going to use on directories.
 * NOTE:    the caller must ensure that the inode represents a directory and
 *	    not a regular file since we don't check here anymore.
 */
void antfs_inode_init_dir(struct inode *inode)
{
	inode->i_fop = &antfs_dir_operations;
}

/**
 * @brief set inode operations for symlinks
 *
 * @param inode	the inode to set the operations
 *
 * antfs_dir_init provides symlink inodes with the function calls
 * that the vfs is going to use on symlinks.
 */
void antfs_inode_init_symlink(struct inode *inode)
{
	inode->i_op = &antfs_symlink_inode_operations;
}
