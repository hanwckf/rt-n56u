/*
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _UCLIBC_PAGE_H
#define _UCLIBC_PAGE_H

/*
 * AARCH64 supports 4k, 16k, 64k pages (build time).
 */

#include <features.h>

#if defined(__CONFIG_AARCH64_PAGE_SIZE_64K__)
#define PAGE_SHIFT		16
#elif defined(__CONFIG_AARCH64_PAGE_SIZE_16K__)
#define PAGE_SHIFT		14
#else
#define PAGE_SHIFT		12
#endif

#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#endif /* _UCLIBC_PAGE_H */
