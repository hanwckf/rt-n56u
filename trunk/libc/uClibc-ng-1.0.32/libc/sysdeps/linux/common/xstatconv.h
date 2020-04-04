/* Convert between the kernel's `struct stat' format, and libc's.
   Copyright (C) 1991,1995,1996,1997,2000,2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.

   Modified for uClibc by Erik Andersen <andersen@codepoet.org>
   */

/* Pull in whatever this particular arch's kernel thinks the kernel version of
 * struct stat should look like.  It turns out that each arch has a different
 * opinion on the subject, and different kernel revs use different names... */
#include <features.h>
#include <bits/kernel_stat.h>

#ifdef __ARCH_HAS_DEPRECATED_SYSCALLS__
extern void __xstat_conv(struct kernel_stat *kbuf, struct stat *buf) attribute_hidden;
extern void __xstat32_conv(struct kernel_stat64 *kbuf, struct stat *buf) attribute_hidden;
extern void __xstat64_conv(struct kernel_stat64 *kbuf, struct stat64 *buf) attribute_hidden;
#endif

