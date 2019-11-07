/*
 * Syscalls for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
/* Extracted from ../common/syscalls.c by Erik Andersen <andersen@codepoet.org>
 * Adapted to FR-V by Alexandre Oliva <aoliva@redhat.com>
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <sys/stat.h>

_syscall2(int, stat64, const char *, file_name, struct stat64 *, buf)
libc_hidden_def(stat64)
