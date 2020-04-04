/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_MMAN_H
# error "Never use <bits/mman.h> directly; include <sys/mman.h> instead."
#endif

/* The following definitions basically come from the kernel headers.
   But the kernel header is not namespace clean.  */

#ifdef __USE_MISC
/* These are Linux-specific.  */
# define MAP_NONBLOCK	0x00080		/* Do not block on IO.  */
# define MAP_GROWSDOWN	0x00100		/* Stack-like segment.  */
# define MAP_STACK	MAP_GROWSDOWN	/* Provide convenience alias.  */
# define MAP_LOCKED	0x00200		/* Lock the mapping.  */
# define MAP_NORESERVE	0x00400		/* Don't check for reservations.  */
# define MAP_DENYWRITE	0x00800		/* ETXTBSY */
# define MAP_EXECUTABLE	0x01000		/* Mark it as an executable.  */
# define MAP_POPULATE	0x00040		/* Populate (prefault) pagetables.  */
# define MAP_HUGETLB	0x04000		/* Create huge page mapping.  */
#endif

/* Include generic Linux declarations.  */
#include <bits/mman-linux.h>
