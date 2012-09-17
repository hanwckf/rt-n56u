/* vi: set sw=4 ts=4: */
/*
 * geteuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_geteuid32
# undef __NR_geteuid
# define __NR_geteuid __NR_geteuid32
#endif

#ifdef __NR_geteuid
_syscall_noerr0(uid_t, geteuid)
libc_hidden_def(geteuid)
#endif
