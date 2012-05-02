/* Add SIG to the calling process' signal mask.
   Copyright (C) 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define __need_NULL
#include <stddef.h>
#define __USE_GNU
#include <signal.h>

int sighold (int sig)
{
    sigset_t set;

    /* Retrieve current signal set.  */
    if (sigprocmask (SIG_SETMASK, NULL, &set) < 0)
	return -1;

    /* Add the specified signal.  */
    if (__sigaddset (&set, sig) < 0)
	return -1;

    /* Set the new mask.  */
    return sigprocmask (SIG_SETMASK, &set, NULL);
}
