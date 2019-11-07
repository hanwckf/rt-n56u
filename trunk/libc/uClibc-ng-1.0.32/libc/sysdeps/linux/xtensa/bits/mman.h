/* Definitions for POSIX memory map interface.  Linux/Xtensa version.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_MMAN_H
# error "Never use <bits/mman.h> directly; include <sys/mman.h> instead."
#endif

/* These are Linux-specific.  */
#ifdef __USE_MISC
# define MAP_NORESERVE	0x0400		/* Don't check for reservations.  */
# define MAP_GROWSDOWN	0x1000		/* Stack-like segment.  */
# define MAP_DENYWRITE	0x2000		/* ETXTBSY  */
# define MAP_EXECUTABLE	0x4000		/* Mark it as an executable.  */
# define MAP_LOCKED	0x8000		/* Lock the mapping.  */
# define MAP_POPULATE	0x10000		/* Populate (prefault) pagetables.  */
# define MAP_NONBLOCK	0x20000		/* Do not block on IO.  */
# define MAP_STACK	0x40000		/* Allocation is for a stack.  */
# define MAP_HUGETLB	0x80000		/* Create huge page mapping.  */
# define MAP_UNINITIALIZED 0x4000000    /* For anonymous mmap, memory could -   					   be uninitialized.  */
#endif

#define __MAP_ANONYMOUS	0x0800		/* Don't use a file  */

/* Include generic Linux declarations.  */
#include <bits/mman-linux.h>

#ifdef __USE_MISC
# define MAP_RENAME	MAP_ANONYMOUS
#endif
