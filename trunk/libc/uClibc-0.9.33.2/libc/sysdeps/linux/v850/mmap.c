/* Use new style mmap for v850 */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/syscall.h>


_syscall6 (__ptr_t, mmap, __ptr_t, addr, size_t, len, int, prot,
	   int, flags, int, fd, __off_t, offset)
libc_hidden_def(mmap)
