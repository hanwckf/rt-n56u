/* Copyright (C) 1991, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <_lfs_64.h>

#include <sys/types.h>
#include <sys/resource.h>
#include <bits/wordsize.h>

/* the regular getrlimit will work just fine for 64bit users */

#if defined __UCLIBC_HAS_LFS__ && __WORDSIZE == 32


/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  */
int getrlimit64 (__rlimit_resource_t resource, struct rlimit64 *rlimits)
{
    struct rlimit rlimits32;

    if (getrlimit (resource, &rlimits32) < 0)
	return -1;

    if (rlimits32.rlim_cur == RLIM_INFINITY)
	rlimits->rlim_cur = RLIM64_INFINITY;
    else
	rlimits->rlim_cur = rlimits32.rlim_cur;
    if (rlimits32.rlim_max == RLIM_INFINITY)
	rlimits->rlim_max = RLIM64_INFINITY;
    else
	rlimits->rlim_max = rlimits32.rlim_max;

    return 0;
}
#endif
