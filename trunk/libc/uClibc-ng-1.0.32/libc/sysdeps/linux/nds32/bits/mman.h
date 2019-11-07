/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Definitions for POSIX memory map interface.  Linux/NDS32 version.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_MMAN_H
# error "Never use <bits/mman.h> directly; include <sys/mman.h> instead."
#endif

/* The following definitions basically come from the kernel headers.
   But the kernel header is not namespace clean.  */

/* These are Linux-specific.  */
#ifdef __USE_MISC
# define MAP_GROWSDOWN	0x0100		/* stack-like segment */
# define MAP_DENYWRITE	0x0800		/* ETXTBSY */
# define MAP_EXECUTABLE	0x1000		/* mark it as an executable */
# define MAP_LOCKED	0x2000		/* pages are locked */
# define MAP_NORESERVE  0x4000          /* don't check for reservations */
# define MAP_POPULATE   0x08000         /* populate (prefault) pagetables */
# define MAP_NONBLOCK   0x10000         /* do not block on IO */
# define MAP_STACK	0x20000		/* Allocation is for a stack.  */
# define MAP_HUGETLB	0x40000		/* Create huge page mapping.  */
#endif

/* Include generic Linux declarations.  */
#include <bits/mman-linux.h>
