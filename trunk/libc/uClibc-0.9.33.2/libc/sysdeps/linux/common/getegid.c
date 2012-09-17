/* vi: set sw=4 ts=4: */
/*
 * getegid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_getegid32
# undef __NR_getegid
# define __NR_getegid __NR_getegid32
#endif

#ifdef __NR_getegid
_syscall_noerr0(gid_t, getegid)
libc_hidden_def(getegid)
#endif
