/*
 * remap_file_pages() for uClibc
 *
 * Copyright (C) 2008 Will Newton <will.newton@imgtec.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <sys/mman.h>
#include <sys/syscall.h>

#ifdef __NR_remap_file_pages

_syscall5(int, remap_file_pages, void *, __start, size_t, __size,
			int, __prot, size_t, __pgoff, int, __flags)

#endif
