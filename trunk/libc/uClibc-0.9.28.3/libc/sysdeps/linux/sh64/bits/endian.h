/*
 * libc/sysdeps/linux/sh64/bits/endian.h
 *
 * Copyright (C) 2003  Paul Mundt
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#ifdef __LITTLE_ENDIAN__
#  define __BYTE_ORDER __LITTLE_ENDIAN
#else
#  define __BYTE_ORDER __BIG_ENDIAN
#endif

