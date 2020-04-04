/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _UCLIBC_PAGE_H
#define _UCLIBC_PAGE_H

/*
 * ARC700/linux supports 4k, 8k, 16k pages (build time).
 *
 * Although uClibc determines page size dynamically, from kernel's auxv which
 * ARC Linux does pass, still the generic code needs a fall back
 *  _dl_pagesize = auxvt[AT_PAGESZ].a_un.a_val ? : PAGE_SIZE
 *
 */

#include <features.h>

#if defined(__CONFIG_ARC_PAGE_SIZE_16K__)
#define PAGE_SHIFT		14
#elif defined(__CONFIG_ARC_PAGE_SIZE_4K__)
#define PAGE_SHIFT		12
#else
#define PAGE_SHIFT		13
#endif

#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

/* TBD: fix this with runtime value for a PAGE_SIZE agnostic uClibc */
#define MMAP2_PAGE_SHIFT PAGE_SHIFT

#endif /* _UCLIBC_PAGE_H */
