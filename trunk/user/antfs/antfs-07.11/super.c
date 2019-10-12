/**
 * super.c - NTFS Driver by AVM GmbH (ANTFS)
 *           Based on ntfs-3g
 *
 * Copyright (c) 2005-2007 Yura Pakhuchiy
 * Copyright (c) 2005 Yuval Fledel
 * Copyright (c) 2006-2009 Szabolcs Szakacsits
 * Copyright (c) 2007-2015 Jean-Pierre Andre
 * Copyright (c) 2009 Erik Larsson
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
#include <linux/sched/task.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AVM GmbH");
MODULE_DESCRIPTION("NTFS Filesystem");

struct kmem_cache *antfs_inode_cachep;

struct _logger_priv *global_logger;

enum {
	antfs_opt_utf8,
	antfs_opt_uid,
	antfs_opt_gid,
	antfs_opt_umask,
	antfs_opt_err
};

static const match_table_t tokens = {
	{antfs_opt_utf8, "utf8"},
	{antfs_opt_uid, "uid=%u"},
	{antfs_opt_gid, "gid=%u"},
	{antfs_opt_umask, "umask=%u"},
	{antfs_opt_err, NULL}
};

/**
 * @brief free's all the allocated structures in the sbi and the sbi itself.
 *
 * @param sbi	the sbi to free
 *
 * antfs_sbi_destroy checks if certain pointers are pointing to allocated
 * structs and free's them if needed. In the end the sbi itself gets free'd.
 */
void antfs_sbi_destroy(struct antfs_sb_info *sbi)
{
	kfree(sbi->dev);
	kfree(sbi);
}

/**
 * @brief sets the standard mount options in the sbi
 *
 * @param sb	    super_block for the ntfs device
 * @param silent    setting the silent bit yes/no
 *
 * antfs_mnt_opts_init sets the standard mount options for a ntfs device
 * in the given sbi, so that the mounting of a device can be started.
 */
static void antfs_mnt_opts_init(struct super_block *sb, int silent)
{
	struct antfs_sb_info *sbi = ANTFS_SB(sb);
	unsigned char ro = (sb->s_flags & MS_RDONLY) ? 1 : 0;

	sbi->atime = ATIME_RELATIVE;
	sbi->silent = silent;
	sbi->recover = 1;
	sbi->blkdev = 1;
	sbi->ro = ro;
	sbi->umask = 0777;
	sbi->uid = 0;
	sbi->gid = 0;

	if (ro)
		sbi->hiberfile = 1;
}

/**
 * @brief parses the given mount options
 *
 * @param sbi	    file system specific informations
 * @param data	    mount options
 *
 * antfs_parse_options parses the ntfs specific mount options that the vfs
 * simply passes to antfs_fill_super(). If we encounter a mount option that
 * we don't know we just ignore it. Otherwise we set the corresponding field
 * in @sbi according to the mount option.
 */
void antfs_parse_options(struct antfs_sb_info *sbi, char *data)
{
	char *p;
	int option;
	substring_t args[MAX_OPT_ARGS];

	while ((p = strsep(&data, ",")) != NULL) {
		if (!*p)
			continue;

		switch (match_token(p, tokens, args)) {
		case antfs_opt_utf8:
			sbi->utf8 = 1;
			break;
		case antfs_opt_uid:
			if (match_int(&args[0], &option)) {
				antfs_log_error("Parsing opt=uid failed!");
				break;
			}
			sbi->uid = option;
			break;
		case antfs_opt_gid:
			if (match_int(&args[0], &option)) {
				antfs_log_error("Parsing opt=gid failed!");
				break;
			}
			sbi->gid = option;
			break;
		case antfs_opt_umask:
			if (match_octal(&args[0], &option)) {
				antfs_log_error("Parsing opt=umask failed!");
				break;
			}
			sbi->umask = option;
		case antfs_opt_err:
		default:
			break;
		}

	}
}

/**
 * @brief finishes the ntfs volume after mounting a ntfs device.
 *
 * @param sbi	antfs_sb_info which contains the ntfs volume
 *
 * antfs_volume_finish sets the last fields that the ntfs driver needs
 * to work properly which didn't get set during the actual mount.
 */
static void antfs_volume_init_complete(struct antfs_sb_info *sbi)
{
	struct ntfs_volume *vol = sbi->vol;

	vol->mftbmp_bh = ntfs_load_bitmap_attr(vol, vol->mftbmp_na, 0);
	/* if bufferhead is already NULL, no need to set it to NULL */
	if (IS_ERR(vol->mftbmp_bh))
		vol->mftbmp_bh = NULL;

	/* zone_pos holds offset into bitmap in BITS.
	 * lcnbmp_start is in blocks. */
	vol->lcnbmp_start =
	    (vol->data1_zone_pos >> sbi->sb->s_blocksize_bits) >> 3;
	vol->lcnbmp_bh =
	    ntfs_load_bitmap_attr(vol, vol->lcnbmp_na, vol->data1_zone_pos);
	/* if bufferhead is already NULL, no need to set it to NULL */
	if (IS_ERR(vol->lcnbmp_bh))
		vol->lcnbmp_bh = NULL;
	/* TODO: we don't have xattr and maybe never will be. also the
	 *      secure_flags will probably need some work when someone
	 *      decides to implement user permissions
	 */
	vol->secure_flags = 0;
#ifdef HAVE_SETXATTR		/* extended attributes interface required */
	vol->efs_raw = 0;
#endif
/* ---  ntfs_open_secure(vol); --- */
#if defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS)
	vol->xattr_mapping = ntfs_xattr_build_mapping(vol, sbi->xattrmap_path);
#endif /* defined(HAVE_SETXATTR) && defined(XATTR_MAPPINGS) */
}

/**
 * @brief returns the number of free records in the mft
 *
 * @param vol	ntfs device which mft will be looked at
 *
 * @return  number of free records left on the mft
 *
 * ntfs_get_nr_free_mft_records converts the bitmap of free records
 * into an integer number of free records and returns that value.
 */
static s64 ntfs_get_nr_free_mft_records(struct ntfs_volume *vol)
{
	struct ntfs_attr *na = vol->mftbmp_na;
	s64 nr_free = ntfs_attr_get_free_bits(na);

	return nr_free;
}

/**
 * @brief mounts an ntfs device
 *
 * @param sb	super_block that represents the ntfs device to be mounted
 *
 * @return 0 if the ntfs device got successfully mounted, -EINVAL otherwise
 *
 * antfs_open_device prepares the mounting of a ntfs device by setting up the
 * mount options like device name (e.g. /dev/sda1) and flags and then calls
 * ntfs_mount to receive the volume structure which will be safed in the sbi of
 * the super_block.
 * After mounting the device the volume struct needs some fields to be set and
 * gets set to only show regular, system and hidden files but not dot files.
 */
static int antfs_open_device(struct super_block *sb)
{
	struct antfs_sb_info *sbi = ANTFS_SB(sb);
	struct ntfs_volume *vol;
	unsigned long flags = 0;
	char *dev;
	int err = 0;

	dev = kmalloc(sizeof(char) * 32, GFP_KERNEL);
	if (!dev) {
		err = -ENOMEM;
		goto out;
	}

	sprintf(dev, "/dev/%s", sb->s_id);
	sbi->dev = dev;

	if (!sbi->blkdev)
		flags |= NTFS_MNT_EXCLUSIVE;
	if (sbi->ro)
		flags |= NTFS_MNT_RDONLY;
	else if (!sbi->hiberfile)
		flags |= NTFS_MNT_MAY_RDONLY;
	if (sbi->recover)
		flags |= NTFS_MNT_RECOVER;
	if (sbi->hiberfile)
		flags |= NTFS_MNT_IGNORE_HIBERFILE;

	antfs_log_debug("mount flags: 0x%lx, device: %s", flags, dev);
	antfs_log_debug("blkdev=0x%x|ro=0x%x|recover=0x%x|hiberfile=0x%x",
			sbi->blkdev, sbi->ro, sbi->recover, sbi->hiberfile);

	sbi->vol = ntfs_mount(sb, flags);
	if (IS_ERR(sbi->vol)) {
		err = PTR_ERR(sbi->vol);
		sbi->vol = NULL;
		antfs_log_warning("Failed to mount '%s' (%d)", dev, err);
		goto out;
	}

	vol = sbi->vol;
	vol->free_clusters = ntfs_attr_get_free_bits(vol->lcnbmp_na);
	antfs_log_debug("vol->free_clusters: %lld", vol->free_clusters);
	if (vol->free_clusters < 0) {
		antfs_log_error("Failed to read NTFS $Bitmap");
		err = vol->free_clusters;
		goto out;
	}

	vol->free_mft_records = ntfs_get_nr_free_mft_records(vol);
	antfs_log_debug("free_mft_records: %lld", vol->free_mft_records);
	if (vol->free_mft_records < 0) {
		antfs_log_error("Failed to calculate free MFT records");
		err = vol->free_mft_records;
		goto out;
	}

	/*FIXME we show system and hidden files and hide dot files by default.
	 *      later on there should be a way to set up what files we want
	 *      to be visible. Maybe there is a better way to do this through
	 *      mount options?
	 */
	if (ntfs_set_shown_files(vol, 1, 1, 1)) {
		antfs_log_error("Failed to set shown files");
		kfree(dev);
	}
out:
	return err;

}

/**
 * @brief connects the super_block and its sbi
 *
 * @param sb	super_block
 * @param sbi	s_fs_info for the super_block
 *
 * antfs_sbi_init sets the super_block->s_fs_info field to the given sbi
 * and also points the sbi to its super_block.
 */
static void antfs_sbi_init(struct super_block *sb, struct antfs_sb_info *sbi)
{
	sb->s_fs_info = sbi;
	sbi->sb = sb;
}

/**
 * @brief starts the mounting process of a ntfs device
 *
 * @param sb	    super_block for the ntfs device to mount
 * @param data	    mount options that should be ntfs specific
 * @param silent    TODO idk what it really does...
 *
 * @return  0 if the mount of the ntfs device was successful, error code
 *	    otherwise
 *
 * antfs_fill_super is the starting point for mounting a new ntfs device.
 * When it is called we have an almost empty super_block that needs to be filled
 * in with information about the ntfs device. The first step is to set up the
 * filesystem specifics in sb->s_fs_info with a antfs_sb_info struct sbi.
 * Then the ntfs device gets mounted through antfs_open_device() and the
 * volume struct gets finished up for actual usage. When the device is usable
 * we fill in the super_block fields that were missing so far and provide the
 * super_block with functions to handle the filesystem on the device.
 * To use the device the backing device info needs to be set up and stored in
 * the super_block. For a proper usage of Reparse Point Symlinks from Windows
 * the mount point needs to be specified. The last step is then to setup the
 * root inode and dentry so vfs can access the device.
 * If anything in the mount process goes wrong, we assume that the arguments
 * where invalid and we return -EINVAL. This is needed if Linux just tries out
 * if a device might be a specific filesystem and needs -EINVAL as a return
 * value to continue checking other filesystems.
 */
static int antfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct antfs_sb_info *sbi;
	int err = 0;
#ifdef CONFIG_AVM_ENHANCED
	char *loggername = "antfs_logger";
#endif

	sbi = kzalloc(sizeof(struct antfs_sb_info), GFP_KERNEL);
	if (!sbi) {
		err = -ENOMEM;
		goto out;
	}

	antfs_sbi_init(sb, sbi);

#ifdef CONFIG_AVM_ENHANCED
	/* set up the avm_logger to submit critical errors that corrupt the
	 * ntfs device
	 */
	if (!global_logger) {
		global_logger = avm_logger_create(PAGE_SIZE, loggername,
					logger_log_sd_dir);
	/* if we fails to create a logger, set it null so we still get printk */
		if (IS_ERR(global_logger))
			global_logger = NULL;
	}
#endif

	antfs_mnt_opts_init(sb, silent);
	antfs_parse_options(sbi, data);

	antfs_fill_super_operations(sb);
	err = antfs_open_device(sb);
	if (err)
		goto err;

	antfs_volume_init_complete(sbi);

	/* TODO: permission mapping doesn't really do anything right now, and
	 *       for the time we actually implement permission checking we
	 *       probably want to do it the proper way...
	 */
	/* --- antfs_permissions_mapping(sbi); --- */

	if (sb->s_flags & MS_MANDLOCK)
		goto err;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
	sb->s_flags &= ~MS_NOSEC;
#endif
	sb->s_flags |= MS_POSIXACL;

	/* TODO: something to think about: do we set sb->s_max_links? */
	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_time_gran = 1;

	err = antfs_inode_setup_root(sb);
	if (!err)
		goto out;
err:
	antfs_put_super(sb);
	if (!err)
		err = -EINVAL;

	antfs_log_info("could not mount antfs! err:%d", err);

out:
	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
/**
 * @brief mount function that is called by vfs
 *
 * @param fs_type	information about the filesystem to mount
 * @param flags		mount options for the device
 * @param dev_name	name of the device to mount (e.g. sda1)
 * @param raw_data	mount options that are filesystem specific (char *)
 *
 * @return dentry of root of the device
 *
 * antfs_dev_mount calls the linux function mount_bdev which calls the
 * provided function antfs_fill_super to mount the ntfs device. This is
 * the first function that is going to be called by the vfs to mount a
 * ntfs device.
 */
static struct dentry *antfs_dev_mount(struct file_system_type *fs_type,
				      int flags, const char *dev_name,
				      void *raw_data)
{
	return mount_bdev(fs_type, flags, dev_name, raw_data, antfs_fill_super);
}
#else
/**
 * @brief mount function that is called by vfs
 *
 * @param fs_type	information about the filesystem to mount
 * @param flags		mount options for the device
 * @param dev_name	name of the device to mount (e.g. sda1)
 * @param data		mount options that are filesystem specific (char *)
 * @param mnt		Pointer to vsfmount struct
 *
 * @return 0 if ok or negative error code
 *
 * antfs_get_sb calls the linux function get_sb_bdev which calls the
 * provided function antfs_fill_super to mount the ntfs device. This is
 * the first function that is going to be called by the vfs to mount a
 * ntfs device.
 */
static int antfs_get_sb(struct file_system_type *fs_type,
				      int flags, const char *dev_name,
				      void *data, struct vfsmount *mnt)
{
	return get_sb_bdev(fs_type, flags, dev_name, data, antfs_fill_super,
			mnt);
}
#endif

static struct file_system_type antfs_fs_type = {
	.owner = THIS_MODULE,
	.name = "antfs",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
	.mount = antfs_dev_mount,
#else
	.get_sb = antfs_get_sb,
#endif
	.kill_sb = kill_block_super,
	.fs_flags = FS_REQUIRES_DEV | FS_HAS_SUBTYPE,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
MODULE_ALIAS_FS("antfs");
#endif

/**
 * @brief initialization for inodes that have to happen only once
 *
 * @param foo	inode to initialize
 *
 * antfs_inode_init_once calls inode_init_once() to set up every inode
 * after its creation with the essentials that have to be set up only once
 * in the lifetime of an inode.
 */
static void antfs_inode_init_once(void *foo)
{
	struct inode *inode = foo;

	inode_init_once(inode);
}

/**
 * @brief init for the ntfs filesystem
 *
 * antfs_fs_init set's up the allocation size for inodes and registers
 * the ntfs driver for usage in linux. The allocation size for inodes is
 * crucial for the system since the vfs inode is embedded into the
 * antfs_inode_info struct which holds the ntfs_inode and the ntfs_attribute
 * for the given inode.
 */
static int __init antfs_fs_init(void)
{
	int err = 0;

	antfs_inode_cachep = kmem_cache_create("antfs_inode",
					       sizeof(struct antfs_inode_info),
					       0, SLAB_HWCACHE_ALIGN,
					       antfs_inode_init_once);

	if (!antfs_inode_cachep)
		goto out;

	err = register_filesystem(&antfs_fs_type);
	if (!err)
		goto out;

	kmem_cache_destroy(antfs_inode_cachep);
out:
	return err;
}

/**
 * @brief unregisters the ntfs filesystem
 *
 * antfs_fs_cleanup unregisters the ntfs filesystem and clears the
 * kmem_cache after all inodes are flushed.
 */
static void antfs_fs_cleanup(void)
{
	unregister_filesystem(&antfs_fs_type);

	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(antfs_inode_cachep);
}

/**
 * @brief entry point for the filesystem init
 *
 * antfs_init prints some information about the ntfs driver version and
 * starts antfs_fs_init() which registers the ntfs filesystem.
 */
static int __init antfs_init(void)
{
	int err = 0;

	pr_info("ANTFS Module: Version %s\n",
		ANTFS_VERSION);

	err = antfs_fs_init();

#ifdef CONFIG_AVM_ENHANCED
	global_logger = NULL;
#endif
	return err;
}

/**
 * @brief entry point for closing the filesystem
 *
 * antfs_exit prints a message that the filesystem gets shut down and starts
 * antfs_fs_cleannup() which will unregister the filesystem.
 */
static void __exit antfs_exit(void)
{
	pr_info("ANTFS Module unloaded.\n");

	antfs_fs_cleanup();

#ifdef CONFIG_AVM_ENHANCED
	avm_logger_close(global_logger);
#endif
}

module_init(antfs_init);
module_exit(antfs_exit);
