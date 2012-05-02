/* vi: set sw=4 ts=4: */
/*
 * umount() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"


/* arch provides umount() syscall */
#ifdef __NR_umount

#include <sys/mount.h>
_syscall1(int, umount, const char *, specialfile);


/* arch provides umount2() syscall */
#elif defined __NR_umount2

#define __NR___syscall_umount2 __NR_umount2
static inline _syscall2(int, __syscall_umount2, const char *, special_file, int, flags);

int umount(const char *special_file)
{
	return (__syscall_umount2(special_file, 0));
}


/* arch doesn't provide any umount syscall !? */
#else

int umount(const char *special_file)
{
	__set_errno(ENOSYS);
	return -1;
}

#endif
