/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <endian.h>

#ifdef __NR_pread64
# define __NR_pread __NR_pread64
#endif

#ifdef __NR_pread
# define __NR___syscall_pread __NR_pread
static _syscall6(ssize_t, __syscall_pread, int, fd, void *, buf,
		 size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo)
# define MY_PREAD(fd, buf, count, offset) \
	__syscall_pread(fd, buf, count, 0, OFF_HI_LO(offset))
# define MY_PREAD64(fd, buf, count, offset) \
	__syscall_pread(fd, buf, count, 0, OFF64_HI_LO(offset))
#endif

#ifdef __NR_pwrite64
# define __NR_pwrite __NR_pwrite64
#endif

#ifdef __NR_pwrite
# define __NR___syscall_pwrite __NR_pwrite
static _syscall6(ssize_t, __syscall_pwrite, int, fd, const void *, buf,
		 size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo)
# define MY_PWRITE(fd, buf, count, offset) \
	__syscall_pwrite(fd, buf, count, 0, OFF_HI_LO(offset))
# define MY_PWRITE64(fd, buf, count, offset) \
	__syscall_pwrite(fd, buf, count, 0, OFF64_HI_LO(offset))
#endif

#include "../common/pread_write.c"
