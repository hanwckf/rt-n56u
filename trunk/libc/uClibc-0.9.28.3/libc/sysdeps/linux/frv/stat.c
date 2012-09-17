/*
 * Syscalls for uClibc
 *
 * Copyright (C) 2001-2003 by Erik Andersen
 * Extracted from ../common/syscalls.c by Erik Andersen <andersen@codepoet.org>
 * Adapted to FR-V by Alexandre Oliva <aoliva@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <features.h>
#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 *  *  * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <endian.h>

#include <unistd.h>
#define _SYS_STAT_H
#include <bits/stat.h>

#define __NR___syscall_stat __NR_stat
_syscall2(int, __syscall_stat, const char *, file_name, struct stat *, buf);
strong_alias(__syscall_stat, stat)
