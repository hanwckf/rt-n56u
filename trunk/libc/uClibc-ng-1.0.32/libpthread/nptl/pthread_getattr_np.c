/* Copyright (C) 2002, 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "pthreadP.h"
#include <lowlevellock.h>
#include <ldsodefs.h>


int
pthread_getattr_np (
     pthread_t thread_id,
     pthread_attr_t *attr)
{
  struct pthread *thread = (struct pthread *) thread_id;
  struct pthread_attr *iattr = (struct pthread_attr *) attr;
  int ret = 0;

  lll_lock (thread->lock, LLL_PRIVATE);

  /* The thread library is responsible for keeping the values in the
     thread desriptor up-to-date in case the user changes them.  */
  memcpy (&iattr->schedparam, &thread->schedparam,
	  sizeof (struct sched_param));
  iattr->schedpolicy = thread->schedpolicy;

  /* Clear the flags work.  */
  iattr->flags = thread->flags;

  /* The thread might be detached by now.  */
  if (IS_DETACHED (thread))
    iattr->flags |= ATTR_FLAG_DETACHSTATE;

  /* This is the guardsize after adjusting it.  */
  iattr->guardsize = thread->reported_guardsize;

  /* The sizes are subject to alignment.  */
  if (__builtin_expect (thread->stackblock != NULL, 1))
    {
      iattr->stacksize = thread->stackblock_size;
      iattr->stackaddr = (char *) thread->stackblock + iattr->stacksize;
    }
  else
    {
      /* No stack information available.  This must be for the initial
	 thread.  Get the info in some magical way.  */

      /* Stack size limit.  */
      struct rlimit rl;

      /* The safest way to get the top of the stack is to read
	 /proc/self/maps and locate the line into which
	 __libc_stack_end falls.  */
      FILE *fp = fopen ("/proc/self/maps", "rc");
      if (fp == NULL)
	ret = errno;
      /* We need the limit of the stack in any case.  */
      else
	{
	  if (getrlimit (RLIMIT_STACK, &rl) != 0)
	    ret = errno;
	  else
	    {
	      /* We need no locking.  */
	      __fsetlocking (fp, FSETLOCKING_BYCALLER);

	      /* Until we found an entry (which should always be the case)
		 mark the result as a failure.  */
	      ret = ENOENT;

	      char *line = NULL;
	      size_t linelen = 0;
	      uintptr_t last_to = 0;

	      while (! feof_unlocked (fp))
		{
		  if (getdelim (&line, &linelen, '\n', fp) <= 0)
		    break;

		  uintptr_t from;
		  uintptr_t to;
		  if (sscanf (line, "%" SCNxPTR "-%" SCNxPTR, &from, &to) != 2)
		    continue;
		  if (from <= (uintptr_t) __libc_stack_end
		      && (uintptr_t) __libc_stack_end < to)
		    {
		      /* Found the entry.  Now we have the info we need.  */
		      iattr->stacksize = rl.rlim_cur;
		      iattr->stackaddr = (void *) to;

		      /* The limit might be too high.  */
		      if ((size_t) iattr->stacksize
			  > (size_t) iattr->stackaddr - last_to)
			iattr->stacksize = (size_t) iattr->stackaddr - last_to;

		      /* We succeed and no need to look further.  */
		      ret = 0;
		      break;
		    }
		  last_to = to;
		}

	      free (line);
	    }

	  fclose (fp);
	}
    }

  iattr->flags |= ATTR_FLAG_STACKADDR;

  if (ret == 0)
    {
      size_t size = 16;
      cpu_set_t *cpuset = NULL;

      do
	{
	  size <<= 1;

	  void *newp = realloc (cpuset, size);
	  if (newp == NULL)
	    {
	      ret = ENOMEM;
	      break;
	    }
	  cpuset = (cpu_set_t *) newp;

	  ret = __pthread_getaffinity_np (thread_id, size, cpuset);
	}
      /* Pick some ridiculous upper limit.  Is 8 million CPUs enough?  */
      while (ret == EINVAL && size < 1024 * 1024);

      if (ret == 0)
	{
	  iattr->cpuset = cpuset;
	  iattr->cpusetsize = size;
	}
      else
	{
	  free (cpuset);
	  if (ret == ENOSYS)
	    {
	      /* There is no such functionality.  */
	      ret = 0;
	      iattr->cpuset = NULL;
	      iattr->cpusetsize = 0;
	    }
	}
    }

  lll_unlock (thread->lock, LLL_PRIVATE);

  return ret;
}
