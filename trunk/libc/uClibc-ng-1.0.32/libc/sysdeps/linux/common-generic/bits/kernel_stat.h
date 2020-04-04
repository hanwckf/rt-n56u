/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

#include <sys/stat.h>

/*
 * The stat structure defined in
 * libc/sysdeps/linux/common-generic/bits/stat.h
 * is the same as the kernel one for new architectures
 *
 * For the common-generic ABI we really don't need this file at all
 * However that requires more #ifndef in relevant wrappers,
 * further uglifying them
 */
#define kernel_stat64	stat

#endif	/*  _BITS_STAT_STRUCT_H */

