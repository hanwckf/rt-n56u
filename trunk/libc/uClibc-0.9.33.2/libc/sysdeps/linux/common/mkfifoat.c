/*
 * mkfifoat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef __NR_mknodat
int mkfifoat(int fd, const char *path, mode_t mode)
{
	return mknodat(fd, path, mode | S_IFIFO, 0);
}
#else
/* should add emulation with mkfifo() and /proc/self/fd/ ... */
#endif
