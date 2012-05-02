/* vi: set sw=4 ts=4: */
/*
 * inotify interface for uClibc
 *
 * Copyright (C) 2006 Austin Morgan <admorgan@morgancomputers.net>
 * Copyright (C) 2006 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "syscalls.h"
#include <sys/inotify.h>

#ifdef __NR_inotify_init
_syscall0(int, inotify_init);
#endif

#ifdef __NR_inotify_add_watch
_syscall3(int, inotify_add_watch, int, fd, const char *, path, uint32_t, mask);
#endif

#ifdef __NR_inotify_rm_watch
_syscall2(int, inotify_rm_watch, int, fd, uint32_t, wd);
#endif
