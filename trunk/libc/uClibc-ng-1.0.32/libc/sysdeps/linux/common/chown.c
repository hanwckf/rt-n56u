/*
 * chown() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <bits/wordsize.h>

#if defined __NR_fchownat && !defined __NR_chown
# include <fcntl.h>
int chown(const char *path, uid_t owner, gid_t group)
{
	return fchownat(AT_FDCWD, path, owner, group, 0);
}

#else

# if (__WORDSIZE == 32 && defined(__NR_chown32)) || __WORDSIZE == 64
#  ifdef __NR_chown32
#   undef __NR_chown
#   define __NR_chown __NR_chown32
#  endif

_syscall3(int, chown, const char *, path, uid_t, owner, gid_t, group)

# else

#  define __NR___syscall_chown __NR_chown
static __inline__ _syscall3(int, __syscall_chown, const char *, path,
		__kernel_uid_t, owner, __kernel_gid_t, group)

int chown(const char *path, uid_t owner, gid_t group)
{
	if (((owner + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((group + 1) > (gid_t) ((__kernel_gid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_chown(path, owner, group));
}
# endif

#endif
libc_hidden_def(chown)
