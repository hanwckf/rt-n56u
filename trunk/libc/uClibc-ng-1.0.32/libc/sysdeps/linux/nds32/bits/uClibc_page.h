/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _UCLIBC_PAGE_H
#define _UCLIBC_PAGE_H

/*
 * Linux/NDS32 supports 4k and 8k pages (build time).
 *
 * Although uClibc determines page size dynamically from kernel's auxv
 * still the generic code needs a fall back
 *  _dl_pagesize = auxvt[AT_PAGESZ].a_un.a_val ? : PAGE_SIZE
 */

#include <features.h>

#if defined(__CONFIG_NDS32_PAGE_SIZE_8K__)
#define PAGE_SHIFT		13
#else
#define PAGE_SHIFT		12
#endif

#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

/* TBD: fix this with runtime value for a PAGE_SIZE agnostic uClibc */
#define MMAP2_PAGE_SHIFT PAGE_SHIFT

#endif /* _UCLIBC_PAGE_H */
