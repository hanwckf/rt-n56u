/*
 * getgid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_getxgid
# undef __NR_getgid
# define __NR_getgid __NR_getxgid
#endif
#ifdef __NR_getgid32
# undef __NR_getgid
# define __NR_getgid __NR_getgid32
#endif

_syscall_noerr0(gid_t, getgid)
libc_hidden_def(getgid)
#if !defined __NR_getegid32 && !defined __NR_getegid
strong_alias(getgid,getegid)
libc_hidden_def(getegid)
#endif
