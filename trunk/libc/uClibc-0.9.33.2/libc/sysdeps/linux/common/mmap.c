/* vi: set sw=4 ts=4: */
/*
 * mmap() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bits/uClibc_page.h>

#ifdef __NR_mmap


#ifdef __UCLIBC_MMAP_HAS_6_ARGS__

_syscall6(void *, mmap, void *, start, size_t, length,
		int, prot, int, flags, int, fd, off_t, offset)

#else

# define __NR__mmap __NR_mmap
static __inline__ _syscall1(__ptr_t, _mmap, unsigned long *, buffer)
__ptr_t mmap(__ptr_t addr, size_t len, int prot,
			 int flags, int fd, __off_t offset)
{
	unsigned long buffer[6];

	buffer[0] = (unsigned long) addr;
	buffer[1] = (unsigned long) len;
	buffer[2] = (unsigned long) prot;
	buffer[3] = (unsigned long) flags;
	buffer[4] = (unsigned long) fd;
	buffer[5] = (unsigned long) offset;
	return (__ptr_t) _mmap(buffer);
}

#endif

libc_hidden_def(mmap)

#elif defined(__NR_mmap2)


#define __NR___syscall_mmap2 __NR_mmap2
static __inline__ _syscall6(__ptr_t, __syscall_mmap2, __ptr_t, addr,
	size_t, len, int, prot, int, flags, int, fd, off_t, offset)

/* Some architectures always use 12 as page shift for mmap2() eventhough the
 * real PAGE_SHIFT != 12.  Other architectures use the same value as
 * PAGE_SHIFT...
 */
# ifndef MMAP2_PAGE_SHIFT
#  define MMAP2_PAGE_SHIFT 12
# endif

__ptr_t mmap(__ptr_t addr, size_t len, int prot, int flags, int fd, __off_t offset)
{
	if (offset & ((1 << MMAP2_PAGE_SHIFT) - 1)) {
		__set_errno(EINVAL);
		return MAP_FAILED;
	}
	return __syscall_mmap2(addr, len, prot, flags, fd, offset >> MMAP2_PAGE_SHIFT);
}

libc_hidden_def(mmap)

#endif
