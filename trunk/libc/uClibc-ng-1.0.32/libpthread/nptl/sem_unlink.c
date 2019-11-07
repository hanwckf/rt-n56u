/* Copyright (C) 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "semaphoreP.h"


int
sem_unlink (
     const char *name)
{
  char *fname;
  size_t namelen;

  /* Determine where the shmfs is mounted.  */
  INTUSE(__pthread_once) (&__namedsem_once, __where_is_shmfs);

  /* If we don't know the mount points there is nothing we can do.  Ever.  */
  if (mountpoint.dir == NULL)
    {
      __set_errno (ENOSYS);
      return -1;
    }

  /* Construct the filename.  */
  while (name[0] == '/')
    ++name;

  if (name[0] == '\0')
    {
      /* The name "/" is not supported.  */
      __set_errno (ENOENT);
      return -1;
    }
  namelen = strlen (name);

  /* Create the name of the file.  */
  fname = (char *) alloca (mountpoint.dirlen + namelen + 1);
  mempcpy (mempcpy (fname, mountpoint.dir, mountpoint.dirlen),
	     name, namelen + 1);

  /* Now try removing it.  */
  int ret = unlink (fname);
  if (ret < 0 && errno == EPERM)
    __set_errno (EACCES);
  return ret;
}
