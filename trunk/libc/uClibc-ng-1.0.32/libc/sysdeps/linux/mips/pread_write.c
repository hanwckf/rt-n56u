/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <endian.h>
#include <sgidefs.h>

/* We should generalize this for 32bit userlands w/64bit regs.  This applies
 * to the x86_64 x32 and the mips n32 ABIs.  */
#if _MIPS_SIM == _MIPS_SIM_NABI32
# define __NR___syscall_pread __NR_pread64
static _syscall4(ssize_t, __syscall_pread, int, fd, void *, buf, size_t, count, off_t, offset)
# define MY_PREAD(fd, buf, count, offset) \
	__syscall_pread(fd, buf, count, offset)
# define MY_PREAD64(fd, buf, count, offset) \
	__syscall_pread(fd, buf, count, offset)

# define __NR___syscall_pwrite __NR_pwrite64
static _syscall4(ssize_t, __syscall_pwrite, int, fd, const void *, buf, size_t, count, off_t, offset)
# define MY_PWRITE(fd, buf, count, offset) \
	__syscall_pwrite(fd, buf, count, offset)
# define MY_PWRITE64(fd, buf, count, offset) \
	__syscall_pwrite(fd, buf, count, offset)
#endif

#include "../common/pread_write.c"
