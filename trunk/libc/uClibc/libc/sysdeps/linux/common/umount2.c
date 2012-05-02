/* vi: set sw=4 ts=4: */
/*
 * umount2() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#ifdef __NR_umount2	/* Old kernels don't have umount2 */
#include <sys/mount.h>
_syscall2(int, umount2, const char *, special_file, int, flags);
#else
int umount2(const char *special_file, int flags)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif
