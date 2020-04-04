/* Copyright (C) 2003, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <pthreadP.h>


/* Defined in pthread_setaffinity.c.  */
extern size_t __kernel_cpumask_size attribute_hidden;
extern int __determine_cpumask_size (pid_t tid);
libpthread_hidden_proto(__determine_cpumask_size)

int
pthread_attr_setaffinity_np (pthread_attr_t *attr, size_t cpusetsize,
				const cpu_set_t *cpuset)
{
  struct pthread_attr *iattr;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  if (cpuset == NULL || cpusetsize == 0)
    {
      free (iattr->cpuset);
      iattr->cpuset = NULL;
      iattr->cpusetsize = 0;
    }
  else
    {
      if (__kernel_cpumask_size == 0)
	{
	  int res = __determine_cpumask_size (THREAD_SELF->tid);
	  if (res != 0)
	    /* Some serious problem.  */
	    return res;
	}

      /* Check whether the new bitmask has any bit set beyond the
	 last one the kernel accepts.  */
      size_t cnt;
      for (cnt = __kernel_cpumask_size; cnt < cpusetsize; ++cnt)
	if (((char *) cpuset)[cnt] != '\0')
	  /* Found a nonzero byte.  This means the user request cannot be
	     fulfilled.  */
	  return EINVAL;

      if (iattr->cpusetsize != cpusetsize)
	{
	  void *newp = (cpu_set_t *) realloc (iattr->cpuset, cpusetsize);
	  if (newp == NULL)
	    return ENOMEM;

	  iattr->cpuset = newp;
	  iattr->cpusetsize = cpusetsize;
	}

      memcpy (iattr->cpuset, cpuset, cpusetsize);
    }

  return 0;
}
