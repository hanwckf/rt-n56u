/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#if !defined __UCLIBC_LINUX_SPECIFIC__
#undef __NR_setresuid
#undef __NR_setresuid32
#endif

int seteuid(uid_t uid)
{
    int result;

    if (uid == (uid_t) ~0)
    {
	__set_errno (EINVAL);
	return -1;
    }

#if (defined __NR_setresuid || defined __NR_setresuid32) && defined __USE_GNU
    result = setresuid(-1, uid, -1);
    if (result == -1 && errno == ENOSYS)
	/* Will also set the saved user ID if euid != uid,
	 * making it impossible to switch back...*/
#endif
	result = setreuid(-1, uid);

    return result;
}
libc_hidden_def(seteuid)
