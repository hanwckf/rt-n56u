/*
 * fchownat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_fchownat
_syscall5(int, fchownat, int, fd, const char *, file, uid_t, owner, gid_t, group, int, flag)
libc_hidden_def(fchownat)
#else
/* should add emulation with fchown() and /proc/self/fd/ ... */
#endif
