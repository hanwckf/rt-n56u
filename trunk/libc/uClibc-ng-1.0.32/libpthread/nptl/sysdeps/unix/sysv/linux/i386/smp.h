/* Determine whether the host has multiple processors.  Linux version.
   Copyright (C) 1996, 2002, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/utsname.h>
#include <not-cancel.h>

/* Test whether the machine has more than one processor.  This is not the
   best test but good enough.  More complicated tests would require `malloc'
   which is not available at that time.  */
static inline int
is_smp_system (void)
{
  union
  {
    struct utsname uts;
    char buf[512];
  } u;
  char *cp;

  /* Try reading the number using `sysctl' first.  */
  if (uname (&u.uts) == 0)
    cp = u.uts.version;
  else
    {
      /* This was not successful.  Now try reading the /proc filesystem.  */
      int fd = open_not_cancel_2 ("/proc/sys/kernel/version", O_RDONLY);
      if (__builtin_expect (fd, 0) == -1
	  || read_not_cancel (fd, u.buf, sizeof (u.buf)) <= 0)
	/* This also didn't work.  We give up and say it's a UP machine.  */
	u.buf[0] = '\0';

      close_not_cancel_no_status (fd);
      cp = u.buf;
    }

  return strstr (cp, "SMP") != NULL;
}
