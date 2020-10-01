/**
 * misc.c : miscellaneous :
 *		- dealing with errors in memory allocation
 *
 * Copyright (c) 2008 Jean-Pierre Andre
 * Copyright (c) 2016 Jens Krieg
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

#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/highmem.h>

#include "antfs.h"
#include "types.h"
#include "misc.h"

/**
 * ntfs_malloc
 *
 * @param size	Size in bytes to allocate
 *
 * Return a pointer to the allocated memory or NULL if the request fails.
 */
void *ntfs_malloc(size_t size)
{
	if (size <= PAGE_SIZE) {
		/* kmalloc() has per-CPU caches so is faster for now. */
		return kmalloc(size, GFP_KERNEL);
	}

	return __vmalloc(size, GFP_KERNEL, PAGE_KERNEL);
}

/**
 * ntfs_calloc
 *
 * @param size	Size in bytes to allocate
 *
 * Return a pointer to the allocated memory or NULL if the request fails.
 */
void *ntfs_calloc(size_t size)
{
	void *p;

	p = kcalloc(1, size, GFP_KERNEL);
	if (!p) {
		antfs_log_debug("<ERROR> Failed to calloc %lld bytes",
				(long long)size);
	}

	return p;
}

void *ntfs_realloc(void *ptr, size_t size)
{
	if (is_vmalloc_addr(ptr)) {
		antfs_log_error("Cannot vmrealloc");
		return NULL;
	}

	return krealloc(ptr, size, GFP_KERNEL);
}

void ntfs_free(void *addr)
{
	if (!addr)
		return;
	if (!is_vmalloc_addr(addr)) {
		kfree(addr);
		return;
	}
	vfree(addr);
}
