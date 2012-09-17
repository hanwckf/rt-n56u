/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/syscall.h>


#if !defined __UCLIBC_LINUX_SPECIFIC__
#undef __NR_setresgid
#undef __NR_setresgid32
#endif

int setegid(gid_t gid)
{
    int result;

    if (gid == (gid_t) ~0)
    {
	__set_errno (EINVAL);
	return -1;
    }

#if (defined __NR_setresgid || defined __NR_setresgid32) && defined __USE_GNU
    result = setresgid(-1, gid, -1);
    if (result == -1 && errno == ENOSYS)
	/* Will also set the saved group ID if egid != gid,
	 * making it impossible to switch back...*/
#endif
	result = setregid(-1, gid);

    return result;
}
