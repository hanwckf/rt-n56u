/**
 * file.c - NTFS Driver by AVM GmbH (ANTFS)
 *          Based on ntfs-3g
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
#include <linux/sched/task.h>
#endif
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/swap.h>
#include <linux/aio.h>
#include <linux/falloc.h>
#include <linux/mpage.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#include <linux/uio.h>
#else
#include <linux/socket.h> //memcpy_fromiovec
#endif

#include "dir.h"
#include "lcnalloc.h"
#include "attrib.h"

/**
 * @brief Open a file
 *
 * Calls generic VFS @ref generic_file_open
 *
 * @param inode
 * @param file
 *
 * @return 0 on success or negative error code
 */
static int antfs_open(struct inode *inode, struct file *file)
{
	/* Open means access --> dirty inode */
	ntfs_inode_mark_dirty(ANTFS_NI(inode));
	return generic_file_open(inode, file);
}

/**
 * @brief Writeback file specfic attributes and buffers
 *
 * @param file
 * @param start
 * @param end
 * @param datasync
 *
 * This function writes all modified data back to the filesystem.
 * Further, sync of MFT and LCN bitmap buffers.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int antfs_fsync(struct file *filp, struct dentry *dentry, int datasync)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0)
int antfs_fsync(struct file *filp, int datasync)
#else
int antfs_fsync(struct file *filp, loff_t start, loff_t end, int datasync)
#endif
{
	struct inode *inode = file_inode(filp);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	int err;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
	err = simple_fsync(filp, dentry, datasync);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0)
	err = generic_file_fsync(filp, datasync);
#else
	err = generic_file_fsync(filp, start, end, datasync);
#endif

	ntfs_inode_mark_dirty(ni);
	ntfs_inode_sync(ni);

	return err;
}

struct antfs_fill_data {
	struct file *file;
	struct ntfs_inode *ni;
	unsigned nr_pages;
};

/**
 * @brief reads content from the disk to a buffer
 *
 * @param ni	    ntfs inodes that holds the attributes
 * @param buf	    buffer that content should be written into
 * @param size	    amount of bytes to be transferred
 * @param offset    starting point to read from in the file
 *
 * @return amount of read bytes or error code
 *
 * antfs_read checks if the amount of data to be read and transferred to
 * @buf doesn't exceed the data size of the file. If everything is ok
 * ntfs_attr_pread() is being called until the file is completely read and
 * written into @buf. The total amount of read bytes is tracked and returned.
 */
static int antfs_read(struct ntfs_inode *ni, char *buf, size_t size,
		      off_t offset)
{
	int err = 0;
	s64 total = 0;
	s64 max_read;

	if (!size)
		goto exit;

	max_read = ANTFS_NA(ni)->data_size;
	if (offset + (off_t) size > max_read) {
		if (max_read < offset)
			goto ok;
		size = max_read - offset;
	}
	while (size > 0) {
		s64 ret = ntfs_attr_pread(ANTFS_NA(ni), offset, size,
					  buf + total);
		if (ret != (s64) size)
			antfs_log_error
			    ("ntfs_attr_pread error reading mft_no %llu at "
			     "offset %lld: %lld <> %lld",
			     ni->mft_no, (long long)offset,
			     (long long)size, (long long)ret);
		if (ret <= 0 || ret > (s64) size) {
			err = (ret < 0) ? ret : -EIO;
			goto exit;
		}
		size -= ret;
		offset += ret;
		total += ret;
	}
ok:
	err = total;
exit:
	antfs_log_leave("err=%d", err);
	return err;
}

/**
 * @brief reads and fills a page with the content from a file
 *
 * @param _data	    struct that keeps the needed info's for reading
 * @param page	    the page to be filled with the content of a file
 *
 * @return alwas 0
 *
 * antfs_readpages_fill is reading the content of the file which is specified
 * by its corresponding inode's held in @_data from the disk and writes it into
 * @page. The ntfs inode's size is adjusted if the file's size in the vfs inode
 * is denominated smaller than it is in reality. The page is set as uptodate if
 * the data was successfully read from the disk and transcribed into the page.
 * Otherwise the page is set erroneous, so that .readpage() can be used to try
 * again.
 * TODO: this is the slow path we have when we read compressed files. This needs
 *	 to be improved!
 */
static int antfs_readpages_fill(void *_data, struct page *page)
{
	struct antfs_fill_data *data = _data;
	struct ntfs_inode *ni = data->ni;
	int err;
	char *buf;

	/* Ganze page lesen antfs_send_readpages) */
	buf = kmap(page);
	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL))
		return -ERESTARTSYS;

	err = antfs_read(ni, buf, PAGE_SIZE, page_offset(page));
	mutex_unlock(&ni->ni_lock);
	kunmap(page);

	get_page(page);
	data->nr_pages--;

	if (err > 0)
		SetPageUptodate(page);
	else
		SetPageError(page);
	unlock_page(page);
	put_page(page);

	return 0;
}

/**
 * @brief Map a buffer head to a physical block on disk
 *
 * @param inode	    vfs inode of a file to get block for
 * @param iblock    logical block number inside this file
 * @param bh_result buffer_head that will be used for read/write
 * @param create    indicates if new blocks should be allocated (for writing)
 *
 * @return 0 if everything worked out, negative error code otherwise
 *
 * This function is used to map physical sectors/blocks on the backing block
 * device to logical blocks inside a file.
 * When the requested block is mapped in NTFS we try to get as many continuous
 * blocks as fit in bh_result->b_size or set b_size accordingly and
 * "buffer_boundary" is set if we don't find as many. bh_result is set to
 * "mapped" in this case.
 * If the blocks are marked as "hole", bh_size is set to the size of the hole
 * at most and not marked as mapped.
 * If the requested block is not mapped in NTFS and @ref create is set, new
 * blocks/clusters are allocated on disk.
 */
int antfs_get_block(struct inode *inode, sector_t iblock,
			   struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_attr *na = ANTFS_NA(ni);
	struct ntfs_volume *vol = ni->vol;
	unsigned int cb_diff_bits =
		vol->cluster_size_bits - sb->s_blocksize_bits;
	struct runlist_element *rl;
	sector_t first_blk = 0;
	size_t count = bh_result->b_size, total = 0;
	s64 pos = (iblock << sb->s_blocksize_bits), offs;
	VCN vcn = pos >> vol->cluster_size_bits;
	LCN lcn;
	int err = 0;
	/* Blocks offset into cluster. */
	int blk_offs = iblock & ((1 << cb_diff_bits) - 1);

	antfs_log_enter("blk %llu, pos %lld (vcn=%lld), b_size %zu",
			(unsigned long long)iblock,
			(long long)pos,
			(long long)vcn,
			count);

	/* TODO: Can this ever happen? */
	if (!count) {
		antfs_log_leave("!b_size");
		goto out_no_unlock;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out_no_unlock;
	}


	if (create) {
		LCN lcn;
		VCN alloc_vcn = na->allocated_size >> vol->cluster_size_bits;
		/* Check if we have to allocate a new cluster at the end of the
		 * file and if so, do it here.
		 */
		if (vcn >= alloc_vcn) {
			if (vcn > alloc_vcn) {
				antfs_log_debug("Need to truncate: 0x%llx to "
						"0x%llx",
						(long long)alloc_vcn,
						(long long)vcn);
				/* truncate to VCN in front of cluster we want
				 * to allocate --> truncate to vcn.
				 */
				err = ntfs_attr_truncate(na,
						vcn << vol->cluster_size_bits);
				if (err) {
					antfs_log_warning("Could not truncate "
							"data_size: %d", err);
					goto out;
				}
			}
			err = antfs_do_cluster_alloc(ni, na, vcn, 1, &lcn);
			if (err) {
				antfs_log_warning("Cluster alloc failed: %d",
						err);
				goto out;
			}
			first_blk = (lcn << cb_diff_bits) + blk_offs;
			map_bh(bh_result, sb, first_blk);
			set_buffer_new(bh_result);
			antfs_log_debug("Mapped to 0x%llx",
					(long long)first_blk);
			goto out;
		}
	} else {
		/* Truncate reads beyond end of attribute */
		/* i_size is incremented in write_end generic */
		if (pos + count > inode->i_size) {
			if (pos >= inode->i_size) {
				err = 0;
				goto out;
			} else {
				/* ...to EOF and not more */
				antfs_log_debug("trunc count from %d to %d",
						(int)count,
						(int)(inode->i_size - pos));
				count = inode->i_size - pos;
			}
		}

		/* efs_raw-stuff is not processed here */

		/* Truncate buffer for reads beyond initialized size. */
		/* FIXME: Can this ever happen? */
		if (unlikely(pos + count > na->initialized_size)) {
			if (pos >= na->initialized_size) {
				antfs_log_debug("pos (%lld) + count (%lld) "
						"(= %lld) > max_init (%lld)",
						(long long)pos,
						(long long)count,
						(long long)(pos + count),
						(long long)na->
						initialized_size);
				goto out;
			}
			antfs_log_debug("blk %u truncate count from %zu to "
					"%lld",
					(unsigned int)iblock, count,
					(long long)na->initialized_size - pos);
			count = na->initialized_size - pos;
		}
	}

	/* Code inspired from attrib.c, ntfs_attr_pread_i */

	/* Find the runlist element containing the vcn. */
	rl = ntfs_attr_find_vcn(na, vcn);
	if (IS_ERR_OR_NULL(rl)) {
		/*
		 * If the vcn is not present it is an out of bounds read.
		 * However, we already truncated the read to the data_size,
		 * so getting this here is an error.
		 */
		err = PTR_ERR(rl);
		if (err == -ENOENT)
			antfs_log_error("Failed to find VCN #1");
		goto out;
	}

	/*
	 * Gather the requested block number. Try to get as many contiguous
	 * blocks as fit in b_size. Note, a partial final vcn is taken care of
	 * by the @count capping of read length.
	 */
	antfs_log_debug("Going to look up blocks in rl");
	for (offs = vcn - rl->vcn;
			count;
			rl++, offs = 0, blk_offs = 0, vcn = rl->vcn) {
		sector_t blk;
		s64 alloc_cnt;

		if (rl->lcn == LCN_RL_NOT_MAPPED) {
			rl = ntfs_attr_find_vcn(na, rl->vcn);
			if (IS_ERR(rl)) {
				err = PTR_ERR(rl);
				if (err == -ENOENT)
					antfs_log_error
						("Failed to find VCN #2");
				goto rl_err_out;
			}
			vcn = rl->vcn;
		}

		if (!rl->length) {
			antfs_log_error("Unexpected runlength 0");
			goto rl_err_out;
		}

		if (rl->lcn < (LCN) 0) {
			if (rl->lcn < (LCN) LCN_HOLE) {
				antfs_log_error("Bad run (%lld)", rl->lcn);
				goto rl_err_out;
			}
			/* Got a hole. */
			if (total) {
				/* We already mapped something:
				 * Break here since we likely get a
				 * discontiguity when allocating new blocks
				 * anyway and merging stuff here makes it more
				 * complicated.
				 * For reading we can't map real data and signal
				 * a hole at the same time.
				 */
				break;
			}

			if (create) {
				/* For writing: Insert a new run that maps the
				 * requested cluster(s).
				 */
				alloc_cnt = max((int)(count >>
							vol->cluster_size_bits),
							1);
				alloc_cnt = min_t(s64, rl->length, alloc_cnt);

				/* Force cluster allocation inside the hole.
				 * This splits/replaces the hole.
				 */
				err = antfs_do_cluster_alloc(ni, na, vcn,
						alloc_cnt, &lcn);
				if (err) {
					antfs_log_error("Cluster allocation in "
							"hole failed: %d", err);
					goto rl_err_out;
				}
				/* After stuffing a hole we very likely get a
				 * discontiguity. */
				alloc_cnt <<= vol->cluster_size_bits;
				if (count <= alloc_cnt) {
					count = 0;
				} else {
					count -= alloc_cnt;
					total = alloc_cnt;
				}
				first_blk = (lcn << cb_diff_bits) + blk_offs;
				set_buffer_new(bh_result);
				set_buffer_zeronew(bh_result);
				break;

			} else {
				/* For reading:
				 * Either map everything up to the hole or
				 * don't map anything and set b_size to end of
				 * hole at most.
				 */
				alloc_cnt =
					rl->length << vol->cluster_size_bits;
				if (count <= alloc_cnt) {
					count = 0;
				} else {
					count -= alloc_cnt;
					total = alloc_cnt;
				}
				break;
			}
		}

		blk = ((rl->lcn + offs) << cb_diff_bits) + blk_offs;
		if (first_blk) {
			if (blk != first_blk +
					(total >> sb->s_blocksize_bits)) {
				/* Next block in rl is somewhere else. Stop. */
				antfs_log_debug
					("Combo breaker: got %llu, "
					 "expexted %llu",
					 (long long)blk,
					 (long long)(first_blk +
					 (total >> sb->s_blocksize_bits)));
				set_buffer_boundary(bh_result);
				break;
			}
		} else {
			first_blk = blk;
		}

		alloc_cnt = (rl->length << vol->cluster_size_bits) -
			(blk_offs << sb->s_blocksize_bits);
		if (count <= alloc_cnt) {
			count = 0;
			break;
		}

		count -= alloc_cnt;
		total += alloc_cnt;
	}

	/* Did not map the whole b_size. Need to update. */
	if (total)
		bh_result->b_size = total;

	if (first_blk)
		map_bh(bh_result, sb, first_blk);

	/* TODO: set_buffer_new when beyond initialized_size? */

	antfs_log_debug("b_size = %zu; blk = %llu", bh_result->b_size,
			(long long)first_blk);
out:
	mutex_unlock(&ni->ni_lock);
out_no_unlock:
	antfs_log_leave("Exit: %d", err);
	return err;

rl_err_out:
	mutex_unlock(&ni->ni_lock);
	antfs_log_leave("rl_err_out");
	return -EIO;
}

/**
 * @brief reads one page from a given file from the disk
 *
 * @param file	file in question which needs to be read
 * @param page	the page to be filled by the contents of the file
 *
 * @return 0 if everything worked out, error code otherwise
 *
 * antfs_readpage reads a single @page from a @file. After validating that
 * a unnamed AT_DATA attribute corresponding to the file is present, the file
 * gets read in two possible ways. @page gets directly filled if the file has
 * a compressed, resident or encrypted attribute through antfs_read(), or
 * antfs_get_block() is used for every other @file.
 */
static int antfs_readpage(struct file *file __attribute__((unused)),
			  struct page *page)
{
	struct inode *inode = page->mapping->host;
	struct ntfs_inode *ni;
	size_t count = PAGE_SIZE;
	loff_t pos = page_offset(page);
	char *buf;
	int err;

	antfs_log_enter();
	err = -EIO;
	if (unlikely(is_bad_inode(inode)))
		goto out;

	ni = ANTFS_NI(inode);
	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}

	/* Directly fill pages when reading compressed or resident
	 * or encrypted attributes. */
	if ((ANTFS_NA(ni)->data_flags & ATTR_COMPRESSION_MASK) ||
	    !NAttrNonResident(ANTFS_NA(ni)) ||
	    (ni->vol->efs_raw &&
	     (ANTFS_NA(ni)->data_flags & ATTR_IS_ENCRYPTED))) {
		buf = kmap(page);
		err = antfs_read(ni, buf, count, pos);
		mutex_unlock(&ni->ni_lock);
		/* TODO: Is this needed here? */
		/*--- flush_dcache_page(page); ---*/
		kunmap(page);
		if (err > 0)
			err = 0;
		unlock_page(page);
		SetPageUptodate(page);
	} else {
		mutex_unlock(&ni->ni_lock);
		err = mpage_readpage(page, antfs_get_block);
	}

out:
	antfs_log_leave("err=%d", err);
	return err;
}

/**
 * @brief reads multiple pages from the disk
 *
 * @param file	    file to read from the disk
 * @param mapping   address space of the file to read
 * @param pages	    list head of the pages to be filled with the file's content
 * @param nr_pages  amount of pages to be read
 *
 * @return 0 if everything worked out, otherwise an error code
 *
 * antfs_readpages gets called by the vfs to read a file from the disk. It is
 * possible to read more than one page at once by specifying @nr_pages.
 * The first step is to get the corresponding ntfs_inode and unnamed AT_DATA
 * attribute for the corresponding file. There are two ways to proceed from here
 * on. The pages will get directly filled if @file has a compressed, resident or
 * encrypted attribute. antfs_readpages_fill() is used for that getting all
 * the needed information passed through a antfs_fill_data struct, @pages and
 * @mapping. For every other @file generic @ref mpage_readpages with
 * @ref antfs_get_block() is used.
 */
static int antfs_readpages(struct file *file, struct address_space *mapping,
			   struct list_head *pages, unsigned nr_pages)
{
	struct antfs_fill_data afd;
	struct ntfs_inode *ni = ANTFS_NI(mapping->host);
	unsigned int page_offs = ANTFS_NA(ni)->initialized_size &
				    (PAGE_SIZE - 1);
	const unsigned long buffer_len = 1 << mapping->host->i_blkbits;
	int err = -EIO;

	/* TODO: Debugging - remove later. */
	if (!ni) {
		char tmpstr[64];

		antfs_log_error("!ni! ino: %lu; s_magic: %lu; dev: %s%s",
				mapping->host->i_ino,
				mapping->host->i_sb->s_magic,
				bdevname(mapping->host->i_sb->s_bdev, tmpstr),
				is_bad_inode(mapping->host) ? "; bad inode" : ""
				);
		BUG();
	}

	/* Directly fill pages when reading compressed or resident
	 * or encrypted attributes. */
	if ((ANTFS_NA(ni)->data_flags & ATTR_COMPRESSION_MASK) ||
	    !NAttrNonResident(ANTFS_NA(ni)) ||
	    (ni->vol->efs_raw &&
	    (ANTFS_NA(ni)->data_flags & ATTR_IS_ENCRYPTED))) {

		afd.file = file;
		afd.ni = ni;
		afd.nr_pages = nr_pages;
		err =
		    read_cache_pages(mapping, pages, antfs_readpages_fill,
				     &afd);
	} else {
		struct page *page;
		unsigned page_idx = nr_pages;
		pgoff_t page_idx_to_init;
		bool do_init_page = false;

		if (page_offs & (buffer_len - 1)) {
			/* If initialized size is not on buffer boundary, walk
			 * all requested pages and check if there is a page
			 * that contains a block parts in front and behind
			 * initialized_data that would need initialization.
			 */
			list_for_each_entry(page, pages, lru) {
				s64 page_addr;

				if (!page_idx)
					break;
				--page_idx;

				page_addr = page->index << PAGE_SHIFT;
				if (page_addr > ANTFS_NA(ni)->initialized_size
				    || page_addr + PAGE_SIZE <=
				    ANTFS_NA(ni)->initialized_size)
					continue;

				/* "Es kann nur einen geben!" */
				antfs_log_debug("Got page to init "
						"@index 0x%llx; "
						"page_addr=0x%llx; "
						"init_size=0x%llx",
						(long long)page->index,
						(long long)page_addr,
						(long long)ANTFS_NA(ni)->
						initialized_size);
				page_idx_to_init = page->index;
				do_init_page = true;
				break;
			}
		}

		err = mpage_readpages(mapping, pages, nr_pages,
				antfs_get_block);
		if (!err && do_init_page) {
			/* Initialize stuff past initialized_size with zero. */
			page = grab_cache_page(mapping, page_idx_to_init);

			if (!page)
				return -ENOMEM;

			antfs_log_debug("Zero @page_offs 0x%llx, len 0x%x, "
					"buffer_len 0x%x",
					(long long)page_offs,
					(int)(buffer_len - (page_offs &
							(buffer_len - 1))),
					(int)buffer_len);
			/* Only need to initialize to buffer boundary. Later
			 * buffers should not be mapped and get initialized
			 * elsewhere.
			 */
			zero_user(page, page_offs, buffer_len -
					(page_offs & (buffer_len - 1)));
			unlock_page(page);
			put_page(page);
		}
	}

	return err;
}

/**
 * @brief performs the actual writing back to the disk
 *
 * @param page	    page of the file to write
 * @param wbc	    control struct for the writing process
 *
 * @return 0 if everything worked out, error code otherwise
 */
static int antfs_writepage(struct page *page, struct writeback_control *wbc)
{
	return block_write_full_page(page, antfs_get_block, wbc);
}

/**
 * @brief performs the actual writing back to the disk
 *
 * @param mapping   address space of the file to write
 * @param wbc	    control struct for the writing process
 *
 * @return 0 if everything worked out, error code otherwise
 *
 * antfs_writepages gets called after all bh's are commited to the bio and
 * the actual writing process should be performed. All data in memory is
 * uptodate and should be written back how it is. We use mpage_writepages()
 * to actually perform the write for non resident data attributes.
 */
static int antfs_writepages(struct address_space *mapping,
			    struct writeback_control *wbc)
{
	return mpage_writepages(mapping, wbc, antfs_get_block);
}

/**
 * @brief Return logical block number to virtual block number in mapping
 *
 * @param mapping  Mapping (e.g. file) to get logical block number to
 * @param block    Virtual block number (relative to start of mapping)
 *                 to get logical block number for (relative to start of
 *                 backing device)
 *
 * @return Logical block number or 0
 *
 * @note @ref antfs_get_block may fail, but since it doesn't map anything
 *       in this case, this is ok.
 *
 * TODO: Untested! Any nasty races with truncation?
 */
static sector_t antfs_bmap(struct address_space *mapping, sector_t block)
{
	return generic_block_bmap(mapping, block, antfs_get_block);
}

/**
 * antfs_zero_page - Zero (parts of) a page
 *
 * @page:      Page to work on
 * @mapping:   Address space mapping to use
 * @offset:    Start offset into page to start zeroing
 * @len:       Number of bytes to zero
 * @base_blk:  1st block of this page on volume
 *
 * Zeros a range of bytes inside a page. Takes care that buffers in page get
 * mapped (relative to @ref base_blk) if they are changed and get set dirty and
 * uptodate.
 */
static void antfs_zero_page(struct page *page, struct address_space *mapping,
		const loff_t offset, const loff_t len, const sector_t base_blk)
{
	struct buffer_head *bh_start, *bh;
	struct super_block *sb = mapping->host->i_sb;
	const unsigned int blkbits = mapping->host->i_blkbits;
	bool fully_mapped = true;
	bool all_uptodate = true;

	if (!page_has_buffers(page))
		create_empty_buffers(page, 1 << blkbits, 0);
	bh = bh_start = page_buffers(page);
	antfs_log_enter("Zero for idx 0x%llx; offs=0x%llx, len=0x%llx, "
			"base_blk=0x%llx",
			(long long)page->index, (long long)offset,
			(long long)len, (long long)base_blk);
	zero_user(page, offset, len);
	do {
		if ((bh_offset(bh) < offset + len) &&
				(bh_offset(bh) + bh->b_size > offset)) {
			set_buffer_uptodate(bh);
			if (!buffer_mapped(bh)) {
				map_bh(bh, sb, base_blk + (bh_offset(bh) >>
						blkbits));
				antfs_log_debug("Map to 0x%llx",
						(long long)bh->b_blocknr);
			}
			mark_buffer_dirty(bh);
		} else {
			fully_mapped = false;
			if (!buffer_uptodate(bh))
				all_uptodate = false;
		}
		bh = bh->b_this_page;
	} while (bh != bh_start);

	if (all_uptodate)
		SetPageUptodate(page);
	if (fully_mapped)
		SetPageMappedToDisk(page);
	antfs_log_leave();
}

/**
 * antfs_zero_clusters_on_rl  - Zero a range of clusters for a NTFS inode
 *
 * @ni:     NTFS inode to zero clusters on
 * @from:   Offset in inode to start zeroing in byte
 * @to:     Offset in inode to stop zeroing in byte (exclusive)
 *
 * Zero a range of bytes in a NTFS inode. This is meant to be used to
 * initialize any space between last initialized_size and last write_position
 * in @antfs_write_end. For this purpose we assume that if we meet a hole
 * in the given range, it should stretch to the last allocated cluster that
 * must be initialized elsewhere.
 *
 * Note: The range must not cover the latest allocated cluster that might
 *       contain the page we hold in @antfs_write_end. See note in
 *       @antfs_zero_space.
 */
static void antfs_zero_clusters_on_rl(struct ntfs_inode *ni,
		loff_t from, loff_t to)
{
	struct ntfs_volume *vol = ni->vol;
	struct runlist_element *rl = ANTFS_NA(ni)->rl;
	struct address_space *mapping = ANTFS_I(ni)->i_mapping;
	const unsigned int blkbits = mapping->host->i_blkbits;
	const sector_t blk_per_page = 1 << (PAGE_SHIFT - blkbits);

	antfs_log_enter("from=0x%llx; to=0x%llx",
			(long long)from, (long long)to);

	if (from >= to)
		return;

	if (!rl) {
		antfs_log_warning("In write_end but without runlist?!"
				" ino %lld", (long long)ni->mft_no);
		return;
	}

	/* Skip to run containing start offset. */
	for (; rl->length && rl->vcn + rl->length <=
			from >> vol->cluster_size_bits; rl++)
		;

	antfs_log_debug("Am @vcn 0x%llx; lcn 0x%llx; len 0x%llx",
			rl->vcn, rl->lcn, rl->length);
	/* If we run into a hole after our start offset, exit.
	 * This should stretch to end_offs.
	 */
	for (; rl->length && from < to && rl->lcn >= 0; ++rl) {
		loff_t offset, zero_len, next_from;
		pgoff_t index;
		sector_t base_blk;
		unsigned int pg_offs;

		/* Offset in current run */
		offset = from - (rl->vcn << vol->cluster_size_bits);
		/* Start offset for next run: */
		next_from = (rl->vcn + rl->length) << vol->cluster_size_bits;
		/* Number of bytes to zero in this run: */
		zero_len = min_t(loff_t, to - from, next_from - from);
		/* "from" to page index: */
		index = from >> PAGE_SHIFT;
		/* And the base blk for this run from lcn: */
		base_blk = rl->lcn << (vol->cluster_size_bits - blkbits);
		/* Add offset to page start in run: */
		base_blk += (offset & ~(loff_t)(PAGE_SIZE - 1)) >> blkbits;

		/* Offset into 1st page: */
		pg_offs = from & (PAGE_SIZE - 1);

		while (zero_len) {
			struct page *page = grab_cache_page(mapping, index);
			loff_t pg_zero_len = min_t(loff_t,
					PAGE_SIZE - pg_offs, zero_len);

			if (!page)
				break;

			antfs_zero_page(page, mapping, pg_offs, pg_zero_len,
					base_blk);

			unlock_page(page);
			put_page(page);

			pg_offs = 0;
			zero_len -= pg_zero_len;
			base_blk += blk_per_page;
			++index;
		}

		from = next_from;
	}

	antfs_log_leave();
}

/**
 * antfs_zero_cluster - Zero a range of bytes in a NTFS file clusters
 *
 * @ni:        NTFS inode to work on
 * @curr_page: Pointer to a page that we already hold (e.g. in
 *                      @antfs_write_end)
 * @from:      Start offset in bytes in file data
 * @to:        Offset to stop zeroing in bytes in file data (exclusive)
 *
 * Zeros the range @from to (exclusive) @to in file data associated with
 * given NTFS inode.
 * This can be used to initialize space in new allocated clusters with zero.
 *
 * Note: Make sure @from and @to don't cross cluster boundaries as we
 *       don't load cluster mappings from runlist here.
 */
static void antfs_zero_cluster(struct ntfs_inode *ni, struct page *curr_page,
		loff_t from, loff_t to)
{
	struct ntfs_volume *vol = ni->vol;
	struct address_space *mapping = ANTFS_I(ni)->i_mapping;
	struct buffer_head *bh, *bh_start;
	const unsigned int blkbits = mapping->host->i_blkbits;
	const sector_t cluster_blk_mask = (1 <<
			(vol->cluster_size_bits - blkbits)) - 1;
	sector_t curr_cluster_startblk = 0;

	antfs_log_enter("from 0x%llx to 0x%llx, ino %lld",
			(long long)from, (long long)to,
			(long long)ni->mft_no);

	if (from >= to)
		return;

	bh = bh_start = page_buffers(curr_page);
	do {
		if (buffer_mapped(bh)) {
			/* bring this block to cluster boundary */
			curr_cluster_startblk = bh->b_blocknr &
				~cluster_blk_mask;
			break;
		}
		bh = bh->b_this_page;
	} while (bh != bh_start);

	if (!curr_cluster_startblk) {
		antfs_log_error("No buffers mapped in curr_page?! Cannot "
				"get position on disk!");
		return;
	}

	while (from < to) {
		pgoff_t index = from >> PAGE_SHIFT;
		struct page *page = index == curr_page->index ? curr_page :
			grab_cache_page(mapping, index);
		loff_t offset = from & (PAGE_SIZE - 1);
		loff_t len = min_t(loff_t, to - from, PAGE_SIZE - offset);

		if (page) {
			antfs_zero_page(page, mapping, offset, len,
					curr_cluster_startblk +
					((index << (PAGE_SHIFT - blkbits)) &
					 cluster_blk_mask));

			if (page != curr_page) {
				unlock_page(page);
				put_page(page);
			}
		} else {
			antfs_log_error("Failed to zero page @blk 0x%llx",
					(long long)index <<
					(blkbits - PAGE_SHIFT));
		}
		from += len;
	}

	antfs_log_leave();
}
/**
 * @brief Truncate page cache after failed write opeTruncate page cache after
 *	  failed write operations
 */
static void antfs_write_failed(struct address_space *mapping, loff_t to)
{
	struct inode *inode = mapping->host;

	if (to > inode->i_size) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0)
		truncate_pagecache(inode, to, inode->i_size);
#else
		truncate_pagecache(inode, inode->i_size);
#endif
	}
}

/**
 * @brief prepares the writing of a block on a ntfs device
 *
 * @param filp	    file in question to write to
 * @param mapping   address space of the file in question
 * @param pos	    position where to start the writing
 * @param len	    length of the changes to write
 * @param flags	    0 or @ref AOP_FLAG_UNINTERRUPTIBLE
 *                  Set for writes with source in kernel address space
 *                  to prevent short writes.
 * @param pagep	    pointer to pages to be used
 * @param fsdata    private data pointer (unused)
 *
 * @return 0 if everything worked out, negative error code otherwise
 *
 * This prepares the a page to receive data that is going to be written to disk.
 * For most part it is a wrapper for @ref block_write_begin.
 * We only get here for non-resident files (resident files are made non-resident
 * in @ref antfs_aio_write -- writing to resident files may be handleded there
 * as well).
 * What this does is make sure we opened the AT_DATA attribute for the file and
 * and prepare/allocate a single block on disk if needed.
 *
 * @note File truncation is done by setting the file size in @ref antfs_setattr
 *       function. File size only goes up here.
 *
 * @note If the actual writing malfunctioned after we successfully allocated the
 *       space on disk we know for sure, that we can write the needed amount of
 *       bytes back to disk! So even if we fail during the writing process, the
 *       vfs will repeat till it works!
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static int antfs_write_begin(struct file *filp, struct address_space *mapping,
			     loff_t pos, unsigned len, unsigned flags,
			     struct page **pagep,
			     void **fsdata __attribute__((unused)))
#else
static int antfs_write_begin(struct file *filp, struct address_space *mapping,
			     loff_t pos, unsigned len, unsigned flags,
			     struct page **pagep,
			     void **fsdata)
#endif
{
	struct inode *inode = file_inode(filp);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	int err = 0;

	antfs_log_enter("inode %ld", file_inode(filp)->i_ino);

	if (NInoTestAndSetWritePending(ni)) {
		err = -EAGAIN;
		goto out;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	err = block_write_begin(mapping, pos, len, flags, pagep,
				antfs_get_block);
#else
	*pagep = NULL;
	err = block_write_begin(filp, mapping, pos, len, flags, pagep,
				fsdata, antfs_get_block);
#endif
	if (err < 0) {
		if (err != -ENOSPC)
			antfs_log_error("Write_begin failed: %d", err);
		antfs_write_failed(mapping, pos + len);
		NInoClearWritePending(ni);
	}

out:
	antfs_log_leave("err: %d", err);
	return err;
}

/**
 * @brief finishes up the writing of a block on a ntfs device
 *
 * @param filp	    file in question to write to
 * @param mapping   address space of the file in question
 * @param pos	    position where to start the writing
 * @param len	    length of the changes to write
 * @param copied    amount of bytes copied to the block
 * @param pagep	    pointer to pages to be used
 * @param fsdata    private data pointer (unused)
 *
 * @return amount of bytes written
 *
 * antfs_write_end is finishing up the process of writing to a specific block
 * of @file which @pos belongs to that was started with @ref antfs_write_begin.
 * As with @ref antfs_write_begin this function gets only called for resident
 * file attributes.
 * The generic_write_end() function is doing the vfs related finish ups, like
 * increasing the i_size in case of appending to a file and returns the
 * actual amount of bytes written.
 * This process can fail and needs to be checked, how many bytes we actually
 * have written. In case of a fail the whole process is started again with
 * calling @ref antfs_write_begin. Either for the same page (if nothing was
 * copied) or for a different one if we copied at least parts.
 *
 * After a successful write operation we need to check if we appended to the
 * file and change its initialized size attribute and write it back to disk.
 * (Currently we only do this on inode release).
 *
 * @NOTE:   We haven't written anything back to disk so far, all the work is
 *	    done later by antfs_writepages(). Which will simply write the
 *	    pages back to disk. Since this is done through a bio and bh's it
 *	    will happen after all the file's changes are comitted.
 */
static int antfs_write_end(struct file *filp, struct address_space *mapping,
			   loff_t pos, unsigned len, unsigned copied,
			   struct page *page, void *fsdata)
{
	struct inode *inode = file_inode(filp);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_attr *na = ANTFS_NA(ni);
	struct ntfs_volume *vol = ni->vol;
	bool got_buffer_exhole = false;
	const loff_t ncluster_mask = ~((loff_t)vol->cluster_size - 1);
	struct buffer_head *bh, *head = page_buffers(page);
	s64 newsize;
	int err;

	antfs_log_enter("ino %lld; pos=0x%llx; copied=0x%llx",
			(long long)ni->mft_no,
			(long long)pos,
			(long long)copied);
	err = generic_write_end(filp, mapping, pos, len, copied, page, fsdata);
	if (err < 0 || (unsigned int)err < len) {
		antfs_write_failed(mapping, pos + len);
		antfs_log_error("Write end failed!");
		goto out_unlocked;
	}

	/* Test if we have BUFFER_ZERONEW flags here. This is a strong hint that
	 * we may have to initialize a hole.
	 */
	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out_unlocked;
	}

	bh = head;
	do {
		if (test_clear_buffer_zeronew(bh)) {
			got_buffer_exhole = true;
			antfs_log_debug("Buffer zeronew@ %lld",
					(long long)bh->b_blocknr);
			break;
		}
		bh = bh->b_this_page;
	} while (head != bh);

	/* Initialize clusters with zero if needed. */
	newsize = pos + copied;
	if (pos >= na->initialized_size) {
		/* If we wrote past initialized_size, we only need to
		 * initialize the space before pos and nothing after.
		 */
		if ((pos & ncluster_mask) < na->initialized_size) {
			/* We did not allocate new clusters. Initialize space
			 * in current cluster up to pos.
			 */
			antfs_log_debug("Zero cluster 0?");
			antfs_zero_cluster(ni, page, na->initialized_size, pos);
		} else {
			/*
			 * Any previous clusters that need initialization?
			 * Look through runlist.
			 */
			antfs_log_debug("Zero cluster 1?");
			antfs_zero_clusters_on_rl(ni,
					na->initialized_size,
					pos & ncluster_mask);
			/* Need to initialize start of the current cluster? */
			antfs_zero_cluster(ni, page, pos & ncluster_mask, pos);
		}
	} else if (got_buffer_exhole) {
		/* If we wrote in front of initialized_size and have buffers
		 * marked as new we are writing into a former hole.
		 * Have to zero the (w)hole thing around the part that was
		 * written.
		 */
		antfs_log_debug("Zero cluster 2?");
		antfs_zero_cluster(ni, page, pos & ncluster_mask, pos);
		if (newsize & (vol->cluster_size - 1))
			antfs_zero_cluster(ni, page, newsize,
					(newsize & ncluster_mask) +
					vol->cluster_size);
	}

	/* Last: Update initialized size and data_size. */
	if (newsize > na->initialized_size)
		na->initialized_size = newsize;

	if (newsize > na->data_size) {
		na->data_size = newsize;

		if (na->type == AT_DATA
				&& na->name == AT_UNNAMED) {
			/* Set the inode dirty so it is written out
			 * later.
			 */
			ntfs_inode_mark_dirty(na->ni);
			NInoFileNameSetDirty(na->ni);
		} else {
			antfs_log_error("Got na not AT_DATA (0x%02x) and "
					"AT_UNNAMED (??) for ino %lld",
					(int)le32_to_cpu(na->type),
					(long long)ni->mft_no);
		}
	}

	mutex_unlock(&ni->ni_lock);
out_unlocked:
	NInoClearWritePending(ni);
	/* Done! */
	return err;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
static ssize_t antfs_aio_write(struct kiocb *iocb, const struct iovec *_iov,
			       unsigned long nr_segs, loff_t pos)
#else
static ssize_t antfs_file_write_iter(struct kiocb *iocb, struct iov_iter *from)
#endif
{
	struct inode *inode = file_inode(iocb->ki_filp);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_attr *na = ANTFS_NA(ni);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	struct iovec *iov = (struct iovec *)_iov;
#endif
	int err = 0;
#ifdef ANTFS_EARLY_BLALLOC
	struct ntfs_volume *vol = ni->vol;
	int blocksize_bits = inode->i_sb->s_blocksize_bits;
	int cb_diff_bits = vol->cluster_size_bits - blocksize_bits;
	size_t ocount;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
	loff_t pos;
#endif
#endif

	antfs_log_enter();

	/* In case we still get here for compressed files: error out */
	if (na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_ENCRYPTED)) {
		/* File is compressed --> RO */
		antfs_log_debug("Compressed/encrypted --> read only!");
		err = -EPERM;
		goto out;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}

	/* TODO: Add write support for compressed files */
	if (unlikely(!NAttrNonResident(na))) {
		struct file *file = iocb->ki_filp;
		struct ntfs_attr_search_ctx *ctx;
		size_t count;
		s64 rpos = (s64) iocb->ki_pos;
		char *val;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
		generic_segment_checks(iov, &nr_segs, &count, VERIFY_READ);
		if (file->f_flags & O_APPEND) {
#else
		count = iov_iter_count(from);
		if (iocb->ki_flags & IOCB_APPEND) {
#endif
		/* case appending: we wanna start copying from the end of the
		 * file -> rpos = file size
		 */
			rpos = (loff_t)na->data_size;
		}

		err = ntfs_attr_truncate(na, rpos + count);
		if (err) {
			antfs_log_error("Failed to extend data attribute!");
			mutex_unlock(&ni->ni_lock);
			goto out;
		}
		if (NAttrNonResident(na)) {
			/* Truncate already fixed the attribute and made it
			 * nonresident. We only have to continue like we were a
			 * nonresident attribute all along.
			 */
			goto unlock_mutex;
		}

		/* write content directly into the data attribute */
		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (IS_ERR(ctx)) {
			mutex_unlock(&ni->ni_lock);
			err = PTR_ERR(ctx);
			goto out;
		}

		err = ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				       0, NULL, 0, ctx);
		if (err) {
			ntfs_attr_put_search_ctx(ctx);
			mutex_unlock(&ni->ni_lock);
			goto out;
		}
		val = (char *)ctx->attr + le16_to_cpu(ctx->attr->value_offset);
		if (val < (char *)ctx->attr || val +
		    le32_to_cpu(ctx->attr->value_length) >
		    (char *)ctx->mrec + na->ni->vol->mft_record_size) {
			err = -ERANGE;
			/* log? sanity check failed! */
			antfs_log_error("Data attribute outside of mft record");
			mutex_unlock(&ni->ni_lock);
			goto out;
		}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
		err = memcpy_fromiovec(val + rpos, iov, count);
		if (err < 0) {
			antfs_log_error("Failed to copy data to buffer!");
			if (file->f_flags & O_APPEND) {
#else
		err = copy_from_iter(val + rpos, count, from);
		if (err != count) {
			antfs_log_error("Failed to copy data to buffer!");
			if (iocb->ki_flags & IOCB_APPEND) {
#endif
				/* shrink back to original size, this should not
				 * fail if nothing really bad happens!
				 */
				err = ntfs_attr_truncate(na, inode->i_size);
			} else {
				/* file is filled with carbage now! vfs sets
				 * file size to 0, so we should do the same!
				 */
				err = ntfs_attr_truncate(na, 0);
			}
			if (err)
				antfs_log_critical("Corrupted file");
			mutex_unlock(&ni->ni_lock);
			err = -EFAULT;
			goto out;
		}
		ntfs_attr_put_search_ctx(ctx);
		mutex_unlock(&ni->ni_lock);

		/* make sure no bullshit might be synced back with a possible
		 * page that still contains old data
		 */
		truncate_inode_pages(file->f_mapping, 0);
		i_size_write(inode, na->data_size);

		/* we wrote count bytes */
		err = count;
		if (likely(count > 0))
			iocb->ki_pos += count;
		goto out;
	} else {
unlock_mutex:
		mutex_unlock(&ni->ni_lock);
	}
#ifdef ANTFS_EARLY_BLALLOC
	/* early alloc clusters: For transfers > 2 clusters, allocate stuff here
	 * This should reduce fragmentation and speed things up a bit. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	generic_segment_checks(iov, &nr_segs, &ocount, VERIFY_READ);
#else
	pos = iocb->ki_pos;
	#error "TODO: Need a way to get total data size from iov_iter."
	/*--- ocount = ---*/
#endif
	ocount >>= blocksize_bits;
	if (ocount >> cb_diff_bits > 2) {
		err =
		    antfs_alloc_blocks(inode, pos >> blocksize_bits, ocount,
				       NULL);
		if (err) {
			antfs_log_error("early alloc blocks failed: %d", err);
			return err;
		}
	}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	err = generic_file_aio_write(iocb, iov, nr_segs, pos);
#else
	err = generic_file_write_iter(iocb, from);
#endif
out:
	antfs_log_leave("err: %d", err);
	return err;
}

static ssize_t antfs_splice_write(struct pipe_inode_info *pipe,
				  struct file *out, loff_t *ppos, size_t len,
				  unsigned int flags)
{
	struct inode *inode = file_inode(out);
	struct ntfs_inode *ni = ANTFS_NI(inode);
#ifdef ANTFS_EARLY_BLALLOC
	int blocksize_bits = inode->i_sb->s_blocksize_bits;
#endif
	struct ntfs_attr *na = ANTFS_NA(ni);
	int err = 0;

	antfs_log_enter();
	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}

	/* In case we still get here for compressed files: error out */
	if (na->data_flags & (ATTR_COMPRESSION_MASK | ATTR_IS_ENCRYPTED)) {
		/* File is compressed --> RO */
		antfs_log_error("Compressed/encrypted --> read only!");
		err = -EPERM;
		goto out_locked;
	}

#ifdef ANTFS_EARLY_BLALLOC
	/* early alloc clusters: For transfers > 2 clusters, allocate stuff here
	 * This should reduce fragmentation and speed things up a bit. */
	if (len >> blocksize_bits > 2) {
		err =
		    antfs_alloc_blocks(inode, *ppos >> blocksize_bits,
				       len >> blocksize_bits, NULL);
		if (err) {
			antfs_log_error("early alloc blocks failed: %d", err);
			goto out_locked;
		}
	}
#endif
	mutex_unlock(&ni->ni_lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)
	err = generic_file_splice_write(pipe, out, ppos, len, flags);
#else
	err = iter_file_splice_write(pipe, out, ppos, len, flags);
#endif
out:
	antfs_log_leave("err: %d", err);
	return err;

out_locked:
	mutex_unlock(&ni->ni_lock);
	goto out;
}

/**
 * @brief (De)Allocate clusters in a file
 *
 * @param file      file to work on
 * @param mode      one of the modes defined in fallocate.h
 *                  Currently only a value of 0 and FALLOCATE_FL_KEEP_SIZE
 *                  is supported which means allocate @length bytes at @offset.
 *                  If FALLOCATE_FL_KEEP_SIZE is set, dont update data_size
 *                  and inode->i_size.
 * @param offset    Offset where we have to (de)allocate clusters.
 *                  Currently only offset 0 is supported.
 * @param length    Length of the area in bytes
 *
 * Currently this only supports allocating new clusters for files shorter than
 * @offset + @length and with an @offset of 0.
 *
 * @return 0 on success or negative error code
 */
static long antfs_file_fallocate(struct file *file, int mode, loff_t offset,
				 loff_t length)
{
	struct inode *inode = file_inode(file);
	struct ntfs_inode *ni = ANTFS_NI(inode);
	struct ntfs_attr *na = ANTFS_NA(ni);
	s64 old_data_size = na->data_size;
	int err = 0;

	antfs_log_enter("ino %llu mode %x offset %llu len %llu", ni->mft_no,
			mode, offset, length);


	if (mode & ~FALLOC_FL_KEEP_SIZE ||
	    offset > 0 ||
	    offset + length < na->allocated_size ||
	    offset + length < na->data_size ||
	    !S_ISREG(inode->i_mode)) {
		err = -EOPNOTSUPP;
		goto out;
	}

	if (mutex_lock_interruptible_nested(&ni->ni_lock, NI_MUTEX_NORMAL)) {
		err = -ERESTARTSYS;
		goto out;
	}

	err = ntfs_attr_truncate_i(na, offset + length, HOLES_NO);
	mutex_unlock(&ni->ni_lock);

	if (err)
		goto out;

	if (mode & FALLOC_FL_KEEP_SIZE)
		na->data_size = old_data_size;

	i_size_write(inode, na->data_size);
	mark_inode_dirty(inode);

out:
	antfs_log_leave("err: %d", err);
	return err;
}

static const struct file_operations antfs_file_operations = {
	.llseek = generic_file_llseek,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
	.read = do_sync_read,
	.write = do_sync_write,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	.aio_read = generic_file_aio_read,
	.aio_write = antfs_aio_write,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	.read_iter = generic_file_read_iter,
	.write_iter = antfs_file_write_iter,
#endif
	.mmap = generic_file_mmap,
	.open = antfs_open,
	.fsync = antfs_fsync,
	.splice_read = generic_file_splice_read,
	.splice_write = antfs_splice_write,
	.fallocate = antfs_file_fallocate,
};

static const struct address_space_operations antfs_file_aops = {
	.readpage = antfs_readpage,
	.readpages = antfs_readpages,
	.writepage = antfs_writepage,
	.writepages = antfs_writepages,
	.write_begin = antfs_write_begin,
	.write_end = antfs_write_end,
	.bmap = antfs_bmap,
};

/**
 * @brief sets file_operations of a given iode
 *
 * @param inode	the vfs inode in need of file_operations
 *
 * antfs_init_file_inode gets called during the initialization of an inode and
 * sets the inode's i_fop and i_fata.a_ops operations.
 * NOTE:    the caller must ensure that the inode represents a regular file and
 *	    not a directory since we don't check here anymore.
 */
void antfs_inode_init_file(struct inode *inode)
{
	inode->i_fop = &antfs_file_operations;
	inode->i_data.a_ops = &antfs_file_aops;
}
