/* Determine whether block of given size can be allocated on the stack or not.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <alloca.h>
#include <stdlib.h>
#include <sys/param.h>
#include "internals.h"
#include <sysdep-cancel.h>

int
__libc_alloca_cutoff (size_t size)
{
  if (! SINGLE_THREAD_P)
    {
      pthread_descr self = thread_self ();
      return size <= LIBC_THREAD_GETMEM (self, p_alloca_cutoff);
    }

  return size <= __MAX_ALLOCA_CUTOFF;
}
