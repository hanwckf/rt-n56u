/* Copyright (C) 1991, 1994-1998, 2001-2002 Free Software Foundation, Inc.
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

/*#define __UCLIBC_HIDE_DEPRECATED__*/
#include <signal.h>

#include "sigset-cvt-mask.h"

/* Block signals in MASK, returning the old mask.  */
static int sigblock (int mask)
{
  sigset_t set, oset;

  sigset_set_old_mask (&set, mask);
  sigprocmask (SIG_BLOCK, &set, &oset); /* can't fail */
  return sigset_get_old_mask (&oset);
}
