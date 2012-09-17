/* vi: set sw=4 ts=4: */
/*
 * Common header file for uClibc syscalls
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define _LARGEFILE64_SOURCE
#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <endian.h>

#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 * being renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif

#include <bits/kernel_types.h>
