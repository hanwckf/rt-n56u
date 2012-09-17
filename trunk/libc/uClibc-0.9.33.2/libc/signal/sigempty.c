/* Copyright (C) 1991,96,97,2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#include <string.h>


/* Clear all signals from SET.  */
int sigemptyset (sigset_t *set)
{
#if 0 /* is it really required by standards?! */
  if (set == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }
#endif

  __sigemptyset (set);

  return 0;
}
libc_hidden_def(sigemptyset)
