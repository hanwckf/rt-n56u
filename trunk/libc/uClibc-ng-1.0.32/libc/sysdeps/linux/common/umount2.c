/*
 * umount2() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __UCLIBC_LINUX_SPECIFIC__ && defined __NR_umount2
#include <sys/mount.h>
_syscall2(int, umount2, const char *, special_file, int, flags)
libc_hidden_def(umount2)
#endif

#if defined __UCLIBC_LINUX_SPECIFIC__ && defined __NR_oldumount
_syscall2(int, umount, const char *, special_file, int, flags)
strong_alias(umount,umount2)
#endif
