/*
 * inotify interface for uClibc
 *
 * Copyright (C) 2006 Austin Morgan <admorgan@morgancomputers.net>
 * Copyright (C) 2006 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/inotify.h>

#ifdef __NR_inotify_init
_syscall0(int, inotify_init)
#endif

#ifdef __NR_inotify_init1
_syscall1(int, inotify_init1, int, flags)
#endif

#if defined __NR_inotify_init1 && !defined __NR_inotify_init
int inotify_init(void)
{
	return INLINE_SYSCALL(inotify_init1, 1, 0);
}
#endif

#ifdef __NR_inotify_add_watch
_syscall3(int, inotify_add_watch, int, fd, const char *, path, uint32_t, mask)
#endif

#ifdef __NR_inotify_rm_watch
_syscall2(int, inotify_rm_watch, int, fd, int, wd)
#endif
