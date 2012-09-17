/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>


static _syscall6(__ptr_t, mmap2, __ptr_t, addr, size_t, len, int, prot,
		 int, flags, int, fd, __off_t, pgoff)

__ptr_t mmap(__ptr_t addr, size_t len, int prot, int flags, int fd, __off_t offset)
{
	unsigned long page_size = sysconf(_SC_PAGESIZE);
	unsigned long pgoff;

	if (offset & (page_size - 1)) {
		__set_errno(EINVAL);
		return MAP_FAILED;
	}

	pgoff = (unsigned long)offset >> (31 - __builtin_clz(page_size));

	return mmap2(addr, len, prot, flags, fd, pgoff);
}
libc_hidden_def(mmap)
