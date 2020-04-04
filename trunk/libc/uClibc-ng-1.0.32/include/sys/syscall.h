/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYSCALL_H
#define _SYSCALL_H	1

/* User application code should use syscall(). */

#include <features.h>
#include <bits/sysnum.h>
#ifdef _LIBC
/* The _syscall#() macros are for uClibc internal use only.
 *
 * The kernel provided _syscall[0-6] macros from asm/unistd.h are not suitable
 * for use in uClibc as they lack PIC support etc, so for uClibc we use our own
 * local _syscall# macros to be certain all such variations are handled
 * properly.
 */
# include <bits/syscalls.h>
# include <bits/syscalls-common.h>
#endif

#endif
