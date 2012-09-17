/* Determine whether the host has multiple processors.  Linux version.
   Copyright (C) 1996, 2002 Free Software Foundation, Inc.
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
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sys/sysctl.h>

/* Test whether the machine has more than one processor.  This is not the
   best test but good enough.  More complicated tests would require `malloc'
   which is not available at that time.  */
static __inline__ int
is_smp_system (void)
{
  static const int sysctl_args[] = { CTL_KERN, KERN_VERSION };
  char buf[512];
  size_t reslen = sizeof (buf);

  /* Try reading the number using `sysctl' first.  */
  if (__sysctl ((int *) sysctl_args,
		sizeof (sysctl_args) / sizeof (sysctl_args[0]),
		buf, &reslen, NULL, 0) < 0)
    {
      /* This was not successful.  Now try reading the /proc filesystem.  */
      int fd = __open ("/proc/sys/kernel/version", O_RDONLY);
      if (__builtin_expect (fd, 0) == -1
	  || (reslen = __read (fd, buf, sizeof (buf))) <= 0)
	/* This also didn't work.  We give up and say it's a UP machine.  */
	buf[0] = '\0';

      __close (fd);
    }

  return strstr (buf, "SMP") != NULL;
}
