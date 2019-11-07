/* Copyright (C) 1996, 1999 Free Software Foundation, Inc.
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

/* Interfaces to control the various kernel daemons.  */

#ifndef _SYS_KDAEMON_H

#define _SYS_KDAEMON_H	1
#include <features.h>

__BEGIN_DECLS

/* Start, flush, or tune the kernel's buffer flushing daemon.  */
extern int bdflush (int __func, long int __data) __THROW;

__END_DECLS

#endif /* _SYS_KDAEMON_H */
