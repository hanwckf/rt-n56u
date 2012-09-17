/* Copyright (C) 1991, 1997, 2002 Free Software Foundation, Inc.
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

#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>


/* Return the foreground process group ID of FD.  */
pid_t tcgetpgrp (int fd)
{
  pid_t pgrp;

  if (ioctl (fd, TIOCGPGRP, &pgrp) < 0)
    return -1;
  return pgrp;
}
libc_hidden_def (tcgetpgrp)
