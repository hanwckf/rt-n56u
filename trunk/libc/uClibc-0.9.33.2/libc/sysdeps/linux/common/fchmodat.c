/*
 * fchmodat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 * Copyright (C) 2012 Mike Frysinger
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef __NR_fchmodat
/*
 * The kernel takes 3 args, but userland takes 4.
 * We have to process all the flags ourselves.
 */
int fchmodat(int fd, const char *file, mode_t mode, int flag)
{
	/* We only support one flag atm ... */
	if (flag & ~AT_SYMLINK_NOFOLLOW) {
		__set_errno(EINVAL);
		return -1;
	}

	/* ... but Linux doesn't support perms on symlinks. */
	if (flag & AT_SYMLINK_NOFOLLOW) {
		__set_errno(ENOTSUP);
		return -1;
	}

	return INLINE_SYSCALL(fchmodat, 3, fd, file, mode);
}
#else
/* should add emulation with fchmod() and /proc/self/fd/ ... */
#endif
