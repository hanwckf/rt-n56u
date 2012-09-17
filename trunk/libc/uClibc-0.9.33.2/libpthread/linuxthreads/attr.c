/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Handling of thread attributes */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/resource.h>
#include "pthread.h"
#include "internals.h"


int __pthread_attr_init(pthread_attr_t *attr)
{
  size_t ps = __getpagesize ();

  attr->__detachstate = PTHREAD_CREATE_JOINABLE;
  attr->__schedpolicy = SCHED_OTHER;
  attr->__schedparam.sched_priority = 0;
  attr->__inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr->__scope = PTHREAD_SCOPE_SYSTEM;
#ifdef NEED_SEPARATE_REGISTER_STACK
  attr->__guardsize = ps + ps;
#else
  attr->__guardsize = ps;
#endif
  attr->__stackaddr = NULL;
  attr->__stackaddr_set = 0;
  attr->__stacksize = STACK_SIZE - ps;
  return 0;
}
strong_alias (__pthread_attr_init, pthread_attr_init)

int __pthread_attr_destroy(pthread_attr_t *attr)
{
  return 0;
}
strong_alias (__pthread_attr_destroy, pthread_attr_destroy)

int __pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
  if (detachstate < PTHREAD_CREATE_JOINABLE ||
      detachstate > PTHREAD_CREATE_DETACHED)
    return EINVAL;
  attr->__detachstate = detachstate;
  return 0;
}
strong_alias (__pthread_attr_setdetachstate, pthread_attr_setdetachstate)

int __pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
  *detachstate = attr->__detachstate;
  return 0;
}
strong_alias (__pthread_attr_getdetachstate, pthread_attr_getdetachstate)

int __pthread_attr_setschedparam(pthread_attr_t *attr,
                                 const struct sched_param *param)
{
  int max_prio = __sched_get_priority_max(attr->__schedpolicy);
  int min_prio = __sched_get_priority_min(attr->__schedpolicy);

  if (param->sched_priority < min_prio || param->sched_priority > max_prio)
    return EINVAL;
  memcpy (&attr->__schedparam, param, sizeof (struct sched_param));
  return 0;
}
strong_alias (__pthread_attr_setschedparam, pthread_attr_setschedparam)

int __pthread_attr_getschedparam(const pthread_attr_t *attr,
                                 struct sched_param *param)
{
  memcpy (param, &attr->__schedparam, sizeof (struct sched_param));
  return 0;
}
strong_alias (__pthread_attr_getschedparam, pthread_attr_getschedparam)

int __pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
  if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR)
    return EINVAL;
  attr->__schedpolicy = policy;
  return 0;
}
strong_alias (__pthread_attr_setschedpolicy, pthread_attr_setschedpolicy)

int __pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
  *policy = attr->__schedpolicy;
  return 0;
}
strong_alias (__pthread_attr_getschedpolicy, pthread_attr_getschedpolicy)

int __pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
  if (inherit != PTHREAD_INHERIT_SCHED && inherit != PTHREAD_EXPLICIT_SCHED)
    return EINVAL;
  attr->__inheritsched = inherit;
  return 0;
}
strong_alias (__pthread_attr_setinheritsched, pthread_attr_setinheritsched)

int __pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit)
{
  *inherit = attr->__inheritsched;
  return 0;
}
strong_alias (__pthread_attr_getinheritsched, pthread_attr_getinheritsched)

int __pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
  switch (scope) {
  case PTHREAD_SCOPE_SYSTEM:
    attr->__scope = scope;
    return 0;
  case PTHREAD_SCOPE_PROCESS:
    return ENOTSUP;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_attr_setscope, pthread_attr_setscope)

int __pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
  *scope = attr->__scope;
  return 0;
}
strong_alias (__pthread_attr_getscope, pthread_attr_getscope)

int __pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
  /* The guard size must not be larger than the stack itself */
  if (guardsize >= attr->__stacksize) return EINVAL;

  attr->__guardsize = guardsize;

  return 0;
}
weak_alias (__pthread_attr_setguardsize, pthread_attr_setguardsize)

int __pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
  *guardsize = attr->__guardsize;
  return 0;
}
weak_alias (__pthread_attr_getguardsize, pthread_attr_getguardsize)

#if 0 /* uClibc: deprecated stuff disabled */
int __pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
  attr->__stackaddr = stackaddr;
  attr->__stackaddr_set = 1;
  return 0;
}
weak_alias (__pthread_attr_setstackaddr, pthread_attr_setstackaddr)

link_warning (pthread_attr_setstackaddr,
	      "the use of `pthread_attr_setstackaddr' is deprecated, use `pthread_attr_setstack'")

int __pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
  /* XXX This function has a stupid definition.  The standard specifies
     no error value but what is if no stack address was set?  We simply
     return the value we have in the member.  */
  *stackaddr = attr->__stackaddr;
  return 0;
}
weak_alias (__pthread_attr_getstackaddr, pthread_attr_getstackaddr)

link_warning (pthread_attr_getstackaddr,
	      "the use of `pthread_attr_getstackaddr' is deprecated, use `pthread_attr_getstack'")
#endif


int __pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
#ifdef FLOATING_STACKS
  /* We have to check against the maximum allowed stack size.  This is no
     problem if the manager is already started and we determined it.  If
     this hasn't happened, we have to find the limit outself.  */
  if (__pthread_max_stacksize == 0)
    __pthread_init_max_stacksize ();

  if (stacksize > __pthread_max_stacksize)
    return EINVAL;
#else
  /* We have a fixed size limit.  */
  if (stacksize > STACK_SIZE)
    return EINVAL;
#endif

  /* We don't accept value smaller than PTHREAD_STACK_MIN.  */
  if (stacksize < PTHREAD_STACK_MIN)
    return EINVAL;

  attr->__stacksize = stacksize;
  return 0;
}

#if PTHREAD_STACK_MIN == 16384 || defined __UCLIBC__
weak_alias (__pthread_attr_setstacksize, pthread_attr_setstacksize)
#else
versioned_symbol (libpthread, __pthread_attr_setstacksize,
                  pthread_attr_setstacksize, GLIBC_2_3_3);

# if SHLIB_COMPAT(libpthread, GLIBC_2_1, GLIBC_2_3_3)

int __old_pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
#  ifdef FLOATING_STACKS
  /* We have to check against the maximum allowed stack size.  This is no
     problem if the manager is already started and we determined it.  If
     this hasn't happened, we have to find the limit outself.  */
  if (__pthread_max_stacksize == 0)
    __pthread_init_max_stacksize ();

  if (stacksize > __pthread_max_stacksize)
    return EINVAL;
#  else
  /* We have a fixed size limit.  */
  if (stacksize > STACK_SIZE)
    return EINVAL;
#  endif

  /* We don't accept value smaller than old PTHREAD_STACK_MIN.  */
  if (stacksize < 16384)
    return EINVAL;

  attr->__stacksize = stacksize;
  return 0;
}
compat_symbol (libpthread, __old_pthread_attr_setstacksize,
	       pthread_attr_setstacksize, GLIBC_2_1);
# endif
#endif


int __pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
  *stacksize = attr->__stacksize;
  return 0;
}
weak_alias (__pthread_attr_getstacksize, pthread_attr_getstacksize)

int __pthread_attr_setstack (pthread_attr_t *attr, void *stackaddr,
			     size_t stacksize)
{
  int err;

  if ((((uintptr_t) stackaddr)
       & (__alignof__ (struct _pthread_descr_struct) - 1)) != 0)
    err = EINVAL;
  else
    err = __pthread_attr_setstacksize (attr, stacksize);
  if (err == 0)
    {
#ifndef _STACK_GROWS_UP
      attr->__stackaddr = (char *) stackaddr + stacksize;
#else
      attr->__stackaddr = stackaddr;
#endif
      attr->__stackaddr_set = 1;
    }

  return err;
}

#if PTHREAD_STACK_MIN == 16384 || defined __UCLIBC__
weak_alias (__pthread_attr_setstack, pthread_attr_setstack)
#else
versioned_symbol (libpthread, __pthread_attr_setstack, pthread_attr_setstack,
                  GLIBC_2_3_3);
# if SHLIB_COMPAT(libpthread, GLIBC_2_2, GLIBC_2_3_3)
int __old_pthread_attr_setstack (pthread_attr_t *attr, void *stackaddr,
				 size_t stacksize)
{
  int err;

  if ((((uintptr_t) stackaddr)
       & (__alignof__ (struct _pthread_descr_struct) - 1)) != 0)
    err = EINVAL;
  else
    err = __old_pthread_attr_setstacksize (attr, stacksize);
  if (err == 0)
    {
#  ifndef _STACK_GROWS_UP
      attr->__stackaddr = (char *) stackaddr + stacksize;
#  else
      attr->__stackaddr = stackaddr;
#  endif
      attr->__stackaddr_set = 1;
    }

  return err;
}

compat_symbol (libpthread, __old_pthread_attr_setstack, pthread_attr_setstack,
               GLIBC_2_2);

# endif
#endif

int __pthread_attr_getstack (const pthread_attr_t *attr, void **stackaddr,
			     size_t *stacksize)
{
  /* XXX This function has a stupid definition.  The standard specifies
     no error value but what is if no stack address was set?  We simply
     return the value we have in the member.  */
#ifndef _STACK_GROWS_UP
  *stackaddr = (char *) attr->__stackaddr - attr->__stacksize;
#else
  *stackaddr = attr->__stackaddr;
#endif
  *stacksize = attr->__stacksize;
  return 0;
}
weak_alias (__pthread_attr_getstack, pthread_attr_getstack)

int pthread_getattr_np (pthread_t thread, pthread_attr_t *attr)
{
  pthread_handle handle = thread_handle (thread);
  pthread_descr descr;
  int ret = 0;

  if (handle == NULL)
    return ENOENT;

  descr = handle->h_descr;

  attr->__detachstate = (descr->p_detached
			 ? PTHREAD_CREATE_DETACHED
			 : PTHREAD_CREATE_JOINABLE);

  attr->__schedpolicy = __sched_getscheduler (descr->p_pid);
  if (attr->__schedpolicy == -1)
    return errno;

  if (__sched_getparam (descr->p_pid,
			(struct sched_param *) &attr->__schedparam) != 0)
    return errno;

  attr->__inheritsched = descr->p_inheritsched;
  attr->__scope = PTHREAD_SCOPE_SYSTEM;

#ifdef _STACK_GROWS_DOWN
# ifdef USE_TLS
  attr->__stacksize = descr->p_stackaddr - (char *)descr->p_guardaddr
		      - descr->p_guardsize;
# else
  attr->__stacksize = (char *)(descr + 1) - (char *)descr->p_guardaddr
		      - descr->p_guardsize;
# endif
#else
# ifdef USE_TLS
  attr->__stacksize = (char *)descr->p_guardaddr - descr->p_stackaddr;
# else
  attr->__stacksize = (char *)descr->p_guardaddr - (char *)descr;
# endif
#endif
  attr->__guardsize = descr->p_guardsize;
  attr->__stackaddr_set = descr->p_userstack;
#ifdef NEED_SEPARATE_REGISTER_STACK
  if (descr->p_userstack == 0)
    attr->__stacksize *= 2;
  /* XXX This is awkward.  The guard pages are in the middle of the
     two stacks.  We must count the guard size in the stack size since
     otherwise the range of the stack area cannot be computed.  */
  attr->__stacksize += attr->__guardsize;
#endif
#ifdef USE_TLS
  attr->__stackaddr = descr->p_stackaddr;
#else
# ifndef _STACK_GROWS_UP
  attr->__stackaddr = (char *)(descr + 1);
# else
  attr->__stackaddr = (char *)descr;
# endif
#endif

#ifdef USE_TLS
  if (attr->__stackaddr == NULL)
#else
  if (descr == &__pthread_initial_thread)
#endif
    {
      /* Stack size limit.  */
      struct rlimit rl;

      /* The safest way to get the top of the stack is to read
	 /proc/self/maps and locate the line into which
	 __libc_stack_end falls.  */
      FILE *fp = fopen ("/proc/self/maps", "rc");
      if (fp == NULL)
	ret = errno;
      /* We need the limit of the stack in any case.  */
      else if (getrlimit (RLIMIT_STACK, &rl) != 0)
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
		  attr->__stacksize = rl.rlim_cur;
#ifdef _STACK_GROWS_UP
		  /* Don't check to enforce a limit on the __stacksize */
		  attr->__stackaddr = (void *) from;
#else
		  attr->__stackaddr = (void *) to;

		  /* The limit might be too high.  */
		  if ((size_t) attr->__stacksize
		      > (size_t) attr->__stackaddr - last_to)
		    attr->__stacksize = (size_t) attr->__stackaddr - last_to;
#endif

		  /* We succeed and no need to look further.  */
		  ret = 0;
		  break;
		}
	      last_to = to;
	    }

	  fclose (fp);
	  free (line);
	}
    }

  return 0;

}
