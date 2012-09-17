/* vi: set sw=4 ts=4: */
/*
 * getuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_getxuid
# undef __NR_getuid
# define __NR_getuid __NR_getxuid
#endif
#ifdef __NR_getuid32
# undef __NR_getuid
# define __NR_getuid __NR_getuid32
#endif

_syscall_noerr0(uid_t, getuid)
libc_hidden_def(getuid)
#if !defined __NR_geteuid32 && !defined __NR_geteuid
strong_alias(getuid,geteuid)
libc_hidden_def(geteuid)
#endif
