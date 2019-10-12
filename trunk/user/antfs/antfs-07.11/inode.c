/**
 * inode.c - NTFS Driver by AVM GmbH (ANTFS)
 *           Based on ntfs-3g
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
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/parser.h>
#include <linux/statfs.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/exportfs.h>
#include <linux/delay.h>

#include "dir.h"
#include "volume.h"
#include "misc.h"

/* "NTFS" in hex, interpreted by "stat -f" command */
#define ANTFS_SUPER_MAGIC 0x5346544e

struct antfs_mount_data {
	int fd;
	unsigned rootmode;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
	uid_t user_id;
	gid_t group_id;
#else
	kuid_t user_id;
	kgid_t group_id;
#endif
	unsigned fd_present:1;
	unsigned rootmode_present:1;
	unsigned user_id_present:1;
	unsigned group_id_present:1;
	unsigned flags;
	unsigned max_read;
	unsigned blksize;
};

/**
 * @brief allocation of a new inode.
 *
 * @param sb	super_block of the device the new inode should be created for
 *
 * @return newly created vfs inode or NULL if out of memory
 *
 * antfs_alloc_inode allocates the memory for a vfs inode in the size of a
 * antfs_inode_info struct since the vfs inode is embedded in there.
 */
static struct inode *antfs_alloc_inode(struct super_block *sb
				       __attribute__((unused)))
{
	struct inode *inode = NULL;

	inode = kmem_cache_alloc(antfs_inode_cachep, GFP_KERNEL);

	return inode;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)
/**
 * @brief callback for destroying an inode
 *
 * @param head	the list element of the inode to destroy
 *
 * antfs_i_callback is freeing the memory that was used for the inode out of
 * the kmem_cache.
 */
static void antfs_i_callback(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
	INIT_HLIST_HEAD(&inode->i_dentry);
#else
	INIT_LIST_HEAD(&inode->i_dentry);
#endif
	ntfs_inode_real_close(ANTFS_NI(inode));

	kmem_cache_free(antfs_inode_cachep, inode);
}
#endif

/**
 * @brief destroys an inode
 *
 * @param inode	the vfs inode to free
 *
 * antfs_destroy_inode calls antfs_i_callback to free the memory occupied by
 * the inode and antfs_inode_info.
 */
static void antfs_destroy_inode(struct inode *inode)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)
	call_rcu(&inode->i_rcu, antfs_i_callback);
#else
	ntfs_inode_real_close(ANTFS_NI(inode));

	kmem_cache_free(antfs_inode_cachep, inode);
#endif
}

/**
 * @brief "evict" an inode
 *
 * @param inode Inode to delete
 *
 * This is called to clean up an inode before destroying it and releasing
 * associated memory. If i_nlink is 0 when this is called, the associated mft
 * record and its extents also get removed from disk.
 *
 * @note For Linux kernel < 2.6.36 this is "delete_inode".
 *       i_nlink is always zero in this case.
 */
static void antfs_evict_inode(struct inode *inode)
{
	struct ntfs_inode *ni = ANTFS_NI(inode);
	int err, ext;
	int want_delete = 0;
	enum antfs_inode_mutex_lock_class lc = NI_MUTEX_NORMAL;

	antfs_log_enter("ino: %ld", inode->i_ino);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	if (!inode->i_nlink)
#endif
		want_delete = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 10)
	truncate_inode_pages_final(&inode->i_data);
#else
	truncate_inode_pages(&inode->i_data, 0);
#endif
	if (want_delete)
		/* truncate to 0 */
		inode->i_size = 0;
	else if (NInoDirty(ni))
		ntfs_inode_sync(ni);

	invalidate_inode_buffers(inode);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0) && \
	LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	end_writeback(inode);
#else
	clear_inode(inode);
#endif

	/* Is this a base inode with mapped extent inodes? */
	if (unlikely(ni->nr_extents == -1 && ni->base_ni)) {
		struct ntfs_inode **tmp_nis;
		struct ntfs_inode *base_ni;
		s32 i;

		/* Use lower locking class for extents. Normal and parents may
		 * already be locked here.
		 */
		lc = NI_MUTEX_EXTENT;
		/*
		 * If the inode is an extent inode, disconnect it from the
		 * base inode before destroying it.
		 */
		base_ni = ni->base_ni;

		for (i = 0; i < base_ni->nr_extents; ++i) {
			tmp_nis = base_ni->extent_nis;
			if (tmp_nis[i] != ni)
				continue;
			/* Found it. Disconnect. */
			memmove(tmp_nis + i, tmp_nis + i + 1,
				(base_ni->nr_extents - i - 1) *
				sizeof(struct ntfs_inode *));
			/* Buffer should be for multiple of four extents. */
			if ((--base_ni->nr_extents) & 3) {
				i = -1;
				break;
			}
			/*
			 * ElectricFence is unhappy with realloc(x,0) as free(x)
			 * thus we explicitly separate these two cases.
			 */
			if (base_ni->nr_extents) {
				/* Resize the memory buffer. */
				tmp_nis = ntfs_realloc(tmp_nis,
						base_ni->nr_extents *
						sizeof(struct ntfs_inode *));
				/* Ignore errors, they don't really matter. */
				if (tmp_nis)
					base_ni->extent_nis = tmp_nis;
			} else if (tmp_nis) {
				ntfs_free(tmp_nis);
				base_ni->extent_nis =
				    (struct ntfs_inode **)NULL;
			}
			/* Allow for error checking. */
			i = -1;
			break;
		}

		/*
		 *  We could successfully sync, so only log this error
		 *  and try to sync other inode extents too.
		 */
		if (i != -1)
			antfs_log_error("Extent inode %lld was not found",
					(long long)ni->mft_no);
	}

	if (want_delete) {
		/* Really delete inode */
		if (ni->mft_no < FILE_FIRST_USER) {
			antfs_log_error("mft_no %lld < FILE_first_user",
					ni->mft_no);
			goto out;
		}
		/* We have no parent / dir_ni here (nlink is 0!)
		 * --> free inode
		 */
		if (mutex_lock_interruptible_nested(&ni->ni_lock, lc)) {
			antfs_log_error("Could not get ni_lock for ni %lld;",
					(long long)ni->mft_no);
			return;
		}
		mutex_unlock(&ni->ni_lock);
		err = ntfs_inode_free(ni);
		if (err) {
			antfs_log_error("Error freeing inode: %lld (%d)",
					ni->mft_no, err);
		}
	}
	if (ni->nr_extents > 0) {
		/* We have to NULL base_ni since we can't guarantee that the
		 * extent ni's will call ntfs_inode_real_close before the
		 * base_ni does. Danger of accessing corrupted memory */
		for (ext = 0; ext < ni->nr_extents; ext++) {
			ni->extent_nis[ext]->base_ni = NULL;
			iput(ANTFS_I(ni->extent_nis[ext]));
		}
		ntfs_free(ni->extent_nis);
	}
out:
	antfs_log_leave();
}

/**
 * @brief remounts the device with new parameters
 *
 * @param sb	    super_block of the device to remount
 * @param flags	    bitmask of new universal mount options like rw or ro
 * @param data	    ntfs specific mount options that need to be parsed
 *
 * @return 0 if everything worked out
 *
 * antfs_remount_fs is remounting the ntfs device with given parameters in
 * @data and @flags. We just check if MS_MANDLOCK is set which we don't support
 * and set in the sbi if we are now mounted as read-only or not. At last the
 * mount options will be parsed.
 */
static int antfs_remount_fs(struct super_block *sb, int *flags, char *data)
{
	struct antfs_sb_info *sbi = ANTFS_SB(sb);

	if (*flags & MS_MANDLOCK)
		return -EINVAL;

	if (*flags & MS_RDONLY)
		sbi->ro = 1;
	else
		sbi->ro = 0;

	antfs_parse_options(sbi, data);

	return 0;
}

/**
 * @brief cleans up memory after umount
 *
 * @param sb	super_block of the device that gets unmounted
 *
 * antfs_put_super is destroying the antfs_sb_info struct that was allocated
 * in the mount process and also destroys the backing_device_info that was also
 * allocated during mounting the device. Since the bdi was allocated, kfree()
 * needs to get called on it in order to prevent a memory leak.
 */
void antfs_put_super(struct super_block *sb)
{
	struct antfs_sb_info *sbi = ANTFS_SB(sb);

	ntfs_umount(sbi->vol, 0);
	antfs_sbi_destroy(sbi);
}

/**
 * @brief retrieves information about the volume the given dentry resides on.
 *
 * @param dentry    the dentry which resides on the volume in question
 * @param buf	    kstatfs struct to be filled
 *
 * @return 0 if buf is correctly filled, -ENODEV if there is no volume
 *
 * antfs_statfs() transcribes the volume information of the volume where the
 * given dentry resides on into the kstatfs struct @buf provided by the vfs.
 */
static int antfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct ntfs_inode *ni = ANTFS_NI(dentry->d_inode);
	struct ntfs_volume *vol = ni->vol;
	s64 size;
	int delta_bits, err = 0;

	if (!vol) {
		err = -ENODEV;
		goto out;
	}

	antfs_log_enter();
	buf->f_type = ANTFS_SUPER_MAGIC;
	/*
	 * File system block size. Used to calculate used/free space by df.
	 * Incorrectly documented as "optimal transfer block size".
	 */
	buf->f_bsize = vol->cluster_size;

	/* Fundamental file system block size, used as the unit. */
	buf->f_frsize = vol->cluster_size;
	antfs_log_debug("f_bsize = f_frsize = %lld", (long long)buf->f_frsize);

	/*
	 * Total number of blocks on file system in units of f_frsize.
	 * Since inodes are also stored in blocks ($MFT is a file) hence
	 * this is the number of clusters on the volume.
	 */
	buf->f_blocks = vol->nr_clusters;

	/* Free blocks available for all and for non-privileged processes. */
	size = vol->free_clusters - antfs_reserved_clusters(vol);
	if (size < 0)
		size = 0;
	buf->f_bavail = buf->f_bfree = size;
	if (buf->f_blocks < buf->f_bavail) {
		/* Hm, this must not happen. */
		antfs_log_error("f_blocks < f_bavail (%lld < %lld)",
				(long long)buf->f_blocks,
				(long long)buf->f_bavail);
		buf->f_bavail = buf->f_bfree = vol->free_clusters =
		    vol->nr_clusters;
	}

	/* Free inodes on the free space */
	delta_bits = vol->cluster_size_bits - vol->mft_record_size_bits;
	if (delta_bits >= 0)
		size <<= delta_bits;
	else
		size >>= -delta_bits;

	/* Number of ALL inodes at this point in time (includes free space).
	 *
	 * This is number of USED inodes + free space on disk for new inodes.
	 * Number of used inodes is size of MFT bitmap minus the number of free
	 * mft records that we keep track of.
	 */
	if (vol->mftbmp_na->data_size << 3 >= vol->free_mft_records) {
		buf->f_files = size + (vol->mftbmp_na->data_size << 3) -
		    vol->free_mft_records;
		buf->f_ffree = size;
	} else {
		/* This is also bad! */
		antfs_log_error
		    ("mftbmp->datasize << 3 < free_mft_records (%lld < %lld)",
		     vol->mftbmp_na->data_size << 3, vol->free_mft_records);
		buf->f_files = buf->f_ffree = 0;
	}

	/* Free inodes available for all and for non-privileged processes.
	 * This is basically just the number of MFT records the free space on
	 * disk can hold. */
	antfs_log_debug("vol->free_mft_records: %lld; size: %lld; delta_bits: "
			"%d; vol->free_clusters: %lld; vol->nr_clusters: %lld; "
			"mftbmp->allocated_size: %lld; mftbmp->data_size: %lld",
			(long long)vol->free_mft_records, (long long)size,
			(int)delta_bits, (long long)vol->free_clusters,
			(long long)vol->nr_clusters,
			(long long)vol->mftbmp_na->allocated_size,
			(long long)vol->mftbmp_na->data_size);

	antfs_log_debug("f_blocks = %lld; f_bfree = f_bavail = %lld; f_files "
			"= %lld; f_ffree = %lld", (long long)buf->f_blocks,
			(long long)buf->f_bavail, (long long)buf->f_files,
			(long long)buf->f_ffree);

	buf->f_fsid.val[0] = vol->serial_no & 0xffffffff;
	buf->f_fsid.val[1] = (vol->serial_no >> 32) & 0xffffffff;
	/* Maximum length of filenames. */
	buf->f_namelen = NTFS_MAX_NAME_LEN;
out:
	return err;
}

/**
 * antfs_inode_init: sets an inode's operations according to its mode
 *
 * @inode    the vfs inode to initialize
 * @create   Control the way inode collisions are handeled.
 *
 * antfs_inode_init sets the inode operations according to the type of
 * file the inode represents.
 * If an inode collision in VFS is detected and create is 0, the inode supplied
 * to this function is discarded and replaced with the colliding inode.
 * If create is positive on inode collision, an error is reported and the inode
 * is discarded but not replaced.
 * If create is negative, the inode is discarded and deleted for extent inodes
 * or NOT discarded at all for base inodes (caller is responsible for cleanup
 * e.g. in @antfs_create_i).
 *
 * Return: 0 on success or negative error code.
 */
int antfs_inode_init(struct inode **inode_in, enum antfs_inode_init_mode create)
{
	struct inode *inode = *inode_in;
	struct ntfs_inode *ni = ANTFS_NI(inode);
	int err = 0;

#if KERNEL_VERSION(4, 0, 0) > LINUX_VERSION_CODE
	inode->i_data.backing_dev_info = inode->i_sb->s_bdi;
#endif
	inode->i_ino = (loff_t)ni->mft_no;

	if (likely(ni->nr_extents >= 0 && (inode->i_ino >=
					(unsigned long)FILE_FIRST_USER ||
					inode->i_ino ==
					(unsigned long)FILE_ROOT))) {
		struct antfs_sb_info *sbi = ANTFS_SB(inode->i_sb);
		struct TIMESPEC ts;

		/* Init a base mft record ("regular" inode):
		 * this has to be done in context of either:
		 * antfs_create/mkdir or ntfs_inode_open
		 */
		inode->i_blocks = (ANTFS_NA(ni)->allocated_size + 511) >> 9;

		if (ni->flags & FILE_ATTR_REPARSE_POINT) {
			if (IS_ENABLED(CONFIG_ANTFS_SYMLINKS)) {
				/* - symlink with reparse point - */
				inode->i_mode = S_IFLNK | sbi->umask;

				antfs_inode_init_symlink(inode);
				set_nlink(inode, le16_to_cpu(
							ni->mrec->link_count));
			} else {
				/* Don't support symlinks here. */
				make_bad_inode(inode);
				/* Shut up unlock_new_inode in iget_failed. */
				inode->i_state |= I_NEW;
				err = -EPERM;
				goto out;
			}
		} else if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY) {
			inode->i_mode = S_IFDIR | sbi->umask;
			/* TODO: do we need to do this? */
			inode->i_blocks = ANTFS_NA(ni)->allocated_size >> 9;

			antfs_inode_init_common(inode);
			antfs_inode_init_dir(inode);
		} else {
			/* - regular file - */
			inode->i_mode = S_IFREG | sbi->umask /* & ~S_IXUGO */;
			/* Set file read only if it is compressed */
			/* TODO: Remove this as soon as we can write compressed
			 * files.
			 */
			if (ANTFS_NA(ni)->data_flags & ATTR_COMPRESSION_MASK) {
				/* File is compressed --> RO */
				inode->i_mode &= ~S_IWUGO;
				antfs_log_debug("Compressed --> set read only");
			}

			antfs_inode_init_common(inode);
			antfs_inode_init_file(inode);
			set_nlink(inode, le16_to_cpu(ni->mrec->link_count));
		}

		err = ntfs_inode_na_open(ni);
		if (unlikely(err)) {
			antfs_log_error("Couldn't open DATA/INDEX_ROOT for "
					"ni(%lld)", ni->mft_no);
			goto out;
		}

		i_size_write(inode, ANTFS_NA(ni)->data_size);
		/* FIXME: we have to figure out how to deal with user and group
		 *        id's everything root is ok for now, but we will need
		 *        to change that as soon as we are not always running
		 *        as root. If passed during mount we can get sbi->uid
		 *        and sbi->gid tho.
		 */
		ts = ntfs2timespec(ni->last_access_time);
		inode->i_atime.tv_sec = ts.tv_sec;
		inode->i_atime.tv_nsec = ts.tv_nsec;
		ts = ntfs2timespec(ni->last_data_change_time);
		inode->i_mtime.tv_sec = ts.tv_sec;
		inode->i_mtime.tv_nsec = ts.tv_nsec;
		ts = ntfs2timespec(ni->last_mft_change_time);
		inode->i_ctime.tv_sec = ts.tv_sec;
		inode->i_ctime.tv_nsec = ts.tv_nsec;
	} else {
		/* handle a new extent inode */
		make_bad_inode(inode);
		/* Extent inodes always get nlink of 1 so they always get
		 * deleted if we delete the base inode. No need to count the
		 * nlink here aswell.
		 */
	}

	/* This fails if we already have an inode with the same
	 * i_ino and i_sb that is not I_FREEING or I_WILL_FREE.
	 */
	if (insert_inode_locked(inode) < 0) {
		struct inode *c_inode = ilookup(inode->i_sb, inode->i_ino);
		struct ntfs_inode *c_ni = c_inode ? ANTFS_NI(c_inode) : NULL;

		/* Shut up unlock_new_inode in iget_failed. */
		inode->i_state |= I_NEW;
		NInoSetCollided(ni);
		if (create == ANTFS_INODE_INIT_REPLACE &&
				c_ni && c_ni->mft_no == ni->mft_no) {
			antfs_log_info("Ino 0x%llx collided. Discard inode and "
					"use valid counterpart instead.",
					(long long)ni->mft_no);
			*inode_in = c_inode;
			iget_failed(inode);
			/* Keep i_count at 1 for extents. */
			if (c_ni->nr_extents < 0 && c_ni->base_ni &&
					atomic_read(&c_inode->i_count) > 1)
				iput(c_inode);
			goto out;
		}

		antfs_logger(inode->i_sb->s_id,
			    "[%s] insert_inode_locked failed.\n", __func__);

		antfs_log_error_ext("insert_inode_locked failed. ino collision?"
				" (%lu)", inode->i_ino);
		if (c_inode) {
			struct ntfs_inode *orig_ni = ANTFS_NI(inode);

			if (is_bad_inode(c_inode))
				antfs_log_error("Colliding inode is bad.");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
			antfs_log_error("Colliding inode i_mode: 0x%lx"
					" i_count: %ld; "
					"i_nlink: %d; "
					"inode_unhashed: %d; "
					"i_state: 0x%x",
					(unsigned long)c_inode->i_mode,
					(long)atomic_read
					(&c_inode->i_count) - 1,
					(int)c_inode->i_nlink,
					inode_unhashed(c_inode),
					(int)c_inode->i_state);
#else
			antfs_log_error("Colliding inode i_mode: 0x%lx"
					" i_count: %ld; "
					"i_nlink: %d; "
					"i_state: 0x%x",
					(unsigned long)c_inode->i_mode,
					(long)atomic_read
					(&c_inode->i_count) - 1,
					(int)c_inode->i_nlink,
					(int)c_inode->i_state);
#endif
			antfs_log_error("Colliding inode is "
					"valid ntfs inode: "
					"mft: %lld;"
					"our ni: %p;",
					(long long)
					c_ni->mft_no,
					orig_ni);

			if (ANTFS_NI(c_inode)->base_ni)
				antfs_log_error("c nr_extents: %d; "
					"c base_ino: %lld",
					(int)c_ni->nr_extents,
					(long long)
					c_ni->base_ni->mft_no);

			/* Trigger a rewrite of the original inode. */
			ntfs_inode_mark_dirty(c_ni);
			mark_inode_dirty(c_inode);
			iput(c_inode);
		}
		err = -EEXIST;
		if (create != ANTFS_INODE_INIT_REPLACE) {
			if (create == ANTFS_INODE_INIT_DELETE) {
				/* DELETE new extent inodes that failed to
				 * insert. Don't delete base inodes though.
				 * Caller is expected to do cleanup.
				 */
				if (ni->nr_extents < 0)
					clear_nlink(inode);
				else
					goto out;
			}
			iget_failed(inode);
		}
		goto out;
	}

	unlock_new_inode(inode);

out:
	return err;
}

/**
 * @brief fetches the root vfs_inode
 *
 * @param sb	super_block of the ntfs device
 *
 * @return the roots vfs inode of the ntfs device in question.
 *
 * antfs_get_root_inode allocates the vfs inode for the root directory
 * of the ntfs device represented by its super_block. It reads the ntfs root
 * inode from disk and then creates the corresponding vfs inode through
 * antfs_iget().
 * NOTE:    the root directory is always located at the same position on every
 *	    device at mft_no 5!
 */
static struct inode *antfs_get_root_inode(struct super_block *sb)
{
	struct antfs_sb_info *sbi = ANTFS_SB(sb);
	struct inode *root;
	struct ntfs_inode *ni;

	/* FILE_ROOT = 5 is the position of the root inode on the filesystem */
	ni = ntfs_inode_open(sbi->vol, FILE_ROOT, NULL);
	/* 1.) ni is error code -> return error code
	 * 2.) ni is correctly allocated -> ANTFS_I(ni) might be bad though!
	 */
	if (IS_ERR(ni))
		root = (struct inode *)ni;
	else
		root = ANTFS_I(ni);

	return root;
}

/**
 * @brief initializes the root inode
 *
 * @param sb	super_block of the ntfs device
 *
 * @return 0 if root set up successful, -EINVAL if not possible
 *
 * antfs_inode_setup_root gets the vfs inode for the root directory and
 * connects its to a dentry which will be stored inside the given super_block
 * as s_root.
 */
int antfs_inode_setup_root(struct super_block *sb)
{
	struct inode *root;
	struct dentry *root_dentry;
	int err = 0;

	root = antfs_get_root_inode(sb);
	if (IS_ERR(root)) {
		err = PTR_ERR(root);
		goto out;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
	root_dentry = d_alloc_root(root);
	if (!root_dentry) {
		iput(root);
		err = -EINVAL;
		goto out;
	}
#else
	root_dentry = d_make_root(root);
	if (!root_dentry) {
		iput(root);
		err = -EINVAL;
		goto out;
	}
#endif
	sb->s_root = root_dentry;
out:
	return err;
}

/**
 * @brief prints device options into given seq_file.
 *
 * @param m seq_file to be filled with options
 * @param root	dentry of the root directory
 *
 * @return 0 if successfully filled m
 *
 * FIXME
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
static int antfs_show_options(struct seq_file *m, struct vfsmount *vfs)
#else
static int antfs_show_options(struct seq_file *m, struct dentry *root)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
	struct super_block *sb = vfs->mnt_sb;
#else
	struct super_block *sb = root->d_sb;
#endif
	struct antfs_sb_info *sbi = ANTFS_SB(sb);

	if (sbi->utf8)
		seq_puts(m, ",utf8");
	if (sbi->umask)
		seq_printf(m, ",umask=%04o", sbi->umask);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
static int antfs_write_inode(struct inode *inode, struct writeback_control *wbc)
#else
static int antfs_write_inode(struct inode *inode,
			     int do_sync __attribute__((unused)))
#endif
{
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_attr *na = ANTFS_NA(ni);
	struct ntfs_attr_search_ctx *ctx = NULL;
	int err = 0;

	antfs_log_enter();
	if (is_bad_inode(inode))
		goto out_unlocked;

	if (NInoWritePending(ni)) {
		err = -EAGAIN;
		goto out_unlocked;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out_unlocked;
	}

	if (!IS_ERR_OR_NULL(na)
		&& NAttrNonResident(na)) {
		err = ntfs_attr_truncate(na, inode->i_size);
		if (err)
			goto out;

		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (IS_ERR(ctx)) {
			antfs_log_error("No ctx");
			err = PTR_ERR(ctx);
			goto out;
		}

		err = ntfs_attr_lookup(na->type, na->name, na->name_len,
				       0, 0, NULL, 0, ctx);
		if (err) {
			antfs_log_error("Attribute lookup failed!");
			ntfs_attr_put_search_ctx(ctx);
			goto out;
		}
		ctx->attr->initialized_size =
			cpu_to_sle64(na->initialized_size);
		ctx->attr->data_size = cpu_to_sle64(inode->i_size);
		ctx->attr->allocated_size = cpu_to_sle64(na->allocated_size);
		if (na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_SPARSE))
			ctx->attr->compressed_size = cpu_to_sle64(
				na->compressed_size);

		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_put_search_ctx(ctx);
	}

	/* In case of simple file access this is set too.
	 * Needed to update atime.
	 */
	if (NInoDirty(ni)) {
		if (na->type == AT_DATA && na->name == AT_UNNAMED) {
			antfs_log_debug("ninofilenamesetdirty");
			NInoFileNameSetDirty(ni);
		}
		ntfs_inode_sync(ni);
	}
out:
	mutex_unlock(&ni->ni_lock);
out_unlocked:
	antfs_log_leave("err: %d", err);
	return err;
}

/* TODO: export_operations
 * Stuff for NFS. ... */

static const struct super_operations antfs_super_operations = {
	.alloc_inode = antfs_alloc_inode,
	.destroy_inode = antfs_destroy_inode,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.evict_inode = antfs_evict_inode,
#else
	.delete_inode = antfs_evict_inode,
#endif
	.remount_fs = antfs_remount_fs,
	.put_super = antfs_put_super,
	.statfs = antfs_statfs,
	.show_options = antfs_show_options,
	.write_inode	= antfs_write_inode,
};

/**
 * @brief sets operations for the super_block
 *
 * @param sb	super_block in question
 *
 * antfs_fill_super_operations sets the s_op and s_export_op of the given
 * super_block.
 */
void antfs_fill_super_operations(struct super_block *sb)
{
	sb->s_op = &antfs_super_operations;
}
