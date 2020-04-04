/*
 * umount() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#include <sys/mount.h>
#ifdef __NR_umount
_syscall1(int, umount, const char *, specialfile)
#elif defined __NR_umount2
# ifndef  __UCLIBC_LINUX_SPECIFIC__
static __always_inline _syscall2(int, umount2, const char *, special_file, int, flags)
# endif
int umount(const char *special_file)
{
	return umount2(special_file, 0);
}
#endif
