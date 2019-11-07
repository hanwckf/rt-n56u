/*
 * mmap() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/mman.h>
#include <sys/syscall.h>

#if defined __UCLIBC_MMAP_HAS_6_ARGS__ && defined __NR_mmap

# ifndef _syscall6
#  error disable __UCLIBC_MMAP_HAS_6_ARGS__ for this arch
# endif

# define __NR__mmap __NR_mmap
static _syscall6(void *, _mmap, void *, addr, size_t, len,
		 int, prot, int, flags, int, fd, __off_t, offset)

#elif defined __NR_mmap2 && defined _syscall6


# include <errno.h>
# include <bits/uClibc_page.h>
# ifndef MMAP2_PAGE_SHIFT
#  define MMAP2_PAGE_SHIFT 12
# endif

# define __NR___syscall_mmap2 __NR_mmap2
static __inline__ _syscall6(void *, __syscall_mmap2, void *, addr, size_t, len,
			    int, prot, int, flags, int, fd, __off_t, offset)

static void *_mmap(void *addr, size_t len, int prot, int flags,
		   int fd, __off_t offset)
{
	const int mmap2_shift = MMAP2_PAGE_SHIFT;
	const __off_t mmap2_mask = ((__off_t) 1 << MMAP2_PAGE_SHIFT) - 1;
	/* check if offset is page aligned */
	if (offset & mmap2_mask) {
		__set_errno(EINVAL);
		return MAP_FAILED;
	}
# ifdef __USE_FILE_OFFSET64
	return __syscall_mmap2(addr, len, prot, flags, fd,
				((__u_quad_t) offset >> mmap2_shift));
# else
	return __syscall_mmap2(addr, len, prot, flags, fd,
				((__u_long) offset >> mmap2_shift));
# endif
}

#elif defined __NR_mmap
# define __NR___syscall_mmap __NR_mmap
static __inline__ _syscall1(void *, __syscall_mmap, unsigned long *, buffer)

static void *_mmap(void *addr, size_t len, int prot, int flags,
		   int fd, __off_t offset)
{
	unsigned long buffer[6];

	buffer[0] = (unsigned long) addr;
	buffer[1] = (unsigned long) len;
	buffer[2] = (unsigned long) prot;
	buffer[3] = (unsigned long) flags;
	buffer[4] = (unsigned long) fd;
	buffer[5] = (unsigned long) offset;
	return __syscall_mmap(buffer);
}

#else

# error "Your architecture doesn't seem to provide mmap() !?"

#endif

strong_alias(_mmap,mmap)
libc_hidden_def(mmap)
